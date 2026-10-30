# Targets

STM32'nin test edilen ECU / PCB'yi tanımak için kullandığı tanımlar ve protokol
kodu bu katmanda yer alır. Buradaki kod STM32'de çalışır; ECU firmware'i değildir.

- Hedef kimliğini, haberleşme ayarlarını ve bağlantı bilgilerini tanımlar.
- İstek paketlerini oluşturur ve gelen baytları ayrıştırır.
- Paket bütünlüğünü ve protokol geçerliliğini değerlendirir.
- Çözülen yanıt içeriğini `App` katmanına sunar.

Doğrudan UART/HAL erişimi yapmaz ve testin PASS/FAIL kararını vermez.
`Platform` katmanını çağırmaz; veri akışını `App` yönetir.

İlk gerçek hedef henüz tanımlı değildir. Hedefe özgü kaynaklar hedef
netleştirildiğinde bu dizinin altında ayrı bir klasörde oluşturulacaktır.
