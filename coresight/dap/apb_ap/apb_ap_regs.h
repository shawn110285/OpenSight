//-------------------------------------------------------------------------
// Module:  apb_ap
// File:    apb_ap_regs.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: reg definition for the APB_AP
//--------------------------------------------------------------------------

#ifndef __APB_AP_REGS_H__
#define __APB_AP_REGS_H__

/*===== Name            Offset        Type  Reset          Description  ===========================*/
#define APB_AP_CSW      0x00          // RW  0x00000002     APB-AP Control/Status Word register
#define APB_AP_TAR      0x04          // RW  0x00           APB-AP Transfer Address Register
//                      0x08 - - Reserved.
#define APB_AP_DRW      0x0C          // RW  0x00           APB-AP Data Read/Write register
#define APB_AP_BD0      0x10          // RW  UNDEFINED      APB-AP Banked Data registers, BD0-BD3, 0x10-0x1C
#define APB_AP_BD1      0x14          // RW  UNDEFINED      APB-AP Banked Data registers, BD0-BD3, 0x10-0x1C
#define APB_AP_BD2      0x18
#define APB_AP_BD3      0x1C
//                      0x20 - 0xF4 - - Reserved, SBZ.
#define APB_AP_BASE     0xF8          // RO 0x80000003      APB-AP Debug Base Address register
#define APB_AP_IDR      0xFC          // RO 0x44770002      APB-AP Identification Register

//APB-AP CSW
#define APB_AP_CSW_DBGSWEN		(1 << 31) // apb-ap csw, Software access enable.  0: Disable software access, 1:Enable software access.
#define APB_AP_CSW_SPIDEN		(1 << 23) // ro
#define APB_AP_CSW_TRBUSY		(1 << 7)  // ro
#define APB_AP_CSW_DEVICEEN		(1 << 6)  // ro

#define APB_AP_CSW_INCR_NONE	(0 << 4) // apb-ap csw, Auto address increment and packing mode on Read or Write data access. 0b00: Auto increment OFF. 0b01: Increment.
#define APB_AP_CSW_INCR_SINGLE	(1 << 4)
#define APB_AP_CSW_INCR_PACKED	(2 << 4) // may not be supported

#define APB_AP_CSW_SIZE8		(0 << 0) // may not be supported
#define APB_AP_CSW_SIZE16		(1 << 0) // may not be supported

#define APB_AP_CSW_SIZE32		(2 << 0) //Size of the access to perform. Fixed at 0b010, 32 bits.  The reset value is 0b010.

// APB-AP Debug Base Address register
// [31:0] RO Debug APB ROM Address Base address of a ROM table. The ROM provides a look-up table for system components.
// Bit[1] is SBO.
// bit[0] to 1 if there are debug components on this bus. For most debug APB systems, this value is 0x80000003 .


#endif /* __APB_AP_REGS_H__ */