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


#include "../include/osalfs.h"


static char                 sgstrFsRoot[MAX_PATH];
static osal_fs_fd_t  		sgtFdTbl[OSAL_FS_MAX_FD_NUM]; 	 /*global FD table*/
static osal_fs_file_tbl_t	sgtFileTable[OSAL_FS_MAX_FD_NUM]; /*global File table*/


uint32_t  osal_file_open(char  * IszFileName,uint32_t  IdwFlag)
{
    uint32_t   dwRealFD ;               /* OPEN 调用返回的实际文件句柄 */
    int16_t   sFileTablePt;
    int16_t   sFreeFD;
    osal_pid_t   tSelfPid;

    char      szName[OSAL_FS_MAX_PATH_LENGTH];
    memset(szName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szName+strlen(sgstrFsRoot),IszFileName,strlen(IszFileName));

    /* judge the length of lszFileName nd then call vxwroks's open function*/
    if (OSAL_FS_MAX_PATH_LENGTH < strlen(szName))
        return OSAL_ERROR;

    dwRealFD =OsPort_OpenFile(szName,IdwFlag,0);
    if (-1 == dwRealFD)
        return OSAL_ERROR;

    /* Alloc free item from FD table;*/
    sFreeFD = OsFsAllocFdItem();
    if ( -1 == sFreeFD)
        return  OSAL_ERROR;

    /*search the File Table for IszFileName*/
    sFileTablePt = OsFsSearchFileTable(szName);
    if (-1 == sFileTablePt)
    {
        sFileTablePt = OsFsAllocFileItem();
        if(-1 == sFileTablePt)
            return OSAL_ERROR;
        else
        {
            strncpy(sgtFileTable[sFileTablePt].szFileName, szName,OSAL_FS_MAX_PATH_LENGTH);
            sgtFileTable[sFileTablePt].bUseFlag   = 1;
            sgtFileTable[sFileTablePt].wOpenCount = 1;
        }
    }
    else
        sgtFileTable[sFileTablePt].wOpenCount = sgtFileTable[sFileTablePt].wOpenCount + 1;

    /*fill the Alloced FD item with szFileName,dwRealFD,wFileTablePt*/
    strncpy((char *)sgtFdTbl[sFreeFD].szFileName,szName,OSAL_FS_MAX_PATH_LENGTH);
    sgtFdTbl[sFreeFD].dwRealFD     = dwRealFD;
    sgtFdTbl[sFreeFD].wFileTablePt = sFileTablePt;

	tSelfPid.dwPno = osal_task_get_self_pno();
    if(tSelfPid.dwPno == OSAL_ERR_INVALID_PNO)
    {
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_file_open: this process is not a Dos process!\n");
    	return  OSAL_ERROR;
    }

    sgtFdTbl[sFreeFD].wOpenPno    =  tSelfPid.dwPno;

    return sFreeFD;
}


uint32_t   osal_file_close(uint32_t  IfdFileId)
{
    uint16_t     wFileTablePt;
    uint32_t    dwRealFD;
    osal_pid_t    tSelfPid;

    /*Check the validity of IfdField*/
    if (OSAL_FALSE == OsFsValifyFd(IfdFileId))
        return OSAL_ERROR_INVALIDFID;

    wFileTablePt = sgtFdTbl[IfdFileId].wFileTablePt;
    dwRealFD      = sgtFdTbl[IfdFileId].dwRealFD;

    /*
    if the file is opened only once ,then Free the corresponding File Table item and FD table item
    if the file is opened more than once,then Free the corresponding FD table item
    if the file is locked by the same IfdFileId, then unlock the file
    */

	tSelfPid.dwPno = osal_task_get_self_pno();
    if(tSelfPid.dwPno == OSAL_ERR_INVALID_PNO)
    {
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_file_open: this process is not a Dos process!\n");
    	return  OSAL_ERROR;
    }

    if ( sgtFdTbl[IfdFileId].wOpenPno != tSelfPid.dwPno)
         return OSAL_ERROR;

    /* Check the file's opening count.*/
    sgtFileTable[wFileTablePt].wOpenCount = sgtFileTable[wFileTablePt].wOpenCount -1;
    if (0 == sgtFileTable[wFileTablePt].wOpenCount )
    {    /*if the file is opened only once ,then Free the corresponding File Table item */
        OsFsFreeFileTableItem(wFileTablePt);
        OsFsFreeFdTableItem(IfdFileId);
    }
    else
    {   /*the file is opened more than once*/
        /*check if the file is locked by the same IfdFileId*/

       if ((sgtFileTable[wFileTablePt].blLocked) &&
           (sgtFileTable[wFileTablePt].dwLockedFD == IfdFileId)&&
           (sgtFileTable[wFileTablePt].LockPno == tSelfPid.dwPno) )
       {
           /*Unlock the file*/
           sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].blLocked = OSAL_FALSE;
           sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].LockPno    = 0;
           sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].dwLockedFD = 0;
           /*Free FD table item specified by IfdFileId*/
           OsFsFreeFdTableItem(IfdFileId);
       }
       else
           /*Free FD table item specified by IfdFileId*/
           OsFsFreeFdTableItem(IfdFileId);
    }

    /*Call close function*/
    if (-1 == OsPort_CloseFile(dwRealFD))
        return OSAL_ERROR;
    return OSAL_OK;
}


uint32_t  osal_file_create(char  *  IszFileName)
{

    char      szName[OSAL_FS_MAX_PATH_LENGTH];
    memset(szName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szName+strlen(sgstrFsRoot),IszFileName,strlen(IszFileName));

    /* judge the length of lszFileName */
    if (OSAL_FS_MAX_PATH_LENGTH < strlen(szName))
        return OSAL_ERROR_INVALIDNAME;

    if(OSAL_FALSE == OsPort_CreatFile(szName,2))     /* O_RDONLY, O_WRONLY, or O_RDWR */
    {
    	  return  OSAL_ERROR;
    }
	else
		  return OSAL_OK;

}


uint32_t   osal_file_remove(char *IszFileName)
{
    uint32_t  sfd;

    char      szName[OSAL_FS_MAX_PATH_LENGTH];
    memset(szName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szName+strlen(sgstrFsRoot),IszFileName,strlen(IszFileName));

    /* Check the length of lszFileName */
    if (OSAL_FS_MAX_PATH_LENGTH < strlen(szName))
        return OSAL_ERROR_INVALIDNAME;

    sfd = OsPort_OpenFile(szName,2,0);
    if (-1 == sfd)
       return OSAL_ERROR_FILENOTEXIST;
    else
       OsPort_CloseFile(sfd);

    if (OSAL_FALSE ==  OsPort_DeleteFile(szName))
        return OSAL_ERROR;

    return OSAL_OK;
}


uint32_t   osal_file_lock(uint32_t  IfdFileId)
{
   osal_pid_t  tSelfPid;

   /*Check the validity of IfdField*/
   if (OSAL_FALSE == OsFsValifyFd(IfdFileId))
       return OSAL_ERROR_INVALIDFID;

   tSelfPid.dwPno = osal_task_get_self_pno();
   if(tSelfPid.dwPno == OSAL_ERR_INVALID_PNO)
   {
    	   osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"osal_file_open: this process is not a Dos process!\n");
    	   return  0;
   }

   if(sgtFdTbl[IfdFileId].wOpenPno != tSelfPid.dwPno)
       return OSAL_ERR_INVALID_PNO;

   /*Check if the file is locked*/
   if (sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].blLocked)
       return OSAL_ERROR_HAVELOCK;

   /*Lock the file*/
   sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].blLocked   = OSAL_TRUE;
   /*lock mark*/
   sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].LockPno    = tSelfPid.dwPno;
   sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].dwLockedFD = IfdFileId;

   return OSAL_OK;
}


uint32_t   osal_file_unlock(uint32_t  IfdFileId)
{
   uint32_t dwFileTablePt;
   osal_pid_t tSelfPid;

   /*Check the validity of IfdField*/
   if (OSAL_FALSE == OsFsValifyFd(IfdFileId))
       return OSAL_ERROR_INVALIDFID;

   dwFileTablePt = sgtFdTbl[IfdFileId].wFileTablePt;

	tSelfPid.dwPno = osal_task_get_self_pno();
    if(tSelfPid.dwPno == OSAL_ERR_INVALID_PNO)
   {
    	   osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"osal_file_open: this process is not a Dos process!\n");
    	   return  0;
   }
   /* if the file is locked by this IfdFileId, then  unlock the file*/
   if (sgtFileTable[dwFileTablePt].blLocked)
   {
       if ((sgtFileTable[dwFileTablePt].dwLockedFD == IfdFileId ) &&
           (sgtFileTable[dwFileTablePt].LockPno == tSelfPid.dwPno) )
       {
           sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].blLocked = OSAL_FALSE;
           sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].LockPno    = 0;
           sgtFileTable[sgtFdTbl[IfdFileId].wFileTablePt].dwLockedFD = 0;
       }
       else
           return OSAL_ERROR_LOCKEDBYOTHER;
   }
   else
       return OSAL_ERROR_NOLOCK;

   return OSAL_OK;
}


uint32_t osal_file_length( char  *IszFileName )
{

    char      szName[OSAL_FS_MAX_PATH_LENGTH];
	int      iFileLength;

    memset(szName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szName+strlen(sgstrFsRoot),IszFileName,strlen(IszFileName));

    /* Check the validity of file name*/
    if (OSAL_FS_MAX_PATH_LENGTH < strlen(szName))
        return OSAL_ERROR_INVALIDNAME;

    iFileLength = OsPort_GetFileLength(szName);

	if(iFileLength == -1)
		return  OSAL_ERROR_INVALIDNAME;
	else
		return iFileLength;

}


uint32_t  osal_file_seek (uint32_t  IfdFileId, uint32_t  IdwFilePosition, uint32_t  IdwOffset )
{
    uint32_t  dwRealFD ;
//    uint16_t   wFileTablePt;
    osal_pid_t  tSelfPid;

    /*Check the validity of IfdField*/
    if (OSAL_FALSE == OsFsValifyFd(IfdFileId))
        return OSAL_ERROR_INVALIDFID;

     /*Check the validity of IdwFilePosition*/
    if ((IdwFilePosition != OSAL_SEEK_SET) &&
        (IdwFilePosition != OSAL_SEEK_CUR) &&
        (IdwFilePosition != OSAL_SEEK_END))
            return OSAL_ERROR_INVALI_POSITION;

    dwRealFD     = sgtFdTbl[IfdFileId].dwRealFD;
//    wFileTablePt = sgtFdTbl[IfdFileId].wFileTablePt;

	tSelfPid.dwPno = osal_task_get_self_pno();
    if(tSelfPid.dwPno == OSAL_ERR_INVALID_PNO)
    {
    	   osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_file_open: this process is not a Dos process!\n");
    	   return  0;
    }

    if(sgtFdTbl[IfdFileId].wOpenPno != tSelfPid.dwPno)
       return OSAL_ERROR;

    /*Call lseek function to reLocation the file's descriptor pointer*/
    if ( -1 == lseek(dwRealFD,IdwOffset,IdwFilePosition) )
        return OSAL_ERROR;
    else
        return OSAL_OK;

}


uint32_t   osal_file_read (uint32_t  IfdFileId, void * IpvBuffer, uint32_t  IdwCount, uint32_t *OpdwCountBeRead )
{
   uint32_t   dwRealFD;
   int        sCountBeRead;
   osal_pid_t      tSelfPid;

   /*Check the validity of IfdField*/
   if (OSAL_FALSE == OsFsValifyFd(IfdFileId))
       return OSAL_ERROR_INVALIDFID;

   dwRealFD     = sgtFdTbl[IfdFileId].dwRealFD;

	tSelfPid.dwPno = osal_task_get_self_pno();
    if(tSelfPid.dwPno == OSAL_ERR_INVALID_PNO)
    {
    	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_file_open: this process is not a Dos process!\n");
    	return  OSAL_ERROR;
    }

   if(sgtFdTbl[IfdFileId].wOpenPno != tSelfPid.dwPno)
       return OSAL_ERROR;

   /*read the file */
   sCountBeRead = read(dwRealFD,      /* file descriptor from which to read */
                       IpvBuffer,     /* pointer to buffer to receive bytes */
                       IdwCount       /* max no. of bytes to read into buffer */
                      );
   if (sCountBeRead == -1)
        return OSAL_ERROR;

   /*set the value of OpdwCountbeRead*/
   (* OpdwCountBeRead) = sCountBeRead;
    return OSAL_OK;
}


uint32_t   osal_file_write
(
   uint32_t  IfdFileId,
   void * IpvBuffer,
   uint32_t  IdwCount,
   uint32_t *OpdwCountBeWrite
)
{
   uint32_t   dwRealFD;
   uint32_t   dwFileTablePt;
   int     iCountBeWrite;
   osal_pid_t   tSelfPid;

   /*Check the validity of IfdField*/
   if (OSAL_FALSE == OsFsValifyFd(IfdFileId))
       return OSAL_ERROR;

   dwRealFD      = sgtFdTbl[IfdFileId].dwRealFD;
   dwFileTablePt = sgtFdTbl[IfdFileId].wFileTablePt;

	tSelfPid.dwPno = osal_task_get_self_pno();
    if(tSelfPid.dwPno == OSAL_ERR_INVALID_PNO)
   {
    	   osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:osal_file_open: this process is not a Dos process!\n");
    	   return  OSAL_ERROR;
   }

 /*
   if(sgtFdTbl[IfdFileId].wOpenPno != tSelfPid.dwPno)
       return OSAL_ERROR;
*/

   /*Check if the file is locked*/
   switch (sgtFileTable[dwFileTablePt].blLocked)
   {
       case OSAL_TRUE:
           if((sgtFileTable[dwFileTablePt].dwLockedFD == IfdFileId)&&(sgtFileTable[dwFileTablePt].LockPno    == tSelfPid.dwPno))
           {
               iCountBeWrite = write(dwRealFD,IpvBuffer,IdwCount);
               if (iCountBeWrite == -1)
                   return OSAL_ERROR;
               /*set the value of OpdwCountbeRead*/
               (*OpdwCountBeWrite) = iCountBeWrite;
           }
           else
               return OSAL_ERROR;
           break;

       case OSAL_FALSE:
            iCountBeWrite = write(dwRealFD,IpvBuffer,IdwCount);   /* 找到问题所在也，write()会把0x0a,做1个0x0d,0x0a转换，用二进制的方式打开就可以解决问题。*/

            if (iCountBeWrite == -1)
                  return OSAL_ERROR;
           /*set the value of OpdwCountbeRead*/
           (*OpdwCountBeWrite) = iCountBeWrite;
           break;

       default:
           break;
   }

   return OSAL_OK;
}


uint32_t   osal_file_rename(char  *IszOldFileName,char  *IszNewFileName)
{

    char     szOldName[OSAL_FS_MAX_PATH_LENGTH],szNewName[OSAL_FS_MAX_PATH_LENGTH];

    memset(szOldName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szOldName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szOldName+strlen(sgstrFsRoot),IszOldFileName,strlen(IszOldFileName));

    memset(szNewName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szNewName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szNewName+strlen(sgstrFsRoot),IszNewFileName,strlen(IszNewFileName));


    if(OSAL_FALSE == OsPort_RenameFile(szNewName,szOldName))
		return OSAL_ERROR;
	else
		return OSAL_OK;
}


uint32_t   osal_dir_create(char  *IszDirName)
{
    char     szName[OSAL_FS_MAX_PATH_LENGTH];

    memset(szName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szName+strlen(sgstrFsRoot),IszDirName,strlen(IszDirName));

    if(OSAL_FALSE == OsPort_CreateDirectory(szName))
		return OSAL_ERROR;
	else
		return OSAL_OK;
}


uint32_t   osal_dir_remove(char  *IszDirName)
{
    char     szName[OSAL_FS_MAX_PATH_LENGTH];

    memset(szName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szName+strlen(sgstrFsRoot),IszDirName,strlen(IszDirName));

   /*Check the validity of directory */
    if (OSAL_FS_MAX_PATH_LENGTH < strlen(szName))
        return OSAL_ERROR_INVALIDNAME;

    if(OSAL_FALSE == OsPort_DeleteDirectory(szName))
		return OSAL_ERROR;
	else
		return OSAL_OK;
}



uint32_t   osal_dir_move(char *pszDestDirName,char *pszSrcDirName)
{
//    char psrcFile[OSAL_FS_MAX_PATH_LENGTH] ="";
//    char ptgtFile[OSAL_FS_MAX_PATH_LENGTH] ="";

    char     szOldName[OSAL_FS_MAX_PATH_LENGTH],szNewName[OSAL_FS_MAX_PATH_LENGTH];

    memset(szOldName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szOldName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szOldName+strlen(sgstrFsRoot),pszSrcDirName,strlen(pszSrcDirName));

    memset(szNewName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szNewName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szNewName+strlen(sgstrFsRoot),pszDestDirName,strlen(pszDestDirName));

    return OsPort_MoveDirectory(szNewName,szOldName);
}


uint32_t  osal_dir_rename(char  *IszOldDirName, char  *IszNewDirName)
{

    char     szOldName[OSAL_FS_MAX_PATH_LENGTH],szNewName[OSAL_FS_MAX_PATH_LENGTH];

    memset(szOldName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szOldName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szOldName+strlen(sgstrFsRoot),IszOldDirName,strlen(IszOldDirName));

    memset(szNewName,0,OSAL_FS_MAX_PATH_LENGTH);
    memcpy(szNewName,sgstrFsRoot,strlen(sgstrFsRoot));
    memcpy(szNewName+strlen(sgstrFsRoot),IszNewDirName,strlen(IszNewDirName));

    return OsPort_RenameDirectory(szNewName,szOldName);
}


void OsFsInitFileTable(void)
{
    uint16_t wFileTablePt;
    for (wFileTablePt = 0; wFileTablePt < OSAL_FS_MAX_FD_NUM ;  wFileTablePt++)
    {
        sgtFileTable[wFileTablePt].bUseFlag   = 0;          /*usage flag*/
        sgtFileTable[wFileTablePt].blLocked   = 0;	       /*lock flag*/
        sgtFileTable[wFileTablePt].dwLockedFD = 0;	       /*locked FD*/
        sgtFileTable[wFileTablePt].LockPno    = 0;	       /*pno of process which lock                                            this file*/
        sgtFileTable[wFileTablePt].wOpenCount = 0;	       /*count of openning file*/
        sgtFileTable[wFileTablePt].wNextItem  = 0;
        memset(sgtFileTable[wFileTablePt].szFileName, 0, OSAL_FS_MAX_PATH_LENGTH);  /*file name*/
    }
}


void OsFsInitFdTable(void)
{
    uint16_t wFDTablePt;
    for (wFDTablePt = 0; wFDTablePt < OSAL_FS_MAX_FD_NUM; wFDTablePt++)
    {
        sgtFdTbl[wFDTablePt].bUseFlag       = 0;          /*usage flag*/
        sgtFdTbl[wFDTablePt].dwRealFD       = 0;
        sgtFdTbl[wFDTablePt].wFileTablePt   = 0;
        sgtFdTbl[wFDTablePt].wOpenPno      = 0;
        memset(sgtFdTbl[wFDTablePt].szFileName, 0, OSAL_FS_MAX_PATH_LENGTH);  /*file name*/
    }
}


int16_t OsFsSearchFdTable(char * szFileName)
{
    uint16_t wFTItem = 0;
    int32_t bMatch = OSAL_FALSE;
    while ( (wFTItem < OSAL_FS_MAX_FD_NUM) && (!bMatch))
    {
        if (sgtFdTbl[wFTItem].bUseFlag != 1)
            wFTItem ++;
        else
        {
            if (strcmp(sgtFdTbl[wFTItem].szFileName,szFileName) ==0)
                bMatch = OSAL_TRUE;
            else
                wFTItem++;
        }
    }

    if (bMatch == OSAL_TRUE)
        return wFTItem; /*return the Index of the filename_matched item*/
    else
        return -1;
}


int16_t OsFsSearchFileTable(char * szFileName)
{
    int16_t iFTItem =0;
    int32_t bMatch = OSAL_FALSE;

    while ((iFTItem < OSAL_FS_MAX_FD_NUM) && (!bMatch))
    {
        if (sgtFileTable[iFTItem].bUseFlag != 1 )
            iFTItem ++;
        else
        {
            if (strcmp(sgtFileTable[iFTItem].szFileName, szFileName) ==0)
                bMatch = OSAL_TRUE;
            else
                iFTItem++;
        }
    }

    if (bMatch == OSAL_TRUE)
        return iFTItem; /*return the Index of the filename_matched item*/
    else
        return -1;
}


int16_t OsFsAllocFileItem(void)
{
    uint16_t    wFdItem = 0;
    int16_t   bFound  = OSAL_FALSE;

    while ((wFdItem < OSAL_FS_MAX_FD_NUM) && (!bFound))
    {
        if (sgtFileTable[wFdItem].bUseFlag == 1)
        {
            wFdItem++;
        }
        else
            bFound = OSAL_TRUE;
    }

    if (bFound != OSAL_TRUE)
        return -1;
    else
        return wFdItem;

}


int16_t OsFsAllocFdItem(void)
{
    uint16_t     wFdItem = 0;
    int16_t     bFound  = OSAL_FALSE;

    while ((wFdItem < OSAL_FS_MAX_FD_NUM) && (!bFound))
    {
        if (sgtFdTbl[wFdItem].bUseFlag == 1)
        {
            wFdItem++;
        }
        else
            bFound = OSAL_TRUE;
    }

    if (bFound == OSAL_TRUE )
    {
        sgtFdTbl[wFdItem].bUseFlag = 1;
        return wFdItem;
    }
    else
        return  -1;
}


void OsFsFreeFileTableItem(uint16_t IwFileTablePt)
{
    sgtFileTable[IwFileTablePt].bUseFlag   = 0;          /*usage flag*/
    sgtFileTable[IwFileTablePt].blLocked   = 0;	       /*lock flag*/
    sgtFileTable[IwFileTablePt].dwLockedFD = 0;	       /*locked FD*/
    sgtFileTable[IwFileTablePt].LockPno    = 0;	       /*pno of process which lock                                            this file*/
    sgtFileTable[IwFileTablePt].wOpenCount = 0;	       /*count of openning file*/
    sgtFileTable[IwFileTablePt].wNextItem  = 0;
    memset(sgtFileTable[IwFileTablePt].szFileName, 0, OSAL_FS_MAX_PATH_LENGTH);  /*file name*/

}


void OsFsFreeFdTableItem(uint32_t  IfdFileId)
{
    sgtFdTbl[IfdFileId].bUseFlag     = 0;    		        /*usage flag*/
    sgtFdTbl[IfdFileId].dwRealFD     = 0;			/*file descriptor*/
    sgtFdTbl[IfdFileId].wFileTablePt = 0;		        /*pointer to File Table*/
    sgtFdTbl[IfdFileId].wOpenPno     = 0;                     /*id of process which open this file*/
    memset(sgtFdTbl[IfdFileId].szFileName, 0, OSAL_FS_MAX_PATH_LENGTH);/*file name*/
}


int16_t OsFsCheckFileLock(uint32_t wFileTablePt)
{

    if ((sgtFileTable[wFileTablePt].bUseFlag) && (sgtFileTable[wFileTablePt].blLocked))
    {
        return 1;
    }
    else
        return -1;
}


uint32_t  OsFsValifyFd(uint32_t  IfdFileId)
{
   if ((IfdFileId < 0) || (IfdFileId > OSAL_FS_MAX_FD_NUM) )
       return OSAL_FALSE;
   if (sgtFdTbl[IfdFileId].bUseFlag == OSAL_FALSE)
       return OSAL_FALSE;
   return OSAL_TRUE;
}


uint32_t  OsFsFileGetFatherDir(char  * pIszDirName,char  * pFatherDir)
{
    int16_t sIndex;
    int16_t sFound;
    sIndex = strlen(pIszDirName);
    sFound = OSAL_FALSE;
    while( (sFound != OSAL_TRUE) && (sIndex >0))
    {
        if (pIszDirName[sIndex] == '/')
           sFound = OSAL_TRUE;
        else
           sIndex = sIndex - 1;
    }

    if (sFound == OSAL_TRUE)
    {
        strncpy(pFatherDir,pIszDirName,sIndex);
        return OSAL_OK;
    }
    else
        return OSAL_ERROR;
}


uint32_t    OsFsInit(void)
{

   memset(sgstrFsRoot,0,MAX_PATH);
   strncpy(sgstrFsRoot,osal_get_root_dir(),MAX_PATH);
   OsFsInitFileTable();
   OsFsInitFdTable();

   return OSAL_OK;
}



