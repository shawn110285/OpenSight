/*-------------------------------------------------------------------------
// Module:  DP (Debug Port)
// File:    dap.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the interface to dp(debug port) and ap (applicaiton) of dap
--------------------------------------------------------------------------*/

/*===========================================================================================
The DAP consists of the following components:
    A DP to manage the connection to an external debugger.
    APs to access on-chip system resources. There can be more than one of each type of AP.
    DAPBUS interconnect to connect the DP to one or more APs.

The APs provide non-invasive access to:
    The programmer's model of CoreSight components. This is normally done through a
    system-wide CoreSight APB bus, through an APB-AP.
    Memory-mapped system components, normally using an AXI-AP or AHB-AP.
    Legacy JTAG-configured debug components, using a JTAG-AP.

	Jtag----dp-jtag ---- DAPBUS Interconnect -------AHB-AP
							|-------APB-AP ---------APB Interconnect---Funnel
							|-------JTAG-AP --- RISC-V CPU       |-----ETF
							|-------AXI-AP                       |-----Replicator
									 							 |-----ETR/ETB
																 |-----TPIU
=============================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dp_regs.h"
#include "dp.h"


//jtag-dp(debug port) manipulating functions
static void jtag_dp_ir_write(dap_t * dap, uint32_t ir)
{
	if (dap->cached_ir != ir)
	{
		dap->cached_ir = ir;
		jtag_ir_wr(dap->jtag, 4, &ir);
	}
}


// force ir write even if redundant
// used for a timing hack
void jtag_dp_ir_write_through(dap_t *dap, uint32_t ir)
{
	dap->cached_ir = ir;
	jtag_ir_wr(dap->jtag, 4, &ir);
}


void jtag_dp_dr_io(dap_t *dap, uint32_t bitcount, uint64_t wdata, uint64_t * rdata)
{
	if (rdata)
	{
		*rdata = 0;
		jtag_dr_io(dap->jtag, bitcount, &wdata, rdata);
	}
	else
	{
		jtag_dr_wr(dap->jtag, bitcount, &wdata);
	}
}


// It forces a DAP abort on SW-DP. It also clears error and sticky flag conditions.
// The AP Abort is always accessible, and returns an OK response if a valid transaction is received.
void jtag_dp_abort(dap_t *dap)
{
	uint32_t x;
	uint64_t u;

	x = DP_IR_ABORT;
	jtag_ir_wr(dap->jtag, 4, &x);  //ir len = 4

	u = 8;
	jtag_dr_wr(dap->jtag, 35, &u);  //coresight dp, scan len = 35 bits
	dap->cached_ir = 0xFFFFFFFF;
}


// queue a DPCSW status query, commit jtag txn
int dap_commit(dap_t *dap)
{
	uint64_t a, b;

	// DPACC: Present in all debug port implementations. It provides control to the debug port,
	// and status information about the debug port.

	//slect the DPACC, access the dp registers
	jtag_dp_ir_write(dap, DP_IR_DPACC);

	//read DPACC_CSW, when the IR contains DPACC
	jtag_dp_dr_io(dap, 35, XPACC_RD(DPACC_CSW), &a);

	//read DPACC_RDBUFF, when the IR contains DPACC
	jtag_dp_dr_io(dap, 35, XPACC_RD(DPACC_RDBUFF), &b);

	dap->cached_ir = 0xFFFFFFFF;
	if (jtag_commit(dap->jtag))
	{
		fprintf(stderr, "dap_commit failed due to the jtag_commit failure, file:%s, line: %d\n", __FILE__, __LINE__);
		return -1;
	}

	if (XPACC_STATUS(a) != XPACC_OK)
	{
		fprintf(stderr, "invalid txn status, file:%s, line: %d\n", __FILE__, __LINE__);
		return -1;
	}

	if (XPACC_STATUS(b) != XPACC_OK)
	{
		fprintf(stderr, "cannot read status, file:%s, line: %d\n", __FILE__, __LINE__);
		return -1;
	}

    // skip the lowest 3 bits, the scan len = 35 bits, the highest 32 bits are payload
	b >>= 3;
	if (b & DPCSW_STICKYORUN)
	{
		fprintf(stderr, "overrun, file:%s, line: %d\n", __FILE__, __LINE__);
		return -1;
	}

	if (b & DPCSW_STICKYERR)
	{
		fprintf(stderr, "error, file:%s, line: %d\n", __FILE__, __LINE__);
		return -1;
	}
	return 0;
}



//Debug port read, it can be used to read the registers(DPACC_CSW, DPACC_SELECT) of dp
int dap_dp_read(dap_t *dap, uint32_t addr, uint32_t *val)
{
	uint64_t u;

	//select the dp channel
	jtag_dp_ir_write(dap, DP_IR_DPACC);
	jtag_dp_dr_io(dap, 35, XPACC_RD(addr), NULL);
	jtag_dp_dr_io(dap, 35, XPACC_RD(DPACC_RDBUFF), &u);

	if (jtag_commit(dap->jtag))
	{
		fprintf(stderr, "dap_dp_read failed, file:%s, line: %d\n", __FILE__, __LINE__);
		return -1;
	}

	if (XPACC_STATUS(u) != XPACC_OK)
	{
		return -1;
	}
	*val = u >> 3;
	return 0;
}


//added two specific function for the DPACC_CSW, DPACC_SELECT registers
int dap_dp_read_csw(dap_t *dap, uint32_t *val)
{
	uint32_t addr = DPACC_CSW;
	return dap_dp_read(dap, addr, val);
}


int dap_dp_read_apsel(dap_t *dap, uint32_t *val)
{
	uint32_t addr = DPACC_SELECT;
	return dap_dp_read(dap, addr, val);
}


//Debug port, it can be used to write the registers(DPACC_CSW, DPACC_SELECT) of dp
int dap_dp_write(dap_t *dap, uint32_t addr, uint32_t val)
{
	jtag_dp_ir_write(dap, DP_IR_DPACC);
	jtag_dp_dr_io(dap, 35, XPACC_WR(addr, val), NULL);
	return jtag_commit(dap->jtag);
}

//added two specific function for the DPACC_CSW, DPACC_SELECT registers
int dap_dp_write_csw(dap_t *dap, uint32_t val)
{
	uint32_t addr = DPACC_CSW;
	return dap_dp_write(dap, addr, val);
}


int dap_dp_write_apsel(dap_t *dap, uint32_t val)
{
	uint32_t addr = DPACC_SELECT;
	return dap_dp_write(dap, addr, val);
}


//read from the reg/memory of the specified ap
int dap_ap_read(dap_t * dap, uint32_t apnum, uint32_t addr, uint32_t *val)
{
	uint64_t u;

	// operation on the DP, select the specified ap
	jtag_dp_ir_write(dap, DP_IR_DPACC);
	jtag_dp_dr_io(dap, 35, XPACC_WR(DPACC_SELECT, DPSEL_APSEL(apnum) | DPSEL_APBANKSEL(addr)), NULL);

	// operation on the ap, write the address
	jtag_dp_ir_write(dap, DP_IR_APACC);
	jtag_dp_dr_io(dap, 35, XPACC_RD(addr), NULL);

	// operation on the dap, read the rd_buff
	jtag_dp_ir_write(dap, DP_IR_DPACC);
	jtag_dp_dr_io(dap, 35, XPACC_RD(DPACC_RDBUFF), &u);

	// TODO: redundant ir wr
	if (dap_commit(dap))
	{
		return -1;
	}
	*val = (u >> 3);
	return 0;
}

//write to the reg/memory of the specified ap
int dap_ap_write(dap_t *dap, uint32_t apnum, uint32_t addr, uint32_t val)
{
	//op on the dp
	jtag_dp_ir_write(dap, DP_IR_DPACC);
	jtag_dp_dr_io(dap, 35, XPACC_WR(DPACC_SELECT, DPSEL_APSEL(apnum) | DPSEL_APBANKSEL(addr)), NULL);

    // op on the ap
	jtag_dp_ir_write(dap, DP_IR_APACC);
	jtag_dp_dr_io(dap, 35, XPACC_WR(addr, val), NULL);
	return 0;
}


dap_t * dap_init(jtag_t *jtag, uint32_t id)
{
	dap_t * dap = malloc(sizeof(dap_t));
	memset(dap, 0, sizeof(dap_t));
	dap->jtag = jtag;
	dap->cached_ir = 0xFFFFFFFF;
	dap->device_id = id;
	return dap;
}


int dap_attach(dap_t *dap)
{
	unsigned n;
	uint32_t x;
	uint8_t  err_flag= 0, sys_powerup_ack = 0, dbg_powerup_ack = 0;

	if (jtag_enumerate(dap->jtag) < 0)
	{
		fprintf(stderr, "dap: jtag enumeration failed\n");
		return -1;
	}

	if (jtag_select_device(dap->jtag, dap->device_id))
	{
		fprintf(stderr, "dap: cannot find device on chain\n");
		return -1;
	}

	// make sure we abort any ongoing transactions first
	jtag_dp_abort(dap);

	// attempt to power up and clear errors
	for (n = 0; n < 10; n++)
	{
		if (dap_dp_write(dap, DPACC_CSW, CSW_ERRORS | CSW_ENABLES))
			continue;

		if (dap_dp_read(dap, DPACC_CSW, &x))
			continue;

		if (x & CSW_ERRORS)
		{
			err_flag = 1;
			continue;
		}
		else
		{
			err_flag = 0;
		}

		if (!(x & DPCSW_CSYSPWRUPACK))
		{
			sys_powerup_ack = 0;
			continue;
		}
		else
		{
			sys_powerup_ack = 1;
		}

		if (!(x & DPCSW_CDBGPWRUPACK))
		{
			dbg_powerup_ack = 0;
			continue;
		}
		else
		{
			dbg_powerup_ack = 1;
		}

		return 0;
	}
	fprintf(stderr,"dap: attach failed, err = %d, sys_powerup_ack = %d, dbg_powerup_ack = %d\n", err_flag, sys_powerup_ack, dbg_powerup_ack);
	return -1;
}


typedef struct tagCoresightComponent
{
    uint32_t     idcode;
	const char * strName;
	const char * strVersion;
}coresight_component_t;

int dap_probe(dap_t *dap)
{
	unsigned n, i;
	uint32_t idcode, ap_csw;

    coresight_component_t coresight_component_list[] =
	{
        { 0x0BA03477, "ARM CoreSight JTAG-DP (8-bit IR)", "r0" },
        { 0x6BA00477, "ARM CoreSight JTAG-DP",            "r6" },
        { 0x6BA02477, "ARM CoreSight SW-DP",              "r6" },

        { 0x5BA00477, "ARM CoreSight JTAG-DP",            "r5" },
        { 0x5BA02477, "ARM CoreSight SW-DP",              "r5" },

        { 0x4BA00477, "ARM CoreSight JTAG-DP",            "r4" },

        { 0x54770002, "ARM AMBA APB2/3 Mem-AP",           "r5" },
        { 0x44770002, "ARM AMBA APB2/3 Mem-AP",           "r4" },
        { 0x34770002, "ARM AMBA APB2/3 Mem-AP",           "r3" },
        { 0x24770002, "ARM AMBA APB2/3 Mem-AP",           "r2" },
        { 0x14770002, "ARM AMBA APB2/3 Mem-AP",           "r1" },
        { 0x04770002, "ARM AMBA APB2/3 Mem-AP",           "r0" },

        { 0x84770001, "ARM AMBA AHB Mem-AP",              "r8" },
        { 0x74770001, "ARM AMBA AHB Mem-AP",              "r7" },
        { 0x64770001, "ARM AMBA AHB Mem-AP",              "r6" },
        { 0x54770001, "ARM AMBA AHB Mem-AP",              "r5" },
        { 0x44770001, "ARM AMBA AHB Mem-AP",              "r4" },
        { 0x34770001, "ARM AMBA AHB Mem-AP",              "r3" },
        { 0x24770001, "ARM AMBA AHB Mem-AP",              "r2" },
        { 0x14770001, "ARM AMBA AHB Mem-AP",              "r1" },
        { 0x04770001, "ARM AMBA AHB Mem-AP",              "r0" },

        { 0x44770004, "ARM AMBA AXI3/4 Mem-AP",           "r4" },
        { 0x34770004, "ARM AMBA AXI3/4 Mem-AP",           "r3" },
        { 0x24770004, "ARM AMBA AXI3/4 Mem-AP",           "r2" },
        { 0x14770004, "ARM AMBA AXI3/4 Mem-AP",           "r1" },
        { 0x04770004, "ARM AMBA AXI3/4 Mem-AP",           "r0" },

        { 0x34760010, "ARM JTAG-AP",                      "r3" },
        { 0x24760010, "ARM JTAG-AP",                      "r2" }
	};

    // loop all the possible ap
	for (n = 0; n < 256; n++)
	{
		// read the ap id
		if (dap_ap_read(dap, n, APACC_IDR, &idcode))
			break;

		if (idcode == 0)
			break;

		// read the ctrl_status_word
		if (dap_ap_read(dap, n, APACC_CSW, &ap_csw) == 0)
		{
			fprintf(stderr,"AP%d, id=0x%0xx CSW=0x%08x\n", n, idcode, ap_csw);
			for(i=0; i<sizeof(coresight_component_list)/sizeof(coresight_component_list[0]); i++)
			{
				if(coresight_component_list[i].idcode == idcode)
				{
					fprintf(stderr,"the device is: %s, version=%s\n", coresight_component_list[i].strName, coresight_component_list[i].strVersion );
				}
			}
		}
	}
	return 0;
}


int dap_reset(dap_t *dap)
{
	uint32_t dp_csw;
	if (dap_dp_write(dap, DPACC_CSW, CSW_ENABLES | DPCSW_CDBGRSTREQ))
	{
		return -1;
	}

	if (dap_dp_read(dap, DPACC_CSW, &dp_csw))
	{
		return -1;
	}

	fprintf(stderr,"reset the dap done, csw = 0x%08x\n", dp_csw);
	return 0;
}




//=============================================================================
// Function    : dap_clear_stickyerr
// Description : Clear DP Sticky Error flag
// Returns     : 0/-1
//=============================================================================
int dap_clear_stickyerr(dap_t * dap)
{
    // Clear STICKYERR bit
    uint32_t dp_csw;

	if(dap_dp_read(dap, DPACC_CSW, &dp_csw) == -1)
		return -1;

	dp_csw |= CTRL_STICKYERR;

	if(dap_dp_write(dap, DPACC_CSW, dp_csw) == -1)
		return -1;

    return 0;
}


//=============================================================================
// Function    : dap_check_clear_stickyerr
// Description : Check DP Sticky Error flag, then clear the bit
// Returns     : 0/-1. -1 returned when the STICKYERR bit is set in DP
//               or if there are errors at trying to read/clear said bit.
//=============================================================================
int dap_check_clear_stickyerr(dap_t * dap)
{
    uint32_t dp_csw;

    if(dap_dp_read(dap, DPACC_CSW, &dp_csw) == -1)
        return -1;

    if(dp_csw & DPCSW_STICKYERR )
    {
		// StickyError was set
		fprintf(stderr, "Accessing DP resulted in an error - the DP STICKYERR bit was set\n");
		dap_clear_stickyerr(dap); // this might fail in theory, diagnose through error messages
		return -1;
    }

    return 0;
}

