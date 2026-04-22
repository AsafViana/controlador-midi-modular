#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"
#include "ui/components/ListComponent.h"

class OledApp;
class Storage;

/**
 * Tela de configurações do teclado.
 *
 * Lista de parâmetros configuráveis. Navegação:
 *   - SINGLE_CLICK: Desce na lista
 *   - DOUBLE_CLICK: Entra na sub-tela do item selecionado
 *   - LONG_PRESS:   Voltar
 */
class ConfigScreen : public Screen {
public:
    ConfigScreen(OledApp* app, Storage* storage);

    void handleInput(ButtonEvent event) override;
    void onMount() override;

private:
    OledApp* _app;
    Storage* _storage;
    TextComponent _titulo;
    ListComponent _lista;

    static const char* _configs[];
    static constexpr uint8_t NUM_CONFIGS = 3;
};
