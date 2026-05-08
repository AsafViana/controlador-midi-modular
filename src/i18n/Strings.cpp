#include "i18n/Strings.h"

static Idioma _idiomaAtivo = Idioma::PT;

// Tabela de strings: [StringId][Idioma]
static const char *STRING_TABLE[STR_COUNT][2] = {
    // STR_MENU_PRINCIPAL
    {"Menu Principal", "Main Menu"},
    // STR_PERFORMANCE
    {"Performance", "Performance"},
    // STR_PRESETS
    {"Presets", "Presets"},
    // STR_CONFIGURACOES
    {"Configuracoes", "Settings"},
    // STR_SOBRE
    {"Sobre", "About"},
    // STR_MAPA_CC
    {"Mapa CC", "CC Map"},
    // STR_CANAL_MIDI
    {"Canal MIDI", "MIDI Channel"},
    // STR_OITAVA
    {"Oitava", "Octave"},
    // STR_VELOCIDADE
    {"Velocidade", "Velocity"},
    // STR_PROG_CHANGE
    {"Prog Change", "Prog Change"},
    // STR_CALIBRAR
    {"Calibrar", "Calibrate"},
    // STR_BACKUP
    {"Backup", "Backup"},
    // STR_RESTAURAR
    {"Restaurar", "Reset"},
    // STR_SALVO
    {"Salvo!", "Saved!"},
    // STR_CANCELAR
    {"Cancelar", "Cancel"},
    // STR_CONFIRMAR
    {"Confirmar", "Confirm"},
    // STR_ENVIAR
    {"Enviar Config", "Send Config"},
    // STR_RECEBER
    {"Receber Config", "Receive Config"},
    // STR_CARREGAR
    {"Carregar", "Load"},
    // STR_SALVAR
    {"Salvar", "Save"},
    // STR_IDIOMA
    {"Idioma", "Language"},
};

void Strings::setIdioma(Idioma idioma) { _idiomaAtivo = idioma; }

Idioma Strings::getIdioma() { return _idiomaAtivo; }

const char *Strings::get(StringId id) {
  if (id >= STR_COUNT)
    return "???";
  return STRING_TABLE[id][static_cast<uint8_t>(_idiomaAtivo)];
}

const char *Strings::getNomeIdioma(Idioma idioma) {
  switch (idioma) {
  case Idioma::PT:
    return "Portugues";
  case Idioma::EN:
    return "English";
  default:
    return "???";
  }
}
