#pragma once

#include "storage/Storage.h"
#include "ui/Screen.h"
#include "ui/components/TextComponent.h"

class OledApp;

class CanalScreen : public Screen {
public:
  CanalScreen(Storage *storage);

  void setApp(OledApp *app);

  void handleInput(NavInput input) override;
  bool handleBack() override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

private:
  Storage *_storage;
  OledApp *_app = nullptr;
  TextComponent _titulo;
  TextComponent _hint;
  TextComponent _valorComp;
  uint8_t _canal = 1;
  uint8_t _canalOriginal = 1;
  char _buf[4];
};
