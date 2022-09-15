/*-------------------------------------------------------------------------
// Module:  jtag
// File:    jtag_drv_ftdi.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the ftdi based jtag driver
--------------------------------------------------------------------------*/

#ifndef __JTAG_DRV_FTDI_H__
#define __JTAG_DRV_FTDI_H__

#include "jtag_drv.h"

// FTDI MPSSE Device Info
typedef struct tagDeviceInfo
{
	uint16_t     vid;
	uint16_t     pid;
	uint8_t      ep_in;    // the address of a valid endpoint to communicate with (read in)
	uint8_t      ep_out;   // the address of a valid endpoint to communicate with (write out)
	const char * name;
}device_info_t;


/* MPSSE engine command definitions */
enum mpsse_cmd
{
	/* Mode commands */
	MC_SETB_LOW = 0x80, /* Set Data bits LowByte */
	MC_READB_LOW = 0x81, /* Read Data bits LowByte */
	MC_SETB_HIGH = 0x82, /* Set Data bits HighByte */
	MC_READB_HIGH = 0x83, /* Read data bits HighByte */
	MC_LOOPBACK_EN = 0x84, /* Enable loopback */
	MC_LOOPBACK_DIS = 0x85, /* Disable loopback */
	MC_SET_CLK_DIV = 0x86, /* Set clock divisor */
	MC_FLUSH = 0x87, /* Flush buffer fifos to the PC. */
	MC_WAIT_H = 0x88, /* Wait on GPIOL1 to go high. */
	MC_WAIT_L = 0x89, /* Wait on GPIOL1 to go low. */
	MC_TCK_X5 = 0x8A, /* Disable /5 div, enables 60MHz master clock */
	MC_TCK_D5 = 0x8B, /* Enable /5 div, backward compat to FT2232D */
	MC_EN_3PH_CLK = 0x8C, /* Enable 3 phase clk, DDR I2C */
	MC_DIS_3PH_CLK = 0x8D, /* Disable 3 phase clk */
	MC_CLK_N = 0x8E, /* Clock every bit, used for JTAG */
	MC_CLK_N8 = 0x8F, /* Clock every byte, used for JTAG */
	MC_CLK_TO_H = 0x94, /* Clock until GPIOL1 goes high */
	MC_CLK_TO_L = 0x95, /* Clock until GPIOL1 goes low */
	MC_EN_ADPT_CLK = 0x96, /* Enable adaptive clocking */
	MC_DIS_ADPT_CLK = 0x97, /* Disable adaptive clocking */
	MC_CLK8_TO_H = 0x9C, /* Clock until GPIOL1 goes high, count bytes */
	MC_CLK8_TO_L = 0x9D, /* Clock until GPIOL1 goes low, count bytes */
	MC_TRI = 0x9E, /* Set IO to only drive on 0 and tristate on 1 */
	/* CPU mode commands */
	MC_CPU_RS = 0x90, /* CPUMode read short address */
	MC_CPU_RE = 0x91, /* CPUMode read extended address */
	MC_CPU_WS = 0x92, /* CPUMode write short address */
	MC_CPU_WE = 0x93, /* CPUMode write extended address */
};


extern int ftdi_init(jtag_driver_t *d);
extern int ftdi_close(jtag_driver_t *d);
extern int ftdi_setspeed(jtag_driver_t *d, int khz);
extern int ftdi_commit(jtag_driver_t *d);
extern int ftdi_scan_tms(jtag_driver_t *d, uint32_t obit, uint32_t count, uint8_t *tbits, uint32_t ioffset, uint8_t *ibits);
extern int ftdi_scan_io(jtag_driver_t *d, uint32_t count, uint8_t *obits, uint8_t *ibits);

#endif /* __JTAG_DRV_FTDI_H__ */

