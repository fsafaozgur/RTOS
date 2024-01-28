#ifndef __RTOS_H_
#define __RTOS_H_

#include <stdint.h>
#include "bsp.h"


typedef struct 
{
	uint32_t *sp;		/* Stack Pointer */
	uint8_t prio;		/* Thread priority level */ /* Simdilik kullanilmayacak, ileride Priority-Based RTOS tasarlarken kullanilacak */
	uint32_t delay; /* Delay verildigi taktirde bu degiskene atanacak */
}RTOS_Thread;


typedef void (*RTOS_ThreadHandler)(void);				/* Bir altta yer alan fonksiyonda, argüman olarak fonsiyon adresi(threadHandler) yollayacagimiz için bunu tanimladik */

void RTOS_Thread_Create(RTOS_Thread *thread, RTOS_ThreadHandler threadHandler, uint32_t *stack, uint32_t stackSize);

void RTOS_ThreadSwitch(void);

void RTOS_init(void);

void RTOS_decrement(void);

void RTOS_delay(uint32_t delay);

#endif  /* __RTOS_H_ */