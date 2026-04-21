#include "i2c/WireI2CBus.h"

#ifdef ARDUINO

#include <Wire.h>

void WireI2CBus::begin() {
    // ESP32-S3: SDA=pino 1, SCL=pino 2 (conforme pinagem DB-9)
    Wire.begin(1, 2);
}

bool WireI2CBus::probe(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

bool WireI2CBus::write(uint8_t address, const uint8_t* data, uint8_t length) {
    Wire.beginTransmission(address);
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(data[i]);
    }
    return Wire.endTransmission() == 0;
}

uint8_t WireI2CBus::requestFrom(uint8_t address, uint8_t* buffer,
                                 uint8_t length, uint32_t timeoutMs) {
    Wire.setTimeOut(timeoutMs);
    uint8_t received = Wire.requestFrom(address, length);
    uint8_t count = 0;
    while (Wire.available() && count < length) {
        buffer[count++] = Wire.read();
    }
    return count;
}

#else
// ── Native build stubs — safe no-ops ────────────────────

void WireI2CBus::begin() {
    // No-op in native builds
}

bool WireI2CBus::probe(uint8_t /*address*/) {
    return false;
}

bool WireI2CBus::write(uint8_t /*address*/, const uint8_t* /*data*/, uint8_t /*length*/) {
    return false;
}

uint8_t WireI2CBus::requestFrom(uint8_t /*address*/, uint8_t* /*buffer*/,
                                 uint8_t /*length*/, uint32_t /*timeoutMs*/) {
    return 0;
}

#endif
