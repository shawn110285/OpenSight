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

#include "../include/osalbuf.h"


/************************************************/

#define OSAL_BUF_GRANULARITY     64
#define OSAL_MAX_MEM_INDEX       512

#define OSAL_BUF_MGMT_DEBUG      1

#define PRINT_WIDTH             (2 * sizeof(unsigned long int))


osal_msg_buf_tbl_t gtOsMsgBufTbl[] =
{
    {  1,  1000    },        /*    64byte *1000        */
    {  2,  1000    },        /*   128*1000         */
    {  4,  1000    },        /*    256         */
    {  8,  1000    },        /*    512         */
    { 16,  1000    },        /*    1024         */
    { 32,  1000    },        /*   2048         */
    { 64,  500     },        /*   4096         */
    {128,  200     },        /*    8192         */
    {256,  50      },        /*  16384         */
    {512,  50      },        /*  32768         */
};


uint32_t   gdwOsMsgBufPoolNum = sizeof(gtOsMsgBufTbl)/sizeof(gtOsMsgBufTbl[0]);
uint32_t   gdwMapSizeToPool[OSAL_MAX_MEM_INDEX];


osal_msg_buf_pool_t * gptOsMsgBufPool;


uint32_t   OsInitMsgBuf(void)
{
    uint32_t        j=0;
	uint8_t         i;

    osal_msg_buf_head_t *  ptOsMsgBuf;

    gptOsMsgBufPool = (osal_msg_buf_pool_t *)OsPort_Malloc(sizeof(osal_msg_buf_pool_t)*gdwOsMsgBufPoolNum);
    if(gptOsMsgBufPool == NULL)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsInitMsgBuf:OsPort_Malloc for gptOsMsgBufPool return NULL!\n");
        return OSAL_ERROR;
    }
    memset ((char *)gptOsMsgBufPool,0,sizeof(osal_msg_buf_pool_t)*gdwOsMsgBufPoolNum);


    for (i=0;i<gdwOsMsgBufPoolNum; i++)
    {
        osal_dbg_assert ( gtOsMsgBufTbl[i].dwBufNum <= OSAL_BUF_MAX_NUM_PER_SIZE);

        gptOsMsgBufPool[i].dwSize  = gtOsMsgBufTbl[i].wBufSize * OSAL_BUF_GRANULARITY;
        gptOsMsgBufPool[i].dwBufNum = gtOsMsgBufTbl[i].dwBufNum;

        gptOsMsgBufPool[i].SemWait = OsPort_SemMCreate();
        if(gptOsMsgBufPool[i].SemWait==NULL)
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsInitMsgBuf:OsPort_SemMCreate Failed !\n");
            return OSAL_ERROR;
        }

        gptOsMsgBufPool[i].dwCount = gtOsMsgBufTbl[i].dwBufNum;
        gptOsMsgBufPool[i].dwHead  = 0;
        gptOsMsgBufPool[i].dwTail  = 0;


        for (j=0; j < gptOsMsgBufPool[i].dwBufNum; j++)
        {
            ptOsMsgBuf = (osal_msg_buf_head_t *)OsPort_Malloc(gptOsMsgBufPool[i].dwSize + sizeof(osal_msg_buf_head_t));
            ptOsMsgBuf->iSign      = (unsigned long int)ptOsMsgBuf;
            ptOsMsgBuf->ucUsedFlag  =  BUF_NOT_MALLOC;
            ptOsMsgBuf->ucBufPoolId =  i;
            gptOsMsgBufPool[i].aptMsgBuf[j] = ptOsMsgBuf;
        }
    }

    j=0;
    for(i=0;i<gdwOsMsgBufPoolNum; i++)
    {
    	for(; j < gtOsMsgBufTbl[i].wBufSize; j++)
        {
            gdwMapSizeToPool[j] = i;
        }
    }

	return OSAL_OK;
}



void * osal_msg_malloc_buf (uint32_t IdwBufSize)
{

    osal_msg_buf_head_t  *buf;
    osal_msg_buf_pool_t  *pPool;
    uint32_t              index;

    osal_dbg_assert (IdwBufSize > 0);

    index = (IdwBufSize-1) / OSAL_BUF_GRANULARITY;
    if (index > OSAL_MAX_MEM_INDEX )
    {
       osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_malloc_buf: Get too large buffer, size=%d\n",IdwBufSize );
	   return NULL;
    }

    pPool = &gptOsMsgBufPool[gdwMapSizeToPool[index]];

    if (OSAL_FALSE == OsPort_SemTake(pPool->SemWait, WAIT_FOREVER))
          return NULL;

    if(pPool->dwCount <= 0)
    {
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[osal]:osal_msg_malloc_buf: buffer is empty %d \n",pPool->dwSize);
        OsPort_SemGive(pPool->SemWait);
		return NULL;
    }
    -- pPool->dwCount;

    buf = pPool->aptMsgBuf[pPool->dwHead];

    pPool->aptMsgBuf[pPool->dwHead] = 0;

    ++pPool->dwHead;
    if (pPool->dwHead >= pPool->dwBufNum)
		pPool->dwHead = 0;

    OsPort_SemGive(pPool->SemWait);

    osal_dbg_assert (buf && (buf->iSign == (unsigned long int)buf));
    osal_dbg_assert (buf->ucUsedFlag == BUF_NOT_MALLOC);

    buf->ucUsedFlag = BUF_MALLOCED;
	buf->wReserved	= 2;

    return ++buf;
}



uint32_t  osal_msg_free_buf (void *IpBuf)
{

    osal_msg_buf_head_t * buf;
    osal_msg_buf_pool_t * pPool;
    uint32_t              index;

    if (IpBuf == NULL)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_free_buf: the Param = NULL \n");
        return OSAL_ERROR;
    }

    buf = (osal_msg_buf_head_t *)((uint8_t *)IpBuf - sizeof(osal_msg_buf_head_t));
    if (buf->iSign != (unsigned long int)buf)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[osal]:osal_msg_free_buf: Free error buffer 0x%x \n",buf);
		return OSAL_ERROR;
    }

    if (buf->ucUsedFlag!=BUF_MALLOCED)
    {
		uint32_t dwMsgId;
        uint32_t dwSelfPno = osal_task_get_self_pno();
        if(OSAL_ERR_INVALID_PNO == dwSelfPno)
        {
            dwSelfPno=0;
            dwMsgId  =0;
        }
		else
		{
			dwMsgId   = osal_msg_get_id();
		}
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[osal]:osal_msg_free_buf:free the unused buffer(0x%x),Pno=%d,Msg=%d,Reserver=%d!\n",buf,dwSelfPno,dwMsgId,buf->wReserved);
        return OSAL_ERROR;
    }

    index = buf->ucBufPoolId;
    if (index > OSAL_MAX_MEM_INDEX)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[osal]:osal_msg_free_buf: buffer PoolIndex=%d ERROR! \n",buf->ucBufPoolId);
        return OSAL_ERROR;
    }

    pPool = gptOsMsgBufPool+index;

    if (OSAL_FALSE == OsPort_SemTake(pPool->SemWait, WAIT_FOREVER))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[osal]:osal_msg_free_buf: OsPort_SemTake(pPool->SemWait, WAIT_FOREVER) Failed! \n");
        return OSAL_ERROR;
    }

    buf->ucUsedFlag = BUF_NOT_MALLOC;
	buf->wReserved  = 1;

    ++ pPool->dwCount;

    if (pPool->dwCount > pPool->dwBufNum)
    {
        pPool->dwCount--;
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[osal]:osal_msg_free_buf: return buffer %d to the Pool,but the pool's count is wrong!\n", pPool->dwSize);
        OsPort_SemGive(pPool->SemWait);
        return OSAL_ERROR;
    }

    pPool->aptMsgBuf[pPool->dwTail] = buf;
    if (++pPool->dwTail >= pPool->dwBufNum)
        pPool->dwTail = 0;

    if (OSAL_ERROR == OsPort_SemGive(pPool->SemWait))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[osal]:osal_msg_free_buf: OsPort_SemGive error\n");
        return OSAL_ERROR;
    }

    return OSAL_OK;
}


uint32_t  osal_msg_get_buf_count (uint32_t IdwBufSize,uint32_t * pdwTotalNum,uint32_t *pdwFreeNum)
{
    uint32_t               index;
    osal_msg_buf_pool_t  * pPool;

    osal_dbg_assert (IdwBufSize > 0);

    index = (IdwBufSize-1) / OSAL_BUF_GRANULARITY;
    if (index > OSAL_MAX_MEM_INDEX )
    {
       osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_msg_get_freebuf_count: Get too large buffer, size=%d\n",IdwBufSize );
	   return OSAL_ERROR;
    }

    pPool = &gptOsMsgBufPool[gdwMapSizeToPool[index]];

   *pdwTotalNum = pPool->dwBufNum;
   *pdwFreeNum = pPool->dwCount;

    return OSAL_OK;
}


void * osal_mem_alloc(uint32_t dwSize)
{
	return OsPort_Malloc(dwSize);
}


uint32_t  osal_mem_free(void* pucBuf)
{
	OsPort_Free(pucBuf);
	return OSAL_OK;
}

