#include "KeyService.h"

#include <freertos/task.h>

namespace KeyService
{
namespace
{
struct ButtonState
{
  KeyId key;
  const char *name;
  uint8_t pin;
  uint32_t threshold;
  bool rawPressed;
  bool stablePressed;
  bool longEventSent;
  uint32_t lastRawChangeMs;
  uint32_t pressedAtMs;
  uint32_t lastTouchValue;
};

QueueHandle_t sEventQueue = nullptr;
TaskHandle_t sTaskHandle = nullptr;

ButtonState sButtons[] = {
    {KeyId::Left, "left", KEYSERVICE_LEFT_TOUCH_PIN, KEYSERVICE_LEFT_TOUCH_THRESHOLD, false, false, false, 0, 0, 0},
    {KeyId::Right, "right", KEYSERVICE_RIGHT_TOUCH_PIN, KEYSERVICE_RIGHT_TOUCH_THRESHOLD, false, false, false, 0, 0, 0},
};

bool isPressed(uint32_t value, uint32_t threshold)
{
#if KEYSERVICE_TOUCH_ACTIVE_BELOW_THRESHOLD
  return value <= threshold;
#else
  return value >= threshold;
#endif
}

void sendEvent(const ButtonState &button, KeyAction action, uint32_t durationMs, uint32_t touchValue)
{
  if (sEventQueue == nullptr)
  {
    return;
  }

  KeyEvent event = {
      button.key,
      action,
      durationMs,
      touchValue,
      millis(),
  };

  if (xQueueSend(sEventQueue, &event, 0) != pdTRUE)
  {
    KeyEvent droppedEvent = {};
    xQueueReceive(sEventQueue, &droppedEvent, 0);
    xQueueSend(sEventQueue, &event, 0);
    Serial.println("KeyService queue full, oldest event dropped");
  }

  Serial.print("KeyService ");
  Serial.print(button.name);
  Serial.print(" ");
  Serial.print(actionName(action));
  Serial.print(" duration=");
  Serial.print(durationMs);
  Serial.print("ms touch=");
  Serial.println(touchValue);
}

void updateButton(ButtonState &button, uint32_t nowMs)
{
  const uint32_t touchValue = touchRead(button.pin);
  const bool rawPressed = isPressed(touchValue, button.threshold);
  button.lastTouchValue = touchValue;

  if (rawPressed != button.rawPressed)
  {
    button.rawPressed = rawPressed;
    button.lastRawChangeMs = nowMs;
  }

  if ((button.rawPressed != button.stablePressed) &&
      (nowMs - button.lastRawChangeMs >= KEYSERVICE_DEBOUNCE_MS))
  {
    button.stablePressed = button.rawPressed;

    if (button.stablePressed)
    {
      button.pressedAtMs = nowMs;
      button.longEventSent = false;
    }
    else
    {
      const uint32_t durationMs = nowMs - button.pressedAtMs;
      if (!button.longEventSent)
      {
        sendEvent(button, KeyAction::ShortPress, durationMs, touchValue);
      }
    }
  }

  if (button.stablePressed && !button.longEventSent)
  {
    const uint32_t durationMs = nowMs - button.pressedAtMs;
    if (durationMs >= KEYSERVICE_LONG_PRESS_MS)
    {
      button.longEventSent = true;
      sendEvent(button, KeyAction::LongPress, durationMs, touchValue);
    }
  }
}

void keyTask(void *pvParameters)
{
  (void)pvParameters;

  // touchSetCycles(KEYSERVICE_TOUCH_MEASURE_CYCLES, KEYSERVICE_TOUCH_SLEEP_CYCLES);

  const uint32_t nowMs = millis();
  for (ButtonState &button : sButtons)
  {
    button.lastTouchValue = touchRead(button.pin);
    button.rawPressed = isPressed(button.lastTouchValue, button.threshold);
    button.stablePressed = button.rawPressed;
    button.longEventSent = false;
    button.lastRawChangeMs = nowMs;
    button.pressedAtMs = button.stablePressed ? nowMs : 0;
  }

  Serial.print("KeyService started, left threshold=");
  Serial.print(KEYSERVICE_LEFT_TOUCH_THRESHOLD);
  Serial.print(" right threshold=");
  Serial.print(KEYSERVICE_RIGHT_TOUCH_THRESHOLD);
  Serial.print(" active=");
#if KEYSERVICE_TOUCH_ACTIVE_BELOW_THRESHOLD
  Serial.println("below");
#else
  Serial.println("above");
#endif

  for (;;)
  {
    const uint32_t loopMs = millis();
    for (ButtonState &button : sButtons)
    {
      updateButton(button, loopMs);
    }
    vTaskDelay(pdMS_TO_TICKS(KEYSERVICE_POLL_INTERVAL_MS));
  }
}
}

bool begin()
{
  if (sEventQueue == nullptr)
  {
    sEventQueue = xQueueCreate(KEYSERVICE_EVENT_QUEUE_LENGTH, sizeof(KeyEvent));
    if (sEventQueue == nullptr)
    {
      Serial.println("KeyService queue create failed");
      return false;
    }
  }

  if (sTaskHandle != nullptr)
  {
    return true;
  }

  const BaseType_t result = xTaskCreate(
      keyTask,
      "key_service",
      KEYSERVICE_TASK_STACK_SIZE,
      nullptr,
      KEYSERVICE_TASK_PRIORITY,
      &sTaskHandle);

  if (result != pdPASS)
  {
    Serial.println("KeyService task create failed");
    sTaskHandle = nullptr;
    return false;
  }

  return true;
}

bool getEvent(KeyEvent &event, TickType_t timeoutTicks)
{
  if (sEventQueue == nullptr)
  {
    return false;
  }
  return xQueueReceive(sEventQueue, &event, timeoutTicks) == pdTRUE;
}

QueueHandle_t eventQueue()
{
  return sEventQueue;
}

const char *keyName(KeyId key)
{
  switch (key)
  {
  case KeyId::Left:
    return "left";
  case KeyId::Right:
    return "right";
  default:
    return "unknown";
  }
}

const char *actionName(KeyAction action)
{
  switch (action)
  {
  case KeyAction::ShortPress:
    return "short";
  case KeyAction::LongPress:
    return "long";
  default:
    return "unknown";
  }
}
}
