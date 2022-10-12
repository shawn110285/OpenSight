
/*-------------------------------------------------------------------------
// Module:  jtag
// File:    jtag_drv_ftdi.c
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: the ftdi based jtag driver
--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jtag_drv.h"
#include "jtag_drv_ftdi.h"

#define TRACE_IO                0
#define TRACE_DIS               0
#define TRACE_USB               0
#define TRACE_TXN               0

// how to process the reply buffer
#define OP_END	        	    0 // done
#define OP_BITS		            1 // copy top n (1-8) bits to ptr
#define OP_BYTES	            2 // copy n (1-65536) bytes to ptr
#define OP_1BIT		            3 // copy bitmask n to bitmask x of ptr

#define FTDI_REQTYPE_OUT	    (LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT)
#define FTDI_CTL_RESET		    0x00
#define FTDI_CTL_SET_BITMODE	0x0B
#define FTDI_CTL_SET_EVENT_CH	0x06
#define FTDI_CTL_SET_ERROR_CH	0x07

#define FTDI_IFC_A              1
#define FTDI_IFC_B              2


static device_info_t devinfo[] =
{
	/*vid     pid     endpoint-in   endpoint-out,  name */
	{ 0x0403, 0x6010, 0x81,         0x02,          "ftdi" },  //k7
	{ 0x0403, 0x6014, 0x81,         0x02,          "ftdi" },  //a7
	{ 0x0000, 0x0000, 0},
};

#define clk_divisor   600               //60M/600 = 100k
static unsigned char mpsse_init[] =
{
	MC_LOOPBACK_DIS,             // loopback off
	MC_TCK_X5,                   // disable clock/5
	MC_SET_CLK_DIV, (clk_divisor-1)&0xff, (clk_divisor-1)>>8, // set divisor, actual clock is 60MHz/(clkdiv), (clkdiv-1) & 0xff, (clkdiv-1) >> 8
	MC_SETB_LOW, 0xe8, 0xeb,     // set low state and direction, in openocd it was defined by ftdi_init_layout
	MC_SETB_HIGH, 0x00, 0x00,    // set high state and direction
};


static void ftdi_dump(char *prefix, void *data, int len)
{
	unsigned char *x = data;
	fprintf(stderr,"%s: (%d)", prefix, len);
	while (len > 0)
	{
		fprintf(stderr," %02x", *x++);
		len--;
	}
	fprintf(stderr,"\n");
}


static uint32_t ftdi_cmd_avail(jtag_driver_t *d)
{
	return CMD_MAX - (d->next - d->cmd);
}

static void ftdi_reset_state(jtag_driver_t *d)
{
	d->status = 0;
	d->next = d->cmd;
	d->nextop = d->op;
	d->expected = 0;
}


static int ftdi_open(jtag_driver_t *d)
{
	struct libusb_device_handle *udev;
	int n;

	/*
	int libusb_init	( libusb_context ** ctx	)
	Initialize libusb. This function must be called before calling any other libusb function.
	If you do not provide an output location for a context pointer, a default context will be created.
	If there was already a default context, it will be reused (and nothing will be initialized/reinitialized).
	*/
	if (libusb_init(NULL) < 0)
	{
		fprintf(stderr, "jtag_init: failed to init libusb \r\n");
		return -1;
	}


	for (n = 0; devinfo[n].name; n++)
	{
		/*
		libusb_device_handle * libusb_open_device_with_vid_pid
		(
			libusb_context * 	ctx,
			uint16_t 	vendor_id,
			uint16_t 	product_id
		)
		Convenience function for finding a device with a particular idVendor/idProduct combination.
		This function is intended for those scenarios where you are using libusb to knock up a quick test application
		it allows you to avoid calling libusb_get_device_list() and worrying about traversing/freeing the list.
		This function has limitations and is hence not intended for use in real applications: if multiple devices have the same IDs it will only give you the first one, etc.

		Parameters
			ctx	        the context to operate on, or NULL for the default context
			vendor_id	the idVendor value to search for
			product_id	the idProduct value to search for

		Returns
			a device handle for the first found device, or NULL on error or if the device could not be found.
		*/

		udev = libusb_open_device_with_vid_pid(NULL, devinfo[n].vid, devinfo[n].pid);
		if (udev == 0)
		{
			fprintf(stderr, "ftdi_open: libusb_open_device_with_vid_pid(vid=0x%x, pid=0x%x) failed, skip it \r\n", devinfo[n].vid, devinfo[n].pid);
	       	continue;
		}

		/*
		int libusb_detach_kernel_driver
		(
			libusb_device_handle * 	dev_handle,
			int                 	interface_number
		)
		Detach a kernel driver from an interface. If successful, you will then be able to claim the interface and perform I/O.
		This functionality is not available on Windows.
		Note that libusb itself also talks to the device through a special kernel driver, if this driver is already attached to the device,
		this call will not detach it and return LIBUSB_ERROR_NOT_FOUND.

		Parameters
		    dev_handle      	a device handle
		    interface_number	the interface to detach the driver from
		*/

		libusb_detach_kernel_driver(udev, 0);

		/*
		int libusb_claim_interface
		(
			libusb_device_handle * 	dev_handle,
			int 	interface_number
		)
		Claim an interface on a given device handle.
		You must claim the interface you wish to use before you can perform I/O on any of its endpoints.
		It is legal to attempt to claim an already-claimed interface, in which case libusb just returns 0 without doing anything.
		If auto_detach_kernel_driver is set to 1 for dev, the kernel driver will be detached if necessary, on failure the detach error is returned.
		Claiming of interfaces is a purely logical operation; it does not cause any requests to be sent over the bus.
		Interface claiming is used to instruct the underlying operating system that your application wishes to take ownership of the interface.
		*/

		if (libusb_claim_interface(udev, 0) < 0)
		{
			fprintf(stderr, "ftdi_open: libusb_claim_interface(vid=0x%x, pid=0x%x) failed, skip it \r\n", devinfo[n].vid, devinfo[n].pid);
			//TODO: close
			continue;
		}
		d->udev = udev;
		d->read_ptr = d->read_buffer;
		d->read_size = 512;
		d->read_count = 0;
		d->ep_in = devinfo[n].ep_in;   // the address of a valid endpoint to communicate with (read in)
		d->ep_out = devinfo[n].ep_out; // the address of a valid endpoint to communicate with (write out)
		fprintf(stderr, "ftdi_open: find device(vid=0x%x, pid=0x%x) \r\n", devinfo[n].vid, devinfo[n].pid);
		return 0;
	}
	fprintf(stderr, "jtag_init: failed to find usb device \r\n");
	return -1;
}


static int ftdi_reset(jtag_driver_t *d)
{
	struct libusb_device_handle * udev = d->udev;

	/*
	int libusb_control_transfer
	(	libusb_device_handle * 	dev_handle,
		uint8_t 	bmRequestType,
		uint8_t 	bRequest,
		uint16_t 	wValue,
		uint16_t 	wIndex,
		unsigned char * 	data,
		uint16_t 	wLength,
		unsigned int 	timeout
	)
	Perform a USB control transfer.
	The direction of the transfer is inferred from the bmRequestType field of the setup packet.
	The wValue, wIndex and wLength fields values should be given in host-endian byte order.

	Parameters
		dev_handle   	a handle for the device to communicate with
		bmRequestType	the request type field for the setup packet
		bRequest	    the request field for the setup packet
		wValue	        the value field for the setup packet
		wIndex      	the index field for the setup packet
		data	        a suitably-sized data buffer for either input or output (depending on direction bits within bmRequestType)
		wLength	        the length field for the setup packet. The data buffer should be at least this size.
		timeout	        timeout (in milliseconds) that this function should wait before giving up due to no response being received. For an unlimited timeout, use value 0.
	*/

	if (libusb_control_transfer(udev, FTDI_REQTYPE_OUT, FTDI_CTL_RESET, 0, FTDI_IFC_A, NULL, 0, 10000) < 0)
	{
		fprintf(stderr, "ftdi: reset failed\n");
		return -1;
	}
	return 0;
}


static int ftdi_mpsse_enable(jtag_driver_t *d)
{
	struct libusb_device_handle *udev = d->udev;
	if (libusb_control_transfer(udev, FTDI_REQTYPE_OUT, FTDI_CTL_SET_BITMODE, 0x0000, FTDI_IFC_A, NULL, 0, 10000) < 0)
	{
		fprintf(stderr, "ftdi: set bitmode failed\n");
		return -1;
	}

	if (libusb_control_transfer(udev, FTDI_REQTYPE_OUT, FTDI_CTL_SET_BITMODE, 0x020b, FTDI_IFC_A, NULL, 0, 10000) < 0)
	{
		fprintf(stderr, "ftdi: set bitmode failed\n");
		return -1;
	}

	if (libusb_control_transfer(udev, FTDI_REQTYPE_OUT, FTDI_CTL_SET_EVENT_CH, 0, FTDI_IFC_A, NULL, 0, 10000) < 0)
	{
		fprintf(stderr, "ftdi: disable event character failed\n");
		return -1;
	}

	return 0;  //?
	if (libusb_control_transfer(udev, FTDI_REQTYPE_OUT, FTDI_CTL_SET_ERROR_CH,0, FTDI_IFC_A, NULL, 0, 10000) < 0)
	{
		fprintf(stderr, "ftdi: disable error character failed\n");
		return -1;
	}
	return 0;
}


static int ftdi_usb_bulk(struct libusb_device_handle * udev, unsigned char ep, void *data, int len, unsigned timeout)
{
	int r, xfer;

#if TRACE_USB
	if (!(ep & 0x80))
		ftdi_dump("xmit", data, len);
#endif

	/*
	int libusb_bulk_transfer
	(
		libusb_device_handle * dev_handle,
		unsigned char 	       endpoint,
		unsigned char        * data,
		int 	               length,
		int                  * transferred,
		unsigned int 	       timeout
	)

	Perform a USB bulk transfer.The direction of the transfer is inferred from the direction bits of the endpoint address.
	For bulk reads, the length field indicates the maximum length of data you are expecting to receive.
	If less data arrives than expected, this function will return that data, so be sure to check the transferred output parameter.
	You should also check the transferred parameter for bulk writes. Not all of the data may have been written.

	Parameters
		dev_handle	  a handle for the device to communicate with
		endpoint	  the address of a valid endpoint to communicate with
		data	      a suitably-sized data buffer for either input or output (depending on endpoint)
		length	      for bulk writes, the number of bytes from data to be sent. for bulk reads, the maximum number of bytes to receive into the data buffer.
		transferred	  output location for the number of bytes actually transferred. Since version 1.0.21 (LIBUSB_API_VERSION >= 0x01000105),
		              it is legal to pass a NULL pointer if you do not wish to receive this information.
		timeout	      timeout (in milliseconds) that this function should wait before giving up due to no response being received. For an unlimited timeout, use value 0.
	*/

	r = libusb_bulk_transfer(udev, ep, data, len, &xfer, timeout);
	if (r < 0)
	{
		fprintf(stderr,"libusb_bulk_transfer error(%d), File=%s, Line=%d \r\n", r, __FILE__, __LINE__);
		return r;
	}
#if TRACE_USB
	if (ep & 0x80)
		ftdi_dump("recv", data, xfer);
#endif
	return xfer;
}



int ftdi_setspeed(jtag_driver_t *d, int frequency)
{
	int base_clock = 60 * 1000000;  //if MC_TCK_X5, then base clock = 60M, otherwise base_clock = 12M
    int divisor = 0;
	uint8_t command_serial[3] = {0};

	if(d == NULL)
	{
		fprintf(stderr,"ftdi set speed, invalid jtag_driver adapter\n");
		return -1;
	}

    divisor = (base_clock / 2 + frequency - 1) / frequency - 1;
    if (divisor > 65535)
        divisor = 65535;

    if(1)
	{
        // MC_SET_CLK_DIV, (clk_divisor-1)&0xff, (clk_divisor-1)>>8, // set divisor, actual clock is 60MHz/(clkdiv), (clkdiv-1) & 0xff, (clkdiv-1) >> 8
		command_serial[0] = MC_SET_CLK_DIV;
		command_serial[1] = (clk_divisor-1)&0xff;
		command_serial[2] = (clk_divisor-1)>>8;

	    if (ftdi_usb_bulk(d->udev, d->ep_out, command_serial, sizeof(command_serial), 1000) != sizeof(command_serial))
		{
			fprintf(stderr,"ftdi set speed, failed\n");
	        return -1;
		}
	}

    frequency = base_clock / 2 / (1 + divisor);
    fprintf(stderr,"ftdi set speed, actually %d Hz", frequency);
    d->speed = frequency;
	return d->speed;
}

/* TODO: handle smaller packet size for lowspeed version of the part */
/* TODO: multi-packet reads */
/* TODO: asynch/background reads */
static int ftdi_read(jtag_driver_t *d, unsigned char *buffer, int count, int timeout)
{
	int xfer;
	while (count > 0)
	{
		if (d->read_count >= count)
		{
			memcpy(buffer, d->read_ptr, count);
			d->read_count -= count;
			d->read_ptr += count;
			return 0;
		}
		if (d->read_count > 0)
		{
			memcpy(buffer, d->read_ptr, d->read_count);
			count -= d->read_count;
			buffer += d->read_count;
			d->read_count = 0;
		}
		xfer = ftdi_usb_bulk(d->udev, d->ep_in, d->read_buffer, d->read_size, 1000);
		if (xfer < 0)
			return -1;
		if (xfer < 2)
			return -1;
		/* discard header */
		d->read_ptr = d->read_buffer + 2;
		d->read_count = xfer - 2;
	}
	return 0;
}


int ftdi_close(jtag_driver_t *d)
{
	if (d->udev)
	{
		//TODO: close
	}
	free(d);
	return 0;
}


int ftdi_init(jtag_driver_t *d)
{
	d->speed = 15000;
	ftdi_reset_state(d);

	if (ftdi_open(d))
		goto fail;

	if (ftdi_reset(d))
		goto fail;

	if (ftdi_mpsse_enable(d))
		goto fail;

	if (ftdi_usb_bulk(d->udev, d->ep_out, mpsse_init, sizeof(mpsse_init), 1000) != sizeof(mpsse_init))
		goto fail;
	return 0;

fail:
	ftdi_close(d);
	return -1;
}


#if TRACE_DIS
static void pbin(uint32_t val, uint32_t bits)
{
	uint32_t n;
	for (n = 0; n < bits; n++)
	{
		fprintf(stderr, "%c", (val & 1) ? '1' : '0');
		val >>= 1;
	}
}


// display mpsse command stream in a (sortof) human readable form
static void ftdi_display_mpsse(uint8_t * data, uint32_t n)
{
	uint32_t x, i;
	while (n > 0)
	{
		fprintf(stderr,"%02x: ", data[0]);
		switch(data[0])
		{
			case 0x6B: // [ox6B, Length Bytes1]: write out 0~Length+1 bits to TMS, started with LSB, and then read from TDO
				fprintf(stderr, "x1 <- TDO, ");
				// fall through
                break;

			case 0x4B: // [ox4B, Length Bytes1]: write out 0~Length+1 bits to TMS, started with LSB
				fprintf(stderr, "TMS <- ");
				pbin(data[2],data[1]+1);
				fprintf(stderr, ", TDI <- ");
				pbin((data[2] & 0x80) ? 0xFF : 0, data[1] + 1);
				fprintf(stderr, "\n");
				data += 3;
				n -= 3;
				break;

			case 0x2A: // [0x2A, Length]: read in length+1 bits via tdo, started with LSB
				fprintf(stderr, "x%d <- TDO\n", data[1] + 1);
				data += 2;
				n -= 2;
				break;

			case 0x28: // [0x28, LengthL, LengthH]:read in (LengthH|LengthL)+1 bytes from the tdo, started with LSB
				x = ((data[2] << 8) | data[1]) + 1;
				fprintf(stderr, "x%d <- TDO\n", (int) x * 8);
				data += 3;
				n -= 3;
				break;

			case 0x1B: // [0x1b, Length, byte]: write out length+1 bits started with LSB via tdi
			case 0x3B: // [0x3B, Length, byte]: read and write in bits
				fprintf(stderr, "TDI <- ");
				pbin(data[2], data[1] + 1);
				if (data[0] == 0x3B)
				{
					fprintf(stderr, ", x%d <- TDO\n", data[1] + 1);
				}
				else
				{
					fprintf(stderr, "\n");
				}
				data += 3;
				n -= 3;
				break;

			case 0x19: // [0x19, LengthL, LengthH, byte1, ..., byte 65535]: write out (LengthH|LengthL) + 1 bytes started with LSB via tdi
			case 0x39: // [0x39, LengthL, LengthH, byte1, ..., bytes65535]:read and write in bytes
				x = ((data[2] << 8) | data[1]) + 1;
				fprintf(stderr, "TDI <- ");
				for (i = 0; i < x; i++) pbin(data[3+i], 8);
				if (data[0] == 0x1B)
				{
					fprintf(stderr, ", x%d <- TDO\n", (int) x);
				}
				else
				{
					fprintf(stderr,"\n");
				}
				data += (3 + x);
				n -= (3 + x);
				break;

			case 0x87: // send immediately
				fprintf(stderr,"FLUSH\n");
				data += 1;
				n -= 1;
				break;

			default:
				fprintf(stderr,"??? 0x%02x\n", data[0]);
				n = 0;
		}
	}
}
#endif  // TRACE_DIS



static uint8_t MASKBITS[9] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

int ftdi_commit(jtag_driver_t *d)
{
	jtag_operation_link_t * op;
	uint8_t               * x;
	unsigned n = d->next - d->cmd;

#if TRACE_TXN
	fprintf(stderr, "jtag_commit: tx(%d) rx(%d)\n", n, (int) d->expected);
#endif

	if (d->status)
	{
		// if we failed during prep, error out immediately
		fprintf(stderr, "jtag_commit: pre-existing errors\n");
		goto fail;
	}

	if (n == 0)
	{
		goto done;
	}

	// always complete with an ioflush
	*d->next = 0x87;
	n++;
	d->nextop->op = OP_END;

#if TRACE_IO
	ftdi_dump("tx", d->cmd, n);
#endif

#if TRACE_DIS
	ftdi_display_mpsse(d->cmd, n);
#endif

	if (ftdi_usb_bulk(d->udev, d->ep_out, d->cmd, n, 1000) != n)
	{
		fprintf(stderr, "jtag_commit: write failed\n");
		goto fail;
	}

	if (ftdi_read(d, d->cmd, d->expected, 1000))
	{
		fprintf(stderr, "jtag_commit: read failed\n");
		goto fail;
	}

#if TRACE_IO
	ftdi_dump("rx", d->cmd, d->expected);
#endif

	for (op = d->op, x = d->cmd; ; op++)
	{
		switch(op->op)
		{
			case OP_END:
				goto done;

			case OP_BITS:
				*op->ptr = ((*x) >> (8 - op->n)) & MASKBITS[op->n];
				x++;
				break;

			case OP_BYTES:
				memcpy(op->ptr, x, op->n);
				x += op->n;
				break;

			case OP_1BIT:
				if (*x & op->n)
				{
					*op->ptr |= op->x;
				}
				else
				{
					*op->ptr &= (~op->x);
				}
				x++;
				break;
		}
	}

done:
	ftdi_reset_state(d);
	return 0;

fail:
	ftdi_reset_state(d);
	return -1;
}


int ftdi_scan_tms(jtag_driver_t *d, uint32_t obit, uint32_t count, uint8_t *tbits, uint32_t ioffset, uint8_t *ibits)
{
	if ((count > 6) || (count == 0))
	{
		fprintf(stderr, "jtag_scan_tms: invalid count %d\n", (int) count);
		return (d->status = -1);
	}

	if (ftdi_cmd_avail(d) < 4)
	{
		if (ftdi_commit(d))
			return (d->status = -1);
	}

	*d->next++ = ibits ? 0x6B : 0x4B;
	*d->next++ = count - 1;
	*d->next++ = ((obit & 1) << 7) | (*tbits);
	if (ibits)
	{
		if (ioffset > 7)
		{
			ibits += (ioffset >> 3);
			ioffset &= 7;
		}
		d->nextop->op = OP_1BIT;
		d->nextop->ptr = ibits;
		d->nextop->n = 1 << (8 - count);
		d->nextop->x = 1 << ioffset;
		d->nextop++;
		d->expected++;
	}
	return -1;
}


int ftdi_scan_io(jtag_driver_t *d, uint32_t count, uint8_t *obits, uint8_t *ibits)
{
	uint32_t n;
	uint32_t bcount = count >> 3;
	uint8_t  bytecmd;
	uint8_t  bitcmd;

	// determine operating mode and ftdi mpsse command byte
	if (obits)
	{
		if (ibits)
		{
			// read-write
			bytecmd = 0x39;
			bitcmd = 0x3B;
		}
		else
		{
			// write-only
			bytecmd = 0x19;
			bitcmd = 0x1B;
		}
	}
	else
	{
		if (ibits)
		{
			// read-only
			bytecmd = 0x28;
			bitcmd = 0x2A;
		}
		else
		{
			// do nothing?!
			fprintf(stderr, "jtag_scan_io: no work?!\n");
			return (d->status = -1);
		}
	}

	// do as many bytemoves as possible first
	// TODO: for exactly 1 byte, bitmove command is more efficient
	while (bcount > 0)
	{
		n = ftdi_cmd_avail(d);

		if (n < 16)
		{
			if (ftdi_commit(d))
				return (d->status = -1);
			continue;
		}
		n -= 4; // leave room for header and io commit
		if (n > bcount)
			n = bcount;

		*d->next++ = bytecmd;
		*d->next++ = (n - 1);
		*d->next++ = (n - 1) >> 8;
		if (obits)
		{
			memcpy(d->next, obits, n);
			d->next += n;
			obits += n;
		}
		if (ibits)
		{
			d->nextop->op = OP_BYTES;
			d->nextop->ptr = ibits;
			d->nextop->n = n;
			d->nextop++;
			ibits += n;
			d->expected += n;
		}
		bcount -= n;
	}

	// do a bitmove for any leftover bits
	count = count & 7;
	if (count == 0)
       	return 0;

	if (ftdi_cmd_avail(d) < 4)
	{
		if (ftdi_commit(d))
			return (d->status = -1);
	}

	*d->next++ = bitcmd;
	*d->next++ = count - 1;
	if (obits)
	{
		*d->next++ = *obits;
	}
	if (ibits)
	{
		d->nextop->op = OP_BITS;
		d->nextop->ptr = ibits;
		d->nextop->n = count;
		d->nextop++;
		d->expected++;
	}
	return 0;
}


