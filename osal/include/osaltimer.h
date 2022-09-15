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

#ifndef __OSAL_TIMER_H__
#define __OSAL_TIMER_H__


#include "osalapi.h"
#include "osalsche.h"
//#include "osaldbg.h"



#define  TVN_BITS     6
#define  TVR_BITS     8
#define  TVN_SIZE     (1 << TVN_BITS)   /* 64  */
#define  TVR_SIZE     (1 << TVR_BITS)   /* 256 */
#define  TVN_MASK     (TVN_SIZE - 1)    /*63 */
#define  TVR_MASK     (TVR_SIZE - 1)    /*255 */


#define  OSAL_MAX_TMCB_NUM              10000
#define  OSAL_INVALID_TCM_INDEX         (uint32_t)0xFFFFFFFF
#define  OSAL_INVALID_TV_INDEX          0xFF

#define OSAL_TIMER_TYPE_INVALID             0
#define OSAL_TIMER_TYPE_NORMAL	        	1
#define OSAL_TIMER_TYPE_ABSOLUTE		    2
#define OSAL_TIMER_TYPE_CYCLE               3


typedef struct tagTMCB
{
    uint8_t            bIsUse;
    uint8_t            bIsTimeOut;
    uint8_t            bTimerType;
	uint8_t            ucTimeWheelId;
    uint32_t	       dwTickIndex;
    uint32_t     	   dwPno;
    uint32_t           dwMsgId;
    uint32_t	       dwCount;
	uint32_t           dwExpireTime;
    unsigned long int  iTimerParam;
    uint32_t	       dwPrevTid;
    uint32_t	       dwNextTid;
}T_TMCB ;


typedef  struct  tagTimeIdNode
{
     uint8_t		   ucUsed;
     uint32_t		   dwNext;
}T_TidNode;


typedef struct tagTMCBPoolItem
{
    sem_ctrl_blk_t *        MutexTMCBPool;
    uint32_t        dwFreeCount;
    uint32_t        dwHead;
    uint32_t        dwTail;
}T_TMCBPoolItem ;


typedef struct tagRelTimerQueue
{
    uint32_t       dwElementCount;
    uint32_t       dwTimeWheelCurPos;
    uint32_t       adwTimerWheel[TVR_SIZE];
}T_TV;


uint32_t   	      OsTimerInit(void);
uint32_t  	      OsTimerInitTimeWheel(void);
uint32_t            OsTimerInitCtrlBlk(void);
uint32_t 		      OsTimerGetTimerCtrlBlk(void);
uint32_t 		      OsTimerFreeTimerCtrlBlk(uint32_t dwTid);

uint32_t            OsTimerInitCurrentClock();

uint32_t            OsTimerInitWatchDog();
int               OsTimer10msProcess();


void 		      OsTimerScanTaskEntry();
uint32_t  	      OsTimerScanTimeWheel(void);


uint32_t         	  OsTimerAttachTimerToTimeWheel(uint32_t IdwTid);
uint32_t  	      OsTimerRemoveTimerFromTimeWheel(uint32_t IdwTid);

uint32_t   	      OsTimerSendMsgToTask(uint32_t IdwTid);



#endif	/* __OSAL_TIMER_H__ */


