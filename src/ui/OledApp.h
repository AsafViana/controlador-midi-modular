#pragma once

#include "Adafruit_SSD1306.h"
#include "ui/NavInput.h"
#include "ui/Router.h"
#include "ui/components/MidiActivityComponent.h"
#include <stdint.h>

namespace App {
class Button;
}

class Screen;

class OledApp {
public:
  OledApp() : _midiActivity(112, 0, 6), _display(nullptr) {}
  ~OledApp() { delete _display; }

  bool begin(uint8_t i2cAddress = 0x3C);
  void showSplash(const char *nome, const char *versao,
                  uint16_t duracaoMs = 1500);
  void showSaveConfirm();
  void update();

  void setButtonUp(App::Button *btn);
  void setButtonDown(App::Button *btn);
  void setButtonSelect(App::Button *btn);
  void setButtonBack(App::Button *btn);

  Router &getRouter();
  MidiActivityComponent &getMidiActivity();

  /// Define a tela raiz para timeout de inatividade (volta para ela após N
  /// segundos)
  void setIdleScreen(Screen *screen);

  /// Define o timeout de inatividade em segundos (0 = desligado)
  void setIdleTimeoutSeconds(uint16_t seconds);

  /// Reseta o timer de inatividade (chamado internamente a cada input)
  void resetIdleTimer();

  /// Notifica atividade externa (controles, potenciômetros, etc.)
  /// Acorda o display se estiver em screensaver e reseta o timer.
  void notifyExternalActivity();

  /// Define tempos do screensaver em segundos (0 = desligado)
  void setScreensaverTimes(uint16_t dimSeconds, uint16_t offSeconds);

private:
  Adafruit_SSD1306
      *_display; // instanciado em begin(), nao no construtor global
  Router _router;
  MidiActivityComponent _midiActivity;

  App::Button *_btnUp = nullptr;
  App::Button *_btnDown = nullptr;
  App::Button *_btnSelect = nullptr;
  App::Button *_btnBack = nullptr;

  uint32_t _lastFrameTime = 0;
  static constexpr uint32_t FRAME_INTERVAL_MS = 33;

  // Timeout de inatividade
  Screen *_idleScreen = nullptr;
  uint32_t _lastInputTime = 0;
  uint16_t _idleTimeoutSeconds = 60; // padrão: 60s

  // Feedback "Salvo!" textual (overlay não-bloqueante)
  bool _showingSaveOverlay = false;
  uint32_t _saveOverlayStart = 0;
  static constexpr uint32_t SAVE_OVERLAY_DURATION_MS = 800;

  // Screensaver (proteção OLED)
  enum class DisplayState : uint8_t { ACTIVE, DIMMED, OFF };
  DisplayState _displayState = DisplayState::ACTIVE;
  uint16_t _dimTimeoutSeconds = 120; // 2 min para dim
  uint16_t _offTimeoutSeconds = 300; // 5 min para desligar

  void wakeDisplay();
};
