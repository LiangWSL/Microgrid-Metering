//========================================================================
// ��  Ŀ: [��ҵ���]����΢���ڲ�����ϵͳ
// ��  ��: Li Haolin (lihaolinwork@foxmail.com)
// ��  ��: 2018/05/1
// ��  ��: 
// ��Ŀ��ַ:https://github.com/lihaolin123/Microgrid-Metering
//
//========================================================================
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//==================== �������� ==============================================
//#define DEBUG_DOG
#define DEBUG_FLASH
//#define DEBUG_DEFAULT_FLASH
//#define STM32_FLASH
//====================ͷ�ļ� ==============================================
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "string.h"
#include "includes.h"
#include "led.h"
#include "key.h"
#include "oled.h"
#include "rtc.h"
#include "modbus.h"
#include "cs5464.h"
#include "convert.h"
#include "iwdg.h"
#include "spi.h"
#include "w25qxx.h"
#include "stmflash.h" 
#include "exti.h"
#include "dc_cs5464.h" 
#include "stdio.h"
//==================== UCOSII�������� ====================================
///////////////START ����///////////
//�����������ȼ�
#define START_TASK_PRIO      			30 
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 //////////////LED0����/////////////   
#define LED0_TASK_PRIO       			29 
#define LED0_STK_SIZE  		    		64
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
void LED0_task(void *pdata);
///////////////OLED����/////////////
#define OLED_TASK_PRIO       			27 
#define OLED_STK_SIZE  					256
__align(8) OS_STK OLED_TASK_STK[OLED_STK_SIZE];
void OLED_task(void *pdata);
///////////����ɨ������///////////////
#define KEY_TASK_PRIO					28
#define KEY_STK_SIZE					64
OS_STK 	KEY_TASK_STK[KEY_STK_SIZE]; 
void Key_task(void *pdata);
/////////////RTC����//////////////////
#define RTC_TASK_PRIO       			26 
#define RTC_STK_SIZE  		    		256
OS_STK  RTC_TASK_STK[RTC_STK_SIZE];
void RTC_task(void *pdata);
/////////////MODBUSͨѶ����///////////
#define MODBUS_TASK_PRIO       			8 
#define MODBUS_STK_SIZE  		    	256
OS_STK  MODBUS_TASK_STK[MODBUS_STK_SIZE];
void Modbus_task(void *pdata);
/////////////CS5464�����ɼ�����/////
#define CS5464_TASK_PRIO       			11 
#define CS5464_STK_SIZE  		    	256
OS_STK  CS5464_TASK_STK[CS5464_STK_SIZE];
void CS5464_task(void *pdata);
/////////////ֱ��CS5464�����ɼ�����/////
#define DC_CS5464_TASK_PRIO       		5 
#define DC_CS5464_STK_SIZE  		    256
OS_STK  DC_CS5464_TASK_STK[DC_CS5464_STK_SIZE];
void DC_CS5464_task(void *pdata);
/////////////FLASH����//////////////////
#define FLASH_TASK_PRIO       			15 
#define FLASH_STK_SIZE  		    	256
OS_STK  FLASH_TASK_STK[FLASH_STK_SIZE];
void FLASH_task(void *pdata);
/////////////ң������//////////////////
#define YC_TASK_PRIO       				14 
#define YC_STK_SIZE  		    		64
OS_STK  YC_TASK_STK[YC_STK_SIZE];
void YC_task(void *pdata);
/////////////ң������//////////////////
#define YK_TASK_PRIO       				12 
#define YK_STK_SIZE  		    		256
OS_STK  YK_TASK_STK[YK_STK_SIZE];
void YK_task(void *pdata);
//==================== ���� ====================================
////////////////////////////////////////
//û��EEPROM,���Բ�����FLASH
//����FLASH��д��������,STM32�ڲ�10k,�ⲿ��100k
//���Դ˴���Ҫ�Ż�.���Գ��Ե���д��������һ������ʱ���߹�һ��ʱ��,��������(ʱ���ϵ��Ū��)
//�ⲿw25q16��512������.��stm32f407vetֻ��8��,̫����,�ʲ��������ڲ���,���һ��Ҫ��,��������ϧ
//���Ȿ����Ŀǰ60+kB,����ǰ5����Ϊ�û�������һ����Ҫ��!
#define STM32_SAVE_ADDR  			0X08040000 	//����STM32 FLASH���Ա����ַ,ע��Ҫ��4����!
#define FLASH_SAVE_SIZE  			0X1000 //������С4k
#define FLASH_SAVE_ADDR0  			0X0000 //��0����
#define FLASH_SAVE_ADDR1  			0X1000 //��1����
#define FLASH_SAVE_ADDR2  			0X2000 //��2����
#define FLASH_USE_SIZE				0x28   //ʹ�ô�С,��4����
#define FLOAT_REG_SIZE				4	
#define SHORT_REG_SIZE				2	
#define CHAR_REG_SIZE				1	
#define AC_ALL_POWER_REG 			0x00//��������
#define AC_PEAK_POWER_REG 			0x04//�������
#define AC_VALLEY_POWER_REG 		0x08//�����ȵ�
#define DC_POWER_REG 				0x0C//ֱ������
#define LAST_AC_ALL_POWER_REG		0x10//��һ�ν�������
#define LAST_AC_PEAK_POWER_REG 		0x14//��һ�ν������
#define LAST_AC_VALLEY_POWER_REG 	0x18//��һ�ν����ȵ�
#define LAST_DC_POWER_REG 			0x1C//��һ��ֱ������
#define PEAK_TIME_L_REG				0x20//���
#define PEAK_TIME_H_REG				0x21//���
#define VALLEY_TIME_L_REG			0x22//�ȵ�
#define VALLEY_TIME_H_REG			0x23//�ȵ�	
#define LOAD_CTRL_U_REG				0x24//����
#define LOAD_CTRL_I_REG				0x25
#define LOAD_CTRL_P_REG				0x26
#define FROZEN_MODE_REG				0x27//����
#define RTC_FLAG			((ucRegCoilsBuf[0]>>7)&0x01) 		//RTCʱ��
#define SET_RTC_FLAG 		(ucRegCoilsBuf[0]|=(0x01<<7))
#define RESET_RTC_FLAG 		(ucRegCoilsBuf[0]&=(~(0x01<<7)))
#define SSR_FLAG			((ucRegCoilsBuf[0]>>6)&0x01) 		//SSR
#define SET_SSR_FLAG 		(ucRegCoilsBuf[0]|=(0x01<<6))		
#define RESET_SSR_FLAG 		(ucRegCoilsBuf[0]&=(~(0x01<<6)))	
#define RESET_FLAG			((ucRegCoilsBuf[0]>>5)&0x01) 		//Flash����
#define SET_RESET_FLAG 		(ucRegCoilsBuf[0]|=(0x01<<5))		
#define RESET_RESET_FLAG 	(ucRegCoilsBuf[0]&=(~(0x01<<5)))
#define TIME_FLAG			((ucRegCoilsBuf[0]>>4)&0x01) 		//TIME����
#define SET_TIME_FLAG 		(ucRegCoilsBuf[0]|=(0x01<<4))		
#define RESET_TIME_FLAG 	(ucRegCoilsBuf[0]&=(~(0x01<<4)))	
#define ERROR_FLAG			((ucRegCoilsBuf[0]>>3)&0x01) 		//�쳣
#define SET_ERROR_FLAG 		(ucRegCoilsBuf[0]|=(0x01<<3))		
#define RESET_ERROR_FLAG 	(ucRegCoilsBuf[0]&=(~(0x01<<3)))	
#define GET_RTC_FLAG		((ucRegCoilsBuf[0]>>2)&0x01) 		//��ѯRTC
#define SET_GET_RTC_FLAG 	(ucRegCoilsBuf[0]|=(0x01<<2))		
#define RESET_GET_RTC_FLAG 	(ucRegCoilsBuf[0]&=(~(0x01<<2)))	
#define GET_TIME_FLAG		((ucRegCoilsBuf[0]>>1)&0x01) 		//���ʱ��
#define SET_GET_TIME_FLAG 	(ucRegCoilsBuf[0]|=(0x01<<1))		
#define RESET_GET_TIME_FLAG (ucRegCoilsBuf[0]&=(~(0x01<<1)))	
#define LOAD_CTRL_FLAG		 ((ucRegCoilsBuf[0])&0x01) 			//���ɿ���
#define SET_LOAD_CTRL_FLAG 	 (ucRegCoilsBuf[0]|=0x01)		
#define RESET_LOAD_CTRL_FLAG (ucRegCoilsBuf[0]&=(~0x01))	
#define YEAR  				(usRegHoldingBuf[0])//��
#define MONTH				(usRegHoldingBuf[1])//��
#define DATE				(usRegHoldingBuf[2])//��
#define WEEK				(usRegHoldingBuf[3])//��
#define HOUR				(usRegHoldingBuf[4])//ʱ
#define MINUTE				(usRegHoldingBuf[5])//��
#define SECOND				(usRegHoldingBuf[6])//��
#define PEAK_TIME_L			(usRegHoldingBuf[7])//��
#define PEAK_TIME_H			(usRegHoldingBuf[8])
#define VALLEY_TIME_L		(usRegHoldingBuf[9])//��
#define VALLEY_TIME_H		(usRegHoldingBuf[10])
#define LOAD_CTRL_U			(usRegHoldingBuf[11])//����
#define LOAD_CTRL_I			(usRegHoldingBuf[12])
#define LOAD_CTRL_P			(usRegHoldingBuf[13])
#define FROZEN_MODE			(usRegHoldingBuf[14])//����
//#define FROZEN_MODE2_YEAR	(usRegHoldingBuf[15])//��ʱ��
//#define FROZEN_MODE2_DATE	(usRegHoldingBuf[16])//����
//#define FROZEN_MODE2_TIME	(usRegHoldingBuf[17])//ʱ��

//==================== ������� =========================================================
OS_CPU_SR cpu_sr=0;
typedef union{
	float	f;  
	u8		b[4];  
}conv; 
RTC_TimeTypeDef RTC_TimeStruct;	//RTCʱ��
RTC_DateTypeDef RTC_DateStruct;	//RTC���� 
u8 OLEDPage=1;					//OLED��ʾ��������
u8 write_last_flag=0;	
float AC_voltage=0,AC_current=0,activePower=0,reactivePower=0,apparentPower=0,powerFactor=0;//�����
float DC_voltage=0,DC_current=0,DC_activePower=0;
conv ac_all_power;			//��������	
conv ac_peak_power;			//���		
conv ac_valley_power;		//�ȵ�			
conv dc_power;				//ֱ������		
conv last_ac_all_power;		//��һ�ε�����			
conv last_ac_peak_power;					
conv last_ac_valley_power;	
conv last_dc_power;					
u8 peak_time_L=8,peak_time_H=22;		//�߷�
u8 valley_time_L=22,valley_time_H=8;	//�͹�
u8 flat_time_L=0,flat_time_H=0;			//ƽ��
u8 load_temp=0xff;
u8 stm32_read_datatemp[FLASH_USE_SIZE]={0};		
u8 stm32_write_datatemp[FLASH_USE_SIZE]={0};			
//==================== �ж����� =====================================================
void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_SetVectorTable(NVIC_VectTab_FLASH,0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//����ϵͳ�ж����ȼ����� 2
	//Usart1
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;	//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);								//����ָ���Ĳ�����ʼ��VIC�Ĵ���	
 	//TIM2 
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//��ռ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;			//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);								//����ָ���Ĳ�����ʼ��VIC�Ĵ���	
	//PVD
	NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;           //ʹ��PVD���ڵ��ⲿ�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       //�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          //ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);
}
//==================== �������� ==================================================
void Load_Data()
{
	char i=0;
	#ifdef DEBUG_DEFAULT_FLASH
	for(i=0;i<3;i++)
	{
		W25QXX_Write(ac_all_power.b,	AC_ALL_POWER_REG+FLASH_SAVE_SIZE*i,		FLOAT_REG_SIZE);	
		W25QXX_Write(ac_peak_power.b,	AC_PEAK_POWER_REG+FLASH_SAVE_SIZE*i,	FLOAT_REG_SIZE);	
		W25QXX_Write(ac_valley_power.b,	AC_VALLEY_POWER_REG+FLASH_SAVE_SIZE*i,	FLOAT_REG_SIZE);
		W25QXX_Write(dc_power.b,		DC_POWER_REG+FLASH_SAVE_SIZE*i,			FLOAT_REG_SIZE);
		W25QXX_Write(last_ac_all_power.b,	LAST_AC_ALL_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
		W25QXX_Write(last_ac_peak_power.b,	LAST_AC_PEAK_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
		W25QXX_Write(last_ac_valley_power.b,LAST_AC_VALLEY_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
		W25QXX_Write(last_dc_power.b,		LAST_DC_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
		W25QXX_Write(&peak_time_L,		PEAK_TIME_L_REG+FLASH_SAVE_SIZE*i,	CHAR_REG_SIZE);	//32
		W25QXX_Write(&peak_time_H,		PEAK_TIME_H_REG+FLASH_SAVE_SIZE*i,	CHAR_REG_SIZE);	//33
		W25QXX_Write(&valley_time_L,	VALLEY_TIME_L_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	//34
		W25QXX_Write(&valley_time_H,	VALLEY_TIME_H_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	//35
		load_temp=0xff;
		W25QXX_Write(&load_temp,		LOAD_CTRL_U_REG+FLASH_SAVE_SIZE*i,	CHAR_REG_SIZE);	//36
		W25QXX_Write(&load_temp,		LOAD_CTRL_I_REG+FLASH_SAVE_SIZE*i,	CHAR_REG_SIZE);	//37
		W25QXX_Write(&load_temp,		LOAD_CTRL_P_REG+FLASH_SAVE_SIZE*i,	CHAR_REG_SIZE);	//38
		load_temp=0;
		W25QXX_Write(&load_temp,FROZEN_MODE_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	//39
	
	}
	#ifdef STM32_FLASH
	stm32_write_datatemp[PEAK_TIME_L_REG]=peak_time_L;
	stm32_write_datatemp[PEAK_TIME_H_REG]=peak_time_H;
	stm32_write_datatemp[VALLEY_TIME_L_REG]=valley_time_L;
	stm32_write_datatemp[VALLEY_TIME_H_REG]=valley_time_H;
	stm32_write_datatemp[LOAD_CTRL_U_REG]=0xff;
	stm32_write_datatemp[LOAD_CTRL_I_REG]=0xff;
	stm32_write_datatemp[LOAD_CTRL_P_REG]=0xff;
	stm32_write_datatemp[FROZEN_MODE_REG]=0;
	STMFLASH_Write(STM32_SAVE_ADDR,(u32*)stm32_write_datatemp,(FLASH_USE_SIZE/4));	//д��STM32 FLASH����
	#endif
	
	#endif
	u8 *data0 = (u8 *)malloc(sizeof(char) * FLASH_USE_SIZE);//�����ڴ棡
	u8 *data1 = (u8 *)malloc(sizeof(char) * FLASH_USE_SIZE);//�����ڴ棡
	u8 *data2 = (u8 *)malloc(sizeof(char) * FLASH_USE_SIZE);//�����ڴ棡
	W25QXX_Read(data0,FLASH_SAVE_ADDR0,FLASH_USE_SIZE);	
	W25QXX_Read(data1,FLASH_SAVE_ADDR1,FLASH_USE_SIZE);	
	W25QXX_Read(data2,FLASH_SAVE_ADDR2,FLASH_USE_SIZE);	
	for(i=0;i<FLASH_USE_SIZE;i++)
	{
		if(*data0!=*data1++||*data0!=*data2++)
		{
				SET_ERROR_FLAG;
				SSR_OFF();
				OLED_ShowString(1,0,"FLASH DATA ERR!");
				OLED_ShowString(1,4,"err.0");
				while(1);
		}
		data0++;
	}
	free(data0);//�ͷţ�
	free(data1);//�ͷţ�
	free(data2);//�ͷţ�
	data0=NULL;
	data1=NULL;
	data2=NULL;
	W25QXX_Read(ac_all_power.b,			AC_ALL_POWER_REG,FLOAT_REG_SIZE);	
	W25QXX_Read(ac_peak_power.b,		AC_PEAK_POWER_REG,FLOAT_REG_SIZE);
	W25QXX_Read(ac_valley_power.b,		AC_VALLEY_POWER_REG,FLOAT_REG_SIZE);
	W25QXX_Read(dc_power.b,				DC_POWER_REG,FLOAT_REG_SIZE);
	W25QXX_Read(last_ac_all_power.b,	LAST_AC_ALL_POWER_REG,FLOAT_REG_SIZE);	
	W25QXX_Read(last_ac_peak_power.b,	LAST_AC_PEAK_POWER_REG,FLOAT_REG_SIZE);
	W25QXX_Read(last_ac_valley_power.b,	LAST_AC_VALLEY_POWER_REG,FLOAT_REG_SIZE);
	W25QXX_Read(last_dc_power.b,		LAST_DC_POWER_REG,FLOAT_REG_SIZE);
	W25QXX_Read(&peak_time_L,	PEAK_TIME_L_REG,CHAR_REG_SIZE);	
	W25QXX_Read(&peak_time_H,	PEAK_TIME_H_REG,CHAR_REG_SIZE);	
	W25QXX_Read(&valley_time_L,	VALLEY_TIME_L_REG,CHAR_REG_SIZE);	
	W25QXX_Read(&valley_time_H,	VALLEY_TIME_H_REG,CHAR_REG_SIZE);	
	W25QXX_Read(&load_temp,		LOAD_CTRL_U_REG,CHAR_REG_SIZE);	LOAD_CTRL_U=load_temp;
	W25QXX_Read(&load_temp,		LOAD_CTRL_I_REG,CHAR_REG_SIZE);	LOAD_CTRL_I=load_temp;
	W25QXX_Read(&load_temp,		LOAD_CTRL_P_REG,CHAR_REG_SIZE);	LOAD_CTRL_P=load_temp;
	W25QXX_Read(&load_temp,		FROZEN_MODE_REG,CHAR_REG_SIZE);	FROZEN_MODE=load_temp;
	
	#ifdef STM32_FLASH
	STMFLASH_Read(STM32_SAVE_ADDR,(u32*)stm32_read_datatemp,(FLASH_USE_SIZE/4));
	if( stm32_read_datatemp[FROZEN_MODE_REG]!=FROZEN_MODE||\
		stm32_read_datatemp[LOAD_CTRL_P_REG]!=LOAD_CTRL_P||\
		stm32_read_datatemp[LOAD_CTRL_I_REG]!=LOAD_CTRL_I||\
		stm32_read_datatemp[LOAD_CTRL_U_REG]!=LOAD_CTRL_U||\
		stm32_read_datatemp[VALLEY_TIME_H_REG]!=valley_time_H||\
		stm32_read_datatemp[VALLEY_TIME_L_REG]!=valley_time_L||\
		stm32_read_datatemp[PEAK_TIME_H_REG]!=peak_time_H||\
		stm32_read_datatemp[PEAK_TIME_L_REG]!=peak_time_L\
	)
	{
		SET_ERROR_FLAG;
		SSR_OFF();
		OLED_ShowString(1,0,"FLASH DATA ERR!");
		OLED_ShowString(1,4,"err.1");
		while(1);
	}
	for(i=0;i<4;i++)
	{
		if( stm32_read_datatemp[LAST_DC_POWER_REG+i]!=last_dc_power.b[i]||\
			stm32_read_datatemp[LAST_AC_VALLEY_POWER_REG+i]!=last_ac_valley_power.b[i]||\
			stm32_read_datatemp[LAST_AC_PEAK_POWER_REG+i]!=last_ac_peak_power.b[i]||\
			stm32_read_datatemp[LAST_AC_ALL_POWER_REG+i]!=last_ac_all_power.b[i]||\
			stm32_read_datatemp[DC_POWER_REG+i]!=dc_power.b[i]||\
			stm32_read_datatemp[AC_VALLEY_POWER_REG+i]!=ac_valley_power.b[i]||\
			stm32_read_datatemp[AC_PEAK_POWER_REG+i]!=ac_peak_power.b[i]||\
			stm32_read_datatemp[AC_ALL_POWER_REG+i]!=ac_all_power.b[i]\
		)
		{
			SET_ERROR_FLAG;
			SSR_OFF();
			OLED_ShowString(1,0,"FLASH DATA ERR!");
			OLED_ShowString(1,4,"err.2");
			while(1);
		}
	}
	#endif
	
}

//==================== Ӳ����ʼ�� =======================================================
void Initialization(void)
{
	NVIC_Configuration();	//�����ж�
	delay_init(168);	 	//��ʼ����ʱ����
	MyDelay_ms(2000);
	uart_init(115200);		//��ʼ������
	OLED_Init();		 	//��ʼ��OLED	
	OLED_ShowString(1,0,"start...");
	LED_Init();		     	//��ʼ��LED
	SSR_Init();				//��̬�̵������ų�ʼ�� 
	KEY_Init(); 			//��ʼ������		
	My_RTC_Init();		 	//��ʼ��RTC
	//PWR_PVD_Init();		//��ѹ�����PVD
	OLED_ShowString(1,0,"AC...");
	CS5464IOInit();			//CS5464���ų�ʼ��
	CS5464SPIInit();		//CS5464��ʼ��
	OLED_ShowString(1,0,"DC.");
	DC_CS5464_GPIO_Init();	//CS5464���ų�ʼ��
	OLED_ShowString(1,0,"DC..");
	DC_CS5464_Config();		//CS5464��ʼ��
	OLED_ShowString(1,0,"MODBUS...");
	eMBInit(MB_RTU, 0x01, 1, 115200, MB_PAR_NONE);	//MODBUS��ʼ��
	eMBEnable();			
	#ifdef DEBUG_FLASH		//�ⲿFLASH
	OLED_ShowString(1,0,"W25Q16...");
	W25QXX_Init();	 		//W25QXX��ʼ��
	while(W25QXX_ReadID()!=W25Q16)
	{
		delay_ms(2000);
		OLED_ShowString(1,0,"W25Q16 Error!");
	}
	Load_Data();			//��ȡ����
	#endif
	delay_ms(1000);			//��ʱ
	OLED_Clear();			
	#ifdef DEBUG_DOG		//���Ź�
	IWDG_Init(4,1500); 		
	#endif
}
//==================== ������ ===========================================================
int main(void)
{ 
	Initialization();	 	//Ӳ����ʼ��
	OSInit();   		 	//ϵͳ��ʼ��
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();				//����ϵͳ
}
//==================== ���� ============================================================
 //��ʼ����
void start_task(void *pdata)
{
	pdata = pdata; 
	OSStatInit(); 					//��ʼ��ͳ������.�������ʱ 1 ��������
  	OS_ENTER_CRITICAL();			//�����ٽ���    
	OSTaskCreate(OLED_task,		(void *)0,(OS_STK*)&OLED_TASK_STK[OLED_STK_SIZE-1],			OLED_TASK_PRIO);			
	OSTaskCreate(RTC_task,		(void *)0,(OS_STK*)&RTC_TASK_STK[RTC_STK_SIZE-1], 			RTC_TASK_PRIO);
	OSTaskCreate(Modbus_task,	(void *)0,(OS_STK*)&MODBUS_TASK_STK[MODBUS_STK_SIZE-1],		MODBUS_TASK_PRIO);	
	OSTaskCreate(Key_task,		(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1], 			KEY_TASK_PRIO);
	OSTaskCreate(CS5464_task,	(void *)0,(OS_STK*)&CS5464_TASK_STK[CS5464_STK_SIZE-1],		CS5464_TASK_PRIO);	
	OSTaskCreate(DC_CS5464_task,(void *)0,(OS_STK*)&DC_CS5464_TASK_STK[DC_CS5464_STK_SIZE-1],DC_CS5464_TASK_PRIO);	
 	OSTaskCreate(YC_task,		(void *)0,(OS_STK*)&YC_TASK_STK[YC_STK_SIZE-1], 			YC_TASK_PRIO);
	OSTaskCreate(FLASH_task,	(void *)0,(OS_STK*)&FLASH_TASK_STK[FLASH_STK_SIZE-1], 		FLASH_TASK_PRIO);
	OSTaskCreate(YK_task,		(void *)0,(OS_STK*)&YK_TASK_STK[YK_STK_SIZE-1], 			YK_TASK_PRIO);
	OSTaskCreate(LED0_task,		(void *)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],			LED0_TASK_PRIO);				
	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���
} 
//OLED����
//�����ջҪ����!
void OLED_task(void *pdata)
{	  
	
   u8 tbuf[48]={0};
   u8 cpu[3]={0};
	while(1)
	{
		OLED_Clear();
		if(OLEDPage==1)
		{
			cpu[0]=OSCPUUsage/100+48;
			cpu[1]=OSCPUUsage/100%10+48;
			cpu[2]=OSCPUUsage%10+48;
			OLED_ShowString(1,4,"CPU:");
			OLED_ShowString(32,4,cpu);				//��ʾCPUʹ����
			OLED_ShowString(56,4,"%");
			sprintf((char*)tbuf,"Time:%02d:%02d:%02d",RTC_TimeStruct.RTC_Hours,\
				RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds); 
			OLED_ShowString(1,0,tbuf);
			sprintf((char*)tbuf,"Date:20%02d-%02d-%02d",RTC_DateStruct.RTC_Year,\
				RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date); 	
			OLED_ShowString(1,2,tbuf);
			
		}else if(OLEDPage==2)
		{
			sprintf((char*)tbuf,"All   :%.3f",ac_all_power.f); 
			OLED_ShowString(1,0,tbuf);
			sprintf((char*)tbuf,"Peak  :%.3f",ac_peak_power.f); 
			OLED_ShowString(1,2,tbuf);
			sprintf((char*)tbuf,"Valley:%.3f",ac_valley_power.f); 
			OLED_ShowString(1,4,tbuf);
			sprintf((char*)tbuf,"DC    :%.3f",dc_power.f); 
			OLED_ShowString(1,6,tbuf);
		}else if(OLEDPage==3)
		{
			sprintf((char*)tbuf,"U  :%3.0f   V",AC_voltage); 
			OLED_ShowString(1,0,tbuf);
			sprintf((char*)tbuf,"I  :%3.2f  A",AC_current); 
			OLED_ShowString(1,2,tbuf);
			sprintf((char*)tbuf,"P  :%3.0f   W",activePower); 
			OLED_ShowString(1,4,tbuf);
			sprintf((char*)tbuf,"cos:%.3f",powerFactor); 
			OLED_ShowString(1,6,tbuf);
		}else if(OLEDPage==4)
		{
			sprintf((char*)tbuf,"DC V:%.1f V",DC_voltage); 
			OLED_ShowString(1,0,tbuf);
			sprintf((char*)tbuf,"DC I:%.2fmA",DC_current); 
			OLED_ShowString(1,2,tbuf);
			sprintf((char*)tbuf,"DC P:%.2fW",DC_activePower); 
			OLED_ShowString(1,4,tbuf);
			
		}if(OLEDPage==5)
		{
			sprintf((char*)tbuf,"LastAll :%.3f",last_ac_all_power.f); 
			OLED_ShowString(1,0,tbuf);
			sprintf((char*)tbuf,"LastPeak:%.3f",last_ac_peak_power.f); 
			OLED_ShowString(1,2,tbuf);
			sprintf((char*)tbuf,"LastVall:%.3f",last_ac_valley_power.f); 
			OLED_ShowString(1,4,tbuf);
			sprintf((char*)tbuf,"LastDC  :%.3f",last_dc_power.f); 
			OLED_ShowString(1,6,tbuf);
		}if(OLEDPage==6)
		{
			OLED_ShowString(1,4,"Author:");
			OLED_ShowString(1,6,"   Li Haolin");
		}
		delay_ms(1000);
		
	}
}
//LED0����
void LED0_task(void *pdata)
{	 	
	while(1)
	{
		delay_ms(10240);
	}
}
//����ɨ������
void Key_task(void *pdata)
{ 
	u8 key;
	while(1)
	{
		key=KEY_Scan(0);
		switch(key)
		{
			case 1:OLEDPage++;
					if(OLEDPage==7) OLEDPage=1;break;
			case 2:OLEDPage--;
					if(OLEDPage==0) OLEDPage=6;break;
			case 3:
					break;
			default:break;
		}
		delay_ms(300);
	}
}
//Modbus����
void Modbus_task(void *pdata)
{
	while(1)
	{
		(void)eMBPoll();
		OSTimeDlyHMSM(0,0,0,100);
	}										 
}	
//RTC����
//�õ���STM322F4�ڲ���RTC,��������,�����,�����������.����ҪУ׼��
//����1.�����Ҫ��Դ����,�˾��񾫶Ȳ���,���Կ����ö�ʱ������RTC�ľ���,�������붨ʱ��ʱ�ӵľ�׼��
//����2.����
//���黹����ר�ŵ�оƬ��,��,�ɿ� 
void RTC_task(void *pdata)
{
	while(1)
	{	
		RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);//��ȡʱ��
		RTC_GetDate(RTC_Format_BIN,&RTC_DateStruct);//��ȡ����
		if(RTC_FLAG==1)
		{
			OS_ENTER_CRITICAL();
			RTC_Set_Date(usRegHoldingBuf[0],usRegHoldingBuf[1],usRegHoldingBuf[2],usRegHoldingBuf[3]);	
			RTC_Set_Time(usRegHoldingBuf[4],usRegHoldingBuf[5],usRegHoldingBuf[6],RTC_H12_AM);	
			OS_EXIT_CRITICAL();				
			RESET_RTC_FLAG;										
		}
		delay_ms(1000);
	}										 
}

//CS5464����
//��������,������һЩ����,��ûȥ���,��ΪҪ��Ӳ��,ûʱ��Ū			
void CS5464_task(void *pdata)
{
//	u8 ac_get_falg=0;
	u8 ac_cs5464Status[3]={0};
	u8 ac_reg_value[3]={0};	
	while(1)
	{	
		ac_cs5464Status[0]=0;ac_cs5464Status[1]=0;ac_cs5464Status[2]=0;
		getCs5464Status(ac_cs5464Status);	
//		ac_get_falg++;
//		if((ac_cs5464Status[2]&0x01)==0)//���ˣ����ֲᣬ���ⲻ֪���˴���û���ã��жϺ��������ˣ�����
//		{
//			CS5464IOInit();
//			CS5464SPIInit();
//			delay_ms(500);
//		}
//		if((ac_cs5464Status[2]&0x04)==1)
//		{
//			CS5464IOInit();
//			CS5464SPIInit();
//			delay_ms(500);			
//		}
		if(ac_cs5464Status[0]&DRDY)
		{
			#ifdef DEBUG_DOG
			IWDG_Feed();					//ι��
			#endif
			getRegister(V1RMS ,ac_reg_value);	
			AC_voltage=(UnsignedConver(ac_reg_value)*386.8f);		//RMS��ѹͨ��1
			FloatToBinaryByU16Array(AC_voltage,&usRegInputBuf[0]);	
			getRegister(I1RMS ,ac_reg_value);						
			AC_current=(UnsignedConver(ac_reg_value)*16.1f);		//RMS����ͨ��1
			FloatToBinaryByU16Array(AC_current,&usRegInputBuf[2]);		
			getRegister(P1AVG,ac_reg_value);						
			activePower=(ComplementConver(ac_reg_value)*6188.04f);		//�й�����ͨ��1
			FloatToBinaryByU16Array(activePower,&usRegInputBuf[4]);
			getRegister(Q1AVG,ac_reg_value);	
			reactivePower=(ComplementConver(ac_reg_value)*6188.04f);	//�޹�����ͨ��1
			FloatToBinaryByU16Array(reactivePower,&usRegInputBuf[6]);	
			getRegister(S1,ac_reg_value);		
			apparentPower=(ComplementConver(ac_reg_value)*6188.04f);	//���ڹ���ͨ��
			FloatToBinaryByU16Array(apparentPower,&usRegInputBuf[8]);	
			getRegister(PF1,ac_reg_value);		
			powerFactor=ComplementConver(ac_reg_value);				//����ϵ��ͨ��1
			FloatToBinaryByU16Array(powerFactor,&usRegInputBuf[10]);
			getRegister(EPULSE,ac_reg_value);	
			float temp_power=(ComplementConver(ac_reg_value)*6188.04f*1.00064f/3600000.0f);
			ac_all_power.f=ac_all_power.f+temp_power;
			if(RTC_TimeStruct.RTC_Hours>=peak_time_L&&RTC_TimeStruct.RTC_Hours<peak_time_H)
			{
				ac_peak_power.f=ac_peak_power.f+temp_power;
			}else if(RTC_TimeStruct.RTC_Hours>=valley_time_L||RTC_TimeStruct.RTC_Hours<valley_time_H)
			{
				ac_valley_power.f=ac_valley_power.f+temp_power;
			}
			FloatToBinaryByU16Array(ac_all_power.f,&usRegInputBuf[12]);
			FloatToBinaryByU16Array(ac_peak_power.f,&usRegInputBuf[14]);
			FloatToBinaryByU16Array(ac_valley_power.f,&usRegInputBuf[16]);
			clearCs5464Status(ac_cs5464Status);//���
//			ac_get_falg=0;
			LED0=!LED0;
		}
//		if((ac_cs5464Status[1]&I1FAULT)||(ac_cs5464Status[1]&V1SAG))//��������//��ѹ����
//		{
//			CS5464IOInit();
//			CS5464SPIInit();
//			clearCs5464Status(ac_cs5464Status);
//			delay_ms(500);
//		}
//		if(ac_get_falg>=2)
//		{
//			CS5464IOInit();
//			CS5464SPIInit();
//			ac_get_falg=0;
//			delay_ms(500);
//		}
		
		delay_ms(1000);
	}										 
}
void YC_task(void *pdata)
{
	while(1)
	{
		if(GET_RTC_FLAG==1)
		{
			usRegHoldingBuf[0]=RTC_DateStruct.RTC_Year;//��ȡʱ�䣺��
			usRegHoldingBuf[1]=RTC_DateStruct.RTC_Month;
			usRegHoldingBuf[2]=RTC_DateStruct.RTC_Date;
			usRegHoldingBuf[3]=RTC_DateStruct.RTC_WeekDay;
			usRegHoldingBuf[4]=RTC_TimeStruct.RTC_Hours;
			usRegHoldingBuf[5]=RTC_TimeStruct.RTC_Minutes;
			usRegHoldingBuf[6]=RTC_TimeStruct.RTC_Seconds;
			RESET_GET_RTC_FLAG;
		}
		if(GET_TIME_FLAG==1)
		{
			PEAK_TIME_L=peak_time_L;
			PEAK_TIME_H=peak_time_H;
			VALLEY_TIME_L=valley_time_L;
			VALLEY_TIME_H=valley_time_H;
			RESET_GET_TIME_FLAG;
		}
		delay_ms(200);
	}

}

//FLASH����
void FLASH_task(void *pdata)
{	 	
	u8 mode0_write_flag=0;
	u8 mode1_write_flag=0;
	u8 i=0;
	while(1)
	{
		if(RESET_FLAG==1)
		{
			#ifdef DEBUG_FLASH
			OS_ENTER_CRITICAL();
			ac_all_power.f=0.0f;//Ĭ��ֵ
			ac_peak_power.f=0.0f;
			ac_valley_power.f=0.0f;
			peak_time_L=8;peak_time_H=22;		
			valley_time_L=22;valley_time_H=8;
			for(i=0;i<3;i++)
			{
				W25QXX_Write(ac_all_power.b,	AC_ALL_POWER_REG+FLASH_SAVE_SIZE*i,		FLOAT_REG_SIZE);	
				W25QXX_Write(ac_peak_power.b,	AC_PEAK_POWER_REG+FLASH_SAVE_SIZE*i,	FLOAT_REG_SIZE);	
				W25QXX_Write(ac_valley_power.b,	AC_VALLEY_POWER_REG+FLASH_SAVE_SIZE*i,	FLOAT_REG_SIZE);
				W25QXX_Write(dc_power.b,		DC_POWER_REG+FLASH_SAVE_SIZE*i,			FLOAT_REG_SIZE);
				W25QXX_Write(last_ac_all_power.b,	LAST_AC_ALL_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
				W25QXX_Write(last_ac_peak_power.b,	LAST_AC_PEAK_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
				W25QXX_Write(last_ac_valley_power.b,LAST_AC_VALLEY_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
				W25QXX_Write(last_dc_power.b,		LAST_DC_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
				W25QXX_Write(&peak_time_L,PEAK_TIME_L_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				W25QXX_Write(&peak_time_H,PEAK_TIME_H_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				W25QXX_Write(&valley_time_L,VALLEY_TIME_L_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				W25QXX_Write(&valley_time_H,VALLEY_TIME_H_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				load_temp=0xff;
				W25QXX_Write(&load_temp,LOAD_CTRL_U_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				W25QXX_Write(&load_temp,LOAD_CTRL_I_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				W25QXX_Write(&load_temp,LOAD_CTRL_P_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				load_temp=0;
				W25QXX_Write(&load_temp,FROZEN_MODE_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
			}
			OS_EXIT_CRITICAL();	
			#endif
			RESET_RESET_FLAG;
		}
		if(TIME_FLAG==1)
		{
			#ifdef DEBUG_FLASH
			OS_ENTER_CRITICAL();
			peak_time_L=PEAK_TIME_L;//��
			peak_time_H=PEAK_TIME_H;
			valley_time_L=VALLEY_TIME_L;//��
			valley_time_H=VALLEY_TIME_H;
			for(i=0;i<3;i++)//д��
			{
				W25QXX_Write(&peak_time_L,PEAK_TIME_L_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				W25QXX_Write(&peak_time_H,PEAK_TIME_H_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				W25QXX_Write(&valley_time_L,VALLEY_TIME_L_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				W25QXX_Write(&valley_time_H,VALLEY_TIME_H_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
			}
			OS_EXIT_CRITICAL();	
			#endif
			RESET_TIME_FLAG;
		}
		if(LOAD_CTRL_FLAG==1)
		{
			#ifdef DEBUG_FLASH
			OS_ENTER_CRITICAL();
			for(i=0;i<3;i++)
			{
				load_temp=LOAD_CTRL_U;
				W25QXX_Write(&load_temp,LOAD_CTRL_U_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				load_temp=LOAD_CTRL_I;
				W25QXX_Write(&load_temp,LOAD_CTRL_I_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
				load_temp=LOAD_CTRL_P;
				W25QXX_Write(&load_temp,LOAD_CTRL_P_REG+FLASH_SAVE_SIZE*i,CHAR_REG_SIZE);	
			}
			OS_EXIT_CRITICAL();	
			#endif
			RESET_LOAD_CTRL_FLAG;
		}
		#ifdef DEBUG_FLASH
		if(FROZEN_MODE==0)
		{
			if(RTC_TimeStruct.RTC_Hours==0&&mode0_write_flag==0)
			{
				mode0_write_flag=1;
				OS_ENTER_CRITICAL();
				for(i=0;i<3;i++)
				{
					W25QXX_Write(ac_all_power.b,AC_ALL_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
					W25QXX_Write(ac_peak_power.b,AC_PEAK_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
					W25QXX_Write(ac_valley_power.b,AC_VALLEY_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
					W25QXX_Write(dc_power.b,DC_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
				}
				OS_EXIT_CRITICAL();	
			}
			if(RTC_TimeStruct.RTC_Hours!=0)
			{
				mode0_write_flag=0;
			}
			mode1_write_flag=0;
		}else if(FROZEN_MODE==1)
		{
			
			if(RTC_TimeStruct.RTC_Minutes==0&&mode1_write_flag==0)
			{
				mode1_write_flag=1;
				OS_ENTER_CRITICAL();
				for(i=0;i<3;i++)
				{
					W25QXX_Write(ac_all_power.b,AC_ALL_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
					W25QXX_Write(ac_peak_power.b,AC_PEAK_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
					W25QXX_Write(ac_valley_power.b,AC_VALLEY_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
					W25QXX_Write(dc_power.b,DC_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
				}
				OS_EXIT_CRITICAL();	
			}
			if(RTC_TimeStruct.RTC_Minutes!=0)
			{
				mode1_write_flag=0;
			}
			mode0_write_flag=0;
		}
		#endif
		if(RTC_DateStruct.RTC_Date==1&&RTC_TimeStruct.RTC_Hours==0&&RTC_TimeStruct.RTC_Minutes==0&&write_last_flag==0)
		{
			write_last_flag=1;
			last_ac_all_power.f=ac_all_power.f;
			last_ac_peak_power.f=ac_peak_power.f;
			last_ac_valley_power.f=ac_valley_power.f;
			last_dc_power.f=dc_power.f;
			FloatToBinaryByU16Array(last_ac_all_power.f,&usRegInputBuf[26]);	
			FloatToBinaryByU16Array(last_ac_peak_power.f,&usRegInputBuf[28]);	
			FloatToBinaryByU16Array(last_ac_valley_power.f,&usRegInputBuf[30]);	
			FloatToBinaryByU16Array(last_dc_power.f,&usRegInputBuf[32]);	
			#ifdef DEBUG_FLASH
			OS_ENTER_CRITICAL();
				for(i=0;i<3;i++)
				{
					W25QXX_Write(last_ac_all_power.b,LAST_AC_ALL_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
					W25QXX_Write(last_ac_peak_power.b,LAST_AC_PEAK_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);	
					W25QXX_Write(last_ac_valley_power.b,LAST_AC_VALLEY_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
					W25QXX_Write(last_dc_power.b,LAST_DC_POWER_REG+FLASH_SAVE_SIZE*i,FLOAT_REG_SIZE);
				}
			OS_EXIT_CRITICAL();		
			#endif
		}
		if(RTC_DateStruct.RTC_Date!=1)
		{
			write_last_flag=0;
		}
		delay_ms(2000);
	}
}
//ң������
void YK_task(void *pdata)
{	 	
	while(1)
	{
		if(SSR_FLAG==1)
		{
			SSR_ON();//�򿪼̵���
		}else
		{
			SSR_OFF();//�رռ̵���
		}
		if(AC_voltage>=LOAD_CTRL_U)
		{
			SSR_OFF();
		
		}
		if(AC_current>=LOAD_CTRL_I)
		{
			SSR_OFF();
		
		}
		if(activePower>=LOAD_CTRL_P*1000)
		{
			SSR_OFF();
		}
		delay_ms(150);
	}
}
void DC_CS5464_task(void *pdata)
{	
	u8 dc_cs5464Status[3]={0};	
	u8 dc_reg_value[3]={0};	
	while(1)
	{	
		dc_cs5464Status[0]=0;dc_cs5464Status[1]=0;dc_cs5464Status[2]=0;
		Get_DC_CS5464_Status(dc_cs5464Status);	
		if(dc_cs5464Status[0]&DRDY)
		{
			LED1=!LED1;
			Get_DC_CS6464_Register(V1,dc_reg_value);	DC_voltage=ComplementConver(dc_reg_value)*39.99f;		FloatToBinaryByU16Array(DC_voltage,&usRegInputBuf[18]);//��ѹ
			Get_DC_CS6464_Register(I1,dc_reg_value);	DC_current=ComplementConver(dc_reg_value)*1112.1f;		FloatToBinaryByU16Array(DC_current,&usRegInputBuf[20]);//����
			Get_DC_CS6464_Register(P1AVG,dc_reg_value);	DC_activePower=ComplementConver(dc_reg_value)*44.473f;	FloatToBinaryByU16Array(DC_activePower,&usRegInputBuf[22]);//ֱ������
			float power=DC_activePower/3600000.0f;		dc_power.f=dc_power.f+power;						FloatToBinaryByU16Array(dc_power.f,&usRegInputBuf[24]);//ֱ������
			Clear_DC_CS5464_Status(dc_cs5464Status);
		}
		delay_ms(1000);
	}
}




































































/* ���,����~~~
����������   ���������Щ������Щ������Щ������� ���������Щ������Щ������Щ������� ���������Щ������Щ������Щ������� ���������Щ������Щ������� 
��Esc   ��   �� F1   �� F2   �� F3   �� F4   �� �� F5   �� F6   �� F7   �� F8   �� �� F9   ��F10   ��F11   ��F12   �� ��P/S   ��S L   ��P/B   ��       ����    ����    ���� 
����������   ���������ة������ة������ة������� ���������ة������ة������ة������� ���������ة������ة������ة������� ���������ة������ة�������       ����    ����    ���� 
���������Щ������Щ������Щ������Щ������Щ������Щ������Щ������Щ������Щ������Щ������Щ������Щ������Щ��������������� ���������Щ������Щ������� ���������Щ������Щ������Щ������� 
��~ `   ��! 1   ��@ 2   ��# 3   ��$ 4   ��% 5   ��^ 6   ��& 7   ��* 8   ��( 9   ��) 0   ��_ -   ��+ =   �� BacSp        �� ��Ins   ��Hom   ��PUp   �� ��N L   �� /    �� *    �� -    ��
���������ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ����������� ���������੤�����੤������ ���������੤�����੤�����੤������ 
�� Tab      ��   Q  ��   W  ��   E  ��   R  ��   T  ��   Y  ��   U  ��   I  ��   O  ��   P  ��  { [ ��  } ] ��   | \    �� ��Del   ��End   ��PDn   �� �� 7    �� 8    �� 9    ��      �� 
�������������ةЩ����ةЩ����ةЩ����ةЩ����ةЩ����ةЩ����ةЩ����ةЩ����ةЩ����ةЩ����ةЩ����ةЩ����ة����������� ���������ة������ة������� ���������੤�����੤������ +    �� 
�� Caps �� A  ��   S  ��   D  ��   F  ��   G  ��   H  ��   J  ��   K  ��   L  ��  : ; ��  " ' ��              Enter     ��                            �� 4    �� 5    �� 6    ��      �� 
���������������ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة��Щ��ة�����������������         ����������         ���������੤�����੤�����੤������ 
�� Shift          ��   Z  ��   X  ��   C  ��   V  ��   B  ��   N  ��   M  ��  <  ,��  >  .��  ? / ��      Shift         ��         �� ��   ��         �� 1    �� 2    �� 3    ��      �� 
�������������Щ����ة��Щ��ة����Щة������ة������ة������ة������ة������ة����Щة������੤�����ةЩ��������Щ��������� ���������੤�����੤������ ���������ة������੤������ E  ���� 
�� Ctrl     ��        ��   Alt  ��                Space                         ��   Alt  ��        ��        ��   Ctrl �� �� ��   �� ��   �� ��   �� ��   0          �� .    ���������� 
�������������ة��������ة��������ة����������������������������������������������ة��������ة��������ة��������ة��������� ���������ة������ة������� �����������������ة������ة������� 
*/
