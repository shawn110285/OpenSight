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

#include "../../osal/include/osalmsgdef.h"
#include "../include/telnetapi.h"
#include "../include/telnet.h"


extern osal_pid_t tTelnetPid;


static int32_t CheckByte(char *pChar)
{
	unsigned short i = 0;
	int iValue = 0;

	if (NULL == pChar)
	{
		return OSAL_FALSE;
	}

	if (strlen(pChar) > 3)
	{
		return OSAL_FALSE;
	}

	for (i = 0; i < strlen(pChar); i++)
	{
		if (*(pChar+i) < '0' || *(pChar+i) > '9')
		{
			return OSAL_FALSE;
		}
	}
	iValue = atoi(pChar);

	if (iValue > 255)
	{
		return OSAL_FALSE;
	}
	return OSAL_TRUE;
}


static int32_t CheckWord(char *pChar)
{
	unsigned short i = 0;
	int iValue = 0;

	if (NULL == pChar)
	{
		return OSAL_FALSE;
	}

	if (strlen(pChar) > 5)
	{
		return OSAL_FALSE;
	}

	for (i = 0; i < strlen(pChar); i++)
	{
		if (*(pChar+i) < '0' || *(pChar+i) > '9')
		{
			return OSAL_FALSE;
		}
	}
	iValue = atoi(pChar);

	if (iValue > 0xFFFF)
	{
		return OSAL_FALSE;
	}
	return OSAL_TRUE;
}


static int32_t CheckDword(char *pChar)
{
	unsigned short i = 0;

	if (NULL == pChar)
	{
		return OSAL_FALSE;
	}

	if (strlen(pChar) > 10)
	{
		return OSAL_FALSE;
	}

	for (i = 0; i < strlen(pChar); i++)
	{
		if (*(pChar+i) < '0' || *(pChar+i) > '9')
		{
			return OSAL_FALSE;
		}
	}

	return OSAL_TRUE;
}


static int32_t CheckInt(char *pChar)
{
	unsigned short i = 0;
	int32_t     blFlag = OSAL_FALSE;

	if (NULL == pChar)
	{
		return OSAL_FALSE;
	}


	if (*pChar >= '0' && *pChar <= '9')
	{
		blFlag = OSAL_TRUE;
	}
	else if( '-' == *pChar || '+' == *pChar)
	{
	     blFlag = OSAL_TRUE;
	}
	else
	{
	      blFlag = OSAL_FALSE;
	}

    if(blFlag == OSAL_FALSE)
		return  OSAL_FALSE;

	for (i = 1; i < strlen(pChar); i++)
	{
		if (*(pChar+i) < '0' || *(pChar+i) > '9')
		{
			return OSAL_FALSE;
		}
	}

	return OSAL_TRUE;
}


static int32_t CheckDigit(char *pChar)
{
	unsigned short i = 0;

	if (NULL == pChar)
	{
		return OSAL_FALSE;
	}

	for (i = 0; i < strlen(pChar); i++)
	{
		if((*pChar>='0')&&(*pChar<='9'))
		{
			pChar++;
		}
		else
		{
			return OSAL_FALSE;
		}
	}

	return OSAL_TRUE;
}


static int32_t CheckLetter(char *pChar)
{
	unsigned short i = 0;

	if (NULL == pChar)
	{
		return OSAL_FALSE;
	}

	for (i = 0; i < strlen(pChar); i++)
	{
		if(((*pChar>='a') && (*pChar<='z')) ||((*pChar>='A')&&(*pChar<='Z')))
		{
			pChar++;
		}
		else
		{
			return OSAL_FALSE;
		}
	}

	return OSAL_TRUE;
}


static int32_t CheckBool(char *pChar)
{
	if (NULL == pChar)
	{
		return OSAL_FALSE;
	}
	if( ( 0 == strncasecmp(pChar, "TRUE",strlen(pChar)) )  ||  ( 0 == strncasecmp(pChar, "FALSE", strlen(pChar))) )
    {
        return OSAL_TRUE;
    }

	return OSAL_FALSE;
}


static int32_t CheckFlag(char *pChar)
{
	if (NULL == pChar)
	{
		return OSAL_FALSE;
	}

	if( ( 0 == strncasecmp(pChar, "ON",strlen(pChar)) )  ||  ( 0 == strncasecmp(pChar, "OFF", strlen(pChar))) )
    {
        return OSAL_TRUE;
    }
	return OSAL_FALSE;
}


int32_t CheckPara(char *pChar, uint16_t wParaType)
{
	int32_t bRet = OSAL_FALSE;

	if (NULL == pChar || wParaType > E_TYPE_MAX)
	{
		return OSAL_FALSE;
	}

	switch(wParaType)
	{
		case E_TYPE_BYTE:
			bRet = CheckByte(pChar);
			break;

		case E_TYPE_DWORD:
			bRet = CheckDword(pChar);
			break;

		case E_TYPE_WORD:
			bRet = CheckWord(pChar);
			break;

		case E_TYPE_STRING:
			bRet = OSAL_TRUE;
			break;

		case E_TYPE_INT:
			bRet = CheckInt(pChar);
			break;

		case E_TYPE_BOOL:
			bRet = CheckBool(pChar);
			break;

		case E_TYPE_STRING_DIGIT:
			bRet = CheckDigit(pChar);
			break;

		case E_TYPE_STRING_LETTER:
			bRet = CheckLetter(pChar);
			break;

		case E_TYPE_FLAG:
			bRet = CheckFlag(pChar);
			break;

		default:
			break;
	}
	return bRet;
}


char * GetParaTypeString(uint16_t wParaType)
{
	if (wParaType > E_TYPE_MAX)
	{
		return OSAL_FALSE;
	}

	switch(wParaType)
	{
		case E_TYPE_BYTE:
			return "uint8_t";

		case E_TYPE_DWORD:
			return "uint32_t";

		case E_TYPE_WORD:
			return "uint16_t";

		case E_TYPE_STRING:
			return "STRING";

		case E_TYPE_INT:
			return "INT";

		case E_TYPE_BOOL:
			return "int32_t";

		case E_TYPE_STRING_DIGIT:
			return "DIGIT";

		case E_TYPE_STRING_LETTER:
			return "LETTER";

		case E_TYPE_FLAG:
			return "FLAG";

		default:
			break;
	}
	return NULL;;
}



#define TELNET_MAX_PRN_BUF_LENGTH               1000

void  telnet_print(const char *IszPrnMsg,...)
{
    char           szTelnetPrnbuf[TELNET_MAX_PRN_BUF_LENGTH];
	uint16_t         wTelnetPrnInfoLength = 0;
    va_list        argptr;
	char *pChar = NULL;
	uint16_t wi = 0;
	int i = 0;

    memset(szTelnetPrnbuf,0,TELNET_MAX_PRN_BUF_LENGTH);

    va_start(argptr,IszPrnMsg);

    vsnprintf(szTelnetPrnbuf,TELNET_MAX_PRN_BUF_LENGTH-2,IszPrnMsg,argptr);

    va_end(argptr);

    wTelnetPrnInfoLength = strlen(szTelnetPrnbuf);

	if(wTelnetPrnInfoLength>=TELNET_MAX_PRN_BUF_LENGTH)
	{
		memset(szTelnetPrnbuf,0,TELNET_MAX_PRN_BUF_LENGTH);
		strncpy(szTelnetPrnbuf,"error,telnet_print,print too long string!\n",TELNET_MAX_PRN_BUF_LENGTH);
		wTelnetPrnInfoLength = strlen(szTelnetPrnbuf);
	}

	pChar = szTelnetPrnbuf;

	while (pChar[wi] == ' ')
        wi ++;

	if (wi > 2)
	{
		for(i=0; i < wTelnetPrnInfoLength - wi + 2; i++)
		{
			szTelnetPrnbuf[i] = szTelnetPrnbuf[i+wi-2];
		}
		szTelnetPrnbuf[wTelnetPrnInfoLength - wi + 2] = 0;
		wTelnetPrnInfoLength = wTelnetPrnInfoLength - wi + 2;
	}
	else if (wi < 2)
	{
        if ((wTelnetPrnInfoLength + wi) >= TELNET_MAX_PRN_BUF_LENGTH) /* 长度保护 */
		{
			wTelnetPrnInfoLength = TELNET_MAX_PRN_BUF_LENGTH - wi - 1;
		}

		for(i=wTelnetPrnInfoLength-1; i >= 0; i--)
		{
			szTelnetPrnbuf[i+2-wi] = szTelnetPrnbuf[i];
		}
		for (i=0; i<2-wi; i++)
		{
			szTelnetPrnbuf[i] = ' ';
		}
		szTelnetPrnbuf[wTelnetPrnInfoLength + 2 - wi] = 0;
		wTelnetPrnInfoLength = wTelnetPrnInfoLength + 2 - wi;
	}

	if ((szTelnetPrnbuf[wTelnetPrnInfoLength-1] == '\n') && (wTelnetPrnInfoLength < TELNET_MAX_PRN_BUF_LENGTH - 1))
	{
		szTelnetPrnbuf[wTelnetPrnInfoLength] = '\r';
		wTelnetPrnInfoLength = wTelnetPrnInfoLength + 1;
		szTelnetPrnbuf[wTelnetPrnInfoLength] = 0;
	}

	osal_msg_post_out(tTelnetPid, (uint32_t)MSG_TELNET_EXEC_PRINT, (uint8_t*)szTelnetPrnbuf, wTelnetPrnInfoLength);
}


#define LOWER(x)    (isupper(x) ? tolower(x) : (x))


static char * ambiguous;        /* special return value for command routines */
static int  Ambiguous(char *s)
{
    return((char **)s == &ambiguous);
}


static int isprefix(register char *s1, register char *s2)
{
    char *os1;
    register char c1, c2;

    if (*s1 == '\0')
        return(-1);

    os1 = s1;
    c1 = *s1;
    c2 = *s2;

    while (LOWER(c1) == LOWER(c2))
    {
        if (c1 == '\0')
            break;

        c1 = *++s1;
        c2 = *++s2;
    }
    return(*s1 ? 0 : (*s2 ? (s1 - os1) : (os1 - s1)));
}



static T_Telnet_Exec_Command * getcmd(char *name, uint16_t * pwHindCmdNum, T_Telnet_Exec_Command * ptCmdTab, uint16_t wCount)
{
    T_Telnet_Exec_Command * cm  = NULL;
    uint16_t                  wItem;
    register int            n;
    uint16_t                  wFoundNum=0;

    if (name == 0)
        return 0;

    for(wItem = 0; wItem < wCount; wItem++)
    {
        n = isprefix(name, ptCmdTab[wItem].name);

        if (n == 0)
            continue;

        if (n < 0)
        {
            cm = &ptCmdTab[wItem];
            return cm;
        }

        if(n > 0)
        {
            wFoundNum ++;

            if(wFoundNum ==1)
            {
                cm= &ptCmdTab[wItem];
            }
            else
            {
                if(wFoundNum == 2)
                {
                    cm =(T_Telnet_Exec_Command * )&ambiguous;
                }
            }

        }

    }
    * pwHindCmdNum = wFoundNum;

    if(wFoundNum)
        return cm;
    else
        return NULL;
}










extern  void telnetSendResult(uint16_t wRetCode);
#define  MAX_PARA_NUMBER  20

/*the telnet cmd, the msg length, the local cmd dict, the cmd item number */
void telnet_Match_And_Exec_One_Cmd(T_TELNET_TO_EXEC * ptTelnetToExec, uint32_t dwMsgLen, T_Telnet_Exec_Command * ptCmdTab, uint16_t wCount)
{
	int	   margc;
	char   acmargv[MAX_PARA_NUMBER][TELNET_MAX_CHARS_LENGTH];
	char * margv[MAX_PARA_NUMBER];

    uint16_t                   wHindCmdNum=0;
    int                      i = 0;
    uint32_t                   dwLen= 0;
    T_Telnet_Exec_Cmd_Para * ptPara = NULL;
	T_Telnet_Exec_Command  * c;

    if (NULL == ptTelnetToExec)
    {
        telnetSendResult(SUCC_AND_NOPARA);
        return;
    }

    if (ptTelnetToExec->wCmdLen > dwMsgLen)
    {
        telnetSendResult(SUCC_AND_NOPARA);
        return;
    }

    if (ptTelnetToExec->wCmdLen >= TELNET_MAX_CHARS_LENGTH || ptTelnetToExec->wParaNum > MAX_PARA_NUMBER)
    {
        telnetSendResult(SUCC_AND_NOPARA);
        return;
    }

    memset(acmargv, 0, MAX_PARA_NUMBER * TELNET_MAX_CHARS_LENGTH * sizeof(char));
    for (i = 0; i < MAX_PARA_NUMBER; i++)
    {
        margv[i] = acmargv[i];
    }

	//copy the command name
    memcpy(acmargv[0], ptTelnetToExec->pData, ptTelnetToExec->wCmdLen);
    dwLen = sizeof(ptTelnetToExec->wCmdLen) + sizeof(ptTelnetToExec->wParaNum) + sizeof(ptTelnetToExec->wMessageLen) + ptTelnetToExec->wCmdLen;
    ptPara = (T_Telnet_Exec_Cmd_Para *)(ptTelnetToExec->pData + ptTelnetToExec->wCmdLen);
    for (i = 0; i < ptTelnetToExec->wParaNum; i++)
    {
        dwLen = dwLen + ptPara->wParaLen + sizeof(ptPara->wParaLen) + sizeof(ptPara->wParaType);
        if (dwLen > dwMsgLen || ptPara->wParaLen >= TELNET_MAX_CHARS_LENGTH)
        {
            telnetSendResult(SUCC_AND_NOPARA);
            return;
        }
        // copy the command param list
        memcpy(acmargv[i+1], ptPara->pData, ptPara->wParaLen);
        ptPara = (T_Telnet_Exec_Cmd_Para *)(ptPara->pData + ptPara->wParaLen);
    }
    margc = ptTelnetToExec->wParaNum;

	c = getcmd(margv[0], &wHindCmdNum, ptCmdTab, wCount);
    if (Ambiguous((char *)c))
    {
        telnet_print("  There are %d Ambiguous command,use ? to get help!\r\n",wHindCmdNum);
        telnetSendResult(SUCC_AND_NOPARA);
        return;
    }

    if (c == NULL)
    {
        telnet_print("  Invalid command,use ? to get help!\r\n");
        telnetSendResult(SUCC_AND_NOPARA);
        return;
    }

    (*c->handler)(margc, margv+1);

    telnetSendResult(SUCC_AND_NOPARA);
	return;
}








