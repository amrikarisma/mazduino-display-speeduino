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
#include "splash/zettech.h"

#define AA_FONT_SMALL NotoSansBold15
#define AA_FONT_LARGE NotoSansBold36

// Forward declarations
void handleRoot();
void handleUpdate();
void handleToggle();
void drawSplashScreenWithImage();
void startUpDisplay();
void drawData();
void drawDataBox(int x, int y, const char* label, const int value, uint16_t labelColor, const int valueToCompare, const int decimal, bool setup);
void itemDraw(bool setup);

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
  // Draw enhanced ZetTech ECU splash image
  drawZetTechSplash(display, 0, 0, TFT_WHITE, TFT_BLACK);
  
  // Add version info at the bottom
  display.loadFont(AA_FONT_SMALL);
  display.setTextColor(TFT_YELLOW, TFT_BLACK);
  display.setTextDatum(BR_DATUM);
  display.drawString("v" + String(version), display.width() - 5, display.height() - 5);
  
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
        height: 100vh;
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
    <form method="POST" action="/update" enctype="multipart/form-data">
      <input type="file" name="firmware">
      <input type="submit" class="btn" value="Upload">
    </form>
    <hr>
    <h2>Display Control</h2>
    <p>Sabar ya nanti diupdate :) </p>
    <!-- <button class="btn toggle-btn" onclick="toggleState(this)">Display 1</button> -->

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
    delay(1000);
    ESP.restart();
  }
}

void handleRoot() {
  server.send(200, "text/html", uploadPage);
}

void setup() {
  display.init();
  display.setRotation(3);
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
  server.on("/toggle", HTTP_POST, handleToggle);  // Endpoint untuk toggle

  server.begin();
  // Serial.println("Web server aktif.");
  esp_wifi_set_max_tx_power(78);

  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(0, 1);
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
















