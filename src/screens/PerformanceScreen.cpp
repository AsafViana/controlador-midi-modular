#include "screens/PerformanceScreen.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include "config.h"
#include <cstdio>

PerformanceScreen::PerformanceScreen(MidiEngine* engine, Storage* storage)
    : _engine(engine)
    , _storage(storage)
    , _app(nullptr)
    , _titulo(0, 0, "Performance", 2)
    , _notaLabel(0, CONTENT_Y + 2, "Nota: --", 1)
    , _ccLabel(0, CONTENT_Y + 15, "Mod: 0", 1)
    , _barra(0, CONTENT_Y + CONTENT_HEIGHT - 12, OLED_WIDTH, 10)
{
    addChild(&_titulo);
    addChild(&_notaLabel);
    addChild(&_ccLabel);
    addChild(&_barra);
}

void PerformanceScreen::onMount() {
    _notaLabel.setText("Nota: --");
    _ccLabel.setText("Mod: 0");
    _barra.setValue(0);
    markDirty();
}

void PerformanceScreen::handleInput(ButtonEvent event) {
    if (event == ButtonEvent::LONG_PRESS) {
        if (_app) _app->getRouter().pop();
        return;
    }

    // Só envia notas se o teclado está habilitado
    if (!_storage->isTecladoHabilitado()) {
        if (event == ButtonEvent::PRESSED) {
            _notaLabel.setText("Teclado OFF");
            markDirty();
        }
        return;
    }

    if (event == ButtonEvent::PRESSED) {
        uint8_t oitava = _storage->getOitava();
        uint8_t vel = _storage->getVelocidade();
        uint8_t canal = _storage->getCanalMidi();
        MidiNote nota(MIDI_Notes::C(oitava), vel, canal);
        _engine->sendNoteOn(nota);
        _notaLabel.setText("Nota: ON");
        markDirty();
    }
    else if (event == ButtonEvent::RELEASED) {
        uint8_t oitava = _storage->getOitava();
        uint8_t canal = _storage->getCanalMidi();
        MidiNote nota(MIDI_Notes::C(oitava), 0, canal);
        _engine->sendNoteOff(nota);
        _notaLabel.setText("Nota: --");
        markDirty();
    }
}

void PerformanceScreen::atualizarCC(uint8_t valor) {
    uint8_t canal = _storage->getCanalMidi();
    MidiCC mod(1, valor, canal);
    _engine->sendCC(mod);
    _barra.setValue((valor * 100) / 127);
    snprintf(_ccBuf, sizeof(_ccBuf), "Mod: %d", valor);
    _ccLabel.setText(_ccBuf);
    markDirty();
}
