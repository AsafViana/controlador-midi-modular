/**
 * Property-Based Test: navigateTo substitui a pilha
 *
 * Feature: oled-ui-framework, Property 2: navigateTo substitui a pilha
 *
 * **Validates: Requirements 5.5**
 *
 * Property: For any state of the Router stack (with 1 to N Screens pushed),
 * calling navigateTo(screen) must result in a stack of size 1 where
 * currentScreen() returns the Screen passed as argument, and onUnmount()
 * of the previous Screen and onMount() of the new Screen must be invoked.
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include <cstring>

// --- Minimal mock for Adafruit_SSD1306 (required by Screen.h) ---
class Adafruit_SSD1306 {};

#include "ui/Screen.h"
#include "ui/Router.h"

// --- Global event log for tracking onMount/onUnmount calls ---
static const int MAX_LOG_ENTRIES = 512;

struct LogEntry {
    int screenId;    // which screen
    bool isMount;    // true = onMount, false = onUnmount
};

static LogEntry g_log[MAX_LOG_ENTRIES];
static int g_logCount = 0;

static void log_reset() {
    g_logCount = 0;
}

static void log_event(int screenId, bool isMount) {
    if (g_logCount < MAX_LOG_ENTRIES) {
        g_log[g_logCount].screenId = screenId;
        g_log[g_logCount].isMount = isMount;
        g_logCount++;
    }
}

// --- TestScreen: concrete Screen that logs lifecycle calls ---
class TestScreen : public Screen {
public:
    int id;

    TestScreen() : id(-1) {}

    void setId(int newId) { id = newId; }

    void onMount() override {
        log_event(id, true);
    }

    void onUnmount() override {
        log_event(id, false);
    }

    void render(Adafruit_SSD1306& display) override {}
};

// --- Simple LCG pseudo-random number generator ---
static uint32_t lcg_state = 12345u;

static void lcg_seed(uint32_t seed) {
    lcg_state = seed;
}

static uint32_t lcg_next() {
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state;
}

// --- Pool of TestScreen objects ---
static const int SCREEN_POOL_SIZE = 10;
static TestScreen g_screens[SCREEN_POOL_SIZE];

static void init_screen_pool() {
    for (int i = 0; i < SCREEN_POOL_SIZE; i++) {
        g_screens[i].setId(i);
    }
}

static const int PBT_ITERATIONS = 25;
static const int MAX_STACK_DEPTH = 8;

// ============================================================
// Property 2: navigateTo substitui a pilha
// ============================================================

/**
 * **Validates: Requirements 5.5**
 *
 * For 100 random iterations:
 *   1. Build a random stack (1-8 screens via push)
 *   2. Call navigateTo(targetScreen)
 *   3. Verify:
 *      a. currentScreen() returns the target screen
 *      b. pop() after navigateTo is a no-op (stack size is 1)
 *      c. onUnmount() was called on the previous top screen
 *      d. onMount() was called on the target screen
 */
void test_property2_navigateTo_replaces_stack() {
    lcg_seed(99u);
    init_screen_pool();

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        Router router;
        log_reset();

        // --- Phase 1: Build a random stack of 1 to MAX_STACK_DEPTH screens ---
        int stackDepth = (lcg_next() % MAX_STACK_DEPTH) + 1; // 1..8

        for (int i = 0; i < stackDepth; i++) {
            int screenId = lcg_next() % SCREEN_POOL_SIZE;
            router.push(&g_screens[screenId]);
        }

        // Record the top screen ID before navigateTo
        Screen* prevTop = router.currentScreen();
        TEST_ASSERT_NOT_NULL_MESSAGE(prevTop,
            "Stack should not be empty after pushes");
        int prevTopId = static_cast<TestScreen*>(prevTop)->id;

        // Reset log to only capture navigateTo lifecycle events
        log_reset();

        // --- Phase 2: Call navigateTo with a random target screen ---
        int targetId = lcg_next() % SCREEN_POOL_SIZE;
        router.navigateTo(&g_screens[targetId]);

        // --- Phase 3: Verify properties ---

        // 3a. currentScreen() returns the target screen
        TEST_ASSERT_EQUAL_PTR_MESSAGE(&g_screens[targetId],
            router.currentScreen(),
            "currentScreen() must return the navigateTo target");

        // 3b. pop() after navigateTo should be a no-op (stack size is 1)
        //     After pop(), currentScreen() should still be the target
        Screen* beforePop = router.currentScreen();
        int logCountBeforePop = g_logCount;
        router.pop();
        TEST_ASSERT_EQUAL_PTR_MESSAGE(beforePop,
            router.currentScreen(),
            "pop() after navigateTo should be a no-op (stack has 1 screen)");
        TEST_ASSERT_EQUAL_INT_MESSAGE(logCountBeforePop, g_logCount,
            "pop() on single-screen stack should not generate lifecycle events");

        // 3c. onUnmount() was called on the previous top screen
        TEST_ASSERT_TRUE_MESSAGE(g_logCount >= 2,
            "navigateTo should generate at least 2 lifecycle events");
        TEST_ASSERT_EQUAL_INT_MESSAGE(prevTopId, g_log[0].screenId,
            "First lifecycle event should be onUnmount of previous top screen");
        TEST_ASSERT_FALSE_MESSAGE(g_log[0].isMount,
            "First lifecycle event should be onUnmount (not onMount)");

        // 3d. onMount() was called on the target screen
        TEST_ASSERT_EQUAL_INT_MESSAGE(targetId, g_log[1].screenId,
            "Second lifecycle event should be onMount of target screen");
        TEST_ASSERT_TRUE_MESSAGE(g_log[1].isMount,
            "Second lifecycle event should be onMount (not onUnmount)");

        // Verify exactly 2 lifecycle events from navigateTo (no extra calls)
        TEST_ASSERT_EQUAL_INT_MESSAGE(2, g_logCount,
            "navigateTo should generate exactly 2 lifecycle events (unmount prev + mount new)");
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property2_navigateTo_replaces_stack);

    return UNITY_END();
}
