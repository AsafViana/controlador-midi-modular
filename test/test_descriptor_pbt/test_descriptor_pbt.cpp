/**
 * Property-Based Test: ModuleDescriptor Serialization
 *
 * Feature: i2c-modular-expansion, Property 3: Round-trip de serialização do ModuleDescriptor
 *
 * Validates: Requirements 3.2, 11.2, 11.4
 *
 * Property 3: For any valid ModuleDescriptor (numControles 1-16, known
 * TipoControle values, ASCII labels up to 12 chars, valores 0-127),
 * serializing and then deserializing must produce an equivalent
 * ModuleDescriptor — same numControles, same tipos, same labels,
 * and same valores for each control.
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include <cstring>
#include "i2c/ModuleDescriptor.h"

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

// --- Random generators for ModuleDescriptor fields ---

/// Generate a random numControles in [1, 16]
static uint8_t rand_numControles() {
    return static_cast<uint8_t>((lcg_next() % 16) + 1);
}

/// Generate a random valid TipoControle (0-3)
static TipoControle rand_tipo() {
    return static_cast<TipoControle>(lcg_next() % 4);
}

/// Generate a random valor in [0, 127]
static uint8_t rand_valor() {
    return static_cast<uint8_t>(lcg_next() % 128);
}

/// Generate a random ASCII label of length [0, 12]
/// Uses printable ASCII range (32-126)
static void rand_label(char* label) {
    uint8_t len = static_cast<uint8_t>(lcg_next() % (I2CProtocol::LABEL_MAX_LEN + 1));
    for (uint8_t j = 0; j < len; j++) {
        label[j] = static_cast<char>(32 + (lcg_next() % 95)); // printable ASCII
    }
    label[len] = '\0';
    // Null-pad the rest
    for (uint8_t j = len + 1; j <= I2CProtocol::LABEL_MAX_LEN; j++) {
        label[j] = '\0';
    }
}

/// Generate a random valid ModuleDescriptor
static void rand_descriptor(ModuleDescriptor& desc) {
    desc.numControles = rand_numControles();
    for (uint8_t i = 0; i < desc.numControles; i++) {
        desc.controles[i].tipo = rand_tipo();
        rand_label(desc.controles[i].label);
        desc.controles[i].valor = rand_valor();
    }
}

// ============================================================
// Property 3: Round-trip de serialização do ModuleDescriptor
// ============================================================

static const int PBT_ITERATIONS = 100;

/**
 * **Validates: Requirements 3.2, 11.2, 11.4**
 *
 * For 100 random valid ModuleDescriptors:
 *   - serialize() produces non-zero bytes
 *   - deserialize() of those bytes returns true
 *   - The deserialized descriptor matches the original:
 *     same numControles, same tipo, same label, same valor
 */
void test_property3_descriptor_roundtrip() {
    lcg_seed(42u);

    // Buffer large enough for max descriptor: 1 + 16*14 = 225 bytes
    uint8_t buffer[256];

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        // Generate random valid descriptor
        ModuleDescriptor original;
        memset(&original, 0, sizeof(original));
        rand_descriptor(original);

        // Serialize
        uint16_t written = I2CProtocol::serialize(original, buffer, sizeof(buffer));
        TEST_ASSERT_GREATER_THAN_UINT16_MESSAGE(
            0, written,
            "serialize() must return non-zero for valid descriptor");

        // Verify written size matches expected
        uint16_t expectedSize = 1 + static_cast<uint16_t>(original.numControles) * I2CProtocol::BYTES_POR_CONTROLE;
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(
            expectedSize, written,
            "serialize() must write exactly the expected number of bytes");

        // Deserialize
        ModuleDescriptor restored;
        memset(&restored, 0, sizeof(restored));
        bool ok = I2CProtocol::deserialize(buffer, written, restored);
        TEST_ASSERT_TRUE_MESSAGE(ok,
            "deserialize() must succeed for valid serialized data");

        // Compare numControles
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(
            original.numControles, restored.numControles,
            "numControles must match after round-trip");

        // Compare each control
        for (uint8_t c = 0; c < original.numControles; c++) {
            // Compare tipo
            TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                static_cast<uint8_t>(original.controles[c].tipo),
                static_cast<uint8_t>(restored.controles[c].tipo),
                "tipo must match after round-trip");

            // Compare label (first 12 bytes)
            TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(
                original.controles[c].label,
                restored.controles[c].label,
                I2CProtocol::LABEL_MAX_LEN,
                "label must match after round-trip");

            // Compare valor
            TEST_ASSERT_EQUAL_UINT8_MESSAGE(
                original.controles[c].valor,
                restored.controles[c].valor,
                "valor must match after round-trip");
        }
    }
}

// ============================================================
// Property 12: Desserialização rejeita dados inválidos
// ============================================================

/**
 * Feature: i2c-modular-expansion, Property 12: Desserialização rejeita dados inválidos
 *
 * **Validates: Requirements 11.3, 11.5**
 *
 * For 100 random iterations, generate buffers with invalid data:
 *   - Case A: numControles = 0 → deserialize() returns false
 *   - Case B: numControles > 16 → deserialize() returns false
 *   - Case C: valid numControles but at least one invalid tipo (>3) → deserialize() returns false
 *
 * In all cases, deserialize() must return false.
 */
void test_property12_deserialize_rejects_invalid() {
    lcg_seed(9999u);

    // Buffer large enough for max descriptor
    uint8_t buffer[256];

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        // Decide which invalid case to generate (rotate through 3 cases)
        uint8_t caseType = static_cast<uint8_t>(i % 3);

        ModuleDescriptor result;
        memset(&result, 0, sizeof(result));
        bool ok;

        if (caseType == 0) {
            // Case A: numControles = 0
            // Build a buffer with numControles = 0 and some trailing data
            uint8_t numCtrl = 0;
            buffer[0] = numCtrl;
            // Fill some trailing bytes to ensure buffer has content
            uint16_t bufLen = 1 + 14; // at least one control's worth of data
            for (uint16_t b = 1; b < bufLen; b++) {
                buffer[b] = static_cast<uint8_t>(lcg_next() % 256);
            }

            ok = I2CProtocol::deserialize(buffer, bufLen, result);
            TEST_ASSERT_FALSE_MESSAGE(ok,
                "deserialize() must reject numControles=0");

        } else if (caseType == 1) {
            // Case B: numControles > 16
            // Generate a random value in [17, 255]
            uint8_t numCtrl = static_cast<uint8_t>(17 + (lcg_next() % 239));
            buffer[0] = numCtrl;

            // Provide enough buffer data for the claimed controls (up to buffer size)
            uint16_t claimed = 1 + static_cast<uint16_t>(numCtrl) * I2CProtocol::BYTES_POR_CONTROLE;
            uint16_t bufLen = (claimed < sizeof(buffer)) ? claimed : sizeof(buffer);
            for (uint16_t b = 1; b < bufLen; b++) {
                buffer[b] = static_cast<uint8_t>(lcg_next() % 256);
            }

            ok = I2CProtocol::deserialize(buffer, bufLen, result);
            TEST_ASSERT_FALSE_MESSAGE(ok,
                "deserialize() must reject numControles>16");

        } else {
            // Case C: valid numControles (1-16) but at least one invalid tipo
            uint8_t numCtrl = static_cast<uint8_t>((lcg_next() % 16) + 1);

            // Build a valid-looking descriptor first
            ModuleDescriptor temp;
            memset(&temp, 0, sizeof(temp));
            temp.numControles = numCtrl;
            for (uint8_t c = 0; c < numCtrl; c++) {
                temp.controles[c].tipo = rand_tipo(); // valid tipo
                rand_label(temp.controles[c].label);
                temp.controles[c].valor = rand_valor();
            }

            // Serialize it to get a well-formed buffer
            uint16_t written = I2CProtocol::serialize(temp, buffer, sizeof(buffer));
            TEST_ASSERT_GREATER_THAN_UINT16(0, written);

            // Now corrupt one or more tipo bytes to an invalid value (>3)
            // Pick a random control index to corrupt
            uint8_t corruptIdx = static_cast<uint8_t>(lcg_next() % numCtrl);
            // Tipo byte offset: 1 (numControles) + corruptIdx * 14 + 0 (tipo is first byte of each control)
            uint16_t tipoOffset = 1 + static_cast<uint16_t>(corruptIdx) * I2CProtocol::BYTES_POR_CONTROLE;
            // Set to an invalid tipo value in [4, 255]
            buffer[tipoOffset] = static_cast<uint8_t>(4 + (lcg_next() % 252));

            ok = I2CProtocol::deserialize(buffer, written, result);
            TEST_ASSERT_FALSE_MESSAGE(ok,
                "deserialize() must reject buffer with invalid tipo");
        }
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property3_descriptor_roundtrip);
    RUN_TEST(test_property12_deserialize_rejects_invalid);

    return UNITY_END();
}
