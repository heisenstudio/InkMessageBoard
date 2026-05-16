#include "AppCore.h"

#include <Arduino.h>

namespace AppCore
{
namespace
{
App::Model sModel = {
    App::Screen::Boot,
    false,
    false,
    0,
};
}

bool begin()
{
  sModel.updatedAtMs = millis();
  return true;
}

void loop()
{
  sModel.updatedAtMs = millis();
}

const App::Model &model()
{
  return sModel;
}
}
