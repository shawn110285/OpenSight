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

#ifndef  __OSAL_CFG_H__
#define  __OSAL_CFG_H__

#include "../include/osalbasic.h"

#pragma pack (1)  /* due to not include osalapi.h,so instruct here */

typedef  struct  tagProcMutexParam
{
    uint16_t   wMutexPort;
}proc_mutex_param_t;

typedef  struct tagLogParm
{
    uint32_t    dwDbgPrnFlag;
	uint32_t    dwDbgPrnLevel;
}logger_param_t;

typedef struct tagTelnetServerParam
{
    uint32_t   dwTelnetServerFlag;
    uint32_t   dwTelnetServerIpAddr;
    uint16_t   wTelnetServerPort;
}telnet_svr_param_t;

typedef struct tagGdbServerParam
{
    uint32_t   dwGdbServerFlag;
    uint32_t   dwGdbServerIpAddr;
    uint16_t   wHartNum;                 // specify the HART(Core) number
    uint16_t   awGdbServerPort[100];      // the tcp ports for the service binded with HART
}gdb_svr_param_t;


typedef struct tagCoreSightCfgParam
{
    uint32_t   dwJtagClkDiv;          // 100
    uint32_t   dwDapId;               // 0x6ba00477
    uint32_t   dwJtagApNum;
    uint32_t   dwApbApNum;
    uint32_t   dwFunnelAddr;
    uint32_t   dwEtfAddr;
    uint32_t   dwReplicatorAddr;
    uint32_t   dwEtbAddr;
    uint32_t   dwTpiuAddr;
}coresight_param_t;


typedef struct tagSystemCfgParam
{
    proc_mutex_param_t      tProcMutexParam;
    logger_param_t          tLoggerParam;
	telnet_svr_param_t      tTelnetSvrParam;
	gdb_svr_param_t 		tGdbSvrParam;
    coresight_param_t       tCoreSightParam;
}config_param_t;

#endif   /* __OSAL_CFG_H__ */



