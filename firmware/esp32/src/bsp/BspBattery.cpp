#include "BspBattery.h"

#include <Wire.h>
#include "INA226.h"

#define BSP_BATTERY_REG_SHUNT_VOLTAGE 0x01
#define BSP_BATTERY_REG_BUS_VOLTAGE 0x02
#define BSP_BATTERY_REG_POWER 0x03
#define BSP_BATTERY_REG_CURRENT 0x04
#define BSP_BATTERY_REG_MANUFACTURER 0xFE
#define BSP_BATTERY_REG_DIE_ID 0xFF

namespace BspBattery
{
namespace
{
INA226 sIna226(BSP_BATTERY_I2C_ADDRESS, &Wire);
bool sReady = false;

bool readRegister(uint8_t reg, uint16_t &value)
{
  Wire.beginTransmission(BSP_BATTERY_I2C_ADDRESS);
  Wire.write(reg);
  if (Wire.endTransmission() != 0)
  {
    return false;
  }

  const uint8_t bytesRead = Wire.requestFrom(
      static_cast<uint8_t>(BSP_BATTERY_I2C_ADDRESS),
      static_cast<uint8_t>(2));
  if (bytesRead != 2)
  {
    while (Wire.available() > 0)
    {
      Wire.read();
    }
    return false;
  }

  value = static_cast<uint16_t>(Wire.read()) << 8;
  value |= static_cast<uint16_t>(Wire.read());
  return true;
}
}

bool begin()
{
  Wire.end();
  pinMode(BSP_BATTERY_SDA_PIN, INPUT_PULLUP);
  pinMode(BSP_BATTERY_SCL_PIN, INPUT_PULLUP);

  if (!Wire.begin(BSP_BATTERY_SDA_PIN, BSP_BATTERY_SCL_PIN, BSP_BATTERY_I2C_SPEED_HZ))
  {
    Serial.println("I2C begin failed");
    sReady = false;
    return false;
  }
  Wire.setTimeOut(50);
  delay(10);

  if (!sIna226.begin())
  {
    Serial.println("INA226 not connected");
    sReady = false;
    return false;
  }

  uint16_t manufacturer = 0;
  uint16_t die = 0;
  if (readRegister(BSP_BATTERY_REG_MANUFACTURER, manufacturer) &&
      readRegister(BSP_BATTERY_REG_DIE_ID, die))
  {
    Serial.print("INA226 manufacturer=0x");
    Serial.print(manufacturer, HEX);
    Serial.print(" die=0x");
    Serial.println(die, HEX);
  }

  const int calibrationError = sIna226.setMaxCurrentShunt(BSP_BATTERY_MAX_CURRENT_A, BSP_BATTERY_SHUNT_OHM);
  if (calibrationError != INA226_ERR_NONE)
  {
    Serial.print("INA226 calibration error=0x");
    Serial.println(calibrationError, HEX);
    sReady = false;
    return false;
  }

  Serial.print("INA226 max current=");
  Serial.print(sIna226.getMaxCurrent(), 3);
  Serial.print("A shunt=");
  Serial.print(sIna226.getShunt(), 4);
  Serial.print("R current_lsb=");
  Serial.print(sIna226.getCurrentLSB_mA(), 6);
  Serial.println("mA");

  sReady = true;
  return true;
}

bool isReady()
{
  return sReady;
}

bool readRaw(RawSample &sample)
{
  if (!sReady && !begin())
  {
    return false;
  }

  if (!readRegister(BSP_BATTERY_REG_SHUNT_VOLTAGE, sample.shuntVoltage) ||
      !readRegister(BSP_BATTERY_REG_BUS_VOLTAGE, sample.busVoltage) ||
      !readRegister(BSP_BATTERY_REG_POWER, sample.power) ||
      !readRegister(BSP_BATTERY_REG_CURRENT, sample.current))
  {
    Serial.println("INA226 read failed");
    sReady = false;
    return false;
  }

  return true;
}

bool read(Sample &sample)
{
  RawSample raw = {};
  if (!readRaw(raw))
  {
    return false;
  }

  sample.raw = raw;
  sample.busVoltageV = raw.busVoltage * 1.25e-3f;
  sample.shuntVoltageMv = static_cast<int16_t>(raw.shuntVoltage) * 2.5e-3f;
  sample.currentA = static_cast<int16_t>(raw.current) * sIna226.getCurrentLSB();
  sample.powerW = raw.power * sIna226.getCurrentLSB() * 25.0f;
  sample.timestampMs = millis();
  return true;
}

float currentLsbA()
{
  return sIna226.getCurrentLSB();
}
}
