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

#ifndef _DOS_BufMgr_H
#define _DOS_BufMgr_H


#include "osalbasic.h"
#include "osalapi.h"

#define  BUF_NOT_MALLOC            0
#define  BUF_MALLOCED              1


#define  OSAL_BUF_MAX_NUM_PER_SIZE               (uint32_t)5*24000

typedef struct tagOsMsgBufHead
{
    uint8_t     ucUsedFlag;
    uint8_t     ucBufPoolId;
    uint16_t    wReserved;
    unsigned long int   iSign;
} osal_msg_buf_head_t;

typedef struct tagOsMsgBufTbl
{
   uint16_t    wBufSize;
   uint32_t    dwBufNum;
}osal_msg_buf_tbl_t;

typedef struct tagOsMsgBufPool
{
    uint32_t               dwSize;
    uint32_t               dwBufNum;
    sem_ctrl_blk_t *       SemWait;
    uint32_t               dwCount;
    uint32_t               dwHead;
    uint32_t               dwTail;
    osal_msg_buf_head_t   *aptMsgBuf[OSAL_BUF_MAX_NUM_PER_SIZE];
}osal_msg_buf_pool_t;

#endif
