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



#ifndef  __OSAL_BASIC_H__
#define  __OSAL_BASIC_H__

#include "osallinux.h"
#include "osalconst.h"

#define  MAX_SEM_COUNT             1000
#define  SEM_CTL_NO_USED           0
#define  SEM_CTL_USED              1


typedef struct tagSemCtrlBlk
{
   linux_sem_t          tSemId;
   uint8_t              ucUsed;
}sem_ctrl_blk_t;


#define  MAX_QUEUE_COUNT           500
#define  QUEUE_CTL_NO_USED         0
#define  QUEUE_CTL_USED            1

typedef struct tagDLElement
{
    struct  tagDLElement     * ptPreElement;
    struct  tagDLElement     * ptNextElement;
	uint8_t                  * pucPayLoad;
    uint32_t                   dwPayloadLength;
}double_link_element_t;


typedef struct tagQueueCtrlBlk
{
    uint8_t                  ucUsed;
    double_link_element_t  * ptHead;
    double_link_element_t  * ptTail;
    sem_ctrl_blk_t *         ptMuxSem;
    sem_ctrl_blk_t *         ptCountSem;
    uint32_t                 dwTotalCount;
    uint32_t                 dwPayloadSize;
    uint32_t                 dwUsedCount;
}msg_queue_ctrl_blk_t;



extern int32_t                 OsPortInit(void);
extern int32_t                 OsPortIsProcExist(uint16_t        wPort);

extern void *                  OsPort_Malloc(uint32_t  dwSize);
extern int32_t                 OsPort_Free(void* pucBuf);

extern int32_t                 OsPort_CreateTaskKey();
extern int32_t                 OsPort_SetTaskKeyparam(void *ptr);
extern void *                  OsPort_Get_Task_Key_param();

extern sem_ctrl_blk_t *        OsPort_SemMCreate ();
extern sem_ctrl_blk_t *        OsPort_SemBCreate(uint32_t initialState );
extern sem_ctrl_blk_t *        OsPort_SemCCreate(uint32_t initialState,uint32_t dwMaxValue);
extern int32_t                 OsPort_SemTake(sem_ctrl_blk_t * ptSemCtrlBlk, int timeout);
extern int32_t                 OsPort_SemGive(sem_ctrl_blk_t * ptSemCtrlBlk);

extern msg_queue_ctrl_blk_t *  OsPort_msgQCreate(uint32_t maxMsgs,uint32_t maxMsgLength);
extern int32_t                 OsPort_msgQSend(msg_queue_ctrl_blk_t *  ptMsgQueCtrlBlk,uint8_t * buffer,uint32_t  nBytes,int  timeout);
extern uint32_t                OsPort_msgQReceive(msg_queue_ctrl_blk_t *  ptMsgQueCtrlBlk,uint8_t *   buffer,uint32_t   maxNBytes,int  timeout);
extern uint32_t                OsPort_msgQNumMsgs(msg_queue_ctrl_blk_t *  ptMsgQueCtrlBlk);
extern int32_t                 OsPort_msgQGetStatus(msg_queue_ctrl_blk_t *  ptMsgQueCtrlBlk,uint32_t * pdwTotalCount, uint32_t * pdwUsedCount,uint32_t * pdwPayloadSize);

extern int                     OsPort_taskSpawn(char *name,FUNCPTR entryPt,int priority,int stackSize,long int arg);
extern int32_t                 OsPort_taskDelay(int dwMillSecond);
extern int32_t                 OsPort_taskSuspend( );
extern int                     OsPort_taskIdSelf(void);

extern int32_t                 OsPort_wdStart(int delay,FUNCPTR ptRoutine);
extern uint32_t                OsPort_GetTick();
extern int32_t                 OsPort_GetCurrentClock(uint16_t * pwYear,uint8_t * pbMon,uint8_t * pbDay,uint8_t * pbHour,uint8_t * pbMin,uint8_t * pbSec,uint8_t * pbWeek);
extern int32_t                 OsPort_SetCurrentClock(uint16_t wYear,uint8_t  bMon,uint8_t bDay,uint8_t bHour,uint8_t bMin,uint8_t bSec,uint8_t bWeek);

extern int32_t                 OsPort_RevocationPrivilege();
extern int                     OsPort_OpenFile(char *strFileName,int iFlag,int iMode);
extern int32_t                 OsPort_CloseFile(int iFileId);
extern int32_t                 OsPort_CreatFile(char *strFileName,int iFlag);
extern int32_t                 OsPort_DeleteFile(char *strFileName);
extern int32_t                 OsPort_RenameFile(char *strNewName,char * strOldName);
extern int32_t                 OsPort_GetFileLength(char *strFileName);
extern int32_t                 OsPort_CopyFile(char * strDestFile,char * strSrcFile);
extern int32_t                 OsPort_CreateDirectory(char * strName);
extern int32_t                 OsPort_DeleteDirectory(char * strName);
extern int32_t                 OsPort_MoveDirectory(char * strNewName,char * strOldName);
extern int32_t                 OsPort_RenameDirectory(char * strNewName,char * strOldName);

extern int32_t                 OsPort_InitNetwork();
extern int32_t                 OsPort_SetSocketNonBlock(int iSocketFd);
extern int                     OsPort_GetLastError();
extern void                    OsPort_HandSocketError();

extern uint32_t                OsPort_GetCpuUsage();

#endif /* __OSAL_BASIC_H__   */




