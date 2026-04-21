#pragma once

#include "i2c/I2CBus.h"

/**
 * WireI2CBus — Implementação real do I2CBus usando a biblioteca Wire do Arduino.
 *
 * Utiliza Wire.begin() com pinos SDA/SCL configuráveis (ESP32-S3).
 * Em builds nativos (sem ARDUINO), os métodos são no-ops seguros.
 */
class WireI2CBus : public I2CBus {
public:
    void begin() override;
    bool probe(uint8_t address) override;
    bool write(uint8_t address, const uint8_t* data, uint8_t length) override;
    uint8_t requestFrom(uint8_t address, uint8_t* buffer,
                        uint8_t length, uint32_t timeoutMs = 50) override;
};
