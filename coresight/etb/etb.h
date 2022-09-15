/*-------------------------------------------------------------------------
// Module:  ETB
// File:    etb.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the definition of the etb interface
--------------------------------------------------------------------------*/

#ifndef __ETB_API_H__
#define __ETB_API_H__

#include "../dp/dp.h"

extern int32_t etb_init(dap_t * dap, uint32_t ap_num, uint32_t baseAddr);
extern int32_t etb_start();
extern int32_t etb_stop();

#endif /* __ETB_API_H__ */