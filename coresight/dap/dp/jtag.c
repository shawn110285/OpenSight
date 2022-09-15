
/*-------------------------------------------------------------------------
// Module:  jtag
// File:    jtag.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: implementation on the jtag interface (including tap fsm op)
--------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "jtag_drv.h"
#include "jtag_drv_ftdi.h"
#include "jtag_dev_list.h"
#include "jtag.h"



#define TRACE_STATEMACHINE 0

static uint8_t ONES[1024];



void jtag_clear_state(jtag_t *jtag)
{
	jtag->ir.idlestate = JTAG_IDLE;
	jtag->ir.scanstate = JTAG_IRSHIFT;
	jtag->dr.idlestate = JTAG_IDLE;
	jtag->dr.scanstate = JTAG_DRSHIFT;
	jtag->ir.prebits = 0;
	jtag->ir.precount = 0;
	jtag->ir.postbits = 0;
	jtag->ir.postcount = 0;
	jtag->dr.prebits = 0;
	jtag->dr.precount = 0;
	jtag->dr.postbits = 0;
	jtag->dr.postcount = 0;
}


static jtag_driver_table_t jtag_ftdi_drv_tbl =
{
	.init     = ftdi_init,
	.close    = ftdi_close,
	.setspeed = ftdi_setspeed,
	.commit   = ftdi_commit,
	.scan_tms = ftdi_scan_tms,
	.scan_io  = ftdi_scan_io,
};



int jtag_init(jtag_t ** _jtag)
{
	jtag_driver_t * drv;
	jtag_t        * jtag;

    if ((drv = malloc(sizeof(jtag_driver_t))) == 0)
	{
		return -1;
	}
	memset(drv, 0, sizeof(jtag_driver_t));

	if ((jtag = malloc(sizeof(jtag_t))) == 0)
	{
		return -1;
	}

	memset(jtag, 0, sizeof(jtag_t));
	jtag->drv = drv;
	jtag->drvTbl = &jtag_ftdi_drv_tbl;

	// init the drv
	if( jtag->drvTbl->init(jtag->drv) )
	   return -1;

	jtag_clear_state(jtag);

	*_jtag = jtag;
	memset(ONES, 0xFF, sizeof(ONES));
	return 0;
}


void jtag_close(jtag_t *jtag)
{
	jtag->drvTbl->close(jtag->drv);
	free(jtag);
}


int jtag_setspeed(jtag_t *jtag, int khz)
{
	return jtag->drvTbl->setspeed(jtag->drv, khz);
}


void jtag_set_ir_idle(jtag_t *jtag, unsigned state)
{
	jtag->ir.idlestate = state;
}


void jtag_set_dr_idle(jtag_t *jtag, unsigned state)
{
	jtag->dr.idlestate = state;
}


void jtag_set_ir_prefix(jtag_t *jtag, unsigned count, const void *bits)
{
	jtag->ir.prebits = count ? (uint8_t*) bits : 0;
	jtag->ir.precount = count;
}


void jtag_set_ir_postfix(jtag_t *jtag, unsigned count, const void *bits)
{
	jtag->ir.postbits = count ? (uint8_t*) bits : 0;
	jtag->ir.postcount = count;
}


void jtag_set_dr_prefix(jtag_t *jtag, unsigned count, const void *bits)
{
	jtag->dr.prebits = count ? (uint8_t*) bits : 0;
	jtag->dr.precount = count;
}


void jtag_set_dr_postfix(jtag_t *jtag, unsigned count, const void *bits)
{
	jtag->dr.postbits = count ? (uint8_t*) bits : 0;
	jtag->dr.postcount = count;
}


static uint32_t lastbit(const uint8_t *bits, uint32_t count)
{
	count -= 1;
	return (bits[count >> 3] >> (count & 7)) & 1;
}


static const char * jtag_tap_state_name[16] =
{
	"RESET",
	"IDLE",
	"DRSELECT",
	"DRCAPTURE",
	"DRSHIFT",
	"DREXIT1",
	"DRPAUSE",
	"DREXIT2",
	"DRUPDATE",
	"IRSELECT",
	"IRCAPTURE",
	"IRSHIFT",
	"IREXIT1",
	"IRPAUSE",
	"IREXIT1",
	"IRUPDATE"
};

#define JPATH(x,c) { static uint8_t out = x; *bits = &out; return c; }


static uint32_t jtag_plot(uint32_t from, uint32_t to, uint8_t **bits)
{
#if TRACE_STATEMACHINE
	fprintf(stderr,"jtag_plot: move from %s to %s\n", jtag_tap_state_name[from], jtag_tap_state_name[to]);
#endif
	switch (from)
	{
		case JTAG_RESET:
			if (to == JTAG_IDLE) JPATH(0x00, 1); // 0
			if (to == JTAG_IRSHIFT) JPATH(0x06, 5); // 01100
			if (to == JTAG_DRSHIFT) JPATH(0x02, 4); // 0100
			break;

		case JTAG_IDLE:
			if (to == JTAG_IRSHIFT) JPATH(0x03, 4); // 1100
			if (to == JTAG_DRSHIFT) JPATH(0x01, 3); // 100
			break;

		case JTAG_IRSHIFT:
			if (to == JTAG_IDLE) JPATH(0x03, 3); // 110
			if (to == JTAG_IRPAUSE) JPATH(0x01, 2); // 10
			break;

		case JTAG_DRSHIFT:
			if (to == JTAG_IDLE) JPATH(0x03, 3); // 110
			if (to == JTAG_DRPAUSE) JPATH(0x01, 2); // 10
			break;

		case JTAG_IRPAUSE:
			if (to == JTAG_IDLE) JPATH(0x03, 3); // 110
			if (to == JTAG_IRSHIFT) JPATH(0x01, 2); // 10
			if (to == JTAG_DRSHIFT) JPATH(0x07, 5); // 11100
			break;

		case JTAG_DRPAUSE:
			if (to == JTAG_IDLE) JPATH(0x03, 3); // 110
			if (to == JTAG_DRSHIFT) JPATH(0x01, 2); // 10
			if (to == JTAG_IRSHIFT) JPATH(0x0F, 6); // 111100
			break;
	}
	if (to == JTAG_RESET) JPATH(0x3F, 6); // 111111

	if (from == to) return 0;

	fprintf(stderr,"jtag_plot: cannot move from %s to %s\n", jtag_tap_state_name[from], jtag_tap_state_name[to]);
	return 0;
};


void jtag_goto(jtag_t * jtag, unsigned state)
{
	uint32_t mcount;
	uint8_t *mbits;
	mcount = jtag_plot(jtag->state, state, &mbits);
	if (mcount != 0)
	{
		jtag->drvTbl->scan_tms(jtag->drv, 0, mcount, mbits, 0, 0);
		jtag->state = state;
	}
}


void jtag_idle(jtag_t *jtag, unsigned count)
{
	unsigned zero = 0;
	jtag_goto(jtag, JTAG_IDLE);
	while (count > 0)
	{
		if (count > 6)
		{
			jtag->drvTbl->scan_tms(jtag->drv, 0, 6, (void*) &zero, 0, 0);
			count -= 6;
		}
		else
		{
			jtag->drvTbl->scan_tms(jtag->drv, 0, count, (void*) &zero, 0, 0);
			count = 0;
		}
	}
}


static void jtag_xr_wr(jtag_t *jtag, jtag_reg_t *xr, uint32_t count, uint8_t *wbits)
{
	uint32_t mcount;
	uint8_t *mbits;
	jtag_goto(jtag, xr->scanstate);
	mcount = jtag_plot(xr->scanstate, xr->idlestate, &mbits);

	if (xr->prebits)
	{
		jtag->drvTbl->scan_io(jtag->drv,  xr->precount, xr->prebits, 0);
	}
	if (xr->postbits)
	{
		jtag->drvTbl->scan_io(jtag->drv,  count, wbits, 0);
		jtag->drvTbl->scan_io(jtag->drv,  xr->postcount - 1, xr->postbits, 0);
		jtag->drvTbl->scan_tms(jtag->drv, lastbit(xr->postbits, xr->postcount), mcount, mbits, 0, 0);
	}
	else
	{
		jtag->drvTbl->scan_io(jtag->drv,  count - 1, wbits, 0);
		jtag->drvTbl->scan_tms(jtag->drv, lastbit(wbits, count), mcount, mbits, 0, 0);
	}
	jtag->state = xr->idlestate;
}


static void jtag_xr_rd(jtag_t *jtag, jtag_reg_t *xr, uint32_t count, uint8_t *rbits)
{
	uint32_t mcount;
	uint8_t *mbits;
	jtag_goto(jtag, xr->scanstate);
	mcount = jtag_plot(xr->scanstate, xr->idlestate, &mbits);
	if (xr->prebits)
	{
		jtag->drvTbl->scan_io(jtag->drv,  xr->precount, xr->prebits, 0);
	}
	if (xr->postbits)
	{
		jtag->drvTbl->scan_io(jtag->drv,  count, 0, rbits);
		jtag->drvTbl->scan_io(jtag->drv,  xr->postcount - 1, xr->postbits, 0);
		jtag->drvTbl->scan_tms(jtag->drv, lastbit(xr->postbits, xr->postcount), mcount, mbits, 0, 0);
	}
	else
	{
		jtag->drvTbl->scan_io(jtag->drv,  count - 1, 0, rbits);
		jtag->drvTbl->scan_tms(jtag->drv, 0, mcount, mbits, count - 1, rbits);
	}
	jtag->state = xr->idlestate;
}


static void jtag_xr_io(jtag_t *jtag, jtag_reg_t *xr, uint32_t count, uint8_t *wbits, uint8_t *rbits)
{
	uint32_t mcount;
	uint8_t *mbits;
	jtag_goto(jtag, xr->scanstate);
	mcount = jtag_plot(xr->scanstate, xr->idlestate, &mbits);
	if (xr->prebits)
	{
		jtag->drvTbl->scan_io(jtag->drv,  xr->precount, xr->prebits, 0);
	}
	if (xr->postbits)
	{
		jtag->drvTbl->scan_io(jtag->drv,  count, (void*) wbits, rbits);
		jtag->drvTbl->scan_io(jtag->drv,  xr->postcount - 1, xr->postbits, 0);
		jtag->drvTbl->scan_tms(jtag->drv, lastbit(xr->postbits, xr->postcount), mcount, mbits, 0, 0);
	}
	else
	{
		jtag->drvTbl->scan_io(jtag->drv,  count - 1, (void*) wbits, rbits);
		jtag->drvTbl->scan_tms(jtag->drv, lastbit(wbits, count), mcount, mbits, count - 1, rbits);
	}
	jtag->state = xr->idlestate;
}


void jtag_ir_wr(jtag_t *jtag, unsigned count, const void *wbits)
{
	jtag_xr_wr(jtag, &jtag->ir, count, (void*) wbits);
}


void jtag_ir_rd(jtag_t *jtag, unsigned count, void *rbits)
{
	jtag_xr_rd(jtag, &jtag->ir, count, rbits);
}


void jtag_ir_io(jtag_t *jtag, unsigned count, const void *wbits, void *rbits)
{
	jtag_xr_io(jtag, &jtag->ir, count, (void*) wbits, rbits);
}


void jtag_dr_wr(jtag_t *jtag, unsigned count, const void *wbits)
{
	jtag_xr_wr(jtag, &jtag->dr, count, (void*) wbits);
}


void jtag_dr_rd(jtag_t *jtag, unsigned count, void *rbits)
{
	jtag_xr_rd(jtag, &jtag->dr, count, rbits);
}


void jtag_dr_io(jtag_t *jtag, unsigned count, const void *wbits, void *rbits)
{
	jtag_xr_io(jtag, &jtag->dr, count, (void*) wbits, rbits);
}


int jtag_commit(jtag_t *jtag)
{
	return jtag->drvTbl->commit(jtag->drv);
}



int jtag_enumerate(jtag_t *jtag)
{
	jtag_info_t * info;
	uint32_t data[DEVMAX];
	int n;

	jtag_clear_state(jtag);
	jtag->devcount = 0;

	jtag_goto(jtag, JTAG_RESET);
	jtag_goto(jtag, JTAG_RESET);
	jtag_goto(jtag, JTAG_RESET);
	jtag_dr_io(jtag, DEVMAX * 4 * 8, ONES, data);

	if (jtag_commit(jtag))
	{
		return -1;
	}

	for (n = 0; n < DEVMAX; n++)
	{
		if (data[n] == 0xffffffff)
		{
			if (n == 0)
			{
				fprintf(stderr, "no devices found\n");
				return -1;
			}
			jtag->devcount = n;
			return n;
		}

		if (!(data[n] & 1))
		{
			fprintf(stderr, "device %d has no idcode, the data=%08x\n", n, data[n]);
			return -1;
		}
		else
		{
			fprintf(stderr, "device %d has idcode[%08x]\n", n, data[n]);
		}

		if ((info = jtag_lookup_device(data[n])) == NULL)
		{
			fprintf(stderr, "device %d (id %08x) unknown\n", n, (unsigned) data[n]);
			return -1;
		}
		else
		{
			fprintf(stderr, "device %d (id %08x) is a %s \n", n, data[n], info->name);
		}
		memcpy(jtag->devinfo + n, info, sizeof(jtag_info_t));
	}
	fprintf(stderr, "too many devices\n");
	return -1;
}


void jtag_print_chain(jtag_t *jtag)
{
	int n;
	for (n = 0; n < jtag->devcount; n++)
	{
		jtag_info_t *info = jtag->devinfo + n;
		fprintf(stderr, "device %02d idcode: %08x name: %-16s family: %s\n", n, info->idcode, info->name, info->family);
	}
}


jtag_info_t * jtag_get_nth_device(jtag_t *jtag, int n)
{
	if ((n >= jtag->devcount) || (n < 0))
	{
		return NULL;
	}
	return jtag->devinfo + n;
}


int jtag_select_device_nth(jtag_t *jtag, int num)
{
	uint32_t irpre = 0;
	uint32_t irpost = 0;
	uint32_t drpre = 0;
	uint32_t drpost = 0;
	int n;
	if ((num >= jtag->devcount) || (num < 0))
	{
		return -1;
	}
	for (n = 0; n < jtag->devcount; n++)
	{
		if (n < num)
		{
			irpre += jtag->devinfo[n].irsize;
			drpre += 1;
		}
		else if (n > num)
		{
			irpost += jtag->devinfo[n].irsize;
			drpost += 1;
		}
	}
	jtag_set_ir_prefix(jtag, irpre, ONES);
	jtag_set_ir_postfix(jtag, irpost, ONES);
	jtag_set_dr_prefix(jtag, drpre, ONES);
	jtag_set_dr_postfix(jtag, drpost, ONES);
	return 0;
}



int jtag_select_device(jtag_t *jtag, unsigned idcode)
{
	int n;
	for (n = 0; n < jtag->devcount; n++)
	{
		if (jtag->devinfo[n].idcode == idcode)
		{
			return jtag_select_device_nth(jtag, n);
		}
	}
	return -1;
}



int jtag_select_by_family(jtag_t *jtag, const char *family)
{
	int i, n;
	int count = 0;
	for (i = 0; i < jtag->devcount; i++)
	{
		if (!strcmp(jtag->devinfo[i].family, family))
		{
			count++;
			n = i;
		}
	}

	if (count == 0)
	{
		fprintf(stderr, "jtag: no devices of family '%s' found.\n", family);
		return -1;
	}

	if (count > 1)
	{
		fprintf(stderr, "jtag: multiple devices of family '%s' found.\n", family);
		return -1;
	}
	return jtag_select_device_nth(jtag, n);
}

