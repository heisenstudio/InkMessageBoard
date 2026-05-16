#pragma once

#include <Arduino.h>
#include <GxEPD2_BW.h>

#ifndef BSP_EPAPER_SPI_SCK_PIN
#define BSP_EPAPER_SPI_SCK_PIN 33
#endif

#ifndef BSP_EPAPER_SPI_MOSI_PIN
#define BSP_EPAPER_SPI_MOSI_PIN 34
#endif

#ifndef BSP_EPAPER_SPI_MISO_PIN
#define BSP_EPAPER_SPI_MISO_PIN -1
#endif

#ifndef BSP_EPAPER_CS_PIN
#define BSP_EPAPER_CS_PIN 47
#endif

#ifndef BSP_EPAPER_DC_PIN
#define BSP_EPAPER_DC_PIN 48
#endif

#ifndef BSP_EPAPER_RST_PIN
#define BSP_EPAPER_RST_PIN 35
#endif

#ifndef BSP_EPAPER_BUSY_PIN
#define BSP_EPAPER_BUSY_PIN 8
#endif

#ifndef BSP_EPAPER_POWER_EN_PIN
#define BSP_EPAPER_POWER_EN_PIN 9
#endif

#ifndef BSP_EPAPER_SPI_SPEED_HZ
#define BSP_EPAPER_SPI_SPEED_HZ 1000000
#endif

namespace BspEpaper
{
using Display = GxEPD2_BW<GxEPD2_260, GxEPD2_260::HEIGHT>;

bool begin();
void enablePower();
void powerOff();

Display &device();

int16_t width();
int16_t height();
int16_t panelWidth();
int16_t panelHeight();
int16_t panel();

void setRotation(uint8_t rotation);
void setFullWindow();
void firstPage();
bool nextPage();

void fillScreen(uint16_t color);
void drawPixel(int16_t x, int16_t y, uint16_t color);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color);
void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

void setFont(const GFXfont *font = nullptr);
void setTextSize(uint8_t size);
void setTextColor(uint16_t color);
void setCursor(int16_t x, int16_t y);
void print(const char *text);
}
