/*-------------------------------------------------------------------------
// Module:  Replicator
// File:    replicator.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: implemetation the interface of replicator
--------------------------------------------------------------------------*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../dap/dp/dp.h"
#include "../dap/dp/dp_regs.h"
#include "../dap/apb_ap/apb_ap.h"
#include "replicator_regs.h"


static  dap_t   * sgtReplicatorDap = NULL;
static  uint32_t  sguiReplicatorApbNum = 1;
static  uint32_t  sguiReplicatorBaseAddr = 0;
static  uint32_t  blReplicatorInitFlag = 0;


static int32_t replicator_reg_write(uint32_t offset, uint32_t  value)
{
    if(blReplicatorInitFlag)
    {
        return apb_ap_write32(sgtReplicatorDap, sguiReplicatorApbNum, sguiReplicatorBaseAddr + offset, value);
    }
    else
    {
        fprintf(stderr, "replicator_reg_write failed, please initialize the replicator module firstly\n");
        return -1;
    }
}


static int32_t replicator_reg_read(uint32_t offset, uint32_t  * value)
{
    if(blReplicatorInitFlag)
    {
        return apb_ap_read32(sgtReplicatorDap, sguiReplicatorApbNum, sguiReplicatorBaseAddr + offset, value);
    }
    else
    {
        fprintf(stderr, "replicator_reg_read failed, please initialize the replicator module firstly\n");
        return -1;
    }

}


//=============================================================================
// Function    : replicator_enable
// Description : This is function which configures the replicator for sending
//               the trace data with certain ID's through each of its arms
//=============================================================================

static int32_t replicator_enable(uint32_t port, uint32_t enable)
{
    uint32_t write_data;
    int32_t status;

    if(port == 0)
    {
        // Send trace data with all IDs through port0
        // [7] ID1_70_7F Enable or disable ID filtering for IDs 0x70 - 0x7F
        // [6] ID1_60_6F Enable or disable ID filtering for IDs 0x60 - 0x6F .
        if(enable)
        {
            write_data = 0x0;
            replicator_reg_write(REPLICATOR_IDFILTER0, write_data); //0: Transactions with these IDs are passed on to ATB master port 1.
        }
        else
        {
            write_data = 0xff;
            replicator_reg_write(REPLICATOR_IDFILTER0, write_data); //1: Transactions with these IDs are discarded by the replicator.
        }
        return 0;
    }
    else if(port == 1)
    {
        // Send trace data with all IDs through port0
        // [7] ID1_70_7F Enable or disable ID filtering for IDs 0x70 - 0x7F
        // [6] ID1_60_6F Enable or disable ID filtering for IDs 0x60 - 0x6F .
        if(enable)
        {
            write_data = 0x0;
            replicator_reg_write(REPLICATOR_IDFILTER1, write_data); //0: Transactions with these IDs are passed on to ATB master port 1.
        }
        else
        {
            write_data = 0xff;
            replicator_reg_write(REPLICATOR_IDFILTER1, write_data); //1: Transactions with these IDs are discarded by the replicator.
        }
        return 0;
    }
    else
    {
        fprintf(stderr, "replicator_enable, invalid port %d \n", port);
        return -1;
    }
}


int32_t replicator_init(dap_t * dap, uint32_t ap_num, uint32_t baseAddr)
{
    sgtReplicatorDap = dap;
    sguiReplicatorApbNum = ap_num;
    sguiReplicatorBaseAddr = baseAddr;
    //mark the etb module was initialized successfully
    blReplicatorInitFlag = 1;
}


//=============================================================================
// Function    : replicator_start
// Description : This is function which configures the replicator to accept trace
// Arguments   : the port to accept trace
//=============================================================================
int32_t replicator_start(uint32_t port)
{
    uint32_t write_data;
    uint32_t status;

    if(blReplicatorInitFlag == 0)
    {
        fprintf(stderr, "replicator_start failed, please initialize the module firstly\n");
        return -1;
    }

    fprintf(stderr, "start the port:%d of the replicator\n", port);

    // Unlock ETB, so other components can write to ETB
    write_data = REPLICATOR_SW_UNLOCK_VALUE;
    replicator_reg_write(REPLICATOR_LAR, write_data);

    status = dap_check_clear_stickyerr(sgtReplicatorDap);
    if(status == -1)
    {
        return -1;
    }

    // Enable Trace capture
    return replicator_enable(port, 1);
}


//=============================================================================
// Function    : etb_stop
// Description : stop the specified replicator port
//=============================================================================
int32_t replicator_stop(uint32_t port)
{
    uint32_t write_data;
    uint32_t status;

    if(blReplicatorInitFlag == 0)
    {
        fprintf(stderr, "replicator_stop failed, please initialize the module firstly\n");
        return -1;
    }

    fprintf(stderr, "stop the port:%d of the replicator\n", port);

    // Unlock ETB, so other components can write to ETB
    write_data = REPLICATOR_SW_UNLOCK_VALUE;
    replicator_reg_write(REPLICATOR_LAR, write_data);
    status = dap_check_clear_stickyerr(sgtReplicatorDap);
    if(status == -1)
    {
        return -1;
    }

    // disable trace capture
    return replicator_enable(port, 0);
}
