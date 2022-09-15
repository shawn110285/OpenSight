/*-------------------------------------------------------------------------
// Module:  jtag
// File:    jtag_dev_list.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: list the known jtag idcode and its vendor info
--------------------------------------------------------------------------*/

#ifndef __JTAG_DEV_LIST_H__
#define __JTAG_DEV_LIST_H__

typedef struct tagJtagInfo
{
	unsigned     idcode;
	unsigned     idmask;
	unsigned     irsize;
	const char * name;
	const char * family;
} jtag_info_t;

extern jtag_info_t *jtag_lookup_device(unsigned idcode);

#endif /* __JTAG_DEV_LIST_H__ */