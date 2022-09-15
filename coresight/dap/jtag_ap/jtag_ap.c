/*-------------------------------------------------------------------------
// Module:  JTAG-AP
// File:    jtag_ap.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the implementation of th jtag-ap
--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jtag_ap_regs.h"
#include "jtag_ap.h"

//------------------------------------------------------------------------------
// Function    : jtag_ap_init
// Description : Initialise (and reset) a TAP beneath the JTAG-AP
// Arguments   : JTAG-AP Address
// Returns     :
//------------------------------------------------------------------------------

int jtag_ap_init(dap_t * dap, uint32_t apnum)
{
    // Initialise TAP on port (indicated by addr.memaddr) beneath JTAG-AP
    // Reset TAP and leave it in TLR state
    if(jtag_ap_select(dap, apnum) != 0)
        return -1;

    // Perform TAP reset to get TAP into TLR state
    if(dap_ap_write(dap, apnum, JTAG_AP_CSW, (1 << 1)) != 0)
        return -1;

    // Clock 5 cycles with TMS=1
    if(jtag_ap_transition(dap, apnum, 0x1F, 5) != 0)
        return -1;

    // Clear nTRST
    if(dap_ap_write(dap, apnum, JTAG_AP_CSW, 0) != 0)
        return -1;

    // Move to RTI as default resting state, to avoid resetting IR
    // TLR->RTI
    // 0
    return jtag_ap_transition(dap, apnum, 0x0, 1);
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_select
// Description : Select a TAP beneath the JTAG-AP
// Arguments   : JTAG-AP Address
// Returns     :
//------------------------------------------------------------------------------

int jtag_ap_select(dap_t * dap, uint32_t apnum)
{
    uint32_t data;

    if(apnum > 7)
    {
        fprintf(stderr, "JTAG-AP supports a maximum of 8 JTAG devices, requested port %d\n", apnum);
        return -1;
    }

    // Loop until JTAG-AP is idle before updating PORTSEL
    do
    {
        if(dap_ap_read(dap, apnum, JTAG_AP_CSW, &data) != 0)
        {
            return -1;
        }
    } while ((data & 0xF0000000) != 0);

    // Select JTAG Port
    if(dap_ap_write(dap, apnum, JTAG_AP_PORTSEL, (1 << (apnum & 0x7))) != 0)
        return -1;

    // Check that port is connected
    if(dap_ap_read(dap, apnum, JTAG_AP_CSW, &data) != 0)
        return -1;

    if(!(data & (1 << 3)))
    {
        fprintf(stderr, "JTAG-AP reports that port %u is not connected\n", ((uint32_t) (apnum & 0x7)));
        return -1;
    }

    return 0;
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_scan_ir
// Description : Shift data into IR of a TAP beneath the JTAG-AP
// Arguments   : JTAG-AP Address, instruction data, length
// Returns     :
//------------------------------------------------------------------------------
int jtag_ap_scan_ir(dap_t * dap, uint32_t apnum, uint32_t * instr_p, uint32_t instrlen)
{
    return jtag_ap_scan(dap, apnum, instr_p, instrlen, 1); // IR
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_scan_dr
// Description : Shift data into and out of DR of a TAP beneath the JTAG-AP
// Arguments   : JTAG-AP Address, data, length
// Returns     :
//------------------------------------------------------------------------------

int jtag_ap_scan_dr(dap_t * dap, uint32_t apnum, uint32_t * data_p, uint32_t datalen)
{
    return jtag_ap_scan(dap, apnum, data_p, datalen, 0); // DR
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_scan
// Description : Shift data into and out of a TAP beneath the JTAG-AP
// Arguments   : JTAG-AP Address, data, length, ir flag
//------------------------------------------------------------------------------

int jtag_ap_scan(dap_t *dap, uint32_t apnum, uint32_t * p, uint32_t len, uint32_t ir)
{
    // Transition TAP FSM
    if(ir)
    {
        // RTI->SIR
        // 0011
        if(jtag_ap_transition(dap, apnum, 0x3, 4) != 0)
            return -1;
    }
    else
    {
        // RTI->SDR
        // 001
        if(jtag_ap_transition(dap, apnum, 0x1, 3) != 0)
            return -1;
    }

    // Shift Data
    if(jtag_ap_shift(dap, apnum, p, len) != 0)
        return -1;

    // Transition TAP FSM

    // E1IR or E1DR -> RTI
    // 01
    return jtag_ap_transition(dap, apnum, 0x1, 2);
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_transition
// Description : Transition a TAP FSM beneath the JTAG-AP
// Arguments   : JTAG-AP Address, TMS data, TMS length in bits
// Returns     :
//------------------------------------------------------------------------------

int jtag_ap_transition(dap_t * dap, uint32_t apnum, uint32_t tms, uint8_t numbits)
{
    uint32_t tmp;

    if(numbits > 32)
    {
        fprintf(stderr,"JTAG-AP: Attempted to shift more than 32 bits of TMS\n");
        return -1;
    }

    while(numbits > 5)
    {
        // Send blocks of 5 TMS bits
        if(dap_ap_write(dap, apnum, JTAG_AP_BFIFO0, ((0x1 << 5) | (tms & 0x1F))) != 0)
            return -1;
        numbits -= 5;
        tms = tms >> 5;
    }

    if(numbits)
    {
        // Send any remaining TMS bits
        if(dap_ap_write(dap, apnum, JTAG_AP_BFIFO0, ((0x1 << numbits) | (tms & 0x1F))) != 0)
            return -1;
    }

    // Loop while JTAG-AP is shifting out the transition data
    do
    {
        if(dap_ap_read(dap, apnum, JTAG_AP_CSW, &tmp) != 0)
            return -1;
    } while ((tmp & 0xF0000000) != 0);

    return 0;
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_shift_zero
// Description : Shift a series of 0s into the TAP FSM
// Arguments   : JTAG-AP Address, number of bits to shift, final bit
// Returns     :
//------------------------------------------------------------------------------

int jtag_ap_shift_zero(dap_t * dap, uint32_t apnum, uint16_t numbits, uint8_t finalbit)
{
    return jtag_ap_shift_constant(dap, apnum, 0, numbits, finalbit);
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_shift_one
// Description : Shift a series of 1s into the TAP FSM
// Arguments   : JTAG-AP Address, number of bits to shift, final bit
// Returns     :
//------------------------------------------------------------------------------

int jtag_ap_shift_one(dap_t *dap, uint32_t apnum, uint16_t numbits, uint8_t finalbit)
{
    return jtag_ap_shift_constant(dap, apnum, 1, numbits, finalbit);
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_shift_constant
// Description : Shift a series of 1s into the TAP FSM
// Arguments   : JTAG-AP Address, const value,  number of bits to shift, final bit
// Returns     :
//------------------------------------------------------------------------------

int jtag_ap_shift_constant(dap_t *dap, uint32_t apnum, uint8_t value, uint16_t numbits, uint8_t finalbit)
{
    uint8_t opcode;
    uint8_t length;
    uint32_t tmp;

    while(numbits > 0)
    {
        if(numbits > 128)
        {
            // Shift 128, not final
            opcode =  ( (0x8 << 4) | // TDI_TDO Packet Header
                        ((value & 0x1) << 1) | // Hold TDI at value
                        (0x1) ); // Use TDI bit
            length = 0x7F;
            numbits -= 128;
        }
        else
        {
            // Shift less than 128, use final flag
            opcode = ( (0x8 << 4) | // TDI_TDO Packet Header
                        (finalbit & 0x1) << 3 | // TMS on last shift
                        ((value & 0x1) << 1) | // Hold TDI at value
                        (0x1) ); // Use TDI bit
            length = ((numbits -1) & 0x7F);
            numbits = 0;
        }

        // Instruct JTAG-AP to do the shift
        if(dap_ap_write(dap, apnum, JTAG_AP_BFIFO1, (length << 8 | opcode) ) != 0)
            return -1;

        do
        {
            if(dap_ap_read(dap, apnum, JTAG_AP_CSW, &tmp) != 0)
                return -1;
        } while ((tmp & 0xF0000000) != 0);
    }
    return 0;
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_shift
// Description : Shift data through a TAP beneath JTAG-AP
// Arguments   : Address, pointer to data to shift, number of bits to shift
// Returns     :
//------------------------------------------------------------------------------

int jtag_ap_shift(dap_t *dap, uint32_t apnum, uint32_t * p, uint32_t numbits)
{
    //
    // Shift numbits bits of data, LSB first, from p
    // Capture the same number of bits, put them back into p
    //
    uint8_t opcode;
    uint8_t length;
    uint32_t bytes; // Bytes to send in this command

    while(numbits > 0)
    {
        //
        // Create TDI_TDO Header
        //
        if(numbits > 128)
        {
            // Create a 128 bit header
            bytes = 128/8;

            opcode = ( (0x8 << 4) | // TDI_TDO Packet Header
                        (0x0 << 3) | // TMS low on last shift
                        (0x1 << 2) | // Read TDO
                        (0x0 << 1) | // TDI value (unused)
                        (0x0 << 0)); // Don't use TDI bit

            length = ( (0x0 << 7) | // Normal format
                        (128 - 1) ); // 128 bits
        }
        else
        {
            // Create a numbits bit header
            bytes = ( (numbits - 1) / 8 ) + 1;

            opcode = ( (0x8 << 4) | // TDI_TDO Packet Header
                        (0x1 << 3) | // TMS high on last shift
                        (0x1 << 2) | // Read TDO
                        (0x0 << 1) | // TDI value (unused)
                        (0x0 << 0)); // Don't use TDI bit

            length = ( (0x0 << 7) | // Normal format
                        (numbits - 1) ); // numbits bits
        }


        //
        // Write TDI_TDO header to JTAG-AP
        //
        if(dap_ap_write(dap, apnum, JTAG_AP_BFIFO1, (length << 8 | opcode) ) != 0)
            return -1;

        //
        // Write TDI data and read TDO data
        //
        while(bytes > 0)
        {
            if(bytes > 4)
            {
                // 4 bytes
                if(jtag_ap_fifo(dap, apnum, p, numbits) != 0)
                    return -1;
                p++;

                bytes -= 4;
                numbits -= (4*8);
            }
            else
            {
                //
                // Handle the final bits in this packet
                //

                // Ensure unused top bits are masked to zero
                *p &= ~((uint32_t) (0xFFFFFFFFUL << numbits));

                if(jtag_ap_fifo(dap, apnum, p, numbits) != 0)
                    return -1;

                bytes = 0;
                numbits = 0;
            }
        }
    }
    return 0;
}


//------------------------------------------------------------------------------
// Function    : jtag_ap_fifo
// Description : Write data to and from JTAG-AP FIFO registers
// Arguments   : JTAG-AP Address, pointer to data, number of bits
// Returns     :
//------------------------------------------------------------------------------

int jtag_ap_fifo(dap_t *dap, uint32_t apnum, uint32_t * p, uint32_t numbits)
{
    uint32_t tmp;
    uint32_t fiforeg;
    uint32_t numbytes;

    if(numbits > 24)
    {
        fiforeg = JTAG_AP_BFIFO3;
        numbytes = 4;
    }
    else if(numbits > 16)
    {
        fiforeg = JTAG_AP_BFIFO2;
        numbytes = 3;
    }
    else if(numbits > 8)
    {
        fiforeg = JTAG_AP_BFIFO1;
        numbytes = 2;
    }
    else
    {
        fiforeg = JTAG_AP_BFIFO0;
        numbytes = 1;
    }

    // Write the data to the WFIFO
    if(dap_ap_write(dap, apnum, fiforeg, *p) != 0)
        return -1;

    // Poll until sufficient bytes appear in the RFIFO
    do
    {
        if(dap_ap_read(dap, apnum, JTAG_AP_CSW, &tmp) != 0)
            return -1;
    } while (((tmp >> 24) & 0x7) < numbytes);

    // Read the data from the RFIFO
    return dap_ap_read(dap, apnum, fiforeg, p);
}

