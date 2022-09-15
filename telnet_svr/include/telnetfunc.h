
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

#ifndef   __TELNET_FUNC_H__
#define   __TELNET_FUNC_H__

#include "telnet.h"
#include "telnetapi.h"

typedef struct tagMSG_TELNET_WORD
{
	uint16_t    wWordNum;
	char 	sWord[TELNET_MAX_WORDS][TELNET_MAX_CHARS_LENGTH];
} T_TELNET_WORD;

extern T_Telnet_Command   *g_ptCmdTab;
extern osal_pid_t          tCmdTargetPid;
extern osal_pid_t          tTelnetPid;
extern T_TELNET_VTY        sgtTelnetVty;
extern uint32_t            sdwCmdRunTimerId;

extern void telnetCmdRelease();
extern void telnet_ParseAndExec_Cmd(T_TELNET_VTY *ptTelnetVty);

extern void telnet_Output_Exec_Result(uint8_t * pMsgData);
extern void telnet_Output_TimeOut();
extern void DisposeInnerCmd(T_TELNET_TO_EXEC *ptTelnetToExec, T_TELNET_VTY *ptTelnetVty);

#endif  /* __TELNET_FUNC_H__ */



