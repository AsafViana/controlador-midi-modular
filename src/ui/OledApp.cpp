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

    // Instancia apenas aqui, apos Wire.begin() — nunca no construtor global
    _display = new Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

    if (!_display->begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
        Serial.println("Erro: falha ao inicializar display SSD1306");
        return false;
    }

    _midiActivity = MidiActivityComponent(OLED_WIDTH - 8, 1, 6);

    _display->clearDisplay();
    _display->display();
    return true;
}

void OledApp::setButtonUp(App::Button* btn)     { _btnUp     = btn; }
void OledApp::setButtonDown(App::Button* btn)   { _btnDown   = btn; }
void OledApp::setButtonSelect(App::Button* btn) { _btnSelect = btn; }

Router& OledApp::getRouter()                         { return _router; }
MidiActivityComponent& OledApp::getMidiActivity()    { return _midiActivity; }

void OledApp::update() {
    if (_display == nullptr) return;

    uint32_t now = millis();
    if (now - _lastFrameTime < FRAME_INTERVAL_MS) return;
    _lastFrameTime = now;

    // --- DEBUG: leitura raw dos pinos ---
    static uint32_t lastDebugPrint = 0;
    if (now - lastDebugPrint > 2000) {
        lastDebugPrint = now;
        Serial.print("[BTN RAW] UP=");
        Serial.print(digitalRead(HardwareMap::PIN_BTN_UP));
        Serial.print(" DOWN=");
        Serial.print(digitalRead(HardwareMap::PIN_BTN_DOWN));
        Serial.print(" SELECT=");
        Serial.println(digitalRead(HardwareMap::PIN_BTN_SELECT));
        Serial.print("[BTN PTR] up=");
        Serial.print(_btnUp != nullptr ? "OK" : "NULL");
        Serial.print(" down=");
        Serial.print(_btnDown != nullptr ? "OK" : "NULL");
        Serial.print(" select=");
        Serial.println(_btnSelect != nullptr ? "OK" : "NULL");
        Screen* s = _router.currentScreen();
        Serial.print("[ROUTER] screen=");
        Serial.println(s != nullptr ? "OK" : "NULL");
    }

    if (_btnUp) {
        ButtonEvent ev = _btnUp->update();
        if (ev == ButtonEvent::PRESSED) {
            Serial.println("[NAV] UP PRESSED");
            _router.handleInput(NavInput::UP);
        }
    }
    if (_btnDown) {
        ButtonEvent ev = _btnDown->update();
        if (ev == ButtonEvent::PRESSED) {
            Serial.println("[NAV] DOWN PRESSED");
            _router.handleInput(NavInput::DOWN);
        }
    }
    if (_btnSelect) {
        ButtonEvent ev = _btnSelect->update();
        if (ev == ButtonEvent::PRESSED) {
            Serial.println("[NAV] SELECT PRESSED");
            _router.handleInput(NavInput::SELECT);
        }
    }

    Screen* screen   = _router.currentScreen();
    bool needsRedraw = (screen != nullptr && screen->isDirty());
    bool midiActive  = _midiActivity.isActive();

    if (needsRedraw || midiActive) {
        _display->clearDisplay();
        if (screen != nullptr) {
            screen->render(*_display);
            screen->clearDirty();
        }
        _midiActivity.render(*_display);
        _display->display();
    }
}
