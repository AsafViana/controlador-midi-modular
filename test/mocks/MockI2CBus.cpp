#include "MockI2CBus.h"
#include <cstring>

// ── I2CBus interface ─────────────────────────────────────

void MockI2CBus::begin() {
    // Nada a fazer no mock
}

bool MockI2CBus::probe(uint8_t address) {
    MockModule* mod = findModule(address);
    if (!mod) return false;
    return mod->respondePing;
}

bool MockI2CBus::write(uint8_t address, const uint8_t* data, uint8_t length) {
    MockModule* mod = findModule(address);
    if (!mod || !mod->respondePing) return false;

    if (length >= 1) {
        _lastCommand = data[0];
    }
    return true;
}

uint8_t MockI2CBus::requestFrom(uint8_t address, uint8_t* buffer,
                                 uint8_t length, uint32_t timeoutMs) {
    MockModule* mod = findModule(address);
    if (!mod || !mod->respondePing) return 0;

    if (_lastCommand == I2CProtocol::CMD_DESCRIPTOR) {
        // Responder com descritor serializado via I2CProtocol::serialize()
        if (!mod->respondeDescritor) return 0;

        // Montar ModuleDescriptor a partir do MockModule
        ModuleDescriptor desc;
        desc.numControles = mod->numControles;
        for (uint8_t i = 0; i < mod->numControles; i++) {
            desc.controles[i].tipo = static_cast<TipoControle>(mod->tipos[i]);
            memcpy(desc.controles[i].label, mod->labels[i], 13);
            desc.controles[i].valor = mod->valores[i];
        }

        // Serializar para buffer temporário
        uint8_t tempBuf[256];
        uint16_t written = I2CProtocol::serialize(desc, tempBuf, sizeof(tempBuf));
        if (written == 0) return 0;

        // Copiar até o tamanho solicitado
        uint8_t toCopy = (written < length) ? static_cast<uint8_t>(written) : length;
        memcpy(buffer, tempBuf, toCopy);
        return toCopy;

    } else if (_lastCommand == I2CProtocol::CMD_READ_VALUES) {
        // Responder com valores brutos dos controles
        uint8_t count = mod->numControles;
        uint8_t toCopy = (count < length) ? count : length;
        memcpy(buffer, mod->valores, toCopy);
        return toCopy;
    }

    return 0;
}

// ── Configuração de módulos simulados ────────────────────

bool MockI2CBus::addModule(const MockModule& mod) {
    if (_moduleCount >= MAX_MOCK_MODULES) return false;
    _modules[_moduleCount++] = mod;
    return true;
}

void MockI2CBus::clearModules() {
    _moduleCount = 0;
    _lastCommand = 0;
}

void MockI2CBus::setControlValue(uint8_t address, uint8_t controlIdx, uint8_t value) {
    MockModule* mod = findModule(address);
    if (!mod) return;
    if (controlIdx >= mod->numControles) return;
    mod->valores[controlIdx] = value;
}

void MockI2CBus::setModuleConnected(uint8_t address, bool connected) {
    MockModule* mod = findModule(address);
    if (!mod) return;
    mod->respondePing = connected;
}

// ── Helpers privados ─────────────────────────────────────

MockModule* MockI2CBus::findModule(uint8_t address) {
    for (uint8_t i = 0; i < _moduleCount; i++) {
        if (_modules[i].address == address) {
            return &_modules[i];
        }
    }
    return nullptr;
}
