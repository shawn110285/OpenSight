
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

#include "../include/osalyaml.h"
#include "../include/osalcfg.h"
#include "../include/osalapi.h"

static int32_t parser_mutex_argument(yaml_iter_t * root_iter, config_param_t  * ptConfigParam)
{
    yaml_iter_t iter;
    yaml_iter_recurse(root_iter, &iter);
    while(yaml_iter_next(&iter))
    {
        const char *value_key = yaml_iter_key(&iter);
        if(!value_key)
        {
            printf("value key is NULL, File:%s, line:%d \r\n", __FILE__, __LINE__);
            return OSAL_FALSE;
        }

        if (!strcmp(value_key, "port"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tProcMutexParam.wMutexPort = atoi(v);
        }
        else
        {
            printf("unknown value key:%s, File:%s, line:%d  \r\n", value_key, __FILE__, __LINE__);
            return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}


static int32_t parser_logger_argument(yaml_iter_t * root_iter, config_param_t  * ptConfigParam)
{
    yaml_iter_t iter;
    yaml_iter_recurse(root_iter, &iter);
    while(yaml_iter_next(&iter))
    {
        const char * value_key = yaml_iter_key(&iter);
        if(!value_key)
        {
            printf("value key is NULL,  File:%s, line:%d \r\n", __FILE__, __LINE__);
            return OSAL_FALSE;
        }
        else if (!strcmp(value_key, "enable"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tLoggerParam.dwDbgPrnFlag = atoi(v);
        }
        else if (!strcmp(value_key, "level"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tLoggerParam.dwDbgPrnLevel = atoi(v);
        }
        else
        {
            printf("unknown value key:%s,  File:%s, line:%d \r\n", value_key, __FILE__, __LINE__);
            return OSAL_FALSE;
        }
    }//while(yaml_iter_next(&logger_iter))
    return OSAL_TRUE;
}



static int32_t parser_telnet_argument(yaml_iter_t * root_iter, config_param_t  * ptConfigParam)
{
    yaml_iter_t iter;
    yaml_iter_recurse(root_iter, &iter);
    while(yaml_iter_next(&iter))
    {
        const char *value_key = yaml_iter_key(&iter);
        if(!value_key)
        {
            printf("value key is NULL,  File:%s, line:%d \r\n", __FILE__, __LINE__);
            return OSAL_FALSE;
        }

        if (!strcmp(value_key, "server"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tTelnetSvrParam.dwTelnetServerIpAddr = ntohl(inet_addr(v));
        }
        else if (!strcmp(value_key, "port"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tTelnetSvrParam.wTelnetServerPort = atoi(v);
        }
        else if (!strcmp(value_key, "enable"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tTelnetSvrParam.dwTelnetServerFlag = atoi(v);
        }
        else
        {
            printf("unknown value key:%s,  File:%s, line:%d  \r\n", value_key, __FILE__, __LINE__);
            return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}



static int32_t parser_gdb_argument(yaml_iter_t * root_iter, config_param_t  * ptConfigParam)
{
    yaml_iter_t iter;
    yaml_iter_recurse(root_iter, &iter);
    while(yaml_iter_next(&iter))
    {
        const char *value_key = yaml_iter_key(&iter);
        if(!value_key)
        {
            printf("value key is NULL, File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return OSAL_FALSE;
        }

        if (!strcmp(value_key, "enable"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tGdbSvrParam.dwGdbServerFlag  = atoi(v);
        }
        else if (!strcmp(value_key, "server"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tGdbSvrParam.dwGdbServerIpAddr = ntohl(inet_addr(v));
        }
        else if (!strcmp(value_key, "hart_num"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tGdbSvrParam.wHartNum = atoi(v);
        }
        else if (!strcmp(value_key, "port"))
        {
            uint16_t  wHardId = 0;
            char    * token = NULL;

            const char delimiter[2] = ":";              //split the port list, the delimiter is :
            const char *v = yaml_iter_value(&iter);
            if (v)
            {
                /* get the first token */
                token = strtok(v, delimiter);

                /* walk through other tokens */
                while( token != NULL && wHardId < 100)
                {
                    ptConfigParam->tGdbSvrParam.awGdbServerPort[wHardId] = atoi(token);
                    token = strtok(NULL, delimiter);
                    wHardId ++ ;
                }
            }
        }
        else
        {
            printf("unknown value key:%s,  File:%s, line:%d \r\n", value_key, __FILE__, __LINE__);
            return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}


static int32_t parser_coresight_argument(yaml_iter_t * root_iter, config_param_t  * ptConfigParam)
{
    yaml_iter_t iter;
    yaml_iter_recurse(root_iter, &iter);
    while(yaml_iter_next(&iter))
    {
        const char *value_key = yaml_iter_key(&iter);
        if(!value_key)
        {
            printf("value key is NULL, File:%s, line:%d  \r\n", __FILE__, __LINE__);
            return OSAL_FALSE;
        }

        if (!strcmp(value_key, "clk_div"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tCoreSightParam.dwJtagClkDiv = strtoul(v, 0, 0);
            else   ptConfigParam->tCoreSightParam.dwJtagClkDiv = 100;
        }
        else if (!strcmp(value_key, "dap_idcode"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tCoreSightParam.dwDapId = strtoul(v, 0, 0);
            else   ptConfigParam->tCoreSightParam.dwDapId = 0x6ba00477;
        }
        else if (!strcmp(value_key, "jtag_ap_num"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tCoreSightParam.dwJtagApNum  = strtoul(v, 0, 0);
            else ptConfigParam->tCoreSightParam.dwJtagApNum = 0;

        }
        else if (!strcmp(value_key, "apb_ap_num"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tCoreSightParam.dwApbApNum = strtoul(v, 0, 0);
            else  ptConfigParam->tCoreSightParam.dwApbApNum = 1;
        }
        else if (!strcmp(value_key, "funnel_addr"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tCoreSightParam.dwFunnelAddr = strtoul(v, 0, 0);
            else ptConfigParam->tCoreSightParam.dwFunnelAddr =  0x80001000;
        }
        else if (!strcmp(value_key, "etf_addr"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tCoreSightParam.dwEtfAddr  = strtoul(v, 0, 0);
            else ptConfigParam->tCoreSightParam.dwEtfAddr =  0x80002000;
        }
        else if (!strcmp(value_key, "replicator_addr"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tCoreSightParam.dwReplicatorAddr =strtoul(v, 0, 0);
            else ptConfigParam->tCoreSightParam.dwReplicatorAddr =  0x80003000;
        }
        else if (!strcmp(value_key, "etb_base_addr"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tCoreSightParam.dwEtbAddr = strtoul(v, 0, 0);
            else ptConfigParam->tCoreSightParam.dwEtbAddr =  0x80004000;
        }
        else if (!strcmp(value_key, "tpiu_addr"))
        {
            const char *v = yaml_iter_value(&iter);
            if (v) ptConfigParam->tCoreSightParam.dwTpiuAddr  = strtoul(v, 0, 0);
            else ptConfigParam->tCoreSightParam.dwTpiuAddr =  0x80005000;
        }
        else
        {
            printf("unknown value key:%s,  File:%s, line:%d \r\n", value_key, __FILE__, __LINE__);
            return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}

int32_t OsLoadInitConfig(config_param_t  * ptConfigParam)
{
    FILE            * file;
    yaml_parser_t     parser;
    yaml_document_t * document = NULL;
    yaml_iter_t       root_iter;

	char  strCfgFileName[MAX_PATH +1] = {0};

    memset(strCfgFileName,0,MAX_PATH+1);
	strncpy(strCfgFileName, osal_get_root_dir(), MAX_PATH);
	strncat(strCfgFileName,"opensight.yaml",MAX_PATH);
	printf("config file is %s !\n",strCfgFileName);

    file = fopen(strCfgFileName, "rb");
    if(!file)
    {
        printf("Failed to read configuration file (%s),  File:%s, line:%d \r\n", strCfgFileName, __FILE__, __LINE__);
        return OSAL_FALSE;
    }

    if(!yaml_parser_initialize(&parser))
    {
        printf("yaml_parser_initialize failed,  File:%s, line:%d \r\n", __FILE__, __LINE__);
        fclose(file);
        return OSAL_FALSE;
    }

    yaml_parser_set_input_file(&parser, file);
    document = (yaml_document_t *) osal_mem_alloc(sizeof(yaml_document_t));
    if(document == NULL)
    {
        printf("alloc memory for yaml document failed,  File:%s, line:%d \r\n", __FILE__, __LINE__);
        fclose(file);
        return OSAL_FALSE;
    }

    if (!yaml_parser_load(&parser, document))
    {
        printf("yaml_parer_load failed,  File:%s, line:%d \r\n", __FILE__, __LINE__);
        osal_mem_free(document);
        fclose(file);
        return OSAL_FALSE;
    }

    yaml_parser_delete(&parser);
    fclose(file);

    yaml_iter_init(&root_iter, document);
    while(yaml_iter_next(&root_iter))
    {
        const char * root_key = yaml_iter_key(&root_iter);
        if(!root_key)
        {
            printf("yaml_iter_key (root) failed , File:%s, line:%d \r\n", __FILE__, __LINE__);
            return OSAL_FALSE;
        }

        if (!strcmp(root_key, "mutex"))
        {
            if(!parser_mutex_argument(&root_iter, ptConfigParam))
                return OSAL_FALSE;
        }
        else if (!strcmp(root_key, "logger"))
        {
            if(!parser_logger_argument(&root_iter, ptConfigParam))
                return OSAL_FALSE;
        }
        else if (!strcmp(root_key, "telnet"))
        {
            if(!parser_telnet_argument(&root_iter, ptConfigParam))
                return OSAL_FALSE;
        }
        else if (!strcmp(root_key, "gdb"))
        {
            if(!parser_gdb_argument(&root_iter, ptConfigParam))
                return OSAL_FALSE;
        }
        else if (!strcmp(root_key, "coresight"))
        {
            if(!parser_coresight_argument(&root_iter, ptConfigParam))
                return OSAL_FALSE;
        }
        else
        {
            printf("unknow yaml root_key: %s ,  File:%s, line:%d \r\n", root_key, __FILE__, __LINE__);
            return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}