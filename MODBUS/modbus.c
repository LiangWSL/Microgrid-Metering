//========================================================================
// �ļ���: modbus.c
// ��  ��: 
// ��  ��: 
// ��  ��: ��Ҫʵ����4��modbus�ص�����
//
// ��  ��:
//      
//========================================================================
#include "modbus.h"
extern void xMBUtilSetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits, UCHAR ucValue );
extern UCHAR xMBUtilGetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits );
USHORT usRegInputBuf[REG_INPUT_NREGS]={0};
uint16_t usRegHoldingBuf[REG_HOLDING_NREGS]= {0};
UCHAR ucRegCoilsBuf[REG_COILS_SIZE]={0};
UCHAR ucRegDiscreteBuf[REG_DISCRETE_SIZE]= {0};
//////////////////////////////////////////////////////////////////////////////////////////
///
//void NVIC_Configuration(void)
//{
//  NVIC_InitTypeDef NVIC_InitStructure;
//  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
//  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);	
//	
//	//Usart1 NVIC ����
//    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�0
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�2
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
//	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���	

//// 	//TIM2 
//	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	
//	NVIC_Init(&NVIC_InitStructure);	
//}
/****************************************************************************
* ��	  �ƣ�eMBRegInputCB
* ��    �ܣ���ȡ����Ĵ�������Ӧ�������� 04 eMBFuncReadInputRegister
* ��ڲ�����pucRegBuffer: ���ݻ�������������Ӧ����   
*						usAddress: �Ĵ�����ַ
*						usNRegs: Ҫ��ȡ�ļĴ�������
* ���ڲ�����
* ע	  �⣺��λ�������� ֡��ʽ��: SlaveAddr(1 Byte)+FuncCode(1 Byte)
*								+StartAddrHiByte(1 Byte)+StartAddrLoByte(1 Byte)
*								+LenAddrHiByte(1 Byte)+LenAddrLoByte(1 Byte)+
*								+CRCAddrHiByte(1 Byte)+CRCAddrLoByte(1 Byte)
*							3 ��
****************************************************************************/
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START ) && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - REG_INPUT_START );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( UCHAR )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ = ( UCHAR )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/****************************************************************************
* ��	  �ƣ�eMBRegHoldingCB
* ��    �ܣ���Ӧ��������  06 д���ּĴ��� eMBFuncWriteHoldingRegister 
*						 16 д������ּĴ��� eMBFuncWriteMultipleHoldingRegister
*						 03 �����ּĴ��� eMBFuncReadHoldingRegister
*						 23 ��д������ּĴ��� eMBFuncReadWriteMultipleHoldingRegister
* ��ڲ�����pucRegBuffer: ���ݻ�������������Ӧ����   
*						usAddress: �Ĵ�����ַ
*						usNRegs: Ҫ��д�ļĴ�������
*						eMode: ������
* ���ڲ�����
* ע	  �⣺4 ��
****************************************************************************/
eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
	eMBErrorCode    eStatus = MB_ENOERR;
	int             iRegIndex;
	if((usAddress >= REG_HOLDING_START)&&\
	((usAddress+usNRegs) <= (REG_HOLDING_START + REG_HOLDING_NREGS)))
	{
		iRegIndex = (int)(usAddress - REG_HOLDING_START);
		switch(eMode)
		{                                       
			case MB_REG_READ://�� MB_REG_READ = 0
        while(usNRegs > 0)
				{
					*pucRegBuffer++ = (u8)(usRegHoldingBuf[iRegIndex] >> 8);            
					*pucRegBuffer++ = (u8)(usRegHoldingBuf[iRegIndex] & 0xFF); 
					iRegIndex++;
					usNRegs--;					
				}                            
        break;
			case MB_REG_WRITE://д MB_REG_WRITE = 0
				while(usNRegs > 0)
				{         
					usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
					usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
					iRegIndex++;
					usNRegs--;
        }				
			}
	}
	else//����
	{
		eStatus = MB_ENOREG;
	}	
	
	return eStatus;
}

/****************************************************************************
* ��	  �ƣ�eMBRegCoilsCB
* ��    �ܣ���Ӧ�������� 01 ����Ȧ eMBFuncReadCoils
*						 05 д��Ȧ eMBFuncWriteCoil
*						 15 д�����Ȧ eMBFuncWriteMultipleCoils
* ��ڲ�����pucRegBuffer: ���ݻ�������������Ӧ����   
*						usAddress: ��Ȧ��ַ
*						usNCoils: Ҫ��д����Ȧ����
*						eMode: ������
* ���ڲ�����
* ע	  �⣺ 
*						0 ��
****************************************************************************/

/** 
* @brief ��Ȧ�Ĵ���������
* @param pucRegBuffer ������---��������ָ�룬д����--��������ָ�� 
* usAddress �Ĵ�����ʼ��ַ 
* usNRegs �Ĵ������� 
* eMode ������ʽ
* @retval eStatus �Ĵ���״̬ 
*/  
eMBErrorCode  eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,  eMBRegisterMode eMode )  
{  
	//����״̬  
	eMBErrorCode eStatus = MB_ENOERR;  
	//�Ĵ�������  
	int16_t iNCoils = ( int16_t )usNCoils;  
	//�Ĵ���ƫ����  
	int16_t usBitOffset;  
  
	//���Ĵ����Ƿ���ָ����Χ��  
	if( ( (int16_t)usAddress >= REG_COILS_START ) &&  ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )  
	{  
		//����Ĵ���ƫ����  
		usBitOffset = ( int16_t )( usAddress - REG_COILS_START );  
		switch ( eMode )  
		{  
			case MB_REG_READ:  
			while( iNCoils > 0 )  
			{  
				*pucRegBuffer++ = xMBUtilGetBits(ucRegCoilsBuf, usBitOffset,(uint8_t)(iNCoils > 8 ? 8 : iNCoils));  
				iNCoils -= 8;  
				usBitOffset += 8;  
			}  
			break;  
			case MB_REG_WRITE:  
			while( iNCoils > 0 )  
			{  
				xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,  ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ),  *pucRegBuffer++ );  
				iNCoils -= 8;  
			}  
			break;  
		}  
	}  
	else  
	{  
		eStatus = MB_ENOREG;  
	}  
	return eStatus;  
}  
 /****************************************************************************
* ��	  �ƣ�eMBRegDiscreteCB
* ��    �ܣ���ȡ��ɢ�Ĵ�������Ӧ�������У�02 ����ɢ�Ĵ��� eMBFuncReadDiscreteInputs
* ��ڲ�����pucRegBuffer: ���ݻ�������������Ӧ����   
*						usAddress: �Ĵ�����ַ
*						usNDiscrete: Ҫ��ȡ�ļĴ�������
* ���ڲ�����
* ע	  �⣺1 ��
****************************************************************************/
 
/** 
* @brief ��������Ĵ�������������������Ĵ������ɶ� 
* @param pucRegBuffer ������---��������ָ�룬д����--��������ָ�� 
* usAddress �Ĵ�����ʼ��ַ 
* usNRegs �Ĵ������� 
* eMode ������ʽ��������д 
* @retval eStatus �Ĵ���״̬ 
*/  
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )  
{  
	//����״̬  
	eMBErrorCode eStatus = MB_ENOERR;  
	//�����Ĵ�������  
	int16_t iNDiscrete = ( int16_t )usNDiscrete;  
	//ƫ����  
	uint16_t usBitOffset;  
	  
	//�жϼĴ���ʱ�����ƶ���Χ��  
	if( ( (int16_t)usAddress >= REG_DISCRETE_START ) &&  ( usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE ) )  
	{  
		usBitOffset = ( uint16_t )( usAddress - REG_DISCRETE_START );  
		  
		while( iNDiscrete > 0 )  
		{  
			*pucRegBuffer++ = xMBUtilGetBits( ucRegDiscreteBuf, usBitOffset,  ( uint8_t)( iNDiscrete > 8 ? 8 : iNDiscrete ) );  
			iNDiscrete -= 8;  
			usBitOffset += 8;  
		}  
	}  
	else  
	{  
		eStatus = MB_ENOREG;  
	}  
	return eStatus;  
}  

