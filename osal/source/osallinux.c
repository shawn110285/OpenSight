
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

#include "../include/osallinux.h"
#include "../include/osalconst.h"


extern void osal_dbg_print(uint32_t  IbmPrnGrade, const char *IszPrnMsg,...);

int32_t OsPort_GetPIDByName(char *acProcName, const int32_t dwPID, int32_t *pdwPID);


int32_t  CreateSem (linux_sem_t * ptOsalSem, unsigned char ucSemType, unsigned int dwInitValue, unsigned int dwMaxValue)
{
    if (NULL == ptOsalSem)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[OSAL]CreateSem: the input parameter is NULL error! \n");
        return OSAL_FALSE;
    }

    if ((SEM_TYPE_MUTEX != ucSemType) && (SEM_TYPE_BINARY != ucSemType) && (SEM_TYPE_COUNT != ucSemType))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[OSAL]CreateSem: the type is invalid! \n");
        return OSAL_FALSE;
    }
    ptOsalSem->ucSemType = ucSemType;

    switch (ucSemType)
    {
        case SEM_TYPE_COUNT:
        {
             if (0 != sem_init(&(ptOsalSem->sem), 0, dwInitValue))
             {
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[OSAL]CreateSem: create SEM_TYPE_COUNT, sem_init failed! \n");
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
                  return OSAL_FALSE;
             }

        }
        break;

        case SEM_TYPE_BINARY:
        {
             if (0 != sem_init(&(ptOsalSem->sem), 0, dwInitValue))
             {
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[OSAL]CreateSem: create SEM_TYPE_BINARY, sem_init failed! \n");
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
                  return OSAL_FALSE;
             }

        }
        break ;


        case SEM_TYPE_MUTEX:
        {
             if (0 != pthread_mutex_init(&(ptOsalSem->mutex), NULL))
             {
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[OSAL]CreateSem: pthread_mutex_init failed! \n");
                  osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
                  return OSAL_FALSE;
             }
        }
        break ;

        default:
        {
             osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[OSAL]CreateSem: the flag is wrong! \n");
             return OSAL_FALSE;
        }
    }
    return OSAL_TRUE;
}





int32_t WaitForSem(linux_sem_t *ptOsalSem, int dwMilliSeconds)
{
	    int dwRet = 0;

        if (NULL == ptOsalSem)
        {
                 osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[OSAL]WaitForSem: the input parameter is NULL error! \n");
		         return OSAL_FALSE;
        }

	    switch (ptOsalSem->ucSemType)
	    {
	            case SEM_TYPE_COUNT:
		        case SEM_TYPE_BINARY:
			    {
                         if (WAIT_FOREVER == dwMilliSeconds)
                         {
	                            do
	                            {
		                             dwRet = sem_wait(&(ptOsalSem->sem));
	                            }while ((dwRet != 0) && (errno == EINTR));

					            if (0 != dwRet)
				                {
					                     osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
                                         return OSAL_FALSE;
				                }

                        }
                        else if (NO_WAIT == dwMilliSeconds)
                        {
	                            do
	                            {
		                              dwRet =  sem_trywait(&(ptOsalSem->sem));
	                            }while ((dwRet != 0) && (errno == EINTR));

		                        if (0 != dwRet)
	                            {
		                	             osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
                                         return OSAL_FALSE;
	                            }
                        }
                        else
                        {
                   	            struct timespec ts;
	                            long   dwTmpnsec = 0;
	                            clock_gettime(CLOCK_REALTIME, &ts);
	                            ts.tv_sec += (dwMilliSeconds / 1000);
	                            dwTmpnsec =  (dwMilliSeconds % 1000) * 1000 * 1000 + ts.tv_nsec;
	                            ts.tv_sec += dwTmpnsec /(1000 * 1000 * 1000);
	                            ts.tv_nsec = dwTmpnsec % (1000 * 1000 * 1000);

	                            do
	                            {
		                                dwRet = sem_timedwait(&(ptOsalSem->sem), &ts);
	                            }while ((dwRet != 0) && (errno == EINTR));

                                if (0 != dwRet)
                                {
                	                    osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
                                        return OSAL_FALSE;
                                }
                        }
			    }
		        break ;

		        case SEM_TYPE_MUTEX:
		        {
				        if (WAIT_FOREVER == dwMilliSeconds)
				        {
					            dwRet = pthread_mutex_lock(&(ptOsalSem->mutex));
				        }
				        else if (NO_WAIT == dwMilliSeconds)
				        {
					            dwRet = pthread_mutex_trylock(&(ptOsalSem->mutex));
				        }
				        else
				        {
                	            struct timespec ts;
	                            long   dwTmpnsec = 0;
	                            clock_gettime(CLOCK_REALTIME, &ts);
	                            ts.tv_sec += (dwMilliSeconds / 1000);
	                            dwTmpnsec =  (dwMilliSeconds % 1000) * 1000 * 1000 + ts.tv_nsec;
	                            ts.tv_sec += dwTmpnsec /(1000 * 1000 * 1000);
	                            ts.tv_nsec = dwTmpnsec % (1000 * 1000 * 1000);
					            dwRet = pthread_mutex_timedlock(&(ptOsalSem->mutex), &ts);
				        }

						if (0 != dwRet)
				        {
					            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
                                return OSAL_FALSE;
                        }
			    }
		        break ;

			    default:
			    {
				  	    return OSAL_FALSE;
			    }
			    break;

        }
	    return OSAL_TRUE;
}





int32_t ReleaseSem(linux_sem_t * ptOsalSem, unsigned int dwReleaseCount)
{
        if (NULL == ptOsalSem)
        {
                osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[OSAL]WaitForSem: the input parameter is NULL error! \n");
		        return OSAL_FALSE;
        }

	    switch (ptOsalSem->ucSemType)
	    {
		        case SEM_TYPE_COUNT:
		        case SEM_TYPE_BINARY:
			    {
                        if (0 != sem_post(&(ptOsalSem->sem)))
                        {
                	            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
                                return OSAL_FALSE;
                        }
			    }
		        break ;

		        case SEM_TYPE_MUTEX:
			    {
	                    if (0 != pthread_mutex_unlock(&(ptOsalSem->mutex)))
                        {
	            	            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
                                return OSAL_FALSE;
                        }
			    }
                break ;

                default:
			    {
				        return OSAL_FALSE;
         	    }
				break;
	    }
	    return OSAL_TRUE;
}


int  OsPort_taskSpawn(char *name, FUNCPTR entryPt,int priority,int  stackSize,long int arg)
{
	pthread_attr_t attr;
	pthread_t tTid;
    uint32_t ret;
	struct sched_param param;
	size_t swMemPageSize = getpagesize();

    if (NULL == entryPt)
    {
	    return 0;
    }

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); /*PTHREAD_SCOPE_PROCESS*/
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_attr_getschedparam(&attr, &param);
    param.sched_priority = priority;
    pthread_attr_setschedparam(&attr, &param);

    pthread_attr_setstacksize(&attr, 1024 * 512); /* 此处有异常，为何不用入参的dwStackSize */
    if ( swMemPageSize > 0 )
    {
        pthread_attr_setguardsize(&attr, swMemPageSize);
    }

    ret = pthread_create(&tTid, &attr, (void *)entryPt, (void *)arg);
    if (ret)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[OSAL]OsPort_taskSpawn: pthread_create failed! \n");
        return 0;
    }
	else
	{
	    return tTid;
	}

}



pid_t gettid()
{
    return syscall(SYS_gettid);
}






int32_t OsPort_taskSuspend ( )
{
    sigset_t   newmask, oldmask;
    int        wait_signo;
//    pthread_t  thread_id;
    pid_t      thread_tid;

//    thread_id = pthread_self();
    thread_tid = gettid();
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &newmask, &oldmask);
    osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "Task %u will suspend!\n", thread_tid);
    sigwait(&newmask, &wait_signo);
    if (SIGUSR2 == wait_signo)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "Task %u will resume ... ...\n", thread_tid);
    }
    else
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_0, "Suspended task %u receives a signal: %d.\n", thread_tid, wait_signo);
    }
    return OSAL_TRUE;
}







int  OsPort_taskIdSelf(void)
{
    pthread_t thread_id = pthread_self();
    return  thread_id;
}







int32_t   OsPort_taskDelay(int dwMillSecond)
{
    struct timespec tTimeout;
    struct timespec tRemaind;
    int dwRet = -1;

    tTimeout.tv_sec     = dwMillSecond / 1000;
    tTimeout.tv_nsec   = (dwMillSecond - tTimeout.tv_sec * 1000) * 1000 * 1000;

    if (0 == dwMillSecond)
    {
        tTimeout.tv_nsec  = 100;
    }

    do
    {
        dwRet = nanosleep(&tTimeout, &tRemaind);
        tTimeout = tRemaind;
    } while ((dwRet != 0) && (errno == EINTR));

    if (0 != dwRet)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "[osal]:Error %d: %s \n", errno, strerror(errno));
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}


pthread_key_t thread_gtpcb_ptr_key;

int32_t  OsPort_CreateTaskKey()
{
    return pthread_key_create(&thread_gtpcb_ptr_key, NULL);
}

int32_t  OsPort_SetTaskKeyparam(void *ptr)
{

    return pthread_setspecific(thread_gtpcb_ptr_key, ptr);

}

void * OsPort_Get_Task_Key_param()
{
    return pthread_getspecific(thread_gtpcb_ptr_key);
}


void OsPort_SendThreadSigNo(pthread_t thread,int signo)
{
     pthread_kill(thread,signo);
}



FUNCPTR  gpfTimeCallback = NULL;


void  sigactioncallback(int para1, siginfo_t * para2, void * para3)
{
	    gpfTimeCallback();
}




void init_time()
{
    struct itimerval value;

    value.it_value.tv_sec = 0;

    value.it_value.tv_usec = 10*1000;   /* 10 ms */

    value.it_interval = value.it_value;

    setitimer(ITIMER_REAL, &value, NULL);
}



int32_t OsPort_wdStart(int delay, FUNCPTR ptRoutine)
{
    struct sigaction tact;

    tact.sa_handler = NULL;
    tact.sa_flags = SA_SIGINFO|SA_ONSTACK;   /* dedicate stack */

    sigemptyset(&tact.sa_mask);
    gpfTimeCallback = ptRoutine;

    tact.sa_sigaction = sigactioncallback;

    sigaction(SIGALRM, &tact, NULL);

    init_time();

    return OSAL_TRUE;
}






unsigned long  OsPort_GetTick()
{
    struct timeval tick;

    memset(&tick, 0, sizeof(struct timeval));
    gettimeofday(&tick, NULL);
    return (tick.tv_sec*1000 + tick.tv_usec/1000);
}






int32_t  OsPort_GetCurrentClock(uint16_t * pwYear,uint8_t *pbMon,uint8_t * pbDay,uint8_t * pbHour,uint8_t * pbMin,uint8_t * pbSec,uint8_t * pbWeek)
{
      struct timeval  tSysTime;
      struct tm       tTmpTime;

      gettimeofday(&tSysTime, NULL);
      localtime_r(&tSysTime.tv_sec, &tTmpTime);

      *pbSec  =(uint8_t)  tTmpTime.tm_sec;
      *pbMin  =(uint8_t)  tTmpTime.tm_min;
      *pbHour =(uint8_t)  tTmpTime.tm_hour;
      *pbDay  =(uint8_t)  tTmpTime.tm_mday;
      *pbMon  =(uint8_t)  tTmpTime.tm_mon + 1;
      *pwYear =(uint16_t) tTmpTime.tm_year + 1900;
      *pbWeek =(uint8_t)  tTmpTime.tm_wday;

      return OSAL_TRUE;
}









int32_t  OsPort_SetCurrentClock(uint16_t wYear,uint8_t  bMon,uint8_t bDay,uint8_t bHour,uint8_t bMin,uint8_t bSec,uint8_t bWeek)
{
    struct tm       tTmpTime;
    struct timeval  tSysTime;

    tTmpTime.tm_year = wYear - 1900 ;
    tTmpTime.tm_mon  = bMon - 1 ;
    tTmpTime.tm_mday = bDay;
    tTmpTime.tm_hour = bHour;
    tTmpTime.tm_min  = bMin;
    tTmpTime.tm_sec  = bSec;

    tSysTime.tv_sec  = mktime(&tTmpTime);
    tSysTime.tv_usec = 0;

    if(tSysTime.tv_sec == -1)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "OsPort_SetCurrentClock: mktime errno %d \n", errno);
        return OSAL_FALSE;
    }

    if( settimeofday(&tSysTime, NULL) == -1)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "OsPort_SetCurrentClock: settimeofday errno %d \n", errno);
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}



int32_t OsPort_CreateDirectory(char *strName);

static void OsPort_PrintErrorInfo()
{
    osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Error %d: %s \n", errno, strerror(errno));
}


int32_t  OsPort_RevocationPrivilege()
{
     setuid(getuid());

	 return OSAL_TRUE;
}





int OsPort_OpenFile(char *strFileName, int iFlag, int iMode)
{
    return open(strFileName, O_RDWR, S_IRWXU /* iMode */);
}






int32_t OsPort_CloseFile(int iFileId)
{
    close(iFileId);
	return OSAL_TRUE;
}






int32_t OsPort_CreatFile(char *strFileName, int iFlag)
{
    int sfd = -1;

    sfd = creat(strFileName, S_IRWXU|S_IRWXG|S_IRWXO /* iFlag */);/* O_RDONLY, O_WRONLY, or O_RDWR */
    if (-1 == sfd)
    {
        OsPort_PrintErrorInfo();
        return OSAL_FALSE;
    }
    close(sfd);
    return OSAL_TRUE;
}








int32_t OsPort_DeleteFile(char *strFileName)
{
   /*Call unlink function to delete the file */
    if (-1 ==  unlink(strFileName))
    {
        OsPort_PrintErrorInfo();
        return OSAL_FALSE;
    }
    return  OSAL_TRUE;
}


int32_t OsPort_RenameFile(char *strNewName, char *strOldName)
{
    if (-1 == rename(strOldName, strNewName))
    {
        OsPort_PrintErrorInfo();
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}





int OsPort_GetFileLength(char *strFileName)
{
	struct stat fileStat; /* file  stat structure */

	if (-1 == stat(strFileName, &fileStat))
	{
       return -1;
	}
    else
    {
       return fileStat.st_size;
    }
}



int32_t OsPort_CopyFile(char *strDestFile, char *strSrcFile)
{
    int fdSrc = -1;
    int fdTgt = -1;
    int sSrc = 0;
    int sFileLen = -1;
    char pBuf[1] = "";

    fdSrc = open(strSrcFile, O_RDWR, 0);
    if (-1 == fdSrc)
    {
        return OSAL_FALSE;
    }
    fdTgt = creat(strDestFile, O_RDWR);
    if( -1 == fdTgt)
	{
	    return OSAL_FALSE;
	}

    sFileLen = OsPort_GetFileLength(strSrcFile);
    if (-1 == sFileLen)
    {
        close(fdTgt);
        close(fdSrc);
        return OSAL_FALSE;
    }

    lseek(fdSrc, 0, 0);
    lseek(fdTgt, 0, 0);

    for (sSrc = 0; sSrc < sFileLen; sFileLen++)
    {
        read(fdSrc, pBuf, 1);
        write(fdTgt, pBuf, 1);
        memset(pBuf, 0, sizeof(pBuf));
    }
    close(fdTgt);
    close(fdSrc);
    return OSAL_TRUE;
}


int32_t OsPort_CopyAllFiles(char *strDestDir, char *strSrcDir)
{
    DIR *dir;
    struct dirent *dp;
	struct stat stStatBuf;

    if (NULL == (dir = opendir(strSrcDir)))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "opendir in %s Return Error\n", strSrcDir);
        return OSAL_FALSE;
    }

    while (NULL != (dp = readdir(dir)))
	{
        char strDestName[MAX_PATH + 1], strSrcName[MAX_PATH + 1];
        strcpy(strSrcName, strSrcDir);
        strcat(strSrcName, "/");
        strcat(strSrcName, dp->d_name);
        strcpy(strDestName, strDestDir);
        strcat(strDestName, "/");
        strcat(strDestName, dp->d_name);

        if (-1 == stat(strSrcName, &stStatBuf))
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Get the stat error on file:%s\n", dp->d_name);
            continue;
        }
        if ((stStatBuf.st_mode & S_IFDIR)
            && 0 != strcmp(dp->d_name, ".")
            && 0 != strcmp(dp->d_name, ".."))
        {
            /*需要建立该子目录*/
            OsPort_CreateDirectory(strDestName);
            OsPort_CopyAllFiles(strDestName, strSrcName);
        }
        else
        {
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"%s --> %s\n", strSrcName, strDestName);
			OsPort_CopyFile(strDestName, strSrcName);
		}
    }
	closedir(dir);
	return OSAL_TRUE;
}



int32_t OsPort_DeleteAllFiles(char *strSrcDir)
{
    DIR *dir;
    struct dirent *dp;
	struct stat stStatBuf;

	if (NULL == (dir = opendir(strSrcDir)))
	{
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "opendir in %s Return Error \n", strSrcDir);
		return OSAL_FALSE;
	}

	while (NULL != (dp = readdir(dir)))
	{
	    char strSrcName[MAX_PATH + 1];
		strcpy(strSrcName, strSrcDir);
		strcat(strSrcName, "/");
		strcat(strSrcName, dp->d_name);

		if (-1 == stat(strSrcName, &stStatBuf))
		{
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "Get the stat error on file:%s \n", dp->d_name);
			continue;
		}

        if ((stStatBuf.st_mode & S_IFDIR)
			&& 0 != strcmp(dp->d_name, ".")
			&& 0 != strcmp(dp->d_name, ".."))
        {
			OsPort_DeleteAllFiles(strSrcName);
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "rmdir %s \n", strSrcName);
			if (-1 == remove(strSrcName))
			{
			    OsPort_PrintErrorInfo();
			}
        }
		else
		{
			if ((0 == strcmp(dp->d_name, ".") ) || (0 == strcmp(dp->d_name, "..")))
			{
             	continue ;
			}
			else
			{
			    OsPort_DeleteFile(strSrcName);
			}
            osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "OsPort_DeleteFile %s \n", strSrcName);
		}
	}
	closedir(dir);
	return OSAL_TRUE;
}



int32_t OsPort_CreateDirectory(char *strName)
{
	if (-1 == mkdir(strName, S_IRWXU | S_IRWXG | S_IWOTH | S_IROTH | S_IXOTH))
	{
	    OsPort_PrintErrorInfo();
		return OSAL_FALSE;
	}
	return OSAL_TRUE;
}


int32_t OsPort_DeleteDirectory(char *strName)
{
    if (OSAL_FALSE == OsPort_DeleteAllFiles(strName))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "OsPort_DeleteAllFiles failed! \n");
        return OSAL_FALSE;
    }

    if (-1 == remove(strName))
    {
        OsPort_PrintErrorInfo();
        return OSAL_FALSE;
    }
    return OSAL_TRUE;
}


int32_t OsPort_CopyDirectory(char *strNewName, char *strOldName)
{
    OsPort_CreateDirectory(strNewName);
    OsPort_CopyAllFiles(strNewName, strOldName);
	return OSAL_TRUE;
}


int32_t OsPort_MoveDirectory(char *strNewName, char *strOldName)
{
    OsPort_CopyDirectory(strNewName, strOldName);
    OsPort_DeleteDirectory(strOldName);
    return OSAL_TRUE;
}


int32_t OsPort_RenameDirectory(char *strNewName, char *strOldName)
{
    OsPort_MoveDirectory(strNewName,strOldName);
    return OSAL_TRUE;
}






static int32_t   sgbNetworkInitFlag = OSAL_FALSE;

int32_t   OsPort_InitNetwork()
{
    sgbNetworkInitFlag = OSAL_TRUE;
    return OSAL_TRUE;

}

int32_t  OsPort_SetSocketNonBlock(int iSocketFd)
{
    int flags = fcntl(iSocketFd,F_GETFL,0);
    if(flags < 0)
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_SetSocketNonBlock,Failed!\n");
        return OSAL_FALSE;
    }

    if(0> fcntl(iSocketFd,F_SETFL,flags|O_NONBLOCK))
    {
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:OsPort_SetSocketNonBlock,Failed!\n");
        return OSAL_FALSE;
    }

    return OSAL_TRUE;
}

int OsPort_GetLastError()
{
	return  errno;
}

void OsPort_HandSocketError()
{
	osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "errno =%d: msg= %s \n", errno, strerror(errno));
}



void OsPort_SetKeepLive(int sockfd)
{
    int keepAlive    = 1;
    int keepIdle     = 5;
    int keepInterval = 5;
    int keepCount    = 3;

    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepAlive, sizeof(keepAlive)))
    {
        OsPort_HandSocketError();
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "setsockopt SO_KEEPALIVE error!\n");
	 return ;
    }

    if (-1 == setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle)))
    {
        OsPort_HandSocketError();
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "setsockopt TCP_KEEPIDLE error!\n");
        return ;
    }

    if (-1 == setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval)))
    {
        OsPort_HandSocketError();
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "setsockopt TCP_KEEPINTVL error!\n");
	 return ;
    }

    if (-1 == setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount)))
    {
        OsPort_HandSocketError();
        osal_dbg_print(OSAL_DBG_PRN_LEVEL_2, "setsockopt TCP_KEEPCNT error!\n");
	 return ;
    }
}
