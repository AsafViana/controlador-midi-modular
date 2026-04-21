/**
 * Property-Based Tests: I2CScanner
 *
 * Feature: i2c-modular-expansion, Property 2: Scanner registra módulos com descritores válidos
 * Feature: i2c-modular-expansion, Property 4: Leitura de valores remotos preserva dados
 * Feature: i2c-modular-expansion, Property 5: Resiliência — últimos valores mantidos após falha
 * Feature: i2c-modular-expansion, Property 11: Desconexão após 3 falhas consecutivas
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include <cstring>
#include "i2c/I2CScanner.h"
#include "MockI2CBus.h"

// --- Simple LCG pseudo-random number generator ---
static uint32_t lcg_state = 12345u;

static void lcg_seed(uint32_t seed) {
    lcg_state = seed;
}

static uint32_t lcg_next() {
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state;
}

// --- Random generators ---

/// Generate a random numControles in [1, 16]
static uint8_t rand_numControles() {
    return static_cast<uint8_t>((lcg_next() % 16) + 1);
}

/// Generate a random valid TipoControle (0-3)
static uint8_t rand_tipo() {
    return static_cast<uint8_t>(lcg_next() % 4);
}

/// Generate a random valor in [0, 127]
static uint8_t rand_valor() {
    return static_cast<uint8_t>(lcg_next() % 128);
}

/// Generate a random ASCII label of length [1, 12]
/// Uses printable ASCII range (33-126) to avoid empty labels
static void rand_label(char* label) {
    uint8_t len = static_cast<uint8_t>(1 + (lcg_next() % I2CProtocol::LABEL_MAX_LEN));
    for (uint8_t j = 0; j < len; j++) {
        label[j] = static_cast<char>(33 + (lcg_next() % 94)); // printable non-space ASCII
    }
    label[len] = '\0';
    // Null-pad the rest
    for (uint8_t j = len + 1; j <= I2CProtocol::LABEL_MAX_LEN; j++) {
        label[j] = '\0';
    }
}

/// Generate a random MockModule with valid descriptor at the given address
static void rand_mock_module(MockModule& mod, uint8_t address) {
    memset(&mod, 0, sizeof(mod));
    mod.address = address;
    mod.numControles = rand_numControles();
    mod.respondePing = true;
    mod.respondeDescritor = true;

    for (uint8_t i = 0; i < mod.numControles; i++) {
        mod.tipos[i] = rand_tipo();
        rand_label(mod.labels[i]);
        mod.valores[i] = rand_valor();
    }
}

/// Pick N distinct addresses from the valid range [0x20, 0x27]
/// Returns the number of addresses picked (1 to maxCount, capped at 8)
static uint8_t pick_distinct_addresses(uint8_t* addrs, uint8_t maxCount) {
    // Available addresses: 0x20..0x27 = 8 addresses
    // (0x3C is OLED, outside this range, so no conflict)
    uint8_t pool[8];
    for (uint8_t i = 0; i < 8; i++) {
        pool[i] = I2CProtocol::ADDR_MIN + i;
    }

    // Fisher-Yates shuffle
    for (uint8_t i = 7; i > 0; i--) {
        uint8_t j = static_cast<uint8_t>(lcg_next() % (i + 1));
        uint8_t tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }

    // Pick first N
    uint8_t count = (maxCount <= 8) ? maxCount : 8;
    for (uint8_t i = 0; i < count; i++) {
        addrs[i] = pool[i];
    }
    return count;
}

// ============================================================
// Property 2: Scanner registra módulos com descritores válidos
// ============================================================

static const int PBT_ITERATIONS = 100;

/**
 * **Validates: Requirements 2.3, 2.6**
 *
 * For any set of MockModules with valid descriptors (1-16 controls,
 * known types, ASCII labels) registered on MockI2CBus at distinct
 * addresses in range 0x20-0x27, after scan(), the number of
 * discovered modules must equal the number of registered MockModules,
 * and each module must have matching address, numControles, types
 * and labels.
 */
void test_property2_scanner_registers_valid_modules() {
    lcg_seed(77u);

    MockI2CBus bus;
    I2CScanner scanner(&bus);

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        bus.clearModules();

        // Generate 1-8 modules with distinct addresses
        uint8_t numModules = static_cast<uint8_t>(1 + (lcg_next() % 8));
        uint8_t addresses[8];
        numModules = pick_distinct_addresses(addresses, numModules);

        // Create and register mock modules, keeping a copy for verification
        MockModule expected[8];
        for (uint8_t m = 0; m < numModules; m++) {
            rand_mock_module(expected[m], addresses[m]);
            bus.addModule(expected[m]);
        }

        // Run scan
        uint8_t found = scanner.scan();

        // Verify module count matches
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            numModules, found,
            "scan() must discover all registered mock modules");

        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            numModules, scanner.getModuleCount(),
            "getModuleCount() must match number of registered modules");

        // Verify each discovered module matches its expected MockModule.
        // The scanner iterates addresses in order 0x20..0x27, so we need
        // to find each scanner result in our expected array by address.
        for (uint8_t i = 0; i < found; i++) {
            const ModuleInfo* info = scanner.getModule(i);
            TEST_ASSERT_NOT_NULL_MESSAGE(info,
                "getModule() must return non-null for valid index");

            // Find the corresponding expected module by address
            const MockModule* exp = nullptr;
            for (uint8_t m = 0; m < numModules; m++) {
                if (expected[m].address == info->address) {
                    exp = &expected[m];
                    break;
                }
            }
            TEST_ASSERT_NOT_NULL_MESSAGE(exp,
                "Discovered module address must match a registered MockModule");

            // Verify connected state
            TEST_ASSERT_TRUE_MESSAGE(info->connected,
                "Discovered module must be marked as connected");

            // Verify numControles
            TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                exp->numControles, info->descriptor.numControles,
                "numControles must match between MockModule and discovered module");

            // Verify each control's type and label
            for (uint8_t c = 0; c < exp->numControles; c++) {
                // Verify tipo
                TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                    exp->tipos[c],
                    static_cast<uint8_t>(info->descriptor.controles[c].tipo),
                    "Control tipo must match between MockModule and discovered module");

                // Verify label (compare up to 12 chars)
                TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(
                    exp->labels[c],
                    info->descriptor.controles[c].label,
                    I2CProtocol::LABEL_MAX_LEN,
                    "Control label must match between MockModule and discovered module");
            }
        }
    }
}

// ============================================================
// Property 4: Leitura de valores remotos preserva dados
// ============================================================

/**
 * **Validates: Requirements 3.3**
 *
 * For any MockModule with N controls (1-16) and any values (0-127)
 * configured on MockI2CBus, after scan(), reading values via
 * I2CScanner::readValues(moduleIndex) must return exactly the same
 * N values in the same order.
 */
void test_property4_value_reading_preserves_data() {
    lcg_seed(42u);

    MockI2CBus bus;
    I2CScanner scanner(&bus);

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        bus.clearModules();

        // Generate 1-8 modules with distinct addresses
        uint8_t numModules = static_cast<uint8_t>(1 + (lcg_next() % 8));
        uint8_t addresses[8];
        numModules = pick_distinct_addresses(addresses, numModules);

        // Create and register mock modules, keeping a copy for verification
        MockModule expected[8];
        for (uint8_t m = 0; m < numModules; m++) {
            rand_mock_module(expected[m], addresses[m]);
            bus.addModule(expected[m]);
        }

        // Run scan to discover modules
        uint8_t found = scanner.scan();
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            numModules, found,
            "Property 4: scan() must discover all registered modules");

        // For each discovered module, read values and verify they match
        for (uint8_t i = 0; i < found; i++) {
            const ModuleInfo* info = scanner.getModule(i);
            TEST_ASSERT_NOT_NULL_MESSAGE(info,
                "Property 4: getModule() must return non-null for valid index");

            // Find the corresponding expected module by address
            const MockModule* exp = nullptr;
            for (uint8_t m = 0; m < numModules; m++) {
                if (expected[m].address == info->address) {
                    exp = &expected[m];
                    break;
                }
            }
            TEST_ASSERT_NOT_NULL_MESSAGE(exp,
                "Property 4: discovered module address must match a registered MockModule");

            uint8_t numControles = exp->numControles;

            // Read values via readValues(moduleIndex, ...)
            uint8_t readBuf[16];
            memset(readBuf, 0xFF, sizeof(readBuf));

            bool ok = scanner.readValues(i, readBuf, numControles);
            TEST_ASSERT_TRUE_MESSAGE(ok,
                "Property 4: readValues() must succeed for connected module");

            // Verify each value matches the expected MockModule value
            for (uint8_t c = 0; c < numControles; c++) {
                TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                    exp->valores[c], readBuf[c],
                    "Property 4: readValues() must return exact same value as MockModule");
            }
        }
    }
}

// ============================================================
// Property 5: Resiliência — últimos valores mantidos após falha
// ============================================================

/**
 * **Validates: Requirements 3.5, 5.4, 10.1**
 *
 * For any module with known values, if a successful read is followed
 * by a communication failure (mock configured to not respond), the
 * values from the last successful read must be preserved and
 * accessible via the module's descriptor.
 */
void test_property5_resilience_last_values() {
    lcg_seed(99u);

    MockI2CBus bus;
    I2CScanner scanner(&bus);

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        bus.clearModules();

        // Generate a single module with random controls and values
        uint8_t address = static_cast<uint8_t>(I2CProtocol::ADDR_MIN + (lcg_next() % 8));
        // Skip OLED address
        if (address == I2CProtocol::ADDR_OLED) {
            address = I2CProtocol::ADDR_MIN;
        }

        MockModule mod;
        rand_mock_module(mod, address);
        bus.addModule(mod);

        // Scan to discover the module
        uint8_t found = scanner.scan();
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            1, found,
            "Property 5: scan() must discover the registered module");

        // Store the expected values before the successful read
        uint8_t expectedValues[16];
        uint8_t numControles = mod.numControles;
        for (uint8_t c = 0; c < numControles; c++) {
            expectedValues[c] = mod.valores[c];
        }

        // Perform a successful readValues()
        uint8_t readBuf[16];
        memset(readBuf, 0xFF, sizeof(readBuf));
        bool ok = scanner.readValues(0, readBuf, numControles);
        TEST_ASSERT_TRUE_MESSAGE(ok,
            "Property 5: first readValues() must succeed for connected module");

        // Verify the successful read returned correct values
        for (uint8_t c = 0; c < numControles; c++) {
            TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                expectedValues[c], readBuf[c],
                "Property 5: successful readValues() must return correct values");
        }

        // Now simulate disconnection
        bus.setModuleConnected(address, false);

        // Attempt to read values — should fail
        uint8_t failBuf[16];
        memset(failBuf, 0xFF, sizeof(failBuf));
        bool failResult = scanner.readValues(0, failBuf, numControles);
        TEST_ASSERT_FALSE_MESSAGE(failResult,
            "Property 5: readValues() must fail after module disconnection");

        // Verify that the last known values are preserved in the descriptor
        const ModuleInfo* info = scanner.getModule(0);
        TEST_ASSERT_NOT_NULL_MESSAGE(info,
            "Property 5: getModule() must return non-null after failed read");

        for (uint8_t c = 0; c < numControles; c++) {
            TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                expectedValues[c],
                info->descriptor.controles[c].valor,
                "Property 5: descriptor must preserve last known values after disconnection");
        }
    }
}

// ============================================================
// Property 11: Desconexão após 3 falhas consecutivas
// ============================================================

/**
 * **Validates: Requirements 10.2**
 *
 * For any connected module, if exactly 3 consecutive readValues()
 * calls fail, the module must be marked as disconnected. With only
 * 2 consecutive failures, the module must remain connected.
 *
 * The test generates random modules, performs a successful read,
 * then simulates disconnection and verifies the exact threshold
 * behavior of MAX_FAIL_COUNT = 3.
 */
void test_property11_disconnect_after_3_failures() {
    lcg_seed(311u);

    MockI2CBus bus;
    I2CScanner scanner(&bus);

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        bus.clearModules();

        // Generate a single module with random controls and values
        uint8_t address = static_cast<uint8_t>(I2CProtocol::ADDR_MIN + (lcg_next() % 8));
        // Skip OLED address
        if (address == I2CProtocol::ADDR_OLED) {
            address = I2CProtocol::ADDR_MIN;
        }

        MockModule mod;
        rand_mock_module(mod, address);
        bus.addModule(mod);

        // Scan to discover the module
        uint8_t found = scanner.scan();
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            1, found,
            "Property 11: scan() must discover the registered module");

        // Perform a successful readValues() to confirm module works
        uint8_t numControles = mod.numControles;
        uint8_t readBuf[16];
        memset(readBuf, 0xFF, sizeof(readBuf));
        bool ok = scanner.readValues(0, readBuf, numControles);
        TEST_ASSERT_TRUE_MESSAGE(ok,
            "Property 11: initial readValues() must succeed for connected module");

        // Verify module is connected after successful read
        const ModuleInfo* info = scanner.getModule(0);
        TEST_ASSERT_NOT_NULL_MESSAGE(info,
            "Property 11: getModule() must return non-null after successful read");
        TEST_ASSERT_TRUE_MESSAGE(info->connected,
            "Property 11: module must be connected after successful read");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, info->failCount,
            "Property 11: failCount must be 0 after successful read");

        // Simulate disconnection — module stops responding
        bus.setModuleConnected(address, false);

        // Failure 1: readValues() should fail, but module stays connected
        memset(readBuf, 0xFF, sizeof(readBuf));
        bool fail1 = scanner.readValues(0, readBuf, numControles);
        TEST_ASSERT_FALSE_MESSAGE(fail1,
            "Property 11: readValues() must fail after disconnection (failure 1)");

        info = scanner.getModule(0);
        TEST_ASSERT_TRUE_MESSAGE(info->connected,
            "Property 11: module must remain connected after 1 failure");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, info->failCount,
            "Property 11: failCount must be 1 after first failure");

        // Failure 2: readValues() should fail, but module stays connected
        memset(readBuf, 0xFF, sizeof(readBuf));
        bool fail2 = scanner.readValues(0, readBuf, numControles);
        TEST_ASSERT_FALSE_MESSAGE(fail2,
            "Property 11: readValues() must fail after disconnection (failure 2)");

        info = scanner.getModule(0);
        TEST_ASSERT_TRUE_MESSAGE(info->connected,
            "Property 11: module must remain connected after 2 failures");
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(2, info->failCount,
            "Property 11: failCount must be 2 after second failure");

        // Failure 3: readValues() should fail AND module becomes disconnected
        memset(readBuf, 0xFF, sizeof(readBuf));
        bool fail3 = scanner.readValues(0, readBuf, numControles);
        TEST_ASSERT_FALSE_MESSAGE(fail3,
            "Property 11: readValues() must fail after disconnection (failure 3)");

        info = scanner.getModule(0);
        TEST_ASSERT_FALSE_MESSAGE(info->connected,
            "Property 11: module must be disconnected after exactly 3 failures");
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property2_scanner_registers_valid_modules);
    RUN_TEST(test_property4_value_reading_preserves_data);
    RUN_TEST(test_property5_resilience_last_values);
    RUN_TEST(test_property11_disconnect_after_3_failures);

    return UNITY_END();
}
