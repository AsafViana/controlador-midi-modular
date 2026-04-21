/**
 * Property-Based Test: Ordem de renderização dos filhos
 *
 * Feature: oled-ui-framework, Property 8: Ordem de renderização dos filhos
 *
 * **Validates: Requirements 2.4**
 *
 * Property: For any Screen containing N child components added in a specific
 * order, calling render() on the Screen must invoke render() of each child
 * exactly once, in the same order they were added.
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include <cstring>
#include <cstdio>

// --- Minimal mock for Adafruit_SSD1306 (required by Screen.h) ---
class Adafruit_SSD1306 {};

#include "ui/Screen.h"
#include "ui/UIComponent.h"

// --- Shared render-order tracking ---
static const int MAX_RENDER_LOG = 32;
static int g_renderLog[MAX_RENDER_LOG];  // records component IDs in render order
static int g_renderLogCount = 0;

static void render_log_reset() {
    g_renderLogCount = 0;
}

static void render_log_record(int componentId) {
    if (g_renderLogCount < MAX_RENDER_LOG) {
        g_renderLog[g_renderLogCount] = componentId;
        g_renderLogCount++;
    }
}

// --- MockComponent: records its render call order ---
class MockComponent : public UIComponent {
public:
    int id;

    MockComponent() : id(-1) {}

    void setId(int newId) { id = newId; }

    void render(Adafruit_SSD1306& display) override {
        render_log_record(id);
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

// --- Pool of MockComponent objects ---
static MockComponent g_components[Screen::MAX_CHILDREN];

static void init_component_pool() {
    for (int i = 0; i < Screen::MAX_CHILDREN; i++) {
        g_components[i].setId(i);
    }
}

// --- PBT configuration ---
static const int PBT_ITERATIONS = 25;

// ============================================================
// Property 8: Ordem de renderização dos filhos
// ============================================================

/**
 * **Validates: Requirements 2.4**
 *
 * For 25 random iterations:
 *   - Generate a random number of mock components (1 to MAX_CHILDREN)
 *   - Add them to a Screen via addChild()
 *   - Call screen.render(display)
 *   - Verify each component was called exactly once
 *   - Verify the call order matches the insertion order
 */
void test_property8_render_order() {
    lcg_seed(98765u);
    init_component_pool();

    Adafruit_SSD1306 display;

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        // --- Generate random child count: 1 to MAX_CHILDREN ---
        int childCount = (lcg_next() % Screen::MAX_CHILDREN) + 1;

        // --- Create a concrete Screen (uses base render which iterates children) ---
        // We use Screen directly since its render() iterates children in order
        // Screen is not abstract — it has a virtual render() with a default implementation
        Screen screen;

        // --- Build a random permutation of component IDs to add ---
        // This ensures we test different orderings, not just 0..N-1
        int insertionOrder[Screen::MAX_CHILDREN];
        for (int i = 0; i < childCount; i++) {
            insertionOrder[i] = i;
        }
        // Fisher-Yates shuffle
        for (int i = childCount - 1; i > 0; i--) {
            int j = lcg_next() % (i + 1);
            int tmp = insertionOrder[i];
            insertionOrder[i] = insertionOrder[j];
            insertionOrder[j] = tmp;
        }

        // --- Add children in the shuffled order ---
        for (int i = 0; i < childCount; i++) {
            bool added = screen.addChild(&g_components[insertionOrder[i]]);
            TEST_ASSERT_TRUE_MESSAGE(added, "addChild() should succeed within MAX_CHILDREN");
        }

        // --- Verify child count ---
        TEST_ASSERT_EQUAL_UINT8_MESSAGE((uint8_t)childCount, screen.getChildCount(),
            "Screen should have the expected number of children");

        // --- Call render and capture the order ---
        render_log_reset();
        screen.render(display);

        // --- Verify each component was called exactly once ---
        TEST_ASSERT_EQUAL_INT_MESSAGE(childCount, g_renderLogCount,
            "render() should invoke each child exactly once");

        // --- Verify the call order matches the insertion order ---
        for (int i = 0; i < childCount; i++) {
            char msg[128];
            snprintf(msg, sizeof(msg),
                "Iter %d: child at position %d should be component %d but was %d",
                iter, i, insertionOrder[i], g_renderLog[i]);
            TEST_ASSERT_EQUAL_INT_MESSAGE(insertionOrder[i], g_renderLog[i], msg);
        }
    }
}

// --- Unity runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property8_render_order);

    return UNITY_END();
}
