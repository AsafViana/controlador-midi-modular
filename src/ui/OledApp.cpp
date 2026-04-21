#include "ui/OledApp.h"
#include "config.h"
#include "button/Button.h"

#ifdef ARDUINO
  #include <Wire.h>
#else
  #include "Wire.h"
  #include "Serial.h"
#endif

bool OledApp::begin(uint8_t i2cAddress) {
    _display = Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);

    if (!_display.begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
        Serial.println("Erro: falha ao inicializar display SSD1306");
        return false;
    }

    // Posiciona o indicador MIDI no canto superior direito
    // 6px de tamanho, com 2px de margem
    _midiActivity = MidiActivityComponent(DISPLAY_WIDTH - 8, 1, 6);

    _display.clearDisplay();
    _display.display();
    return true;
}

void OledApp::addButton(Button* button) {
    if (button == nullptr) {
        return;
    }
    if (_buttonCount >= MAX_BUTTONS) {
        return;
    }
    _buttons[_buttonCount] = button;
    _buttonCount++;
}

Router& OledApp::getRouter() {
    return _router;
}

MidiActivityComponent& OledApp::getMidiActivity() {
    return _midiActivity;
}

void OledApp::update() {
    uint32_t now = millis();
    if (now - _lastFrameTime < FRAME_INTERVAL_MS) {
        return;
    }
    _lastFrameTime = now;

    // (a) Consultar todos os Buttons registrados e
    // (b) encaminhar eventos != NONE ao Router
    for (uint8_t i = 0; i < _buttonCount; i++) {
        ButtonEvent event = _buttons[i]->update();
        if (event != ButtonEvent::NONE) {
            _router.handleInput(event);
        }
    }

    // (c) Verificar se precisa redesenhar:
    //     - Screen marcada como dirty, OU
    //     - Indicador MIDI ativo (precisa atualizar o piscar)
    Screen* screen = _router.currentScreen();
    bool needsRedraw = (screen != nullptr && screen->isDirty());
    bool midiActive = _midiActivity.isActive();

    if (needsRedraw || midiActive) {
        _display.clearDisplay();

        // Renderiza a tela ativa
        if (screen != nullptr) {
            screen->render(_display);
            screen->clearDirty();
        }

        // Renderiza o indicador MIDI como overlay (por cima de tudo)
        _midiActivity.render(_display);

        _display.display();
    }
}
