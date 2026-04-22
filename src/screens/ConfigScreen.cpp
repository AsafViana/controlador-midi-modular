#include "screens/ConfigScreen.h"
#include "screens/CCMapScreen.h"
#include "screens/CanalScreen.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include "config.h"

extern CCMapScreen ccMapScreen;
extern CanalScreen canalScreen;

const char* ConfigScreen::_configs[] = {
    "Endereco CC",
    "Canal MIDI",
    "Teclado: ---"
};

ConfigScreen::ConfigScreen(OledApp* app, Storage* storage)
    : _app(app)
    , _storage(storage)
    , _titulo(0, 0, "Configuracoes", 1)
    , _lista(0, CONTENT_Y, OLED_WIDTH, CONTENT_HEIGHT, 1)
{
    addChild(&_titulo);
    addChild(&_lista);

    _lista.setItems(_configs, NUM_CONFIGS);
    _lista.setDownButton(ButtonEvent::SINGLE_CLICK);
}

void ConfigScreen::onMount() {
    // Atualiza o label do teclado com o estado atual
    _configs[2] = _storage->isTecladoHabilitado()
        ? "Teclado: ON"
        : "Teclado: OFF";
    _lista.setItems(_configs, NUM_CONFIGS);
    markDirty();
}

void ConfigScreen::handleInput(ButtonEvent event) {
    // LONG_PRESS = voltar
    if (event == ButtonEvent::LONG_PRESS) {
        _app->getRouter().pop();
        return;
    }

    // SINGLE_CLICK = navegar na lista (down)
    if (_lista.handleInput(event)) {
        markDirty();
        return;
    }

    // DOUBLE_CLICK = confirmar / entrar na opção selecionada
    if (event == ButtonEvent::DOUBLE_CLICK) {
        Router& router = _app->getRouter();
        switch (_lista.getSelectedIndex()) {
            case 0:
                router.push(&ccMapScreen);
                break;
            case 1:
                router.push(&canalScreen);
                break;
            case 2: {
                // Toggle teclado ON/OFF
                bool atual = _storage->isTecladoHabilitado();
                _storage->setTecladoHabilitado(!atual);
                _configs[2] = (!atual) ? "Teclado: ON" : "Teclado: OFF";
                _lista.setItems(_configs, NUM_CONFIGS);
                markDirty();
                break;
            }
        }
    }
}
