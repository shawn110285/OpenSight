/*-------------------------------------------------------------------------
// Module:  jtag
// File:    jtag_dev_list.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: list the known jtag idcode and its vendor info
--------------------------------------------------------------------------*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "jtag_dev_list.h"

#define ZYNQID(code)	   ((0x1B<<21)|(0x9<<17)|((code)<<12)|(0x49<<1)|1)
#define ZYNQMASK	       0x0FFFFFFF

static jtag_info_t sgtJtagDevList[] =
{
	/* idcode     idmask     irsize  name         family*/
	{ 0x6ba00477, 0xFFFFFFFF, 4,      "Cortex A9", "ARM A9" },

// Zynq 7000
	{ ZYNQID(0x02), ZYNQMASK, 6,      "XC7X010", "Xilinx 7" },
	{ ZYNQID(0x1b), ZYNQMASK, 6,      "XC7X015", "Xilinx 7" },
	{ ZYNQID(0x07), ZYNQMASK, 6,      "XC7X020", "Xilinx 7" },
	{ ZYNQID(0x0c), ZYNQMASK, 6,      "XC7X030", "Xilinx 7" },
	{ ZYNQID(0x11), ZYNQMASK, 6,      "XC7X045", "Xilinx 7" },

// Artix-7
	{ 0x0362D093, 0x0FFFFFFF, 6,      "XC7A35T",  "Xilinx 7" },
	{ 0x0362C093, 0x0FFFFFFF, 6,      "XC7A50T",  "Xilinx 7" },
	{ 0x03632093, 0x0FFFFFFF, 6,      "XC7A75T",  "Xilinx 7" },
	{ 0x13631093, 0x0FFFFFFF, 6,      "XC7A100T", "Xilinx 7" },
	{ 0x03631093, 0x0FFFFFFF, 6,      "XC7A100T", "Xilinx 7" },
	{ 0x03636093, 0x0FFFFFFF, 6,      "XC7A200T", "Xilinx 7" },

// Kintex-7
	{ 0x03647093, 0x0FFFFFFF, 6,      "XC7K70T",  "Xilinx 7" },
	{ 0x0364C093, 0x0FFFFFFF, 6,      "XC7K160T", "Xilinx 7" },
	{ 0x03651093, 0x0FFFFFFF, 6,      "XC7K325T", "Xilinx 7" },
	{ 0x03747093, 0x0FFFFFFF, 6,      "XC7K355T", "Xilinx 7" },
	{ 0x03656093, 0x0FFFFFFF, 6,      "XC7K410T", "Xilinx 7" },
	{ 0x03752093, 0x0FFFFFFF, 6,      "XC7K420T", "Xilinx 7" },
	{ 0x03751093, 0x0FFFFFFF, 6,      "XC7K480T", "Xilinx 7" },

// Virtex-7 (only parts with 6bit IR)
	{ 0x03671093, 0x0FFFFFFF, 6,      "XC7VT585",  "Xilinx 7" },
	{ 0x03667093, 0x0FFFFFFF, 6,      "XC7VX330T", "Xilinx 7" },
	{ 0x03682093, 0x0FFFFFFF, 6,      "XC7VX415T", "Xilinx 7" },
	{ 0x03687093, 0x0FFFFFFF, 6,      "XC7VX485T", "Xilinx 7" },
	{ 0x03692093, 0x0FFFFFFF, 6,      "XC7VX550T", "Xilinx 7" },
	{ 0x03691093, 0x0FFFFFFF, 6,      "XC7VX690T", "Xilinx 7" },
	{ 0x03696093, 0x0FFFFFFF, 6,      "XC7VX980T", "Xilinx 7" },


};

jtag_info_t * jtag_lookup_device(unsigned idcode)
{
	int n;
	for (n = 0; n < (sizeof(sgtJtagDevList)/sizeof(sgtJtagDevList[0])); n++)
	{
		if ((idcode & sgtJtagDevList[n].idmask) == sgtJtagDevList[n].idcode)
		{
			return sgtJtagDevList + n;
		}
	}
	return NULL;
}
