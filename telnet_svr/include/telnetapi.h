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

#ifndef __TELNET_API_H__
#define __TELNET_API_H__

#include "../../osal/include/osalapi.h"

#define TELNET_MAX_CMD_WORDS           5
#define TELNET_MAX_CHARS_LENGTH        100
#define TELNET_MAX_DATAIL_LENGTH       1024
#define TELNET_PARA_MAX_NUM            32

#define BUFFER_LENGTH                  512

#define SUCC_AND_NOPARA							(1)
#define SUCC_AND_HAVEPARA						(2)


typedef enum _eParamType
{
    E_TYPE_NODEFINE      = 0,
	E_TYPE_BYTE          = 1,
	E_TYPE_DWORD         = 2,
	E_TYPE_WORD          = 3,
	E_TYPE_STRING        = 4,
	E_TYPE_INT           = 5,
	E_TYPE_BOOL          = 6,
	E_TYPE_STRING_DIGIT  = 7,
	E_TYPE_STRING_LETTER = 8,
	E_TYPE_FLAG          = 9,
	E_TYPE_HEX           = 10,
	E_TYPE_MAX
}E_ParamType;


typedef struct tagCmdPara
{
	char        cPara[TELNET_MAX_CHARS_LENGTH];
	uint16_t    wParaType;
} T_Telnet_Command_Para;


typedef struct tagTltCmd
{
    uint16_t    wCmdWordNum;
	char	    cName[TELNET_MAX_CMD_WORDS][TELNET_MAX_CHARS_LENGTH];		    /* command name */
	char	    cDetail[TELNET_MAX_DATAIL_LENGTH];		    /* help string */
	char        cProgressName[TELNET_MAX_CHARS_LENGTH];
	uint16_t    wParaNum;
	T_Telnet_Command_Para tPara[TELNET_PARA_MAX_NUM];
//	uint16_t    wCmdMode;
	struct tagTltCmd *ptNext;
} T_Telnet_Command;


typedef struct tagCmdExecPara
{
	uint16_t    wParaType;
	uint16_t    wParaLen;
	char        pData[1];
} T_Telnet_Exec_Cmd_Para;

typedef struct tagMSG_TELNET_TO_EXEC
{
	uint16_t    wCmdLen;
	uint16_t    wParaNum;
	uint16_t    wMessageLen;
	char	pData[1];
} T_TELNET_TO_EXEC;



typedef struct tagMSG_EXEC_TO_TELNET
{
	uint16_t    wReturnCode;
	uint16_t    wMessageLen;
	int32_t	bLastPacket;
	char	pData[1];
} T_EXEC_TO_TELNET;


typedef struct tagTelnetExecCmd
{
	char	*name;		    /* command name */
	void 	(*handler)(int argc,char *argv[]);	/* routine which executes command */
} T_Telnet_Exec_Command;

extern void    telnet_print(const char *IszPrnMsg,...);
extern void    telnet_Match_And_Exec_One_Cmd(T_TELNET_TO_EXEC * ptTelnetToExec, uint32_t dwMsgLen, T_Telnet_Exec_Command * ptCmdTab, uint16_t wCount);

#endif /* __TELNET_API_H__ */

