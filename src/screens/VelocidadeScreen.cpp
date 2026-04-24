#include "screens/VelocidadeScreen.h"
#include "config.h"
#include "ui/OledApp.h"
#include <cstdio>

VelocidadeScreen::VelocidadeScreen(Storage *storage)
    : _storage(storage), _titulo(0, 0, "Velocidade", 1),
      _voltar(OLED_WIDTH - 48, 4, "<Voltar", 1),
      _valorComp(0, CONTENT_Y, "100", 2) {
  addChild(&_titulo);
  addChild(&_voltar);
  addChild(&_valorComp);
}

void VelocidadeScreen::setApp(OledApp *app) { _app = app; }

void VelocidadeScreen::onMount() {
  _velocidade = _storage->getVelocidade();
  snprintf(_buf, sizeof(_buf), "%d", _velocidade);
  _valorComp.setText(_buf);
  markDirty();
}

void VelocidadeScreen::handleInput(NavInput input) {
  uint8_t passo = 1;

  switch (input) {
  case NavInput::LONG_UP:
    passo = 5;
    // fall through
  case NavInput::UP:
    if (_velocidade < 127) {
      _velocidade += passo;
      if (_velocidade > 127)
        _velocidade = 127;
      snprintf(_buf, sizeof(_buf), "%d", _velocidade);
      _valorComp.setText(_buf);
      markDirty();
    }
    break;
  case NavInput::LONG_DOWN:
    passo = 5;
    // fall through
  case NavInput::DOWN:
    if (_velocidade > 1) {
      if (_velocidade <= passo)
        _velocidade = 1;
      else
        _velocidade -= passo;
      snprintf(_buf, sizeof(_buf), "%d", _velocidade);
      _valorComp.setText(_buf);
      markDirty();
    }
    break;
  case NavInput::SELECT:
    _storage->setVelocidade(_velocidade);
    if (_app) {
      _app->showSaveConfirm();
      _app->getRouter().pop();
    }
    break;
  default:
    break;
  }
}

void VelocidadeScreen::render(Adafruit_SSD1306 &display) {
  renderChildren(display);
}
