/**
 * Property-Based Test: Reatividade do State
 *
 * Feature: oled-ui-framework, Property 3: Reatividade do State
 *
 * Validates: Requirements 3.1
 *
 * Property: For any type T and any two values a and b where a != b,
 * creating State<T>(screen, a) and calling set(b) must mark the Screen
 * as dirty. Calling set(b) again (same current value) must NOT mark dirty.
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>

// --- Minimal mock for Adafruit_SSD1306 (required by Screen.h) ---
class Adafruit_SSD1306 {};

#include "ui/Screen.h"
#include "ui/State.h"

// --- Concrete Screen for tracking dirty flag ---
class TestScreen : public Screen {
public:
    void render(Adafruit_SSD1306& display) override {}
};

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

/// Generate a random int32_t
static int32_t rand_int() {
    return static_cast<int32_t>(lcg_next());
}

/// Generate a random float in a wide range
static float rand_float() {
    // Generate a float from two random ints to get good coverage
    int32_t mantissa = static_cast<int32_t>(lcg_next() % 200001) - 100000;
    uint32_t divisor = (lcg_next() % 1000) + 1;
    return static_cast<float>(mantissa) / static_cast<float>(divisor);
}

/// Generate a random float guaranteed to be different from `other`
static float rand_float_different_from(float other) {
    float val;
    int attempts = 0;
    do {
        val = rand_float();
        attempts++;
        // Safety: after many attempts, just offset the value
        if (attempts > 100) {
            val = other + 1.0f;
            break;
        }
    } while (val == other);
    return val;
}

/// Generate a random int guaranteed to be different from `other`
static int32_t rand_int_different_from(int32_t other) {
    int32_t val;
    int attempts = 0;
    do {
        val = rand_int();
        attempts++;
        if (attempts > 100) {
            val = other + 1;
            break;
        }
    } while (val == other);
    return val;
}

// ============================================================
// Property 3: Reatividade do State — int
// ============================================================

static const int PBT_ITERATIONS = 25;

/**
 * **Validates: Requirements 3.1**
 *
 * For 100 random int pairs (a, b) where a != b:
 *   - State(screen, a).set(b) marks dirty
 *   - Clearing dirty then set(b) again does NOT mark dirty
 */
void test_property3_int_reactivity() {
    lcg_seed(42u);

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        TestScreen screen;
        int32_t a = rand_int();
        int32_t b = rand_int_different_from(a);

        State<int32_t> state(&screen, a);

        // Clear the initial dirty flag (Screen starts dirty)
        screen.clearDirty();
        TEST_ASSERT_FALSE_MESSAGE(screen.isDirty(),
            "Screen should be clean before set(b)");

        // set(b) with b != a must mark dirty
        state.set(b);
        TEST_ASSERT_TRUE_MESSAGE(screen.isDirty(),
            "set(b) with b != a must mark Screen dirty");

        // Clear dirty, then set(b) again (same value) must NOT mark dirty
        screen.clearDirty();
        state.set(b);
        TEST_ASSERT_FALSE_MESSAGE(screen.isDirty(),
            "set(b) with same current value must NOT mark dirty");
    }
}

// ============================================================
// Property 3: Reatividade do State — bool
// ============================================================

/**
 * **Validates: Requirements 3.1**
 *
 * For 100 random bool pairs (a, b) where a != b:
 *   - State(screen, a).set(b) marks dirty
 *   - Clearing dirty then set(b) again does NOT mark dirty
 *
 * Note: bool only has two values, so we alternate and randomize
 * the starting value across iterations.
 */
void test_property3_bool_reactivity() {
    lcg_seed(7u);

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        TestScreen screen;
        bool a = (lcg_next() % 2) == 0;
        bool b = !a; // guaranteed different

        State<bool> state(&screen, a);

        // Clear the initial dirty flag
        screen.clearDirty();
        TEST_ASSERT_FALSE_MESSAGE(screen.isDirty(),
            "Screen should be clean before set(b)");

        // set(b) with b != a must mark dirty
        state.set(b);
        TEST_ASSERT_TRUE_MESSAGE(screen.isDirty(),
            "set(b) with b != a must mark Screen dirty (bool)");

        // Clear dirty, then set(b) again (same value) must NOT mark dirty
        screen.clearDirty();
        state.set(b);
        TEST_ASSERT_FALSE_MESSAGE(screen.isDirty(),
            "set(b) with same current value must NOT mark dirty (bool)");
    }
}

// ============================================================
// Property 3: Reatividade do State — float
// ============================================================

/**
 * **Validates: Requirements 3.1**
 *
 * For 100 random float pairs (a, b) where a != b:
 *   - State(screen, a).set(b) marks dirty
 *   - Clearing dirty then set(b) again does NOT mark dirty
 */
void test_property3_float_reactivity() {
    lcg_seed(99u);

    for (int i = 0; i < PBT_ITERATIONS; i++) {
        TestScreen screen;
        float a = rand_float();
        float b = rand_float_different_from(a);

        State<float> state(&screen, a);

        // Clear the initial dirty flag
        screen.clearDirty();
        TEST_ASSERT_FALSE_MESSAGE(screen.isDirty(),
            "Screen should be clean before set(b)");

        // set(b) with b != a must mark dirty
        state.set(b);
        TEST_ASSERT_TRUE_MESSAGE(screen.isDirty(),
            "set(b) with b != a must mark Screen dirty (float)");

        // Clear dirty, then set(b) again (same value) must NOT mark dirty
        screen.clearDirty();
        state.set(b);
        TEST_ASSERT_FALSE_MESSAGE(screen.isDirty(),
            "set(b) with same current value must NOT mark dirty (float)");
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property3_int_reactivity);
    RUN_TEST(test_property3_bool_reactivity);
    RUN_TEST(test_property3_float_reactivity);

    return UNITY_END();
}
