
#include "ql_type.h"
#include "ql_trace.h"
#include "ql_uart.h"
#include "ql_system.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_gpio.h"
#include "ql_iic.h"
#include "uart.h"
#include "sys.h"
#include "ads1151.h"


//unsigned char ReadBuffer[2];
/*******************************************************************************
* Function Name  : ads1151_i2c_init
* ?�??          : ?�始?�i2c
* Attention  ?�i2c?��*******************************************************************************/
int ads1151_i2c_init(void)
{
	s32 ret;
#if defined(USE_ADS1115) //&& !defined(USE_BOTH_MPU6050_ADS1115)
	ret = Ql_IIC_Init(1,PINNAME_RI,PINNAME_DCD,1);
	if(ret < 0)
	{
		mprintf("\r\n<--Failed!! IIC controller Ql_IIC_Init channel 1 fail ret=%d-->\r\n",ret);
		return ret;	
	}
	mprintf("\r\n<--pins(SCL=%d,SDA=%d) IIC controller Ql_IIC_Init channel 1 ret=%d-->\r\n",PINNAME_RI,PINNAME_DCD,ret);
#endif
    ret = Ql_IIC_Config(1,TRUE, 0x90, 300);
	if(ret < 0)
	{
		mprintf("\r\n<--Failed !! IIC controller Ql_IIC_Config channel 1 fail ret=%d-->\r\n",ret);
		return ret;
	}		
	mprintf("\r\n<--IIC controller Ql_IIC_Config channel 1 ret=%d-->\r\n",ret);
	return ret;
}
/*******************************************************************************
* Function Name  : ads_i2c_read
* ?�??      
                   addr：寄存?�地址
				   *data 存?��				 
* Attention  �?2c?��*******************************************************************************/
 int ads_i2c_read(u8 addr, u8 *data)
{
	int res = 0;
	res=Ql_IIC_Write(ADS1151_CHNUM,0x90,&addr,0x01);
	if(res< 0)
	{
		mprintf("\r\n<--Failed !! IIC controller Ql_IIC_Write channel 1 fail ret=%d-->\r\n",res);
		return res;	
	}
	res = Ql_IIC_Read(ADS1151_CHNUM, 0x90, data, 0x02);
	
	if(res < 0)
	{
		mprintf("\r\n<--Failed !! IIC controller Ql_IIC_Read channel 1 fail ret=%d-->\r\n",res);
		return data;	
	}
	return data;
}
/*******************************************************************************
* Function Name  : ads_i2c_write
* ?�??      
                   addr：寄存?�地址
				   *data 存?��				 
* Attention  �?2c?��*******************************************************************************/
int ads_i2c_write(u8 addr, u8 *data)
{
     u8 buf[3]={0,0,0};
	 buf[0]=addr;
	 buf[1]=data[0];
	 buf[2]=data[1];
	int res = 0;
	res=Ql_IIC_Write(ADS1151_CHNUM,0x90,buf,0x03);
		if(res< 0)
	{
		mprintf("\r\n<--Failed !! IIC controller Ql_IIC_Write channel 1 fail ret=%d-->\r\n",res);
		return res;	
	}
		return res;
	
}


/*******************************************************************************
* Function Name  : Confige1115
* ?�??          : ?�?�0/1/2/3
* Attention :?��*******************************************************************************/

static void Confige1115 (unsigned char port)
{
    static unsigned char chnel, i;
	u8 databuf[2]={0,0}; 

	u8 buf_read[ADS1151_DATA_LEN] = {0};

	 int res = 0;
	 u8 mpuid;
    switch (port)
    {
      case 0:               //0?�?�
          chnel=0xC2; 
      break;
      
      case 1:               //1?�?�  
          chnel=0xD2;
      break;
          
      case 2:               //2?�?�  
          chnel=0xE2;
      break;
          
      case 3:               //3?�?�
          chnel=0xF2;
      break;
          
      default:
      break; 
    }   
	databuf[0]=chnel;
	databuf[1]=0x83;

	res = ads_i2c_write(CMD_CONF_REG,databuf);//?�?�数�	ads_i2c_read(0x01, buf_read);
	if(res < 0)
	{
		return ADS1151_ERR_I2C;
	}
  
       
}

/*******************************************************************************
* 读取ADS1115?�16位?��*******************************************************************************/
static u16 ReadData (unsigned char chnnal1)
{
    unsigned char ReadBuffer[2]={0,0};
	u16  data=0;
	int res = 0;
	u8 buf[ADS1151_DATA_LEN] = {0};
	ads_i2c_read(0x00, buf);////读取?��    Delayus(200);
	ReadBuffer[0]=buf[0];
	ReadBuffer[1]=buf[1];
	Delayus(200);
  	data = ReadBuffer[0]*256+ReadBuffer[1];  ////?��	return data;
}


/*******************************************************************************
* Function Name  : Get_ATOD
* Attention :?�取ADS1115模拟�*******************************************************************************/
float Get_ATOD (unsigned char channel)
{
	static unsigned char chn; 
	u16 data_get;
	chn = channel; 
	float vol; 
	Confige1115(channel); ////?��	data_get = ReadData(chn);  ////从?�?��	/**?��?测?�负?�压，负?�压从8000~ffff,负?�压与正关�?0?�类似对称?�系，?�位?�?�?�+1?�同**/
	if(data_get>=0x8000)  
		vol=((float)(0xffff-data_get)/32768.0)*4.096;
	else
		vol=((float)data_get/32768.0)*4.096;
	vol=vol*1000;
				return vol;
}

