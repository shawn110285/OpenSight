
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

#ifndef  __IPRT_SOCKET_H__
#define  __IPRT_SOCKET_H__

#include "osalapi.h"


/*************************** socket½Ó¿Ú·â×°***************************/

#define IPRT_MAX_SOCKET_NODE_NUM              (uint32_t)30

#define IPRT_TCP_RCV_SIZE_DFLT                (uint32_t)(4 * 1024)
#define IPRT_TCP_SND_SIZE_DFLT                (uint32_t)(4 * 1024)
#define IPRT_TCP_RCV_SIZE_MAX                 (uint32_t)(8 * 1024)

#define IPRT_UDP_RCV_SIZE_DFLT                (uint32_t)(4 * 1024)

#define IPRT_SOCKET_NODE_USED                 (uint32_t)1
#define IPRT_SOCKET_NODE_UNUSED               (uint32_t)0


typedef enum
{
    IPRT_SOCKET_TYPE_UNKNOW,
    IPRT_SOCKET_TYPE_TCP,
    IPRT_SOCKET_TYPE_SCTP,
    IPRT_SOCKET_TYPE_UDP,
    IPRT_SOCKET_TYPE_RAW
}iprt_socket_type;


typedef enum
{
    IPRT_SOCKET_STATE_BLANK,
    IPRT_SOCKET_STATE_LISTEN,
    IPRT_SOCKET_STATE_ACTIVE
}iprt_socket_state;


typedef  struct tagIprtSocketNodePool
{
    sem_ctrl_blk_t *  tMutex;
    uint32_t  dwCount;
    uint32_t  dwHead;
    uint32_t  dwTail;
}T_IPRT_SOCKET_NODE_POOL;


typedef struct tag_T_IPRT_SOCKET_NODE
{
    uint8_t             ucIsUse;

    uint32_t            dwListenId;
    uint32_t            dwSelfId;
    uint32_t            dwNextInPool;


    int            iSockFd;
    osal_pid_t             tOwnerPid;

    iprt_socket_type  ucType;
    iprt_socket_state ucState;

    uint32_t            dwLocalIp;
    uint16_t            wLocalPort;

    uint32_t            dwRemoteIp;
    uint16_t            wRemotePort;

    uint32_t            dwRecvMsgNum;
    uint32_t            dwRecvMsgErrNum;
    uint32_t            dwRecvBytes;

    uint32_t            dwSendMsgNum;
    uint32_t            dwSendMsgErrNum;
    uint32_t            dwSendBytes;
}T_IPRT_SOCKET_NODE;



#endif  /* __IPRT_SOCKET_H__ */


