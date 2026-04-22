#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"
#include "ui/components/ListComponent.h"

class OledApp;

class MenuScreen : public Screen {
public:
    MenuScreen(OledApp* app);

    void handleInput(NavInput input) override;
    void onMount() override;
    void render(Adafruit_SSD1306& display) override;

private:
    OledApp* _app;
    TextComponent _titulo;
    ListComponent _lista;

    static const char* _opcoes[];
    static constexpr uint8_t NUM_OPCOES = 3;
};
