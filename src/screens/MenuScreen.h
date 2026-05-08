#pragma once

#include "ui/Screen.h"
#include "ui/components/ListComponent.h"
#include "ui/components/TextComponent.h"

class OledApp;
class Storage;
class PerformanceScreen;
class ConfigScreen;
class SobreScreen;
class PresetScreen;

/**
 * MenuScreen — Tela principal do sistema.
 *
 * Exibe o menu de navegação e uma linha de status no rodapé
 * com canal MIDI, oitava, velocidade e preset ativo.
 */
class MenuScreen : public Screen {
public:
  MenuScreen(OledApp *app, Storage *storage, PerformanceScreen *perf,
             ConfigScreen *config, SobreScreen *sobre,
             PresetScreen *presets = nullptr);

  void handleInput(NavInput input) override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

private:
  OledApp *_app;
  Storage *_storage;
  PerformanceScreen *_perf;
  ConfigScreen *_config;
  SobreScreen *_sobre;
  PresetScreen *_presets;
  TextComponent _titulo;
  ListComponent _lista;
  TextComponent _status;

  char _bufStatus[28];

  void atualizarStatus();

  static const char *_opcoes[];
  static constexpr uint8_t NUM_OPCOES = 4;
};
