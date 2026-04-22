#pragma once

#include <stdint.h>
#include "Adafruit_SSD1306.h"
#include "ui/Router.h"
#include "ui/NavInput.h"
#include "ui/components/MidiActivityComponent.h"

namespace App { class Button; }

class OledApp {
public:
    OledApp() : _midiActivity(112, 0, 6) {}

    bool begin(uint8_t i2cAddress = 0x3C);
    void update();

    void setButtonUp(App::Button* btn);
    void setButtonDown(App::Button* btn);
    void setButtonSelect(App::Button* btn);

    Router& getRouter();
    MidiActivityComponent& getMidiActivity();

private:
    Adafruit_SSD1306 _display;
    Router _router;
    MidiActivityComponent _midiActivity;

    App::Button* _btnUp     = nullptr;
    App::Button* _btnDown   = nullptr;
    App::Button* _btnSelect = nullptr;

    uint32_t _lastFrameTime = 0;
    static constexpr uint32_t FRAME_INTERVAL_MS = 33;

    void _pollButton(App::Button* btn, NavInput role);
};
