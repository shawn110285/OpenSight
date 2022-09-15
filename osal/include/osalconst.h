//-----------------------------------------------------------------------------
// File:    osalconst.h
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



#ifndef __OSAL_CONST_H__
#define __OSAL_CONST_H__

typedef  int  (*FUNCPTR) ();	               /* ptr to function returning int */
typedef  void (* PROCESS_ENTRY)();             /* ptr to function returning void */


#define  OSAL_FALSE                    (int32_t)(0)
#define  OSAL_TRUE                     (int32_t)(1)

#define  OSAL_INVALID_UINT32           (uint32_t)(0XFFFFFFFF)
#define  OSAL_INVALID_UINT16           (uint16_t)(0XFFFF)
#define  OSAL_INVALID_UINT8            (uint16_t)(0XFF)

#define  OSAL_DBG_PRN_LEVEL_0          (uint32_t)(0x00)
#define  OSAL_DBG_PRN_LEVEL_1          (uint32_t)(0x01)
#define  OSAL_DBG_PRN_LEVEL_2          (uint32_t)(0x02)

#define  OSAL_STD_IN   		           0
#define  OSAL_STD_OUT 		           1
#define  OSAL_STD_ERR  		           2

#define  OSAL_SEEK_SET 		           SEEK_SET
#define  OSAL_SEEK_CUR 		           SEEK_CUR
#define  OSAL_SEEK_END 		           SEEK_END

#define  OSAL_RDONLY    		       0
#define  OSAL_WRONLY    	           1
#define  OSAL_RDWR      	           2


#define  OSAL_ERR_INVALID_PNO          (uint32_t)(0xFFFFFFFF)
#define  OSAL_ERR_INVALID_TIMER_ID     (uint32_t)(0xFFFFFFFF)
#define  OSAL_ERR_INVALID_MUTEX_ID     (uint32_t)(0xFFFFFFFF)
#define  OSAL_ERR_INVALID_SEM_ID       (uint32_t)(0xFFFFFFFF)

#define  OSAL_NEGTIVE_SIGN(x)          (uint32_t)(0xFFFFFFFF - x)
#define  OSAL_OK                        0
#define  OSAL_ERROR                     OSAL_NEGTIVE_SIGN(1)
#define  OSAL_ERROR_TIMEOUT             OSAL_NEGTIVE_SIGN(2)
#define  OSAL_ERROR_PNO                 OSAL_NEGTIVE_SIGN(3)
#define  OSAL_ERROR_QUEUE_FULL          OSAL_NEGTIVE_SIGN(4)
#define  OSAL_ERROR_PARAM_ERR           OSAL_NEGTIVE_SIGN(5)
#define  OSAL_ERROR_SIZE                OSAL_NEGTIVE_SIGN(6)
#define  OSAL_ERROR_NAME                OSAL_NEGTIVE_SIGN(7)
#define  OSAL_ERROR_QUEUEFULL           OSAL_NEGTIVE_SIGN(8)
#define  OSAL_ERROR_DEADLOCK            OSAL_NEGTIVE_SIGN(9)


#define  OSAL_ERROR_FILENOTEXIST        OSAL_NEGTIVE_SIGN(20)
#define  OSAL_ERROR_BUF_EMPTY           OSAL_NEGTIVE_SIGN(21)
#define  OSAL_ERROR_FILEEXIST           OSAL_NEGTIVE_SIGN(22)
#define  OSAL_ERROR_INVALIDNAME         OSAL_NEGTIVE_SIGN(23)
#define  OSAL_ERROR_FILELocked          OSAL_NEGTIVE_SIGN(24)
#define  OSAL_ERROR_INVALIDFID          OSAL_NEGTIVE_SIGN(25)
#define  OSAL_ERROR_HAVELOCK            OSAL_NEGTIVE_SIGN(26)
#define  OSAL_ERROR_NOLOCK              OSAL_NEGTIVE_SIGN(27)
#define  OSAL_ERROR_INVALI_POSITION     OSAL_NEGTIVE_SIGN(28)
#define  OSAL_ERROR_DESTEXIST           OSAL_NEGTIVE_SIGN(29)
#define  OSAL_ERROR_SRCNOTEXIST         OSAL_NEGTIVE_SIGN(30)
#define  OSAL_ERROR_DIRNOTEXIST         OSAL_NEGTIVE_SIGN(31)
#define  OSAL_ERROR_DIREXIST            OSAL_NEGTIVE_SIGN(42)
#define  OSAL_ERROR_LOCKEDBYOTHER       OSAL_NEGTIVE_SIGN(43)

#define  WAIT_FOREVER                   (-1)
#define  NO_WAIT                        (0)

#define  SEM_FULL                        1
#define  SEM_EMPTY                       0

#endif   /* __OSAL_CONST_H__ */

