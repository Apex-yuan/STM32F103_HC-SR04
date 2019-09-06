#include "bsp_hc_sr04.h"
#include "bsp_systick.h"

/*
  ˵����
  1. ��ǰ����������������ѯ������ʽ��
  2. ��ʾ����������֪���Ӳ�����ʼ�źŷ�����������ģ����������źŵ�ʱ����ԼΪ470us�������ѯ����������������֮��Ĳ���ʱ
     ����Ӧ�á�470us��Ŀǰ�������趨Ϊ500us����ʱ�����ȷ����һ���źŻ�û����ʱ����һ���������Ѿ������˲����жϣ����ж�
     ������ɣ��ص����λ�ü���ִ�У�Ȼ�󷢳���һ�������źţ�����ѯ���ƾ����˵�Ƭ������ͬʱ����ͬ�Ļ����źţ�����Ƭ���ܿ�
     �ķ�������������ʼ�źţ����յ������źŵĻ��Ƚ����жϣ��ж��л���еȴ������˴���Ҫ�ϳ���ʱ�䣬��ǰ����Ϊ��0-15ms����
     �ڵ�ǰ�жϴ����������һ���ж�Ҳ�ᴥ�������ж�ֻ�ܵȴ����ϸ��ж�ִ�����ʱ�Ż������жϺ�������˲�����������¼��ǰ
     �����źŸߵ�ƽ��ʱ����������δ������жϺ���ʱ�ߵ�ƽ�ź��Ѿ���ʧ�ˣ���˲�ĵľ��뷵��ֵΪ0��С��ʵ������ֵ������ȡ
     ������һ��ģ��Ĳ���ʱ������
  3. ��ʾ����������֪HC-SR04��ʱ��Ļ����źŻ����140ms����Ӧ�ľ���ֵΪ��2400cm�����ң���ѯ����������ʹ��whileѭ�����ȴ���
     ��˱������������ƣ���ǰ�����Ĵ�����Ժܺõı�������⣨����������������ʱ���������whileѭ����
 */

float distance_ultrasonic[UL_NUM];

//����ֻ�õ���TIM2�Ķ�ʱ�����ܣ�����û�б�Ҫ������ʱ�������ж�
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
  TIM_Cmd(UL_TIM, DISABLE); //Ĭ�Ϲر�TIM2
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
  ultrasonic_timConfig(UL_TIM_ARR, UL_TIM_PSC);   //��ʼ��TIM2��ʱ��������һ��Ϊ1/100000S��10us����1/(72M/(719+1))s
}
	
//���η��ʹ����źţ���ѯ��������ֵ�����ݲ������ƣ��ú���ִ����ϣ��������������Ѿ�ִ����ɡ�
void ultrasonic_startMeasure(void)
{
	GPIO_SetBits(UL_PORT, UL1_TRIG_PIN);
	delay_us(10);
	GPIO_ResetBits(UL_PORT, UL1_TRIG_PIN);
  delay_us(MEASURE_INTERVAL); //��ʱ��֤UL1�Ѿ������ж�

  GPIO_SetBits(UL_PORT, UL2_TRIG_PIN);
  delay_us(10);
  GPIO_ResetBits(UL_PORT, UL2_TRIG_PIN);
  delay_us(MEASURE_INTERVAL); //��ʱ��֤UL2�Ѿ������ж�
}

// void TIM2_IRQHandler(void)
// {
//   if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
//   {
//     TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
//     /* Ŀǰ�жϺ����в���Ҫ���κδ��� */
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
		while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5))  //�����ش����жϺ�ȴ���Ϊ�͵�ƽ
    {
      if(TIM_GetCounter(TIM2) >= UL_TIM_MAX_COUNT) // cnt = 255cm * 2 /(340 * 100) * 100000
      {
        break;  //�����趨�Ĳ��������룬�����ȴ�
      }
    }
		TIM_Cmd(TIM2,DISABLE);
    /* ���㳬�����������ľ��룬�糬�������밴��������ʾ */
		distance_ultrasonic[0] = TIM_GetCounter(TIM2) * 340 / 2000.0;  //cnt * 1/100000 * 340 / 2 *100(��λ��cm)
	}
  
  /* Ultrosonic 2 */
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
    EXTI_ClearITPendingBit(EXTI_Line7); 
		TIM_SetCounter(TIM2, 0);
		TIM_Cmd(TIM2, ENABLE);
		while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7))  //�����ش����жϺ�ȴ���Ϊ�͵�ƽ
    {
      if(TIM_GetCounter(TIM2) >= UL_TIM_MAX_COUNT) // cnt = 255cm * 2 /(340 * 100) * 100000
      {
        break;  //�����趨�Ĳ��������룬�����ȴ�
      }
    }
		TIM_Cmd(TIM2,DISABLE);
    /* ���㳬�����������ľ��룬�糬�������밴��������ʾ */
		distance_ultrasonic[1] = TIM_GetCounter(TIM2) * 340 / 2000.0;  //cnt * 1/100000 * 340 / 2 *100(��λ��cm)
  } 
}

