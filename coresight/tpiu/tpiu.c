/*-------------------------------------------------------------------------
// Module:  TPIU (Trace Port Interface Unit TPIU)
// File:    tpiu.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: implemetation the interface of tpiu
--------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../dap/dp/dp.h"
#include "../dap/dp/dp_regs.h"
#include "../dap/apb_ap/apb_ap.h"

#include "tpiu_regs.h"


static  dap_t   * sgtTpiuDap = NULL;
static  uint32_t  sguiTpiuApbNum = 1;
static  uint32_t  sguiTpiuBaseAddr = 0;
static  uint32_t  blTpiuInitFlag = 0;

static int32_t tpiu_reg_write(uint32_t offset, uint32_t  value)
{
    if(blTpiuInitFlag)
    {
        return apb_ap_write32(sgtTpiuDap, sguiTpiuApbNum, sguiTpiuBaseAddr + offset, value);
    }
    else
    {
        fprintf(stderr, "etb_reg_write failed, please initialize the etb module firstly\n");
        return -1;
    }
}


static int32_t tpiu_reg_read(uint32_t offset, uint32_t  * value)
{
    if(blTpiuInitFlag)
    {
        return apb_ap_read32(sgtTpiuDap, sguiTpiuApbNum, sguiTpiuBaseAddr + offset, value);
    }
    else
    {
        fprintf(stderr, "etb_reg_read failed, please initialize the etb module firstly\n");
        return -1;
    }
}


//=============================================================================
// Function    : tpiu_enable
// Description : This function enables and disables TPIU trace
//=============================================================================
uint32_t tpiu_enable(uint32_t enable)
{
    uint32_t data;

    if(enable == 1)
    {
        // Set Enable Format
        // CS_CONTINUOUSMODE implies CS_FORMATMODE
        if(TPIU_TRACEMODE & TPIU_CONTINUOUSMODE)
        {
            tpiu_reg_write(TPIU_FFCR, (TPIU_TRACEMODE & 0xFFFFFFFE));
        }
        else
        {
            tpiu_reg_write(TPIU_FFCR, TPIU_TRACEMODE);
        }

        // Read of FFCR (read data is discarded here, but the logged value
        // is used during trace post processing)
        return tpiu_reg_read(TPIU_FFCR, &data);
    }
    else
    {
        // Flush and Stop TPIU
        // Formating enabled, StopFl programmed
        tpiu_reg_write(TPIU_FFCR, TPIU_TRACEMODE | 0x1000);  // [12]: Forces the FIFO to drain off any part-completed packets.

        // Formating enabled, FOnMan programmed
        tpiu_reg_write(TPIU_FFCR, TPIU_TRACEMODE | 0x1000 | 0x40); //[6]: Manual flush is initiated.
        do
        {
          // Check if Ftstopped is set
          tpiu_reg_read(TPIU_FFSR, &data);
        } while((data & 0x2) == 0);  // 0:Formatter has not stopped. 1:Formatter has stopped
        return 0;
    }
}



int32_t tpiu_init(dap_t * dap, uint32_t ap_num, uint32_t baseAddr)
{
    sgtTpiuDap = dap;
    sguiTpiuApbNum = ap_num;
    sguiTpiuBaseAddr = baseAddr;
    //mark the tpiu component as initialized successfully
    blTpiuInitFlag = 1;
}


//=============================================================================
// Function    : tpiu_start
// Description : This function configures the TPIU to accept trace
//=============================================================================
int32_t tpiu_start()
{
    uint32_t status;
    uint32_t size;
    uint32_t bits;

    if(blTpiuInitFlag == 0)
    {
        fprintf(stderr, "tpiu_start failed, please initialize the module firstly\n");
        return -1;
    }

    fprintf(stderr, "start the tpiu \n");

    // Unlock TPIU
    tpiu_reg_write(TPIU_LAR, TPIU_SW_UNLOCK_VALUE);
    status= dap_check_clear_stickyerr(sgtTpiuDap);
    if(status == -1)
    {
        fprintf(stderr, "start the tpiu failed, File=%s, Line=%d\n", __FILE__, __LINE__);
        return -1;
    }

    // Set TPIU port to maximum supported size
    tpiu_reg_read(TPIU_Supported_Port_Sizes, &size);

    // Calculate human-readable version,
    // [31] PORT_SIZE_32 Indicates whether the TPIU supports port size of 32-bit.
    // [30] PORT_SIZE_31 Indicates whether the TPIU supports port size of 31-bit.
    for(bits = 0; size != 0; size = size >> 1)
    {
        bits++;
    }

    size = 1 << (bits - 1);

    // Program the TPIU
    fprintf(stderr, "set the tpiu port Size: 0x%08x (%u bits)\n", size, bits);
    tpiu_reg_write(TPIU_Current_port_size, size);

    // Enable TPIU
    return tpiu_enable(1);
}


//=============================================================================
// Function    : tpiu_stop
// Description : stop the tpiu
//=============================================================================
int32_t tpiu_stop()
{
    uint32_t status;
    uint32_t size;
    uint32_t bits;

    if(blTpiuInitFlag == 0)
    {
        fprintf(stderr, "tpiu_stop failed, please initialize the module firstly\n");
        return -1;
    }

    fprintf(stderr, "stop the tpiu \n");

    // Unlock TPIU
    tpiu_reg_write(TPIU_LAR, TPIU_SW_UNLOCK_VALUE);
    status= dap_check_clear_stickyerr(sgtTpiuDap);
    if(status == -1)
    {
        fprintf(stderr, "start the tpiu failed, File=%s, Line=%d\n", __FILE__, __LINE__);
        return -1;
    }

    // disable TPIU
    return tpiu_enable(0);
}
