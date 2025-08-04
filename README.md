# TFT_eSPI Configuration for ILI9488 3.5" TFT with ESP32-C3

## Summary

I have successfully configured the TFT_eSPI library for a 3.5 inch TFT LCD display with ILI9488 driver on ESP32-C3 microcontroller. This configuration has been optimized for ESP32-C3 pin mapping with backlight connected directly to power supply.

## Build Status: ✅ SUCCESS

Project has been successfully compiled with the latest pin configuration. 

## Pin Configuration

Pin mapping used in this configuration:

```
TFT_MISO = 5   // SPI MISO
TFT_MOSI = 6   // SPI MOSI  
TFT_SCLK = 4   // SPI Clock
TFT_CS   = 7   // Chip Select
TFT_DC   = 8   // Data/Command
TFT_RST  = 10  // Reset
```

## What Has Been Configured

1. **TFT_eSPI Library v2.5.43** - Added to dependencies in `platformio.ini`

2. **ILI9488 Driver** - Configured for 3.5 inch display with 320x480 resolution

3. **Pin Configuration** - Pin mapping optimized for ESP32-C3

4. **Build Flags** - All required definitions added to `platformio.ini`:
   - `ILI9488_DRIVER=1`
   - Pin definitions (MISO=5, MOSI=6, SCLK=4, CS=7, DC=8, RST=10)
   - Font loading configurations
   - SPI frequency settings (20MHz for stable operation)

5. **Font System** - Changed from custom fonts to built-in TFT_eSPI fonts:
   - Font 2 (16px height) for small text
   - Font 4 (26px height) for large text

6. **Function Declarations** - Added required function declarations

## Modified Files

1. **`platformio.ini`** - Added dependencies and build flags
2. **`src/main.cpp`** - Fixed font loading and function declarations

## Wiring ILI9488 Display to ESP32-C3

Connect the ILI9488 display to ESP32-C3 according to the pin mapping above:

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
LED/BL            3.3V (direct connection without control)
SDO/MISO          GPIO 5
```

**Note:** Backlight (LED/BL) is connected directly to 3.3V to simplify wiring and ensure the display is always on. No backlight control pin is used in this implementation.

## Build Steps

1. Install dependencies:
   ```bash
   python3 -m platformio lib install
   ```

2. Build project:
   ```bash
   python3 -m platformio run
   ```

3. Upload to ESP32-C3:
   ```bash
   python3 -m platformio run --target upload
   ```

## Important Notes

1. **DMA Warning** - ESP32-C3 does not yet support DMA for TFT_eSPI, but the library works without DMA.

2. **Touch Screen** - If the display has touch screen capability, uncomment the `TOUCH_CS` pin in configuration.

3. **SPI Frequency** - Configured at 20MHz for optimal performance with ILI9488 and ESP32-C3 stability.

4. **Backlight Connection** - Display LED/BL is connected directly to 3.3V for simplified wiring. Display will always be on without dynamic brightness control.

5. **ESP32-C3 Stability** - Specific platform version (6.5.0) and build flags are used for stable operation on ESP32-C3.

## Testing

After uploading firmware, the display will show:
- Splash screen with firmware information
- Speedometer interface with various sensor data
- Status buttons and indicators

The project is now ready to use with the ILI9488 3.5 inch display!
