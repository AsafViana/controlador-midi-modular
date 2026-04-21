#pragma once

#include <cstdint>

/**
 * I2CBus — Interface abstrata para comunicação I2C.
 *
 * Permite trocar entre WireI2CBus (hardware real) e MockI2CBus (testes)
 * sem alterar a lógica de negócio.
 */
class I2CBus {
public:
    virtual ~I2CBus() = default;

    /// Inicializa o barramento I2C.
    virtual void begin() = 0;

    /// Verifica se há dispositivo no endereço (probe).
    /// Retorna true se o dispositivo respondeu com ACK.
    virtual bool probe(uint8_t address) = 0;

    /// Envia bytes para o dispositivo no endereço.
    /// Retorna true se a transmissão foi bem-sucedida.
    virtual bool write(uint8_t address, const uint8_t* data, uint8_t length) = 0;

    /// Solicita N bytes do dispositivo no endereço.
    /// Retorna o número de bytes efetivamente lidos.
    /// Timeout de 50ms por transação.
    virtual uint8_t requestFrom(uint8_t address, uint8_t* buffer,
                                uint8_t length, uint32_t timeoutMs = 50) = 0;
};
