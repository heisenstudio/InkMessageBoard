#pragma once

// Replace these placeholders with your own local values before building.
// Do not publish real Wi-Fi or MQTT credentials.

#define APP_WIFI_SSID "YOUR_WIFI_SSID"
#define APP_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define APP_WIFI_CONNECT_TIMEOUT_MS 15000UL
#define APP_WIFI_RETRY_INTERVAL_MS 5000UL

#define APP_DEVICE_ID "001"
#define APP_FIRMWARE_VERSION "1.0.0"

// Free EMQX Cloud can be used for learning. Use your own broker host here.
#define APP_MQTT_HOST "YOUR_EMQX_CLOUD_HOST.emqxsl.com"
#define APP_MQTT_PORT 8883
#define APP_MQTT_CLIENT_ID "ink-board-001"
#define APP_MQTT_USERNAME "YOUR_MQTT_USERNAME"
#define APP_MQTT_PASSWORD "YOUR_MQTT_PASSWORD"
#define APP_MQTT_TOPIC_MSG_TO_ESP32 "ink/001/msg_to_esp32"
#define APP_MQTT_TOPIC_CMD_TO_ESP32 "ink/001/cmd_to_esp32"
#define APP_MQTT_TOPIC_STATUS_TO_PHONE "ink/001/status_to_phone"
#define APP_MQTT_SUBSCRIBE_QOS 1
#define APP_MQTT_PUBLISH_QOS 1
#define APP_MQTT_CONNECT_RETRY_MS 5000UL
#define APP_MQTT_KEEPALIVE_SEC 60
#define APP_MQTT_SOCKET_TIMEOUT_SEC 15
#define APP_MQTT_BUFFER_SIZE 1536

#define APP_DISPLAY_ROTATION 1
#define APP_BATTERY_EMPTY_V 3.30f
#define APP_BATTERY_FULL_V 4.20f
