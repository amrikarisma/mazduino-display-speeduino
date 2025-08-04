// Simple TFT test for ILI9488
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  
  // Initialize the display
  tft.init();
  tft.setRotation(3); // Landscape mode
  
  // Test basic display functionality
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  
  tft.drawString("TFT_eSPI Test", 10, 10);
  tft.drawString("ILI9488 Driver", 10, 40);
  tft.drawString("ESP32-C3", 10, 70);
  
  // Draw some test shapes
  tft.drawRect(10, 100, 100, 50, TFT_RED);
  tft.fillRect(120, 100, 100, 50, TFT_GREEN);
  tft.drawCircle(270, 125, 25, TFT_BLUE);
  
  Serial.println("TFT initialized and test pattern displayed");
}

void loop() {
  // Test color cycling
  static unsigned long lastUpdate = 0;
  static uint16_t color = TFT_RED;
  
  if (millis() - lastUpdate > 1000) {
    tft.fillCircle(400, 125, 20, color);
    
    // Cycle through colors
    if (color == TFT_RED) color = TFT_GREEN;
    else if (color == TFT_GREEN) color = TFT_BLUE;
    else if (color == TFT_BLUE) color = TFT_YELLOW;
    else if (color == TFT_YELLOW) color = TFT_MAGENTA;
    else if (color == TFT_MAGENTA) color = TFT_CYAN;
    else color = TFT_RED;
    
    lastUpdate = millis();
  }
}
