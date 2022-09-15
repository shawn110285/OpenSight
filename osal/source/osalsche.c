

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

#include "../include/osalsche.h"
#include "../../telnet_svr/include/telnetapi.h"



osal_sch_tcb_t             gtTCB[OSAL_SCH_MAX_PROCESS_NUM];
osal_sch_pcb_t		       gtPCB[OSAL_SCH_MAX_PROCESS_NUM];

T_OS_SCH_MUTEX             gtMutex[OSAL_SCH_MAX_MUTEX_NUM];
T_OS_SCH_SEM               gtSem[OSAL_SCH_MAX_SEM_NUM];
T_OS_SCH_POOL              gtMutexPool;
sem_ctrl_blk_t *           gptMutexPoolMutex;

T_OS_SCH_POOL              gtSemPool;
sem_ctrl_blk_t *           gptSemPoolMutex;


uint16_t                   gwSyncMsgSeq;
void                     * NullMsg = NULL;



static osal_sch_pcb_t * OsSchGetCurrentPCB();

extern void    *  OsPort_Malloc(uint32_t dwSize);
extern uint32_t   rabbitClientPostMsgToServer(osal_schd_msg_t *ptMsg, int dwCount10ms);



uint32_t          OsScheduleInit(void);
uint32_t          OsSchTaskEntry(uint16_t dwPno);
uint32_t          OsSchHandlePriorityMsg (osal_sch_pcb_t *ptPCB, osal_schd_msg_t *ptRcvMsg );
uint32_t          OsSchHandleNormalMsg (osal_sch_pcb_t *ptPCB, osal_schd_msg_t *ptRcvMsg);
uint32_t          OsSchHandleTimerMsg (osal_sch_pcb_t *ptPCB, osal_schd_msg_t *ptRcvMsg);
uint32_t          OsSchHandleSynMsg(osal_sch_pcb_t *ptPCB, osal_schd_msg_t *ptRcvMsg);
uint32_t          OsSchHandleSynMsgAck (osal_schd_msg_t *ptRcvMsg);

uint32_t          OsSchInitMutex();
uint32_t          OsSchInitSem();

uint32_t          OsSchCheckRecvMsg (osal_schd_msg_t *ptMsg);



uint32_t   OsScheduleInit()
{
    if((OsSchInitMutex()!=OSAL_OK)||(OsSchInitSem()!=OSAL_OK))
    {
      osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsScheduleInit,OsSchInitMutex or OsSchInitSem Failed!\n");
      return OSAL_ERROR;
    }
    //init the pcb
    memset(gtPCB, 0, sizeof(osal_sch_pcb_t)*OSAL_SCH_MAX_PROCESS_NUM);

	//init the tcb
    memset(gtTCB, 0, sizeof(osal_sch_tcb_t)*OSAL_SCH_MAX_PROCESS_NUM);


    return OSAL_OK;
}




uint32_t OsSchTaskEntry(uint16_t dwTaskNo)
{
    osal_sch_pcb_t    * ptPCB = NULL;
    osal_sch_tcb_t    * ptTCB = gtTCB + dwTaskNo;
    osal_schd_msg_t  * ptRcvMsg;
    uint32_t            dwCurPno = 0;
	uint32_t            dwProcLoop = 0;

    if(OSAL_OK != OsPort_SetTaskKeyparam(ptTCB))
    {
         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:OsSchTaskEntry, OsPort_SetTaskKeyparam task:%d \n", dwTaskNo);
         return OSAL_ERROR;
    }

    ptTCB->bRunState = OSAL_TASK_RUN;
    for (; OSAL_TRUE; )
    {
        if (OSAL_FALSE == OsPort_SemTake(ptTCB->ptCountSem, WAIT_FOREVER))
        {
            continue;
        }

        for(dwProcLoop=0; dwProcLoop<ptTCB->dwProcNum ; dwProcLoop++)
        {
            dwCurPno = ptTCB->dwCurPno;
            ptPCB = gtPCB + dwCurPno;

            if (ptPCB->AsynMsgQid == NULL)
            {
                osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:OsSchTaskEntry, Process %d AsynMsgQid == NULL\n", dwCurPno);
                return OSAL_ERROR;
            }
            if(ptPCB->SyncMsgQid == NULL)
            {
                osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:OsSchTaskEntry, Process %d SyncMsgQid == NULL\n", dwCurPno);
                return OSAL_ERROR;
            }

            if (OsPort_msgQNumMsgs(ptPCB->SyncMsgQid) > 0)
            {
                if (OsPort_msgQReceive(ptPCB->SyncMsgQid, (uint8_t *)&ptRcvMsg, sizeof(osal_schd_msg_t **), WAIT_FOREVER) != sizeof(osal_schd_msg_t **))  /*2012-01-13 NO_WAIT => WAIT_FOREVER*/
                {
                   osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:OsSchTaskEntry, OsPort_msgQReceive(Sync) Failed, pno = %d\n", dwCurPno);
                   ptPCB->dwRcvMsgCnt++;
                   ptPCB->dwRcvErr++;

                   ptTCB->dwCurPno = ptPCB->dwNextPno;
                   continue;
                }
				else
				{
                    OsSchHandlePriorityMsg(ptPCB, ptRcvMsg);
                    osal_msg_free_buf(ptRcvMsg);
                    ptTCB->dwCurPno = ptPCB->dwNextPno;
                    break;
				}
            }


            if (OsPort_msgQNumMsgs(ptPCB->AsynMsgQid) > 0)
            {
                if (OsPort_msgQReceive(ptPCB->AsynMsgQid, (uint8_t *)&ptRcvMsg, sizeof(osal_schd_msg_t **), WAIT_FOREVER) != sizeof(osal_schd_msg_t **))
                {
                    osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:OsSchTaskEntry, OsPort_msgQReceive(Asyn) Failed, pno = %d\n", dwCurPno);
                    ptPCB->dwRcvMsgCnt++;
                    ptPCB->dwRcvErr++;

                    ptTCB->dwCurPno = ptPCB->dwNextPno;
                    continue;
                }

                if (ptRcvMsg == NULL)
                {
                    osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:OsSchTaskEntry, Recv NULL msg, pno = %d\n", dwCurPno);
                    ptTCB->dwCurPno = ptPCB->dwNextPno;
                    break;
                }

                if (ptRcvMsg->bType == MSG_ASYN)
                {
                    OsSchHandleNormalMsg(ptPCB, ptRcvMsg);
                    osal_msg_free_buf(ptRcvMsg);
                    ptTCB->dwCurPno = ptPCB->dwNextPno;
                    break;
                }
            }
            ptTCB->dwCurPno = ptPCB->dwNextPno;
        }//for(dwProcLoop=0; dwProcLoop<ptTCB->dwProcNum ; dwProcLoop++)
        if(dwProcLoop >= ptTCB->dwProcNum)
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:OsSchTaskEntry, scheduled all the process without qualified, task = %s\n", ptTCB->szName);
        }
     }
}





uint32_t  OsSchHandlePriorityMsg (osal_sch_pcb_t *ptPCB,  osal_schd_msg_t *ptRcvMsg )
{
    if(ptRcvMsg == NULL)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchHandlePriorityMsg,Recv Msg is NULL\n");
        return OSAL_ERROR;
    }


    switch (ptRcvMsg->bType)
    {
    case MSG_ASYN_TIMER:
        return OsSchHandleTimerMsg (ptPCB, ptRcvMsg);

    case MSG_SYN_REQ:
       return OsSchHandleSynMsg (ptPCB, (osal_schd_msg_t *)ptRcvMsg);

    case MSG_BROADCAST:
        return OsSchHandleNormalMsg (ptPCB, ptRcvMsg);

    case MSG_INVALID:
        ptPCB->dwRcvMsgCnt ++;
        ptPCB->dwRcvErr ++;
        break;

    default :
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchHandlePriorityMsg,unknown MSG type!\n");
        ptPCB->dwRcvMsgCnt ++;
        ptPCB->dwRcvErr ++;
        break;
    }
    return OSAL_ERROR;
}









uint32_t  OsSchHandleTimerMsg (osal_sch_pcb_t *ptPCB,osal_schd_msg_t *ptRcvMsg)
{
    uint32_t  status = OSAL_OK;

    if(ptRcvMsg->wMsgLen < sizeof(osal_schd_msg_t))
    {
         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchHandleNormalMsg,MsgLen less sizeof(osal_schd_msg_t)!\n");
         ptPCB->dwRcvMsgCnt ++;
         ptPCB->dwRcvErr ++;
         return OSAL_ERROR;
    }

    if (ptRcvMsg->bVersion != OSAL_SCH_MSG_VERSION_NUM)
    {
    	ptPCB->dwRcvMsgCnt ++;
        ptPCB->dwRcvErr++;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchHandleNormalMsg,version=%d error\n", ptRcvMsg->bVersion);
        return OSAL_ERROR;
    }

    ptPCB->dwRcvMsgCnt++;
    ptPCB->dwRcvByteCnt += ptRcvMsg->wMsgLen;

    ptPCB->ptCurRcvMsg   = ptRcvMsg;
    ptPCB->dwCurRcvMsgId = ptRcvMsg->dwMsgId;
    ptPCB->dwCurrTid     = ptRcvMsg->dwTid;

    ptPCB->tCurSender    = ptRcvMsg->tSender;

    if (ptPCB->bUsed == OSAL_TRUE)
    {
        ptPCB->bRunState = OSAL_TRUE;
        ptPCB->dwRunNum ++;

        ptPCB->pStartEntry();
        ptPCB->bRunState = OSAL_FALSE;
    }
    else
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchHandleNormalMsg,Process %d, not in used\n", ptPCB->tSelfPid.dwPno);
        status = OSAL_ERROR;
    }
    ptPCB->ptCurRcvMsg = NULL;

    return status;
}


uint32_t  OsSchHandleNormalMsg (osal_sch_pcb_t *ptPCB,osal_schd_msg_t *ptRcvMsg)
{
    uint32_t  status = OSAL_OK;

    if(ptRcvMsg->wMsgLen < sizeof(osal_schd_msg_t))
    {
         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchHandleNormalMsg,MsgLen less sizeof(osal_schd_msg_t)!\n");
         ptPCB->dwRcvMsgCnt ++;
         ptPCB->dwRcvErr ++;
         return OSAL_ERROR;
    }

    if (ptRcvMsg->bVersion != OSAL_SCH_MSG_VERSION_NUM)
    {
    	ptPCB->dwRcvMsgCnt ++;
        ptPCB->dwRcvErr++;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchHandleNormalMsg,version=%d error\n", ptRcvMsg->bVersion);
        return OSAL_ERROR;
    }

    ptPCB->dwRcvMsgCnt++;
    ptPCB->dwRcvByteCnt += ptRcvMsg->wMsgLen;

    ptPCB->ptCurRcvMsg   = ptRcvMsg;
    ptPCB->dwCurRcvMsgId = ptRcvMsg->dwMsgId;
    ptPCB->dwCurrTid     = ptRcvMsg->dwTid;
    ptPCB->tCurSender    = ptRcvMsg->tSender;

    if (ptPCB->bUsed == OSAL_TRUE)
    {
        ptPCB->bRunState = OSAL_TRUE;
        ptPCB->dwRunNum ++;

        ptPCB->pStartEntry();
        ptPCB->bRunState = OSAL_FALSE;
    }
    else
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchHandleNormalMsg,Process %d, not in used\n", ptPCB->tSelfPid.dwPno);
        status = OSAL_ERROR;
    }
    ptPCB->ptCurRcvMsg = NULL;

    return status;
}




uint32_t  OsSchHandleSynMsg (osal_sch_pcb_t *ptSelfPCB,osal_schd_msg_t *ptRcvMsg)
{
    osal_sch_pcb_t               *   ptSenderPCB;
    uint32_t                         status = OSAL_ERROR;

    if(ptRcvMsg->tSender.dwPno >= OSAL_SCH_MAX_PROCESS_NUM)
    {
       ptSelfPCB->dwRcvMsgCnt ++;
       ptSelfPCB->dwRcvErr ++;
       osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchHandleSynMsg,Sync Msg's Sender Pno's %d Wrong!\n",ptRcvMsg->tSender.dwPno);
       return OSAL_ERROR;
    }

    ptSenderPCB = &gtPCB[ptRcvMsg->tSender.dwPno];

    if (ptSenderPCB->bUsed != OSAL_TRUE)
    {
       ptSelfPCB->dwRcvMsgCnt ++;
       ptSelfPCB->dwRcvErr ++;
       return OSAL_ERROR;
    }

    OsPort_SemTake (ptSenderPCB->SyncMsgMutex, WAIT_FOREVER);

    if (ptSenderPCB->bSyncMsgState == OSAL_SCH_SYN_MSG_START)
    {
        ptSelfPCB->wSynAckSize = ptRcvMsg->wAckSize;
        status = OsSchHandleNormalMsg (ptSelfPCB,ptRcvMsg);
        if (status == OSAL_OK)
        {
             ptRcvMsg->wAckSize       = ptSelfPCB->wSynAckSize;
             ptSenderPCB->wSyncMsgSeq = ptRcvMsg->wSyncMsgSeq;
        }
        ptSenderPCB->bSyncMsgState = OSAL_SCH_SYN_MSG_END;
        OsPort_SemGive (ptSenderPCB->SyncMsgWaitSem);
    }

    OsPort_SemGive (ptSenderPCB->SyncMsgMutex);
    return status;
}



static osal_sch_pcb_t * OsSchGetCurrentPCB()
{
    pthread_t dwThreadId;
    osal_sch_tcb_t  *ptTCB;

    dwThreadId = OsPort_taskIdSelf();

    ptTCB =(osal_sch_tcb_t  *) OsPort_Get_Task_Key_param();
    if(ptTCB)
    {
        if (ptTCB->dwThreadId == dwThreadId)
        {
            if(ptTCB->dwCurPno < OSAL_SCH_MAX_PROCESS_NUM)
                return &gtPCB[ptTCB->dwCurPno];
        }
    }
    return NULL;
}


uint32_t  OsCoreSendMsgToSysCtrl(uint32_t  dwMsgId)
{
    osal_pid_t        tSender;
    osal_schd_msg_t      * ptMsg;
    osal_sch_pcb_t      * ptPCB;

    tSender.dwPno      = 0;

    ptPCB=&gtPCB[1];
    if (ptPCB->bUsed != OSAL_TRUE)
           return OSAL_ERROR;

    ptMsg = (osal_schd_msg_t *)osal_msg_malloc_buf(sizeof(osal_schd_msg_t));
    if (ptMsg == NULL)
    {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsCoreSendMsgToSysCtrl,osal_msg_malloc_buf return NULL!\n");
            return OSAL_ERROR;
    }

    ptMsg->wMsgLen                 = sizeof(osal_schd_msg_t);
    ptMsg->bVersion                = OSAL_SCH_MSG_VERSION_NUM;
    ptMsg->bType                   = MSG_BROADCAST;
    ptMsg->tReceiver               = ptPCB->tSelfPid;
    ptMsg->tSender                 = tSender;
    ptMsg->dwMsgId                 = dwMsgId;

    if (OSAL_FALSE == PostMsgToTask (ptPCB, ptPCB->SyncMsgQid, ptMsg, WAIT_FOREVER))
    {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsCoreSendMsgToSysCtrl, PostMsgToTask return error!\n");
            osal_msg_free_buf(ptMsg);
			return OSAL_ERROR;
    }

    return OSAL_OK;
}


uint32_t    OsSchInitMutex()
{
	uint16_t i;
	for(i=0;i<OSAL_SCH_MAX_MUTEX_NUM;i++)
	{
		gtMutex[i].blUsed         =  OSAL_FALSE;
		memcpy(gtMutex[i].szName,"unnamed",strlen("unnamed"));
		if((gtMutex[i].MutexId =  OsPort_SemMCreate())==NULL)
		{
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchInitSem,OsPort_SemMCreate Failed!\n");
			return OSAL_ERROR;
		}
		gtMutex[i].dwPno          = 0;
		gtMutex[i].wUseCnt       = 0;
		if(i<OSAL_SCH_MAX_MUTEX_NUM-1)
		    gtMutex[i].wNext = i+1;
		else
		    gtMutex[i].wNext = END;
	}

    gptMutexPoolMutex=OsPort_SemMCreate();
	if(gptMutexPoolMutex==NULL)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchInitSem,OsPort_SemMCreate Failed!\n");
		return OSAL_ERROR;
	}
	gtMutexPool.wCount = OSAL_SCH_MAX_MUTEX_NUM;
	gtMutexPool.wHead  = 0;
	gtMutexPool.wTail  = 0;
        return OSAL_OK;

}





uint32_t    OsSchInitSem()
{
	uint16_t i;
	for(i=0;i<OSAL_SCH_MAX_SEM_NUM;i++)
	{
		gtSem[i].blUsed           =  OSAL_FALSE;
		memcpy(gtSem[i].szName,"unnamed",strlen("unnamed"));
		if((gtSem[i].SemId        = OsPort_SemBCreate(SEM_EMPTY))==NULL)
		{
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchInitSem,OsPort_SemBCreate Failed!\n");
			return OSAL_ERROR;
		}

		gtSem[i].wInitCount     = 1;
		gtSem[i].wMaxCount      = 1;

		if(i<OSAL_SCH_MAX_SEM_NUM-1)
		    gtSem[i].wNext = i+1;
		else
		    gtSem[i].wNext = END;
	}

	if((gptSemPoolMutex=OsPort_SemMCreate())==NULL)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchInitSem,OsPort_SemBCreate Failed!\n");
		return OSAL_ERROR;
	}

	gtSemPool.wCount = OSAL_SCH_MAX_SEM_NUM;
	gtSemPool.wHead  = 0;
	gtSemPool.wTail  = 0;
    return OSAL_OK;

}



uint32_t  OsSchPostMsgByLocation (osal_pid_t tReceiver,uint32_t  dwMsgId,uint8_t bMsgType,void *pvData,uint32_t dwSize, int dwCount10ms)
{
    osal_sch_pcb_t   * ptSelfPCB = OsSchGetCurrentPCB();
    osal_sch_pcb_t   * ptRcvPCB = NULL;
    uint32_t           dwPno;
    osal_schd_msg_t * ptMsg;
    osal_pid_t            testPid;

    if (ptSelfPCB == NULL)
        return OSAL_ERROR;

    dwPno = tReceiver.dwPno;

    if (dwPno >= OSAL_SCH_MAX_PROCESS_NUM)
    {
        ptSelfPCB->dwSendErr++;
        if(osal_task_get_self_pid(&testPid) != OSAL_OK)
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchPostMsgByLocation,osal_task_get_self_pid Failed! \n");
            return OSAL_ERROR_PNO;
        }
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchPostMsgByLocation,Task %d called,the receiver pno=%d error!\n",testPid.dwPno,dwPno);
        return OSAL_ERROR_PNO;
    }

    if (pvData == NULL)
        dwSize = 0;

    if (dwSize <= 0)
        pvData = NULL;

    if (dwSize > ASYN_MSG_MAX_SZ)
    {
        ptSelfPCB->dwSendErr++;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchPostMsgByLocation,data size error,size=%d!\n",dwSize);
        return OSAL_ERROR_SIZE;
    }



        ptRcvPCB = &gtPCB[dwPno];
        if (ptRcvPCB->bUsed != OSAL_TRUE)
        {
            ptSelfPCB->dwSendMsgCnt ++;
            ptSelfPCB->dwSendErr++;
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchPostMsgByLocation,receiver %d not in used,msgid=%d!\n", dwPno,dwMsgId);
            return OSAL_ERROR_PNO;
        }


    ptMsg = (osal_schd_msg_t *)osal_msg_malloc_buf(sizeof(osal_schd_msg_t) + dwSize);
    if (ptMsg == NULL)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchPostMsgByLocation,osal_msg_malloc_buf Error!\n");
        ptSelfPCB->dwSendMsgCnt ++;
        ptSelfPCB->dwSendErr++;
        return OSAL_ERROR;
    }

    ptMsg->wMsgLen               = sizeof(osal_schd_msg_t) + dwSize;
    ptMsg->bVersion              = OSAL_SCH_MSG_VERSION_NUM;
    ptMsg->bType                 = bMsgType;
    ptMsg->tReceiver             = tReceiver;
    ptMsg->tSender               = ptSelfPCB->tSelfPid;
    ptMsg->dwMsgId               = dwMsgId;
    ptMsg->dwTid                 = OSAL_INVALID_UINT32;

    ptMsg->wReqSize              = (uint16_t) dwSize;

    if (pvData != NULL)
    {
        memcpy ((char *)(ptMsg+1),(char *)pvData,dwSize);
        ptMsg->pvReq     = ptMsg+1;
    }
    else
    {
    	ptMsg->pvReq     = NULL;
    }
    ptSelfPCB->dwSendMsgCnt++;
    ptSelfPCB->dwSendByteCnt += ptMsg->wMsgLen;

    ptSelfPCB->dwCurSendMsgId = dwMsgId;
    ptSelfPCB->tCurReceiver  = tReceiver;

        if (OSAL_FALSE == PostMsgToTask (ptRcvPCB, ptRcvPCB->AsynMsgQid,ptMsg,dwCount10ms))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchPostMsgByLocation,PostMsgToTask[pno=%d] Failed!\n",tReceiver.dwPno);
            osal_msg_free_buf (ptMsg);
            ptSelfPCB->dwSendErr++;
            return   OSAL_ERROR;
        }
        else
        {
            return OSAL_OK;
        }

}


uint32_t  OsSchSendMsgLocally(osal_schd_msg_t  *ptMsg,osal_sch_pcb_t *ptSelfPCB,osal_sch_pcb_t *ptRcvPCB,uint32_t dwCount10ms)
{
    uint32_t       status;
    uint16_t       wMsgSeq = ptMsg->wSyncMsgSeq;

    ptSelfPCB->bSyncMsgState = OSAL_SCH_SYN_MSG_START;
    ptSelfPCB->wSyncMsgSeq   = 0x8000 ^ wMsgSeq;

    OsPort_SemTake (ptSelfPCB->SyncMsgWaitSem, WAIT_FOREVER);

    if (OsPort_msgQNumMsgs(ptRcvPCB->SyncMsgQid)>= ptRcvPCB->dwSyncMsgQLen - OSAL_SCH_RESERVE_SYNC_MSG_NUM)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchSendMsgLocally: Sync Msg Queue full!\n");
        ptSelfPCB->dwSendMsgCnt++;
        ptSelfPCB->dwSendErr ++;
        osal_msg_free_buf (ptMsg);
        return OSAL_ERROR;
    }

    if (OSAL_FALSE == PostMsgToTask (ptRcvPCB, ptRcvPCB->SyncMsgQid, ptMsg, WAIT_FOREVER))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchSendMsgLocally: PostMsgToTask return error!\n");
        ptSelfPCB->dwSendMsgCnt++;
        ptSelfPCB->dwSendErr ++;
        osal_msg_free_buf (ptMsg);
        return OSAL_ERROR;
    }
	else
	{
        if(OSAL_FALSE == PostMsgToTask (ptRcvPCB, ptRcvPCB->AsynMsgQid, NullMsg, WAIT_FOREVER))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchSendMsgLocally: PostMsgToTask(NULL MSG) return error!\n");
        }

        if(OSAL_FALSE == OsPort_SemTake (ptSelfPCB->SyncMsgWaitSem, dwCount10ms))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchSendMsgLocally: OsPort_SemTake(ptSelfPCB->SyncMsgWaitSem) return error!\n");
            return OSAL_ERROR;
        }

        if(OSAL_FALSE == OsPort_SemTake (ptSelfPCB->SyncMsgMutex, WAIT_FOREVER))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsSchSendMsgLocally: OsPort_SemTake(ptSelfPCB->SyncMsgMutex) return error!\n");
            return OSAL_ERROR;
        }

        if (ptSelfPCB->bSyncMsgState == OSAL_SCH_SYN_MSG_END)
        {
            status = ptSelfPCB->wSyncMsgSeq == wMsgSeq ? OSAL_OK : OSAL_ERROR;
        }
        else
        {
            ptSelfPCB->bSyncMsgState = OSAL_SCH_SYN_MSG_END;
            ptMsg->bType             = MSG_INVALID;
            status                   = OSAL_ERROR_TIMEOUT;
        }
        OsPort_SemGive (ptSelfPCB->SyncMsgMutex);
        return status;
	}
}


uint32_t  OsSchHandleSynMsgAck (osal_schd_msg_t *ptRcvMsg)
{
    osal_sch_pcb_t          *ptRcvPCB;

    osal_dbg_assert (ptRcvMsg->tReceiver.dwPno < OSAL_SCH_MAX_PROCESS_NUM);

    ptRcvPCB = &gtPCB[ptRcvMsg->tReceiver.dwPno];
    if (ptRcvPCB->bUsed != OSAL_TRUE)
        return OSAL_ERROR;

    OsPort_SemTake (ptRcvPCB->SyncMsgMutex, WAIT_FOREVER);

    if (ptRcvPCB->bSyncMsgState == OSAL_SCH_SYN_MSG_START)
    {
        ptRcvPCB->ptSyncAckMsg  = ptRcvMsg;
        ptRcvPCB->wSyncMsgSeq   = ptRcvMsg->wSyncMsgSeq;
        ptRcvPCB->bSyncMsgState = OSAL_SCH_SYN_MSG_END;

        return OSAL_OK;
    }
    OsPort_SemGive (ptRcvPCB->SyncMsgWaitSem);

    OsPort_SemGive (ptRcvPCB->SyncMsgMutex);

    return OSAL_ERROR;
}



uint32_t  OsSchCheckRecvMsg (osal_schd_msg_t *ptMsg)
{
    uint16_t      minLen, maxLen;
    osal_sch_pcb_t    *ptRcvPCB;

    if (ptMsg->tReceiver.dwPno >= OSAL_SCH_MAX_PROCESS_NUM)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:receiver pno=%d error!\n",ptMsg->tReceiver.dwPno);
        return OSAL_ERROR;
    }

    ptRcvPCB = &gtPCB[ptMsg->tReceiver.dwPno];
    if (ptRcvPCB->bUsed != OSAL_TRUE)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:receiver %d not in used!\n",ptMsg->tReceiver.dwPno);
        return OSAL_ERROR;
    }

    switch (ptMsg->bType)
    {
        case MSG_ASYN:
            minLen = sizeof(osal_schd_msg_t);
            maxLen = sizeof(osal_schd_msg_t) + ASYN_MSG_MAX_SZ;
            break;


        case MSG_SYN_REQ:
            minLen = sizeof(osal_schd_msg_t);
            maxLen = sizeof(osal_schd_msg_t) + SYN_REQ_MAX_SZ;
            break;

        case MSG_SYN_ACK:
            minLen = sizeof(osal_schd_msg_t);
            maxLen = sizeof(osal_schd_msg_t) + SYN_ACK_MAX_SZ;
            break;

        default:
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Msg Type %d error!\n",ptMsg->bType);
            return OSAL_ERROR;
    }

    if((ptMsg->wMsgLen >= minLen) &&(ptMsg->wMsgLen <= maxLen))

		return OSAL_OK;
	else
	    return OSAL_ERROR;
}



int32_t OsSchPostMsgToTask(osal_sch_pcb_t * ptPCB, msg_queue_ctrl_blk_t *ptMsgQueCtrlBlk, char *buffer, uint32_t nBytes, int timeout)
{
    if (!VERIFY_PCB(ptPCB))
    {
        return OSAL_FALSE;
    }

    if ((ptPCB->dwTaskId >= OSAL_SCH_MAX_PROCESS_NUM) || (NULL == gtTCB[ptPCB->dwTaskId].ptCountSem))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:OsSchPostMsgToTask [taskidx = %u pno = %u] Failed!\n", ptPCB->dwTaskId, ptPCB->tSelfPid.dwPno);
        return OSAL_FALSE;
    }

    if (OSAL_FALSE == OsPort_msgQSend(ptMsgQueCtrlBlk,(uint8_t *) buffer, nBytes, timeout))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:OsSchPostMsgToTask [pno = %u] Failed!\n", ptPCB->tSelfPid.dwPno);
        return OSAL_FALSE;
    }
    else
    {
        OsPort_SemGive(gtTCB[ptPCB->dwTaskId].ptCountSem);
        return OSAL_TRUE;
    }
}









uint32_t   osal_task_create(T_TaskParam       * ptTaskParam)
{
	uint16_t		     wTaskIndex = 0;
	uint16_t           wProcIndex = 0;
	uint16_t           wProcLoop  = 0;
	int 		     tThreadId;

    void           *  pvPrivateData = NULL;
	osal_sch_tcb_t   *  ptTCB = NULL;
    osal_sch_pcb_t   *  ptPCB = NULL;
	T_ProcessParam *  ptProcessParam =NULL;

	 for (wTaskIndex = 0, ptTCB = &gtTCB[0]; wTaskIndex < OSAL_SCH_MAX_PROCESS_NUM; wTaskIndex++, ptTCB++)
	 {
		 if (ptTCB->bUsed != OSAL_FALSE)
		 {
			 continue;
		 }

         ptTCB->bUsed = OSAL_TRUE;
		 strncpy(ptTCB->szName, ptTaskParam->szName, OSAL_SCH_MAX_PROCESS_NAME_LENGTH);
		 ptTCB->szName[OSAL_SCH_MAX_PROCESS_NAME_LENGTH] = '\0';

		 ptTCB->bAttr		 = ptTaskParam->bAttr;

		 if (ptTaskParam->bPriLvl < OSAL_SCH_MIN_PROCESS_PRIORITY)
			 ptTCB->bPriLvl = OSAL_SCH_MIN_PROCESS_PRIORITY;
		 else
			 ptTCB->bPriLvl = ptTaskParam->bPriLvl;

		 /* 8kb~512mb */
		 ptTaskParam->dwStkSize = ptTaskParam->dwStkSize * 1024;	/* the unit is K bytes*/
		 if (ptTaskParam->dwStkSize < 0x2000)
			 ptTCB->dwStackSize = 0x2000;
		 else if (ptTaskParam->dwStkSize > 0x20000000)
			 ptTCB->dwStackSize = 0x20000000;
		 else
			 ptTCB->dwStackSize   =(uint16_t) (1+ptTaskParam->dwStkSize/1024)*1024;


		 for(wProcLoop =0; wProcLoop<OSAL_SCH_MAX_PROCESS_NUM_PER_TASK;wProcLoop++)
		 {
			  ptProcessParam =&(ptTaskParam->atProcessParam[wProcLoop]);
			  if(ptProcessParam->ucUsed)
			  {
				  for (wProcIndex =1, ptPCB=&gtPCB[1]; wProcIndex < OSAL_SCH_MAX_PROCESS_NUM; wProcIndex++, ptPCB++)
				  {
					  if (ptPCB->bUsed != OSAL_FALSE)
						  continue;

					  ptPCB->bUsed = OSAL_TRUE;
					  strncpy(ptPCB->szName, ptProcessParam->szName, OSAL_SCH_MAX_PROCESS_NAME_LENGTH);
					  ptPCB->szName[OSAL_SCH_MAX_PROCESS_NAME_LENGTH] = '\0';
					  ptPCB->pStartEntry  = ptProcessParam->pStartEntry;

					  ptPCB->dwDataSize    = ptProcessParam->dwDataSize*1024;

					  if (ptProcessParam->dwAsynMsgQLen < 8)
						  ptPCB->dwAsynMsgQLen = 8;
					  else if (ptProcessParam->dwAsynMsgQLen > 10000)
						  ptPCB->dwAsynMsgQLen = 10000;
					  else
						  ptPCB->dwAsynMsgQLen = ptProcessParam->dwAsynMsgQLen;

					  if (ptProcessParam->dwSyncMsgQLen < 8)
						  ptPCB->dwSyncMsgQLen = 8;
					  else if (ptProcessParam->dwSyncMsgQLen > 320)
						  ptPCB->dwSyncMsgQLen = 320;
					  else
						  ptPCB->dwSyncMsgQLen = ptProcessParam->dwSyncMsgQLen;

					  ptPCB->dwSyncMsgQLen += OSAL_SCH_RESERVE_SYNC_MSG_NUM;
					  ptPCB->dwAsynMsgQLen += OSAL_SCH_RESERVE_ASYN_MSG_NUM;

					  ptPCB->AsynMsgQid = OsPort_msgQCreate (ptPCB->dwAsynMsgQLen, sizeof(osal_schd_msg_t *));
					  if (ptPCB->AsynMsgQid == NULL)
					  {
						  ptPCB->bUsed = OSAL_FALSE;
						  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_task_create(%s), OsPort_msgQCreate(Asyn) Failed \n", ptProcessParam->szName);
						   return OSAL_ERROR;
					  }

					  ptPCB->SyncMsgQid 	=	OsPort_msgQCreate (ptPCB->dwSyncMsgQLen,sizeof(osal_schd_msg_t *));
					  if (ptPCB->SyncMsgQid == NULL)
					  {
						  ptPCB->bUsed = OSAL_FALSE;
						  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_task_create(%s), OsPort_msgQCreate(Sync) Failed\n", ptProcessParam->szName);
						   return OSAL_ERROR;
					  }

					  ptPCB->SyncMsgMutex	=	OsPort_SemMCreate ();
					  ptPCB->SyncMsgWaitSem =	OsPort_SemBCreate (SEM_EMPTY);

					  ptPCB->tSelfPid.dwPno 			  = wProcIndex;

					  pvPrivateData = (uint8_t *)OsPort_Malloc(ptPCB->dwDataSize);
					  if(NULL == pvPrivateData)
					  {
						   osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_task_create(%s), alloc private data Failed\n", ptProcessParam->szName);
						   ptPCB->bUsed = OSAL_FALSE;
						   return OSAL_ERROR;
					  }
					  else
					  {
						   ptPCB->pvPrivateData = pvPrivateData;
					  }

					  ptPCB->bSyncMsgState	= OSAL_SCH_SYN_MSG_END;
					  ptPCB->wSyncMsgSeq	= 0;

					  ptPCB->ptSyncAckMsg = NULL;
					  ptPCB->wSynAckSize  = 0;

					  ptPCB->bCurUserState = 0;

					  ptPCB->ptCurRcvMsg   = NULL;

					  ptPCB->tCurSender.dwPno		 = 0;

					  ptPCB->tCurReceiver.dwPno 	 = 0;

					  ptPCB->dwCurRcvMsgId	= 0;
					  ptPCB->dwCurSendMsgId = 0;
					  ptPCB->dwCurrTid		= OSAL_INVALID_UINT32;

					  ptPCB->dwRcvMsgCnt   = 0;
					  ptPCB->dwRcvByteCnt  = 0;
					  ptPCB->dwRcvErr	   = 0;

					  ptPCB->dwSendMsgCnt  = 0;
					  ptPCB->dwSendByteCnt = 0;
					  ptPCB->dwSendErr	   = 0;

					  ptPCB->dwRtnErr	   = 0;
					  ptPCB->dwRunNum	   = 0;

					  ptTCB->dwMsgQLen += ptPCB->dwSyncMsgQLen + ptPCB->dwAsynMsgQLen;
					  // update the process num attached with the task
					  ptTCB->dwProcNum ++;

                      //update the curr pno
                      if(ptTCB->dwCurPno == 0)
					     ptTCB->dwCurPno = wProcIndex;

                      // update the tcb job list
					  if(ptTCB->dwPno4Head ==0)
					  {
					  	 ptTCB->dwPno4Head = wProcIndex;
						 ptTCB->dwPno4Tail = wProcIndex;
					     ptPCB->dwNextPno  = ptTCB->dwPno4Head;
					  }
					  else
					  {
				          gtPCB[ptTCB->dwPno4Tail].dwNextPno = wProcIndex;
						  ptPCB->dwNextPno  = ptTCB->dwPno4Head;
						  ptTCB->dwPno4Tail = wProcIndex;
					  }

					  ptPCB->dwTaskId = wTaskIndex;
					  ptPCB->iPCBAddr = (unsigned long int)(ptPCB);
					  break;
				 }//for (wProcLoop =1, ptPCB=&gtPCB[1]; wProcLoop < OSAL_SCH_MAX_PROCESS_NUM; wProcLoop++, ptPCB++)
		         if(wProcIndex >= OSAL_SCH_MAX_PROCESS_NUM)
		         {
		             osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_task_create(%s) Failed, no availabe free PCB\n", ptTCB->szName);
		             return OSAL_ERROR;
		         }
			 } // if(ptProcessParam.ucUsed)
		}//for(wProcIndex=0; wProcIndex<OSAL_SCH_MAX_PROCESS_NUM_PER_TASK;wProcIndex++)

		ptTCB->ptCountSem = OsPort_SemCCreate(SEM_EMPTY, ptTCB->dwMsgQLen);
		if (NULL == ptTCB->ptCountSem)
		{
			 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:osal_task_create, Create count semaphore failure!\n");
			 return OSAL_ERROR;
		}

		 ptTCB->bRunState = OSAL_TASK_INIT;

		 tThreadId = OsPort_taskSpawn(ptTCB->szName, (FUNCPTR)OsSchTaskEntry, (int)ptTCB->bPriLvl, (int)ptTCB->dwStackSize, wTaskIndex);
		 if (tThreadId == 0)
		 {
			 ptTCB->bUsed = 0;
			 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[OSAL]:osal_task_create, OsPort_taskSpawn errno = %d, TaskName = %s\n", errno, ptTCB->szName);
		 }
		 ptTCB->dwThreadId = tThreadId;
		 ptTCB->dwTCBAddr  = (unsigned long int)(ptTCB);

		 return OSAL_OK;
	 }

	 return OSAL_ERROR;
}


uint32_t  osal_task_get_pno_by_name (char  *IszProcessName)
{
    osal_sch_pcb_t  *  ptPCB = NULL;
    uint32_t           pno =0 ;

    ptPCB  = &gtPCB[0];
    for (pno=0; pno < OSAL_SCH_MAX_PROCESS_NUM; pno++, ptPCB++)
    {
        if (ptPCB->bUsed == OSAL_TRUE && (strcmp(ptPCB->szName, IszProcessName)==0))
             return pno;
    }
    osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_task_get_pno_by_name for %s error\n", IszProcessName);
    return  OSAL_ERR_INVALID_PNO;
}


uint32_t  osal_task_get_name_by_pno(uint32_t dwPno, char * strProcName, uint8_t ucBufLen)
{
    osal_sch_pcb_t  *  ptPCB = NULL;
    ptPCB  = &gtPCB[0];

     if(dwPno >= OSAL_SCH_MAX_PROCESS_NUM)
  	 	return OSAL_ERR_INVALID_PNO;

	 memset(strProcName,0,ucBufLen);

     if(ptPCB[dwPno].bUsed == OSAL_TRUE)
     {
          strncpy(strProcName,ptPCB[dwPno].szName,ucBufLen);
     }
	 else
	 {
          strncpy(strProcName,"unknow\0",ucBufLen);
	 }

 	 return OSAL_OK;

}


uint32_t   osal_task_get_self_pno ()
{
      osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();

      if(!VERIFY_PCB(ptPCB))
      {
           return OSAL_ERR_INVALID_PNO;
      }

      return ptPCB->tSelfPid.dwPno;
}


uint32_t  osal_task_get_self_pid (osal_pid_t *OptPid)
{
      osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();

      if(!VERIFY_PCB(ptPCB))
      {
      	   OptPid ->dwPno        = 0;
           return OSAL_ERROR;
      }

      *OptPid =ptPCB->tSelfPid;
      return OSAL_OK;
}


uint32_t   osal_task_get_self_state()
{
      osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
      if(!VERIFY_PCB(ptPCB))
      {
           return OSAL_ERROR;
      }
      return ptPCB->bCurUserState;
}


uint32_t   osal_task_get_state_by_pid(osal_pid_t ItPid)
{
      osal_sch_pcb_t * ptPCB=NULL;
      if(ItPid.dwPno >= OSAL_SCH_MAX_PROCESS_NUM)
          return  OSAL_ERROR;

      ptPCB = &gtPCB[ItPid.dwPno] ;
      if(!VERIFY_PCB(ptPCB))
           return OSAL_ERROR;

      if(ptPCB->bUsed)
      {
        return ptPCB->bCurUserState;;
      }

      return OSAL_ERROR;
}


void *  osal_task_get_private_data_ptr()
{
      osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
      if(!VERIFY_PCB(ptPCB))
            return NULL;
      return  ptPCB->pvPrivateData;

}


void *  osal_task_get_private_data_ptr_by_pno(uint32_t dwPno)
{
    osal_sch_pcb_t  *ptPCB = NULL;
    if (dwPno >= OSAL_SCH_MAX_PROCESS_NUM)
    {
        return NULL;
    }

    ptPCB = &gtPCB[dwPno];
    if(!VERIFY_PCB(ptPCB))
    {
        return NULL;
    }

    return  ptPCB->pvPrivateData;
}


uint32_t  osal_task_get_private_data_size( )
{
      osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
      if(!VERIFY_PCB(ptPCB))
         return 0;
      return  ptPCB->dwDataSize;
}


uint32_t   osal_task_set_next_state(uint32_t  IbUserState)
{
      osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
      if(!VERIFY_PCB(ptPCB))
           return OSAL_ERROR;
      ptPCB->bCurUserState =(uint8_t)IbUserState;
      return OSAL_OK;
}


uint32_t  osal_task_delay (uint32_t  dwMillSecond)
{
    if (dwMillSecond > 3000)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_task_delay %d ms, > 3s", dwMillSecond);
        dwMillSecond = 3000;
    }

	if(OsPort_taskDelay (dwMillSecond)== OSAL_TRUE)
		return OSAL_OK;
	else
		return OSAL_ERROR;
}


uint32_t   osal_mutex_create ()
{
     uint32_t    dwPno;
     uint16_t  wIndex;
     osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
     if(!VERIFY_PCB(ptPCB))
     {
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_create ERROR:only the OSS Process Can do this!\n");
         return OSAL_ERR_INVALID_MUTEX_ID;
     }

     if(OsPort_SemTake(gptMutexPoolMutex,WAIT_FOREVER)== OSAL_FALSE)
	 	 return OSAL_ERR_INVALID_MUTEX_ID;

     if(gtMutexPool.wCount ==1)
     {
         OsPort_SemGive(gptMutexPoolMutex);
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_create ERROR:Mutex Pool exhausted!!\n");
         return OSAL_ERR_INVALID_MUTEX_ID;
     }
     dwPno = ptPCB->tSelfPid.dwPno;

     gtMutexPool.wCount --;
     wIndex = gtMutexPool.wHead;
     gtMutexPool.wHead = gtMutex[wIndex].wNext;

     if(OsPort_SemGive(gptMutexPoolMutex) == OSAL_FALSE)
	 	 return OSAL_ERR_INVALID_MUTEX_ID;

     gtMutex[wIndex].blUsed =OSAL_TRUE;
     gtMutex[wIndex].dwPno = dwPno;

     return  wIndex;
}


uint32_t   osal_mutex_delete (uint32_t  IMutexID)
{
     osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
     if(!VERIFY_PCB(ptPCB))
     {
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_delete:Only the OSS process can do this!\n");
         return OSAL_ERROR;
     }
     if(IMutexID >=OSAL_SCH_MAX_MUTEX_NUM)
     {
       	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_delete:Mutex ID ERROR!\n");
     	return  OSAL_ERR_INVALID_MUTEX_ID;
     }
     if(gtMutex[IMutexID].blUsed ==OSAL_FALSE)
     {
     	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_delete:Mutex ID ERROR!\n");
     	return  OSAL_ERR_INVALID_MUTEX_ID;
     }


     if(OsPort_SemTake(gptMutexPoolMutex,WAIT_FOREVER) == OSAL_FALSE)
	 	return OSAL_ERROR;

     gtMutexPool.wCount ++;
     gtMutex[IMutexID].wNext = gtMutexPool.wHead;
     gtMutexPool.wHead =(uint16_t) IMutexID;
     if(OsPort_SemGive(gptMutexPoolMutex) == OSAL_FALSE)
	 	return OSAL_ERROR;

     gtMutex[IMutexID].blUsed = OSAL_FALSE;
     return OSAL_OK;
}



uint32_t  osal_mutex_enter (uint32_t  IMutexID)
{
     osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
     if(!VERIFY_PCB(ptPCB))
     {
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_enter:Only the OSS process can do this!\n");
         return OSAL_ERROR;
     }
     if(IMutexID >=OSAL_SCH_MAX_MUTEX_NUM)
     {
       	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_enter:Mutex ID ERROR!\n");
     	return  OSAL_ERR_INVALID_MUTEX_ID;
     }
     if(gtMutex[IMutexID].blUsed ==OSAL_FALSE)
     {
     	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_enter: Mutex ID ERROR!\n");
     	return  OSAL_ERR_INVALID_MUTEX_ID;
     }

     if(OsPort_SemTake(gtMutex[IMutexID].MutexId,WAIT_FOREVER) == OSAL_FALSE)
     	return OSAL_ERROR;

     return OSAL_OK;
}


uint32_t   osal_mutex_leave (uint32_t  IMutexID)
{
     osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
     if(!VERIFY_PCB(ptPCB))
     {
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_leave:Only the OSS process can do this!\n");
         return OSAL_ERROR;
     }
     if(IMutexID >=OSAL_SCH_MAX_MUTEX_NUM)
     {
       	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_leave:Mutex ID ERROR!\n");
     	return  OSAL_ERR_INVALID_MUTEX_ID;
     }
     if(gtMutex[IMutexID].blUsed ==OSAL_FALSE)
     {
     	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_mutex_leave: Mutex ID ERROR!\n");
     	return  OSAL_ERR_INVALID_MUTEX_ID;
     }
     if(OsPort_SemGive(gtMutex[IMutexID].MutexId) == OSAL_FALSE )
    	return OSAL_ERROR;
     else
        return OSAL_OK;

}


uint32_t   osal_sem_create()
{
     uint16_t  wIndex;
     osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
     if(!VERIFY_PCB(ptPCB))
     {
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_create ERROR:only the OSS Process Can do this!\n");
         return OSAL_ERR_INVALID_SEM_ID;
     }

     if(OsPort_SemTake(gptSemPoolMutex,WAIT_FOREVER) == OSAL_FALSE)
         return OSAL_ERR_INVALID_SEM_ID;

     if(gtSemPool.wCount ==1)
     {
         OsPort_SemGive(gptSemPoolMutex);
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_create ERROR:sem Pool exhausted!!\n");
         return OSAL_ERR_INVALID_SEM_ID;
     }

     gtSemPool.wCount --;
     wIndex = gtSemPool.wHead;
     gtSemPool.wHead = gtSem[wIndex].wNext;

     if(OsPort_SemGive(gptSemPoolMutex) == OSAL_FALSE)
            return OSAL_ERR_INVALID_SEM_ID;

     gtSem[wIndex].blUsed =OSAL_TRUE;
     return  wIndex;
}


uint32_t   osal_sem_delete (uint32_t  ISemID)
{
     osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();

     if(!VERIFY_PCB(ptPCB))
     {
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_delete:Only the OSS process can do this!\n");
         return OSAL_ERROR;
     }
     if(ISemID >=OSAL_SCH_MAX_SEM_NUM)
     {
       	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_delete:SEM ID ERROR!\n");
     	return  OSAL_ERR_INVALID_SEM_ID;
     }
     if(gtSem[ISemID].blUsed ==OSAL_FALSE)
     {
     	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_delete:Sem ID ERROR!\n");
     	return  OSAL_ERR_INVALID_SEM_ID;
     }

     if(OsPort_SemTake(gptSemPoolMutex,WAIT_FOREVER) == OSAL_FALSE)
	 	return OSAL_ERROR;

     gtSemPool.wCount ++;
     gtSem[ISemID].wNext = gtSemPool.wHead;
     gtSemPool.wHead =(uint16_t) ISemID;
     if(OsPort_SemGive(gptSemPoolMutex) == OSAL_FALSE)
	 	return OSAL_ERROR;

     gtSem[ISemID].blUsed = OSAL_FALSE;
     return  OSAL_OK;

}


uint32_t  osal_sem_take (uint32_t  ISemID, uint16_t IwMilliSeconds)
{

     osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
     if(!VERIFY_PCB(ptPCB))
     {
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_take:Only the OSS process can do this!\n");
         return OSAL_ERROR;
     }
     if(ISemID >=OSAL_SCH_MAX_SEM_NUM)
     {
       	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_take:SEM ID ERROR!\n");
     	return  OSAL_ERR_INVALID_SEM_ID;
     }
     if(gtSem[ISemID].blUsed ==OSAL_FALSE)
     {
     	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_take: Sem ID ERROR!\n");
     	return  OSAL_ERR_INVALID_SEM_ID;
     }

     if(OsPort_SemTake(gtSem[ISemID].SemId,IwMilliSeconds)== OSAL_FALSE)
     {
     	return OSAL_ERROR;
     }
     return OSAL_OK;
}


uint32_t  osal_sem_give (uint32_t   ISemID)
{

     osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
     if(!VERIFY_PCB(ptPCB))
     {
     	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_give:Only the OSS process can do this!\n");
         return OSAL_ERROR;
     }
     if(ISemID >=OSAL_SCH_MAX_SEM_NUM)
     {
       	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_give:SEM ID ERROR!\n");
     	return  OSAL_ERR_INVALID_SEM_ID;
     }
     if(gtSem[ISemID].blUsed ==OSAL_FALSE)
     {
     	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_sem_give:Sem ID ERROR!\n");
     	return  OSAL_ERR_INVALID_SEM_ID;
     }
     if(OsPort_SemGive(gtSem[ISemID].SemId) == OSAL_FALSE)
     {
     	return OSAL_ERROR;
     }
     return OSAL_OK;
}


uint32_t  osal_msg_get_sender (osal_pid_t *OptPid)
{

      osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
      if(!VERIFY_PCB(ptPCB))
      {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_get_sender: This Process is not a valid process \n");
           return OSAL_ERROR;
      }
      else
      {
          * OptPid = ptPCB->tCurSender;
            return OSAL_OK;
      }
}


uint32_t  osal_msg_get_id ()
{
      osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
      if(!VERIFY_PCB(ptPCB))
      {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_get_id: This Process is not a valid process \n");
           return 0;
      }
	  else
	  {
           return  ptPCB->dwCurRcvMsgId;
	  }
}


uint32_t   osal_timer_get_current_tid(uint32_t   *OpdwTid)
{
      osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
      if(!VERIFY_PCB(ptPCB))
      {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_get_current_tid: This Process is not a valid process \n");
           *OpdwTid = OSAL_ERR_INVALID_TIMER_ID;
		    return OSAL_ERROR;
      }
	  else
	  {
	        if(ptPCB->dwCurrTid == OSAL_INVALID_UINT32)
	        {
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_get_current_tid: not a timer msg \n");
                 *OpdwTid = OSAL_ERR_INVALID_TIMER_ID;
		          return OSAL_ERROR;
	        }
			else
			{
	              *OpdwTid = ptPCB->dwCurrTid;
                   return  OSAL_OK;
			}
	  }
}


void * osal_msg_get_data ()
{
       osal_schd_msg_t *ptMsg;
       osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
       if(!VERIFY_PCB(ptPCB))
       {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_get_data: This Process is not a valid process \n");
           return NULL;
       }

	   ptMsg = ptPCB->ptCurRcvMsg;
       return  ptMsg->pvReq;
}


uint32_t osal_msg_get_data_length()
{

       osal_schd_msg_t * ptMsg;
       osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
       if(!VERIFY_PCB(ptPCB))
       {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_get_data_length: This Process is not a valid process \n");
           return 0;
       }
       ptMsg = ptPCB->ptCurRcvMsg;
       return ptMsg->wReqSize;
}



void  * osal_msg_get_ack_data_ptr( )
{
       osal_schd_msg_t *ptMsg;
       osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();

       if(!VERIFY_PCB(ptPCB))
       {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_get_ack_data_ptr: This Process is not a valid process \n");
		   return NULL;
       }
       ptMsg = ptPCB->ptCurRcvMsg;
       return ptMsg->pvAck;

}


uint16_t  osal_msg_get_ack_data_length( )
{
       osal_schd_msg_t * ptMsg;
       osal_sch_pcb_t * ptPCB = OsSchGetCurrentPCB();
       if(!VERIFY_PCB(ptPCB))
       {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_get_data_length: This Process is not a valid process\n");
           return 0;
       }
       ptMsg = ptPCB->ptCurRcvMsg;
       return ptMsg->wAckSize;
}


uint32_t  osal_msg_broadcast(uint32_t  dwMsgId)
{
    int          i;
    osal_pid_t        tSender;
    osal_schd_msg_t      * ptMsg;
    osal_sch_pcb_t      * ptPCB;

    ptPCB = OsSchGetCurrentPCB();
    if(!VERIFY_PCB(ptPCB))
    {
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:BroadCastMsg,Only osal Process Can do this!\n");
    	return OSAL_ERROR_PNO;
    }
    tSender = ptPCB ->tSelfPid;

    for (i=0, ptPCB=&gtPCB[0]; i<OSAL_SCH_MAX_PROCESS_NUM; i++, ptPCB++)
    {
        if (ptPCB->bUsed != OSAL_TRUE)
            continue;

        if(ptPCB->tSelfPid.dwPno==tSender.dwPno)
            continue;

        ptMsg = (osal_schd_msg_t *)osal_msg_malloc_buf(sizeof(osal_schd_msg_t));
        if (ptMsg == NULL)
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:BroadCastMsg,osal_msg_malloc_buf return NULL!\n");
            return OSAL_ERROR;
        }

        ptMsg->wMsgLen                 = sizeof(osal_schd_msg_t);
        ptMsg->bVersion                = OSAL_SCH_MSG_VERSION_NUM;
        ptMsg->bType                   = MSG_BROADCAST;
        ptMsg->tReceiver               = ptPCB->tSelfPid;
        ptMsg->tSender                  = tSender;
        ptMsg->dwMsgId                 = dwMsgId;
        ptMsg->wReqSize              =0;
        ptMsg->pvReq                    =  NULL;

        if (OSAL_FALSE == PostMsgToTask (ptPCB, ptPCB->SyncMsgQid, ptMsg, WAIT_FOREVER))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:BroadCastMsg,PostMsgToTask return error!\n");
            osal_msg_free_buf (ptMsg);
        }

    }
    return OSAL_OK;
}


uint32_t  osal_msg_post_out(osal_pid_t ItReceiver, uint32_t  IwMsgId, void * pvData, uint32_t IdwDataSize)
{
    return OsSchPostMsgByLocation(ItReceiver, IwMsgId, MSG_ASYN, pvData,IdwDataSize, WAIT_FOREVER);
}




uint32_t  osal_msg_deliver_from_task(osal_pid_t tReceiver,uint32_t  dwMsgId,void *pvData,uint32_t dwSize)
{
    osal_sch_pcb_t   * ptRcvPCB = NULL;
    uint32_t           dwPno;
    osal_schd_msg_t * ptMsg;


    dwPno = tReceiver.dwPno;

    if (dwPno >= OSAL_SCH_MAX_PROCESS_NUM)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_deliver_from_task,the receiver pno=%d error!\n",dwPno);
        return OSAL_ERROR_PNO;
    }

    if (pvData == NULL)
        dwSize = 0;

    if (dwSize <= 0)
        pvData = NULL;

    if (dwSize > ASYN_MSG_MAX_SZ)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_deliver_from_task,data size error!\n");
        return OSAL_ERROR_SIZE;
    }


        ptRcvPCB = &gtPCB[dwPno];
        if (ptRcvPCB->bUsed != OSAL_TRUE)
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_deliver_from_task,receiver %d not in used,msgid=%d!\n", dwPno,dwMsgId);
            return OSAL_ERROR_PNO;
        }

    ptMsg = (osal_schd_msg_t *)osal_msg_malloc_buf(sizeof(osal_schd_msg_t) + dwSize);
    if (ptMsg == NULL)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_deliver_from_task,osal_msg_malloc_buf Error!\n");
        return OSAL_ERROR;
    }

    ptMsg->wMsgLen               = sizeof(osal_schd_msg_t) + dwSize;
    ptMsg->bVersion              = OSAL_SCH_MSG_VERSION_NUM;
    ptMsg->bType                 = MSG_ASYN;
    ptMsg->tReceiver             = tReceiver;
	ptMsg->tSender.dwPno         = 0;
    ptMsg->dwMsgId               = dwMsgId;

    ptMsg->dwTid                 = OSAL_INVALID_UINT32;

    ptMsg->wReqSize              = (uint16_t) dwSize;

    if (pvData != NULL)
    {
        memcpy ((char *)(ptMsg+1),(char *)pvData,dwSize);
        ptMsg->pvReq     = ptMsg+1;
    }
    else
    {
    	  ptMsg->pvReq     = NULL;
    }


           if (OSAL_FALSE == PostMsgToTask (ptRcvPCB, ptRcvPCB->AsynMsgQid,ptMsg,WAIT_FOREVER))
           {
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_deliver_from_task,PostMsgToTask[pno=%d] Failed!\n",tReceiver.dwPno);
                  osal_msg_free_buf (ptMsg);
                  return   OSAL_ERROR;
           }
		   else
		   {
                  return OSAL_OK;
		   }

}


uint32_t  osal_msg_send_out (osal_pid_t ItReceiver,uint32_t  IwMsgId,void  * IpvData,uint16_t IwDataSize, void  * OpvAck, uint16_t * IOpwAckSize)
{
    osal_sch_pcb_t *    ptSelfPCB = OsSchGetCurrentPCB();
    osal_pid_t             tSelfPid;
    osal_sch_pcb_t *    ptRcvPCB  = NULL;
    osal_sch_pcb_t *    ptPCB = NULL;
    uint32_t            dwPno;
    osal_schd_msg_t   *ptMsg;
    uint16_t            wAckSize = *IOpwAckSize;
    uint32_t            status;
    uint16_t            IwCount10ms = 100;

    if(!VERIFY_PCB(ptSelfPCB))
    {
       osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out,Source Process is not a OSS Process!\n");
       return OSAL_ERROR;
    }



    dwPno = ItReceiver.dwPno;
    if (dwPno >= OSAL_SCH_MAX_PROCESS_NUM)
    {
        ptSelfPCB->dwSendErr++;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out, receiver pno %d error!\n", dwPno);
        return OSAL_ERROR_PNO;
    }

   if(dwPno ==( ptSelfPCB->tSelfPid).dwPno)
   {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out, Process DeadLock\n");
        return  OSAL_ERROR_DEADLOCK;
   }

    tSelfPid = ptSelfPCB->tSelfPid;
    ptPCB = &gtPCB[dwPno];
    if(!VERIFY_PCB(ptPCB))
    {
   		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out,Only OSS Process Can do this\n");
   		return OSAL_ERROR;
    }

    while(ptPCB->bSyncMsgState == OSAL_SCH_SYN_MSG_START)
    {
	    if(ptPCB->tCurReceiver.dwPno == tSelfPid.dwPno)
        {
             	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out, Process DeadLock\n");
                return  OSAL_ERROR_DEADLOCK;
        }
        dwPno = ptPCB->tCurReceiver.dwPno;
        ptPCB = &gtPCB[dwPno];
        if(!VERIFY_PCB(ptPCB))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out,Only OSS Process Can do this\n");
            return OSAL_ERROR;
        }
    }

    if (IpvData == NULL && IwDataSize != 0)
    {
        ptSelfPCB->dwSendErr++;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out, pData=NULL, but dataSize=%d\n",IwDataSize);
        return OSAL_ERROR_SIZE;
    }

    if (IwDataSize == 0)
           IpvData = NULL;

    if (OpvAck == NULL && (*IOpwAckSize != 0))
    {
        ptSelfPCB->dwSendErr++;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out,OpvAck=NULL, but IOpwAckSize=%d\n",IOpwAckSize);
        return OSAL_ERROR_SIZE;
    }

    if ( *IOpwAckSize == 0)    OpvAck = NULL;

    if (IwDataSize > SYN_REQ_MAX_SZ || *IOpwAckSize > SYN_ACK_MAX_SZ)
    {
        ptSelfPCB->dwSendErr++;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out,dataSize=%d or ackSize=%d error!\n",IwDataSize, *IOpwAckSize);
        return OSAL_ERROR_SIZE;
    }

    dwPno = ItReceiver.dwPno;

        ptRcvPCB = &gtPCB[dwPno];
        if(!VERIFY_PCB(ptRcvPCB))
        {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out,Dest Process is not a OSS Process!\n");
           return OSAL_ERROR_PNO;
        }

        if (ptRcvPCB->bUsed != OSAL_TRUE)
        {
            ptSelfPCB->dwSendErr++;
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out,receiver %d not in used!\n",dwPno);
            return OSAL_ERROR_PNO;
        }

        if (ptRcvPCB->SyncMsgQid == NULL)
        {
            ptSelfPCB->dwSendErr++;
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_send_out,receiver %d cannot rcv syn msg error,SyncMsgQid == NULL!\n",dwPno);
            return OSAL_ERROR;
        }

        ptMsg = (osal_schd_msg_t *)osal_msg_malloc_buf(sizeof(osal_schd_msg_t));
        if (ptMsg == NULL)
        {
            ptSelfPCB->dwSendErr++;
            return OSAL_ERROR;
        }
        ptMsg->wMsgLen = sizeof(osal_schd_msg_t);
        ptMsg->pvReq   = IpvData;
        ptMsg->pvAck   = OpvAck;

        ptMsg->bVersion              = OSAL_SCH_MSG_VERSION_NUM;
        ptMsg->bType                 = MSG_SYN_REQ;
        ptMsg->tReceiver             = ItReceiver;
        ptMsg->tSender               = ptSelfPCB->tSelfPid;
        ptMsg->dwMsgId               = IwMsgId;

        ptMsg->wSyncMsgSeq            = ++gwSyncMsgSeq;
        ptMsg->wReqSize               = IwDataSize;
        ptMsg->wAckSize               = *IOpwAckSize;

        ptSelfPCB->dwSendMsgCnt++;
        ptSelfPCB->dwSendByteCnt     += IwDataSize;

        ptSelfPCB->dwCurSendMsgId      = IwMsgId;
        ptSelfPCB->tCurReceiver       = ItReceiver;


        status = OsSchSendMsgLocally (ptMsg, ptSelfPCB, ptRcvPCB, IwCount10ms);
        if (OSAL_OK == status)
            *IOpwAckSize = ptMsg->wAckSize;

        return status;

}



uint32_t  osal_msg_set_ack_data_size (uint16_t IwAckSize)
{
    osal_sch_pcb_t *ptCurPCB = OsSchGetCurrentPCB();
    osal_schd_msg_t *ptMsg;

    if(!VERIFY_PCB(ptCurPCB))
       return OSAL_ERROR;

    ptMsg = ptCurPCB->ptCurRcvMsg;
    if (ptMsg->bType == MSG_SYN_REQ)
    {
        if (IwAckSize <= (ptMsg->wAckSize))
        {
            ptCurPCB->wSynAckSize = IwAckSize;
            return OSAL_OK;
        }
        return OSAL_ERROR_SIZE;
    }
    return  OSAL_ERROR;
}





uint32_t  osal_msg_get_self_queue_status(uint32_t	* pdwSynUsedCount,uint32_t *pdwASynUsedCount)
{

	 uint32_t		dwSynTotalCount =0;
	 uint32_t		dwSynPayloadSize=0;

	 uint32_t		dwASynTotalCount =0;
	 uint32_t		dwASynPayloadSize=0;


	 osal_sch_pcb_t   *  ptCurPCB = OsSchGetCurrentPCB();

	 if(!VERIFY_PCB(ptCurPCB))
	      return OSAL_ERROR;


     if(OSAL_FALSE == OsPort_msgQGetStatus(ptCurPCB->SyncMsgQid, &dwSynTotalCount, pdwSynUsedCount, &dwSynPayloadSize))
		 return OSAL_ERROR;

	 if(OSAL_FALSE == OsPort_msgQGetStatus(ptCurPCB->AsynMsgQid, &dwASynTotalCount, pdwASynUsedCount, &dwASynPayloadSize))
		 return OSAL_ERROR;

	 return OSAL_OK;
}





uint32_t  osal_msg_get_queue_status_by_pno(uint32_t  dwPno, uint32_t * pdwSynFreeCount,uint32_t  *pdwASynFreeCount)
{

	 uint32_t		dwSynTotalCount =0;
	 uint32_t		dwSynPayloadSize=0;
	 uint32_t      dwSynUsedCnt=0;

	 uint32_t		dwASynTotalCount =0;
	 uint32_t		dwASynPayloadSize=0;
	 uint32_t      dwASynUsedCnt=0;


	 osal_sch_pcb_t		* ptPCB;

     if( dwPno >= OSAL_SCH_MAX_PROCESS_NUM)
     {
	 	 return  OSAL_ERROR;
     }

	 ptPCB	= &gtPCB[dwPno];
	 if(ptPCB->bUsed == OSAL_TRUE)
	 {
             if(OSAL_FALSE == OsPort_msgQGetStatus(ptPCB->SyncMsgQid, &dwSynTotalCount, &dwSynUsedCnt, &dwSynPayloadSize))
        		 return OSAL_ERROR;

        	 if(OSAL_FALSE == OsPort_msgQGetStatus(ptPCB->AsynMsgQid, &dwASynTotalCount,&dwASynUsedCnt, &dwASynPayloadSize))
        		 return OSAL_ERROR;
	 }

     * pdwSynFreeCount  = dwSynTotalCount - dwSynUsedCnt;
     * pdwASynFreeCount = dwASynTotalCount- dwASynUsedCnt;

	 return OSAL_OK;
}



uint32_t osal_SendAsynMsg(osal_pid_t tReceiver, uint32_t dwMsgId, void *pvData, uint16_t wDataSize)
{
    osal_schd_msg_t   *ptMsg = NULL;
    osal_sch_pcb_t     *ptPCB = NULL;

    ptPCB = &gtPCB[tReceiver.dwPno];
    if (OSAL_TRUE != ptPCB->bUsed)
    {
        return OSAL_ERROR;
    }
    ptMsg = (osal_schd_msg_t *)osal_msg_malloc_buf(sizeof(osal_schd_msg_t) + wDataSize);
    if (NULL == ptMsg)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_SendAsynMsg, osal_msg_malloc_buf return NULL!\n");
        return OSAL_ERROR;
    }

    ptMsg->wMsgLen         = sizeof(osal_schd_msg_t) + wDataSize;
    ptMsg->bVersion        = OSAL_SCH_MSG_VERSION_NUM;
    ptMsg->bType           = MSG_ASYN;
    ptMsg->tReceiver       = tReceiver;
    ptMsg->dwMsgId         = dwMsgId;
    ptMsg->wReqSize        = wDataSize;

    if (NULL == pvData)
    {
        ptMsg->pvReq = NULL;
    }
    else
    {
        ptMsg->pvReq = (void *)(ptMsg + 1);
        memcpy((void *)(ptMsg + 1), pvData, wDataSize);
    }

    if (OSAL_FALSE == PostMsgToTask(ptPCB, ptPCB->AsynMsgQid, ptMsg, WAIT_FOREVER))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_SendAsynMsg, PostMsgToTask return error!\n");
        return OSAL_ERROR;
    }
    return OSAL_OK;
}



void  osal_cmd_show_Queue(int  argc, char * argv[])
{
   uint16_t                 wIndex=0;
   osal_sch_pcb_t       * ptPCB;
   char                 szName[10];

   uint32_t	            dwSynTotalCount =0;
   uint32_t 	            dwSynUsedCount  =0;
   uint32_t 	            dwSynPayloadSize=0;

   uint32_t	            dwASynTotalCount =0;
   uint32_t 	            dwASynUsedCount  =0;
   uint32_t 	            dwASynPayloadSize=0;

   telnet_print("  +----+-----------+---------------+--------------+----------------+--------------+\r\n");
   telnet_print("  |Pno |Name       |Syn-Que(Total) |Syn-Que(Used) |Asyn-Que(Total) |Asyn-Que(Used)| \r\n");
   telnet_print("  +----+-----------+---------------+--------------+----------------+--------------+\r\n");

   for(wIndex=0;wIndex<OSAL_SCH_MAX_PROCESS_NUM;wIndex++)
   {
         ptPCB  = &gtPCB[wIndex];
		 if(ptPCB->bUsed == OSAL_TRUE)
		 {
               strncpy(szName,ptPCB->szName,10);
			   szName[9]=0;

               OsPort_msgQGetStatus(ptPCB->SyncMsgQid, &dwSynTotalCount, &dwSynUsedCount, &dwSynPayloadSize);
               OsPort_msgQGetStatus(ptPCB->AsynMsgQid, &dwASynTotalCount, &dwASynUsedCount, &dwASynPayloadSize);

               telnet_print("  |%-3u |%-10s |%-14u |%-13u |%-15u |%-14u|\r\n",
		 	            wIndex,
		 	            szName,
		 	            dwSynTotalCount,
		 	            dwSynUsedCount,
		 	            dwASynTotalCount,
		 	            dwASynUsedCount);
		 }
   }

   telnet_print("  +----+-----------+---------------+--------------+----------------+--------------+\r\n");

}





void  osal_cmd_show_proc_statics(int  argc, char * argv[])
{
      uint16_t				       wIndex=0;
      osal_sch_pcb_t	   * ptPCB;
      char				          szName[10];
	  char               szStatue[10];

      telnet_print("  +----+-----------+--------+-----------+-----------+-----------+-----------+-----------+-----------+\r\n");
      telnet_print("  |Pno |Name       |task_id |RecvMsg    |RecvErr    |SendMsg    |SendErr    |runNum     |Curr-Status|\r\n");
      telnet_print("  +----+-----------+--------+-----------+-----------+-----------+-----------+-----------+-----------+\r\n");

      for(wIndex=0;wIndex<OSAL_SCH_MAX_PROCESS_NUM;wIndex++)
      {
      	  ptPCB  = &gtPCB[wIndex];
      	  if(ptPCB->bUsed == OSAL_TRUE)
      	  {
      			strncpy(szName,ptPCB->szName,10);
      			szName[9]=0;

				memset(szStatue,0,10);
                if(ptPCB->bRunState==OSAL_FALSE)
                {
                    strncpy(szStatue,"OSAL_FALSE\0",10);
                }
				else
				{
                    strncpy(szStatue,"OSAL_TRUE\0",10);
				}

      			telnet_print("  |%-3u |%-10s |%-7u |0x%-8x |0x%-8x |0x%-8x |0x%-8x |0x%-8x |%-11s|\r\n",
      					 wIndex,
      					 szName,
      					 ptPCB->dwTaskId,
      					 ptPCB->dwRcvMsgCnt,
      					 ptPCB->dwRcvErr,
					     ptPCB->dwSendMsgCnt,
      					 ptPCB->dwSendErr,
      					 ptPCB->dwRunNum,
      					 szStatue);
      	  }
      }
      telnet_print("  +----+-----------+--------+-----------+-----------+-----------+-----------+-----------+-----------+\r\n");

}



void  osal_cmd_show_task_statics(int  argc, char * argv[])
{
      uint16_t		     wIndex=0;
      osal_sch_tcb_t	   * ptTCB = NULL;
      osal_sch_pcb_t	   * ptPCB = NULL;

      char				 szName[10];
	  char               strProcList[50];
	  uint8_t              ucLoop = 0;

      telnet_print("  +----+-----------+-----------+----------+-----+-----+---------+-----------------------------------------+\r\n");
      telnet_print("  |idx |Name       |thread-id  |Curr-Proc |head |Tail |Proc-Num |Process-List                             |\r\n");
      telnet_print("  +----+-----------+-----------+----------+-----+-----+---------+-----------------------------------------+\r\n");

      for(wIndex=0;wIndex<OSAL_SCH_MAX_PROCESS_NUM;wIndex++)
      {
      	  ptTCB  = &gtTCB[wIndex];
      	  if(ptTCB->bUsed == OSAL_TRUE)
      	  {
      			strncpy(szName,ptTCB->szName,10);
      			szName[9]=0;

				memset(strProcList, 0, 50);
				if(ptTCB->dwCurPno < OSAL_SCH_MAX_PROCESS_NUM)
				{
				    ptPCB = &gtPCB[ptTCB->dwCurPno];
					snprintf(strProcList, sizeof(strProcList) - 1, "%s-", ptPCB->szName);
					for(ucLoop=1; ucLoop<ptTCB->dwProcNum; ucLoop++ )
					{
						if(ptPCB->dwNextPno < OSAL_SCH_MAX_PROCESS_NUM)
						{
						    ptPCB = &gtPCB[ptPCB->dwNextPno];
							snprintf(strProcList+strlen(strProcList), sizeof(strProcList) - 1, "%s-", ptPCB->szName);
						}
					}
     			    telnet_print("  |%-3u |%-10s |0x%-8x |%-9u |%-4u |%-4u |%-9u |%-40s|\r\n",
      					 wIndex,
      					 szName,
      					 ptTCB->dwThreadId,
      					 ptTCB->dwCurPno,
						 ptTCB->dwPno4Head,
						 ptTCB->dwPno4Tail,
					     ptTCB->dwProcNum,
      					 strProcList);
				}
				else
				{
				      osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_cmd_show_task_statics, invalid current pno (%u)!\n", ptTCB->dwCurPno);
				}

      	  }
      }
      telnet_print("  +----+-----------+-----------+----------+-----+-----+---------+-----------------------------------------+\r\n");
}



