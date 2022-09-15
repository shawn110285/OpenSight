//-------------------------------------------------------------------------
// Module:  TPIU
// File:    funnel.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: reg definition for the tpiu
//--------------------------------------------------------------------------

#ifndef __TPIU_REGS_H__
#define __TPIU_REGS_H__


/*=============== Name                Offset   Type    Reset          Description  ===========================*/
#define  TPIU_Supported_Port_Sizes         0x000    // RO   0x00000001     Supported Port Size register
#define  TPIU_Current_port_size            0x004    // RW   0x00000001     Current Port Size register
#define  TPIU_Supported_trigger_modes      0x100    // RO   0x0000011F     Supported Trigger Modes register
#define  TPIU_Trigger_counter_value        0x104    // RW   0x00000000     Trigger Counter Value register
#define  TPIU_Trigger_multiplier           0x108    // RW   0x00000000     Trigger Multiplier register
#define  TPIU_Supported_test_pattern_modes 0x200    // RO   0x0003000F     Supported Test Patterns/Modes register
#define  TPIU_Current_test_pattern_mode    0x204    // RW   0x00000000     Current Test Pattern/Modes register
#define  TPIU_TPRCR                        0x208    // RW   0x00000000     TPIU Test Pattern Repeat Counter Register
#define  TPIU_FFSR                         0x300    // RO   0x00000000     Formatter and Flush Status Register
#define  TPIU_FFCR                         0x304    // RW   0x00000000     Formatter and Flush Control Register
#define  TPIU_FSCR                         0x308    // RW   0x00000040     Formatter Synchronization Counter Register
#define  TPIU_EXTCTL_In_Port               0x400    // RO   0x00000000     TPIU EXCTL In Port register
#define  TPIU_EXTCTL_Out_Port              0x404    // RW   0x00000000     TPIU EXCTL Out Port register
#define  TPIU_ITTRFLINACK                  0xEE4    // WO   0x00000000     Integration Test Trigger In and Flush In Acknowledge register
#define  TPIU_ITTRFLIN                     0xEE8    // RO   0x00000000     Integration Test Trigger In and Flush In register
#define  TPIU_ITATBDATA0                   0xEEC    // RO   0x00000000     Integration Test ATB Data register 0
#define  TPIU_ITATBCTR2                    0xEF0    // WO   0x00000000     Integration Test ATB Control Register 2
#define  TPIU_ITATBCTR1                    0xEF4    // RO   0x00000000     Integration Test ATB Control Register 1
#define  TPIU_ITATBCTR0                    0xEF8    // RO   0x00000000     Integration Test ATB Control Register 0
#define  TPIU_ITCTRL                       0xF00    // RW   0x00000000     Integration Mode Control register
#define  TPIU_CLAIMSET                     0xFA0    // RW   0x0000000F     Claim Tag Set register
#define  TPIU_CLAIMCLR                     0xFA4    // RW   0x00000000     Claim Tag Clear register
#define  TPIU_LAR                          0xFB0    // WO   0x00000000     Lock Access Register
#define  TPIU_LSR                          0xFB4    // RO   0x00000003     Lock Status Register
#define  TPIU_AUTHSTATUS                   0xFB8    // RO   0x00000000     Authentication Status register
#define  TPIU_DEVID                        0xFC8    // RO   0x000000A0     Device Configuration register
#define  TPIU_DEVTYPE                      0xFCC    // RO   0x00000011     Device Type Identifier register
#define  TPIU_PIDR4                        0xFD0    // RO   0x00000004     Peripheral ID4 Register
//                                         0xFD4 - - - Reserved
//                                         0xFD8 - - - Reserved
//                                         0xFDC - - - Reserved
#define  TPIU_PIDR0                        0xFE0    // RO   0x00000012     Peripheral ID0 Register
#define  TPIU_PIDR1                        0xFE4    // RO   0x000000B9     Peripheral ID1 Register
#define  TPIU_PIDR2                        0xFE8    // RO   0x0000004B     Peripheral ID2 Register
#define  TPIU_PIDR3                        0xFEC    // RO   0x00000000     Peripheral ID3 Register
#define  TPIU_CIDR0                        0xFF0    // RO   0x0000000D     Component ID0 Register
#define  TPIU_CIDR1                        0xFF4    // RO   0x00000090     Component ID1 Register
#define  TPIU_CIDR2                        0xFF8    // RO   0x00000005     Component ID2 Register
#define  TPIU_CIDR3                        0xFFC    // RO   0x000000B1     Component ID3 Register


// LAR: Lock Access Register, Controls write access from self-hosted, on-chip accesses.
// The LAR does not affect the accesses that are using the external debugger interface.
// [31:0] Software lock key value. 0xC5ACCE55: Clear the software lock.
//                                 All other write values set the software lock.
#define TPIU_SW_UNLOCK_VALUE               0xC5ACCE55

// FFSR (Formatter and Flush Status Register)
// [31:3]:Reserved,
// [2]:TCPresent,Indicates whether the TRACECTL pin is available for use. 0:TRACECTL pin not present. 1: TRACECTL pin present.
// [1]:FtStopped The formatter has received a stop request signal and all trace data and post-amble is sent.
//     Any additional trace data on the ATB interface is ignored and atreadys goes HIGH. 0:Formatter has not stopped. 1:Formatter has stopped
// [0]: FlInProg Flush in progress.  0: afvalids is LOW.  1:afvalids is HIGH.

// Formatter and Flush Control Register
// [31:14]: Reserved
// [1]: EnFCont, 0: continuous formating disabled, 1: continous formating enabled;
// [0]: EnFTC, 0: formatting disabled, 1: formatting enabled;
#define TPIU_CONTINUOUSMODE                 0x2
#define TPIU_FORMATMODE                     0x1

// Define CS_TRACEMODE for use in TPIU, ETF and ETB FFCR configuration
// Use Continous Mode unless you require compatibility with a legacy Trace Port Analyser
#define TPIU_TRACEMODE (TPIU_CONTINUOUSMODE | TPIU_FORMATMODE)



#endif  /* __TPIU_REGS_H__ */