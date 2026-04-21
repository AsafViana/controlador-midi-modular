/**
 * Property-Based Test: Storage Remote Configuration Round-trip
 *
 * Feature: i2c-modular-expansion, Property 9: Round-trip de persistência de configuração remota
 *
 * Validates: Requirements 6.1, 6.2, 6.3
 *
 * Property 9: For any valid I2C address (0x20-0x27), control index (0-15),
 * CC value (0-127), and enabled state (true/false), after calling
 * setRemoteCC() and setRemoteEnabled(), the calls getRemoteCC() and
 * isRemoteEnabled() must return the same configured values.
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include <cstring>
#include "storage/Storage.h"

// --- Simple LCG pseudo-random number generator ---
// Parameters from Numerical Recipes (period 2^32)
static uint32_t lcg_state = 12345u;

static void lcg_seed(uint32_t seed) {
    lcg_state = seed;
}

static uint32_t lcg_next() {
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state;
}

// --- Random generators for Storage remote config fields ---

/// Generate a random valid I2C address in [0x20, 0x27]
static uint8_t rand_address() {
    return static_cast<uint8_t>(0x20 + (lcg_next() % 8));
}

/// Generate a random control index in [0, 15]
static uint8_t rand_ctrlIdx() {
    return static_cast<uint8_t>(lcg_next() % 16);
}

/// Generate a random CC value in [0, 127]
static uint8_t rand_cc() {
    return static_cast<uint8_t>(lcg_next() % 128);
}

/// Generate a random boolean
static bool rand_bool() {
    return (lcg_next() % 2) == 1;
}

// ============================================================
// Property 9: Round-trip de persistência de configuração remota
// ============================================================

static const int PBT_ITERATIONS = 100;

/**
 * **Validates: Requirements 6.1, 6.2, 6.3**
 *
 * For 100 random combinations of (address, ctrlIdx, cc, enabled):
 *   - setRemoteCC(addr, idx, cc) stores the CC value
 *   - setRemoteEnabled(addr, idx, enabled) stores the enabled state
 *   - getRemoteCC(addr, idx) returns the same CC value
 *   - isRemoteEnabled(addr, idx) returns the same enabled state
 */
void test_property9_storage_remote_roundtrip() {
    lcg_seed(42u);

    Storage storage;
    storage.begin();

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        // Generate random inputs
        uint8_t addr = rand_address();
        uint8_t idx = rand_ctrlIdx();
        uint8_t cc = rand_cc();
        bool enabled = rand_bool();

        // Set values
        storage.setRemoteCC(addr, idx, cc);
        storage.setRemoteEnabled(addr, idx, enabled);

        // Read back and verify
        uint8_t readCC = storage.getRemoteCC(addr, idx);
        bool readEnabled = storage.isRemoteEnabled(addr, idx);

        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            cc, readCC,
            "getRemoteCC must return the same CC value that was set");

        TEST_ASSERT_EQUAL_MESSAGE(
            enabled, readEnabled,
            "isRemoteEnabled must return the same enabled state that was set");
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property9_storage_remote_roundtrip);

    return UNITY_END();
}
