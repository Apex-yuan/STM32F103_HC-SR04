/**
  ******************************************************************************
  * @file    usart.h
  * @author  xiaoyuan
  * @version V2.0
  * @date    2018-1-28
  * @brief   �ṩ��USARTģ�麯��������
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

#ifndef __USART1_H
#define __USART1_H

#include "stm32f10x.h"
/* Ҫ�õ�printf���������Ҫ����C���Եı�׼��������ļ� */
#include "stdio.h"  //�ڴ˴�����stdio.h�ļ���ˣ����ⲿ�ļ���ֻ�����usart.h�����ֱ��ʹ��printf��scanf�Ⱥ���

#define USART1_CLK        RCC_APB2Periph_USART1
#define USART1_GPIO_CLK   RCC_APB2Periph_GPIOA

#define USART1_TX_PORT    GPIOA
#define USART1_TX_PIN     GPIO_Pin_9
#define USART1_RX_PORT    GPIOA
#define USART1_RX_PIN     GPIO_Pin_10


#define USE_MICROLIB      1  //(1)ʹ��΢��/��0����ʹ��΢�� ��ѡ��ʹ��΢����Ҫ��Option->Targetѡ�����й�ѡ��USE MicroLIB
#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����

#define USART1_RX_MAX_LEN  1024
extern uint8_t usart1_rx_buffer[USART1_RX_MAX_LEN];
extern uint16_t usart1_rx_cnt;

void usart1_init(uint32_t baud);


#endif //__USART_H
