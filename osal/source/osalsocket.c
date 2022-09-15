
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
#include "../include/osalmsgdef.h"
#include "../include/osalsocket.h"



extern void  telnet_print(const char *IszPrnMsg,...);



static T_IPRT_SOCKET_NODE_POOL    sgtIprtSocketNodePool;
static T_IPRT_SOCKET_NODE       * sgptIprtSocketNodeBaseAddr = NULL;


static  int32_t   iprtInitSocketNode()
{
	uint32_t               i;
    T_IPRT_SOCKET_NODE   * ptIprtSocketNode=NULL;

	sgptIprtSocketNodeBaseAddr =(T_IPRT_SOCKET_NODE *)malloc(sizeof(T_IPRT_SOCKET_NODE)*IPRT_MAX_SOCKET_NODE_NUM);
	if(NULL == sgptIprtSocketNodeBaseAddr)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtInitSocketNode,malloc Failed!\n");
		return OSAL_FALSE;
	}

	/* init all the node and the pool */
	for(i=0;i<IPRT_MAX_SOCKET_NODE_NUM;i++)
	{
	    ptIprtSocketNode                 = sgptIprtSocketNodeBaseAddr+i;
		ptIprtSocketNode->ucIsUse        = IPRT_SOCKET_NODE_UNUSED;
		ptIprtSocketNode->dwSelfId       = i;
		ptIprtSocketNode->dwListenId     = OSAL_INVALID_UINT32;

        ptIprtSocketNode->ucType              = IPRT_SOCKET_TYPE_UNKNOW;
        ptIprtSocketNode->ucState             = IPRT_SOCKET_STATE_BLANK;
        ptIprtSocketNode->iSockFd             = INVALID_SOCKET;

        ptIprtSocketNode->tOwnerPid.dwPno       = OSAL_INVALID_UINT32;

        ptIprtSocketNode->dwLocalIp           = 0;
        ptIprtSocketNode->wLocalPort          = 0;

        ptIprtSocketNode->dwRemoteIp          = 0;
        ptIprtSocketNode->wRemotePort         = 0;


		if(i<IPRT_MAX_SOCKET_NODE_NUM-1)
		{
		    ptIprtSocketNode->dwNextInPool = i+1;
		}
		else
		{
		    ptIprtSocketNode->dwNextInPool = OSAL_INVALID_UINT32;
		}
	}


    if((sgtIprtSocketNodePool.tMutex  = OsPort_SemMCreate()) ==NULL )
    {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:init socket node,OsPort_SemMCreate failure!\n");
           return OSAL_FALSE;
    }

	sgtIprtSocketNodePool.dwCount = IPRT_MAX_SOCKET_NODE_NUM;
	sgtIprtSocketNodePool.dwHead  = 0;
	sgtIprtSocketNodePool.dwTail  = IPRT_MAX_SOCKET_NODE_NUM-1;

    return OSAL_TRUE;
}


static  T_IPRT_SOCKET_NODE  * iprtAllocSocketNode()
{
      uint32_t                 dwIndex;
	  T_IPRT_SOCKET_NODE  *  ptIprtSocketNode=NULL;

      OsPort_SemTake(sgtIprtSocketNodePool.tMutex,WAIT_FOREVER);

      /* reserve one element in the pool , avoid to break down the link*/
      if(sgtIprtSocketNodePool.dwCount ==1)
      {
		   osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtAllocSocketNode Failed,the pool is empty!\n");
           OsPort_SemGive(sgtIprtSocketNodePool.tMutex);
		   return NULL;
      }

      sgtIprtSocketNodePool.dwCount --;

      dwIndex = sgtIprtSocketNodePool.dwHead;
      sgtIprtSocketNodePool.dwHead = sgptIprtSocketNodeBaseAddr[dwIndex].dwNextInPool;

	  if (sgtIprtSocketNodePool.dwHead == OSAL_INVALID_UINT32 )	  /* no available resource */
		  sgtIprtSocketNodePool.dwTail = OSAL_INVALID_UINT32;

      /*double check */
      if(dwIndex >= IPRT_MAX_SOCKET_NODE_NUM)
      {
		   osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtAllocSocketNode Failed,the link is broken!\n");
           OsPort_SemGive(sgtIprtSocketNodePool.tMutex);
		   return NULL;
      }

	  ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+dwIndex;

      ptIprtSocketNode->ucIsUse         = IPRT_SOCKET_NODE_USED;

	  /*detach from pool link */
	  ptIprtSocketNode->dwNextInPool   = OSAL_INVALID_UINT32;

      /* initialize the members*/
      ptIprtSocketNode->ucType              = IPRT_SOCKET_TYPE_UNKNOW;
      ptIprtSocketNode->ucState             = IPRT_SOCKET_STATE_BLANK;
      ptIprtSocketNode->iSockFd             = INVALID_SOCKET;
	  ptIprtSocketNode->dwListenId	        = OSAL_INVALID_UINT32;

      ptIprtSocketNode->tOwnerPid.dwPno       = OSAL_INVALID_UINT32;

      ptIprtSocketNode->dwLocalIp           = 0;
      ptIprtSocketNode->wLocalPort          = 0;

      ptIprtSocketNode->dwRemoteIp          = 0;
      ptIprtSocketNode->wRemotePort         = 0;

      ptIprtSocketNode->dwRecvMsgNum        = 0;
      ptIprtSocketNode->dwRecvMsgErrNum     = 0;
      ptIprtSocketNode->dwRecvBytes         = 0;

      ptIprtSocketNode->dwSendMsgNum        = 0;
      ptIprtSocketNode->dwSendMsgErrNum     = 0;
      ptIprtSocketNode->dwSendBytes         = 0;

      OsPort_SemGive(sgtIprtSocketNodePool.tMutex);

     // osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[iprt]:iprtAllocSocketNode successfully,NodeId=%d!\n",dwIndex);

      return  ptIprtSocketNode;
}


static  int32_t  iprtFreeSocketNode(T_IPRT_SOCKET_NODE  * ptIprtSocketNode)
{
    uint32_t    dwIndex;

    if(ptIprtSocketNode == NULL)
    {
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtFreeSocketNode Failed,the para is null!\n");
		return OSAL_FALSE;
    }

    dwIndex=ptIprtSocketNode->dwSelfId;
    if((dwIndex >=IPRT_MAX_SOCKET_NODE_NUM))
    {
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtFreeSocketNode,invalid dwIndex!\n");
        return OSAL_FALSE;
    }

    if(ptIprtSocketNode->ucIsUse == IPRT_SOCKET_NODE_UNUSED)
    {
	    osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtFreeSocketNode Failed,the node is not used!\n");
	    return OSAL_FALSE ;
    }


    OsPort_SemTake(sgtIprtSocketNodePool.tMutex,WAIT_FOREVER);

    /* the pool is empty, so both the head and the tail are connected to the node */
	if(OSAL_INVALID_UINT32 == sgtIprtSocketNodePool.dwHead )
	{
		sgtIprtSocketNodePool.dwHead  = dwIndex;
		sgtIprtSocketNodePool.dwTail  = dwIndex;
	}
	else
	{
	    /* not empty, put the node in the tail*/
	    sgptIprtSocketNodeBaseAddr[sgtIprtSocketNodePool.dwTail].dwNextInPool = dwIndex;
		sgtIprtSocketNodePool.dwTail  =  dwIndex;
	}
    sgtIprtSocketNodePool.dwCount ++;

    OsPort_SemGive(sgtIprtSocketNodePool.tMutex);

    ptIprtSocketNode->ucIsUse         = IPRT_SOCKET_NODE_UNUSED;
    ptIprtSocketNode->dwNextInPool   = OSAL_INVALID_UINT32;

    //osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[iprt]:iprtFreeSocketNode successfully,NodeId=%d!\n",dwIndex);
    return OSAL_TRUE;
}


static fd_set tIprtReadFdSet;
static fd_set tIprtWriteFdSet;
static fd_set tIprtExceptFdSet;

static sem_ctrl_blk_t *        sgtFdSetMutex  = NULL;


static uint32_t iprSockettHandleError(T_IPRT_SOCKET_NODE  * ptIprtSocketNode, const char *pErrMsg)
{
    int               dwCurSock;
    iprt_close_msg    tCloseMsg;
	char              strLocalIp[20]={0},strRemoteIp[20]={0};
    uint8_t           ipAddr[4];

    memcpy(&ipAddr, &(ptIprtSocketNode->dwLocalIp), 4 );
    sprintf(strLocalIp, "%d.%d.%d.%d",ipAddr[3],ipAddr[2],ipAddr[1],ipAddr[0]);

    memcpy(&ipAddr, &(ptIprtSocketNode->dwRemoteIp), 4 );
    sprintf(strRemoteIp, "%d.%d.%d.%d",ipAddr[3],ipAddr[2],ipAddr[1],ipAddr[0]);

    osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "errno =%d: msg= %s \n", errno, strerror(errno));
    osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:close socketnode(id=%d, local_ip=%s, local_port=%d, Remote_ip=%s, remote_port=%d) due to %s\n",
                   ptIprtSocketNode->dwSelfId,
                   strLocalIp,
                   ptIprtSocketNode->wLocalPort,
                   strRemoteIp,
                   ptIprtSocketNode->wRemotePort,
                   pErrMsg);
    if (IPRT_SOCKET_STATE_BLANK == ptIprtSocketNode->ucState)
    {
        iprtFreeSocketNode(ptIprtSocketNode);
        return OSAL_OK;
    }

    dwCurSock = ptIprtSocketNode->iSockFd;

    OsPort_SemTake(sgtFdSetMutex, WAIT_FOREVER);
    if (FD_ISSET(dwCurSock, &tIprtReadFdSet))
    {
        FD_CLR(dwCurSock, &tIprtReadFdSet);
    }

    if (FD_ISSET(dwCurSock, &tIprtWriteFdSet))
    {
        FD_CLR(dwCurSock, &tIprtWriteFdSet);
    }

    if (FD_ISSET(dwCurSock, &tIprtExceptFdSet))
    {
        FD_CLR(dwCurSock, &tIprtExceptFdSet);
    }
    OsPort_SemGive(sgtFdSetMutex);

    close(dwCurSock);

    /*notify the app layer */
    tCloseMsg.iSockFd = ptIprtSocketNode->dwSelfId;
    if (OSAL_OK != osal_msg_deliver_from_task(ptIprtSocketNode->tOwnerPid,
                                              MSG_IPRT_SOCKET_CLOSE,
                                              (void *)&tCloseMsg,
                                              (uint16_t)(sizeof(iprt_close_msg))))
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprSockettHandleError, send MSG_IPRT_SOCKET_CLOSE message failed!\n");
    }

    iprtFreeSocketNode(ptIprtSocketNode);

    return OSAL_OK;
}



static uint32_t iprtSocketHandleTcpAccept(T_IPRT_SOCKET_NODE  * ptIprtListenSocketNode)
{
    int                acceptSocket;
    struct sockaddr_in    fromAddr;
    unsigned int          addrSize      = sizeof(fromAddr);
    uint32_t                dwfromIpAddr  = 0;
    uint16_t                wfromPort     = 0;
    iprt_accept_msg       tAcceptMsg;
    T_IPRT_SOCKET_NODE  * ptIprtConnSocketNode = NULL;

    acceptSocket = accept(ptIprtListenSocketNode->iSockFd, (struct sockaddr *)&fromAddr, &addrSize);
    if (INVALID_SOCKET == acceptSocket)
    {
        int32_t   dwErr;
        dwErr = OsPort_GetLastError();
        if (EWOULDBLOCK != dwErr)
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "[iprt]:iprtSocketHandleTcpAccept, accept failed!\n");
	        return OSAL_ERROR;
        }
        else
        {
            return OSAL_OK;
        }
    }

    dwfromIpAddr = (uint32_t)ntohl(fromAddr.sin_addr.s_addr);
    wfromPort    = (uint16_t)ntohs(fromAddr.sin_port);

    /* save the socket */
    ptIprtConnSocketNode = iprtAllocSocketNode();

    if(ptIprtConnSocketNode != NULL)
    {
        // firstly to init the status info before notify to the app level
        ptIprtConnSocketNode->ucType           = ptIprtListenSocketNode->ucType;
        ptIprtConnSocketNode->ucState          = IPRT_SOCKET_STATE_ACTIVE;
		ptIprtConnSocketNode->dwListenId       = ptIprtListenSocketNode->dwSelfId;

        ptIprtConnSocketNode->iSockFd          = acceptSocket;
        ptIprtConnSocketNode->tOwnerPid        = ptIprtListenSocketNode->tOwnerPid;

        ptIprtConnSocketNode->dwLocalIp        = ptIprtListenSocketNode->dwLocalIp;
        ptIprtConnSocketNode->wLocalPort       = ptIprtListenSocketNode->wLocalPort;

        ptIprtConnSocketNode->dwRemoteIp       = dwfromIpAddr;
        ptIprtConnSocketNode->wRemotePort      = wfromPort;

        OsPort_SemTake(sgtFdSetMutex, WAIT_FOREVER);
        FD_SET(ptIprtConnSocketNode->iSockFd, &tIprtReadFdSet);
        OsPort_SemGive(sgtFdSetMutex);

        //notify the app level
        memset(&tAcceptMsg, 0, sizeof(iprt_accept_msg));

        tAcceptMsg.iListenSockFd = ptIprtListenSocketNode->dwSelfId;
        tAcceptMsg.iConnetSockFd = ptIprtConnSocketNode->dwSelfId;

        tAcceptMsg.tPeerAddr      = fromAddr;
        tAcceptMsg.dwAddrlen      = addrSize;

        if (OSAL_OK != osal_msg_deliver_from_task(ptIprtListenSocketNode->tOwnerPid,
                                        MSG_IPRT_SOCKET_ACCEPT,
                                        &tAcceptMsg,
                                        sizeof(iprt_accept_msg)))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "[iprt]:iprtSocketHandleTcpAccept, send MSG_IPRT_SOCKET_ACCEPT message failed!\n");
			close(acceptSocket);
            iprtFreeSocketNode(ptIprtConnSocketNode);
            return OSAL_ERROR;
        }
        return OSAL_OK;
    }
	else
	{
        /* no resource to save the connection ,close it*/
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "[iprt]:iprtSocketHandleTcpAccept, there is no resource to save the connection ,close it!\n");
        close(acceptSocket);
        return OSAL_ERROR;
	}
}


static uint32_t iprtSocketHandleTcpData(T_IPRT_SOCKET_NODE  * ptIprtSocketNode)
{
    int               iRetVal       = 0;
	iprt_recv_msg   * ptRecvMsg   = NULL;
    uint8_t             aucRecvBuf[IPRT_TCP_RCV_SIZE_DFLT+sizeof(iprt_recv_msg)];
    uint8_t           * ptRecvBufPrt = NULL;

    ptRecvMsg = (iprt_recv_msg   *)aucRecvBuf;

	ptRecvBufPrt = ptRecvMsg->aucPktBuf;
    iRetVal = recv(ptIprtSocketNode->iSockFd,ptRecvBufPrt,IPRT_TCP_RCV_SIZE_DFLT, 0);

    if (iRetVal <0)
    {
        int32_t dwErr;
        dwErr = OsPort_GetLastError();
        if (EWOULDBLOCK != dwErr) /*&& (ECONNRESET != dwErr) && (dwErr!=WASEINPROGRESS) */
        {
            iprSockettHandleError(ptIprtSocketNode, "[iprt]:iprtSocketHandleTcpData fail to recv from socket");
            return OSAL_ERROR;
		}
		else
		{
            return OSAL_OK;
		}
    }

    if (0 == iRetVal)
    {
        iprSockettHandleError(ptIprtSocketNode, "iprtSocketHandleTcpData recv null data");
        return OSAL_ERROR;
    }

    ptRecvMsg = (iprt_recv_msg *)aucRecvBuf;
    ptRecvMsg->iSockFd     = ptIprtSocketNode->dwSelfId;
    ptRecvMsg->dwSrcIpAddr  = ptIprtSocketNode->dwRemoteIp;
    ptRecvMsg->wSrcPort     = ptIprtSocketNode->wRemotePort;

    ptRecvMsg->dwDestIpAddr = ptIprtSocketNode->dwLocalIp;
    ptRecvMsg->wDestPort    = ptIprtSocketNode->wLocalPort;

    ptRecvMsg->dwPktLen     = iRetVal;
	ptRecvMsg->wStreamId    = 0;

    if (OSAL_OK != osal_msg_deliver_from_task(ptIprtSocketNode->tOwnerPid,
                                    MSG_IPRT_SOCKET_RECV,
                                    (void *)aucRecvBuf,
                                    (uint16_t)(sizeof(iprt_recv_msg) + iRetVal)))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "[iprt]:iprtSocketHandleTcpData, send MSG_IPRT_SOCKET_RECV message failed!\n");
    }
	//osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "[iprt]:iprtSocketHandleTcpData, send MSG_IPRT_SOCKET_RECV message OK!\n");

    ptIprtSocketNode->dwRecvBytes += iRetVal;
    ptIprtSocketNode->dwRecvMsgNum++;

    return OSAL_OK;
}


static uint32_t  iprtSocketHandleUdpData(T_IPRT_SOCKET_NODE  * ptIprtSocketNode)
{
    int               iRetVal       = 0;
	iprt_recv_msg   * ptRecvMsg   = NULL;
    uint8_t             aucRecvBuf[IPRT_UDP_RCV_SIZE_DFLT+sizeof(iprt_recv_msg)];
    uint8_t           * ptRecvBufPrt = NULL;

    struct sockaddr_in  peer_addr;
	unsigned int        sin_size;

    sin_size=sizeof(struct sockaddr_in);

    ptRecvMsg = (iprt_recv_msg   *)aucRecvBuf;
	ptRecvBufPrt = ptRecvMsg->aucPktBuf;

    iRetVal = recvfrom(ptIprtSocketNode->iSockFd,ptRecvBufPrt,IPRT_TCP_RCV_SIZE_DFLT, 0,(struct sockaddr *)&peer_addr, &sin_size);

    if (iRetVal < 0)
    {
        int32_t dwErr;
        dwErr = OsPort_GetLastError();

        if (EWOULDBLOCK != dwErr) /*&& (ECONNRESET != dwErr) && (dwErr!=WASEINPROGRESS) */
        {
            iprSockettHandleError(ptIprtSocketNode, "[iprt]:iprtSocketHandleUdpData fail to recv from socket");
            return OSAL_ERROR;
        }
        return OSAL_OK;
    }


    ptRecvMsg->iSockFd     = ptIprtSocketNode->dwSelfId;
    ptRecvMsg->dwSrcIpAddr = ntohl(peer_addr.sin_addr.s_addr);
    ptRecvMsg->wSrcPort    = ntohs(peer_addr.sin_port);
    ptRecvMsg->dwDestIpAddr = ptIprtSocketNode->dwLocalIp;
    ptRecvMsg->wDestPort    = ptIprtSocketNode->wLocalPort;
    ptRecvMsg->dwPktLen     = iRetVal;
	ptRecvMsg->wStreamId    = 0;

    if (OSAL_OK != osal_msg_deliver_from_task(ptIprtSocketNode->tOwnerPid,
                                    MSG_IPRT_SOCKET_RECV,
                                    (void *)aucRecvBuf,
                                    (uint16_t)(sizeof(iprt_recv_msg) + iRetVal)))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "[iprt]:iprtSocketHandleTcpData, send MSG_IPRT_SOCKET_RECV message failed!\n");
    }

    ptIprtSocketNode->dwRecvBytes += iRetVal;
    ptIprtSocketNode->dwRecvMsgNum++;

    return OSAL_OK;
}


static uint32_t iprtSocketHandleRawData(T_IPRT_SOCKET_NODE  * ptIprtSocketNode)
{
    int               iRetVal       = 0;
	iprt_recv_msg   * ptRecvMsg   = NULL;
    uint8_t             aucRecvBuf[IPRT_TCP_RCV_SIZE_DFLT+sizeof(iprt_recv_msg)];
    uint8_t           * ptRecvBufPrt = NULL;

    ptRecvMsg = (iprt_recv_msg   *)aucRecvBuf;

	ptRecvBufPrt = ptRecvMsg->aucPktBuf;
    iRetVal = recv(ptIprtSocketNode->iSockFd,ptRecvBufPrt,IPRT_TCP_RCV_SIZE_DFLT, 0);

    if (iRetVal <0)
    {
        int32_t dwErr;
        dwErr = OsPort_GetLastError();

        if (EWOULDBLOCK != dwErr) /*&& (ECONNRESET != dwErr) && (dwErr!=WASEINPROGRESS) */
        {
            iprSockettHandleError(ptIprtSocketNode, "[iprt]:iprtSocketHandleTcpData fail to recv from socket");
            return OSAL_ERROR;
        }
        return OSAL_OK;
    }

    if (0 == iRetVal)
    {
        iprSockettHandleError(ptIprtSocketNode, "[iprt]:iprtSocketHandleTcpData recv null data");
        return OSAL_ERROR;
    }

    ptRecvMsg = (iprt_recv_msg *)aucRecvBuf;
    ptRecvMsg->iSockFd      = ptIprtSocketNode->dwSelfId;
    ptRecvMsg->dwSrcIpAddr  = ptIprtSocketNode->dwRemoteIp;
    ptRecvMsg->wSrcPort     = ptIprtSocketNode->wRemotePort;
    ptRecvMsg->dwDestIpAddr = ptIprtSocketNode->dwLocalIp;
    ptRecvMsg->wDestPort    = ptIprtSocketNode->wLocalPort;
    ptRecvMsg->dwPktLen     = iRetVal;
	ptRecvMsg->wStreamId    = 0;

    if (OSAL_OK != osal_msg_deliver_from_task(ptIprtSocketNode->tOwnerPid,
                                    MSG_IPRT_SOCKET_RECV,
                                    (void *)aucRecvBuf,
                                    (uint16_t)(sizeof(iprt_recv_msg) + iRetVal)))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "[iprt]:iprtSocketHandleTcpData, send MSG_IPRT_SOCKET_RECV message failed!\n");
    }

    ptIprtSocketNode->dwRecvBytes += iRetVal;
    ptIprtSocketNode->dwRecvMsgNum++;

    return OSAL_OK;
}


int iprtSocketHandleTcpSend(T_IPRT_SOCKET_NODE  * ptIprtSocketNode)
{
    return 0;
}


int iprtSocketHandleUdpSend(T_IPRT_SOCKET_NODE  * ptIprtSocketNode)
{
    return 0;
}


static int iprtHandeNetworkEvent(fd_set *pReadSet, fd_set *pWriteSet, fd_set *pExecSet)
{
    uint32_t                dwIndex;
    T_IPRT_SOCKET_NODE  * ptIprtSocketNode = NULL;

    int                curSock;
    int                   result;

    for (dwIndex = 0; dwIndex < IPRT_MAX_SOCKET_NODE_NUM; dwIndex++)
    {
        ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+dwIndex;

        if (  (ptIprtSocketNode->ucIsUse == IPRT_SOCKET_NODE_UNUSED)
			||(ptIprtSocketNode->ucType  ==IPRT_SOCKET_TYPE_UNKNOW)
			||(ptIprtSocketNode->iSockFd == INVALID_SOCKET)
			)
        {
            continue ;
        }

        curSock = ptIprtSocketNode->iSockFd;
        if (FD_ISSET(curSock, pExecSet))
        {
            iprSockettHandleError(ptIprtSocketNode, "disconnect due to exception happen");
        }

        if (FD_ISSET(curSock, pReadSet))
        {
                result = OSAL_OK;

    			if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_TCP)
    			{
                       switch (ptIprtSocketNode->ucState)
                       {
                               case IPRT_SOCKET_STATE_LISTEN:
                               {
                                   result = iprtSocketHandleTcpAccept(ptIprtSocketNode);
                               }
                               break;

                               case IPRT_SOCKET_STATE_ACTIVE:
                               {
                                   result = iprtSocketHandleTcpData(ptIprtSocketNode);
                               }
                               break;

                               default:
							   {
                                      osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtHandeNetworkEvent,tcp socket,invalid state!\n");
                               }
							   break;
                       }
    			}
    			else if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_UDP)
    			{
                       result = iprtSocketHandleUdpData(ptIprtSocketNode);
    			}
    			else if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_RAW)
    			{
                       result = iprtSocketHandleRawData(ptIprtSocketNode);
    			}
				else if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_SCTP)
				{
                       switch (ptIprtSocketNode->ucState)
                       {
                               case IPRT_SOCKET_STATE_LISTEN:
                               {
                                   result = iprtSocketHandleTcpAccept(ptIprtSocketNode);
                               }
                               break;

                               case IPRT_SOCKET_STATE_ACTIVE:
                               {
                                   result = iprtSocketHandleTcpData(ptIprtSocketNode);
                               }
                               break;

                               default:
							   {
                                      osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtHandeNetworkEvent,tcp socket,invalid state!\n");
                               }
							   break;
                       	}
				}
				else
				{
                      osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtHandeNetworkEvent,invalid socket type!\n");
				}
        }

        if (FD_ISSET(curSock, pWriteSet))
        {
                result = OSAL_OK;
    			if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_TCP)
    			{
                     if(ptIprtSocketNode->ucState == IPRT_SOCKET_STATE_ACTIVE)
                     {

                     }
    			}
				else if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_SCTP)
				{
                     if(ptIprtSocketNode->ucState == IPRT_SOCKET_STATE_ACTIVE)
                     {

                     }
				}
    			else if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_UDP)
    			{

    			}
    			else if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_RAW)
    			{

    			}
				else
				{
                      osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:iprtHandeNetworkEvent,invalid socket type!\n");
				}

        }
    }
	return result;
}


static void iprtNetScanEntry(void)
{

    fd_set   reads, writes, excepts;
    int             result;
    struct timeval  timeoutValue;

    timeoutValue.tv_sec  = 0;
    timeoutValue.tv_usec = 1000;

    OsPort_SemTake(sgtFdSetMutex, WAIT_FOREVER);
    reads   = tIprtReadFdSet;
    writes  = tIprtWriteFdSet;
    excepts = tIprtExceptFdSet;
    OsPort_SemGive(sgtFdSetMutex);

    while (1)
    {

        timeoutValue.tv_sec  = 0;
        timeoutValue.tv_usec = 1000;

        result = select(FD_SETSIZE, &reads, &writes, &excepts, &timeoutValue);
		if(result==0)
		{
		}
        else if(result>0)
        {
            iprtHandeNetworkEvent(&reads, &writes, &excepts);
        }
		else /* (result<0) */
		{
		    if(OsPort_GetLastError()== EINTR)
		    {
		        // FD_ZERO(&reads);
                // myhandler(SIGINT);
		    }
			else
			{
		         OsPort_HandSocketError();
			}
		}
        OsPort_SemTake(sgtFdSetMutex, WAIT_FOREVER);
        reads   = tIprtReadFdSet;
        writes  = tIprtWriteFdSet;
        excepts = tIprtExceptFdSet;
        OsPort_SemGive(sgtFdSetMutex);
    }
}



int32_t  SocketInit(void)
{
    if(OSAL_FALSE == iprtInitSocketNode())
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[iprt]:iprtInitSocketNode Failed!\n");
          return OSAL_FALSE;
    }

    if((sgtFdSetMutex  = OsPort_SemMCreate()) ==NULL )
    {
           osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[iprt]:init socket,OsPort_SemMCreate failure!\n");
           return OSAL_FALSE;
    }


    FD_ZERO(&tIprtReadFdSet);
    FD_ZERO(&tIprtWriteFdSet);
    FD_ZERO(&tIprtExceptFdSet);

    OsPort_taskSpawn("tIprtNetTask", (FUNCPTR)iprtNetScanEntry, 40, 20000, 0);

	return OSAL_TRUE;
}


int  iprt_socket(int domain, int type, int protocol)
{
    int                iSocket;
    T_IPRT_SOCKET_NODE  * ptIprtSocketNode = NULL;

    if(domain != AF_INET)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_socket fail,invalid domain.\n");
        return INVALID_SOCKET;
    }

    ptIprtSocketNode = iprtAllocSocketNode();
    if(ptIprtSocketNode == NULL)
    {
         osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_socket fail, no resource.\n");
		 return INVALID_SOCKET;
    }
	else
    {

        if(type == SOCK_STREAM)
        {

            ptIprtSocketNode->ucType  = IPRT_SOCKET_TYPE_TCP;
            ptIprtSocketNode->ucState = IPRT_SOCKET_STATE_BLANK;
		}
		else if(type == SOCK_DGRAM)
        {
             ptIprtSocketNode->ucType  =IPRT_SOCKET_TYPE_UDP;
             ptIprtSocketNode->ucState = IPRT_SOCKET_STATE_ACTIVE;

		}
		else if(type == SOCK_RAW)
		{
             ptIprtSocketNode->ucType  =IPRT_SOCKET_TYPE_RAW;
             ptIprtSocketNode->ucState = IPRT_SOCKET_STATE_ACTIVE;
		}
		else
		{
            iprtFreeSocketNode(ptIprtSocketNode);
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_socket fail,invalid type.\n");
            return INVALID_SOCKET;
		}

        iSocket = socket(domain, type, protocol);
        if (INVALID_SOCKET == iSocket)
        {
            iprtFreeSocketNode(ptIprtSocketNode);
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_socket fail to create socket.\n");
            return INVALID_SOCKET;
        }

        ptIprtSocketNode->iSockFd      = iSocket;
        ptIprtSocketNode->tOwnerPid.dwPno    = osal_task_get_self_pno();

        //osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_socket create successfully,socket_node=%d,socket_fd=%d \n",ptIprtSocketNode->dwSelfId,iSocket);
        return ptIprtSocketNode->dwSelfId;
    }
}


int iprt_bind(int  sockfd, struct sockaddr *my_addr, int addrlen)
{
    T_IPRT_SOCKET_NODE  * ptIprtSocketNode = NULL;

    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_bind failed, the socket(sockfd:%d) is invalid!\n", sockfd);
          return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_bind failed, the socket(sockfd: %d) is not created.\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_bind failed, the socket(sockfd: %d) is not created by the Process.\n", sockfd);
        return -1;
    }


    if((ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_TCP)||(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_SCTP))
    {
        if (IPRT_SOCKET_STATE_BLANK != ptIprtSocketNode->ucState)
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_bind failed, the socket(sockfd: %d) state is error.\n", sockfd);
            return -1;
        }
	}

    ptIprtSocketNode->dwLocalIp   = ntohl(((struct sockaddr_in *)my_addr)->sin_addr.s_addr);
    ptIprtSocketNode->wLocalPort  = ntohs(((struct sockaddr_in *)my_addr)->sin_port);

    if(   (ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_UDP)
		||(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_RAW)
		||(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_TCP)
		)
    {
		if (SOCKET_ERROR == bind(ptIprtSocketNode->iSockFd, my_addr, addrlen))
		{
			iprSockettHandleError(ptIprtSocketNode,"Failed to bind!\n");
			return -1;
		}
	}

    if(   (ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_UDP) ||(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_RAW))
    {
         ptIprtSocketNode->ucState = IPRT_SOCKET_STATE_ACTIVE;

		 OsPort_SemTake(sgtFdSetMutex, WAIT_FOREVER);
         FD_SET((uint32_t)ptIprtSocketNode->iSockFd, &tIprtReadFdSet);
    	 OsPort_SemGive(sgtFdSetMutex);
    }
    return 0;
}


int iprt_connect(int sockfd, struct sockaddr *serv_addr, int addrlen)
{
    T_IPRT_SOCKET_NODE  *ptIprtSocketNode = NULL;

    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_connect fail, the socket(sockfd:%d) is invalid!\n", sockfd);
        return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_connect fail, the socket(sockfd: %d) is not created.\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_connect fail, the socket(sockfd: %d) is not create by local.\n", sockfd);
        return -1;
    }

    if (IPRT_SOCKET_STATE_BLANK != ptIprtSocketNode->ucState)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_connect fail, the socket(sockfd: %d) state is error.\n", sockfd);
        return -1;
    }

    ptIprtSocketNode->dwRemoteIp  = ntohl(((struct sockaddr_in *)serv_addr)->sin_addr.s_addr);
    ptIprtSocketNode->wRemotePort = ntohs(((struct sockaddr_in *)serv_addr)->sin_port);

	if(   (ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_UDP)
		||(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_RAW)
		||(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_TCP)
		)
	{
		if (SOCKET_ERROR == connect(ptIprtSocketNode->iSockFd, serv_addr, addrlen))
		{
	        char              strPeerIP[20]={0};
            uint8_t           ipAddr[4];

            memcpy(&ipAddr, &(((struct sockaddr_in *)serv_addr)->sin_addr.s_addr), 4 );
            sprintf(strPeerIP, "%d.%d.%d.%d",ipAddr[0],ipAddr[1],ipAddr[2],ipAddr[3]);
			iprSockettHandleError(ptIprtSocketNode,"Failed to connect!\n");
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_connect fail, socket(sockfd: %d),connect [%s] return error.\n", ptIprtSocketNode->iSockFd,strPeerIP);
			return -1;
		}

	}

    ptIprtSocketNode->ucState = IPRT_SOCKET_STATE_ACTIVE;

    OsPort_SemTake(sgtFdSetMutex, WAIT_FOREVER);
    FD_SET((uint32_t)ptIprtSocketNode->iSockFd, &tIprtReadFdSet);
	OsPort_SemGive(sgtFdSetMutex);

    return 0;
}


int iprt_listen(int sockfd, int backlog)
{
    T_IPRT_SOCKET_NODE  *ptIprtSocketNode = NULL;

    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_listen fail, the socket(sockfd:%d) is invalid!\n", sockfd);
          return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_listen fail, the socket(sockfd: %d) is not created.\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_listen fail, the socket(sockfd: %d) is not create by local.\n", sockfd);
        return -1;
    }

    if (IPRT_SOCKET_STATE_BLANK != ptIprtSocketNode->ucState)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_listen fail, the socket(sockfd: %d) state is error.\n", sockfd);
        return -1;
    }

    if (SOCKET_ERROR == listen(ptIprtSocketNode->iSockFd, backlog))
    {
        iprSockettHandleError(ptIprtSocketNode,"Failed to listen!\n");
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_listen fail (sockfd: %d),listen return error.\n", sockfd);
        return -1;
    }

    ptIprtSocketNode->ucState = IPRT_SOCKET_STATE_LISTEN;


	OsPort_SemTake(sgtFdSetMutex, WAIT_FOREVER);
    FD_SET((uint32_t)ptIprtSocketNode->iSockFd, &tIprtReadFdSet);
	OsPort_SemGive(sgtFdSetMutex);


    return 0;
}


int iprt_send(int sockfd, char *data, int datalen, int flags)
{
    int                   dwRet            = 0;
    int                   dwTotalSent      = 0;
    T_IPRT_SOCKET_NODE  * ptIprtSocketNode = NULL;

    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_send fail, the socket(sockfd:%d) is invalid!\n", sockfd);
          return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_send fail, the socket(sockfd: %d) is not created.\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_send fail, the socket(sockfd: %d) is not create by local.\n", sockfd);
        return -1;
    }

    if ((IPRT_SOCKET_TYPE_TCP != ptIprtSocketNode->ucType)&&(IPRT_SOCKET_TYPE_SCTP != ptIprtSocketNode->ucType))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_send fail, the socket(sockfd: %d) is not a tcp .\n", sockfd);
        return -1;
    }

    if (IPRT_SOCKET_STATE_ACTIVE != ptIprtSocketNode->ucState)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_send fail, the socket(sockfd: %d) state is error.\n", sockfd);
        return -1;
    }

    //osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:start to Send, datalen=%u!\n",datalen);
    dwRet = send(ptIprtSocketNode->iSockFd, data, datalen, flags);
    dwTotalSent = dwRet;
    while (dwRet>0 && dwTotalSent < datalen)
    {
        ptIprtSocketNode->dwSendBytes += dwRet;
        ptIprtSocketNode->dwSendMsgNum++;
        dwTotalSent += dwRet;
        dwRet = send(ptIprtSocketNode->iSockFd, data+dwTotalSent, datalen-dwTotalSent, flags);
        if(dwRet < 0)
        {
            int32_t      wErrorno;
            wErrorno = OsPort_GetLastError();
  		    if((wErrorno != EWOULDBLOCK)&&(wErrorno!=EINPROGRESS)&&(wErrorno!=ECONNRESET))
  		    {
                ptIprtSocketNode->dwSendMsgErrNum++;
                iprSockettHandleError(ptIprtSocketNode,"Failed to Send!\n");
                return dwRet;
  		    }
        }
    }
    //osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:Sent completed!\n");
    return dwRet<=0? dwRet:dwTotalSent;
}


int iprt_sendto(int            sockfd,
                  char            * data,
                  int               datalen,
                  int               flags,
                  struct sockaddr * to,
                  int               tolen)
{
    int                   iRetVal  = 0;
    int                   dwTotalSent      = 0;

    T_IPRT_SOCKET_NODE  * ptIprtSocketNode = NULL;
    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_sendto fail, the socket(sockfd:%d) is invalid!\n", sockfd);
        return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_sendto failed, the socket(sockfd: %d) is not used .\n", sockfd);
        return -1;
    }

    if (IPRT_SOCKET_TYPE_UDP != ptIprtSocketNode->ucType)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_sendto failed, the socket(sockfd: %d) is not a udp .\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_sendto failed, the socket(sockfd: %d) is not create by local.\n", sockfd);
        return -1;
    }

    if (IPRT_SOCKET_STATE_ACTIVE != ptIprtSocketNode->ucState)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_sendto faild, the socket(sockfd: %d) state is error.\n", sockfd);
        return -1;
    }

    iRetVal = sendto(ptIprtSocketNode->iSockFd, data, datalen, flags, (struct sockaddr *)to, tolen);
    dwTotalSent = iRetVal;
    while (iRetVal>0 && dwTotalSent < datalen)
    {
        ptIprtSocketNode->dwSendBytes += iRetVal;
        ptIprtSocketNode->dwSendMsgNum++;
        dwTotalSent += iRetVal;
        iRetVal = sendto(ptIprtSocketNode->iSockFd, data, datalen, flags, (struct sockaddr *)to, tolen);
        if(iRetVal < 0)
        {
            int32_t      wErrorno;
            wErrorno = OsPort_GetLastError();
  		    if((wErrorno != EWOULDBLOCK)&&(wErrorno!=EINPROGRESS)&&(wErrorno!=ECONNRESET))
  		    {
                ptIprtSocketNode->dwSendMsgErrNum++;
                iprSockettHandleError(ptIprtSocketNode,"Failed to Send!\n");
                return iRetVal;
  		    }
        }
    }
    return iRetVal<=0? iRetVal:dwTotalSent;
}


int iprt_close(int sockfd)
{
    T_IPRT_SOCKET_NODE  * ptIprtSocketNode = NULL;
	int                iSockFd = INVALID_SOCKET;

    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_close fail, the socket(sockfd:%d) is invalid!\n", sockfd);
          return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_close fail, the socket(sockfd: %d) is not created.\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_close fail, the socket(sockfd: %d) is not create by local.\n", sockfd);
        return -1;
    }

	OsPort_SemTake(sgtFdSetMutex, WAIT_FOREVER);
    if (FD_ISSET(ptIprtSocketNode->iSockFd, &tIprtReadFdSet))
    {
        FD_CLR((uint32_t)ptIprtSocketNode->iSockFd, &tIprtReadFdSet);
    }
    if (FD_ISSET(ptIprtSocketNode->iSockFd, &tIprtWriteFdSet))
    {
        FD_CLR((uint32_t)ptIprtSocketNode->iSockFd, &tIprtWriteFdSet);
    }
    if (FD_ISSET(ptIprtSocketNode->iSockFd, &tIprtExceptFdSet))
    {
        FD_CLR((uint32_t)ptIprtSocketNode->iSockFd, &tIprtExceptFdSet);
    }
	OsPort_SemGive(sgtFdSetMutex);


    iSockFd = ptIprtSocketNode->iSockFd;

    iprtFreeSocketNode(ptIprtSocketNode);

    close(iSockFd);

	return 0;
}


int iprt_setsockopt(int sockfd, int level, int optname, char *optval, int optlen)
{
    T_IPRT_SOCKET_NODE  * ptIprtSocketNode = NULL;

    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_send fail, the socket(sockfd:%d) is invalid!\n", sockfd);
          return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_setsockopt fail to set the socket opt, the socket(sockfd: %d) is not created.\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_getsockopt fail, the socket(sockfd: %d) is create by Pno=%u not %u.\n", sockfd, ptIprtSocketNode->tOwnerPid.dwPno ,osal_task_get_self_pno());
        return -1;
    }

    return setsockopt(ptIprtSocketNode->iSockFd, level, optname, optval, optlen);

}


int iprt_getsockopt(int sockfd, int level, int optname, char *optval, unsigned int *optlen)
{
    T_IPRT_SOCKET_NODE  *ptIprtSocketNode = NULL;

    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_send fail, the socket(sockfd:%d) is invalid!\n", sockfd);
          return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_getsockopt fail to get the socket opt, the socket(sockfd: %d) is not created.\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_getsockopt fail, the socket(sockfd: %d) is create by Pno=%u not %u.\n", sockfd, ptIprtSocketNode->tOwnerPid.dwPno ,osal_task_get_self_pno());
        return -1;
    }

    return getsockopt(ptIprtSocketNode->iSockFd, level, optname, optval, optlen);
}


int iprt_getsockname(int sockfd, struct sockaddr *addr, unsigned int *addr_len)
{
    T_IPRT_SOCKET_NODE  *ptIprtSocketNode = NULL;

    if ((NULL == addr) || (NULL == addr_len))
    {
        return -1;
    }

    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_send fail, the socket(sockfd:%d) is invalid!\n", sockfd);
          return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_getsockname fail to get the socket local info, the socket(sockfd: %d) is not created.\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_getsockname fail to get the socket local info, the socket(sockfd: %d) is not create by local.\n", sockfd);
        return -1;
    }

    return getsockname(ptIprtSocketNode->iSockFd, (struct sockaddr *)addr, addr_len);
}



int iprt_getpeername(int sockfd, struct sockaddr *addr, unsigned int  *addr_len)
{
    T_IPRT_SOCKET_NODE  *ptIprtSocketNode = NULL;

    if ((NULL == addr) || (NULL == addr_len))
    {
        return -1;
    }

    if(sockfd>=IPRT_MAX_SOCKET_NODE_NUM)
    {
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_send fail, the socket(sockfd:%d) is invalid!\n", sockfd);
          return -1;
    }
    ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+sockfd;

    if (IPRT_SOCKET_NODE_UNUSED == ptIprtSocketNode->ucIsUse)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_getpeername fail to get the socket remote info, the socket(sockfd: %d) is not created.\n", sockfd);
        return -1;
    }

    if (osal_task_get_self_pno() != ptIprtSocketNode->tOwnerPid.dwPno)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[iprt]:iprt_getpeername fail to get the socket remote info, the socket(sockfd: %d) is not create by local.\n", sockfd);
        return -1;
    }

    getpeername(ptIprtSocketNode->iSockFd, (struct sockaddr *)addr, addr_len);
    return 0;
}


void iprt_cmd_show_socket(int argc, char *argv[])
{
    T_IPRT_SOCKET_NODE * ptIprtSocketNode = NULL;
    uint32_t               dwIndex          = 0;
	char                 strType[5];
	char                 strState[5];

    telnet_print("  +--------+------+-------+-----+-----------+-----------+-----------+----------+\r\n");
    telnet_print("  |SockFd  |Type  |State  |Pno  |RecvErr    |RecvByte   |SendErr    |SendBytes | \r\n");
    telnet_print("  +--------+------+-------+-----+-----------+-----------+-----------+----------+\r\n");

    for (dwIndex = 0; dwIndex < IPRT_MAX_SOCKET_NODE_NUM; dwIndex++)
    {
        ptIprtSocketNode = sgptIprtSocketNodeBaseAddr+dwIndex;

		if(ptIprtSocketNode->ucIsUse == IPRT_SOCKET_NODE_UNUSED)
		{
		    continue;
		}

        memset(strType,0,5);
        if(ptIprtSocketNode->ucType== IPRT_SOCKET_TYPE_TCP)
        {
             strncpy(strType,"TCP\0",5);
        }
        else if(ptIprtSocketNode->ucType== IPRT_SOCKET_TYPE_SCTP)
        {
             strncpy(strType,"SCTP\0",5);
        }
		else if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_UDP)
		{
             strncpy(strType,"UDP\0",5);
		}
		else if(ptIprtSocketNode->ucType == IPRT_SOCKET_TYPE_RAW)
		{
             strncpy(strType,"RAW\0",5);
		}
		else
		{
             strncpy(strType,"UNK\0",5);
		}

        memset(strState,0,5);
        if(ptIprtSocketNode->ucState == IPRT_SOCKET_STATE_BLANK)
        {
             strncpy(strState,"BLK\0",5);
        }
		else if(ptIprtSocketNode->ucState == IPRT_SOCKET_STATE_LISTEN)
		{
             strncpy(strState,"LSN\0",5);
		}
		else if(ptIprtSocketNode->ucState == IPRT_SOCKET_STATE_ACTIVE)
		{
             strncpy(strState,"ACT\0",5);
		}
		else
		{
             strncpy(strState,"UNK\0",5);
		}


        telnet_print("  |%-6d  |%-4s  |%-5s  |%-3d  |0x%-8x |0x%-8x |0x%-8x |0x%-8x| \r\n",
                               dwIndex,
                               strType,
                               strState,
                               ptIprtSocketNode->tOwnerPid.dwPno,
                               ptIprtSocketNode->dwRecvMsgErrNum,
                               ptIprtSocketNode->dwRecvBytes,
                               ptIprtSocketNode->dwSendMsgErrNum,
                               ptIprtSocketNode->dwSendBytes);
    }
    telnet_print("  +--------+------+-------+-----+-----------+-----------+-----------+----------+\r\n");
}






