#include "screens/PerformanceScreen.h"
#include "config.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include <cstdio>

PerformanceScreen::PerformanceScreen(MidiEngine *engine, Storage *storage)
    : _engine(engine), _storage(storage), _titulo(0, 0, "Performance", 1),
      _voltar(OLED_WIDTH - 48, 4, "<Voltar", 1),
      _infoCanal(0, CONTENT_Y, "Canal: 1  Oit: 4", 1),
      _monitorLabel(0, CONTENT_Y + 12, "-- nenhum --", 1),
      _monitorCC(0, CONTENT_Y + 22, "CC: ---  Val: ---", 1),
      _monitorModulo(0, CONTENT_Y + 32, "Modulo: ---", 1),
      _barraCC(0, CONTENT_Y + 42, OLED_WIDTH, 6) {
  addChild(&_titulo);
  addChild(&_voltar);
  addChild(&_infoCanal);
  addChild(&_monitorLabel);
  addChild(&_monitorCC);
  addChild(&_monitorModulo);
  addChild(&_barraCC);
}

void PerformanceScreen::setApp(OledApp *app) { _app = app; }

void PerformanceScreen::onMount() {
  _ultimoCC = 0;
  _barraCC.setValue(0);
  snprintf(_bufMonLabel, sizeof(_bufMonLabel), "-- nenhum --");
  _monitorLabel.setText(_bufMonLabel);
  snprintf(_bufMonCC, sizeof(_bufMonCC), "CC: ---  Val: ---");
  _monitorCC.setText(_bufMonCC);
  snprintf(_bufMonModulo, sizeof(_bufMonModulo), "Modulo: ---");
  _monitorModulo.setText(_bufMonModulo);
  atualizarTextos();
  markDirty();
}

void PerformanceScreen::atualizarTextos() {
  uint8_t canal = _storage->getCanalMidi();
  uint8_t oitava = _storage->getOitava();

  snprintf(_bufCanal, sizeof(_bufCanal), "Ch:%d Oit:%d", canal, oitava);
  _infoCanal.setText(_bufCanal);
}

void PerformanceScreen::handleInput(NavInput input) {
  switch (input) {
  case NavInput::UP: {
    uint8_t oitava = _storage->getOitava();
    if (oitava < 8) {
      _storage->setOitava(oitava + 1);
      atualizarTextos();
      markDirty();
    }
    break;
  }
  case NavInput::DOWN: {
    uint8_t oitava = _storage->getOitava();
    if (oitava > 0) {
      _storage->setOitava(oitava - 1);
      atualizarTextos();
      markDirty();
    }
    break;
  }
  case NavInput::SELECT:
    if (_app)
      _app->getRouter().pop();
    break;
  default:
    break;
  }
}

void PerformanceScreen::atualizarCCInfo(const CCActivityInfo &info) {
  _ultimoCC = info.valor;

  // Linha 1: Nome do controle
  snprintf(_bufMonLabel, sizeof(_bufMonLabel), "%s",
           info.label ? info.label : "???");
  _monitorLabel.setText(_bufMonLabel);

  // Linha 2: CC number + valor
  snprintf(_bufMonCC, sizeof(_bufMonCC), "CC:%d  Val:%d", info.cc, info.valor);
  _monitorCC.setText(_bufMonCC);

  // Linha 3: Módulo de origem
  if (info.isRemoto) {
    snprintf(_bufMonModulo, sizeof(_bufMonModulo), "Modulo: [%02X]",
             info.moduleAddress);
  } else {
    snprintf(_bufMonModulo, sizeof(_bufMonModulo), "Modulo: Local");
  }
  _monitorModulo.setText(_bufMonModulo);

  // Barra de progresso
  _barraCC.setValue((info.valor * 100) / 127);

  markDirty();
}

void PerformanceScreen::atualizarCC(uint8_t valor) {
  // Fallback legado — usa atualizarCCInfo quando possível
  CCActivityInfo info;
  info.label = "???";
  info.cc = 0;
  info.valor = valor;
  info.canal = 0;
  info.isRemoto = false;
  info.moduleAddress = 0;
  atualizarCCInfo(info);
}

void PerformanceScreen::render(Adafruit_SSD1306 &display) {
  renderChildren(display);
}
