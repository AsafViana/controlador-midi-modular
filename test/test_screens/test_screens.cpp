#include "config.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include "ui/Router.h"
#include <unity.h>


// Screens under test
#include "screens/CanalScreen.h"
#include "screens/ConfigScreen.h"
#include "screens/MenuScreen.h"
#include "screens/OitavaScreen.h"
#include "screens/PerformanceScreen.h"
#include "screens/SobreScreen.h"
#include "screens/VelocidadeScreen.h"

// Mocks
#include "Arduino.h"

static Storage storage;
static OledApp app;
static MidiEngine engine;

// Dummy screens for MenuScreen/ConfigScreen dependencies
static PerformanceScreen *perfScreen = nullptr;
static ConfigScreen *configScreen = nullptr;
static SobreScreen *sobreScreen = nullptr;
static CCMapScreen *ccMapScreen = nullptr;
static CanalScreen *canalScreen = nullptr;
static OitavaScreen *oitavaScreen = nullptr;
static VelocidadeScreen *velScreen = nullptr;

void setUp() {
  mock::reset();
  storage.begin();
  // OledApp::begin() needs Wire — in native mock it's a no-op that succeeds
  app.begin(0x3C);
}

void tearDown() {}

// ═══════════════════════════════════════════════════════════
// CanalScreen
// ═══════════════════════════════════════════════════════════

void test_canal_screen_loads_from_storage() {
  storage.setCanalMidi(5);
  CanalScreen screen(&storage);
  screen.setApp(&app);
  screen.onMount();
  // After onMount, internal _canal should be 5
  // We verify by pressing SELECT (saves) and checking storage
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(5, storage.getCanalMidi());
}

void test_canal_screen_up_increments() {
  storage.setCanalMidi(1);
  CanalScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::UP);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(2, storage.getCanalMidi());
}

void test_canal_screen_down_decrements() {
  storage.setCanalMidi(5);
  CanalScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::DOWN);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(4, storage.getCanalMidi());
}

void test_canal_screen_clamps_at_max() {
  storage.setCanalMidi(16);
  CanalScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::UP);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(16, storage.getCanalMidi());
}

void test_canal_screen_clamps_at_min() {
  storage.setCanalMidi(1);
  CanalScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::DOWN);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(1, storage.getCanalMidi());
}

void test_canal_screen_long_up_accelerates() {
  storage.setCanalMidi(1);
  CanalScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::LONG_UP);
  screen.handleInput(NavInput::SELECT);
  // LONG_UP adds 3 (passo=3 for canal)
  TEST_ASSERT_EQUAL_UINT8(4, storage.getCanalMidi());
}

void test_canal_screen_long_down_accelerates() {
  storage.setCanalMidi(10);
  CanalScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::LONG_DOWN);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(7, storage.getCanalMidi());
}

// ═══════════════════════════════════════════════════════════
// OitavaScreen
// ═══════════════════════════════════════════════════════════

void test_oitava_screen_loads_from_storage() {
  storage.setOitava(3);
  OitavaScreen screen(&storage);
  screen.setApp(&app);
  screen.onMount();
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(3, storage.getOitava());
}

void test_oitava_screen_up_increments() {
  storage.setOitava(4);
  OitavaScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::UP);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(5, storage.getOitava());
}

void test_oitava_screen_clamps_at_8() {
  storage.setOitava(8);
  OitavaScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::UP);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(8, storage.getOitava());
}

void test_oitava_screen_clamps_at_0() {
  storage.setOitava(0);
  OitavaScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::DOWN);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(0, storage.getOitava());
}

void test_oitava_screen_long_up_step_2() {
  storage.setOitava(3);
  OitavaScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::LONG_UP);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(5, storage.getOitava());
}

// ═══════════════════════════════════════════════════════════
// VelocidadeScreen
// ═══════════════════════════════════════════════════════════

void test_velocidade_screen_loads_from_storage() {
  storage.setVelocidade(80);
  VelocidadeScreen screen(&storage);
  screen.setApp(&app);
  screen.onMount();
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(80, storage.getVelocidade());
}

void test_velocidade_screen_up_increments() {
  storage.setVelocidade(100);
  VelocidadeScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::UP);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(101, storage.getVelocidade());
}

void test_velocidade_screen_clamps_at_127() {
  storage.setVelocidade(127);
  VelocidadeScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::UP);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(127, storage.getVelocidade());
}

void test_velocidade_screen_clamps_at_1() {
  storage.setVelocidade(1);
  VelocidadeScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::DOWN);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(1, storage.getVelocidade());
}

void test_velocidade_screen_long_up_step_5() {
  storage.setVelocidade(50);
  VelocidadeScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::LONG_UP);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(55, storage.getVelocidade());
}

void test_velocidade_screen_long_down_step_5() {
  storage.setVelocidade(50);
  VelocidadeScreen screen(&storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::LONG_DOWN);
  screen.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_UINT8(45, storage.getVelocidade());
}

// ═══════════════════════════════════════════════════════════
// PerformanceScreen
// ═══════════════════════════════════════════════════════════

void test_performance_screen_oitava_up() {
  storage.setOitava(4);
  PerformanceScreen screen(&engine, &storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::UP);
  TEST_ASSERT_EQUAL_UINT8(5, storage.getOitava());
}

void test_performance_screen_oitava_down() {
  storage.setOitava(4);
  PerformanceScreen screen(&engine, &storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::DOWN);
  TEST_ASSERT_EQUAL_UINT8(3, storage.getOitava());
}

void test_performance_screen_oitava_clamps_at_8() {
  storage.setOitava(8);
  PerformanceScreen screen(&engine, &storage);
  screen.setApp(&app);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::UP);
  TEST_ASSERT_EQUAL_UINT8(8, storage.getOitava());
}

void test_performance_screen_cc_info_marks_dirty() {
  PerformanceScreen screen(&engine, &storage);
  screen.setApp(&app);
  screen.onMount();
  screen.clearDirty();

  CCActivityInfo info;
  info.label = "Pot Volume";
  info.cc = 7;
  info.valor = 64;
  info.canal = 1;
  info.isRemoto = false;
  info.moduleAddress = 0;

  screen.atualizarCCInfo(info);
  TEST_ASSERT_TRUE(screen.isDirty());
}

// ═══════════════════════════════════════════════════════════
// SobreScreen
// ═══════════════════════════════════════════════════════════

void test_sobre_screen_renders_without_crash() {
  SobreScreen screen(&storage);
  screen.setApp(&app);
  screen.onMount();

  Adafruit_SSD1306 display;
  screen.render(display);
  // Should not crash and should print something
  TEST_ASSERT_TRUE(display.printCallCount > 0);
}

void test_sobre_screen_select_pops() {
  // Push a base screen, then SobreScreen
  SobreScreen screen(&storage);
  screen.setApp(&app);

  // Create a dummy base screen
  class DummyScreen : public Screen {
  public:
    void render(Adafruit_SSD1306 &d) override {}
  };
  DummyScreen base;
  app.getRouter().push(&base);
  app.getRouter().push(&screen);

  screen.handleInput(NavInput::SELECT);
  // After SELECT, router should have popped back to base
  TEST_ASSERT_EQUAL_PTR(&base, app.getRouter().currentScreen());
}

// ═══════════════════════════════════════════════════════════
// ConfigScreen — Factory Reset
// ═══════════════════════════════════════════════════════════

void test_config_screen_factory_reset_flow() {
  // Change some settings
  storage.setCanalMidi(10);
  storage.setVelocidade(50);
  storage.setOitava(7);

  // Create all sub-screens needed by ConfigScreen
  canalScreen = new CanalScreen(&storage);
  oitavaScreen = new OitavaScreen(&storage);
  velScreen = new VelocidadeScreen(&storage);
  ccMapScreen = new CCMapScreen(&storage);

  ConfigScreen screen(&app, &storage, ccMapScreen, canalScreen, oitavaScreen,
                      velScreen);
  app.getRouter().push(&screen);

  // Navigate to "Restaurar" (index 4)
  screen.handleInput(NavInput::DOWN);   // 0 -> 1
  screen.handleInput(NavInput::DOWN);   // 1 -> 2
  screen.handleInput(NavInput::DOWN);   // 2 -> 3
  screen.handleInput(NavInput::DOWN);   // 3 -> 4
  screen.handleInput(NavInput::SELECT); // Enter confirmation mode

  // Confirm reset
  screen.handleInput(NavInput::SELECT);

  // Verify factory defaults restored
  TEST_ASSERT_EQUAL_UINT8(1, storage.getCanalMidi());
  TEST_ASSERT_EQUAL_UINT8(100, storage.getVelocidade());
  TEST_ASSERT_EQUAL_UINT8(4, storage.getOitava());

  delete canalScreen;
  delete oitavaScreen;
  delete velScreen;
  delete ccMapScreen;
}

void test_config_screen_factory_reset_cancel() {
  storage.setCanalMidi(10);

  canalScreen = new CanalScreen(&storage);
  oitavaScreen = new OitavaScreen(&storage);
  velScreen = new VelocidadeScreen(&storage);
  ccMapScreen = new CCMapScreen(&storage);

  ConfigScreen screen(&app, &storage, ccMapScreen, canalScreen, oitavaScreen,
                      velScreen);
  app.getRouter().push(&screen);

  // Navigate to "Restaurar" (index 4)
  for (int i = 0; i < 4; i++)
    screen.handleInput(NavInput::DOWN);
  screen.handleInput(NavInput::SELECT); // Enter confirmation

  // Cancel with UP
  screen.handleInput(NavInput::UP);

  // Settings should be unchanged
  TEST_ASSERT_EQUAL_UINT8(10, storage.getCanalMidi());

  delete canalScreen;
  delete oitavaScreen;
  delete velScreen;
  delete ccMapScreen;
}

// ═══════════════════════════════════════════════════════════
// MenuScreen — Navigation
// ═══════════════════════════════════════════════════════════

void test_menu_screen_select_performance() {
  perfScreen = new PerformanceScreen(&engine, &storage);
  sobreScreen = new SobreScreen(&storage);
  canalScreen = new CanalScreen(&storage);
  oitavaScreen = new OitavaScreen(&storage);
  velScreen = new VelocidadeScreen(&storage);
  ccMapScreen = new CCMapScreen(&storage);
  configScreen = new ConfigScreen(&app, &storage, ccMapScreen, canalScreen,
                                  oitavaScreen, velScreen);

  MenuScreen menu(&app, &storage, perfScreen, configScreen, sobreScreen);
  app.getRouter().push(&menu);

  // First item is "Performance" — SELECT should push perfScreen
  menu.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_PTR(perfScreen, app.getRouter().currentScreen());

  delete perfScreen;
  delete sobreScreen;
  delete canalScreen;
  delete oitavaScreen;
  delete velScreen;
  delete ccMapScreen;
  delete configScreen;
}

void test_menu_screen_select_config() {
  perfScreen = new PerformanceScreen(&engine, &storage);
  sobreScreen = new SobreScreen(&storage);
  canalScreen = new CanalScreen(&storage);
  oitavaScreen = new OitavaScreen(&storage);
  velScreen = new VelocidadeScreen(&storage);
  ccMapScreen = new CCMapScreen(&storage);
  configScreen = new ConfigScreen(&app, &storage, ccMapScreen, canalScreen,
                                  oitavaScreen, velScreen);

  MenuScreen menu(&app, &storage, perfScreen, configScreen, sobreScreen);
  app.getRouter().push(&menu);

  // Navigate to "Configuracoes" (index 1)
  menu.handleInput(NavInput::DOWN);
  menu.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_PTR(configScreen, app.getRouter().currentScreen());

  delete perfScreen;
  delete sobreScreen;
  delete canalScreen;
  delete oitavaScreen;
  delete velScreen;
  delete ccMapScreen;
  delete configScreen;
}

void test_menu_screen_select_sobre() {
  perfScreen = new PerformanceScreen(&engine, &storage);
  sobreScreen = new SobreScreen(&storage);
  canalScreen = new CanalScreen(&storage);
  oitavaScreen = new OitavaScreen(&storage);
  velScreen = new VelocidadeScreen(&storage);
  ccMapScreen = new CCMapScreen(&storage);
  configScreen = new ConfigScreen(&app, &storage, ccMapScreen, canalScreen,
                                  oitavaScreen, velScreen);

  MenuScreen menu(&app, &storage, perfScreen, configScreen, sobreScreen);
  app.getRouter().push(&menu);

  // Navigate to "Sobre" (index 2)
  menu.handleInput(NavInput::DOWN);
  menu.handleInput(NavInput::DOWN);
  menu.handleInput(NavInput::SELECT);
  TEST_ASSERT_EQUAL_PTR(sobreScreen, app.getRouter().currentScreen());

  delete perfScreen;
  delete sobreScreen;
  delete canalScreen;
  delete oitavaScreen;
  delete velScreen;
  delete ccMapScreen;
  delete configScreen;
}

// ═══════════════════════════════════════════════════════════
// Runner
// ═══════════════════════════════════════════════════════════

int main() {
  UNITY_BEGIN();

  // CanalScreen
  RUN_TEST(test_canal_screen_loads_from_storage);
  RUN_TEST(test_canal_screen_up_increments);
  RUN_TEST(test_canal_screen_down_decrements);
  RUN_TEST(test_canal_screen_clamps_at_max);
  RUN_TEST(test_canal_screen_clamps_at_min);
  RUN_TEST(test_canal_screen_long_up_accelerates);
  RUN_TEST(test_canal_screen_long_down_accelerates);

  // OitavaScreen
  RUN_TEST(test_oitava_screen_loads_from_storage);
  RUN_TEST(test_oitava_screen_up_increments);
  RUN_TEST(test_oitava_screen_clamps_at_8);
  RUN_TEST(test_oitava_screen_clamps_at_0);
  RUN_TEST(test_oitava_screen_long_up_step_2);

  // VelocidadeScreen
  RUN_TEST(test_velocidade_screen_loads_from_storage);
  RUN_TEST(test_velocidade_screen_up_increments);
  RUN_TEST(test_velocidade_screen_clamps_at_127);
  RUN_TEST(test_velocidade_screen_clamps_at_1);
  RUN_TEST(test_velocidade_screen_long_up_step_5);
  RUN_TEST(test_velocidade_screen_long_down_step_5);

  // PerformanceScreen
  RUN_TEST(test_performance_screen_oitava_up);
  RUN_TEST(test_performance_screen_oitava_down);
  RUN_TEST(test_performance_screen_oitava_clamps_at_8);
  RUN_TEST(test_performance_screen_cc_info_marks_dirty);

  // SobreScreen
  RUN_TEST(test_sobre_screen_renders_without_crash);
  RUN_TEST(test_sobre_screen_select_pops);

  // ConfigScreen
  RUN_TEST(test_config_screen_factory_reset_flow);
  RUN_TEST(test_config_screen_factory_reset_cancel);

  // MenuScreen
  RUN_TEST(test_menu_screen_select_performance);
  RUN_TEST(test_menu_screen_select_config);
  RUN_TEST(test_menu_screen_select_sobre);

  return UNITY_END();
}
