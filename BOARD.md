# diymore ESP32 Development Board with 1.9" LCD Display

> **Reference document** — compiled from product listings, Waveshare documentation, community research, and confirmed working configurations.

---

## Overview

The diymore ESP32 board integrates the ESP32-WROOM-32 module with a 1.9-inch ST7789V2 LCD screen, combining full ESP32 functionality with an onboard colour display. The LCD driver code is based on Waveshare's standard LCD library (confirmed from product documentation showing `LCD_Driver.cpp`, `WIFI_Driver.c` etc., authored by the Waveshare team, dated 2023-11-22). Suitable for beginners and professionals building IoT projects.

---

## Hardware Specifications

| Parameter | Value |
|-----------|-------|
| Module | ESP32-WROOM-32 |
| USB Interface | Type-C |
| Working Voltage | USB 5V |
| USB-Serial Chip | CH340 |
| LCD Size | 1.9 inches |
| LCD Resolution | 170 × 320 pixels |
| LCD Driver IC | ST7789V2 |
| LCD Colour Depth | 262K colours (RGB565) |
| LCD Interface | 4-wire SPI (write-only, no MISO) |
| SPI Flash | 32Mbit default |
| Serial Speed | 115200 bps |
| WiFi | 2.4GHz, 802.11 b/g/n, 2412–2484 MHz |
| Bluetooth | 4.2 BR/EDR and BLE |
| Antenna | Onboard PCB, 2dBi gain |

---

## Supported Interfaces

UART, SPI, SDIO, I2C, PWM, I2S, IR, ADC, DAC

---

## Pinout — Header Pins

### Left Side (top to bottom)

| Outer Label | Inner Label | GPIO |
|-------------|-------------|------|
| — | EN | RESET |
| — | ADC0 | GPIO36 |
| — | ADC3 | GPIO39 |
| — | ADC6 | GPIO34 |
| — | ADC7 | GPIO35 |
| CH9 | ADC4 | GPIO32 |
| CH8 | ADC5 | GPIO33 |
| DAC1 | ADC18 | GPIO25 |
| DAC2 | ADC19 | GPIO26 |
| CH7 | ADC17 | GPIO27 |
| CH6 | ADC16 | GPIO14 |
| CH5 | ADC15 | GPIO12 |
| CH4 | ADC14 | GPIO13 |
| — | — | GND |
| — | — | VIN (5V) |

### Right Side (top to bottom)

| GPIO | Function | Outer Label |
|------|----------|-------------|
| GPIO23 | MOSI (SPI) | — |
| GPIO22 | I2C SCL | — |
| GPIO1 | UART0 TX | — |
| GPIO3 | UART0 RX | — |
| GPIO21 | I2C SDA | — |
| GPIO19 | MISO (SPI) | — |
| GPIO18 | SCK (SPI) | — |
| GPIO5 | SS/CS (SPI) | — |
| GPIO17 | UART2 TX | — |
| GPIO16 | UART2 RX | — |
| GPIO4 | ADC10 | CH0 |
| GPIO2 | ADC12 | CH2 |
| GPIO15 | ADC13 | CH3 |
| — | GND | — |
| — | 3.3V | — |

### Onboard Buttons

| Button | Function |
|--------|----------|
| BOOT | Hold during power-on/reset to enter firmware download mode |
| EN | Hardware reset |

---

## Internal LCD Wiring — Confirmed ✅

The onboard ST7789V2 display is wired to the ESP32 internally via dedicated GPIOs. These are **separate from and independent of the broken-out SPI header pins**. Confirmed from the ideaspark board (identical hardware design) official documentation, and cross-referenced against multiple working community configurations.

| LCD Signal | GPIO | Notes |
|-----------|------|-------|
| MOSI | GPIO23 | Shared with header — do not use header GPIO23 for other SPI devices simultaneously |
| SCLK | GPIO18 | Shared with header |
| CS | GPIO15 | Strapping pin — dedicated to LCD |
| DC (Data/Command) | GPIO2 | Strapping pin — see WiFi caveat below |
| RST | GPIO4 | Dedicated to LCD reset |
| BL (Backlight) | GPIO32 | PWM-capable — can be used to dim the display |

> **Strapping pin note:** GPIO2 and GPIO15 are ESP32 strapping pins. The board manages their boot states correctly in normal operation. Do not add strong external pull-resistors to these pins.

> **WiFi + GPIO2 caveat:** GPIO2 is on ADC2, which shares resources with the WiFi radio. The DC signal is output-only so display operation is unaffected, but do not attempt to use GPIO2 for analogue reads when WiFi is active.

---

## Arduino IDE Board Configuration

From the official diymore product documentation images:

| Setting | Value |
|---------|-------|
| Board | **ESP32 Wrover Module** |
| Core Debug Level | None |
| Erase All Flash Before Sketch Upload | Disabled |
| Flash Frequency | 80MHz |
| Flash Mode | QIO |
| Partition Scheme | Default 4MB with spiffs (1.2MB APP / 1.5MB SPIFFS) |
| Upload Speed | 921600 |

> **Important:** Use **"ESP32 Wrover Module"** — not the generic `esp32dev`. If uploads fail at 921600 baud, drop to 460800.

Board manager URL:
```
https://dl.espressif.com/dl/package_esp32_index.json
```

---

## TFT_eSPI Configuration

Complete `User_Setup.h` for the onboard display. Place this in your `TFT_eSPI` library folder, replacing the default `User_Setup.h`.

```cpp
// User_Setup.h for diymore ESP32 1.9" LCD (ST7789V2, 170x320)
// Confirmed pin mapping from ideaspark board (identical hardware)
// and Waveshare LCD_Driver source code

#define USER_SETUP_ID 100

// Driver
#define ST7789_DRIVER

// Display dimensions
#define TFT_WIDTH  170
#define TFT_HEIGHT 320

// Colour order — ST7789V2 uses BGR
#define TFT_RGB_ORDER TFT_BGR

// Inversion ON required for correct colours on this panel
#define TFT_INVERSION_ON

// Internal LCD SPI wiring (NOT the header pins)
#define TFT_MOSI  23   // SDA
#define TFT_SCLK  18   // SCL / CLK
#define TFT_CS    15   // Chip Select
#define TFT_DC     2   // Data/Command
#define TFT_RST    4   // Reset
#define TFT_BL    32   // Backlight (PWM capable)
#define TFT_MISO  -1   // Not connected — display is write-only

// Backlight ON level
#define TFT_BACKLIGHT_ON HIGH

// Fonts to load
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI speed — 80MHz is stable on this board
#define SPI_FREQUENCY       80000000
#define SPI_READ_FREQUENCY  20000000
```

---

## Starter Sketch — Display Test

```cpp
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  // Turn on backlight
  pinMode(32, OUTPUT);
  digitalWrite(32, HIGH);

  tft.init();
  tft.setRotation(0);         // 0 = portrait. 1 = landscape.
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("diymore ESP32");
  tft.setCursor(10, 40);
  tft.println("1.9\" ST7789V2");
  tft.setCursor(10, 70);
  tft.println("170 x 320");

  // Colour blocks
  tft.fillRect(10,  120, 45, 45, TFT_RED);
  tft.fillRect(65,  120, 45, 45, TFT_GREEN);
  tft.fillRect(120, 120, 45, 45, TFT_BLUE);
}

void loop() {}
```

---

## Backlight Dimming (PWM)

GPIO32 is PWM-capable, allowing smooth brightness control:

```cpp
const int BL_PIN     = 32;
const int BL_CHANNEL = 0;
const int BL_FREQ    = 5000;  // 5kHz
const int BL_RES     = 8;     // 8-bit resolution (0–255)

void setup() {
  ledcSetup(BL_CHANNEL, BL_FREQ, BL_RES);
  ledcAttachPin(BL_PIN, BL_CHANNEL);
  ledcWrite(BL_CHANNEL, 200); // 0 = off, 255 = full brightness
}
```

---

## GPIO Usage Guide

### Pins used internally by the LCD — do not reuse

| GPIO | LCD Function |
|------|-------------|
| GPIO2 | DC (Data/Command) |
| GPIO4 | RST |
| GPIO15 | CS |
| GPIO18 | SCLK — shared with header |
| GPIO23 | MOSI — shared with header |
| GPIO32 | Backlight |

### Free to use for your project

| GPIO | Notes |
|------|-------|
| GPIO13, 14 | General purpose |
| GPIO21 | Default I2C SDA — free, broken out on header |
| GPIO22 | Default I2C SCL — free, broken out on header |
| GPIO25 | Also DAC1 |
| GPIO26 | Also DAC2 |
| GPIO27 | General purpose |
| GPIO33 | General purpose |
| GPIO34, 35, 36, 39 | **Input only** — no pull-up/down, no output |

### Strapping pins — handle with care

| GPIO | State at boot | In normal use |
|------|---------------|---------------|
| GPIO0 | LOW = flash download mode | Fine after boot |
| GPIO2 | Must be LOW or floating | Used for LCD DC |
| GPIO5 | Must be HIGH | Fine after boot |
| GPIO12 | Controls flash voltage — leave unconnected | Avoid |
| GPIO15 | Must be HIGH | Used for LCD CS |

### ADC2 pins — avoid for analogue when WiFi is active

GPIO2, 4, 12, 13, 14, 15, 25, 26, 27 are all ADC2 channels. ADC2 is unreliable for analogue reads while WiFi is running. Use **ADC1 pins** (GPIO32–39) for any analogue sensing in WiFi projects.

---

## Libraries

### Display

| Library | Notes |
|---------|-------|
| **TFT_eSPI** (Bodmer) | Recommended. Fast, full-featured, ST7789 support built-in. |
| **LovyanGFX** | High-performance alternative, has auto-detect mode |
| **Adafruit ST7789** | Simpler, slower alternative |
| **Arduino_GFX** | Waveshare's preferred library in their own demo code |
| **LVGL** | Full GUI framework — widgets, animations, events |

### WiFi

| Library | Notes |
|---------|-------|
| `WiFi.h` | Built into Arduino core |
| **AsyncTCP + ESPAsyncWebServer** | Non-blocking async web server |
| **HTTPClient** | REST / HTTP requests |
| **WiFiManager** | Captive portal for runtime WiFi credential entry |

### Bluetooth

| Library | Notes |
|---------|-------|
| `BluetoothSerial.h` | Classic Bluetooth (BR/EDR) — built-in |
| `BLEDevice.h` | Bluetooth Low Energy — built-in |

### IoT / Protocols

| Library | Notes |
|---------|-------|
| **PubSubClient** | MQTT client |
| **ArduinoJson** | JSON parsing / serialisation |

---

## Development Environment Setup

### Arduino IDE

1. Install **CH340 driver** (search "CH340 Driver" for your OS; Linux usually has it built in)
2. Open Arduino IDE → File → Preferences → Additional Boards Manager URLs → paste:
   `https://dl.espressif.com/dl/package_esp32_index.json`
3. Tools → Board → Boards Manager → search **ESP32** → install **esp32 by Espressif Systems**
4. Select Tools → Board → **ESP32 Wrover Module**
5. Set: Flash Mode: QIO | Flash Frequency: 80MHz | Upload Speed: 921600
6. Install **TFT_eSPI** via Library Manager
7. Replace `TFT_eSPI/User_Setup.h` with the config from this document

### PlatformIO (VS Code) — Recommended

`platformio.ini`:
```ini
[env:esp32wrover]
platform  = espressif32
board     = esp32wrover
framework = arduino
monitor_speed = 115200
upload_speed  = 921600

lib_deps =
    bodmer/TFT_eSPI
    bblanchon/ArduinoJson
    knolleary/PubSubClient
```

Place your custom `User_Setup.h` in the project's `lib/TFT_eSPI/` folder to override the library default without editing the library itself.

### Firmware Download Mode

If upload fails: hold **BOOT**, press **EN** to reset, release **BOOT** once the upload starts. Alternatively, hold BOOT while plugging in the USB cable.

---

## Official Sample Code

The official diymore demo sketch is called **`WIFI_AP_LCD`** and consists of these files (all based on Waveshare's standard library structure):

| File | Purpose |
|------|---------|
| `LCD_Driver.cpp` / `.h` | ST7789 SPI driver |
| `GUI_Paint.cpp` / `.h` | Drawing primitives (lines, rectangles, text, images) |
| `WIFI_Driver.cpp` / `.h` | WiFi access point setup |
| `font12.cpp` etc. | Bitmap font data |

After flashing, the board creates a WiFi access point (`HW-656 Server` at `192.168.4.1`). Connect to it and browse to that address to control display output (monochrome or image mode).

---

## Known Issues & Caveats

| Issue | Detail |
|-------|--------|
| **GPIO2 / WiFi** | GPIO2 (DC pin) is ADC2; ADC2 shares resources with WiFi. No display issue, but don't use GPIO2 for ADC reads when WiFi is active. |
| **Strapping pins** | GPIO2 and GPIO15 are boot strapping pins used by the LCD. Do not add external pull-ups/downs. |
| **GPIO12** | Controls ESP32 flash voltage. Never pull HIGH at boot — can prevent startup. Avoid using it. |
| **Upload failures** | Drop upload speed from 921600 to 460800 if experiencing timeouts. |
| **USB-C to USB-C** | Some users report USB-C to USB-C cables not powering the board — use USB-A to USB-C. |
| **ADC2 with WiFi** | All ADC2 pins (GPIO2, 4, 12–15, 25–27) are unreliable for analogue reads when WiFi is active. |
| **SPI bus sharing** | GPIO18 (SCLK) and GPIO23 (MOSI) are shared between the LCD and the external SPI header. Adding another SPI device on these pins will work electrically, but requires careful CS management. |
| **Board type** | Must use "ESP32 Wrover Module" not generic esp32dev — wrong partition/memory layout otherwise. |

---

## Typical Applications

- WiFi-connected IoT sensors with live display readout
- Smart home status dashboards
- Mesh networking nodes with status screen
- BLE peripheral devices
- WiFi access point with web configuration UI
- Data logging with on-device display
- General Arduino-compatible prototyping

---

## Resources

| Resource | Link |
|----------|------|
| diymore official store | https://www.diymore.cc |
| Amazon UK listing | https://www.amazon.co.uk/dp/B0DK399Y9Q |
| ESP32 Arduino Core docs | https://docs.espressif.com/projects/arduino-esp32/en/latest/ |
| ESP-IDF GPIO reference | https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html |
| TFT_eSPI library | https://github.com/Bodmer/TFT_eSPI |
| LovyanGFX library | https://github.com/lovyan03/LovyanGFX |
| LVGL | https://lvgl.io |
| Waveshare 1.9" LCD wiki | https://www.waveshare.com/wiki/1.9inch_LCD_Module |
| ST7789 170×320 TFT_eSPI config | https://github.com/mboehmerm/IPS-Display-ST7789-170x320 |
| ideaspark example code (same HW) | https://github.com/jwkunz/ideaspark_esp32_114LCD_example |
| Board manager JSON URL | https://dl.espressif.com/dl/package_esp32_index.json |