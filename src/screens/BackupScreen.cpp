#include "screens/BackupScreen.h"
#include "config.h"
#include "midi/SysExManager.h"
#include "ui/OledApp.h"

const char *BackupScreen::_opcoes[] = {"Enviar Config", "Receber Config"};

BackupScreen::BackupScreen(OledApp *app, SysExManager *sysex)
    : _app(app), _sysex(sysex), _titulo(0, 0, "Backup SysEx", 1),
      _lista(0, CONTENT_Y, OLED_WIDTH, CONTENT_HEIGHT, 1) {
  addChild(&_titulo);
  addChild(&_lista);
  _lista.setItems(_opcoes, NUM_OPCOES);
  _lista.setUpButton(ButtonEvent::LONG_PRESS);
  _lista.setDownButton(ButtonEvent::SINGLE_CLICK);
}

void BackupScreen::onMount() { markDirty(); }

void BackupScreen::handleInput(NavInput input) {
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
    if (_lista.getSelectedIndex() == 0) {
      // Enviar dump
      if (_sysex)
        _sysex->sendDump();
      if (_app)
        _app->showSaveConfirm();
    }
    // Receber: o SysEx é processado automaticamente pelo MidiEngine::update()
    // quando chega uma mensagem. Não precisa de ação aqui.
    break;
  default:
    break;
  }
}

void BackupScreen::render(Adafruit_SSD1306 &display) {
  renderChildren(display);
}
