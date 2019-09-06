#include "bsp_hc_sr04.h"
#include "bsp_systick.h"

/*
  ˵����
  1. ��ǰ��������������ͬ��������ʽ��ע���ⲿ�жϴ�������ѡ���������½��ض����Դ�����
  2. ���趨����������Ϊ255cm����Ӧ��ʱ��Ϊ15ms�������ж��ٸ�������ģ�飬�ò�����ʽ���������Ĳ�������Ϊ15ms��
     �����ݸ�ģ��ʹ���ֲ�Ĳ������ڽ���Ϊ60ms���ϣ���˾���ʹģ��Ĳ������ھ������ڸ÷�Χ��
  3. �ò�����ʽ��һ�����Ե�ȱ���ǣ���ͬģ��֮��Ļز����ܻ����������ŵ�������������������ģ��֮�価���ܴ�Ƕ�
     ����ͬ�ķ���
 */
static __IO uint32_t tim2_10us_tick = 0;  //ϵͳ����������¼ϵͳ����ʱ��10usΪ��λ
UltrasonicDef ultrasonic[UL_NUM] = {0};

//����ֻ�õ���TIM2�Ķ�ʱ�����ܣ�����û�б�Ҫ������ʱ�������ж�
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
  TIM_Cmd(UL_TIM, ENABLE); //����TIM2
}

static void ultrasonic_gpioConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  UL_GPIO_CLK_FUNCTION(UL_GPIO_CLK, ENABLE);
  
  GPIO_InitStruct.GPIO_Pin = UL1_TRIG_PIN | UL2_TRIG_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;         //�������
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(UL_PORT, &GPIO_InitStruct);
  GPIO_ResetBits(UL_PORT, UL1_TRIG_PIN | UL2_TRIG_PIN); //��λ��������
  
  GPIO_InitStruct.GPIO_Pin = UL1_ECHO_PIN | UL2_ECHO_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init(UL_PORT, &GPIO_InitStruct);
}

static void ultrasonic_extiConfig(void)
{
  EXTI_InitTypeDef EXTI_InitStruct;
  NVIC_InitTypeDef NVIC_InitStruct;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  //EXTI��Ҫʹ��AFIOʱ��
  
  GPIO_EXTILineConfig(UL_GPIO_PORT_SOURCE, UL1_GPIO_PIN_SOURCE | UL2_GPIO_PIN_SOURCE); //���ж������ӵ�GPIO�˿�
  
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
  ultrasonic_timConfig(UL_TIM_ARR, UL_TIM_PSC);   //��ʼ��TIM2��ʱ��
}
	
//ͬ�������������η��ʹ����źţ����м䲻�ü���ʱ����
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
      ultrasonic[0].start_time = tim2_10us_tick; //��¼�ߵ�ƽ�źſ�ʼ��ʱ��
    }
    else   //read falling
    {
      if(1 == ultrasonic[0].flag) 
      {
        ultrasonic[0].flag = 0;
        ultrasonic[0].end_time = tim2_10us_tick; //��¼�ߵ�ƽ�źŽ�����ʱ��
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
      ultrasonic[1].start_time = tim2_10us_tick; //��¼�ߵ�ƽ�źſ�ʼ��ʱ��
    }
    else   //read falling
    {
      if(1 == ultrasonic[1].flag) 
      {
        ultrasonic[1].flag = 0;
        ultrasonic[1].end_time = tim2_10us_tick; //��¼�ߵ�ƽ�źŽ�����ʱ��
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

