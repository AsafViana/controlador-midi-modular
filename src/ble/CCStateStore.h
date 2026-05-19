#pragma once

#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * CCStateStore — Repositório central de todos os valores CC MIDI.
 *
 * Mantém 2048 entradas (128 controllers × 16 canais) protegidas
 * por mutex FreeRTOS para acesso thread-safe entre o loop principal
 * e as callbacks do stack BLE.
 */
class CCStateStore {
public:
  /// Inicializa todas as 2048 entradas com 0 e cria o mutex.
  void begin();

  /// Define um valor CC. Retorna true se válido, false se fora do range.
  /// channel: 1–16, controller: 0–127, value: 0–127.
  /// Thread-safe.
  bool set(uint8_t channel, uint8_t controller, uint8_t value);

  /// Obtém um valor CC. Retorna -1 se coordenadas inválidas.
  /// channel: 1–16, controller: 0–127.
  /// Thread-safe.
  int16_t get(uint8_t channel, uint8_t controller) const;

  /// Copia todos os 128 valores CC de um canal para o buffer (snapshot
  /// atômico). Retorna false se o canal for inválido.
  bool getChannelSnapshot(uint8_t channel, uint8_t outBuffer[128]) const;

  /// Tipo de callback para mudanças de valor.
  using ChangeCallback = void (*)(uint8_t channel, uint8_t controller,
                                  uint8_t value);

  /// Registra callback chamado após set() bem-sucedido.
  void onChange(ChangeCallback callback);

private:
  mutable SemaphoreHandle_t _mutex;
  uint8_t _values[16][128]; // [channel-1][controller]
  ChangeCallback _changeCallback = nullptr;
};
