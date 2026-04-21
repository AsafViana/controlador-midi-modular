#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"
#include "ui/components/ProgressBarComponent.h"
#include "midi/MidiEngine.h"
#include "midi/MidiCC.h"

class OledApp;
class Storage;

/**
 * Tela de performance — onde o músico toca.
 *
 * Respeita o flag de teclado habilitado/desabilitado do Storage.
 * Se desabilitado, não envia notas.
 */
class PerformanceScreen : public Screen {
public:
    PerformanceScreen(MidiEngine* engine, Storage* storage);

    void setApp(OledApp* app) { _app = app; }

    void handleInput(ButtonEvent event) override;
    void onMount() override;

    void atualizarCC(uint8_t valor);

private:
    MidiEngine* _engine;
    Storage* _storage;
    OledApp* _app;
    TextComponent _titulo;
    TextComponent _notaLabel;
    TextComponent _ccLabel;
    ProgressBarComponent _barra;
    char _ccBuf[16];
};
