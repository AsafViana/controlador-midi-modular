#include "screens/MenuScreen.h"
#include "screens/PerformanceScreen.h"
#include "screens/ConfigScreen.h"
#include "ui/OledApp.h"
#include "config.h"

extern PerformanceScreen perfScreen;
extern ConfigScreen configScreen;

const char* MenuScreen::_opcoes[] = {
    "Performance",
    "Configuracoes",
    "Sobre"
};

MenuScreen::MenuScreen(OledApp* app)
    : _app(app)
    , _titulo(0, 0, "Menu Principal", 1)
    , _lista(0, CONTENT_Y, OLED_WIDTH, CONTENT_HEIGHT, 1)
{
    addChild(&_titulo);
    addChild(&_lista);
    _lista.setItems(_opcoes, NUM_OPCOES);
}

void MenuScreen::onMount() { markDirty(); }

void MenuScreen::render(Adafruit_SSD1306& display) {
    renderChildren(display);
}

void MenuScreen::handleInput(NavInput input) {
    Router& router = _app->getRouter();

    switch (input) {
        case NavInput::UP:
            _lista.selectPrev();
            markDirty();
            break;
        case NavInput::DOWN:
            _lista.selectNext();
            markDirty();
            break;
        case NavInput::SELECT:
            switch (_lista.getSelectedIndex()) {
                case 0: router.push(&perfScreen);   break;
                case 1: router.push(&configScreen); break;
                case 2: break;
            }
            break;
        default: break;
    }
}
