#pragma once

#include "storage/Storage.h"
#include "ui/Screen.h"
#include "ui/components/TextComponent.h"


class OledApp;

/**
 * VelocidadeScreen — Tela para configurar a velocidade MIDI (1-127).
 *
 * UP incrementa, DOWN decrementa, SELECT confirma e volta.
 */
class VelocidadeScreen : public Screen {
public:
  VelocidadeScreen(Storage *storage);

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
  uint8_t _velocidade = 100;
  char _buf[4];
};
