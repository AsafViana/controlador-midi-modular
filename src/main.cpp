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
#include "screens/PresetScreen.h"
#include "screens/ProgramChangeScreen.h"
#include "screens/SobreScreen.h"
#include "screens/VelocidadeScreen.h"
#include "storage/PresetManager.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"

#ifdef ARDUINO
#include <esp_task_wdt.h>
#endif

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "dev"
#endif

static MidiEngine *engine = nullptr;
static Storage *storage = nullptr;
static WireI2CBus *i2cBus = nullptr;
static I2CScanner *scanner = nullptr;
static UnifiedControlList *ucl = nullptr;
static OledApp *app = nullptr;
static App::Button *btnUp = nullptr;
static App::Button *btnDown = nullptr;
static App::Button *btnSelect = nullptr;
static App::Button *btnBack = nullptr;
static ControlReader *controlReader = nullptr;
static PerformanceScreen *perfScreen = nullptr;
static CCMapScreen *ccMapScreen = nullptr;
static CanalScreen *canalScreen = nullptr;
static OitavaScreen *oitavaScreen = nullptr;
static VelocidadeScreen *velScreen = nullptr;
static ConfigScreen *configScreen = nullptr;
static MenuScreen *menuScreen = nullptr;
static SobreScreen *sobreScreen = nullptr;
static PresetManager *presetManager = nullptr;
static PresetScreen *presetScreen = nullptr;
static ProgramChangeScreen *progChangeScreen = nullptr;

static bool headlessMode = false;
static uint32_t headlessLedTimer = 0;

void onMidiActivity() {
  if (app)
    app->getMidiActivity().trigger();
}

void onMidiCCReceived(uint8_t cc, uint8_t valor, uint8_t canal) {
  // Atualiza PerformanceScreen com CC recebido externamente
  if (perfScreen) {
    CCActivityInfo info;
    info.label = "MIDI IN";
    info.cc = cc;
    info.valor = valor;
    info.canal = canal;
    info.isRemoto = false;
    info.moduleAddress = 0;
    perfScreen->atualizarCCInfo(info);
  }
}

void onCCActivity(const CCActivityInfo &info) {
  if (perfScreen)
    perfScreen->atualizarCCInfo(info);

  if (ccMapScreen && app && app->getRouter().currentScreen() == ccMapScreen) {
    if (ucl) {
      uint8_t total = ucl->getNumControles();
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
              addr == info.moduleAddress && ucl->getLabel(i) == info.label) {
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
  engine = new MidiEngine();
  engine->begin();

  // [2] Serial debug
  Serial.begin(115200);
  delay(500);
  Serial.println("=== BOOT START ===");

  Serial.println("[3] storage");
  storage = new Storage();
  storage->begin();

  presetManager = new PresetManager(storage);
  presetManager->begin();

  Serial.println("[4] i2c");
  i2cBus = new WireI2CBus();
  scanner = new I2CScanner(i2cBus);
  ucl = new UnifiedControlList(scanner);
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
    Serial.println("ERRO: Display OLED falhou — modo headless ativo");
    headlessMode = true;
    // LED pisca 2x rápido para indicar modo headless
    for (int i = 0; i < 2; i++) {
      digitalWrite(HardwareMap::PIN_LED, HIGH);
      delay(100);
      digitalWrite(HardwareMap::PIN_LED, LOW);
      delay(100);
    }
    delete app;
    app = nullptr;
  }

  if (!headlessMode) {
    app->showSplash("MIDI Ctrl", FIRMWARE_VERSION);

    Serial.println("[8] screens");
    perfScreen = new PerformanceScreen(engine, storage);
    ccMapScreen = new CCMapScreen(storage, ucl);
    canalScreen = new CanalScreen(storage);
    oitavaScreen = new OitavaScreen(storage);
    velScreen = new VelocidadeScreen(storage);
    sobreScreen = new SobreScreen(storage, ucl);
    presetScreen = new PresetScreen(app, storage, presetManager);
    progChangeScreen = new ProgramChangeScreen(engine, storage);

    configScreen = new ConfigScreen(app, storage, ccMapScreen, canalScreen,
                                    oitavaScreen, velScreen, progChangeScreen);
    menuScreen = new MenuScreen(app, storage, perfScreen, configScreen,
                                sobreScreen, presetScreen);

    perfScreen->setApp(app);
    ccMapScreen->setApp(app);
    canalScreen->setApp(app);
    oitavaScreen->setApp(app);
    velScreen->setApp(app);
    sobreScreen->setApp(app);
    progChangeScreen->setApp(app);

    Serial.println("[9] activity");
    engine->onActivity(onMidiActivity);
    engine->onCCReceived(onMidiCCReceived);
    controlReader->onCCActivity(onCCActivity);

    Serial.println("[10] buttons");
    btnUp = new App::Button(HardwareMap::PIN_BTN_UP, true);
    btnDown = new App::Button(HardwareMap::PIN_BTN_DOWN, true);
    btnSelect = new App::Button(HardwareMap::PIN_BTN_SELECT, true);
    btnBack = new App::Button(HardwareMap::PIN_BTN_BACK, true);
    btnUp->begin();
    btnDown->begin();
    btnSelect->begin();
    btnBack->begin();
    app->setButtonUp(btnUp);
    app->setButtonDown(btnDown);
    app->setButtonSelect(btnSelect);
    app->setButtonBack(btnBack);

    Serial.println("[11] router");
    app->getRouter().push(menuScreen);

    // [12] Timeout de inatividade — volta para Performance após 60s sem input
    app->setIdleScreen(perfScreen);
    app->setIdleTimeoutSeconds(60);
  } else {
    // Modo headless: apenas registra callback de atividade MIDI
    engine->onActivity(onMidiActivity);
    engine->onCCReceived(onMidiCCReceived);
    controlReader->onCCActivity(onCCActivity);
  }

  // [13] Watchdog Timer — reinicia se o loop travar por mais de 5s
#ifdef ARDUINO
  esp_task_wdt_config_t wdtConfig = {
      .timeout_ms = 5000, .idle_core_mask = 0, .trigger_panic = true};
  esp_task_wdt_reconfigure(&wdtConfig);
  esp_task_wdt_add(NULL);
#endif

  Serial.println("=== BOOT OK ===");
}

void loop() {
  if (engine)
    engine->update();
  if (controlReader)
    controlReader->update();
  if (scanner)
    scanner->periodicScan();
  if (ucl && scanner && scanner->needsRebuild()) {
    ucl->rebuild();
    scanner->clearRebuildFlag();
  }

  if (headlessMode) {
    // Modo headless: LED pisca 2x curtas a cada 3 segundos
    uint32_t now = millis();
    uint32_t phase = (now - headlessLedTimer) % 3000;
    if (phase < 100 || (phase >= 200 && phase < 300)) {
      digitalWrite(HardwareMap::PIN_LED, HIGH);
    } else {
      digitalWrite(HardwareMap::PIN_LED, LOW);
    }
  } else {
    if (app)
      app->update();
  }

#ifdef ARDUINO
  esp_task_wdt_reset();
#endif
}
