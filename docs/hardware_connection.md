# Hardware Connection

这里先按当前固件里的引脚定义整理，后续如果硬件版本变化，需要同步检查原理图和代码。

## 主要硬件

| 部分 | 说明 |
| --- | --- |
| MCU | ESP32-S3 |
| 显示 | 2.66 寸黑白墨水屏，固件使用 `GxEPD2_260` |
| 电量检测 | INA226 |
| 输入 | 左右触摸按键 |
| 通信 | Wi-Fi + MQTT over TLS |

## 墨水屏 SPI

| 功能 | ESP32-S3 GPIO | 固件宏 |
| --- | --- | --- |
| SCK | GPIO33 | `BSP_EPAPER_SPI_SCK_PIN` |
| MOSI | GPIO34 | `BSP_EPAPER_SPI_MOSI_PIN` |
| MISO | 未使用 | `BSP_EPAPER_SPI_MISO_PIN = -1` |
| CS | GPIO47 | `BSP_EPAPER_CS_PIN` |
| DC | GPIO48 | `BSP_EPAPER_DC_PIN` |
| RST | GPIO35 | `BSP_EPAPER_RST_PIN` |
| BUSY | GPIO8 | `BSP_EPAPER_BUSY_PIN` |
| Power Enable | GPIO9 | `BSP_EPAPER_POWER_EN_PIN` |

## INA226 电量检测

| 功能 | ESP32-S3 GPIO | 固件宏 |
| --- | --- | --- |
| SDA | GPIO12 | `BSP_BATTERY_SDA_PIN` |
| SCL | GPIO13 | `BSP_BATTERY_SCL_PIN` |
| I2C 地址 | `0x40` | `BSP_BATTERY_I2C_ADDRESS` |

## 触摸按键

| 功能 | ESP32-S3 GPIO | 固件宏 |
| --- | --- | --- |
| 左触摸 | GPIO11 | `KEYSERVICE_LEFT_TOUCH_PIN` |
| 右触摸 | GPIO10 | `KEYSERVICE_RIGHT_TOUCH_PIN` |

## 注意

- 这些引脚来自当前固件，开源发布前最好再和 AD/嘉立创 EDA 原理图逐项核对。
- 墨水屏排线方向、电源使能、电池检测电阻值都建议在装配说明里继续补图。
- 如果你改板，请同步改 `firmware/esp32/src/bsp/` 和 `firmware/esp32/src/service/` 里的默认宏。
