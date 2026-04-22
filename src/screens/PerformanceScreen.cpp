#include "screens/PerformanceScreen.h"
#include "ui/OledApp.h"
#include "config.h"

PerformanceScreen::PerformanceScreen(MidiEngine* engine, Storage* storage)
    : _engine(engine)
    , _storage(storage)
    , _titulo(0, 0, "Performance", 1)
    , _info(0, CONTENT_Y, "Aguardando MIDI...", 1)
{
    addChild(&_titulo);
    addChild(&_info);
}

void PerformanceScreen::onMount() { markDirty(); }

void PerformanceScreen::handleInput(NavInput input) {
    if (input == NavInput::SELECT) {
        if (_app) _app->getRouter().pop();
    }
}

void PerformanceScreen::setApp(OledApp* app) { _app = app; }

void PerformanceScreen::render(Adafruit_SSD1306& display) {
    renderChildren(display);
}
