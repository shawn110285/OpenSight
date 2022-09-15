
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
#include "../include/telnetfunc.h"

extern void oam_sty_page(T_TELNET_VTY *ptTelnetVty, char  c);
extern int32_t CombCmdAndPara(T_Telnet_Command *ptCmd, char *pChar);
extern int32_t SplitCmdLine(char *pChar, T_TELNET_WORD *ptWord);

static char strBanner[]="\r\n\
********************************************************************\r\n\
          Welcome to OpenSight  \r\n\
          help/?-------get help \r\n\
********************************************************************\r\n";

T_TELNET_VTY   sgtTelnetVty;

static  char   telnet_backward_char = 0x08;
static  char   telnet_space_char = ' ';

#define oam_sty_ring_full(ptTelnetVty) \
((ptTelnetVty)->ring_end - (ptTelnetVty)->ring_start + 1 == 0)
#define oam_sty_ring_empty(ptTelnetVty) \
  ((ptTelnetVty)->ring_start == (ptTelnetVty)->ring_end)

static void telnet_read_loop(T_TELNET_VTY *ptTelnetVty);
static int  telnet_getch(T_TELNET_VTY *ptTelnetVty);


void SendNegoEcho(char  cNegoEcho1, char  cNegoEcho2)
{
    char  aucNegoEcho[3] = {0};

    aucNegoEcho[0] = (char )IAC;
    aucNegoEcho[1] = cNegoEcho1;
    aucNegoEcho[2] = cNegoEcho2;

    telnet_send(aucNegoEcho, sizeof(aucNegoEcho));
}


void tnInit()
{
    init_tel_vty(&sgtTelnetVty);

	SendNegoEcho(WILL,(char ) TELOPT_ECHO);
	SendNegoEcho(DONT, (char )TELOPT_ECHO);
	SendNegoEcho(WILL, (char )TELOPT_SGA);
    SendNegoEcho(DONT, (char )TELOPT_SGA);

    osal_task_delay(1000);

    telnet_Inner_print("\r\n login:");
}


void vty_out(T_TELNET_VTY *ptTelnetVty, char cmd[], int len)
{
     telnet_send(cmd,(uint16_t)len);
}


void init_tel_vty(T_TELNET_VTY *ptTelnetVty)
{
    int i=0;

	memset(sgtTelnetVty.ring_buf, 0, OAM_STY_RING_SIZE);
    sgtTelnetVty.ring_end = sgtTelnetVty.ring_start = 0;

	ptTelnetVty->ucLoginFlag = 0;
	ptTelnetVty->ucUserNameFlag = 0;
	ptTelnetVty->ucPasswdFlag = 0;
    ptTelnetVty->ucTryNum = 0;

//	ptTelnetVty->ucRole = 0;
	memset(ptTelnetVty->aucUserName, 0, TELNET_USER_NAME_LENGTH);
	memset(ptTelnetVty->aucPasswd, 0, TELNET_PASSWD_LENGTH);
	memset(ptTelnetVty->aucConfirmPasswd, 0 , TELNET_PASSWD_LENGTH);
    memset(ptTelnetVty->strCurrentCmd, 0, TELNET_MAX_CMD_LENGTH);

    ptTelnetVty->dwCurrPos = 0;

    ptTelnetVty->dwCmdLength = 0;

    for (i = 0; i< TELNET_MAX_HISTORY_CMD; i++)
    {
        memset(ptTelnetVty->strHistoryCmd[i],0,TELNET_MAX_CMD_LENGTH);
    }
    ptTelnetVty->dwCurrHistCmdLineNum = 0;
    ptTelnetVty->dwHistCmdTotalNum = 0;

	ptTelnetVty->wWorkMode = CMD_MODE_INVALID;
	memset(ptTelnetVty->strMode, 0, TELNET_MAX_MODE_LENGTH);

	ptTelnetVty->ucCurrRunState = Telnet_MsgHandle_INIT;
//	ptTelnetVty->ucPwdStatus = PWD_IDLE;

	memset(&ptTelnetVty->tPageInfo, 0, sizeof(T_Page_Info));
	ptTelnetVty->ucLocalPidExec = 1;
}


void vty_clear(T_TELNET_VTY *ptTelnetVty)
{
    memset(ptTelnetVty->strCurrentCmd,0, ptTelnetVty->dwCmdLength);
    ptTelnetVty->dwCmdLength = 0;
	ptTelnetVty->dwCurrPos   = 0;
}


void vty_backsapce(T_TELNET_VTY *ptTelnetVty)
{
	int i,j,step;
	char cur_char;

	if(0 >= ptTelnetVty->dwCurrPos)
		return;

	if(ptTelnetVty->dwCurrPos < ptTelnetVty->dwCmdLength)
	{
		step = ptTelnetVty->dwCmdLength - ptTelnetVty->dwCurrPos;
		for(i=ptTelnetVty->dwCurrPos;i<ptTelnetVty->dwCmdLength;i++)
		{
			ptTelnetVty->strCurrentCmd[i-1]=ptTelnetVty->strCurrentCmd[i];

			cur_char=ptTelnetVty->strCurrentCmd[i];
			if(i==ptTelnetVty->dwCurrPos)
				vty_out(ptTelnetVty, &telnet_backward_char, 1);
			vty_out(ptTelnetVty, &cur_char, 1);
		}

		vty_out(ptTelnetVty, &telnet_space_char, 1);
		vty_out(ptTelnetVty, &telnet_backward_char, 1);
		for(j=0;j<step;j++)
		{
			vty_out(ptTelnetVty, &telnet_backward_char, 1);
		}

		ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCmdLength-1]=0;

		ptTelnetVty->dwCmdLength--;
		ptTelnetVty->dwCurrPos--;

		return;
	}

    if(ptTelnetVty->dwCmdLength > 0)
    {
		ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCmdLength-1]=0;
		ptTelnetVty->dwCmdLength--;
		ptTelnetVty->dwCurrPos--;
		vty_out(ptTelnetVty, CUT_CHAR, 3);
    }
}


void vty_delete(T_TELNET_VTY *ptTelnetVty)
{
	int i,j,step;
	char cur_char;

	if(ptTelnetVty->dwCurrPos >= ptTelnetVty->dwCmdLength)
		return;

	if(0 >= ptTelnetVty->dwCmdLength)
		return;

	step = ptTelnetVty->dwCmdLength - ptTelnetVty->dwCurrPos;

	if(1 == step)
	{
		ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCmdLength-1]=0;
		ptTelnetVty->dwCmdLength--;
		vty_out(ptTelnetVty, &telnet_space_char, 1);
		vty_out(ptTelnetVty, &telnet_backward_char, 1);
		return;
	}

	for(i = ptTelnetVty->dwCurrPos; i <= ptTelnetVty->dwCmdLength - 2; i++)
	{
		ptTelnetVty->strCurrentCmd[i] = ptTelnetVty->strCurrentCmd[i+1];

		cur_char=ptTelnetVty->strCurrentCmd[i+1];
		vty_out(ptTelnetVty, &cur_char, 1);
	}

	vty_out(ptTelnetVty, &telnet_space_char, 1);
	vty_out(ptTelnetVty, &telnet_backward_char, 1);

	for(j = 0; j < step - 1; j++)
	{
		vty_out(ptTelnetVty, &telnet_backward_char, 1);
	}

	ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCmdLength-1] = 0;

	ptTelnetVty->dwCmdLength--;
}


void vty_left_char(T_TELNET_VTY *ptTelnetVty)
{
    if(ptTelnetVty->dwCurrPos <= 0)
		return;

	ptTelnetVty->dwCurrPos--;
	vty_out(ptTelnetVty, &telnet_backward_char, 1);
}


void vty_right_char(T_TELNET_VTY *ptTelnetVty)
{
	char cur_char;

    if(ptTelnetVty->dwCurrPos >= ptTelnetVty->dwCmdLength)
		return;

	cur_char = ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCurrPos];
	ptTelnetVty->dwCurrPos ++;

	vty_out(ptTelnetVty, &cur_char, 1);
}


void vty_insert(T_TELNET_VTY *ptTelnetVty,char cc)
{
	int i,j,step;
	char cur_char;

	if(ptTelnetVty->dwCurrPos >= ptTelnetVty->dwCmdLength)
		return;

	step = ptTelnetVty->dwCmdLength - ptTelnetVty->dwCurrPos;

	for(i = ptTelnetVty->dwCurrPos + 1; i < ptTelnetVty->dwCmdLength + 1; i++)
	{
		cur_char = ptTelnetVty->strCurrentCmd[i-1];
		if(i == ptTelnetVty->dwCurrPos+1)
			vty_out(ptTelnetVty, &cc, 1);
		vty_out(ptTelnetVty, &cur_char, 1);
	}

	for(i = ptTelnetVty->dwCmdLength; i > ptTelnetVty->dwCurrPos; i--)
	{
		ptTelnetVty->strCurrentCmd[i] = ptTelnetVty->strCurrentCmd[i-1];
	}
	ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCurrPos] = cc;


	for(j = 0;j < step; j++)
	{
		vty_out(ptTelnetVty, &telnet_backward_char, 1);
	}

	ptTelnetVty->dwCmdLength++;
	ptTelnetVty->dwCurrPos++;
}


void vty_question(T_TELNET_VTY *ptTelnetVty, char cc)
{
	int i = 0;
	int32_t bIsNormalChar = OSAL_FALSE;
	int   iCnt = 0;
	char *pChar = NULL;
	T_Telnet_Command *ptCmd = NULL;
	uint16_t wCmdCnt = 0;
	char *acuCmdName[TELNET_MAX_TAB_CMD] = { 0 };
	T_Telnet_Command *ptOneCmd = NULL;
	T_TELNET_WORD  tWord;

	if (NULL == ptTelnetVty)
	{
		return;
	}

	ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCmdLength] = '\0';

	if (0 == strlen(ptTelnetVty->strCurrentCmd))
	{
		vty_insert_char(ptTelnetVty, (char )cc);
		return;
	}
	for (iCnt = strlen(ptTelnetVty->strCurrentCmd); iCnt > 0; iCnt--)
	{
		if ('\\' == ptTelnetVty->strCurrentCmd[iCnt - 1])
		{
			bIsNormalChar = bIsNormalChar ? OSAL_FALSE : OSAL_TRUE;
		}
		else
		{
			break;
		}
	}

	if (OSAL_TRUE == bIsNormalChar)
	{
		vty_insert_char(ptTelnetVty, (char )cc);
		return;
	}

	telnet_Inner_print("%c", cc);

	pChar = ptTelnetVty->strCurrentCmd;
    while (*pChar != MARK_STRINGEND && *pChar == MARK_BLANK)
        pChar ++;

    if (strlen(pChar) == 0)
    {
        return;
    }

	memset(&tWord, 0, sizeof(T_TELNET_WORD));
	if (OSAL_FALSE == SplitCmdLine(pChar, &tWord))
	{
		return;
	}

	ptCmd = g_ptCmdTab;
	if (NULL == ptCmd)
	{
		return ;
	}

	while (NULL != ptCmd)
	{
		int i = 0, iTime = 0;
		int iMinWordsNum = ((tWord.wWordNum > ptCmd->wCmdWordNum)?ptCmd->wCmdWordNum:tWord.wWordNum);

		for (i = 0; i < iMinWordsNum; i++)
		{
			if (   strlen(tWord.sWord[i]) <= strlen(ptCmd->cName[i+1]) 	&& 0 == strncasecmp(ptCmd->cName[i+1], tWord.sWord[i], strlen(tWord.sWord[i])) )
			{
				iTime ++;
			}
		}


		if (iTime == iMinWordsNum && wCmdCnt < TELNET_MAX_TAB_CMD)
		{
			acuCmdName[wCmdCnt++] = ptCmd->cName[0];
			ptOneCmd = ptCmd;

			if (strlen(tWord.sWord[0]) == strlen(ptCmd->cName[0]))
			{
				wCmdCnt = 1;
				break;
			}
		}

		ptCmd = ptCmd->ptNext;
	}

	if (0 == wCmdCnt)
    {
		telnet_Inner_print(ptTelnetVty->strMode);
		telnet_Inner_print("Command does not exist, please check it!");
		telnet_Inner_print(ptTelnetVty->strMode);
		telnet_Inner_print(ptTelnetVty->strCurrentCmd);
        return;
    }
	else if (1 == wCmdCnt)
    {
        char  cBuffer[BUFFER_LENGTH] = {0};

		memset(cBuffer, 0, BUFFER_LENGTH*sizeof(char));
		telnet_Inner_print(ptTelnetVty->strMode);

		strcat(cBuffer, "Command is \"");
		pChar = cBuffer + strlen("Command is \"");
		CombCmdAndPara(ptOneCmd, pChar);
		strcat(cBuffer, "\"!");
		telnet_Inner_print(cBuffer);

		telnet_Inner_print(ptTelnetVty->strMode);
		telnet_Inner_print(ptTelnetVty->strCurrentCmd);
		return;
    }
	else
	{
		for (i = 0; i < wCmdCnt; i ++)
		{
			telnet_Inner_print("\r\n  ");
			telnet_Inner_print(acuCmdName[i]);
		}
		telnet_Inner_print(sgtTelnetVty.strMode);
		telnet_Inner_print(ptTelnetVty->strCurrentCmd);
	}

	return;
}


void vty_insert_char(T_TELNET_VTY *ptTelnetVty, char cc)
{
	if((ptTelnetVty->dwCmdLength < TELNET_MAX_CMD_LENGTH - 1) && isascii(cc) && isprint(cc) )
	{
		if((0 == ptTelnetVty->ucPasswdFlag && 1 == ptTelnetVty->ucUserNameFlag))
		{
			ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCmdLength++] =cc;
			ptTelnetVty->dwCurrPos++;
			cc ='*';
			telnet_Inner_print("%c", cc);
		}
		else
		{
			if(ptTelnetVty->dwCurrPos >= ptTelnetVty->dwCmdLength)
			{
				ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCmdLength++] =cc;
				ptTelnetVty->dwCurrPos++;
				telnet_Inner_print("%c", cc);
			}
			else
			{
				vty_insert(ptTelnetVty,cc);
			}
		}
	}
}


void vty_history_add(T_TELNET_VTY *ptTelnetVty)
{
    int index;

    if (ptTelnetVty->dwCmdLength == 0)
        return;

    index = ptTelnetVty->dwHistCmdTotalNum;

    if(index>0)
    {
		if(strlen(ptTelnetVty->strHistoryCmd[index-1]) != 0)
		{
		   if(strncmp(ptTelnetVty->strCurrentCmd, ptTelnetVty->strHistoryCmd[index-1],TELNET_MAX_CMD_LENGTH) == 0)
		   {
			   return;
		   }
		}
    }

    ptTelnetVty->dwHistCmdTotalNum++;

    if(ptTelnetVty->dwHistCmdTotalNum > TELNET_MAX_HISTORY_CMD)
    {
		int i=0;
		for(i=0;i<ptTelnetVty->dwHistCmdTotalNum-2;i++)
		{
		   memset(ptTelnetVty->strHistoryCmd[i],0,TELNET_MAX_CMD_LENGTH);
		   strncpy(ptTelnetVty->strHistoryCmd[i],ptTelnetVty->strHistoryCmd[i+1],TELNET_MAX_CMD_LENGTH);
		}
		ptTelnetVty->dwHistCmdTotalNum = TELNET_MAX_HISTORY_CMD;
    }
    strncpy(ptTelnetVty->strHistoryCmd[ptTelnetVty->dwHistCmdTotalNum-1],ptTelnetVty->strCurrentCmd,TELNET_MAX_CMD_LENGTH);

    ptTelnetVty->dwCurrHistCmdLineNum = ptTelnetVty->dwHistCmdTotalNum;
}


void vty_next_line(T_TELNET_VTY *ptTelnetVty)
{
    int try_index;
    try_index = ptTelnetVty->dwCurrHistCmdLineNum;

    if (try_index >= (ptTelnetVty->dwHistCmdTotalNum -1) )
    {
        try_index = 0;
    }
	else
    {
         try_index++;
	}

    if(strlen(ptTelnetVty->strHistoryCmd[try_index]) == 0)
    {
        return;
    }
	else
	{
        ptTelnetVty->dwCurrHistCmdLineNum = try_index;
	}

    vty_histroy_print(ptTelnetVty);
	return;
}


void vty_previous_line(T_TELNET_VTY *ptTelnetVty)
{
    int try_index;

    try_index = ptTelnetVty->dwCurrHistCmdLineNum;

    if (try_index < 1)
    {
        try_index = ptTelnetVty->dwHistCmdTotalNum - 1;
    }
    else
    {
        try_index--;
    }

    if(strlen(ptTelnetVty->strHistoryCmd[try_index]) == 0)
        return;
    else
        ptTelnetVty->dwCurrHistCmdLineNum = try_index;

    vty_histroy_print(ptTelnetVty);
    return;
}


void vty_histroy_print(T_TELNET_VTY *ptTelnetVty)
{
   int dwCmdLength;

   char cur_char;

   while(ptTelnetVty->dwCurrPos<ptTelnetVty->dwCmdLength)
   {
	     cur_char = ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCurrPos];
	     ptTelnetVty->dwCurrPos ++;
	     vty_out(ptTelnetVty, &cur_char, 1);
   }

   while(ptTelnetVty->dwCmdLength > 0)
   {
	   ptTelnetVty->strCurrentCmd[ptTelnetVty->dwCmdLength-1]=0;
	   ptTelnetVty->dwCmdLength--;
	   ptTelnetVty->dwCurrPos--;
	   vty_out(ptTelnetVty, CUT_CHAR, 3);
   }

    dwCmdLength = strlen(ptTelnetVty->strHistoryCmd[ptTelnetVty->dwCurrHistCmdLineNum]);

    strncpy(ptTelnetVty->strCurrentCmd, ptTelnetVty->strHistoryCmd[ptTelnetVty->dwCurrHistCmdLineNum], dwCmdLength);

    ptTelnetVty->dwCurrPos = ptTelnetVty->dwCmdLength = dwCmdLength;

    vty_out(ptTelnetVty, ptTelnetVty->strCurrentCmd, ptTelnetVty->dwCmdLength);
	return;
}


int vty_complete(T_TELNET_VTY * ptTelnetVty)
{
	uint16_t wi = 0;
	char *pChar = NULL, *pCmdChar = NULL;
	T_Telnet_Command *ptCmd = NULL;
	uint16_t wCmdCnt = 0;
	char *acuCmdName[TELNET_MAX_TAB_CMD] = { 0 };
	int i = 0;

	pChar = ptTelnetVty->strCurrentCmd;
    while (pChar[wi] == ' ')
        wi ++;

    if (strlen(pChar + wi) == 0)
    {
        return OSAL_FALSE;
    }

	ptCmd = g_ptCmdTab;
	if (NULL == ptCmd)
	{
		return OSAL_FALSE;
	}

	while (NULL != ptCmd)
	{
		if (0 == strncasecmp(ptCmd->cName[0], pChar + wi, strlen(pChar + wi)))
        {
			if (wCmdCnt < TELNET_MAX_TAB_CMD)
			{
                acuCmdName[wCmdCnt++] = ptCmd->cName[0];
			}
        }
		ptCmd = ptCmd->ptNext;
	}

	if (0 == wCmdCnt)
    {
        return OSAL_FALSE;
    }
	else if (1 == wCmdCnt)
    {
		pCmdChar = acuCmdName[0];
		pCmdChar = pCmdChar + strlen(pChar + wi) ;
        strcat(ptTelnetVty->strCurrentCmd, pCmdChar);
		ptTelnetVty->dwCmdLength = ptTelnetVty->dwCmdLength + strlen(pCmdChar);
		ptTelnetVty->dwCurrPos = ptTelnetVty->dwCurrPos + strlen(pCmdChar);
		telnet_Inner_print(pCmdChar);
    }
	else
	{
		for (i = 0; i < wCmdCnt; i ++)
		{
			telnet_Inner_print("\r\n  ");
			telnet_Inner_print(acuCmdName[i]);
		}
		telnet_Inner_print(sgtTelnetVty.strMode);
		telnet_Inner_print(ptTelnetVty->strCurrentCmd);
	}

	return OSAL_TRUE;
}


void vty_excute(T_TELNET_VTY *ptTelnetVty)
{
    vty_history_add(ptTelnetVty);

    telnet_ParseAndExec_Cmd(ptTelnetVty);
	return;
}

void ExcuteTelnetCmd(T_TELNET_VTY *ptTelnetVty)
{
	switch(ptTelnetVty->ucCurrRunState)
	{
		case Telnet_MsgHandle_IDLE:
			vty_excute(ptTelnetVty);
			ExcuteToOutput(ptTelnetVty);
			break;

		case Telnet_MsgHandle_EXECUTE:
			telnet_Inner_print(ptTelnetVty->strMode);
			telnet_Inner_print("A command executing, please wait a moment!");
			telnet_Inner_print(ptTelnetVty->strMode);
			break;

		default:
			break;
	}

	vty_clear(ptTelnetVty);
	return;
}


void TelnetLogin()
{
	uint8_t ucAuthenticated = 0;

	if(sgtTelnetVty.ucUserNameFlag == 0)
	{
		strncpy(sgtTelnetVty.aucUserName, sgtTelnetVty.strCurrentCmd,TELNET_USER_NAME_LENGTH);
		sgtTelnetVty.ucUserNameFlag = 1;

		vty_clear(&sgtTelnetVty);
		telnet_Inner_print("\r\n password:");
	}
	else
	{
		strncpy(sgtTelnetVty.aucPasswd,sgtTelnetVty.strCurrentCmd,TELNET_PASSWD_LENGTH);
		sgtTelnetVty.ucPasswdFlag = 1;
		vty_clear(&sgtTelnetVty);

	    if( (0 == strncasecmp(sgtTelnetVty.aucUserName, "debug", strlen(sgtTelnetVty.aucUserName)))
		   && (0 == strncasecmp(sgtTelnetVty.aucPasswd, "debug", strlen("sgtTelnetVty.aucPasswd"))) )
		{
			ucAuthenticated = 1;
		}

		if (0 != ucAuthenticated)
		{
			sgtTelnetVty.ucLoginFlag = 1;
			telnet_Inner_print(strBanner);
			memset(sgtTelnetVty.strMode, 0, TELNET_MAX_MODE_LENGTH);
			snprintf(sgtTelnetVty.strMode,TELNET_MAX_MODE_LENGTH, "\r\n%s(debug)#",osal_get_product_name());
			telnet_Inner_print(sgtTelnetVty.strMode);
			sgtTelnetVty.ucCurrRunState = Telnet_MsgHandle_IDLE;
		}
		else
		{
			if(sgtTelnetVty.ucTryNum < 2)
			{
				sgtTelnetVty.ucUserNameFlag =0;
				sgtTelnetVty.ucPasswdFlag = 0;
				sgtTelnetVty.ucTryNum++;
				telnet_Inner_print("\r\n Invalid user or password,retry!\r\n");
				telnet_Inner_print("\r\n login:");
			}
			else
			{
				telnet_shutdown();
			}
		}
	}
	return;
}


void ExcuteToOutput(T_TELNET_VTY *ptTelnetVty)
{
	if (1 == ptTelnetVty->ucLocalPidExec)
	{
		telnet_Inner_print(ptTelnetVty->strMode);
	}
	else
	{
		telnet_Inner_print("\r\n");
		ptTelnetVty->ucLocalPidExec = 1;
	}
}


void ReadTelnet(char *buffer, uint16_t nRecvLen)
{
	uint32_t nLoop = 0;
	uint8_t  cTmp = 0;

	if (NULL == buffer)
	{
		return;
	}

	memset(sgtTelnetVty.ring_buf, 0, OAM_STY_RING_SIZE);
    sgtTelnetVty.ring_end = sgtTelnetVty.ring_start = 0;

	for (nLoop = 0; nLoop <  nRecvLen && !oam_sty_ring_full(&sgtTelnetVty); nLoop++)
    {
        cTmp = buffer[nLoop];
        sgtTelnetVty.ring_buf[sgtTelnetVty.ring_end++] = cTmp;
        if (sgtTelnetVty.ring_end >= sizeof(sgtTelnetVty.ring_buf))
        {
            sgtTelnetVty.ring_end = 0;
        }
    }
	telnet_read_loop(&sgtTelnetVty);
}

static void telnet_read_loop(T_TELNET_VTY *ptTelnetVty)
{
	uint32_t c = 0;
	static uint8_t ucPreCRLF = 0;
	T_Page_Info  *ptPageInfo = NULL;

	if (NULL == ptTelnetVty)
	{
		return;
	}

	ptPageInfo = &ptTelnetVty->tPageInfo;

	while ((c = telnet_getch(ptTelnetVty)) != -10000)
	{
		if (IAC == c)
        {
            telnet_getch(ptTelnetVty);
            telnet_getch(ptTelnetVty);
            continue;
        }

		if (ptPageInfo->p_disp_head)
        {
            oam_sty_page(ptTelnetVty, (char )c);
            return;
        }
        else
        {
            ptPageInfo->disp_line = 1;
        }

        if(1)
		{
            static char  cTmp = 0;

            if (c == 27)
            {
                cTmp = 1;
                continue;
            }

            if(1 == cTmp)
            {
                cTmp = 2;
                continue;
            }
            else if(2 == cTmp)
            {
                cTmp = 0;
				switch(c)
                {
					case CONTROL_UP:
						c = 'P' & 077;
						break;       /*- up  -*/

					case CONTROL_DOWN:
						c = 'N' & 077;
						break;       /*-down -*/

					case CONTROL_RIGHT:
						c = 'F' & 077;
						break;       /*-right-*/

					case CONTROL_LEFT:
						c = 'B' & 077;
						break;       /*-left -*/

					default:
						break;
				}
            }
		}

		switch (c)
		{
			case CONTROL_RETURN:
			case CONTROL_NEWLINE:
			{
				if (0 == ucPreCRLF)
				{
					ucPreCRLF = (uint8_t)c;
				}
				else if (c != ucPreCRLF)
				{
					ucPreCRLF = 0;
					break;
				}
				else
				{
					ucPreCRLF = 0;
				}

				if (ptTelnetVty->dwCmdLength > 0)
				{
					if(ptTelnetVty->ucLoginFlag == 0)
					{
						TelnetLogin();
					}
					else
					{
						ExcuteTelnetCmd(ptTelnetVty);
					}
				}
			}
			break;

			case '?':
				vty_question(ptTelnetVty, (char)c);
				break;

			case CONTROL_DEL:
			case 'D' & 077:
				vty_delete(ptTelnetVty);
				break;

			case CONTROL_TAB:
				vty_complete(ptTelnetVty);
				break;

			case CONTROL_BKSPACE:
				vty_backsapce(ptTelnetVty);
				break;

			case 'P' & 077:
				vty_previous_line(ptTelnetVty);
				break;

			case 'N' & 077:
				vty_next_line(ptTelnetVty);
				break;

			case 'B' & 077:
				vty_left_char(ptTelnetVty);
				break;

			case 'F' & 077:
				vty_right_char(ptTelnetVty);
				break;

			default:
				vty_insert_char(ptTelnetVty, (char)c);
				break;
		}
	}//while ((c = telnet_getch(ptTelnetVty)) != -10000)
}


static int telnet_getch(T_TELNET_VTY *ptTelnetVty)
{
    if (NULL == ptTelnetVty)
    {
		 return -10000;
    }

    if (!oam_sty_ring_empty(ptTelnetVty))
    {
        uint32_t c = ptTelnetVty->ring_buf[ptTelnetVty->ring_start++];
        if (ptTelnetVty->ring_start >= sizeof(ptTelnetVty->ring_buf))
        {
            ptTelnetVty->ring_start = 0;
        }
        return c;
    }
    return -10000;
}


void telnet_Inner_print(const char *IszPrnMsg,...)
{
    char           szTelnetPrnbuf[TELNET_MAX_INNER_BUF_LENGTH];
	uint16_t         wTelnetPrnInfoLength = 0;
    va_list        argptr;

    memset(szTelnetPrnbuf,0,TELNET_MAX_INNER_BUF_LENGTH);

    va_start(argptr,IszPrnMsg);
    vsprintf(szTelnetPrnbuf,IszPrnMsg,argptr);

    va_end(argptr);

    wTelnetPrnInfoLength = strlen(szTelnetPrnbuf);
	if(wTelnetPrnInfoLength>=TELNET_MAX_INNER_BUF_LENGTH)
	{
		memset(szTelnetPrnbuf,0,TELNET_MAX_INNER_BUF_LENGTH);
		strncpy(szTelnetPrnbuf,"error, telnet_Inner_print, print too long string!\n",TELNET_MAX_INNER_BUF_LENGTH);
		wTelnetPrnInfoLength = strlen(szTelnetPrnbuf);
	}

    telnet_send((char *)szTelnetPrnbuf,wTelnetPrnInfoLength);
}



