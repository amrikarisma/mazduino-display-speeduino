// User_Setup.h for ILI9488 3.5" TFT with ESP32-C3

#define USER_SETUP_ID 999

// Driver for ILI9488
#define ILI9488_DRIVER

// Define TFT display resolution
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// ESP32-C3 pin configuration (adjust according to your wiring)
#define TFT_MISO 2  // MISO
#define TFT_MOSI 3  // MOSI
#define TFT_SCLK 4  // SCLK
#define TFT_CS   5  // Chip select control pin
#define TFT_DC   6  // Data Command control pin
#define TFT_RST  7  // Reset pin (could connect to RST pin)
#define TFT_BL   8  // Backlight control pin

// Optional: Uncomment if you want to use touch
// #define TOUCH_CS 9

// Font loading
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel high font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel high font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel high font needs ~3256 bytes in FLASH, only characters 1234567890:-.

#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// Comment out the #define below to stop the SPIFFS filing system and smooth font code being loaded
// this will save ~20kbytes of FLASH
#define SMOOTH_FONT

// SPI frequency for ILI9488 (can be up to 27MHz for reading and 80MHz for writing)
#define SPI_FREQUENCY  27000000  // 27MHz for display writes
#define SPI_READ_FREQUENCY  20000000  // 20MHz for reading (for some displays)
#define SPI_TOUCH_FREQUENCY  2500000  // 2.5MHz for touch (if using touch)

// Optional reduced SPI frequency for troubleshooting
// #define SPI_FREQUENCY  20000000
// #define SPI_READ_FREQUENCY  20000000

// Comment out the line below if "SPI Transactions" do not need to be
// supported. This will save ~700 bytes of code and RAM.
#define SUPPORT_TRANSACTIONS
