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



#ifndef  __OSAL_LINUX_H__
#define  __OSAL_LINUX_H__

/* if want to use pthread_get_attr_np,you should define the macro _GNU_SOURCE */
#define _GNU_SOURCE     /* To get pthread_getattr_np() declaration */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <strings.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <malloc.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <semaphore.h>
#include <pthread.h>

#include <bits/pthreadtypes.h>

#include <linux/unistd.h>
#include <linux/hdreg.h>
#include <linux/sysctl.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>   // tun/tap


#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/timeb.h>
#include <sys/shm.h>
#include <sys/ucontext.h>
// #include <sys/sysctl.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <netpacket/packet.h>


#define  SEM_TYPE_MUTEX           ((unsigned char)1)  /* 互斥锁类型*/
#define  SEM_TYPE_BINARY          ((unsigned char)2)  /* 同步信号量类型 */
#define  SEM_TYPE_COUNT           ((unsigned char)3)  /* 计数信号量类型 */


typedef struct tagLinuxSem
{
    unsigned char      ucSemType;   /*0:invalid,1:mutex,2:sem,3:count*/
    pthread_mutex_t    mutex;
    sem_t              sem;
}linux_sem_t;


#ifndef  MAX_PATH
#define  MAX_PATH              ((int)255)
#endif

#ifndef  O_BINARY
#define  O_BINARY              0x0
#endif

#define  INVALID_SOCKET        (int)-1
#define  SOCKET_ERROR          (int)-1

#define  min(a,b)  (((a)<(b))?(a):(b))
#define  max(a,b)  (((a)>(b))?(a):(b))

#ifndef assert
#define assert
#endif

#endif   /*  __OSAL_LINUX_H__  */


