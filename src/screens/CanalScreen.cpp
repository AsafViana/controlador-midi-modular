#include "screens/CanalScreen.h"
#include "config.h"
#include "ui/OledApp.h"
#include <cstdio>

CanalScreen::CanalScreen(Storage *storage)
    : _storage(storage), _titulo(0, 0, "Canal MIDI", 1),
      _voltar(OLED_WIDTH - 48, 4, "<Voltar", 1),
      _valorComp(0, CONTENT_Y, "1", 2) {
  addChild(&_titulo);
  addChild(&_voltar);
  addChild(&_valorComp);
}

void CanalScreen::setApp(OledApp *app) { _app = app; }

void CanalScreen::onMount() {
  _canal = _storage->getCanalMidi();
  snprintf(_buf, sizeof(_buf), "%d", _canal);
  _valorComp.setText(_buf);
  markDirty();
}

void CanalScreen::handleInput(NavInput input) {
  uint8_t passo = 1;

  switch (input) {
  case NavInput::LONG_UP:
    passo = 3;
    // fall through
  case NavInput::UP:
    if (_canal < 16) {
      _canal += passo;
      if (_canal > 16)
        _canal = 16;
      snprintf(_buf, sizeof(_buf), "%d", _canal);
      _valorComp.setText(_buf);
      markDirty();
    }
    break;
  case NavInput::LONG_DOWN:
    passo = 3;
    // fall through
  case NavInput::DOWN:
    if (_canal > 1) {
      if (_canal <= passo)
        _canal = 1;
      else
        _canal -= passo;
      snprintf(_buf, sizeof(_buf), "%d", _canal);
      _valorComp.setText(_buf);
      markDirty();
    }
    break;
  case NavInput::SELECT:
    _storage->setCanalMidi(_canal);
    if (_app) {
      _app->showSaveConfirm();
      _app->getRouter().pop();
    }
    break;
  default:
    break;
  }
}

void CanalScreen::render(Adafruit_SSD1306 &display) { renderChildren(display); }
