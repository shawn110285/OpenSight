
//-------------------------------------------------------------------------
// Module:  apb interconnect
// File:    apb_ic_regs.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: reg definition for the APB interconnect
//--------------------------------------------------------------------------

#ifndef __APB_IC_REGS_H__
#define __APB_IC_REGS_H__

/*===== Name                   Offset    Type         Reset        Description  ===========================*/
#define  APB_IC_ROM_ENTRY_00   0x000     // RO                      ROM Table Entry (0x000 - 0x0FC)
#define  APB_IC_ROM_ENTRY_63   0x0FC
#define  APB_IC_PIDR4          0xFD0     // RO        UNKNOWN       Peripheral ID4 Register
//                             0xFD4 - - - Reserved
//                             0xFD8 - - - Reserved
//                             0xFDC - - - Reserved
#define  APB_IC_PIDR0          0xFE0     // RO                      Peripheral ID0 Register
#define  APB_IC_PIDR1          0xFE4     // RO        UNKNOWN       Peripheral ID1 Register
#define  APB_IC_PIDR2          0xFE8     // RO        UNKNOWN       Peripheral ID2 Register
#define  APB_IC_PIDR3          0xFEC     // RO        0x00000000    Peripheral ID3 Register
#define  APB_IC_CIDR0          0xFF0     // RO        0x0000000D    Component ID0 Register
#define  APB_IC_CIDR1          0xFF4     // RO        0x00000010    Component ID1 Register
#define  APB_IC_CIDR2          0xFF8     // RO        0x00000005    Component ID2 Register
#define  APB_IC_CIDR3          0xFFC     // RO        0x000000B1    Component ID3 Register

// ROM_ENTRY_n: Returns the value of ROM_ENTRY_n, where n is 0-63, The content of this register is determined by the configuration parameters when the rtl is generated
// [31:12] Base_Addr, Base address for master interface 0. Bit[31] is always 0.
// [11:9] Reserved
// [8:4] POWER_DOMAIN_ID, ndicates the power domain ID of the component. This field is only valid when bit[2] is 0b1 . Otherwise this field is 0b1 .
// [3] Reserved
// [2] POWER_DOMAIN_ID_VALID Indicates whether there is a power domain ID specified in the ROM Table entry.
// [1] FORMAT Indicates the ROM table entry format, for example, 32-bit or other formats. 1: ROM table entry is of 32-bit format.
// [0] ENTRY_PRESENT Indicates whether there is a valid ROM entry at this location. 0: Valid ROM table entry not present at this address location.


#endif /* __APB_IC_REGS_H__ */