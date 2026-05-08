#pragma once

#include <cstdint>

class Storage;

/**
 * PresetManager — Gerencia 4 slots de preset.
 *
 * Cada preset armazena uma cópia completa da configuração:
 * canal MIDI, oitava, velocidade, mapa de CC (local + remoto).
 *
 * Presets são armazenados no NVS com prefixo por slot (p0_, p1_, p2_, p3_).
 * O preset ativo é indicado por um índice (0-3) salvo no NVS.
 */
class PresetManager {
public:
  static constexpr uint8_t NUM_PRESETS = 4;
  static constexpr uint8_t MAX_NAME_LEN = 8; // 8 chars + null

  PresetManager(Storage *storage);

  /// Inicializa o gerenciador de presets. Chamar após Storage::begin().
  void begin();

  /// Retorna o índice do preset ativo (0-3)
  uint8_t getActivePreset() const;

  /// Carrega um preset (aplica configuração no Storage)
  void loadPreset(uint8_t index);

  /// Salva a configuração atual no preset especificado
  void savePreset(uint8_t index);

  /// Retorna o nome do preset (buffer interno, não modificar)
  const char *getPresetName(uint8_t index) const;

  /// Define o nome do preset (máximo 8 caracteres)
  void setPresetName(uint8_t index, const char *name);

  /// Verifica se um slot tem dados salvos
  bool hasData(uint8_t index) const;

private:
  Storage *_storage;
  uint8_t _activePreset = 0;

  struct PresetSlot {
    char name[MAX_NAME_LEN + 1]; // nome + null terminator
    bool occupied = false;       // slot tem dados?
  };
  PresetSlot _slots[NUM_PRESETS];

  void loadNames();
  void saveName(uint8_t index);
};
