#include "exti.h"
/**
*����:���õ�����
*����:
*����:
**/
void PWR_PVD_Init(void) 
{ 
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//ʹ��PWRʱ��
	
	EXTI_StructInit(&EXTI_InitStructure);
	EXTI_InitStructure.EXTI_Line = EXTI_Line16;             //PVD���ӵ��ж���16��
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;     //ʹ���ж�ģʽ
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  //��ѹ���ڷ�ֵʱ�����ж�
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;               //ʹ���ж���
	EXTI_Init(&EXTI_InitStructure);                         //��ʼ

	PWR_PVDLevelConfig(PWR_PVDLevel_7);//�趨��ط�ֵ 
	PWR_PVDCmd(ENABLE);//ʹ��PVD     
}


