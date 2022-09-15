//-----------------------------------------------------------------------------
// File:    dapTestEntry.c
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

#include "../osal/include/osalcfg.h"
#include "../osal/include/osalapi.h"
#include "../osal/include/osalmsgdef.h"
#include "../telnet_svr/include/telnetapi.h"


#include "dapTestEntry.h"


jtag_t * sgptJtagInst = NULL;
dap_t  * sgptDapInst = NULL;
int      sgiJtagApNum = 0;
int      sgiApbApNum = 1;
uint32_t dwJtagClkDiv = 100;


extern config_param_t  gtSysCfgParam;

static T_Telnet_Exec_Command sgatDapCmdTab[] =
{
    //jtag-dp
	{ "jtag_read_reg",	        cmd_jtag_read_reg 		    	},
	{ "jtag_read_idcode",	    cmd_jtag_read_idcode 			},
	{ "jtag_enumerate",	        cmd_jtag_enumerate 			    },

    //dp
	{ "dp_read_reg",	        cmd_dp_read_reg		      	    },
	{ "dp_write_reg",	        cmd_dp_write_reg		    	},
	{ "ap_read_reg",	        cmd_ap_read_reg  			    },
	{ "ap_write_reg",	        cmd_ap_write_reg	            },

    //the tap beneath the jtag_ap
	{ "read_riscv_tap_reg",	    cmd_jtag_ap_read_reg		   	},
	{ "read_riscv_tap_idcode",	cmd_jtag_ap_read_idcode			},
	{ "read_riscv_tap_dtmctrl",	cmd_jtag_ap_read_dtmctrl		},
	{ "read_riscv_dm_reg",   	cmd_jtag_ap_read_riscv_dm_reg	},
	{ "write_riscv_dm_reg",     cmd_jtag_ap_write_riscv_dm_reg	},

    //apb-ap
	{ "apb_ap_probe",	        cmd_apb_ap_probe		    	},
	{ "apb_ap_dump_rom_table",	cmd_apb_ap_dump_rom_table		},
	{ "apb_ap_read_reg",	    cmd_apb_ap_read_reg			    },
	{ "apb_ap_write_reg",	    cmd_apb_ap_write_reg			}
};


static int32_t coresight_init()
{
    dwJtagClkDiv = gtSysCfgParam.tCoreSightParam.dwJtagClkDiv;
    sgiJtagApNum = gtSysCfgParam.tCoreSightParam.dwJtagApNum;
    sgiApbApNum = gtSysCfgParam.tCoreSightParam.dwApbApNum;

    uint32_t funnel_addr  = gtSysCfgParam.tCoreSightParam.dwFunnelAddr;
    uint32_t etf_addr = gtSysCfgParam.tCoreSightParam.dwEtfAddr;                  //embedded trace FIFO
    uint32_t replicator_addr = gtSysCfgParam.tCoreSightParam.dwReplicatorAddr;
    uint32_t etb_addr = gtSysCfgParam.tCoreSightParam.dwEtbAddr;
    uint32_t tpiu_addr = gtSysCfgParam.tCoreSightParam.dwTpiuAddr;

    // Declare a temporary array of uint32_t large enough to hold the data for/from the longest scanchain
    uint32_t tmp[2];  // tmp can hold 64 bits of data

    if (jtag_init(&sgptJtagInst))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[dap]: jtag init failed, File:%s, Line:%d\n", __FILE__, __LINE__);
        return -1;
    }

    if ((sgptDapInst = dap_init(sgptJtagInst, gtSysCfgParam.tCoreSightParam.dwDapId)) == NULL)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[dap]:dap init failed, File:%s, Line:%d\n", __FILE__, __LINE__);
        return -1;
    }

    if (dap_attach(sgptDapInst))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[dap]:dap attach failed, File:%s, Line:%d\n", __FILE__, __LINE__);
        return -1;
    }

#if 0
    // init the components
    tpiu_init(sgptDapInst, sgiApbApNum, tpiu_addr);
    etb_init(sgptDapInst, sgiApbApNum, etb_addr);
    replicator_init(sgptDapInst, sgiApbApNum, replicator_addr);
    funnel_init(sgptDapInst, sgiApbApNum, funnel_addr);

    // enable the components
    tpiu_start();
    etb_start();
    replicator_start(0);
    funnel_start(0);
#endif

    return 0;
}


void dapTestEntry()
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

					if(coresight_init()== OSAL_FALSE)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[dap]:Init dap failed, file: %s, Line = %d!\n", __FILE__, __LINE__);
						return;
                    }

					tSysCtrlPid.dwPno = osal_task_get_pno_by_name("sysctrl");
					if(OSAL_ERR_INVALID_PNO == tSysCtrlPid.dwPno)
					{
						osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[dap]:osal_task_get_pno_by_name for sysCtrl return invalid value! file: %s, Line = %d!\n", __FILE__, __LINE__);
						return;
					}
					osal_msg_post_out(tSysCtrlPid, MSG_DAP_SYSCTRL_READY, NULL, 0);
					osal_task_set_next_state(WORK_STATE);
				}
				break;

				default:
				{
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[dap]:gdbEntry: recv unknow msg %d on INIT_STATE \n",dwMsgId);
				}
				break;
            }
        }
		break;


        case WORK_STATE:
        {
            switch( dwMsgId )
            {
                case MSG_TELNET_TO_EXEC:  //message from telnet
                {
                    telnet_Match_And_Exec_One_Cmd((T_TELNET_TO_EXEC *)pMsgData, dwMsgLen, sgatDapCmdTab, sizeof(sgatDapCmdTab)/sizeof(sgatDapCmdTab[0]));
                }
                break;

				default:
				{
					osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[dap]:gdbEntry recv unknow msg %d on WORK_STATE \n",dwMsgId);
				}
				break;
            }
        }
		break;

		case PEND_STATE:
		{
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[dap]: receive unexpected MsgId %d in PEND_STATE\n ", dwMsgId);
		}
        break;

		default:
			osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[dap]:gdbEntry recv unknow msg %d on unknow state \n",dwMsgId);
			break;
    }
}

