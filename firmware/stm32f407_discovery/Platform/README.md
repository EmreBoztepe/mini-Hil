# Platform

Discovery kartının donanım erişimi bu katmanda yer alır.

- USB CDC ve ileride UART/I/O işlemleri için aktarım tamponlarını yönetir.
- Zaman okuma ve donanım hata bilgilerini sağlar.
- STM32 HAL ile uygulama kodu arasındaki erişim sınırını oluşturur.

Hedef mesajlarının anlamını veya test beklentilerini bilmez. `App` ve
`Targets` katmanlarını çağırmaz. UART'tan gelen baytların handshake yanıtı
olup olmadığına bu katmanda karar verilmez.

Clock, pinmux ve çevre birimi başlangıcı `.ioc` üzerinden üretilen `Core`
ve `USB_DEVICE` kodunda kalır. UART/I/O sürücüleri henüz uygulanmamıştır.

## USB CDC

`hil_usb_cdc.h` uygulamanın kullanacağı arayüzdür. HAL/USB sınıf tiplerini
dışarı açmaz. `Init`, `MX_USB_DEVICE_Init` öncesinde çağrılır; diğer public
fonksiyonlar ana döngü içindir. `Process` her turda çağrılmalıdır.

- `IsConfigured`: USB yapılandırılmış mı? PC uygulamasının portu açtığını
  veya veriyi aldığını göstermez; suspend sırasında false döner.
- `Read`: eldeki paketten kapasite kadar bayt kopyalar. Veri yoksa sıfır döner.
  Kısmi okuma desteklenir. Paket tamamen okununca `Process` yeni alımı açar.
- `CanWrite` / `Write`: tek bir aktarım kabul edilir; 1–64 bayt özel TX
  tamponuna kopyalanır. Çağıranın tamponu dönüşten sonra yeniden kullanılabilir.
  `OK` kabul edildi demektir; alıcı uygulamanın onayı değildir.
- `BUSY` halinde çağıran veriyi saklayıp daha sonra tekrar denemelidir.
  `NOT_READY` yapılandırılmamış bağlantıyı, `INVALID_ARGUMENT` geçersiz
  parametreyi, `IO_ERROR` alt arayüzün bildirdiği hatayı gösterir.

Bir adet 64 bayt RX ve bir adet 64 bayt TX tamponu kullanılır. RX okunmadan
OUT endpoint yeniden kurulmaz; USB NAK ile akış kontrolü uygular. Böylece
yavaş tüketici, okunmamış Platform verisinin üzerine yazılmasına yol açmaz.
Bu tercih yüksek throughput için optimize edilmemiştir. TX tamamlanınca
tampon serbest bırakılır. Reset/deconfiguration/disconnect callback'i eldeki
veriyi ve aktarım durumunu temizler; yeniden bağlantı yeni bir oturumdur.
COM portunu kapatıp açmak tek başına USB deconfiguration anlamına gelmez.

`hil_usb_cdc_port.h`, yalnızca `USB_DEVICE/App/usbd_cdc_if.c` içindeki
Init/DeInit/Receive/TransmitCplt callback bağlantıları içindir. Kesmede
senaryo veya protokol işlenmez. Paylaşılan durum ve en fazla 64 baytlık
kopyalar, önceki PRIMASK değerini koruyan kısa kritik bölgelerle korunur.
Dinamik bellek ve aktarım tamamlanana kadar bekleyen döngü kullanılmaz.

CDC bir bayt akışıdır; 64 bayt USB paket sınırı uygulama mesaj sınırı değildir.
Mesaj çerçeveleme ve komutlar ileride üst katmanda tanımlanacaktır.
Bağlantı ve test adımları: [USB CDC doğrulama](../../../docs/USB_CDC_DOGRULAMA.md).
