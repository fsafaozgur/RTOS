#include "rtos.h"
#include "bsp.h"


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


void main_Thread1 (void);
void main_Thread2 (void);

uint32_t count = 0;
uint32_t count1 = 0;
uint32_t sysTick_count = 0;


/* Thread1 tanimlamalari */
uint32_t thread1_stack[32];											/* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" �zelligi sebebiyle 8'in kati olan bellek se�ildi */
RTOS_Thread thread1;
void main_Thread1(void)
{
	while (1)
	{
		count++;
	}
}



/* Thread2 tanimlamalari */
uint32_t thread2_stack[32];										 /* 32 adet 32 bitlik verinin atanabilecegi bir Stack Bellek olusturduk, ARM'in "8 byte boundary" �zelligi sebebiyle 8'in kati olan bellek se�ildi */
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


	
	Configuration();																		/* Systick �zellikle burada baslatildiki, direk baslatilsaydi devreye girdigi anda daha main() i�erisinde olan
																											ve bu satira kadar olan kodlar baslayamadan direk olarak PendSV yi tetikleyebilecek ve ilk thread isleme alinmaya
																											�alisilacak ve kalan kodlarin �alistirilmasi i�in main() e geri d�n�s saglanamayacakti. Bu sebeple t�m ayarlar
																											yapildiktan ve t�m Thread ler create edildikten sonra SysTick devreye alinmistir */

	
	
/* Ilk olarak context-switching islemi burada baslatiliyor, burada SysTick_Handler in yaptigini elle yapmis oluyoruz */
	__disable_irq();																		/* Olusabilecek Race-Conditions durumunun �n�ne ge�mek i�in interruptlari kapattik */
	
	RTOS_ThreadSwitch();																/* Eger devreye girecek  Thread varsa burada belirleyip, PendSV yi devreye alacak biti set ediyoruz,
																											//ve alt satirda interruptlari aktif ettigimizde PendSV devreye girecek ve context-switching  baslayacaktir */
	
	__enable_irq();																		  /* Interuptlari yeniden aktif ediyoruz */

	
/* while (1) seklinde bir d�ng�ye ihtiya� kalmamistir ��nk� yukarida context-switching islemi baslatilmistir, haliyle bir daha main() in bu satirina d�n�lemeyecektir */


return 0;
}
















												








