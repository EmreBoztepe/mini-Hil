# Platform

Discovery kartının donanım erişimi bu katmanda yer alır.

- UART/I/O işlemleri ve aktarım tamponlarını yönetir.
- Zaman okuma ve donanım hata bilgilerini sağlar.
- STM32 HAL ile uygulama kodu arasındaki erişim sınırını oluşturur.

Hedef mesajlarının anlamını veya test beklentilerini bilmez. `App` ve
`Targets` katmanlarını çağırmaz. UART'tan gelen baytların handshake yanıtı
olup olmadığına bu katmanda karar verilmez.

Clock, pinmux ve çevre birimi başlangıcı `.ioc` üzerinden üretilen `Core`
kodunda kalır. Gerekli callback bağlantıları uygulama aşamasında tanımlanacaktır.

Bu dizinde henüz özel UART/I/O sürücüsü uygulanmamıştır.
