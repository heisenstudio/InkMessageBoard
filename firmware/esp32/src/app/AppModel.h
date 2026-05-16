#pragma once

#include <Arduino.h>

namespace App
{
enum class Screen : uint8_t
{
  Boot,
  Idle,
  Message,
  ReplySent,
  Offline,
};

struct Model
{
  Screen screen;
  bool wifiConnected;
  bool batteryReady;
  uint32_t updatedAtMs;
};
}
