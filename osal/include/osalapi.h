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



/*���̺Ŷ��壬����һ���ṹ�壬��Ҫ��Ϊ�˷��������ܵ���չ����������ģ��� ��ʵ�ֲַ�ʽ����*/

typedef struct tagPID
{
    uint32_t         dwPno;
} osal_pid_t;






/************************************  ����ʱ�ӽṹ **************************************/
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
    ԭ�osal_task_get_pno_by_name
    ���ܣ��õ�ָ����������Ӧ�Ľ��̺�
    ������
          IszProcessName����������
    ���أ�
           OSAL_ERR_INVALID_PNO:��������ʧ��
           ����ֵ: �ý��������ڵĽ��̺�
******************************************************************************************************************/
extern uint32_t  osal_task_get_pno_by_name (char  *IszProcessName);
extern uint32_t  osal_task_get_name_by_pno(uint32_t dwPno, char * strProcName, uint8_t ucBufLen);



/*****************************************************************************************************************
    ԭ�osal_task_get_self_pno
    ���ܣ��õ������̵Ľ��̺�
    ���أ�
          OSAL_ERR_INVALID_PNO: ��������ʧ��
          ����ֵ: �ý��������ڵĽ��̺�
******************************************************************************************************************/
extern uint32_t   osal_task_get_self_pno ();




/*****************************************************************************************************************
    ԭ�osal_task_get_self_pid
    ���ܣ��õ������̵�PID
    ���أ�
          OSAL_OK: �������óɹ�,OptPid�д����������̵�PID
          ����ֵ: ��������ʧ��
******************************************************************************************************************/
extern uint32_t  osal_task_get_self_pid (osal_pid_t *OptPid);



/******************************************************************************************************************
    ԭ�osal_task_get_self_state
    ���ܣ��õ������̵ĵ�ǰ�û�״̬
    ���أ�
           OSAL_ERROR:����ʧ��
           ����: �����û�״̬
    ע�⣺
          ���̵��û�״̬��ͬ�ڲ���ϵͳ����Ľ���״̬������״̬��ָRUN��READY��PEND��SUSPEND�ȣ�
		  ���û�״̬�ǽ��̿��������߼���״̬����OS�޹ء����н��̵ĳ�ʼ�û�״̬��INIT_STATE(0)
*******************************************************************************************************************/
extern uint32_t   osal_task_get_self_state ( );





/*****************************************************************************************************************
    ԭ��: osal_task_get_state_by_pid
    ����: �õ�ָ�����̵ĵ�ǰ״̬
	������ItPid ��ָ�����̵�PID
    ���أ�
         OSAL_ERR��ʧ��
         ����:�����û�״̬
    ע�⣺
          ���̵��û�״̬��ͬ�ڲ���ϵͳ����Ľ���״̬������״̬��ָRUN��READY��PEND��SUSPEND�ȣ�
		  ���û�״̬�ǽ��̿��������߼���״̬����OS�޹ء����н��̵ĳ�ʼ�û�״̬��INIT_STATE(0)
******************************************************************************************************************/
extern uint32_t   osal_task_get_state_by_pid (osal_pid_t ItPid);






/******************************************************************************************
    ԭ�osal_task_set_next_state
	���ܣ����õ�ǰ���̵��û�״̬
    ������IUserState �������û�״̬
    ���أ�
          OSAL_OK: ����ִ�гɹ�
    ˵����
          ���õ�ǰ���ý��̵��û�״̬��osal_task_get_self_state���õķ���ֵ����IUserState
*******************************************************************************************/
extern uint32_t   osal_task_set_next_state (uint32_t  IbUserState);






/******************************************************************************************
    ԭ�osal_task_get_private_data_ptr
    ���ܣ����ؽ��̵�˽��������ָ��
    ���أ�
          NULL :   �������ô���
          ��NULL:  �ý��̵�˽��������ָ��
*******************************************************************************************/
extern void *  osal_task_get_private_data_ptr();

/******************************************************************************************
    ԭ��(12)��osal_task_get_private_data_ptr_by_pno
    ������
         dwPno
    ���أ�
         NULL :   �������ô���
         ��NULL:  �ý��̵�˽��������ָ��

    ˵�����õ�ָ�����̵�˽��������ָ��
*******************************************************************************************/
extern void *  osal_task_get_private_data_ptr_by_pno(uint32_t dwPno);







/******************************************************************************************
    ԭ�osal_task_get_private_data_size
    ���ܣ��õ����̵�˽�����������ȡ�
    ���أ�
           0�� ��������ʧ��
          >0:  ����˽���������Ĵ�С
*******************************************************************************************/
extern uint32_t  osal_task_get_private_data_size( );






/******************************************************************************************
    ԭ�osal_task_delay
	���ܣ�ʹ��ǰ����˯��һ��ʱ��,�����ͷ�CPUʹ��Ȩ
    ������IwDelayCount100ms: ˯��ʱ�䣬�� ����Ϊ��λ
    ���أ�
          OSAL_OK  :  ����ִ�гɹ�
          OSAL_ERROR :  ����ִ��ʧ��
    ע�⣺˯��ʱ����಻����3��
*******************************************************************************************/
extern uint32_t  osal_task_delay (uint32_t dwMillSecond);




/******************************************************************************************
    ԭ�osal_mutex_create
    ���ܣ�����������
    ���أ�
          OSAL_ERROR_INVALID_MUTEXID : ����������ʧ�ܡ�
          ����: ���ػ�������ʶ��
*******************************************************************************************/
extern uint32_t   osal_mutex_create ();




/******************************************************************************************
    ԭ�osal_mutex_delete
    ���ܣ�ɾ��ָ��������
	������IMutexID:������ID,��osal_mutex_create����
    ���أ�
          OSAL_OK : ������ɾ���ɹ���
          OSAL_ERROR_INVALID_MUTEXID : ��Ч�Ļ�����ID��
          OSAL_ERROR  : δ֪�Ĵ���
*******************************************************************************************/
extern uint32_t   osal_mutex_delete (uint32_t  IMutexID);


/******************************************************************************************
    ԭ�osal_mutex_enter
	����: �����ٽ���,����ʱ��������������̱�������
    ������IMutexID����������ʶ
    ���أ�
          OSAL_OK :			 ����ִ�гɹ���
          OSAL_ERROR_INVALID_MUTEXID  :     ��Ч���ٽ���ID
*******************************************************************************************/
extern uint32_t  osal_mutex_enter (uint32_t  IMutexID);


/******************************************************************************************
    ԭ�osal_mutex_leave
	���ܣ��뿪�ٽ���
    ������IMutexID����������ʶ
    ���أ�
           OSAL_OK :	 ����ִ�гɹ���
           OSAL_ERROR_INVALID_MUTEXID  :     ��Ч���ٽ���ID
******************************************************************************************/
extern uint32_t   osal_mutex_leave (uint32_t  IMutexID);




/******************************************************************************************
    ԭ�osal_sem_create
    ���ܣ������ź���
    ���أ�
          OSAL_ERROR_INVALID_SEMID : �����ź���ʧ�ܡ�
          >0      : ���ص��ź�����־����
*******************************************************************************************/
extern uint32_t   osal_sem_create();


/******************************************************************************************
    ԭ�osal_sem_delete
	���ܣ�ɾ��ָ�����ź���
    ������ISemID ���ź���ID
    ���أ�
          OSAL_OK : �ź���ɾ���ɹ���
          OSAL_ERROR_INVALID_SEMID : ��Ч���ź���ID��
          OSAL_ERROR: δ֪�Ĵ���
*******************************************************************************************/
extern uint32_t   osal_sem_delete (uint32_t  ISemID);



/******************************************************************************************
    ԭ�osal_sem_take
	���ܣ��ź���Take����
    ������
          ISemID             �ź����ı�ʶ
          IwMilliSeconds     ���̵����ź��������ȴ�ʱ�䣬�Ժ���Ϊ��λ����ʱ��С��3000���룬0��ʾ���ȴ�
    ���أ�
          OSAL_OK            		�ɹ�
          DOS_TIME_OUT      		���̳�ʱ
          OSAL_ERROR_INVALID_SEMID         ��ʶ��
          OSAL_ERROR           		��������
*******************************************************************************************/
extern uint32_t  osal_sem_take (uint32_t  ISemID, uint16_t IwMilliSeconds);


/*****************************************************************************************
    ԭ�osal_sem_give
	���ܣ��ź���Give����
    ������ISemID���ź����ı�ʶ
    ���أ�
          DOS_OK���ɹ�
          OSAL_ERROR_INVALID_SEMID����ʶ��
******************************************************************************************/
extern uint32_t  osal_sem_give (uint32_t   ISemID);






/**********************************************************************************************
    ԭ�osal_msg_malloc
	���ܣ�Ϊ��Ϣ���뻺����
    ������
          IdwBufSize            ��������С
    ���أ�
          NULL              ʧ��
          ��NULL            ������ָ��
***********************************************************************************************/
extern void * osal_msg_malloc_buf (uint32_t IdwBufSize);



/**********************************************************************************************
    ԭ�osal_msg_free
	���ܣ��ͷ���osal_msg_malloc�õ��Ļ�����
    ������IpvBuf��������ָ��
    ���أ�
          OSAL_OK :       �ɹ�
          OSAL_ERROR:       ʧ��
*********************************************************************************************/
extern uint32_t  osal_msg_free_buf (void *IpvBuf);





/**********************************************************************************************
    ԭ�osal_msg_get_buf_count
	���ܣ��õ�ָ������������Ŀ
    ������IdwBufSize : ��������С
    ���أ�
          OSAL_OK :       �ɹ�
          OSAL_ERROR:       ʧ��
*********************************************************************************************/

extern uint32_t  osal_msg_get_buf_count(uint32_t IdwBufSize,uint32_t * pdwTotalNum,uint32_t *pdwFreeNum);





/******************************************************************************************
    ԭ�osal_msg_get_sender
	���ܣ����ص�ǰ������Ϣ�ķ��ͽ��̵�PID
    ������ptPid��������Ϣ���ͽ��̵�PID
    ���أ�
          OSAL_OK ���ɹ�
          OSAL_ERR��ʧ�ܣ����ý��̷�OSAL��װ�Ľ���
*******************************************************************************************/
extern uint32_t  osal_msg_get_sender (osal_pid_t *OptPid);



/*******************************************************************************************
    ԭ�osal_msg_get_id
	���ܣ����ص�ǰ������Ϣ����Ϣ��
    ���أ�
          0  ��      ��������ʧ��
          >0 :       ��ϢID��
********************************************************************************************/
extern uint32_t  osal_msg_get_id ();






/*******************************************************************************************
    ԭ�osal_msg_get_data
	���ܣ�����ָ��ǰ������Ϣ����Ϣ���ݵ�ָ��
    ���أ�
          NULL    :  ��������ʧ��
          ��NULL  :  ��Ϣ������ָ��
********************************************************************************************/
extern void * osal_msg_get_data ();





/*******************************************************************************************
    ԭ�osal_msg_get_data_length
	���ܣ����ص�ǰ������Ϣ�ĳ���
    ���أ���Ϣ����
********************************************************************************************/
extern uint32_t osal_msg_get_data_length ();





/*******************************************************************************************
    ԭ�osal_msg_get_ack_data_ptr
    ���ܣ��õ�ͬ����ϢӦ��������ָ��
    ���أ�
          NULL    :  ��������ʧ��
          ��NULL  :  ��Ϣ������ָ��
********************************************************************************************/
extern void  * osal_msg_get_ack_data_ptr( );





/*******************************************************************************************
    ԭ�osal_msg_get_ack_data_length
	���ܣ��õ�ͬ����ϢӦ������������
    ���أ�Ӧ������������
********************************************************************************************/
extern uint16_t  osal_msg_get_ack_data_length( );




/********************************************************************************************
    ԭ�osal_msg_post_out
	���ܣ���ָ�����̷����첽��Ϣ
    ������
          ItReceiver          ������Ϣ�Ľ���PID
          IwMsgId              ��Ϣ��
          IpvData             ��Ϣ��ָ��
          IdwDataSize         ��Ϣ�峤��
    ���أ�
          OSAL_OK               �ɹ�
          OSAL_ERROR_NODE        Ŀ��ڵ����
          OSAL_ERROR_PNO         Ŀ����̺Ŵ���
          OSAL_ERROR_SIZE        �Ƿ�����Ϣ����
          OSAL_ERROR_COMM        ����ͨѶ�쳣
          OSAL_ERROR             ��������
    ע�⣺
          ����̷����첽��Ϣ���õ��÷���DOS_OK������ʾ���ս��̳ɹ����ա�
          ���ڱ�CPU�ڵķ��ͣ�����DOS_OK��ʾ��Ϣ�ɹ�������ս��̵���Ϣ���ն��У�
          ���ڷǱ�CPU�ڵķ��ͣ�����DOS_OK��ʾ��Ϣ�ɹ����뵽��CPU����Ŀ��CPU���
          Ӧ����Ϣ���Ͷ��У�
*******************************************************************************************/
extern uint32_t  osal_msg_post_out(osal_pid_t  ItReceiver,uint32_t IwMsgId, void  * pvData, uint32_t   IdwDataSize);


/*�ӵײ����ϵͳ�̻߳���������ϲ���̷���Ϣ��֧�ֱ��غ�Զ��*/
extern uint32_t  osal_msg_deliver_from_task(osal_pid_t tReceiver,uint32_t  dwMsgId,void *pvData,uint32_t dwSize);





 /******************************************************************************************
   ԭ�osal_msg_broadcast
   ���ܣ���CPU�����н��̹㲥ָ������Ϣ
   ������dwMsgId����Ҫ�㲥����ϢID
******************************************************************************************/
extern uint32_t     osal_msg_broadcast(uint32_t  dwMsgId);





/************************************************************************************************
    ԭ�osal_msg_send_out
	���ܣ���ָ�����̷���ͬ����Ϣ
    ������
          ItReceiver          ������Ϣ�Ľ���PID
          IwMsgId             ��Ϣ��
          IpvData             ��Ϣ��ָ��
          IwDataSize          ��Ϣ�峤��
          OpvAck              Ӧ�𻺳���ָ��
          IOpwAckSize         Ӧ�𻺳������ȵ�ָ��
          IwCount10ms         �ȴ�Ӧ�����ȴ���ʱ�䣬
                              ��10����Ϊ��λ����ʱ��С�ڵ���5000���룬
                              0��ʾʹ��Ĭ�ϵĵȴ�ʱ�䣬������ڲ�3000���룬
                              �����֮��5000����
    ���أ�
          OSAL_OK                �ɹ�
          OSAL_ERROR_NODE        Ŀ��ڵ����
          OSAL_ERROR_PNO         Ŀ����̺Ŵ���
          OSAL_ERROR_SIZE        �Ƿ�����Ϣ����
          OSAL_ERROR_COMM        ����ͨѶ�쳣
          OSAL_ERROR             ��������
    ע�⣺
          ��1������̷���ͬ����Ϣ���õ��÷���DOS_OK����ʾ���ս��̳ɹ����ղ����ء�
          ��2��IOpwAckSize�ڵ���osal_msg_send_outǰ����Ӧ�ý�������ΪpAck�������Ĵ�С��
          ��3��osal_msg_send_out����DOS_OKʱ��*IOpwAckSize��ֵΪ�Է�ʵ�ʷ��ص�Ӧ���С��
          ��4��Ϊ�˷�ֹͬ����Ϣ���������������������ڴ���ͬ����Ϣʱ�����������̷���ͬ����Ϣ
************************************************************************************************/
extern uint32_t  osal_msg_send_out (osal_pid_t  ItReceiver,uint32_t  IwMsgId,void  *IpvData,uint16_t   IwDataSize,void  *OpvAck,uint16_t  *IOpwAckSize);





/***********************************************************************************************
    ԭ�osal_msg_set_ack_data_size
	���ܣ�ͬ����Ϣ�Ľ��շ�����Ӧ��Ĵ�С
    ������IwAckSize         Ӧ�𻺳�������
    ���أ�
          OSAL_OK            �ɹ�
          OSAL_ERROR_SIZE      IwAckSize����ͬ����Ϣ���ͷ��ڴ���Ӧ���С
          OSAL_ERROR           ���ø�ԭ���Ӧ�ý��̲���ͬ����Ϣ����ѭ��
    ע�⣺
          �õ�����������Ҫ����Ӧ���С�ɱ�����������Ӧ���С�̶�����
          ����Ӧ���С���ڷ��ͷ��ڴ���Ӧ���Сʱ�����Բ�������Ӧ���С��
***********************************************************************************************/
extern uint32_t  osal_msg_set_ack_data_size (uint16_t IwAckSize);


extern uint32_t   osal_msg_get_self_queue_status(uint32_t	* pdwSynUsedCount,uint32_t *pdwASynUsedCount);


extern uint32_t	osal_msg_get_queue_status_by_pno(uint32_t  dwPno, uint32_t * pdwSynFreeCount,uint32_t  *pdwASynFreeCount);



extern uint32_t osal_SendAsynMsg(osal_pid_t tReceiver, uint32_t dwMsgId, void *pvData, uint16_t wDataSize);




/*============================== ������ԭ�� ===================================================*/

extern void * osal_mem_alloc(uint32_t  dwSize);   /*������C���Ե�mallocһ��,����Ϊ�˹��ϵͳ�ڴ�й¶*/


extern uint32_t osal_mem_free(void * pBuf);     /*������C���Ե�free һ��������Ϊ�˹��ϵͳ�ڴ�й¶*/





/*============================== ��ʱ��ԭ�� ================================================*/

/*********************************************************************************************
    ԭ�osal_timer_start_normal
    ������
           ItmMsgId            ��ʱ����ϢID
           IdwParam            ��ʱ������
           IdwMilliSeconds       ��ʱ��ʱ��,��λ����

    ���أ�
           Tid                     ��ʱ��id
           OSAL_ERR_INVALID_TIMER_ID      �Ƿ��Ķ�ʱ��

    ˵���� ���ò�������ͨ����������ʱ��
********************************************************************************************/

extern uint32_t osal_timer_start_normal
(
   uint32_t  	ItmMsgId,
   unsigned long int 	IdwParam,
   uint32_t  	IdwMilliSeconds
);






/*********************************************************************************************
    ԭ�osal_timer_start_absolute
    ������
           ItmMsgId            ��ʱ����ϢID
           IdwParam            ��ʱ������
           IptABSClock         ����ʱ�ӽṹָ��

    ���أ�
           Tid                     ��ʱ����id
           OSAL_ERR_INVALID_TIMER_ID      �Ƿ��Ķ�ʱ��

    ˵���� ���þ��Զ�ʱ��
********************************************************************************************/

extern uint32_t osal_timer_start_absolute
(
   uint32_t  		    	ItmMsgId,
   unsigned long int 	  	    	IdwParam,
   osal_clk_t  	      *	IptABSClock
);






/*********************************************************************************************
    ԭ�osal_timer_start_cycle
    ������
           ItmMsgId            ��ʱ����ϢID
           IdwParam            ��ʱ������
           IdwMilliSeconds       ��ʱ��ʱ��,��λ����

    ���أ�
           Tid                     ��ʱ��ID
           OSAL_ERR_INVALID_TIMER_ID      �Ƿ��Ķ�ʱ��

    ˵���� ����ѭ����ʱ��
********************************************************************************************/
extern uint32_t osal_timer_start_cycle
(
   uint32_t           ItmMsgId,
   unsigned long int 			IdwParam,
   uint32_t           IdwMilliSeconds
);




/*********************************************************************************************
    ԭ�osal_timer_enable_tone
    ������
          ucToneFlag: 0���ر�ÿ20ms��RTP���̷��Ͷ�ʱ��Ϣ������ϵͳ���ɣ�
		              1����ÿ20ms��RTP���̷��Ͷ�ʱ��Ϣ��RTP���̿��Խ��з���

    ˵���� ʹ��/�رո�RTP���̷��͵�20ms��ʱ����Ϣ
********************************************************************************************/

void  osal_timer_enable_tone(uint8_t ucToneFlag);




/********************************************************************************************
    ԭ�osal_timer_stop_by_tid
    ������
          IdwTid            ��ʱ��id
    ���أ�
          OSAL_OK            �ɹ�
          OSAL_ERROR           ʧ��
    ˵����
          ����TIDɾ��ָ���Ķ�ʱ��
********************************************************************************************/
extern uint32_t  osal_timer_stop_by_tid(uint32_t IdwTid);





/********************************************************************************************
    ԭ�osal_timer_get_param_by_tid
    ������
          IdwTid            ָ���Ķ�ʱ��id
          OpiParam         ���صĲ���ָ��
    ���أ�
          OSAL_OK                 �ɹ�
          OSAL_INVALID_TIMER_ID    �Ƿ��Ķ�ʱ��
    ˵����
          ����ָ����ʱ���Ŀ�ѡ����
********************************************************************************************/
extern uint32_t   osal_timer_get_param_by_tid(uint32_t  IdwTid, unsigned long int * OpiParam);



/********************************************************************************************
    ԭ�osal_timer_get_current_tid
    ������
          OpdwTid         ��ŵ�ǰ��ʱ��ʱ����Ϣ��TID
    ���أ�
          OSAL_OK                  �ɹ�
          OSAL_INVALID_TIMER_ID    �Ƿ��Ķ�ʱ��
    ˵����
          �õ����̵�ǰ��ʱ��ʱ����TID(ֻ���ڴ���ʱ����Ϣʱ���������)
********************************************************************************************/

extern  uint32_t   osal_timer_get_current_tid(uint32_t   *OpdwTid);




/*********************************************************************************************
    ԭ�osal_sys_get_timestamp
    ������
          ��
    ���أ�
          ��1900.1.1 0:0:0 ��ʼ�������
    ˵����
          ��ȡϵͳ��1900.1.1 0:0:0 ��ʼ�������(ϵͳ��ǰʱ�䣩
**********************************************************************************************/

extern uint32_t    osal_sys_get_timestamp();






/*********************************************************************************************
    ԭ��(51)��osal_sys_get_current_second
    ������
          ��
    ���أ�
          ��2000.1.1 0:0:0 ��ʼ�������
    ˵����
          ��ȡϵͳ��2000.1.1 0:0:0 ��ʼ�������(ϵͳ��ǰʱ�䣩
**********************************************************************************************/
extern uint32_t osal_sys_get_current_second( );




/*********************************************************************************************
    ԭ�osal_sys_get_current_10ms
    ������
          ��
    ���أ�
         �ӿ�ʼ��10ms ����
    ˵����
          ��ȡϵͳ�ӿ�ʼ��10ms ����(ϵͳ��ǰʱ�䣩,���ھ�ȷ��ʱ��Ҫ��
**********************************************************************************************/
extern uint32_t osal_sys_get_current_10ms( );




/*********************************************************************************************
    ԭ�osal_sys_get_tick
    ������
          ��
    ���أ�
         ��ϵͳ��ʼ��tick ����
    ˵����
          ��ȡϵͳ�ӿ�ʼ��tick ����(ϵͳ��ǰʱ�䣩,���ھ�ȷ��ʱ��Ҫ��
**********************************************************************************************/

uint32_t osal_sys_get_tick();



/**********************************************************************************************
    ԭ�osal_sys_get_current_clock
    ������
          OptABSClock       ����ʱ��ָ��
    ���أ�
          OSAL_OK            �ɹ�
          OSAL_ERROR           ʧ��
    ˵����
          ��ȡϵͳ��ǰʱ������ڱ�ʾ��
***********************************************************************************************/
extern uint32_t  osal_sys_get_current_clock(osal_clk_t* OptABSClock);






/**********************************************************************************************
    ԭ�osal_sys_get_real_clock
    ������
          OptABSClock       ����ʱ��ָ��
    ���أ�
          OSAL_OK            �ɹ�
          OSAL_ERROR           ʧ��
    ˵����
          ��ȡϵͳ��ǰʱ������ڱ�ʾ�� (  ��ȷʱ�䣬ֱ�ӷ���ϵͳ��RTC)
***********************************************************************************************/

extern void   osal_sys_get_real_clock(osal_clk_t  *OptABSClock);



/**********************************************************************************************
    ԭ�osal_sys_exit()
    ˵����
          �˳���ǰϵͳ
***********************************************************************************************/

extern void   osal_sys_exit(int32_t  iReason);





/**********************************************************************************************
    ԭ�osal_sys_set_current_clock
    ������
          IptABSClock     ����ʱ��ָ��
    ���أ�
          OSAL_OK          �ɹ�
          DOS_PARAM_ERR   �����Ƿ�
          OSAL_ERROR         �ź���ͬ����
    ˵����
          ����ϵͳ��ǰʱ������ڱ�ʾ��
***********************************************************************************************/
extern uint32_t  osal_sys_set_current_clock(osal_clk_t* IptABSClock);



/*********************************************************************************************
    ԭ�osal_sys_clock_to_second
    ������
          IptABSClock       ����ʱ��ָ��
          OpdwTime          ��������ľ���ʱ���ָ��
    ���أ�
          OSAL_OK          �ɹ�
          DOS_PARAM_ERR   �����Ƿ�
    ˵����
          �Ѿ���ʱ��ת������������ľ���ʱ�䡣
**********************************************************************************************/
extern uint32_t  osal_sys_clock_to_second(osal_clk_t* IptABSClock,uint32_t* OpdwTime);



/*********************************************************************************************
    ԭ�osal_sys_second_to_clock
    ������
          IdwTime           ��������ľ���ʱ��(��2001.1.1.0Ϊ��׼)
          OptABSClock       ����ʱ��ָ��

    ���أ�
          OSAL_OK          �ɹ�
          DOS_PARAM_ERR   �����Ƿ�
    ˵����
         ����������ľ���ʱ��ת���ɾ���ʱ�ӡ�
**********************************************************************************************/
extern uint32_t  osal_sys_second_to_clock(uint32_t IdwTime,osal_clk_t * OptABSClock);




/*********************************************************************************************
    ԭ�osal_sys_clock_to_timestamp
    ������
          IptABSClock       ����ʱ��ָ��
          OpdwTime          ��������ľ���ʱ���ָ��
    ���أ�
          OSAL_OK          �ɹ�
          DOS_PARAM_ERR   �����Ƿ�
    ˵����
          �Ѿ���ʱ��ת������������ľ���ʱ�䡣
**********************************************************************************************/

extern uint32_t  osal_sys_clock_to_timestamp(osal_clk_t* IptAbsClock,uint32_t* OpdwTime);




/*********************************************************************************************
    ԭ�osal_sys_get_rand
    ������
          IdwMinValue : ��Сֵ
          IdwMaxValue : ���ֵ
    ���أ�
          �÷�Χ�ڵ�һ���������
    ˵����
          �õ�IdwMinValue��IdwMaxValue֮���һ���������
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
    ԭ�osal_file_close
    ������
          IfdFileId����ǰ����DOS_OpenFile�����ص��ļ���ʶ
    ���أ�
          OSS _OK                        �ɹ�
	  DOS_INVALID_FD                     ָ������Ч���ļ�������
	  OSAL_ERROR                         ����δ֪ԭ��Ĵ���
    ˵����
         �ر�һ���Ѵ򿪵��ļ�
**********************************************************************************************/

extern uint32_t   osal_file_close(uint32_t  IfdFileId);


/*********************************************************************************************
    ԭ�osal_file_create
    ������
          IszFileName    �ļ���
          IdwFileSize    �ļ���С�����ļ�Ԥ���ռ䣬�Ա�֤�ļ��洢������
                         �粻��ȷ���ļ���С���˲�����0
    ���أ�
          OSAL_OK                       �ɹ�
	  OSAL_ERROR_FILEEXIST        �ļ��Ѿ�����
	  OSAL_ERROR_INVALIDNAME     �Ƿ����ļ���
	  OSAL_ERROR                 ����δ֪ԭ��Ĵ���
    ˵����
         ����һ���ļ������Ҫ�������ļ����Ѿ����ڣ��򴴽�ʧ��
**********************************************************************************************/

extern uint32_t  osal_file_create
(
   char    *  IszFileName
);



/*********************************************************************************************
    ԭ�osal_file_remove
    ������
          IszFileName      ��ɾ�����ļ���
    ���أ�
          OSAL_OK                     ���óɹ�
	  OSAL_ERROR_INVALIDNAME        �Ƿ����ļ���
	  OSAL_ERROR_FILENOTEXIST       Ҫɾ�����ļ�������
	  OSAL_ERROR_FILELocked         Ҫɾ�����ļ�������
          OSAL_ERROR                    ����δ֪ԭ��Ĵ���
    ˵����
         ɾ��ָ���ļ�
**********************************************************************************************/

extern uint32_t   osal_file_remove(char *IszFileName);



/*********************************************************************************************
    ԭ�osal_file_lock
    ������
          IfdFileId����ǰ����DOS_OpenFile�����ص��ļ���ʶ
    ���أ�
          OSAL_OK                   ���óɹ�
          OSAL_ERROR_INVALIDFID   �Ƿ���FileId
          OSAL_ERROR_HAVELOCK     �ڸ��ļ����Ѿ�������һ����
          OSAL_ERROR              ����δ֪ԭ��Ĵ���
    ˵����
         ���ļ�ϵͳ�е�һ���ļ�����(��ռ��)��
**********************************************************************************************/

extern uint32_t   osal_file_lock(uint32_t  IfdFileId);



/*********************************************************************************************
    ԭ�osal_file_unlock
    ������
          IfdFileId����ǰ����DOS_OpenFile�����ص��ļ���ʶ
    ���أ�
          OSAL_OK                    ���óɹ�
	  OSAL_ERROR_INVALIDFID        �Ƿ���FileId
	  OSAL_ERROR_NOLOCK            ���ļ���δ����
	  OSAL_ERROR                   ����δ֪ԭ��Ĵ���
    ˵����
         ���ļ�ϵͳ�е�һ���Ѿ��������ļ�������
**********************************************************************************************/
extern uint32_t   osal_file_unlock(uint32_t  IfdFileId);





/*********************************************************************************************
    ԭ�osal_file_length
    ������
          IszFileName��ָ��·���������ַ�����ָ��
    ���أ�
          �ļ�����(����0)               ���óɹ�
          OSAL_ERROR_INVALIDNAME           �Ƿ���·����
    ˵����
         ��ȡһ���ļ��ĳ��ȡ�
    ����˵��:
         �����ڵ���ʱ�������Ϣ����ͨ����ȥ��
**********************************************************************************************/

extern uint32_t osal_file_length
(
   char  *IszFileName
);






/*********************************************************************************************
    ԭ�osal_file_seek
    ������
          IfdFileId��      ��ǰ����OpenFile���õ����ļ���ʶ
	  IdwFilePosition ���ض�λ�ļ�ָ��Ĳο�λ��,����ȡֵ���£�
          		    OSAL_SEEK_SET ��ʾ�ļ�ͷ
          		    DOS_SEEK_CUR��ʾ�ļ���ǰλ��
                            DOS_SEEK_ END��ʾ�ļ�β
          IdwOffset���ƶ��ļ�ָ����ֽ������������ļ�β�ƣ��������ļ�ͷ��
    ���أ�
          OSAL_OK                     ���óɹ�
	  OSAL_ERROR_INVALIDFID         �Ƿ���FileId
	  OSAL_ERROR_INVALI_POSITION    �Ƿ����ļ�λ�ò���
	  OSAL_ERROR                    ����δ֪ԭ��Ĵ���
    ˵����
         �ض�λ�ļ�ָ��
**********************************************************************************************/

extern uint32_t   osal_file_seek
(
   uint32_t  IfdFileId,
   uint32_t  IdwFilePosition,
   uint32_t  IdwOffset
);







/*********************************************************************************************
    ԭ�osal_file_read
    ������
          IfileId          ��ǰ����DOS_OpenFile���õ����ļ���ʶ
          IpvBuffer��      ��������������ݵĻ�����
          IdwCount��       Ҫ�������ֽ���
          OpdwCountBeRead��ʵ�ʶ����ֽ���
    ���أ�
          OSAL_OK               ���óɹ�
	  OSAL_ERROR_INVALIDFID   �Ƿ���FileId
	  OSAL_ERROR_FILELocked   Ҫɾ�����ļ�������
	  OSAL_ERROR              ����δ֪ԭ��Ĵ���
    ˵����
         ��һ���Ѿ����򿪵��ļ��дӵ�ǰλ�ö���һ�����ݡ�
**********************************************************************************************/

extern uint32_t   osal_file_read
(
   uint32_t  IfdFileId,
   void * IpvBuffer,
   uint32_t  IdwCount,
   uint32_t *OpdwCountBeRead
);







/*********************************************************************************************
    ԭ�osal_file_write
    ������
          IfdFileId         ��ǰ����DOS_OpenFile���õ����ļ���ʶ
	  IpdwBuffer        ָ��Ҫд�����������ڴ滺�������׵�ַ
	  IdwCount          Ҫд����ֽ���
	  OpdwCountBeWrite  ʵ��д���ֽ���
    ���أ�
           OSAL_OK                 ���óɹ�
	   OSAL_ERROR_INVALIDFID     �Ƿ���FileId
	   OSAL_ERROR_FILELocked     Ҫɾ�����ļ�������
	   OSAL_ERROR                ����δ֪ԭ��Ĵ���
    ˵����
         ��һ���Ѿ����򿪵��ļ��дӵ�ǰλ��д��һ�����ݡ�
**********************************************************************************************/

extern uint32_t   osal_file_write
(
   uint32_t  IfdFileId,
   void * IpvBuffer,
   uint32_t  IdwCount,
   uint32_t *OpdwCountBeWrite
);




/*********************************************************************************************
    ԭ�osal_file_rename
    ������
          IszOldFileName��ָ��Դ�ļ������ַ�����ָ��
	  IszNewFileName��ָ��Ŀ���ļ������ַ�����ָ��
    ���أ�
          OSAL_OK                 ���óɹ�
	  OSAL_ERROR_DESTEXIST      Ŀ���ļ��Ѿ�����
	  OSAL_ERROR_SRCNOTEXIST    Դ�ļ�������
	  OSAL_ERROR_FILELocked     Ҫɾ�����ļ�������
	  OSAL_ERROR                ����δ֪ԭ��Ĵ���
    ˵����
         �ļ�������
**********************************************************************************************/

extern uint32_t   osal_file_rename
(
    char  *IszOldFileName,
    char  *IszNewFileName
);



extern uint32_t   osal_dir_move(char *pszDestDirName,char *pszSrcDirName);





/*********************************************************************************************
    ԭ�osal_dir_rename
    ������
          IszOldDirName��ָ��ԴĿ¼�����ַ�����ָ��
	  IszNewDirName��ָ��Ŀ��Ŀ¼�����ַ�����ָ��
    ���أ�
          OSAL_OK                ���óɹ�
	  OSAL_ERROR_DESTEXIST     Ŀ��Ŀ¼�Ѿ�����
	  OSAL_ERROR_SRCNOTEXIST   ԴĿ¼������
	  OSAL_ERROR               ����δ֪ԭ��Ĵ���
    ˵����
         Ŀ¼������
**********************************************************************************************/

extern uint32_t   osal_dir_rename
(
   char  *IszOldDirName,
   char  *IszNewDirName
);



/*********************************************************************************************
    ԭ�osal_dir_create
    ������
          IszDirName��ָ��·���������ַ�����ָ��
    ���أ�
          OSAL_OK                     ���óɹ�
          OSAL_ERROR_INVALIDNAME        �Ƿ���·����
          OSAL_ERROR_DIRNOTEXIST        Ҫ������·��������
          OSAL_ERROR                    ����δ֪ԭ��Ĵ���
    ˵����
         ����һ���ļ�Ŀ¼
**********************************************************************************************/

extern uint32_t   osal_dir_create(char  *IszDirName);





/*********************************************************************************************
    ԭ�osal_dir_remove
    ������
          IszDirName��ָ��·���������ַ�����ָ��
    ���أ�
          OSAL_OK                     ���óɹ�
	  OSAL_ERROR_INVALIDNAME   �Ƿ���·����
          OSAL_ERROR_DIRNOTEXIST   Ҫɾ����·��������
          OSAL_ERROR               ����δ֪ԭ��Ĵ���
    ˵����
         ���̵��øú�����ǰ̨�ļ�ϵͳ��ɾ��һ��Ŀ¼��
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

