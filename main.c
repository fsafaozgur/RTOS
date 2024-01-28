#include "rtos.h"
#include "bsp.h"


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


void main_Thread1 (void);
void main_Thread2 (void);

uint32_t count = 0;
uint32_t count1 = 0;
uint32_t sysTick_count = 0;


/* Thread1 tanimlamalari */
uint32_t thread1_stack[32];											/* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" özelligi sebebiyle 8'in kati olan bellek seçildi */
RTOS_Thread thread1;
void main_Thread1(void)
{
	while (1)
	{
		count++;
	}
}



/* Thread2 tanimlamalari */
uint32_t thread2_stack[32];										 /* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" özelligi sebebiyle 8'in kati olan bellek seçildi */
RTOS_Thread thread2;
void main_Thread2(void)
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
																											//ve alt satirda interruptlari aktif ettigimizde PendSV devreye girecek ve context-switching  baslayacaktir */
	
	__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */

	
/* while (1) seklinde bir döngüye ihtiyaç kalmamistir çünkü yukarida context-switching islemi baslatilmistir, haliyle bir daha main() in bu satirina dönülemeyecektir */


return 0;
}
















												








