#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"

class Storage;
class OledApp;

/**
 * Tela de seleção do canal MIDI (1-16).
 *
 * Navegação:
 *   - SINGLE_CLICK: Incrementa canal
 *   - DOUBLE_CLICK: Confirma e salva
 *   - LONG_PRESS:   Voltar
 */
class CanalScreen : public Screen {
public:
    CanalScreen(Storage* storage);

    void setApp(OledApp* app) { _app = app; }

    void handleInput(ButtonEvent event) override;
    void onMount() override;
    void render(Adafruit_SSD1306& display) override;

private:
    Storage* _storage;
    OledApp* _app;
    TextComponent _titulo;
    uint8_t _canal = 1;
    bool _confirmado = false;
    char _canalBuf[8];
};
