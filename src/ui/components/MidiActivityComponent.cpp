#include "ui/components/MidiActivityComponent.h"
#include "Adafruit_SSD1306.h"

#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include "Arduino.h"
#endif

MidiActivityComponent::MidiActivityComponent(int16_t x, int16_t y, uint8_t tamanho)
    : _x(x), _y(y), _tamanho(tamanho) {}

void MidiActivityComponent::render(Adafruit_SSD1306& display) {
    uint32_t now = millis();

    // Desativa se o tempo expirou
    if (_active && (now - _triggerTime) >= DURACAO_MS) {
        _active = false;
    }

    if (_active) {
        // Quadrado preenchido = sinal ativo
        display.fillRect(_x, _y, _tamanho, _tamanho, SSD1306_WHITE);
    } else {
        // Contorno vazio = sem sinal
        display.drawRect(_x, _y, _tamanho, _tamanho, SSD1306_WHITE);
    }
}

void MidiActivityComponent::trigger() {
    _active = true;
    _triggerTime = millis();
}

bool MidiActivityComponent::isActive() const {
    return _active;
}
