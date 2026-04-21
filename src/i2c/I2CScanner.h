#pragma once

#include <cstdint>
#include "i2c/I2CBus.h"
#include "i2c/ModuleDescriptor.h"

/**
 * ModuleInfo — Estado em runtime de um módulo externo descoberto.
 *
 * Armazena o endereço I2C, descritor completo, estado de conexão
 * e contagem de falhas consecutivas para resiliência.
 */
struct ModuleInfo {
    uint8_t address;
    ModuleDescriptor descriptor;
    bool connected;
    uint8_t failCount;  // contagem de falhas consecutivas
};

/**
 * I2CScanner — Descobre e gerencia módulos externos no barramento I2C.
 *
 * Varre endereços 0x20–0x27 (exceto 0x3C do OLED), lê descritores,
 * monitora conexões/desconexões e lê valores de controles remotos.
 *
 * Resiliência: após MAX_FAIL_COUNT falhas consecutivas, o módulo
 * é marcado como desconectado. Varreduras periódicas podem
 * redescobrir módulos reconectados.
 */
class I2CScanner {
public:
    static constexpr uint8_t MAX_MODULES = 8;
    static constexpr uint8_t MAX_FAIL_COUNT = 3;
    static constexpr uint32_t RESCAN_INTERVAL_MS = 5000;

    explicit I2CScanner(I2CBus* bus);

    /// Varredura completa do barramento. Chamada na inicialização.
    /// Retorna o número de módulos descobertos.
    uint8_t scan();

    /// Varredura periódica (chamada no loop). Detecta conexões/desconexões.
    void periodicScan();

    /// Lê os valores atuais de todos os controles de um módulo.
    /// Retorna true se a leitura foi bem-sucedida.
    bool readValues(uint8_t moduleIndex, uint8_t* values, uint8_t maxLen);

    /// Retorna o número de módulos atualmente conectados.
    uint8_t getModuleCount() const;

    /// Retorna informações de um módulo pelo índice.
    const ModuleInfo* getModule(uint8_t index) const;

    /// Retorna o número total de controles remotos (soma de todos os módulos).
    uint8_t getTotalRemoteControls() const;

private:
    I2CBus* _bus;
    ModuleInfo _modules[MAX_MODULES];
    uint8_t _moduleCount = 0;
    uint32_t _lastScanTime = 0;

    /// Tenta ler o descritor de um módulo no endereço dado.
    bool probeAndRead(uint8_t address, ModuleDescriptor& desc);

    /// Incrementa falha de um módulo. Remove se exceder MAX_FAIL_COUNT.
    void registerFailure(uint8_t moduleIndex);
};
