#pragma once

#include "ui/Screen.h"
#include "ui/components/ListComponent.h"
#include "ui/components/TextComponent.h"

class OledApp;
class SysExManager;

/**
 * BackupScreen — Tela para backup/restore via SysEx.
 *
 * Opções:
 *   - Enviar (dump config via SysEx)
 *   - Receber (aguarda SysEx do host)
 */
class BackupScreen : public Screen {
public:
  BackupScreen(OledApp *app, SysExManager *sysex);

  void handleInput(NavInput input) override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

private:
  OledApp *_app;
  SysExManager *_sysex;
  TextComponent _titulo;
  ListComponent _lista;

  static const char *_opcoes[];
  static constexpr uint8_t NUM_OPCOES = 2;
};
