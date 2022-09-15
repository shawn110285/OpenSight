/*-------------------------------------------------------------------------
// Module:  Funnel
// File:    funnel.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the definition of the funnel interface
--------------------------------------------------------------------------*/

#ifndef __FUNNEL_API_H__
#define __FUNNEL_API_H__

#include <stdint.h>
#include "../dp/dp.h"

extern int32_t funnel_init(dap_t * dap, uint32_t ap_num, uint32_t baseAddr);
extern int32_t funnel_start(uint32_t port);
extern int32_t funnel_stop(uint32_t port);

#endif /* __FUNNEL_API_H__ */