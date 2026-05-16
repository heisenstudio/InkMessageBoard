#pragma once

#include <Arduino.h>

namespace TimeService
{
bool begin();
bool isSynced();
uint32_t now();
}
