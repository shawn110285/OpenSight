
//-------------------------------------------------------------------------
// Module:  DP (Debug Port)
// File:    dp_regs.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the register definition for dp
//--------------------------------------------------------------------------

#ifndef __DP_REGS_H__
#define __DP_REGS_H__

/* ARM dap_t Controller (4bit IR) */
#define DP_IR_SIZE		    4

// JTAG-DP IR summary, shows all implemented registers accessible through the JTAG interface.
// All other Instruction Register (IR) instructions are implemented as BYPASS.

// JTAG-DP register | 4-bit IR instruction value | 8-bit IR instruction value | DR scan width | Description
#define DP_IR_ABORT       0x08   // 0b1000                0b11111000                   35              JTAG-DP Abort Register, ABORT.
#define DP_IR_DPACC       0x0A   // 0b1010                0b11111010                   35              JTAG DPAP Access Registers, DPACC.
#define DP_IR_APACC       0x0B   // 0b1011                0b11111011                   35              JTAG DPAP Access Registers,APACC.
#define DP_IR_IDCODE      0x0E   // 0b1110                0b11111110                   32              JTAG Device ID Code Register, IDCODE.
#define DP_IR_BYPASS      0x0F   // 0b1111                0b11111111                   1               JTAG Bypass Register, BYPASS.


// Debug port register summary (the address offset when IR = DPACC or IR = ABORT)
/*====== Name           Offset            JTAG-DP  SW-DP          Description  ===========================*/
#define  ABORT          0x0            //   Yes      Yes           AP Abort Register. Present in all debug port implementations. It forces a DAP abort, for JTAG-DP, 0x0 when the IR contains ABORT
#define  DPACC_CSW      0x4            //   Yes      Yes           Control/Status Register. JTAG-DP is at address 0x4 when the IR contains DPACC.
#define  DPACC_SELECT   0x8            //   Yes      Yes           AP Select Register. It is at address 0x8 when the IR contains DPACC
#define  DPACC_RDBUFF   0xC            //   Yes      Yes           Read Buffer Register. It is at address 0xC when the IR contains DPACC, and is a RAZ, RAZ/WI register.
#define  IDCODE         0xE            //   Yes      No            ID Code Register. Provides identification information about the JTAG-DP. The IDCODE register is accessed through its own scan chain

#define  DPIDR          0xF            //   No       Yes           Debug Port Identification Register.
#define  DLCR           0xF            //   No       Yes           Data Link Control Register.
#define  TARGETID       0xF            //   No       Yes           Target Identification Register.
#define  DLPIDR         0xF            //   No       Yes           Data Link Protocol Identification Register.
#define  RESEND         0xF            //   No       Yes           Read Resend Register.


/* scan bits definition , scan len=35, including 32 bits data, and 3 bits status */
// xxPACC Read or write, to debug port or access port register.
#define XPACC_STATUS(n)		((n) & 0x3)
#define XPACC_WAIT	   	    0x1
#define XPACC_OK		    0x2

#define XPACC_RD(addr)		    (0x1 | (((addr) >> 1) & 6))                                 //0x1: stands for reading,
#define XPACC_WR(addr,val)		((0x0 | (((addr) >> 1) & 6))) | (((uint64_t)(val)) << 3)    //0x0: stands for writing,

// APSEL
#define DPSEL_APSEL(n)		    (((n) & 0xFF) << 24)  //[31:24]: AP Sel
#define DPSEL_APBANKSEL(a)	    ((a) & 0xF0) // [7:4] Selects the active 4-word register window on the current access port.
#define DPSEL_CTRLSEL		    (1 << 0)     // [3:0] DPBANKSEL, Selects the register that appears at DP register 0x4.
                                             // 0x0 CTRL/STAT, RW;  0x1 DLCR, RW;   0x2 TARGETID, RO;  0x3 DLPIDR, RO.

// Control/Status register (CTRL/STAT)
#define DPCSW_CSYSPWRUPACK	    (1 << 31)   // System powerup acknowledge.
#define DPCSW_CSYSPWRUPREQ	    (1 << 30)   // System powerup request. The reset value is 0.
#define DPCSW_CDBGPWRUPACK	    (1 << 29)   // Debug powerup acknowledge.
#define DPCSW_CDBGPWRUPREQ	    (1 << 28)   // Debug powerup request. The reset value is 0.
#define DPCSW_CDBGRSTACK	    (1 << 27)   // Debug reset acknowledge.
#define DPCSW_CDBGRSTREQ	    (1 << 26)   // Debug reset request. The reset value is 0.

#define DPCSW_TRNCNT(n)		    (((n) & 0x3FF) << 12)  // [23:12], Transaction counter. The reset value is UNPREDICTABLE .
#define DPCSW_MASKLANE(n)	    (((n) & 0xF) << 8)     // Indicates the bytes to be masked in pushed compare and pushed verify operations.
#define DPCSW_WDATAERR		    (1 << 7) // This bit is set to 1 if a Write Data Error occurs. It is set if:
#define DPCSW_READOK		    (1 << 6) // This bit is set to 1 if the response to a previous access port or RDBUFF was OK. It is set to 0 if the response was not OK.
#define DPCSW_STICKYERR		    (1 << 5) // This bit is set to 1 if an error is returned by an access port transaction.
#define DPCSW_STICKYCMP		    (1 << 4) // This bit is set to 1 when a match occurs on a pushed compare or a pushed verify operation.

#define DPCSW_TRNMODE_NORMAL	(0 << 2) // [3:2] This field sets the transfer mode for access port operations.
#define DPCSW_TRNMODE_PUSH_VRFY	(1 << 2)
#define DPCSW_TRNMODE_PUSH_CMP	(2 << 2)

#define DPCSW_STICKYORUN	    (1 << 1)  // If overrun detection is enabled, this bit is set to 1 when an overrun occurs.
#define DPCSW_ORUNDETECT	    (1 << 0)  // This bit is set to 1 to enable overrun detection.

#define CSW_ERRORS              (DPCSW_STICKYERR | DPCSW_STICKYCMP | DPCSW_STICKYORUN)
#define CSW_ENABLES             (DPCSW_CSYSPWRUPREQ | DPCSW_CDBGPWRUPREQ | DPCSW_ORUNDETECT)

#define CTRL_STICKYERR            (1UL <<  5)  // Sticky Error

// some unified ap registers
#define APACC_IDR		        0xFC  // APB-AP/JTAG-AP Identification Register, IDR
#define APACC_CSW		        0x00  // JTAG-AP/APB-AP Control/Status Word register, CSW, 0x00

#endif  // __DP_REGS_H__