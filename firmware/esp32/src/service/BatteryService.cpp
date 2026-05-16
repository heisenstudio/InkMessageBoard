#include "BatteryService.h"

#include <freertos/task.h>

namespace BatteryService
{
namespace
{
State sState = {};
TaskHandle_t sTaskHandle = nullptr;
bool sBeginAttempted = false;

void printSample(const BspBattery::Sample &sample)
{
  Serial.print("BatteryService current=");
  Serial.print(sample.currentA, 3);
  Serial.print("A bus=");
  Serial.print(sample.busVoltageV, 3);
  Serial.print("V shunt=");
  Serial.print(sample.shuntVoltageMv, 3);
  Serial.print("mV power=");
  Serial.print(sample.powerW, 3);
  Serial.println("W");
}

void batteryTask(void *pvParameters)
{
  (void)pvParameters;

  for (;;)
  {
    update();
    vTaskDelay(pdMS_TO_TICKS(BATTERYSERVICE_POLL_INTERVAL_MS));
  }
}
}

bool begin()
{
  sBeginAttempted = true;
  if (!BspBattery::begin())
  {
    sState.ready = false;
    return false;
  }

  return update();
}

bool update()
{
  if (!BspBattery::isReady())
  {
    sBeginAttempted = true;
    if (!BspBattery::begin())
    {
      sState.ready = false;
      return false;
    }
  }

  BspBattery::Sample sample = {};
  if (!BspBattery::read(sample))
  {
    sState.ready = false;
    return false;
  }

  sState.ready = true;
  sState.sample = sample;
  sState.updatedAtMs = millis();
  printSample(sample);
  return true;
}

bool getState(State &state)
{
  state = sState;
  return sState.ready;
}

bool startTask()
{
  if (sTaskHandle != nullptr)
  {
    return true;
  }

  if (!sBeginAttempted)
  {
    begin();
  }

  const BaseType_t result = xTaskCreate(
      batteryTask,
      "battery_service",
      BATTERYSERVICE_TASK_STACK_SIZE,
      nullptr,
      BATTERYSERVICE_TASK_PRIORITY,
      &sTaskHandle);

  if (result != pdPASS)
  {
    Serial.println("BatteryService task create failed");
    sTaskHandle = nullptr;
    return false;
  }

  return true;
}
}
