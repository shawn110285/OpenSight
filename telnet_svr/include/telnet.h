
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

#ifndef   __TELNET_H__
#define   __TELNET_H__

#include "../../osal/include/osalapi.h"
#include "../../osal/include/osalmsgdef.h"
#include "../../osal/include/osalcfg.h"
#include "telnetpageinfo.h"

/* telnet input states */

#define TS_DATA		0
#define TS_IAC		1
#define TS_CR		2
#define TS_BEGINNEG	3
#define TS_ENDNEG	4
#define TS_WILL		5
#define TS_WONT		6
#define TS_DO		7
#define TS_DONT		8


#define   NUL        (uint8_t)0      /*  NULL */
#define   LF         (uint8_t)10     /*  Line Feed(LF) */
#define   CR         (uint8_t)13     /*  Carriage Return */
#define   BEL        (uint8_t)7      /*  BELL */
#define   BS         (uint8_t)8      /*Back Space*/
#define   HT         (uint8_t)9      /*Horizontal Tab*/
#define   VT         (uint8_t)11     /*Vertical Tab */
#define   FF         (uint8_t)12     /*Form Feed */

#define   EOR        (uint8_t)239
#define   SE         (uint8_t)240
#define   NOP        (uint8_t)241
#define   DM         (uint8_t)242    /* data mark */
#define   BREAK      (uint8_t)243
#define   IP         (uint8_t)244    /*Interrupt Process */
#define   AO         (uint8_t)245    /*Abort Output*/
#define   AYT        (uint8_t)246    /*Are You There*/
#define   EC         (uint8_t)247    /*Erase Character*/
#define   EL         (uint8_t)248    /* Erase Line */
#define   GA         (uint8_t)249    /* Go ahead   */
#define   SB         (uint8_t)250
#define   WILL       (uint8_t)251
#define   WONT       (uint8_t)252
#define   DO         (uint8_t)253
#define   DONT       (uint8_t)254
#define   IAC        (uint8_t)255

#define  TELOPT_BINARY   0
#define  TELOPT_ECHO     1
#define  TELOPT_RCP      2
#define  TELOPT_SGA      3
#define  TELOPT_NAMS     4
#define  TELOPT_STATUS   5
#define  TELOPT_TM       6
#define  TELOPT_RCTE     7
#define  TELOPT_NAOL     8
#define  TELOPT_NAOP     9
#define  TELOPT_NAOCRD   10
#define  TELOPT_NAOHTS   11
#define  TELOPT_NAOHTD   12
#define  TELOPT_NAOFFD   13
#define  TELOPT_NAOVTS   14
#define  TELOPT_NAOVTD   15
#define  TELOPT_NAOLFD   16
#define  TELOPT_XASCII   17
#define  TELOPT_LOGOUT   18
#define  TELOPT_BM       19
#define  TELOPT_DET      20
#define  TELOPT_SUPDUP   21
#define  TELOPT_SUPDUPOUTPUT   22
#define  TELOPT_SNDLOC   23
#define  TELOPT_TTYPE    24
#define  TELOPT_EOR      25
#define  TELOPT_WINCH    31

#define  TELOPT_NAWS          31        /* window size */
#define  TELOPT_TSPEED        32        /* terminal speed */
#define  TELOPT_LFLOW         33        /* remote flow control */
#define  TELOPT_LINEMODE      34        /* Linemode option */
#define  TELOPT_XDISPLOC      35        /* X Display Location */
#define  TELOPT_ENVIRON       36        /* Environment opt for Port ID */
#define  TELOPT_AUTHENTICATION 37       /* authentication */
#define  TELOPT_ENCRYPT       38        /* authentication */
#define  TELOPT_NEWENV        39        /* Environment opt for Port ID */
#define  TELOPT_STARTTLS      46        /* Transport Layer Security */

/* Inofficial, mud specific telnet options */
#define  TELOPT_COMPRESS      85        /* Mud Compression Protocol, v.1 */
#define  TELOPT_COMPRESS2     86        /* Mud Compression Protocol, v.2 */
#define  TELOPT_MSP           90        /* Mud Sound Protocol */
#define  TELOPT_MXP           91        /* Mud Extension Protocol */
#define  TELOPT_EXOPL         255        /* extended-options-list */
#define  NTELOPTS             256        /* was: (1+TELOPT_NEWENV) */

#define  TELQUAL_IS         0
#define  TELQUAL_SEND       1

#define  SYNCH              242


#define CONTROL_TAB         (uint8_t)9
#define CONTROL_DEL         (uint8_t)127
#define CONTROL_BKSPACE     (uint8_t)8
#define CONTROL_RETURN      (uint8_t)13
#define CONTROL_NEWLINE     (uint8_t)10

#define CONTROL_UP          (uint8_t)65
#define CONTROL_DOWN        (uint8_t)66
#define CONTROL_LEFT        (uint8_t)68
#define CONTROL_RIGHT       (uint8_t)67

#define TELNET_MAX_HISTORY_CMD         40
#define TELNET_MAX_CMD_LENGTH          300

#define TELNET_USER_NAME_LENGTH        20
#define TELNET_PASSWD_LENGTH           20
#define TELNET_MAX_MODE_LENGTH         64
#define TELNET_MAX_TAB_CMD             512
#define TELNET_MAX_WORDS               32

#define MARK_BLANK					   ' '
#define MARK_STRINGEND				   '\0'


#define OAM_STY_RING_SIZE              512

#define MARK_END                '\000'
#define MARK_LEFT               '\010'
#define CUT_CHAR                "\010 \010"

#define TELNET_MAX_INNER_BUF_LENGTH 1000

typedef enum
{
    CMD_MODE_INVALID = 0,
    CMD_MODE_NORMAL,
    CMD_MODE_CONFIG,
    CMD_MODE_ADMIN,
    CMD_MODE_DEBUG,
    CMD_MODE_MAX
}E_CMD_MODE;


/* current run status */
typedef enum
{
    Telnet_MsgHandle_INIT = 0,
    Telnet_MsgHandle_IDLE,
	Telnet_MsgHandle_EXECUTE
}E_Cur_Run_Status;





typedef struct tagTelnetVty
{
	uint8_t    ring_buf[OAM_STY_RING_SIZE];    /* input character ring buffer */
    uint16_t    ring_start;
    uint16_t    ring_end;        /* portion of ring buffer in use */

	uint8_t  ucLoginFlag;
	uint8_t  ucUserNameFlag;
	uint8_t  ucPasswdFlag;
    uint8_t  ucTryNum;

//	uint8_t   ucRole;
	char   aucUserName[TELNET_USER_NAME_LENGTH];
    char   aucPasswd[TELNET_PASSWD_LENGTH];
	char   aucConfirmPasswd[TELNET_PASSWD_LENGTH];

    char strCurrentCmd[TELNET_MAX_CMD_LENGTH];
    int dwCurrPos;
    int dwCmdLength;

    char strHistoryCmd[TELNET_MAX_HISTORY_CMD][TELNET_MAX_CMD_LENGTH];
    int dwCurrHistCmdLineNum;
    int dwHistCmdTotalNum;

	uint16_t  wWorkMode;
	char  strMode[TELNET_MAX_MODE_LENGTH];

	uint8_t ucCurrRunState;
//	uint8_t ucPwdStatus;

	T_Page_Info  tPageInfo;
	uint8_t  ucLocalPidExec;
}T_TELNET_VTY;

extern void tnInit();
extern void init_tel_vty(T_TELNET_VTY *ptTelnetVty);
extern void ReadTelnet(char *buffer, uint16_t nRecvLen);

extern void vty_out(T_TELNET_VTY *ptTelnetVty, char cmd[], int len);
extern void vty_clear(T_TELNET_VTY *ptTelnetVty);

extern void vty_delete(T_TELNET_VTY *ptTelnetVty);
extern void vty_backsapce(T_TELNET_VTY *ptTelnetVty);
extern void vty_history_add(T_TELNET_VTY *ptTelnetVty);
extern void vty_histroy_print(T_TELNET_VTY *ptTelnetVty);
extern void vty_next_line(T_TELNET_VTY *ptTelnetVty);
extern void vty_previous_line(T_TELNET_VTY *ptTelnetVty);
extern int  vty_complete(T_TELNET_VTY * ptTelnetVty);
extern void vty_excute(T_TELNET_VTY *ptTelnetVty);
extern void vty_question(T_TELNET_VTY *ptTelnetVty, char cc);

extern void vty_left_char(T_TELNET_VTY *ptTelnetVty);
extern void vty_right_char(T_TELNET_VTY *ptTelnetVty);
extern void vty_insert(T_TELNET_VTY *ptTelnetVty,char cc);
extern void vty_insert_char(T_TELNET_VTY *ptTelnetVty,char cc);
extern void ExcuteTelnetCmd(T_TELNET_VTY *ptTelnetVty);
extern void TelnetLogin();
extern void ExcuteToOutput(T_TELNET_VTY *ptTelnetVty);
extern void telnet_Inner_print(const char *IszPrnMsg,...);

extern void telnet_send(char * ucBufToSend, uint16_t wBufLength);
extern void telnet_shutdown();
extern int32_t CheckPara(char *pChar, uint16_t wParaType);
extern char * GetParaTypeString(uint16_t wParaType);


#endif  /* __TELNET_ENTRY_H__ */

