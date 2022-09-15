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

#ifndef   __OSAL_SSCHE_H__
#define   __OSAL_SSCHE_H__


#include "osalbasic.h"
#include "osalapi.h"

#define MSG_INVALID             ((uint8_t)0)
#define MSG_ASYN                ((uint8_t)1)
#define MSG_ASYN_TIMER          ((uint8_t)2)
#define MSG_SYN_REQ             ((uint8_t)3)
#define MSG_SYN_ACK             ((uint8_t)4)
#define MSG_BROADCAST           ((uint8_t)5)

#define ASYN_MSG_MAX_SZ         ((uint16_t)(0x8000-0x40))
#define SYN_REQ_MAX_SZ          ((uint16_t)(0x4000-0x40))
#define SYN_ACK_MAX_SZ          ((uint16_t)(0x4000-0x40))

#define OSAL_TASK_INIT          (0)
#define OSAL_TASK_RUN           (1)

#define OSAL_SCH_TASK_MIN_PRIORITY			    120
#define OSAL_SCH_MAX_PROCESS_NUM		        256
#define OSAL_SCH_RESERVE_SYNC_MSG_NUM			4
#define OSAL_SCH_RESERVE_ASYN_MSG_NUM			4
#define OSAL_SCH_MSG_VERSION_NUM                1
#define OSAL_SCH_MIN_PROCESS_PRIORITY           5

#define OSAL_SCH_MAX_MUTEX_NUM            	    127
#define OSAL_SCH_MAX_SEM_NUM              		127

#define OSAL_SCH_SYN_MSG_START               		0x5A
#define OSAL_SCH_SYN_MSG_END              	    	0xA5

#define RESERVED				                0x02
#define END                                     0xFFFF


#define	HOWMANY(x, y)	                      ((unsigned int)(((x)+((y)-1)))/(unsigned int)(y))
#define VERIFY_PCB(p)                         ( ( p== NULL)? 0:(p)->iPCBAddr == (unsigned long int)(p))


typedef struct tagOsalSchdMsg
{
    uint16_t        wMsgLen;
	uint8_t         bVersion;
	uint8_t         bType;
	osal_pid_t      tReceiver;
	osal_pid_t      tSender;
	uint32_t        dwMsgId;
	uint32_t        dwTid;
	uint16_t        wSyncMsgSeq;
	uint16_t        wReqSize;
	uint16_t        wAckSize;
	void          * pvReq;
	void          * pvAck;
} osal_schd_msg_t;


typedef struct tagTCB_STRUC
{
    unsigned long int    dwTCBAddr;
    pthread_t            dwThreadId;
    uint8_t              bUsed;
    char                 szName[OSAL_SCH_MAX_PROCESS_NAME_LENGTH + 1];
    uint8_t              bAttr;
    uint8_t              bPriLvl;
    uint32_t             dwStackSize;
    uint32_t             dwMsgQLen;
    sem_ctrl_blk_t      *ptCountSem;
	uint32_t             dwProcNum;
    uint32_t             dwPno4Head;
    uint32_t             dwPno4Tail;
    uint32_t             dwCurPno;
    int32_t              bRunState;
}osal_sch_tcb_t;


typedef	struct tagPCB_STRUC
{
    unsigned long int         iPCBAddr;
    uint32_t          dwTaskId;
	uint8_t			bUsed;
	char			szName[OSAL_SCH_MAX_PROCESS_NAME_LENGTH+1];
	PROCESS_ENTRY   pStartEntry;
	void		  * pvPrivateData;
	uint32_t			dwDataSize;
    uint32_t          dwNextPno;

	msg_queue_ctrl_blk_t *    	    AsynMsgQid;
    uint32_t			dwAsynMsgQLen;

	msg_queue_ctrl_blk_t *    		 SyncMsgQid;
    uint32_t			 dwSyncMsgQLen;
	uint8_t			 bSyncMsgState;
	uint16_t			 wSyncMsgSeq;
	sem_ctrl_blk_t *			 SyncMsgMutex;
	sem_ctrl_blk_t *			 SyncMsgWaitSem;
	osal_schd_msg_t * ptSyncAckMsg;
    uint16_t			 wSynAckSize;
    osal_pid_t			 tSelfPid;

	uint8_t		  	   bCurUserState;
	osal_schd_msg_t   * ptCurRcvMsg;
	osal_pid_t			   tCurSender;
	osal_pid_t			   tCurReceiver;
	uint32_t 			   dwCurRcvMsgId;
	uint32_t 			   dwCurSendMsgId;
	uint32_t             dwCurrTid;

	uint32_t			   dwRcvMsgCnt;
	uint32_t			   dwRcvByteCnt;
	uint32_t			   dwSendMsgCnt;
	uint32_t			   dwSendByteCnt;
	uint32_t			   dwSendErr;
	uint32_t			   dwRcvErr;
	uint32_t			   dwRtnErr;

	uint16_t             wMemUsedNum;
	uint32_t             dwMemUsedTotal;
	uint32_t             dwRunNum;
    uint32_t             dwLastRunNum;
    int32_t               bRunState;
} osal_sch_pcb_t;


extern int32_t OsSchPostMsgToTask(osal_sch_pcb_t * ptPCB, msg_queue_ctrl_blk_t *ptMsgQueCtrlBlk, char *buffer, uint32_t nBytes, int timeout);
#define PostMsgToTask(pcb, msgQid, pMsg, timeout)  OsSchPostMsgToTask(pcb, msgQid, (char *)&pMsg, sizeof(osal_schd_msg_t **), timeout)


typedef struct tagMTX_STRUC
{
	int32_t         blUsed;
	char         szName[OSAL_SCH_MAX_PROCESS_NAME_LENGTH+1];
	sem_ctrl_blk_t *       MutexId;
	uint32_t       dwPno;
	uint16_t       wUseCnt;
	uint16_t       wNext;
} T_OS_SCH_MUTEX ;


typedef struct tagSEM_STRUC
{
	int32_t         blUsed;
	char         szName[OSAL_SCH_MAX_PROCESS_NAME_LENGTH+1];
    sem_ctrl_blk_t *       SemId;
    uint16_t       wInitCount;
    uint16_t       wMaxCount;
    uint16_t       wNext;
} T_OS_SCH_SEM;



typedef  struct tagSpool
{
    uint16_t  wCount;
    uint16_t  wHead;
    uint16_t  wTail;
}T_OS_SCH_POOL;



extern  osal_sch_pcb_t  gtPCB[];


#endif   /*__OSAL_SSCHE_H__ */



