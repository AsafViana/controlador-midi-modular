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
  _display = new Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

  if (!_display->begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
    Serial.println("Erro: falha ao inicializar display SSD1306");
    return false;
  }

  _midiActivity = MidiActivityComponent(OLED_WIDTH - 8, 1, 6);
  _lastInputTime = millis();

  _display->clearDisplay();
  _display->display();
  return true;
}

void OledApp::showSplash(const char *nome, const char *versao,
                         uint16_t duracaoMs) {
  if (_display == nullptr)
    return;

  _display->clearDisplay();

  _display->setTextSize(2);
  _display->setTextColor(SSD1306_WHITE);

  int16_t nomeLen = 0;
  const char *p = nome;
  while (*p++)
    nomeLen++;
  int16_t nomeX = (OLED_WIDTH - nomeLen * 12) / 2;
  if (nomeX < 0)
    nomeX = 0;
  _display->setCursor(nomeX, 14);
  _display->print(nome);

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
  // Ativa overlay "Salvo!" não-bloqueante
  _showingSaveOverlay = true;
  _saveOverlayStart = millis();
}

void OledApp::setButtonUp(App::Button *btn) { _btnUp = btn; }
void OledApp::setButtonDown(App::Button *btn) { _btnDown = btn; }
void OledApp::setButtonSelect(App::Button *btn) { _btnSelect = btn; }
void OledApp::setButtonBack(App::Button *btn) { _btnBack = btn; }

Router &OledApp::getRouter() { return _router; }
MidiActivityComponent &OledApp::getMidiActivity() { return _midiActivity; }

void OledApp::setIdleScreen(Screen *screen) { _idleScreen = screen; }

void OledApp::setIdleTimeoutSeconds(uint16_t seconds) {
  _idleTimeoutSeconds = seconds;
}

void OledApp::resetIdleTimer() { _lastInputTime = millis(); }

void OledApp::update() {
  if (_display == nullptr)
    return;

  uint32_t now = millis();
  if (now - _lastFrameTime < FRAME_INTERVAL_MS)
    return;
  _lastFrameTime = now;

  // ── Leitura de botões ──────────────────────────────────────────────────
  bool hadInput = false;

  if (_btnUp) {
    ButtonEvent ev = _btnUp->update();
    if (ev == ButtonEvent::PRESSED) {
      _router.handleInput(NavInput::UP);
      hadInput = true;
    } else if (ev == ButtonEvent::LONG_PRESS) {
      _router.handleInput(NavInput::LONG_UP);
      hadInput = true;
    }
  }
  if (_btnDown) {
    ButtonEvent ev = _btnDown->update();
    if (ev == ButtonEvent::PRESSED) {
      _router.handleInput(NavInput::DOWN);
      hadInput = true;
    } else if (ev == ButtonEvent::LONG_PRESS) {
      _router.handleInput(NavInput::LONG_DOWN);
      hadInput = true;
    }
  }
  if (_btnSelect) {
    ButtonEvent ev = _btnSelect->update();
    if (ev == ButtonEvent::PRESSED) {
      _router.handleInput(NavInput::SELECT);
      hadInput = true;
    }
  }
  if (_btnBack) {
    ButtonEvent ev = _btnBack->update();
    if (ev == ButtonEvent::PRESSED) {
      _router.handleInput(NavInput::BACK);
      hadInput = true;
    }
  }

  if (hadInput) {
    _lastInputTime = now;
  }

  // ── Timeout de inatividade ─────────────────────────────────────────────
  if (_idleScreen != nullptr && _idleTimeoutSeconds > 0) {
    uint32_t elapsed = now - _lastInputTime;
    uint32_t timeoutMs = static_cast<uint32_t>(_idleTimeoutSeconds) * 1000UL;
    Screen *current = _router.currentScreen();
    // Só aplica timeout se não estiver já na tela idle ou no menu raiz
    if (elapsed >= timeoutMs && current != nullptr && current != _idleScreen) {
      _router.navigateTo(_idleScreen);
    }
  }

  // ── Overlay "Salvo!" ───────────────────────────────────────────────────
  if (_showingSaveOverlay) {
    if (now - _saveOverlayStart >= SAVE_OVERLAY_DURATION_MS) {
      _showingSaveOverlay = false;
      // Força redesenho da tela normal
      Screen *screen = _router.currentScreen();
      if (screen)
        screen->markDirty();
    } else {
      // Renderiza overlay centralizado
      _display->clearDisplay();
      _display->setTextSize(2);
      _display->setTextColor(SSD1306_WHITE);
      // "Salvo!" = 6 chars * 12px = 72px → centralizado em 128px = x=28
      _display->setCursor(28, 24);
      _display->print("Salvo!");
      _display->display();
      return; // Não renderiza a tela normal durante o overlay
    }
  }

  // ── Renderização normal ────────────────────────────────────────────────
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
