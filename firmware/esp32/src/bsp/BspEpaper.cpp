#include "BspEpaper.h"

#include <SPI.h>

namespace BspEpaper
{
namespace
{
Display sDisplay(
    GxEPD2_260(BSP_EPAPER_CS_PIN, BSP_EPAPER_DC_PIN, BSP_EPAPER_RST_PIN, BSP_EPAPER_BUSY_PIN));
}

bool begin()
{
  enablePower();
  SPI.begin(BSP_EPAPER_SPI_SCK_PIN, BSP_EPAPER_SPI_MISO_PIN, BSP_EPAPER_SPI_MOSI_PIN, BSP_EPAPER_CS_PIN);
  sDisplay.epd2.selectSPI(SPI, SPISettings(BSP_EPAPER_SPI_SPEED_HZ, MSBFIRST, SPI_MODE0));
  sDisplay.init(0, true, 100, false);
  return true;
}

void enablePower()
{
  pinMode(BSP_EPAPER_POWER_EN_PIN, OUTPUT);
  digitalWrite(BSP_EPAPER_POWER_EN_PIN, LOW);
  delay(200);
}

void powerOff()
{
  sDisplay.powerOff();
}

Display &device()
{
  return sDisplay;
}

int16_t width()
{
  return sDisplay.width();
}

int16_t height()
{
  return sDisplay.height();
}

int16_t panelWidth()
{
  return sDisplay.epd2.WIDTH;
}

int16_t panelHeight()
{
  return sDisplay.epd2.HEIGHT;
}

int16_t panel()
{
  return static_cast<int16_t>(sDisplay.epd2.panel);
}

void setRotation(uint8_t rotation)
{
  sDisplay.setRotation(rotation);
}

void setFullWindow()
{
  sDisplay.setFullWindow();
}

void firstPage()
{
  sDisplay.firstPage();
}

bool nextPage()
{
  return sDisplay.nextPage();
}

void fillScreen(uint16_t color)
{
  sDisplay.fillScreen(color);
}

void drawPixel(int16_t x, int16_t y, uint16_t color)
{
  sDisplay.drawPixel(x, y, color);
}

void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  sDisplay.drawLine(x0, y0, x1, y1, color);
}

void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  sDisplay.drawFastHLine(x, y, w, color);
}

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  sDisplay.drawFastVLine(x, y, h, color);
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  sDisplay.drawRect(x, y, w, h, color);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  sDisplay.fillRect(x, y, w, h, color);
}

void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
  sDisplay.drawRoundRect(x, y, w, h, r, color);
}

void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
  sDisplay.fillRoundRect(x, y, w, h, r, color);
}

void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  sDisplay.drawCircle(x, y, r, color);
}

void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color)
{
  sDisplay.fillCircle(x, y, r, color);
}

void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  sDisplay.drawTriangle(x0, y0, x1, y1, x2, y2, color);
}

void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  sDisplay.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

void setFont(const GFXfont *font)
{
  sDisplay.setFont(font);
}

void setTextSize(uint8_t size)
{
  sDisplay.setTextSize(size);
}

void setTextColor(uint16_t color)
{
  sDisplay.setTextColor(color);
}

void setCursor(int16_t x, int16_t y)
{
  sDisplay.setCursor(x, y);
}

void print(const char *text)
{
  sDisplay.print(text);
}
}
