# Speeduino ECU Display using ESP32-C3 and ILI9488 3.5" TFT

## Summary

This project creates a digital dashboard display for Speeduino ECU using ESP32-C3 microcontroller and a 3.5 inch TFT LCD display with ILI9488 driver. The display shows real-time engine data from the Speeduino ECU including RPM, AFR, TPS, advance timing, MAP, and other sensor readings. The configuration has been optimized for ESP32-C3 pin mapping with reliable serial communication to the ECU.

## Build Status: ✅ SUCCESS

Project has been successfully compiled with the latest pin configuration. 

## Pin Configuration

### TFT Display Pin Mapping

Pin mapping used for ILI9488 TFT display:

```
TFT_MISO = 5   // SPI MISO
TFT_MOSI = 6   // SPI MOSI  
TFT_SCLK = 4   // SPI Clock
TFT_CS   = 7   // Chip Select
TFT_DC   = 8   // Data/Command
TFT_RST  = 10  // Reset
```

### Serial Communication to Speeduino ECU

Pin mapping used for UART communication to Speeduino ECU:

```
RX_PIN   = 20  // Serial RX (connect to Speeduino TX)
TX_PIN   = 21  // Serial TX (connect to Speeduino RX)
BAUD_RATE = 115200  // Communication speed
```

**Note:** Uses Serial1 (hardware UART) for reliable communication with Speeduino ECU.

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

## Wiring Connections

### ILI9488 Display to ESP32-C3

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

### Speeduino ECU to ESP32-C3

Connect the Speeduino ECU serial pins to ESP32-C3:

```
Speeduino ECU     ESP32-C3
-------------     --------
TX (Serial Out)   GPIO 20 (RX)
RX (Serial In)    GPIO 21 (TX)
GND               GND
```

**Note:** 
- Backlight (LED/BL) is connected directly to 3.3V to simplify wiring and ensure the display is always on. No backlight control pin is used in this implementation.
- Serial communication uses 115200 baud rate, 8 data bits, no parity, 1 stop bit (8N1)
- Make sure to connect GND between all devices for proper communication

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

## References

This project uses and references the following resources:

- **[Speeduino ECU](https://speeduino.com/home/)** - Open source engine management system
- **[TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)** - Arduino TFT graphics library for ESP32/ESP8266
- **[Image2CPP Tool](https://github.com/amrikarisma/image2cpp)** - Online tool for converting images to C/C++ arrays for embedded displays

The project is now ready to use with the ILI9488 3.5 inch display!

## Support & Documentation

### 🌐 **Official Website**
For complete documentation, tutorials, and support visit:
**[www.mazduino.com](https://www.mazduino.com)**

### 📚 **What You'll Find:**
- **Setup Guides** - Complete installation and configuration tutorials
- **Troubleshooting** - Common issues and solutions
- **Feature Documentation** - Detailed feature explanations
- **Community Forum** - Get help from other users
- **Contact Support** - Direct support channels
- **Latest Updates** - Firmware updates and new features

### 💬 **Need Help?**
- 🔧 **Technical Issues** - Visit our troubleshooting section
- 💡 **Feature Requests** - Submit ideas for new features  
- 🐛 **Bug Reports** - Report issues for quick resolution
- 📖 **Documentation** - Access complete user guides
- 🤝 **Community** - Connect with other Mazduino users

### 📧 **Contact Information**
Visit [www.mazduino.com](https://www.mazduino.com) for:
- Direct contact forms
- Support ticket system
- Community forums
- Live chat support
- Email support

---

**© 2025 Mazduino Project | Visit [www.mazduino.com](https://www.mazduino.com) for support**
