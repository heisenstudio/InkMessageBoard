#include "WifiService.h"

#include <cstring>
#include <WiFi.h>
#include "../config/AppConfig.h"

namespace WifiService
{
namespace
{
bool sBeginAttempted = false;
const char *sStatus = "idle";

bool isPlaceholder(const char *value, const char *placeholder)
{
  return value == nullptr || value[0] == '\0' || strcmp(value, placeholder) == 0;
}
}

bool begin()
{
  sBeginAttempted = true;

  if (!isConfigured())
  {
    sStatus = "wifi config missing";
    Serial.println("[wifi] config missing");
    return false;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    sStatus = "connected";
    return true;
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(APP_WIFI_SSID, APP_WIFI_PASSWORD);

  Serial.print("[wifi] connecting");
  const uint32_t startedAtMs = millis();
  while (WiFi.status() != WL_CONNECTED &&
         millis() - startedAtMs < APP_WIFI_CONNECT_TIMEOUT_MS)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED)
  {
    sStatus = "connect failed";
    Serial.println("[wifi] connect failed");
    return false;
  }

  sStatus = "connected";
  Serial.print("[wifi] connected ip=");
  Serial.println(WiFi.localIP());
  return true;
}

bool isConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

bool isConfigured()
{
  return !isPlaceholder(APP_WIFI_SSID, "YOUR_WIFI_SSID");
}

void disconnect()
{
  if (WiFi.status() == WL_CONNECTED || sBeginAttempted)
  {
    WiFi.disconnect(true, false);
    WiFi.mode(WIFI_OFF);
  }
  sStatus = "off";
}

String localIp()
{
  if (!isConnected())
  {
    return String("-");
  }
  return WiFi.localIP().toString();
}

int32_t rssi()
{
  if (!isConnected())
  {
    return 0;
  }
  return WiFi.RSSI();
}

const char *statusText()
{
  if (isConnected())
  {
    return "connected";
  }
  return sStatus;
}
}
