# 软件介绍

这个项目的软件部分分成两块：

- ESP32-S3 固件：负责联网、接收 MQTT 消息、驱动墨水屏、上报状态。
- Android App：负责输入留言、连接 MQTT Broker、发布消息和命令、显示设备状态。

整体上它是一个学习用的 IoT 原型，不是完整产品级架构。代码能作为参考，但仍然有很多地方可以继续整理。

## 通信流程

![三条 Topic 总体数据流](images/system_arch.png)

![留言发送与状态回执流程](images/mqtt_flow.png)

```text
Android App
   |
   | MQTT over TLS
   v
EMQX Cloud / MQTT Broker
   |
   | MQTT over TLS
   v
ESP32-S3
   |
   v
E-paper display
```

测试时使用过免费的 EMQX Cloud。仓库里不包含真实 host、用户名和密码，请在本地配置自己的 MQTT 信息。

## ESP32-S3 固件

固件位置：

```text
firmware/esp32/
```

主要模块：

| 模块 | 作用 |
| --- | --- |
| `main.cpp` | 主循环、消息解析、屏幕刷新调度 |
| `service/WifiService.*` | Wi-Fi 连接 |
| `service/MqttService.*` | MQTT TLS 连接、订阅、发布 |
| `service/KeyService.*` | 左右触摸按键 |
| `service/BatteryService.*` | 电量采样和状态 |
| `bsp/BspEpaper.*` | 墨水屏底层封装 |
| `bsp/BspBattery.*` | INA226 电量检测 |
| `data/fonts/` | HZK 点阵字体和 Unicode/GB2312 映射 |

固件当前使用 PlatformIO + Arduino framework。第三方库通过 `platformio.ini` 的 `lib_deps` 下载，不提交 `.pio` 缓存。

## Android App

App 位置：

```text
android/
```

主要模块：

| 模块 | 作用 |
| --- | --- |
| `MainActivity.kt` | Compose 页面和交互 |
| `GuestbookController.kt` | App 状态、MQTT 配置、消息发送 |
| `mqtt/SimpleMqttClient.kt` | 一个简单的 MQTT over TLS 客户端实现 |

App 目前比较直接：启动后使用配置连接 MQTT，留言页发布消息，状态页显示 topic 和设备回传事件，设置页可以修改 broker、账号、设备 ID 等信息。

## 还可以完善的方向

- 把 MQTT 配置做成更清晰的首次启动引导。
- 增加连接失败原因提示。
- App 端可以增加本地配置导入/导出。
- 固件可以增加更稳健的断线重连策略。
- 固件中文字符串和字体资源还可以继续整理。
- 可以把系统架构图和 MQTT 流程图补成图片。
