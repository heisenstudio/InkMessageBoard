#pragma once

#include <Arduino.h>

namespace UI
{
struct Model
{
  const char *title;
  const char *message;
  bool wifiConnected;
  bool batteryReady;
};
}
