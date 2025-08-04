# TFT_eSPI Configuration for ILI9488 3.5" TFT with ESP32-C3

## Summary

Saya telah berhasil mengonfigurasi library TFT_eSPI untuk display TFT LCD 3.5 inch dengan driver ILI9488 pada microcontroller ESP32-C3. Konfigurasi ini telah disesuaikan dengan pin mapping yang dioptimalkan untuk ESP32-C3 dan backlight yang disambungkan langsung ke power supply.

## Status Build: ✅ BERHASIL

Project telah berhasil dikompilasi dengan konfigurasi pin terbaru. 

## Konfigurasi Pin

Pin mapping yang digunakan dalam konfigurasi:

```
TFT_MISO = 5   // SPI MISO
TFT_MOSI = 6   // SPI MOSI  
TFT_SCLK = 4   // SPI Clock
TFT_CS   = 7   // Chip Select
TFT_DC   = 8   // Data/Command
TFT_RST  = 10  // Reset
TFT_BL   = 0   // Backlight Control (tidak digunakan - langsung ke 3.3V)
```

## Yang Telah Dikonfigurasi

1. **Library TFT_eSPI v2.5.43** - Ditambahkan ke dependencies dalam `platformio.ini`

2. **Driver ILI9488** - Dikonfigurasi untuk display 3.5 inch dengan resolusi 320x480

3. **Pin Configuration** - Pin mapping disesuaikan untuk ESP32-C3

4. **Build Flags** - Semua definisi yang diperlukan ditambahkan ke `platformio.ini`:
   - `ILI9488_DRIVER=1`
   - `TFT_WIDTH=320` dan `TFT_HEIGHT=480`
   - Pin definitions (MISO=5, MOSI=6, SCLK=4, CS=7, DC=8, RST=10, BL=0)
   - Font loading configurations
   - SPI frequency settings

5. **Font System** - Diubah dari custom fonts ke built-in TFT_eSPI fonts:
   - Font 2 (16px height) untuk teks kecil
   - Font 4 (26px height) untuk teks besar

6. **Function Declarations** - Ditambahkan deklarasi fungsi yang diperlukan

## File yang Dimodifikasi

1. **`platformio.ini`** - Ditambahkan dependency dan build flags
2. **`src/main.cpp`** - Fixed font loading dan function declarations
3. **`include/User_Setup.h`** - Konfigurasi TFT_eSPI (tidak digunakan dalam build akhir)

## Wiring Display ILI9488 ke ESP32-C3

Hubungkan display ILI9488 ke ESP32-C3 sesuai dengan pin mapping di atas:

```
ILI9488 Display    ESP32-C3
---------------    --------
VCC               3.3V
GND               GND
CS                GPIO 7
RESET/RST         GPIO 10
DC/RS             GPIO 8
SDI/MOSI          GPIO 6
SCK/CLK           GPIO 4
LED/BL            3.3V (langsung tanpa kontrol)
SDO/MISO          GPIO 5
```

**Catatan:** Backlight (LED/BL) disambungkan langsung ke 3.3V untuk menyederhanakan wiring dan memastikan display selalu menyala. Pin GPIO 0 yang didefinisikan sebagai TFT_BL tidak digunakan dalam implementasi ini.

## Langkah-langkah Build

1. Install dependencies:
   ```bash
   python3 -m platformio lib install
   ```

2. Build project:
   ```bash
   python3 -m platformio run
   ```

3. Upload ke ESP32-C3:
   ```bash
   python3 -m platformio run --target upload
   ```

## Catatan Penting

1. **DMA Warning** - ESP32-C3 belum mendukung DMA untuk TFT_eSPI, tapi library tetap berfungsi tanpa DMA.

2. **Touch Screen** - Jika display memiliki touch screen, uncomment `TOUCH_CS` pin di konfigurasi.

3. **SPI Frequency** - Dikonfigurasi pada 27MHz untuk performa optimal dengan ILI9488.

4. **Backlight Connection** - LED/BL display disambungkan langsung ke 3.3V untuk penyederhanaan wiring. Display akan selalu menyala tanpa kontrol brightness dinamis.

5. **Pin GPIO 0** - Meskipun didefinisikan sebagai TFT_BL, pin ini tidak digunakan karena backlight disambungkan langsung ke power supply.

## Testing

Setelah upload firmware, display akan menampilkan:
- Splash screen dengan informasi firmware
- Interface speedometer dengan berbagai data sensor
- Tombol status dan indikator

Project sekarang sudah siap untuk digunakan dengan display ILI9488 3.5 inch!
