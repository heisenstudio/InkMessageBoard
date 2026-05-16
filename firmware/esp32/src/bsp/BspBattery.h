#pragma once

#include <Arduino.h>

#ifndef BSP_BATTERY_SDA_PIN
#define BSP_BATTERY_SDA_PIN 12
#endif

#ifndef BSP_BATTERY_SCL_PIN
#define BSP_BATTERY_SCL_PIN 13
#endif

#ifndef BSP_BATTERY_I2C_ADDRESS
#define BSP_BATTERY_I2C_ADDRESS 0x40
#endif

#ifndef BSP_BATTERY_I2C_SPEED_HZ
#define BSP_BATTERY_I2C_SPEED_HZ 100000
#endif

#ifndef BSP_BATTERY_MAX_CURRENT_A
#define BSP_BATTERY_MAX_CURRENT_A 5.0f
#endif

#ifndef BSP_BATTERY_SHUNT_OHM
#define BSP_BATTERY_SHUNT_OHM 0.002f
#endif

namespace BspBattery
{
struct RawSample
{
  uint16_t shuntVoltage;
  uint16_t busVoltage;
  uint16_t power;
  uint16_t current;
};

struct Sample
{
  RawSample raw;
  float busVoltageV;
  float shuntVoltageMv;
  float currentA;
  float powerW;
  uint32_t timestampMs;
};

bool begin();
bool isReady();
bool readRaw(RawSample &sample);
bool read(Sample &sample);
float currentLsbA();
}
