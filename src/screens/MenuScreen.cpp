#include "screens/MenuScreen.h"
#include "screens/PerformanceScreen.h"
#include "screens/ConfigScreen.h"
#include "ui/OledApp.h"
#include "config.h"

// Declarações externas das telas (definidas em main.cpp)
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
    // SINGLE_CLICK = down, LONG_PRESS = up, DOUBLE_CLICK = select
    _lista.setDownButton(ButtonEvent::SINGLE_CLICK);
    _lista.setUpButton(ButtonEvent::LONG_PRESS);
}

void MenuScreen::onMount() {
    markDirty();
}

void MenuScreen::handleInput(ButtonEvent event) {
    // Navegação na lista
    if (_lista.handleInput(event)) {
        markDirty();
        return;
    }

    // DOUBLE_CLICK = entrar na opção selecionada
    if (event == ButtonEvent::DOUBLE_CLICK) {
        Router& router = _app->getRouter();
        switch (_lista.getSelectedIndex()) {
            case 0:
                router.push(&perfScreen);
                break;
            case 1:
                router.push(&configScreen);
                break;
            case 2:
                // Sobre — futuro
                break;
        }
    }
}
