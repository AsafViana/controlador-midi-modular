#include "screens/CanalScreen.h"
#include "config.h"
#include "ui/OledApp.h"
#include <cstdio>

CanalScreen::CanalScreen(Storage *storage)
    : _storage(storage), _titulo(0, 0, "Canal MIDI", 1),
      _hint(0, OLED_HEIGHT - 8, "SEL=Salvar  BACK=Canc", 1),
      _valorComp(0, CONTENT_Y + 8, "1", 2) {
  addChild(&_titulo);
  addChild(&_hint);
  addChild(&_valorComp);
}

void CanalScreen::setApp(OledApp *app) { _app = app; }

void CanalScreen::onMount() {
  _canal = _storage->getCanalMidi();
  _canalOriginal = _canal;
  snprintf(_buf, sizeof(_buf), "%d", _canal);
  _valorComp.setText(_buf);
  markDirty();
}

bool CanalScreen::handleBack() {
  // Restaura valor original e volta
  _canal = _canalOriginal;
  if (_app)
    _app->getRouter().pop();
  return true;
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
