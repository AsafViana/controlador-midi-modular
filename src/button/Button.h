#pragma once
#include <Arduino.h>

// Tipos de evento que o botão pode gerar
enum class ButtonEvent {
  NONE,
  PRESSED,       // pressionado (borda de descida)
  RELEASED,      // solto (borda de subida)
  SINGLE_CLICK,  // clique único (solto antes de LONG_PRESS_MS)
  LONG_PRESS,    // mantido por LONG_PRESS_MS
  DOUBLE_CLICK   // dois cliques rápidos
};

namespace App {

class Button {
public:
  static constexpr uint16_t DEBOUNCE_MS     = 50;
  static constexpr uint16_t LONG_PRESS_MS   = 800;
  static constexpr uint16_t DOUBLE_CLICK_MS = 300;

  Button(uint8_t pin, bool activeLow = true);

  void begin();
  ButtonEvent update();

  bool isHeld() const;
  uint32_t heldDuration() const;

private:
  uint8_t  _pin;
  bool     _activeLow;

  bool     _lastRaw      = false;
  bool     _state        = false;
  bool     _held         = false;

  uint32_t _lastDebounce = 0;
  uint32_t _pressTime    = 0;
  uint32_t _releaseTime  = 0;

  bool     _longFired    = false;
  uint8_t  _clickCount   = 0;

  bool rawRead() const;
};

} // namespace App
