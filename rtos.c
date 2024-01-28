#include <stdint.h>
#include "rtos.h"

#define MaxThreadNumber 32



/* 
	PendSV içerisinde yapacagimiz context-switching islemi ile Thread ler arasinda geçis yapilirken RTOS_ThreadNext ve RTOS_ThreadCurr içerisindeki pointerlar
	sürekli olarak degisecegi için (pointer larin isaret ettigi bellek degerleri degil, pointer larin kendi degerleri degisecegi için) burada pointer lar volatile yapilmistir.
*/
RTOS_Thread * volatile RTOS_ThreadNext;			/* Thread leri sirayla çalistiracaz ve context-switching yapilarak çalistirilacak bir sonraki Thread bu olacak */
RTOS_Thread * volatile RTOS_ThreadCurr;			/* Thread leri sirayla çalistiracaz ve halihazirda çalisiyor olan Thread bu olacak */




/* 
	Preemptive, Priority-Based RTOS tasariminda kullanilacak Thread dizisini olusturalim, ancak suan daha basit olan sirali çalisan RTOS tasarlanacaktir. 
	Yani simdilik bu diziyi priority level için degil, sirali çalismayi saglamak için kullanacagiz
*/
RTOS_Thread *RTOS_threads[32+1];					/* 32 adet thread yerlestirebilecegimiz bir sistem olusturuyoruz, Thread lerin öncelik siralamasi dizideki yerleri ile esit olacak
																					Fazladan 1 eklememizin sebebi ise Idle Thread 'dir, Idle Thread 'in priority leveli olmadigi için o ayri birakilmistir
																					ve hiçbir Thread devrede degilken o çalisir. Idle Thread içine pil tasarrufu yapacak kodlar koyarsak, hiçbir Thread çalismazken sistem güç tasarrufu yapar. */

uint8_t RTOS_threadNumber = 0u;						/*Sistemimizde kaç adet Thread oldugunu bir degiskene atiyoruz */
uint8_t RTOS_currentThreadNumber = 0u;		/*Halihazirda çalisan Thread in index numarasini kaydediyoruz (sirali çalistirmak için gerekli) */

uint32_t RTOS_ReadyBitSet = 0u;						/* Çalismaya hazir Thread ler */
uint32_t RTOS_DelayBitSet = 0u;						/* Delay verilerek Blocking islemi yapilacak Thread ler */





/* PendSV ile RTOS_ThreadNext adresinin gösterdigi Thread i devreye aldigimiz için, bu fonksiyonun temel görevi
			RTOS_ThreadNext in hangi Thread 'i gösterecegini belirlemektir. 
*/
void RTOS_ThreadSwitch()
{

	if (RTOS_ReadyBitSet == 0u){
		 RTOS_ThreadNext = RTOS_threads[0];											/* Eger hiçbir Thread aktif degilse IdleThread devreye girecek */
	}
	else {
		uint8_t clz = 32 - ( __CLZ(RTOS_ReadyBitSet) );							/* RTOS_ReadyBitSet 'in logic-1 olan MSB bitini buluyoruz */
		RTOS_ThreadNext = RTOS_threads[clz];										/* Böylelikle Priority Level en yüksek olan Thread i devreye almak üzere atiyoruz */
	}

	
	/* Context-Switching islemi yaparak RTOS_ThreadNext i devreye alacak olan PendSV 'yi çalistiriyoruz */ 
	if (RTOS_ThreadNext != RTOS_ThreadCurr)
	{
			(*(volatile uint32_t *)0xE000ED04) |= (1<<28);    /* Devreye girmesi gereken bir Thread var ise (RTOS_ThreadNext) o halde PendSV 'yi çalistir */
	}	
}


void RTOS_init()
{
	(*(volatile uint32_t *)0xE000ED20) |= (1u<<20);								/* PendSV'nin priority leveli SysTick'e göre düsükte tutuldu ki Tail-Chaining saglanabilsin */
																																/* Böylece, SysTick_Handler içerisindeki kodlar bitince, geri dönüs olmadan PendSV_Handler'a geçis saglanmis oldu*/
																																/* Tüm islemcilerde PendSV'nin register adresi ayni oldugu için bunun ayarlanmasini rtos.c ye aldik /*
																																/* SysTick Timer için priority level ayarlamasini, islemciden islemciye göre degistigi için buraya almadik */
}




void RTOS_Thread_Create(RTOS_Thread *thread, RTOS_ThreadHandler threadHandler, uint32_t *stack, uint32_t stackSize)
{

	/* Thread için, DataSheet de yer alan Exception Stack Frame sirasina uygun bir Stack Bellek olusturuyoruz,  */
	
	
	/* Ilk olarak Stack Bellegi olusturalim */
	uint32_t *sp = (uint32_t *)((((uint32_t)stack + stackSize)/8)*8);		/* ARM mimarisinde bellek atamalari eksi yönde oldugu için, olusturdugumuz Stack Bellegin yigin yapmaya baslayacagi 
																				                              ilk nokta en yüksek adrestir, buradan düsük adrese dogru yigin yapmaya baslar, bu sebeple bellegin baslangic noktasi, aslinda dizinin son elemanidir.
																																			8 byte baundary kuralina uydurabilmek için, adresi önce 8'e bölüp sonra 8 ile çarpiyoruz. (Biz 32 elemanli dizi seçerek aslinda 
																																			bu sarti saglamistik ama bu kodu baska biri kullanir ve bu kurala uygun bellek seçmezse diye bunu garantiliyoruz
		
	
	/* sp adresli stack bellekte yer alan ilk degeri bos birakiyoruzki buradan align islemi yapabilelim diye, o yüzden ilk deger atamasi, --sp adresinden baslayacak */
	
	*(--sp) = (1<<24);   			  						/* EPSR */  /* EPSR register da 24. bit olarak yer alan  THUMB komut seti degeri daima 1 dir ve bunu Exception Stack Framedeki sirasina yaziyoruz*/ 
	*(--sp) = (uint32_t)threadHandler;			/* PC */   /* Exception Stack Frame sirasina göre burasi PC register oluyor. Thread devreye alindiginda main_Thread() fonksiyonunun çalismasini istedigimiz için PC ye bu fonksiyonun adresini atadik */
	*(--sp) = 0x00000001u;									/* LR */	 /* Bu ve bundan sonrakilere atanacak degerler önemsiz, yalnizca Exception Stack Frame sirasina göre registerleri belirtiyoruz */
	*(--sp) = 0x00000002u;									/* R12 */
	*(--sp) = 0x00000003u;									/* R3 */
	*(--sp) = 0x00000004u;									/* R2 */
	*(--sp) = 0x00000005u;									/* R1 */
	*(--sp) = 0x00000006u;									/* R0 */
	/* R4-R11 Registerlari Stack Bellege kaydedecez*/
	*(--sp) = 0x00000007u;									/* R11 */
	*(--sp) = 0x00000008u;									/* R10 */
	*(--sp) = 0x00000009u;									/* R9 */
	*(--sp) = 0x0000000Au;									/* R8 */
	*(--sp) = 0x0000000Bu;									/* R7 */	
	*(--sp) = 0x0000000Cu;									/* R6 */
	*(--sp) = 0x0000000Du;									/* R5 */
	*(--sp) = 0x0000000Eu;									/* R4 */
	
	/* Stack Bellegi olusturduktan sonra bunu ilgili Thread'in elemani olan sp pointera(Thread'a özel olan Stack Bellegin adresini tutan degisken) atiyoruz */ 
	thread->sp = sp;

	
	/* assert(RTOS_threadNumber < MaxThreadNumber);			 Eger maksimum thread den daha çok eklenmeye çalisilirsa, programdan çikar */ 
																											/* Sistemi direk kapatmak yanlis bir uygulamadir, örnek olarak buraya eklenmistir*/
	
	thread -> prio = RTOS_threadNumber;									/* Thread icin belirledigimiz Priority level atandi */
	RTOS_threads[RTOS_threadNumber] = thread;						/* Yeni olusturulan Thread, tüm Thread 'lerin yer aldigi diziye ataniyor */
	
	if (RTOS_threadNumber != 0){												  /* IdleThread ise onu hiçbir zaman RTOS_ReadyBitSet içine koymayacaz çünkü o hiçbir zaman Delay edilememeli */
			RTOS_ReadyBitSet |= (1 << --RTOS_threadNumber);		/* Yeni olusturulan Thread i aktif yani çalismaya hazir hale getirdik */ 
			RTOS_threadNumber += 2;														/* IdleThread degilse RTOS_ReadyBitSet in LSB sine konulacak, daha sonra 1 arttiracaktik 
																													 ancak halihazirda fazladan 1 decrement islemi yapildigi için 2 arttiriyoruz */
	}else {
		++RTOS_threadNumber;
	}
	
}




void RTOS_decrement(void)
{
		uint32_t tempDelay = RTOS_DelayBitSet;
		uint8_t iter = 1;
	
		while (tempDelay != 0u )
		{
			if (tempDelay & 1u)
			{
				 --RTOS_threads[iter]->delay;
				if (RTOS_threads[iter]->delay == 0)
				{
						RTOS_ReadyBitSet |= (1 << (RTOS_threads[iter]->prio - 1));
						RTOS_DelayBitSet &= ~(1 << (RTOS_threads[iter]->prio - 1));
				}
			}
			tempDelay = tempDelay >> 1;	
			++iter;
			//RTOS_Thread tempThread = RTOS_threads[];

		}


}


void RTOS_delay(uint32_t delay)
{
		RTOS_ThreadCurr->delay = delay;
		RTOS_ReadyBitSet &= ~(1 << (RTOS_ThreadCurr->prio - 1));
		RTOS_DelayBitSet |= (1 << (RTOS_ThreadCurr->prio - 1));
	
		__disable_irq();																		/* Olusabilecek Race-Conditions durumunun önüne geçmek için interruptlari kapattik */
	
		RTOS_ThreadSwitch();																/* Eger devreye girecek  Thread varsa burada belirleyip, PendSV yi devreye alacak biti set ediyoruz,
																												ancak SysTick_Handler çalismasini bitirdikten sonra, PendSV devreye girebilecek ve context-switching islemini yapabilecektir*/
	
		__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */
	
		

}
