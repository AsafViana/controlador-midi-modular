#include "ui/components/TextComponent.h"
#include "Adafruit_SSD1306.h"

#include <cstring>

TextComponent::TextComponent(int16_t x, int16_t y, const char* text,
                             uint8_t fontSize, uint16_t color)
    : _x(x), _y(y), _text(text), _fontSize(fontSize), _color(color) {}

void TextComponent::render(Adafruit_SSD1306& display) {
    // Treat nullptr as empty string
    const char* text = _text ? _text : "";

    // Nothing to render for empty string
    if (text[0] == '\0') {
        return;
    }

    display.setTextSize(_fontSize);
    display.setTextColor(_color);
    display.setCursor(_x, _y);

    // Calculate text width to check if truncation is needed
    int16_t x1, y1;
    uint16_t textW, textH;
    display.getTextBounds(text, _x, _y, &x1, &y1, &textW, &textH);

    if (static_cast<int16_t>(_x + textW) <= DISPLAY_WIDTH) {
        // Text fits — render as-is
        display.print(text);
    } else {
        // Text overflows — truncate by trying progressively shorter substrings
        size_t len = strlen(text);
        char truncated[128];

        // Find the longest substring that fits within DISPLAY_WIDTH
        while (len > 0) {
            len--;
            if (len == 0) {
                // No characters fit — render nothing
                return;
            }
            // Copy substring to buffer
            size_t copyLen = (len < sizeof(truncated) - 1) ? len : sizeof(truncated) - 1;
            memcpy(truncated, text, copyLen);
            truncated[copyLen] = '\0';

            display.getTextBounds(truncated, _x, _y, &x1, &y1, &textW, &textH);
            if (static_cast<int16_t>(_x + textW) <= DISPLAY_WIDTH) {
                display.print(truncated);
                return;
            }
        }
        // If even a single character doesn't fit, render nothing
    }
}

void TextComponent::setText(const char* text) {
    _text = text;
}

void TextComponent::setPosition(int16_t x, int16_t y) {
    _x = x;
    _y = y;
}
