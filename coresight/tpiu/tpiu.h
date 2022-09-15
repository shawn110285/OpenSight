/*-------------------------------------------------------------------------
// Module:  TPIU
// File:    tpiu.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the definition of the tpiu interface
--------------------------------------------------------------------------*/

#ifndef __TPIU_API_H__
#define __TPIU_API_H__

#include <stdint.h>
#include "../dp/dp.h"

extern int32_t tpiu_init(dap_t * dap, uint32_t ap_num, uint32_t baseAddr);
extern int32_t tpiu_start();
extern int32_t tpiu_stop();

#endif /* __TPIU_API_H__ */