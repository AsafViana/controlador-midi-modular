#include "hardware/StatusLed.h"
#include "hardware/HardwareMap.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "Arduino.h"
#endif

void StatusLed::begin() {
  _rgbEnabled = HardwareMap::RGB_LED_ENABLED;

  if (_rgbEnabled) {
#ifdef ARDUINO
    pinMode(HardwareMap::PIN_LED_R, OUTPUT);
    pinMode(HardwareMap::PIN_LED_G, OUTPUT);
    pinMode(HardwareMap::PIN_LED_B, OUTPUT);
    // Configura canais PWM (ESP32 LEDC)
    ledcAttach(HardwareMap::PIN_LED_R, 5000, 8);
    ledcAttach(HardwareMap::PIN_LED_G, 5000, 8);
    ledcAttach(HardwareMap::PIN_LED_B, 5000, 8);
#endif
  } else {
    pinMode(HardwareMap::PIN_LED, OUTPUT);
  }

  setEstado(Estado::OK);
}

void StatusLed::setEstado(Estado estado) { _estado = estado; }

void StatusLed::triggerMidiFlash() {
  _midiFlashing = true;
  _midiFlashStart = millis();
}

void StatusLed::update() {
  uint32_t now = millis();

  // Flash de atividade MIDI (prioridade sobre estado base)
  if (_midiFlashing) {
    if (now - _midiFlashStart < MIDI_FLASH_DURATION_MS) {
      if (_rgbEnabled) {
        setRGB(0, 0, MAX_BRIGHTNESS); // Azul
      } else {
        setSimpleLed(true);
      }
      return;
    }
    _midiFlashing = false;
  }

  // Estado base
  switch (_estado) {
  case Estado::OFF:
    if (_rgbEnabled)
      setRGB(0, 0, 0);
    else
      setSimpleLed(false);
    break;

  case Estado::OK:
    if (_rgbEnabled)
      setRGB(0, MAX_BRIGHTNESS / 4, 0); // Verde suave
    else
      setSimpleLed(false); // LED simples desligado quando OK
    break;

  case Estado::MIDI_ACTIVE:
    // Pisca azul (500ms on/off)
    if (_rgbEnabled) {
      bool on = ((now / 500) % 2) == 0;
      setRGB(0, 0, on ? MAX_BRIGHTNESS : 0);
    } else {
      bool on = ((now / 500) % 2) == 0;
      setSimpleLed(on);
    }
    break;

  case Estado::ERRO:
    if (_rgbEnabled)
      setRGB(MAX_BRIGHTNESS, 0, 0); // Vermelho
    else
      setSimpleLed(true); // LED simples ligado = erro
    break;

  case Estado::ESPECIAL:
    if (_rgbEnabled)
      setRGB(MAX_BRIGHTNESS / 2, 0, MAX_BRIGHTNESS / 2); // Roxo
    else {
      // Pisca rápido
      bool on = ((now / 200) % 2) == 0;
      setSimpleLed(on);
    }
    break;
  }
}

void StatusLed::setRGB(uint8_t r, uint8_t g, uint8_t b) {
#ifdef ARDUINO
  if (!_rgbEnabled)
    return;
  ledcWrite(HardwareMap::PIN_LED_R, r);
  ledcWrite(HardwareMap::PIN_LED_G, g);
  ledcWrite(HardwareMap::PIN_LED_B, b);
#else
  (void)r;
  (void)g;
  (void)b;
#endif
}

void StatusLed::setSimpleLed(bool on) {
  digitalWrite(HardwareMap::PIN_LED, on ? HIGH : LOW);
}
