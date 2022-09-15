
/*-------------------------------------------------------------------------
// Module:  jtag
// File:    jtag.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
// Description: implementation on the jtag interface (including tap fsm op)
--------------------------------------------------------------------------*/

#ifndef _JTAG_
#define _JTAG_

#include "jtag_drv.h"
#include "jtag_dev_list.h"

#define DEVMAX          32

#define JTAG_RESET   	0
#define JTAG_IDLE	    1
#define JTAG_DRSELECT	2
#define JTAG_DRCAPTURE	3
#define JTAG_DRSHIFT	4
#define JTAG_DREXIT1	5
#define JTAG_DRPAUSE	6
#define JTAG_DREXIT2	7
#define JTAG_DRUPDATE	8
#define JTAG_IRSELECT	9
#define JTAG_IRCAPTURE	10
#define JTAG_IRSHIFT	11
#define JTAG_IREXIT1	12
#define JTAG_IRPAUSE	13
#define JTAG_IREXIT2	14
#define JTAG_IRUPDATE	15


// configuration and state or IR or DR
typedef struct tagJtagRegister
{
	uint8_t  * prebits;
	uint8_t  * postbits;
	uint32_t   precount;
	uint32_t   postcount;
	uint32_t   scanstate;
	uint32_t   idlestate;
} jtag_reg_t;


typedef struct tagJtag
{
	jtag_driver_t        * drv;
	jtag_driver_table_t  * drvTbl;
	jtag_reg_t             ir;
	jtag_reg_t             dr;
	uint32_t               state;
	int                    devcount;
	jtag_info_t            devinfo[DEVMAX];
}jtag_t;


int jtag_init(jtag_t ** jtag);

void jtag_close(jtag_t *jtag);

int jtag_setspeed(jtag_t *jtag, int khz);

// which state to arrive at upon completion of jtag_ir_*()
void jtag_set_ir_idle(jtag_t *jtag, unsigned state);

// which state to arrive at upon completion of jtag_dr_*()
void jtag_set_dr_idle(jtag_t *jtag, unsigned state);

// bits to shift in ahead/behind the args for jtag_xr_*()
void jtag_set_ir_prefix(jtag_t *jtag, unsigned count, const void *bits);
void jtag_set_ir_postfix(jtag_t *jtag, unsigned count, const void *bits);
void jtag_set_dr_prefix(jtag_t *jtag, unsigned count, const void *bits);
void jtag_set_dr_postfix(jtag_t *jtag, unsigned count, const void *bits);

// clear all prefix/postfix patterns and return to default
// idle states
void jtag_clear_state(jtag_t *jtag);

// Move jtag state machine from current state to new state.
// Moving to JTAG_RESET will work even if current state
// is out of sync.
void jtag_goto(jtag_t *jtag, unsigned state);

// Move to IRSHIFT, shift count bits, then move to after_ir state.
void jtag_ir_wr(jtag_t *jtag, unsigned count, const void *wbits);
void jtag_ir_rd(jtag_t *jtag, unsigned count, void *rbits);
void jtag_ir_io(jtag_t *jtag, unsigned count, const void *wbits, void *rbits);

// Move to DRSHIFT, shift count bits, then move to after_dr state;
void jtag_dr_wr(jtag_t *jtag, unsigned count, const void *wbits);
void jtag_dr_rd(jtag_t *jtag, unsigned count, void *rbits);
void jtag_dr_io(jtag_t *jtag, unsigned count, const void *wbits, void *rbits);

// Move to IDLE and stay there for count clocks
void jtag_idle(jtag_t *jtag, unsigned count);

int jtag_commit(jtag_t *jtag);

// reset the bus and probe it
// returns number of devices detected, negative on error
int jtag_enumerate(jtag_t *jtag);

void jtag_print_chain(jtag_t *jtag);

// get information about the nth device on the chain
jtag_info_t *jtag_get_nth_device(jtag_t *jtag, int n);

// configure for communication with a single device
// will setup ir/dr prefix and postfix
int jtag_select_device(jtag_t *jtag, unsigned idcode);

// select by position in scan chain
int jtag_select_device_nth(jtag_t *jtag, int n);

int jtag_select_by_family(jtag_t *jtag, const char *family);

#endif
