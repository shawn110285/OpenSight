
/*-------------------------------------------------------------------------
// Module:  jtag_ap
// File:    jtag_ap_cmd.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: some telnet commands supported on the jtag-ap interface
--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jtag_ap.h"

extern void    telnet_print(const char *IszPrnMsg,...);

extern dap_t  * sgptDapInst;
extern int      sgiJtagApNum;


// cmd line: read_riscv_tap_idcode
void cmd_jtag_ap_read_idcode(int  argc, char * argv[])
{
      int jtag_ap_apnum = sgiJtagApNum;
      uint32_t tmp[2];  // tmp can hold 64 bits of data

      if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
      {
            telnet_print("  usage: read_riscv_tap_idcode \r\n"  );
            return;
      }

      tmp[0] = 0x1;
      jtag_ap_scan_ir(sgptDapInst, jtag_ap_apnum, &tmp[0], 5);   //risc-v irlen = 5
      telnet_print("try to read the idcode from A710 CPU TAP\n");

      tmp[0] = 0x0;
      jtag_ap_scan_dr(sgptDapInst, jtag_ap_apnum, &tmp[0], 32);
      telnet_print("the idcode = 0x%08x\n", tmp[0]);
}


// cmd line: read_riscv_tap_reg reg_addr
void cmd_jtag_ap_read_reg(int  argc, char * argv[])
{
      uint32_t addr;
      uint32_t val;
      int      jtag_ap_apnum = sgiJtagApNum;
      uint32_t tmp[2];  // tmp can hold 64 bits of data

      if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
      {
            telnet_print("  usage: read_riscv_tap_reg reg_addr \r\n"  );
            return;
      }

      addr = strtoul(argv[0], 0, 0);

      tmp[0] = addr;
      jtag_ap_scan_ir(sgptDapInst, jtag_ap_apnum, &tmp[0], 5);   //risc-v irlen = 5
      telnet_print("try to read the reg from the beneath TAP\n");

      tmp[0] = 0x0;
      jtag_ap_scan_dr(sgptDapInst, jtag_ap_apnum, &tmp[0], 32);
      telnet_print("Reg (0x%x) beneath jtag_ap = 0x%08x\n", addr, tmp[0]);
}


// cmd line: read_riscv_tap_dtmctrl
void cmd_jtag_ap_read_dtmctrl(int  argc, char * argv[])
{
      int      jtag_ap_apnum = sgiJtagApNum;
      uint32_t tmp[2];  // tmp can hold 64 bits of data
      uint32_t val;

      if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
      {
            telnet_print("  usage: read_riscv_tap_dtmctrl\r\n"  );
            return;
      }

      // read the risc-v dtmcontrol register
      tmp[0] = 0x10;
      jtag_ap_scan_ir(sgptDapInst, jtag_ap_apnum, &tmp[0], 5);   //risc-v irlen = 5
      telnet_print("try to read the dtmcontrol from RISC-V CPU\n");

      tmp[0] = 0x0;
      jtag_ap_scan_dr(sgptDapInst, jtag_ap_apnum, &tmp[0], 32);
      telnet_print("dtmcontrol reg (0x10) = 0x%x \r\n", val);
}



// cmd line: dap_read_reg reg_addr
void cmd_jtag_ap_write_riscv_dm_reg(int  argc, char * argv[])
{
      int      jtag_ap_apnum = sgiJtagApNum;
      uint32_t tmp[2];  // tmp can hold 64 bits of data
      uint32_t val;
      uint32_t addr;

      if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
      {
            telnet_print("  usage: jtag_ap_write_riscv_dm_reg reg_addr \r\n"  );
            return;
      }

      addr = strtoul(argv[0], 0, 0);
      val  = strtoul(argv[1], 0, 0);

      // access the risc-v dm registers via the dmi (bus = 7 bits dap, jtag_ap_apnum + 32 bits data + 2bits opcode [1: read, 2: write])
      tmp[0] = 0x11; // DMI
      jtag_ap_scan_ir(sgptDapInst, jtag_ap_apnum, &tmp[0], 5);   //risc-v irlen = 5
      telnet_print("  scan_ir to DMI\n");

      //prepare the risc-v dmi data (7 bits address + 32 bits data + 2 bits opcode)
      // write the dmControl (dmControl = 0x80000001), active the debug module(bit0=1, bit31=1)
      tmp[1] = ((addr&0x7f) << 2) | (val>>30);
      tmp[0] = ((val&0x3FFFFFFF) << 2) | 0x2;
      jtag_ap_scan_dr(sgptDapInst, jtag_ap_apnum, &tmp[0], 41);
      telnet_print("  write the riscv dm reg (0x%x) and op code (write) to DR, dr = 0x%08x - 0x%08x \n", addr, tmp[1], tmp[0]);
}



// cmd line: dap_read_reg reg_addr
void cmd_jtag_ap_read_riscv_dm_reg(int  argc, char * argv[])
{
      int      jtag_ap_apnum = sgiJtagApNum;
      uint32_t tmp[2];  // tmp can hold 64 bits of data
      uint32_t val;
      uint32_t addr;

      if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
      {
            telnet_print("  usage: jtag_ap_write_riscv_dm_reg reg_addr \r\n"  );
            return;
      }

      addr = strtoul(argv[0], 0, 0);

      // access the risc-v dm registers via the dmi (bus = 7 bits dap, jtag_ap_apnum + 32 bits data + 2bits opcode [1: read, 2: write])
      tmp[0] = 0x11; // DMI
      jtag_ap_scan_ir(sgptDapInst, jtag_ap_apnum, &tmp[0], 5);   //risc-v irlen = 5
      telnet_print("  scan_ir to DMI\n");

      //prepare the risc-v dmi data (7 bits address + 32 bits data + 2 bits opcode)
      tmp[1] = ((addr&0x7f) << 2) | (val>>30);
      tmp[0] = ((val&0x3FFFFFFF) << 2) | 0x1;
      jtag_ap_scan_dr(sgptDapInst, jtag_ap_apnum, &tmp[0], 41);
      telnet_print("  write the riscv dm reg (0x%x) and op code (read) to DR, dr = 0x%08x - 0x%08x \n", addr, tmp[1], tmp[0]);

      //read the data
      jtag_ap_scan_dr(sgptDapInst, jtag_ap_apnum, &tmp[0], 41);
      val = ( (tmp[1]&0x3) << 30 )|( (tmp[1] & 0xFFFFFFFC) >> 2);
      telnet_print(" DM Reg(0x%x) = 0x%08x\n", addr, val);
}

