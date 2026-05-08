#include "hardware/ResponseCurve.h"

/**
 * Tabela logarítmica: simula percepção humana de volume.
 * Fórmula: out = 127 * log(1 + in/127 * (e-1)) / log(e)
 * Simplificado: crescimento lento no início, rápido no final.
 */
const uint8_t ResponseCurve::TABELA_LOG[128] = {
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
    30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
    45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
    60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  72,  73,  74,  76,
    77,  78,  80,  81,  83,  84,  86,  87,  89,  90,  92,  93,  95,  96,  98,
    99,  101, 102, 104, 105, 107, 108, 110, 111, 113, 114, 115, 116, 117, 118,
    119, 120, 121, 122, 123, 124, 124, 125, 125, 126, 126, 126, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127};

/**
 * Tabela exponencial: resposta rápida no início, suave no final.
 * Fórmula: out = 127 * (exp(in/127) - 1) / (e - 1)
 * Ideal para filtros e expressão.
 */
const uint8_t ResponseCurve::TABELA_EXP[128] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,
    1,   2,   2,   2,   2,   2,   3,   3,   3,   3,   4,   4,   4,   5,   5,
    5,   6,   6,   7,   7,   8,   8,   9,   9,   10,  10,  11,  12,  12,  13,
    14,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
    28,  30,  31,  32,  34,  35,  37,  38,  40,  41,  43,  45,  46,  48,  50,
    52,  54,  56,  58,  60,  62,  64,  66,  68,  70,  73,  75,  77,  80,  82,
    85,  87,  90,  92,  95,  97,  100, 102, 105, 107, 110, 112, 114, 116, 118,
    119, 120, 121, 122, 123, 124, 124, 125, 125, 126, 126, 126, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127};

const char *ResponseCurve::getNomeCurva(CurvaResposta curva) {
  switch (curva) {
  case CurvaResposta::LINEAR:
    return "Linear";
  case CurvaResposta::LOGARITMICA:
    return "Log";
  case CurvaResposta::EXPONENCIAL:
    return "Exp";
  default:
    return "???";
  }
}

uint8_t ResponseCurve::aplicar(CurvaResposta curva, uint8_t valor) {
  if (valor > 127)
    valor = 127;

  switch (curva) {
  case CurvaResposta::LOGARITMICA:
    return TABELA_LOG[valor];
  case CurvaResposta::EXPONENCIAL:
    return TABELA_EXP[valor];
  case CurvaResposta::LINEAR:
  default:
    return valor;
  }
}
