#include "midi/MidiEngine.h"
#include "config.h"
#include "button/Button.h"
#include "ui/OledApp.h"
#include "storage/Storage.h"
#include "hardware/HardwareMap.h"
#include "hardware/ControlReader.h"
#include "hardware/UnifiedControlList.h"
#include "i2c/WireI2CBus.h"
#include "i2c/I2CScanner.h"
#include "screens/MenuScreen.h"
#include "screens/PerformanceScreen.h"
#include "screens/ConfigScreen.h"
#include "screens/CCMapScreen.h"
#include "screens/CanalScreen.h"

// ── Módulos ──────────────────────────────────────────────
MidiEngine engine;
Storage storage;

// ── I2C — Expansão modular ───────────────────────────────
WireI2CBus i2cBus;
I2CScanner scanner(&i2cBus);
UnifiedControlList ucl(&scanner);

// ── UI ───────────────────────────────────────────────────
OledApp app;
App::Button btnUp(HardwareMap::PIN_BTN_UP, true);
App::Button btnDown(HardwareMap::PIN_BTN_DOWN, true);
App::Button btnSelect(HardwareMap::PIN_BTN_SELECT, true);

// ── Leitura automática de controles analógicos ───────────
ControlReader controlReader(&engine, &storage, &ucl, &scanner);

// ── Telas ────────────────────────────────────────────────
PerformanceScreen perfScreen(&engine, &storage);
ConfigScreen configScreen(&app, &storage);
CCMapScreen ccMapScreen(&storage, &ucl);
CanalScreen canalScreen(&storage);
MenuScreen menuScreen(&app);

// ── Callback de atividade MIDI ───────────────────────────
void onMidiActivity() {
    app.getMidiActivity().trigger();
}

// ── setup() ──────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    engine.begin();
    storage.begin();

    // ── I2C: inicializar barramento, descobrir módulos, montar lista ──
    i2cBus.begin();
    scanner.scan();
    ucl.rebuild();

    controlReader.begin();

    pinMode(HardwareMap::PIN_LED, OUTPUT);

    if (!app.begin(DISPLAY_I2C_ADDRESS)) {
        Serial.println("Falha ao inicializar display OLED!");
    }

    engine.onActivity(onMidiActivity);

    perfScreen.setApp(&app);
    ccMapScreen.setApp(&app);
    canalScreen.setApp(&app);

    btnUp.begin();
    btnDown.begin();
    btnSelect.begin();
    app.addButton(&btnUp);
    app.addButton(&btnDown);
    app.addButton(&btnSelect);

    app.getRouter().push(&menuScreen);

    Serial.println("Sistema iniciado.");
}

// ── loop() ───────────────────────────────────────────────
void loop() {
    controlReader.update();
    scanner.periodicScan();
    ucl.rebuild();
    app.update();
}
