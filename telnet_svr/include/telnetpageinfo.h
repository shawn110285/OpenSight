//-----------------------------------------------------------------------------
// File:    osalbasic.h
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

#ifndef __TELNET_PAGE_INFO_H__
#define __TELNET_PAGE_INFO_H__

#include "../../osal/include/osalapi.h"

#define MAX_OAMMSG_LEN               (1024 * 16)  /**< packet size */
#define MAX_RESULT_DATA_LEN         MAX_OAMMSG_LEN

#define OAM_STY_LINE_MAX            80
#define OAM_STY_OUTPUT_LINE_MAX     200
#define OAM_STY_PAGE_LINE           25
#define OAM_STY_PROMPT_LEN        60

typedef struct tagDISPLAY_NODE
{
    char    *disp_info;
    struct tagDISPLAY_NODE     *p_next;
}DISPLAY_NODE;

typedef struct tagTelnetPage
{
	uint16_t    fill_len;
    uint16_t    disp_line;
	DISPLAY_NODE    *p_disp_head;
}T_Page_Info;

extern void OutputExecResult(uint8_t* pMsgPara, uint16_t data_len);
extern void telnet_Inner_print(const char *IszPrnMsg,...);

#endif /* __TELNET_PAGE_INFO_H__ */

