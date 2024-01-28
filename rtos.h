#ifndef __RTOS_H_
#define __RTOS_H_

#include <stdint.h>
#include "bsp.h"

#define MaxThreadNumber 32

typedef struct 
{
	uint32_t *sp;		/* Stack Pointer */
	uint8_t prio;		/* Thread priority level */
	uint32_t delay; /* Delay verildigi taktirde bu degiskene atanacak */
}RTOS_Thread;


typedef void (*RTOS_ThreadHandler)(void);		 /* Bir altta yer alan fonksiyonda, argüman olarak fonsiyon adresi(threadHandler) yollayacagimiz için bunu tanimladik */

void RTOS_Thread_Create(RTOS_Thread *thread, RTOS_ThreadHandler threadHandler, uint32_t *stack, uint32_t stackSize);  /* "Thread initialize" isleminin yapan fonksiyon */

void RTOS_ThreadSwitch(void);		/* Schedule islemini yaparak bir sonraki çalistirilacak Thread 'i belirleyen fonksiyon */

void RTOS_init(void);			/* PendSV, interrupt priority ayarlarinin yapildigi fonksiyon  */

void RTOS_decrement(void);		/* Thread 'lere atanan delay degerlerini sayarak, sifira ulasinca Bitset 'leri ayarlayan fonksiyon */

void RTOS_delay(uint32_t delay);		/* Thread 'lere delay atayan ve aninda devreden çikaran fonksiyon */

#endif  /* __RTOS_H_ */