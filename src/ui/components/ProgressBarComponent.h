#pragma once

#include "ui/UIComponent.h"
#include <cstdint>

// Forward declaration
class Adafruit_SSD1306;

/**
 * Componente de barra de progresso para o framework OLED.
 *
 * Renderiza uma barra horizontal com borda de 1px e preenchimento
 * proporcional ao valor (0–100). Valores fora da faixa são clamped.
 */
class ProgressBarComponent : public UIComponent {
public:
    /**
     * @param x Posição horizontal (pixels)
     * @param y Posição vertical (pixels)
     * @param w Largura total da barra (pixels)
     * @param h Altura total da barra (pixels)
     */
    ProgressBarComponent(int16_t x, int16_t y, int16_t w, int16_t h);

    void render(Adafruit_SSD1306& display) override;

    /// Atualiza o valor da barra (0–100). Valores > 100 são clamped para 100.
    void setValue(uint8_t value);

    /// Retorna o valor atual da barra.
    uint8_t getValue() const;

private:
    int16_t _x;
    int16_t _y;
    int16_t _w;
    int16_t _h;
    uint8_t _value = 0;
};
