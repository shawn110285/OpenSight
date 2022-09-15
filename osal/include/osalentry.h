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

#ifndef  __OSAL_ENTRY_H__
#define  __OSAL_ENTRY_H__

#include "osalapi.h"
#include "osalsche.h"

#include "../include/osalversion.h"

extern int32_t    OsPort_InitNetwork();
extern uint32_t   OsDbgPrnInit();
extern int32_t    OsPortInit(void);
extern uint32_t   OsFsInit(void);
extern uint32_t   OsInitMsgBuf(void);
extern uint32_t   OsTimerInit (void);
extern uint32_t   OsCommInit(void);
extern uint32_t   OsScheduleInit(void);
extern uint32_t   OsCreateProcess(char * szName, PROCESS_ENTRY pStartEntry, uint8_t bAttr, uint8_t bPriLvl, uint32_t dwStkSize, uint32_t dwDataSize, uint32_t dwAsynMsgQLen, uint32_t dwSyncMsgQLen);
extern uint32_t   OsCoreSendMsgToSysCtrl(uint32_t   msgID);
extern int32_t    SocketInit();
extern void       sysctrlEntry(void);

#endif   /*  __OSAL_ENTRY_H__ */


