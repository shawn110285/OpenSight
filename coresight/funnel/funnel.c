

//-------------------------------------------------------------------------
// Module:  Funnel
// File:    funnel.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: implemetation the interface of funnel
// The functional interfaces of the TPIU are:
//    ATB slave interface, for receiving trace data.
//    APB slave interface, for accessing the TPIU registers.
//    Trace out port, for connecting to the external trace port pins.
//    trigin and flushin event interfaces. These implement synchronizers so that they can be connected to a CTI in a different clock domain.
//--------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../dap/dp/dp.h"
#include "../dap/dp/dp_regs.h"
#include "../dap/apb_ap/apb_ap.h"
#include "funnel_regs.h"

static  dap_t   * sgtFunnelDap = NULL;
static  uint32_t  sguiFunnelApbNum = 1;
static  uint32_t  sguiFunnelBaseAddr = 0;
static  uint32_t  blFunnelInitFlag = 0;

static int32_t funnel_reg_write(uint32_t offset, uint32_t  value)
{
    if(blFunnelInitFlag)
    {
        return apb_ap_write32(sgtFunnelDap, sguiFunnelApbNum, sguiFunnelBaseAddr + offset, value);
    }
    else
    {
        fprintf(stderr, "funnel_reg_write failed, please initialize the funnel module firstly\n");
        return -1;
    }
}


static int32_t funnel_reg_read(uint32_t offset, uint32_t  * value)
{
    if(blFunnelInitFlag)
    {
        return apb_ap_read32(sgtFunnelDap, sguiFunnelApbNum, sguiFunnelBaseAddr + offset, value);
    }
    else
    {
        fprintf(stderr, "funnel_reg_read failed, please initialize the funnel module firstly\n");
        return -1;
    }

}



//==================================================================================
// Function    : funnel_enable
// Description : This is function which configures the funnel to accept trace or not
//==================================================================================
static int32_t funnel_enable(uint32_t port, uint32_t enable)
{
    uint32_t  read_data;
    int32_t   status;
    uint32_t  write_data;

    // Read control bit
    status = funnel_reg_read(FUNNEL_Ctrl_Reg, &read_data);
    if(status == -1)
    {
        return -1;
    }

    // check the port range
    if(port > 7)
    {
        fprintf(stderr, "funnel_enable failed, invalid port, port=%d\n", port);
        return -1;
    }

    if(enable)
    {
        // Set port enable, [7:0] EnS7 Enable slave port [n].
        status = funnel_reg_write(FUNNEL_Ctrl_Reg, (1 << port) | read_data);
        if(status == -1)
        {
            return -1;
        }
    }
    else
    {
        // Set port to disable, [7:0] EnS7 Enable slave port [n].
        status = funnel_reg_write(FUNNEL_Ctrl_Reg, (~(1 << port)) & read_data);
        if(status == -1)
        {
            return -1;
        }
    }
    return 0;
}


int32_t funnel_init(dap_t * dap, uint32_t ap_num, uint32_t baseAddr)
{
    sgtFunnelDap = dap;
    sguiFunnelApbNum = ap_num;
    sguiFunnelBaseAddr = baseAddr;
    //mark the etb module was initialized successfully
    blFunnelInitFlag = 1;
}


//=============================================================================
// Function    : funnel_start
// Description : This is function which configures the funnel to accept trace
// Arguments   : the port to accept trace
//=============================================================================
int32_t funnel_start(uint32_t port)
{
    uint32_t write_data;
    uint32_t status;

    if(blFunnelInitFlag == 0)
    {
        fprintf(stderr, "funnel_start failed, please initialize the module firstly\n");
        return -1;
    }

    fprintf(stderr, "start the port:%d of the funnel\n", port);

    // Unlock Funnel
    write_data = FUNNEL_SW_UNLOCK_VALUE;
    funnel_reg_write(FUNNEL_LOCKACCESS, write_data);
    status= dap_check_clear_stickyerr(sgtFunnelDap);
    if(status == -1)
    {
        return -1;
    }

    // Enable Trace capture
    return funnel_enable(port, 1);
}


//=============================================================================
// Function    : etb_stop
// Description : stop the specified funnel port
//=============================================================================
int32_t funnel_stop(uint32_t port)
{
    uint32_t write_data;
    uint32_t status;

    if(blFunnelInitFlag == 0)
    {
        fprintf(stderr, "funnel_stop failed, please initialize the module firstly\n");
        return -1;
    }

    fprintf(stderr, "stop the port:%d of the funnel\n", port);

    // Unlock Funnel
    write_data = FUNNEL_SW_UNLOCK_VALUE;
    funnel_reg_write(FUNNEL_LOCKACCESS, write_data);
    status= dap_check_clear_stickyerr(sgtFunnelDap);
    if(status == -1)
    {
        return -1;
    }

    // disable trace capture
    return funnel_enable(port, 0);
}
