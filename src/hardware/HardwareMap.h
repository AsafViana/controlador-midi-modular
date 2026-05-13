#pragma once

#include <cstdint>

/**
 * HardwareMap — Registro centralizado de todos os controles físicos.
 *
 * Módulo Principal do Controlador MIDI Modular (ESP32-S3-WROOM-1-N16R8)
 *
 * Interfaces:
 *   - USB nativo (GPIO19/20) → MIDI USB + alimentação
 *   - I2C (GPIO4/5) → Display OLED + barramento modular
 *   - MIDI DIN (GPIO9/10) → Saída/entrada MIDI física (conector 5 pinos)
 *   - 4 botões de navegação
 *   - 1 potenciômetro (ADC)
 *
 * Pinos proibidos (PSRAM Octal): IO35, IO36, IO37
 * Pinos de strapping: GPIO0 (BOOT), EN (RESET)
 * Pinos reservados USB: GPIO19 (D-), GPIO20 (D+)
 *
 * CCs padrão são atribuídos automaticamente pelo sistema de auto-assign.
 * A lista CC_AUTO_POOL define a ordem de preferência.
 */

enum class TipoControle : uint8_t {
  BOTAO,                 // Botão de navegação (não MIDI)
  POTENCIOMETRO,         // Controle analógico contínuo
  SENSOR,                // Sensor analógico (ex: luz)
  ENCODER,               // Encoder rotativo
  BOTAO_MIDI_MOMENTANEO, // Botão MIDI: press=127, release=0
  BOTAO_MIDI_TOGGLE      // Botão MIDI: alterna entre 0 e 127
};

struct ControleHW {
  const char *label;
  uint8_t gpio;
  TipoControle tipo;
  uint8_t ccPadrao; ///< 0 = auto-assign a partir do CC_AUTO_POOL
  bool invertido;
};

namespace HardwareMap {

// ── I2C — Display OLED + Barramento Modular ──────────
constexpr uint8_t PIN_I2C_SDA = 5;
constexpr uint8_t PIN_I2C_SCL = 4;

// ── Botões de navegação (não são controles MIDI) ─────
constexpr uint8_t PIN_BTN_UP = 11;
constexpr uint8_t PIN_BTN_DOWN = 12;
constexpr uint8_t PIN_BTN_SELECT = 13;
constexpr uint8_t PIN_BTN_BACK = 14;

// ── MIDI DIN (Serial1) — Conector 5 pinos ───────────
constexpr uint8_t PIN_MIDI_TX = 9;  // TX para MIDI DIN OUT
constexpr uint8_t PIN_MIDI_RX = 10; // RX para MIDI DIN IN

// ── Pool de CCs para auto-atribuição ─────────────────
constexpr uint8_t CC_AUTO_POOL[] = {
    1,              // Modulation Wheel
    7,              // Volume
    10,             // Pan
    11,             // Expression
    74,             // Brightness / Filter Cutoff
    71,             // Resonance
    73,             // Attack
    75,             // Decay
    72,             // Release
    76,             // Vibrato Rate
    77,             // Vibrato Depth
    78,             // Vibrato Delay
    16, 17, 18, 19, // General Purpose 1-4
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, // Undefined (livres)
    80, 81, 82, 83,                                 // General Purpose 5-8
    85, 86, 87,                                     // Undefined (livres)
};
constexpr uint8_t CC_AUTO_POOL_SIZE =
    sizeof(CC_AUTO_POOL) / sizeof(CC_AUTO_POOL[0]);

// ── Controles MIDI endereçáveis a CC ─────────────────
constexpr ControleHW CONTROLES[] = {
    {"Pot Extra", 7, TipoControle::POTENCIOMETRO, 0, false}, // CC auto
};

constexpr uint8_t NUM_CONTROLES = sizeof(CONTROLES) / sizeof(CONTROLES[0]);

inline const char *getLabel(uint8_t i) {
  return i < NUM_CONTROLES ? CONTROLES[i].label : "???";
}
inline uint8_t getGpio(uint8_t i) {
  return i < NUM_CONTROLES ? CONTROLES[i].gpio : 0;
}
inline TipoControle getTipo(uint8_t i) {
  return i < NUM_CONTROLES ? CONTROLES[i].tipo : TipoControle::BOTAO;
}
inline bool isInvertido(uint8_t i) {
  return i < NUM_CONTROLES ? CONTROLES[i].invertido : false;
}
inline bool isAnalogico(uint8_t i) {
  if (i >= NUM_CONTROLES)
    return false;
  TipoControle t = CONTROLES[i].tipo;
  return t == TipoControle::POTENCIOMETRO || t == TipoControle::SENSOR;
}

inline bool isBotaoMidi(uint8_t i) {
  if (i >= NUM_CONTROLES)
    return false;
  TipoControle t = CONTROLES[i].tipo;
  return t == TipoControle::BOTAO_MIDI_MOMENTANEO ||
         t == TipoControle::BOTAO_MIDI_TOGGLE;
}

/**
 * Retorna o CC padrão para o controle i.
 */
inline uint8_t getCCPadrao(uint8_t i) {
  if (i >= NUM_CONTROLES)
    return 0;

  if (CONTROLES[i].ccPadrao != 0)
    return CONTROLES[i].ccPadrao;

  bool usado[128] = {};

  for (uint8_t j = 0; j < NUM_CONTROLES; j++) {
    if (j == i)
      continue;
    if (CONTROLES[j].ccPadrao != 0) {
      usado[CONTROLES[j].ccPadrao] = true;
    }
  }

  for (uint8_t j = 0; j < i; j++) {
    if (CONTROLES[j].ccPadrao == 0) {
      for (uint8_t p = 0; p < CC_AUTO_POOL_SIZE; p++) {
        uint8_t candidato = CC_AUTO_POOL[p];
        if (!usado[candidato]) {
          usado[candidato] = true;
          break;
        }
      }
    }
  }

  for (uint8_t p = 0; p < CC_AUTO_POOL_SIZE; p++) {
    uint8_t candidato = CC_AUTO_POOL[p];
    if (!usado[candidato]) {
      return candidato;
    }
  }

  return 14;
}

} // namespace HardwareMap
