#pragma once

#include <stdint.h>
#include "Adafruit_SSD1306.h"
#include "ui/Router.h"
#include "ui/components/MidiActivityComponent.h"

// Forward declaration com namespace correto
namespace App { class Button; }

/**
 * OledApp — fachada principal do framework de UI OLED.
 */
class OledApp {
public:
    OledApp() : _midiActivity(112, 0, 6) {}

    bool begin(uint8_t i2cAddress = 0x3C);
    void update();
    void addButton(App::Button* button);
    Router& getRouter();
    MidiActivityComponent& getMidiActivity();

private:
    Adafruit_SSD1306 _display;
    Router _router;
    MidiActivityComponent _midiActivity;

    static constexpr uint8_t MAX_BUTTONS = 4;
    App::Button* _buttons[MAX_BUTTONS] = {};
    uint8_t _buttonCount = 0;

    uint32_t _lastFrameTime = 0;
    static constexpr uint32_t FRAME_INTERVAL_MS = 33;
};
