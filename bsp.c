#include <stdint.h>
#include "bsp.h"
#include "rtos.h"


extern int sysTick_count;


void Configuration(void)
{
	SystemCoreClockUpdate();																			
	SysTick->VAL = 0x0u;																					/* DataSheet'de yer alan "A write of any value clears the field to 0" ifadesine binaen "0" atayarak temizledik*/  
	SysTick->LOAD = (uint32_t) (SystemCoreClock / 8000);					/* 1 ms s�rede reload yapcak sekilde ayarlandi*/
	SysTick->CTRL |= 0x07u;																				/* Interrupt aktif */ /* Clock Source AHB/8 ayarlandi */ /* Counter aktif */ 
	
	
	(*(volatile uint32_t *)0xE000ED20) |= (0u<<28);							/* SyscTick'in priority leveli PendSV'e g�re y�ksekte tutuldu ki Tail-Chaining saglanabilsin */
																															/* PendSV'nin priority leveli SysTick'e g�re d�s�kte tutuldu ki Tail-Chaining saglanabilsin (rtos.c dosyasinda) */	
	/* 
	Tail-Chaining yapisindan kasit, context-switching islemine gerek kalmadan, interruptlari arka arkaya ekleyerek daha verimli bir �alisma saglamaktir.
	S�yle ki, her intterupt �alistiginda, main() de kalinan nokta (islemcinin kendi register larindaki degerler) Stack'e yazilarak ilgili Interrupt_Handler �alistirilir,
	Interrupt_Handler �alistirildiktan sonra main() e geri d�n�s yapilirken, interrupt �ncesindeki duruma d�nmek i�in Stack'e yazilan  o anki context geri y�klenir 
	bu duruma context-swithhing denir ve haliyle maliyetli bir islemdir. Ancak biz Systick_Handler �alistirilip main() e geri d�nmeden yani contect switching islemi yapilmadan
	direk PendsV_Handler i �alistirirsak, iki Interrupt_Handler i arka arkaya (Tail-Chaining) eklemis ve context-swithing islemini yapmamis oluruz.
*/

}



void SysTick_Handler(void)
{
	
	++sysTick_count;
	
	if (sysTick_count > 1000) {
	
	__disable_irq();																		/* Olusabilecek Race-Conditions durumunun �n�ne ge�mek i�in interruptlari kapattik */
	
	RTOS_ThreadSwitch();																/* Eger devreye girecek  Thread varsa burada belirleyip, PendSV yi devreye alacak biti set ediyoruz,
																											ancak SysTick_Handler �alismasini bitirdikten sonra, PendSV devreye girebilecek ve context-switching islemini yapabilecektir*/
	
	__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */

	}
}


void PendSV_Handler(void)
{

	/* __disable_irq() */
	__asm("CPSID					I"); 
	
	/* if (RTOS_ThreadCurr != (RTOS_Thread *)0) { */			/*Bu kontrol� yapmamizin sebebi; eger halihazirda �alisan yani baslangi� durumunda hi� Thread yoksa direk RTOS_ThreadNext i�erisindeki ilk ve tek Thread i kullan, Thread varsa i�erisindeki sp ye (RTOS_ThreadCurr->sp), islemci sp sini ata ve b�ylece kaldigin noktayi kaydetmis ol demek i�indir */ 
	
	__asm("LDR 					  r1,=RTOS_ThreadCurr");
	__asm("LDR						r1,[r1,#0x00]");
	__asm("CBZ						r1,Label_first");
		
	/* Push r4-r11 komutu ile islemcinin bu register larini Stack Bellege yolluruz, bu Thread e ileride geri d�nerken r4-r11 registerleri geri y�kleyecez
	Bunu yapmamizin nedeni; Thread ler islem yaparken R4-R11 araligindaki islemci register larina veri yazmis olabilir ve context-switching ile yeniden devreye
	alindiklarinda bu register lari kullanacaklari i�in bunlari biz manuel olarak Stack Bellege yazariz, ��nk� interrupt devreye girerken bu register lari
	otomatik olarak Stack Bellege yollamaz */
	__asm("PUSH					{r4-r11}");
	
	/* RTOS_ThreadCurr->sp = sp */   									   /*	Islemcinin Stack Pointer ini yani Stack Bellegin top noktasinin adresini, �alismakta olan Thread in sp pointer inin i�ine kaydediyoruz */
	__asm("LDR 					r1,=RTOS_ThreadCurr");										 /* Ileride bu Thread i tekrar devreye aldigimizda, bu sp degerini islemciye geri y�kleyerek kaldigimiz yerden devam etmis olacagiz */
	__asm("LDR						r1,[r1,#0x00]");
	__asm("STR						sp,[r1,#0x00]");
	
	__asm("Label_first:");
	/* sp = RTOS_ThreadNext->sp */											 /* Simdi devreye alacagimiz Thread in i�erisindeki, o Thread a �zel olusturdugumuz Stack Bellegin top adresini, islemcinin sp registerine y�kl�yoruz */
	__asm("LDR 					r1,=RTOS_ThreadNext");									  
	__asm("LDR						r1,[r1,#0x00]");
	__asm("LDR						sp,[r1,#0x00]");
	
	/* RTOS_ThreadCurr = RTOS_ThreadNext */					 		/* Devreye alacagimiz sonraki Thread olan RTOS_ThreadNext i, artik RTOS_ThreadCurr 'e atiyoruz, b�ylece yeni Thread i devreye almis oluyoruz */
	__asm("LDR 					r1,=RTOS_ThreadNext");									  
	__asm("LDR						r1,[r1,#0x00]");
	__asm("LDR						r2,=RTOS_ThreadCurr");
	__asm("STR						r1,[r2,#0x00]");

	/* Pop r4-r11 komutu ile, suanda devreye alinacak Thread a �zel olusturulmus Stack Bellekten, R4-R11 registerleri geri y�kleyecegiz (islemcinin r4-r11 register larina y�klenecek )
		 Bunu, context-swithing isleminin ger�eklesecegi, interrupt dan �ikis komutu olan BX komutundan hemen �nce yapiyoruz, ��nk� islemci bunu manuel yapmiyor */
	__asm("POP					 {r4-r11}");

	/* __enable_irq() */																/* Interuptlari yeniden aktif ediyoruz */
	__asm("CPSIE					I");
	
	__asm("BX						lr");																		/*Interrupt dan �ik */	
	
	/* 
		Bu komutun ardindan islemci kendi sp sinde yer alan (biz yukarida "sp = RTOS_ThreadNext->sp" islemini yaptirarak islemci sp sini manip�le ettik) adresin g�sterdigi
		Stack Bellek 'ten degerleri okuyarak bunlari, registerlarina (PC, LR, R0, ...) yaziyor, b�ylece preempt ettigi Thread 'e (RTOS_ThreadCurr)d�necegini zannederken bizim manip�lasyonumuz sayesinde
		bizim istedigimiz Thread 'e (RTOS_ThreadNext) d�nm�s oluyor. 
	
		Threadlar arasi ge�is yaparken, devreye alinan her Thread, kendi main_Thread() fonksiyonunun basindan degil, bir �nce context-switching yapilarak devreden �ikarilirken
		kalmis oldugu noktadan devam edecektir. Bunu PC register sayesinde yapabiliyoruz. Stack Bellegi kaydederken PC yi de kaydettigimiz i�in, program akisinda tam olarak
		hangi komutta kaldiysak bunu da PC yi kaydederken kaydetmis oluyoruz, context-switching ile tekrar devreye alinan Thread 'in Stack Belleginindeki kayitlari islemci kendi
		register larina atarken bu PC yi de y�kledigi i�in haliyle son kalinan noktaya direk giderek oradan itibaren kalan kodlar islenmeye devam edecektir.
	*/ 
	
}


