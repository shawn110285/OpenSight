

/*-------------------------------------------------------------------------
// Module:  DP (Debug Port)
// File:    dp_cmd.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: telnet commands supported on the dp interface
--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dp.h"

extern void    telnet_print(const char *IszPrnMsg,...);

jtag_t * sgptJtagInst;
dap_t  * sgptDapInst;

// cmd line: dap_read_reg reg_addr
void cmd_dp_read_reg(int  argc, char * argv[])
{
    uint32_t apnum ;
    uint32_t addr;
    uint32_t val;

	if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
    {
		telnet_print("  usage: dap_read_reg reg_addr \r\n"  );
        return;
    }

    addr = strtoul(argv[0], 0, 0);
    dap_dp_read(sgptDapInst, addr, &val);
    telnet_print("reg (0x%x) = 0x%x \r\n", addr, val);
}


// cmd line: dap_write_reg reg_addr value
void cmd_dp_write_reg(int  argc, char * argv[])
{
    uint32_t apnum ;
    uint32_t addr;
    uint32_t val;

	if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
    {
		telnet_print("  usage: dap_write_reg reg_addr value \r\n"  );
        return;
    }

    addr = strtoul(argv[0], 0, 0);
    val = strtoul(argv[1], 0, 0);
    dap_dp_write(sgptDapInst, addr, val);
    telnet_print("set reg (0x%x) = 0x%x \r\n", addr, val);
}

// cmd line: dap_ap_read_reg ap_num reg_addr
void cmd_ap_read_reg(int  argc, char * argv[])
{
    uint32_t apnum ;
    uint32_t addr;
    uint32_t val;

	if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
    {
		telnet_print("  usage: dap_ap_read_reg ap_num reg_addr \r\n"  );
        return;
    }

    apnum = strtoul(argv[1], 0, 0);
    addr = strtoul(argv[2], 0, 0);
    dap_ap_read(sgptDapInst, apnum, addr, &val);
    telnet_print("reg(0x%x) at apnum (%d) return value=0x%x \r\n", addr, apnum, val);

}


// cmd line: dap_ap_write_reg ap_num reg_addr value
void cmd_ap_write_reg(int  argc, char * argv[])
{
    uint32_t apnum ;
    uint32_t addr;
    uint32_t val;

	if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
    {
		telnet_print("  usage: dap_ap_read_reg ap_num reg_addr \r\n"  );
        return;
    }

    apnum = strtoul(argv[1], 0, 0);
    addr = strtoul(argv[2], 0, 0);
    val = strtoul(argv[3], 0, 0);
    dap_ap_write(sgptDapInst, apnum, addr, val);
    telnet_print("set reg(0x%x) at apnum (%d) = 0x%x \r\n", addr, apnum, val);
}