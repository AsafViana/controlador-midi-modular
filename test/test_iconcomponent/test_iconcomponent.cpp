#include <unity.h>
#include "Adafruit_SSD1306.h"
#include "Wire.h"
#include "ui/components/IconComponent.h"

// --- Test display instance ---
static Adafruit_SSD1306 display(128, 64, &Wire, -1);

// --- Sample bitmap data (8x8 = 8 bytes) ---
static const uint8_t testBitmap8x8[] = {
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF
};

// --- setUp / tearDown ---

void setUp() {
    display.reset();
}

void tearDown() {}

// ============================================================
// render() — basic bitmap rendering
// ============================================================

void test_render_calls_drawBitmap() {
    IconComponent icon(10, 20, testBitmap8x8, 8, 8);
    icon.render(display);

    TEST_ASSERT_EQUAL(1, display.drawBitmapCallCount);
}

void test_render_passes_correct_position() {
    IconComponent icon(15, 30, testBitmap8x8, 8, 8);
    icon.render(display);

    TEST_ASSERT_EQUAL_INT16(15, display.lastDrawBitmapX);
    TEST_ASSERT_EQUAL_INT16(30, display.lastDrawBitmapY);
}

void test_render_passes_correct_dimensions() {
    IconComponent icon(0, 0, testBitmap8x8, 8, 8);
    icon.render(display);

    TEST_ASSERT_EQUAL_INT16(8, display.lastDrawBitmapW);
    TEST_ASSERT_EQUAL_INT16(8, display.lastDrawBitmapH);
}

void test_render_passes_bitmap_pointer() {
    IconComponent icon(0, 0, testBitmap8x8, 8, 8);
    icon.render(display);

    TEST_ASSERT_EQUAL_PTR(testBitmap8x8, display.lastDrawBitmapData);
}

void test_render_default_color_is_white() {
    IconComponent icon(0, 0, testBitmap8x8, 8, 8);
    icon.render(display);

    TEST_ASSERT_EQUAL_UINT16(SSD1306_WHITE, display.lastDrawBitmapColor);
}

void test_render_custom_color() {
    IconComponent icon(0, 0, testBitmap8x8, 8, 8, SSD1306_BLACK);
    icon.render(display);

    TEST_ASSERT_EQUAL_UINT16(SSD1306_BLACK, display.lastDrawBitmapColor);
}

// ============================================================
// render() — nullptr bitmap
// ============================================================

void test_render_nullptr_bitmap_does_not_draw() {
    IconComponent icon(10, 20, nullptr, 8, 8);
    icon.render(display);

    TEST_ASSERT_EQUAL(0, display.drawBitmapCallCount);
}

// ============================================================
// render() — various positions and dimensions
// ============================================================

void test_render_at_origin() {
    IconComponent icon(0, 0, testBitmap8x8, 8, 8);
    icon.render(display);

    TEST_ASSERT_EQUAL_INT16(0, display.lastDrawBitmapX);
    TEST_ASSERT_EQUAL_INT16(0, display.lastDrawBitmapY);
}

void test_render_different_dimensions() {
    // 16x16 bitmap (32 bytes needed, but mock doesn't read data)
    static const uint8_t bitmap16x16[32] = {0};
    IconComponent icon(5, 10, bitmap16x16, 16, 16);
    icon.render(display);

    TEST_ASSERT_EQUAL_INT16(16, display.lastDrawBitmapW);
    TEST_ASSERT_EQUAL_INT16(16, display.lastDrawBitmapH);
}

void test_render_at_display_edge() {
    IconComponent icon(120, 56, testBitmap8x8, 8, 8);
    icon.render(display);

    TEST_ASSERT_EQUAL(1, display.drawBitmapCallCount);
    TEST_ASSERT_EQUAL_INT16(120, display.lastDrawBitmapX);
    TEST_ASSERT_EQUAL_INT16(56, display.lastDrawBitmapY);
}

// --- Runner ---

int main() {
    UNITY_BEGIN();

    // Basic render
    RUN_TEST(test_render_calls_drawBitmap);
    RUN_TEST(test_render_passes_correct_position);
    RUN_TEST(test_render_passes_correct_dimensions);
    RUN_TEST(test_render_passes_bitmap_pointer);
    RUN_TEST(test_render_default_color_is_white);
    RUN_TEST(test_render_custom_color);

    // nullptr bitmap
    RUN_TEST(test_render_nullptr_bitmap_does_not_draw);

    // Position and dimensions
    RUN_TEST(test_render_at_origin);
    RUN_TEST(test_render_different_dimensions);
    RUN_TEST(test_render_at_display_edge);

    return UNITY_END();
}
