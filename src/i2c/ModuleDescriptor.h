#pragma once

#include <cstdint>
#include "hardware/HardwareMap.h"  // TipoControle

/**
 * RemoteControl — Representa um controle físico em um módulo externo.
 */
struct RemoteControl {
    TipoControle tipo;
    char label[13];     // 12 chars + null terminator
    uint8_t valor;      // 0-127
};

/**
 * ModuleDescriptor — Descritor de um módulo externo I2C.
 *
 * Contém a lista de controles reportados pelo módulo.
 * Formato binário: [numControles:1][tipo:1][label:12][valor:1] × N
 */
struct ModuleDescriptor {
    uint8_t numControles;               // 1-16
    RemoteControl controles[16];

    /// Tamanho total em bytes quando serializado.
    /// 1 (numControles) + numControles * 14 (tipo:1 + label:12 + valor:1)
    uint16_t serializedSize() const;
};

/// Comandos e constantes do protocolo I2C
namespace I2CProtocol {
    constexpr uint8_t CMD_DESCRIPTOR  = 0x01;
    constexpr uint8_t CMD_READ_VALUES = 0x02;

    constexpr uint8_t MAX_CONTROLES_POR_MODULO = 16;
    constexpr uint8_t LABEL_MAX_LEN            = 12;
    constexpr uint8_t BYTES_POR_CONTROLE       = 14;  // tipo:1 + label:12 + valor:1

    constexpr uint8_t ADDR_MIN  = 0x20;
    constexpr uint8_t ADDR_MAX  = 0x27;
    constexpr uint8_t ADDR_OLED = 0x3C;

    /// Serializa um ModuleDescriptor em buffer.
    /// Retorna o número de bytes escritos, ou 0 se o buffer for insuficiente.
    uint16_t serialize(const ModuleDescriptor& desc, uint8_t* buffer, uint16_t bufferSize);

    /// Desserializa bytes em um ModuleDescriptor.
    /// Retorna true se o parsing foi bem-sucedido.
    bool deserialize(const uint8_t* buffer, uint16_t length, ModuleDescriptor& out);

    /// Valida se um tipo de controle é conhecido.
    bool isValidTipo(uint8_t tipo);
}
