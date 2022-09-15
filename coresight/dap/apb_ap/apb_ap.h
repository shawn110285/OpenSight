
/*-------------------------------------------------------------------------
// Module:  apb-ap
// File:    apb_ap.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the function definition on the  apb-ap
--------------------------------------------------------------------------*/

#ifndef __APB_AP_H__
#define __APB_AP_H__

#include "../dp/dp.h"

// single word io -- must be 32bit aligned
extern int apb_ap_write32(dap_t *dap, uint32_t apnum, uint32_t addr, uint32_t val);
extern int apb_ap_read32(dap_t *dap, uint32_t apnum, uint32_t addr, uint32_t *val);

// multi-word io -- must be 32bit aligned, len in bytes, must be 32bit aligned
extern int apb_ap_read(dap_t *dap, uint32_t apnum, uint32_t addr, void *data, uint32_t len);
extern int apb_ap_write(dap_t *dap, uint32_t apnum, uint32_t addr, void *data, uint32_t len);

//print the summary info of the apb-ap
extern int apb_ap_read_xid(dap_t *dap, uint32_t apnum, uint32_t addr, uint32_t *val);
extern int apb_ap_read_component_info(dap_t *dap, uint32_t apnum, uint32_t base, uint32_t *cid, uint32_t *pid0, uint32_t *pid4);

#endif /* __APB_AP_H__ */