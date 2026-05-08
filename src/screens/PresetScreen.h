#pragma once

#include "ui/Screen.h"
#include "ui/components/ListComponent.h"
#include "ui/components/TextComponent.h"

class OledApp;
class Storage;
class PresetManager;

/**
 * PresetScreen — Tela de seleção e gerenciamento de presets.
 *
 * Exibe os 4 slots de preset com nome e indicador de ativo.
 * SELECT em um preset abre submenu: Carregar / Salvar / Renomear.
 * BACK volta ao menu anterior.
 */
class PresetScreen : public Screen {
public:
  PresetScreen(OledApp *app, Storage *storage, PresetManager *presets);

  void handleInput(NavInput input) override;
  bool handleBack() override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

private:
  OledApp *_app;
  Storage *_storage;
  PresetManager *_presets;
  TextComponent _titulo;

  enum class Modo : uint8_t { LISTA, ACAO, CONFIRMAR_SALVAR };
  Modo _modo = Modo::LISTA;

  uint8_t _selectedSlot = 0;
  uint8_t _selectedAction = 0; // 0=Carregar, 1=Salvar

  char _slotLabels[4][16];
  const char *_slotPtrs[4];

  ListComponent _lista;

  static const char *_acoes[];
  static constexpr uint8_t NUM_ACOES = 2;

  void atualizarLabels();
};
