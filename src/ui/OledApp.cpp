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

void OledApp::update() {
    uint32_t now = millis();
    if (now - _lastFrameTime < FRAME_INTERVAL_MS) return;
    _lastFrameTime = now;

    // UP e DOWN: respondem imediatamente no PRESSED (sem esperar double-click)
    if (_btnUp) {
        ButtonEvent ev = _btnUp->update();
        if (ev == ButtonEvent::PRESSED) _router.handleInput(NavInput::UP);
    }
    if (_btnDown) {
        ButtonEvent ev = _btnDown->update();
        if (ev == ButtonEvent::PRESSED) _router.handleInput(NavInput::DOWN);
    }
    // SELECT: aguarda SINGLE_CLICK (soltar o botão) para evitar acionamento duplo
    if (_btnSelect) {
        ButtonEvent ev = _btnSelect->update();
        if (ev == ButtonEvent::SINGLE_CLICK) _router.handleInput(NavInput::SELECT);
    }

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
