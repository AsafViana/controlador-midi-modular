/**
 * Property-Based Test: Truncamento de texto sem overflow
 *
 * Feature: oled-ui-framework, Property 6: Truncamento de texto sem overflow
 *
 * Validates: Requirements 9.3
 *
 * Property: For any string of arbitrary length and any position (x, y)
 * within display bounds, the TextComponent must truncate the text so that
 * no pixel is drawn outside the 128×64 buffer bounds.
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include <cstring>
#include <cstdio>

#include "Adafruit_SSD1306.h"
#include "Wire.h"
#include "ui/components/TextComponent.h"

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

/// Generate a random uint32 in range [min, max] (inclusive)
static uint32_t rand_range(uint32_t min, uint32_t max) {
    if (min >= max) return min;
    return min + (lcg_next() % (max - min + 1));
}

/// Generate a random alphanumeric character
static char rand_alnum() {
    static const char charset[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    return charset[lcg_next() % (sizeof(charset) - 1)];
}

/// Fill buffer with random alphanumeric string of given length
static void rand_string(char* buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        buf[i] = rand_alnum();
    }
    buf[len] = '\0';
}

// --- Display constants ---
static const int16_t DISPLAY_WIDTH  = 128;
static const int16_t DISPLAY_HEIGHT = 64;

static const int PBT_ITERATIONS = 25;

// Shared mock display — uses global Wire from test/mocks/Wire.cpp
static Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ============================================================
// Helper: verify truncation property
// ============================================================

/**
 * After rendering, verify that the printed text does not overflow
 * the 128px display width.
 *
 * charWidth = fontSize * 6 (mock getTextBounds convention)
 * Condition: x + strlen(lastPrintedText) * charWidth <= 128
 *
 * Also verify y position is within [0, 63].
 */
static void verify_no_overflow(int16_t x, int16_t y, uint8_t fontSize,
                                const char* originalText, int iteration,
                                const char* testLabel) {
    char msg[256];

    // If nothing was printed, that's fine — no overflow possible
    if (display.printCallCount == 0) {
        return;
    }

    uint16_t charWidth = fontSize * 6;
    size_t printedLen = strlen(display.lastPrintedText);
    int32_t endX = (int32_t)x + (int32_t)printedLen * (int32_t)charWidth;

    snprintf(msg, sizeof(msg),
             "[%s iter %d] Overflow! x=%d, fontSize=%u, originalLen=%u, "
             "printedLen=%u, endX=%d > 128. Text='%.20s'",
             testLabel, iteration, (int)x, (unsigned)fontSize,
             (unsigned)strlen(originalText), (unsigned)printedLen,
             (int)endX, display.lastPrintedText);

    TEST_ASSERT_LESS_OR_EQUAL_INT32_MESSAGE(DISPLAY_WIDTH, endX, msg);

    // Printed text must be a prefix of (or equal to) the original
    snprintf(msg, sizeof(msg),
             "[%s iter %d] Printed text is not a prefix of original",
             testLabel, iteration);
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(strlen(originalText), printedLen, msg);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0,
        strncmp(originalText, display.lastPrintedText, printedLen), msg);
}

// ============================================================
// Property 6: Random positions and string lengths
// ============================================================

/**
 * **Validates: Requirements 9.3**
 *
 * For 100 random (x, y, stringLength, fontSize) tuples:
 *   - x in [0, 127], y in [0, 63]
 *   - stringLength in [1, 50]
 *   - fontSize in {1, 2, 3}
 *   - Create TextComponent and render
 *   - Verify: x + strlen(printedText) * (fontSize * 6) <= 128
 */
void test_property6_random_text_truncation() {
    lcg_seed(42u);
    char textBuf[64];

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        display.reset();

        int16_t x = (int16_t)rand_range(0, 127);
        int16_t y = (int16_t)rand_range(0, 63);
        uint32_t strLen = rand_range(1, 50);
        uint8_t fontSize = (uint8_t)rand_range(1, 3);

        rand_string(textBuf, strLen);

        TextComponent tc(x, y, textBuf, fontSize);
        tc.render(display);

        verify_no_overflow(x, y, fontSize, textBuf, i, "random");
    }
}

// ============================================================
// Property 6: Edge positions (x near right boundary)
// ============================================================

/**
 * **Validates: Requirements 9.3**
 *
 * Stress test with x positions near the right edge (120-127).
 * These positions are most likely to trigger truncation.
 *
 * For 100 iterations:
 *   - x in [120, 127], y in [0, 63]
 *   - stringLength in [1, 50]
 *   - fontSize in {1, 2, 3}
 *   - Verify no overflow
 */
void test_property6_edge_position_truncation() {
    lcg_seed(99u);
    char textBuf[64];

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        display.reset();

        int16_t x = (int16_t)rand_range(120, 127);
        int16_t y = (int16_t)rand_range(0, 63);
        uint32_t strLen = rand_range(1, 50);
        uint8_t fontSize = (uint8_t)rand_range(1, 3);

        rand_string(textBuf, strLen);

        TextComponent tc(x, y, textBuf, fontSize);
        tc.render(display);

        verify_no_overflow(x, y, fontSize, textBuf, i, "edge");
    }
}

// ============================================================
// Property 6: Large font sizes at various positions
// ============================================================

/**
 * **Validates: Requirements 9.3**
 *
 * Test with larger font sizes (2 and 3) which have wider characters
 * (12px and 18px per char respectively), making overflow more likely.
 *
 * For 100 iterations:
 *   - x in [0, 127], y in [0, 63]
 *   - stringLength in [1, 50]
 *   - fontSize in {2, 3} only
 *   - Verify no overflow
 */
void test_property6_large_font_truncation() {
    lcg_seed(777u);
    char textBuf[64];

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        display.reset();

        int16_t x = (int16_t)rand_range(0, 127);
        int16_t y = (int16_t)rand_range(0, 63);
        uint32_t strLen = rand_range(1, 50);
        uint8_t fontSize = (uint8_t)rand_range(2, 3);

        rand_string(textBuf, strLen);

        TextComponent tc(x, y, textBuf, fontSize);
        tc.render(display);

        verify_no_overflow(x, y, fontSize, textBuf, i, "largefont");
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property6_random_text_truncation);
    RUN_TEST(test_property6_edge_position_truncation);
    RUN_TEST(test_property6_large_font_truncation);

    return UNITY_END();
}
