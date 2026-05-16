#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "bsp/BspEpaper.h"
#include "config/AppConfig.h"
#include "service/BatteryService.h"
#include "service/KeyService.h"
#include "service/MqttService.h"
#include "service/WifiService.h"

namespace
{
struct HzkFont
{
  const char *path;
  uint8_t size;
  uint8_t bytesPerRow;
  uint8_t bytesPerGlyph;
};

struct AppState
{
  String messageId;
  String messageText;
  uint32_t messageTs;
  bool hasMessage;
  String lastMessageId;
  String lastCommandId;
  String lastReplyText;
  int batteryPercentage;
  float batteryVoltageV;
  bool batteryReady;
  bool displayDirty;
  String pendingDisplayedId;
};

constexpr HzkFont kHzk16 = {"/fonts/HZK16", 16, 2, 32};
constexpr HzkFont kHzk24 = {"/fonts/HZK24", 24, 3, 72};
constexpr const char *kMapPath = "/fonts/unicode_gb2312.idx";
constexpr uint32_t kInitialWifiAttemptDelayMs = 100;

AppState sState = {};
bool sLittleFsReady = false;
bool sMqttWasConnected = false;
uint32_t sLastWifiAttemptMs = 0;

uint8_t clampBatteryPercentage(float voltage)
{
  if (voltage <= APP_BATTERY_EMPTY_V)
  {
    return 0;
  }
  if (voltage >= APP_BATTERY_FULL_V)
  {
    return 100;
  }

  const float range = APP_BATTERY_FULL_V - APP_BATTERY_EMPTY_V;
  return static_cast<uint8_t>(((voltage - APP_BATTERY_EMPTY_V) * 100.0f / range) + 0.5f);
}

void updateBattery()
{
  if (!BatteryService::begin())
  {
    sState.batteryReady = false;
    sState.batteryPercentage = 0;
    sState.batteryVoltageV = 0.0f;
    return;
  }

  BatteryService::State battery = {};
  if (!BatteryService::getState(battery))
  {
    sState.batteryReady = false;
    sState.batteryPercentage = 0;
    sState.batteryVoltageV = 0.0f;
    return;
  }

  sState.batteryReady = true;
  sState.batteryVoltageV = battery.sample.busVoltageV;
  sState.batteryPercentage = clampBatteryPercentage(battery.sample.busVoltageV);
}

void drawAscii(const char *text, int16_t x, int16_t y, uint8_t size = 1)
{
  BspEpaper::setFont(nullptr);
  BspEpaper::setTextSize(size);
  BspEpaper::setTextColor(GxEPD_BLACK);
  BspEpaper::setCursor(x, y);
  BspEpaper::print(text);
}

void drawMissingGlyph(int16_t x, int16_t y, uint8_t size)
{
  BspEpaper::drawRect(x, y, size, size, GxEPD_BLACK);
  BspEpaper::drawLine(x, y, x + size - 1, y + size - 1, GxEPD_BLACK);
  BspEpaper::drawLine(x, y + size - 1, x + size - 1, y, GxEPD_BLACK);
}

bool readGlyph(const HzkFont &font, uint8_t high, uint8_t low, uint8_t *glyph)
{
  if (high < 0xA1 || low < 0xA1)
  {
    return false;
  }

  const uint32_t area = static_cast<uint32_t>(high - 0xA0);
  const uint32_t position = static_cast<uint32_t>(low - 0xA0);
  if (area == 0 || area > 94 || position == 0 || position > 94)
  {
    return false;
  }

  const uint32_t index = (area - 1) * 94 + (position - 1);
  const uint32_t offset = index * font.bytesPerGlyph;

  File file = LittleFS.open(font.path, "r");
  if (!file)
  {
    return false;
  }

  if (offset + font.bytesPerGlyph > file.size() || !file.seek(offset, SeekSet))
  {
    file.close();
    return false;
  }

  const size_t readBytes = file.read(glyph, font.bytesPerGlyph);
  file.close();
  return readBytes == font.bytesPerGlyph;
}

void drawGlyph(const HzkFont &font, int16_t x, int16_t y, uint8_t high, uint8_t low)
{
  uint8_t glyph[72] = {};
  if (!readGlyph(font, high, low, glyph))
  {
    drawMissingGlyph(x, y, font.size);
    return;
  }

  for (uint8_t row = 0; row < font.size; ++row)
  {
    for (uint8_t byteIndex = 0; byteIndex < font.bytesPerRow; ++byteIndex)
    {
      const uint8_t bits = glyph[row * font.bytesPerRow + byteIndex];
      for (uint8_t bit = 0; bit < 8; ++bit)
      {
        const uint8_t col = byteIndex * 8 + bit;
        if (col < font.size && (bits & (0x80 >> bit)) != 0)
        {
          BspEpaper::drawPixel(x + col, y + row, GxEPD_BLACK);
        }
      }
    }
  }
}

bool lookupGb2312(uint32_t codepoint, uint8_t &high, uint8_t &low)
{
  if (codepoint > 0xFFFF)
  {
    return false;
  }

  File file = LittleFS.open(kMapPath, "r");
  if (!file)
  {
    return false;
  }

  const uint32_t offset = codepoint * 2;
  if (offset + 2 > file.size() || !file.seek(offset, SeekSet))
  {
    file.close();
    return false;
  }

  uint8_t bytes[2] = {};
  const size_t readBytes = file.read(bytes, sizeof(bytes));
  file.close();
  if (readBytes != sizeof(bytes))
  {
    return false;
  }

  const uint16_t gbValue = static_cast<uint16_t>(bytes[0]) |
                           (static_cast<uint16_t>(bytes[1]) << 8);
  if (gbValue == 0)
  {
    return false;
  }

  high = static_cast<uint8_t>(gbValue >> 8);
  low = static_cast<uint8_t>(gbValue & 0xFF);
  return high >= 0xA1 && high <= 0xF7 && low >= 0xA1 && low <= 0xFE;
}

bool decodeNextUtf8(const String &text, size_t &index, uint32_t &codepoint)
{
  if (index >= static_cast<size_t>(text.length()))
  {
    return false;
  }

  const uint8_t first = static_cast<uint8_t>(text[index++]);
  if (first < 0x80)
  {
    codepoint = first;
    return true;
  }

  uint8_t expected = 0;
  uint32_t value = 0;
  if ((first & 0xE0) == 0xC0)
  {
    expected = 1;
    value = first & 0x1F;
  }
  else if ((first & 0xF0) == 0xE0)
  {
    expected = 2;
    value = first & 0x0F;
  }
  else if ((first & 0xF8) == 0xF0)
  {
    expected = 3;
    value = first & 0x07;
  }
  else
  {
    codepoint = '?';
    return true;
  }

  for (uint8_t i = 0; i < expected; ++i)
  {
    if (index >= static_cast<size_t>(text.length()))
    {
      codepoint = '?';
      return true;
    }

    const uint8_t next = static_cast<uint8_t>(text[index++]);
    if ((next & 0xC0) != 0x80)
    {
      codepoint = '?';
      return true;
    }
    value = (value << 6) | (next & 0x3F);
  }

  codepoint = value;
  return true;
}

int16_t drawUtf8Text(const HzkFont &font,
                     const String &text,
                     int16_t x,
                     int16_t y,
                     int16_t maxX,
                     int16_t maxY)
{
  const int16_t startX = x;
  const int16_t lineStep = font.size + 6;
  int16_t cursorX = x;
  int16_t cursorY = y;
  size_t index = 0;

  while (index < static_cast<size_t>(text.length()) && cursorY + font.size <= maxY)
  {
    uint32_t codepoint = 0;
    if (!decodeNextUtf8(text, index, codepoint))
    {
      break;
    }

    if (codepoint == '\n')
    {
      cursorX = startX;
      cursorY += lineStep;
      continue;
    }

    const bool ascii = codepoint >= 0x20 && codepoint <= 0x7E;
    const int16_t charWidth = ascii ? 7 : font.size + 2;
    if (cursorX + charWidth > maxX)
    {
      cursorX = startX;
      cursorY += lineStep;
      if (cursorY + font.size > maxY)
      {
        break;
      }
    }

    if (ascii)
    {
      char asciiText[2] = {static_cast<char>(codepoint), '\0'};
      drawAscii(asciiText, cursorX, cursorY + max<int16_t>((font.size - 8) / 2, 0));
    }
    else
    {
      uint8_t high = 0;
      uint8_t low = 0;
      if (lookupGb2312(codepoint, high, low))
      {
        drawGlyph(font, cursorX, cursorY, high, low);
      }
      else
      {
        drawMissingGlyph(cursorX, cursorY, font.size);
      }
    }

    cursorX += charWidth;
  }

  return cursorY + font.size;
}

void drawBatteryIcon(int16_t x, int16_t y, int percentage)
{
  BspEpaper::drawRect(x, y, 24, 12, GxEPD_BLACK);
  BspEpaper::fillRect(x + 24, y + 4, 3, 4, GxEPD_BLACK);
  const int fillW = constrain((percentage * 20) / 100, 0, 20);
  if (fillW > 0)
  {
    BspEpaper::fillRect(x + 2, y + 2, fillW, 8, GxEPD_BLACK);
  }
}

void drawStatusBar()
{
  const int16_t w = BspEpaper::width();
  BspEpaper::drawFastHLine(6, 23, w - 12, GxEPD_BLACK);
  drawAscii("INK 001", 8, 8);
  drawAscii(MqttService::isConnected() ? "MQTT" : "OFF", 66, 8);

  char info[32] = {};
  snprintf(info, sizeof(info), "RSSI %ld", static_cast<long>(WifiService::rssi()));
  drawAscii(info, 112, 8);

  drawBatteryIcon(w - 38, 6, sState.batteryPercentage);
}

void drawReplyHints()
{
  const int16_t y = BspEpaper::height() - 20;
  BspEpaper::drawFastHLine(6, y - 7, BspEpaper::width() - 12, GxEPD_BLACK);
  drawUtf8Text(kHzk16, "左短 收到啦  右短 想你", 10, y, BspEpaper::width() - 8, BspEpaper::height() - 2);
}

void drawMessagePage()
{
  const int16_t w = BspEpaper::width();
  const int16_t h = BspEpaper::height();
  BspEpaper::fillScreen(GxEPD_WHITE);
  drawStatusBar();

  if (sState.hasMessage)
  {
    drawUtf8Text(kHzk24, sState.messageText, 12, 36, w - 10, h - 54);

    if (sState.lastReplyText.length() > 0)
    {
      String reply = "已回复：";
      reply += sState.lastReplyText;
      drawUtf8Text(kHzk16, reply, 12, h - 44, w - 10, h - 26);
    }
  }
  else
  {
    drawUtf8Text(kHzk24, "等待留言", 74, 58, w - 10, h - 48);
  }

  drawReplyHints();
}

bool renderDisplay()
{
  Serial.println("[display] refresh");
  BspEpaper::begin();
  BspEpaper::setRotation(APP_DISPLAY_ROTATION);
  BspEpaper::setFullWindow();
  BspEpaper::firstPage();
  do
  {
    if (!sLittleFsReady)
    {
      BspEpaper::fillScreen(GxEPD_WHITE);
      drawAscii("LittleFS mount failed", 10, 40);
      drawAscii("Upload filesystem.", 10, 58);
    }
    else
    {
      drawMessagePage();
    }
  } while (BspEpaper::nextPage());
  BspEpaper::powerOff();
  Serial.println("[display] done");
  return true;
}

bool publishJsonDocument(const JsonDocument &doc, bool retain = false)
{
  char payload[512] = {};
  const size_t length = serializeJson(doc, payload, sizeof(payload));
  if (length == 0 || length >= sizeof(payload))
  {
    Serial.println("[mqtt] serialize failed");
    return false;
  }
  return MqttService::publishJson(APP_MQTT_TOPIC_STATUS_TO_PHONE, payload, retain);
}

void publishOnlineStatus()
{
  updateBattery();

  JsonDocument doc;
  doc["online"] = true;
  doc["battery"] = sState.batteryReady ? sState.batteryPercentage : 0;
  doc["rssi"] = WifiService::rssi();
  doc["fw"] = APP_FIRMWARE_VERSION;
  publishJsonDocument(doc, true);
}

void publishMessageState(const String &msgId, const char *state)
{
  if (msgId.length() == 0)
  {
    return;
  }

  JsonDocument doc;
  doc["msg_id"] = msgId;
  doc["state"] = state;
  publishJsonDocument(doc, false);
}

void publishReply(const char *state, const char *text)
{
  if (!sState.hasMessage)
  {
    Serial.println("[key] ignored, no active message");
    return;
  }

  JsonDocument doc;
  doc["msg_id"] = sState.messageId;
  doc["state"] = state;
  doc["text"] = text;
  publishJsonDocument(doc, false);

  sState.lastReplyText = text;
  sState.displayDirty = true;
}

String payloadToString(const uint8_t *payload, unsigned int length)
{
  String text;
  text.reserve(length + 1);
  for (unsigned int i = 0; i < length; ++i)
  {
    text += static_cast<char>(payload[i]);
  }
  return text;
}

void handleIncomingMessage(const uint8_t *payload, unsigned int length)
{
  const String body = payloadToString(payload, length);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error)
  {
    Serial.print("[msg] json error: ");
    Serial.println(error.c_str());
    return;
  }

  const char *id = doc["id"] | "";
  const char *type = doc["type"] | "";
  const char *text = doc["text"] | "";
  const uint32_t ts = doc["ts"] | 0;

  if (id[0] == '\0' || text[0] == '\0')
  {
    Serial.println("[msg] missing id/text");
    return;
  }

  if (sState.lastMessageId == id)
  {
    Serial.print("[msg] duplicate ignored id=");
    Serial.println(id);
    return;
  }

  sState.messageId = id;
  sState.lastMessageId = id;
  sState.messageText = text;
  sState.messageTs = ts;
  sState.hasMessage = true;
  sState.lastReplyText = "";
  sState.displayDirty = true;
  sState.pendingDisplayedId = id;

  Serial.print("[msg] accepted id=");
  Serial.print(id);
  Serial.print(" type=");
  Serial.println(type);
  publishMessageState(sState.messageId, "accepted");
}

void handleIncomingCommand(const uint8_t *payload, unsigned int length)
{
  const String body = payloadToString(payload, length);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error)
  {
    Serial.print("[cmd] json error: ");
    Serial.println(error.c_str());
    return;
  }

  const char *id = doc["id"] | "";
  const char *cmd = doc["cmd"] | "";
  if (id[0] == '\0' || cmd[0] == '\0')
  {
    Serial.println("[cmd] missing id/cmd");
    return;
  }

  if (sState.lastCommandId == id)
  {
    Serial.print("[cmd] duplicate ignored id=");
    Serial.println(id);
    return;
  }
  sState.lastCommandId = id;

  if (strcmp(cmd, "refresh") == 0)
  {
    sState.displayDirty = true;
    Serial.println("[cmd] refresh");
  }
  else if (strcmp(cmd, "clear") == 0)
  {
    sState.messageId = "";
    sState.messageText = "";
    sState.pendingDisplayedId = "";
    sState.hasMessage = false;
    sState.lastReplyText = "";
    sState.displayDirty = true;
    Serial.println("[cmd] clear");
  }
}

void handleMqttMessage(const char *topic, const uint8_t *payload, unsigned int length)
{
  if (strcmp(topic, APP_MQTT_TOPIC_MSG_TO_ESP32) == 0)
  {
    handleIncomingMessage(payload, length);
  }
  else if (strcmp(topic, APP_MQTT_TOPIC_CMD_TO_ESP32) == 0)
  {
    handleIncomingCommand(payload, length);
  }
}

void maintainWifi()
{
  if (WifiService::isConnected())
  {
    return;
  }

  const uint32_t nowMs = millis();
  if (sLastWifiAttemptMs != 0 &&
      nowMs - sLastWifiAttemptMs < APP_WIFI_RETRY_INTERVAL_MS)
  {
    return;
  }

  sLastWifiAttemptMs = nowMs == 0 ? kInitialWifiAttemptDelayMs : nowMs;
  WifiService::begin();
}

void maintainMqtt()
{
  if (!WifiService::isConnected())
  {
    sMqttWasConnected = false;
    return;
  }

  MqttService::loop();
  const bool connected = MqttService::isConnected();
  if (connected && !sMqttWasConnected)
  {
    Serial.println("[mqtt] online event");
    publishOnlineStatus();
    sState.displayDirty = true;
  }
  sMqttWasConnected = connected;
}

void handleKeyEvent(const KeyService::KeyEvent &event)
{
  if (event.key == KeyService::KeyId::Left && event.action == KeyService::KeyAction::ShortPress)
  {
    publishReply("seen", "收到啦");
  }
  else if (event.key == KeyService::KeyId::Right && event.action == KeyService::KeyAction::ShortPress)
  {
    publishReply("miss_you", "想你");
  }
  else if (event.key == KeyService::KeyId::Left && event.action == KeyService::KeyAction::LongPress)
  {
    publishReply("later", "嘬嘬");
  }
  else if (event.key == KeyService::KeyId::Right && event.action == KeyService::KeyAction::LongPress)
  {
    publishReply("love_you", "爱你");
  }
}

void processKeys()
{
  KeyService::KeyEvent event = {};
  while (KeyService::getEvent(event, 0))
  {
    handleKeyEvent(event);
  }
}

void processDisplay()
{
  if (!sState.displayDirty)
  {
    return;
  }

  sState.displayDirty = false;
  const String displayedId = sState.pendingDisplayedId;
  const bool rendered = renderDisplay();
  if (rendered && displayedId.length() > 0)
  {
    publishMessageState(displayedId, "displayed");
    sState.pendingDisplayedId = "";
  }
}
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("ink mqtt terminal");

  sLittleFsReady = LittleFS.begin(false);
  Serial.print("[fs] LittleFS=");
  Serial.println(sLittleFsReady ? "ok" : "fail");

  updateBattery();
  KeyService::begin();
  MqttService::setMessageHandler(handleMqttMessage);

  sState.displayDirty = true;
  processDisplay();
}

void loop()
{
  maintainWifi();
  maintainMqtt();
  processKeys();
  processDisplay();
  delay(10);
}
