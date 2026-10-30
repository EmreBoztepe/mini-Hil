# mini-Hil
STM32F407-based Hardware-in-the-Loop (HIL) test platform for automated embedded SW validation, featuring UART communication, configurable I/O simulation, and a Python GUI.

Aktif geliştirme kapsamı STM32F407 Discovery firmware'idir. Kararlaştırılan
mimaride test senaryoları PC aracında hazırlanacak, sabit C/H şablonlarından
`App/Generated` altında senaryo tabloları üretilecek ve STM32 üzerindeki ortak
`App/Runtime` tarafından yürütülecektir. `Targets` hedef tanımlarını ve protokolü,
`Platform` donanım erişimini içerir. Bu üretim altyapısı henüz uygulanmamıştır.

- [Proje kapsamı](docs/PROJE_KAPSAMI.md)
- [STM32 yazılım mimarisi ve senaryo üretim kararı](docs/STM32_YAZILIM_MIMARISI.md)
- [PC–Discovery USB CDC bağlantısı ve doğrulama](docs/USB_CDC_DOGRULAMA.md)

İlk Platform uygulaması USB OTG FS Device / CDC haberleşmesidir. Mevcut
firmware, CN5 üzerinden PC'den gelen ikili veriyi geri gönderir. ECU UART
bağlantısı ve senaryo yürütücüsü sonraki adımlardır.
