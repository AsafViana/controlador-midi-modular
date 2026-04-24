#include "screens/MenuScreen.h"
#include "config.h"
#include "screens/ConfigScreen.h"
#include "screens/PerformanceScreen.h"
#include "screens/SobreScreen.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"

const char *MenuScreen::_opcoes[] = {"Performance", "Configuracoes", "Sobre"};

MenuScreen::MenuScreen(OledApp *app, Storage *storage, PerformanceScreen *perf,
                       ConfigScreen *config, SobreScreen *sobre)
    : _app(app), _storage(storage), _perf(perf), _config(config), _sobre(sobre),
      _titulo(0, 0, "Menu Principal", 1),
      _lista(0, CONTENT_Y, OLED_WIDTH, CONTENT_HEIGHT - 10, 1),
      _status(0, OLED_HEIGHT - 8, "Ch:1 Oit:4 Vel:100", 1) {
  addChild(&_titulo);
  addChild(&_lista);
  addChild(&_status);
  _lista.setItems(_opcoes, NUM_OPCOES);
  _lista.setUpButton(ButtonEvent::LONG_PRESS);
  _lista.setDownButton(ButtonEvent::SINGLE_CLICK);
}

void MenuScreen::atualizarStatus() {
  uint8_t canal = _storage->getCanalMidi();
  uint8_t oitava = _storage->getOitava();
  uint8_t vel = _storage->getVelocidade();
  snprintf(_bufStatus, sizeof(_bufStatus), "Ch:%d Oit:%d Vel:%d", canal, oitava,
           vel);
  _status.setText(_bufStatus);
}

void MenuScreen::onMount() {
  atualizarStatus();
  markDirty();
}

void MenuScreen::render(Adafruit_SSD1306 &display) { renderChildren(display); }

void MenuScreen::handleInput(NavInput input) {
  Router &router = _app->getRouter();

  switch (input) {
  case NavInput::UP:
    _lista.handleInput(ButtonEvent::LONG_PRESS);
    markDirty();
    break;
  case NavInput::DOWN:
    _lista.handleInput(ButtonEvent::SINGLE_CLICK);
    markDirty();
    break;
  case NavInput::SELECT:
    switch (_lista.getSelectedIndex()) {
    case 0:
      if (_perf)
        router.push(_perf);
      break;
    case 1:
      if (_config)
        router.push(_config);
      break;
    case 2:
      if (_sobre)
        router.push(_sobre);
      break;
    }
    break;
  default:
    break;
  }
}
