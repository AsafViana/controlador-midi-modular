#pragma once

#include <cstdint>
#include "hardware/HardwareMap.h"

class I2CScanner;

/**
 * ControlInfo — Informações de um controle na lista unificada.
 *
 * Representa tanto controles locais (HardwareMap) quanto remotos
 * (módulos I2C). Para controles locais, moduleAddress e moduleCtrlIdx
 * não são utilizados.
 */
struct ControlInfo {
    const char* label;
    TipoControle tipo;
    uint8_t valor;          // valor atual (0-127)
    uint8_t ccPadrao;       // CC padrão
    bool isRemoto;          // true se vem de módulo externo
    uint8_t moduleAddress;  // endereço I2C (só para remotos)
    uint8_t moduleCtrlIdx;  // índice dentro do módulo (só para remotos)
};

/**
 * UnifiedControlList — Fachada que combina controles locais e remotos.
 *
 * Combina os controles definidos em HardwareMap (locais, constexpr)
 * com os controles descobertos pelo I2CScanner (remotos, runtime)
 * numa interface uniforme.
 *
 * Índices 0..N-1 = locais (HardwareMap), N..M = remotos (módulos I2C).
 * Máximo de MAX_TOTAL_CONTROLS controles no total.
 */
class UnifiedControlList {
public:
    static constexpr uint8_t MAX_TOTAL_CONTROLS = 32;

    explicit UnifiedControlList(I2CScanner* scanner);

    /// Reconstrói a lista unificada a partir do HardwareMap + módulos descobertos.
    void rebuild();

    /// Número total de controles (locais + remotos).
    uint8_t getNumControles() const;

    /// Número de controles locais.
    uint8_t getNumLocais() const;

    /// Retorna informações de um controle pelo índice unificado.
    /// Índices 0..N-1 = locais, N..M = remotos.
    bool getControlInfo(uint8_t index, ControlInfo& out) const;

    /// Retorna o label de um controle pelo índice unificado.
    const char* getLabel(uint8_t index) const;

    /// Retorna o tipo de um controle pelo índice unificado.
    TipoControle getTipo(uint8_t index) const;

    /// Verifica se o controle é analógico (pot ou sensor).
    bool isAnalogico(uint8_t index) const;

    /// Verifica se o controle é remoto.
    bool isRemoto(uint8_t index) const;

    /// Retorna o CC padrão do controle.
    uint8_t getCCPadrao(uint8_t index) const;

    /// Para controles remotos: retorna endereço I2C e índice no módulo.
    /// Retorna false se o índice for inválido ou o controle for local.
    bool getRemoteInfo(uint8_t index, uint8_t& address, uint8_t& ctrlIdx) const;

private:
    I2CScanner* _scanner;
    ControlInfo _controls[MAX_TOTAL_CONTROLS];
    uint8_t _numControles = 0;
    uint8_t _numLocais = 0;

    /// Buffers para labels de controles remotos.
    /// Necessário porque o label do ModuleDescriptor pode estar na memória
    /// do scanner e ser sobrescrito em rescans.
    char _remoteLabels[MAX_TOTAL_CONTROLS][13];
};
