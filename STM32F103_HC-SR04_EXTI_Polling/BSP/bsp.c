#include "bsp.h"

void bsp_init(void)
{ 
  /* �����ж����ȼ����� */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  systick_init();
  usart1_init(115200);
  ultrasonic_init();  //��ʼ��������ģ��
}


