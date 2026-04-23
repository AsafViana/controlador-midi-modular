#include "ui/OledApp.h"
#include "config.h"
#include "button/Button.h"
#include "hardware/HardwareMap.h"

#ifdef ARDUINO
  #include <Wire.h>
#else
  #include "Wire.h"
  #include "Serial.h"
#endif

bool OledApp::begin(uint8_t i2cAddress) {
    Wire.begin(HardwareMap::PIN_I2C_SDA, HardwareMap::PIN_I2C_SCL);

    _display = Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

    if (!_display.begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
        Serial.println("Erro: falha ao inicializar display SSD1306");
        return false;
    }

    _midiActivity = MidiActivityComponent(OLED_WIDTH - 8, 1, 6);

    _display.clearDisplay();
    _display.display();
    return true;
}

void OledApp::setButtonUp(App::Button* btn)     { _btnUp     = btn; }
void OledApp::setButtonDown(App::Button* btn)   { _btnDown   = btn; }
void OledApp::setButtonSelect(App::Button* btn) { _btnSelect = btn; }

Router& OledApp::getRouter()                         { return _router; }
MidiActivityComponent& OledApp::getMidiActivity()    { return _midiActivity; }

// Todos os botoes de navegacao disparam no PRESSED — sem esperar janela de double-click
static void pollNav(App::Button* btn, NavInput role, Router& router) {
    if (btn == nullptr) return;
    if (btn->update() == ButtonEvent::PRESSED)
        router.handleInput(role);
}

void OledApp::update() {
    uint32_t now = millis();
    if (now - _lastFrameTime < FRAME_INTERVAL_MS) return;
    _lastFrameTime = now;

    pollNav(_btnUp,     NavInput::UP,     _router);
    pollNav(_btnDown,   NavInput::DOWN,   _router);
    pollNav(_btnSelect, NavInput::SELECT, _router);

    Screen* screen   = _router.currentScreen();
    bool needsRedraw = (screen != nullptr && screen->isDirty());
    bool midiActive  = _midiActivity.isActive();

    if (needsRedraw || midiActive) {
        _display.clearDisplay();
        if (screen != nullptr) {
            screen->render(_display);
            screen->clearDirty();
        }
        _midiActivity.render(_display);
        _display.display();
    }
}
