#pragma once

#include <cstdint>

/**
 * StatusLed — Controle do LED RGB de status.
 *
 * Padrões de cor:
 *   - Verde fixo: sistema OK, idle
 *   - Azul piscando: atividade MIDI
 *   - Vermelho fixo: erro (display falhou, NVS corrompido)
 *   - Roxo: modo especial (OTA, backup)
 *   - Desligado: brilho 0
 *
 * Se RGB_LED_ENABLED == false no HardwareMap, usa apenas o LED simples
 * (PIN_LED) com padrões de piscar para indicar status.
 *
 * Brilho limitado a 10-20% para não distrair em palco.
 */
class StatusLed {
public:
  enum class Estado : uint8_t {
    OFF,
    OK,          // Verde fixo
    MIDI_ACTIVE, // Azul piscando
    ERRO,        // Vermelho fixo
    ESPECIAL     // Roxo
  };

  void begin();
  void update();

  void setEstado(Estado estado);
  void triggerMidiFlash(); // Flash breve de atividade MIDI

private:
  Estado _estado = Estado::OK;
  bool _rgbEnabled = false;

  // Para flash de atividade MIDI
  bool _midiFlashing = false;
  uint32_t _midiFlashStart = 0;
  static constexpr uint32_t MIDI_FLASH_DURATION_MS = 50;

  // Brilho máximo (0-255) — 20% = 51
  static constexpr uint8_t MAX_BRIGHTNESS = 51;

  void setRGB(uint8_t r, uint8_t g, uint8_t b);
  void setSimpleLed(bool on);
};
