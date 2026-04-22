#include "screens/CanalScreen.h"
#include "ui/OledApp.h"
#include "config.h"
#include <cstdio>

CanalScreen::CanalScreen(Storage* storage)
    : _storage(storage)
    , _titulo(0, 0, "Canal MIDI", 1)
    , _valorComp(0, CONTENT_Y, "1", 2)
{
    addChild(&_titulo);
    addChild(&_valorComp);
}

void CanalScreen::setApp(OledApp* app) { _app = app; }

void CanalScreen::onMount() {
    _canal = _storage->getCanalMidi();
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", _canal);
    _valorComp.setText(buf);
    markDirty();
}

void CanalScreen::handleInput(NavInput input) {
    char buf[4];
    switch (input) {
        case NavInput::UP:
            if (_canal < 16) {
                _canal++;
                snprintf(buf, sizeof(buf), "%d", _canal);
                _valorComp.setText(buf);
                markDirty();
            }
            break;
        case NavInput::DOWN:
            if (_canal > 1) {
                _canal--;
                snprintf(buf, sizeof(buf), "%d", _canal);
                _valorComp.setText(buf);
                markDirty();
            }
            break;
        case NavInput::SELECT:
            _storage->setCanalMidi(_canal);
            if (_app) _app->getRouter().pop();
            break;
        default: break;
    }
}

void CanalScreen::render(Adafruit_SSD1306& display) {
    renderChildren(display);
}
