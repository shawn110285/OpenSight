

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

#include "../include/telnetapi.h"
#include "../include/telnetfunc.h"

T_Telnet_Command * g_ptCmdTab    = NULL;

void TelnetSetupCmdList()
{
    T_Telnet_Command * ptCmdBaseAddr = NULL;
	T_Telnet_Command * ptCurrNode    = NULL;
	T_Telnet_Command * ptNextNode    = NULL;
	T_Telnet_Command * ptCmd         = NULL;

	uint32_t  dwTotalCmdNum  = 150;
	uint32_t  dwCurrCmdIndex = 0;
	uint32_t  dwNextCmdIndex = 0;
	uint32_t  dwLoop         = 0;
	uint32_t  dwCmdIndex     = 0;

    ptCmdBaseAddr =(T_Telnet_Command *)osal_mem_alloc(dwTotalCmdNum*sizeof(T_Telnet_Command));
	if(ptCmdBaseAddr == NULL)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,osal_mem_alloc failed!\r\n");
		return;
	}

    g_ptCmdTab = ptCmdBaseAddr;

    /*===================== ?:print help information ====================*/
    ptCmd = ptCmdBaseAddr + dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
		osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[telnet]:TelnetSetupPlatformCmdList,there are too many command !\r\n");
		return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "?", strlen("?"));
    strncpy(ptCmd->cName[1], "?",  strlen("?"));
    strncpy(ptCmd->cDetail, "print help information",  strlen("print help information"));
    strncpy(ptCmd->cProgressName, "telnet",  strlen("telnet"));
    ptCmd->wParaNum = 0;

   /*===================== help:print help information ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[telnet]:TelnetSetupPlatformCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "help",  strlen("help"));
    strncpy(ptCmd->cName[1], "help",  strlen("help"));
    strncpy(ptCmd->cDetail, "print help information",  strlen("print help information"));
    strncpy(ptCmd->cProgressName, "telnet",  strlen("telnet"));
    ptCmd->wParaNum = 0;

    /*===================== quit: exit telnet ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[telnet]:TelnetSetupPlatformCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "quit", strlen("quit"));
    strncpy(ptCmd->cName[1], "quit",  strlen("quit"));
    strncpy(ptCmd->cDetail, "exit telnet",  strlen("exit telnet"));
    strncpy(ptCmd->cProgressName, "telnet",  strlen("telnet"));
    ptCmd->wParaNum = 0;

    /*=====================  history: list the history commands ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[telnet]:TelnetSetupPlatformCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "history",  strlen("history"));
    strncpy(ptCmd->cName[1], "history",  strlen("history"));
    strncpy(ptCmd->cDetail, "list the history commands",  strlen("list the history commands"));
    strncpy(ptCmd->cProgressName, "telnet",  strlen("telnet"));
    ptCmd->wParaNum = 0;

    /*=====================setlogger: show, or set remote monitoring parameters  ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "setlogger",  strlen("setlogger"));
    strncpy(ptCmd->cName[1], "setlogger",  strlen("setlogger"));
    strncpy(ptCmd->cDetail, "show, or set remote monitoring parameters",  strlen("show, or set remote monitoring parameters"));
    strncpy(ptCmd->cProgressName, "telnet",  strlen("telnet"));
    ptCmd->wParaNum = 2;
    strncpy(ptCmd->tPara[0].cPara, "off|on",  strlen("off|on"));
    ptCmd->tPara[0].wParaType = 8;
    strncpy(ptCmd->tPara[1].cPara, "level(0,1,2)",  strlen("level(0,1,2)"));
    ptCmd->tPara[1].wParaType = 1;

    /*=====================showversion: show the version infomation  ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "showversion",  strlen("showversion"));
    strncpy(ptCmd->cName[1], "showversion",  strlen("showversion"));
    strncpy(ptCmd->cDetail, "show the version infomation",  strlen("show the version infomation"));
    strncpy(ptCmd->cProgressName, "telnet",  strlen("telnet"));
    ptCmd->wParaNum = 0;


#define DP_IR_ABORT       0x08   // 0b1000                0b11111000                   35              JTAG-DP Abort Register, ABORT.
#define DP_IR_DPACC       0x0A   // 0b1010                0b11111010                   35              JTAG DPAP Access Registers, DPACC.
#define DP_IR_APACC       0x0B   // 0b1011                0b11111011                   35              JTAG DPAP Access Registers,APACC.
#define DP_IR_IDCODE      0x0E   // 0b1110                0b11111110                   32              JTAG Device ID Code Register, IDCODE.
#define DP_IR_BYPASS      0x0F   // 0b1111                0b11111111                   1               JTAG Bypass Register, BYPASS.

#if 0
    /*=====================jtag_read_reg: read the jtag registers(idcode, bypass, DPACC, APACC)  ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "jtag_read_reg",  strlen("jtag_read_reg"));
    strncpy(ptCmd->cName[1], "jtag_read_reg",  strlen("jtag_read_reg"));
    strncpy(ptCmd->cDetail, "read jtag registers(idcode, bypass, DPACC, APACC, etc)",  strlen("read jtag registers(idcode, bypass, DPACC, APACC, etc)"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 1;
    strncpy(ptCmd->tPara[0].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[0].wParaType = E_TYPE_HEX;
#endif

    /*=====================jtag_read_idcode: read jtag tap id(idcode)  ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "jtag_read_idcode",  strlen("jtag_read_idcode"));
    strncpy(ptCmd->cName[1], "jtag_read_idcode",  strlen("jtag_read_idcode"));
    strncpy(ptCmd->cDetail, "read jtag tap id(idcode)",  strlen("read jtag tap id(idcode)"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 0;

    /*=====================jtag_enumerate: enumerate the jtag chain  ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "jtag_enumerate",  strlen("jtag_enumerate"));
    strncpy(ptCmd->cName[1], "jtag_enumerate",  strlen("jtag_enumerate"));
    strncpy(ptCmd->cDetail, "enumerate the jtag chain",  strlen("enumerate the jtag chain"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 0;

    /*=====================dp_read_reg: read dp registers (CTRL_STAT, RBBUFF, APSEL) ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "dp_read_reg",  strlen("dp_read_reg"));
    strncpy(ptCmd->cName[1], "dp_read_reg",  strlen("dp_read_reg"));
    strncpy(ptCmd->cDetail, "read dp registers(CTRL_STAT, AP Select, etc)",  strlen("read dp registers(idcode, bypass, DPACC, AP Select, etc)"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 1;
    strncpy(ptCmd->tPara[0].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[0].wParaType = E_TYPE_HEX;

   /*=====================dp_write_reg: write dp registers (CTRL_STAT, RBBUFF, APSEL) ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "dp_write_reg",  strlen("dp_write_reg"));
    strncpy(ptCmd->cName[1], "dp_write_reg",  strlen("dp_write_reg"));
    strncpy(ptCmd->cDetail, "write dp registers(CTRL_STAT, AP Select, etc)",  strlen("write dp registers(CTRL_STAT, AP Select, etc)"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 2;
    strncpy(ptCmd->tPara[0].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[0].wParaType = E_TYPE_HEX;
    strncpy(ptCmd->tPara[1].cPara, "value",  strlen("value"));
    ptCmd->tPara[1].wParaType = E_TYPE_HEX;

   /*=====================ap_read_reg: read ap registers (CSW, portsel, PSTA, ap_idcode) ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "ap_read_reg",  strlen("ap_read_reg"));
    strncpy(ptCmd->cName[1], "ap_read_reg",  strlen("ap_read_reg"));
    strncpy(ptCmd->cDetail, "read ap registers (CSW, portsel, PSTA, ap_idcode)",  strlen("read ap registers (CSW, portsel, PSTA, ap_idcode)" ) );
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 2;
    strncpy(ptCmd->tPara[0].cPara, "ap_num",  strlen("ap_num"));
    ptCmd->tPara[0].wParaType = E_TYPE_DWORD;
    strncpy(ptCmd->tPara[1].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[1].wParaType = E_TYPE_HEX;

    /*=====================ap_write_reg: write ap registers (CSW, portsel, PSTA, ap_idcode) ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "ap_write_reg",  strlen("ap_write_reg"));
    strncpy(ptCmd->cName[1], "ap_write_reg",  strlen("ap_write_reg"));
    strncpy(ptCmd->cDetail, "write ap registers (CSW, portsel, PSTA, ap_idcode)",  strlen("write ap registers (CSW, portsel, PSTA, ap_idcode)" ) );
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 3;
    strncpy(ptCmd->tPara[0].cPara, "ap_num",  strlen("ap_num"));
    ptCmd->tPara[0].wParaType = E_TYPE_DWORD;
    strncpy(ptCmd->tPara[1].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[1].wParaType = E_TYPE_HEX;
    strncpy(ptCmd->tPara[2].cPara, "value",  strlen("value"));
    ptCmd->tPara[2].wParaType = E_TYPE_HEX;

    //the tap beneath the jtag_ap
    /*=====================read_riscv_tap_reg: read riscv tap's registers (idcode, dtmcontrol, bypass, dmi etc) ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "read_riscv_tap_reg",  strlen("read_riscv_tap_reg"));
    strncpy(ptCmd->cName[1], "read_riscv_tap_reg",  strlen("read_riscv_tap_reg"));
    strncpy(ptCmd->cDetail, "read riscv tap's registers (idcode, dtmcontrol, bypass, dmi etc)",  strlen("read riscv tap's registers (idcode, dtmcontrol, bypass, dmi etc)"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 1;
    strncpy(ptCmd->tPara[0].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[0].wParaType = E_TYPE_HEX;

    /*=====================read_riscv_tap_idcode: read risc-v tap's idcode ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "read_riscv_tap_idcode",  strlen("read_riscv_tap_idcode"));
    strncpy(ptCmd->cName[1], "read_riscv_tap_idcode",  strlen("read_riscv_tap_idcode"));
    strncpy(ptCmd->cDetail, "read risc-v tap's idcode",  strlen("read risc-v tap's idcode"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 0;

    /*=====================read_riscv_tap_dtmctrl: read riscv tap register(dtm control) ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "read_riscv_tap_dtmctrl",  strlen("read_riscv_tap_dtmctrl"));
    strncpy(ptCmd->cName[1], "read_riscv_tap_dtmctrl",  strlen("read_riscv_tap_dtmctrl"));
    strncpy(ptCmd->cDetail, "read risc-v tap's dtmctrl reg",  strlen("read risc-v tap's dtmctrl reg"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 0;

    /*=====================read_riscv_dm_reg: read riscv dm module's register(DMCONTROL, DMSTATUS, COMMAND, ABSTRACTCS, etc) ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "read_riscv_dm_reg",  strlen("read_riscv_dm_reg"));
    strncpy(ptCmd->cName[1], "read_riscv_dm_reg",  strlen("read_riscv_dm_reg"));
    strncpy(ptCmd->cDetail, "read riscv dm module's register(DMCONTROL, DMSTATUS, COMMAND, etc)",  strlen("read riscv dm module's register(DMCONTROL, DMSTATUS, COMMAND, etc)"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 1;
    strncpy(ptCmd->tPara[0].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[0].wParaType = E_TYPE_HEX;

    /*=====================write_riscv_dm_reg: write riscv dm module's register(DMCONTROL, DMSTATUS, COMMAND, ABSTRACTCS, etc) ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "write_riscv_dm_reg",  strlen("write_riscv_dm_reg"));
    strncpy(ptCmd->cName[1], "write_riscv_dm_reg",  strlen("write_riscv_dm_reg"));
    strncpy(ptCmd->cDetail, "write riscv dm module's register(DMCONTROL, DMSTATUS, COMMAND, etc)",  strlen("write riscv dm module's register(DMCONTROL, DMSTATUS, COMMAND, etc)"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 2;
    strncpy(ptCmd->tPara[0].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[0].wParaType = E_TYPE_HEX;
    strncpy(ptCmd->tPara[1].cPara, "value",  strlen("value"));
    ptCmd->tPara[1].wParaType = E_TYPE_HEX;

    /*=====================apb_ap_probe: probe the apb ap ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "apb_ap_probe",  strlen("apb_ap_probe"));
    strncpy(ptCmd->cName[1], "apb_ap_probe",  strlen("apb_ap_probe"));
    strncpy(ptCmd->cDetail, "probe the apb ap",  strlen("probe the apb ap"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 0;

    /*=====================apb_ap_dump_rom_table: dump the apb ap rom table ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "apb_ap_dump_rom_table",  strlen("apb_ap_dump_rom_table"));
    strncpy(ptCmd->cName[1], "apb_ap_dump_rom_table",  strlen("apb_ap_dump_rom_table"));
    strncpy(ptCmd->cDetail, "dump the apb ap rom table",  strlen("dump the apb ap rom table"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 0;

    /*=====================apb_ap_read_reg: read apb ap registers ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "apb_ap_read_reg",  strlen("apb_ap_read_reg"));
    strncpy(ptCmd->cName[1], "apb_ap_read_reg",  strlen("apb_ap_read_reg"));
    strncpy(ptCmd->cDetail, "read apb ap registers",  strlen("read apb ap registers"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 1;
    strncpy(ptCmd->tPara[0].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[0].wParaType = E_TYPE_HEX;

    /*=====================apb_ap_write_reg: write apb ap registers ====================*/
    ptCmd->ptNext = ptCmdBaseAddr + dwCmdIndex;
    ptCmd = ptCmdBaseAddr+dwCmdIndex;
	dwCmdIndex++;
	if(dwCmdIndex>=dwTotalCmdNum)
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[telnet]:TelnetSetupCmdList,there are too many command !\r\n");
        return;
	}
    memset(ptCmd, 0, sizeof(T_Telnet_Command));
    ptCmd->wCmdWordNum = 1;
    strncpy(ptCmd->cName[0], "apb_ap_write_reg",  strlen("apb_ap_write_reg"));
    strncpy(ptCmd->cName[1], "apb_ap_write_reg",  strlen("apb_ap_write_reg"));
    strncpy(ptCmd->cDetail, "write apb ap registers",  strlen("write apb ap registers"));
    strncpy(ptCmd->cProgressName, "dap",  strlen("dap"));
    ptCmd->wParaNum = 2;
    strncpy(ptCmd->tPara[0].cPara, "reg_addr",  strlen("reg_addr"));
    ptCmd->tPara[0].wParaType = E_TYPE_HEX;
    strncpy(ptCmd->tPara[1].cPara, "value",  strlen("value"));
    ptCmd->tPara[1].wParaType = E_TYPE_HEX;

    /*ended the link with NULL ptr*/
    ptCmd->ptNext = NULL;

}

