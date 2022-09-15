
#//-----------------------------------------------------------------------------
// File:    dapTestEntry.h
// Author:  shawn Liu
// E-mail:  shawn110285@gmail.com
//-----------------------------------------------------------------------------

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//-----------------------------------------------------------------------------

#ifndef __DAP_TEST_ENTRY_H__
#define __DAP_TEST_ENTRY_H__

#include "./dap/dp/dp.h"
#include "./dap/jtag_ap/jtag_ap.h"
#include "./dap/apb_ap/apb_ap.h"

//jtag-dp cmd
extern void cmd_jtag_read_reg(int  argc, char * argv[]);
extern void cmd_jtag_read_idcode(int  argc, char * argv[]);
extern void cmd_jtag_enumerate(int  argc, char * argv[]);

//dap reg (APACC, DPACC, APselect, etc)
extern void cmd_dp_read_reg(int  argc, char * argv[]);
extern void cmd_dp_write_reg(int  argc, char * argv[]);
extern void cmd_ap_read_reg(int  argc, char * argv[]);
extern void cmd_ap_write_reg(int  argc, char * argv[]);

//jtag ap (id, bypass, dtmControl, DMI, etc)
extern void cmd_jtag_ap_read_reg(int  argc, char * argv[]);
extern void cmd_jtag_ap_read_idcode(int  argc, char * argv[]);
extern void cmd_jtag_ap_read_dtmctrl(int  argc, char * argv[]);
extern void cmd_jtag_ap_read_riscv_dm_reg(int  argc, char * argv[]);
extern void cmd_jtag_ap_write_riscv_dm_reg(int  argc, char * argv[]);

//apb ap
extern void cmd_apb_ap_probe(int  argc, char * argv[]);
extern void cmd_apb_ap_dump_rom_table(int  argc, char * argv[]);
extern void cmd_apb_ap_read_reg(int  argc, char * argv[]);
extern void cmd_apb_ap_write_reg(int  argc, char * argv[]);

#endif /* __DAP_TEST_ENTRY_H__ */