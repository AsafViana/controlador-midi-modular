#include "screens/CanalScreen.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include "Adafruit_SSD1306.h"
#include "config.h"
#include <cstdio>

CanalScreen::CanalScreen(Storage* storage)
    : _storage(storage)
    , _app(nullptr)
    , _titulo(0, 0, "Canal MIDI", 1)
{
    addChild(&_titulo);
}

void CanalScreen::onMount() {
    _canal = _storage->getCanalMidi();
    _confirmado = false;
    markDirty();
}

void CanalScreen::handleInput(ButtonEvent event) {
    // LONG_PRESS = voltar
    if (event == ButtonEvent::LONG_PRESS) {
        if (_app) _app->getRouter().pop();
        return;
    }

    if (event == ButtonEvent::SINGLE_CLICK) {
        if (_canal < 16) {
            _canal++;
            _confirmado = false;
            markDirty();
        }
    }
    else if (event == ButtonEvent::DOUBLE_CLICK) {
        _storage->setCanalMidi(_canal);
        _confirmado = true;
        markDirty();
    }
}

void CanalScreen::render(Adafruit_SSD1306& display) {
    Screen::render(display);

    snprintf(_canalBuf, sizeof(_canalBuf), "%d", _canal);

    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);

    int16_t x = (_canal >= 10) ? 44 : 56;
    display.setCursor(x, CONTENT_Y + 6);
    display.print(_canalBuf);

    display.setTextSize(1);
    if (_confirmado) {
        display.setCursor(30, CONTENT_Y + CONTENT_HEIGHT - 10);
        display.print("Salvo!");
    } else {
        display.setCursor(4, CONTENT_Y + CONTENT_HEIGHT - 10);
        display.print("CLK:+1 DCLK:salvar");
    }
}
