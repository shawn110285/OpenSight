//-------------------------------------------------------------------------
// Module:  Funnel
// File:    funnel_regs.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: reg definition for the funnel
//--------------------------------------------------------------------------

#ifndef __FUNNEL_REGS_H__
#define __FUNNEL_REGS_H__


/*=============== Name           Offset            Type  Reset          Description  ===========================*/
#define FUNNEL_Ctrl_Reg          0x000           // RW   0x00000300     Funnel Control register
#define FUNNEL_Priority_Ctrl_Reg 0x004           // RW   0x00000000     Priority Control Register
#define FUNNEL_ITATBDATA0        0xEEC           // RW   0x00000000     Integration Test ATB Data0 register
#define FUNNEL_ITATBCTR2         0xEF0           // RW   0x00000000     Integration Test ATB Control 2 Register
#define FUNNEL_ITATBCTR1         0xEF4           // RW   0x00000000     Integration Test ATB Control 1 Register
#define FUNNEL_ITATBCTR0         0xEF8           // RW   0x00000000     Integration Test ATB Control 0 Register
#define FUNNEL_ITCTRL            0xF00           // RW   0x00000000     Integration Mode Control register
#define FUNNEL_CLAIMSET          0xFA0           // RW   0x0000000F     Claim Tag Set register
#define FUNNEL_CLAIMCLR          0xFA4           // RW   0x00000000     Claim Tag Clear register
#define FUNNEL_LOCKACCESS        0xFB0           // WO   0x00000000     Lock Access Register
#define FUNNEL_LOCKSTATUS        0xFB4           // RO   0x00000003     Lock Status Register
#define FUNNEL_AUTHSTATUS        0xFB8           // RO   0x00000000     Authentication Status register
#define FUNNEL_DEVID             0xFC8           // RO   0x00000038     Device Configuration register
#define FUNNEL_DEVTYPE           0xFCC           // RO   0x00000012     Device Type Identifier register
#define FUNNEL_PIDR4             0xFD0           // RO   0x00000004     Peripheral ID4 Register
//                               0xFD4 - - - Reserved          //
//                               0xFD8 - - - Reserved          //
//                               0xFDC - - - Reserved          //
#define FUNNEL_PIDR0             0xFE0            //RO    x00000008      eripheral ID0 Register
#define FUNNEL_PIDR1             0xFE4            //RO    x000000B9      eripheral ID1 Register
#define FUNNEL_PIDR2             0xFE8            //RO    x0000002B      eripheral ID2 Register
#define FUNNEL_PIDR3             0xFEC            //RO    x00000000      eripheral ID3 Register
#define FUNNEL_CIDR0             0xFF0            //RO    x0000000D      omponent ID0 Register
#define FUNNEL_CIDR1             0xFF4            //RO    x00000090      omponent ID1 Register
#define FUNNEL_CIDR2             0xFF8            //RO    x00000005      omponent ID2 Register
#define FUNNEL_CIDR3             0xFFC            //RO    x000000B1      omponent ID3 Register


// LAR: Lock Access Register, Controls write access from self-hosted, on-chip accesses.
// The LAR does not affect the accesses that are using the external debugger interface.
// [31:0] Software lock key value. 0xC5ACCE55: Clear the software lock.
//                                 All other write values set the software lock.
#define FUNNEL_SW_UNLOCK_VALUE               0xC5ACCE55

#endif  /* __FUNNEL_REGS_H__ */
