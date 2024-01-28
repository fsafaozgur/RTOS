#ifndef __BSP_H_
#define __BSP_H_

#include "stm32f4xx.h"                  /* Device header */


/* Microislemciye �zel t�m fonksiyonlar BSP (Board Support Package) i�erisinde tanimlaniyor, 
	b�ylelikle bu RTOS, baska bir mikroislemciye tasindiginda yalnizca bu fonksiyonlara ait kod bloklari (bsp.c)
	degistirilerek yeni sisteme entegre edilebilir hale getiriliyor.
*/
void SysTick_Handler(void);			
void PendSV_Handler(void);
void Configuration(void);


#endif  /* __BSP_H_ */