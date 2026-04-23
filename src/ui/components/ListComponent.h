#pragma once

#include "ui/UIComponent.h"
#include <cstdint>

// Forward declaration
class Adafruit_SSD1306;

/**
 * Componente de lista selecionável para o framework OLED.
 *
 * Renderiza uma lista de itens de texto com indicador visual (inversão)
 * no item selecionado. Suporta navegação via botões cima/baixo e scroll
 * automático para manter o item selecionado visível.
 *
 * Os ponteiros para strings (const char**) devem ter lifetime maior que
 * o componente (tipicamente constantes ou buffers estáticos).
 */
class ListComponent : public UIComponent {
public:
    /**
     * @param x        Posição horizontal (pixels)
     * @param y        Posição vertical (pixels)
     * @param w        Largura da lista (pixels)
     * @param h        Altura da lista (pixels)
     * @param fontSize Tamanho da fonte: 1, 2 ou 3 (default: 1)
     */
    ListComponent(int16_t x, int16_t y, int16_t w, int16_t h,
                  uint8_t fontSize = 1);

    void render(Adafruit_SSD1306& display) override;
    bool handleInput(ButtonEvent event) override;

    /// Define os itens da lista. nullptr items com count 0 limpa a lista.
    void setItems(const char** items, uint8_t count);

    /// Retorna o índice do item atualmente selecionado.
    uint8_t getSelectedIndex() const;

    /// Define o evento de botão para navegar para cima.
    void setUpButton(ButtonEvent upEvent);

    /// Define o evento de botão para navegar para baixo.
    void setDownButton(ButtonEvent downEvent);

    /// Tipo do callback chamado quando a seleção muda.
    using OnSelectionChanged = void(*)(uint8_t index);

    /// Registra callback para mudança de seleção.
    void onSelectionChanged(OnSelectionChanged callback);

private:
    int16_t _x;
    int16_t _y;
    int16_t _w;
    int16_t _h;
    uint8_t _fontSize;

    const char** _items = nullptr;
    uint8_t _itemCount = 0;
    uint8_t _selectedIndex = 0;
    uint8_t _scrollOffset = 0;
    uint8_t _visibleCount = 0;

    ButtonEvent _upEvent = ButtonEvent::NONE;
    ButtonEvent _downEvent = ButtonEvent::NONE;
    OnSelectionChanged _onSelectionChanged = nullptr;

    /// Ajusta scrollOffset para manter selectedIndex na janela visível.
    void scrollToSelected();

    /// Padding à esquerda do texto dentro de cada item (pixels).
    static constexpr int16_t TEXT_PADDING_LEFT = 2;
};
