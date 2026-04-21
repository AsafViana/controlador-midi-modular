#include "ui/components/IconComponent.h"
#include "Adafruit_SSD1306.h"

IconComponent::IconComponent(int16_t x, int16_t y,
                             const uint8_t* bitmap, uint8_t w, uint8_t h,
                             uint16_t color)
    : _x(x), _y(y), _bitmap(bitmap), _w(w), _h(h), _color(color) {}

void IconComponent::render(Adafruit_SSD1306& display) {
    // Do not render if bitmap pointer is null
    if (_bitmap == nullptr) {
        return;
    }

    display.drawBitmap(_x, _y, _bitmap, _w, _h, _color);
}
