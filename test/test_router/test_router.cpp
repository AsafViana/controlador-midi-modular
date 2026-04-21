#include <unity.h>
#include "ui/Router.h"

// --- Mock Adafruit_SSD1306 (minimal) ---
class Adafruit_SSD1306 {};

// --- Concrete Screen for testing ---
class TestScreen : public Screen {
public:
    int mountCount = 0;
    int unmountCount = 0;
    ButtonEvent lastEvent = ButtonEvent::NONE;

    void onMount() override { mountCount++; }
    void onUnmount() override { unmountCount++; }
    void render(Adafruit_SSD1306& display) override {}
    void handleInput(ButtonEvent event) override { lastEvent = event; }
};

// --- currentScreen tests ---

void test_current_screen_returns_nullptr_when_empty() {
    Router router;
    TEST_ASSERT_NULL(router.currentScreen());
}

void test_current_screen_returns_pushed_screen() {
    Router router;
    TestScreen screen;
    router.push(&screen);
    TEST_ASSERT_EQUAL_PTR(&screen, router.currentScreen());
}

// --- push tests ---

void test_push_calls_on_mount() {
    Router router;
    TestScreen screen;
    router.push(&screen);
    TEST_ASSERT_EQUAL_INT(1, screen.mountCount);
}

void test_push_calls_on_unmount_of_previous() {
    Router router;
    TestScreen screenA;
    TestScreen screenB;
    router.push(&screenA);
    router.push(&screenB);
    TEST_ASSERT_EQUAL_INT(1, screenA.unmountCount);
    TEST_ASSERT_EQUAL_INT(1, screenB.mountCount);
}

void test_push_when_full_is_ignored() {
    Router router;
    TestScreen screens[9];
    // Push 8 screens (MAX_STACK_SIZE)
    for (int i = 0; i < 8; i++) {
        router.push(&screens[i]);
    }
    // 9th push should be silently ignored
    router.push(&screens[8]);
    TEST_ASSERT_EQUAL_PTR(&screens[7], router.currentScreen());
    TEST_ASSERT_EQUAL_INT(0, screens[8].mountCount);
}

void test_push_nullptr_is_ignored() {
    Router router;
    TestScreen screen;
    router.push(&screen);
    router.push(nullptr);
    TEST_ASSERT_EQUAL_PTR(&screen, router.currentScreen());
}

// --- pop tests ---

void test_pop_returns_to_previous_screen() {
    Router router;
    TestScreen screenA;
    TestScreen screenB;
    router.push(&screenA);
    router.push(&screenB);
    router.pop();
    TEST_ASSERT_EQUAL_PTR(&screenA, router.currentScreen());
}

void test_pop_calls_unmount_on_current_and_mount_on_previous() {
    Router router;
    TestScreen screenA;
    TestScreen screenB;
    router.push(&screenA);
    router.push(&screenB);

    // Reset counts after push sequence
    screenA.mountCount = 0;
    screenB.unmountCount = 0;

    router.pop();
    TEST_ASSERT_EQUAL_INT(1, screenB.unmountCount);
    TEST_ASSERT_EQUAL_INT(1, screenA.mountCount);
}

void test_pop_with_single_screen_is_ignored() {
    Router router;
    TestScreen screen;
    router.push(&screen);

    // Reset counts
    screen.mountCount = 0;
    screen.unmountCount = 0;

    router.pop();
    TEST_ASSERT_EQUAL_PTR(&screen, router.currentScreen());
    TEST_ASSERT_EQUAL_INT(0, screen.unmountCount);
    TEST_ASSERT_EQUAL_INT(0, screen.mountCount);
}

void test_pop_with_empty_stack_is_ignored() {
    Router router;
    router.pop(); // Should not crash
    TEST_ASSERT_NULL(router.currentScreen());
}

// --- navigateTo tests ---

void test_navigate_to_replaces_stack() {
    Router router;
    TestScreen screenA;
    TestScreen screenB;
    TestScreen screenC;
    router.push(&screenA);
    router.push(&screenB);
    router.navigateTo(&screenC);
    TEST_ASSERT_EQUAL_PTR(&screenC, router.currentScreen());
}

void test_navigate_to_calls_unmount_on_current_and_mount_on_new() {
    Router router;
    TestScreen screenA;
    TestScreen screenB;
    router.push(&screenA);

    // Reset counts
    screenA.unmountCount = 0;

    router.navigateTo(&screenB);
    TEST_ASSERT_EQUAL_INT(1, screenA.unmountCount);
    TEST_ASSERT_EQUAL_INT(1, screenB.mountCount);
}

void test_navigate_to_from_empty_stack() {
    Router router;
    TestScreen screen;
    router.navigateTo(&screen);
    TEST_ASSERT_EQUAL_PTR(&screen, router.currentScreen());
    TEST_ASSERT_EQUAL_INT(1, screen.mountCount);
}

void test_navigate_to_stack_size_is_one() {
    Router router;
    TestScreen screens[5];
    for (int i = 0; i < 5; i++) {
        router.push(&screens[i]);
    }
    TestScreen target;
    router.navigateTo(&target);
    TEST_ASSERT_EQUAL_PTR(&target, router.currentScreen());

    // Pop should be ignored since stack has only 1 screen
    router.pop();
    TEST_ASSERT_EQUAL_PTR(&target, router.currentScreen());
}

void test_navigate_to_nullptr_is_ignored() {
    Router router;
    TestScreen screen;
    router.push(&screen);
    router.navigateTo(nullptr);
    TEST_ASSERT_EQUAL_PTR(&screen, router.currentScreen());
}

// --- handleInput tests ---

void test_handle_input_forwards_to_current_screen() {
    Router router;
    TestScreen screen;
    router.push(&screen);
    router.handleInput(ButtonEvent::SINGLE_CLICK);
    TEST_ASSERT_EQUAL(ButtonEvent::SINGLE_CLICK, screen.lastEvent);
}

void test_handle_input_with_empty_stack_does_not_crash() {
    Router router;
    router.handleInput(ButtonEvent::PRESSED); // Should not crash
}

void test_handle_input_forwards_various_events() {
    Router router;
    TestScreen screen;
    router.push(&screen);

    router.handleInput(ButtonEvent::LONG_PRESS);
    TEST_ASSERT_EQUAL(ButtonEvent::LONG_PRESS, screen.lastEvent);

    router.handleInput(ButtonEvent::DOUBLE_CLICK);
    TEST_ASSERT_EQUAL(ButtonEvent::DOUBLE_CLICK, screen.lastEvent);
}

// --- Runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    // currentScreen
    RUN_TEST(test_current_screen_returns_nullptr_when_empty);
    RUN_TEST(test_current_screen_returns_pushed_screen);

    // push
    RUN_TEST(test_push_calls_on_mount);
    RUN_TEST(test_push_calls_on_unmount_of_previous);
    RUN_TEST(test_push_when_full_is_ignored);
    RUN_TEST(test_push_nullptr_is_ignored);

    // pop
    RUN_TEST(test_pop_returns_to_previous_screen);
    RUN_TEST(test_pop_calls_unmount_on_current_and_mount_on_previous);
    RUN_TEST(test_pop_with_single_screen_is_ignored);
    RUN_TEST(test_pop_with_empty_stack_is_ignored);

    // navigateTo
    RUN_TEST(test_navigate_to_replaces_stack);
    RUN_TEST(test_navigate_to_calls_unmount_on_current_and_mount_on_new);
    RUN_TEST(test_navigate_to_from_empty_stack);
    RUN_TEST(test_navigate_to_stack_size_is_one);
    RUN_TEST(test_navigate_to_nullptr_is_ignored);

    // handleInput
    RUN_TEST(test_handle_input_forwards_to_current_screen);
    RUN_TEST(test_handle_input_with_empty_stack_does_not_crash);
    RUN_TEST(test_handle_input_forwards_various_events);

    return UNITY_END();
}
