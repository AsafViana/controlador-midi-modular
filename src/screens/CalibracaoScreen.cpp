#include "screens/CalibracaoScreen.h"
#include "config.h"
#include "hardware/HardwareMap.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include <cstdio>

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "Arduino.h"
#endif

CalibracaoScreen::CalibracaoScreen(Storage *storage)
    : _storage(storage), _titulo(0, 0, "Calibracao", 1) {
  addChild(&_titulo);
}

void CalibracaoScreen::setApp(OledApp *app) { _app = app; }

void CalibracaoScreen::setControleIndex(uint8_t index) {
  _controleIndex = index;
}

void CalibracaoScreen::onMount() {
  _etapa = Etapa::MINIMO;
  _capturedMin = 0;
  _capturedMax = 4095;
  markDirty();
}

bool CalibracaoScreen::handleBack() {
  // Cancela calibração sem salvar
  if (_app)
    _app->getRouter().pop();
  return true;
}

uint16_t CalibracaoScreen::lerADCBruto() const {
  if (_controleIndex >= HardwareMap::NUM_CONTROLES)
    return 0;
  return static_cast<uint16_t>(
      analogRead(HardwareMap::getGpio(_controleIndex)));
}

void CalibracaoScreen::handleInput(NavInput input) {
  if (input != NavInput::SELECT)
    return;

  switch (_etapa) {
  case Etapa::MINIMO:
    _capturedMin = lerADCBruto();
    _etapa = Etapa::MAXIMO;
    markDirty();
    break;
  case Etapa::MAXIMO:
    _capturedMax = lerADCBruto();
    // Garante que min < max (troca se necessário)
    if (_capturedMin > _capturedMax) {
      uint16_t tmp = _capturedMin;
      _capturedMin = _capturedMax;
      _capturedMax = tmp;
    }
    // Margem de segurança: min não pode ser igual a max
    if (_capturedMax - _capturedMin < 100) {
      // Calibração inválida — range muito pequeno
      _etapa = Etapa::MINIMO;
      markDirty();
      break;
    }
    _storage->setCalibration(_controleIndex, _capturedMin, _capturedMax);
    _etapa = Etapa::CONCLUIDO;
    markDirty();
    break;
  case Etapa::CONCLUIDO:
    if (_app) {
      _app->showSaveConfirm();
      _app->getRouter().pop();
    }
    break;
  }
}

void CalibracaoScreen::render(Adafruit_SSD1306 &display) {
  renderChildren(display);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  const char *label = HardwareMap::getLabel(_controleIndex);
  display.setCursor(0, CONTENT_Y);
  display.print(label);

  switch (_etapa) {
  case Etapa::MINIMO: {
    display.setCursor(0, CONTENT_Y + 14);
    display.print("Gire ate o MINIMO");
    uint16_t atual = lerADCBruto();
    snprintf(_bufInfo, sizeof(_bufInfo), "ADC: %d", atual);
    display.setCursor(0, CONTENT_Y + 26);
    display.print(_bufInfo);
    display.setCursor(0, CONTENT_Y + 40);
    display.print("SELECT = Capturar");
    break;
  }
  case Etapa::MAXIMO: {
    display.setCursor(0, CONTENT_Y + 14);
    display.print("Gire ate o MAXIMO");
    uint16_t atual = lerADCBruto();
    snprintf(_bufInfo, sizeof(_bufInfo), "ADC: %d  Min: %d", atual,
             _capturedMin);
    display.setCursor(0, CONTENT_Y + 26);
    display.print(_bufInfo);
    display.setCursor(0, CONTENT_Y + 40);
    display.print("SELECT = Capturar");
    break;
  }
  case Etapa::CONCLUIDO:
    display.setCursor(0, CONTENT_Y + 14);
    display.print("Calibrado!");
    snprintf(_bufInfo, sizeof(_bufInfo), "Min:%d Max:%d", _capturedMin,
             _capturedMax);
    display.setCursor(0, CONTENT_Y + 26);
    display.print(_bufInfo);
    display.setCursor(0, CONTENT_Y + 40);
    display.print("SELECT = OK");
    break;
  }
}
