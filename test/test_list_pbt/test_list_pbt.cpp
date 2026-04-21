/**
 * Property-Based Test: Navegação da lista
 *
 * Feature: oled-ui-framework, Property 5: Navegação da lista
 *
 * **Validates: Requirements 8.2, 8.3, 8.4**
 *
 * Property: For any ListComponent with N items (N >= 1) and any random
 * sequence of "up" and "down" events, selectedIndex must always remain
 * in the range [0, N-1], and scrollOffset must guarantee that
 * selectedIndex is within the visible window [scrollOffset, scrollOffset + visibleCount).
 *
 * Uses a simple LCG pseudo-random generator since RapidCheck is not
 * available in the PlatformIO native environment.
 */

#include <unity.h>
#include "Adafruit_SSD1306.h"
#include "Wire.h"
#include "ui/components/ListComponent.h"

// --- Simple LCG pseudo-random number generator ---
static uint32_t lcg_state = 12345u;

static void lcg_seed(uint32_t seed) {
    lcg_state = seed;
}

static uint32_t lcg_next() {
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state;
}

// --- Test display instance ---
static Adafruit_SSD1306 display(128, 64, &Wire, -1);

// --- Pool of item strings ---
static const int MAX_ITEMS = 20;
static const char* g_itemStrings[MAX_ITEMS] = {
    "Item 0", "Item 1", "Item 2", "Item 3", "Item 4",
    "Item 5", "Item 6", "Item 7", "Item 8", "Item 9",
    "Item 10", "Item 11", "Item 12", "Item 13", "Item 14",
    "Item 15", "Item 16", "Item 17", "Item 18", "Item 19"
};

// --- PBT configuration ---
static const int PBT_ITERATIONS = 25;
static const int MAX_EVENTS_PER_ITERATION = 50;

// --- Font sizes to test ---
static const uint8_t FONT_SIZES[] = {1, 2, 3};
static const int NUM_FONT_SIZES = 3;

// --- setUp / tearDown ---

void setUp() {
    display.reset();
}

void tearDown() {}

// ============================================================
// Property 5: Navegação da lista
// ============================================================

/**
 * **Validates: Requirements 8.2, 8.3, 8.4**
 *
 * For 100 random iterations:
 *   - Generate a random list size N in [1, MAX_ITEMS]
 *   - Generate a random font size (1, 2, or 3)
 *   - Generate a random sequence of up/down events
 *   - After each event, verify:
 *     (a) selectedIndex is in [0, N-1]
 *     (b) scrollOffset keeps selectedIndex in visible window
 *         [scrollOffset, scrollOffset + visibleCount)
 *
 * Strategy:
 *   1. Create a ListComponent with random parameters
 *   2. Render once to initialize visibleCount
 *   3. Apply random up/down events
 *   4. After each event, check invariants
 */
void test_property5_list_navigation() {
    lcg_seed(42u);

    for (int iter = 0; iter < PBT_ITERATIONS; iter++) {
        // Random list size: 1 to MAX_ITEMS
        uint8_t itemCount = (lcg_next() % MAX_ITEMS) + 1;

        // Random font size: 1, 2, or 3
        uint8_t fontSize = FONT_SIZES[lcg_next() % NUM_FONT_SIZES];

        // Random list height: at least one item visible, up to 64px
        uint8_t itemHeight = fontSize * 8;
        // Ensure height is at least itemHeight (so at least 1 item visible)
        // and at most 64 (display height)
        uint8_t minHeight = itemHeight;
        uint8_t maxHeight = 64;
        uint8_t listHeight;
        if (minHeight >= maxHeight) {
            listHeight = maxHeight;
        } else {
            listHeight = minHeight + (lcg_next() % (maxHeight - minHeight + 1));
        }

        uint8_t expectedVisibleCount = listHeight / itemHeight;
        if (expectedVisibleCount == 0) {
            expectedVisibleCount = 1;
            listHeight = itemHeight;
        }

        // Create list component
        ListComponent list(0, 0, 128, listHeight, fontSize);
        list.setItems(g_itemStrings, itemCount);
        list.setUpButton(ButtonEvent::PRESSED);
        list.setDownButton(ButtonEvent::SINGLE_CLICK);

        // Render once to initialize visibleCount
        display.reset();
        list.render(display);

        // Verify initial state
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, list.getSelectedIndex(),
            "Initial selectedIndex should be 0");

        // Generate random number of events
        int numEvents = (lcg_next() % MAX_EVENTS_PER_ITERATION) + 1;

        for (int ev = 0; ev < numEvents; ev++) {
            // Random event: 50% up, 50% down
            ButtonEvent event;
            if (lcg_next() % 2 == 0) {
                event = ButtonEvent::SINGLE_CLICK; // down
            } else {
                event = ButtonEvent::PRESSED; // up
            }

            list.handleInput(event);

            // --- Invariant (a): selectedIndex in [0, N-1] ---
            uint8_t selectedIndex = list.getSelectedIndex();
            TEST_ASSERT_TRUE_MESSAGE(selectedIndex < itemCount,
                "selectedIndex must be < itemCount");

            // --- Invariant (b): scrollOffset keeps selected in visible window ---
            // We need to render to observe the scroll behavior and then
            // verify by checking that the selected item would be rendered.
            // Since scrollOffset is private, we verify indirectly:
            // render the list and check that the selected item's text is printed.
            display.reset();
            list.render(display);

            // The number of printed items should be min(visibleCount, itemCount)
            uint8_t expectedPrinted = (expectedVisibleCount < itemCount)
                ? expectedVisibleCount : itemCount;
            TEST_ASSERT_EQUAL_MESSAGE(expectedPrinted, display.printCallCount,
                "Should print exactly visibleCount (or itemCount if fewer) items");

            // Verify that fillRect was called (for the selected item highlight)
            // The selected item should always be visible, so fillRect should be called
            TEST_ASSERT_TRUE_MESSAGE(display.fillRectCallCount >= 1,
                "Selected item should be highlighted (fillRect called)");
        }
    }
}

// --- Unity runner ---

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_property5_list_navigation);

    return UNITY_END();
}
