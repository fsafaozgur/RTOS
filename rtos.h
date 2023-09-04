#ifndef __RTOS_H_
#define __RTOS_H_

#include <stdint.h>


typedef struct 
{
	uint32_t *sp;		/* Stack Pointer */
	uint8_t prio;		/* Thread priority level */ /* Simdilik kullanilmayacak, ileride Priority-Based RTOS tasarlarken kullanilacak */
		
}RTOS_Thread;


typedef void (*RTOS_ThreadHandler)();				/* Bir altta yer alan fonksiyonda, argüman olarak fonsiyon adresi(threadHandler) yollayacagimiz için bunu tanimladik */

void RTOS_Thread_Create(RTOS_Thread *thread, RTOS_ThreadHandler threadHandler, uint32_t *stack, uint32_t stackSize);

void RTOS_ThreadSwitch();

void RTOS_init();

#endif  /* __RTOS_H_ */