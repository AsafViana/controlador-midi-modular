#include "screens/OitavaScreen.h"
#include "config.h"
#include "ui/OledApp.h"
#include <cstdio>

OitavaScreen::OitavaScreen(Storage *storage)
    : _storage(storage), _titulo(0, 0, "Oitava", 1),
      _voltar(OLED_WIDTH - 48, 4, "<Voltar", 1),
      _valorComp(0, CONTENT_Y, "4", 2) {
  addChild(&_titulo);
  addChild(&_voltar);
  addChild(&_valorComp);
}

void OitavaScreen::setApp(OledApp *app) { _app = app; }

void OitavaScreen::onMount() {
  _oitava = _storage->getOitava();
  snprintf(_buf, sizeof(_buf), "%d", _oitava);
  _valorComp.setText(_buf);
  markDirty();
}

void OitavaScreen::handleInput(NavInput input) {
  uint8_t passo = 1;

  switch (input) {
  case NavInput::LONG_UP:
    passo = 2;
    // fall through
  case NavInput::UP:
    if (_oitava < 8) {
      _oitava += passo;
      if (_oitava > 8)
        _oitava = 8;
      snprintf(_buf, sizeof(_buf), "%d", _oitava);
      _valorComp.setText(_buf);
      markDirty();
    }
    break;
  case NavInput::LONG_DOWN:
    passo = 2;
    // fall through
  case NavInput::DOWN:
    if (_oitava > 0) {
      if (_oitava < passo)
        _oitava = 0;
      else
        _oitava -= passo;
      snprintf(_buf, sizeof(_buf), "%d", _oitava);
      _valorComp.setText(_buf);
      markDirty();
    }
    break;
  case NavInput::SELECT:
    _storage->setOitava(_oitava);
    if (_app) {
      _app->showSaveConfirm();
      _app->getRouter().pop();
    }
    break;
  default:
    break;
  }
}

void OitavaScreen::render(Adafruit_SSD1306 &display) {
  renderChildren(display);
}
