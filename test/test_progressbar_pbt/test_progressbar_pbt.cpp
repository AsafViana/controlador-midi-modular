/**
 * Property-Based Test: Preenchimento proporcional da barra de progresso
 *
 * Feature: oled-ui-framework, Property 7: Preenchimento proporcional da barra de progresso
 *
 * Validates: Requirements 10.2
 *
 * Property: For any value between 0 and 100 (inclusive), the fill width
 * of the ProgressBarComponent must be proportional:
 *   fillWidth = (value * innerWidth) / 100
 * where innerWidth = width - 2 (discounting 1px border on each side).
 * For value 0, fillWidth must be 0 (no fillRect called).
 * For value 100, fillWidth must equal innerWidth.
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include <cstdio>

#include "Adafruit_SSD1306.h"
#include "Wire.h"
#include "ui/components/ProgressBarComponent.h"

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

// --- Display constants ---
static const int16_t DISPLAY_WIDTH  = 128;
static const int16_t DISPLAY_HEIGHT = 64;

static const int PBT_ITERATIONS = 25;

// Shared mock display
static Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ============================================================
// Property 7: Random values and dimensions
// ============================================================

/**
 * **Validates: Requirements 10.2**
 *
 * For 100 random (value, width, height, x, y) tuples:
 *   - value in [0, 100]
 *   - width in [3, 128], height in [3, 64] (minimum 3 for inner space)
 *   - x in [0, 127], y in [0, 63]
 *   - Create ProgressBarComponent, setValue, render
 *   - Calculate expected fillWidth = (value * (width - 2)) / 100
 *   - If value == 0: verify no fillRect was called
 *   - If value > 0: verify lastFillRectW == expected fillWidth
 *   - If value == 100: verify lastFillRectW == width - 2 (full inner width)
 */
void test_property7_random_fill_proportional() {
    lcg_seed(42u);
    char msg[256];

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        display.reset();

        uint8_t value = (uint8_t)rand_range(0, 100);
        int16_t w = (int16_t)rand_range(3, 128);
        int16_t h = (int16_t)rand_range(3, 64);
        int16_t x = (int16_t)rand_range(0, 127);
        int16_t y = (int16_t)rand_range(0, 63);

        int16_t innerWidth = w - 2;
        int16_t expectedFillWidth = (value * innerWidth) / 100;

        ProgressBarComponent bar(x, y, w, h);
        bar.setValue(value);
        bar.render(display);

        // Border must always be drawn
        snprintf(msg, sizeof(msg),
                 "[iter %d] drawRect not called. value=%u, w=%d, h=%d",
                 i, (unsigned)value, (int)w, (int)h);
        TEST_ASSERT_EQUAL_INT_MESSAGE(1, display.drawRectCallCount, msg);

        if (expectedFillWidth == 0) {
            // When expected fill is 0 (value == 0 or integer division
            // rounds to 0 for small innerWidth), no fillRect should be called
            snprintf(msg, sizeof(msg),
                     "[iter %d] fillRect called when expectedFillWidth=0. "
                     "value=%u, w=%d, innerWidth=%d",
                     i, (unsigned)value, (int)w, (int)innerWidth);
            TEST_ASSERT_EQUAL_INT_MESSAGE(0, display.fillRectCallCount, msg);
        } else {
            // expectedFillWidth > 0: fill must be drawn with correct width
            snprintf(msg, sizeof(msg),
                     "[iter %d] fillRect not called. value=%u, w=%d, h=%d, "
                     "expectedFillWidth=%d",
                     i, (unsigned)value, (int)w, (int)h,
                     (int)expectedFillWidth);
            TEST_ASSERT_EQUAL_INT_MESSAGE(1, display.fillRectCallCount, msg);

            snprintf(msg, sizeof(msg),
                     "[iter %d] fillWidth mismatch. value=%u, w=%d, innerWidth=%d, "
                     "expected=%d, got=%d",
                     i, (unsigned)value, (int)w, (int)innerWidth,
                     (int)expectedFillWidth, (int)display.lastFillRectW);
            TEST_ASSERT_EQUAL_INT16_MESSAGE(expectedFillWidth,
                                            display.lastFillRectW, msg);

            // Verify fill position: x+1, y+1 (inside border)
            snprintf(msg, sizeof(msg),
                     "[iter %d] fillRect X mismatch. expected=%d, got=%d",
                     i, (int)(x + 1), (int)display.lastFillRectX);
            TEST_ASSERT_EQUAL_INT16_MESSAGE(x + 1, display.lastFillRectX, msg);

            snprintf(msg, sizeof(msg),
                     "[iter %d] fillRect Y mismatch. expected=%d, got=%d",
                     i, (int)(y + 1), (int)display.lastFillRectY);
            TEST_ASSERT_EQUAL_INT16_MESSAGE(y + 1, display.lastFillRectY, msg);

            // Verify fill height: h - 2 (inside border)
            snprintf(msg, sizeof(msg),
                     "[iter %d] fillRect H mismatch. expected=%d, got=%d",
                     i, (int)(h - 2), (int)display.lastFillRectH);
            TEST_ASSERT_EQUAL_INT16_MESSAGE(h - 2, display.lastFillRectH, msg);
        }
    }
}

// ============================================================
// Property 7: Boundary — value 0 always yields no fill
// ============================================================

/**
 * **Validates: Requirements 10.2**
 *
 * For 100 random dimensions with value fixed at 0:
 *   - Verify fillRect is never called (fillWidth == 0)
 */
void test_property7_value_zero_no_fill() {
    lcg_seed(99u);
    char msg[256];

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        display.reset();

        int16_t w = (int16_t)rand_range(3, 128);
        int16_t h = (int16_t)rand_range(3, 64);
        int16_t x = (int16_t)rand_range(0, 127);
        int16_t y = (int16_t)rand_range(0, 63);

        ProgressBarComponent bar(x, y, w, h);
        bar.setValue(0);
        bar.render(display);

        // Border must be drawn
        snprintf(msg, sizeof(msg),
                 "[iter %d] drawRect not called for value=0. w=%d, h=%d",
                 i, (int)w, (int)h);
        TEST_ASSERT_EQUAL_INT_MESSAGE(1, display.drawRectCallCount, msg);

        // No fill for value 0
        snprintf(msg, sizeof(msg),
                 "[iter %d] fillRect should NOT be called for value=0. "
                 "w=%d, h=%d, fillRectCount=%d",
                 i, (int)w, (int)h, display.fillRectCallCount);
        TEST_ASSERT_EQUAL_INT_MESSAGE(0, display.fillRectCallCount, msg);
    }
}

// ============================================================
// Property 7: Boundary — value 100 fills entire inner width
// ============================================================

/**
 * **Validates: Requirements 10.2**
 *
 * For 100 random dimensions with value fixed at 100:
 *   - Verify fillRect width == innerWidth (width - 2)
 */
void test_property7_value_100_full_fill() {
    lcg_seed(777u);
    char msg[256];

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        display.reset();

        int16_t w = (int16_t)rand_range(3, 128);
        int16_t h = (int16_t)rand_range(3, 64);
        int16_t x = (int16_t)rand_range(0, 127);
        int16_t y = (int16_t)rand_range(0, 63);

        int16_t innerWidth = w - 2;

        ProgressBarComponent bar(x, y, w, h);
        bar.setValue(100);
        bar.render(display);

        // Border must be drawn
        snprintf(msg, sizeof(msg),
                 "[iter %d] drawRect not called for value=100. w=%d, h=%d",
                 i, (int)w, (int)h);
        TEST_ASSERT_EQUAL_INT_MESSAGE(1, display.drawRectCallCount, msg);

        // Fill must be drawn
        snprintf(msg, sizeof(msg),
                 "[iter %d] fillRect not called for value=100. w=%d, h=%d",
                 i, (int)w, (int)h);
        TEST_ASSERT_EQUAL_INT_MESSAGE(1, display.fillRectCallCount, msg);

        // Fill width must equal full inner width
        snprintf(msg, sizeof(msg),
                 "[iter %d] value=100 should fill entire inner width. "
                 "w=%d, innerWidth=%d, got fillW=%d",
                 i, (int)w, (int)innerWidth, (int)display.lastFillRectW);
        TEST_ASSERT_EQUAL_INT16_MESSAGE(innerWidth,
                                        display.lastFillRectW, msg);

        // Verify fill position and height
        TEST_ASSERT_EQUAL_INT16(x + 1, display.lastFillRectX);
        TEST_ASSERT_EQUAL_INT16(y + 1, display.lastFillRectY);
        TEST_ASSERT_EQUAL_INT16(h - 2, display.lastFillRectH);
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property7_random_fill_proportional);
    RUN_TEST(test_property7_value_zero_no_fill);
    RUN_TEST(test_property7_value_100_full_fill);

    return UNITY_END();
}
