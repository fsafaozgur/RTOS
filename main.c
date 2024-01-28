#include "rtos.h"
#include "bsp.h"


/*
	Multi Thread olarak �alisan, bu Thread leri �ncelik sirasina (priority level) g�re �alistiran,
	Preemptive, Priority-Based RTOS tasarimi yapilmistir. 
	
	Diger Thread 'ler �alismazken devrede olan Idle_Thread yardimiyla g�� tasarrufu modu �alistirilarak verimli bir sistem
	tasarlanabilecek iken, g�rsel olarak hangi Thread 'in devrede oldugu g�r�lebilsin diye Idle Thread ve diger Thread 'lerin
	kod bloklarina count degiskenleri verilerek sistem �alismasi g�zlemlenebilir hale getirilmistir.
*/


/* GENEL NOT:

	RTOS 'un temeli Context-Switching  mekanizmasidir. Context-Switching islemi, islemcinin halihazirda kullanmis oldugu Stack Bellegi degistirme islemidir ve bu Stack Pointer 
	degerini degistirmek ile olur. Stack Pointer i manupile edebilmenin tek yolu Assembly kodlari oldugu i�in bu islem i�in Assembly dili kullanilacaktir.

	Context-Switching islemi Systick_Handler i�erisinde degil, Systick_Handler tarafindan tetiklenen PendSV exceptionu ile PendSV_Handler i�erisinde yapilacaktir. 
	Bunun nedenleri; Systick_Handler pek �ok ama� i�in kullanilir, biz ise context-switching islemini bazen otomatik switch islemlerinde bazen de herhangi bir anda 
	bir Thread in s�resi dolmussa onu manuel olara devreden �ikarmak i�in context-switching islemi yapmamiz gerekebilir. Bu durumda Systick 'i manuel devreye alirsak beklenmeyen
	sonu�lar olabilir. �rnegin 1ms de bir devreye girerek bir�ok uygulamayla ilgili islemler yapan ya da delay saglayan SysTick_Handler 'i, s�resi dolan Thread 'i devreden �ikarmak i�in �agirirsak
	bu SysTick_Handlerin fazladan devreye girip, i�erisinde yer alan ve diger Thread 'lere islem yapan kodlarin fazladan �alismasina ya da sayma i�in kullanilan bir degiskenin 
	degerinin fazladan degistirilmesine sebep olacagi asikardir, bu sebeple sadece context-switching i�in ayri bir Interrup_Handler (PendSV_Handler) kullanmali, 
	SysTick_Handleri ise Thread lerin blocking s�relerini saymada, s�resi biten varsa gerekli islemleri yapmada ve diger Thread ler i�in bagimsiz islemleri yapmada kullanabiliriz.
	Bizim uygulamamizda ise SysTick_Handler 'i, PendSV yi devreye almada ve Blocking islemi yaparken, delay degerlerini saymada kullaniyor olacagiz.

	Ayrica m�mk�nse bir Interrup_Handler i�erisinde sadece 1 dil kullanilmalidir(pure language) SysTick_Handler i�erisinde C dili kullaniliyorsa biz Assembly dili 
	i�in baska bir interrupt (PendSV) kullanmaliyiz. �zellikle PendSV se�memizin sebebi ise; islemci �reticilerinin bu interrupt i RTOS i�in kullanilmak �zere olusturmus
  olmasi ve buna ayri hi�bir g�rev y�klemememis olmalaridir. DataSheet inceledndiginde bunun RTOS i�in kullanilmasinin uygun oldugunun yazilmis oldugu g�r�lebilir
*/		


void main_Thread1 (void);
void main_Thread2 (void);


uint32_t count = 0u;
uint32_t count1 = 0u;
uint32_t count2 = 0u;




/* IdleThread tanimlamalari */
uint32_t idleThread_stack[32];				/* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" �zelligi sebebiyle 8'in kati olan bellek se�ildi */
RTOS_Thread idleThread;
void main_IdleThread(void)
{
	while (1)
	{
		/* IdleThread i�erisinde �alistirilacak kodlar "Power Safer" g�revi g�ren kodlar olabilir, ��nk� hi�bir
				Thread �alismazken biz IdleThread i devreye alacagiz, kisacasi islemci zamaninin �ok b�y�k b�l�m�n� burda ge�irecek.
				Ancak, g�zlemlenebilsin diye simdilik bir degiskeni arttirmak i�in kullaniyoruz.
		*/
		
		count++;
	}
}




/* Thread-1 tanimlamalari */
uint32_t thread1_stack[32];				     /* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" �zelligi sebebiyle 8'in kati olan bellek se�ildi */
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
uint32_t thread2_stack[32];						/* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" �zelligi sebebiyle 8'in kati olan bellek se�ildi */
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
	
	/* Ilk olusturulan Thread mutlaka IdleThread olmali, tasarim buna g�re yapilmistir */
	RTOS_Thread_Create(&idleThread, &main_IdleThread, idleThread_stack, sizeof(idleThread_stack));			/* IdleThread olusturuldu */
	RTOS_Thread_Create(&thread1, &main_Thread1, thread1_stack, sizeof(thread1_stack));									/* Thread-1 olusturuldu */
	RTOS_Thread_Create(&thread2, &main_Thread2, thread2_stack, sizeof(thread2_stack));									/* Thread-2 olusturuldu */


	
	Configuration();																		/* Systick �zellikle burada baslatildiki, direk baslatilsaydi devreye girdigi anda daha main() i�erisinde olan
																											ve bu satira kadar olan kodlar baslayamadan direk olarak PendSV yi tetikleyebilecek ve ilk thread isleme alinmaya
																											�alisilacak ve kalan kodlarin �alistirilmasi i�in main() e geri d�n�s saglanamayacakti. Bu sebeple t�m ayarlar
																											yapildiktan ve t�m Thread ler olusturulduktan sonra SysTick devreye alinmistir */

	
	
/* Ilk olarak context-switching islemi burada baslatiliyor, burada SysTick_Handler in yaptigini elle yapmis oluyoruz */
	__disable_irq();																		/* Olusabilecek Race-Conditions durumunun �n�ne ge�mek i�in interruptlari kapattik */
	
	RTOS_ThreadSwitch();																/* Eger devreye girecek  Thread varsa burada belirleyip, PendSV yi devreye alacak biti set ediyoruz,
																											ve alt satirda interruptlari aktif ettigimizde PendSV devreye girecek ve context-switching  baslayacaktir */
	
	__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */

	
/* while (1) seklinde bir d�ng�ye ihtiya� kalmamistir ��nk� yukarida context-switching islemi baslatilmistir, haliyle bir daha main() in bu satirina d�n�lemeyecektir */


return 0;
}
















												








