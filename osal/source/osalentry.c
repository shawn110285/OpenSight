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

#include "../include/osalentry.h"
#include "../include/osalmsgdef.h"
#include "../include/osalcfg.h"




config_param_t   gtSysCfgParam;

extern  int32_t OsLoadInitConfig(config_param_t  *ptInitCfg);



static   char * sapucWeek[7]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};



int32_t osal_init_system(void)
{

    osal_clk_t   tAbsClock;

    if(OSAL_FALSE == OsLoadInitConfig(&gtSysCfgParam))
		return OSAL_FALSE;

    if (OSAL_TRUE == OsPortIsProcExist(gtSysCfgParam.tProcMutexParam.wMutexPort))
    {
        printf("Another %s is running \r\n",osal_get_product_name_in_capital());
	    return OSAL_FALSE;
    }

    OsDbgPrnInit();

    if(OSAL_OK != OsPort_CreateTaskKey())
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[osal]:OsPort_Create_Task_Key, error!\n");
        return OSAL_FALSE;
    }

  	osal_sys_get_real_clock(&tAbsClock);
    osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"Product: %s \n",osal_get_product_name());
	osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"Version: %s\n", PRODUCT_VERSION);
    osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"Bulid at: %s %s \n", __DATE__, __TIME__);

    if(OSAL_TRUE == OsPortInit())
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[osal]:Init OS Adapt, ......................................OK \n");
    }
    else
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[osal]:Init OS Adapt, ......................................Failed!\n");
        return OSAL_FALSE;
    }

    if(OSAL_OK == OsFsInit())
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[osal]:Init File Management SubSystem, .....................OK \n");
    }
	else
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0,"[osal]:Init File Management SubSystem, .....................Failed!\n");
        return OSAL_FALSE;
	}

    if(OSAL_OK == OsInitMsgBuf())
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Init Memory Mangement SubSystem, ....................OK \n");
    }
	else
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Init Memory Mangement SubSystem, ....................Failed!\n");
        return OSAL_FALSE;
	}

    if(OSAL_OK == OsTimerInit())
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Init Timer Mangement subSystem, .....................OK \n");
    }
	else
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Init Timer Mangement subSystem, .....................Failed!\n");
        return OSAL_FALSE;
	}

    if(OSAL_OK == OsScheduleInit())
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Init Schedule  SubSystem, ...........................OK \n");
	}
	else
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Init Schedule  SubSystem, ...........................Failed!\n");
        return OSAL_FALSE;
	}

    if(OSAL_TRUE == SocketInit())
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Init socket pool, ...................................OK \n");
	}
	else
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Init socket pool, ...................................Failed!\n");
        return OSAL_FALSE;
	}


    return OSAL_TRUE;
}


extern  int32_t osCreateProcess();

int32_t osal_start_system(void)
{
    if(OSAL_FALSE == osCreateProcess())
		return OSAL_FALSE;

    OsCoreSendMsgToSysCtrl(MSG_MasterPowerOn);

    return OSAL_TRUE;
}

void  osal_shutdown_system()
{
    OsCoreSendMsgToSysCtrl(MSG_MasterPowerOff);
}

void  osal_sys_exit(int32_t iReason)
{
     exit(iReason);
}

char * osal_sys_getversion()
{
     static char strVersion[60+1] = {0};
	 snprintf(strVersion, 60, "%s(%s %s)", PRODUCT_VERSION, __DATE__, __TIME__);
     return (char *)strVersion;
}




