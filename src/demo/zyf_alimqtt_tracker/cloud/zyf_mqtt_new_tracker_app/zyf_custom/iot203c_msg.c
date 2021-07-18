/******************************************************************************

                  ��Ȩ���� (C) 2018, �촴��ũ�Ƽ�

 ******************************************************************************
  �ļ�   : iot203c_msg.c
  �汾   : ����
  ����   : LiCM
  ����   : 2018��02��24��
  ����   : �����ļ�
  ����   : ��������֮�䷢��msg��Ϣ����

  �޸ļ�¼:
  ����   : 2018��02��24��
    ����   : LiCM
    ����   : �����ļ�

******************************************************************************/

#include "ql_trace.h"
#include "ql_system.h"
#include "ql_gpio.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_uart.h"
#include "iot203c_msg.h"
#include "sys.h"


/*!
 * @brief ������Ϣ��MQTT��������MQTT��������
 * \n
 *
 * @param void
 * \n
 * @see
 */

void SendMsg2KernelFroCheckGSMorGPRS(void)
{
    Ql_OS_SendMessage(main_task_id, MSG_IOT_TASK_START_JUDGE_GSMGPRS, 0, 0);
}
void SendMsg2KernelForMqttStart(void)
{
	 if(systemset.snuser)								//ֻҪ������� SN����֮��,�ſ��Խ�������
	 	{
	 		Ql_OS_SendMessage(subtask1_id, MSG_IOT_TASK_START_MQTT, 0, 0);
	 	}
}


void SendMsg2KernelForBLEStart(void)
{
	 Ql_OS_SendMessage(user_mqtt_task_id, MSG_IOT_TASK_START_BLE, 0, 0);
}

void SendMsg2KernelForIotHeart(void)
{
	 Ql_OS_SendMessage(user_mqtt_task_id, MSG_IOT_TASK_HeartDATA_MQTT, 0, 0);
}


void SendMsg2KernelForIotData(void)
{
	 Ql_OS_SendMessage(user_mqtt_task_id, MSG_IOT_TASK_SENDDATA_MQTT, 0, 0);
}

void SendMsg2KernelForModuleData(void)
{
	 Ql_OS_SendMessage(user_mqtt_task_id, MSG_IOT_TASK_Get_ModuData, 0, 0);
}

void SendMsg2KernelForReIotData(void)
{
	 Ql_OS_SendMessage(user_overlook_task_id, MSG_IOT_TASK_REDATA_MQTT, 0, 0);
}


void SendMsg2KernelForOTAData(void)
{
	 Ql_OS_SendMessage(subtask1_id, MSG_IOT_TASK_OTA_MQTT, 0, 0);
}


void SendMsg2KernelForMcuOffP(void *nexttime)
{
	 Ql_OS_SendMessage(user_overlook_task_id, MSG_IOT_TASK_MCU_OFF, nexttime, 0);
}

void SendMsg2KernelForGetLocation(void* mode)
{
	 Ql_OS_SendMessage(user_overlook_task_id, MSG_IOT_TASK_GET_LOCATION, mode, 0);
}

void SendMsg2KernelForSendSaveData(void)
{
	 Ql_OS_SendMessage(user_overlook_task_id, MSG_IOT_TASK_SENDSAVEDATA_MQTT, 0, 0);
}
void SendMsg2KernelForSendPMTKData(void)
{
	 Ql_OS_SendMessage(user_overlook_task_id, MSG_IOT_TASK_PMTK_SET_STATIC_NAV_THD, 0, 0);
}


void SendMsg2KernelForErrorLog(void)
{
	 Ql_OS_SendMessage(user_mqtt_task_id, MSG_IOT_TASK_ERR_LOG, 0, 0);
}



