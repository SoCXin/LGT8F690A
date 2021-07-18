
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   main.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __ZFY_TRACKER_MQTT_APP__; CLOUD_SOLUTION =ZYF_TRACKER_MQTT_SOLUTION " in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/

/******************************************************************************

                  ��Ȩ���� (C) 2018, �촴��ũ�Ƽ�

 ******************************************************************************
  �ļ�   : main.c
  �汾   : ����
  ����   : LiCM
  ����   : 2018��02��24��
  ����   : �����ļ�
  ����   : M203C3.0�����嵥�̰߳汾

  �޸ļ�¼:
  ����   : 2018��02��24��
    ����   : LiCM
    ����   : �����ļ�

******************************************************************************/


#include "ql_type.h"
#include "ql_trace.h"
#include "ql_uart.h"
#include "ql_system.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "iot203c_msg.h"
#include "uart.h"
#include "ql_timer.h"
#include "user_mqtt.h"
#include "sys.h"
#include "fota_main.h"
#include "fota.h"
#include "gps.h"
#include "tiem.h"
#include "module_data.h"
#include "ril.h"
#include "ril_sim.h"
#include "ril_network.h"
#include "logfile.h"
#include "flash.h"
 

/*!
 * @brief ������:��ʼ��BSP,��ɶ�ʱ�ɼ����ݲ��ҷ��͵����Ʒ�
 * \n
 *
 * @param taskId ��ϢͨѶID
 * @return NULL
 * \n
 * @see
 */

void proc_main_task(s32 taskId)
{
	ST_MSG msg;	
    M203C_BSP_init();
    SYS_Parameter_Init();
	Led_Timer_init(TIMER_ID_USER_START + LED_TIMER_ID, LED_TIMER_MS);
	aliot_Timer_init(TIMER_ID_USER_START + AIOT_TIMER_ID, 1000);
    TEST_Timer_init(TIMER_ID_USER_START + HEART_TIMER_ID, 5000);
    s_iflashMutex = Ql_OS_CreateMutex("FlashMutex");
    while (1)
    {
        
        Ql_OS_GetMessage(&msg);
        switch (msg.message)
        {
	        case MSG_ID_RIL_READY:				
		        {
		            Ql_RIL_Initialize();				//�ȴ�RIL���ʼ�����,�ſ��Ե���AT CMDָ��,�û��������
		            DisUpdateRTC();
                    if(systemset.snuser)
                    {
                        Ql_SleepEnable();
                        JudgeGpsAndGSM();               //�ж��Ƿ��gps�Լ��Ƿ񱣴����ݵ�
                    }
                    Ql_Timer_Start(TIMER_ID_USER_START + HEART_TIMER_ID,5000,TRUE);     //�����������timer
                }
			 break;
            case MSG_IOT_TASK_START_JUDGE_GSMGPRS:
                JudgeGpsAndGSM();
                break;
			case MSG_ID_FTP_RESULT_IND:
	            mprintf("\r\n<##### Restart FTP 3s later #####>\r\n");
	            Ql_Sleep(3000);
	            ftp_downfile_timer(FotaUpdate_Start_TmrId, NULL);
             break;
        	case MSG_ID_RESET_MODULE_REQ:
	            mprintf("\r\n<##### Restart the module... #####>\r\n");
	            Ql_Sleep(50);
	            Ql_Reset(0);
			 break;
	        default:
	            break;
        }
    }
}

/*!
 * @brief MQTT����:��ʼ������TCP/IP��Ȼ�������ӵ����Ʒ�,����MQTT����������,���ҿ����Զ���ɶ�������
 * \n
 *
 * @param taskId ��ϢͨѶID
 * @return NULL
 * \n
 * @see
 */

void user_mqtt_task(s32 TaskId)
{
    ST_MSG msg;
    while (1)
    {
        Ql_OS_GetMessage(&msg);
        switch (msg.message)
        {
        	 
        	 case MSG_IOT_TASK_START_BLE:
				  Ble_InitConnect_Start();
				break;
			 case MSG_IOT_TASK_Get_ModuData:
				  GetModuleData();
				break;
			 case MSG_IOT_TASK_HeartDATA_MQTT:				
		           MqttPubUserHeartData();					//������������
		        break;
			 case MSG_IOT_TASK_SENDDATA_MQTT:
		           MqttPubUserSensorData();					//�����������ݵ�������
		           if(YorNupgrading == 0)                   //�Ƿ�������־
		           {
                        nextfaststart = 0;//
				        SendMsg2KernelForMcuOffP(&nextfaststart);				//�ɼ��������,����֪ͨMCU��
		           }
                break;
             case MSG_IOT_TASK_ERR_LOG:
				  	LoadLogbuf();                   //���ļ�������־����
					ClearLogbuf();                  //�����־����
				break;
	        default:
	            break;

        }

    }
}


/*!
 * @brief ��������:��ֹϵͳ����
 * \n
 *
 * @param taskId ��ϢͨѶID
 * @return NULL
 * \n
 * @see
 */

void user_overlook_task(s32 TaskId)
{
    ST_MSG msg;
    s32 iRet = 0;
    while (1)
    {
        Ql_OS_GetMessage(&msg);
        switch (msg.message)
        {
        	case MSG_IOT_TASK_MCU_OFF:
                 if(user_mqtt_clint_sta != USER_MQTT_GPRS_INIT && systemset.Interval < 3600)
                 {
				    MqttPubUserDisData();
                    GetSysTimetemp(&time);
                    SetSysTime(&time);      //��������ʱ���ֹʹ��epoʱ����ʱ����8��ʱ��
				    Ql_Sleep(3000);
                 }
                 else if(user_mqtt_clint_sta != USER_MQTT_GPRS_INIT)
                 {
                    MqttPubUserDisData();
                    Ql_Sleep(3000);
                 }
                 GetSysTime(&time);
				 //setmcu2poweroff203c(msg.param1);       //���͵�stm8����
				 setmcu2poweroff203c(&msg.param1);
				 Ql_PowerDown(1);
				break;
			 case MSG_IOT_TASK_REDATA_MQTT:
				   MqttPubUserReSensorData();
                break;
             case MSG_IOT_TASK_SENDSAVEDATA_MQTT:
				   MqttPubUserSaveSensorData();
                   Ql_Sleep(1000);              //����ʡȥ�����޷�����
                   SendMsg2KernelForIotData();
                break;
             case MSG_IOT_TASK_GET_LOCATION:
//                iRet = GetGpsLocation(45,1); //�źż��� ��ʹ��gps��λ ֱ�ӱ�����һ�ε����ݵ�
                SaveGpsdatapoint(msg.param1); //��ʹ�ö�λ���� 
				break;
            case MSG_IOT_TASK_PMTK_SET_STATIC_NAV_THD:
                Speed_threshold();
	        default:
	            break;
        }

    }
}

void proc_subtask1(s32 TaskId)
{
	ST_MSG msg;
    while (1)
    {
        Ql_OS_GetMessage(&msg);
        switch (msg.message)
       	{
        	case MSG_IOT_TASK_START_MQTT:
				  Mqtt_InitConnect_Start();
				  Iotdata_Timer_init(TIMER_ID_USER_START + IOTDATA_TIMER_ID, IOTDATA_TIMER_MS);
				  break;

			case MSG_IOT_TASK_OTA_MQTT:
				  fota_app();
				  break;
       	}
	}
}

