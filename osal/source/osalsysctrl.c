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

#include "../include/osalapi.h"
#include "../include/osalmsgdef.h"

static void  sysctrlEntry();
extern void  telnetEntry();
extern void  gdbEntry();
extern void  dapTestEntry();

static  T_TaskParam  sgatPlatfromTask[ ] =
{
    /* Name,          Attr, PriLvl, StkSz(k), */
    {"sysctrl",       0,    80,     4096,
	    {
  	        /* used,         Name           StartEntry,             DataSz(k),     AsynMsgQLen,   SyncMsgQLen */
            {  OSAL_TRUE,    "sysctrl",     sysctrlEntry,           4096,          160,           80 },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
		}
	},

	/* Name,          Attr, PriLvl, StkSz(k), */
    {"telnet",        0,    80,     4096,
	    {
  	        /* used,         Name            StartEntry,             DataSz(k),     AsynMsgQLen,   SyncMsgQLen */
            {  OSAL_TRUE,    "telnet",       telnetEntry,     4096,          160,           80 },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
		}
	},

	/* Name,          Attr, PriLvl, StkSz(k), */
    {"gdb",        0,    80,     4096,
	    {
  	        /* used,         Name            StartEntry,             DataSz(k),     AsynMsgQLen,   SyncMsgQLen */
            {  OSAL_TRUE,    "gdbSvr",       gdbEntry,               4096,          160,           80 },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
		}
	},

	/* Name,          Attr, PriLvl, StkSz(k), */
    {"dap",        0,    80,     4096,
	    {
  	        /* used,         Name            StartEntry,             DataSz(k),     AsynMsgQLen,   SyncMsgQLen */
            {  OSAL_TRUE,    "dap",          dapTestEntry,           4096,          160,           80 },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
			{  OSAL_FALSE },
		}
	}
};


static uint32_t  dwCheckCpu = 0;
static int32_t    sgdwRunFlag = OSAL_FALSE;

int32_t osCreateProcess()
{
    uint8_t  ucTaskIndex = 0;
    for(ucTaskIndex=0; ucTaskIndex<sizeof(sgatPlatfromTask)/sizeof(T_TaskParam);ucTaskIndex++)
    {
	    if(OSAL_OK != osal_task_create(&sgatPlatfromTask[ucTaskIndex]))
        {
	        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:create task(%s), .................................Failed!\n",sgatPlatfromTask[ucTaskIndex].szName);
	        return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}



void  sysctrlEntry()
{
	uint32_t              dwMsgId;
	osal_pid_t            tRecvPid = {0};

	dwMsgId	   = osal_msg_get_id ();

	switch(dwMsgId)
	{

	    case MSG_MasterPowerOn:
		{
			tRecvPid.dwPno= osal_task_get_pno_by_name("telnet");
			if(OSAL_ERR_INVALID_PNO == tRecvPid.dwPno)
			{
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[sysCtrl]:Telnet Server is not running...................ERROR \n");
			}
			else
			{
					osal_msg_post_out( tRecvPid, MSG_MasterPowerOn, NULL, 0);
			}

    	}
		break;

		case MSG_TELNET_SYSCTRL_READY:
		{
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[sysCtrl]:telnet server PowerOn,............................OK \n");
			tRecvPid.dwPno= osal_task_get_pno_by_name("gdbSvr");
			if(OSAL_ERR_INVALID_PNO == tRecvPid.dwPno)
			{
				osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[sysCtrl]:gdb server is not running,......................ERROR \n");
			}
			else
			{
				osal_msg_post_out( tRecvPid, MSG_MasterPowerOn, NULL, 0);
			}
		}
		break;

		case MSG_GDB_SYSCTRL_READY:
		{
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[sysCtrl]:gdb server PowerOn,...............................OK \n");
			tRecvPid.dwPno= osal_task_get_pno_by_name("dap");
			if(OSAL_ERR_INVALID_PNO == tRecvPid.dwPno)
			{
				osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[sysCtrl]:dap is not running,.............................ERROR \n");
			}
			else
			{
				osal_msg_post_out( tRecvPid, MSG_MasterPowerOn, NULL, 0);
			}
		}
		break;

		case MSG_DAP_SYSCTRL_READY:
		{
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[sysCtrl]:dap Process PowerOn,..............................OK \n");
		}
		break;
    }
}


