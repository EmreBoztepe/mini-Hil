# PC–Discovery USB CDC doğrulaması

## Amaç ve veri yolu

Bu aşama, STM32F407 Discovery'nin PC ile USB üzerinden ikili veri taşımasını
doğrular. Firmware, gelen baytları ana döngüde aynen geri gönderir. ECU testi,
komut protokolü, senaryo generator'ı ve runtime henüz uygulanmamıştır.

```text
PC / pyserial (COM portu)
        ↕ USB kablosu
CN5 → USB OTG FS → HAL PCD → ST USB Device / CDC
        ↕ usbd_cdc_if.c callback'leri
Platform / hil_usb_cdc
        ↕ Read / Write
Core / main.c: ham binary echo
```

PC USB Host, Discovery USB Device'dır. CDC-ACM PC'de sanal seri port olarak
görünür. Baudrate alanı USB veri hızını belirlemez; firmware SET/GET_LINE_CODING
bilgisini saklar ve geri verir. Gerçek UART bu yolda kullanılmaz.

## CubeMX ve proje ayarları

- Kart: STM32F407G-DISC1, MCU: STM32F407VGT6.
- USB_OTG_FS: Device Only, dahili FS PHY, VBUS sensing açık.
- USB_DEVICE: CDC / Virtual Port Com.
- PA9: VBUS, PA11: D−, PA12: D+; OTG_FS kesmesi açık.
- HSE 8 MHz; PLL M=8, N=336, P=2, Q=7. SYSCLK 168 MHz, USB 48 MHz.
- PC0 / OTG_FS_PowerSwitchOn yüksek: Device rolünde kartın USB host güç
  anahtarı kapalı tutulur.
- Debug ve Release yapılandırmalarına `Platform` kaynak dizini ve
  `../Platform` C include yolu eklenmiştir.

CubeMX kaynakları `.ioc` üzerinden üretilir. Özel callback bağlantıları
`usbd_cdc_if.c`, echo bağlantısı `main.c` içindeki `USER CODE` alanlarındadır.
Yeniden Generate Code sonrasında bu alanların ve özel Platform derleme
ayarlarının korunduğu kontrol edilmelidir. ST middleware/HAL kaynakları elle
değiştirilmez.

## Karta yükleme ve bağlantı

1. CubeIDE'de `hilSoftware` projesini Refresh ve Build et.
2. CN1 ST-LINK bağlantısıyla firmware'i karta yükle ve çalıştır; debugger
   kullanılıyorsa işlemciyi Resume et.
3. PC'nin USB portunu veri taşıyan bir Micro-USB kablosuyla kartın **CN5
   USB OTG FS** konnektörüne bağla. CN1 debug/programlama içindir; bu CDC
   uygulamasının veri bağlantısı CN5'tir.
4. Aygıt Yöneticisi'nden yeni oluşan Discovery CDC COM portunu belirle.
   Aynı portu kullanan terminal/diğer uygulamaları kapat.

## PC testi

Repo kökünde:

```powershell
python -m pip install -r tools/requirements.txt
python -m serial.tools.list_ports
python tools/usb_cdc_echo_test.py COM5
```

`COM5` yerine kartın gerçek portunu yaz. Gerekirse `--timeout 5` ile her
aktarımın toplam yazma/okuma süresi sınırı değiştirilebilir; varsayılan 3 s.

Araç 1, 63, 64, 65, 127, 128, 257 ve 1024 baytlık ikili veri gönderir;
alınan akışı beklenen uzunluğa kadar toplar ve bayt bayt karşılaştırır.
Sıfır baytı, 64 bayt paket sınırı ve birden fazla USB paketine yayılan
aktarımlar kapsanır. Her boyut için `PASS` ve sonunda
`USB CDC binary echo passed.` görülmelidir. Timeout, eksik aktarım veya
veri uyuşmazlığı başarısızlık olarak programı sonlandırır.

Ana döngüde alt arayüzün bildirdiği RX yeniden kurma veya TX kabul hatası
olursa kırmızı LD5 yanar ve reset'e kadar açık kalır. Bu LED PC testi
yerine geçmez. USB yapılandırılmamışken veri gönderilmez. PC portu açılmış
mı veya uygulama veriyi aldı mı bilgisi `IsConfigured` ile ölçülmez.

CN1 ile kartın gücü korunurken CN5'i çıkarıp yeniden bağla; yeni COM port
numarası varsa onu kullanarak testi yeniden çalıştır. Beklenen davranış,
eski bağlantının bekleyen Platform verisini temizleyip tekrar echo verebilmesidir.
USB reset/deconfiguration sırasında da tamponlar temizlenir. Yalnız COM
portunu kapatıp açmak aynı olayı oluşturmayabilir.

## Yazılım kontrolleri ve kanıt sınırı

```powershell
python tests/host/run_tests.py
```

Host testleri gerçek `hil_usb_cdc.c` dosyasını USB/CMSIS mock'larıyla derler.
Çalıştırmak için PATH üzerinde C11 destekli `gcc` gerekir; farklı bir derleyici
yolu `CC` ortam değişkeniyle verilebilir.
Kısmi RX okuma, yeniden alımın tüketim sonrasına bırakılması, sıfır uzunluklu
paket, hata dönüşü/tekrar deneme, TX tampon sahipliği, busy, bağlantı durumunun
temizlenmesi ve PRIMASK'ın korunmasını kontrol eder. Mock testleri gerçek
USB kesmesini veya kablo bağlantısını kanıtlamaz.

Debug/Release hedef derlemesi kaynakların birlikte derlendiğini ve bağlandığını
gösterir. USB enumeration, gerçek aktarım, 64 bayt sonrası ZLP ve yeniden
bağlantı davranışı, yukarıdaki fiziksel testle ayrıca doğrulanmalıdır.

30 Ekim 2026 yazılım doğrulaması: host testleri ve ARM GCC 12.3 ile Debug /
Release derlemeleri başarılı, derlemelerde uyarı yok. Karta yükleme ve fiziksel
USB echo / çıkarıp takma testi henüz yapılmadı.
