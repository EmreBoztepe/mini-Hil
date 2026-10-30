# mini-Hil — Proje Kapsamı

## Amaç

STM32F407 Discovery kartını, farklı ECU / hedef PCB'ler üzerinde çalışan gömülü
yazılımların doğrulanmasında kullanılacak, yeniden kullanılabilir bir mini HIL
test platformunun temeli haline getirmek.

Platform tek bir hedef karta özel tasarlanmayacaktır. Hedef kart değiştiğinde,
desteklenen donanım yetenekleri içinde ortak HIL altyapısını değiştirmeden hedef
profilini, bağlantı adaptörünü ve test senaryolarını uyarlamak amaçlanır.

İlk adım, Discovery kartı ile ECU arasında UART üzerinden basit bir istek–yanıt
(handshake) haberleşmesi kurmak ve bu haberleşmeyi ölçülebilir kabul kriterleriyle
doğrulamaktır. UART protokolü iki taraf için de tasarlanacaktır.

Bu doküman kapsamı ve önerilen yaklaşımı tanımlar. Pin bağlantıları, elektriksel
uyumluluk, paket formatı ve zamanlama değerleri henüz kesinleştirilmemiştir.

## Doğrulanan başlangıç bilgileri

- Test kartı: STM32F407 Discovery.
- Test edilen cihaz: değiştirilebilir gerçek ECU / hedef PCB.
- İlk hedef için Discovery–ECU haberleşmesi: UART.
- İlk hedef için mevcut bir UART test protokolü bulunmuyor; protokol tasarlanacak.
- İlk test: basit bir mesaj gönderilmesi ve beklenen yanıtın alınması.
- Temel tasarım gereksinimi: farklı hedef kartlara hızlı uyarlanabilmek.
- Projenin uzun vadeli amacı: otomatik gömülü yazılım validasyonu için mini HIL.

## Mimari ve sorumluluklar

| Bileşen | Sorumluluk |
| --- | --- |
| STM32F407 Discovery | İlk aşamada UART test isteğini göndermek, yanıtı almak ve süreyi ölçmek. İleride fiziksel uyaran üretimi ve ölçüm görevlerini üstlenmek. |
| ECU / hedef PCB | Gerçek uygulama yazılımını çalıştırmak ve tanımlanan UART isteğine yanıt vermek. |
| PC / Python | İlerleyen aşamada testleri yönetmek, sonuçları değerlendirmek ve raporlamak. PC bağlantısı ve GUI ilk handshake testinin ön koşulu değildir. |

ECU tarafındaki haberleşme işleme, uygulamanın normal görevlerini bloke etmeyecek
şekilde tasarlanmalıdır. Zamanlaması kritik fiziksel sinyal üretimi gerektiğinde
bu iş Discovery üzerinde, uygun timer donanımıyla yürütülmelidir.

## Farklı hedef kartlara uyarlanabilirlik

Önerilen ayrım aşağıdaki gibidir. Bu ayrım sorumlulukları tanımlar; başlangıçta
ayrı bir eklenti sistemi veya kapsamlı bir framework kurulmasını gerektirmez.

| Alan | İçerik ve değişim sınırı |
| --- | --- |
| Ortak HIL altyapısı | Desteklenen çevre birimlerine erişim, zaman ölçümü ve ortak test işlemleri. Hedef karta ait pin adları, mesaj içerikleri veya geçme/kalma eşikleri burada sabitlenmez. |
| Hedef profili | Hedef kimliği, mantıksal sinyallerin HIL kanallarına eşlenmesi, haberleşme ayarları, elektriksel sınırlar ve kullanılacak adaptör bilgisi. |
| Hedef haberleşme tanımı | İstek/yanıt formatları ve bunların işlenmesi. Gerektiğinde hedefe özel protokol işleme kodu içerir. |
| Fiziksel bağlantı adaptörü | Kablo demeti, konnektör eşlemesi ve gereken seviye dönüştürme, koruma veya izolasyon devreleri. |
| Test senaryoları | Hedefe uygulanacak adımlar, beklenen yanıtlar, süre sınırları ve kabul kriterleri. |

Testler, mümkün olduğunda fiziksel pin numarası yerine mantıksal sinyal veya
kanal adları kullanmalıdır. Bu adların gerçek HIL kaynağına ve hedef pinine
eşlenmesi hedefe ait bağlantı tanımında tutulmalıdır.

Hedef değişimi için öngörülen iş akışı:

1. Hedefin sinyal, haberleşme, elektriksel seviye ve zamanlama ihtiyaçlarını çıkar.
2. Mevcut HIL kaynaklarının ve adaptörün bu ihtiyaçları karşılayabildiğini doğrula.
3. Hedef profilini, bağlantı tanımını ve gerekiyorsa protokol işleme kodunu hazırla.
4. Hedefe ait test senaryolarını seç veya oluştur; önce bağlantı/handshake testini çalıştır.
5. Sonuçları kullanılan hedef profili ve adaptör revizyonuyla birlikte kaydet.

Esneklik, her kartın yalnızca yazılım ayarıyla bağlanabileceği anlamına gelmez.
Farklı elektriksel arayüzler adaptör değişikliği gerektirebilir. Desteklenmeyen
bir sinyal tipi veya zamanlama ihtiyacı ise ortak HIL yeteneklerine kontrollü bir
ekleme gerektirebilir. Uyumsuz veya çakışan kaynak eşlemeleri, uyaran üretilmeden
önce reddedilmelidir.

Hedef profilinin derleme sırasında mı yoksa çalışma sırasında mı seçileceği,
ilk ihtiyaçlar netleştiğinde kararlaştırılacaktır. İlk aşamada yalnızca ilk UART
handshake testinin gerektirdiği ayrım uygulanacaktır.

### Uyarlanabilirliğin doğrulanması

- Desteklenen UART ayarları, hedef mesajları ve timeout değerleri ortak altyapıya
  sabitlenmeden hedef tanımı üzerinden değiştirilebilir olmalıdır.
- Mevcut yetenekleri kullanan yeni bir hedef için değişiklikler hedef profili,
  protokol işleme, bağlantı adaptörü ve senaryolarla sınırlı kalmalıdır.
- İkinci hedef belirlendiğinde bu geçiş gerçek donanım üzerinde doğrulanmalıdır;
  ilk hedefteki handshake başarısı tek başına yeniden kullanılabilirlik kanıtı değildir.

## İlk aşamanın kapsamı: UART handshake

Bu test, yeniden kullanılabilir altyapının ilk hedef profiliyle gerçekleştirilen
ilk kullanım örneğidir. Mesaj içeriği ve hedefe özgü ayarlar ortak altyapıya
gömülmemelidir. İlk hedef için tasarlanacak UART protokolünün gelecekteki bütün
hedef kartlar tarafından kullanılması zorunlu değildir.

Önerilen başlangıç akışı:

1. Discovery, ECU'ya bir handshake isteği gönderir.
2. ECU, tamamlanmış ve geçerli isteği ayrıştırır.
3. ECU, protokolde tanımlanan yanıtı gönderir.
4. Discovery, yanıtın beklenen isteğe ait olduğunu ve içeriğini doğrular.
5. Discovery, yanıt süresini ölçer ve sonucu başarılı veya başarısız olarak kaydeder.

Yanıt alınamaması, geçersiz yanıt ve haberleşme hatası birbirinden ayırt
edilebilmelidir. Sınırsız bekleme kullanılmamalıdır.

Handshake başarısı, tanımlanan UART istek–yanıt yolunun çalıştığını gösterir.
ECU'nun bütün yazılımının, görev zamanlamasının veya fiziksel I/O davranışının
doğrulandığı anlamına gelmez.

### Protokol tasarımında ele alınacak konular

- Paket sınırının ve paket uzunluğunun belirlenmesi.
- İstek ve yanıt mesajlarının kimlikleri ve içerikleri.
- Yanıtı ilgili isteğe bağlayacak sıra numarası veya eşdeğer mekanizma.
- Paket bütünlüğü kontrolü; CRC kullanımı ve parametreleri.
- Yanıt timeout'u ve sürenin hangi olaylar arasında ölçüleceği.
- Eksik, bozuk veya beklenmeyen paket sonrası yeniden paket hizası bulma davranışı.
- Protokol sürümünün nasıl tanımlanacağı.

Bu maddeler tasarım başlıklarıdır; henüz kesin bir paket formatı veya uygulama
seçimi yapılmamıştır. İlk protokol yalnızca handshake için gereken işlevleri
içermelidir.

### Kabul kriterleri

- Geçerli istek, tanımlanan içeriğe sahip ve doğru istekle eşleşen yanıtı üretir.
- Yanıt, önceden belirlenecek süre sınırı içinde alınır.
- ECU yanıt vermediğinde test timeout ile sonlanır; süresiz beklemez.
- Eksik, bozuk veya farklı bir isteğe ait yanıt başarı olarak değerlendirilmez.
- Hatalı paket sonrasında yeni bir geçerli handshake işlenebilir.
- Belirlenecek tekrar sayısı boyunca sonuçlar ve yanıt süreleri kaydedilir.
- Test kaydı; kullanılan ECU ve Discovery yazılım sürümlerini, UART ayarlarını,
  hedef profili ve adaptör revizyonunu, gönderilen/alınan mesajları, süreyi ve
  başarı/başarısızlık nedenini içerir.

Sayısal süre sınırı ve tekrar sayısı, ECU görev yapısı ve UART ayarları
netleştirildikten sonra belirlenecektir.

## Elektriksel bağlantı ön koşulları

Bağlantı yapılmadan önce ECU'nun UART pinleri, elektriksel seviyesi ve arayüz
tipi doğrulanmalıdır. TTL/CMOS UART, RS-232 ve RS-485 aynı elektriksel arayüz
olarak kabul edilmemelidir.

Discovery ve ECU şemalarından pin uygunluğu, gerilim uyumluluğu ve referans
toprak bağlantısı değerlendirilmelidir. Gerekli seviye dönüştürme, koruma veya
izolasyon devresi bu incelemeye göre seçilmelidir.

## Sonraki aşamalar ve HIL sınırı

UART handshake, hedef donanım üzerinde haberleşme doğrulamasının ilk adımıdır.
Fiziksel I/O uyaranı ve ölçüm içermediği için tek başına tam bir HIL doğrulaması
olarak değerlendirilmez.

İleride, seçilecek test gereksinimlerine göre şu kapsamlar değerlendirilebilir:

- PC / Python üzerinden otomatik test yürütme ve raporlama.
- Dijital, analog veya PWM girişlerinin uygun elektriksel devrelerle uyarılması.
- ECU fiziksel çıkışlarının ölçülmesi ve beklenen davranışla karşılaştırılması.
- Gereksinimlere bağlı hata senaryoları ve zamanlama testleri.
- Haberleşme kaybında test donanımının tanımlanmış güvenli duruma geçmesi.

Bu genişlemeler ilk handshake aşamasının teslim kapsamına dahil değildir.
Fiziksel ölçüm ile ECU'nun UART üzerinden bildirdiği iç durum ayrı kanıtlar
olarak kaydedilmelidir.

## Uygulama öncesi açık kararlar

1. ECU modeli, işlemcisi, yazılım altyapısı ve UART işleme noktası.
2. Her iki kartın UART birimi, pinleri ve elektriksel bağlantısı.
3. Baudrate, veri bitleri, parity ve stop bitleri.
4. Handshake paket formatı, bütünlük kontrolü ve yanıt içeriği.
5. Timeout, süre ölçüm noktaları ve test tekrar sayısı.
6. İlk aşama sonuçlarının hangi araç veya arayüz üzerinden kaydedileceği.
7. İlk hedef profilinin içeriği ve derleme/çalışma sırasında seçim yöntemi.

## İlerleme ve doğrulama planı

1. **Bağlantı ve protokol tasarımı:** Şemalar ve mesaj örnekleri üzerinden
   bağlantıyı ve istek–yanıt sözleşmesini gözden geçirmek.
2. **İki tarafın uygulaması:** UART alma/gönderme ve handshake işleme akışını
   gerçekleştirmek; normal ve hatalı paket davranışlarını doğrulamak.
3. **Gerçek donanım testi:** Discovery–ECU bağlantısında kabul kriterlerini
   çalıştırmak; UART trafiği ve ölçülen sürelerle sonuçları kaydetmek.

Kaynak kod incelemesi, derleme ve gerçek donanım testi ayrı doğrulama
seviyeleridir. Donanım üzerinde ölçüm yapılmadan fiziksel doğrulama tamamlandı
olarak raporlanmamalıdır.
