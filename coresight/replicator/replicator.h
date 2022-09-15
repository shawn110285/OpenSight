/*-------------------------------------------------------------------------
// Module:  Replicator
// File:    Replicator.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the definition of the replicator interface
--------------------------------------------------------------------------*/

#ifndef __REPLICATOR_API_H__
#define __REPLICATOR_API_H__

#include "../dp/dp.h"

extern int32_t replicator_init(dap_t * dap, uint32_t ap_num, uint32_t baseAddr);
extern int32_t replicator_start(uint32_t port);
extern int32_t replicator_stop(uint32_t port);

#endif /* __REPLICATOR_API_H__ */