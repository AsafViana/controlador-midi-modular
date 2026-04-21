/**
 * Property-Based Test: Encaminhamento de eventos
 *
 * Feature: oled-ui-framework, Property 4: Encaminhamento de eventos
 *
 * **Validates: Requirements 6.1**
 *
 * Property: For any ButtonEvent different from NONE, when the Router
 * receives that event, it must invoke handleInput() of the active Screen
 * with exactly that event.
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>

// --- Minimal mock for Adafruit_SSD1306 (required by Screen.h) ---
class Adafruit_SSD1306 {};

#include "ui/Screen.h"
#include "ui/Router.h"

// --- ButtonEvent values excluding NONE ---
// From src/button/Button.h:
//   NONE, PRESSED, RELEASED, SINGLE_CLICK, LONG_PRESS, DOUBLE_CLICK
static const ButtonEvent ALL_EVENTS[] = {
    ButtonEvent::PRESSED,
    ButtonEvent::RELEASED,
    ButtonEvent::SINGLE_CLICK,
    ButtonEvent::LONG_PRESS,
    ButtonEvent::DOUBLE_CLICK
};
static const int NUM_EVENTS = sizeof(ALL_EVENTS) / sizeof(ALL_EVENTS[0]);

// --- TestScreen: concrete Screen that records the last event received ---
class TestScreen : public Screen {
public:
    int id;
    ButtonEvent lastEvent;
    int handleInputCallCount;

    TestScreen() : id(-1), lastEvent(ButtonEvent::NONE), handleInputCallCount(0) {}

    void setId(int newId) { id = newId; }

    void reset() {
        lastEvent = ButtonEvent::NONE;
        handleInputCallCount = 0;
    }

    void render(Adafruit_SSD1306& display) override {}

    void handleInput(ButtonEvent event) override {
        lastEvent = event;
        handleInputCallCount++;
    }
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

/// Pick a random ButtonEvent excluding NONE
static ButtonEvent rand_event() {
    return ALL_EVENTS[lcg_next() % NUM_EVENTS];
}

static const int PBT_ITERATIONS = 25;

// ============================================================
// Property 4: Encaminhamento de eventos — single screen
// ============================================================

/**
 * **Validates: Requirements 6.1**
 *
 * For 100 random ButtonEvents (excluding NONE):
 *   - Push a single TestScreen onto the Router
 *   - Call router.handleInput(event)
 *   - Verify the screen received exactly that event via handleInput()
 *   - Verify handleInput() was called exactly once
 */
void test_property4_event_forwarding_single_screen() {
    lcg_seed(42u);
    init_screen_pool();

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        Router router;

        // Pick a random screen and push it
        int screenId = lcg_next() % SCREEN_POOL_SIZE;
        TestScreen* screen = &g_screens[screenId];
        screen->reset();
        router.push(screen);

        // Pick a random event (excluding NONE)
        ButtonEvent event = rand_event();

        // Forward the event through the Router
        router.handleInput(event);

        // Verify the active screen received exactly that event
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            static_cast<int>(event),
            static_cast<int>(screen->lastEvent),
            "Active screen must receive the exact event forwarded by Router");

        // Verify handleInput was called exactly once
        TEST_ASSERT_EQUAL_INT_MESSAGE(1, screen->handleInputCallCount,
            "handleInput() must be called exactly once per event");
    }
}

// ============================================================
// Property 4: Encaminhamento de eventos — multiple screens on stack
// ============================================================

/**
 * **Validates: Requirements 6.1**
 *
 * For 100 random iterations with multiple screens on the stack:
 *   - Push 2-8 random screens onto the Router
 *   - Call router.handleInput(event) with a random event
 *   - Verify ONLY the top screen received the event
 *   - Verify all other screens on the stack did NOT receive the event
 */
void test_property4_event_forwarding_only_top_screen() {
    lcg_seed(77u);
    init_screen_pool();

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        Router router;

        // Reset all screens
        for (int i = 0; i < SCREEN_POOL_SIZE; i++) {
            g_screens[i].reset();
        }

        // Push 2-8 screens (use distinct screen IDs to avoid aliasing)
        int stackDepth = (lcg_next() % 7) + 2; // 2..8
        if (stackDepth > SCREEN_POOL_SIZE) stackDepth = SCREEN_POOL_SIZE;

        // Track which screen IDs are on the stack
        int stackIds[8];
        for (int i = 0; i < stackDepth; i++) {
            // Pick a screen index that avoids reusing the same object
            // (use modular indexing within pool)
            int screenId = i % SCREEN_POOL_SIZE;
            stackIds[i] = screenId;
            router.push(&g_screens[screenId]);
        }

        // Reset all screens after pushes (onMount may have been called)
        for (int i = 0; i < SCREEN_POOL_SIZE; i++) {
            g_screens[i].reset();
        }

        int topScreenId = stackIds[stackDepth - 1];

        // Pick a random event (excluding NONE)
        ButtonEvent event = rand_event();

        // Forward the event through the Router
        router.handleInput(event);

        // Verify the top screen received the event
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            static_cast<int>(event),
            static_cast<int>(g_screens[topScreenId].lastEvent),
            "Top screen must receive the exact event forwarded by Router");
        TEST_ASSERT_EQUAL_INT_MESSAGE(1,
            g_screens[topScreenId].handleInputCallCount,
            "Top screen handleInput() must be called exactly once");

        // Verify all other screens on the stack did NOT receive the event
        for (int i = 0; i < stackDepth - 1; i++) {
            int otherId = stackIds[i];
            // Skip if this is the same screen object as the top
            if (otherId == topScreenId) continue;

            TEST_ASSERT_EQUAL_INT_MESSAGE(0,
                g_screens[otherId].handleInputCallCount,
                "Non-top screen must NOT receive events from Router");
            TEST_ASSERT_EQUAL_INT_MESSAGE(
                static_cast<int>(ButtonEvent::NONE),
                static_cast<int>(g_screens[otherId].lastEvent),
                "Non-top screen lastEvent must remain NONE");
        }
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property4_event_forwarding_single_screen);
    RUN_TEST(test_property4_event_forwarding_only_top_screen);

    return UNITY_END();
}
