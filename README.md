									
Hazırlayan: Fatih Safa Özgür

# RTOS Uygulaması

## Giriş
Proje kapsamında; öncelik seviyesine (Priority level) göre  çalışan Thread 'lerden oluşan bir Multi Thread yapı oluşturularak bir RTOS tasarlanmıştır. 

## Hedef
Proje ile birlikte; basit bir RTOS tasarımı yapılarak; context-switching işleminin nasıl yapıldığı, temel anlamda bir RTOS 'un nasıl tasarlanacağının ortaya konulması amaçlanmıştır. RTOS tasarımı, Preemptive Priority Based olarak yapılmış, böylece Thread 'lerin önem sırasına göre çalışması sağlanmıştır. Shared Sources kullanıldığı durumlarda da Mutex kullanımı ileride projeye örnek olarak eklenecek, proje zamanla geliştirilecektir.

## Çalışma Prensibi
Sisteme ilk olarak Thread 'ler tanımlanarak bunlara ait olan ve çalıştırılacak kodlar belirlenmekte ve daha sonra RTOS devreye alınmaktadır.

RTOS çalışma mantığı context-switching yapısıdır. Bu yapı kullanılarak Thread 'ler arasında geçiş yapılmakta ve sanki eş zamanlı olarak birden çok Thread çalışmakta gibi bir sonuç elde edilmektedir. Buna Multi Thread çalışma denmektedir. 

SysTick Timer kullanılarak belli aralıklarla oluşturulan kesmeler ile işlemler yapılmakta (scheduling ve decrement) ve Thread 'ler arası geçişi sağlamak yani context-switching işlemini yapmak için PendSV kesmesi kullanılmaktadır. 

Context-switching işlemini, SysTick_Handler içerisinde yapılmamakta, bu işlemi gerçekleştirmek için, bu amaçla işlemci üreticileri tarafından oluşturulmuş PendSV kesmesi kullanılmaktadır. Bunun bir sebebi, SysTick Timer 'ın pek çok farklı amaç için kullanılabilmesi ve bazı zamanlarda manuel olarak context-switching yaptırmamız gerektiğinde (örneğin, delay vermek suretiyle "current thread" devreden çıkarılırken) SysTick 'i çağırırsak bu durumda SysTick_Handler tarafından çalıştırılan ve sistemimizde kullanılan diğer işlemlerin etkilenebilecek olması veya belli aralıklarla değeri değiştirilen değişkenlerin fazladan işlem görmesi nedeniyle, sistemde kullanılan diğer işlemler için beklenmeyen sonuçların doğabilecek olmasıdır.

PendSV kullanımının bir diğer sebebi ise; işlemcinin SP (Stack Pointer) register değerine ancak Assembly ile müdahele edilebilmesi yani context-switching işleminin Assembly kodları ile yapılabilmesidir. Systick içerisinde hem C kodları hem Assembly kodları çok hoş bir durum oluşturmayacağı için Assembly kodlarını ayrı bir Interrupt Handler içerisinde kullanmak daha mantıklıdır.

Sonuç olarak sisteme istenilen kadar Thread eklenmekte, bu Thread 'lere ait main fonksiyonları, ait olduğu Thread 'lerin öncelik seviyelerine (priority level) göre çalıştırılmaktadır. RTOS çalışırken hiçbir Thread devrede değilken ise Idle Thread devreye alınmaktadır. RTOS çalışma zamanının büyük bölümünde Idle Thread devrede olduğu için, istenilirse buna ait çalıştırılacak kod bloğuna bir "power saver" yapısı tanımlanarak güç tasarrufu sağlanabilecektir.
