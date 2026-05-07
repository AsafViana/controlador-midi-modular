#include "button/Button.h"
#include "config.h"
#include "hardware/ControlReader.h"
#include "hardware/HardwareMap.h"
#include "hardware/UnifiedControlList.h"
#include "i2c/I2CScanner.h"
#include "i2c/WireI2CBus.h"
#include "midi/MidiEngine.h"
#include "screens/CCMapScreen.h"
#include "screens/CanalScreen.h"
#include "screens/ConfigScreen.h"
#include "screens/MenuScreen.h"
#include "screens/OitavaScreen.h"
#include "screens/PerformanceScreen.h"
#include "screens/SobreScreen.h"
#include "screens/VelocidadeScreen.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "dev"
#endif

static MidiEngine*         engine        = nullptr;
static Storage*            storage       = nullptr;
static WireI2CBus*         i2cBus        = nullptr;
static I2CScanner*         scanner       = nullptr;
static UnifiedControlList* ucl           = nullptr;
static OledApp*            app           = nullptr;
static App::Button*        btnUp         = nullptr;
static App::Button*        btnDown       = nullptr;
static App::Button*        btnSelect     = nullptr;
static ControlReader*      controlReader = nullptr;
static PerformanceScreen*  perfScreen    = nullptr;
static CCMapScreen*        ccMapScreen   = nullptr;
static CanalScreen*        canalScreen   = nullptr;
static OitavaScreen*       oitavaScreen  = nullptr;
static VelocidadeScreen*   velScreen     = nullptr;
static ConfigScreen*       configScreen  = nullptr;
static MenuScreen*         menuScreen    = nullptr;
static SobreScreen*        sobreScreen   = nullptr;

void onMidiActivity() {
    if (app) app->getMidiActivity().trigger();
}

void onCCActivity(const CCActivityInfo& info) {
    if (perfScreen) perfScreen->atualizarCCInfo(info);

    if (ccMapScreen && app && app->getRouter().currentScreen() == ccMapScreen) {
        if (ucl) {
            uint8_t total     = ucl->getNumControles();
            uint8_t numLocais = ucl->getNumLocais();
            if (!info.isRemoto) {
                for (uint8_t i = 0; i < numLocais; i++) {
                    if (ucl->getLabel(i) == info.label) {
                        ccMapScreen->notifyControlMoved(i);
                        break;
                    }
                }
            } else {
                for (uint8_t i = numLocais; i < total; i++) {
                    uint8_t addr, ctrlIdx;
                    if (ucl->getRemoteInfo(i, addr, ctrlIdx) &&
                        addr == info.moduleAddress &&
                        ucl->getLabel(i) == info.label) {
                        ccMapScreen->notifyControlMoved(i);
                        break;
                    }
                }
            }
        }
    }
}

void setup() {
    // [1] USB MIDI — DEVE ser o primeiro, antes de Serial.begin()
    // O TinyUSB precisa ser configurado antes que qualquer outro
    // subsistema USB (incluindo Serial CDC) seja iniciado.
    engine = new MidiEngine();
    engine->begin();

    // [2] Serial debug — apenas apos USB estar configurado
    Serial.begin(115200);
    delay(500);
    Serial.println("=== BOOT START ===");

    Serial.println("[3] storage");
    storage = new Storage();
    storage->begin();

    // [4] I2C — Wire.begin() chamado UMA vez aqui via WireI2CBus.
    // OledApp::begin() reutiliza o barramento sem chamar Wire.begin() novamente.
    Serial.println("[4] i2c");
    i2cBus  = new WireI2CBus();
    scanner = new I2CScanner(i2cBus);
    ucl     = new UnifiedControlList(scanner);
    i2cBus->begin();
    scanner->scan();
    ucl->rebuild();

    Serial.println("[5] controlReader");
    controlReader = new ControlReader(engine, storage, ucl, scanner);
    controlReader->begin();

    Serial.println("[6] LED");
    pinMode(HardwareMap::PIN_LED, OUTPUT);

    Serial.println("[7] app/display");
    app = new OledApp();
    if (!app->begin(DISPLAY_I2C_ADDRESS)) {
        Serial.println("ERRO: Falha ao inicializar display OLED!");
        for (int i = 0; i < 10; i++) {
            digitalWrite(HardwareMap::PIN_LED, HIGH); delay(100);
            digitalWrite(HardwareMap::PIN_LED, LOW);  delay(100);
        }
    }

    app->showSplash("MIDI Ctrl", FIRMWARE_VERSION);

    Serial.println("[8] screens");
    perfScreen   = new PerformanceScreen(engine, storage);
    ccMapScreen  = new CCMapScreen(storage, ucl);
    canalScreen  = new CanalScreen(storage);
    oitavaScreen = new OitavaScreen(storage);
    velScreen    = new VelocidadeScreen(storage);
    sobreScreen  = new SobreScreen(storage, ucl);

    configScreen = new ConfigScreen(app, storage, ccMapScreen, canalScreen, oitavaScreen, velScreen);
    menuScreen   = new MenuScreen(app, storage, perfScreen, configScreen, sobreScreen);

    perfScreen->setApp(app);
    ccMapScreen->setApp(app);
    canalScreen->setApp(app);
    oitavaScreen->setApp(app);
    velScreen->setApp(app);
    sobreScreen->setApp(app);

    Serial.println("[9] activity");
    engine->onActivity(onMidiActivity);
    controlReader->onCCActivity(onCCActivity);

    Serial.println("[10] buttons");
    btnUp     = new App::Button(HardwareMap::PIN_BTN_UP,     true);
    btnDown   = new App::Button(HardwareMap::PIN_BTN_DOWN,   true);
    btnSelect = new App::Button(HardwareMap::PIN_BTN_SELECT, true);
    btnUp->begin();
    btnDown->begin();
    btnSelect->begin();
    app->setButtonUp(btnUp);
    app->setButtonDown(btnDown);
    app->setButtonSelect(btnSelect);

    Serial.println("[11] router");
    app->getRouter().push(menuScreen);

    Serial.println("=== BOOT OK ===");
}

void loop() {
    if (controlReader) controlReader->update();
    if (scanner)       scanner->periodicScan();
    if (ucl && scanner && scanner->needsRebuild()) {
        ucl->rebuild();
        scanner->clearRebuildFlag();
    }
    if (app) app->update();
}
