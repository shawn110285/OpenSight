/*-------------------------------------------------------------------------
// Module:  APB-AP
// File:    apb_ap.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: implemetation the interface for apb-ap/ahb-ap/axi-ap
--------------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../dp/dp.h"
#include "../dp/dp_regs.h"
#include "apb_ap_regs.h"

int apb_ap_write32(dap_t *dap, uint32_t apnum, uint32_t addr, uint32_t val)
{
	if (addr & 3)
	{
		fprintf(stderr, "apb_ap_write32, address is not aligned with words, addr=0x%8x\n", addr);
		return -1;
	}

	dap_ap_write(dap, apnum, APB_AP_CSW, APB_AP_CSW_DBGSWEN | APB_AP_CSW_INCR_NONE | APB_AP_CSW_SIZE32);
	dap_ap_write(dap, apnum, APB_AP_TAR, addr);
	dap_ap_write(dap, apnum, APB_AP_DRW, val);

	return dap_commit(dap);
}


int apb_ap_read32(dap_t *dap, uint32_t apnum, uint32_t addr, uint32_t *val)
{
	if (addr & 3)
	{
		fprintf(stderr, "apb_ap_read32, address is not aligned with words, addr=0x%8x\n", addr);
		return -1;
	}

	dap_ap_write(dap, apnum, APB_AP_TAR, addr);
	if (dap_ap_read(dap, apnum, APB_AP_DRW, val))
	{
		fprintf(stderr, "apb_ap_read32, dap_ap_read return failure\n");
		return -1;
	}

	return 0;
}


int apb_ap_read(dap_t *dap, uint32_t apnum, uint32_t addr, void *data, uint32_t len)
{
	uint64_t scratch[1024];
	uint32_t *x = data;
	uint32_t n;

	if ((addr & 3) || (((uint64_t) data) & 3))
	{
		fprintf(stderr, "apb_ap_read32, address is not aligned with words, addr=0x%8x\n", addr);
		return -1;
	}

	while (len > 0)
	{
		// max transfer is 1K
		// transfer may not cross 1K boundary
		uint32_t xfer = 1024 - (addr & 0x3FF);
		if (xfer > len)
		{
			xfer = len;
		}

		dap_ap_write(dap, apnum, APB_AP_CSW, APB_AP_CSW_DBGSWEN | APB_AP_CSW_INCR_SINGLE | APB_AP_CSW_SIZE32);
		jtag_dp_dr_io(dap, 35, XPACC_WR(APB_AP_TAR, addr), NULL);

		// read txn will be returned on the next txn
		jtag_dp_dr_io(dap, 35, XPACC_RD(APB_AP_DRW), NULL);
		jtag_dp_ir_write_through(dap, DP_IR_APACC); // HACK, timing

		for (n = 0; n < (xfer-4); n += 4)
		{
			jtag_dp_dr_io(dap, 35, XPACC_RD(APB_AP_DRW), &scratch[n/4]);
			jtag_dp_ir_write_through(dap, DP_IR_APACC); // HACK, timing
		}

		// dummy read of TAR to pick up last read value
		jtag_dp_dr_io(dap, 35, XPACC_RD(APB_AP_TAR), &scratch[n/4]);
		if (dap_commit(dap))
		{
			return -1;
		}

		for (n = 0; n < xfer; n += 4)
		{
			switch(XPACC_STATUS(scratch[n/4]))
			{
				case XPACC_WAIT:
				    fprintf(stderr,"wait");
					break;

				case XPACC_OK:
				    fprintf(stderr,"ok");
					break;

				default:
				    fprintf(stderr,"?");
					break;
			}
			*x++ = scratch[n/4] >> 3;
		}
		fprintf(stderr,"\n");
		len -= xfer;
		addr += xfer;
	}
	return 0;
}


int apb_ap_write(dap_t *dap, uint32_t apnum, uint32_t addr, void *data, uint32_t len)
{
	uint32_t *x = data;
	uint32_t n;

	if ((addr & 3) || (((uint64_t) data) & 3))
	{
		fprintf(stderr, "apb_ap_read32, address is not aligned with words, addr=0x%8x\n", addr);
		return -1;
	}

	while (len > 0)
	{
		// max transfer is 1K
		// transfer may not cross 1K boundary
		uint32_t xfer = 1024 - (addr & 0x3FF);
		if (xfer > len)
		{
			xfer = len;
		}

		dap_ap_write(dap, apnum, APB_AP_CSW, APB_AP_CSW_DBGSWEN | APB_AP_CSW_INCR_SINGLE | APB_AP_CSW_SIZE32);
		jtag_dp_dr_io(dap, 35, XPACC_WR(APB_AP_TAR, addr), NULL);
		for (n = 0; n < xfer; n += 4)
		{
			jtag_dp_ir_write_through(dap, DP_IR_APACC); // HACK, timing
			jtag_dp_dr_io(dap, 35, XPACC_WR(APB_AP_DRW, *x++), NULL);
		}

		if (dap_commit(dap))
		{
			return -1;
		}
		len -= xfer;
		addr += xfer;
	}
	return 0;
}



int apb_ap_read_xid(dap_t *dap, uint32_t apnum, uint32_t addr, uint32_t *val)
{
	uint32_t a,b,c,d;
	if (apb_ap_read32(dap, apnum, addr + 0x00, &a))
	    return -1;
	if (apb_ap_read32(dap, apnum, addr + 0x04, &b))
	    return -1;
	if (apb_ap_read32(dap, apnum, addr + 0x08, &c))
	    return -1;
	if (apb_ap_read32(dap, apnum, addr + 0x0C, &d))
	    return -1;

	*val = (a & 0xFF) | ((b & 0xFF) << 8) | ((c & 0xFF) << 16) | ((d & 0xFF) << 24);
	return 0;
}

// 0xFF0 CIDR0 RO 0x0000000D Component ID0 Register
// 0xFE0 PIDR0 RO 0x00000007 Peripheral ID0 Register
// 0xFD0 PIDR4 RO 0x00000004 Peripheral ID4 Register
int apb_ap_read_component_info(dap_t *dap, uint32_t apnum, uint32_t base, uint32_t *cid, uint32_t *pid0, uint32_t *pid4)
{
	if (apb_ap_read_xid(dap, apnum, base + 0xFF0, cid))    // Component ID0 Register
	    return -1;
	if (apb_ap_read_xid(dap, apnum, base + 0xFE0, pid0))   // Peripheral ID0 Register. [7:0]: Contains bits[7:0] of the component identification code.
	    return -1;
	if (apb_ap_read_xid(dap, apnum, base + 0xFD0, pid4))  // Peripheral ID4 Register
	    return -1;
	return 0;
}


