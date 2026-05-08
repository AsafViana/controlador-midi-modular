#include "screens/WizardScreen.h"
#include "config.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include <cstdio>

#ifdef ARDUINO
#include <Preferences.h>
#endif

WizardScreen::WizardScreen(OledApp *app, Storage *storage)
    : _app(app), _storage(storage), _titulo(0, 0, "Assistente", 1) {
  addChild(&_titulo);
}

bool WizardScreen::deveExibir(Storage *storage) {
#ifdef ARDUINO
  Preferences p;
  p.begin("midi_meta", true);
  bool done = p.getBool("wizard_done", false);
  p.end();
  return !done;
#else
  (void)storage;
  return false;
#endif
}

void WizardScreen::marcarConcluido(Storage *storage) {
#ifdef ARDUINO
  Preferences p;
  p.begin("midi_meta", false);
  p.putBool("wizard_done", true);
  p.end();
#else
  (void)storage;
#endif
}

void WizardScreen::onMount() {
  _etapa = Etapa::BOAS_VINDAS;
  _canal = _storage->getCanalMidi();
  markDirty();
}

bool WizardScreen::handleBack() {
  // BACK pula o wizard (marca como concluído)
  marcarConcluido(_storage);
  if (_app)
    _app->getRouter().pop();
  return true;
}

void WizardScreen::handleInput(NavInput input) {
  switch (_etapa) {
  case Etapa::BOAS_VINDAS:
    if (input == NavInput::SELECT) {
      _etapa = Etapa::CANAL;
      markDirty();
    }
    break;

  case Etapa::CANAL:
    switch (input) {
    case NavInput::UP:
      if (_canal < 16) {
        _canal++;
        markDirty();
      }
      break;
    case NavInput::DOWN:
      if (_canal > 1) {
        _canal--;
        markDirty();
      }
      break;
    case NavInput::SELECT:
      _storage->setCanalMidi(_canal);
      _etapa = Etapa::CONCLUIDO;
      markDirty();
      break;
    default:
      break;
    }
    break;

  case Etapa::CONCLUIDO:
    if (input == NavInput::SELECT) {
      marcarConcluido(_storage);
      if (_app)
        _app->getRouter().pop();
    }
    break;
  }
}

void WizardScreen::render(Adafruit_SSD1306 &display) {
  _titulo.render(display);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  switch (_etapa) {
  case Etapa::BOAS_VINDAS:
    display.setCursor(0, CONTENT_Y + 2);
    display.print("Bem-vindo!");
    display.setCursor(0, CONTENT_Y + 14);
    display.print("UP/DOWN = Navegar");
    display.setCursor(0, CONTENT_Y + 24);
    display.print("SELECT = Confirmar");
    display.setCursor(0, CONTENT_Y + 34);
    display.print("BACK = Voltar");
    display.setCursor(0, CONTENT_Y + 46);
    display.print("SELECT = Continuar");
    break;

  case Etapa::CANAL:
    display.setCursor(0, CONTENT_Y + 2);
    display.print("Canal MIDI:");
    snprintf(_bufCanal, sizeof(_bufCanal), "%d", _canal);
    display.setTextSize(2);
    display.setCursor(0, CONTENT_Y + 16);
    display.print(_bufCanal);
    display.setTextSize(1);
    display.setCursor(0, CONTENT_Y + 38);
    display.print("UP/DN=Mudar SEL=OK");
    break;

  case Etapa::CONCLUIDO:
    display.setCursor(0, CONTENT_Y + 8);
    display.print("Configurado!");
    display.setCursor(0, CONTENT_Y + 22);
    display.print("Bom som!");
    display.setCursor(0, CONTENT_Y + 40);
    display.print("SELECT = Iniciar");
    break;
  }
}
