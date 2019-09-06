#include "bsp_hc_sr04.h"
#include "bsp_systick.h"

/*
  说明：
  1. 当前超声波测量采用同步测量方式，注意外部中断触发必须选择上升沿下降沿都可以触发。
  2. 若设定测量最大距离为255cm，对应的时间为15ms。不管有多少个超声波模块，该测量方式理论上最快的测量周期为15ms。
     但根据该模块使用手册的测量周期建议为60ms以上，因此尽量使模块的测量周期尽量处于该范围内
  3. 该测量方式有一个明显的缺点是，不同模块之间的回波可能会产生互相干扰的情况，解决方法：各个模块之间尽可能大角度
     朝不同的方向。
 */
static __IO uint32_t tim2_10us_tick = 0;  //系统计数器，记录系统运行时间10us为单位
UltrasonicDef ultrasonic[UL_NUM] = {0};

//由于只用到了TIM2的定时器功能，所以没有必要开启定时器更新中断
static void ultrasonic_timConfig(uint16_t arr, uint16_t psc)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;
  
  UL_TIM_CLK_FUNCTION(UL_TIM_CLK, ENABLE);
  
  NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  
  TIM_TimeBaseInitStruct.TIM_Period = arr;
  TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(UL_TIM, &TIM_TimeBaseInitStruct);
  
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  TIM_Cmd(UL_TIM, ENABLE); //开启TIM2
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
  EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
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
  ultrasonic_timConfig(UL_TIM_ARR, UL_TIM_PSC);   //初始化TIM2定时器
}
	
//同步测量可以依次发送触发信号，而中间不用加延时函数
void ultrasonic_startMeasure(void)
{
	GPIO_SetBits(UL_PORT, UL1_TRIG_PIN);
	delay_us(15);
	GPIO_ResetBits(UL_PORT, UL1_TRIG_PIN);

  GPIO_SetBits(UL_PORT, UL2_TRIG_PIN);
  delay_us(15);
  GPIO_ResetBits(UL_PORT, UL2_TRIG_PIN);
}

void TIM2_IRQHandler(void)
{
  if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
    tim2_10us_tick++;
  }
}

void EXTI9_5_IRQHandler(void)
{		
  /*Ultrasonic 1*/	
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
    EXTI_ClearITPendingBit(EXTI_Line5);	 
		if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)))  //read rising 
    {
      ultrasonic[0].flag = 1;
      ultrasonic[0].start_time = tim2_10us_tick; //记录高电平信号开始的时间
    }
    else   //read falling
    {
      if(1 == ultrasonic[0].flag) 
      {
        ultrasonic[0].flag = 0;
        ultrasonic[0].end_time = tim2_10us_tick; //记录高电平信号结束的时间
        if(ultrasonic[0].end_time - ultrasonic[0].start_time > UL_TIM_MAX_COUNT)  //cnt overflow
        {
          ultrasonic[0].distance = MAX_MEASURE_DISTANCE;
        }
        else
        {
          ultrasonic[0].distance = (ultrasonic[0].end_time - ultrasonic[0].start_time) * 340 / 2000.0;
        }
      }
    }
	}
  
  /* Ultrosonic 2 */
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
    EXTI_ClearITPendingBit(EXTI_Line7); 
		if((GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)))  //read rising 
    {
      ultrasonic[1].flag = 1;
      ultrasonic[1].start_time = tim2_10us_tick; //记录高电平信号开始的时间
    }
    else   //read falling
    {
      if(1 == ultrasonic[1].flag) 
      {
        ultrasonic[1].flag = 0;
        ultrasonic[1].end_time = tim2_10us_tick; //记录高电平信号结束的时间
        if(ultrasonic[1].end_time - ultrasonic[1].start_time > UL_TIM_MAX_COUNT)  //cnt overflow
        {
          ultrasonic[1].distance = MAX_MEASURE_DISTANCE;
        }
        else
        {
          ultrasonic[1].distance = (ultrasonic[1].end_time - ultrasonic[1].start_time) * 340 / 2000.0;
        }
      }
    }
  } 
}

