#pragma once

#include <cstdint>

/**
 * Strings — Sistema de internacionalização (i18n).
 *
 * Suporta PT (Português) e EN (English).
 * Strings curtas (máximo 20 chars) para caber na tela OLED 128x64.
 *
 * Uso: Strings::get(STR_MENU_PRINCIPAL) retorna a string no idioma ativo.
 */

enum StringId : uint8_t {
  STR_MENU_PRINCIPAL = 0,
  STR_PERFORMANCE,
  STR_PRESETS,
  STR_CONFIGURACOES,
  STR_SOBRE,
  STR_MAPA_CC,
  STR_CANAL_MIDI,
  STR_OITAVA,
  STR_VELOCIDADE,
  STR_PROG_CHANGE,
  STR_CALIBRAR,
  STR_BACKUP,
  STR_RESTAURAR,
  STR_SALVO,
  STR_CANCELAR,
  STR_CONFIRMAR,
  STR_ENVIAR,
  STR_RECEBER,
  STR_CARREGAR,
  STR_SALVAR,
  STR_IDIOMA,
  STR_COUNT // Deve ser o último
};

enum class Idioma : uint8_t { PT = 0, EN = 1 };

namespace Strings {

/// Define o idioma ativo
void setIdioma(Idioma idioma);

/// Retorna o idioma ativo
Idioma getIdioma();

/// Retorna a string no idioma ativo
const char *get(StringId id);

/// Retorna o nome do idioma para exibição
const char *getNomeIdioma(Idioma idioma);

} // namespace Strings
