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

void setup() {
    // PRIMEIRA LINHA: garante Serial antes de qualquer init
    Serial.begin(115200);
    delay(500); // aguarda USB enumerar
    Serial.println("=== BOOT START ===");

    Serial.println("[1] engine.begin");
    engine.begin();

    Serial.println("[2] storage.begin");
    storage.begin();

    Serial.println("[3] i2c.begin");
    i2cBus.begin();

    Serial.println("[4] scanner.scan");
    scanner.scan();

    Serial.println("[5] ucl.rebuild");
    ucl.rebuild();

    Serial.println("[6] controlReader.begin");
    controlReader.begin();

    Serial.println("[7] pinMode LED");
    pinMode(HardwareMap::PIN_LED, OUTPUT);

    Serial.println("[8] app.begin");
    if (!app.begin(DISPLAY_I2C_ADDRESS)) {
        Serial.println("Falha ao inicializar display OLED!");
    }

    Serial.println("[9] engine.onActivity");
    engine.onActivity(onMidiActivity);

    Serial.println("[10] setApp");
    perfScreen.setApp(&app);
    ccMapScreen.setApp(&app);
    canalScreen.setApp(&app);

    Serial.println("[11] buttons");
    btnUp.begin();
    btnDown.begin();
    btnSelect.begin();
    app.setButtonUp(&btnUp);
    app.setButtonDown(&btnDown);
    app.setButtonSelect(&btnSelect);

    Serial.println("[12] router.push");
    app.getRouter().push(&menuScreen);

    Serial.println("=== BOOT OK ===");
}

void loop() {
    controlReader.update();
    scanner.periodicScan();
    ucl.rebuild();
    app.update();
}
