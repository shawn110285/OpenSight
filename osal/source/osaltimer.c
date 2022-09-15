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

#include "../include/osalapi.h"
#include "../include/osaltimer.h"
#include "../include/osalmsgdef.h"

#include "../../telnet_svr/include/telnetapi.h"


static  sem_ctrl_blk_t *     sgptTvMutex;
static  sem_ctrl_blk_t *     sgptWatchDogSem;


static  uint32_t     sgdwCount10ms = 0;

static  uint32_t     sgdwCount1s = 0;
static  uint32_t     sgdwStartTime1s=0;

static  uint8_t      sgucEnableToneTimer=0;

static volatile  uint32_t     sgdwTimerUsedCnt=0,sgdwGetTimerErr=0,sgdwDelTimerErr=0;
static volatile  uint32_t     sgdwTimerCycleCnt=0,sgdwTimerCommonCnt=0,sgdwTimerABSCnt=0;


static  T_TidNode             sgatTidNode[OSAL_MAX_TMCB_NUM];
static  T_TMCBPoolItem     *  gptTMCBPool;

static  T_TMCB             *  gptTMCB;



static  T_TV        tv[5];

static uint8_t    aucDeltaDays[12] = {0, 3, 3, 6, 8, 11, 13, 16, 19, 21, 24, 26};

extern void   * OsPort_Malloc(uint32_t IdwSize);




uint32_t   OsTimerInit()
{

    sgptWatchDogSem = OsPort_SemBCreate(SEM_EMPTY);
    if(sgptWatchDogSem == NULL)
    {
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerInitCtrlBlk: semBCreate Failed,suspend OSS!\n");
        return OSAL_ERROR;
    }

    if (OSAL_ERROR == OsTimerInitTimeWheel())
        return OSAL_ERROR;

    if (OSAL_ERROR == OsTimerInitCtrlBlk())
        return OSAL_ERROR;

    sgdwCount1s = OsTimerInitCurrentClock();
	sgdwStartTime1s=sgdwCount1s;


    sgdwCount10ms=0;

    if(0 == OsPort_taskSpawn("tTimerTask",(FUNCPTR)OsTimerScanTaskEntry, 99, 10240, 0))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerInit: taskSpawn tTimerTask error,suspend OSS !!!!\n");
        return OSAL_ERROR;
    }

    OsTimerInitWatchDog();

    return OSAL_OK;
}



uint32_t  OsTimerInitTimeWheel(void)
{
	uint32_t                dwIndex;
    uint32_t                dwTvItem;

    sgptTvMutex = OsPort_SemMCreate();
    if(sgptTvMutex == NULL)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerInitTimeWheel:OsPort_SemMCreate Failed!\n");
          return OSAL_ERROR;
    }

	for(dwTvItem=0;dwTvItem<5;dwTvItem++)
	{
           tv[dwTvItem].dwElementCount	  = 0;
           tv[dwTvItem].dwTimeWheelCurPos  = 0;

           for(dwIndex=0;dwIndex<TVR_SIZE;dwIndex++)
           {
                    tv[dwTvItem].adwTimerWheel[dwIndex] = OSAL_INVALID_TCM_INDEX ;
           }
	}
    return OSAL_OK;
}





uint32_t   OsTimerInitCtrlBlk(void)
{
    uint32_t		      dwIndex;
    T_TidNode        *ptTidNode;

    gptTMCBPool = (T_TMCBPoolItem*)OsPort_Malloc(sizeof(T_TMCBPoolItem));
    if(gptTMCBPool ==  NULL)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerInitCtrlBlk: OsPort_Malloc for ptTMCBPool Return NULL,failed!\n");
		return OSAL_ERROR;
    }

    gptTMCBPool->MutexTMCBPool      = OsPort_SemMCreate();
    if(gptTMCBPool->MutexTMCBPool == NULL)
    {
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerInitCtrlBlk: semBCreate Failed!\n");
        return OSAL_ERROR;
    }

    gptTMCBPool->dwFreeCount          = OSAL_MAX_TMCB_NUM;
    gptTMCBPool->dwHead               = 0;
    gptTMCBPool->dwTail               = OSAL_MAX_TMCB_NUM-1;

	ptTidNode = sgatTidNode;
	for ( dwIndex = 0; dwIndex <OSAL_MAX_TMCB_NUM-1 ; dwIndex ++)
	{
		ptTidNode->ucUsed	= OSAL_FALSE;
		ptTidNode->dwNext	= dwIndex + 1;
		ptTidNode ++;
	}
	ptTidNode->ucUsed	= OSAL_FALSE;
	ptTidNode->dwNext	= OSAL_INVALID_TCM_INDEX;

    gptTMCB  = (T_TMCB*)OsPort_Malloc(sizeof(T_TMCB)*OSAL_MAX_TMCB_NUM);
    if(gptTMCB ==  NULL)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerInitCtrlBlk: OsPort_Malloc for ptTMCB  Return NULL,failed!\n");
		return OSAL_ERROR;
    }


    for (dwIndex = 0; dwIndex < OSAL_MAX_TMCB_NUM; dwIndex ++)
    {
        gptTMCB[dwIndex].bIsUse        	     = OSAL_FALSE;
        gptTMCB[dwIndex].bIsTimeOut		     = OSAL_FALSE;
        gptTMCB[dwIndex].dwPno	     	     = OSAL_ERR_INVALID_PNO;
		gptTMCB[dwIndex].dwPrevTid           = OSAL_INVALID_TCM_INDEX;
		gptTMCB[dwIndex].dwNextTid           = OSAL_INVALID_TCM_INDEX;
		gptTMCB[dwIndex].dwTickIndex         = OSAL_INVALID_TCM_INDEX;
		gptTMCB[dwIndex].ucTimeWheelId       = OSAL_INVALID_TV_INDEX;
    }

    return OSAL_OK;
}


uint32_t  OsTimerGetTimerCtrlBlk(void)
{
    uint32_t	         dwTid;
    uint32_t 	         dwHead;
    int32_t   	         rc;
    T_TMCB  	   * ptTMCB;
    T_TMCBPoolItem * ptTMCBPool;

    ptTMCBPool	= gptTMCBPool;

    rc = OsPort_SemTake(ptTMCBPool->MutexTMCBPool,WAIT_FOREVER);
    if (rc == OSAL_FALSE)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerGetTimerCtrlBlk: semTake Failed! \n");
	    return OSAL_INVALID_TCM_INDEX;
    }

    if (ptTMCBPool->dwFreeCount <= 1)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerGetTimerCtrlBlk:No free TMCB \n");
	    OsPort_SemGive(ptTMCBPool->MutexTMCBPool);
        return  OSAL_INVALID_TCM_INDEX;
    }

    dwHead  = ptTMCBPool->dwHead;
    ptTMCBPool->dwHead = sgatTidNode[dwHead].dwNext;

	if (ptTMCBPool->dwHead == OSAL_INVALID_TCM_INDEX )
		ptTMCBPool->dwTail = OSAL_INVALID_TCM_INDEX;

    ptTMCBPool->dwFreeCount --;
    sgdwTimerUsedCnt++;

	sgatTidNode[dwHead].dwNext = OSAL_INVALID_TCM_INDEX;
	sgatTidNode[dwHead].ucUsed = OSAL_TRUE;

    rc = OsPort_SemGive(ptTMCBPool->MutexTMCBPool);

    dwTid           = dwHead;
    ptTMCB		    = gptTMCB + dwTid;

    ptTMCB->bIsUse 	             = OSAL_TRUE;

	ptTMCB->bIsTimeOut 		     = OSAL_FALSE;
	ptTMCB->dwPno				 = OSAL_ERR_INVALID_PNO;
	ptTMCB->dwPrevTid			 = OSAL_INVALID_TCM_INDEX;
	ptTMCB->dwNextTid			 = OSAL_INVALID_TCM_INDEX;
	ptTMCB->dwTickIndex		     = OSAL_INVALID_TCM_INDEX;
	ptTMCB->ucTimeWheelId		 = OSAL_INVALID_TV_INDEX;

    return dwTid;
}


uint32_t  OsTimerFreeTimerCtrlBlk(uint32_t dwTid)
{
    T_TMCB  		    *ptTMCB;
    T_TMCBPoolItem    	*ptTMCBPool;
    int32_t 		         rc;

    if((dwTid >= OSAL_MAX_TMCB_NUM)||(dwTid==OSAL_ERR_INVALID_TIMER_ID))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerFreeTimerCtrlBlk: tid>= OSAL_MAX_TMCB_NUM \n");
        return OSAL_ERROR;
    }


	if(sgatTidNode[dwTid].ucUsed == OSAL_FALSE)
	{
		 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerFreeTimerCtrlBlk,the Tid is not used!\n");
		 return OSAL_ERROR ;
	}


    ptTMCBPool 	= gptTMCBPool;
    ptTMCB	    = gptTMCB + dwTid;


    rc = OsPort_SemTake(ptTMCBPool->MutexTMCBPool,WAIT_FOREVER);
    if (rc == OSAL_FALSE)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerFreeTimerCtrlBlk: semTake Failed! \n");
    	return OSAL_ERROR;
    }

    if(ptTMCB->bIsUse != OSAL_TRUE)
	{
         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerFreeTimerCtrlBlk:TMCB not Used \n");
		 OsPort_SemGive(ptTMCBPool->MutexTMCBPool);
		 return OSAL_ERROR ;
	}

    if (ptTMCBPool->dwFreeCount < OSAL_MAX_TMCB_NUM)
    {

        ptTMCB->bIsUse        	     = OSAL_FALSE;
        ptTMCB->bIsTimeOut		     = OSAL_FALSE;
        ptTMCB->dwPno	     	     = OSAL_ERR_INVALID_PNO;
		ptTMCB->bTimerType           = OSAL_TIMER_TYPE_INVALID;

		ptTMCB->dwPrevTid           = OSAL_INVALID_TCM_INDEX;
		ptTMCB->dwNextTid           = OSAL_INVALID_TCM_INDEX;
		ptTMCB->dwTickIndex         = OSAL_INVALID_TCM_INDEX;
		ptTMCB->ucTimeWheelId       = OSAL_INVALID_TV_INDEX;


		if(OSAL_INVALID_TCM_INDEX == ptTMCBPool->dwHead )
		{
			ptTMCBPool->dwHead  = dwTid;
			ptTMCBPool->dwTail  = dwTid;
		}
		else
		{
		    sgatTidNode[ptTMCBPool->dwTail].dwNext = dwTid;
			ptTMCBPool->dwTail= dwTid;
		}

		sgatTidNode[dwTid].dwNext =OSAL_INVALID_TCM_INDEX;
        sgatTidNode[dwTid].ucUsed = OSAL_FALSE;

		ptTMCBPool->dwFreeCount ++;
		sgdwTimerUsedCnt--;
    }
    else
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerFreeTimerCtrlBlk:ptTMCBPool->dwFreeCount >= OSAL_MAX_TMCB_NUM \n");
    }

    OsPort_SemGive(ptTMCBPool->MutexTMCBPool);

    return OSAL_OK;

}


uint32_t   OsTimerInitCurrentClock()
{
	 osal_clk_t    tABSClock={0};
     uint32_t     dwTime = 0;
     uint16_t     wYear  = 0;
	 uint8_t      bMon   = 0;
	 uint8_t      bDay   = 0;
	 uint8_t      bHour  = 0;
	 uint8_t      bMin   = 0;
	 uint8_t      bSec   = 0;
	 uint8_t      bWeek  = 0;

     OsPort_GetCurrentClock(&wYear,&bMon,&bDay,&bHour,&bMin,&bSec,&bWeek);
     tABSClock.wYear = wYear;
	 tABSClock.bMon  = bMon;
	 tABSClock.bDay  = bDay;
	 tABSClock.bHour = bHour;
	 tABSClock.bMin  = bMin;
	 tABSClock.bSec  = bSec;
	 tABSClock.bWeek = bWeek;

     osal_sys_clock_to_second(&tABSClock,&dwTime);

     return dwTime;

}










int  OsTimer10msProcess()
{
    if(OSAL_FALSE == OsPort_SemGive(sgptWatchDogSem))
         {
        	osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"OsTimer10msProcess，OsPort_SemGive FAILED! \n");
         }

	   return OSAL_OK;
}



uint32_t   OsTimerInitWatchDog()
{
    if(OsPort_wdStart(10, OsTimer10msProcess) == OSAL_FALSE)
    {
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerInitWatchDog，OsPort_wdStart:Failed! \n");
    	return OSAL_ERROR;
    }
 else
    {
        return  OSAL_OK;
    }

}





void   OsTimerScanTaskEntry()
{
   int32_t                  status;

   while(OSAL_TRUE)
   {
       status = OsPort_SemTake(sgptWatchDogSem,WAIT_FOREVER);
       if(OSAL_FALSE == status)
                  {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTaskEntry, semTake sgptWatchDogSem error\n");
            continue;
                  }
        sgdwCount10ms++;

        OsTimerScanTimeWheel();

		if(0 == sgdwCount10ms%100)
		{
		     sgdwCount1s++;
		}
    }   /* OSAL_TRUE */
}







uint32_t  OsTimerScanTimeWheel(void)
{
    T_TV            *   ptTv;

    uint32_t	            dwCursor;
    uint32_t	            dwCurrentTid;
	uint32_t              dwTempTid;
	uint32_t              dwTimerNumOnThisCursor=0;


    ptTv = &tv[0];
	ptTv->dwTimeWheelCurPos	= (ptTv->dwTimeWheelCurPos + 1)%TVR_SIZE;
    dwCursor                 = ptTv->dwTimeWheelCurPos;

    if(OSAL_FALSE == OsPort_SemTake(sgptTvMutex,WAIT_FOREVER) )
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel: semTake failed !\n");
        return OSAL_ERROR;
    }

	dwTimerNumOnThisCursor = 0;

    dwCurrentTid  = ptTv->adwTimerWheel[dwCursor];

	while (dwCurrentTid != OSAL_INVALID_TCM_INDEX)
    {
        if(dwCurrentTid >= OSAL_MAX_TMCB_NUM)
        {
			 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel: invalid tid(%d),CurrTimerNumOnCursor=%d\n",dwCurrentTid,dwTimerNumOnThisCursor);
             OsPort_SemGive(sgptTvMutex);
             return OSAL_ERROR;

        }

        if((gptTMCB[dwCurrentTid].dwTickIndex != dwCursor)||(gptTMCB[dwCurrentTid].ucTimeWheelId!= 0))
        {
			 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel(0,%d),TimerNumOnCursor=%d: Tid (%d) in the wrong timewheel id(%d) or TickIndex(%d) \n",dwCursor,dwCurrentTid,dwTimerNumOnThisCursor,gptTMCB[dwCurrentTid].ucTimeWheelId,gptTMCB[dwCurrentTid].dwTickIndex);
             OsPort_SemGive(sgptTvMutex);
             return OSAL_ERROR;

        }

		dwTimerNumOnThisCursor++;

		dwTempTid  = gptTMCB[dwCurrentTid].dwNextTid;

		OsTimerSendMsgToTask(dwCurrentTid);

        dwCurrentTid = dwTempTid;

    }

    if(tv[0].dwTimeWheelCurPos == 0)
    {
            ptTv = &tv[1];
        	ptTv->dwTimeWheelCurPos	= (ptTv->dwTimeWheelCurPos + 1)%TVN_SIZE;
            dwCursor                 = ptTv->dwTimeWheelCurPos;

			dwTimerNumOnThisCursor = 0;

            dwCurrentTid  = ptTv->adwTimerWheel[dwCursor];
    		while (dwCurrentTid != OSAL_INVALID_TCM_INDEX)
            {

					if(dwCurrentTid >= OSAL_MAX_TMCB_NUM)
					{
						 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel: invalid tid(%d),CurrTimerNumOnCursor=%d\n",dwCurrentTid,dwTimerNumOnThisCursor);
						 OsPort_SemGive(sgptTvMutex);
						 return OSAL_ERROR;

					}


					if((gptTMCB[dwCurrentTid].dwTickIndex != dwCursor)||(gptTMCB[dwCurrentTid].ucTimeWheelId!= 1))
					{
						 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel(1,%d),TimerNumOnCursor=%d: Tid (%d) in the wrong timewheel id(%d) or TickIndex(%d) \n",dwCursor,dwCurrentTid,dwTimerNumOnThisCursor,gptTMCB[dwCurrentTid].ucTimeWheelId,gptTMCB[dwCurrentTid].dwTickIndex);
						 OsPort_SemGive(sgptTvMutex);
						 return OSAL_ERROR;

					}
					dwTimerNumOnThisCursor++;

            		dwTempTid  = gptTMCB[dwCurrentTid].dwNextTid;

            		OsTimerRemoveTimerFromTimeWheel(dwCurrentTid);
    				OsTimerAttachTimerToTimeWheel(dwCurrentTid);

                    dwCurrentTid = dwTempTid;
            }

            if(tv[1].dwTimeWheelCurPos == 0)
            {
                    ptTv = &tv[2];
                	ptTv->dwTimeWheelCurPos	= (uint16_t)((ptTv->dwTimeWheelCurPos + 1)%TVN_SIZE);
                    dwCursor                 = ptTv->dwTimeWheelCurPos;
					dwTimerNumOnThisCursor = 0;
                    dwCurrentTid  = ptTv->adwTimerWheel[dwCursor];

            		while (dwCurrentTid != OSAL_INVALID_TCM_INDEX)
                    {
    						if(dwCurrentTid >= OSAL_MAX_TMCB_NUM)
    						{
    							 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel: invalid tid(%d),CurrTimerNumOnCursor=%d\n",dwCurrentTid,dwTimerNumOnThisCursor);
    							 OsPort_SemGive(sgptTvMutex);
    							 return OSAL_ERROR;

    						}

    						if((gptTMCB[dwCurrentTid].dwTickIndex != dwCursor)||(gptTMCB[dwCurrentTid].ucTimeWheelId!= 2))
    						{
    							 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel(2,%d),TimerNumOnCursor=%d: Tid (%d) in the wrong timewheel id(%d) or TickIndex(%d) \n",dwCursor,dwCurrentTid,dwTimerNumOnThisCursor,gptTMCB[dwCurrentTid].ucTimeWheelId,gptTMCB[dwCurrentTid].dwTickIndex);
    							 OsPort_SemGive(sgptTvMutex);
    							 return OSAL_ERROR;

    						}
    						dwTimerNumOnThisCursor++;

                    		dwTempTid  = gptTMCB[dwCurrentTid].dwNextTid;

                    		OsTimerRemoveTimerFromTimeWheel(dwCurrentTid);
            				OsTimerAttachTimerToTimeWheel(dwCurrentTid);

                            dwCurrentTid = dwTempTid;
                    }

                    if(tv[2].dwTimeWheelCurPos == 0)
                    {
                            ptTv = &tv[3];
                        	ptTv->dwTimeWheelCurPos	= (uint16_t)((ptTv->dwTimeWheelCurPos + 1)%TVN_SIZE);
                            dwCursor                 = ptTv->dwTimeWheelCurPos;
							dwTimerNumOnThisCursor = 0;
                            dwCurrentTid  = ptTv->adwTimerWheel[dwCursor];

                    		while (dwCurrentTid != OSAL_INVALID_TCM_INDEX)
                            {
									if(dwCurrentTid >= OSAL_MAX_TMCB_NUM)
									{
										 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel: invalid tid(%d),CurrTimerNumOnCursor=%d\n",dwCurrentTid,dwTimerNumOnThisCursor);
										 OsPort_SemGive(sgptTvMutex);
										 return OSAL_ERROR;

									}
									if((gptTMCB[dwCurrentTid].dwTickIndex != dwCursor)||(gptTMCB[dwCurrentTid].ucTimeWheelId!= 3))
									{
										 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel(3,%d),TimerNumOnCursor=%d: Tid (%d) in the wrong timewheel id(%d) or TickIndex(%d) \n",dwCursor,dwCurrentTid,dwTimerNumOnThisCursor,gptTMCB[dwCurrentTid].ucTimeWheelId,gptTMCB[dwCurrentTid].dwTickIndex);
										 OsPort_SemGive(sgptTvMutex);
										 return OSAL_ERROR;

									}
									dwTimerNumOnThisCursor++;

                            		dwTempTid  = gptTMCB[dwCurrentTid].dwNextTid;

                            		OsTimerRemoveTimerFromTimeWheel(dwCurrentTid);
                    				OsTimerAttachTimerToTimeWheel(dwCurrentTid);

                                    dwCurrentTid = dwTempTid;
                            }

                             if(tv[3].dwTimeWheelCurPos == 0)
                             {
                                     ptTv = &tv[4];
                                 	 ptTv->dwTimeWheelCurPos	= (uint16_t)((ptTv->dwTimeWheelCurPos + 1)%TVN_SIZE);
                                     dwCursor                 = ptTv->dwTimeWheelCurPos;

									 dwTimerNumOnThisCursor = 0;
                                     dwCurrentTid  = ptTv->adwTimerWheel[dwCursor];

                             		 while (dwCurrentTid != OSAL_INVALID_TCM_INDEX)
                                     {
    										 if(dwCurrentTid >= OSAL_MAX_TMCB_NUM)
    										 {
    											  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel: invalid tid(%d),CurrTimerNumOnCursor=%d\n",dwCurrentTid,dwTimerNumOnThisCursor);
    											  OsPort_SemGive(sgptTvMutex);
    											  return OSAL_ERROR;

    										 }

    										 if((gptTMCB[dwCurrentTid].dwTickIndex != dwCursor)||(gptTMCB[dwCurrentTid].ucTimeWheelId!= 4))
    										 {
    											  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerScanTimeWheel(4,%d),TimerNumOnCursor=%d: Tid (%d) in the wrong timewheel id(%d) or TickIndex(%d) \n",dwCursor,dwCurrentTid,dwTimerNumOnThisCursor,gptTMCB[dwCurrentTid].ucTimeWheelId,gptTMCB[dwCurrentTid].dwTickIndex);
    											  OsPort_SemGive(sgptTvMutex);
    											  return OSAL_ERROR;

    										 }
    										 dwTimerNumOnThisCursor++;

                                     		 dwTempTid  = gptTMCB[dwCurrentTid].dwNextTid;

                                     		 OsTimerRemoveTimerFromTimeWheel(dwCurrentTid);
                             				 OsTimerAttachTimerToTimeWheel(dwCurrentTid);

                                             dwCurrentTid = dwTempTid;
                                     }
                           }
                    }
             }
    }

    OsPort_SemGive(sgptTvMutex);

    return OSAL_OK;
}


uint32_t   OsTimerAttachTimerToTimeWheel(uint32_t dwTid)
{
    T_TMCB 		        * ptTMCB;

    uint32_t		          dwHeadTid;

	uint32_t                idx;
	uint8_t                 ucTvId;
	uint16_t                wTvPos;
	uint16_t                wDistance;

    ptTMCB	        = gptTMCB + dwTid;

    if(ptTMCB->dwExpireTime < sgdwCount10ms)
	{
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerAttachTimerToTimeWheel:error \n");
        return OSAL_ERROR;
	}


    idx = ptTMCB->dwExpireTime - sgdwCount10ms;

    if (idx < TVR_SIZE)
 	{
 	      ucTvId  = 0;
          wTvPos = (uint16_t)((tv[0].dwTimeWheelCurPos + idx)%TVR_SIZE);
    }
	else if (idx < (1 << (TVR_BITS + TVN_BITS)))
	{
 	      ucTvId  = 1;
		  wDistance = (uint16_t)((idx+tv[0].dwTimeWheelCurPos)>> TVR_BITS);
          wTvPos = (uint16_t)((tv[1].dwTimeWheelCurPos +wDistance)%TVN_SIZE);
    }
	else if (idx < (1 << (TVR_BITS + 2 * TVN_BITS)))
    {
 	      ucTvId  = 2;
		  wDistance =(uint16_t)((idx+tv[1].dwTimeWheelCurPos) >> (TVR_BITS + TVN_BITS));
          wTvPos = (uint16_t)((tv[2].dwTimeWheelCurPos+wDistance) & TVN_MASK);
    }
	else if (idx < (1 << (TVR_BITS + 3 * TVN_BITS)))
    {
 	      ucTvId  = 3;
		  wDistance =(uint16_t)((idx+tv[2].dwTimeWheelCurPos) >> (TVR_BITS + 2*TVN_BITS));
          wTvPos = (uint16_t)((tv[3].dwTimeWheelCurPos + wDistance) & TVN_MASK);
    }
	else
	{
 	      ucTvId  = 4;
		  wDistance =(uint16_t)((idx+tv[3].dwTimeWheelCurPos) >> (TVR_BITS + 3*TVN_BITS));
          wTvPos = (uint16_t)((tv[4].dwTimeWheelCurPos + wDistance) & TVN_MASK);
    }

    ptTMCB->ucTimeWheelId = ucTvId;
    ptTMCB->dwTickIndex	  = wTvPos;

	dwHeadTid	       = tv[ucTvId].adwTimerWheel[wTvPos];

    if (dwHeadTid == OSAL_INVALID_TCM_INDEX)
    {
//       	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerAttachTimerToTimeWheel:insert dwTid=%d  in the Tvid=%d,Pos=%d \n",dwTid,ucTvId,wTvPos);
            tv[ucTvId].adwTimerWheel[wTvPos]  = dwTid;
            ptTMCB->dwPrevTid		     = OSAL_INVALID_TCM_INDEX;
            ptTMCB->dwNextTid		     = OSAL_INVALID_TCM_INDEX;
    }
    else
    {
//       	 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerAttachTimerToTimeWheel:dwTid=%d with %d in the Tvid=%d,Pos=%d \n",dwTid,dwHeadTid,ucTvId,wTvPos);
            tv[ucTvId].adwTimerWheel[wTvPos]  = dwTid;
            ptTMCB->dwPrevTid		     = OSAL_INVALID_TCM_INDEX;
            ptTMCB->dwNextTid		     = dwHeadTid;

			if(dwHeadTid>OSAL_MAX_TMCB_NUM)
			{
				 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerAttachTimerToTimeWheel(not Head):dwHeadTid(%d) is wrong \n",dwHeadTid);
			}
			else
			{
                 gptTMCB[dwHeadTid].dwPrevTid = dwTid;
			}
    }

    tv[ucTvId].dwElementCount ++;

    return OSAL_OK;
}


uint32_t 	OsTimerRemoveTimerFromTimeWheel(uint32_t IdwTid)
{
   	 uint32_t              dwTimerWheelPos;
	 uint8_t               ucTvId;

     uint32_t	             dwHeadTid;
	 uint32_t              dwPrevTid;
	 uint32_t              dwNextTid;


     if((IdwTid >= OSAL_MAX_TMCB_NUM)||(IdwTid==OSAL_ERR_INVALID_TIMER_ID))
     {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerRemoveTimerFromTimeWheel,dwTid >= OSAL_MAX_TMCB_NUM\n");
        	return OSAL_ERROR;
     }

	 if(gptTMCB[IdwTid].bIsUse == OSAL_FALSE)
	 {
			  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerRemoveTimerFromTimeWheel:the tid(%d) is not used!\n",IdwTid);
			  return  OSAL_ERROR;
	 }

	 ucTvId		 = gptTMCB[IdwTid].ucTimeWheelId;
	 dwTimerWheelPos = gptTMCB[IdwTid].dwTickIndex;

	 if((ucTvId>4)||(ucTvId==0&&dwTimerWheelPos>TVR_SIZE)||(ucTvId>0&&dwTimerWheelPos>TVN_SIZE))
	 {
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerRemoveTimerFromTimeWheel:ucTvId=%d,wTvPos=%d,sgdwCount10ms=%d\n",ucTvId,dwTimerWheelPos,sgdwCount10ms);
		return  OSAL_ERROR;
	 }

     dwHeadTid  = tv[ucTvId].adwTimerWheel[dwTimerWheelPos];

     if(dwHeadTid == IdwTid)
     {
          dwNextTid = gptTMCB[IdwTid].dwNextTid;
          tv[ucTvId].adwTimerWheel[dwTimerWheelPos]= dwNextTid;

		  if(dwNextTid != OSAL_INVALID_TCM_INDEX)
		  {
               if(dwNextTid >= OSAL_MAX_TMCB_NUM)
               {
				     osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerRemoveTimerFromTimeWheel(Head Tid):dwNextTid(%d) is wrong \n",dwNextTid);
               }
			   else
			   {
		             gptTMCB[dwNextTid].dwPrevTid = OSAL_INVALID_TCM_INDEX;
			   }
		  }

     }
	 else
	 {
	      dwPrevTid = gptTMCB[IdwTid].dwPrevTid;

		  if(dwPrevTid==OSAL_INVALID_TCM_INDEX||dwPrevTid>=OSAL_MAX_TMCB_NUM)
		  {
			  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerRemoveTimerFromTimeWheel(not Head Tid):dwPrevTid(%d) is wrong \n",dwPrevTid);
			  return  OSAL_ERROR;
		  }

		  dwNextTid = gptTMCB[IdwTid].dwNextTid;
		  gptTMCB[dwPrevTid].dwNextTid = dwNextTid;

          if(dwNextTid != OSAL_INVALID_TCM_INDEX)
          {
    			  if(dwNextTid >= OSAL_MAX_TMCB_NUM)
    			  {
    					osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerRemoveTimerFromTimeWheel(not head Tid):dwNextTid(%d) is wrong \n",dwNextTid);
    			  }
    			  else
    			  {
    					gptTMCB[dwNextTid].dwPrevTid = dwPrevTid;
    			  }
          }

	 }

     gptTMCB[IdwTid].ucTimeWheelId    = OSAL_INVALID_TV_INDEX;
     gptTMCB[IdwTid].dwTickIndex	  = OSAL_INVALID_TCM_INDEX;
     gptTMCB[IdwTid].dwPrevTid	      = OSAL_INVALID_TCM_INDEX;
     gptTMCB[IdwTid].dwNextTid	      = OSAL_INVALID_TCM_INDEX;

	 tv[ucTvId].dwElementCount --;

	 return OSAL_OK;

}


uint32_t  OsTimerSendMsgToTask(uint32_t dwTid)
{
    T_TMCB            *  ptTMCB = gptTMCB + dwTid;
    osal_schd_msg_t    *  ptMsg;

    osal_pid_t              tPid;
    uint32_t             dwRcvPno;
    uint32_t             dwMsgId;

   // void              * NullMsg = NULL;

    dwRcvPno  = ptTMCB->dwPno;
    dwMsgId   = ptTMCB->dwMsgId;

    if(ptTMCB->bTimerType == OSAL_TIMER_TYPE_CYCLE)
    {
	      OsTimerRemoveTimerFromTimeWheel(dwTid);

		  /* update expire time*/
	      ptTMCB->dwExpireTime  = ptTMCB->dwCount + sgdwCount10ms ;

          OsTimerAttachTimerToTimeWheel(dwTid);
    }
    else
    {
          OsTimerRemoveTimerFromTimeWheel(dwTid);

          if(ptTMCB->bTimerType == OSAL_TIMER_TYPE_ABSOLUTE)
          {
               sgdwTimerABSCnt --;
          }
		  else
		  {
               sgdwTimerCommonCnt --;
          }
	      OsTimerFreeTimerCtrlBlk(dwTid);
//        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerSendMsgToTask: delete Tid: %d!\n",tid);

    }


    if (dwRcvPno > OSAL_SCH_MAX_PROCESS_NUM)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerSendMsgToTask: Receiver Pno = %d error!\n",dwRcvPno);
        return OSAL_ERROR_PNO;
    }

    if (gtPCB[dwRcvPno].bUsed != OSAL_TRUE)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerSendMsgToTask:Receiver Pno = %d not in used\n",dwRcvPno);
        return OSAL_ERROR;
    }

    if(NULL == gtPCB[dwRcvPno].SyncMsgQid )
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerSendMsgToTask:Receiver Pno = %d can not recv syn_msg \n",dwRcvPno);
        return OSAL_ERROR;
    }

    if(NULL == gtPCB[dwRcvPno].AsynMsgQid)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerSendMsgToTask:Receiver Pno = %d can not recv asyn_msg \n",dwRcvPno);
          return OSAL_ERROR;
    }


    ptMsg = (osal_schd_msg_t *)osal_msg_malloc_buf(sizeof(osal_schd_msg_t)+sizeof(uint32_t));
    if (ptMsg == NULL)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"OsTimerSendMsgToTask:osal_msg_malloc_buf return NULL!\n");
          return OSAL_ERROR_BUF_EMPTY;
    }

    tPid.dwPno                    = dwRcvPno;

    ptMsg->wMsgLen               = sizeof(osal_schd_msg_t)+sizeof(uint32_t);
    ptMsg->bVersion              = OSAL_SCH_MSG_VERSION_NUM;
    ptMsg->bType                 = MSG_ASYN_TIMER;
    ptMsg->tReceiver             = tPid;

    tPid.dwPno                   = 0;

    ptMsg->tSender               = tPid;
    ptMsg->dwMsgId               = dwMsgId;

	ptMsg->dwTid                 = dwTid;

    ptMsg->pvReq                 = ptMsg+1;
    ptMsg->wReqSize              = sizeof(uint32_t);
    ptMsg->wAckSize              = 0;
    ptMsg->pvAck                 = NULL;
    memcpy ((char *)(ptMsg->pvReq),(char *)&(ptTMCB->iTimerParam), sizeof(unsigned long int));

    if (OSAL_FALSE == PostMsgToTask(&gtPCB[dwRcvPno], gtPCB[dwRcvPno].SyncMsgQid, ptMsg, WAIT_FOREVER))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsTimerSendMsgToTask: Post Sync MsgToTask Pno = %d Failed!\n",dwRcvPno);
        osal_msg_free_buf (ptMsg);
        return OSAL_ERROR;
    }
	else
	{
		gtPCB[dwRcvPno].dwCurrTid = dwTid;

	}
    return OSAL_OK;
}


uint32_t  osal_timer_start_normal(uint32_t  ItmMsgId,unsigned long int  IdwParam,uint32_t IdwMilliSeconds )
{

    T_TMCB  	        *  ptTMCB;
    osal_pid_t             tPid;

    uint32_t 	           dwNewTid;
    uint32_t  	           dwPno;

    if ( IdwMilliSeconds <= 0 )
    {
        sgdwGetTimerErr++;
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_start_normal,IdwMilliSeconds <= 0\n");
    	return OSAL_ERR_INVALID_TIMER_ID;
    }

    tPid.dwPno = osal_task_get_self_pno();
    if ( tPid.dwPno == OSAL_ERR_INVALID_PNO)
	{
        sgdwGetTimerErr++;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_start_normal,not a valid Process\n");
        return OSAL_ERR_INVALID_TIMER_ID;
	}

    dwPno = tPid.dwPno;

    dwNewTid = OsTimerGetTimerCtrlBlk();
    if (OSAL_INVALID_TCM_INDEX == dwNewTid)
	{
                 sgdwGetTimerErr++;
    	         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_start_normal, OsTimerGetTimerCtrlBlk() Failed!\n");
                 return OSAL_ERR_INVALID_TIMER_ID;
	}

    ptTMCB 		                = gptTMCB + dwNewTid;
    ptTMCB->iTimerParam	        = IdwParam;
    ptTMCB->bTimerType	        = OSAL_TIMER_TYPE_NORMAL;
    ptTMCB->dwMsgId             = ItmMsgId;
    ptTMCB->dwCount	            = IdwMilliSeconds/10;
	ptTMCB->dwExpireTime        = ptTMCB->dwCount + sgdwCount10ms ;


    ptTMCB->bIsTimeOut  	    = OSAL_FALSE;
    ptTMCB->dwPno		        = dwPno;

	if(OSAL_FALSE == OsPort_SemTake(sgptTvMutex,WAIT_FOREVER) )
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_start_normal: semTake failed !\n");
        return OSAL_ERR_INVALID_TIMER_ID;
	}

    if(OSAL_OK == OsTimerAttachTimerToTimeWheel(dwNewTid))
    {
        sgdwTimerCommonCnt++;
        OsPort_SemGive(sgptTvMutex);
        return dwNewTid;
    }
    else
    {
        OsPort_SemGive(sgptTvMutex);
        sgdwGetTimerErr++;
        OsTimerFreeTimerCtrlBlk(dwNewTid);
        return OSAL_ERR_INVALID_TIMER_ID;
    }

}


uint32_t osal_timer_start_cycle( uint32_t    ItmMsgId,unsigned long int    IdwParam, uint32_t    IdwMilliSeconds )
{
//    osal_sch_pcb_t 	    *ptPCB;
    T_TMCB  	        *ptTMCB;
//    T_TMCBPoolItem      *ptTMCBPool;

    osal_pid_t                tPid;

    uint32_t	             dwNewTid;
    uint32_t  	         dwPno;

//    ptTMCBPool	= gptTMCBPool;

    if ( IdwMilliSeconds <= 0 )
    {
        sgdwGetTimerErr++;
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"osal_start_cycle_timer:IdwMilliSeconds <= 0!\n");
    	return OSAL_ERR_INVALID_TIMER_ID;
    }


    tPid.dwPno = osal_task_get_self_pno();
    if (tPid.dwPno == OSAL_ERR_INVALID_PNO)
    {
        sgdwGetTimerErr++;
		return OSAL_ERR_INVALID_TIMER_ID;
    }
    dwPno = tPid.dwPno;

//    ptPCB =&gtPCB[dwPno];

    dwNewTid = OsTimerGetTimerCtrlBlk();
    if (OSAL_INVALID_TCM_INDEX == dwNewTid)
    {
        sgdwGetTimerErr++;
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"osal_timer_start_cycle: OsTimerGetTimerCtrlBlk() Failed!\n");
        return OSAL_ERR_INVALID_TIMER_ID;
    }

    ptTMCB 		                = gptTMCB + dwNewTid;
    ptTMCB->iTimerParam	        = IdwParam;
    ptTMCB->bTimerType	        = OSAL_TIMER_TYPE_CYCLE;
    ptTMCB->dwMsgId             = ItmMsgId;
    ptTMCB->dwCount	            = IdwMilliSeconds/10;
	ptTMCB->dwExpireTime        = ptTMCB->dwCount + sgdwCount10ms ;

    ptTMCB->bIsTimeOut  	    = OSAL_FALSE;
    ptTMCB->dwPno		        = dwPno;

	if(OSAL_FALSE == OsPort_SemTake(sgptTvMutex,WAIT_FOREVER) )
	{
	       osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_start_cycle: semTake failed !\n");
	       return OSAL_ERR_INVALID_TIMER_ID;
	}

    if(OSAL_OK == OsTimerAttachTimerToTimeWheel(dwNewTid))
    {
		    sgdwTimerCycleCnt++;
            OsPort_SemGive(sgptTvMutex);
            return dwNewTid;
    }
    else
    {
		    OsPort_SemGive(sgptTvMutex);
            sgdwGetTimerErr++;
    	    OsTimerFreeTimerCtrlBlk(dwNewTid);
		    return  OSAL_ERR_INVALID_TIMER_ID;
    }
}


uint32_t   osal_timer_start_absolute(uint32_t  ItmMsgId,unsigned long int  IdwParam,osal_clk_t  *IptABSClock)
{
    T_TMCB  	    *ptTMCB;
//    T_TMCBPoolItem  *ptTMCBPool;
    osal_pid_t            tPid;
    uint32_t           dwPno;


    uint32_t          dwSecondsTo,dwSecondsNow;
    uint32_t	        dwNewTid;

//    ptTMCBPool	= gptTMCBPool;

    if(OSAL_OK != osal_sys_clock_to_second(IptABSClock,&dwSecondsTo))
    {
        sgdwGetTimerErr++;

    	return OSAL_ERR_INVALID_TIMER_ID;
    }
    dwSecondsNow  = osal_sys_get_current_second();

    if (dwSecondsTo <= dwSecondsNow)
    {
        sgdwGetTimerErr++;

    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"osal_start_abs_timer: the time has past!\n");
    	return OSAL_ERR_INVALID_TIMER_ID;
    }

    tPid.dwPno = osal_task_get_self_pno();
    if ( tPid.dwPno == OSAL_ERR_INVALID_PNO)
    {
           sgdwGetTimerErr++;
		   return OSAL_ERR_INVALID_TIMER_ID;
    }

    dwNewTid = OsTimerGetTimerCtrlBlk();
    if (OSAL_INVALID_TCM_INDEX == dwNewTid)
    {
        sgdwGetTimerErr++;
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"osal_timer_start_absolute: OsTimerGetTimerCtrlBlk() Failed!\n");
		return OSAL_ERR_INVALID_TIMER_ID;
    }

    dwPno = tPid.dwPno;

    ptTMCB 		         = gptTMCB + dwNewTid;
    ptTMCB->iTimerParam	 = IdwParam;
    ptTMCB->bTimerType	 = OSAL_TIMER_TYPE_ABSOLUTE;
    ptTMCB->dwMsgId      = ItmMsgId;

    ptTMCB->dwCount	     = 1000*(dwSecondsTo - dwSecondsNow)/10;
	ptTMCB->dwExpireTime = ptTMCB->dwCount + sgdwCount10ms ;

    ptTMCB->bIsTimeOut   = OSAL_FALSE;
    ptTMCB->dwPno	     = dwPno ;

	if(OSAL_FALSE == OsPort_SemTake(sgptTvMutex,WAIT_FOREVER) )
	{
	       osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_start_absolute: semTake failed !\n");
	       return OSAL_ERR_INVALID_TIMER_ID;
	}

    if(OSAL_OK == OsTimerAttachTimerToTimeWheel(dwNewTid))
    {
   	    sgdwTimerABSCnt++;

        OsPort_SemGive(sgptTvMutex);
        return dwNewTid;
    }
    else
    {
        OsPort_SemGive(sgptTvMutex);
        sgdwGetTimerErr++;
   	    OsTimerFreeTimerCtrlBlk(dwNewTid);
		return OSAL_ERR_INVALID_TIMER_ID;
    }
}


void  osal_timer_enable_tone(uint8_t  ucToneFlag)
{

   sgucEnableToneTimer = ucToneFlag;

}


uint32_t  osal_timer_stop_by_tid(uint32_t dwTid)
{
    T_TMCB 		* ptTMCB;
    osal_pid_t         tPid;
	uint32_t        dwReturnValue = OSAL_OK;

    tPid.dwPno = osal_task_get_self_pno();
    if (tPid.dwPno == OSAL_ERR_INVALID_PNO)
    {
            sgdwDelTimerErr++;
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_stop_by_tid,not a valid Process1\n");
        	return OSAL_ERROR;
    }

    if((dwTid >= OSAL_MAX_TMCB_NUM)||(dwTid==OSAL_ERR_INVALID_TIMER_ID))
    {
		    sgdwDelTimerErr++;
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:pno(%d) call osal_timer_stop_by_tid (%d),dwTid >= OSAL_MAX_TMCB_NUM\n",tPid.dwPno,dwTid);
        	return OSAL_ERROR;
    }

    ptTMCB	= gptTMCB + dwTid;
	if(OSAL_FALSE == OsPort_SemTake(sgptTvMutex,WAIT_FOREVER) )
	{
	   osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_stop_by_tid: semTake failed !\n");
	   return OSAL_ERROR;
	}

    if (OSAL_FALSE == ptTMCB->bIsUse)
    {

		OsPort_SemGive(sgptTvMutex);
//		sgdwDelTimerErr++;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[osal]:pno(%d) osal_timer_stop_by_tid (%d),OSAL_TRUE != ptTMCB->bIsUse \n",tPid.dwPno,dwTid);
        return OSAL_ERROR;
    }

    if(tPid.dwPno !=  ptTMCB->dwPno)
    {
		   OsPort_SemGive(sgptTvMutex);
		   sgdwDelTimerErr++;
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_stop_by_tid,tPid.dwPno !=  ptTMCB->dwPno,Failed! tPid.dwPno = %d, ptTMCB->dwPno =%d \n",tPid.dwPno,ptTMCB->dwPno );
           return OSAL_ERROR;
    }


    OsTimerRemoveTimerFromTimeWheel(dwTid);

	switch(ptTMCB->bTimerType)
	{
        case  OSAL_TIMER_TYPE_ABSOLUTE:
            sgdwTimerABSCnt--;
            break;

        case OSAL_TIMER_TYPE_NORMAL:
            sgdwTimerCommonCnt--;
            break;

        case OSAL_TIMER_TYPE_CYCLE:
            sgdwTimerCycleCnt--;
            break;

        default:
        {
            sgdwDelTimerErr++;
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_timer_stop_by_tid: unknown timer type!\n");
            dwReturnValue = OSAL_ERROR;
        }
        break;
	}

    OsTimerFreeTimerCtrlBlk(dwTid);

    OsPort_SemGive(sgptTvMutex);

    return dwReturnValue;
}


uint32_t    osal_timer_get_param_by_tid( uint32_t    IdwTid,unsigned long int   *OpiParam )
{
    T_TMCB *ptTMCB;

    if (IdwTid > OSAL_MAX_TMCB_NUM )
    	return OSAL_ERR_INVALID_TIMER_ID;


    ptTMCB  =  gptTMCB + IdwTid;

    if(ptTMCB->bIsUse == OSAL_FALSE)
    {
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"DOS_GetTimerParam: the TMCB isn't used!\n");
        return  OSAL_ERR_INVALID_TIMER_ID;
    }

    *OpiParam = ptTMCB->iTimerParam;

    return OSAL_OK;
}


uint32_t  osal_sys_get_current_second()
{
    return  sgdwCount1s;
}


uint32_t  osal_sys_get_current_10ms()
{
    return  sgdwCount10ms;
}


uint32_t osal_sys_get_tick()
{
	return OsPort_GetTick();
}


uint32_t    osal_sys_get_current_clock( osal_clk_t  *OptABSClock)
{

    	return osal_sys_second_to_clock(sgdwCount1s,OptABSClock);
}


void   osal_sys_get_real_clock(osal_clk_t  *OptABSClock)
{
   uint16_t     wYear  = 0;
	 uint8_t      bMon   = 0;
	 uint8_t      bDay   = 0;
	 uint8_t      bHour  = 0;
	 uint8_t      bMin   = 0;
	 uint8_t      bSec   = 0;
	 uint8_t      bWeek  = 0;

     OsPort_GetCurrentClock(&wYear,&bMon,&bDay,&bHour,&bMin,&bSec,&bWeek);
     OptABSClock->wYear = wYear;
	 OptABSClock->bMon  = bMon;
	 OptABSClock->bDay  = bDay;
	 OptABSClock->bHour = bHour;
	 OptABSClock->bMin  = bMin;
	 OptABSClock->bSec  = bSec;
	 OptABSClock->bWeek = bWeek;

}


uint32_t    osal_sys_get_timestamp()
{
      osal_clk_t    tABSClock;
      uint32_t dwTime;
      if(OSAL_OK != osal_sys_get_current_clock(&tABSClock))
          return  0;

      if(OSAL_OK != osal_sys_clock_to_timestamp(&tABSClock,&dwTime))
          return  0;

      return dwTime;

}


uint32_t    osal_sys_set_current_clock(osal_clk_t *IptABSClock)
{
   uint32_t        status = OSAL_ERROR;
   uint32_t        dwTime;


   if(OSAL_OK != osal_sys_clock_to_second(IptABSClock,&dwTime))
       return OSAL_ERROR_PARAM_ERR;

#if 0
   if(dwTime <= sgdwCount1s)
   {
      uint32_t        dwTimeSpawn;
      dwTimeSpawn =  sgdwCount1s - dwTime;
      OsTimerAbsTimerIncrease1s( dwTimeSpawn );
   }
   else
   {
      uint32_t        dwTimeSpawn;
      dwTimeSpawn = dwTime - sgdwCount1s;
      OsTimerScanAbsTimerQueue(dwTimeSpawn);
   }
#endif

   sgdwCount1s = dwTime;
   return  status;

}


uint32_t  osal_sys_clock_to_second(osal_clk_t* IptAbsClock,uint32_t* OpdwTime)
{
    uint16_t  wDaysSince2001;
    uint32_t dwSecondsSince2001;

    if(NULL == IptAbsClock ||NULL == OpdwTime )
        return OSAL_ERROR_PARAM_ERR;

    if(   IptAbsClock->bSec >= 60 || IptAbsClock->bMin >=60 ||IptAbsClock->bHour >= 24
       || IptAbsClock->bDay < 1 || IptAbsClock->bDay > 31
       || IptAbsClock->bMon < 1  || IptAbsClock->bMon > 12
       || IptAbsClock->wYear < 2000 || IptAbsClock->wYear > 2130)
    {
        return OSAL_ERROR_PARAM_ERR;
    }

    switch(IptAbsClock->bMon)
    {
        case 4:
        case 6:
        case 9:
        case 11:
            if( IptAbsClock->bDay > 30 )     return OSAL_ERROR_PARAM_ERR;
            break;

        case 2:                                /*每4年有个2月是29天*/
            if((IptAbsClock->wYear%4 == 0) && (IptAbsClock->wYear%100!=0) )
            {
                if(IptAbsClock->bDay > 29 )
                     return OSAL_ERROR_PARAM_ERR;
            }
            else
            {
            	if(IptAbsClock->bDay > 28 )
                      return OSAL_ERROR_PARAM_ERR;
            }

            break;

        default:
            break;
    }


    wDaysSince2001  =   IptAbsClock->bDay -1;
    wDaysSince2001 +=  (IptAbsClock->bMon - 1) * 28;
    wDaysSince2001 +=  aucDeltaDays[IptAbsClock->bMon - 1];

    if((IptAbsClock->bMon > 2) && (0 == (IptAbsClock->wYear%4))&& (IptAbsClock->wYear%100!=0) )
    {
        wDaysSince2001 ++;
    }

    wDaysSince2001 += (IptAbsClock->wYear - 2001) * 365;
    if(IptAbsClock->wYear > 2001)
    {
        if(IptAbsClock->wYear <= 2100)
            wDaysSince2001 += (IptAbsClock->wYear - 2001) / 4;
        else
            wDaysSince2001 += (IptAbsClock->wYear - 2001) / 4 - 1 ;

    }

    dwSecondsSince2001 = wDaysSince2001 * 86400 + IptAbsClock->bHour * 3600 + IptAbsClock->bMin * 60 + IptAbsClock->bSec;

    *OpdwTime = dwSecondsSince2001;

    return OSAL_OK;
}


uint32_t    osal_sys_second_to_clock(uint32_t IdwTime, osal_clk_t* OptABSClock)
{
    uint8_t    DaysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint16_t    DaysPerYear,year,days;
    uint32_t   SecondsSince2001 = IdwTime;
    uint32_t   DaysSince2001,seconds;
    uint8_t    month,week;

    if(NULL == OptABSClock)
        return  OSAL_ERROR_PARAM_ERR;

    DaysSince2001 = SecondsSince2001/86400;
    seconds = SecondsSince2001%86400;
    OptABSClock->bHour =(uint8_t)(seconds/3600);
    seconds = seconds%3600;
    OptABSClock->bMin =(uint8_t)(seconds/60);
    OptABSClock->bSec =(uint8_t)(seconds%60);

    /*compute week*/
    week =(uint8_t) (DaysSince2001+1)%7;
    if(0 == week)
        week = 7;

    /* compute year */
    DaysPerYear = 365;
    year = 2001;
    while(DaysSince2001 >= DaysPerYear)
    {
        DaysSince2001 -= DaysPerYear;
        if ((++ year % 4)||2100 == year)
            DaysPerYear = 365;
        else
            DaysPerYear = 366;
    }

    days =(uint16_t) DaysSince2001;

    /* compute month */
    if(year%4||2100 == year)
        DaysPerMonth[1] = 28;
    else
        DaysPerMonth[1] = 29;

    month = 1;

    while(days >= DaysPerMonth[month-1])
    {
        days -= DaysPerMonth[month-1];
        month++;
    }

    OptABSClock->bWeek = week;
    OptABSClock->bDay = days + 1;
    OptABSClock->bMon  = month;
    OptABSClock->wYear = year;
    return OSAL_OK;
}


uint32_t  osal_sys_clock_to_timestamp(osal_clk_t* IptAbsClock,uint32_t* OpdwTime)
{
    uint16_t  wDaysSince1900;
    uint32_t dwSecondsSince1900;

    if(NULL == IptAbsClock ||NULL == OpdwTime )
        return OSAL_ERROR_PARAM_ERR;

    if(   IptAbsClock->bSec >= 60 || IptAbsClock->bMin >=60 ||IptAbsClock->bHour >= 24
       || IptAbsClock->bDay < 1 || IptAbsClock->bDay > 31
       || IptAbsClock->bMon < 1  || IptAbsClock->bMon > 12
       || IptAbsClock->wYear < 2000 || IptAbsClock->wYear > 2130)
    {
        return OSAL_ERROR_PARAM_ERR;
    }

    switch(IptAbsClock->bMon)
    {
        case 4:
        case 6:
        case 9:
        case 11:
            if( IptAbsClock->bDay > 30 )     return OSAL_ERROR_PARAM_ERR;
            break;

        case 2:
            if((IptAbsClock->wYear%4 == 0) && (IptAbsClock->wYear%100!=0) )
            {
                if(IptAbsClock->bDay > 29 )
                     return OSAL_ERROR_PARAM_ERR;
            }
            else
            {
            	if(IptAbsClock->bDay > 28 )
                      return OSAL_ERROR_PARAM_ERR;
            }

            break;

        default:
            break;
    }

    wDaysSince1900  =   IptAbsClock->bDay -1;
    wDaysSince1900 +=  (IptAbsClock->bMon - 1) * 28;
    wDaysSince1900 +=  aucDeltaDays[IptAbsClock->bMon - 1];

    if((IptAbsClock->bMon > 2) && (0 == (IptAbsClock->wYear%4))&& (IptAbsClock->wYear%100!=0) )
    {
        wDaysSince1900 ++;
    }

    wDaysSince1900 += (IptAbsClock->wYear - 1900) * 365;
    if(IptAbsClock->wYear > 2001)
    {
        wDaysSince1900 += (IptAbsClock->wYear - 1901) / 4;
    }

    dwSecondsSince1900 = wDaysSince1900 * 86400 + IptAbsClock->bHour * 3600 + IptAbsClock->bMin * 60
          + IptAbsClock->bSec;

    *OpdwTime = dwSecondsSince1900;

    return OSAL_OK;
}


uint32_t   osal_sys_get_rand( uint32_t IdwMinValue,uint32_t IdwMaxValue )
{
    uint32_t dwRandDelay;
    uint32_t dwResult;

    if ( IdwMinValue > IdwMaxValue )
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"osal_sys_get_rand:The MinValue must small than MaxValue.\n");
        return  0;
    }

    if (IdwMinValue == IdwMaxValue)
    {
    	return(IdwMinValue);
    }

    dwRandDelay  = (uint32_t)rand();
    dwRandDelay %= IdwMaxValue - IdwMinValue;
    dwResult     =  IdwMinValue + dwRandDelay;
    return (dwResult);
}


void  osal_cmd_show_timer(int  argc, char * argv[])
{
   if(sgucEnableToneTimer == 0)
   {
        telnet_print("  Tone Timer:  Disabled \r\n");
   }
   else
   {
	    telnet_print("  Tone Timer:	Enabled \r\n");
   }

   telnet_print("  +------------+---------+-----------+-------------+----------+------------+--------+--------+------------+\r\n");
   telnet_print("  |Total-Timer |Used_Cnt |Free-Timer |Normal-Timer |Abs-Timer |Cycle-Timer |Get-Err |Del-Err |Head  |Tail |\r\n");
   telnet_print("  +------------+---------+-----------+-------------+----------+------------+--------+--------+------------+\r\n");

   telnet_print("  |%-11u |%-8u |%-10u |%-12u |%-9u |%-11u |%-7u |%-7u |%-5u |%-5u| \r\n",
   	                       OSAL_MAX_TMCB_NUM,
   	                       sgdwTimerUsedCnt,
   	                       gptTMCBPool->dwFreeCount,
   	                       sgdwTimerCommonCnt,
   	                       sgdwTimerABSCnt,
   	                       sgdwTimerCycleCnt,
   	                       sgdwGetTimerErr,
   	                       sgdwDelTimerErr,
	                       gptTMCBPool->dwHead,
	                       gptTMCBPool->dwTail
   	                       );
   telnet_print("  +------------+---------+-----------+-------------+----------+------------+--------+--------+------------+\r\n");

}


void  osal_cmd_show_system_time(int  argc, char * argv[])
{
   osal_clk_t    tSysClk;
   uint32_t     dwTimeSpan;

   osal_sys_second_to_clock(sgdwStartTime1s,&tSysClk);
   telnet_print("  System Start   Time: %d-%d-%d  %d:%d:%d \r\n",tSysClk.wYear,tSysClk.bMon,tSysClk.bDay,tSysClk.bHour,tSysClk.bMin,tSysClk.bSec);

   osal_sys_get_current_clock(&tSysClk);
   telnet_print("  System Current Time: %d-%d-%d  %d:%d:%d \r\n",tSysClk.wYear,tSysClk.bMon,tSysClk.bDay,tSysClk.bHour,tSysClk.bMin,tSysClk.bSec);

   dwTimeSpan = sgdwCount1s-sgdwStartTime1s;
   telnet_print("  System running Time: %d:%d:%d \r\n",dwTimeSpan/3600,(dwTimeSpan%3600)/60,dwTimeSpan%60);
}

