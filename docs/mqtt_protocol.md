# MQTT Protocol

这个项目目前使用 MQTT 做 App 和 ESP32-S3 之间的通信。测试时使用过免费的 EMQX Cloud，端口为 `8883`，连接走 TLS。

仓库不提供真实 broker 地址、用户名和密码。请在本地填自己的配置。

## Topics

默认设备 ID 是 `001`。

| 方向 | Topic | 作用 |
| --- | --- | --- |
| App -> ESP32 | `ink/001/msg_to_esp32` | 发送留言 |
| App -> ESP32 | `ink/001/cmd_to_esp32` | 发送控制命令 |
| ESP32 -> App | `ink/001/status_to_phone` | 上报设备状态、留言状态和按键回复 |

## 留言消息

App 发布到 `ink/001/msg_to_esp32`：

```json
{
  "id": "phone-1778844537408",
  "type": "guestbook",
  "text": "今晚早点休息",
  "ts": 1778844537
}
```

字段：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| `id` | string | 是 | 消息 ID，用于去重和状态回传 |
| `type` | string | 否 | 当前 App 使用 `guestbook` |
| `text` | string | 是 | 显示到墨水屏上的文字 |
| `ts` | number | 否 | App 侧生成的 Unix 时间戳 |

## 控制命令

App 发布到 `ink/001/cmd_to_esp32`：

```json
{
  "id": "cmd-1778844537408",
  "cmd": "refresh"
}
```

当前命令：

| 命令 | 说明 |
| --- | --- |
| `refresh` | 强制刷新屏幕 |
| `clear` | 清除当前留言并刷新 |

## 设备状态

ESP32 发布到 `ink/001/status_to_phone`：

```json
{
  "online": true,
  "battery": 80,
  "rssi": -38,
  "fw": "1.0.0"
}
```

## 留言状态回传

ESP32 收到、显示或按键回复后，也发布到 `ink/001/status_to_phone`：

```json
{
  "msg_id": "phone-1778844537408",
  "state": "displayed"
}
```

带回复文本的例子：

```json
{
  "msg_id": "phone-1778844537408",
  "state": "seen",
  "text": "收到啦"
}
```

常见 `state`：

| state | 说明 |
| --- | --- |
| `accepted` | ESP32 已接收消息 |
| `displayed` | ESP32 已刷新到墨水屏 |
| `seen` | 左短按回复 |
| `miss_you` | 右短按回复 |
| `later` | 左长按回复 |
| `love_you` | 右长按回复 |

## 注意

- 免费 MQTT Broker 适合学习测试，不建议传隐私内容。
- 真实 host、用户名、密码不要提交到 GitHub。
- 当前固件使用 `setInsecure()`，也就是没有校验证书链，后续可以继续完善 TLS 证书校验。
