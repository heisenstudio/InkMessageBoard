#pragma once

#include <Arduino.h>

namespace WifiService
{
bool begin();
bool isConnected();
bool isConfigured();
void disconnect();
String localIp();
int32_t rssi();
const char *statusText();
}
