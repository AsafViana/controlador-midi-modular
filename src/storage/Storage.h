#pragma once

#include <cstdint>
#include "hardware/HardwareMap.h"

/**
 * Módulo de armazenamento persistente (NVS).
 *
 * Salva configurações na flash do ESP32 que sobrevivem a
 * reinicializações. O número de controles e seus defaults
 * vêm do HardwareMap.
 */
class Storage {
public:
    static constexpr uint8_t MAX_CONTROLES = HardwareMap::NUM_CONTROLES;

    void begin();

    // ── Canal MIDI ───────────────────────────────────────
    uint8_t getCanalMidi() const;
    void setCanalMidi(uint8_t canal);

    // ── Endereçamento CC por controle ────────────────────
    uint8_t getControladorCC(uint8_t indice) const;
    void setControladorCC(uint8_t indice, uint8_t cc);

    // ── Habilitar/Desabilitar controles CC ───────────────
    bool isControleHabilitado(uint8_t indice) const;
    void setControleHabilitado(uint8_t indice, bool habilitado);

    // ── Habilitar/Desabilitar teclado (notas) ────────────
    bool isTecladoHabilitado() const;
    void setTecladoHabilitado(bool habilitado);

    // ── Oitava ───────────────────────────────────────────
    uint8_t getOitava() const;
    void setOitava(uint8_t oitava);

    // ── Velocidade ───────────────────────────────────────
    uint8_t getVelocidade() const;
    void setVelocidade(uint8_t vel);

    // ── Controles Remotos ────────────────────────────────
    /// Retorna o CC configurado para um controle remoto.
    /// Chave: endereço I2C + índice do controle.
    uint8_t getRemoteCC(uint8_t i2cAddr, uint8_t ctrlIdx) const;
    void setRemoteCC(uint8_t i2cAddr, uint8_t ctrlIdx, uint8_t cc);

    /// Retorna se um controle remoto está habilitado.
    bool isRemoteEnabled(uint8_t i2cAddr, uint8_t ctrlIdx) const;
    void setRemoteEnabled(uint8_t i2cAddr, uint8_t ctrlIdx, bool enabled);

    /// Carrega configurações salvas para um módulo redescoberto.
    /// Retorna true se havia configurações salvas.
    bool loadRemoteConfig(uint8_t i2cAddr, uint8_t ctrlIdx,
                          uint8_t& cc, bool& enabled) const;

private:
    uint8_t _canalMidi = 1;
    uint8_t _oitava = 4;
    uint8_t _velocidade = 100;
    uint8_t _ccMap[HardwareMap::NUM_CONTROLES];
    bool    _ccHabilitado[HardwareMap::NUM_CONTROLES];
    bool    _tecladoHabilitado = true;

    // ── Cache de configurações remotas ───────────────────
    struct RemoteCCConfig {
        uint8_t cc;
        bool enabled;
        bool hasData;  // true se foi carregado do NVS ou configurado
    };

    // Cache: 8 endereços (0x20-0x27) × 16 controles = 128 entradas
    RemoteCCConfig _remoteConfig[8][16];

    /// Converte endereço I2C (0x20-0x27) para índice do array (0-7).
    /// Retorna 0xFF se o endereço estiver fora do intervalo.
    uint8_t addrToIndex(uint8_t i2cAddr) const;
};
