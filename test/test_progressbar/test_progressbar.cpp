#include <unity.h>
#include "Adafruit_SSD1306.h"
#include "Wire.h"
#include "ui/components/ProgressBarComponent.h"

// --- Test display instance ---
static Adafruit_SSD1306 display(128, 64, &Wire, -1);

// --- setUp / tearDown ---

void setUp() {
    display.reset();
}

void tearDown() {}

// ============================================================
// setValue() — clamping behavior
// ============================================================

void test_default_value_is_zero() {
    ProgressBarComponent bar(0, 0, 100, 10);
    TEST_ASSERT_EQUAL_UINT8(0, bar.getValue());
}

void test_setValue_stores_value() {
    ProgressBarComponent bar(0, 0, 100, 10);
    bar.setValue(50);
    TEST_ASSERT_EQUAL_UINT8(50, bar.getValue());
}

void test_setValue_clamps_above_100() {
    ProgressBarComponent bar(0, 0, 100, 10);
    bar.setValue(150);
    TEST_ASSERT_EQUAL_UINT8(100, bar.getValue());
}

void test_setValue_clamps_255_to_100() {
    ProgressBarComponent bar(0, 0, 100, 10);
    bar.setValue(255);
    TEST_ASSERT_EQUAL_UINT8(100, bar.getValue());
}

void test_setValue_zero_stays_zero() {
    ProgressBarComponent bar(0, 0, 100, 10);
    bar.setValue(0);
    TEST_ASSERT_EQUAL_UINT8(0, bar.getValue());
}

void test_setValue_100_stays_100() {
    ProgressBarComponent bar(0, 0, 100, 10);
    bar.setValue(100);
    TEST_ASSERT_EQUAL_UINT8(100, bar.getValue());
}

// ============================================================
// render() — border drawing
// ============================================================

void test_render_draws_border() {
    ProgressBarComponent bar(10, 20, 80, 12);
    bar.setValue(50);
    bar.render(display);

    TEST_ASSERT_EQUAL(1, display.drawRectCallCount);
    TEST_ASSERT_EQUAL_INT16(10, display.lastDrawRectX);
    TEST_ASSERT_EQUAL_INT16(20, display.lastDrawRectY);
    TEST_ASSERT_EQUAL_INT16(80, display.lastDrawRectW);
    TEST_ASSERT_EQUAL_INT16(12, display.lastDrawRectH);
    TEST_ASSERT_EQUAL_UINT16(SSD1306_WHITE, display.lastDrawRectColor);
}

void test_render_draws_border_at_value_zero() {
    ProgressBarComponent bar(0, 0, 100, 10);
    bar.setValue(0);
    bar.render(display);

    // Border is always drawn
    TEST_ASSERT_EQUAL(1, display.drawRectCallCount);
}

// ============================================================
// render() — fill drawing
// ============================================================

void test_render_no_fill_at_value_zero() {
    ProgressBarComponent bar(0, 0, 100, 10);
    bar.setValue(0);
    bar.render(display);

    // No fill when value is 0
    TEST_ASSERT_EQUAL(0, display.fillRectCallCount);
}

void test_render_fill_at_value_50() {
    // width=102 → innerWidth = 100, value=50 → fillWidth = (50*100)/100 = 50
    ProgressBarComponent bar(0, 0, 102, 10);
    bar.setValue(50);
    bar.render(display);

    TEST_ASSERT_EQUAL(1, display.fillRectCallCount);
    TEST_ASSERT_EQUAL_INT16(1, display.lastFillRectX);   // x + 1
    TEST_ASSERT_EQUAL_INT16(1, display.lastFillRectY);   // y + 1
    TEST_ASSERT_EQUAL_INT16(50, display.lastFillRectW);  // fillWidth
    TEST_ASSERT_EQUAL_INT16(8, display.lastFillRectH);   // h - 2
    TEST_ASSERT_EQUAL_UINT16(SSD1306_WHITE, display.lastFillRectColor);
}

void test_render_fill_at_value_100() {
    // width=102 → innerWidth = 100, value=100 → fillWidth = (100*100)/100 = 100
    ProgressBarComponent bar(0, 0, 102, 10);
    bar.setValue(100);
    bar.render(display);

    TEST_ASSERT_EQUAL(1, display.fillRectCallCount);
    TEST_ASSERT_EQUAL_INT16(100, display.lastFillRectW); // fills entire inner width
}

void test_render_fill_position_offset() {
    // Bar at (10, 20), fill should start at (11, 21)
    ProgressBarComponent bar(10, 20, 102, 12);
    bar.setValue(50);
    bar.render(display);

    TEST_ASSERT_EQUAL_INT16(11, display.lastFillRectX);  // x + 1
    TEST_ASSERT_EQUAL_INT16(21, display.lastFillRectY);  // y + 1
    TEST_ASSERT_EQUAL_INT16(10, display.lastFillRectH);  // h - 2
}

void test_render_fill_formula_correctness() {
    // width=52 → innerWidth = 50, value=75 → fillWidth = (75*50)/100 = 37
    ProgressBarComponent bar(0, 0, 52, 10);
    bar.setValue(75);
    bar.render(display);

    TEST_ASSERT_EQUAL_INT16(37, display.lastFillRectW);
}

// --- Runner ---

int main() {
    UNITY_BEGIN();

    // setValue clamping
    RUN_TEST(test_default_value_is_zero);
    RUN_TEST(test_setValue_stores_value);
    RUN_TEST(test_setValue_clamps_above_100);
    RUN_TEST(test_setValue_clamps_255_to_100);
    RUN_TEST(test_setValue_zero_stays_zero);
    RUN_TEST(test_setValue_100_stays_100);

    // render — border
    RUN_TEST(test_render_draws_border);
    RUN_TEST(test_render_draws_border_at_value_zero);

    // render — fill
    RUN_TEST(test_render_no_fill_at_value_zero);
    RUN_TEST(test_render_fill_at_value_50);
    RUN_TEST(test_render_fill_at_value_100);
    RUN_TEST(test_render_fill_position_offset);
    RUN_TEST(test_render_fill_formula_correctness);

    return UNITY_END();
}
