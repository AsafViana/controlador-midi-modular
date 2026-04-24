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
// TecladoScreen: infraestrutura pronta mas não integrada (aguardando hardware
// de teclado)
#include "storage/Storage.h"
#include "ui/OledApp.h"

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
static ControlReader *controlReader = nullptr;
static PerformanceScreen *perfScreen = nullptr;
static CCMapScreen *ccMapScreen = nullptr;
static CanalScreen *canalScreen = nullptr;
static OitavaScreen *oitavaScreen = nullptr;
static VelocidadeScreen *velScreen = nullptr;
static ConfigScreen *configScreen = nullptr;
static MenuScreen *menuScreen = nullptr;
static SobreScreen *sobreScreen = nullptr;

void onMidiActivity() {
  if (app)
    app->getMidiActivity().trigger();
}

void onCCActivity(const CCActivityInfo &info) {
  if (perfScreen)
    perfScreen->atualizarCCInfo(info);
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
  i2cBus = new WireI2CBus();
  scanner = new I2CScanner(i2cBus);
  ucl = new UnifiedControlList(scanner);
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
    Serial.println("ERRO: Falha ao inicializar display OLED!");
    // Pisca LED rapidamente para indicar erro ao usuário
    for (int i = 0; i < 10; i++) {
      digitalWrite(HardwareMap::PIN_LED, HIGH);
      delay(100);
      digitalWrite(HardwareMap::PIN_LED, LOW);
      delay(100);
    }
  }

  // Splash screen — identidade do produto
  app->showSplash("MIDI Ctrl", FIRMWARE_VERSION);

  // Telas sem dependencias entre si primeiro
  Serial.println("[7] screens");
  perfScreen = new PerformanceScreen(engine, storage);
  ccMapScreen = new CCMapScreen(storage, ucl);
  canalScreen = new CanalScreen(storage);
  oitavaScreen = new OitavaScreen(storage);
  velScreen = new VelocidadeScreen(storage);
  sobreScreen = new SobreScreen(storage, ucl);

  // ConfigScreen recebe as sub-telas como destinos
  configScreen = new ConfigScreen(app, storage, ccMapScreen, canalScreen,
                                  oitavaScreen, velScreen);

  // MenuScreen recebe storage para exibir status no rodapé
  menuScreen =
      new MenuScreen(app, storage, perfScreen, configScreen, sobreScreen);

  perfScreen->setApp(app);
  ccMapScreen->setApp(app);
  canalScreen->setApp(app);
  oitavaScreen->setApp(app);
  velScreen->setApp(app);
  sobreScreen->setApp(app);

  Serial.println("[8] activity");
  engine->onActivity(onMidiActivity);
  controlReader->onCCActivity(onCCActivity);

  Serial.println("[9] buttons");
  btnUp = new App::Button(HardwareMap::PIN_BTN_UP, true);
  btnDown = new App::Button(HardwareMap::PIN_BTN_DOWN, true);
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
  if (controlReader)
    controlReader->update();
  if (scanner)
    scanner->periodicScan();
  if (ucl && scanner && scanner->needsRebuild()) {
    ucl->rebuild();
    scanner->clearRebuildFlag();
  }
  if (app)
    app->update();
}
