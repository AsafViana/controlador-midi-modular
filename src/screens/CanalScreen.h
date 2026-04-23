#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"
#include "storage/Storage.h"

class OledApp;

class CanalScreen : public Screen {
public:
    CanalScreen(Storage* storage);

    void setApp(OledApp* app);

    void handleInput(NavInput input) override;
    void onMount() override;
    void render(Adafruit_SSD1306& display) override;

private:
    Storage* _storage;
    OledApp* _app = nullptr;
    TextComponent _titulo;
    TextComponent _voltar;
    TextComponent _valorComp;
    uint8_t _canal = 1;
};
