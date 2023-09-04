#include "stm32f4xx.h"                  // Device header
#include "rtos.h"

/*
	Multi Thread olarak �alisan, ancak bu Thread leri sira ile �alistiran bir RTOS tasarlanmistir
	Ileride Preemptive, Priority-Based  RTOS tasarimi yapilacaktir, suan priority level e g�re degil sirayla �alisan bir sistem yapilmistir.
	Preemptive, Priority-Based  RTOS yapi devreye alindigi zaman Idle_Thread de eklenecek b�ylece diger sistemler �alismazken devrede olacak Idle_Thread yardimiyla
	g�� tasarrufu modu �alistirilarak verimli bir sistem tasarlanacaktir.
*/


/* GENEL NOT:

	RTOS 'un temeli Context-Switching  mekanizmasidir. Context-Switching islemi, islemcinin halihazirda kullanmis oldugu Stack Bellegi degistirme islemidir ve bu Stack Pointer 
	degerini degistirmek ile olur. Stack Pointer i manupile edebilmenin tek yolu Assembly kodlari oldugu i�in bu islem i�in Assembly dili kullanilacaktir.

	Context-Switching islemi Systick_Handler i�erisinde degil, Systick_Handler tarafindan tetiklenen PendSV exceptionu ile PendSV_Handler i�erisinde yapilacaktir. 
	Bunun nedenleri; Systick_Handler pek �ok ama� i�in kullanilir, biz ise context-switching islemini bazen otomatik switch islemlerinde bazen de herhangi bir anda 
	bir Thread in s�resi dolmussa onu manuel olara devreden �ikarmak i�in context-switching islemi yapmamiz gerekebilir. Bu durumda Systick 'i manuel devreye alirsak beklenmeyen
	sonu�lar olabilir. �rnegin 1ms de bir devreye girerek bir�ok uygulamayla ilgili islemler yapan ya da delay saglayan SysTick_Handler i, s�resi dolan Thread i devreden �ikarmak i�in �agirirsak
	bu SysTick_Handlerin fazladan devreye girip i�erisinde yer alan ve diger Thread 'lere islem yapan kodlarin fazladan �alismasina ya da sayma i�in kullanilan bir degiskenin 
	degerinin fazladan degistirilmesine sebep olacagi asikardir, bu sebeple sadece context-switching i�in ayri bir Interrup_Handler (PendSV_Handler) kullanmali, 
	SysTick_Handleri ise Thread lerin blocking s�relerini saymada, s�resi biten varsa gerekli islemleri yapmada ve diger Thread ler i�in bagimsiz islemleri yapmada kullanabiliriz.
	Bizim uygulamamizda ise PendSV yi devreye almada ve ileride Blocking islemi yaparken, delay degerlerini saymada kullaniyor olacagiz.

	Ayrica m�mk�nse bir Interrup_Handler i�erisinde sadece 1 dil kullanilmalidir(pure language) SysTick_Handler i�erisinde C dili kullaniliyorsa biz Assembly dili 
	i�in baska bir interrupt (PendSV) kullanmaliyiz. �zellikle PendSV se�memizin sebebi ise; islemci �reticilerinin bu interrupt i RTOS i�in kullanilmak �zere olusturmus
  olmasi ve buna ayri hi�bir g�rev y�klemememis olmalaridir. DataSheet inceledndiginde bunun RTOS i�in kullanilmasinin uygun oldugunun yazilmis oldugu g�r�lebilir
*/		


void PendsV_Handler(void);
void SysTick_Handler(void);
void Configuration(void);



uint32_t count = 0;
uint32_t count1 = 0;



/* Thread1 tanimlamalari */
uint32_t thread1_stack[32];											/* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" �zelligi sebebiyle 8'in kati olan bellek se�ildi */
RTOS_Thread thread1;
void main_Thread1 ()
{
	while (1)
	{
		count++;
	}
}



/* Thread2 tanimlamalari */
uint32_t thread2_stack[32];										 /* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" �zelligi sebebiyle 8'in kati olan bellek se�ildi */
RTOS_Thread thread2;
void main_Thread2 ()
{
	while (1)
	{
		count1++;
	}
}






int main ()
{
	
	RTOS_init();

	RTOS_Thread_Create(&thread1, &main_Thread1, thread1_stack, sizeof(thread1_stack));			/* Thread1 olusturuldu */
	RTOS_Thread_Create(&thread2, &main_Thread2, thread2_stack, sizeof(thread2_stack));			/* Thread1 olusturuldu */


	
	Configuration();																		/* Systick �zellikle burada baslatildiki, direk baslatilsaydi devreye girdigi anda daha main() i�erisinde olan
																											ve bu satira kadar olan kodlar baslayamadan direk olarak PendSV yi tetikleyebilecek ve ilk thread isleme alinmaya
																											�alisilacak ve kalan kodlarin �alistirilmasi i�in main() e geri d�n�s saglanamayacakti. Bu sebeple t�m ayarlar
																											yapildiktan ve t�m Thread ler create edildikten sonra SysTick devreye alinmistir */

	
	
/* Ilk olarak context-switching islemi burada baslatiliyor, burada SysTick_Handler in yaptigini elle yapmis oluyoruz */
	__disable_irq();																		/* Olusabilecek Race-Conditions durumunun �n�ne ge�mek i�in interruptlari kapattik */
	
	RTOS_ThreadSwitch();																/* Eger devreye girecek  Thread varsa burada belirleyip, PendSV yi devreye alacak biti set ediyoruz,
																											ve alt satirda interruptlari aktif ettigimizde PendSV devreye girecek ve context-switching  baslayacaktir */
	
	__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */

	
//while (1) seklinde bir d�ng�ye ihtiya� kalmamistir ��nk� yukarida context-switching islemi baslatilmistir, haliyle bir daha main() in bu satirina d�n�lemeyecektir */


return 0;
}












void SysTick_Handler(void)
{
	
	__disable_irq();																		/* Olusabilecek Race-Conditions durumunun �n�ne ge�mek i�in interruptlari kapattik */
	
	RTOS_ThreadSwitch();																/* Eger devreye girecek  Thread varsa burada belirleyip, PendSV yi devreye alacak biti set ediyoruz,
																											ancak SysTick_Handler �alismasini bitirdikten sonra, PendSV devreye girebilecek ve context-switching islemini yapabilecektir*/
	
	__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */

}



__asm 													/* Assembly kodlari �alistiracagimizi belirtiyoruz */
void PendSV_Handler(void)
{

	
	IMPORT RTOS_ThreadCurr				/* Extern olarak aliyoruz */
	IMPORT RTOS_ThreadNext				/* Extern olarak aliyoruz */
	
	/* __disable_irq() */
	CPSID					I 
	
	/* if (RTOS_ThreadCurr != (RTOS_Thread *)0) { */			/*Bu kontrol� yapmamizin sebebi; eger halihazirda �alisan yani baslangi� durumunda hi� Thread yoksa direk RTOS_ThreadNext i�erisindeki ilk ve tek Thread i kullan, Thread varsa i�erisindeki sp ye (RTOS_ThreadCurr->sp), islemci sp sini ata ve b�ylece kaldigin noktayi kaydetmis ol demek i�indir */ 
	
	LDR 					r1,=RTOS_ThreadCurr
	LDR						r1,[r1,#0x00]
	CBZ						r1,Label_first
		
	/* Push r4-r11 komutu ile islemcinin bu register larini Stack Bellege yolluruz, bu Thread e ileride geri d�nerken r4-r11 registerleri geri y�kleyecez
	Bunu yapmamizin nedeni; Thread ler islem yaparken R4-R11 araligindaki islemci register larina veri yazmis olabilir ve context-switching ile yeniden devreye
	alindiklarinda bu register lari kullanacaklari i�in bunlari biz manuel olarak Stack Bellege yazariz, ��nk� interrupt devreye girerken bu register lari
	otomatik olarak Stack Bellege yollamaz */
	PUSH					{r4,r11}
	
	/* RTOS_ThreadCurr->sp = sp */   									   /*	Islemcinin Stack Pointer ini yani Stack Bellegin top noktasinin adresini, �alismakta olan Thread in sp pointer inin i�ine kaydediyoruz */
	LDR 					r1,=RTOS_ThreadCurr										 /* Ileride bu Thread i tekrar devreye aldigimizda, bu sp degerini islemciye geri y�kleyerek kaldigimiz yerden devam etmis olacagiz */
	LDR						r1,[r1,#0x00]
	STR						sp,[r1,#0x00]
	
	Label_first
	/* sp = RTOS_ThreadNext->sp */											 /* Simdi devreye alacagimiz Thread in i�erisindeki, o Thread a �zel olusturdugumuz Stack Bellegin top adresini, islemcinin sp registerine y�kl�yoruz */
	LDR 					r1,=RTOS_ThreadNext									  
	LDR						r1,[r1,#0x00]
	LDR						sp,[r1,#0x00]
	
	/* RTOS_ThreadCurr = RTOS_ThreadNext */					 		/* Devreye alacagimiz sonraki Thread olan RTOS_ThreadNext i, artik RTOS_ThreadCurr 'e atiyoruz, b�ylece yeni Thread i devreye almis oluyoruz */
	LDR 					r1,=RTOS_ThreadNext									  
	LDR						r1,[r1,#0x00]
	LDR						r2,=RTOS_ThreadCurr
	STR						r1,[r2,#0x00]

	/* Pop r4-r11 komutu ile, suanda devreye alinacak Thread a �zel olusturulmus Stack Bellekten, R4-R11 registerleri geri y�kleyecegiz (islemcinin r4-r11 register larina y�klenecek )
		 Bunu, context-swithing isleminin ger�eklesecegi, interrupt dan �ikis komutu olan BX komutundan hemen �nce yapiyoruz, ��nk� islemci bunu manuel yapmiyor */
	POP					 {r4,r11}

	/* __enable_irq() */																/* Interuptlari yeniden aktif ediyoruz */
	CPSIE					I 
	
	BX						lr																		/*Interrupt dan �ik */	
	
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
