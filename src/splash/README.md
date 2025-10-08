# Dynamic Splash Screen System

## Overview

The Mazduino Display now supports dynamic splash screen selection with multiple logo options that can be changed without reprogramming the device.

## Features

- **Multiple Splash Screens**: Currently supports ZetTech ECU and Mazduino ECU logos
- **EEPROM Storage**: Selection is stored persistently in EEPROM 
- **Web Interface**: Change splash screens via web interface
- **Real-time Switching**: No restart required for display changes
- **Extensible**: Easy to add more splash screen options

## Available Splash Screens

1. **ZetTech ECU** - Professional ECU branding logo
2. **Mazduino ECU** - Mazduino project logo
3. **Speeduino ECU** - Official Speeduino project logo

## How to Use

### Via Web Interface

1. Connect to the device's WiFi hotspot: `MAZDUINO_Display`
2. Open browser and go to: `192.168.1.80`
3. **Splash Screen Control:**
   - Click "Next Splash Screen" button to cycle through available options
   - Current splash screen name is displayed on the page
4. **Display Mode Control:**
   - Click "Switch to FP Mode" or "Switch to FPS Mode" button to toggle
   - Current display mode is shown (FPS = Frames Per Second, FP = Fuel Pump)
   - Changes take effect immediately with instant label redraw
   - Settings persist across power cycles

### Programmatically

```cpp
// Get current splash screen
SplashType current = getCurrentSplashScreen();

// Set specific splash screen
setSplashScreen(SPLASH_ZETTECH);    // ZetTech logo
setSplashScreen(SPLASH_MAZDUINO);   // Mazduino logo
setSplashScreen(SPLASH_SPEEDUINO);  // Speeduino logo

// Cycle to next splash screen
SplashType next = nextSplashScreen();

// Display current splash screen
drawCurrentSplash(display);

// Get splash screen name
const char* name = getSplashScreenName();
```

## Files Structure

```
src/splash/
├── zettech.h          - ZetTech logo bitmap and functions
├── mazduino.h         - Mazduino logo bitmap and functions
├── speeduino.h        - Speeduino logo bitmap and functions
└── splash_manager.h   - Dynamic splash screen manager
```

## EEPROM Layout

- Address 0: Display mode (0=FP, 1=FPS) - Persistent across power cycles
- Address 1: Splash screen selection (0=ZetTech, 1=Mazduino, 2=Speeduino)  
- Address 2: Initialization marker (0xAA) - Prevents reset of user settings

## Adding New Splash Screens

1. Create new header file in `src/splash/` with bitmap data
2. Add proper header structure similar to existing files
3. Update `SplashType` enum in `splash_manager.h`
4. Add entry to `splashScreens` array
5. Update `SPLASH_COUNT` value

## Technical Details

- **Image Format**: 480x320 monochrome bitmap
- **Memory Usage**: ~19KB per splash screen in PROGMEM
- **Storage**: Selection persisted in EEPROM
- **Display Time**: 5 seconds (configurable)
- **Web Control**: RESTful API endpoints for switching

## API Endpoints

### Splash Screen Control
- `GET /splash` - Returns current splash screen name
- `POST /splash` - Switches to next splash screen

### Display Mode Control  
- `GET /displaymode` - Returns current display mode (FPS or FP)
- `POST /displaymode` - Toggles between FPS and FP modes

### Web Interface
- `GET /` - Main web interface with all controls

## Memory Usage

With 3 splash screens in dynamic system:
- RAM: 12.4% (40,556 bytes)
- Flash: 71.9% (942,052 bytes)
- Still room for 1-2 additional splash screens

## Notes

- Splash screen selection is independent of display mode
- Settings persist through power cycles
- Web interface only accessible when RPM ≤ 100
- Changes take effect immediately on next startup display

## Support & Documentation

### 🌐 **Need Help?**
For complete documentation, tutorials, and support visit:
**[www.mazduino.com](https://www.mazduino.com)**

### 📚 **Available Resources:**
- Complete setup guides and tutorials
- Troubleshooting and FAQ section
- Feature documentation and examples
- Community forum and support
- Direct contact for technical issues

### 💬 **Get Involved:**
- Submit feature requests
- Report bugs and issues
- Share your custom splash screens
- Help other users in the community
- Contribute to documentation

**Visit [www.mazduino.com](https://www.mazduino.com) for all support needs!**