#pragma once

#include "i2c/I2CBus.h"
#include "i2c/ModuleDescriptor.h"
#include <cstring>

/**
 * MockModule — Representa um módulo I2C simulado para testes.
 *
 * Permite configurar controles, valores e comportamento de resposta
 * sem hardware real.
 */
struct MockModule {
    uint8_t address;
    uint8_t numControles;
    uint8_t tipos[16];
    char labels[16][13];    // 12 chars + null
    uint8_t valores[16];
    bool respondePing;      // false = simula módulo desconectado
    bool respondeDescritor; // false = simula timeout no descritor
};

/**
 * MockI2CBus — Implementação de teste do barramento I2C.
 *
 * Simula até 8 módulos I2C com descritores e valores configuráveis.
 * Usa I2CProtocol::serialize() para gerar respostas de descritor
 * e retorna valores brutos para CMD_READ_VALUES.
 */
class MockI2CBus : public I2CBus {
public:
    static constexpr uint8_t MAX_MOCK_MODULES = 8;

    // ── I2CBus interface ─────────────────────────────────
    void begin() override;
    bool probe(uint8_t address) override;
    bool write(uint8_t address, const uint8_t* data, uint8_t length) override;
    uint8_t requestFrom(uint8_t address, uint8_t* buffer,
                        uint8_t length, uint32_t timeoutMs = 50) override;

    // ── Configuração de módulos simulados ────────────────

    /// Registra um módulo simulado. Retorna false se já há 8 módulos.
    bool addModule(const MockModule& mod);

    /// Remove todos os módulos simulados.
    void clearModules();

    /// Altera o valor de um controle de um módulo simulado.
    void setControlValue(uint8_t address, uint8_t controlIdx, uint8_t value);

    /// Simula conexão/desconexão de um módulo (afeta respondePing).
    void setModuleConnected(uint8_t address, bool connected);

private:
    MockModule _modules[MAX_MOCK_MODULES];
    uint8_t _moduleCount = 0;
    uint8_t _lastCommand = 0;  // último comando recebido via write()

    /// Busca módulo pelo endereço. Retorna nullptr se não encontrado.
    MockModule* findModule(uint8_t address);
};
