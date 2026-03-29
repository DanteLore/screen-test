#pragma once

#include <stdint.h>

// Internal LCD wiring (diymore ESP32 1.9" board)
#define ST7789_PIN_MOSI  23
#define ST7789_PIN_SCLK  18
#define ST7789_PIN_CS    15
#define ST7789_PIN_DC     2
#define ST7789_PIN_RST    4
#define ST7789_PIN_BL    32

#define ST7789_WIDTH  170
#define ST7789_HEIGHT 320

// RGB565 colour helpers
#define RGB565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     RGB565(255,   0,   0)
#define COLOR_GREEN   RGB565(  0, 255,   0)
#define COLOR_BLUE    RGB565(  0,   0, 255)
#define COLOR_CYAN    RGB565(  0, 255, 255)
#define COLOR_MAGENTA RGB565(255,   0, 255)
#define COLOR_YELLOW  RGB565(255, 255,   0)

void st7789_init(void);
void st7789_fill(uint16_t colour);
void st7789_fill_rect(int x, int y, int w, int h, uint16_t colour);
