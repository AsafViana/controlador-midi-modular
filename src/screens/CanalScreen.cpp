#include "screens/CanalScreen.h"
#include "ui/OledApp.h"
#include "config.h"

CanalScreen::CanalScreen(Storage* storage)
    : _storage(storage)
    , _titulo(0, 0, "Canal MIDI", 1)
    , _valorComp(0, CONTENT_Y, OLED_WIDTH, CONTENT_HEIGHT)
{
    addChild(&_titulo);
    addChild(&_valorComp);
}

void CanalScreen::onMount() {
    _canal = _storage->getMidiChannel();
    _valorComp.setValue(_canal);
    markDirty();
}

void CanalScreen::handleInput(NavInput input) {
    switch (input) {
        case NavInput::UP:
            if (_canal < 16) { _canal++; _valorComp.setValue(_canal); markDirty(); }
            break;

        case NavInput::DOWN:
            if (_canal > 1)  { _canal--; _valorComp.setValue(_canal); markDirty(); }
            break;

        case NavInput::SELECT:
            _storage->setMidiChannel(_canal);
            if (_app) _app->getRouter().pop();
            break;

        default: break;
    }
}

void CanalScreen::setApp(OledApp* app) { _app = app; }

void CanalScreen::render(Adafruit_SSD1306& display) {
    renderChildren(display);
}
