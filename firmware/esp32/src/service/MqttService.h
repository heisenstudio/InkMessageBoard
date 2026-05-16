#pragma once

#include <Arduino.h>

namespace MqttService
{
using MessageHandler = void (*)(const char *topic, const uint8_t *payload, unsigned int length);

void setMessageHandler(MessageHandler handler);
bool begin();
bool isConnected();
bool loop();
bool publishJson(const char *topic, const char *payload, bool retain = false);
void disconnect();
}
