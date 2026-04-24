#pragma once
#include <cstdint>

/**
 * Minimal mock of the Arduino Wire (I2C) library for native testing.
 */
class TwoWire {
public:
  void begin() {}
  void begin(uint8_t sda, uint8_t scl) {
    (void)sda;
    (void)scl;
  }
  void beginTransmission(uint8_t) {}
  uint8_t endTransmission() { return 0; }
  void write(uint8_t) {}
};

extern TwoWire Wire;
