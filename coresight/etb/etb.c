/*-------------------------------------------------------------------------
// Module:  etb (Embedded Trace Buffer)
// File:    etb.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: implemetation the interface of ETB
// The ETB stores trace data in an on-chip RAM for later inspection by debug tools.
// The trace data buffer RAM size is configurable.
--------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "../dap/apb_ap/apb_ap.h"
#include "etb_regs.h"

static  dap_t   * sgtEtbDap = NULL;
static  uint32_t  sguiEtbApbNum = 1;
static  uint32_t  sguiEtbBaseAddr = 0;
static  uint32_t  blEtbInitFlag = 0;


static int32_t etb_reg_write(uint32_t offset, uint32_t  value)
{
    if(blEtbInitFlag)
    {
        return apb_ap_write32(sgtEtbDap, sguiEtbApbNum, sguiEtbBaseAddr + offset, value);
    }
    else
    {
        fprintf(stderr, "etb_reg_write failed, please initialize the etb module firstly\n");
        return -1;
    }
}


static int32_t etb_reg_read(uint32_t offset, uint32_t  * value)
{
    if(blEtbInitFlag)
    {
        return apb_ap_read32(sgtEtbDap, sguiEtbApbNum, sguiEtbBaseAddr + offset, value);
    }
    else
    {
        fprintf(stderr, "etb_reg_read failed, please initialize the etb module firstly\n");
        return -1;
    }

}


//=============================================================================
// Function    : etb_enable
// Description : This function enables and disables ETB trace capture or not
//=============================================================================
static int32_t etb_enable(uint32_t enable)
{
    uint32_t status;
    uint32_t data;
    uint32_t write_pointer;
    uint32_t wordcount;

    // enable the ETB
    if(enable == 1)
    {
        // Enable Formatting
        status = etb_reg_write(ETB_FFCR, ETB_TRACEMODE);
        if(status == -1)
        {
            return -1;
        }

        // Read FFCR
        status = etb_reg_read(ETB_FFCR, &data);
        if(status == -1)
        {
            return -1;
        }

        // Enable Trace Capture En
        status = etb_reg_write(ETB_CTL, 0x001);
        if(status == -1)
        {
            return -1;
        }
    }
    else
    {
        // Flush and Stop
        // Formating enabled, StopFl programmed
        etb_reg_write(ETB_FFCR, ETB_TRACEMODE | 0x1000);

        // Formating enabled, FOnMan programmed
        etb_reg_write(ETB_FFCR,ETB_TRACEMODE | 0x1000 | 0x40);
        do
        {
            // Check if Ftstopped is set
            etb_reg_read(ETB_FFSR, &data);
        } while((data & 0x2) == 0);


        // Dump Buffer contents
        fprintf(stderr, "Reading data captured in ETB RAM\n");

        // Read ETB RAM Depth Register, Defines the depth, in words, of the trace RAM.
        etb_reg_read(ETB_RDP, &data);

        // Read Status Register, Indicates the status of the ETB.
        etb_reg_read(ETB_STS, &data);

        // Read RWP Register (ETB RAM Write Pointer register)
        etb_reg_read(ETB_RWP, &write_pointer);

        // If write_pointer is non-zero, there is data in the buffer
        if(write_pointer != 0)
        {
            // Set the read pointer to 0
            etb_reg_write(ETB_RRP, 0x0);

            // Read the read pointer contents
            etb_reg_read(ETB_RRP, &data);

            for(wordcount=0; wordcount < write_pointer; wordcount++)
            {
                // Read data
                etb_reg_read(ETB_RRD, &data);
            }
        }
    }

    return 0;
}


int32_t etb_init(dap_t * dap, uint32_t ap_num, uint32_t baseAddr)
{
    sgtEtbDap = dap;
    sguiEtbApbNum = ap_num;
    sguiEtbBaseAddr = baseAddr;
    //mark the etb module was initialized successfully
    blEtbInitFlag = 1;
}


//=============================================================================
// Function    : etb_start
// Description : This is function which configures the ETB to accept trace
// Arguments   : Component_address information
//=============================================================================
int32_t etb_start()
{
    uint32_t write_data;
    uint32_t status;

    if(blEtbInitFlag == 0)
    {
        fprintf(stderr, "etb_start failed, please initialize the etb module firstly\n");
        return -1;
    }

    fprintf(stderr, "start the etb\n");

    // Unlock ETB, so other components can write to ETB
    write_data = ETB_SW_UNLOCK_VALUE;
    etb_reg_write(ETB_LAR, write_data);
    status = dap_check_clear_stickyerr(sgtEtbDap);
    if(status == -1)
    {
        return -1;
    }

    // Enable Trace capture
    return etb_enable(1);
}


//=============================================================================
// Function    : etb_stop
// Description : stop the etb
//=============================================================================
int32_t etb_stop()
{
    uint32_t write_data;
    uint32_t status;

    if(blEtbInitFlag == 0)
    {
        fprintf(stderr, "etb_stop failed, please initialize the etb module firstly\n");
        return -1;
    }

    fprintf(stderr, "stop the etb\n");

    // Unlock ETB, so other components can write to ETB
    write_data = ETB_SW_UNLOCK_VALUE;
    etb_reg_write(ETB_LAR, write_data);
    status = dap_check_clear_stickyerr(sgtEtbDap);
    if(status == -1)
    {
        return -1;
    }

    // disable trace capture
    return etb_enable(0);
}



