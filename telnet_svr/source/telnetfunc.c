
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

#include "../include/telnetfunc.h"


void telnetCmdRelease()
{
	T_Telnet_Command *ptCmd = NULL;

	ptCmd = g_ptCmdTab;
	osal_mem_free(ptCmd);

	g_ptCmdTab = NULL;

	return;
}


int32_t SplitCmdLine(char *pChar, T_TELNET_WORD *ptWord)
{
	char *pCurChar = NULL;
	uint16_t wCharIndex = 0;
	uint16_t wWordSum = 0;

	if (NULL == pChar || NULL == ptWord)
	{
		return OSAL_FALSE;
	}

	pCurChar = pChar;

	while (* pCurChar != MARK_STRINGEND && wWordSum < TELNET_MAX_WORDS)
	{
		while ( *pCurChar != MARK_STRINGEND && *pCurChar == MARK_BLANK)
		{
			pCurChar ++;
		}

		if (*pCurChar == MARK_STRINGEND)
		{
		    break;
		}

		wCharIndex = 0;
		while (* pCurChar != MARK_STRINGEND
			&& * pCurChar != MARK_BLANK
			&& wCharIndex < TELNET_MAX_CHARS_LENGTH)
		{
			ptWord->sWord[wWordSum][wCharIndex] = * pCurChar;
			wCharIndex++;
			pCurChar++;
		}

		wWordSum ++;
    }
	ptWord->wWordNum = wWordSum;
	return OSAL_TRUE;
}


int32_t CombCmdAndPara(T_Telnet_Command *ptCmd, char *pChar)
{
	int i = 0;

	if (NULL == ptCmd || NULL == pChar)
	{
		return OSAL_FALSE;
	}

	strcat(pChar, ptCmd->cName[0]);

	for (i = 0; i < ptCmd->wParaNum; i++)
	{
		if (' ' == ptCmd->tPara[i].cPara[0])
		{
			continue;
		}
		strcat(pChar, " ");
        strcat(pChar, ptCmd->tPara[i].cPara);
	}
	return OSAL_TRUE;
}


int32_t ConstructMsgData(T_TELNET_WORD *ptWord, T_Telnet_Command *ptCmd, T_TELNET_TO_EXEC *ptTelnetToExec, T_TELNET_VTY *ptTelnetVty)
{
	int i = 0;
	T_Telnet_Exec_Cmd_Para *ptPara = NULL;
	int iCmdParaNum = 0;
	char  cBuffer[BUFFER_LENGTH];
	char  *pChar = NULL;

	if (NULL == ptWord || NULL == ptCmd || NULL == ptTelnetToExec)
	{
		return OSAL_FALSE;
	}

	ptTelnetToExec->wCmdLen = strlen(ptCmd->cName[0]);
    memcpy(ptTelnetToExec->pData, ptCmd->cName[0], strlen(ptCmd->cName[0]));
	ptTelnetToExec->wMessageLen = sizeof(ptTelnetToExec->wCmdLen) + sizeof(ptTelnetToExec->wParaNum)
		                          + sizeof(ptTelnetToExec->wMessageLen) +  ptTelnetToExec->wCmdLen;

	ptPara = (T_Telnet_Exec_Cmd_Para *)(ptTelnetToExec->pData + ptTelnetToExec->wCmdLen);

	if ((ptWord->wWordNum - ptCmd->wCmdWordNum) > ptCmd->wParaNum)
	{
		memset(cBuffer, 0, BUFFER_LENGTH*sizeof(char));
		telnet_Inner_print(ptTelnetVty->strMode);
		strcat(cBuffer, "Wrong number of paramenters, command is \"");
		pChar = cBuffer + strlen("Wrong number of paramenters, command is \"");
		CombCmdAndPara(ptCmd, pChar);
		strcat(cBuffer, "\"!");
		telnet_Inner_print(cBuffer);
		return OSAL_FALSE;
	}

    for (i = ptCmd->wCmdWordNum; i < ptWord->wWordNum; i++)
    {
		if (OSAL_FALSE == CheckPara(ptWord->sWord[i], ptCmd->tPara[iCmdParaNum].wParaType))
		{
			memset(cBuffer, 0, BUFFER_LENGTH*sizeof(char));
			telnet_Inner_print(ptTelnetVty->strMode);
			strcat(cBuffer, "Paramenter error, command is \"");
			pChar = cBuffer + strlen("Paramenter error, command is \"");
			CombCmdAndPara(ptCmd, pChar);
			strcat(cBuffer, "\"!");
		    telnet_Inner_print(cBuffer);

			memset(cBuffer, 0, BUFFER_LENGTH*sizeof(char));
			sprintf(cBuffer,"\r\nparamenter \"%s\" type is %s , ipput type error!", ptCmd->tPara[iCmdParaNum].cPara, GetParaTypeString(ptCmd->tPara[iCmdParaNum].wParaType));
            telnet_Inner_print(cBuffer);
			return OSAL_FALSE;
		}

		ptPara->wParaLen = strlen(ptWord->sWord[i]);
        memcpy(ptPara->pData, ptWord->sWord[i], strlen(ptWord->sWord[i]));
        ptPara->wParaType = ptCmd->tPara[iCmdParaNum].wParaType;
		ptTelnetToExec->wMessageLen = ptTelnetToExec->wMessageLen + sizeof(ptPara->wParaLen) + sizeof(ptPara->wParaType)
			                          + ptPara->wParaLen;
		iCmdParaNum ++;
		ptPara = (T_Telnet_Exec_Cmd_Para *)(ptPara->pData + ptPara->wParaLen);
    }

	ptTelnetToExec->wParaNum = iCmdParaNum;
	return OSAL_TRUE;
}


int32_t GetSendCmdPid(osal_pid_t *ptPid, char *pChar)
{
	if (NULL == ptPid || NULL == pChar)
	{
		return OSAL_FALSE;
	}

	ptPid->dwPno = osal_task_get_pno_by_name (pChar);

	if (OSAL_ERR_INVALID_PNO == ptPid->dwPno)
	{
		return OSAL_FALSE;
	}
    return OSAL_TRUE;
}


void ExecOneTelnetCmd(T_TELNET_WORD *ptWord, T_Telnet_Command *ptCmd, T_TELNET_VTY *ptTelnetVty)
{
	T_TELNET_TO_EXEC   *ptTelnetToExec = NULL;
	uint32_t  tSendOK = OSAL_OK;
	char  cBuffer[BUFFER_LENGTH];

	memset(cBuffer, 0, BUFFER_LENGTH * sizeof(char));

	ptTelnetToExec = (T_TELNET_TO_EXEC   *)cBuffer;

	if (OSAL_FALSE == ConstructMsgData(ptWord, ptCmd, ptTelnetToExec, ptTelnetVty))
	{
		return;
	}

	if (0 == memcmp(ptCmd->cProgressName, "telnet", strlen("telnet")))
	{
		DisposeInnerCmd(ptTelnetToExec, ptTelnetVty);
		ptTelnetVty->ucLocalPidExec = 1;
		return;
	}

	if (OSAL_FALSE == GetSendCmdPid(&tCmdTargetPid, ptCmd->cProgressName))
	{
		telnet_Inner_print(ptTelnetVty->strMode);
		telnet_Inner_print("Command pid error,pid = %s !");
		return;
	}

	tSendOK = osal_msg_post_out(tCmdTargetPid, (uint32_t)MSG_TELNET_TO_EXEC, (uint8_t*)cBuffer, ptTelnetToExec->wMessageLen);
	if(tSendOK != OSAL_OK)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:Send telnet command to exec pid failed!\r\n");
		return;
	}

	ptTelnetVty->ucLocalPidExec = 0;
	sgtTelnetVty.ucCurrRunState = Telnet_MsgHandle_EXECUTE;
	sdwCmdRunTimerId = osal_timer_start_normal(MSG_CMD_RUN_TIME_OUT, 0, 8000);
	return;
}


void telnet_ParseAndExec_Cmd(T_TELNET_VTY *ptTelnetVty)
{
	T_TELNET_WORD  tWord;
	char             *pChar = NULL;
	T_Telnet_Command *ptCmd = NULL;
	T_Telnet_Command *ptExecCmd = NULL;
	uint16_t wCmdCnt = 0;
	char *acuCmdName[TELNET_MAX_TAB_CMD] = { 0 };

	if (NULL == ptTelnetVty)
	{
		return;
	}

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
		return;
	}

	while (NULL != ptCmd)
	{
	    int i = 0, iTime = 0;
		int iMinWordsNum = ((tWord.wWordNum > ptCmd->wCmdWordNum)?ptCmd->wCmdWordNum:tWord.wWordNum);

		for (i = 0; i < iMinWordsNum; i++)
		{
			if ( strlen(tWord.sWord[i]) <= strlen(ptCmd->cName[i+1]) && 0 == strncasecmp(ptCmd->cName[i+1], tWord.sWord[i], strlen(tWord.sWord[i])) )
			{
				iTime ++;
			}
		}

		if (iTime == iMinWordsNum && wCmdCnt < TELNET_MAX_TAB_CMD)
		{
			acuCmdName[wCmdCnt++] = ptCmd->cName[0];
			ptExecCmd = ptCmd;

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
		telnet_Inner_print("Command error, please input '?' or 'help' to check it!");
        return;
    }
	else if (1 == wCmdCnt)
    {
    	ExecOneTelnetCmd(&tWord, ptExecCmd, ptTelnetVty);
    }
	else
	{
		int i = 0;

		telnet_Inner_print("\r\nMatching multiple commands, as follows:");

		for (i = 0; i < wCmdCnt; i ++)
		{
			telnet_Inner_print("\r\n  ");
			telnet_Inner_print(acuCmdName[i]);
		}
	}
	return;
}


void telnetSendResult(uint16_t wRetCode)
{
    uint32_t             tSendOK = OSAL_OK;
    T_EXEC_TO_TELNET * ptRetunResult = NULL;
    char               cBuffer[BUFFER_LENGTH];

    memset(cBuffer, 0, BUFFER_LENGTH * sizeof(char));

    ptRetunResult = (T_EXEC_TO_TELNET *)cBuffer;
    ptRetunResult->wReturnCode = wRetCode;
    ptRetunResult->bLastPacket = OSAL_TRUE;
    ptRetunResult->wMessageLen = sizeof(ptRetunResult->wReturnCode) + sizeof(ptRetunResult->wMessageLen) + sizeof(ptRetunResult->bLastPacket);
    tSendOK = osal_msg_post_out(tTelnetPid, (uint32_t)MSG_EXEC_TO_TELNET, (uint8_t*)cBuffer, ptRetunResult->wMessageLen);
    if(tSendOK != OSAL_OK)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:Send command failed!\r\n");
    }

    return;
}


void telnet_Output_Exec_Result(uint8_t * pMsgData)
{
	T_EXEC_TO_TELNET *ptExecToTelnet = NULL;

	if (NULL == pMsgData)
	{
		return;
	}

	ptExecToTelnet = (T_EXEC_TO_TELNET *)pMsgData;

	switch(ptExecToTelnet->wReturnCode)
	{
	case SUCC_AND_NOPARA:
		if (NULL == sgtTelnetVty.tPageInfo.p_disp_head)
		{
			telnet_Inner_print(sgtTelnetVty.strMode);
		}
		sgtTelnetVty.ucCurrRunState = Telnet_MsgHandle_IDLE;
		break;

	case SUCC_AND_HAVEPARA:
	{
		int iDataLen = 0;
		int iHeadLen = sizeof(ptExecToTelnet->wMessageLen) + sizeof(ptExecToTelnet->wReturnCode) + sizeof(ptExecToTelnet->bLastPacket);

		iDataLen = ptExecToTelnet->wMessageLen - iHeadLen;
		if (ptExecToTelnet->wMessageLen > iHeadLen)
		{
			OutputExecResult((uint8_t *)ptExecToTelnet->pData, (uint16_t)iDataLen);
		}

		if (OSAL_TRUE == ptExecToTelnet->bLastPacket)
		{
//			telnet_Inner_print(sgtTelnetVty.strMode);
			sgtTelnetVty.ucCurrRunState = Telnet_MsgHandle_IDLE;
		}
		break;
	}
	default:
		break;
	}
	return;
}


void telnet_Output_TimeOut()
{
	telnet_Inner_print("The command execution timeout!");
	telnet_Inner_print(sgtTelnetVty.strMode);
	return;
}


