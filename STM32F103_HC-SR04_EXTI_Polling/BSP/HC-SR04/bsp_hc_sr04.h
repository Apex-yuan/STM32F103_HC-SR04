#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "stm32f10x.h"

//端口宏定义
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
#define UL_TIM_ARR 0xffff     //将周期设为最大值
#define UL_TIM_PSC 719        //计数频率：72M/(719+1) = 100000，即记一个数的时间为：1S/100000=10us
#define MAX_MEASURE_DISTANCE 255      //单位：cm
#define UL_TIM_MAX_COUNT 1500 //最大测量距离对应的定时器计数  //cnt = 255cm * 2 /(340 * 100) * 100000
/* 该时间设置非常关键，当前设定时间要确保上个超声波已经进入了中断（因为进入了中断就会自动等到测量完成），
   待上个测量完成后才能开启下个超声波的测量。否则两个测量信号都发送出去，外部中断会依次触发先执行先收到
   触发信号的中断服务函数，另一个中断会等到当前中断执行完毕之后才会进入，这时后面信号的高电平时间可能很
   小或没有了，因而收到的数据会显示0或较小的数值。 */
#define MEASURE_INTERVAL 500  //单位：us  //由实验测量大于等于500us较为合适，

extern float distance_ultrasonic[];


void ultrasonic_init(void);
void ultrasonic_startMeasure(void);



#endif /* __ULTRASONIC_H*/





