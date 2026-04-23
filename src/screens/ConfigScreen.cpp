#include "screens/ConfigScreen.h"
#include "screens/CCMapScreen.h"
#include "screens/CanalScreen.h"
#include "ui/OledApp.h"
#include "storage/Storage.h"
#include "config.h"

const char* ConfigScreen::_opcoes[] = {
    "Mapa CC",
    "Canal MIDI",
    "Voltar"
};

ConfigScreen::ConfigScreen(OledApp* app, Storage* storage, CCMapScreen* ccMap, CanalScreen* canal)
    : _app(app)
    , _storage(storage)
    , _ccMap(ccMap)
    , _canal(canal)
    , _titulo(0, 0, "Configuracoes", 1)
    , _voltar(OLED_WIDTH - 48, 4, "<Voltar", 1)
    , _lista(0, CONTENT_Y, OLED_WIDTH, CONTENT_HEIGHT, 1)
{
    addChild(&_titulo);
    addChild(&_voltar);
    addChild(&_lista);
    _lista.setItems(_opcoes, NUM_OPCOES);
    _lista.setUpButton(ButtonEvent::LONG_PRESS);
    _lista.setDownButton(ButtonEvent::SINGLE_CLICK);
}

void ConfigScreen::onMount() { markDirty(); }

void ConfigScreen::render(Adafruit_SSD1306& display) {
    renderChildren(display);
}

void ConfigScreen::handleInput(NavInput input) {
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
                case 0: if (_ccMap) router.push(_ccMap); break;
                case 1: if (_canal) router.push(_canal); break;
                case 2: router.pop();                    break;
            }
            break;
        default: break;
    }
}
