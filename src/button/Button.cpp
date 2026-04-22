#include "Button.h"

namespace App {

Button::Button(uint8_t pin, bool activeLow)
  : _pin(pin), _activeLow(activeLow) {}

void Button::begin() {
  pinMode(_pin, _activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
}

bool Button::rawRead() const {
  bool raw = digitalRead(_pin);
  return _activeLow ? !raw : raw;
}

bool Button::isHeld() const { return _held; }

uint32_t Button::heldDuration() const {
  if (!_held) return 0;
  return millis() - _pressTime;
}

ButtonEvent Button::update() {
  bool raw = rawRead();
  uint32_t now = millis();

  if (raw != _lastRaw) {
    _lastDebounce = now;
    _lastRaw = raw;
  }

  if ((now - _lastDebounce) < DEBOUNCE_MS) return ButtonEvent::NONE;

  ButtonEvent evt = ButtonEvent::NONE;

  if (raw && !_state) {
    _state     = true;
    _held      = true;
    _pressTime = now;
    _longFired = false;
    evt = ButtonEvent::PRESSED;
  }
  else if (!raw && _state) {
    _state       = false;
    _held        = false;
    _releaseTime = now;
    if (!_longFired) _clickCount++;
    evt = ButtonEvent::RELEASED;
  }
  else if (_held && !_longFired && (now - _pressTime) >= LONG_PRESS_MS) {
    _longFired  = true;
    _clickCount = 0;
    evt = ButtonEvent::LONG_PRESS;
  }
  else if (!_held && _clickCount > 0 && (now - _releaseTime) >= DOUBLE_CLICK_MS) {
    if (_clickCount == 1)      evt = ButtonEvent::SINGLE_CLICK;
    else if (_clickCount >= 2) evt = ButtonEvent::DOUBLE_CLICK;
    _clickCount = 0;
  }

  return evt;
}

} // namespace App
