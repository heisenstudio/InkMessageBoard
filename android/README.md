# Android App

这里是 InkMessageBoard 的 Android App 工程。

这个 App 是学习原型，不是完整产品。它主要用来测试 Android 端通过 MQTT 给 ESP32-S3 发送留言，并查看设备回传状态。

## 功能

- 填写 MQTT host、端口、用户名、密码和设备 ID
- 连接 MQTT Broker
- 发送留言到 ESP32-S3
- 发送刷新/清除命令
- 显示设备状态 topic 和事件日志

## 构建

见 [../docs/build_android.md](../docs/build_android.md)。

## 隐私

仓库里不应该包含真实 MQTT 密码、签名文件或 `local.properties`。

当前默认 MQTT 配置是占位符。请在本地填写自己的 EMQX Cloud 或其他 MQTT Broker 信息。
