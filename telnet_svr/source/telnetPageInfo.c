
//-----------------------------------------------------------------------------
// File:    osalbasic.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
//-----------------------------------------------------------------------------

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//-----------------------------------------------------------------------------

#include "../include/telnet.h"
#include "../include/telnetpageinfo.h"

extern T_TELNET_VTY   sgtTelnetVty;
static char  more_page[] = "--------MORE PAGE--------";
static char  result_data_buffer[MAX_RESULT_DATA_LEN];


static int32_t char_is_chinese(char  ucIn)
{
	return ((ucIn & 0x80) == 0x80);
}


static void telnet_putline(T_Page_Info  *ptPageInfo, char  *text)
{
    DISPLAY_NODE *pnewnode = NULL, *plastnode = NULL;

    if (NULL == ptPageInfo || NULL == text || strlen(text) == 0)
    {
        return;
    }

    if ((ptPageInfo->disp_line < OAM_STY_PAGE_LINE) &&
         (NULL == ptPageInfo->p_disp_head))
    {
		telnet_Inner_print(text);

        if(text[strlen(text) - 1] == '\n')
        {
            telnet_Inner_print("\r");
        }
        else
        {
            telnet_Inner_print("\n\r");
        }
    }
    else
    {
        pnewnode = (DISPLAY_NODE *)osal_mem_alloc(sizeof(DISPLAY_NODE));
        if (!pnewnode)
        {
            return;
        }
        memset(pnewnode, 0, sizeof(DISPLAY_NODE));
        pnewnode->disp_info = (char  *)osal_mem_alloc(strlen(text) + 1);
        if (!pnewnode->disp_info)
        {
            osal_mem_free(pnewnode);
            return;
        }
        memset(pnewnode->disp_info, 0, strlen(text) + 1);
        memcpy(pnewnode->disp_info, text, strlen(text));
        pnewnode->p_next = NULL;

        if (NULL == ptPageInfo->p_disp_head)
        {
            telnet_Inner_print(more_page);
            ptPageInfo->p_disp_head= pnewnode;
        }
        else
        {
            plastnode = ptPageInfo->p_disp_head;
            while (plastnode->p_next != NULL)
            {
                plastnode = plastnode->p_next;
            }

			plastnode->p_next = pnewnode;
        }
    }
}


uint16_t sdp_puts(T_TELNET_VTY *ptTelnetVty, char  *text)
{
    int str_len = 0;
    char  *p = NULL;
	T_Page_Info  *ptPageInfo = NULL;
    char  tmptext[OAM_STY_OUTPUT_LINE_MAX * 2] = {0};

    if (NULL == ptTelnetVty || NULL == text)
    {
        return 0;
    }

    p=text;
	ptPageInfo = &ptTelnetVty->tPageInfo;

    str_len = 0;
    while (*p != 0)
    {
		if ('\r' == *p)
		{
			p++;
			continue;
		}
        tmptext[str_len] = *p;
        if(*p == MARK_LEFT)
        {
            if(ptPageInfo->fill_len  > 0) ptPageInfo->fill_len--;
        }
        else
        {
            if (char_is_chinese(*p))
            {
                if (ptPageInfo->fill_len == OAM_STY_OUTPUT_LINE_MAX)
                {
                    p--;
                    tmptext[str_len] = ' ';
                }
                else
                {
                    p++;
                    str_len++;
                    tmptext[str_len] = *p;
                    if(*p == MARK_LEFT)
                    {
						if(ptPageInfo->fill_len  > 0)
						{
							ptPageInfo->fill_len--;
						}
                        else
                        {
                            ptPageInfo->fill_len ++;
                        }
                    }
                }
            }
            ptPageInfo->fill_len ++;
        }
        str_len ++;

        if ((*p == '\n') || (ptPageInfo->fill_len >= OAM_STY_OUTPUT_LINE_MAX))
        {
            tmptext[str_len] = MARK_END;
            str_len = 0;
            ptPageInfo->disp_line ++;

            telnet_putline(ptPageInfo, tmptext);
            ptPageInfo->fill_len = 0;
        }
        p ++;
    }

	if((ptPageInfo->fill_len == str_len) && (str_len))
    {
        ptPageInfo->disp_line ++;
    }
    tmptext[str_len] = MARK_END;
    telnet_putline(ptPageInfo, tmptext);
	ptPageInfo->fill_len = 0;

    if(ptPageInfo->disp_line > OAM_STY_PAGE_LINE)
    {
        return 0;
    }

    return 1;
}


void OutputExecResult(uint8_t* pMsgPara, uint16_t data_len)
{
    uint16_t  result_len = 0;

	if (NULL == pMsgPara)
	{
		return;
	}

	result_len = (MAX_RESULT_DATA_LEN - 1) > data_len ? data_len : MAX_RESULT_DATA_LEN - 1;
	memset(result_data_buffer, 0, sizeof(result_data_buffer));
	memcpy(result_data_buffer, (char  *)pMsgPara, result_len);
    result_data_buffer[result_len] = '\0';

    sdp_puts(&sgtTelnetVty, result_data_buffer);

	return;
}

void oam_sty_page(T_TELNET_VTY *ptTelnetVty, char  c)
{
    uint16_t i = 0;
    DISPLAY_NODE *p_node = NULL, *release_node = NULL;
    uint16_t wLen = 0;
	T_Page_Info  *ptPageInfo = NULL;

    if (NULL == ptTelnetVty)
    {
		return;
    }

	ptPageInfo = &ptTelnetVty->tPageInfo;

    p_node = ptPageInfo->p_disp_head;

    wLen = strlen(more_page);
    for (i = 0; i < wLen; i ++)
    {
        telnet_Inner_print(CUT_CHAR);
    }

    switch(c)
    {
    case ' ':
    case '\r':
    case '\n':
        if (' ' == c)
        {
            ptPageInfo->disp_line = 1;
        }

        for (i = 1; i < OAM_STY_PAGE_LINE; i ++)
        {
            uint16_t wDispLen = 0;

            if (p_node == NULL || p_node->disp_info == NULL)
            {
                break;
            }

            wDispLen = strlen(p_node->disp_info);

            ptPageInfo->disp_line ++;
            if ((p_node->disp_info != NULL) && (wDispLen >= 1))
            {
                telnet_Inner_print(p_node->disp_info);
                if(*(p_node->disp_info + wDispLen - 1) != '\n')
                {
                    telnet_Inner_print("\n");
                }

                telnet_Inner_print("\r");
                osal_mem_free(p_node->disp_info);
            }
            release_node = p_node;
            p_node = p_node->p_next;
            ptPageInfo->p_disp_head = p_node;
            osal_mem_free(release_node);

            if (NULL == p_node)
            {
                break;
            }

            if(c != ' ')
            {
                break;
            }
        }

        ptPageInfo->p_disp_head = p_node;
        if (p_node == NULL)
        {
			ptPageInfo->disp_line = 1;
			telnet_Inner_print(ptTelnetVty->strMode);
        }
        else
        {
            telnet_Inner_print(more_page);
        }
        return;

	case 3:
		while (p_node != NULL)
        {
            if (p_node->disp_info)
            {
                osal_mem_free(p_node->disp_info);
            }
            release_node = p_node;
            p_node = p_node->p_next;
            osal_mem_free(release_node);
        }
        ptPageInfo->disp_line = 1;
        ptPageInfo->p_disp_head = NULL;

        telnet_Inner_print(ptTelnetVty->strMode);
		return;
    default:
        return;
    }
}

