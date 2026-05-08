#include "storage/PresetManager.h"
#include "storage/Storage.h"
#include <cstdio>
#include <cstring>


#ifdef ARDUINO
#include <Preferences.h>
static Preferences prefsPreset;
#endif

static const char *DEFAULT_NAMES[] = {"Preset 1", "Preset 2", "Preset 3",
                                      "Preset 4"};

PresetManager::PresetManager(Storage *storage) : _storage(storage) {
  for (uint8_t i = 0; i < NUM_PRESETS; i++) {
    strncpy(_slots[i].name, DEFAULT_NAMES[i], MAX_NAME_LEN);
    _slots[i].name[MAX_NAME_LEN] = '\0';
    _slots[i].occupied = false;
  }
}

void PresetManager::begin() {
#ifdef ARDUINO
  prefsPreset.begin("midi_presets", false);
  _activePreset = prefsPreset.getUChar("active", 0);
  if (_activePreset >= NUM_PRESETS)
    _activePreset = 0;
  loadNames();
#endif
}

uint8_t PresetManager::getActivePreset() const { return _activePreset; }

const char *PresetManager::getPresetName(uint8_t index) const {
  if (index >= NUM_PRESETS)
    return "???";
  return _slots[index].name;
}

bool PresetManager::hasData(uint8_t index) const {
  if (index >= NUM_PRESETS)
    return false;
  return _slots[index].occupied;
}

void PresetManager::setPresetName(uint8_t index, const char *name) {
  if (index >= NUM_PRESETS || name == nullptr)
    return;
  strncpy(_slots[index].name, name, MAX_NAME_LEN);
  _slots[index].name[MAX_NAME_LEN] = '\0';
  saveName(index);
}

void PresetManager::savePreset(uint8_t index) {
  if (index >= NUM_PRESETS || _storage == nullptr)
    return;

#ifdef ARDUINO
  char ns[16];
  snprintf(ns, sizeof(ns), "midi_p%d", index);

  Preferences p;
  p.begin(ns, false);
  p.clear();

  p.putUChar("canal", _storage->getCanalMidi());
  p.putUChar("oitava", _storage->getOitava());
  p.putUChar("vel", _storage->getVelocidade());

  char key[8];
  for (uint8_t i = 0; i < Storage::MAX_CONTROLES; i++) {
    snprintf(key, sizeof(key), "cc%d", i);
    p.putUChar(key, _storage->getControladorCC(i));
  }

  p.putBool("valid", true);
  p.end();

  _slots[index].occupied = true;
  _activePreset = index;
  prefsPreset.putUChar("active", _activePreset);
#else
  _slots[index].occupied = true;
  _activePreset = index;
#endif
}

void PresetManager::loadPreset(uint8_t index) {
  if (index >= NUM_PRESETS || _storage == nullptr)
    return;

#ifdef ARDUINO
  char ns[16];
  snprintf(ns, sizeof(ns), "midi_p%d", index);

  Preferences p;
  p.begin(ns, true); // read-only

  if (!p.getBool("valid", false)) {
    p.end();
    return; // Slot vazio
  }

  uint8_t canal = p.getUChar("canal", 1);
  uint8_t oitava = p.getUChar("oitava", 4);
  uint8_t vel = p.getUChar("vel", 100);

  // Aplica no Storage (sem salvar individualmente — fazemos save() no final)
  _storage->setCanalMidi(canal);
  _storage->setOitava(oitava);
  _storage->setVelocidade(vel);

  char key[8];
  for (uint8_t i = 0; i < Storage::MAX_CONTROLES; i++) {
    snprintf(key, sizeof(key), "cc%d", i);
    uint8_t cc = p.getUChar(key, HardwareMap::getCCPadrao(i));
    _storage->setControladorCC(i, cc);
  }

  p.end();

  _activePreset = index;
  prefsPreset.putUChar("active", _activePreset);
#else
  _activePreset = index;
#endif
}

void PresetManager::loadNames() {
#ifdef ARDUINO
  for (uint8_t i = 0; i < NUM_PRESETS; i++) {
    char key[8];
    snprintf(key, sizeof(key), "name%d", i);
    if (prefsPreset.isKey(key)) {
      prefsPreset.getString(key, _slots[i].name, MAX_NAME_LEN + 1);
    }

    // Verifica se o slot tem dados
    char ns[16];
    snprintf(ns, sizeof(ns), "midi_p%d", i);
    Preferences p;
    p.begin(ns, true);
    _slots[i].occupied = p.getBool("valid", false);
    p.end();
  }
#endif
}

void PresetManager::saveName(uint8_t index) {
#ifdef ARDUINO
  char key[8];
  snprintf(key, sizeof(key), "name%d", index);
  prefsPreset.putString(key, _slots[index].name);
#endif
}
