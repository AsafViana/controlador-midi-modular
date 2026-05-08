#pragma once

#include "hardware/HardwareMap.h"
#include <cstdint>

/**
 * Módulo de armazenamento persistente (NVS).
 *
 * Todos os controles analógicos (locais e remotos) são sempre ativos.
 * Não existe flag de habilitado — isControleHabilitado e isRemoteEnabled
 * são stubs inline que retornam true para compatibilidade de API.
 */
class Storage {
public:
  static constexpr uint8_t MAX_CONTROLES = HardwareMap::NUM_CONTROLES;
  static constexpr uint8_t NVS_SCHEMA_VERSION = HardwareMap::NUM_CONTROLES + 1;

  void begin();

  // ── Canal MIDI ───────────────────────────────────────
  uint8_t getCanalMidi() const;
  void setCanalMidi(uint8_t canal);

  // ── CC por controle ──────────────────────────────────
  uint8_t getControladorCC(uint8_t indice) const;
  void setControladorCC(uint8_t indice, uint8_t cc);

  // ── Controles sempre ativos (stubs) ──────────────────
  bool isControleHabilitado(uint8_t) const { return true; }
  void setControleHabilitado(uint8_t, bool) {}

  // ── Teclado ──────────────────────────────────────────
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

  // Controles remotos também sempre ativos (stubs)
  bool isRemoteEnabled(uint8_t, uint8_t) const { return true; }
  void setRemoteEnabled(uint8_t, uint8_t, bool) {}

  bool loadRemoteConfig(uint8_t i2cAddr, uint8_t ctrlIdx,
                        uint8_t &cc, bool &enabled) const;

  void factoryReset();

private:
  uint8_t _canalMidi   = 1;
  uint8_t _oitava      = 4;
  uint8_t _velocidade  = 100;
  bool    _tecladoHabilitado = true;
  uint8_t _ccMap[HardwareMap::NUM_CONTROLES];

  struct RemoteCCConfig {
    uint8_t cc      = 0;
    bool    hasData = false;
  };
  RemoteCCConfig _remoteConfig[8][16];

  uint8_t addrToIndex(uint8_t i2cAddr) const;
};
