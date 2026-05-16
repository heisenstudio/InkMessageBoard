#include "TimeService.h"

namespace TimeService
{
bool begin()
{
  return false;
}

bool isSynced()
{
  return false;
}

uint32_t now()
{
  return millis();
}
}
