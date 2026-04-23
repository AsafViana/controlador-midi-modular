#include "screens/MenuScreen.h"
#include "screens/PerformanceScreen.h"
#include "screens/ConfigScreen.h"
#include "ui/OledApp.h"
#include "config.h"

const char* MenuScreen::_opcoes[] = {
    "Performance",
    "Configuracoes",
    "Sobre"
};

MenuScreen::MenuScreen(OledApp* app, PerformanceScreen* perf, ConfigScreen* config)
    : _app(app)
    , _perf(perf)
    , _config(config)
    , _titulo(0, 0, "Menu Principal", 1)
    , _lista(0, CONTENT_Y, OLED_WIDTH, CONTENT_HEIGHT, 1)
{
    addChild(&_titulo);
    addChild(&_lista);
    _lista.setItems(_opcoes, NUM_OPCOES);
    _lista.setUpButton(ButtonEvent::LONG_PRESS);
    _lista.setDownButton(ButtonEvent::SINGLE_CLICK);
}

void MenuScreen::onMount() { markDirty(); }

void MenuScreen::render(Adafruit_SSD1306& display) {
    renderChildren(display);
}

void MenuScreen::handleInput(NavInput input) {
    Router& router = _app->getRouter();

    switch (input) {
        case NavInput::UP:
            _lista.handleInput(ButtonEvent::LONG_PRESS);
            markDirty();
            break;
        case NavInput::DOWN:
            _lista.handleInput(ButtonEvent::SINGLE_CLICK);
            markDirty();
            break;
        case NavInput::SELECT:
            switch (_lista.getSelectedIndex()) {
                case 0: if (_perf)   router.push(_perf);   break;
                case 1: if (_config) router.push(_config); break;
                case 2: break;
            }
            break;
        default: break;
    }
}
