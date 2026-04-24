#include "ui/OledApp.h"
#include "button/Button.h"
#include "config.h"
#include "hardware/HardwareMap.h"

#ifdef ARDUINO
#include <Wire.h>
#else
#include "Serial.h"
#include "Wire.h"

#endif

bool OledApp::begin(uint8_t i2cAddress) {
  Wire.begin(HardwareMap::PIN_I2C_SDA, HardwareMap::PIN_I2C_SCL);

  // Instancia apenas aqui, apos Wire.begin() — nunca no construtor global
  _display = new Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

  if (!_display->begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
    Serial.println("Erro: falha ao inicializar display SSD1306");
    return false;
  }

  _midiActivity = MidiActivityComponent(OLED_WIDTH - 8, 1, 6);

  _display->clearDisplay();
  _display->display();
  return true;
}

void OledApp::showSplash(const char *nome, const char *versao,
                         uint16_t duracaoMs) {
  if (_display == nullptr)
    return;

  _display->clearDisplay();

  // Nome do produto centralizado, fonte grande
  _display->setTextSize(2);
  _display->setTextColor(SSD1306_WHITE);

  // Calcular posição X para centralizar (cada char size 2 = 12px largura)
  int16_t nomeLen = 0;
  const char *p = nome;
  while (*p++)
    nomeLen++;
  int16_t nomeX = (OLED_WIDTH - nomeLen * 12) / 2;
  if (nomeX < 0)
    nomeX = 0;
  _display->setCursor(nomeX, 14);
  _display->print(nome);

  // Versão centralizada, fonte pequena
  _display->setTextSize(1);
  int16_t verLen = 0;
  p = versao;
  while (*p++)
    verLen++;
  int16_t verX = (OLED_WIDTH - verLen * 6) / 2;
  if (verX < 0)
    verX = 0;
  _display->setCursor(verX, 40);
  _display->print(versao);

  _display->display();
  delay(duracaoMs);
}

void OledApp::showSaveConfirm() {
  if (_display == nullptr)
    return;

  // Inverte o display brevemente como feedback visual
  _display->invertDisplay(true);
  delay(150);
  _display->invertDisplay(false);
}

void OledApp::setButtonUp(App::Button *btn) { _btnUp = btn; }
void OledApp::setButtonDown(App::Button *btn) { _btnDown = btn; }
void OledApp::setButtonSelect(App::Button *btn) { _btnSelect = btn; }

Router &OledApp::getRouter() { return _router; }
MidiActivityComponent &OledApp::getMidiActivity() { return _midiActivity; }

void OledApp::update() {
  if (_display == nullptr)
    return;

  uint32_t now = millis();
  if (now - _lastFrameTime < FRAME_INTERVAL_MS)
    return;
  _lastFrameTime = now;

  if (_btnUp) {
    ButtonEvent ev = _btnUp->update();
    if (ev == ButtonEvent::PRESSED) {
      _router.handleInput(NavInput::UP);
    } else if (ev == ButtonEvent::LONG_PRESS) {
      _router.handleInput(NavInput::LONG_UP);
    }
  }
  if (_btnDown) {
    ButtonEvent ev = _btnDown->update();
    if (ev == ButtonEvent::PRESSED) {
      _router.handleInput(NavInput::DOWN);
    } else if (ev == ButtonEvent::LONG_PRESS) {
      _router.handleInput(NavInput::LONG_DOWN);
    }
  }
  if (_btnSelect) {
    ButtonEvent ev = _btnSelect->update();
    if (ev == ButtonEvent::PRESSED) {
      _router.handleInput(NavInput::SELECT);
    }
  }

  Screen *screen = _router.currentScreen();
  bool needsRedraw = (screen != nullptr && screen->isDirty());
  bool midiActive = _midiActivity.isActive();

  if (needsRedraw || midiActive) {
    _display->clearDisplay();
    if (screen != nullptr) {
      screen->render(*_display);
      screen->clearDirty();
    }
    _midiActivity.render(*_display);
    _display->display();
  }
}
