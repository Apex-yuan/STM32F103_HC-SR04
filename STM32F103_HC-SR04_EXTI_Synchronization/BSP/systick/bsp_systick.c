/**
  ******************************************************************************
  * @file    bsp_systick.c 
  * @author  Apex yuan
  * @version V1.0.0
  * @date    2019-8-16
  * @brief   Main program body
  ******************************************************************************
  * @attention 
  * 1.��ϵͳ�δ�ʱ��ͬʱʵ��ϵͳʱ���¼��us ms��ʱ��
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "bsp_systick.h"

volatile uint32_t _us_tick = 0;
volatile uint32_t _ms_tick = 0;

/**
  * @brief  initialize systick
  * @param  None
  * @retval None
  */
void systick_init(void) 
{
  _us_tick = 0;
  _ms_tick = 0;  
  
  /* SystemCoreClock / 1000000  1us�ж�һ�� */
  /* SystemCoreClock / 1000     1ms�ж�һ�� */
	if(SysTick_Config(SystemCoreClock / 1000000))
  {
    /*capture error*/
    while(1);
  }
  
  /* Configure the SysTick handler priority */
  //NVIC_SetPriority(SysTick_IRQn, 0x0);
}

/**
  * @brief  delay (ms)
  * @param  delay time
  * @retval None
  */
void delay_ms(uint32_t millis) 
{ 
	uint32_t target;
	
	target = _ms_tick + millis;
	while(_ms_tick < target);
} 

/**
  * @brief  delay (us)
  * @param  delay time
  * @retval None
  */
void delay_us(uint32_t uillis)
{ 
	uint32_t target;
	target = _us_tick + uillis;
	while(_us_tick < target);
}

/**
  * @brief  ��ȡϵͳ��ǰ��ms����ֵ
  * @param  None
  * @retval ϵͳ��ǰʱ��ms
  */
uint32_t millis(void) 
{
	return _ms_tick;
}

/**
  * @brief  ��ȡϵͳ��ǰ��us����ֵ
  * @param  delay time
  * @retval ϵͳ��ǰʱ��us
  */
uint32_t micros(void)
{
  return _us_tick;
}

/**
  * @brief  ��λϵͳ����
  * @param  None
  * @retval None
  */
void systick_reset(void) 
{
	_us_tick = 0;
  _ms_tick = 0;
}

/**
  * @brief  SYSTICK�жϷ�����
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	_us_tick++;
  _ms_tick = _us_tick / 1000;
  //_ms_tick++;

}

