/**
 * Unit Tests: UnifiedControlList
 *
 * Testes unitários da lista unificada cobrindo cenários específicos:
 * - test_rebuild_no_modules: lista contém apenas locais sem módulos
 * - test_rebuild_max_controls: lista respeita limite de 32 controles
 * - test_rebuild_after_disconnect: lista atualizada após desconexão
 *
 * Requisitos: 1.4, 4.2
 */

#include "Arduino.h"
#include "MockI2CBus.h"
#include "hardware/HardwareMap.h"
#include "hardware/UnifiedControlList.h"
#include "i2c/I2CScanner.h"
#include <cstdio>
#include <cstring>
#include <unity.h>


// ── Helpers ──────────────────────────────────────────────

/// Cria um MockModule com N controles no endereço dado
static MockModule makeModule(uint8_t address, uint8_t numControles) {
  MockModule mod;
  memset(&mod, 0, sizeof(mod));
  mod.address = address;
  mod.numControles = numControles;
  mod.respondePing = true;
  mod.respondeDescritor = true;

  for (uint8_t i = 0; i < numControles; i++) {
    mod.tipos[i] = 1; // POTENCIOMETRO
    char lbl[13];
    snprintf(lbl, sizeof(lbl), "M%02X_C%u", address, i);
    memcpy(mod.labels[i], lbl, 13);
    mod.valores[i] = static_cast<uint8_t>(i * 8);
  }
  return mod;
}

// ── setUp / tearDown ─────────────────────────────────────

void setUp() { mock::reset(); }

void tearDown() {}

// ============================================================
// test_rebuild_no_modules
// Validates: Requirement 1.4
// Sem módulos externos, a lista contém apenas controles locais
// ============================================================

void test_rebuild_no_modules() {
  MockI2CBus bus;
  I2CScanner scanner(&bus);

  // Scan em barramento vazio
  scanner.scan();

  UnifiedControlList ucl(&scanner);
  ucl.rebuild();

  // Número total de controles deve ser igual ao HardwareMap
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      HardwareMap::NUM_CONTROLES, ucl.getNumControles(),
      "No modules: numControles must equal HardwareMap::NUM_CONTROLES");

  // Número de locais deve ser igual ao total
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      HardwareMap::NUM_CONTROLES, ucl.getNumLocais(),
      "No modules: numLocais must equal HardwareMap::NUM_CONTROLES");

  // Todos os controles devem ser locais e corresponder ao HardwareMap
  for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
    TEST_ASSERT_FALSE_MESSAGE(
        ucl.isRemoto(i),
        "No modules: all controls must be local (isRemoto==false)");

    TEST_ASSERT_EQUAL_STRING_MESSAGE(
        HardwareMap::CONTROLES[i].label, ucl.getLabel(i),
        "No modules: label must match HardwareMap");

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        static_cast<uint8_t>(HardwareMap::CONTROLES[i].tipo),
        static_cast<uint8_t>(ucl.getTipo(i)),
        "No modules: tipo must match HardwareMap");

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        HardwareMap::getCCPadrao(i), ucl.getCCPadrao(i),
        "No modules: ccPadrao must match HardwareMap");
  }

  // getRemoteInfo deve retornar false para todos os locais
  for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
    uint8_t addr, idx;
    TEST_ASSERT_FALSE_MESSAGE(
        ucl.getRemoteInfo(i, addr, idx),
        "No modules: getRemoteInfo must return false for local controls");
  }
}

// ============================================================
// test_rebuild_max_controls
// Validates: Requirement 4.2
// Lista respeita limite de MAX_TOTAL_CONTROLS (32)
// ============================================================

void test_rebuild_max_controls() {
  MockI2CBus bus;

  // HardwareMap tem NUM_CONTROLES locais (4).
  // Precisamos de módulos remotos suficientes para exceder 32 no total.
  // Cada módulo pode ter até 16 controles. Vamos criar módulos com
  // controles suficientes para ultrapassar o limite.
  //
  // 4 locais + precisamos de > 28 remotos para exceder 32.
  // Usamos 3 módulos com 16 controles cada = 48 remotos.
  // Total sem cap: 4 + 48 = 52, mas deve ser limitado a 32.

  uint8_t addresses[] = {0x20, 0x21, 0x22};
  for (uint8_t i = 0; i < 3; i++) {
    MockModule mod = makeModule(addresses[i], 16);
    bus.addModule(mod);
  }

  I2CScanner scanner(&bus);
  scanner.scan();

  UnifiedControlList ucl(&scanner);
  ucl.rebuild();

  // Total deve ser exatamente MAX_TOTAL_CONTROLS
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      UnifiedControlList::MAX_TOTAL_CONTROLS, ucl.getNumControles(),
      "Max controls: numControles must be capped at MAX_TOTAL_CONTROLS (32)");

  // Locais devem estar todos presentes
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      HardwareMap::NUM_CONTROLES, ucl.getNumLocais(),
      "Max controls: all local controls must be present");

  // Remotos = 32 - NUM_CONTROLES
  uint8_t expectedRemotes =
      UnifiedControlList::MAX_TOTAL_CONTROLS - HardwareMap::NUM_CONTROLES;
  uint8_t actualRemotes = ucl.getNumControles() - ucl.getNumLocais();
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      expectedRemotes, actualRemotes,
      "Max controls: remote controls must fill up to MAX_TOTAL_CONTROLS");

  // Verificar que todos os locais estão corretos
  for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
    TEST_ASSERT_FALSE(ucl.isRemoto(i));
  }

  // Verificar que todos os remotos estão corretos
  for (uint8_t i = HardwareMap::NUM_CONTROLES; i < ucl.getNumControles(); i++) {
    TEST_ASSERT_TRUE_MESSAGE(
        ucl.isRemoto(i), "Max controls: indices after locals must be remote");
  }

  // Verificar que getControlInfo retorna false para índice fora do limite
  ControlInfo info;
  TEST_ASSERT_FALSE_MESSAGE(
      ucl.getControlInfo(UnifiedControlList::MAX_TOTAL_CONTROLS, info),
      "Max controls: getControlInfo must return false for index >= "
      "MAX_TOTAL_CONTROLS");
}

// ============================================================
// test_rebuild_after_disconnect
// Validates: Requirements 1.4, 4.2
// Lista é atualizada após desconexão de módulo
// ============================================================

void test_rebuild_after_disconnect() {
  MockI2CBus bus;

  // Criar 2 módulos: 0x20 com 3 controles, 0x21 com 2 controles
  MockModule mod1 = makeModule(0x20, 3);
  MockModule mod2 = makeModule(0x21, 2);
  bus.addModule(mod1);
  bus.addModule(mod2);

  I2CScanner scanner(&bus);
  mock::setMillis(0);
  scanner.scan();

  UnifiedControlList ucl(&scanner);
  ucl.rebuild();

  // Estado inicial: 4 locais + 5 remotos = 9 total
  uint8_t initialTotal = HardwareMap::NUM_CONTROLES + 3 + 2;
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      initialTotal, ucl.getNumControles(),
      "Before disconnect: total must include all modules");

  // Desconectar módulo 0x20 (simular 3 falhas consecutivas)
  bus.setModuleConnected(0x20, false);

  // Precisamos de 3 periodicScans com falha para marcar como desconectado
  // Cada periodicScan precisa respeitar o intervalo de 5s
  for (uint8_t fail = 0; fail < I2CScanner::MAX_FAIL_COUNT; fail++) {
    mock::advanceMillis(I2CScanner::RESCAN_INTERVAL_MS);
    scanner.periodicScan();
  }

  // Verificar que o módulo 0x20 foi marcado como desconectado
  const ModuleInfo *mod0x20 = scanner.getModule(0);
  TEST_ASSERT_NOT_NULL(mod0x20);
  TEST_ASSERT_FALSE_MESSAGE(
      mod0x20->connected, "After 3 failures: module 0x20 must be disconnected");

  // Rebuild a lista após desconexão
  ucl.rebuild();

  // Agora deve ter: 4 locais + 2 remotos (apenas 0x21) = 6 total
  uint8_t afterDisconnectTotal = HardwareMap::NUM_CONTROLES + 2;
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      afterDisconnectTotal, ucl.getNumControles(),
      "After disconnect: total must exclude disconnected module's controls");

  // Locais devem permanecer inalterados
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      HardwareMap::NUM_CONTROLES, ucl.getNumLocais(),
      "After disconnect: local controls must remain unchanged");

  // Os remotos restantes devem ser do módulo 0x21
  for (uint8_t i = HardwareMap::NUM_CONTROLES; i < ucl.getNumControles(); i++) {
    uint8_t addr, ctrlIdx;
    bool ok = ucl.getRemoteInfo(i, addr, ctrlIdx);
    TEST_ASSERT_TRUE_MESSAGE(
        ok,
        "After disconnect: getRemoteInfo must succeed for remaining remotes");
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(
        0x21, addr,
        "After disconnect: remaining remotes must be from module 0x21");
  }
}

// ── Unity runner ─────────────────────────────────────────

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_rebuild_no_modules);
  RUN_TEST(test_rebuild_max_controls);
  RUN_TEST(test_rebuild_after_disconnect);

  return UNITY_END();
}
