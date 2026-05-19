#include "BleProtocol.h"

ParseResult BleProtocol::parse(const uint8_t *data, size_t length,
                               CCMessage &out) {
  if (length < 3) {
    return ParseResult::ERROR_TOO_SHORT;
  }

  uint8_t channel = data[0];
  uint8_t controller = data[1];
  uint8_t value = data[2];

  if (channel < 1 || channel > 16) {
    return ParseResult::ERROR_INVALID_CHANNEL;
  }

  if (controller > 127) {
    return ParseResult::ERROR_INVALID_CONTROLLER;
  }

  if (value > 127) {
    return ParseResult::ERROR_INVALID_VALUE;
  }

  out.channel = channel;
  out.controller = controller;
  out.value = value;

  return ParseResult::OK;
}

void BleProtocol::serialize(const CCMessage &msg, uint8_t *outBuffer) {
  outBuffer[0] = msg.channel;
  outBuffer[1] = msg.controller;
  outBuffer[2] = msg.value;
}

bool BleProtocol::isValid(const CCMessage &msg) {
  if (msg.channel < 1 || msg.channel > 16)
    return false;
  if (msg.controller > 127)
    return false;
  if (msg.value > 127)
    return false;
  return true;
}
