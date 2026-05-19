#include "CCStateStore.h"
#include <cstring>

void CCStateStore::begin() {
  _mutex = xSemaphoreCreateMutex();
  memset(_values, 0, sizeof(_values));
  _changeCallback = nullptr;
}

bool CCStateStore::set(uint8_t channel, uint8_t controller, uint8_t value) {
  if (channel < 1 || channel > 16)
    return false;
  if (controller > 127)
    return false;
  if (value > 127)
    return false;

  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5)) != pdTRUE) {
    return false;
  }

  _values[channel - 1][controller] = value;

  xSemaphoreGive(_mutex);

  if (_changeCallback) {
    _changeCallback(channel, controller, value);
  }

  return true;
}

int16_t CCStateStore::get(uint8_t channel, uint8_t controller) const {
  if (channel < 1 || channel > 16)
    return -1;
  if (controller > 127)
    return -1;

  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5)) != pdTRUE) {
    return -1;
  }

  int16_t val = _values[channel - 1][controller];

  xSemaphoreGive(_mutex);

  return val;
}

bool CCStateStore::getChannelSnapshot(uint8_t channel,
                                      uint8_t outBuffer[128]) const {
  if (channel < 1 || channel > 16)
    return false;

  if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(5)) != pdTRUE) {
    return false;
  }

  memcpy(outBuffer, _values[channel - 1], 128);

  xSemaphoreGive(_mutex);

  return true;
}

void CCStateStore::onChange(ChangeCallback callback) {
  _changeCallback = callback;
}
