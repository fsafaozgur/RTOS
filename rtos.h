#ifndef __RTOS_H_
#define __RTOS_H_

#include <stdint.h>


typedef struct 
{
	uint32_t *sp;		/* Stack Pointer */
	uint8_t prio;		/* Thread priority level */ /* Simdilik kullanilmayacak, ileride Priority-Based RTOS tasarlarken kullanilacak */
		
}RTOS_Thread;


typedef void (*RTOS_ThreadHandler)(void);				/* Bir altta yer alan fonksiyonda, arg�man olarak fonsiyon adresi(threadHandler) yollayacagimiz i�in bunu tanimladik */

void RTOS_Thread_Create(RTOS_Thread *thread, RTOS_ThreadHandler threadHandler, uint32_t *stack, uint32_t stackSize);

void RTOS_ThreadSwitch(void);

void RTOS_init(void);

#endif  /* __RTOS_H_ */