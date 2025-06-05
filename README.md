# Salata Ustaları Problemi

git pull --rebase origin main
## Derleme

```bash
gcc salata -o salata.c
```

## Çalıştırma

Programı çalıştırmak için bir malzeme çiftleri dosyası gereklidir. Örnek kullanım:

```bash
./salata -i girdi.txt
```

## Komut Satırı Argümanları

- `-i <dosyaYolu>`: Malzeme çiftlerinin bulunduğu dosyanın yolu.

## Girdi Dosyası Formatı

Her satırda iki karakterden oluşan malzeme çifti bulunmalıdır. Örneğin:

```
YT
YS
TL
SL
```

Malzeme harfleri:
- Y: Yağ
- T: Tuz
- L: Limon
- S: Sebze

## Program Akışı

- Toptancı, dosyadan malzeme çiftlerini okur ve uygun ustaya gönderir.
- Usta, sahip olmadığı iki malzeme ile salata yapar ve toptancıya teslim eder.
- Tüm senkronizasyon işlemleri semaforlar ve mutex ile sağlanır.
- Program sonunda tüm kaynaklar (semafor, mutex, dinamik bellek) serbest bırakılır.

## Kurallar ve Özellikler

- Her usta aynı fonksiyonu kullanarak çalışır.
- Eksik veya hatalı argümanda kullanım bilgisi gösterilir.
- Hatalı dosya açma veya diğer hatalarda anlamlı hata mesajı verilir.
- Bellek ve kaynak yönetimi kurallara uygundur.

## Analiz

Bellek sızıntısı ve zombi süreçler için analiz araçları (ör. `valgrind`) ile kontrol edilmelidir.
