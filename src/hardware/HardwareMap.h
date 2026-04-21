#pragma once

#include <cstdint>

/**
 * HardwareMap — Registro centralizado de todos os controles físicos.
 *
 * Este é o ÚNICO lugar onde você define os componentes de hardware
 * do controlador. Todo o resto do projeto (telas, storage, lógica)
 * consulta este arquivo para saber quais controles existem.
 *
 * Para adicionar um novo controle:
 *   1. Adicione uma entrada no array CONTROLES[]
 *   2. Atualize NUM_CONTROLES
 *   3. Pronto — as telas e o storage já vão reconhecer
 */

// ── Tipos de controle físico ─────────────────────────────

enum class TipoControle : uint8_t {
    BOTAO,           // Botão push (digital)
    POTENCIOMETRO,   // Potenciômetro (analógico)
    SENSOR,          // Sensor genérico (analógico: LDR, pressão, etc.)
    ENCODER          // Encoder rotativo (futuro)
};

// ── Definição de um controle ─────────────────────────────

struct ControleHW {
    const char*  label;       // Nome exibido nas telas (ex: "Volume")
    uint8_t      gpio;        // Pino GPIO
    TipoControle tipo;        // Tipo do componente
    uint8_t      ccPadrao;    // CC padrão atribuído (0-127)
    bool         invertido;   // true = inverter leitura (ex: LDR)
};

// ══════════════════════════════════════════════════════════
//  REGISTRE SEUS CONTROLES AQUI
// ══════════════════════════════════════════════════════════

namespace HardwareMap {

    // ── Botões de navegação (não são controles MIDI) ─────
    constexpr uint8_t PIN_BTN_UP     = 8;
    constexpr uint8_t PIN_BTN_DOWN   = 9;
    constexpr uint8_t PIN_BTN_SELECT = 10;
    constexpr uint8_t PIN_LED        = 0;

    // ── Controles MIDI endereçáveis a CC ─────────────────
    // Cada entrada aqui aparece na tela de endereçamento CC
    // e pode ser mapeada para qualquer CC (0-127).

    constexpr ControleHW CONTROLES[] = {
        // label            gpio  tipo                    ccPadrao  invertido
        { "Pot Volume",      4,   TipoControle::POTENCIOMETRO,  7,  false },
        { "Pot Pan",         5,   TipoControle::POTENCIOMETRO, 10,  false },
        { "Pot Modulacao",   6,   TipoControle::POTENCIOMETRO,  1,  false },
        { "Sensor Luz",      7,   TipoControle::SENSOR,        11,  true  },
        // Adicione mais controles aqui...
    };

    constexpr uint8_t NUM_CONTROLES = sizeof(CONTROLES) / sizeof(CONTROLES[0]);

    // ── Helpers ──────────────────────────────────────────

    /// Retorna o label do controle pelo índice.
    inline const char* getLabel(uint8_t indice) {
        if (indice >= NUM_CONTROLES) return "???";
        return CONTROLES[indice].label;
    }

    /// Retorna o GPIO do controle pelo índice.
    inline uint8_t getGpio(uint8_t indice) {
        if (indice >= NUM_CONTROLES) return 0;
        return CONTROLES[indice].gpio;
    }

    /// Retorna o tipo do controle pelo índice.
    inline TipoControle getTipo(uint8_t indice) {
        if (indice >= NUM_CONTROLES) return TipoControle::BOTAO;
        return CONTROLES[indice].tipo;
    }

    /// Retorna o CC padrão do controle pelo índice.
    inline uint8_t getCCPadrao(uint8_t indice) {
        if (indice >= NUM_CONTROLES) return 0;
        return CONTROLES[indice].ccPadrao;
    }

    /// Retorna se o controle tem leitura invertida.
    inline bool isInvertido(uint8_t indice) {
        if (indice >= NUM_CONTROLES) return false;
        return CONTROLES[indice].invertido;
    }

    /// Verifica se o controle é analógico (pot ou sensor).
    inline bool isAnalogico(uint8_t indice) {
        if (indice >= NUM_CONTROLES) return false;
        TipoControle t = CONTROLES[indice].tipo;
        return t == TipoControle::POTENCIOMETRO || t == TipoControle::SENSOR;
    }

} // namespace HardwareMap
