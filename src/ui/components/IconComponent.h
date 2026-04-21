#pragma once

#include "ui/UIComponent.h"
#include <cstdint>

// Forward declaration
class Adafruit_SSD1306;

/**
 * Componente de ícone (bitmap) para o framework OLED.
 *
 * Renderiza um bitmap monocromático na posição (x, y) com dimensões (w, h).
 * O ponteiro para o bitmap deve apontar para dados em PROGMEM.
 * Se o ponteiro for nullptr, nada é renderizado.
 */
class IconComponent : public UIComponent {
public:
    /**
     * @param x      Posição horizontal (pixels)
     * @param y      Posição vertical (pixels)
     * @param bitmap Ponteiro para bitmap em PROGMEM (nullptr = nada renderizado)
     * @param w      Largura do bitmap (pixels)
     * @param h      Altura do bitmap (pixels)
     * @param color  Cor do bitmap: SSD1306_WHITE ou SSD1306_BLACK
     */
    IconComponent(int16_t x, int16_t y,
                  const uint8_t* bitmap, uint8_t w, uint8_t h,
                  uint16_t color = 1 /* SSD1306_WHITE */);

    void render(Adafruit_SSD1306& display) override;

private:
    int16_t _x;
    int16_t _y;
    const uint8_t* _bitmap;  // ponteiro para bitmap em PROGMEM
    uint8_t _w;
    uint8_t _h;
    uint16_t _color;
};
