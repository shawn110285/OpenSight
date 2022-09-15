
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

#include "../include/telnetfunc.h"


extern void TelnetSetupCmdList();

static  uint32_t   sdwSetupListenSocketTid = OSAL_ERR_INVALID_TIMER_ID ;
static  uint32_t   sdwIdleTimerId = OSAL_ERR_INVALID_TIMER_ID ;
static  int32_t    bRecvPacket    = OSAL_FALSE;
static  int        sgiTelnetListenSocket   = INVALID_SOCKET;
static  int        iTelnetSocket           = INVALID_SOCKET;

uint32_t  sdwCmdRunTimerId = OSAL_ERR_INVALID_TIMER_ID ;

extern config_param_t          gtSysCfgParam;

static  int32_t    telnetInitSocket();



osal_pid_t tTelnetPid;
osal_pid_t tCmdTargetPid;


void telnetEntry()
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
					osal_task_get_self_pid(&tTelnetPid);

					TelnetSetupCmdList();

					if(telnetInitSocket()== OSAL_FALSE)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:Init socket failed!\n");
						return;
                    }

					tSysCtrlPid.dwPno = osal_task_get_pno_by_name("sysctrl");
					if(OSAL_ERR_INVALID_PNO == tSysCtrlPid.dwPno)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:osal_task_get_pno_by_name for sysCtrl return invalid value!\n");
						return;
					}
					osal_msg_post_out(tSysCtrlPid,MSG_TELNET_SYSCTRL_READY,NULL,0);

					osal_task_set_next_state(WORK_STATE);
				}
				break;

				default:
				{
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:telnetEntry: recv unknow msg %d on INIT_STATE \n",dwMsgId);
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
					osal_clk_t          tABSClock;
					iprt_accept_msg   * ptAcceptMsg = NULL;

					if(dwMsgLen == sizeof(iprt_accept_msg))
					{
						ptAcceptMsg =(iprt_accept_msg *)pMsgData;
					}
					else
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:telnetEntry,recv ACCEPT msg,but length is wrong! \n");
						return;
					}

					if(iTelnetSocket == INVALID_SOCKET)
					{
						iTelnetSocket = ptAcceptMsg->iConnetSockFd;
					}
					else
					{
						char  strInfo[]="another guy is doing configuration,please retry later,thanks";
						iprt_send(ptAcceptMsg->iConnetSockFd,strInfo,strlen(strInfo),0);
						iprt_close(ptAcceptMsg->iConnetSockFd);
						return;
					}

					tnInit();

					if(sdwIdleTimerId != OSAL_ERR_INVALID_TIMER_ID)
					{
						osal_timer_stop_by_tid(sdwIdleTimerId);
						sdwIdleTimerId = OSAL_ERR_INVALID_TIMER_ID;
					}

					sdwIdleTimerId = osal_timer_start_cycle(MSG_TELNET_IDLE_TIME_OUT,0,5*60*1000);
					if(sdwIdleTimerId == OSAL_ERR_INVALID_TIMER_ID)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:telnetEntry: set the Idle timer Failed! \n");
					}
				}
				break;


                case  MSG_IPRT_SOCKET_CLOSE:
				{
					iprt_close_msg    * ptIprtCloseMsg  = NULL;

					if(dwMsgLen == sizeof(iprt_close_msg))
					{
						ptIprtCloseMsg =(iprt_close_msg *)pMsgData;

						if(iTelnetSocket == ptIprtCloseMsg->iSockFd)
						{
							iTelnetSocket = INVALID_SOCKET;
							if(sdwIdleTimerId != OSAL_ERR_INVALID_TIMER_ID)
							{
								osal_clk_t  tABSClock;
								osal_timer_stop_by_tid(sdwIdleTimerId);
								sdwIdleTimerId = OSAL_ERR_INVALID_TIMER_ID;
							}
						}
						else if(sgiTelnetListenSocket == ptIprtCloseMsg->iSockFd)
						{
							sgiTelnetListenSocket = INVALID_SOCKET;
							sdwSetupListenSocketTid = osal_timer_start_normal(MSG_TELNET_SETUP_LISTEN_TIME_OUT,0,1*60*1000);
							if(sdwSetupListenSocketTid == OSAL_ERR_INVALID_TIMER_ID)
							{
								osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:telnetEntry: set the rebuild listen socket timer Failed! \n");
							}
						}
						else
						{
							osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:telnetEntry,recv close msg,but the socket id is wrong! \n");
							return;
						}
					}
					else
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:telnetEntry,recv close msg,but length is wrong! \n");
						return;
					}
                }
				break;

				case  MSG_IPRT_SOCKET_RECV:
				{
					iprt_recv_msg    * ptIprtRecvMsg  = NULL;
					ptIprtRecvMsg =(iprt_recv_msg *)pMsgData;
					if(iTelnetSocket != ptIprtRecvMsg->iSockFd)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:telnetEntry,recv data msg,but the socket id is wrong! \n");
						return;
					}
					if(dwMsgLen != ptIprtRecvMsg->dwPktLen+sizeof(iprt_recv_msg))
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:telnetEntry,recv data msg,but the msg length is wrong! \n");
						return;
                    }
					bRecvPacket = OSAL_TRUE;
					ReadTelnet((char *)ptIprtRecvMsg->aucPktBuf, (uint16_t)ptIprtRecvMsg->dwPktLen);
				}
				break;

				case MSG_TELNET_IDLE_TIME_OUT:
				{
					if(bRecvPacket == OSAL_FALSE)
					{
						if(sdwIdleTimerId != OSAL_ERR_INVALID_TIMER_ID)
						{
							osal_timer_stop_by_tid(sdwIdleTimerId);
							sdwIdleTimerId = OSAL_ERR_INVALID_TIMER_ID;
						}
						telnet_shutdown();
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:telnetEntry: Disconnet with the client duo to Idle too long \n");
					}
					else
					{
						bRecvPacket = OSAL_FALSE;
					}
				}
				break;

				case MSG_EXEC_TO_TELNET:
				{
					osal_timer_stop_by_tid(sdwCmdRunTimerId);
				    sdwCmdRunTimerId = OSAL_ERR_INVALID_TIMER_ID;
					if (Telnet_MsgHandle_EXECUTE != sgtTelnetVty.ucCurrRunState)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:Receive a response, but telnet is not in exec status \n");
					}
					else
					{
					    telnet_Output_Exec_Result(pMsgData);
					}

				}
				break;

				case MSG_CMD_RUN_TIME_OUT:
				{
					sdwCmdRunTimerId = OSAL_ERR_INVALID_TIMER_ID;
					if (Telnet_MsgHandle_EXECUTE == sgtTelnetVty.ucCurrRunState)
					{
						sgtTelnetVty.ucCurrRunState = Telnet_MsgHandle_IDLE;
						telnet_Output_TimeOut();
					}
					else
					{
                        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:Receive a timer, but not exec status, time id = %d \n", MSG_CMD_RUN_TIME_OUT);
					}
				}
				break;

				case MSG_TELNET_EXEC_PRINT:
				{
					if (NULL == pMsgData || dwMsgLen > TELNET_MAX_INNER_BUF_LENGTH)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:Receive a error message! \n");
					}
					else
					{
					    OutputExecResult((uint8_t *)pMsgData, (uint16_t)dwMsgLen);
					}
				}
				break;

				case MSG_TELNET_SETUP_LISTEN_TIME_OUT:
				{
					telnetInitSocket();
				}
			    break;

				case MSG_MasterPowerOff:
				{
					telnetCmdRelease();
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"\n[Telnet]: Telnet Parsing Process PowerOff! \n");
					osal_task_set_next_state(PEND_STATE);
				}
				break;

				default:
				{
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:telnetEntry recv unknow msg %d on WORK_STATE \n",dwMsgId);
				}
				break;
            }
        }
		break;

		case PEND_STATE:
		{
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[telnet]: receive unexpected MsgId %d in PEND_STATE\n ", dwMsgId);
		}
        break;

		default:
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:telnetEntry recv unknow msg %d on unknow state \n",dwMsgId);
			break;
    }
}


static int32_t telnetInitSocket()
{
	struct sockaddr_in      tTeletSrvAddr;
	int opt = 1;

	sgiTelnetListenSocket = iprt_socket(AF_INET,SOCK_STREAM, 0);
	if(sgiTelnetListenSocket==INVALID_SOCKET)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:iprt_socket Failed! \n");
		return OSAL_FALSE;
	}

	if (-1 == iprt_setsockopt(sgiTelnetListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)))
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:iprt_setsockopt Failed! \n");
		return OSAL_FALSE;
	}

	memset(&tTeletSrvAddr,0,sizeof(struct sockaddr_in));
	tTeletSrvAddr.sin_family    = AF_INET;
	tTeletSrvAddr.sin_port      = htons(gtSysCfgParam.tTelnetSvrParam.wTelnetServerPort);
	tTeletSrvAddr.sin_addr.s_addr = htonl(gtSysCfgParam.tTelnetSvrParam.dwTelnetServerIpAddr);
	if(0 != iprt_bind(sgiTelnetListenSocket,(struct sockaddr *)&tTeletSrvAddr, sizeof(tTeletSrvAddr)))
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:ListenTask,bind failed!\n");
		iprt_close(sgiTelnetListenSocket);
		return OSAL_FALSE;
	}

	if(iprt_listen(sgiTelnetListenSocket,5)==-1)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[Telnet]:iprt_listen Failed! \n");
		return OSAL_FALSE;
	}

	return OSAL_TRUE;
}


void  telnet_send(char * ucBufToSend, uint16_t wBufLength)
{
	if(iTelnetSocket == INVALID_SOCKET)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:telnet_send Failed, the socket is invalid! \n");
		return;
	}
	else
	{
		iprt_send(iTelnetSocket,ucBufToSend,wBufLength,0);
	}
}


void  telnet_shutdown()
{
	if(iTelnetSocket == INVALID_SOCKET)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[Telnet]:telnet_shutdown Failed, the socket is invalid! \n");
		return;
	}
	else
	{
		iprt_close(iTelnetSocket);
		iTelnetSocket = INVALID_SOCKET;
	}
}
