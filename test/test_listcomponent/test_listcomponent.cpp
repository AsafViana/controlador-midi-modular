#include <unity.h>
#include "Adafruit_SSD1306.h"
#include "Wire.h"
#include "ui/components/ListComponent.h"

// --- Test display instance ---
static Adafruit_SSD1306 display(128, 64, &Wire, -1);

// --- Callback tracking ---
static uint8_t lastCallbackIndex = 255;
static int callbackCount = 0;

static void selectionCallback(uint8_t index) {
    lastCallbackIndex = index;
    callbackCount++;
}

// --- setUp / tearDown ---

void setUp() {
    display.reset();
    lastCallbackIndex = 255;
    callbackCount = 0;
}

void tearDown() {}

// ============================================================
// Construction and defaults
// ============================================================

void test_default_selected_index_is_zero() {
    ListComponent list(0, 0, 128, 64);
    TEST_ASSERT_EQUAL_UINT8(0, list.getSelectedIndex());
}

// ============================================================
// render() — empty list
// ============================================================

void test_render_empty_list_draws_nothing() {
    ListComponent list(0, 0, 128, 64);
    list.render(display);

    TEST_ASSERT_EQUAL(0, display.printCallCount);
    TEST_ASSERT_EQUAL(0, display.setCursorCallCount);
    TEST_ASSERT_EQUAL(0, display.fillRectCallCount);
}

void test_render_null_items_draws_nothing() {
    ListComponent list(0, 0, 128, 64);
    list.setItems(nullptr, 0);
    list.render(display);

    TEST_ASSERT_EQUAL(0, display.printCallCount);
}

// ============================================================
// render() — single item
// ============================================================

void test_render_single_item_prints_text() {
    const char* items[] = {"Item A"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 1);
    list.render(display);

    TEST_ASSERT_EQUAL(1, display.printCallCount);
    TEST_ASSERT_EQUAL_STRING("Item A", display.lastPrintedText);
}

void test_render_single_item_is_highlighted() {
    const char* items[] = {"Item A"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 1);
    list.render(display);

    // Selected item should have fillRect for highlight
    TEST_ASSERT_EQUAL(1, display.fillRectCallCount);
    TEST_ASSERT_EQUAL_UINT16(SSD1306_WHITE, display.lastFillRectColor);
}

// ============================================================
// render() — multiple items
// ============================================================

void test_render_multiple_items_prints_all_visible() {
    const char* items[] = {"One", "Two", "Three"};
    // fontSize 1 → itemHeight = 8px, height = 64 → visibleCount = 8
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 3);
    list.render(display);

    // All 3 items should be printed
    TEST_ASSERT_EQUAL(3, display.printCallCount);
}

void test_render_selected_item_has_inverse_highlight() {
    const char* items[] = {"One", "Two", "Three"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 3);
    list.render(display);

    // First item is selected by default → fillRect called once for highlight
    TEST_ASSERT_EQUAL(1, display.fillRectCallCount);
    TEST_ASSERT_EQUAL_UINT16(SSD1306_WHITE, display.lastFillRectColor);
}

void test_render_cursor_positions_are_correct() {
    const char* items[] = {"A", "B", "C"};
    // fontSize 1 → itemHeight = 8px, list at y=10
    ListComponent list(5, 10, 120, 64);
    list.setItems(items, 3);
    list.render(display);

    // Last setCursor call should be for item C at y = 10 + 2*8 = 26
    TEST_ASSERT_EQUAL(3, display.setCursorCallCount);
    TEST_ASSERT_EQUAL_INT16(5, display.lastCursorX);
    TEST_ASSERT_EQUAL_INT16(26, display.lastCursorY);
}

// ============================================================
// render() — font size affects item height
// ============================================================

void test_render_font_size_2_item_height() {
    const char* items[] = {"A", "B", "C", "D", "E"};
    // fontSize 2 → itemHeight = 16px, height = 64 → visibleCount = 4
    ListComponent list(0, 0, 128, 64, 2);
    list.setItems(items, 5);
    list.render(display);

    // Only 4 items should be visible (64 / 16 = 4)
    TEST_ASSERT_EQUAL(4, display.printCallCount);
}

void test_render_font_size_3_item_height() {
    const char* items[] = {"A", "B", "C", "D", "E"};
    // fontSize 3 → itemHeight = 24px, height = 64 → visibleCount = 2
    ListComponent list(0, 0, 128, 64, 3);
    list.setItems(items, 5);
    list.render(display);

    // Only 2 items should be visible (64 / 24 = 2)
    TEST_ASSERT_EQUAL(2, display.printCallCount);
}

// ============================================================
// handleInput() — empty list
// ============================================================

void test_handle_input_empty_list_returns_false() {
    ListComponent list(0, 0, 128, 64);
    list.setUpButton(ButtonEvent::SINGLE_CLICK);
    list.setDownButton(ButtonEvent::LONG_PRESS);

    bool consumed = list.handleInput(ButtonEvent::SINGLE_CLICK);
    TEST_ASSERT_FALSE(consumed);
}

void test_handle_input_none_event_returns_false() {
    const char* items[] = {"A"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 1);
    list.setUpButton(ButtonEvent::SINGLE_CLICK);

    bool consumed = list.handleInput(ButtonEvent::NONE);
    TEST_ASSERT_FALSE(consumed);
}

// ============================================================
// handleInput() — navigation
// ============================================================

void test_handle_input_down_moves_selection() {
    const char* items[] = {"A", "B", "C"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 3);
    list.setDownButton(ButtonEvent::SINGLE_CLICK);

    // Render first to calculate _visibleCount
    list.render(display);

    bool consumed = list.handleInput(ButtonEvent::SINGLE_CLICK);
    TEST_ASSERT_TRUE(consumed);
    TEST_ASSERT_EQUAL_UINT8(1, list.getSelectedIndex());
}

void test_handle_input_up_moves_selection() {
    const char* items[] = {"A", "B", "C"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 3);
    list.setUpButton(ButtonEvent::PRESSED);
    list.setDownButton(ButtonEvent::SINGLE_CLICK);

    // Render first to calculate _visibleCount
    list.render(display);

    // Move down first, then up
    list.handleInput(ButtonEvent::SINGLE_CLICK); // down to 1
    bool consumed = list.handleInput(ButtonEvent::PRESSED); // up to 0
    TEST_ASSERT_TRUE(consumed);
    TEST_ASSERT_EQUAL_UINT8(0, list.getSelectedIndex());
}

void test_handle_input_up_at_top_stays_at_zero() {
    const char* items[] = {"A", "B", "C"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 3);
    list.setUpButton(ButtonEvent::SINGLE_CLICK);

    // Render first to calculate _visibleCount
    list.render(display);

    list.handleInput(ButtonEvent::SINGLE_CLICK); // try up at index 0
    TEST_ASSERT_EQUAL_UINT8(0, list.getSelectedIndex());
}

void test_handle_input_down_at_bottom_stays_at_last() {
    const char* items[] = {"A", "B"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 2);
    list.setDownButton(ButtonEvent::SINGLE_CLICK);

    // Render first to calculate _visibleCount
    list.render(display);

    list.handleInput(ButtonEvent::SINGLE_CLICK); // down to 1
    list.handleInput(ButtonEvent::SINGLE_CLICK); // try down at last
    TEST_ASSERT_EQUAL_UINT8(1, list.getSelectedIndex());
}

void test_handle_input_unmatched_event_returns_false() {
    const char* items[] = {"A", "B"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 2);
    list.setUpButton(ButtonEvent::SINGLE_CLICK);
    list.setDownButton(ButtonEvent::LONG_PRESS);

    bool consumed = list.handleInput(ButtonEvent::DOUBLE_CLICK);
    TEST_ASSERT_FALSE(consumed);
}

// ============================================================
// Scroll behavior
// ============================================================

void test_scroll_down_when_selection_exceeds_visible() {
    const char* items[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J"};
    // fontSize 1 → itemHeight = 8px, height = 24 → visibleCount = 3
    ListComponent list(0, 0, 128, 24);
    list.setItems(items, 10);
    list.setDownButton(ButtonEvent::SINGLE_CLICK);

    // Render to calculate _visibleCount
    list.render(display);

    // Move down 3 times: 0→1→2→3
    list.handleInput(ButtonEvent::SINGLE_CLICK);
    list.handleInput(ButtonEvent::SINGLE_CLICK);
    list.handleInput(ButtonEvent::SINGLE_CLICK);

    TEST_ASSERT_EQUAL_UINT8(3, list.getSelectedIndex());

    // Re-render and check that item at index 3 is visible
    display.reset();
    list.render(display);

    // With scrollOffset adjusted, the last printed text should be one of the visible items
    // visibleCount = 3, selectedIndex = 3, so scrollOffset should be 1
    // Items visible: 1, 2, 3
    TEST_ASSERT_EQUAL(3, display.printCallCount);
}

void test_scroll_up_when_selection_goes_above_visible() {
    const char* items[] = {"A", "B", "C", "D", "E"};
    // fontSize 1 → itemHeight = 8px, height = 24 → visibleCount = 3
    ListComponent list(0, 0, 128, 24);
    list.setItems(items, 5);
    list.setUpButton(ButtonEvent::PRESSED);
    list.setDownButton(ButtonEvent::SINGLE_CLICK);

    // Render to calculate _visibleCount
    list.render(display);

    // Move down to index 3
    list.handleInput(ButtonEvent::SINGLE_CLICK); // 1
    list.handleInput(ButtonEvent::SINGLE_CLICK); // 2
    list.handleInput(ButtonEvent::SINGLE_CLICK); // 3

    // Now move up to index 0
    list.handleInput(ButtonEvent::PRESSED); // 2
    list.handleInput(ButtonEvent::PRESSED); // 1
    list.handleInput(ButtonEvent::PRESSED); // 0

    TEST_ASSERT_EQUAL_UINT8(0, list.getSelectedIndex());

    // Re-render — item 0 should be visible
    display.reset();
    list.render(display);
    TEST_ASSERT_EQUAL(3, display.printCallCount);
}

// ============================================================
// Callback
// ============================================================

void test_callback_called_on_selection_change() {
    const char* items[] = {"A", "B", "C"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 3);
    list.setDownButton(ButtonEvent::SINGLE_CLICK);
    list.onSelectionChanged(selectionCallback);

    // Render first to calculate _visibleCount
    list.render(display);

    list.handleInput(ButtonEvent::SINGLE_CLICK); // down to 1

    TEST_ASSERT_EQUAL(1, callbackCount);
    TEST_ASSERT_EQUAL_UINT8(1, lastCallbackIndex);
}

void test_callback_not_called_when_at_boundary() {
    const char* items[] = {"A", "B"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 2);
    list.setUpButton(ButtonEvent::SINGLE_CLICK);
    list.onSelectionChanged(selectionCallback);

    // Render first to calculate _visibleCount
    list.render(display);

    // Try to move up at index 0 — selection doesn't change
    list.handleInput(ButtonEvent::SINGLE_CLICK);

    TEST_ASSERT_EQUAL(0, callbackCount);
}

void test_no_callback_when_not_set() {
    const char* items[] = {"A", "B"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 2);
    list.setDownButton(ButtonEvent::SINGLE_CLICK);

    // Render first to calculate _visibleCount
    list.render(display);

    // Should not crash when no callback is set
    list.handleInput(ButtonEvent::SINGLE_CLICK);
    TEST_ASSERT_EQUAL_UINT8(1, list.getSelectedIndex());
}

// ============================================================
// setItems() — clamp selectedIndex
// ============================================================

void test_set_items_clamps_selected_index() {
    const char* items5[] = {"A", "B", "C", "D", "E"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items5, 5);
    list.setDownButton(ButtonEvent::SINGLE_CLICK);

    // Render first to calculate _visibleCount
    list.render(display);

    // Move to index 4
    list.handleInput(ButtonEvent::SINGLE_CLICK); // 1
    list.handleInput(ButtonEvent::SINGLE_CLICK); // 2
    list.handleInput(ButtonEvent::SINGLE_CLICK); // 3
    list.handleInput(ButtonEvent::SINGLE_CLICK); // 4
    TEST_ASSERT_EQUAL_UINT8(4, list.getSelectedIndex());

    // Now set a shorter list — selectedIndex should be clamped
    const char* items2[] = {"X", "Y"};
    list.setItems(items2, 2);
    TEST_ASSERT_EQUAL_UINT8(1, list.getSelectedIndex());
}

void test_set_items_empty_resets_index() {
    const char* items[] = {"A", "B"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 2);
    list.setDownButton(ButtonEvent::SINGLE_CLICK);

    // Render first to calculate _visibleCount
    list.render(display);

    list.handleInput(ButtonEvent::SINGLE_CLICK); // move to 1

    list.setItems(nullptr, 0);
    TEST_ASSERT_EQUAL_UINT8(0, list.getSelectedIndex());
}

// ============================================================
// setUpButton / setDownButton
// ============================================================

void test_set_up_down_buttons() {
    const char* items[] = {"A", "B", "C"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 3);
    list.setUpButton(ButtonEvent::DOUBLE_CLICK);
    list.setDownButton(ButtonEvent::LONG_PRESS);

    // Render first to calculate _visibleCount
    list.render(display);

    // LONG_PRESS should move down
    bool consumed = list.handleInput(ButtonEvent::LONG_PRESS);
    TEST_ASSERT_TRUE(consumed);
    TEST_ASSERT_EQUAL_UINT8(1, list.getSelectedIndex());

    // DOUBLE_CLICK should move up
    consumed = list.handleInput(ButtonEvent::DOUBLE_CLICK);
    TEST_ASSERT_TRUE(consumed);
    TEST_ASSERT_EQUAL_UINT8(0, list.getSelectedIndex());
}

// ============================================================
// render() — null item in list
// ============================================================

void test_render_null_item_in_list_prints_empty() {
    const char* items[] = {"A", nullptr, "C"};
    ListComponent list(0, 0, 128, 64);
    list.setItems(items, 3);
    list.render(display);

    // All 3 items should be printed (nullptr treated as "")
    TEST_ASSERT_EQUAL(3, display.printCallCount);
}

// --- Runner ---

int main() {
    UNITY_BEGIN();

    // Construction
    RUN_TEST(test_default_selected_index_is_zero);

    // render() — empty list
    RUN_TEST(test_render_empty_list_draws_nothing);
    RUN_TEST(test_render_null_items_draws_nothing);

    // render() — single item
    RUN_TEST(test_render_single_item_prints_text);
    RUN_TEST(test_render_single_item_is_highlighted);

    // render() — multiple items
    RUN_TEST(test_render_multiple_items_prints_all_visible);
    RUN_TEST(test_render_selected_item_has_inverse_highlight);
    RUN_TEST(test_render_cursor_positions_are_correct);

    // render() — font size
    RUN_TEST(test_render_font_size_2_item_height);
    RUN_TEST(test_render_font_size_3_item_height);

    // handleInput() — empty list
    RUN_TEST(test_handle_input_empty_list_returns_false);
    RUN_TEST(test_handle_input_none_event_returns_false);

    // handleInput() — navigation
    RUN_TEST(test_handle_input_down_moves_selection);
    RUN_TEST(test_handle_input_up_moves_selection);
    RUN_TEST(test_handle_input_up_at_top_stays_at_zero);
    RUN_TEST(test_handle_input_down_at_bottom_stays_at_last);
    RUN_TEST(test_handle_input_unmatched_event_returns_false);

    // Scroll
    RUN_TEST(test_scroll_down_when_selection_exceeds_visible);
    RUN_TEST(test_scroll_up_when_selection_goes_above_visible);

    // Callback
    RUN_TEST(test_callback_called_on_selection_change);
    RUN_TEST(test_callback_not_called_when_at_boundary);
    RUN_TEST(test_no_callback_when_not_set);

    // setItems clamp
    RUN_TEST(test_set_items_clamps_selected_index);
    RUN_TEST(test_set_items_empty_resets_index);

    // setUpButton / setDownButton
    RUN_TEST(test_set_up_down_buttons);

    // Null item in list
    RUN_TEST(test_render_null_item_in_list_prints_empty);

    return UNITY_END();
}
