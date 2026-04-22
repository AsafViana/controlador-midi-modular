#pragma once

#include <cstdint>

/**
 * HardwareMap — Registro centralizado de todos os controles físicos.
 */

enum class TipoControle : uint8_t {
    BOTAO,
    POTENCIOMETRO,
    SENSOR,
    ENCODER
};

struct ControleHW {
    const char*  label;
    uint8_t      gpio;
    TipoControle tipo;
    uint8_t      ccPadrao;
    bool         invertido;
};

namespace HardwareMap {

    // ── I2C — Display OLED ───────────────────────────────
    constexpr uint8_t PIN_I2C_SDA    = 5;   // fio SDA da tela
    constexpr uint8_t PIN_I2C_SCL    = 4;   // fio SCL da tela

    // ── MIDI DIN (serial 5-pin) ─────────────────────────
    constexpr uint8_t PIN_MIDI_DIN_TX = 17;  // TX para conector DIN 5 pinos

    // ── Botões de navegação (não são controles MIDI) ─────
    constexpr uint8_t PIN_BTN_UP     = 11;
    constexpr uint8_t PIN_BTN_DOWN   = 12;
    constexpr uint8_t PIN_BTN_SELECT = 13;
    constexpr uint8_t PIN_LED        = 0;

    // ── Controles MIDI endereçáveis a CC ─────────────────
    // GPIOs 1, 2, 3, 6, 7, 8, 9, 10 estão livres para potenciômetros
    constexpr ControleHW CONTROLES[] = {
        // label            gpio  tipo                          ccPadrao  invertido
        { "Pot Volume",      1,   TipoControle::POTENCIOMETRO,  7,  false },
        { "Pot Pan",         2,   TipoControle::POTENCIOMETRO, 10,  false },
        { "Pot Modulacao",   3,   TipoControle::POTENCIOMETRO,  1,  false },
        { "Sensor Luz",      6,   TipoControle::SENSOR,        11,  true  },
    };

    constexpr uint8_t NUM_CONTROLES = sizeof(CONTROLES) / sizeof(CONTROLES[0]);

    inline const char* getLabel(uint8_t i)     { return i < NUM_CONTROLES ? CONTROLES[i].label     : "???"; }
    inline uint8_t     getGpio(uint8_t i)      { return i < NUM_CONTROLES ? CONTROLES[i].gpio      : 0; }
    inline TipoControle getTipo(uint8_t i)     { return i < NUM_CONTROLES ? CONTROLES[i].tipo      : TipoControle::BOTAO; }
    inline uint8_t     getCCPadrao(uint8_t i)  { return i < NUM_CONTROLES ? CONTROLES[i].ccPadrao  : 0; }
    inline bool        isInvertido(uint8_t i)  { return i < NUM_CONTROLES ? CONTROLES[i].invertido : false; }
    inline bool        isAnalogico(uint8_t i)  {
        if (i >= NUM_CONTROLES) return false;
        TipoControle t = CONTROLES[i].tipo;
        return t == TipoControle::POTENCIOMETRO || t == TipoControle::SENSOR;
    }

} // namespace HardwareMap
