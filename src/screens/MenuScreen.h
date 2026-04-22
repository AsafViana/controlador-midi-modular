#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"
#include "ui/components/ListComponent.h"

class OledApp;

/**
 * Tela de menu principal do teclado.
 *
 * Exibe uma lista de opções navegável. Clique simples desce,
 * long press sobe, double click confirma a opção selecionada.
 */
class MenuScreen : public Screen {
public:
    MenuScreen(OledApp* app);

    void handleInput(ButtonEvent event) override;
    void onMount() override;

private:
    OledApp* _app;
    TextComponent _titulo;
    ListComponent _lista;

    static const char* _opcoes[];
    static constexpr uint8_t NUM_OPCOES = 3;
};
