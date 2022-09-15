/*-------------------------------------------------------------------------
// Module:  dp-jtag (JTAG Debug Port)
// File:    jtag_cmd.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: some telnet commands supported on  the dp-jtag interface
--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jtag.h"

extern void    telnet_print(const char *IszPrnMsg,...);

extern jtag_t * sgptJtagInst;


// cmd line: jtag_read_reg reg_addr
void cmd_jtag_read_reg(int  argc, char * argv[])
{
	unsigned ir, x;
	unsigned p0 = 0x80808080;

    // read the first tap device id
	ir = strtoul(argv[0], 0, 0);
	jtag_goto(sgptJtagInst, JTAG_RESET);
	jtag_ir_wr(sgptJtagInst, 4, &ir);
	jtag_dr_io(sgptJtagInst, 32, &p0, &x);
	jtag_commit(sgptJtagInst);
	telnet_print("the reg (%08x) = 0x%08x \r\n", ir, x);
}

// cmd_line: jtag_read_idcode
void cmd_jtag_read_idcode(int  argc, char * argv[])
{
	unsigned ir, x;
	unsigned p0 = 0x80808080;

    // read the first tap device id
	ir = 0x01;
	jtag_goto(sgptJtagInst, JTAG_RESET);
	jtag_ir_wr(sgptJtagInst, 4, &ir);
	jtag_dr_io(sgptJtagInst, 32, &p0, &x);
	jtag_commit(sgptJtagInst);
	telnet_print("the idcoe:0x%08x\r\n", x);
}


//cmd_line: jtag_enumerate
static uint8_t ONES[1024];

void cmd_jtag_enumerate(int  argc, char * argv[])
{
	jtag_info_t * info;
	uint32_t data[DEVMAX];
	int n;

	jtag_clear_state(sgptJtagInst);
	sgptJtagInst->devcount = 0;

	jtag_goto(sgptJtagInst, JTAG_RESET);
	jtag_goto(sgptJtagInst, JTAG_RESET);
	jtag_goto(sgptJtagInst, JTAG_RESET);
	jtag_dr_io(sgptJtagInst, DEVMAX * 4 * 8, ONES, data);

	if (jtag_commit(sgptJtagInst))
	{
		return;
	}

	for (n = 0; n < DEVMAX; n++)
	{
		if (data[n] == 0xffffffff)
		{
			if (n == 0)
			{
				telnet_print("no devices found\n");
				return;
			}
			sgptJtagInst->devcount = n;
			return;
		}

		if (!(data[n] & 1))
		{
			telnet_print("device %d has no idcode, the data=%08x\n", n, data[n]);
			return;
		}
		else
		{
			telnet_print("device %d has idcode[%08x]\n", n, data[n]);
		}

		if ((info = jtag_lookup_device(data[n])) == NULL)
		{
			telnet_print("device %d (id %08x) unknown\n", n, (unsigned) data[n]);
			return;
		}
		else
		{
			telnet_print("device %d (id %08x) is a %s \n", n, data[n], info->name);
		}
		memcpy(sgptJtagInst->devinfo + n, info, sizeof(jtag_info_t));
	}
	telnet_print("too many devices\n");
}

