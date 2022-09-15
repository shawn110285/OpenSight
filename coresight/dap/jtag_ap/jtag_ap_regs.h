//-------------------------------------------------------------------------
// Module:  Funnel
// File:    jtag_ap_regs.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: reg definition for the JTAG_AP
//--------------------------------------------------------------------------

#ifndef __JTAG_AP_REGS_H__
#define __JTAG_AP_REGS_H__

/*===== Name            Offset        Type  Reset          Description  ===========================*/
#define JTAG_AP_CSW     0x00          // RW  0x00000000   JTAG-AP Control/Status Word register
#define JTAG_AP_PORTSEL 0x04          // RW  0x00         JTAG-AP Port Select register
#define JTAG_AP_PSTA    0x08          // RW  0x00         JTAG-AP Port Status register
//                      0x0C - - Reserved.
#define JTAG_AP_BFIFO0  0x10          // RW  UNDEFINED     JTAG-AP Byte FIFO registers, BFIFOn, 0x10-0x1C
#define JTAG_AP_BFIFO1  0x14
#define JTAG_AP_BFIFO2  0x18
#define JTAG_AP_BFIFO3  0x1C
//                      0x20 - 0xF8 - - Reserved, SBZ.
#define JTAG_AP_IDR     0xFC         // RO 0x24760010     JTAG-AP Identification Register.

#endif /* __JTAG_AP_REGS_H__ */