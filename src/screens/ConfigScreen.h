#pragma once

#include "ui/Screen.h"
#include "ui/components/ListComponent.h"
#include "ui/components/TextComponent.h"

class OledApp;
class Storage;
class CCMapScreen;
class CanalScreen;
class OitavaScreen;
class VelocidadeScreen;

class ConfigScreen : public Screen {
public:
  ConfigScreen(OledApp *app, Storage *storage, CCMapScreen *ccMap,
               CanalScreen *canal, OitavaScreen *oitava,
               VelocidadeScreen *velocidade);

  void handleInput(NavInput input) override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

private:
  OledApp *_app;
  Storage *_storage;
  CCMapScreen *_ccMap;
  CanalScreen *_canal;
  OitavaScreen *_oitava;
  VelocidadeScreen *_velocidade;
  TextComponent _titulo;
  TextComponent _voltar;
  ListComponent _lista;

  bool _confirmandoReset = false;

  static const char *_opcoes[];
  static constexpr uint8_t NUM_OPCOES = 6;
};
