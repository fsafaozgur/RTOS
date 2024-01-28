#include "rtos.h"
#include "bsp.h"


/*
	Multi Thread olarak çalisan, bu Thread leri öncelik sirasina (priority level) göre çalistiran,
	Preemptive, Priority-Based RTOS tasarimi yapilmistir. 
	
	Diger Thread 'ler çalismazken devrede olan Idle_Thread yardimiyla güç tasarrufu modu çalistirilarak verimli bir sistem
	tasarlanabilecek iken, görsel olarak hangi Thread 'in devrede oldugu görülebilsin diye Idle Thread ve diger Thread 'lerin
	kod bloklarina count degiskenleri verilerek sistem çalismasi gözlemlenebilir hale getirilmistir.
*/


/* GENEL NOT:

	RTOS 'un temeli Context-Switching  mekanizmasidir. Context-Switching islemi, islemcinin halihazirda kullanmis oldugu Stack Bellegi degistirme islemidir ve bu Stack Pointer 
	degerini degistirmek ile olur. Stack Pointer i manupile edebilmenin tek yolu Assembly kodlari oldugu için bu islem için Assembly dili kullanilacaktir.

	Context-Switching islemi Systick_Handler içerisinde degil, Systick_Handler tarafindan tetiklenen PendSV exceptionu ile PendSV_Handler içerisinde yapilacaktir. 
	Bunun nedenleri; Systick_Handler pek çok amaç için kullanilir, biz ise context-switching islemini bazen otomatik switch islemlerinde bazen de herhangi bir anda 
	bir Thread in süresi dolmussa onu manuel olara devreden çikarmak için context-switching islemi yapmamiz gerekebilir. Bu durumda Systick 'i manuel devreye alirsak beklenmeyen
	sonuçlar olabilir. Örnegin 1ms de bir devreye girerek birçok uygulamayla ilgili islemler yapan ya da delay saglayan SysTick_Handler 'i, süresi dolan Thread 'i devreden çikarmak için çagirirsak
	bu SysTick_Handlerin fazladan devreye girip, içerisinde yer alan ve diger Thread 'lere islem yapan kodlarin fazladan çalismasina ya da sayma için kullanilan bir degiskenin 
	degerinin fazladan degistirilmesine sebep olacagi asikardir, bu sebeple sadece context-switching için ayri bir Interrup_Handler (PendSV_Handler) kullanmali, 
	SysTick_Handleri ise Thread lerin blocking sürelerini saymada, süresi biten varsa gerekli islemleri yapmada ve diger Thread ler için bagimsiz islemleri yapmada kullanabiliriz.
	Bizim uygulamamizda ise SysTick_Handler 'i, PendSV yi devreye almada ve Blocking islemi yaparken, delay degerlerini saymada kullaniyor olacagiz.

	Ayrica mümkünse bir Interrup_Handler içerisinde sadece 1 dil kullanilmalidir(pure language) SysTick_Handler içerisinde C dili kullaniliyorsa biz Assembly dili 
	için baska bir interrupt (PendSV) kullanmaliyiz. Özellikle PendSV seçmemizin sebebi ise; islemci üreticilerinin bu interrupt i RTOS için kullanilmak üzere olusturmus
  olmasi ve buna ayri hiçbir görev yüklemememis olmalaridir. DataSheet inceledndiginde bunun RTOS için kullanilmasinin uygun oldugunun yazilmis oldugu görülebilir
*/		


void main_Thread1 (void);
void main_Thread2 (void);


uint32_t count = 0u;
uint32_t count1 = 0u;
uint32_t count2 = 0u;




/* IdleThread tanimlamalari */
uint32_t idleThread_stack[32];				/* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" özelligi sebebiyle 8'in kati olan bellek seçildi */
RTOS_Thread idleThread;
void main_IdleThread(void)
{
	while (1)
	{
		/* IdleThread içerisinde çalistirilacak kodlar "Power Safer" görevi gören kodlar olabilir, çünkü hiçbir
				Thread çalismazken biz IdleThread i devreye alacagiz, kisacasi islemci zamaninin çok büyük bölümünü burda geçirecek.
				Ancak, gözlemlenebilsin diye simdilik bir degiskeni arttirmak için kullaniyoruz.
		*/
		
		count++;
	}
}




/* Thread-1 tanimlamalari */
uint32_t thread1_stack[32];				     /* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" özelligi sebebiyle 8'in kati olan bellek seçildi */
RTOS_Thread thread1;
void main_Thread1(void)
{
	while (1)
	{
		count1++;
		RTOS_delay(1000);
	}
}



/* Thread-2 tanimlamalari */
uint32_t thread2_stack[32];						/* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" özelligi sebebiyle 8'in kati olan bellek seçildi */
RTOS_Thread thread2;
void main_Thread2(void)
{
	while (1)
	{
		count2++;
		RTOS_delay(3000);
	}
}







int main ()
{
	
	RTOS_init();
	
	/* Ilk olusturulan Thread mutlaka IdleThread olmali, tasarim buna göre yapilmistir */
	RTOS_Thread_Create(&idleThread, &main_IdleThread, idleThread_stack, sizeof(idleThread_stack));			/* IdleThread olusturuldu */
	RTOS_Thread_Create(&thread1, &main_Thread1, thread1_stack, sizeof(thread1_stack));									/* Thread-1 olusturuldu */
	RTOS_Thread_Create(&thread2, &main_Thread2, thread2_stack, sizeof(thread2_stack));									/* Thread-2 olusturuldu */


	
	Configuration();																		/* Systick özellikle burada baslatildiki, direk baslatilsaydi devreye girdigi anda daha main() içerisinde olan
																											ve bu satira kadar olan kodlar baslayamadan direk olarak PendSV yi tetikleyebilecek ve ilk thread isleme alinmaya
																											çalisilacak ve kalan kodlarin çalistirilmasi için main() e geri dönüs saglanamayacakti. Bu sebeple tüm ayarlar
																											yapildiktan ve tüm Thread ler olusturulduktan sonra SysTick devreye alinmistir */

	
	
/* Ilk olarak context-switching islemi burada baslatiliyor, burada SysTick_Handler in yaptigini elle yapmis oluyoruz */
	__disable_irq();																		/* Olusabilecek Race-Conditions durumunun önüne geçmek için interruptlari kapattik */
	
	RTOS_ThreadSwitch();																/* Eger devreye girecek  Thread varsa burada belirleyip, PendSV yi devreye alacak biti set ediyoruz,
																											ve alt satirda interruptlari aktif ettigimizde PendSV devreye girecek ve context-switching  baslayacaktir */
	
	__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */

	
/* while (1) seklinde bir döngüye ihtiyaç kalmamistir çünkü yukarida context-switching islemi baslatilmistir, haliyle bir daha main() in bu satirina dönülemeyecektir */


return 0;
}
















												








