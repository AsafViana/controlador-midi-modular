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

    // (c) Verificar dirty flag da Screen ativa e
    // (d) se dirty: clearDisplay → render → display
    Screen* screen = _router.currentScreen();
    if (screen != nullptr && screen->isDirty()) {
        _display.clearDisplay();
        screen->render(_display);
        _display.display();
        screen->clearDirty();
    }
}
