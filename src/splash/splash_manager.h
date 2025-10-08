/**
 * @file splash_manager.h
 * @brief Dynamic Splash Screen Manager
 * @version 1.0
 * @date 2025-10-08
 * 
 * This file provides dynamic splash screen selection functionality.
 * Supports multiple splash screen options with EEPROM storage for persistence.
 * 
 * Features:
 * - Multiple splash screen options
 * - EEPROM storage for user preference
 * - Easy selection interface
 * - Consistent API across all splash screens
 */

#ifndef SPLASH_MANAGER_H
#define SPLASH_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <EEPROM.h>

// EEPROM configuration
#ifndef EEPROM_SIZE
#define EEPROM_SIZE 512
#endif

#include "zettech.h"
#include "mazduino.h"
#include "speeduino.h"

// EEPROM addresses for splash screen settings
#define SPLASH_SELECTION_ADDR 1  // EEPROM address for splash selection (address 0 is used by main code)

// Splash screen types
enum SplashType {
    SPLASH_ZETTECH = 0,
    SPLASH_MAZDUINO = 1,
    SPLASH_SPEEDUINO = 2,
    SPLASH_COUNT = 3  // Total number of splash screens
};

// Forward declaration
bool setSplashScreen(SplashType splashType);

// Splash screen information structure
struct SplashInfo {
    const char* name;
    const unsigned char* bitmap;
    void (*drawFunction)(TFT_eSPI&, int16_t, int16_t, uint16_t, uint16_t);
    void (*showFunction)(TFT_eSPI&, uint16_t);
};

// Splash screen registry
const SplashInfo splashScreens[SPLASH_COUNT] = {
    {
        "ZetTech ECU",
        epd_bitmap_ztech,
        drawZetTechSplash,
        showZetTechSplash
    },
    {
        "Mazduino ECU", 
        epd_bitmap_mazduino_logo,
        drawMazduinoSplash,
        showMazduinoSplash
    },
    {
        "Speeduino ECU",
        epd_bitmap_speeduino,
        drawSpeeduinoSplash,
        showSpeeduinoSplash
    }
};

/**
 * @brief Initialize splash screen manager
 * Reads current selection from EEPROM
 */
void initSplashManager() {
    // Ensure EEPROM is initialized
    if (!EEPROM.begin(EEPROM_SIZE)) {
        Serial.println("EEPROM initialization failed!");
    }
    
    // Validate stored splash selection
    uint8_t currentSelection = EEPROM.read(SPLASH_SELECTION_ADDR);
    if (currentSelection >= SPLASH_COUNT) {
        // Invalid selection, reset to default
        setSplashScreen(SPLASH_ZETTECH);
    }
}

/**
 * @brief Get current splash screen selection
 * @return Current splash screen type
 */
SplashType getCurrentSplashScreen() {
    uint8_t selection = EEPROM.read(SPLASH_SELECTION_ADDR);
    return (selection < SPLASH_COUNT) ? (SplashType)selection : SPLASH_ZETTECH;
}

/**
 * @brief Set splash screen selection
 * @param splashType The splash screen type to set
 * @return true if successful, false if invalid type
 */
bool setSplashScreen(SplashType splashType) {
    if (splashType >= SPLASH_COUNT) {
        return false;
    }
    
    EEPROM.write(SPLASH_SELECTION_ADDR, splashType);
    EEPROM.commit();
    return true;
}

/**
 * @brief Cycle to next splash screen
 * @return New splash screen type
 */
SplashType nextSplashScreen() {
    SplashType current = getCurrentSplashScreen();
    SplashType next = (SplashType)((current + 1) % SPLASH_COUNT);
    setSplashScreen(next);
    return next;
}

/**
 * @brief Get splash screen name
 * @param splashType The splash screen type
 * @return Name of the splash screen
 */
const char* getSplashScreenName(SplashType splashType = getCurrentSplashScreen()) {
    if (splashType >= SPLASH_COUNT) {
        return "Unknown";
    }
    return splashScreens[splashType].name;
}

/**
 * @brief Draw current splash screen
 * @param tft TFT display object
 * @param x X position (default: 0)
 * @param y Y position (default: 0)
 * @param color_fg Foreground color (default: TFT_WHITE)
 * @param color_bg Background color (default: TFT_BLACK)
 */
void drawCurrentSplash(TFT_eSPI& tft, int16_t x = 0, int16_t y = 0, 
                       uint16_t color_fg = TFT_WHITE, uint16_t color_bg = TFT_BLACK) {
    SplashType current = getCurrentSplashScreen();
    splashScreens[current].drawFunction(tft, x, y, color_fg, color_bg);
}

/**
 * @brief Show current splash screen with delay
 * @param tft TFT display object
 * @param delay_ms Display duration in milliseconds (default: 3000)
 */
void showCurrentSplash(TFT_eSPI& tft, uint16_t delay_ms = SPLASH_DISPLAY_TIME) {
    SplashType current = getCurrentSplashScreen();
    splashScreens[current].showFunction(tft, delay_ms);
}

/**
 * @brief Draw specific splash screen
 * @param tft TFT display object
 * @param splashType Specific splash screen type
 * @param x X position (default: 0)
 * @param y Y position (default: 0)
 * @param color_fg Foreground color (default: TFT_WHITE)
 * @param color_bg Background color (default: TFT_BLACK)
 */
void drawSplash(TFT_eSPI& tft, SplashType splashType, int16_t x = 0, int16_t y = 0,
                uint16_t color_fg = TFT_WHITE, uint16_t color_bg = TFT_BLACK) {
    if (splashType >= SPLASH_COUNT) return;
    splashScreens[splashType].drawFunction(tft, x, y, color_fg, color_bg);
}

/**
 * @brief Show specific splash screen with delay
 * @param tft TFT display object
 * @param splashType Specific splash screen type
 * @param delay_ms Display duration in milliseconds
 */
void showSplash(TFT_eSPI& tft, SplashType splashType, uint16_t delay_ms = SPLASH_DISPLAY_TIME) {
    if (splashType >= SPLASH_COUNT) return;
    splashScreens[splashType].showFunction(tft, delay_ms);
}

/**
 * @brief Print available splash screens to Serial
 */
void listSplashScreens() {
    Serial.println("Available Splash Screens:");
    for (int i = 0; i < SPLASH_COUNT; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(splashScreens[i].name);
    }
    Serial.print("Current: ");
    Serial.println(getSplashScreenName());
}

#endif // SPLASH_MANAGER_H