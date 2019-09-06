#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
 extern "C" {
#endif
   
#include "stm32f10x.h"
   
#include "bsp_systick.h"
#include "bsp_usart.h"
#include "bsp_hc_sr04.h"

void bsp_init(void);

#ifdef __cplusplus
 }
#endif

#endif /*__BSP_H */

