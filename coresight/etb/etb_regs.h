//-------------------------------------------------------------------------
// Module:  ETB
// File:    etb_regs.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: Register definition for the etb (Embedded Trace Buffer)
//--------------------------------------------------------------------------

#ifndef __ETB_REGS_H__
#define __ETB_REGS_H__


/*=============== Name           Offset            Type  Reset          Description  ===========================*/
#define  ETB_RDP          0x004   // RO 0x00000000 ETB RAM Depth register
#define  ETB_STS          0x00C   // RO 0x00000008 ETB Status register
#define  ETB_RRD          0x010   // RO 0x00000000 ETB RAM Read Data register
#define  ETB_RRP          0x014   // RW 0x00000000 ETB RAM Read Pointer register
#define  ETB_RWP          0x018   // RW 0x00000000 ETB RAM Write Pointer register
#define  ETB_TRG          0x01C   // RW 0x00000000 ETB Trigger Counter register
#define  ETB_CTL          0x020   // RW 0x00000000 ETB Control register
#define  ETB_RWD          0x024   // WO 0x00000000 ETB RAM Write Data register
#define  ETB_FFSR         0x300   // RO 0x00000002 ETB Formatter and Flush Status Register
#define  ETB_FFCR         0x304   // RW 0x00000000 ETB Formatter and Flush Control Register
#define  ETB_ITMISCOP0    0xEE0   // WO 0x00000000 Integration Test Miscellaneous Output register 0
#define  ETB_ITTRFLINACK  0xEE4   // WO 0x00000000 Integration Test Trigger In and Flush In Acknowledge register
#define  ETB_ITTRFLIN     0xEE8   // RO 0x00000000 Integration Test Trigger In and Flush In register
#define  ETB_ITATBDATA0   0xEEC   // RO 0x00000000 Integration Test ATB Data register 0
#define  ETB_ITATBCTR2    0xEF0   // WO 0x00000000 Integration Test ATB Control Register 2
#define  ETB_ITATBCTR1    0xEF4   // RO 0x00000000 Integration Test ATB Control Register 1
#define  ETB_ITATBCTR0    0xEF8   // RO 0x00000000 Integration Test ATB Control Register 0
#define  ETB_ITCTRL       0xF00   // RW 0x00000000 Integration Mode Control register
#define  ETB_CLAIMSET     0xFA0   // RW 0x0000000F Claim Tag Set register
#define  ETB_CLAIMCLR     0xFA4   // RW 0x00000000 Claim Tag Clear register
#define  ETB_LAR          0xFB0   // WO 0x00000000 Lock Access Register
#define  ETB_LSR          0xFB4   // RO 0x00000003 Lock Status Register
#define  ETB_AUTHSTATUS   0xFB8   // RO 0x00000000 Authentication Status register
#define  ETB_DEVID        0xFC8   // RO 0x00000000 Device Configuration register
#define  ETB_DEVTYPE      0xFCC   // RO 0x00000021 Device Type Identifier register
#define  ETB_PIDR4        0xFD0   // RO 0x00000004 Peripheral ID4 Register
//                        0xFD4 - - - Reserved
//                        0xFD8 - - - Reserved
//                        0xFDC - - - Reserved
#define  ETB_PIDR0        0xFE0   //RO 0x00000007 Peripheral ID0 Register
#define  ETB_PIDR1        0xFE4   //RO 0x000000B9 Peripheral ID1 Register
#define  ETB_PIDR2        0xFE8   //RO 0x0000003B Peripheral ID2 Register
#define  ETB_PIDR3        0xFEC   //RO 0x00000000 Peripheral ID3 Register
#define  ETB_CIDR0        0xFF0   //RO 0x0000000D Component ID0 Register
#define  ETB_CIDR1        0xFF4   //RO 0x00000090 Component ID1 Register
#define  ETB_CIDR2        0xFF8   //RO 0x00000005 Component ID2 Register
#define  ETB_CIDR3        0xFFC   //RO 0x000000B1 Component ID3 Register


// LAR: Lock Access Register, Controls write access from self-hosted, on-chip accesses.
// The LAR does not affect the accesses that are using the external debugger interface.
// [31:0] Software lock key value. 0xC5ACCE55: Clear the software lock.
//                                 All other write values set the software lock.
#define ETB_SW_UNLOCK_VALUE               0xC5ACCE55


// ETB Control register, Controls trace capture by the ETB, [0] TraceCaptEn, 0: Disable trace capture. 1: Enable trace capture


// ETB Formatter and Flush Status Register, Indicates the implemented trigger counter multipliers and other supported features of the trigger system.


// ETB Formatter and Flush Control Register,
// Selects the formatter mode, and controls the generation of stop, trigger, and flush events.
// [1] FtStopped Formatter stopped. The formatter has received a stop request signal and all trace data and post-amble is sent.
// Any additional trace data on the ATB interface is ignored and atreadys goes HIGH.
// 0: Formatter is not stopped. 1:Formatter is stopped.
// [0] FlInProg Flush In Progress. This is an indication of the current state of afvalids.
// 0:afvalids is LOW. 1: afvalids is HIGH.


// Formatter and Flush Control Register
// [31:14]: Reserved
// [1]: EnFCont, 0: continuous formating disabled, 1: continous formating enabled;
// [0]: EnFTC, 0: formatting disabled, 1: formatting enabled;
#define ETB_CONTINUOUSMODE                 0x2
#define ETB_FORMATMODE                     0x1

// Define CS_TRACEMODE for use in TPIU, ETF and ETB FFCR configuration
// Use Continous Mode unless you require compatibility with a legacy Trace Port Analyser
#define ETB_TRACEMODE (ETB_CONTINUOUSMODE | ETB_FORMATMODE)


#endif  /* __ETB_REGS_H__ */