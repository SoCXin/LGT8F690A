//-----------------------------------------
// SPI Controller
//	SPI Interface
//	SPCK @ RA4, MOSI @ RA5, MISO @ RB0, SPSS @ RA6
//	Master Write and Read Operation
//-----------------------------------------

#include "lgt8f690a.h"
#include "spi.h"

void spi_init_mst()
{
	RA6 = 0x1;
	RA4 = 0x0;
	RA5 = 0x0;
	RB0 = 0x0;
	// Set direction for Input/Output
	TRISA4 = 0x0;		// spck
	TRISA5 = 0x0;		// mosi 
	TRISA6 = 0x0; 		// spcs 
	TRISB0 = 0x1;		// miso
	// SPI Configuration 
	SPSR = 0x00;
	SPCR = 0x11;
	SPCR = 0x51;
}

void spi_exch(unsigned char length, unsigned char *rxbuf, unsigned char *txbuf)
{
	unsigned char i;
	unsigned char tmp;

// clear tx & rx buffer status ���TX��RX������״̬
	SPFR = 0x44;	

// dummy read SPDR for 3times first @2T/4T, no dummy read @1T
// ��2T��4Tָ��ģʽ�£�Ԥ�ȶ�ȡSPDR���Σ��Ա��ں˵õ���ȷ�Ľ������ݣ�1Tģʽ�²���Ҫ
	if(L_MCUCRbits.TCYC > 0x0)	
	{
		tmp = SPDR;tmp = SPDR;tmp = SPDR; 
	}

// fill tx buffer as many as possible��no more than 4
// �����ܶ�����TX�����������4��
	for(i=0; ((i<length) && (i < 4)); i++) { SPDR = *txbuf++; }	

	while(length > 4)
	{
// get rx and fill tx when tx buffer is not full
// ��TX���������ڷ���״̬ʱ����ȡSPDR�õ��������ݣ����ٴ����TX������
		if (!(SPFR & (1 << 3))) {*rxbuf++ = SPDR; length--; SPDR = *txbuf++;}	
	}

	while(length > 0)
	{
// get rx when tx buffer is empty
// ��TX���������ڿ�״̬ʱ����ȡSPDR�õ���������
		if ((SPFR & (1 << 2))) {*rxbuf++ = SPDR; length--;}	
	}
}



