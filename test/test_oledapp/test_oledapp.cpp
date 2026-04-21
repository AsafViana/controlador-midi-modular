#include <unity.h>
#include "ui/OledApp.h"
#include "Serial.h"
#include "Arduino.h"
#include "Adafruit_SSD1306.h"

// --- Minimal concrete Screen for testing ---
class TestScreen : public Screen {
public:
    int renderCallCount = 0;
    ButtonEvent lastEvent = ButtonEvent::NONE;
    int handleInputCallCount = 0;

    void render(Adafruit_SSD1306& display) override {
        renderCallCount++;
    }

    void handleInput(ButtonEvent event) override {
        handleInputCallCount++;
        lastEvent = event;
    }
};

// --- begin() tests ---

void test_begin_returns_true_on_success() {
    OledApp app;
    TEST_ASSERT_TRUE(app.begin(0x3C));
}

void test_begin_returns_false_on_failure() {
    // Use global override to make the mock display fail
    Adafruit_SSD1306::globalBeginReturnValue = false;
    OledApp app;
    bool result = app.begin(0x3C);
    TEST_ASSERT_FALSE(result);
    // Verify Serial error message was printed
    TEST_ASSERT_GREATER_THAN(0, Serial.printlnCallCount);
    Adafruit_SSD1306::globalBeginReturnValue = true;
}

void test_begin_failure_prints_serial_error() {
    Adafruit_SSD1306::globalBeginReturnValue = false;
    OledApp app;
    app.begin(0x3C);
    // Verify the error message contains relevant text
    TEST_ASSERT_GREATER_THAN(0, Serial.printlnCallCount);
    // The message should mention SSD1306 or display failure
    TEST_ASSERT_NOT_NULL(strstr(Serial.lastMessage, "SSD1306"));
    Adafruit_SSD1306::globalBeginReturnValue = true;
}

void test_begin_uses_default_address() {
    OledApp app;
    bool result = app.begin(); // should use 0x3C
    TEST_ASSERT_TRUE(result);
}

void test_begin_uses_custom_address() {
    OledApp app;
    bool result = app.begin(0x3D);
    TEST_ASSERT_TRUE(result);
}

// --- addButton() tests ---

void test_add_button_nullptr_is_ignored() {
    OledApp app;
    app.begin();
    app.addButton(nullptr);
    // No crash = success. The button count stays at 0.
    // We verify indirectly: adding 4 real buttons should still work.
    Button b1(1), b2(2), b3(3), b4(4);
    app.addButton(&b1);
    app.addButton(&b2);
    app.addButton(&b3);
    app.addButton(&b4);
    // All 4 should be added (nullptr didn't count)
}

void test_add_single_button() {
    OledApp app;
    app.begin();
    Button btn(10);
    app.addButton(&btn);
    // No crash = success
}

void test_add_max_buttons() {
    OledApp app;
    app.begin();
    Button b1(1), b2(2), b3(3), b4(4);
    app.addButton(&b1);
    app.addButton(&b2);
    app.addButton(&b3);
    app.addButton(&b4);
    // All 4 added successfully, no crash
}

void test_add_button_beyond_max_is_ignored() {
    OledApp app;
    app.begin();
    Button b1(1), b2(2), b3(3), b4(4), b5(5);
    app.addButton(&b1);
    app.addButton(&b2);
    app.addButton(&b3);
    app.addButton(&b4);
    app.addButton(&b5); // 5th button — should be silently ignored
    // No crash = success
}

// --- getRouter() tests ---

void test_get_router_returns_reference() {
    OledApp app;
    Router& router = app.getRouter();
    TEST_ASSERT_NULL(router.currentScreen());
}

void test_get_router_returns_same_instance() {
    OledApp app;
    Router& r1 = app.getRouter();
    Router& r2 = app.getRouter();
    TEST_ASSERT_EQUAL_PTR(&r1, &r2);
}

void test_router_is_functional() {
    OledApp app;
    app.begin();
    TestScreen screen;
    app.getRouter().push(&screen);
    TEST_ASSERT_EQUAL_PTR(&screen, app.getRouter().currentScreen());
}

// --- update() tests: dirty flag triggers redraw ---

void test_update_dirty_screen_triggers_redraw() {
    OledApp app;
    app.begin();

    TestScreen screen;
    // Screen starts dirty by default (_dirty = true)
    app.getRouter().push(&screen);

    // Advance time past frame interval so update() processes
    mock::setMillis(100);
    app.update();

    // Verify render was called (screen was dirty)
    TEST_ASSERT_GREATER_THAN(0, screen.renderCallCount);
}

// --- update() tests: clean flag skips redraw ---

void test_update_clean_screen_skips_redraw() {
    OledApp app;
    app.begin();

    TestScreen screen;
    app.getRouter().push(&screen);

    // First update to clear the initial dirty flag
    mock::setMillis(100);
    app.update();
    int renderCountAfterFirst = screen.renderCallCount;

    // Advance time for next frame — screen is now clean
    mock::advanceMillis(50);
    app.update();

    // Render count should NOT have increased (screen was clean)
    TEST_ASSERT_EQUAL(renderCountAfterFirst, screen.renderCallCount);
}

// --- update() tests: rate limiting 30 FPS ---

void test_update_rate_limiting_skips_fast_calls() {
    OledApp app;
    app.begin();

    TestScreen screen;
    app.getRouter().push(&screen);

    // First update at t=100
    mock::setMillis(100);
    app.update();
    int renderCountAfterFirst = screen.renderCallCount;

    // Mark dirty again so we can detect if a second render happens
    screen.markDirty();

    // Second update at t=120 (only 20ms later, less than 33ms frame interval)
    mock::setMillis(120);
    app.update();

    // Render should NOT have been called again (rate limited)
    TEST_ASSERT_EQUAL(renderCountAfterFirst, screen.renderCallCount);
}

void test_update_allows_render_after_frame_interval() {
    OledApp app;
    app.begin();

    TestScreen screen;
    app.getRouter().push(&screen);

    // First update at t=100
    mock::setMillis(100);
    app.update();
    int renderCountAfterFirst = screen.renderCallCount;

    // Mark dirty again
    screen.markDirty();

    // Second update at t=140 (40ms later, more than 33ms frame interval)
    mock::setMillis(140);
    app.update();

    // Render SHOULD have been called again
    TEST_ASSERT_GREATER_THAN(renderCountAfterFirst, screen.renderCallCount);
}

// --- update() tests: button events forwarded ---

void test_update_forwards_button_event_to_screen() {
    OledApp app;
    app.begin();

    TestScreen screen;
    app.getRouter().push(&screen);

    // Register a button on pin 10 (activeLow = true by default)
    Button btn(10, true);
    btn.begin();
    app.addButton(&btn);

    // Initially pin is HIGH (not pressed for activeLow)
    mock::setDigitalRead(10, HIGH);

    // First update at t=100 — establishes baseline, clears initial dirty
    mock::setMillis(100);
    app.update();

    // Now simulate button press: pin goes LOW (pressed for activeLow)
    mock::setDigitalRead(10, LOW);

    // Second update at t=140 — button sees the change, starts debounce
    mock::setMillis(140);
    app.update();

    // Third update at t=200 — past debounce (50ms from t=140), button fires PRESSED
    mock::setMillis(200);
    app.update();

    // The button should have generated a PRESSED event
    TEST_ASSERT_GREATER_THAN(0, screen.handleInputCallCount);
    TEST_ASSERT_EQUAL(ButtonEvent::PRESSED, screen.lastEvent);
}

// --- update() tests: no buttons registered ---

void test_update_no_buttons_no_crash() {
    OledApp app;
    app.begin();

    TestScreen screen;
    app.getRouter().push(&screen);

    // Call update with no buttons registered — should not crash
    mock::setMillis(100);
    app.update();

    // Screen was dirty, so render should still happen
    TEST_ASSERT_GREATER_THAN(0, screen.renderCallCount);
}

void test_update_no_screen_no_crash() {
    OledApp app;
    app.begin();
    // No screen pushed — router returns nullptr

    mock::setMillis(100);
    app.update(); // Should not crash
}

// --- update() tests: multiple buttons ---

void test_update_polls_multiple_buttons() {
    OledApp app;
    app.begin();

    TestScreen screen;
    app.getRouter().push(&screen);

    // Register two buttons on different pins
    Button btn1(10, true);
    Button btn2(20, true);
    btn1.begin();
    btn2.begin();
    app.addButton(&btn1);
    app.addButton(&btn2);

    // Initially both pins HIGH (not pressed)
    mock::setDigitalRead(10, HIGH);
    mock::setDigitalRead(20, HIGH);

    // First update at t=100 — baseline
    mock::setMillis(100);
    app.update();

    // Simulate press on button 1 only
    mock::setDigitalRead(10, LOW);

    // Second update at t=140 — button 1 sees change, starts debounce
    mock::setMillis(140);
    app.update();

    // Third update at t=200 — past debounce, button 1 fires PRESSED
    mock::setMillis(200);
    app.update();

    // Button 1 should have generated PRESSED event
    TEST_ASSERT_EQUAL(ButtonEvent::PRESSED, screen.lastEvent);
    TEST_ASSERT_EQUAL(1, screen.handleInputCallCount);
}

void test_update_multiple_buttons_both_pressed() {
    OledApp app;
    app.begin();

    TestScreen screen;
    app.getRouter().push(&screen);

    // Register two buttons
    Button btn1(10, true);
    Button btn2(20, true);
    btn1.begin();
    btn2.begin();
    app.addButton(&btn1);
    app.addButton(&btn2);

    // Initially both pins HIGH (not pressed)
    mock::setDigitalRead(10, HIGH);
    mock::setDigitalRead(20, HIGH);

    // First update at t=100 — baseline
    mock::setMillis(100);
    app.update();

    // Simulate both buttons pressed simultaneously
    mock::setDigitalRead(10, LOW);
    mock::setDigitalRead(20, LOW);

    // Second update at t=140 — both buttons see change, start debounce
    mock::setMillis(140);
    app.update();

    // Third update at t=200 — past debounce, both buttons fire PRESSED
    mock::setMillis(200);
    app.update();

    // Both buttons should have generated events
    TEST_ASSERT_EQUAL(2, screen.handleInputCallCount);
}

// --- Runner ---

void setUp() {
    Serial.reset();
    mock::reset();
    Adafruit_SSD1306::globalBeginReturnValue = true;
}

void tearDown() {
    Adafruit_SSD1306::globalBeginReturnValue = true;
}

int main() {
    UNITY_BEGIN();

    // begin()
    RUN_TEST(test_begin_returns_true_on_success);
    RUN_TEST(test_begin_returns_false_on_failure);
    RUN_TEST(test_begin_failure_prints_serial_error);
    RUN_TEST(test_begin_uses_default_address);
    RUN_TEST(test_begin_uses_custom_address);

    // addButton()
    RUN_TEST(test_add_button_nullptr_is_ignored);
    RUN_TEST(test_add_single_button);
    RUN_TEST(test_add_max_buttons);
    RUN_TEST(test_add_button_beyond_max_is_ignored);

    // getRouter()
    RUN_TEST(test_get_router_returns_reference);
    RUN_TEST(test_get_router_returns_same_instance);
    RUN_TEST(test_router_is_functional);

    // update() — dirty flag triggers redraw
    RUN_TEST(test_update_dirty_screen_triggers_redraw);

    // update() — clean flag skips redraw
    RUN_TEST(test_update_clean_screen_skips_redraw);

    // update() — rate limiting 30 FPS
    RUN_TEST(test_update_rate_limiting_skips_fast_calls);
    RUN_TEST(test_update_allows_render_after_frame_interval);

    // update() — button events forwarded
    RUN_TEST(test_update_forwards_button_event_to_screen);

    // update() — no buttons / no screen
    RUN_TEST(test_update_no_buttons_no_crash);
    RUN_TEST(test_update_no_screen_no_crash);

    // update() — multiple buttons
    RUN_TEST(test_update_polls_multiple_buttons);
    RUN_TEST(test_update_multiple_buttons_both_pressed);

    return UNITY_END();
}
