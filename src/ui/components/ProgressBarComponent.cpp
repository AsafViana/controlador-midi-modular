#include "ui/components/ProgressBarComponent.h"
#include "Adafruit_SSD1306.h"

ProgressBarComponent::ProgressBarComponent(int16_t x, int16_t y, int16_t w, int16_t h)
    : _x(x), _y(y), _w(w), _h(h) {}

void ProgressBarComponent::render(Adafruit_SSD1306& display) {
    // Draw 1px border around the entire bar
    display.drawRect(_x, _y, _w, _h, SSD1306_WHITE);

    // Draw proportional fill inside the border
    if (_value > 0 && _w > 2 && _h > 2) {
        int16_t innerWidth = _w - 2;
        int16_t fillWidth = (_value * innerWidth) / 100;
        if (fillWidth > 0) {
            display.fillRect(_x + 1, _y + 1, fillWidth, _h - 2, SSD1306_WHITE);
        }
    }
}

void ProgressBarComponent::setValue(uint8_t value) {
    _value = (value > 100) ? 100 : value;
}

uint8_t ProgressBarComponent::getValue() const {
    return _value;
}
