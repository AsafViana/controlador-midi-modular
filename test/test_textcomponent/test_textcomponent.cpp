#include <unity.h>
#include "Adafruit_SSD1306.h"
#include "Wire.h"
#include "ui/components/TextComponent.h"

// --- Test display instance ---
static Adafruit_SSD1306 display(128, 64, &Wire, -1);

// --- setUp / tearDown ---

void setUp() {
    display.reset();
}

void tearDown() {}

// ============================================================
// render() — basic rendering
// ============================================================

void test_render_sets_cursor_position() {
    TextComponent tc(10, 20, "Hello");
    tc.render(display);

    TEST_ASSERT_EQUAL(1, display.setCursorCallCount);
    TEST_ASSERT_EQUAL_INT16(10, display.lastCursorX);
    TEST_ASSERT_EQUAL_INT16(20, display.lastCursorY);
}

void test_render_sets_text_size() {
    TextComponent tc(0, 0, "Hi", 2);
    tc.render(display);

    TEST_ASSERT_EQUAL(1, display.setTextSizeCallCount);
    TEST_ASSERT_EQUAL_UINT8(2, display.lastTextSize);
}

void test_render_sets_text_color_white() {
    TextComponent tc(0, 0, "Hi", 1, SSD1306_WHITE);
    tc.render(display);

    TEST_ASSERT_EQUAL(1, display.setTextColorCallCount);
    TEST_ASSERT_EQUAL_UINT16(SSD1306_WHITE, display.lastTextColor);
}

void test_render_sets_text_color_black() {
    TextComponent tc(0, 0, "Hi", 1, SSD1306_BLACK);
    tc.render(display);

    TEST_ASSERT_EQUAL_UINT16(SSD1306_BLACK, display.lastTextColor);
}

void test_render_prints_text() {
    TextComponent tc(0, 0, "Hello");
    tc.render(display);

    TEST_ASSERT_EQUAL(1, display.printCallCount);
    TEST_ASSERT_EQUAL_STRING("Hello", display.lastPrintedText);
}

void test_render_default_font_size_is_1() {
    TextComponent tc(0, 0, "Test");
    tc.render(display);

    TEST_ASSERT_EQUAL_UINT8(1, display.lastTextSize);
}

void test_render_default_color_is_white() {
    TextComponent tc(0, 0, "Test");
    tc.render(display);

    TEST_ASSERT_EQUAL_UINT16(SSD1306_WHITE, display.lastTextColor);
}

// ============================================================
// render() — nullptr and empty string handling
// ============================================================

void test_render_nullptr_text_does_not_print() {
    TextComponent tc(0, 0, nullptr);
    tc.render(display);

    // nullptr treated as empty string — nothing rendered
    TEST_ASSERT_EQUAL(0, display.printCallCount);
    TEST_ASSERT_EQUAL(0, display.setCursorCallCount);
}

void test_render_empty_string_does_not_print() {
    TextComponent tc(0, 0, "");
    tc.render(display);

    TEST_ASSERT_EQUAL(0, display.printCallCount);
    TEST_ASSERT_EQUAL(0, display.setCursorCallCount);
}

// ============================================================
// render() — truncation (fontSize 1: 6px per char)
// ============================================================

void test_render_short_text_no_truncation() {
    // "Hi" at x=0, fontSize 1 → 2 chars × 6px = 12px, fits in 128px
    TextComponent tc(0, 0, "Hi");
    tc.render(display);

    TEST_ASSERT_EQUAL_STRING("Hi", display.lastPrintedText);
}

void test_render_text_exactly_fills_display() {
    // fontSize 1: 6px per char. At x=0, 128/6 = 21.33 → 21 chars fit exactly (21*6=126 ≤ 128)
    // 22 chars = 132px > 128 → won't fit
    // Let's use 21 chars at x=0: 21*6 = 126 ≤ 128 → fits
    const char* text21 = "AAAAAAAAAAAAAAAAAAAAA"; // 21 chars
    TextComponent tc(0, 0, text21);
    tc.render(display);

    TEST_ASSERT_EQUAL_STRING(text21, display.lastPrintedText);
}

void test_render_text_truncated_when_exceeds_display() {
    // fontSize 1: 6px per char. At x=0, 30 chars = 180px > 128px
    // Max chars that fit: floor(128/6) = 21 chars (21*6=126 ≤ 128)
    const char* text30 = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"; // 30 chars
    TextComponent tc(0, 0, text30);
    tc.render(display);

    // Should be truncated to 21 chars
    TEST_ASSERT_EQUAL(1, display.printCallCount);
    TEST_ASSERT_EQUAL(21, (int)strlen(display.lastPrintedText));
}

void test_render_truncation_accounts_for_x_offset() {
    // fontSize 1: 6px per char. At x=100, available width = 128 - 100 = 28px
    // Max chars: floor(28/6) = 4 chars (4*6=24, 24+100=124 ≤ 128)
    const char* text10 = "ABCDEFGHIJ"; // 10 chars
    TextComponent tc(100, 0, text10);
    tc.render(display);

    TEST_ASSERT_EQUAL(1, display.printCallCount);
    TEST_ASSERT_EQUAL(4, (int)strlen(display.lastPrintedText));
    TEST_ASSERT_EQUAL_STRING("ABCD", display.lastPrintedText);
}

// ============================================================
// render() — truncation with larger font sizes
// ============================================================

void test_render_truncation_font_size_2() {
    // fontSize 2: 12px per char. At x=0, 128/12 = 10.67 → 10 chars fit (10*12=120 ≤ 128)
    const char* text15 = "AAAAAAAAAAAAAAA"; // 15 chars
    TextComponent tc(0, 0, text15, 2);
    tc.render(display);

    TEST_ASSERT_EQUAL(1, display.printCallCount);
    TEST_ASSERT_EQUAL(10, (int)strlen(display.lastPrintedText));
}

void test_render_truncation_font_size_3() {
    // fontSize 3: 18px per char. At x=0, 128/18 = 7.11 → 7 chars fit (7*18=126 ≤ 128)
    const char* text10 = "ABCDEFGHIJ"; // 10 chars
    TextComponent tc(0, 0, text10, 3);
    tc.render(display);

    TEST_ASSERT_EQUAL(1, display.printCallCount);
    TEST_ASSERT_EQUAL(7, (int)strlen(display.lastPrintedText));
    TEST_ASSERT_EQUAL_STRING("ABCDEFG", display.lastPrintedText);
}

// ============================================================
// render() — edge case: x position at or beyond display width
// ============================================================

void test_render_x_at_display_edge_no_room() {
    // x=128 → no room for any character at any font size
    TextComponent tc(128, 0, "A");
    tc.render(display);

    // getTextBounds for "A" at x=128: width=6, 128+6=134 > 128 → truncate
    // Try 0 chars → nothing to print
    TEST_ASSERT_EQUAL(0, display.printCallCount);
}

// ============================================================
// setText() and setPosition()
// ============================================================

void test_set_text_updates_rendered_text() {
    TextComponent tc(0, 0, "Old");
    tc.setText("New");
    tc.render(display);

    TEST_ASSERT_EQUAL_STRING("New", display.lastPrintedText);
}

void test_set_position_updates_cursor() {
    TextComponent tc(0, 0, "Hi");
    tc.setPosition(50, 30);
    tc.render(display);

    TEST_ASSERT_EQUAL_INT16(50, display.lastCursorX);
    TEST_ASSERT_EQUAL_INT16(30, display.lastCursorY);
}

void test_set_text_to_nullptr_after_construction() {
    TextComponent tc(0, 0, "Hello");
    tc.setText(nullptr);
    tc.render(display);

    TEST_ASSERT_EQUAL(0, display.printCallCount);
}

// ============================================================
// Font size values
// ============================================================

void test_render_font_size_1() {
    TextComponent tc(0, 0, "A", 1);
    tc.render(display);
    TEST_ASSERT_EQUAL_UINT8(1, display.lastTextSize);
}

void test_render_font_size_2() {
    TextComponent tc(0, 0, "A", 2);
    tc.render(display);
    TEST_ASSERT_EQUAL_UINT8(2, display.lastTextSize);
}

void test_render_font_size_3() {
    TextComponent tc(0, 0, "A", 3);
    tc.render(display);
    TEST_ASSERT_EQUAL_UINT8(3, display.lastTextSize);
}

// --- Runner ---

int main() {
    UNITY_BEGIN();

    // Basic rendering
    RUN_TEST(test_render_sets_cursor_position);
    RUN_TEST(test_render_sets_text_size);
    RUN_TEST(test_render_sets_text_color_white);
    RUN_TEST(test_render_sets_text_color_black);
    RUN_TEST(test_render_prints_text);
    RUN_TEST(test_render_default_font_size_is_1);
    RUN_TEST(test_render_default_color_is_white);

    // nullptr and empty string
    RUN_TEST(test_render_nullptr_text_does_not_print);
    RUN_TEST(test_render_empty_string_does_not_print);

    // Truncation — fontSize 1
    RUN_TEST(test_render_short_text_no_truncation);
    RUN_TEST(test_render_text_exactly_fills_display);
    RUN_TEST(test_render_text_truncated_when_exceeds_display);
    RUN_TEST(test_render_truncation_accounts_for_x_offset);

    // Truncation — larger font sizes
    RUN_TEST(test_render_truncation_font_size_2);
    RUN_TEST(test_render_truncation_font_size_3);

    // Edge cases
    RUN_TEST(test_render_x_at_display_edge_no_room);

    // setText / setPosition
    RUN_TEST(test_set_text_updates_rendered_text);
    RUN_TEST(test_set_position_updates_cursor);
    RUN_TEST(test_set_text_to_nullptr_after_construction);

    // Font sizes
    RUN_TEST(test_render_font_size_1);
    RUN_TEST(test_render_font_size_2);
    RUN_TEST(test_render_font_size_3);

    return UNITY_END();
}
