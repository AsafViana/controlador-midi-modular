#pragma once

#include "hardware/HardwareMap.h"
#include <cstdint>

/**
 * Módulo de armazenamento persistente (NVS).
 *
 * Salva configurações na flash do ESP32 que sobrevivem a
 * reinicializações. O número de controles e seus defaults
 * vêm do HardwareMap.
 *
 * Schema version: sempre que NUM_CONTROLES mudar, o boot detecta
 * a versão desatualizada na NVS e executa factoryReset() automático,
 * evitando CCs errados herdados de arrays antigos.
 */
class Storage {
public:
  static constexpr uint8_t MAX_CONTROLES = HardwareMap::NUM_CONTROLES;

  // Versão do schema NVS — muda automaticamente com NUM_CONTROLES.
  // Se quiser forçar um reset por outra razão, incremente o +1.
  static constexpr uint8_t NVS_SCHEMA_VERSION =
      HardwareMap::NUM_CONTROLES + 1;

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
  uint8_t getRemoteCC(uint8_t i2cAddr, uint8_t ctrlIdx) const;
  void setRemoteCC(uint8_t i2cAddr, uint8_t ctrlIdx, uint8_t cc);

  bool isRemoteEnabled(uint8_t i2cAddr, uint8_t ctrlIdx) const;
  void setRemoteEnabled(uint8_t i2cAddr, uint8_t ctrlIdx, bool enabled);

  bool loadRemoteConfig(uint8_t i2cAddr, uint8_t ctrlIdx, uint8_t &cc,
                        bool &enabled) const;

  /// Restaura todas as configurações para os valores padrão de fábrica.
  /// Limpa o NVS e recarrega defaults do HardwareMap.
  void factoryReset();

private:
  uint8_t _canalMidi = 1;
  uint8_t _oitava = 4;
  uint8_t _velocidade = 100;
  uint8_t _ccMap[HardwareMap::NUM_CONTROLES];
  bool _ccHabilitado[HardwareMap::NUM_CONTROLES];
  bool _tecladoHabilitado = true;

  struct RemoteCCConfig {
    uint8_t cc;
    bool enabled;
    bool hasData;
  };

  RemoteCCConfig _remoteConfig[8][16];

  uint8_t addrToIndex(uint8_t i2cAddr) const;
};
