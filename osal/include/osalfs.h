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


#ifndef  __OSAL_FILE_H__
#define  __OSAL_FILE_H__

#include "osalapi.h"


#define OSAL_FS_MAX_FD_NUM              100
#define OSAL_FS_MAX_PATH_LENGTH	        300


/********************************************************************************************
FD table definition
********************************************************************************************/
typedef struct tagOsalFileSystemFd
{
    uint8_t       bUseFlag;                             /*usage flag*/
    char          szFileName[OSAL_FS_MAX_PATH_LENGTH];	/*file name*/
    uint32_t      dwRealFD;			                    /*file descriptor*/
    uint32_t      wOpenPno;                             /*pno of process which open*/
    uint16_t      wFileTablePt;                         /*pointer to File Table*/
}osal_fs_fd_t;

/********************************************************************************************
File TAble definition
********************************************************************************************/
typedef struct tagFileTable
{
    uint8_t    bUseFlag;                             /*usage flag*/
    char       szFileName[OSAL_FS_MAX_PATH_LENGTH];	 /*file name*/
    int32_t    blLocked;		            	     /*lock flag*/
    uint32_t   dwLockedFD;		            	     /*locked FD*/
    uint32_t   LockPno;	   	            	         /*pno of process which lock this file*/
    uint16_t   wOpenCount;		            	     /*count of openning file*/
    uint16_t   wNextItem;  	            	         /*next tagFileTable*/
}osal_fs_file_tbl_t;


uint32_t    OsFsInit(void);
uint32_t  	OsFsFileCopy(char  * psrcFile,char  * ptgtFile);
uint32_t 	OsFsFileGetLength( char  *IszFileName );
uint32_t  	OsFsFileGetFatherDir(char  * pIszDirName,char  * pFatherDir);
uint32_t  	OsFsValifyFd(uint32_t  IfdFileId);
int16_t     OsFsCheckFileLock(uint32_t wFileTablePt);
void 		OsFsFreeFdTableItem(uint32_t  IfdFileId);
void 		OsFsFreeFileTableItem(uint16_t IwFileTablePt);
int16_t     OsFsAllocFdItem(void);
int16_t     OsFsAllocFileItem(void);
int16_t     OsFsSearchFileTable(char * szFileName);
int16_t     OsFsSearchFdTable(char * szFileName);
void 		OsFsInitFdTable(void);
void 		OsFsInitFileTable(void);

#endif   /* __OSAL_FILE_H__ */


