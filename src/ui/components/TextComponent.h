#pragma once

#include "ui/UIComponent.h"
#include <cstdint>

// Forward declaration
class Adafruit_SSD1306;

/**
 * Componente de texto para o framework OLED.
 *
 * Renderiza uma string na posição (x, y) com tamanho de fonte e cor configuráveis.
 * Trunca automaticamente o texto que ultrapassaria a largura do display (128px).
 * Trata text == nullptr como string vazia.
 */
class TextComponent : public UIComponent {
public:
    /**
     * @param x        Posição horizontal (pixels)
     * @param y        Posição vertical (pixels)
     * @param text     Texto a renderizar (nullptr tratado como "")
     * @param fontSize Tamanho da fonte: 1, 2 ou 3
     * @param color    Cor do texto: SSD1306_WHITE ou SSD1306_BLACK
     */
    TextComponent(int16_t x, int16_t y, const char* text,
                  uint8_t fontSize = 1, uint16_t color = 1 /* SSD1306_WHITE */);

    void render(Adafruit_SSD1306& display) override;

    /// Atualiza o texto exibido.
    void setText(const char* text);

    /// Atualiza a posição do componente.
    void setPosition(int16_t x, int16_t y);

private:
    int16_t _x;
    int16_t _y;
    const char* _text;
    uint8_t _fontSize;   // 1, 2 ou 3
    uint16_t _color;

    /// Display width limit for truncation (128px)
    static constexpr int16_t DISPLAY_WIDTH = 128;
};
