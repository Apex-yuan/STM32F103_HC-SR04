#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "stm32f10x.h"

//�˿ں궨��
#define UL_PORT        GPIOA
#define UL_GPIO_CLK    RCC_APB2Periph_GPIOA
#define UL_GPIO_CLK_FUNCTION  RCC_APB2PeriphClockCmd
#define UL_GPIO_PORT_SOURCE  GPIO_PortSourceGPIOA

#define UL_TIM     TIM2
#define UL_TIM_CLK RCC_APB1Periph_TIM2
#define UL_TIM_CLK_FUNCTION RCC_APB1PeriphClockCmd

#define UL1_TRIG_PIN   GPIO_Pin_4
#define UL1_ECHO_PIN   GPIO_Pin_5
#define UL1_EXTI_LINE  EXTI_Line5
#define UL1_GPIO_PIN_SOURCE  GPIO_PinSource5 

#define UL2_TRIG_PIN   GPIO_Pin_6
#define UL2_ECHO_PIN   GPIO_Pin_7
#define UL2_EXTI_LINE  EXTI_Line7
#define UL2_GPIO_PIN_SOURCE  GPIO_PinSource7 

#define UL_EXTI_IRQ    EXTI9_5_IRQn

//
#define UL_NUM 2
#define UL_TIM_ARR 9         //�ж����ڣ�1us*��9+1��= 10us 
#define UL_TIM_PSC 71        //����Ƶ�ʣ�72M/(71+1) = 1000000������һ������ʱ��Ϊ��1S/1000000=1us
#define MAX_MEASURE_DISTANCE 255      //��λ��cm
#define UL_TIM_MAX_COUNT 1500 //�����������Ӧ�Ķ�ʱ������  //cnt = 255cm * 2 /(340 * 100) * 100000��


typedef struct
{
  float distance;        //��������ֵ
  uint8_t flag;          //�����ش�����־λ
  uint32_t start_time;   //��ʼ����ʱ���
  uint32_t end_time;     //��������ʱ���
}UltrasonicDef;

extern UltrasonicDef ultrasonic[UL_NUM];


void ultrasonic_init(void);
void ultrasonic_startMeasure(void);



#endif /* __ULTRASONIC_H*/





