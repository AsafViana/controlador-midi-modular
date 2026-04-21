#pragma once

#include <stdint.h>
#include "Adafruit_SSD1306.h"
#include "ui/Router.h"
#include "ui/components/MidiActivityComponent.h"

class Button;

/**
 * OledApp — fachada principal do framework de UI OLED.
 *
 * Ponto de entrada para o desenvolvedor: inicializa o display,
 * registra botões, expõe o Router para navegação, e executa o
 * loop de renderização via update().
 *
 * Inclui um indicador de atividade MIDI no canto superior direito
 * que pisca a cada mensagem MIDI enviada.
 */
class OledApp {
public:
    bool begin(uint8_t i2cAddress = 0x3C);
    void update();
    void addButton(Button* button);
    Router& getRouter();

    /// Retorna referência ao indicador de atividade MIDI.
    /// Use para conectar ao MidiEngine via onActivity().
    MidiActivityComponent& getMidiActivity();

private:
    Adafruit_SSD1306 _display;
    Router _router;
    MidiActivityComponent _midiActivity;

    static constexpr uint8_t MAX_BUTTONS = 4;
    Button* _buttons[MAX_BUTTONS] = {};
    uint8_t _buttonCount = 0;

    uint32_t _lastFrameTime = 0;
    static constexpr uint32_t FRAME_INTERVAL_MS = 33; // ~30 FPS
};
