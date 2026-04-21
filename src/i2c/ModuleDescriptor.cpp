#include "i2c/ModuleDescriptor.h"
#include <cstring>

// ── ModuleDescriptor ─────────────────────────────────────

uint16_t ModuleDescriptor::serializedSize() const {
    return 1 + static_cast<uint16_t>(numControles) * I2CProtocol::BYTES_POR_CONTROLE;
}

// ── I2CProtocol ──────────────────────────────────────────

namespace I2CProtocol {

bool isValidTipo(uint8_t tipo) {
    return tipo <= static_cast<uint8_t>(TipoControle::ENCODER);
}

uint16_t serialize(const ModuleDescriptor& desc, uint8_t* buffer, uint16_t bufferSize) {
    if (desc.numControles == 0 || desc.numControles > MAX_CONTROLES_POR_MODULO) {
        return 0;
    }

    uint16_t needed = 1 + static_cast<uint16_t>(desc.numControles) * BYTES_POR_CONTROLE;
    if (bufferSize < needed) {
        return 0;
    }

    uint16_t pos = 0;
    buffer[pos++] = desc.numControles;

    for (uint8_t i = 0; i < desc.numControles; i++) {
        const RemoteControl& ctrl = desc.controles[i];

        // tipo: 1 byte
        buffer[pos++] = static_cast<uint8_t>(ctrl.tipo);

        // label: 12 bytes, null-padded
        uint8_t labelLen = 0;
        while (labelLen < LABEL_MAX_LEN && ctrl.label[labelLen] != '\0') {
            labelLen++;
        }
        memcpy(&buffer[pos], ctrl.label, labelLen);
        if (labelLen < LABEL_MAX_LEN) {
            memset(&buffer[pos + labelLen], 0, LABEL_MAX_LEN - labelLen);
        }
        pos += LABEL_MAX_LEN;

        // valor: 1 byte
        buffer[pos++] = ctrl.valor;
    }

    return pos;
}

bool deserialize(const uint8_t* buffer, uint16_t length, ModuleDescriptor& out) {
    if (length < 1) {
        return false;
    }

    uint8_t numControles = buffer[0];

    // Validar numControles: deve estar entre 1 e 16
    if (numControles == 0 || numControles > MAX_CONTROLES_POR_MODULO) {
        return false;
    }

    uint16_t expected = 1 + static_cast<uint16_t>(numControles) * BYTES_POR_CONTROLE;
    if (length < expected) {
        return false;
    }

    // Validar todos os tipos antes de preencher a struct
    uint16_t pos = 1;
    for (uint8_t i = 0; i < numControles; i++) {
        uint8_t tipo = buffer[pos];
        if (!isValidTipo(tipo)) {
            return false;
        }
        pos += BYTES_POR_CONTROLE;
    }

    // Dados válidos — preencher a struct
    out.numControles = numControles;
    pos = 1;

    for (uint8_t i = 0; i < numControles; i++) {
        RemoteControl& ctrl = out.controles[i];

        // tipo: 1 byte
        ctrl.tipo = static_cast<TipoControle>(buffer[pos++]);

        // label: 12 bytes, garantir null-terminator
        memcpy(ctrl.label, &buffer[pos], LABEL_MAX_LEN);
        ctrl.label[LABEL_MAX_LEN] = '\0';
        pos += LABEL_MAX_LEN;

        // valor: 1 byte
        ctrl.valor = buffer[pos++];
    }

    return true;
}

} // namespace I2CProtocol
