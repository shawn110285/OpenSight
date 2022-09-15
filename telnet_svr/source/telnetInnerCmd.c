
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
#include "../include/telnetapi.h"

extern int32_t CombCmdAndPara(T_Telnet_Command *ptCmd, char *pChar);
extern T_Telnet_Command * g_ptCmdTab;

static void DisposeHelpCmd()
{
    T_Telnet_Command   *ptCmd = NULL;

    ptCmd = g_ptCmdTab;
	if (NULL == ptCmd)
	{
		telnet_Inner_print("No commands!");
		return;
	}

	telnet_Inner_print("\r\n");
	telnet_Inner_print("  +-------------------------------+--------------------------------------------------------------------------+\r\n");
	telnet_Inner_print("  |command                        |     Description                                                          |\r\n");
	telnet_Inner_print("  +-------------------------------+--------------------------------------------------------------------------+\r\n");

	while (NULL != ptCmd)
	{
		telnet_Inner_print("  |%-30s |%-73s |\r\n", ptCmd->cName, ptCmd->cDetail);
		ptCmd = ptCmd->ptNext;
	}
	telnet_Inner_print("  +-------------------------------+--------------------------------------------------------------------------+\r\n");

	return;
}


static void DisposeHistoryCmd(T_TELNET_VTY *ptTelnetVty)
{
	int  i = 0;

	if (NULL == ptTelnetVty)
	{
		return;
	}

	telnet_Inner_print("\r\n");
	telnet_Inner_print("  +-----------------------------------------------------------------------------+\r\n");
	telnet_Inner_print("  |  history-comand                                                             |\r\n");
	telnet_Inner_print("  +-----------------------------------------------------------------------------+\r\n");


	for (i = 0; i < ptTelnetVty->dwHistCmdTotalNum; i++)
	{
		telnet_Inner_print("  |%-77s|\r\n", ptTelnetVty->strHistoryCmd[i]);
	}

	telnet_Inner_print("  +-----------------------------------------------------------------------------+\r\n");

	return;
}




/**************************************************************************

 * cmdline : setlogger   off|on  level(0,1,2)

 *****************************************************************/
static void DisposeLoggerCmd(T_TELNET_TO_EXEC *ptTelnetToExec)
{
	T_Telnet_Exec_Cmd_Para *ptPara = NULL;
	char cParaValue[10];

	if (NULL == ptTelnetToExec )
	{
		return;
	}


    /*display the logger info */
/*
	if (ptTelnetToExec->wParaNum == 0)
	{
		DisposeShowClockCmd();
		return;
	}

	if (ptTelnetToExec->wParaNum != 6)
	{
		telnet_Inner_print("\r\n  Parameter number error!");
		telnet_Inner_print("\r\n  Usage: setclock Year Month Day Hour Minute Second");
		return;
	}

	ptPara = (T_Telnet_Exec_Cmd_Para *)(ptTelnetToExec->pData + ptTelnetToExec->wCmdLen);
	if (ptPara->wParaLen > 4)
	{
		telnet_Inner_print("\r\n  Year error, values:1970-9999");
		telnet_Inner_print("\r\n  Usage: setclock Year Month Day Hour Minute Second");
		return;
	}
	memset(cParaValue, 0, 10);
	memcpy(cParaValue, ptPara->pData, ptPara->wParaLen);
	iYear = atoi(cParaValue);
	if (iYear < 1970 || iYear > 9999)
	{
		telnet_Inner_print("\r\n  Year error: %d , values:1970-9999", iYear);
		telnet_Inner_print("\r\n  Usage: setclock Year Month Day Hour Minute Second");
		return;
	}

	ptPara = (T_Telnet_Exec_Cmd_Para *)(ptPara->pData + ptPara->wParaLen);
	if (ptPara->wParaLen > 2)
	{
		telnet_Inner_print("\r\n  Month error, values:0-12");
		telnet_Inner_print("\r\n  Usage: setclock Year Month Day Hour Minute Second");
		return;
	}
	memset(cParaValue, 0, 10);
	memcpy(cParaValue, ptPara->pData, ptPara->wParaLen);
	iMonth = atoi(cParaValue);
	if (iMonth < 0 || iMonth > 12)
	{
		telnet_Inner_print("\r\n  Month error: %d , values:0-12", iMonth);
		telnet_Inner_print("\r\n  Usage: setclock Year Month Day Hour Minute Second");
		return;
	}

    if(argc==0)
    {
	    telnet_print("  Flag      =%d \r\n",gdwDbgPrnFlag);
	    telnet_print("  level     =%d \r\n",gdwDbgPrnLevel);
        return;
    }

    if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
    {
	    telnet_print("  usage: setlogger off|on level(0,1,2) \r\n");
        return;
    }

    if(strncasecmp(argv[0],"on",strlen("on")) == 0)
    {
        gdwDbgPrnFlag   = OSAL_TRUE;
    }
    else
    {
        if(strncasecmp(argv[0],"off",strlen("off")) == 0)
        {
            gdwDbgPrnFlag = OSAL_FALSE;
        }
        else
        {
            telnet_print("  --invalid flag parameter,setlogger off|on \r\n" );
            return;
        }
   }

   telnet_print("  set monitor Flag = %d \r\n",gdwDbgPrnFlag);
   if(argc<2)  return;

    gdwDbgPrnLevel = atoi(argv[1]);
    if(gdwDbgPrnLevel>2)  gdwDbgPrnLevel =2;
    if(gdwDbgPrnLevel<0)  gdwDbgPrnLevel =0;

   telnet_print("  set monitor level = %d \r\n",gdwDbgPrnLevel);
   */
}


static void  DisposeVersionCmd()
{
	/*
   telnet_print("   Version: %s  \r\n",PRODUCT_VERSION);
   telnet_print("  Bulit at: %s %s \n", __DATE__, __TIME__);
   */
}


void DisposeInnerCmd(T_TELNET_TO_EXEC *ptTelnetToExec, T_TELNET_VTY *ptTelnetVty)
{
	char cCmdName[TELNET_MAX_CHARS_LENGTH];

	if (NULL == ptTelnetToExec || NULL == ptTelnetVty)
	{
		return;
	}

	memset(cCmdName, 0, TELNET_MAX_CHARS_LENGTH);
    memcpy(cCmdName, ptTelnetToExec->pData, ptTelnetToExec->wCmdLen);

	if (0 == strncasecmp(cCmdName, "quit", strlen("quit")))
	{
        telnet_shutdown();
		return;
	}
	else if ((0 == strncasecmp(cCmdName, "?", strlen("?"))) || (0 == strncasecmp(cCmdName, "help", strlen("help"))))
	{
		DisposeHelpCmd();
		return;
	}
	else if (0 == strncasecmp(cCmdName, "history", strlen("history")))
	{
        DisposeHistoryCmd(ptTelnetVty);
		return;
	}
	else if (0 == strncasecmp(cCmdName, "setlogger", strlen("setlogger")))
	{
        DisposeLoggerCmd(ptTelnetToExec);
		return;
	}
	else if (0 == strncasecmp(cCmdName, "showversion", strlen("showversion")))
	{
        DisposeVersionCmd();
		return;
	}
	else
	{
        telnet_Inner_print("\r\n command not existed!");
		return;
	}
	return;
}


