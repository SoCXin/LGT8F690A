#ifndef __SYS_H__
#define __SYS_H__


#include "ql_gpio.h"
#include "ql_adc.h"
#include "zyf_protocol.h"

#define MAX_POINT_LEN (20) //can not exceed (255)u8

#define LED1_H	 Ql_GPIO_SetLevel(LED1, PINLEVEL_HIGH)
#define LED1_L	 Ql_GPIO_SetLevel(LED1, PINLEVEL_LOW)
#define LED2_H	 Ql_GPIO_SetLevel(LED2, PINLEVEL_HIGH)
#define LED2_L	 Ql_GPIO_SetLevel(LED2, PINLEVEL_LOW)


extern Enum_PinName LED1;
extern Enum_PinName LED2;
extern Enum_PinName LED3;
extern volatile u8 Chargsta;

typedef enum
{
    MSG_LOCK_CTRL_SERVICE_NONE = 0x100,  		//Ԥ��    
    MSG_LOCK_CTRL_SERVICE_RENT ,     			//�⳵
    MSG_RD_ALIYUN_MQTT_RE,						//����������������
    MSG_LOCK_CTRL_SERVICE_RETURN,       		//����
    MSG_SEND_ALIYUN_MQTT_HRET, 				//����������
    MSG_SEND_ALIYUN_IOTSTA, 		//����ҵ������ 
    MSG_SEND_ALIYUN_INIT, 		//��ʼ������������
} MSG_LOCK_CTRL_SERVICE_TYPE_T;

enum
{
    CHARG_OFF = 0,
    CHARG_ON,
};

typedef struct{
    u32 year;
    u32 mon;
    u32 day;
    u32 hour;
    u32 min;
    u32 sec;
}__collecttime;

typedef struct
{
    u32 latitude;											  
    u32 longitude;
    __collecttime collecttime;
    u8 bvol;
    u8 sysruntime;
    u8 csq;
}_save_location_point;

typedef struct 
{
	u32 HandInter;			//��������ʱ����
	u32 Interval;				//�����Զ��ϴ�ʱ����
	u8 SN[16];				//SN
	u8 snuser;				//SN�Ƿ��Ѿ�����
	u8 secret[48];
	u8 key[16];
	u8 saveflag;
	u8 fotaflag;				//������־
	u8 fotaaddr[200];			//�̼���ַ����Ϣ
	u32 SysTime;
	u8 CallMode;				//Ԥ��
	u8 updateinfo;			//������־
}_system_setings;


typedef struct
{
    u8 save_data_num;       //�������ݵ���� ��һ�� +1
    u8 preflag;             //�ϴζ�λ���
    u8 gorlflag;            //lbs or gps 1 gps 2 lbs
    u8 faststart;           //���ٿ������� 10min ���¿���
    u32 prellng;            //��һ�λ�ȡ���Ķ�λ���� ����
    u32 prellat;            //γ��
	_save_location_point save_point[MAX_POINT_LEN]; //�������ݵ� ���ַ���������ݵ�
}_save_data_point;

extern volatile u8 YorNupgrading;


extern volatile u8 SocketBufRdBit;
extern volatile u8 nextfaststart;
extern volatile int savemode;




extern u8 Need_Lbs_Data;

extern _system_setings	systemset;
extern _save_data_point savedata;



void DisUpdateRTC(void);
void GpsDelay(u16 delaytime);
void LbsDelay(u16 delaytime);
void power_drv_init(void);
void LcdPin_init(void);
void Delayus(u32 data);
void SysSleep(void);
void  LoadDefualtCfg(void);
void SYS_Parameter_Init(void);
void ShowSysInfo(void);
void InternetsetSN_Analysis(u8 *snuf,u8 len);
void Callback_ChageInit(Enum_ADCPin adcPin, u32 adcValue, void *customParam);
void SysWakeUp(void);
void ADC_Program(void);
void DrvBell_Ring_R(u8 num);
void M203C_BSP_init(void);
void Set_FirstData_location(u8 gorlflag,s32 llng,u32 llat);

void JudgeGpsAndGSM(void);
void SaveGpsdatapoint(u32 parm);



#endif

