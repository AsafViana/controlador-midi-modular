/**
 * Unit Tests: I2CScanner
 *
 * Testes unitários do scanner I2C cobrindo cenários específicos:
 * - Varredura em barramento vazio
 * - Endereço OLED (0x3C) é ignorado
 * - Módulo que não responde ao descritor é ignorado
 * - Varredura periódica respeita intervalo de 5s
 *
 * Requisitos: 2.1, 2.4, 2.5, 10.4
 */

#include <unity.h>
#include <cstring>
#include "i2c/I2CScanner.h"
#include "MockI2CBus.h"
#include "Arduino.h"

// ── Helpers ──────────────────────────────────────────────

/// Cria um MockModule simples com 1 controle no endereço dado
static MockModule makeSimpleModule(uint8_t address) {
    MockModule mod;
    memset(&mod, 0, sizeof(mod));
    mod.address = address;
    mod.numControles = 1;
    mod.tipos[0] = 1; // POTENCIOMETRO
    strncpy(mod.labels[0], "Ctrl", 12);
    mod.labels[0][12] = '\0';
    mod.valores[0] = 64;
    mod.respondePing = true;
    mod.respondeDescritor = true;
    return mod;
}

// ── setUp / tearDown ─────────────────────────────────────

void setUp() {
    mock::reset();
}

void tearDown() {}

// ============================================================
// test_scan_empty_bus
// Validates: Requirement 2.1
// Varredura sem módulos retorna 0
// ============================================================

void test_scan_empty_bus() {
    MockI2CBus bus;
    I2CScanner scanner(&bus);

    uint8_t found = scanner.scan();

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        0, found,
        "scan() on empty bus must return 0 modules");

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        0, scanner.getModuleCount(),
        "getModuleCount() must be 0 on empty bus");

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        0, scanner.getTotalRemoteControls(),
        "getTotalRemoteControls() must be 0 on empty bus");
}

// ============================================================
// test_scan_skips_0x3C
// Validates: Requirement 2.5
// Endereço do OLED (0x3C) é ignorado durante a varredura
// ============================================================

void test_scan_skips_0x3C() {
    MockI2CBus bus;
    I2CScanner scanner(&bus);

    // Registrar um módulo no endereço 0x3C (OLED)
    MockModule oledModule = makeSimpleModule(0x3C);
    bus.addModule(oledModule);

    // Registrar um módulo válido em 0x20
    MockModule validModule = makeSimpleModule(0x20);
    bus.addModule(validModule);

    uint8_t found = scanner.scan();

    // Deve encontrar apenas o módulo em 0x20, ignorando 0x3C
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        1, found,
        "scan() must find only 1 module (0x3C must be skipped)");

    const ModuleInfo* info = scanner.getModule(0);
    TEST_ASSERT_NOT_NULL(info);
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(
        0x20, info->address,
        "The discovered module must be at address 0x20, not 0x3C");
}

// ============================================================
// test_scan_timeout_skips_module
// Validates: Requirement 2.4
// Módulo que responde ao probe mas não ao descritor é ignorado
// ============================================================

void test_scan_timeout_skips_module() {
    MockI2CBus bus;
    I2CScanner scanner(&bus);

    // Módulo em 0x20: responde ao ping mas NÃO ao descritor
    MockModule timeoutModule = makeSimpleModule(0x20);
    timeoutModule.respondeDescritor = false;
    bus.addModule(timeoutModule);

    // Módulo em 0x21: responde normalmente
    MockModule goodModule = makeSimpleModule(0x21);
    bus.addModule(goodModule);

    uint8_t found = scanner.scan();

    // Deve encontrar apenas o módulo em 0x21
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        1, found,
        "scan() must skip module that doesn't respond to descriptor");

    const ModuleInfo* info = scanner.getModule(0);
    TEST_ASSERT_NOT_NULL(info);
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(
        0x21, info->address,
        "The discovered module must be at 0x21 (0x20 timed out on descriptor)");
}

// ============================================================
// test_periodic_scan_interval
// Validates: Requirement 10.4
// Varredura periódica respeita intervalo de 5s
// ============================================================

void test_periodic_scan_interval() {
    MockI2CBus bus;
    I2CScanner scanner(&bus);

    // Tempo inicial = 0
    mock::setMillis(0);

    // Scan inicial — descobre 0 módulos
    scanner.scan();
    TEST_ASSERT_EQUAL_UINT8(0, scanner.getModuleCount());

    // Adicionar um módulo após o scan inicial
    MockModule mod = makeSimpleModule(0x20);
    bus.addModule(mod);

    // Chamar periodicScan antes de 5s — NÃO deve varrer
    mock::setMillis(4999);
    scanner.periodicScan();
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        0, scanner.getModuleCount(),
        "periodicScan() must NOT scan before 5s interval");

    // Chamar periodicScan exatamente em 5s — DEVE varrer
    mock::setMillis(5000);
    scanner.periodicScan();
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        1, scanner.getModuleCount(),
        "periodicScan() must scan at exactly 5s interval");

    // Verificar que o módulo foi descoberto corretamente
    const ModuleInfo* info = scanner.getModule(0);
    TEST_ASSERT_NOT_NULL(info);
    TEST_ASSERT_EQUAL_HEX8(0x20, info->address);
    TEST_ASSERT_TRUE(info->connected);

    // Chamar periodicScan novamente antes de mais 5s — NÃO deve varrer novamente
    // Remover o módulo para verificar que não houve nova varredura
    bus.setModuleConnected(0x20, false);

    mock::setMillis(9999);
    scanner.periodicScan();
    // O módulo ainda deve estar conectado (periodicScan não rodou)
    info = scanner.getModule(0);
    TEST_ASSERT_NOT_NULL(info);
    TEST_ASSERT_TRUE_MESSAGE(info->connected,
        "periodicScan() must NOT re-scan before next 5s interval");

    // Em 10s (5s após a última varredura) — DEVE varrer e detectar desconexão
    mock::setMillis(10000);
    scanner.periodicScan();
    info = scanner.getModule(0);
    TEST_ASSERT_NOT_NULL(info);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        1, info->failCount,
        "periodicScan() at 10s must detect disconnection and increment failCount");
}

// ── Unity runner ─────────────────────────────────────────

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_scan_empty_bus);
    RUN_TEST(test_scan_skips_0x3C);
    RUN_TEST(test_scan_timeout_skips_module);
    RUN_TEST(test_periodic_scan_interval);

    return UNITY_END();
}
