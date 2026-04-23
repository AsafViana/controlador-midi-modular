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
static ConfigScreen*       configScreen  = nullptr;
static MenuScreen*         menuScreen    = nullptr;

void onMidiActivity() {
    if (app) app->getMidiActivity().trigger();
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("=== BOOT START ===");

    Serial.println("[1] engine");
    engine = new MidiEngine();
    engine->begin();

    Serial.println("[2] storage");
    storage = new Storage();
    storage->begin();

    Serial.println("[3] i2c");
    i2cBus  = new WireI2CBus();
    scanner = new I2CScanner(i2cBus);
    ucl     = new UnifiedControlList(scanner);
    i2cBus->begin();
    scanner->scan();
    ucl->rebuild();

    Serial.println("[4] controlReader");
    controlReader = new ControlReader(engine, storage, ucl, scanner);
    controlReader->begin();

    Serial.println("[5] LED");
    pinMode(HardwareMap::PIN_LED, OUTPUT);

    Serial.println("[6] app");
    app = new OledApp();
    if (!app->begin(DISPLAY_I2C_ADDRESS)) {
        Serial.println("Falha ao inicializar display OLED!");
    }

    // Telas sem dependencias entre si primeiro
    Serial.println("[7] screens");
    perfScreen  = new PerformanceScreen(engine, storage);
    ccMapScreen = new CCMapScreen(storage, ucl);
    canalScreen = new CanalScreen(storage);

    // ConfigScreen recebe ccMap e canal como destinos
    configScreen = new ConfigScreen(app, storage, ccMapScreen, canalScreen);

    // MenuScreen recebe perf e config como destinos
    menuScreen = new MenuScreen(app, perfScreen, configScreen);

    perfScreen->setApp(app);
    ccMapScreen->setApp(app);
    canalScreen->setApp(app);

    Serial.println("[8] activity");
    engine->onActivity(onMidiActivity);

    Serial.println("[9] buttons");
    btnUp     = new App::Button(HardwareMap::PIN_BTN_UP,     true);
    btnDown   = new App::Button(HardwareMap::PIN_BTN_DOWN,   true);
    btnSelect = new App::Button(HardwareMap::PIN_BTN_SELECT, true);
    btnUp->begin();
    btnDown->begin();
    btnSelect->begin();
    app->setButtonUp(btnUp);
    app->setButtonDown(btnDown);
    app->setButtonSelect(btnSelect);

    Serial.println("[10] router");
    app->getRouter().push(menuScreen);

    Serial.println("=== BOOT OK ===");
}

void loop() {
    if (controlReader) controlReader->update();
    if (scanner)       scanner->periodicScan();
    if (ucl)           ucl->rebuild();
    if (app)           app->update();
}
