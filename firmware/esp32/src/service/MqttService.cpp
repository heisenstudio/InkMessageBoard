#include "MqttService.h"

#include <Arduino.h>
#include <cstring>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "../config/AppConfig.h"

namespace MqttService
{
namespace
{
WiFiClientSecure sSecureClient;
PubSubClient sClient(sSecureClient);
MessageHandler sMessageHandler = nullptr;
bool sConfigured = false;
uint32_t sLastConnectAttemptMs = 0;
uint16_t sNextPacketId = 1;

constexpr char kOfflineWill[] = "{\"online\":false}";

bool hasText(const char *value)
{
  return value != nullptr && value[0] != '\0';
}

String defaultClientId()
{
  if (hasText(APP_MQTT_CLIENT_ID))
  {
    return String(APP_MQTT_CLIENT_ID);
  }

  uint64_t mac = ESP.getEfuseMac();
  String clientId = "esp32-s3-";
  clientId += String(static_cast<uint32_t>(mac >> 32), HEX);
  clientId += String(static_cast<uint32_t>(mac), HEX);
  return clientId;
}

void onMessage(char *topic, uint8_t *payload, unsigned int length)
{
  Serial.print("[mqtt] rx topic=");
  Serial.print(topic);
  Serial.print(" length=");
  Serial.println(length);

  if (sMessageHandler != nullptr)
  {
    sMessageHandler(topic, payload, length);
  }
}

void configureClient()
{
  if (sConfigured)
  {
    return;
  }

  sSecureClient.setInsecure();
  sSecureClient.setTimeout(APP_MQTT_SOCKET_TIMEOUT_SEC);
  sSecureClient.setHandshakeTimeout(APP_MQTT_SOCKET_TIMEOUT_SEC);

  sClient.setServer(APP_MQTT_HOST, APP_MQTT_PORT);
  sClient.setCallback(onMessage);
  sClient.setKeepAlive(APP_MQTT_KEEPALIVE_SEC);
  sClient.setSocketTimeout(APP_MQTT_SOCKET_TIMEOUT_SEC);
  sClient.setBufferSize(APP_MQTT_BUFFER_SIZE);

  sConfigured = true;
  Serial.println("[mqtt] TLS verification disabled");
}

bool subscribeTopics()
{
  const bool msgOk = sClient.subscribe(APP_MQTT_TOPIC_MSG_TO_ESP32, APP_MQTT_SUBSCRIBE_QOS);
  const bool cmdOk = sClient.subscribe(APP_MQTT_TOPIC_CMD_TO_ESP32, APP_MQTT_SUBSCRIBE_QOS);

  Serial.print("[mqtt] subscribe msg=");
  Serial.print(msgOk ? "ok" : "fail");
  Serial.print(" cmd=");
  Serial.println(cmdOk ? "ok" : "fail");
  return msgOk && cmdOk;
}

bool connectOnce()
{
  const String clientId = defaultClientId();

  Serial.print("[mqtt] connecting ");
  Serial.print(APP_MQTT_HOST);
  Serial.print(":");
  Serial.print(APP_MQTT_PORT);
  Serial.print(" clientId=");
  Serial.println(clientId);

  bool connected = false;
  if (hasText(APP_MQTT_USERNAME))
  {
    connected = sClient.connect(
        clientId.c_str(),
        APP_MQTT_USERNAME,
        APP_MQTT_PASSWORD,
        APP_MQTT_TOPIC_STATUS_TO_PHONE,
        1,
        true,
        kOfflineWill);
  }
  else
  {
    connected = sClient.connect(
        clientId.c_str(),
        APP_MQTT_TOPIC_STATUS_TO_PHONE,
        1,
        true,
        kOfflineWill);
  }

  if (!connected)
  {
    Serial.print("[mqtt] connect failed state=");
    Serial.println(sClient.state());
    return false;
  }

  Serial.println("[mqtt] connected");
  if (!subscribeTopics())
  {
    sClient.disconnect();
    return false;
  }

  return true;
}

bool ensureConnected()
{
  configureClient();

  if (sClient.connected())
  {
    return true;
  }

  const uint32_t nowMs = millis();
  if (sLastConnectAttemptMs != 0 &&
      nowMs - sLastConnectAttemptMs < APP_MQTT_CONNECT_RETRY_MS)
  {
    return false;
  }

  sLastConnectAttemptMs = nowMs;
  return connectOnce();
}

size_t encodeRemainingLength(uint32_t length, uint8_t *out)
{
  size_t written = 0;
  do
  {
    uint8_t encoded = length % 128;
    length /= 128;
    if (length > 0)
    {
      encoded |= 0x80;
    }
    out[written++] = encoded;
  } while (length > 0 && written < 4);
  return written;
}

bool writeAll(const uint8_t *data, size_t length)
{
  return sSecureClient.write(data, length) == length;
}

bool writeByte(uint8_t value)
{
  return sSecureClient.write(&value, 1) == 1;
}

bool writeMqttString(const char *value)
{
  const size_t length = strlen(value);
  if (length > 0xFFFF)
  {
    return false;
  }

  const uint8_t prefix[2] = {
      static_cast<uint8_t>(length >> 8),
      static_cast<uint8_t>(length & 0xFF),
  };
  return writeAll(prefix, sizeof(prefix)) &&
         writeAll(reinterpret_cast<const uint8_t *>(value), length);
}

uint16_t nextPacketId()
{
  if (sNextPacketId == 0)
  {
    sNextPacketId = 1;
  }
  return sNextPacketId++;
}
}

void setMessageHandler(MessageHandler handler)
{
  sMessageHandler = handler;
}

bool begin()
{
  return ensureConnected();
}

bool isConnected()
{
  return sClient.connected();
}

bool loop()
{
  if (!ensureConnected())
  {
    return false;
  }

  sClient.loop();
  return true;
}

bool publishJson(const char *topic, const char *payload, bool retain)
{
  if (!ensureConnected() || topic == nullptr || payload == nullptr)
  {
    return false;
  }

  const size_t topicLength = strlen(topic);
  const size_t payloadLength = strlen(payload);
  if (topicLength > 0xFFFF)
  {
    return false;
  }

  const uint16_t packetId = nextPacketId();
  const uint32_t remainingLength = 2 + topicLength + 2 + payloadLength;
  uint8_t remaining[4] = {};
  const size_t remainingBytes = encodeRemainingLength(remainingLength, remaining);
  const uint8_t fixedHeader = 0x30 | 0x02 | (retain ? 0x01 : 0x00);
  const uint8_t packetIdBytes[2] = {
      static_cast<uint8_t>(packetId >> 8),
      static_cast<uint8_t>(packetId & 0xFF),
  };

  const bool ok = writeByte(fixedHeader) &&
                  writeAll(remaining, remainingBytes) &&
                  writeMqttString(topic) &&
                  writeAll(packetIdBytes, sizeof(packetIdBytes)) &&
                  writeAll(reinterpret_cast<const uint8_t *>(payload), payloadLength);

  Serial.print("[mqtt] tx qos1 topic=");
  Serial.print(topic);
  Serial.print(" retain=");
  Serial.print(retain ? "true" : "false");
  Serial.print(" ok=");
  Serial.println(ok ? "true" : "false");
  return ok;
}

void disconnect()
{
  if (sClient.connected())
  {
    sClient.disconnect();
  }
  sSecureClient.stop();
}
}
