/**
 * Property-Based Test: Dead Zone Applied Uniformly to Remote Controls
 *
 * Feature: i2c-modular-expansion, Property 8: Zona morta aplicada uniformemente a controles remotos
 *
 * Validates: Requirements 5.2, 5.5
 *
 * Property 8: For any two consecutive values (v1, v2) of a remote control,
 * a MIDI CC message must be sent if and only if |v2 - v1| > ZONA_MORTA.
 * Values within the dead zone must not generate a send.
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include <cstring>
#include <cstdio>
#include "Arduino.h"
#include "Control_Surface.h"
#include "MockI2CBus.h"
#include "i2c/I2CScanner.h"
#include "hardware/UnifiedControlList.h"
#include "hardware/ControlReader.h"
#include "midi/MidiEngine.h"
#include "storage/Storage.h"

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

/// Generate a random value in [0, 127]
static uint8_t rand_valor() {
    return static_cast<uint8_t>(lcg_next() % 128);
}

// ============================================================
// Property 8: Zona morta aplicada uniformemente a controles remotos
// ============================================================

static const int PBT_ITERATIONS = 100;

/**
 * **Validates: Requirements 5.2, 5.5**
 *
 * For 100 random pairs of consecutive values (v1, v2):
 *   - Set up a remote POTENCIOMETRO control on a mock I2C module
 *   - Set value to v1, call update() (first call always sends CC)
 *   - Set value to v2, call update()
 *   - Verify: CC is sent iff |v2 - v1| > ZONA_MORTA
 */
void test_property8_dead_zone_uniform() {
    lcg_seed(42u);

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        // Generate random pair of values
        uint8_t v1 = rand_valor();
        uint8_t v2 = rand_valor();

        // Reset all mocks
        mock::reset();
        mock_midi::reset();

        // Set up MockI2CBus with one module at 0x20 with 1 POTENCIOMETRO
        MockI2CBus bus;
        bus.begin();

        MockModule mod;
        memset(&mod, 0, sizeof(mod));
        mod.address = 0x20;
        mod.numControles = 1;
        mod.tipos[0] = static_cast<uint8_t>(TipoControle::POTENCIOMETRO);
        strncpy(mod.labels[0], "RemotePot", 12);
        mod.labels[0][12] = '\0';
        mod.valores[0] = v1;
        mod.respondePing = true;
        mod.respondeDescritor = true;
        bus.addModule(mod);

        // Create scanner and scan
        I2CScanner scanner(&bus);
        scanner.scan();

        // Create unified control list and rebuild
        UnifiedControlList ucl(&scanner);
        ucl.rebuild();

        // Create storage and configure remote CC
        Storage storage;
        storage.begin();
        // Set a known CC for the remote control and enable it
        storage.setRemoteCC(0x20, 0, 10);
        storage.setRemoteEnabled(0x20, 0, true);

        // Create MidiEngine
        MidiEngine engine;

        // Create ControlReader with UCL and scanner
        ControlReader reader(&engine, &storage, &ucl, &scanner);

        // --- First update with v1 ---
        // Ensure enough time has passed for the interval check
        mock::setMillis(100);
        reader.update();

        // First call always sends CC because _ultimoValorRemoto starts at 255
        // (impossible value that forces first send)
        // Record message count after first update
        int countAfterV1 = mock_midi::messageCount;

        // --- Second update with v2 ---
        // Change the mock value to v2
        bus.setControlValue(0x20, 0, v2);

        // Advance time past INTERVALO_MS (10ms)
        mock::advanceMillis(20);
        reader.update();

        int countAfterV2 = mock_midi::messageCount;
        int ccSentOnV2 = countAfterV2 - countAfterV1;

        // Compute expected behavior
        int diff = static_cast<int>(v2) - static_cast<int>(v1);
        if (diff < 0) diff = -diff;
        bool shouldSend = (diff > ControlReader::ZONA_MORTA);

        // Verify property
        if (shouldSend) {
            char msg[128];
            snprintf(msg, sizeof(msg),
                "Iter %d: v1=%d, v2=%d, diff=%d > ZONA_MORTA=%d => CC should be sent",
                i, v1, v2, diff, ControlReader::ZONA_MORTA);
            TEST_ASSERT_EQUAL_INT_MESSAGE(1, ccSentOnV2, msg);

            // Also verify the CC value matches v2
            TEST_ASSERT_TRUE_MESSAGE(mock_midi::lastMessage.isCC, msg);
            TEST_ASSERT_EQUAL_UINT8_MESSAGE(v2, mock_midi::lastMessage.ccValue, msg);
        } else {
            char msg[128];
            snprintf(msg, sizeof(msg),
                "Iter %d: v1=%d, v2=%d, diff=%d <= ZONA_MORTA=%d => CC should NOT be sent",
                i, v1, v2, diff, ControlReader::ZONA_MORTA);
            TEST_ASSERT_EQUAL_INT_MESSAGE(0, ccSentOnV2, msg);
        }
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property8_dead_zone_uniform);

    return UNITY_END();
}
