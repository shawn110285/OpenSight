
//-----------------------------------------------------------------------------
// File:    gdbEntry.c
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

#include "../../osal/include/osalcfg.h"
#include "../../osal/include/osalapi.h"
#include "../../osal/include/osalmsgdef.h"
#include "../../telnet_svr/include/telnetapi.h"


static  int     sgiGdbSvrListenSocket   = INVALID_SOCKET;
static  int     sgiConnSocket           = INVALID_SOCKET;

static int32_t  gdb_svr_init_socket();
static int32_t  gdb_server_init();
static int32_t  gdb_svr_handle_packet(uint8_t * aucPktBuf,  uint16_t wPktLen);


void gdbEntry()
{
    uint32_t           dwState;
    uint32_t           dwMsgId;
    uint8_t        *   pMsgData;
    uint32_t    	   dwMsgLen;

    dwState =  osal_task_get_self_state ( );
    dwMsgId	 = osal_msg_get_id ();
    pMsgData = osal_msg_get_data();
    dwMsgLen = osal_msg_get_data_length();

    switch( dwState )
    {
        case INIT_STATE:
        {
            switch( dwMsgId )
            {
                case  MSG_MasterPowerOn:
                {
					osal_pid_t         tSysCtrlPid={0};

					if(gdb_svr_init_socket()== OSAL_FALSE)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:Init socket failed!\n");
						return;
                    }

					tSysCtrlPid.dwPno = osal_task_get_pno_by_name("sysctrl");
					if(OSAL_ERR_INVALID_PNO == tSysCtrlPid.dwPno)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:osal_task_get_pno_by_name for sysCtrl return invalid value!\n");
						return;
					}
					osal_msg_post_out(tSysCtrlPid, MSG_GDB_SYSCTRL_READY, NULL, 0);
					osal_task_set_next_state(WORK_STATE);
				}
				break;

				default:
				{
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[gdb]:gdbEntry: recv unknow msg %d on INIT_STATE \n",dwMsgId);
				}
				break;
            }
        }
		break;


        case WORK_STATE:
        {
            switch( dwMsgId )
            {
                case  MSG_IPRT_SOCKET_ACCEPT:
				{
					iprt_accept_msg   * ptAcceptMsg = NULL;

					if(dwMsgLen == sizeof(iprt_accept_msg))
					{
						ptAcceptMsg =(iprt_accept_msg *)pMsgData;
					}
					else
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:gdbEntry,recv ACCEPT msg,but length is wrong! \n");
						return;
					}

					if(sgiConnSocket == INVALID_SOCKET)
					{
						sgiConnSocket = ptAcceptMsg->iConnetSockFd;
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[gdb]:gdb got a connection from the gdb client \n");
					}
					else
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[gdb]:another gdb client try to connect, but was declined \n");
						iprt_close(ptAcceptMsg->iConnetSockFd);
						return;
					}

					gdb_server_init();
				}
				break;


                case  MSG_IPRT_SOCKET_CLOSE:
				{
					iprt_close_msg    * ptIprtCloseMsg  = NULL;

					if(dwMsgLen == sizeof(iprt_close_msg))
					{
						ptIprtCloseMsg =(iprt_close_msg *)pMsgData;

						if(sgiConnSocket == ptIprtCloseMsg->iSockFd)
						{
							sgiConnSocket = INVALID_SOCKET;
							osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[gdb]:gdbEntry,recv close msg from the gdb client! \n");
						}
						else if(sgiGdbSvrListenSocket == ptIprtCloseMsg->iSockFd)
						{
							sgiGdbSvrListenSocket = INVALID_SOCKET;
							osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:gdbEntry, the gdb server socket was closed for some reason! \n");
						}
						else
						{
							osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:gdbEntry,recv close msg,but the socket id is wrong! \n");
							return;
						}
					}
					else
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:gdbEntry,recv close msg,but length is wrong! \n");
						return;
					}
                }
				break;

				case  MSG_IPRT_SOCKET_RECV:
				{
					iprt_recv_msg    * ptIprtRecvMsg  = NULL;
					ptIprtRecvMsg =(iprt_recv_msg *)pMsgData;
					if(sgiConnSocket != ptIprtRecvMsg->iSockFd)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:gdbEntry,recv data msg,but the socket id is wrong! \n");
						return;
					}
					if(dwMsgLen != ptIprtRecvMsg->dwPktLen+sizeof(iprt_recv_msg))
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:gdbEntry,recv data msg,but the msg length is wrong! \n");
						return;
                    }

					gdb_svr_handle_packet((char *)ptIprtRecvMsg->aucPktBuf, (uint16_t)ptIprtRecvMsg->dwPktLen);
				}
				break;


				case MSG_MasterPowerOff:
				{
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"\n[gdb]: gdb Process PowerOff! \n");
					osal_task_set_next_state(PEND_STATE);
				}
				break;

				default:
				{
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[gdb]:gdbEntry recv unknow msg %d on WORK_STATE \n",dwMsgId);
				}
				break;
            }
        }
		break;

		case PEND_STATE:
		{
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[gdb]: receive unexpected MsgId %d in PEND_STATE\n ", dwMsgId);
		}
        break;

		default:
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[gdb]:gdbEntry recv unknow msg %d on unknow state \n",dwMsgId);
			break;
    }
}



extern config_param_t  gtSysCfgParam;

static int32_t gdb_svr_init_socket()
{
	struct sockaddr_in  tGdbSrvAddr;
	int opt = 1;

	sgiGdbSvrListenSocket = iprt_socket(AF_INET,SOCK_STREAM, 0);
	if(sgiGdbSvrListenSocket==INVALID_SOCKET)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[gdb]:iprt_socket Failed! \n");
		return OSAL_FALSE;
	}

	if (-1 == iprt_setsockopt(sgiGdbSvrListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)))
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[gdb]:iprt_setsockopt Failed! \n");
		return OSAL_FALSE;
	}

	memset(&tGdbSrvAddr,0,sizeof(struct sockaddr_in));
	tGdbSrvAddr.sin_family    = AF_INET;
	tGdbSrvAddr.sin_port      = htons(gtSysCfgParam.tGdbSvrParam.wGdbServerPort);
	tGdbSrvAddr.sin_addr.s_addr = htonl(gtSysCfgParam.tGdbSvrParam.dwGdbServerIpAddr);
	if(0 != iprt_bind(sgiGdbSvrListenSocket,(struct sockaddr *)&tGdbSrvAddr, sizeof(tGdbSrvAddr)))
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:ListenTask,bind failed!\n");
		iprt_close(sgiGdbSvrListenSocket);
		return OSAL_FALSE;
	}

	if(iprt_listen(sgiGdbSvrListenSocket,5)==-1)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[gdb]:iprt_listen Failed! \n");
		return OSAL_FALSE;
	}

	return OSAL_TRUE;
}




void  gdb_send(char * ucBufToSend, uint16_t wBufLength)
{
	if(sgiConnSocket == INVALID_SOCKET)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[gdb]:gdb_send Failed, the socket is invalid! \n");
		return;
	}
	else
	{
		iprt_send(sgiConnSocket,ucBufToSend, wBufLength, 0);
	}
}


void  gdb_disconnect_client()
{
	if(sgiConnSocket == INVALID_SOCKET)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[gdb]:gdb_disconnect_client Failed, the socket is invalid! \n");
		return;
	}
	else
	{
		iprt_close(sgiConnSocket);
		sgiConnSocket = INVALID_SOCKET;
	}
}


static int32_t gdb_server_init()
{

}

static int32_t gdb_svr_handle_packet(uint8_t * aucPktBuf,  uint16_t wPktLen)
{

}
