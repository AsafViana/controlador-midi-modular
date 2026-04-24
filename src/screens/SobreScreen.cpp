#include "screens/SobreScreen.h"
#include "config.h"
#include "hardware/HardwareMap.h"
#include "hardware/UnifiedControlList.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include <cstdio>

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "dev"
#endif

SobreScreen::SobreScreen(Storage *storage, UnifiedControlList *ucl)
    : _storage(storage), _ucl(ucl), _titulo(0, 0, "Sobre", 1),
      _voltar(OLED_WIDTH - 48, 4, "<Voltar", 1),
      _sobre(0, CONTENT_Y, "Controlador MIDI", 1),
      _versao(0, CONTENT_Y + 10, FIRMWARE_VERSION, 1),
      _infoCanal(0, CONTENT_Y + 24, "Canal: 1", 1),
      _infoControles(0, CONTENT_Y + 34, "Controles: 4", 1) {
  addChild(&_titulo);
  addChild(&_voltar);
  addChild(&_sobre);
  addChild(&_versao);
  addChild(&_infoCanal);
  addChild(&_infoControles);
}

void SobreScreen::setApp(OledApp *app) { _app = app; }

void SobreScreen::onMount() {
  uint8_t canal = _storage->getCanalMidi();
  snprintf(_bufCanal, sizeof(_bufCanal), "Canal MIDI: %d", canal);
  _infoCanal.setText(_bufCanal);

  uint8_t total = HardwareMap::NUM_CONTROLES;
  if (_ucl) {
    total = _ucl->getNumControles();
  }
  uint8_t locais = HardwareMap::NUM_CONTROLES;
  uint8_t remotos = total - locais;

  if (remotos > 0) {
    snprintf(_bufControles, sizeof(_bufControles), "Ctrl: %d (%d+%dR)", total,
             locais, remotos);
  } else {
    snprintf(_bufControles, sizeof(_bufControles), "Controles: %d", total);
  }
  _infoControles.setText(_bufControles);

  markDirty();
}

void SobreScreen::handleInput(NavInput input) {
  if (input == NavInput::SELECT) {
    if (_app)
      _app->getRouter().pop();
  }
}

void SobreScreen::render(Adafruit_SSD1306 &display) { renderChildren(display); }
