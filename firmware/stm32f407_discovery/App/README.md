# App

STM32 üzerinde çalışan test senaryoları ve bunların ortak yürütücüsü bu
katmanda yer alır. Kararlaştırılan yerleşim:

```text
App/
├── Generated/    Şablonlardan üretilen senaryo .c/.h dosyaları
└── Runtime/      Elle geliştirilen ortak yürütücü ve adım tipleri
```

`Generated`, PC'deki senaryo tanımından `templates/scenario.c.in` ve
`templates/scenario.h.in` kullanılarak üretilecektir. Adımların sayısı, sırası,
işlem türleri, parametreleri, beklenen verileri ve süreleri `const` tablolarda
tutulur. Header dosyaları senaryoların dışarıya sunulan bildirimlerini içerir.
Üretilen kaynaklara elle müdahale edilmez; senaryo değişikliği kaynak tanımında
yapılır, ardından dosyalar yeniden üretilir, derlenir ve STM32'ye yüklenir.

`Runtime`:

- Ortak adım tiplerini tanımlar ve üretilen tabloları yürütür.
- `Targets` üzerinden istek paketini oluşturur ve yanıtı ayrıştırır.
- `Platform` üzerinden aktarımı ve zaman okumayı gerçekleştirir.
- Yanıtı ve süreyi değerlendirerek testin PASS/FAIL kararını verir.

Her senaryo için ayrı C durum makinesi üretilmez. Desteklenen işlemlerle
senaryo değişikliği runtime değişikliği gerektirmez; yeni bir işlem yeteneği
gerektiğinde runtime ve generator doğrulaması birlikte geliştirilir.

Doğrudan HAL veya register erişimi içermez. Hedefe özgü paket formatlarını
buraya gömmek yerine `Targets` kullanılır. Farklı bir hedefin test adımları
değişiyorsa senaryonun değişmesi normaldir.

İlk planlanan senaryo UART handshake'tir. `Generated` ve `Runtime` alt
dizinleri, C/H şablonları ve generator henüz uygulanmamıştır. İlk uygulama
adımı, ortak adım veri yapısını ve senaryo arayüzünü tanımlamaktır.

Ayrıntılar: [STM32 yazılım mimarisi](../../../docs/STM32_YAZILIM_MIMARISI.md).
