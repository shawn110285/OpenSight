
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


#include "../include/osalbasic.h"
#include "../include/osalsche.h"
#include "../include/osalmsgdef.h"
#include "../include/osalcfg.h"

#define     OSAL_DBG_MAX_PRN_BUF_LENGTH           300

extern config_param_t      gtSysCfgParam;

static uint32_t            gdwDbgPrnFlag   = 1;
static uint32_t            gdwDbgPrnLevel = 2;

//#define DBG_WRITE_FILE   1

#ifdef DBG_WRITE_FILE
static int                iFileHanlde = -1 ;
#endif


/* init the server socket and create the log file if necessary */
uint32_t   OsDbgPrnInit()
{
    gdwDbgPrnFlag      = gtSysCfgParam.tLoggerParam.dwDbgPrnFlag;
    gdwDbgPrnLevel     = gtSysCfgParam.tLoggerParam.dwDbgPrnLevel;

#ifdef DBG_WRITE_FILE
    if(1)
    {
        char strDbgFileName[MAX_PATH];
        memset(strDbgFileName,0,MAX_PATH);
        strncpy(strDbgFileName,osal_get_root_dir(),MAX_PATH);

        /*2015-08-12, shawn, modified to Linux Style*/
        strncat(strDbgFileName,"./",MAX_PATH);
        strncat(strDbgFileName,osal_get_product_name(),MAX_PATH);
        strncat(strDbgFileName,"opensight.log",MAX_PATH);
        iFileHanlde=open(strDbgFileName,O_RDWR|O_CREAT|O_TRUNC);
        if(iFileHanlde < 0)
        {
            printf("OsDbgPrnInit:Can not open file");
            return OSAL_ERROR;
        }
    }
#endif

    return  OSAL_OK;
}



void  osal_dbg_set_prn_param(uint32_t dwDbgPrnFlag, uint32_t dwPrnLevel)
{
    gdwDbgPrnFlag   = dwDbgPrnFlag;
    gdwDbgPrnLevel  = dwPrnLevel;
}



void  osal_dbg_print(uint32_t  IbmPrnGrade, const char *IszPrnMsg,...)
{
    int32_t           bPrnFlag = OSAL_FALSE;
    char              szDbgPrnbuf[OSAL_DBG_MAX_PRN_BUF_LENGTH];
    uint32_t          dwDbgPrnInfoLength = 0;
    va_list           argptr;

    uint32_t          dwResult;
    uint32_t   	      dwPno;
    uint32_t          dwBufLen=0;

    if(gdwDbgPrnFlag == 0)
        return;

    if(IbmPrnGrade >= gdwDbgPrnLevel)
    {
        memset(szDbgPrnbuf,0,OSAL_DBG_MAX_PRN_BUF_LENGTH);

        va_start(argptr,IszPrnMsg);
        vsnprintf(szDbgPrnbuf, OSAL_DBG_MAX_PRN_BUF_LENGTH, IszPrnMsg, argptr);
        va_end(argptr);

#ifdef DBG_WRITE_FILE
        if(iFileHanlde >0 )
        {
            write(iFileHanlde,szDbgPrnbuf,dwDbgPrnInfoLength);
        }
#endif
        fprintf(stderr,"%s",szDbgPrnbuf);
    }
}




