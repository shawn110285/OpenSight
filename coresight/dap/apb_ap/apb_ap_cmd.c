/*-------------------------------------------------------------------------
// Module:  apb_ap
// File:    apb_ap_cmd.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the telnet cmd on the apb-ap interface
--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "apb_ap.h"
#include "apb_ap_regs.h"

extern void    telnet_print(const char *IszPrnMsg,...);

extern dap_t  * sgptDapInst;
extern int      sgiApbApNum;


static void apb_ap_dump_romtable(dap_t *dap, uint32_t apnum, uint32_t base)
{
	uint32_t cid, pid0, pid4, memtype;
	uint32_t x, addr;
	int i;

	telnet_print("TABLE   @%08x ", base);
	if (apb_ap_read_component_info(dap, apnum, base, &cid, &pid0, &pid4))
	{
		telnet_print("<error reading cid & pid>\n");
		return;
	}

    //0xFCC DEVTYPE RO 0x00000021 Device Type Identifier register
	if (apb_ap_read32(dap, apnum, base + 0xFCC, &memtype))
	{
		telnet_print("<error reading memtype>\n");
		return;
	}

	telnet_print("CID %08x  PID %08x %08x  %dKB%s\n", cid, pid4, pid0, 4 * (1 + ((pid4 & 0xF0) >> 4)), (memtype & 1) ? "  SYSMEM": "");
	for (i = 0; i < 128; i++)
	{
		if (apb_ap_read32(dap, apnum, base + i * 4, &x))
		    break;

		if (x == 0)
		    break;

		if ((x & 3) != 3)
		    continue;

		addr = base + (x & 0xFFFFF000);  // the higher 20 bits
		if (apb_ap_read_component_info(dap, apnum, addr, &cid, &pid0, &pid4))
		{
			telnet_print("<error reading cid & pid>\n");
			continue;
		}

		telnet_print("%02d: @%08x CID %08x  PID %08x %08x  %dKB\n", i, addr, cid, pid4, pid0, 4 * (1 + ((pid4 & 0xF0) >> 4)));
		if (((cid >> 12) & 0xF) == 1) // ????
		{
			apb_ap_dump_romtable(dap, apnum, addr);
		}
	}
}


void cmd_apb_ap_probe(int  argc, char * argv[])
{
      int apnum = sgiApbApNum;
	uint32_t x, y;

	// read the ap id
	if (dap_ap_read(sgptDapInst, apnum, APB_AP_IDR, &x))
	{
		telnet_print("read the idr failed \n");
		return;
	}

	if (x == 0)
	{
		telnet_print("invalid apb_ap idcode (0)\n");
		return;
	}

	// read the ctrl_status_word
	if (dap_ap_read(sgptDapInst, apnum, APB_AP_CSW, &y) == 0)
	{
		telnet_print("id = 0x%08x,  csw = %08x\n", x, y);
	}

	y = 0;
	dap_ap_read(sgptDapInst, apnum, APB_AP_BASE, &y);
	// todo: convert the y into base address
	y = y & 0xFFFFFFFE;  //Remove the lower 2 bits
      telnet_print("base addr = %08x\n", y);

	if (y && (y != 0xFFFFFFFF))
	{
		apb_ap_dump_romtable(sgptDapInst, apnum, y);
	}
}



void cmd_apb_ap_dump_rom_table(int  argc, char * argv[])
{
      int      apnum = sgiApbApNum;
	uint32_t idcode, csw, base, addr;
	uint32_t cid, pid0, pid4, memtype;
	int      i, x;

	// read the ap id
	if (dap_ap_read(sgptDapInst, apnum, APB_AP_IDR, &idcode))
	{
		telnet_print("read the idr failed \n");
		return;
	}

	if (idcode == 0)
	{
		telnet_print("invalid apb_ap idcode \n");
		return;
	}

	// read the ctrl_status_word
	if ( dap_ap_read(sgptDapInst, apnum, APB_AP_CSW, &csw) )
	{
		telnet_print("failed to read csw \n");
	      return;
	}

      // read the apb base addr
	base = 0;
	dap_ap_read(sgptDapInst, apnum, APB_AP_BASE, &base);

	// convert the y into base address
	base = base & 0xFFFFFFFE;  //Remove the lower 2 bits

      //apbid = 0x54770002, csw: 80000042(DbgSwEnable=1; size=0b010, 32 bits; AddrInc = 0b00, auto increment off; DeviceEn=1, TrInpro=0)
	telnet_print("apb_ap(%d),  id = %08x, csw = 0x%8x, base_addr = %08x \n", apnum, idcode, csw, base);
	if (base && (base != 0xFFFFFFFF))
	{
            uint32_t rom_entry_addr = 0;
            rom_entry_addr = base;
            for (i = 0; i < 128; i++)
            {
                  telnet_print("ROM Entry (%d) at address(%08x) \r\n",  i, rom_entry_addr);
                  if (apb_ap_read_component_info(sgptDapInst, apnum, rom_entry_addr, &cid, &pid0, &pid4))
                  {
                        telnet_print("<error reading cid & pid>\n");
                        continue;
                  }

                  telnet_print("ROM Entry(%02d): Addr = @%08x,  CID = %08x, PID = %08x %08x, size = %dKB\n", i, addr, cid, pid4, pid0, 4 * (1 + ((pid4 & 0xF0) >> 4)));
                  if (((cid >> 12) & 0xF) != 1)
                  {
                      break;
                  }

                  if (apb_ap_read32(sgptDapInst, apnum, base + i * 4, &x)) break;
                  if (x == 0) break;
                  if ((x & 3) != 3) break;
                  addr = base + (x & 0xFFFFF000);  // the higher 20 bits
            }
      }
}




void cmd_apb_ap_read_reg(int  argc, char * argv[])
{
      uint32_t addr;
      uint32_t val;
      int      apnum = sgiApbApNum;
      uint32_t tmp[2];  // tmp can hold 64 bits of data

      if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
      {
            telnet_print("  usage: apb_ap_read_reg reg_addr \r\n"  );
            return;
      }

      addr = strtoul(argv[0], 0, 0);
      dap_ap_read(sgptDapInst, apnum, addr, &val);
      telnet_print("reg(0x%x) at apb-ap (%d) return value=0x%x \r\n", addr, apnum, val);
}


void cmd_apb_ap_write_reg(int  argc, char * argv[])
{
      uint32_t apnum = sgiApbApNum;
      uint32_t addr;
      uint32_t val;

      if((argc==1) &&(strncmp(argv[0],"?",strlen("?"))==0))
      {
            telnet_print("usage:cmd_apb_ap_write_reg reg_addr val \r\n"  );
            return;
      }

      addr = strtoul(argv[0], 0, 0);
      val = strtoul(argv[1], 0, 0);
      dap_ap_write(sgptDapInst, apnum, addr, val);
      telnet_print("set reg(0x%x) at apb-ap (%d) = 0x%x \r\n", addr, apnum, val);
}

