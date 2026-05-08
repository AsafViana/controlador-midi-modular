#include "screens/OitavaScreen.h"
#include "config.h"
#include "ui/OledApp.h"
#include <cstdio>

OitavaScreen::OitavaScreen(Storage *storage)
    : _storage(storage), _titulo(0, 0, "Oitava", 1),
      _hint(0, OLED_HEIGHT - 8, "SEL=Salvar  BACK=Canc", 1),
      _valorComp(0, CONTENT_Y + 8, "4", 2) {
  addChild(&_titulo);
  addChild(&_hint);
  addChild(&_valorComp);
}

void OitavaScreen::setApp(OledApp *app) { _app = app; }

void OitavaScreen::onMount() {
  _oitava = _storage->getOitava();
  _oitavaOriginal = _oitava;
  snprintf(_buf, sizeof(_buf), "%d", _oitava);
  _valorComp.setText(_buf);
  markDirty();
}

bool OitavaScreen::handleBack() {
  // Restaura valor original e volta
  _oitava = _oitavaOriginal;
  if (_app)
    _app->getRouter().pop();
  return true;
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
