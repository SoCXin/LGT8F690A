
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_uart.h"
#include "ql_system.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_gpio.h"
#include "ql_iic.h"
#include "mpu6050.h"
#include "module_data.h"
#include "sys.h"
#include "math.h"
#include "203c_call.h"

#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT1
#define DBG_BUF_LEN   512
static char DBG_BUFFER[DBG_BUF_LEN];
#define mprintf(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
    }\
}
#else
#define mprintf(FORMAT,...)
#endif

unsigned short Preexercise = 0;
u8 MPU_Detect = 1;

int MPU_i2c_init(void)
{
	s32 ret;
	ret = Ql_IIC_Init(1,PINNAME_RI,PINNAME_DCD,1);
    if(ret < 0)
       {
           mprintf("\r\n<--Failed!! IIC controller Ql_IIC_Init channel 1 fail ret=%d-->\r\n",ret);
           return ret;
       }
    mprintf("\r\n<--pins(SCL=%d,SDA=%d) IIC controller Ql_IIC_Init channel 1 ret=%d-->\r\n",PINNAME_RI,PINNAME_DCD,ret);
	ret = Ql_IIC_Config(1,TRUE, MPU_ADDR, 40);
    if(ret < 0)
       {
          mprintf("\r\n<--Failed !! IIC controller Ql_IIC_Config channel 1 fail ret=%d-->\r\n",ret);
           return ret;
       }
     mprintf("\r\n<--IIC controller Ql_IIC_Config channel 1 ret=%d-->\r\n",ret);
	 return ret;
}

u8 MPU_Init(void)
{ 
	u8 res;
	u8 mpuid;
    Ql_Sleep(100);
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);	//��λMPU6050
    Ql_Sleep(300);
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);	//����MPU6050 
	MPU_Write_Byte(MPU_GYRO_CFG_REG,0x18);//���������������̷�Χ
	MPU_Write_Byte(MPU_ACCEL_CFG_REG,0);//���ü��ٶȴ����������̷�Χ  
	MPU_Set_Rate(50);						//���ò�����50Hz
	MPU_Write_Byte(MPU_INT_EN_REG,0X00);	//�ر������ж�
	MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);	//I2C��ģʽ�ر�
	MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);	//�ر�FIFO
	//MPU_Write_Byte(MPU_INTBP_CFG_REG,0X80);	//INT���ŵ͵�ƽ��Ч
	res=MPU_Read_Byte(MPU_DEVICE_ID_REG, &mpuid);
	if(mpuid==0x68)//����ID��ȷ
	{
		MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);	//����CLKSEL,PLL X��Ϊ�ο�
		MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);	//���ٶ��������Ƕ�����
		MPU_Set_Rate(50);
		mprintf("\r\n<--IIC Device Addr=0x%02x-->\r\n",mpuid);
 	}else return 1;
	return 0;
}

u8 MPU_Sleep(void)
{
    Ql_Sleep(200);//�ȴ�����
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X40);	//����sleep
	return 0;
}

u8 MPU_Wakeup(void)
{
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);	//����wakeup
    Ql_Sleep(200);//�ȴ�����
	return 0;
}

u8 MPU_Set_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=188)data=1;
	else if(lpf>=98)data=2;
	else if(lpf>=42)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return MPU_Write_Byte(MPU_CFG_REG,data);//�������ֵ�ͨ�˲���  
}

u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if(rate>1000)rate=1000;//4~1000(Hz)
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);	//�������ֵ�ͨ�˲���
 	return MPU_Set_LPF(rate/2);	//�Զ�����LPFΪ�����ʵ�һ��
}

//�õ��¶�ֵ
//����ֵ:�¶�ֵ(������100��)
short MPU_Get_Temperature(void)
{
    u8 buf[2]; 
    short raw;
	float temp;
	MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
    raw=((u16)buf[0]<<8)|buf[1];  
    temp=36.53+((double)raw)/340;  
    return temp*100;;
}

//�õ�������ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
int MPU_Get_Gyroscope()
{
    u8 buf[6],res;
	short gx, gy, gz;
	int gx1,gy1,gz1;
	u8 buft[6];
	float result;
	u8 xx;
	res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		gx=((u16)buf[0]<<8)|buf[1];  
		gy=((u16)buf[2]<<8)|buf[3];  
		gz=((u16)buf[4]<<8)|buf[5];
		gx1=(float)(gx-32768)*2/32768.0;
		gy1=(float)(gy-32768)*2/32768.0;
		gz1=(float)(gz-32768)*2/32768.0;
		gx = gx & 0xff;
		gy = gy & 0xff;
		gz = gz & 0xff;
		
		mpu6050data.gx = gx;
		mpu6050data.gy = gy;
		mpu6050data.gz = gz;
		//mprintf("gyr : %hd-%hd-%hd",gx, gy, gz);
		result = (float)sqrt(gx1*gx1+gy1*gy1+gz1*gz1);

		if(result == 3.0){
			mpu6050SportLevel = 0;
			}
		else if((result > 2.4 && result < 2.5)||(result > 3.4 && result < 3.5)){
			mpu6050SportLevel = 1;
			}
		else if((result > 2.0 && result < 2.4)||(result > 3.5 && result < 4.6)){
			mpu6050SportLevel = 2;
			}
		else if(result > 4.6 && result < 5.0){
			mpu6050SportLevel = 3;
			}
		else if(result > 5.0){
			mpu6050SportLevel = 4;
			}
	} 	
    return res;
}

//�õ����ٶ�ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
int  MPU_Get_Accelerometer()
{
    u8 buf[6],res;  
	short ax,ay,az;
	float result;
	int exercise,agx,agy,agz;
	
	res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
	if(res==0)
	{
		ax=((u16)buf[0]<<8)|buf[1];  
		ay=((u16)buf[2]<<8)|buf[3];  
		az=((u16)buf[4]<<8)|buf[5];
		agx=(float)(ax-32768)*2/32768.0;
		agy=(float)(ay-32768)*2/32768.0;
		agz=(float)(az-32768)*2/32768.0;
		
		ax = ax & 0xff;
		ay = ay & 0xff;
		az = az & 0xff;
		mpu6050data.ax = ax;
		mpu6050data.ay = ay;
		mpu6050data.az = az;
		//mprintf("acc : %hd-%hd-%hd\r\n",ax, ay, az);
		result = (float)sqrt(agx*agx+agy*agy+agz*agz);
	
	}
  return 0;
}
//IIC������
//addr:������ַ
//reg:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//buf:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	int res = 0;
	res=Ql_IIC_Write(MPU_CHNUM,addr,&reg,0x01);
	if(res< 0)
	{
		mprintf("\r\n<--Failed !! IIC controller Ql_IIC_Write channel 1 fail ret=%d-->\r\n",res);
		return res;
	}
	res = Ql_IIC_Read(MPU_CHNUM, addr, buf, len);
	if(res < 0)
	{
		mprintf("\r\n<--Failed !! IIC controller Ql_IIC_Read channel 1 fail ret=%d-->\r\n",res);
		return res;
	}
	return 0;	
}
//IICдһ���ֽ� 
//reg:�Ĵ�����ַ
//data:����
//����ֵ:0,����
//    ����,�������
u8 MPU_Write_Byte(u8 reg,u8 data) 				 
{
	int err=0;
	u8 buf[5] = {0};

	buf[0] = reg;
	buf[1] = data;

    err = Ql_IIC_Write(MPU_CHNUM,MPU_ADDR, buf, 0x02);
    if (err < 0)
	{
        mprintf("send command error!!\n");
        return err;
    } 
	return 0;
}
//IIC��һ���ֽ� 
//reg:�Ĵ�����ַ 
//����ֵ:����������
u8 MPU_Read_Byte(u8 reg, u8 *data)
{
	u8 res;
	res=Ql_IIC_Write(MPU_CHNUM,MPU_ADDR,&reg,0x01);
	if(res< 0)
	{
		mprintf("\r\n<--Failed !! IIC controller Ql_IIC_Write channel 1 fail ret=%d-->\r\n",res);
		return res;
	}
	res = Ql_IIC_Read(MPU_CHNUM, MPU_ADDR, data, 0x01);
	if(res < 0)
	{
		mprintf("\r\n<--Failed !! IIC controller Ql_IIC_Read channel 1 fail ret=%d-->\r\n",res);
		return res;
	}
	return res;		
}

