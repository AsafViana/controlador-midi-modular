/**
 * Property-Based Tests: UnifiedControlList
 *
 * Feature: i2c-modular-expansion, Property 1: Modo standalone preserva comportamento local
 * Feature: i2c-modular-expansion, Property 6: Ordenação da lista unificada — locais primeiro, remotos depois
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include <cstring>
#include <cstdio>
#include "hardware/UnifiedControlList.h"
#include "hardware/HardwareMap.h"
#include "MockI2CBus.h"
#include "i2c/I2CScanner.h"

// --- Simple LCG pseudo-random number generator ---
static uint32_t lcg_state = 12345u;

static void lcg_seed(uint32_t seed) {
    lcg_state = seed;
}

static uint32_t lcg_next() {
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state;
}

// ============================================================
// Property 1: Modo standalone preserva comportamento local
// ============================================================

static const int PBT_ITERATIONS = 100;

/**
 * Feature: i2c-modular-expansion, Property 1: Modo standalone preserva comportamento local
 *
 * **Validates: Requirements 1.1, 1.4, 4.5**
 *
 * Without external modules (scanner = nullptr), the UnifiedControlList
 * must contain only local controls from HardwareMap. For each random
 * valid local index:
 *   - getLabel() must return the same pointer as HardwareMap::CONTROLES[idx].label
 *   - getTipo() must return the same value as HardwareMap::CONTROLES[idx].tipo
 *   - getCCPadrao() must return the same value as HardwareMap::CONTROLES[idx].ccPadrao
 *   - isRemoto() must return false
 *
 * Also verifies that getNumControles() == HardwareMap::NUM_CONTROLES
 * and getNumLocais() == HardwareMap::NUM_CONTROLES.
 */
void test_property1_standalone_preserves_local() {
    lcg_seed(42u);

    // Construct UnifiedControlList with no scanner (standalone mode)
    UnifiedControlList ucl(nullptr);
    ucl.rebuild();

    // Verify total count matches HardwareMap
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        HardwareMap::NUM_CONTROLES, ucl.getNumControles(),
        "Standalone: numControles must equal HardwareMap::NUM_CONTROLES");

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
        HardwareMap::NUM_CONTROLES, ucl.getNumLocais(),
        "Standalone: numLocais must equal HardwareMap::NUM_CONTROLES");

    // PBT: 100 iterations picking random valid local indices
    for (int i = 0; i < PBT_ITERATIONS; i++) {
        uint8_t idx = static_cast<uint8_t>(lcg_next() % HardwareMap::NUM_CONTROLES);

        // Verify label matches HardwareMap
        const char* uclLabel = ucl.getLabel(idx);
        const char* hwLabel = HardwareMap::CONTROLES[idx].label;
        TEST_ASSERT_EQUAL_STRING_MESSAGE(
            hwLabel, uclLabel,
            "Standalone: label must match HardwareMap");

        // Verify tipo matches HardwareMap
        TipoControle uclTipo = ucl.getTipo(idx);
        TipoControle hwTipo = HardwareMap::CONTROLES[idx].tipo;
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            static_cast<uint8_t>(hwTipo),
            static_cast<uint8_t>(uclTipo),
            "Standalone: tipo must match HardwareMap");

        // Verify ccPadrao matches HardwareMap
        uint8_t uclCC = ucl.getCCPadrao(idx);
        uint8_t hwCC = HardwareMap::CONTROLES[idx].ccPadrao;
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            hwCC, uclCC,
            "Standalone: ccPadrao must match HardwareMap");

        // Verify isRemoto returns false for local controls
        TEST_ASSERT_FALSE_MESSAGE(
            ucl.isRemoto(idx),
            "Standalone: isRemoto must be false for local controls");
    }
}

// ============================================================
// Property 6: Ordenação da lista unificada — locais primeiro, remotos depois
// ============================================================

/**
 * Feature: i2c-modular-expansion, Property 6: Ordenação da lista unificada — locais primeiro, remotos depois
 *
 * **Validates: Requirements 4.1**
 *
 * For any random configuration of external modules, after rebuild(),
 * all indices from 0 to numLocais-1 must have isRemoto() == false,
 * and all indices from numLocais to numControles-1 must have
 * isRemoto() == true.
 */
void test_property6_ordering_locals_first() {
    lcg_seed(9876u);

    const uint8_t validAddresses[] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
    const uint8_t validTipos[] = {0, 1, 2, 3}; // BOTAO, POT, SENSOR, ENCODER

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        MockI2CBus bus;
        bus.begin();

        // Generate random number of modules (1-4 to stay within limits)
        uint8_t numModules = static_cast<uint8_t>(1 + (lcg_next() % 4));

        // Track which addresses are used to avoid duplicates
        bool addrUsed[8] = {false};
        uint8_t totalRemoteControls = 0;

        for (uint8_t m = 0; m < numModules; m++) {
            // Pick a unique address
            uint8_t addrIdx;
            do {
                addrIdx = static_cast<uint8_t>(lcg_next() % 8);
            } while (addrUsed[addrIdx]);
            addrUsed[addrIdx] = true;

            // Random number of controls (1-4 to keep total manageable)
            uint8_t numCtrl = static_cast<uint8_t>(1 + (lcg_next() % 4));

            // Cap total remote controls so we don't exceed MAX_TOTAL_CONTROLS
            if (totalRemoteControls + numCtrl + HardwareMap::NUM_CONTROLES > UnifiedControlList::MAX_TOTAL_CONTROLS) {
                numCtrl = UnifiedControlList::MAX_TOTAL_CONTROLS - HardwareMap::NUM_CONTROLES - totalRemoteControls;
                if (numCtrl == 0) break;
            }

            MockModule mod;
            memset(&mod, 0, sizeof(mod));
            mod.address = validAddresses[addrIdx];
            mod.numControles = numCtrl;
            mod.respondePing = true;
            mod.respondeDescritor = true;

            for (uint8_t c = 0; c < numCtrl; c++) {
                mod.tipos[c] = validTipos[lcg_next() % 4];
                // Generate a simple label
                char lbl[13];
                snprintf(lbl, sizeof(lbl), "R%02X_%u", mod.address, c);
                memcpy(mod.labels[c], lbl, 13);
                mod.valores[c] = static_cast<uint8_t>(lcg_next() % 128);
            }

            bus.addModule(mod);
            totalRemoteControls += numCtrl;
        }

        // Create scanner, scan, build unified list
        I2CScanner scanner(&bus);
        scanner.scan();

        UnifiedControlList ucl(&scanner);
        ucl.rebuild();

        uint8_t numLocais = ucl.getNumLocais();
        uint8_t numTotal = ucl.getNumControles();

        // numLocais must equal HardwareMap::NUM_CONTROLES
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            HardwareMap::NUM_CONTROLES, numLocais,
            "Property 6: numLocais must equal HardwareMap::NUM_CONTROLES");

        // Verify indices 0..numLocais-1 are local (isRemoto == false)
        for (uint8_t i = 0; i < numLocais; i++) {
            TEST_ASSERT_FALSE_MESSAGE(
                ucl.isRemoto(i),
                "Property 6: indices 0..numLocais-1 must be local (isRemoto==false)");
        }

        // Verify indices numLocais..numTotal-1 are remote (isRemoto == true)
        for (uint8_t i = numLocais; i < numTotal; i++) {
            TEST_ASSERT_TRUE_MESSAGE(
                ucl.isRemoto(i),
                "Property 6: indices numLocais..numTotal-1 must be remote (isRemoto==true)");
        }

        // Verify total count is consistent
        TEST_ASSERT_TRUE_MESSAGE(
            numTotal >= numLocais,
            "Property 6: numTotal must be >= numLocais");
    }
}

// ============================================================
// Property 7: Dados de controles remotos correspondem ao descritor
// ============================================================

/**
 * Feature: i2c-modular-expansion, Property 7: Dados de controles remotos correspondem ao descritor
 *
 * **Validates: Requirements 4.3, 4.4, 4.6**
 *
 * For any valid ModuleDescriptor registered via MockI2CBus, after scan
 * and rebuild of UnifiedControlList, each remote control in the unified
 * list must have:
 *   - getLabel() matching the label from the MockModule's descriptor
 *   - getTipo() matching the tipo from the MockModule's descriptor
 *   - getCCPadrao() matching the expected default CC (rc.valor from descriptor)
 *   - getRemoteInfo() returning the correct moduleAddress and moduleCtrlIdx
 */
void test_property7_remote_data_matches_descriptor() {
    lcg_seed(55555u);

    const uint8_t validAddresses[] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
    const uint8_t validTipos[] = {0, 1, 2, 3}; // BOTAO, POT, SENSOR, ENCODER

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        MockI2CBus bus;
        bus.begin();

        // Generate random number of modules (1-4)
        uint8_t numModules = static_cast<uint8_t>(1 + (lcg_next() % 4));

        // Track used addresses and store module configs for verification
        bool addrUsed[8] = {false};
        uint8_t totalRemoteControls = 0;

        // Store expected data for verification after rebuild
        struct ExpectedRemote {
            uint8_t moduleAddress;
            uint8_t ctrlIdx;
            char label[13];
            uint8_t tipo;
            uint8_t valor; // ccPadrao = valor from descriptor
        };
        ExpectedRemote expected[32];
        uint8_t expectedCount = 0;

        for (uint8_t m = 0; m < numModules; m++) {
            // Pick a unique address
            uint8_t addrIdx;
            do {
                addrIdx = static_cast<uint8_t>(lcg_next() % 8);
            } while (addrUsed[addrIdx]);
            addrUsed[addrIdx] = true;

            // Random number of controls (1-4)
            uint8_t numCtrl = static_cast<uint8_t>(1 + (lcg_next() % 4));

            // Cap total remote controls
            if (totalRemoteControls + numCtrl + HardwareMap::NUM_CONTROLES > UnifiedControlList::MAX_TOTAL_CONTROLS) {
                numCtrl = UnifiedControlList::MAX_TOTAL_CONTROLS - HardwareMap::NUM_CONTROLES - totalRemoteControls;
                if (numCtrl == 0) break;
            }

            MockModule mod;
            memset(&mod, 0, sizeof(mod));
            mod.address = validAddresses[addrIdx];
            mod.numControles = numCtrl;
            mod.respondePing = true;
            mod.respondeDescritor = true;

            for (uint8_t c = 0; c < numCtrl; c++) {
                uint8_t tipo = validTipos[lcg_next() % 4];
                mod.tipos[c] = tipo;

                // Generate random ASCII label (3-10 chars)
                uint8_t labelLen = static_cast<uint8_t>(3 + (lcg_next() % 8));
                if (labelLen > 12) labelLen = 12;
                for (uint8_t ch = 0; ch < labelLen; ch++) {
                    // printable ASCII: 'A'-'Z' range
                    mod.labels[c][ch] = static_cast<char>('A' + (lcg_next() % 26));
                }
                mod.labels[c][labelLen] = '\0';

                // Random value 0-127 (this becomes ccPadrao)
                uint8_t valor = static_cast<uint8_t>(lcg_next() % 128);
                mod.valores[c] = valor;

                // Store expected data
                expected[expectedCount].moduleAddress = mod.address;
                expected[expectedCount].ctrlIdx = c;
                memcpy(expected[expectedCount].label, mod.labels[c], 13);
                expected[expectedCount].tipo = tipo;
                expected[expectedCount].valor = valor;
                expectedCount++;
            }

            bus.addModule(mod);
            totalRemoteControls += numCtrl;
        }

        // Create scanner, scan, build unified list
        I2CScanner scanner(&bus);
        scanner.scan();

        UnifiedControlList ucl(&scanner);
        ucl.rebuild();

        uint8_t numLocais = ucl.getNumLocais();
        uint8_t numTotal = ucl.getNumControles();
        uint8_t numRemotes = numTotal - numLocais;

        // The scanner discovers modules in address order (0x20..0x27),
        // so we need to match remote controls in the order they appear
        // in the unified list. We verify each remote control individually.
        uint8_t remoteIdx = 0;
        for (uint8_t i = numLocais; i < numTotal; i++) {
            ControlInfo info;
            bool ok = ucl.getControlInfo(i, info);
            TEST_ASSERT_TRUE_MESSAGE(ok,
                "Property 7: getControlInfo must succeed for valid remote index");

            // Find the matching expected entry by moduleAddress + ctrlIdx
            uint8_t addr = 0;
            uint8_t cIdx = 0;
            bool gotRemote = ucl.getRemoteInfo(i, addr, cIdx);
            TEST_ASSERT_TRUE_MESSAGE(gotRemote,
                "Property 7: getRemoteInfo must succeed for remote control");

            // Find matching expected entry
            bool found = false;
            for (uint8_t e = 0; e < expectedCount; e++) {
                if (expected[e].moduleAddress == addr && expected[e].ctrlIdx == cIdx) {
                    // Verify label
                    TEST_ASSERT_EQUAL_STRING_MESSAGE(
                        expected[e].label, ucl.getLabel(i),
                        "Property 7: label must match descriptor");

                    // Verify tipo
                    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                        expected[e].tipo,
                        static_cast<uint8_t>(ucl.getTipo(i)),
                        "Property 7: tipo must match descriptor");

                    // Verify ccPadrao (= valor from descriptor)
                    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                        expected[e].valor,
                        ucl.getCCPadrao(i),
                        "Property 7: ccPadrao must match descriptor valor");

                    // Verify moduleAddress from getRemoteInfo
                    TEST_ASSERT_EQUAL_HEX8_MESSAGE(
                        expected[e].moduleAddress, addr,
                        "Property 7: moduleAddress must match");

                    // Verify moduleCtrlIdx from getRemoteInfo
                    TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                        expected[e].ctrlIdx, cIdx,
                        "Property 7: moduleCtrlIdx must match");

                    found = true;
                    break;
                }
            }
            TEST_ASSERT_TRUE_MESSAGE(found,
                "Property 7: every remote control must match an expected entry");

            remoteIdx++;
        }

        // Verify we found all expected remote controls
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            expectedCount, remoteIdx,
            "Property 7: number of remote controls must match expected count");
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property1_standalone_preserves_local);
    RUN_TEST(test_property6_ordering_locals_first);
    RUN_TEST(test_property7_remote_data_matches_descriptor);

    return UNITY_END();
}
