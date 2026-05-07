#include "i2c/WireI2CBus.h"

#ifdef ARDUINO

#include <Wire.h>
#include "hardware/HardwareMap.h"

void WireI2CBus::begin() {
    // Pinos definidos no HardwareMap: SDA=5, SCL=4
    // Wire.begin() deve ser chamado UMA única vez em todo o sistema.
    // OledApp::begin() NÃO deve chamar Wire.begin() — ele reutiliza
    // o barramento já inicializado aqui.
    Wire.begin(HardwareMap::PIN_I2C_SDA, HardwareMap::PIN_I2C_SCL);
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

void WireI2CBus::begin() {}

bool WireI2CBus::probe(uint8_t /*address*/) { return false; }

bool WireI2CBus::write(uint8_t /*address*/, const uint8_t* /*data*/, uint8_t /*length*/) { return false; }

uint8_t WireI2CBus::requestFrom(uint8_t /*address*/, uint8_t* /*buffer*/,
                                 uint8_t /*length*/, uint32_t /*timeoutMs*/) { return 0; }

#endif
