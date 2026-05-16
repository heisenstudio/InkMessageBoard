# Build Android App

Android 工程位于：

```text
android/
```

## 工具

- Android Studio
- 项目自带 Gradle Wrapper
- JDK 使用 Android Studio 自带版本即可

## 打开工程

用 Android Studio 打开 `android/` 目录。

不要提交 `local.properties`，里面会包含你本机 Android SDK 路径。

## MQTT 配置

App 默认值已经改成占位符。运行后可以在设置页填写自己的 MQTT 信息：

| 项目 | 说明 |
| --- | --- |
| MQTT Host | 你的 EMQX Cloud 或其他 broker 地址 |
| Port | TLS 通常为 `8883` |
| Username | 你的 MQTT 用户名 |
| Password | 你的 MQTT 密码 |
| Device ID | 默认 `001` |

## 编译

Windows：

```bat
cd android
gradlew.bat assembleDebug
```

macOS/Linux：

```bash
cd android
./gradlew assembleDebug
```

## 检查点

- App 能正常启动
- 设置页能填 MQTT 配置
- 状态页能显示连接日志
- 留言页能发布消息
- ESP32 能收到消息并刷新墨水屏
