#ifndef __LED_H
#define __LED_H
#include "sys.h"

//LED�˿ڶ���
#define LED0 PAout(6)	// D2
#define LED1 PAout(7)	// D3	 
void LED_Init(void);

//��̬�̵����˿ڶ���
extern void SSR_Init(void);
#define SSR_ON()	GPIO_SetBits(GPIOD,GPIO_Pin_0)//�����
#define SSR_OFF()	GPIO_ResetBits(GPIOD,GPIO_Pin_0)//�����
#endif
