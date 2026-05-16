#include "AppModel.h"

namespace AppReducer
{
void reset(App::Model &model)
{
  model.screen = App::Screen::Boot;
  model.wifiConnected = false;
  model.batteryReady = false;
  model.updatedAtMs = 0;
}
}
