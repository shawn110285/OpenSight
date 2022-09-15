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


#include "../include/osalbasic.h"


extern void osal_dbg_print(char  IbmPrnGrade, const char *IszPrnMsg,...);


#define  INVALID_DWORD_VALUE        (uint32_t)0xFFFFFFFF
#define  INVALID_WORD_VALUE         (uint16_t)0xFFFF


static msg_queue_ctrl_blk_t     sgatMsgQueCtrlBlk[MAX_QUEUE_COUNT];
static linux_sem_t         hQueueCtrlSem;

static sem_ctrl_blk_t        sgatSemCtrlBlk[MAX_SEM_COUNT];
static linux_sem_t         hsemCtrlSem;


static           char      sgstrRootDir[256]     = {0};
static           char      sgstrProductName[255] = {0};
static           char      sgstrProductNameInCapital[255] = {0};

extern  int    CreateSem (linux_sem_t * ptOsalSem,unsigned char ucSemType, unsigned int  dwInitValue,unsigned int  dwMaxValue);
extern  int    WaitForSem(linux_sem_t * ptOsalSem, int dwMilliSeconds);
extern  int    ReleaseSem(linux_sem_t * ptOsalSem,unsigned int dwReleaseCount);



int32_t OsPortIsProcExist(uint16_t        wPort)
{
    int    fd;
    struct sockaddr_in addr;
    int    retval = -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > fd)
    {
        perror("[OSAL]:OsPortIsProcExist, socket() failed!");
        return OSAL_FALSE;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(wPort);
    addr.sin_addr.s_addr = INADDR_ANY;

    retval = bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    if (retval >= 0)
    {
    	//success
        // close(fd);
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}


void osal_set_root_dir(char *pcRootDir)
{
    memcpy(sgstrRootDir, pcRootDir, strlen(pcRootDir));
}


char * osal_get_root_dir()
{
    return sgstrRootDir;
}



void osal_set_product_name(char * strProductName)
{
    uint32_t  dwIndex;
    memcpy(sgstrProductName, strProductName, strlen(strProductName));

	for(dwIndex=0;dwIndex<strlen(sgstrProductName);dwIndex++)
	{
         sgstrProductNameInCapital[dwIndex]= isupper(sgstrProductName[dwIndex]) ? (sgstrProductName[dwIndex]): toupper(sgstrProductName[dwIndex]);
	}

}


char   * osal_get_product_name()
{
    return (char *)sgstrProductName;
}




char   * osal_get_product_name_in_capital()
{
    return (char *)sgstrProductNameInCapital;
}







int32_t   OsPortInit(void)
{
    unsigned long  dwLoop = 0;

    for (dwLoop = 0; dwLoop < MAX_QUEUE_COUNT; dwLoop++)
    {
        memset(&sgatMsgQueCtrlBlk[dwLoop], 0, sizeof(msg_queue_ctrl_blk_t));
        sgatMsgQueCtrlBlk[dwLoop].ucUsed = QUEUE_CTL_NO_USED;
    }


    if(OSAL_FALSE == CreateSem(&hQueueCtrlSem,SEM_TYPE_MUTEX,1,1))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPortInit:CreateSemaphore for hQueueCtrlSem Failed! \n");
		return OSAL_FALSE;
    }

    for (dwLoop = 0; dwLoop < MAX_SEM_COUNT; dwLoop++)
    {
        memset(&sgatSemCtrlBlk[dwLoop], 0, sizeof(sem_ctrl_blk_t));
        sgatSemCtrlBlk[dwLoop].ucUsed = SEM_CTL_NO_USED;
    }


    if(OSAL_FALSE == CreateSem(&hsemCtrlSem,SEM_TYPE_MUTEX,1,1))
    {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPortInit:CreateSemaphore for hsemCtrlSem Failed! \n");
		   return OSAL_FALSE;
    }

    return OSAL_TRUE;
}




void * OsPort_Malloc(uint32_t  dwSize)
{
	void *pBuf = (void *) malloc(dwSize);
	return pBuf;
}





int32_t   OsPort_Free(void* pucBuf)
{
	free(pucBuf);
	return OSAL_TRUE;
}



sem_ctrl_blk_t *      OsPort_SemMCreate ()
{

    uint32_t              dwSemCtlLoop   = 0;
    uint32_t              dwSemId        = INVALID_DWORD_VALUE;
    sem_ctrl_blk_t      * ptSemCtrlBlk   = NULL;


    if(OSAL_FALSE == WaitForSem(&hsemCtrlSem,WAIT_FOREVER))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate:WaitForSem FAILED!\n");
        return NULL;
    }

    for (dwSemCtlLoop = 0; dwSemCtlLoop <MAX_SEM_COUNT; dwSemCtlLoop++)
    {
        if (sgatSemCtrlBlk[dwSemCtlLoop].ucUsed == SEM_CTL_NO_USED)
        {
            dwSemId = dwSemCtlLoop;
            break;
        }
    }

    if (MAX_SEM_COUNT == dwSemCtlLoop)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate: No more sem control block is available!\n");

        if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate:ReleaseSem FAILED!\n");
        }

		return NULL;
    }

    if(dwSemId<MAX_SEM_COUNT && dwSemId>=0)
    {
         ptSemCtrlBlk = &sgatSemCtrlBlk[dwSemId];
    }
    else
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate: No more Sem control block is available!\n");

        if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate:ReleaseSem FAILED!\n");
        }

		return NULL;
    }


    if(OSAL_FALSE == CreateSem(&(ptSemCtrlBlk->tSemId),SEM_TYPE_MUTEX,1,1))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate:CreateSem FAILED!\n");

        if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate:ReleaseSem FAILED!\n");
        }

		return NULL;
    }


    ptSemCtrlBlk->ucUsed  = SEM_CTL_USED;


    if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate:ReleaseSem FAILED!\n");
        return NULL;
    }

    return ptSemCtrlBlk;
}



sem_ctrl_blk_t *      OsPort_SemBCreate(uint32_t initialState )
{

    uint32_t             dwSemCtlLoop   = 0;
    uint32_t             dwSemId      = INVALID_DWORD_VALUE;
    sem_ctrl_blk_t     * ptSemCtrlBlk = NULL;

    if((initialState != SEM_FULL) &&(initialState != SEM_EMPTY))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemBCreate:initialState value is wrong! \n");
		return NULL;
    }


    if(OSAL_FALSE == WaitForSem(&hsemCtrlSem,WAIT_FOREVER))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemBCreate:WaitForSem FAILED!\n");
        return NULL;
    }




    for (dwSemCtlLoop = 0; dwSemCtlLoop <MAX_SEM_COUNT; dwSemCtlLoop++)
    {
        if (sgatSemCtrlBlk[dwSemCtlLoop].ucUsed == SEM_CTL_NO_USED)
        {
            dwSemId = dwSemCtlLoop;
            break;
        }
    }

    if (MAX_SEM_COUNT == dwSemCtlLoop)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemBCreate: No more sem control block is available!\n");

        if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemBCreate:ReleaseSem FAILED!\n");
        }

        return NULL;
    }

    if(dwSemId<MAX_SEM_COUNT && dwSemId>=0)
    {
         ptSemCtrlBlk = &sgatSemCtrlBlk[dwSemId];
    }
	else
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemBCreate: No more Sem control block is available!\n");

        if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemBCreate:ReleaseSem FAILED!\n");
        }

        return NULL;
    }

    if(OSAL_FALSE == CreateSem(&(ptSemCtrlBlk->tSemId),SEM_TYPE_BINARY,initialState,1))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemBCreate:CreateSem FAILED!\n");

        if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemBCreate:ReleaseSem FAILED!\n");
        }

		return NULL;
    }


     ptSemCtrlBlk->ucUsed  = SEM_CTL_USED;


    if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate:ReleaseSem FAILED!\n");
        return NULL;
    }


	 return ptSemCtrlBlk;
}


sem_ctrl_blk_t *      OsPort_SemCCreate(uint32_t initialState,uint32_t dwMaxValue)
{
    uint32_t          dwSemCtlLoop   = 0;
    uint32_t          dwSemId      = INVALID_DWORD_VALUE;
    sem_ctrl_blk_t  * ptSemCtrlBlk = NULL;


    if((initialState != SEM_FULL) &&(initialState != SEM_EMPTY))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate:initialState value is wrong! \n");
		return NULL;
    }


    if(OSAL_FALSE == WaitForSem(&hsemCtrlSem,WAIT_FOREVER))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate:WaitForSem FAILED!\n");
        return NULL;
    }


    for (dwSemCtlLoop = 0; dwSemCtlLoop <MAX_SEM_COUNT; dwSemCtlLoop++)
    {
        if (sgatSemCtrlBlk[dwSemCtlLoop].ucUsed == SEM_CTL_NO_USED)
        {
            dwSemId = dwSemCtlLoop;
            break;
        }
    }

    if (MAX_SEM_COUNT == dwSemCtlLoop)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate: No more sem control block is available!\n");

        if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate:ReleaseSem FAILED!\n");
        }

        return NULL;
    }

    if(dwSemId<MAX_SEM_COUNT&&dwSemId>=0)
    {
         ptSemCtrlBlk = &sgatSemCtrlBlk[dwSemId];
    }
	else
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate: No more Sem control block is available!\n");

        if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate:ReleaseSem FAILED!\n");
        }


        return NULL;
    }

    if(initialState == SEM_EMPTY)
    {

          if(OSAL_FALSE == CreateSem(&(ptSemCtrlBlk->tSemId),SEM_TYPE_COUNT,initialState,dwMaxValue))
          {
              osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate:CreateSem FAILED!\n");

              if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
              {
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate:ReleaseSem FAILED!\n");
              }

			  return NULL;
          }


    }
	if(initialState == SEM_FULL)
	{
          if(OSAL_FALSE == CreateSem(&(ptSemCtrlBlk->tSemId),SEM_TYPE_COUNT,dwMaxValue,dwMaxValue))
          {
              osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate:CreateSem FAILED!\n");

              if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
              {
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemCCreate:ReleaseSem FAILED!\n");
              }

			  return NULL;
          }

	}
    ptSemCtrlBlk->ucUsed  = SEM_CTL_USED;

    if(OSAL_FALSE == ReleaseSem(&hsemCtrlSem,1))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_SemMCreate:ReleaseSem FAILED!\n");
        return NULL;
    }

    return ptSemCtrlBlk;
}








int32_t   OsPort_SemTake(sem_ctrl_blk_t * ptSemCtrlBlk,int timeout)
{
  if(ptSemCtrlBlk->ucUsed != SEM_CTL_USED)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_SemTake:invalid SemID\n");
		return OSAL_FALSE;
    }

    if(OSAL_FALSE == WaitForSem(&(ptSemCtrlBlk->tSemId),timeout))
    {
         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_SemTake:WaitForSem Failed!\n");
	     return OSAL_FALSE;
    }

	return OSAL_TRUE;
}











int32_t   OsPort_SemGive(sem_ctrl_blk_t * ptSemCtrlBlk)
{
    if(ptSemCtrlBlk->ucUsed != SEM_CTL_USED)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_SemGive:invalid SemID\n");
		     return OSAL_FALSE;
    }

	if(OSAL_FALSE == ReleaseSem(&(ptSemCtrlBlk->tSemId),1))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_SemGive:ReleaseSem Failed!\n");
		     return OSAL_FALSE;
    }

    return OSAL_TRUE;
}




static int32_t OsPort_CreateDoubleCycleQueue(uint32_t  dwEntryCount,uint32_t  dwPayloadSize,double_link_element_t **ptEntryHead,double_link_element_t **ptEntryTail)
{
    double_link_element_t      * ptEntryTmpHead;
    double_link_element_t      * ptEntryTmpTail;
    double_link_element_t      * ptEntryTmpElm;
    uint32_t                     dwQueEntryLoop;

    ptEntryTmpHead = (double_link_element_t*)OsPort_Malloc(dwEntryCount * sizeof(double_link_element_t));
    if (NULL == ptEntryTmpHead)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_CreateDoubleCycleQueue: Fail to allocate memory for head of queue!\n");
        return OSAL_FALSE;
    }
    memset((void*)ptEntryTmpHead, 0, dwEntryCount * sizeof(double_link_element_t));

    ptEntryTmpHead->pucPayLoad = OsPort_Malloc(dwEntryCount * dwPayloadSize);
    if (NULL == ptEntryTmpHead->pucPayLoad)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_CreateDoubleCycleQueue: Fail to allocate memory for entry of queue!\n");
        return OSAL_FALSE;
    }
    memset((void*)ptEntryTmpHead->pucPayLoad,0,dwEntryCount * dwPayloadSize);

    ptEntryTmpTail = ptEntryTmpHead + dwEntryCount - 1;
	ptEntryTmpHead->ptPreElement = ptEntryTmpTail;
    ptEntryTmpTail->ptNextElement = ptEntryTmpHead;
    ptEntryTmpTail->dwPayloadLength = 0;  // dwPayloadSize;
    ptEntryTmpTail->pucPayLoad = ptEntryTmpHead->pucPayLoad + (dwEntryCount - 1) * dwPayloadSize;



	ptEntryTmpElm = ptEntryTmpHead;
    for (dwQueEntryLoop = 0;dwQueEntryLoop < dwEntryCount - 1; dwQueEntryLoop++)
    {
        ptEntryTmpElm->ptNextElement = ptEntryTmpElm + 1;
        ptEntryTmpElm->dwPayloadLength = 0;  // dwPayloadSize;
        (ptEntryTmpElm + 1)->ptPreElement = ptEntryTmpElm;
        ptEntryTmpElm->pucPayLoad = ptEntryTmpHead->pucPayLoad + dwQueEntryLoop * dwPayloadSize;
        ptEntryTmpElm = ptEntryTmpElm + 1;
    }
    *ptEntryHead = ptEntryTmpHead;
    *ptEntryTail = ptEntryTmpTail;
    return OSAL_TRUE;
}




msg_queue_ctrl_blk_t *  OsPort_msgQCreate(uint32_t maxMsgs,uint32_t maxMsgLength)
{
    uint32_t             dwQueCtlLoop   = 0;
    uint32_t             dwQueueId      = INVALID_DWORD_VALUE;
    msg_queue_ctrl_blk_t  * ptMsgQueCtrlBlk= NULL;


    if (0 == maxMsgs)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQCreate: Lenth of queue is invalid!\n");
        return NULL;
    }


    if(OSAL_FALSE == WaitForSem(&hQueueCtrlSem,WAIT_FOREVER))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQCreate: WaitForSem Failed!!\n");
        return NULL;
    }

    for (dwQueCtlLoop = 0; dwQueCtlLoop <MAX_QUEUE_COUNT; dwQueCtlLoop++)
    {
        if (sgatMsgQueCtrlBlk[dwQueCtlLoop].ucUsed == QUEUE_CTL_NO_USED)
        {
            dwQueueId = dwQueCtlLoop;
            break;
        }
    }

    if (INVALID_DWORD_VALUE == dwQueueId)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQCreate: No more queue control block is available!\n");

 	    if(OSAL_FALSE == ReleaseSem(&hQueueCtrlSem,1))
 	    {
             osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQCreate: ReleaseSem(1) Failed!\n");
	    }

		return NULL;
    }

    if(dwQueCtlLoop<MAX_QUEUE_COUNT && dwQueCtlLoop>=0)
    {
         ptMsgQueCtrlBlk = &sgatMsgQueCtrlBlk[dwQueCtlLoop];
    }
    else
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQCreate: No more queue control block is available!\n");

 	    if(OSAL_FALSE == ReleaseSem(&hQueueCtrlSem,1))
 	    {
             osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQCreate: ReleaseSem(2) Failed!\n");
	    }

		return NULL;
    }


    ptMsgQueCtrlBlk->dwPayloadSize = maxMsgLength;

    ptMsgQueCtrlBlk->dwTotalCount = maxMsgs;
    ptMsgQueCtrlBlk->dwUsedCount = 0;


    if (OSAL_FALSE == OsPort_CreateDoubleCycleQueue(maxMsgs,maxMsgLength,&(ptMsgQueCtrlBlk->ptHead),&(ptMsgQueCtrlBlk->ptTail)))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQCreate: Fail to make double and cycle queue!\n");

 	    if(OSAL_FALSE == ReleaseSem(&hQueueCtrlSem,1))
 	    {
             osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQCreate: ReleaseSem(3) Failed!\n");
	    }

		return NULL;
    }

    ptMsgQueCtrlBlk->ptHead = ptMsgQueCtrlBlk->ptTail;

    if ((ptMsgQueCtrlBlk->ptCountSem = OsPort_SemCCreate(SEM_EMPTY,maxMsgs))==NULL)
    {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_CreateQueue: Create count semaphore failure!\n");

 	        if(OSAL_FALSE == ReleaseSem(&hQueueCtrlSem,1))
 	        {
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQCreate: ReleaseSem(4) Failed!\n");
	        }

            return NULL;
    }

    if ((ptMsgQueCtrlBlk->ptMuxSem = OsPort_SemMCreate())==NULL)
    {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQCreate: Create mutex semaphore failure!\n");

 	        if(OSAL_FALSE == ReleaseSem(&hQueueCtrlSem,1))
 	        {
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQCreate: ReleaseSem(5) Failed!\n");
	        }

            return NULL;
    }
    ptMsgQueCtrlBlk->ucUsed = QUEUE_CTL_USED;


	if(OSAL_FALSE == ReleaseSem(&hQueueCtrlSem,1))
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQCreate: ReleaseSem Failed!!\n");
        return NULL;
	}

    return ptMsgQueCtrlBlk;
}









int32_t   OsPort_msgQSend(msg_queue_ctrl_blk_t *  ptMsgQueCtrlBlk,uint8_t * buffer,uint32_t  nBytes,int  timeout)
{
    msg_queue_ctrl_blk_t            * ptQueueCtl;
    double_link_element_t        * ptQueueTailElm;

    ptQueueCtl = ptMsgQueCtrlBlk;

    if(ptQueueCtl->ucUsed != QUEUE_CTL_USED)
    {
         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQSend: the Queue is not used!\n");
		 return OSAL_FALSE;
    }

	if (ptQueueCtl->dwPayloadSize < nBytes)
    {
         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQSend: Size of sended element is too large!\n");
         return OSAL_FALSE;
    }

    if (OSAL_FALSE == OsPort_SemTake(ptQueueCtl->ptMuxSem, timeout))
    {
             osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQSend: OsPort_SemTake(ptQueueCtl->ptMuxSem, timeout) failure!\n");
             return OSAL_FALSE;
    }

    if (ptQueueCtl->dwTotalCount == ptQueueCtl->dwUsedCount)
    {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]: OsPort_msgQSend: the Queue  is full!\n");
		    if(OSAL_FALSE == OsPort_SemGive(ptQueueCtl->ptMuxSem))
		    {
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQSend: OsPort_SemGive(ptQueueCtl->ptMuxSem) failure!\n");
		    }
			return OSAL_FALSE;
    }

    ptQueueTailElm = ptQueueCtl->ptTail;
    memcpy(ptQueueTailElm->pucPayLoad,buffer,nBytes);
    ptQueueTailElm->dwPayloadLength = nBytes;

    ptQueueCtl->ptTail = ptQueueTailElm->ptNextElement;
    ptQueueCtl->dwUsedCount += 1;

	if((ptQueueCtl->dwUsedCount <= 0) || (ptQueueCtl->dwUsedCount > ptQueueCtl->dwTotalCount))
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQSend: Msg Used Count is Wrong!\n");
	}

	if (OSAL_FALSE == OsPort_SemGive(ptQueueCtl->ptCountSem ))
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQSend: OsPort_SemGive(ptQueueCtl->ptCountSem ) failure!\n");
	}

    if(OSAL_FALSE == OsPort_SemGive(ptQueueCtl->ptMuxSem))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQSend: OsPort_SemGive(ptQueueCtl->ptMuxSem) failure!\n");
    }

    return OSAL_TRUE;
}







uint32_t  OsPort_msgQReceive(msg_queue_ctrl_blk_t *  ptMsgQueCtrlBlk,uint8_t *   buffer,uint32_t   maxNBytes,int  timeout)
{
    msg_queue_ctrl_blk_t        * ptQueueCtl;
    double_link_element_t    * ptHeadEntryCtl;
    unsigned long            dwRevLen;

    ptQueueCtl = ptMsgQueCtrlBlk;

    if(ptQueueCtl->ucUsed != QUEUE_CTL_USED)
    {
         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_msgQReceive: the Queue is not used!\n");
 		 return 0;
    }

    if (OSAL_FALSE == OsPort_SemTake(ptQueueCtl->ptCountSem , timeout))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQReceive: OsPort_SemTake(ptQueueCtl->ptCountSem , timeout) failure!\n");
        return 0;
    }


	if (OSAL_FALSE == OsPort_SemTake(ptQueueCtl->ptMuxSem, WAIT_FOREVER))
    {
            OsPort_SemGive(ptQueueCtl->ptCountSem );
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQReceive:  OsPort_SemTake(ptQueueCtl->ptMuxSem, (unsigned long )WAIT_FOREVER) failure!\n");
            return 0;
    }

    if(ptQueueCtl->dwUsedCount < 1)
    {
            /* 此处就不   OsPort_SemGive(ptQueueCtl->ptCountSem ); */
            OsPort_SemGive(ptQueueCtl->ptMuxSem);
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQReceive: ptQueueCtl->dwUsedCount < 1 !\n");
            return 0;
    }

    ptHeadEntryCtl = ptQueueCtl->ptHead;
    if(maxNBytes < ptHeadEntryCtl->dwPayloadLength)
    {
            OsPort_SemGive(ptQueueCtl->ptCountSem );
            OsPort_SemGive(ptQueueCtl->ptMuxSem);
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQReceive: maxNBytes < ptHeadEntryCtl->dwPayloadLength!\n");
            return 0;
    }

    memcpy(buffer,ptHeadEntryCtl->pucPayLoad, ptHeadEntryCtl->dwPayloadLength);
    dwRevLen = ptHeadEntryCtl->dwPayloadLength;


    memset(ptHeadEntryCtl->pucPayLoad, 0, ptQueueCtl->dwPayloadSize);
	ptHeadEntryCtl->dwPayloadLength = 0;


    ptQueueCtl->ptHead = ptHeadEntryCtl->ptNextElement;
    ptQueueCtl->dwUsedCount -= 1;

	if((ptQueueCtl->dwUsedCount < 0) || (ptQueueCtl->dwUsedCount > ptQueueCtl->dwTotalCount))
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQReceive: Msg Used Count is Wrong!\n");
	}

    if(OSAL_FALSE == OsPort_SemGive(ptQueueCtl->ptMuxSem))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsPort_msgQReceive: OsPort_SemGive(ptQueueCtl->ptCountSem ) failure!\n");
    }

    return dwRevLen;
}


uint32_t  OsPort_msgQNumMsgs(msg_queue_ctrl_blk_t *  ptMsgQueCtrlBlk)
{
     if(ptMsgQueCtrlBlk->ucUsed == QUEUE_CTL_USED)
          return  ptMsgQueCtrlBlk->dwUsedCount;
	 else
	 	  return  0;
}




int32_t  OsPort_msgQGetStatus(msg_queue_ctrl_blk_t *  ptMsgQueCtrlBlk,uint32_t * pdwTotalCount, uint32_t * pdwUsedCount,uint32_t  * pdwPayloadSize)
{
     if(ptMsgQueCtrlBlk->ucUsed == QUEUE_CTL_USED)
     {
         *pdwTotalCount = ptMsgQueCtrlBlk->dwTotalCount;
		 *pdwUsedCount  = ptMsgQueCtrlBlk->dwUsedCount;
         *pdwPayloadSize= ptMsgQueCtrlBlk->dwPayloadSize;

          return  OSAL_TRUE;
     }
	 else
	 	  return  OSAL_FALSE;


}






