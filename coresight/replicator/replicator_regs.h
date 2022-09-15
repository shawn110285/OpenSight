//-------------------------------------------------------------------------
// Module:  Replicator
// File:    replicator_regs.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: reg definition for the Replicator
//--------------------------------------------------------------------------


#ifndef __REPLICATOR_REGS_H__
#define __REPLICATOR_REGS_H__

/*=============== Name           Offset            Type  Reset          Description  ===========================*/
#define  REPLICATOR_IDFILTER0    0x000  // RW 0x00000000 ID filtering for ATB master port 0
#define  REPLICATOR_IDFILTER1    0x004  // RW 0x00000000 ID filtering for ATB master port 1
#define  REPLICATOR_ITATBCTR0    0xEFC  // WO 0x00000000 Integration Mode ATB Control 0 Register
#define  REPLICATOR_ITATBCTR1    0xEF8  // RO 0x00000000 Integration Mode ATB Control 1 Register
#define  REPLICATOR_ITCTRL       0xF00  // RW 0x00000000 Integration Mode Control register
#define  REPLICATOR_CLAIMSET     0xFA0  // RW 0x0000000F Claim Tag Set register
#define  REPLICATOR_CLAIMCLR     0xFA4  // RW 0x00000000 Claim Tag Clear register
#define  REPLICATOR_LAR          0xFB0  // WO 0x00000000 Lock Access Register
#define  REPLICATOR_LSR          0xFB4  // RO 0x00000003 Lock Status Register
#define  REPLICATOR_AUTHSTATUS   0xFB8  // RO 0x00000000 Authentication Status register
#define  REPLICATOR_DEVID        0xFC8  // RO 0x00000002 Device Configuration register
#define  REPLICATOR_DEVTYPE      0xFCC  // RO 0x00000022 Device Type Identifier register
#define  REPLICATOR_PIDR4        0xFD0  // RO 0x00000004 Peripheral ID4 Register
//                               0xFD4 - - - Reserved
//                               0xFD8 - - - Reserved
//                               0xFDC - - - Reserved
#define  REPLICATOR_PIDR0        0xFE0  // RO 0x00000009 Peripheral ID0 Register
#define  REPLICATOR_PIDR1        0xFE4  // RO 0x000000B9 Peripheral ID1 Register
#define  REPLICATOR_PIDR2        0xFE8  // RO 0x0000001B Peripheral ID2 Register
#define  REPLICATOR_PIDR3        0xFEC  // RO 0x00000000 Peripheral ID3 Register
#define  REPLICATOR_CIDR0        0xFF0  // RO 0x0000000D Component ID0 Register
#define  REPLICATOR_CIDR1        0xFF4  // RO 0x00000090 Component ID1 Register
#define  REPLICATOR_CIDR2        0xFF8  // RO 0x00000005 Component ID2 Register
#define  REPLICATOR_CIDR3        0xFFC  // RO 0x000000B1 Component ID3 Register


// IDFILTER0:
// ID filtering for ATB master port 0, Enables the programming of ID filtering for master port 0.
// [7:0]:Enable or disable ID filtering for IDs 0xn0 - 0xnF .
//       0: Transactions with these IDs are passed on to ATB master port 0
//       1: Transactions with these IDs are discarded by the replicator.

// IDFILTER1:
// ID filtering for ATB master port 1, Enables the programming of ID filtering for master port 1.

// LAR: Lock Access Register, Controls write access from self-hosted, on-chip accesses.
// The LAR does not affect the accesses that are using the external debugger interface.
// [31:0] Software lock key value. 0xC5ACCE55: Clear the software lock.
//                                 All other write values set the software lock.
#define REPLICATOR_SW_UNLOCK_VALUE               0xC5ACCE55

#endif /* __REPLICATOR_REGS_H__ */