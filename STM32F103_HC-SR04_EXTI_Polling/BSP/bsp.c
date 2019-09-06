#include "bsp.h"

void bsp_init(void)
{ 
  /* 配置中断优先级分组 */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  systick_init();
  usart1_init(115200);
  ultrasonic_init();  //初始化超声波模块
}


