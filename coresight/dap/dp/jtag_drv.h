
/*-------------------------------------------------------------------------
// Module:  jtag
// File:    jtag_drv.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the general data struction definition for jtag driver
--------------------------------------------------------------------------*/

#ifndef _JTAG_DRIVER_
#define _JTAG_DRIVER_


#include <libusb-1.0/libusb.h>

// USB bulk xfer fails trying to queue > 8192...
#define CMD_MAX (8*1024)

typedef struct
{
	uint8_t * ptr;
	uint32_t  n;
	uint16_t  op;
	uint16_t  x;
} jtag_operation_link_t;

typedef struct tagJtagDriver
{
	struct libusb_device_handle * udev;

	uint8_t    ep_in;
	uint8_t    ep_out;
	int        speed;
	uint32_t   expected;
	uint32_t   status;
	uint8_t  * next;
	uint32_t   read_count;
	uint32_t   read_size;
	uint8_t  * read_ptr;
	uint8_t    cmd[CMD_MAX];
	uint8_t    read_buffer[512];

	jtag_operation_link_t * nextop;
	jtag_operation_link_t   op[8192];
}jtag_driver_t;

typedef struct tagJtagDriverTable
{
	int (*init)(jtag_driver_t *d);

    // returns actual speed
    // if khz is negative, only query speed, do not set
	int (*setspeed)(jtag_driver_t *drv, int khz);

    // Declare the end of a transaction, block until success or failure.
    // Once this returns, pointers passed via scan_*() may become invalid.
	int (*commit)(jtag_driver_t *d);

    // Shift count TMS=tbits TDI=obit out.
    // If ibits is nonnull, capture the first TDO at offset ioffset in ibits.
	int (*scan_tms)(jtag_driver_t *d, uint32_t obit, uint32_t count, uint8_t *tbits, uint32_t ioffset, uint8_t *ibits);

	// Shift count bits.
	// If obits nonnull, shift out those bits to TDI.
	// If ibits nonnull, capture to those bits from TDO.
	// TMS does not change.
	int (*scan_io)(jtag_driver_t *d, uint32_t count, uint8_t *obits, uint8_t *ibits);

    // Close and release driver.
	int (*close)(jtag_driver_t *d);
} jtag_driver_table_t;


#endif


