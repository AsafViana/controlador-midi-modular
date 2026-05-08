#include "storage/Storage.h"

#ifdef ARDUINO
#include <Preferences.h>
static Preferences prefs;
#endif

#include <cstdio>

static const char *KEY_CANAL      = "canal";
static const char *KEY_OITAVA     = "oitava";
static const char *KEY_VELOCIDADE = "vel";
static const char *KEY_TECLADO    = "teclado";
static const char *KEY_SCHEMA     = "schema_v";

void Storage::begin() {
  // Inicializa defaults em memória
  for (uint8_t i = 0; i < MAX_CONTROLES; i++)
    _ccMap[i] = HardwareMap::getCCPadrao(i);

  _tecladoHabilitado = true;

  for (uint8_t a = 0; a < 8; a++)
    for (uint8_t c = 0; c < 16; c++) {
      _remoteConfig[a][c].cc      = 0;
      _remoteConfig[a][c].hasData = false;
    }

#ifdef ARDUINO
  prefs.begin("midi_cfg", false);

  uint8_t savedSchema = prefs.getUChar(KEY_SCHEMA, 0);
  if (savedSchema != NVS_SCHEMA_VERSION) {
    prefs.clear();
    prefs.putUChar(KEY_SCHEMA,     NVS_SCHEMA_VERSION);
    prefs.putUChar(KEY_CANAL,      _canalMidi);
    prefs.putUChar(KEY_OITAVA,     _oitava);
    prefs.putUChar(KEY_VELOCIDADE, _velocidade);
    prefs.putBool(KEY_TECLADO,     _tecladoHabilitado);
    char key[8];
    for (uint8_t i = 0; i < MAX_CONTROLES; i++) {
      snprintf(key, sizeof(key), "cc%d", i);
      prefs.putUChar(key, _ccMap[i]);
    }
    return;
  }

  _canalMidi         = prefs.getUChar(KEY_CANAL,      1);
  _oitava            = prefs.getUChar(KEY_OITAVA,     4);
  _velocidade        = prefs.getUChar(KEY_VELOCIDADE, 100);
  _tecladoHabilitado = prefs.getBool(KEY_TECLADO,     true);

  char key[8];
  for (uint8_t i = 0; i < MAX_CONTROLES; i++) {
    snprintf(key, sizeof(key), "cc%d", i);
    _ccMap[i] = prefs.getUChar(key, HardwareMap::getCCPadrao(i));
  }
#endif
}

// ── Canal MIDI ───────────────────────────────────────────

uint8_t Storage::getCanalMidi() const { return _canalMidi; }

void Storage::setCanalMidi(uint8_t canal) {
  if (canal < 1)  canal = 1;
  if (canal > 16) canal = 16;
  _canalMidi = canal;
#ifdef ARDUINO
  prefs.putUChar(KEY_CANAL, canal);
#endif
}

// ── CC por controle ──────────────────────────────────────

uint8_t Storage::getControladorCC(uint8_t indice) const {
  if (indice >= MAX_CONTROLES) return 0;
  return _ccMap[indice];
}

void Storage::setControladorCC(uint8_t indice, uint8_t cc) {
  if (indice >= MAX_CONTROLES) return;
  if (cc > 127) cc = 127;
  _ccMap[indice] = cc;
#ifdef ARDUINO
  char key[8];
  snprintf(key, sizeof(key), "cc%d", indice);
  prefs.putUChar(key, cc);
#endif
}

// ── Teclado ──────────────────────────────────────────────

bool Storage::isTecladoHabilitado() const { return _tecladoHabilitado; }

void Storage::setTecladoHabilitado(bool habilitado) {
  _tecladoHabilitado = habilitado;
#ifdef ARDUINO
  prefs.putBool(KEY_TECLADO, habilitado);
#endif
}

// ── Oitava ───────────────────────────────────────────────

uint8_t Storage::getOitava() const { return _oitava; }

void Storage::setOitava(uint8_t oitava) {
  if (oitava > 8) oitava = 8;
  _oitava = oitava;
#ifdef ARDUINO
  prefs.putUChar(KEY_OITAVA, oitava);
#endif
}

// ── Velocidade ───────────────────────────────────────────

uint8_t Storage::getVelocidade() const { return _velocidade; }

void Storage::setVelocidade(uint8_t vel) {
  if (vel > 127) vel = 127;
  _velocidade = vel;
#ifdef ARDUINO
  prefs.putUChar(KEY_VELOCIDADE, vel);
#endif
}

// ── Controles Remotos ────────────────────────────────────

uint8_t Storage::addrToIndex(uint8_t i2cAddr) const {
  if (i2cAddr < 0x20 || i2cAddr > 0x27) return 0xFF;
  return i2cAddr - 0x20;
}

uint8_t Storage::getRemoteCC(uint8_t i2cAddr, uint8_t ctrlIdx) const {
  uint8_t ai = addrToIndex(i2cAddr);
  if (ai == 0xFF || ctrlIdx >= 16) return 0;

  if (_remoteConfig[ai][ctrlIdx].hasData)
    return _remoteConfig[ai][ctrlIdx].cc;

#ifdef ARDUINO
  char key[9];
  snprintf(key, sizeof(key), "rcc%02x%02x", i2cAddr, ctrlIdx);
  if (prefs.isKey(key))
    return prefs.getUChar(key, 0);
#endif
  return 0;
}

void Storage::setRemoteCC(uint8_t i2cAddr, uint8_t ctrlIdx, uint8_t cc) {
  uint8_t ai = addrToIndex(i2cAddr);
  if (ai == 0xFF || ctrlIdx >= 16) return;
  if (cc > 127) cc = 127;

  _remoteConfig[ai][ctrlIdx].cc      = cc;
  _remoteConfig[ai][ctrlIdx].hasData = true;

#ifdef ARDUINO
  char key[9];
  snprintf(key, sizeof(key), "rcc%02x%02x", i2cAddr, ctrlIdx);
  prefs.putUChar(key, cc);
#endif
}

bool Storage::loadRemoteConfig(uint8_t i2cAddr, uint8_t ctrlIdx,
                               uint8_t &cc, bool &enabled) const {
  uint8_t ai = addrToIndex(i2cAddr);
  if (ai == 0xFF || ctrlIdx >= 16) return false;

  if (_remoteConfig[ai][ctrlIdx].hasData) {
    cc      = _remoteConfig[ai][ctrlIdx].cc;
    enabled = true; // sempre ativo
    return true;
  }

#ifdef ARDUINO
  char ccKey[9];
  snprintf(ccKey, sizeof(ccKey), "rcc%02x%02x", i2cAddr, ctrlIdx);
  if (prefs.isKey(ccKey)) {
    cc      = prefs.getUChar(ccKey, 0);
    enabled = true; // sempre ativo
    const_cast<Storage *>(this)->_remoteConfig[ai][ctrlIdx].cc      = cc;
    const_cast<Storage *>(this)->_remoteConfig[ai][ctrlIdx].hasData = true;
    return true;
  }
#endif
  return false;
}

void Storage::factoryReset() {
#ifdef ARDUINO
  prefs.clear();
#endif

  _canalMidi         = 1;
  _oitava            = 4;
  _velocidade        = 100;
  _tecladoHabilitado = true;

  for (uint8_t i = 0; i < MAX_CONTROLES; i++)
    _ccMap[i] = HardwareMap::getCCPadrao(i);

  for (uint8_t a = 0; a < 8; a++)
    for (uint8_t c = 0; c < 16; c++) {
      _remoteConfig[a][c].cc      = 0;
      _remoteConfig[a][c].hasData = false;
    }

#ifdef ARDUINO
  prefs.putUChar(KEY_SCHEMA,     NVS_SCHEMA_VERSION);
  prefs.putUChar(KEY_CANAL,      _canalMidi);
  prefs.putUChar(KEY_OITAVA,     _oitava);
  prefs.putUChar(KEY_VELOCIDADE, _velocidade);
  prefs.putBool(KEY_TECLADO,     _tecladoHabilitado);
  char key[8];
  for (uint8_t i = 0; i < MAX_CONTROLES; i++) {
    snprintf(key, sizeof(key), "cc%d", i);
    prefs.putUChar(key, _ccMap[i]);
  }
#endif
}
