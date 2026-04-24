#pragma once

#include "storage/Storage.h"
#include "ui/Screen.h"
#include "ui/components/TextComponent.h"


class OledApp;

/**
 * TecladoScreen — Tela para habilitar/desabilitar o teclado (notas).
 *
 * UP/DOWN alterna entre ON e OFF, SELECT confirma e volta.
 */
class TecladoScreen : public Screen {
public:
  TecladoScreen(Storage *storage);

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
  bool _habilitado = true;
};
