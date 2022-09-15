
//-------------------------------------------------------------------------
// Module:  DP (Debug Port)
// File:    dp.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the interface of dp(debug port)
//--------------------------------------------------------------------------

#ifndef __DP_H__
#define __DP_H__

#include "jtag.h"

typedef struct tagDap
{
	jtag_t   * jtag;         //the binded jtag
	uint32_t   device_id;
	uint32_t   cached_ir;
}dap_t;


extern dap_t *dap_init(jtag_t *jtag, uint32_t jtag_device_id);
extern void   jtag_dp_dr_io(dap_t *dap, uint32_t bitcount, uint64_t wdata, uint64_t * rdata);
extern void   jtag_dp_ir_write_through(dap_t *dap, uint32_t ir);
extern void   jtag_dp_abort(dap_t *dap);
extern int    dap_commit(dap_t *dap);
extern int    dap_attach(dap_t *dap);
extern int    dap_reset(dap_t *dap);
extern int    dap_probe(dap_t *dap);

// interface to access the debug port registers, DPACC
extern int dap_dp_read(dap_t *dap, uint32_t addr, uint32_t *val);
extern int dap_dp_write(dap_t *dap, uint32_t addr, uint32_t val);

// interface to access the specified ap registers, APACC
extern int dap_ap_read(dap_t *dap, uint32_t apnum, uint32_t addr, uint32_t *val);
extern int dap_ap_write(dap_t *dap, uint32_t apnum, uint32_t addr, uint32_t val);

extern int  dap_check_clear_stickyerr(dap_t *dap);
extern int  dap_clear_stickyerr(dap_t *dap);

#endif /*__DP_H__*/
