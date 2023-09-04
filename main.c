#include "stm32f4xx.h"                  // Device header
#include "rtos.h"

/*
	Multi Thread olarak çalisan, ancak bu Thread leri sira ile çalistiran bir RTOS tasarlanmistir
	Ileride Preemptive, Priority-Based  RTOS tasarimi yapilacaktir, suan priority level e göre degil sirayla çalisan bir sistem yapilmistir.
	Preemptive, Priority-Based  RTOS yapi devreye alindigi zaman Idle_Thread de eklenecek böylece diger sistemler çalismazken devrede olacak Idle_Thread yardimiyla
	güç tasarrufu modu çalistirilarak verimli bir sistem tasarlanacaktir.
*/


/* GENEL NOT:

	RTOS 'un temeli Context-Switching  mekanizmasidir. Context-Switching islemi, islemcinin halihazirda kullanmis oldugu Stack Bellegi degistirme islemidir ve bu Stack Pointer 
	degerini degistirmek ile olur. Stack Pointer i manupile edebilmenin tek yolu Assembly kodlari oldugu için bu islem için Assembly dili kullanilacaktir.

	Context-Switching islemi Systick_Handler içerisinde degil, Systick_Handler tarafindan tetiklenen PendSV exceptionu ile PendSV_Handler içerisinde yapilacaktir. 
	Bunun nedenleri; Systick_Handler pek çok amaç için kullanilir, biz ise context-switching islemini bazen otomatik switch islemlerinde bazen de herhangi bir anda 
	bir Thread in süresi dolmussa onu manuel olara devreden çikarmak için context-switching islemi yapmamiz gerekebilir. Bu durumda Systick 'i manuel devreye alirsak beklenmeyen
	sonuçlar olabilir. Örnegin 1ms de bir devreye girerek birçok uygulamayla ilgili islemler yapan ya da delay saglayan SysTick_Handler i, süresi dolan Thread i devreden çikarmak için çagirirsak
	bu SysTick_Handlerin fazladan devreye girip içerisinde yer alan ve diger Thread 'lere islem yapan kodlarin fazladan çalismasina ya da sayma için kullanilan bir degiskenin 
	degerinin fazladan degistirilmesine sebep olacagi asikardir, bu sebeple sadece context-switching için ayri bir Interrup_Handler (PendSV_Handler) kullanmali, 
	SysTick_Handleri ise Thread lerin blocking sürelerini saymada, süresi biten varsa gerekli islemleri yapmada ve diger Thread ler için bagimsiz islemleri yapmada kullanabiliriz.
	Bizim uygulamamizda ise PendSV yi devreye almada ve ileride Blocking islemi yaparken, delay degerlerini saymada kullaniyor olacagiz.

	Ayrica mümkünse bir Interrup_Handler içerisinde sadece 1 dil kullanilmalidir(pure language) SysTick_Handler içerisinde C dili kullaniliyorsa biz Assembly dili 
	için baska bir interrupt (PendSV) kullanmaliyiz. Özellikle PendSV seçmemizin sebebi ise; islemci üreticilerinin bu interrupt i RTOS için kullanilmak üzere olusturmus
  olmasi ve buna ayri hiçbir görev yüklemememis olmalaridir. DataSheet inceledndiginde bunun RTOS için kullanilmasinin uygun oldugunun yazilmis oldugu görülebilir
*/		


void PendsV_Handler(void);
void SysTick_Handler(void);
void Configuration(void);



uint32_t count = 0;
uint32_t count1 = 0;



/* Thread1 tanimlamalari */
uint32_t thread1_stack[32];											/* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" özelligi sebebiyle 8'in kati olan bellek seçildi */
RTOS_Thread thread1;
void main_Thread1 ()
{
	while (1)
	{
		count++;
	}
}



/* Thread2 tanimlamalari */
uint32_t thread2_stack[32];										 /* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" özelligi sebebiyle 8'in kati olan bellek seçildi */
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


	
	Configuration();																		/* Systick özellikle burada baslatildiki, direk baslatilsaydi devreye girdigi anda daha main() içerisinde olan
																											ve bu satira kadar olan kodlar baslayamadan direk olarak PendSV yi tetikleyebilecek ve ilk thread isleme alinmaya
																											çalisilacak ve kalan kodlarin çalistirilmasi için main() e geri dönüs saglanamayacakti. Bu sebeple tüm ayarlar
																											yapildiktan ve tüm Thread ler create edildikten sonra SysTick devreye alinmistir */

	
	
/* Ilk olarak context-switching islemi burada baslatiliyor, burada SysTick_Handler in yaptigini elle yapmis oluyoruz */
	__disable_irq();																		/* Olusabilecek Race-Conditions durumunun önüne geçmek için interruptlari kapattik */
	
	RTOS_ThreadSwitch();																/* Eger devreye girecek  Thread varsa burada belirleyip, PendSV yi devreye alacak biti set ediyoruz,
																											ve alt satirda interruptlari aktif ettigimizde PendSV devreye girecek ve context-switching  baslayacaktir */
	
	__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */

	
//while (1) seklinde bir döngüye ihtiyaç kalmamistir çünkü yukarida context-switching islemi baslatilmistir, haliyle bir daha main() in bu satirina dönülemeyecektir */


return 0;
}












void SysTick_Handler(void)
{
	
	__disable_irq();																		/* Olusabilecek Race-Conditions durumunun önüne geçmek için interruptlari kapattik */
	
	RTOS_ThreadSwitch();																/* Eger devreye girecek  Thread varsa burada belirleyip, PendSV yi devreye alacak biti set ediyoruz,
																											ancak SysTick_Handler çalismasini bitirdikten sonra, PendSV devreye girebilecek ve context-switching islemini yapabilecektir*/
	
	__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */

}



__asm 													/* Assembly kodlari çalistiracagimizi belirtiyoruz */
void PendSV_Handler(void)
{

	
	IMPORT RTOS_ThreadCurr				/* Extern olarak aliyoruz */
	IMPORT RTOS_ThreadNext				/* Extern olarak aliyoruz */
	
	/* __disable_irq() */
	CPSID					I 
	
	/* if (RTOS_ThreadCurr != (RTOS_Thread *)0) { */			/*Bu kontrolü yapmamizin sebebi; eger halihazirda çalisan yani baslangiç durumunda hiç Thread yoksa direk RTOS_ThreadNext içerisindeki ilk ve tek Thread i kullan, Thread varsa içerisindeki sp ye (RTOS_ThreadCurr->sp), islemci sp sini ata ve böylece kaldigin noktayi kaydetmis ol demek içindir */ 
	
	LDR 					r1,=RTOS_ThreadCurr
	LDR						r1,[r1,#0x00]
	CBZ						r1,Label_first
		
	/* Push r4-r11 komutu ile islemcinin bu register larini Stack Bellege yolluruz, bu Thread e ileride geri dönerken r4-r11 registerleri geri yükleyecez
	Bunu yapmamizin nedeni; Thread ler islem yaparken R4-R11 araligindaki islemci register larina veri yazmis olabilir ve context-switching ile yeniden devreye
	alindiklarinda bu register lari kullanacaklari için bunlari biz manuel olarak Stack Bellege yazariz, çünkü interrupt devreye girerken bu register lari
	otomatik olarak Stack Bellege yollamaz */
	PUSH					{r4,r11}
	
	/* RTOS_ThreadCurr->sp = sp */   									   /*	Islemcinin Stack Pointer ini yani Stack Bellegin top noktasinin adresini, çalismakta olan Thread in sp pointer inin içine kaydediyoruz */
	LDR 					r1,=RTOS_ThreadCurr										 /* Ileride bu Thread i tekrar devreye aldigimizda, bu sp degerini islemciye geri yükleyerek kaldigimiz yerden devam etmis olacagiz */
	LDR						r1,[r1,#0x00]
	STR						sp,[r1,#0x00]
	
	Label_first
	/* sp = RTOS_ThreadNext->sp */											 /* Simdi devreye alacagimiz Thread in içerisindeki, o Thread a özel olusturdugumuz Stack Bellegin top adresini, islemcinin sp registerine yüklüyoruz */
	LDR 					r1,=RTOS_ThreadNext									  
	LDR						r1,[r1,#0x00]
	LDR						sp,[r1,#0x00]
	
	/* RTOS_ThreadCurr = RTOS_ThreadNext */					 		/* Devreye alacagimiz sonraki Thread olan RTOS_ThreadNext i, artik RTOS_ThreadCurr 'e atiyoruz, böylece yeni Thread i devreye almis oluyoruz */
	LDR 					r1,=RTOS_ThreadNext									  
	LDR						r1,[r1,#0x00]
	LDR						r2,=RTOS_ThreadCurr
	STR						r1,[r2,#0x00]

	/* Pop r4-r11 komutu ile, suanda devreye alinacak Thread a özel olusturulmus Stack Bellekten, R4-R11 registerleri geri yükleyecegiz (islemcinin r4-r11 register larina yüklenecek )
		 Bunu, context-swithing isleminin gerçeklesecegi, interrupt dan çikis komutu olan BX komutundan hemen önce yapiyoruz, çünkü islemci bunu manuel yapmiyor */
	POP					 {r4,r11}

	/* __enable_irq() */																/* Interuptlari yeniden aktif ediyoruz */
	CPSIE					I 
	
	BX						lr																		/*Interrupt dan çik */	
	
	/* 
		Bu komutun ardindan islemci kendi sp sinde yer alan (biz yukarida "sp = RTOS_ThreadNext->sp" islemini yaptirarak islemci sp sini manipüle ettik) adresin gösterdigi
		Stack Bellek 'ten degerleri okuyarak bunlari, registerlarina (PC, LR, R0, ...) yaziyor, böylece preempt ettigi Thread 'e (RTOS_ThreadCurr)dönecegini zannederken bizim manipülasyonumuz sayesinde
		bizim istedigimiz Thread 'e (RTOS_ThreadNext) dönmüs oluyor. 
	
		Threadlar arasi geçis yaparken, devreye alinan her Thread, kendi main_Thread() fonksiyonunun basindan degil, bir önce context-switching yapilarak devreden çikarilirken
		kalmis oldugu noktadan devam edecektir. Bunu PC register sayesinde yapabiliyoruz. Stack Bellegi kaydederken PC yi de kaydettigimiz için, program akisinda tam olarak
		hangi komutta kaldiysak bunu da PC yi kaydederken kaydetmis oluyoruz, context-switching ile tekrar devreye alinan Thread 'in Stack Belleginindeki kayitlari islemci kendi
		register larina atarken bu PC yi de yükledigi için haliyle son kalinan noktaya direk giderek oradan itibaren kalan kodlar islenmeye devam edecektir.
	*/ 
}






void Configuration(void)
{
	SystemCoreClockUpdate();																			
	SysTick->VAL = 0x0u;																					/* DataSheet'de yer alan "A write of any value clears the field to 0" ifadesine binaen "0" atayarak temizledik*/  
	SysTick->LOAD = (uint32_t) (SystemCoreClock / 8000);					/* 1 ms sürede reload yapcak sekilde ayarlandi*/
	SysTick->CTRL |= 0x07u;																				/* Interrupt aktif */ /* Clock Source AHB/8 ayarlandi */ /* Counter aktif */ 
	
	
	(*(volatile uint32_t *)0xE000ED20) |= (0u<<28);							/* SyscTick'in priority leveli PendSV'e göre yüksekte tutuldu ki Tail-Chaining saglanabilsin */
																															/* PendSV'nin priority leveli SysTick'e göre düsükte tutuldu ki Tail-Chaining saglanabilsin (rtos.c dosyasinda) */	
	/* 
	Tail-Chaining yapisindan kasit, context-switching islemine gerek kalmadan, interruptlari arka arkaya ekleyerek daha verimli bir çalisma saglamaktir.
	Söyle ki, her intterupt çalistiginda, main() de kalinan nokta (islemcinin kendi register larindaki degerler) Stack'e yazilarak ilgili Interrupt_Handler çalistirilir,
	Interrupt_Handler çalistirildiktan sonra main() e geri dönüs yapilirken, interrupt öncesindeki duruma dönmek için Stack'e yazilan  o anki context geri yüklenir 
	bu duruma context-swithhing denir ve haliyle maliyetli bir islemdir. Ancak biz Systick_Handler çalistirilip main() e geri dönmeden yani contect switching islemi yapilmadan
	direk PendsV_Handler i çalistirirsak, iki Interrupt_Handler i arka arkaya (Tail-Chaining) eklemis ve context-swithing islemini yapmamis oluruz.
*/

}
