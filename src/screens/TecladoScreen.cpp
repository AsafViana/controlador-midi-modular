#include "screens/TecladoScreen.h"
#include "config.h"
#include "ui/OledApp.h"

TecladoScreen::TecladoScreen(Storage *storage)
    : _storage(storage), _titulo(0, 0, "Teclado", 1),
      _voltar(OLED_WIDTH - 48, 4, "<Voltar", 1),
      _valorComp(0, CONTENT_Y, "ON", 2) {
  addChild(&_titulo);
  addChild(&_voltar);
  addChild(&_valorComp);
}

void TecladoScreen::setApp(OledApp *app) { _app = app; }

void TecladoScreen::onMount() {
  _habilitado = _storage->isTecladoHabilitado();
  _valorComp.setText(_habilitado ? "ON" : "OFF");
  markDirty();
}

void TecladoScreen::handleInput(NavInput input) {
  switch (input) {
  case NavInput::UP:
  case NavInput::DOWN:
    _habilitado = !_habilitado;
    _valorComp.setText(_habilitado ? "ON" : "OFF");
    markDirty();
    break;
  case NavInput::SELECT:
    _storage->setTecladoHabilitado(_habilitado);
    if (_app) {
      _app->showSaveConfirm();
      _app->getRouter().pop();
    }
    break;
  default:
    break;
  }
}

void TecladoScreen::render(Adafruit_SSD1306 &display) {
  renderChildren(display);
}
