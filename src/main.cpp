#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <Update.h>
#include <EEPROM.h>
#include <cstring>
#include "Arduino.h"
#include "SPI.h"
#include <TFT_eSPI.h>
#include "NotoSansBold15.h"
#include "NotoSansBold36.h"
#include "splash/splash_manager.h"

#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36

// Forward declarations
void handleRoot();
void handleUpdate();
void handleToggle();
void handleSplash();
void handleDisplayMode();
void handleInfo();
void drawSplashScreenWithImage();
void startUpDisplay();
void drawData();
void drawDataBox(int x, int y, const char* label, const int value, uint16_t labelColor, const int valueToCompare, const int decimal, bool setup);
void itemDraw(bool setup);
void forceRedrawFPSLabel();

const char* version = "0.1.1";

const char* ssid = "MAZDUINO_Display";
const char* password = "12345678";
IPAddress ip(192, 168, 1, 80);
IPAddress netmask(255, 255, 255, 0);

WebServer server(80);

TFT_eSPI display = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&display);

#include "Comms.h"
#include "text_utils.h"
#include "drawing_utils.h"

#define UART_BAUD 115200
// ESP32-C3 Supermini
#define RXD 20
#define TXD 21
// ESP32-DEVKITC-V4
// #define RXD 16
// #define TXD 17

#define EEPROM_SIZE 512

boolean sent = false;
boolean received = true;

bool wifiActive = true;
uint32_t lastClientCheck = 0;
uint32_t lastClientCheckTimeout = 0;
uint32_t wifiTimeout = 30000;
bool clientConnected = true;

uint8_t iat = 0, clt = 0;
uint8_t refreshRate = 0;
unsigned int rpm = 6000, lastRpm;
int mapData, tps, adv, fp;
float bat = 0.0, afrConv = 0.0;
bool syncStatus, fan, ase, wue, rev, launch, airCon, dfco;

int lastIat = -1, lastClt = -1, lastTps = -1, lastAdv = -1, lastMapData = -1, lastFp = -1;
float lastBat = -1, lastAfrConv = -1;
unsigned int lastRefreshRate = -1;

uint32_t startupTime;
uint32_t lazyUpdateTime;
uint16_t spr_width = 0;




void drawData() {
  if (lastRpm != rpm) {
    drawRPMBarBlocks(rpm);
    spr.loadFont(AA_FONT_LARGE);
    spr.createSprite(100, 50);
    spr_width = spr.textWidth("7777");  // 7 is widest numeral in this font
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.setTextDatum(TR_DATUM);
    spr.drawNumber(rpm, 100, 5);
    spr.pushSprite(190, 140);
    spr.deleteSprite();
    
    // Redraw RPM label to ensure it's always visible
    display.loadFont(AA_FONT_SMALL);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextDatum(TC_DATUM);
    display.drawString("RPM", 240, 120);
    
    lastRpm = rpm;
  }
  itemDraw(false);
}

void drawSplashScreenWithImage() {
  // Draw current selected splash screen dynamically
  drawCurrentSplash(display, 0, 0, TFT_WHITE, TFT_BLACK);
  
  // Add splash screen name and version info at the bottom
  display.loadFont(AA_FONT_SMALL);
  display.setTextColor(TFT_YELLOW, TFT_BLACK);
  display.setTextDatum(BL_DATUM);
  display.drawString(getSplashScreenName(), 5, display.height() - 5);
  
  display.setTextDatum(BR_DATUM);
  display.drawString("v" + String(version), display.width() - 5, display.height() - 5);
  
  // Add website info at the top
  display.setTextColor(TFT_CYAN, TFT_BLACK);
  display.setTextDatum(TC_DATUM);
  display.drawString("www.mazduino.com", display.width() / 2, 5);
  
  delay(5000);
}

void handleUpdate() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    // Serial.printf("Memulai update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      // Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      // Serial.printf("Update selesai: %u bytes\n", upload.totalSize);
    } else {
      // Update.printError(Serial);
    }
  }
}



void drawDataBox(int x, int y, const char* label, const int value, uint16_t labelColor, const int valueToCompare, const int decimal, bool setup) {
  const int BOX_WIDTH = 100;  // Reduced width to fit screen
  const int BOX_HEIGHT = 80;  // Adjusted height
  const int LABEL_HEIGHT = BOX_HEIGHT / 2;

  if (setup) {
    spr.loadFont(AA_FONT_SMALL);
    spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
    spr.setTextColor(labelColor, TFT_BLACK, true);
    spr.drawString(label, 50, 5);
    spr.setTextDatum(TC_DATUM);
    if (label == "AFR") {
      spr.pushSprite(x - 10, y);
    } else {
      spr.pushSprite(x, y);
    }
  }
  if (valueToCompare != value) {
    spr.loadFont(AA_FONT_LARGE);
    spr.createSprite(BOX_WIDTH, LABEL_HEIGHT);
    spr.setTextDatum(TC_DATUM);
    spr_width = spr.textWidth("333");
    spr.setTextColor(labelColor, TFT_BLACK, true);
    if (decimal > 0) {
      spr.drawFloat((value / 10.0), decimal, 50, 5);
    } else {
      spr.drawNumber(value, 50, 5);
    }
    spr.pushSprite(x, y + LABEL_HEIGHT - 15);
    spr.deleteSprite();
  }
}

void itemDraw(bool setup) {
  const char* labels[] = { "AFR", "TPS", "ADV", "MAP" };
  int values[] = { afrConv, tps, adv, mapData };
  int lastValues[] = { lastAfrConv, lastTps, lastAdv, lastMapData };
  int positions[][2] = { { 5, 190 }, { 360, 190 }, { 120, 190 }, { 360, 10 } };
  uint16_t colors[] = { (afrConv < 130) ? TFT_ORANGE : ((afrConv > 147) ? TFT_RED : TFT_GREEN), TFT_WHITE, TFT_RED, TFT_WHITE };

  for (int v = 0; v < 4; v++) {
    drawDataBox(positions[v][0], positions[v][1], labels[v], values[v], colors[v], lastValues[v], (v == 0) ? 1 : 0, setup);
    lastValues[v] = values[v];
  }

  if ((millis() - lazyUpdateTime > 1000) || setup) {
    const char* labelsLazy[4] = { "IAT", "Coolant", "Voltage", (EEPROM.read(0) == 1) ? "FPS" : "FP" };
    int valuesLazy[4] = { iat, clt, static_cast<int>(bat), (EEPROM.read(0) == 1) ? refreshRate : fp };
    int lastValuesLazy[4] = { lastIat, lastClt, static_cast<int>(lastBat), (EEPROM.read(0) == 1) ? lastRefreshRate : lastFp };

    int positionsLazy[][2] = { { 5, 10 }, { 5, 100 }, { 360, 100 }, { 240, 190 } };
    uint16_t colorsLazy[] = { TFT_WHITE, (clt > 95) ? TFT_RED : TFT_WHITE,
                              ((bat < 115 || bat > 145) ? TFT_ORANGE : TFT_GREEN), TFT_WHITE };

    for (int l = 0; l < 4; l++) {
      drawDataBox(positionsLazy[l][0], positionsLazy[l][1], labelsLazy[l], valuesLazy[l], colorsLazy[l], lastValuesLazy[l], (l == 2) ? 1 : 0, setup);
      lastValuesLazy[l] = valuesLazy[l];
    }

    lazyUpdateTime = millis();
  }
  // Center buttons
  display.loadFont(AA_FONT_SMALL);
  const char* buttonLabels[] = { "SYNC", "FAN", "ASE", "WUE", "REV", "LCH", "AC", "DFCO" };
  bool buttonStates[] = { syncStatus, fan, ase, wue, rev, launch, airCon, dfco };
  for (int i = 0; i < 8; i++) {
    drawSmallButton((10 + 60 * i), 285, buttonLabels[i], buttonStates[i]);
  }
}

void forceRedrawFPSLabel() {
  // Force redraw just the FPS/FP label and value (position [3] in lazy array)
  const char* label = (EEPROM.read(0) == 1) ? "FPS" : "FP";
  int value = (EEPROM.read(0) == 1) ? refreshRate : fp;
  
  // Clear the label area first
  display.fillRect(240, 190, 100, 80, TFT_BLACK);
  
  // Redraw the label and value
  drawDataBox(240, 190, label, value, TFT_WHITE, -1, 0, true);  // Force setup=true to redraw label
}

void startUpDisplay() {
  display.fillScreen(TFT_BLACK);
  display.loadFont(AA_FONT_SMALL);
  spr.setColorDepth(16);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.setTextDatum(TC_DATUM);  // Set text datum to top center
  display.drawString("RPM", 240, 120);  // Center the label properly
  itemDraw(true);
  spr.loadFont(AA_FONT_LARGE);
  for (int i = rpm; i >= 0; i -= 250) {
    drawRPMBarBlocks(i);
    spr.createSprite(100, 50);
    spr_width = spr.textWidth("7777");  // 7 is widest numeral in this font
    spr.setTextColor(TFT_WHITE, TFT_BLACK, true);
    spr.setTextDatum(TR_DATUM);
    spr.drawNumber(i, 100, 5);
    spr.pushSprite(190, 140);
    spr.deleteSprite();
  }
}
const char* uploadPage PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>MAZDUINO Display OTA Update</title>
    <style>
      body {
        display: flex;
        justify-content: center;
        align-items: center;
        flex-direction: column;
        background-color: black;
        color: white;
      }
      .btn {
        padding: 10px 20px;
        font-size: 16px;
        background-color: #007BFF;
        color: white;
        border: none;
        border-radius: 5px;
        cursor: pointer;
      }
      .toggle-btn.off {
        background-color: #FF0000;
      }
      .warning-box {
        background-color: #ffebcc;
        border: 2px solid #ff9900;
        padding: 15px;
        margin: 10px 0;
        border-radius: 5px;
        color: #333;
      }
      .warning-title {
        color: #cc6600;
        margin-top: 0;
        font-weight: bold;
      }
      .warning-text {
        margin: 5px 0;
        font-weight: bold;
        color: #cc6600;
      }
      .warning-list {
        margin: 5px 0;
        padding-left: 20px;
        color: #333;
      }
      .recovery-box {
        background-color: #ffe6e6;
        border: 1px solid #ff4444;
        padding: 10px;
        margin: 10px 0;
        border-radius: 5px;
      }
      .recovery-text {
        color: #cc0000;
        margin: 0;
        font-size: 14px;
      }
    </style>
    <script>
      function toggleState(button) {
        if (button.classList.contains('off')) {
          button.classList.remove('off');
          button.textContent = "Display 2";
          // Kirim status ON ke server
          fetch('/toggle', { method: 'POST', body: 'on' });
        } else {
          button.classList.add('off');
          button.textContent = "Display 1";
          // Kirim status OFF ke server
          fetch('/toggle', { method: 'POST', body: 'off' });
        }
      }
    </script>
  </head>
  <body>
    <h1>MAZDUINO Display OTA Update</h1>
    <div class="warning-box">
      <h3 class="warning-title">IMPORTANT WARNING - OTA UPDATE</h3>
      <p class="warning-text">Only use official firmware from Mazduino!</p>
      <p>If you upload unofficial firmware or from other sources:</p>
      <ul class="warning-list">
        <li>This web page will likely disappear</li>
        <li>OTA update feature may become inaccessible</li>
        <li>You will need to use USB cable for future updates</li>
      </ul>
      <p class="warning-text">Download official firmware only from: www.mazduino.com</p>
    </div>
    <form method="POST" action="/update" enctype="multipart/form-data">
      <input type="file" name="firmware" accept=".bin">
      <input type="submit" class="btn" value="Upload Firmware" onclick="return confirm('Are you sure this is official Mazduino firmware?')">
    </form>
    <hr>
    <h2>Display Control</h2>
    <h3>Splash Screen Selection</h3>
    <button class="btn" onclick="nextSplash()">Next Splash Screen</button>
    <p id="currentSplash">Current: Loading...</p>
    <hr>
    <h3>Display Mode Control</h3>
    <button class="btn" id="displayModeBtn" onclick="toggleDisplayMode()">Switch to FP Mode</button>
    <p id="currentMode">Current: Loading...</p>
    <hr>
    <h3>Support & Documentation</h3>
    <p>For complete documentation, tutorials, and support:</p>
    <p><a href="https://www.mazduino.com" target="_blank" style="color: #007BFF; text-decoration: none;">Visit www.mazduino.com</a></p>
    <p>Questions, feedback, or need help? Visit our website for:</p>
    <ul style="text-align: left; max-width: 400px;">
      <li>Complete setup guides</li>
      <li>Troubleshooting help</li>
      <li>Feature requests</li>
      <li>Community support</li>
      <li>Direct contact</li>
      <li><strong>Official firmware downloads</strong></li>
    </ul>
    <p><a href="/info" target="_blank" style="color: #28a745; text-decoration: none;">Device Information</a></p>
    <div class="recovery-box">
      <p class="recovery-text"><strong>Backup Recovery:</strong> If this page becomes inaccessible after firmware update, use USB cable to upload the correct firmware.</p>
    </div>
    <hr>
    <!-- <button class="btn toggle-btn" onclick="toggleState(this)">Display 1</button> -->
    
    <script>
      function nextSplash() {
        fetch('/splash', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
          document.getElementById('currentSplash').textContent = 'Current: ' + data;
        });
      }
      
      function toggleDisplayMode() {
        fetch('/displaymode', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
          document.getElementById('currentMode').textContent = 'Current: ' + data + ' Mode';
          // Update button text
          const btn = document.getElementById('displayModeBtn');
          btn.textContent = data === 'FPS' ? 'Switch to FP Mode' : 'Switch to FPS Mode';
        });
      }
      
      // Load current splash screen name on page load
      fetch('/splash', { method: 'GET' })
      .then(response => response.text())
      .then(data => {
        document.getElementById('currentSplash').textContent = 'Current: ' + data;
      });
      
      // Load current display mode on page load
      fetch('/displaymode', { method: 'GET' })
      .then(response => response.text())
      .then(data => {
        document.getElementById('currentMode').textContent = 'Current: ' + data + ' Mode';
        const btn = document.getElementById('displayModeBtn');
        btn.textContent = data === 'FPS' ? 'Switch to FP Mode' : 'Switch to FPS Mode';
      });
    </script>

  </body>
</html>
)rawliteral";

void handleToggle() {
  if (server.method() == HTTP_POST) {
    bool toggleState = EEPROM.read(0) || false;
    String body = server.arg("plain");  // Ambil isi body POST
    if (body == "on") {
      toggleState = 1;
      // Serial.println("Toggle: ON");
    } else if (body == "off") {
      toggleState = 0;
      // Serial.println("Toggle: OFF");
    }
    EEPROM.write(0, toggleState);
    EEPROM.commit();

    server.send(200, "text/plain", "OK");
    // Removed restart - no longer needed for display mode changes
  }
}

void handleSplash() {
  if (server.method() == HTTP_POST) {
    // Switch to next splash screen
    SplashType newSplash = nextSplashScreen();
    server.send(200, "text/plain", getSplashScreenName(newSplash));
  } else if (server.method() == HTTP_GET) {
    // Return current splash screen name
    server.send(200, "text/plain", getSplashScreenName());
  }
}

void handleDisplayMode() {
  if (server.method() == HTTP_POST) {
    // Toggle display mode
    bool currentMode = EEPROM.read(0);
    bool newMode = !currentMode;
    EEPROM.write(0, newMode);
    EEPROM.commit();
    
    // Force immediate redraw of the FPS/FP label
    forceRedrawFPSLabel();
    
    // Also force redraw on next itemDraw call for consistency
    lazyUpdateTime = 0;
    
    // Return new mode name
    server.send(200, "text/plain", newMode ? "FPS" : "FP");
  } else if (server.method() == HTTP_GET) {
    // Return current display mode
    bool currentMode = EEPROM.read(0);
    server.send(200, "text/plain", currentMode ? "FPS" : "FP");
  }
}

void handleInfo() {
  String info = "Mazduino Display v" + String(version) + "\n";
  info += "Hardware: ESP32-C3 + ILI9488 3.5\" TFT\n";
  info += "Current Splash: " + String(getSplashScreenName()) + "\n";
  info += "Display Mode: " + String((EEPROM.read(0) == 1) ? "FPS" : "FP") + "\n";
  info += "WiFi: " + String(WiFi.softAPgetStationNum()) + " clients connected\n";
  info += "Uptime: " + String(millis() / 1000) + " seconds\n";
  info += "Memory: " + String(ESP.getFreeHeap()) + " bytes free\n";
  info += "\nFor support and documentation visit:\n";
  info += "https://www.mazduino.com\n";
  
  server.send(200, "text/plain", info);
}

void handleRoot() {
  server.send(200, "text/html", uploadPage);
}

void setup() {
  display.init();
  display.setRotation(3);
  
  // Initialize EEPROM first
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize splash screen manager
  initSplashManager();
  
  // Draw the selected splash screen
  drawSplashScreenWithImage();
  display.fillScreen(TFT_BLACK);

  // Serial.begin(UART_BAUD);
  Serial1.begin(UART_BAUD, SERIAL_8N1, RXD, TXD);

  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAPConfig(ip, ip, netmask);
  WiFi.softAP(ssid, password);

  server.on("/", HTTP_GET, handleRoot);
  server.on(
    "/update", HTTP_POST, []() {
      server.send(200, "text/plain", (Update.hasError()) ? "Gagal update!" : "Update berhasil! MAZDUINO Display akan restart.");
      delay(1000);
      ESP.restart();
    },
    handleUpdate);
  server.on("/toggle", HTTP_POST, handleToggle);     // Endpoint untuk toggle
  server.on("/splash", HTTP_GET, handleSplash);      // Get current splash screen
  server.on("/splash", HTTP_POST, handleSplash);     // Switch to next splash screen
  server.on("/displaymode", HTTP_GET, handleDisplayMode);   // Get current display mode
  server.on("/displaymode", HTTP_POST, handleDisplayMode);  // Toggle display mode
  server.on("/info", HTTP_GET, handleInfo);          // Get device information

  server.begin();
  // Serial.println("Web server aktif.");
  esp_wifi_set_max_tx_power(34); // Set max WiFi power for ESP32-C3

  // Only set default EEPROM value if it's uninitialized (first boot)
  // Check if EEPROM has been initialized by reading a marker at address 2
  if (EEPROM.read(2) != 0xAA) {
    // First boot - set default values
    EEPROM.write(0, 1);  // Default to FPS mode
    EEPROM.write(2, 0xAA); // Set initialization marker
    EEPROM.commit();
  }
  // If marker exists, keep existing display mode setting
  
  delay(500);
  startUpDisplay();
  startupTime = millis();
  lazyUpdateTime = startupTime;
  lastClientCheckTimeout = startupTime;
}

void loop() {
  static uint32_t lastUpdate = millis();
  if (millis() - lastUpdate > 10) {
    requestData(50);
    lastUpdate = millis();
  }

  static uint32_t lastRefresh = millis();
  uint32_t elapsed = millis() - lastRefresh;
  refreshRate = (elapsed > 0) ? (1000 / elapsed) : 0;
  lastRefresh = millis();
  if (lastRefresh - lazyUpdateTime > 100 || rpm < 100) {
    clt = getByte(7) - 40;
    iat = getByte(6) - 40;
    bat = getByte(9);
  }
  rpm = getWord(14);
  mapData = getWord(4);
  afrConv = getByte(10);
  tps = getByte(24) / 2;
  adv = (int8_t)getByte(23);
  fp = getByte(103);

  syncStatus = getBit(31, 7);
  ase = getBit(2, 2);
  wue = getBit(2, 3);
  rev = getBit(31, 2);
  launch = getBit(31, 0);
  airCon = getByte(122);
  fan = getBit(106, 3);
  dfco = getBit(1, 4);
  drawData();

  if (millis() - lastClientCheck >= 1000) {
    lastClientCheck = millis();
    int clientCount = WiFi.softAPgetStationNum();

    if (clientCount > 0) {
      clientConnected = true;
      lastClientCheck = millis();  // Reset timer jika ada koneksi
    } else if (millis() - lastClientCheckTimeout > wifiTimeout) {
      clientConnected = false;
      lastClientCheckTimeout = millis(); 
    }

    // Matikan WiFi jika RPM > 100 atau tidak ada perangkat terkoneksi selama 30 detik
    if ((rpm > 100 || !clientConnected) && wifiActive) {
      WiFi.mode(WIFI_OFF);
      server.stop();
      wifiActive = false;
    }
    // Nyalakan kembali WiFi jika RPM ≤ 100 dan ada perangkat yang terkoneksi
    else if (rpm <= 100 && clientConnected && !wifiActive) {
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(ip, ip, netmask);
      WiFi.softAP(ssid, password);
      server.begin();
      wifiActive = true;
      server.handleClient();
    }
  }



  if (rpm <= 100 && wifiActive) {
    server.handleClient();
  }
}
















