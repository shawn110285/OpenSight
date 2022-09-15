//-----------------------------------------------------------------------------
// File:    makefile
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

#ifndef __OSAL_API_H__
#define __OSAL_API_H__

#include "osalbasic.h"
#include "osalconst.h"


#pragma pack (1)

#define OSAL_SCH_MAX_PROCESS_NAME_LENGTH		 30
#define OSAL_SCH_MAX_PROCESS_NUM_PER_TASK		 5

typedef struct tagProcessParam
{
  uint8_t              ucUsed;
  char               szName[OSAL_SCH_MAX_PROCESS_NAME_LENGTH+1];
  PROCESS_ENTRY      pStartEntry;
  uint32_t             dwDataSize;
  uint32_t             dwAsynMsgQLen;
  uint32_t             dwSyncMsgQLen;
}T_ProcessParam;


typedef struct tagTaskParam
{
	char			   szName[OSAL_SCH_MAX_PROCESS_NAME_LENGTH+1];
	uint8_t			   bAttr;
	uint8_t			   bPriLvl;
	uint32_t			   dwStkSize;
	T_ProcessParam     atProcessParam[OSAL_SCH_MAX_PROCESS_NUM_PER_TASK];
}T_TaskParam;



/*进程号定义，采用一个结构体，主要是为了方便后面可能的扩展，比如增加模块号 ，实现分布式功能*/

typedef struct tagPID
{
    uint32_t         dwPno;
} osal_pid_t;






/************************************  绝对时钟结构 **************************************/
typedef struct tagClock
{
    unsigned char     bSec;                /* seconds after the minute - [0,  59]      */
    unsigned char     bMin;                /* minutes after the hour   - [0,  59]      */
    unsigned char     bHour;               /* hours after midnight     - [0,  23]      */
    unsigned char     bDay;                /* day of the month         - [1,  31]      */
    unsigned char     bMon;                /* months since January     - [1,  12]      */
    unsigned char     bWeek;               /* week,now,no use                          */
    unsigned short    wYear;               /* years since 2001         - [2000, 2130]    */
}osal_clk_t;



extern void   osal_set_root_dir(char *pcRootDir);
extern char * osal_get_root_dir();

extern void   osal_set_product_name(char * strProductName);
extern char * osal_get_product_name();
extern char * osal_get_product_name_in_capital();

extern char * osal_sys_getversion();





extern uint32_t   osal_task_create(T_TaskParam       * ptTaskParam);

/******************************************************************************************************************
    原语：osal_task_get_pno_by_name
    功能：得到指定进程名对应的进程号
    参数：
          IszProcessName：进程名称
    返回：
           OSAL_ERR_INVALID_PNO:函数调用失败
           其它值: 该进程名对于的进程号
******************************************************************************************************************/
extern uint32_t  osal_task_get_pno_by_name (char  *IszProcessName);
extern uint32_t  osal_task_get_name_by_pno(uint32_t dwPno, char * strProcName, uint8_t ucBufLen);



/*****************************************************************************************************************
    原语：osal_task_get_self_pno
    功能：得到本进程的进程号
    返回：
          OSAL_ERR_INVALID_PNO: 函数调用失败
          其它值: 该进程名对于的进程号
******************************************************************************************************************/
extern uint32_t   osal_task_get_self_pno ();




/*****************************************************************************************************************
    原语：osal_task_get_self_pid
    功能：得到本进程的PID
    返回：
          OSAL_OK: 函数调用成功,OptPid中存放由自身进程的PID
          其它值: 函数调用失败
******************************************************************************************************************/
extern uint32_t  osal_task_get_self_pid (osal_pid_t *OptPid);



/******************************************************************************************************************
    原语：osal_task_get_self_state
    功能：得到本进程的当前用户状态
    返回：
           OSAL_ERROR:调用失败
           其它: 进程用户状态
    注解：
          进程的用户状态不同于操作系统定义的进程状态，进程状态是指RUN，READY，PEND，SUSPEND等，
		  而用户状态是进程控制运行逻辑的状态，与OS无关。所有进程的初始用户状态是INIT_STATE(0)
*******************************************************************************************************************/
extern uint32_t   osal_task_get_self_state ( );





/*****************************************************************************************************************
    原语: osal_task_get_state_by_pid
    功能: 得到指定进程的当前状态
	参数：ItPid ：指定进程的PID
    返回：
         OSAL_ERR：失败
         其它:进程用户状态
    注解：
          进程的用户状态不同于操作系统定义的进程状态，进程状态是指RUN，READY，PEND，SUSPEND等，
		  而用户状态是进程控制运行逻辑的状态，与OS无关。所有进程的初始用户状态是INIT_STATE(0)
******************************************************************************************************************/
extern uint32_t   osal_task_get_state_by_pid (osal_pid_t ItPid);






/******************************************************************************************
    原语：osal_task_set_next_state
	功能：设置当前进程的用户状态
    参数：IUserState ：进程用户状态
    返回：
          OSAL_OK: 函数执行成功
    说明：
          设置当前调用进程的用户状态，osal_task_get_self_state调用的返回值就是IUserState
*******************************************************************************************/
extern uint32_t   osal_task_set_next_state (uint32_t  IbUserState);






/******************************************************************************************
    原语：osal_task_get_private_data_ptr
    功能：返回进程的私有数据区指针
    返回：
          NULL :   函数调用错误
          非NULL:  该进程的私有数据区指针
*******************************************************************************************/
extern void *  osal_task_get_private_data_ptr();

/******************************************************************************************
    原语(12)：osal_task_get_private_data_ptr_by_pno
    参数：
         dwPno
    返回：
         NULL :   函数调用错误
         非NULL:  该进程的私有数据区指针

    说明：得到指定进程的私有数据区指针
*******************************************************************************************/
extern void *  osal_task_get_private_data_ptr_by_pno(uint32_t dwPno);







/******************************************************************************************
    原语：osal_task_get_private_data_size
    功能：得到进程的私有数据区长度。
    返回：
           0： 函数调用失败
          >0:  进程私有数据区的大小
*******************************************************************************************/
extern uint32_t  osal_task_get_private_data_size( );






/******************************************************************************************
    原语：osal_task_delay
	功能：使当前进程睡眠一段时间,主动释放CPU使用权
    参数：IwDelayCount100ms: 睡眠时间，以 毫秒为单位
    返回：
          OSAL_OK  :  函数执行成功
          OSAL_ERROR :  函数执行失败
    注解：睡眠时间最多不超过3秒
*******************************************************************************************/
extern uint32_t  osal_task_delay (uint32_t dwMillSecond);




/******************************************************************************************
    原语：osal_mutex_create
    功能：创建互斥区
    返回：
          OSAL_ERROR_INVALID_MUTEXID : 创建互斥区失败。
          其它: 返回互斥区标识。
*******************************************************************************************/
extern uint32_t   osal_mutex_create ();




/******************************************************************************************
    原语：osal_mutex_delete
    功能：删除指定互斥区
	参数：IMutexID:互斥区ID,由osal_mutex_create返回
    返回：
          OSAL_OK : 互斥区删除成功。
          OSAL_ERROR_INVALID_MUTEXID : 无效的互斥区ID。
          OSAL_ERROR  : 未知的错误。
*******************************************************************************************/
extern uint32_t   osal_mutex_delete (uint32_t  IMutexID);


/******************************************************************************************
    原语：osal_mutex_enter
	功能: 进入临界区,若此时互斥区满，则进程被阻塞。
    参数：IMutexID：互斥区标识
    返回：
          OSAL_OK :			 函数执行成功。
          OSAL_ERROR_INVALID_MUTEXID  :     无效的临界区ID
*******************************************************************************************/
extern uint32_t  osal_mutex_enter (uint32_t  IMutexID);


/******************************************************************************************
    原语：osal_mutex_leave
	功能：离开临界区
    参数：IMutexID：互斥区标识
    返回：
           OSAL_OK :	 函数执行成功。
           OSAL_ERROR_INVALID_MUTEXID  :     无效的临界区ID
******************************************************************************************/
extern uint32_t   osal_mutex_leave (uint32_t  IMutexID);




/******************************************************************************************
    原语：osal_sem_create
    功能：创建信号量
    返回：
          OSAL_ERROR_INVALID_SEMID : 创建信号量失败。
          >0      : 返回的信号量标志符。
*******************************************************************************************/
extern uint32_t   osal_sem_create();


/******************************************************************************************
    原语：osal_sem_delete
	功能：删除指定的信号量
    参数：ISemID ：信号量ID
    返回：
          OSAL_OK : 信号量删除成功。
          OSAL_ERROR_INVALID_SEMID : 无效的信号量ID。
          OSAL_ERROR: 未知的错误。
*******************************************************************************************/
extern uint32_t   osal_sem_delete (uint32_t  ISemID);



/******************************************************************************************
    原语：osal_sem_take
	功能：信号量Take操作
    参数：
          ISemID             信号量的标识
          IwMilliSeconds     进程的在信号量上最大等待时间，以毫秒为单位，总时间小于3000毫秒，0表示不等待
    返回：
          OSAL_OK            		成功
          DOS_TIME_OUT      		进程超时
          OSAL_ERROR_INVALID_SEMID         标识错
          OSAL_ERROR           		其他错误
*******************************************************************************************/
extern uint32_t  osal_sem_take (uint32_t  ISemID, uint16_t IwMilliSeconds);


/*****************************************************************************************
    原语：osal_sem_give
	功能：信号量Give操作
    参数：ISemID：信号量的标识
    返回：
          DOS_OK：成功
          OSAL_ERROR_INVALID_SEMID：标识错
******************************************************************************************/
extern uint32_t  osal_sem_give (uint32_t   ISemID);






/**********************************************************************************************
    原语：osal_msg_malloc
	功能：为消息申请缓冲区
    参数：
          IdwBufSize            缓冲区大小
    返回：
          NULL              失败
          非NULL            缓冲区指针
***********************************************************************************************/
extern void * osal_msg_malloc_buf (uint32_t IdwBufSize);



/**********************************************************************************************
    原语：osal_msg_free
	功能：释放由osal_msg_malloc得到的缓冲区
    参数：IpvBuf：缓冲区指针
    返回：
          OSAL_OK :       成功
          OSAL_ERROR:       失败
*********************************************************************************************/
extern uint32_t  osal_msg_free_buf (void *IpvBuf);





/**********************************************************************************************
    原语：osal_msg_get_buf_count
	功能：得到指定缓冲区的数目
    参数：IdwBufSize : 缓冲区大小
    返回：
          OSAL_OK :       成功
          OSAL_ERROR:       失败
*********************************************************************************************/

extern uint32_t  osal_msg_get_buf_count(uint32_t IdwBufSize,uint32_t * pdwTotalNum,uint32_t *pdwFreeNum);





/******************************************************************************************
    原语：osal_msg_get_sender
	功能：返回当前处理消息的发送进程的PID
    参数：ptPid：返回消息发送进程的PID
    返回：
          OSAL_OK ：成功
          OSAL_ERR：失败，调用进程非OSAL封装的进程
*******************************************************************************************/
extern uint32_t  osal_msg_get_sender (osal_pid_t *OptPid);



/*******************************************************************************************
    原语：osal_msg_get_id
	功能：返回当前处理消息的消息号
    返回：
          0  ：      函数调用失败
          >0 :       消息ID号
********************************************************************************************/
extern uint32_t  osal_msg_get_id ();






/*******************************************************************************************
    原语：osal_msg_get_data
	功能：返回指向当前处理消息的消息内容的指针
    返回：
          NULL    :  函数调用失败
          非NULL  :  消息的数据指针
********************************************************************************************/
extern void * osal_msg_get_data ();





/*******************************************************************************************
    原语：osal_msg_get_data_length
	功能：返回当前处理消息的长度
    返回：消息长度
********************************************************************************************/
extern uint32_t osal_msg_get_data_length ();





/*******************************************************************************************
    原语：osal_msg_get_ack_data_ptr
    功能：得到同步消息应答数据区指针
    返回：
          NULL    :  函数调用失败
          非NULL  :  消息的数据指针
********************************************************************************************/
extern void  * osal_msg_get_ack_data_ptr( );





/*******************************************************************************************
    原语：osal_msg_get_ack_data_length
	功能：得到同步消息应答数据区长度
    返回：应答数据区长度
********************************************************************************************/
extern uint16_t  osal_msg_get_ack_data_length( );




/********************************************************************************************
    原语：osal_msg_post_out
	功能：给指定进程发送异步消息
    参数：
          ItReceiver          接收消息的进程PID
          IwMsgId              消息号
          IpvData             消息体指针
          IdwDataSize         消息体长度
    返回：
          OSAL_OK               成功
          OSAL_ERROR_NODE        目标节点错误
          OSAL_ERROR_PNO         目标进程号错误
          OSAL_ERROR_SIZE        非法的消息长度
          OSAL_ERROR_COMM        网络通讯异常
          OSAL_ERROR             其他错误
    注解：
          向进程发送异步消息，该调用返回DOS_OK并不表示接收进程成功接收。
          对于本CPU内的发送，返回DOS_OK表示消息成功放入接收进程的消息接收队列；
          对于非本CPU内的发送，返回DOS_OK表示消息成功放入到本CPU中与目的CPU相对
          应的消息发送队列；
*******************************************************************************************/
extern uint32_t  osal_msg_post_out(osal_pid_t  ItReceiver,uint32_t IwMsgId, void  * pvData, uint32_t   IdwDataSize);


/*从底层操作系统线程或者任务给上层进程发消息，支持本地和远程*/
extern uint32_t  osal_msg_deliver_from_task(osal_pid_t tReceiver,uint32_t  dwMsgId,void *pvData,uint32_t dwSize);





 /******************************************************************************************
   原语：osal_msg_broadcast
   功能：向本CPU的所有进程广播指定的消息
   参数：dwMsgId：需要广播的消息ID
******************************************************************************************/
extern uint32_t     osal_msg_broadcast(uint32_t  dwMsgId);





/************************************************************************************************
    原语：osal_msg_send_out
	功能：向指定进程发送同步消息
    参数：
          ItReceiver          接收消息的进程PID
          IwMsgId             消息号
          IpvData             消息体指针
          IwDataSize          消息体长度
          OpvAck              应答缓冲区指针
          IOpwAckSize         应答缓冲区长度的指针
          IwCount10ms         等待应答最大等待的时间，
                              以10毫秒为单位，总时间小于等于5000毫秒，
                              0表示使用默认的等待时间，处理机内部3000毫秒，
                              处理机之间5000毫秒
    返回：
          OSAL_OK                成功
          OSAL_ERROR_NODE        目标节点错误
          OSAL_ERROR_PNO         目标进程号错误
          OSAL_ERROR_SIZE        非法的消息长度
          OSAL_ERROR_COMM        网络通讯异常
          OSAL_ERROR             其他错误
    注解：
          （1）向进程发送同步消息，该调用返回DOS_OK并表示接收进程成功接收并返回。
          （2）IOpwAckSize在调用osal_msg_send_out前，由应用进程设置为pAck缓冲区的大小；
          （3）osal_msg_send_out返回DOS_OK时，*IOpwAckSize的值为对方实际返回的应答大小；
          （4）为了防止同步消息引起的死锁，不允许进程在处理同步消息时，向其他进程发送同步消息
************************************************************************************************/
extern uint32_t  osal_msg_send_out (osal_pid_t  ItReceiver,uint32_t  IwMsgId,void  *IpvData,uint16_t   IwDataSize,void  *OpvAck,uint16_t  *IOpwAckSize);





/***********************************************************************************************
    原语：osal_msg_set_ack_data_size
	功能：同步消息的接收方设置应答的大小
    参数：IwAckSize         应答缓冲区长度
    返回：
          OSAL_OK            成功
          OSAL_ERROR_SIZE      IwAckSize大于同步消息发送方期待的应答大小
          OSAL_ERROR           调用该原语的应用进程不在同步消息处理循环
    注解：
          该调用适用于需要返回应答大小可变的情况，对于应答大小固定不变
          并且应答大小等于发送方期待的应答大小时，可以不用设置应答大小。
***********************************************************************************************/
extern uint32_t  osal_msg_set_ack_data_size (uint16_t IwAckSize);


extern uint32_t   osal_msg_get_self_queue_status(uint32_t	* pdwSynUsedCount,uint32_t *pdwASynUsedCount);


extern uint32_t	osal_msg_get_queue_status_by_pno(uint32_t  dwPno, uint32_t * pdwSynFreeCount,uint32_t  *pdwASynFreeCount);



extern uint32_t osal_SendAsynMsg(osal_pid_t tReceiver, uint32_t dwMsgId, void *pvData, uint16_t wDataSize);




/*============================== 缓冲区原语 ===================================================*/

extern void * osal_mem_alloc(uint32_t  dwSize);   /*功能与C语言的malloc一样,后续为了规避系统内存泄露*/


extern uint32_t osal_mem_free(void * pBuf);     /*功能与C语言的free 一样，后续为了规避系统内存泄露*/





/*============================== 定时器原语 ================================================*/

/*********************************************************************************************
    原语：osal_timer_start_normal
    参数：
           ItmMsgId            定时器消息ID
           IdwParam            定时器参数
           IdwMilliSeconds       定时器时长,单位毫秒

    返回：
           Tid                     定时器id
           OSAL_ERR_INVALID_TIMER_ID      非法的定时器

    说明： 设置并启动普通（无名）定时器
********************************************************************************************/

extern uint32_t osal_timer_start_normal
(
   uint32_t  	ItmMsgId,
   unsigned long int 	IdwParam,
   uint32_t  	IdwMilliSeconds
);






/*********************************************************************************************
    原语：osal_timer_start_absolute
    参数：
           ItmMsgId            定时器消息ID
           IdwParam            定时器参数
           IptABSClock         绝对时钟结构指针

    返回：
           Tid                     定时器控id
           OSAL_ERR_INVALID_TIMER_ID      非法的定时器

    说明： 设置绝对定时器
********************************************************************************************/

extern uint32_t osal_timer_start_absolute
(
   uint32_t  		    	ItmMsgId,
   unsigned long int 	  	    	IdwParam,
   osal_clk_t  	      *	IptABSClock
);






/*********************************************************************************************
    原语：osal_timer_start_cycle
    参数：
           ItmMsgId            定时器消息ID
           IdwParam            定时器参数
           IdwMilliSeconds       定时器时长,单位毫秒

    返回：
           Tid                     定时器ID
           OSAL_ERR_INVALID_TIMER_ID      非法的定时器

    说明： 设置循环定时器
********************************************************************************************/
extern uint32_t osal_timer_start_cycle
(
   uint32_t           ItmMsgId,
   unsigned long int 			IdwParam,
   uint32_t           IdwMilliSeconds
);




/*********************************************************************************************
    原语：osal_timer_enable_tone
    参数：
          ucToneFlag: 0：关闭每20ms给RTP进程发送定时消息（降低系统负荷）
		              1：打开每20ms给RTP进程发送定时消息，RTP进程可以进行放音

    说明： 使能/关闭给RTP进程发送的20ms定时器消息
********************************************************************************************/

void  osal_timer_enable_tone(uint8_t ucToneFlag);




/********************************************************************************************
    原语：osal_timer_stop_by_tid
    参数：
          IdwTid            定时器id
    返回：
          OSAL_OK            成功
          OSAL_ERROR           失败
    说明：
          根据TID删除指定的定时器
********************************************************************************************/
extern uint32_t  osal_timer_stop_by_tid(uint32_t IdwTid);





/********************************************************************************************
    原语：osal_timer_get_param_by_tid
    参数：
          IdwTid            指定的定时器id
          OpiParam         返回的参数指针
    返回：
          OSAL_OK                 成功
          OSAL_INVALID_TIMER_ID    非法的定时器
    说明：
          返回指定定时器的可选参数
********************************************************************************************/
extern uint32_t   osal_timer_get_param_by_tid(uint32_t  IdwTid, unsigned long int * OpiParam);



/********************************************************************************************
    原语：osal_timer_get_current_tid
    参数：
          OpdwTid         存放当前超时定时器消息的TID
    返回：
          OSAL_OK                  成功
          OSAL_INVALID_TIMER_ID    非法的定时器
    说明：
          得到进程当前超时定时器的TID(只有在处理定时器消息时候才有意义)
********************************************************************************************/

extern  uint32_t   osal_timer_get_current_tid(uint32_t   *OpdwTid);




/*********************************************************************************************
    原语：osal_sys_get_timestamp
    参数：
          无
    返回：
          从1900.1.1 0:0:0 开始的秒计数
    说明：
          获取系统从1900.1.1 0:0:0 开始的秒计数(系统当前时间）
**********************************************************************************************/

extern uint32_t    osal_sys_get_timestamp();






/*********************************************************************************************
    原语(51)：osal_sys_get_current_second
    参数：
          无
    返回：
          从2000.1.1 0:0:0 开始的秒计数
    说明：
          获取系统从2000.1.1 0:0:0 开始的秒计数(系统当前时间）
**********************************************************************************************/
extern uint32_t osal_sys_get_current_second( );




/*********************************************************************************************
    原语：osal_sys_get_current_10ms
    参数：
          无
    返回：
         从开始的10ms 计数
    说明：
          获取系统从开始的10ms 计数(系统当前时间）,用于精确的时长要求
**********************************************************************************************/
extern uint32_t osal_sys_get_current_10ms( );




/*********************************************************************************************
    原语：osal_sys_get_tick
    参数：
          无
    返回：
         从系统开始的tick 计数
    说明：
          获取系统从开始的tick 计数(系统当前时间）,用于精确的时长要求
**********************************************************************************************/

uint32_t osal_sys_get_tick();



/**********************************************************************************************
    原语：osal_sys_get_current_clock
    参数：
          OptABSClock       绝对时钟指针
    返回：
          OSAL_OK            成功
          OSAL_ERROR           失败
    说明：
          获取系统当前时间的日期表示。
***********************************************************************************************/
extern uint32_t  osal_sys_get_current_clock(osal_clk_t* OptABSClock);






/**********************************************************************************************
    原语：osal_sys_get_real_clock
    参数：
          OptABSClock       绝对时钟指针
    返回：
          OSAL_OK            成功
          OSAL_ERROR           失败
    说明：
          获取系统当前时间的日期表示。 (  精确时间，直接访问系统的RTC)
***********************************************************************************************/

extern void   osal_sys_get_real_clock(osal_clk_t  *OptABSClock);



/**********************************************************************************************
    原语：osal_sys_exit()
    说明：
          退出当前系统
***********************************************************************************************/

extern void   osal_sys_exit(int32_t  iReason);





/**********************************************************************************************
    原语：osal_sys_set_current_clock
    参数：
          IptABSClock     绝对时钟指针
    返回：
          OSAL_OK          成功
          DOS_PARAM_ERR   参数非法
          OSAL_ERROR         信号量同步错。
    说明：
          设置系统当前时间的日期表示。
***********************************************************************************************/
extern uint32_t  osal_sys_set_current_clock(osal_clk_t* IptABSClock);



/*********************************************************************************************
    原语：osal_sys_clock_to_second
    参数：
          IptABSClock       绝对时钟指针
          OpdwTime          以秒表述的绝对时间的指针
    返回：
          OSAL_OK          成功
          DOS_PARAM_ERR   参数非法
    说明：
          把绝对时钟转换成以秒表述的绝对时间。
**********************************************************************************************/
extern uint32_t  osal_sys_clock_to_second(osal_clk_t* IptABSClock,uint32_t* OpdwTime);



/*********************************************************************************************
    原语：osal_sys_second_to_clock
    参数：
          IdwTime           以秒表述的绝对时间(以2001.1.1.0为基准)
          OptABSClock       绝对时钟指针

    返回：
          OSAL_OK          成功
          DOS_PARAM_ERR   参数非法
    说明：
         把以秒表述的绝对时间转换成绝对时钟。
**********************************************************************************************/
extern uint32_t  osal_sys_second_to_clock(uint32_t IdwTime,osal_clk_t * OptABSClock);




/*********************************************************************************************
    原语：osal_sys_clock_to_timestamp
    参数：
          IptABSClock       绝对时钟指针
          OpdwTime          以秒表述的绝对时间的指针
    返回：
          OSAL_OK          成功
          DOS_PARAM_ERR   参数非法
    说明：
          把绝对时钟转换成以秒表述的绝对时间。
**********************************************************************************************/

extern uint32_t  osal_sys_clock_to_timestamp(osal_clk_t* IptAbsClock,uint32_t* OpdwTime);




/*********************************************************************************************
    原语：osal_sys_get_rand
    参数：
          IdwMinValue : 最小值
          IdwMaxValue : 最大值
    返回：
          该范围内的一个随机数。
    说明：
          得到IdwMinValue和IdwMaxValue之间的一个随机数。
**********************************************************************************************/
extern uint32_t osal_sys_get_rand(uint32_t IdwMinValue,uint32_t IdwMaxValue);





extern void  osal_dbg_set_prn_param(uint32_t dwDbgPrnFlag,uint32_t dwPrnLevel);

#define osal_dbg_assert(condition)   \
do {                                 \
    if(!(condition))                   \
    {                                \
          osal_dbg_print(OSAL_DBG_PRN_LEVEL_2,"[osal]:Assertion Failed,File: %s ,line: %ld\n",__FILE__,__LINE__);	\
          OsPort_taskSuspend();      \
    }                                \
}while (0)

extern void  osal_dbg_print(uint32_t IdwPrnLevel,const char * IszPrnMsg,...);



extern uint32_t  osal_file_open
(
  char  * IszFileName,
  uint32_t   IdwFlag
);






/*********************************************************************************************
    原语：osal_file_close
    参数：
          IfdFileId：先前调用DOS_OpenFile所返回的文件标识
    返回：
          OSS _OK                        成功
	  DOS_INVALID_FD                     指定了无效的文件描述符
	  OSAL_ERROR                         发生未知原因的错误
    说明：
         关闭一个已打开的文件
**********************************************************************************************/

extern uint32_t   osal_file_close(uint32_t  IfdFileId);


/*********************************************************************************************
    原语：osal_file_create
    参数：
          IszFileName    文件名
          IdwFileSize    文件大小，给文件预留空间，以保证文件存储连续，
                         如不能确定文件大小，此参数置0
    返回：
          OSAL_OK                       成功
	  OSAL_ERROR_FILEEXIST        文件已经存在
	  OSAL_ERROR_INVALIDNAME     非法的文件名
	  OSAL_ERROR                 发生未知原因的错误
    说明：
         创建一个文件，如果要创建的文件名已经存在，则创建失败
**********************************************************************************************/

extern uint32_t  osal_file_create
(
   char    *  IszFileName
);



/*********************************************************************************************
    原语：osal_file_remove
    参数：
          IszFileName      待删除的文件名
    返回：
          OSAL_OK                     调用成功
	  OSAL_ERROR_INVALIDNAME        非法的文件名
	  OSAL_ERROR_FILENOTEXIST       要删除的文件不存在
	  OSAL_ERROR_FILELocked         要删除的文件被锁定
          OSAL_ERROR                    发生未知原因的错误
    说明：
         删除指定文件
**********************************************************************************************/

extern uint32_t   osal_file_remove(char *IszFileName);



/*********************************************************************************************
    原语：osal_file_lock
    参数：
          IfdFileId：先前调用DOS_OpenFile所返回的文件标识
    返回：
          OSAL_OK                   调用成功
          OSAL_ERROR_INVALIDFID   非法的FileId
          OSAL_ERROR_HAVELOCK     在该文件上已经存在了一个锁
          OSAL_ERROR              发生未知原因的错误
    说明：
         给文件系统中的一个文件加锁(独占锁)。
**********************************************************************************************/

extern uint32_t   osal_file_lock(uint32_t  IfdFileId);



/*********************************************************************************************
    原语：osal_file_unlock
    参数：
          IfdFileId：先前调用DOS_OpenFile所返回的文件标识
    返回：
          OSAL_OK                    调用成功
	  OSAL_ERROR_INVALIDFID        非法的FileId
	  OSAL_ERROR_NOLOCK            该文件尚未加锁
	  OSAL_ERROR                   发生未知原因的错误
    说明：
         给文件系统中的一个已经加锁的文件解锁。
**********************************************************************************************/
extern uint32_t   osal_file_unlock(uint32_t  IfdFileId);





/*********************************************************************************************
    原语：osal_file_length
    参数：
          IszFileName：指向路径名名字字符串的指针
    返回：
          文件长度(大于0)               调用成功
          OSAL_ERROR_INVALIDNAME           非法的路径名
    说明：
         获取一个文件的长度。
    调用说明:
         仅用于调试时简单输出信息，可通过宏去掉
**********************************************************************************************/

extern uint32_t osal_file_length
(
   char  *IszFileName
);






/*********************************************************************************************
    原语：osal_file_seek
    参数：
          IfdFileId：      先前调用OpenFile所得到的文件标识
	  IdwFilePosition ：重定位文件指针的参考位置,可以取值如下：
          		    OSAL_SEEK_SET 表示文件头
          		    DOS_SEEK_CUR表示文件当前位置
                            DOS_SEEK_ END表示文件尾
          IdwOffset：移动文件指针的字节数，正数向文件尾移，负数向文件头移
    返回：
          OSAL_OK                     调用成功
	  OSAL_ERROR_INVALIDFID         非法的FileId
	  OSAL_ERROR_INVALI_POSITION    非法的文件位置参数
	  OSAL_ERROR                    发生未知原因的错误
    说明：
         重定位文件指针
**********************************************************************************************/

extern uint32_t   osal_file_seek
(
   uint32_t  IfdFileId,
   uint32_t  IdwFilePosition,
   uint32_t  IdwOffset
);







/*********************************************************************************************
    原语：osal_file_read
    参数：
          IfileId          先前调用DOS_OpenFile所得到的文件标识
          IpvBuffer：      用来保存读出内容的缓冲区
          IdwCount：       要读出的字节数
          OpdwCountBeRead：实际读的字节数
    返回：
          OSAL_OK               调用成功
	  OSAL_ERROR_INVALIDFID   非法的FileId
	  OSAL_ERROR_FILELocked   要删除的文件被锁定
	  OSAL_ERROR              发生未知原因的错误
    说明：
         从一个已经被打开的文件中从当前位置读出一段数据。
**********************************************************************************************/

extern uint32_t   osal_file_read
(
   uint32_t  IfdFileId,
   void * IpvBuffer,
   uint32_t  IdwCount,
   uint32_t *OpdwCountBeRead
);







/*********************************************************************************************
    原语：osal_file_write
    参数：
          IfdFileId         先前调用DOS_OpenFile所得到的文件标识
	  IpdwBuffer        指向要写入内容所在内存缓冲区的首地址
	  IdwCount          要写入的字节数
	  OpdwCountBeWrite  实际写的字节数
    返回：
           OSAL_OK                 调用成功
	   OSAL_ERROR_INVALIDFID     非法的FileId
	   OSAL_ERROR_FILELocked     要删除的文件被锁定
	   OSAL_ERROR                发生未知原因的错误
    说明：
         向一个已经被打开的文件中从当前位置写入一段数据。
**********************************************************************************************/

extern uint32_t   osal_file_write
(
   uint32_t  IfdFileId,
   void * IpvBuffer,
   uint32_t  IdwCount,
   uint32_t *OpdwCountBeWrite
);




/*********************************************************************************************
    原语：osal_file_rename
    参数：
          IszOldFileName：指向源文件名字字符串的指针
	  IszNewFileName：指向目标文件名字字符串的指针
    返回：
          OSAL_OK                 调用成功
	  OSAL_ERROR_DESTEXIST      目标文件已经存在
	  OSAL_ERROR_SRCNOTEXIST    源文件不存在
	  OSAL_ERROR_FILELocked     要删除的文件被锁定
	  OSAL_ERROR                发生未知原因的错误
    说明：
         文件改名。
**********************************************************************************************/

extern uint32_t   osal_file_rename
(
    char  *IszOldFileName,
    char  *IszNewFileName
);



extern uint32_t   osal_dir_move(char *pszDestDirName,char *pszSrcDirName);





/*********************************************************************************************
    原语：osal_dir_rename
    参数：
          IszOldDirName：指向源目录名字字符串的指针
	  IszNewDirName：指向目标目录名字字符串的指针
    返回：
          OSAL_OK                调用成功
	  OSAL_ERROR_DESTEXIST     目标目录已经存在
	  OSAL_ERROR_SRCNOTEXIST   源目录不存在
	  OSAL_ERROR               发生未知原因的错误
    说明：
         目录改名。
**********************************************************************************************/

extern uint32_t   osal_dir_rename
(
   char  *IszOldDirName,
   char  *IszNewDirName
);



/*********************************************************************************************
    原语：osal_dir_create
    参数：
          IszDirName：指向路径名名字字符串的指针
    返回：
          OSAL_OK                     调用成功
          OSAL_ERROR_INVALIDNAME        非法的路径名
          OSAL_ERROR_DIRNOTEXIST        要创建的路径不存在
          OSAL_ERROR                    发生未知原因的错误
    说明：
         创建一个文件目录
**********************************************************************************************/

extern uint32_t   osal_dir_create(char  *IszDirName);





/*********************************************************************************************
    原语：osal_dir_remove
    参数：
          IszDirName：指向路径名名字字符串的指针
    返回：
          OSAL_OK                     调用成功
	  OSAL_ERROR_INVALIDNAME   非法的路径名
          OSAL_ERROR_DIRNOTEXIST   要删除的路径不存在
          OSAL_ERROR               发生未知原因的错误
    说明：
         进程调用该函数在前台文件系统中删除一个目录。
**********************************************************************************************/
extern uint32_t   osal_dir_remove(char  *IszDirName);



typedef struct tag_iprt_accept_msg_head
{
    int                  iListenSockFd;
    int                  iConnetSockFd;
	uint16_t             wRxStreamNum;
	uint16_t             wTxStreamNum;
    struct sockaddr_in   tPeerAddr;
    int                  dwAddrlen;
}iprt_accept_msg;



typedef struct tag_iprt_recv_msg_head
{
    int        iSockFd;

    uint32_t   dwSrcIpAddr;
    uint16_t   wSrcPort;
    uint32_t   dwDestIpAddr;
    uint16_t   wDestPort;
	uint16_t   wStreamId;  //sctp only

    uint32_t   dwPktLen;
    uint8_t    aucPktBuf[1];
}iprt_recv_msg;



typedef struct tag_iprt_close_msg_head
{
    int           iSockFd;
}iprt_close_msg;

extern int iprt_socket(int domain, int type, int protocol);
extern int iprt_bind(int sockfd, struct sockaddr *my_addr, int addrlen);
extern int iprt_connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
extern int iprt_listen(int sockfd, int backlog);
extern int iprt_send(int sockfd, char *data, int datalen, int flags);
extern int iprt_sendto(int   sockfd, char  *data, int    datalen, int flags, struct sockaddr *to, int    tolen);
extern int iprt_close(int sockfd);
extern int iprt_setsockopt(int sockfd, int level, int optname, char *optval, int optlen);
extern int iprt_getsockopt(int sockfd, int level, int optname, char *optval, unsigned int *optlen);
extern int iprt_getsockname(int sockfd, struct sockaddr *addr, unsigned int *addr_len);
extern int iprt_getpeername(int sockfd, struct sockaddr *addr, unsigned int *addr_len);

#endif   /* __OSAL_API_H__  */

