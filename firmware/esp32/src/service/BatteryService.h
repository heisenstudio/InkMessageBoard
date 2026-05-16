#pragma once

#include <Arduino.h>
#include "../bsp/BspBattery.h"

#ifndef BATTERYSERVICE_POLL_INTERVAL_MS
#define BATTERYSERVICE_POLL_INTERVAL_MS 1000
#endif

#ifndef BATTERYSERVICE_TASK_STACK_SIZE
#define BATTERYSERVICE_TASK_STACK_SIZE 4096
#endif

#ifndef BATTERYSERVICE_TASK_PRIORITY
#define BATTERYSERVICE_TASK_PRIORITY 1
#endif

namespace BatteryService
{
struct State
{
  bool ready;
  BspBattery::Sample sample;
  uint32_t updatedAtMs;
};

bool begin();
bool update();
bool getState(State &state);
bool startTask();
}
