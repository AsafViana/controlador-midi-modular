#pragma once

#include <cstdint>

/**
 * ResponseCurve — Curvas de resposta para controles MIDI.
 *
 * Cada curva é uma lookup table de 128 bytes que mapeia
 * valor linear (0-127) para valor com curva aplicada (0-127).
 *
 * Curvas disponíveis:
 *   LINEAR      — 1:1, sem transformação
 *   LOGARITMICA — resposta lenta no início, rápida no final (ideal para volume)
 *   EXPONENCIAL — resposta rápida no início, lenta no final (ideal para
 * filtros)
 */

enum class CurvaResposta : uint8_t {
  LINEAR = 0,
  LOGARITMICA = 1,
  EXPONENCIAL = 2
};

namespace ResponseCurve {

/// Número de curvas disponíveis
constexpr uint8_t NUM_CURVAS = 3;

/// Nomes das curvas para exibição na UI
const char *getNomeCurva(CurvaResposta curva);

/// Aplica a curva ao valor (0-127) → (0-127)
uint8_t aplicar(CurvaResposta curva, uint8_t valor);

/// Lookup tables (128 bytes cada)
extern const uint8_t TABELA_LOG[128];
extern const uint8_t TABELA_EXP[128];

} // namespace ResponseCurve
