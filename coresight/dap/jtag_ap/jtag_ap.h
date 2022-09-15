
/*-------------------------------------------------------------------------
// Module:  JTAG-AP
// File:    jtag_ap.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the implementation of th jtag-ap
--------------------------------------------------------------------------*/

#ifndef __JTAG_AP_H__
#define __JTAG_AP_H__

#include "../dp/dp.h"

extern int jtag_ap_init(dap_t *dap, uint32_t apnum);
extern int jtag_ap_select(dap_t *dap, uint32_t apnum);
extern int jtag_ap_scan_ir(dap_t *dap, uint32_t apnum, uint32_t * instr_p, uint32_t instrlen);
extern int jtag_ap_scan_dr(dap_t *dap, uint32_t apnum, uint32_t * data_p, uint32_t datalen);
extern int jtag_ap_scan(dap_t *dap, uint32_t apnum, uint32_t * p, uint32_t len, uint32_t ir);
extern int jtag_ap_transition(dap_t *dap, uint32_t apnum, uint32_t tms, uint8_t numbits);
extern int jtag_ap_shift_zero(dap_t *dap, uint32_t apnum, uint16_t numbits, uint8_t finalbit);
extern int jtag_ap_shift_one(dap_t *dap, uint32_t apnum, uint16_t numbits, uint8_t finalbit);
extern int jtag_ap_shift_constant(dap_t *dap, uint32_t apnum, uint8_t value, uint16_t numbits, uint8_t finalbit);
extern int jtag_ap_shift(dap_t *dap, uint32_t apnum, uint32_t * p, uint32_t numbits);
extern int jtag_ap_fifo(dap_t *dap, uint32_t apnum, uint32_t * p, uint32_t numbits);

#endif /* __JTAG_AP_H__ */