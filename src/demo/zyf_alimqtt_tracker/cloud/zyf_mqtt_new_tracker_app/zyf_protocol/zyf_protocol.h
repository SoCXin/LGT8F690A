

/******************************************************************************

                  ��Ȩ���� (C) 2018, �촴��ũ�Ƽ�

 ******************************************************************************
  �ļ�   : zyf_protocol.h
  �汾   : ����
  ����   : LiCM
  ����   : 2018��02��24��
  ����   : �����ļ�
  ����   : ���Ʒ�MQTT�汾ͨ��Э��

  �޸ļ�¼:
  ����   : 2018��02��24��
    ����   : LiCM
    ����   : �����ļ�

******************************************************************************/



#ifndef __ZYF_PROTOCOL_
#define __ZYF_PROTOCOL_

#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_trace.h"
#include "ql_gpio.h"

#define ZYF_PRO_DEBUG
#define _USE_RALL_SENSOR_


#define SW_VERSION	"18052202"
#define ALINK_MAX_BUF 800



#define BLE_CMD_HEAD 0xAA  // �� App ���͸�����
#define BLE_RESP_HEAD 0x55 // ���������͸� App ������Ϊ��֡ͷ��1 �ֽڣ�0x55

#define APP_SUCCESS 0x00
#define APP_FAIL 0x01
#define APP_ERROR_CMD 0x02

#define _USE_MCU_PRO_

#ifdef _USE_MCU_PRO_


extern Enum_PinName M203C_DATA;
extern Enum_PinName M203C_EINT;
extern Enum_PinName M203C_WHG;



#define M203C_DATA_L	 Ql_GPIO_SetLevel(M203C_DATA, PINLEVEL_LOW)
#define M203C_DATA_H	 Ql_GPIO_SetLevel(M203C_DATA, PINLEVEL_HIGH)
#define M203C_EINT_L	 Ql_GPIO_SetLevel(M203C_EINT, PINLEVEL_LOW)
#define M203C_EINT_H	 Ql_GPIO_SetLevel(M203C_EINT, PINLEVEL_HIGH)
#define M203C_WHG_L	     Ql_GPIO_SetLevel(M203C_WHG, PINLEVEL_LOW)
#define M203C_WHG_H	     Ql_GPIO_SetLevel(M203C_WHG, PINLEVEL_HIGH)


#endif


typedef enum
{
    BLE_CMD_AUTH_KEY  = 0X01,      
    BLE_CMD_AUTH_REQU = 0x02,           
    BLE_CMD_DEVRESET = 0XF0,
    BLE_ERROR_CMD = 0xFF,
} BLECMD_CODE_t;


typedef enum
{
    GET_SENSOR_DATA =10,      
    GET_STM_ID = 11,
    GET_FRTAIR_DATA = 12,
    GET_BATTARY_DATA = 13,
    GET_DLDX_DATA = 15,
    GET_SOFT_VERSION = 20,
    UPGATE_SOFT_VERSION = 40,
    SET_MCU_SLEEP = 50,
    SET_MCU_WKPU = 51,
} M203_STM_CMD_T;


typedef enum
{
    MSG_NULL = 1,
	MSG_GET_HS,
	MSG_GET_CS,
	MSG_KJ_ZR,
	MSG_KJ_ZL,
	MSG_GJ_CMD,
} M203_MSG_CMD_T;


typedef struct 
{
	u8 Bh1750gz[10];
	u8 HDCSD[10];
	u8 HDCWD[10];
	u8 NSHTSD[10];
	u8 NSHTWD[10];
	u8 CO2[10];
	u8 SOILT[10];
	u8 SOILH[10];
	u8 VOLBAI[10];
	u8 DLDX[10];
}stm_data;

extern stm_data stm_sensor;

typedef struct 
{
	u8 Dn[20];
	u8 Dm[20];
	u8 Dx[20];
	u8 Sn[20];
	u8 Sm[20];
	u8 Sx[20];
	u8 Ta[20];
	u8 Ua[20];
	u8 Pa[20];
	u8 Rc[20];
	u8 Sr[20];
}frt_stm_data;

extern frt_stm_data frt_sensor;



extern u8 ZYFRECMD[2][100];

extern volatile u8 ZYF_MQTT_CONFIG;




s32 alink_msg_Sensor_data(u8 *data_ptr,void *user_data);
s32 alink_decode_cmd(char *Payload,u16 len);
void setmcu2poweroff(u16 min);
void SendNumFromProPin(u16 num);

#endif


