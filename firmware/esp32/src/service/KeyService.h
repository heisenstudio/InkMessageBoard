#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#ifndef KEYSERVICE_LEFT_TOUCH_PIN
#define KEYSERVICE_LEFT_TOUCH_PIN 11
#endif

#ifndef KEYSERVICE_RIGHT_TOUCH_PIN
#define KEYSERVICE_RIGHT_TOUCH_PIN 10
#endif

#ifndef KEYSERVICE_LEFT_TOUCH_THRESHOLD
#define KEYSERVICE_LEFT_TOUCH_THRESHOLD 64000
#endif

#ifndef KEYSERVICE_RIGHT_TOUCH_THRESHOLD
#define KEYSERVICE_RIGHT_TOUCH_THRESHOLD 64000
#endif

#ifndef KEYSERVICE_TOUCH_ACTIVE_BELOW_THRESHOLD
#define KEYSERVICE_TOUCH_ACTIVE_BELOW_THRESHOLD 0
#endif

#ifndef KEYSERVICE_LONG_PRESS_MS
#define KEYSERVICE_LONG_PRESS_MS 800
#endif

#ifndef KEYSERVICE_DEBOUNCE_MS
#define KEYSERVICE_DEBOUNCE_MS 50
#endif

#ifndef KEYSERVICE_POLL_INTERVAL_MS
#define KEYSERVICE_POLL_INTERVAL_MS 20
#endif

#ifndef KEYSERVICE_EVENT_QUEUE_LENGTH
#define KEYSERVICE_EVENT_QUEUE_LENGTH 8
#endif

#ifndef KEYSERVICE_TASK_STACK_SIZE
#define KEYSERVICE_TASK_STACK_SIZE 4096
#endif

#ifndef KEYSERVICE_TASK_PRIORITY
#define KEYSERVICE_TASK_PRIORITY 2
#endif

#ifndef KEYSERVICE_TOUCH_MEASURE_CYCLES
#define KEYSERVICE_TOUCH_MEASURE_CYCLES 0x800
#endif

#ifndef KEYSERVICE_TOUCH_SLEEP_CYCLES
#define KEYSERVICE_TOUCH_SLEEP_CYCLES 0x1000
#endif

namespace KeyService
{
enum class KeyId : uint8_t
{
  Left,
  Right,
};

enum class KeyAction : uint8_t
{
  ShortPress,
  LongPress,
};

struct KeyEvent
{
  KeyId key;
  KeyAction action;
  uint32_t durationMs;
  uint32_t touchValue;
  uint32_t timestampMs;
};

bool begin();
bool getEvent(KeyEvent &event, TickType_t timeoutTicks = 0);
QueueHandle_t eventQueue();
const char *keyName(KeyId key);
const char *actionName(KeyAction action);
}
