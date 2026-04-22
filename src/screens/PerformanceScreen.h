#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"
#include "midi/MidiEngine.h"

class OledApp;
class Storage;

class PerformanceScreen : public Screen {
public:
    PerformanceScreen(MidiEngine* engine, Storage* storage);

    void setApp(OledApp* app);

    void handleInput(NavInput input) override;
    void onMount() override;
    void render(Adafruit_SSD1306& display) override;

    void atualizarCC(uint8_t valor);

private:
    MidiEngine* _engine;
    Storage* _storage;
    OledApp* _app = nullptr;
    TextComponent _titulo;
    TextComponent _info;
};
