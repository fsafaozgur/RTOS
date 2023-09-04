									
Hazırlayan: Fatih Safa Özgür

# RTOS Uygulaması

## Giriş
Proje kapsamında; sıra ile çalışan Thread 'lerden oluşan bir Multi Thread yapı oluşturularak bir RTOS tasarlanmıştır. 

## Hedef
Proje ile birlikte; basit bir RTOS tasarımı yapılarak; context-switching işleminin nasıl yapıldığı, temel anlamda bir RTOS  'un nasıl tasarlanacağının ortaya konulması amaçlanmıştır. İlerleyen süreçte Preemptive Priority Based bir RTOS tasarımı yapılacak, böylece Thread 'lerin önem sırasına göre çalışması sağlanacak, Shared Sources kullanıldığı durumlarda da Mutex kullanımı ele alınacak, proje zamanla geliştirilecektir.

## Çalışma Prensibi
Sisteme ilk olarak Thread 'lar tanımlanarak bunlara ait olan ve çalıştırılacak kodlar belirlenmekte ve daha sonra RTOS devreye alınmaktadır.

RTOS çalışma mantığı context-switching yapısıdır. Bu yapı kullanılarak Thread 'ler arasında geçiş yapılmakta ve sanki eş zamanlı olarak birden çok Thread çalışmakta gibi bir sonuç elde edilmektedir. Buna Multi Thread çalışma denmektedir. 

SysTick Timer kullanılarak belli aralıklarla oluşturulan kesmeler ile işlemler yapılmakta ve Thread 'lar arası geçişi sağlamak yani context-switching işlemini yapmak için PendSV kesmesi devreye alınmaktadır. 

Context-switching işlemini, SysTick_Handler içerisinde yapılmamakta bunun için, bu amaçla işlemci üreticileri tarafından oluşturulmuş PendSV ile kullanılmaktadır. Bunun bir sebebi, SysTick Timer 'ın pek çok farklı amaç için kullanılabilmesi ve bazı zamanlarda manuel olarak context-switching yaptırmamız gerektiğinde SysTick 'i çağırırsak bu durumda SysTick_Handler tarafından çalıştırılan ve sistemimizde kullanılan diğer işlemlerin etkilenebilecek olması veya belli aralıklarla değeri değiştirilen değişkenlerin fazladan işlem görmesi nedeniyle, sistemde kullanılan diğer işlemler için beklenmeyen sonuçların doğabilecek olmasıdır.

PendSV kullanımının bir diğer sebebi ise; işlemcinin SP (Stack Pointer) register değerine ancak Assembly ile müdahele edilebilmesi yani context-switching işleminin Assembly kodları ile yapılabilmesidir. Systick içerisinde hem C kodları hem Assembly kodları çok hoş bir durum oluşturmayacağı için Assembly kodlarını ayrı bir Handler içerisinde kullanmak daha mantıklıdır.

Sonuç olarak sisteme istenilen kadar Thread eklenmekte, bu Thread 'lere ait main fonksiyonları sıra ile çalıştırılmaktadır. İleride yapılacak tasarım ile bu Thread 'ler sıra ile değil, öncelik seviyelerine (priority level) göre çalıştırılacak, hiçbir Thread devrede değilken ise Idle Thread devreye alınarak güç tasarrufu sağlanacaktır.