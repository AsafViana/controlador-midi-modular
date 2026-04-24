#pragma once

#include "storage/Storage.h"
#include "ui/Screen.h"
#include "ui/components/TextComponent.h"


class OledApp;

/**
 * OitavaScreen — Tela para configurar a oitava do teclado (0-8).
 *
 * UP incrementa, DOWN decrementa, SELECT confirma e volta.
 */
class OitavaScreen : public Screen {
public:
  OitavaScreen(Storage *storage);

  void setApp(OledApp *app);

  void handleInput(NavInput input) override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

private:
  Storage *_storage;
  OledApp *_app = nullptr;
  TextComponent _titulo;
  TextComponent _voltar;
  TextComponent _valorComp;
  uint8_t _oitava = 4;
  char _buf[4];
};
