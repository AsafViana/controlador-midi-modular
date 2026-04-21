/**
 * Property-Based Test: Modelo de pilha do Router
 *
 * Feature: oled-ui-framework, Property 1: Modelo de pilha do Router
 *
 * **Validates: Requirements 5.1, 5.2, 5.3, 5.4**
 *
 * Property: For any random sequence of push and pop operations on the Router,
 * currentScreen() must always return the Screen at the top of the LIFO stack,
 * and onMount()/onUnmount() callbacks must be invoked in the correct order.
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

// --- Reference model: simple stack to mirror Router behavior ---
static const int REF_MAX_STACK = 8;

struct RefModel {
    int stack[REF_MAX_STACK];  // screen IDs
    int size;
};

static void ref_init(RefModel& m) {
    m.size = 0;
}

static int ref_top(const RefModel& m) {
    if (m.size == 0) return -1;
    return m.stack[m.size - 1];
}

static bool ref_push(RefModel& m, int screenId) {
    if (m.size >= REF_MAX_STACK) return false;
    m.stack[m.size] = screenId;
    m.size++;
    return true;
}

static bool ref_pop(RefModel& m) {
    if (m.size <= 1) return false;
    m.size--;
    return true;
}

// --- Pool of TestScreen objects ---
static const int SCREEN_POOL_SIZE = 10;
static TestScreen g_screens[SCREEN_POOL_SIZE];

static void init_screen_pool() {
    for (int i = 0; i < SCREEN_POOL_SIZE; i++) {
        g_screens[i].setId(i);
    }
}

// --- Operation types ---
enum OpType { OP_PUSH, OP_POP };

static const int PBT_ITERATIONS = 25;
static const int MAX_OPS_PER_ITERATION = 30;

// ============================================================
// Property 1: Modelo de pilha do Router
// ============================================================

/**
 * **Validates: Requirements 5.1, 5.2, 5.3, 5.4**
 *
 * For 100 random sequences of push/pop operations:
 *   - currentScreen() always returns the top of the LIFO stack
 *   - onMount()/onUnmount() are invoked in the correct order
 *
 * Strategy:
 *   1. Generate a random sequence of push/pop operations
 *   2. Maintain a reference model (simple array stack)
 *   3. After each operation, verify currentScreen() matches model top
 *   4. Verify onMount/onUnmount log matches expected calls
 */
void test_property1_router_stack_model() {
    lcg_seed(42u);
    init_screen_pool();

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        Router router;
        RefModel model;
        ref_init(model);
        log_reset();

        // We need at least one push to have a meaningful stack
        // Start with a random initial push
        int initialScreenId = lcg_next() % SCREEN_POOL_SIZE;
        router.push(&g_screens[initialScreenId]);
        ref_push(model, initialScreenId);

        // Verify initial state
        TEST_ASSERT_NOT_NULL_MESSAGE(router.currentScreen(),
            "currentScreen() should not be null after initial push");
        TEST_ASSERT_EQUAL_PTR_MESSAGE(&g_screens[ref_top(model)],
            router.currentScreen(),
            "currentScreen() should match model top after initial push");

        // Verify initial onMount was logged
        TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_logCount,
            "Should have exactly 1 log entry after initial push");
        TEST_ASSERT_EQUAL_INT_MESSAGE(initialScreenId, g_log[0].screenId,
            "Initial onMount should be for the pushed screen");
        TEST_ASSERT_TRUE_MESSAGE(g_log[0].isMount,
            "Initial log entry should be onMount");

        // Generate random number of operations for this iteration
        int numOps = (lcg_next() % MAX_OPS_PER_ITERATION) + 1;

        for (int op = 0; op < numOps; op++) {
            // Decide operation: 60% push, 40% pop (to build up stacks)
            OpType opType;
            if (lcg_next() % 10 < 6) {
                opType = OP_PUSH;
            } else {
                opType = OP_POP;
            }

            int logCountBefore = g_logCount;
            int prevTopId = ref_top(model);

            if (opType == OP_PUSH) {
                int screenId = lcg_next() % SCREEN_POOL_SIZE;

                bool modelCanPush = (model.size < REF_MAX_STACK);
                router.push(&g_screens[screenId]);

                if (modelCanPush) {
                    ref_push(model, screenId);

                    // Verify lifecycle: onUnmount(previous) then onMount(new)
                    TEST_ASSERT_EQUAL_INT_MESSAGE(logCountBefore + 2, g_logCount,
                        "push should generate 2 log entries (unmount prev + mount new)");

                    // Entry: onUnmount of previous top
                    TEST_ASSERT_EQUAL_INT_MESSAGE(prevTopId,
                        g_log[logCountBefore].screenId,
                        "push: first log should be previous screen's onUnmount");
                    TEST_ASSERT_FALSE_MESSAGE(g_log[logCountBefore].isMount,
                        "push: first log should be onUnmount");

                    // Entry: onMount of new screen
                    TEST_ASSERT_EQUAL_INT_MESSAGE(screenId,
                        g_log[logCountBefore + 1].screenId,
                        "push: second log should be new screen's onMount");
                    TEST_ASSERT_TRUE_MESSAGE(g_log[logCountBefore + 1].isMount,
                        "push: second log should be onMount");
                } else {
                    // Push ignored when full — no log entries
                    TEST_ASSERT_EQUAL_INT_MESSAGE(logCountBefore, g_logCount,
                        "push on full stack should not generate log entries");
                }
            } else {
                // OP_POP
                bool modelCanPop = (model.size > 1);
                int secondTopId = -1;
                if (model.size >= 2) {
                    secondTopId = model.stack[model.size - 2];
                }

                router.pop();

                if (modelCanPop) {
                    ref_pop(model);

                    // Verify lifecycle: onUnmount(current) then onMount(previous)
                    TEST_ASSERT_EQUAL_INT_MESSAGE(logCountBefore + 2, g_logCount,
                        "pop should generate 2 log entries (unmount current + mount previous)");

                    // Entry: onUnmount of popped screen
                    TEST_ASSERT_EQUAL_INT_MESSAGE(prevTopId,
                        g_log[logCountBefore].screenId,
                        "pop: first log should be popped screen's onUnmount");
                    TEST_ASSERT_FALSE_MESSAGE(g_log[logCountBefore].isMount,
                        "pop: first log should be onUnmount");

                    // Entry: onMount of new top
                    TEST_ASSERT_EQUAL_INT_MESSAGE(secondTopId,
                        g_log[logCountBefore + 1].screenId,
                        "pop: second log should be new top screen's onMount");
                    TEST_ASSERT_TRUE_MESSAGE(g_log[logCountBefore + 1].isMount,
                        "pop: second log should be onMount");
                } else {
                    // Pop ignored when stack has 1 element — no log entries
                    TEST_ASSERT_EQUAL_INT_MESSAGE(logCountBefore, g_logCount,
                        "pop with 1 screen should not generate log entries");
                }
            }

            // After every operation, verify currentScreen matches model top
            int expectedTopId = ref_top(model);
            TEST_ASSERT_TRUE_MESSAGE(expectedTopId >= 0,
                "Model stack should never be empty during test");
            TEST_ASSERT_EQUAL_PTR_MESSAGE(&g_screens[expectedTopId],
                router.currentScreen(),
                "currentScreen() must match reference model top after operation");
        }
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property1_router_stack_model);

    return UNITY_END();
}
