#include "storage/Storage.h"

#ifdef ARDUINO
#include <Preferences.h>
static Preferences prefsA;
static Preferences prefsB;
static Preferences prefsMeta;
#endif

#include <cstdio>
#include <cstring>

// ── CRC8 (polinômio 0x07, CRC-8/SMBUS) ──────────────────────────────────────

uint8_t Storage::crc8(const uint8_t *data, uint16_t length) {
  uint8_t crc = 0x00;
  for (uint16_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80)
        crc = (crc << 1) ^ 0x07;
      else
        crc <<= 1;
    }
  }
  return crc;
}

uint8_t Storage::calculateCRC() const {
  // Serializa dados relevantes para CRC
  uint8_t buf[4 + MAX_CONTROLES]; // canal, oitava, vel, teclado + ccMap
  buf[0] = _canalMidi;
  buf[1] = _oitava;
  buf[2] = _velocidade;
  buf[3] = _tecladoHabilitado ? 1 : 0;
  for (uint8_t i = 0; i < MAX_CONTROLES; i++) {
    buf[4 + i] = _ccMap[i];
  }
  return crc8(buf, 4 + MAX_CONTROLES);
}

// ── Inicialização ────────────────────────────────────────────────────────────

void Storage::begin() {
  // Inicializa defaults em memória
  for (uint8_t i = 0; i < MAX_CONTROLES; i++)
    _ccMap[i] = HardwareMap::getCCPadrao(i);

  _tecladoHabilitado = true;

  for (uint8_t a = 0; a < 8; a++)
    for (uint8_t c = 0; c < 16; c++) {
      _remoteConfig[a][c].cc = 0;
      _remoteConfig[a][c].hasData = false;
    }

#ifdef ARDUINO
  // Tenta carregar do slot ativo (meta namespace guarda qual é)
  prefsMeta.begin("midi_meta", false);
  _activeSlot = (char)prefsMeta.getUChar("slot", 'a');
  if (_activeSlot != 'a' && _activeSlot != 'b')
    _activeSlot = 'a';

  const char *nsActive = (_activeSlot == 'a') ? "midi_cfg_a" : "midi_cfg_b";
  const char *nsBackup = (_activeSlot == 'a') ? "midi_cfg_b" : "midi_cfg_a";

  if (loadFromSlot(nsActive)) {
    return; // Slot ativo válido
  }

  // Slot ativo corrompido — tenta o backup
  if (loadFromSlot(nsBackup)) {
    // Backup válido — troca o slot ativo
    _activeSlot = (_activeSlot == 'a') ? 'b' : 'a';
    prefsMeta.putUChar("slot", (uint8_t)_activeSlot);
    return;
  }

  // Ambos corrompidos — factory reset
  factoryReset();
#endif
}

#ifdef ARDUINO
void Storage::saveToSlot(const char *ns) {
  Preferences &prefs = (strcmp(ns, "midi_cfg_a") == 0) ? prefsA : prefsB;
  prefs.begin(ns, false);
  prefs.clear();

  prefs.putUChar("schema", NVS_SCHEMA_VERSION);
  prefs.putUChar("canal", _canalMidi);
  prefs.putUChar("oitava", _oitava);
  prefs.putUChar("vel", _velocidade);
  prefs.putBool("teclado", _tecladoHabilitado);

  char key[8];
  for (uint8_t i = 0; i < MAX_CONTROLES; i++) {
    snprintf(key, sizeof(key), "cc%d", i);
    prefs.putUChar(key, _ccMap[i]);
  }

  // Salva controles remotos que têm dados
  for (uint8_t a = 0; a < 8; a++) {
    for (uint8_t c = 0; c < 16; c++) {
      if (_remoteConfig[a][c].hasData) {
        char rkey[9];
        snprintf(rkey, sizeof(rkey), "r%02x%02x", 0x20 + a, c);
        prefs.putUChar(rkey, _remoteConfig[a][c].cc);
      }
    }
  }

  // CRC de integridade
  prefs.putUChar("crc", calculateCRC());
  prefs.end();
}

bool Storage::loadFromSlot(const char *ns) {
  Preferences &prefs = (strcmp(ns, "midi_cfg_a") == 0) ? prefsA : prefsB;
  prefs.begin(ns, true); // read-only

  uint8_t schema = prefs.getUChar("schema", 0);
  if (schema != NVS_SCHEMA_VERSION) {
    prefs.end();
    return false;
  }

  // Carrega dados temporariamente
  uint8_t canal = prefs.getUChar("canal", 1);
  uint8_t oitava = prefs.getUChar("oitava", 4);
  uint8_t vel = prefs.getUChar("vel", 100);
  bool teclado = prefs.getBool("teclado", true);

  uint8_t ccMap[MAX_CONTROLES];
  char key[8];
  for (uint8_t i = 0; i < MAX_CONTROLES; i++) {
    snprintf(key, sizeof(key), "cc%d", i);
    ccMap[i] = prefs.getUChar(key, HardwareMap::getCCPadrao(i));
  }

  uint8_t savedCRC = prefs.getUChar("crc", 0xFF);
  prefs.end();

  // Verifica CRC
  uint8_t buf[4 + MAX_CONTROLES];
  buf[0] = canal;
  buf[1] = oitava;
  buf[2] = vel;
  buf[3] = teclado ? 1 : 0;
  for (uint8_t i = 0; i < MAX_CONTROLES; i++) {
    buf[4 + i] = ccMap[i];
  }
  uint8_t computedCRC = crc8(buf, 4 + MAX_CONTROLES);

  if (computedCRC != savedCRC) {
    return false; // CRC inválido
  }

  // CRC válido — aplica dados
  _canalMidi = canal;
  _oitava = oitava;
  _velocidade = vel;
  _tecladoHabilitado = teclado;
  for (uint8_t i = 0; i < MAX_CONTROLES; i++) {
    _ccMap[i] = ccMap[i];
  }

  // Carrega remotos (sem CRC individual — são opcionais)
  prefs.begin(ns, true);
  for (uint8_t a = 0; a < 8; a++) {
    for (uint8_t c = 0; c < 16; c++) {
      char rkey[9];
      snprintf(rkey, sizeof(rkey), "r%02x%02x", 0x20 + a, c);
      if (prefs.isKey(rkey)) {
        _remoteConfig[a][c].cc = prefs.getUChar(rkey, 0);
        _remoteConfig[a][c].hasData = true;
      }
    }
  }
  prefs.end();

  return true;
}
#endif

// ── Save (persiste com double-buffer) ────────────────────────────────────────

void Storage::save() {
#ifdef ARDUINO
  // Grava no slot INATIVO
  char inactiveSlot = (_activeSlot == 'a') ? 'b' : 'a';
  const char *ns = (inactiveSlot == 'a') ? "midi_cfg_a" : "midi_cfg_b";
  saveToSlot(ns);

  // Troca o slot ativo
  _activeSlot = inactiveSlot;
  prefsMeta.putUChar("slot", (uint8_t)_activeSlot);
#endif
}

// ── Canal MIDI ───────────────────────────────────────────────────────────────

uint8_t Storage::getCanalMidi() const { return _canalMidi; }

void Storage::setCanalMidi(uint8_t canal) {
  if (canal < 1)
    canal = 1;
  if (canal > 16)
    canal = 16;
  _canalMidi = canal;
  save();
}

// ── CC por controle ──────────────────────────────────────────────────────────

uint8_t Storage::getControladorCC(uint8_t indice) const {
  if (indice >= MAX_CONTROLES)
    return 0;
  return _ccMap[indice];
}

void Storage::setControladorCC(uint8_t indice, uint8_t cc) {
  if (indice >= MAX_CONTROLES)
    return;
  if (cc > 127)
    cc = 127;
  _ccMap[indice] = cc;
  save();
}

// ── Teclado ──────────────────────────────────────────────────────────────────

bool Storage::isTecladoHabilitado() const { return _tecladoHabilitado; }

void Storage::setTecladoHabilitado(bool habilitado) {
  _tecladoHabilitado = habilitado;
  save();
}

// ── Oitava ───────────────────────────────────────────────────────────────────

uint8_t Storage::getOitava() const { return _oitava; }

void Storage::setOitava(uint8_t oitava) {
  if (oitava > 8)
    oitava = 8;
  _oitava = oitava;
  save();
}

// ── Velocidade ───────────────────────────────────────────────────────────────

uint8_t Storage::getVelocidade() const { return _velocidade; }

void Storage::setVelocidade(uint8_t vel) {
  if (vel > 127)
    vel = 127;
  _velocidade = vel;
  save();
}

// ── Controles Remotos ────────────────────────────────────────────────────────

uint8_t Storage::addrToIndex(uint8_t i2cAddr) const {
  if (i2cAddr < 0x20 || i2cAddr > 0x27)
    return 0xFF;
  return i2cAddr - 0x20;
}

uint8_t Storage::getRemoteCC(uint8_t i2cAddr, uint8_t ctrlIdx) const {
  uint8_t ai = addrToIndex(i2cAddr);
  if (ai == 0xFF || ctrlIdx >= 16)
    return 0;

  if (_remoteConfig[ai][ctrlIdx].hasData)
    return _remoteConfig[ai][ctrlIdx].cc;

  return 0;
}

void Storage::setRemoteCC(uint8_t i2cAddr, uint8_t ctrlIdx, uint8_t cc) {
  uint8_t ai = addrToIndex(i2cAddr);
  if (ai == 0xFF || ctrlIdx >= 16)
    return;
  if (cc > 127)
    cc = 127;

  _remoteConfig[ai][ctrlIdx].cc = cc;
  _remoteConfig[ai][ctrlIdx].hasData = true;
  save();
}

bool Storage::loadRemoteConfig(uint8_t i2cAddr, uint8_t ctrlIdx, uint8_t &cc,
                               bool &enabled) const {
  uint8_t ai = addrToIndex(i2cAddr);
  if (ai == 0xFF || ctrlIdx >= 16)
    return false;

  if (_remoteConfig[ai][ctrlIdx].hasData) {
    cc = _remoteConfig[ai][ctrlIdx].cc;
    enabled = true;
    return true;
  }

  return false;
}

void Storage::factoryReset() {
  _canalMidi = 1;
  _oitava = 4;
  _velocidade = 100;
  _tecladoHabilitado = true;

  for (uint8_t i = 0; i < MAX_CONTROLES; i++)
    _ccMap[i] = HardwareMap::getCCPadrao(i);

  for (uint8_t a = 0; a < 8; a++)
    for (uint8_t c = 0; c < 16; c++) {
      _remoteConfig[a][c].cc = 0;
      _remoteConfig[a][c].hasData = false;
    }

#ifdef ARDUINO
  // Limpa ambos os slots e salva defaults no slot A
  prefsA.begin("midi_cfg_a", false);
  prefsA.clear();
  prefsA.end();
  prefsB.begin("midi_cfg_b", false);
  prefsB.clear();
  prefsB.end();

  _activeSlot = 'a';
  prefsMeta.putUChar("slot", (uint8_t)_activeSlot);
  saveToSlot("midi_cfg_a");
#endif
}
