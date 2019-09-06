#include "bsp_hc_sr04.h"
#include "bsp_systick.h"

/*
  说明：
  1. 当前超声波测量采用轮询测量方式。
  2. 由示波器测量可知：从测量起始信号发出到超声波模块产生回响信号的时间间隔约为470us，因而轮询测量的两个超声波之间的测量时
     间间隔应该≥470us，目前程序中设定为500us（该时间可以确保下一个信号还没发出时，上一个超声波已经进入了测量中断，待中断
     处理完成，回到打断位置继续执行，然后发出下一个测量信号）。轮询机制决定了单片机不能同时处理不同的回响信号，若单片机很快
     的发出两个测量起始信号，先收到回响信号的会先进入中断，中断中会进行等待处理（此处需要较长的时间，当前程序为：0-15ms），
     在当前中断处理过程中下一个中断也会触发，该中断只能等待到上个中断执行完成时才会进入该中断函数，因此并不能完整记录当前
     回响信号高电平的时长，甚至还未进入该中断函数时高电平信号已经消失了，因此测的的距离返回值为0或小于实测距离的值（具体取
     决于上一个模块的测量时长）。
  3. 由示波器测量可知HC-SR04有时候的回响信号会持续140ms（对应的距离值为：2400cm）左右，轮询测量过程中使用while循环来等待，
     因此必须有跳出机制，当前函数的处理可以很好的避免该问题（超出测量的最大距离时，便会跳出while循环）
 */

float distance_ultrasonic[UL_NUM];

//由于只用到了TIM2的定时器功能，所以没有必要开启定时器更新中断
static void ultrasonic_timConfig(uint16_t arr, uint16_t psc)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  // NVIC_InitTypeDef NVIC_InitStruct;
  
  UL_TIM_CLK_FUNCTION(UL_TIM_CLK, ENABLE);
  
  // NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
  // NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  // NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  // NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  // NVIC_Init(&NVIC_InitStruct);
  
  TIM_TimeBaseInitStruct.TIM_Period = arr;
  TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(UL_TIM, &TIM_TimeBaseInitStruct);
  
  //TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  TIM_Cmd(UL_TIM, DISABLE); //默认关闭TIM2
}

static void ultrasonic_gpioConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  UL_GPIO_CLK_FUNCTION(UL_GPIO_CLK, ENABLE);
  
  GPIO_InitStruct.GPIO_Pin = UL1_TRIG_PIN | UL2_TRIG_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;         //推挽输出
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(UL_PORT, &GPIO_InitStruct);
  GPIO_ResetBits(UL_PORT, UL1_TRIG_PIN | UL2_TRIG_PIN); //复位触发引脚
  
  GPIO_InitStruct.GPIO_Pin = UL1_ECHO_PIN | UL2_ECHO_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(UL_PORT, &GPIO_InitStruct);
}

static void ultrasonic_extiConfig(void)
{
  EXTI_InitTypeDef EXTI_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  //EXTI需要使能AFIO时钟
  
  GPIO_EXTILineConfig(UL_GPIO_PORT_SOURCE, UL1_GPIO_PIN_SOURCE | UL2_GPIO_PIN_SOURCE); //将中断线连接到GPIO端口
  
  EXTI_InitStruct.EXTI_Line = UL1_EXTI_LINE | UL2_EXTI_LINE;
  EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStruct.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStruct);
  
  NVIC_InitStruct.NVIC_IRQChannel = UL_EXTI_IRQ;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}

void ultrasonic_init(void)
{
  ultrasonic_gpioConfig();
  ultrasonic_extiConfig();
  ultrasonic_timConfig(UL_TIM_ARR, UL_TIM_PSC);   //初始化TIM2定时器，计数一次为1/100000S（10us）：1/(72M/(719+1))s
}
	
//依次发送触发信号，轮询测量距离值。根据测量机制，该函数执行完毕，整个测量过程已经执行完成。
void ultrasonic_startMeasure(void)
{
	GPIO_SetBits(UL_PORT, UL1_TRIG_PIN);
	delay_us(10);
	GPIO_ResetBits(UL_PORT, UL1_TRIG_PIN);
  delay_us(MEASURE_INTERVAL); //延时保证UL1已经进入中断

  GPIO_SetBits(UL_PORT, UL2_TRIG_PIN);
  delay_us(10);
  GPIO_ResetBits(UL_PORT, UL2_TRIG_PIN);
  delay_us(MEASURE_INTERVAL); //延时保证UL2已经进入中断
}

// void TIM2_IRQHandler(void)
// {
//   if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
//   {
//     TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
//     /* 目前中断函数中不需要做任何处理 */
//   }
// }

void EXTI9_5_IRQHandler(void)
{		
  /*Ultrasonic 1*/	
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
    EXTI_ClearITPendingBit(EXTI_Line5);  
		TIM_SetCounter(TIM2, 0);
		TIM_Cmd(TIM2, ENABLE);
		while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5))  //上升沿触发中断后等待变为低电平
    {
      if(TIM_GetCounter(TIM2) >= UL_TIM_MAX_COUNT) // cnt = 255cm * 2 /(340 * 100) * 100000
      {
        break;  //超出设定的测量最大距离，跳出等待
      }
    }
		TIM_Cmd(TIM2,DISABLE);
    /* 计算超声波测量到的距离，如超出最大距离按最大距离显示 */
		distance_ultrasonic[0] = TIM_GetCounter(TIM2) * 340 / 2000.0;  //cnt * 1/100000 * 340 / 2 *100(单位：cm)
	}
  
  /* Ultrosonic 2 */
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
    EXTI_ClearITPendingBit(EXTI_Line7); 
		TIM_SetCounter(TIM2, 0);
		TIM_Cmd(TIM2, ENABLE);
		while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))  //上升沿触发中断后等待变为低电平
    {
      if(TIM_GetCounter(TIM2) >= UL_TIM_MAX_COUNT) // cnt = 255cm * 2 /(340 * 100) * 100000
      {
        break;  //超出设定的测量最大距离，跳出等待
      }
    }
		TIM_Cmd(TIM2,DISABLE);
    /* 计算超声波测量到的距离，如超出最大距离按最大距离显示 */
		distance_ultrasonic[1] = TIM_GetCounter(TIM2) * 340 / 2000.0;  //cnt * 1/100000 * 340 / 2 *100(单位：cm)
  } 
}

