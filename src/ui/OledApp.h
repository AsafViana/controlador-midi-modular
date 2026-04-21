#pragma once

#include <stdint.h>
#include "Adafruit_SSD1306.h"
#include "ui/Router.h"

class Button;

/**
 * OledApp — fachada principal do framework de UI OLED.
 *
 * Ponto de entrada para o desenvolvedor: inicializa o display,
 * registra botões, expõe o Router para navegação, e executa o
 * loop de renderização via update().
 */
class OledApp {
public:
    /**
     * Inicializa o display SSD1306 via I2C.
     * @param i2cAddress Endereço I2C do display (padrão 0x3C).
     * @return true se a inicialização foi bem-sucedida, false caso contrário.
     */
    bool begin(uint8_t i2cAddress = 0x3C);

    /**
     * Loop principal — chama no loop() do Arduino.
     * Consulta botões, encaminha eventos, e redesenha se necessário.
     */
    void update();

    /**
     * Registra um botão para ser consultado a cada update().
     * Máximo de MAX_BUTTONS botões. Ignora silenciosamente se
     * o limite foi atingido ou se o ponteiro é nullptr.
     */
    void addButton(Button* button);

    /**
     * Retorna referência ao Router interno para navegação de Screens.
     */
    Router& getRouter();

private:
    Adafruit_SSD1306 _display;
    Router _router;

    static constexpr uint8_t MAX_BUTTONS = 4;
    Button* _buttons[MAX_BUTTONS] = {};
    uint8_t _buttonCount = 0;

    uint32_t _lastFrameTime = 0;
    static constexpr uint32_t FRAME_INTERVAL_MS = 33; // ~30 FPS
};
