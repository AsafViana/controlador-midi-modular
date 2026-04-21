#pragma once

#include "button/Button.h"

// Forward declaration — full definition provided by Adafruit SSD1306 library
// or by test mocks in the native environment.
class Adafruit_SSD1306;

/**
 * Classe base abstrata para todos os componentes visuais do framework.
 *
 * Cada componente implementa render() para desenhar no buffer do display
 * e opcionalmente handleInput() para processar eventos de botão.
 */
class UIComponent {
public:
    virtual ~UIComponent() = default;

    /// Desenha o componente no buffer do display (sem enviar à tela).
    virtual void render(Adafruit_SSD1306& display) = 0;

    /// Processa um evento de botão. Retorna true se o evento foi consumido.
    virtual bool handleInput(ButtonEvent event) { return false; }
};
