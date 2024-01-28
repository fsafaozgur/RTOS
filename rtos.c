#include <stdint.h>
#include "rtos.h"

#define MaxThreadNumber 32




/* 
	PendSV i�erisinde yapacagimiz context-switching islemi ile Thread ler arasinda ge�is yapilirken RTOS_ThreadNext ve RTOS_ThreadCurr i�erisindeki pointerlar
	s�rekli olarak degisecegi i�in (pointer larin isaret ettigi bellek degerleri degil, pointer larin kendi degerleri degisecegi i�in) burada pointer lar volatile yapilmistir.
*/
RTOS_Thread * volatile RTOS_ThreadNext;			/* Thread leri sirayla �alistiracaz ve context-switching yapilarak �alistirilacak bir sonraki Thread bu olacak */
RTOS_Thread * volatile RTOS_ThreadCurr;			/* Thread leri sirayla �alistiracaz ve halihazirda �alisiyor olan Thread bu olacak */




/* 
	Preemptive, Priority-Based RTOS tasariminda kullanilacak Thread dizisini olusturalim, ancak suan daha basit olan sirali �alisan RTOS tasarlanacaktir. 
	Yani simdilik bu diziyi priority level i�in degil, sirali �alismayi saglamak i�in kullanacagiz
*/
RTOS_Thread *RTOS_threads[32+1];					/* 32 adet thread yerlestirebilecegimiz bir sistem olusturuyoruz, Thread lerin �ncelik siralamasi dizideki yerleri ile esit olacak
																					Fazladan 1 eklememizin sebebi ise Idle Thread 'dir, Idle Thread 'in priority leveli olmadigi i�in o ayri birakilmistir
																					ve hi�bir Thread devrede degilken o �alisir. Idle Thread i�ine pil tasarrufu yapacak kodlar koyarsak, hi�bir Thread �alismazken sistem g�� tasarrufu yapar. */

uint8_t RTOS_threadNumber = 0;						/*Sistemimizde ka� adet Thread oldugunu bir degiskene atiyoruz */
uint8_t RTOS_currentThreadNumber = 0;			/*Halihazirda �alisan Thread in index numarasini kaydediyoruz (sirali �alistirmak i�in gerekli) */


void RTOS_ThreadSwitch()
{

	++RTOS_currentThreadNumber;																/* Fonsiyon her tetiklendiginde bir sonraki Thread a ge�ilecek*/														
	
	if (RTOS_currentThreadNumber == RTOS_threadNumber)				/* Thread lerin hepsi sirayla �alistiktan sonra basa d�nd�r�yoruz */
	{
		RTOS_currentThreadNumber = 0;
	}
	
	
	/* PendSV ile RTOS_ThreadNext adresinin g�sterdigi Thread i devreye aldigimiz i�in, bu fonksiyonun temel g�revi
			RTOS_ThreadNext in hangi Thread 'i g�sterecegini belirlemektir. 
	*/
	RTOS_ThreadNext = RTOS_threads[RTOS_currentThreadNumber];
	
	
	
	/* Context-Switching islemini yapacak olan PendSV 'yi �alistiriyoruz */ 
	if (RTOS_ThreadNext != RTOS_ThreadCurr)
	{
			(*(volatile uint32_t *)0xE000ED04) |= (1<<28);    /* Devreye girmesi gereken bir Thread var ise (RTOS_ThreadNext) o halde PendSV 'yi �alistir */
	}	
}


void RTOS_init()
{
	(*(volatile uint32_t *)0xE000ED20) |= (1u<<20);								/* PendSV'nin priority leveli SysTick'e g�re d�s�kte tutuldu ki Tail-Chaining saglanabilsin */
																																/* B�ylece, SysTick_Handler i�erisindeki kodlar bitince, geri d�n�s olmadan PendSV_Handler'a ge�is saglanmis oldu*/
																																/* T�m islemcilerde PendSV'nin register adresi ayni oldugu i�in bunun ayarlanmasini rtos.c ye aldik /*
																																/* SysTick Timer i�in priority level ayarlamasini, islemciden islemciye g�re degistigi i�in buraya almadik */
}




void RTOS_Thread_Create(RTOS_Thread *thread, RTOS_ThreadHandler threadHandler, uint32_t *stack, uint32_t stackSize)
{

	/* Thread i�in, DataSheet de yer alan Exception Stack Frame sirasina uygun bir Stack Bellek olusturuyoruz,  */
	
	
	/* Ilk olarak Stack Bellegi olusturalim */
	uint32_t *sp = (uint32_t *)((((uint32_t)stack + stackSize)/8)*8);		/* ARM mimarisinde bellek atamalari eksi y�nde oldugu i�in, olusturdugumuz Stack Bellegin yigin yapmaya baslayacagi 
																				                              ilk nokta en y�ksek adrestir, buradan d�s�k adrese dogru yigin yapmaya baslar, bu sebeple bellegin baslangic noktasi, aslinda dizinin son elemanidir.
																																			8 byte baundary kuralina uydurabilmek i�in, adresi �nce 8'e b�l�p sonra 8 ile �arpiyoruz. (Biz 32 elemanli dizi se�erek aslinda 
																																			bu sarti saglamistik ama bu kodu baska biri kullanir ve bu kurala uygun bellek se�mezse diye bunu garantiliyoruz
		
	
	/* sp adresli stack bellekte yer alan ilk degeri bos birakiyoruzki buradan align islemi yapabilelim diye, o y�zden ilk deger atamasi, --sp adresinden baslayacak */
	
	*(--sp) = (1<<24);   			  						/* EPSR */  /* EPSR register da 24. bit olarak yer alan  THUMB komut seti degeri daima 1 dir ve bunu Exception Stack Framedeki sirasina yaziyoruz*/ 
	*(--sp) = (uint32_t)threadHandler;			/* PC */   /* Exception Stack Frame sirasina g�re burasi PC register oluyor. Thread devreye alindiginda main_Thread() fonksiyonunun �alismasini istedigimiz i�in PC ye bu fonksiyonun adresini atadik */
	*(--sp) = 0x00000001u;									/* LR */	 /* Bu ve bundan sonrakilere atanacak degerler �nemsiz, yalnizca Exception Stack Frame sirasina g�re registerleri belirtiyoruz */
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
	
	/* Stack Bellegi olusturduktan sonra bunu ilgili Thread'in elemani olan sp pointera(Thread'a �zel olan Stack Bellegin adresini tutan degisken) atiyoruz */ 
	thread->sp = sp;

	
	/* assert(RTOS_threadNumber < MaxThreadNumber);			 Eger maksimum thread den daha �ok eklenmeye �alisilirsa, programdan �ikar */ 
																											/* Sistemi direk kapatmak yanlis bir uygulamadir, �rnek olarak buraya eklenmistir*/
	
	RTOS_threads[RTOS_threadNumber] = thread;						/* Yeni olusturulan Thread, t�m Thread 'lerin yer aldigi diziye ataniyor */
	++RTOS_threadNumber;																/* Bir sonraki olusturulacak Thread i�in numara bir arttiriliyor */


}
