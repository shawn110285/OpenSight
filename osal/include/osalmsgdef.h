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

#ifndef __OSAL_MSG_H__
#define __OSAL_MSG_H__

/************************************  the      process states  *************************************/
#define  INIT_STATE             	0
#define  WORK_STATE			        1
#define  PEND_STATE                 2

/******************************************* OSAL message:100 ****************************************/
#define  MSG_OSAL_START_ID                    0

#define  MSG_MasterPowerOn                    (MSG_OSAL_START_ID+1)
#define  MSG_MasterPowerOff                   (MSG_OSAL_START_ID+2)

/******************************************* sysctrl message   ***************************************/
#define  MSG_SYSCTRL_START_ID                  (MSG_OSAL_START_ID+100)
#define  MSG_TELNET_SYSCTRL_READY              (MSG_SYSCTRL_START_ID+4)
#define  MSG_GDB_SYSCTRL_READY                 (MSG_SYSCTRL_START_ID+5)
#define  MSG_DAP_SYSCTRL_READY                 (MSG_SYSCTRL_START_ID+6)

/******************************************* iprt message *******************************************/
#define  MSG_IPRT_START_ID                    (MSG_OSAL_START_ID+200)
#define  MSG_IPRT_SOCKET_ACCEPT               (MSG_IPRT_START_ID+1)
#define  MSG_IPRT_SOCKET_RECV                 (MSG_IPRT_START_ID+2)
#define  MSG_IPRT_SOCKET_CLOSE                (MSG_IPRT_START_ID+3)

/******************************************* TELNET message ***********************************/
#define  MSG_TELNET_START                     (MSG_OSAL_START_ID+300)
#define  MSG_TELNET_IDLE_TIME_OUT             (MSG_TELNET_START+1)
#define  MSG_TELNET_SETUP_LISTEN_TIME_OUT     (MSG_TELNET_START+2)

#define  MSG_TELNET_TO_EXEC                   (MSG_TELNET_START+6)
#define  MSG_EXEC_TO_TELNET                   (MSG_TELNET_START+7)
#define  MSG_CMD_RUN_TIME_OUT                 (MSG_TELNET_START+8)
#define  MSG_TELNET_EXEC_PRINT                (MSG_TELNET_START+9)

/******************************************* DAP message ***********************************/
#define  MSG_DAP_START                        (MSG_OSAL_START_ID+400)




#endif /* __OSAL_MSG_H__ */


