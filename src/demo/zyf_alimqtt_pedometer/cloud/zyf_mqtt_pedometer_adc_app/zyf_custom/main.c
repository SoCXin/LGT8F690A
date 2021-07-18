
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
 *     Set "C_PREDEF=-D __ZFY_MQTT_PEDOMETER__; CLOUD_SOLUTION =ZYF_MQTT_PEDOMETER_SOLUTION " in gcc_makefile file. And compile the 
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
#include "ads1151.h"
#include "module_data.h"
#include "mpu6050.h"
#include "gps.h"
#include "tiem.h"
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
                    if(gpspowersta == 0)
                    {
                        gpspowersta = 1;
                        GpsOpen();
                    }
                    Ql_SleepEnable();  
		            SendMsg2KernelForBLEStart();		//������Ϣ��BLE����,����BLE����
		        }
			 break;
			case MSG_IOT_TASK_GET_SENSOR_DATA:				
		        {
		           //GetUserSensorData();					//��ȡ���贫��������
		        }
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
			case MSG_IOT_TASK_START_JUDGE_GSMGPRS:
                JudgeGpsAndGSM();
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
        	 
        	 case MSG_IOT_TASK_START_BLE:           //��������
				  Ble_InitConnect_Start();
				  SendMsg2KernelForMqttStart();     //��ʼ��������
				break;
			 case MSG_IOT_TASK_Get_ModuData:
				  GetModuleData();                  //��ȡģ������
				break;
			 case MSG_IOT_TASK_HeartDATA_MQTT:				
			 	   Iotdata_Timer_Stop(TIMER_ID_USER_START + IOTDATA_TIMER_ID, IOTDATA_TIMER_MS); //�رն�ʱ��
		           MqttPubUserHeartData();					//������������
		           Iotdata_Timer_Start(TIMER_ID_USER_START + IOTDATA_TIMER_ID, IOTDATA_TIMER_MS);   //������ʱ��
		        break;
			 case MSG_IOT_TASK_SENDDATA_MQTT:
			 	   Iotdata_Timer_Stop(TIMER_ID_USER_START + IOTDATA_TIMER_ID, IOTDATA_TIMER_MS);
		           MqttPubUserSensorData();					//�����������ݵ�������
		           SendMsg2KernelForMcuOff();
		           Iotdata_Timer_Start(TIMER_ID_USER_START + IOTDATA_TIMER_ID, IOTDATA_TIMER_MS);
		        break;
			 case MSG_IOT_TASK_REDATA_MQTT:
				   MqttPubUserReSensorData();       //�ظ���������
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
    while (1)
    {
        Ql_OS_GetMessage(&msg);
        switch (msg.message)
        {
            case MSG_IOT_TASK_PMTK_SET_STATIC_NAV_THD:
                Speed_threshold();
	            break;
			case MSG_IOT_TASK_GET_LOCATION:
//                iRet = GetGpsLocation(45,1); //�źż��� ��ʹ��gps��λ ֱ�ӱ�����һ�ε����ݵ�
                SaveGpsdatapoint(msg.param1); //��ʹ�ö�λ���� 
				break;
			case MSG_IOT_TASK_SENDSAVEDATA_MQTT:
				   MqttPubUserSaveSensorData();
                   Ql_Sleep(1000);              //����ʡȥ�����޷�����
                   SendMsg2KernelForIotData();
                break;
			case MSG_IOT_TASK_MCU_OFF:
#if defined(USE_MPU6050) || defined(USE_ADS1115) || defined(USE_BOTH_MPU6050_ADS1115)
					break;
#endif
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
				 setmcu2poweroff203c(&msg.param1);
				 Ql_PowerDown(1);
				break;
        }

    }
}
/*!
 * @brief adc��ȡ����\n
 *
 * @param taskId  
 * @return NULL
 * \n
 * @see
 */

void proc_subtask1(s32 TaskId)
{
#if defined(USE_ADS1115) || defined(USE_BOTH_MPU6050_ADS1115)	//�ο�sys.h
	int vol1,vol2,vol3,vol4;
	int i;
     int vol[20] = {0};
	while(1)
		{
		
		    for(i=0;i<20;i++)
		    	{
		    	vol[i]=Get_ATOD(0);
		    	}
			   vol1=vol[19];			
			for(i=0;i<20;i++)
				{
				vol[i]=Get_ATOD(1);
				}
			vol2=vol[19];
			for(i=0;i<20;i++)
				{
				vol[i]=Get_ATOD(2);
			}
			vol3=vol[19]; 
            for(i=0;i<20;i++)
				{
				vol[i]=Get_ATOD(3);
				}
			vol4=vol[19];
			module_data.vol1=vol1;
			module_data.vol2=vol2;
			module_data.vol3=vol3;
			module_data.vol4=vol4;
			mprintf("vol1:%d,vol2:%d,vol3:%d,vol4:%d\r\n",vol1,vol2,vol3,vol4);
			Ql_Sleep(1500);
		    
    }
#else
    ST_MSG msg;
    while (1)
    {
        Ql_OS_GetMessage(&msg);
        switch (msg.message)
        {
            default:
                break;
        }

    } 

#endif
    
}


/*!
 * @brief mpu6050�������� \n
 *
 * @param taskId 
 * @return NULL
 * \n
 * @see
 */

void proc_subtask2(s32 TaskId)
{
	 Ql_Sleep(3000);
#if defined(USE_MPU6050) || defined(USE_BOTH_MPU6050_ADS1115)	//�ο�sys.h
	 CountStepInit();
	 MPU_Wakeup();
	 while (1)
	 	{
	      
	 		Ql_Sleep(10);
			     module_data.step=CountStep();
			
	    }	
#else
    ST_MSG msg;
    while (1)
    {
        Ql_OS_GetMessage(&msg);
        switch (msg.message)
        {
	        default:
	            break;
        }

    }    
#endif
}
	 	

void proc_subtask3(s32 TaskId)
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


