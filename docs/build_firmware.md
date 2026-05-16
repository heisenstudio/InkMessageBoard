# Build Firmware

ESP32-S3 固件工程位于：

```text
firmware/esp32/
```

## 工具

- PlatformIO Core 或 VS Code PlatformIO 插件
- ESP32-S3 开发板串口驱动
- 可以访问 PlatformIO 依赖源的网络环境

## 配置

编辑：

```text
firmware/esp32/src/config/AppConfig.h
```

需要替换：

- `APP_WIFI_SSID`
- `APP_WIFI_PASSWORD`
- `APP_MQTT_HOST`
- `APP_MQTT_USERNAME`
- `APP_MQTT_PASSWORD`
- `APP_DEVICE_ID` 和相关 topic

不要把真实 Wi-Fi 密码、MQTT 密码提交到公开仓库。

## 编译

```bash
cd firmware/esp32
pio run
```

## 烧录

```bash
pio run --target upload
```

## 上传字体文件

固件使用 `data/fonts/` 下的 HZK 字体和映射文件，需要上传 LittleFS：

```bash
pio run --target uploadfs
```

## 串口监视

```bash
pio device monitor
```

## 检查点

- Wi-Fi 能连接
- MQTT 能连接到自己的 EMQX Cloud/Broker
- ESP32 订阅了留言和命令 topic
- App 能发出留言
- 墨水屏能刷新显示
- App 能收到 `accepted`、`displayed` 或按键回复状态
