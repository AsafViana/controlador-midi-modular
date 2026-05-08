#include "screens/ProgramChangeScreen.h"
#include "config.h"
#include "midi/MidiEngine.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include <cstdio>

ProgramChangeScreen::ProgramChangeScreen(MidiEngine *engine, Storage *storage)
    : _engine(engine), _storage(storage), _titulo(0, 0, "Program Change", 1),
      _hint(0, OLED_HEIGHT - 8, "SEL=Enviar  UP/DN=Num", 1),
      _valorComp(0, CONTENT_Y + 8, "0", 2),
      _labelEnviado(0, CONTENT_Y + 32, "", 1) {
  addChild(&_titulo);
  addChild(&_hint);
  addChild(&_valorComp);
  addChild(&_labelEnviado);
}

void ProgramChangeScreen::setApp(OledApp *app) { _app = app; }

void ProgramChangeScreen::onMount() {
  _program = 0;
  _enviado = false;
  atualizarValor();
  _bufEnviado[0] = '\0';
  _labelEnviado.setText(_bufEnviado);
  markDirty();
}

void ProgramChangeScreen::atualizarValor() {
  snprintf(_bufValor, sizeof(_bufValor), "%d", _program);
  _valorComp.setText(_bufValor);
}

void ProgramChangeScreen::handleInput(NavInput input) {
  uint8_t passo = 1;

  switch (input) {
  case NavInput::LONG_UP:
    passo = 10;
    // fall through
  case NavInput::UP:
    if (_program < 127) {
      _program += passo;
      if (_program > 127)
        _program = 127;
      _enviado = false;
      atualizarValor();
      _bufEnviado[0] = '\0';
      _labelEnviado.setText(_bufEnviado);
      markDirty();
    }
    break;
  case NavInput::LONG_DOWN:
    passo = 10;
    // fall through
  case NavInput::DOWN:
    if (_program > 0) {
      if (_program <= passo)
        _program = 0;
      else
        _program -= passo;
      _enviado = false;
      atualizarValor();
      _bufEnviado[0] = '\0';
      _labelEnviado.setText(_bufEnviado);
      markDirty();
    }
    break;
  case NavInput::SELECT:
    if (_engine && _storage) {
      uint8_t canal = _storage->getCanalMidi();
      _engine->sendProgramChange(_program, canal);
      _enviado = true;
      snprintf(_bufEnviado, sizeof(_bufEnviado), "Enviado! Ch:%d", canal);
      _labelEnviado.setText(_bufEnviado);
      markDirty();
    }
    break;
  default:
    break;
  }
}

void ProgramChangeScreen::render(Adafruit_SSD1306 &display) {
  renderChildren(display);
}
