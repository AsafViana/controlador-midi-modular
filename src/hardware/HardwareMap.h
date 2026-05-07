#pragma once

#include <cstdint>

/**
 * HardwareMap — Registro centralizado de todos os controles físicos.
 *
 * CCs padrão são atribuídos automaticamente pelo sistema de auto-assign.
 * A lista CC_AUTO_POOL define a ordem de preferência: CCs comuns de
 * controle contínuo primeiro, depois CCs genéricos. Ao adicionar um
 * novo controle, basta usar ccPadrao = 0 e chamar getAutoCCPadrao(i)
 * para obter um CC único sem conflitos. Ou defina manualmente se preferir.
 */

enum class TipoControle : uint8_t { BOTAO, POTENCIOMETRO, SENSOR, ENCODER };

struct ControleHW {
  const char *label;
  uint8_t gpio;
  TipoControle tipo;
  uint8_t ccPadrao; ///< 0 = auto-assign a partir do CC_AUTO_POOL
  bool invertido;
};

namespace HardwareMap {

// ── I2C — Display OLED ───────────────────────────────
constexpr uint8_t PIN_I2C_SDA = 5; // fio SDA da tela
constexpr uint8_t PIN_I2C_SCL = 4; // fio SCL da tela

// ── Botões de navegação (não são controles MIDI) ─────
constexpr uint8_t PIN_BTN_UP = 11;
constexpr uint8_t PIN_BTN_DOWN = 12;
constexpr uint8_t PIN_BTN_SELECT = 13;
constexpr uint8_t PIN_LED = 0;

// ── MIDI DIN (Serial1) ───────────────────────────────
constexpr uint8_t PIN_MIDI_TX = 9;  // TX para MIDI DIN OUT
constexpr uint8_t PIN_MIDI_RX = 10; // RX para MIDI DIN IN

// ── Pool de CCs para auto-atribuição ─────────────────
// Ordem de preferência: CCs de controle contínuo comuns,
// depois CCs genéricos (16-19, 20-31, 74-79, 80-83...).
// Quando ccPadrao == 0, o sistema percorre este pool e
// atribui o primeiro CC que nenhum outro controle já usa.
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
// GPIOs 1, 2, 3, 6, 7, 8 estão livres para potenciômetros
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

/**
 * Retorna o CC padrão para o controle i.
 *
 * Se ccPadrao > 0, retorna o valor fixo definido no array.
 * Se ccPadrao == 0, percorre o CC_AUTO_POOL e atribui o primeiro
 * CC que nenhum outro controle (com ccPadrao fixo ou auto-assign
 * anterior) já esteja usando. Isso garante que novos controles
 * sempre recebam um CC único sem precisar escolher manualmente.
 */
inline uint8_t getCCPadrao(uint8_t i) {
  if (i >= NUM_CONTROLES)
    return 0;

  // CC definido manualmente — retorna direto
  if (CONTROLES[i].ccPadrao != 0)
    return CONTROLES[i].ccPadrao;

  // Auto-assign: coleta CCs já usados por controles anteriores
  // (tanto fixos quanto auto-assigned com índice menor)
  // Usa bitmap para CCs 0-127
  bool usado[128] = {};

  // Marca CCs fixos de todos os outros controles
  for (uint8_t j = 0; j < NUM_CONTROLES; j++) {
    if (j == i)
      continue;
    if (CONTROLES[j].ccPadrao != 0) {
      usado[CONTROLES[j].ccPadrao] = true;
    }
  }

  // Resolve auto-assigns anteriores (índice < i) na mesma ordem
  for (uint8_t j = 0; j < i; j++) {
    if (CONTROLES[j].ccPadrao == 0) {
      // Repete a lógica para achar qual CC j pegou
      for (uint8_t p = 0; p < CC_AUTO_POOL_SIZE; p++) {
        uint8_t candidato = CC_AUTO_POOL[p];
        if (!usado[candidato]) {
          usado[candidato] = true;
          break;
        }
      }
    }
  }

  // Agora encontra o primeiro CC livre no pool para o controle i
  for (uint8_t p = 0; p < CC_AUTO_POOL_SIZE; p++) {
    uint8_t candidato = CC_AUTO_POOL[p];
    if (!usado[candidato]) {
      return candidato;
    }
  }

  // Fallback: se o pool inteiro estiver ocupado, usa CC 14 (genérico)
  return 14;
}

} // namespace HardwareMap
