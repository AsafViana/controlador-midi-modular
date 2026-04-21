#include <unity.h>
#include "ui/Screen.h"
#include "ui/UIComponent.h"

// --- Mock Adafruit_SSD1306 (minimal, just enough for Screen tests) ---
class Adafruit_SSD1306 {};

// --- Mock UIComponent that tracks render calls ---
class MockComponent : public UIComponent {
public:
    int renderCallCount = 0;
    void render(Adafruit_SSD1306& display) override { renderCallCount++; }
};

// --- Concrete Screen for testing ---
class TestScreen : public Screen {
public:
    int renderCount = 0;
    int mountCount = 0;
    int unmountCount = 0;
    ButtonEvent lastEvent = ButtonEvent::NONE;

    void onMount() override { mountCount++; }
    void onUnmount() override { unmountCount++; }
    void render(Adafruit_SSD1306& display) override { renderCount++; }
    void handleInput(ButtonEvent event) override { lastEvent = event; }
};

// --- Dirty flag tests ---

void test_dirty_flag_starts_true() {
    TestScreen screen;
    TEST_ASSERT_TRUE(screen.isDirty());
}

void test_clear_dirty_sets_false() {
    TestScreen screen;
    screen.clearDirty();
    TEST_ASSERT_FALSE(screen.isDirty());
}

void test_mark_dirty_sets_true() {
    TestScreen screen;
    screen.clearDirty();
    TEST_ASSERT_FALSE(screen.isDirty());
    screen.markDirty();
    TEST_ASSERT_TRUE(screen.isDirty());
}

void test_mark_dirty_after_clear_cycle() {
    TestScreen screen;
    // Initial: dirty
    TEST_ASSERT_TRUE(screen.isDirty());
    // Clear
    screen.clearDirty();
    TEST_ASSERT_FALSE(screen.isDirty());
    // Mark dirty again
    screen.markDirty();
    TEST_ASSERT_TRUE(screen.isDirty());
    // Clear again
    screen.clearDirty();
    TEST_ASSERT_FALSE(screen.isDirty());
}

// --- Lifecycle default behavior tests ---

void test_default_on_mount_does_nothing() {
    // A Screen subclass that does NOT override onMount
    class MinimalScreen : public Screen {
        void render(Adafruit_SSD1306& display) override {}
    };
    MinimalScreen screen;
    screen.onMount(); // Should not crash
}

void test_default_on_unmount_does_nothing() {
    class MinimalScreen : public Screen {
        void render(Adafruit_SSD1306& display) override {}
    };
    MinimalScreen screen;
    screen.onUnmount(); // Should not crash
}

void test_default_handle_input_does_nothing() {
    class MinimalScreen : public Screen {
        void render(Adafruit_SSD1306& display) override {}
    };
    MinimalScreen screen;
    screen.handleInput(ButtonEvent::SINGLE_CLICK); // Should not crash
}

// --- Overridden lifecycle tests ---

void test_on_mount_called() {
    TestScreen screen;
    screen.onMount();
    TEST_ASSERT_EQUAL_INT(1, screen.mountCount);
}

void test_on_unmount_called() {
    TestScreen screen;
    screen.onUnmount();
    TEST_ASSERT_EQUAL_INT(1, screen.unmountCount);
}

void test_render_called() {
    TestScreen screen;
    Adafruit_SSD1306 display;
    screen.render(display);
    TEST_ASSERT_EQUAL_INT(1, screen.renderCount);
}

void test_handle_input_receives_event() {
    TestScreen screen;
    screen.handleInput(ButtonEvent::LONG_PRESS);
    TEST_ASSERT_EQUAL(ButtonEvent::LONG_PRESS, screen.lastEvent);
}

// --- Child component tests ---

void test_add_child_returns_true() {
    Screen screen;
    MockComponent comp;
    TEST_ASSERT_TRUE(screen.addChild(&comp));
}

void test_add_child_nullptr_returns_false() {
    Screen screen;
    TEST_ASSERT_FALSE(screen.addChild(nullptr));
}

void test_add_child_increments_count() {
    Screen screen;
    MockComponent comp1, comp2;
    TEST_ASSERT_EQUAL_UINT8(0, screen.getChildCount());
    screen.addChild(&comp1);
    TEST_ASSERT_EQUAL_UINT8(1, screen.getChildCount());
    screen.addChild(&comp2);
    TEST_ASSERT_EQUAL_UINT8(2, screen.getChildCount());
}

void test_add_child_nullptr_does_not_increment_count() {
    Screen screen;
    screen.addChild(nullptr);
    TEST_ASSERT_EQUAL_UINT8(0, screen.getChildCount());
}

void test_add_child_beyond_max_returns_false() {
    Screen screen;
    MockComponent comps[Screen::MAX_CHILDREN + 1];
    for (uint8_t i = 0; i < Screen::MAX_CHILDREN; i++) {
        TEST_ASSERT_TRUE(screen.addChild(&comps[i]));
    }
    // One more should fail
    TEST_ASSERT_FALSE(screen.addChild(&comps[Screen::MAX_CHILDREN]));
    TEST_ASSERT_EQUAL_UINT8(Screen::MAX_CHILDREN, screen.getChildCount());
}

void test_base_render_calls_children() {
    Screen screen;
    MockComponent comp1, comp2, comp3;
    screen.addChild(&comp1);
    screen.addChild(&comp2);
    screen.addChild(&comp3);

    Adafruit_SSD1306 display;
    screen.render(display);

    TEST_ASSERT_EQUAL_INT(1, comp1.renderCallCount);
    TEST_ASSERT_EQUAL_INT(1, comp2.renderCallCount);
    TEST_ASSERT_EQUAL_INT(1, comp3.renderCallCount);
}

void test_base_render_no_children_no_crash() {
    Screen screen;
    Adafruit_SSD1306 display;
    screen.render(display); // Should not crash
}

void test_base_render_calls_children_in_order() {
    // Use a shared counter to verify call order
    static int callOrder;
    callOrder = 0;

    class OrderedComponent : public UIComponent {
    public:
        int myOrder = -1;
        void render(Adafruit_SSD1306& display) override {
            myOrder = callOrder++;
        }
    };

    Screen screen;
    OrderedComponent comp1, comp2, comp3;
    screen.addChild(&comp1);
    screen.addChild(&comp2);
    screen.addChild(&comp3);

    Adafruit_SSD1306 display;
    screen.render(display);

    TEST_ASSERT_EQUAL_INT(0, comp1.myOrder);
    TEST_ASSERT_EQUAL_INT(1, comp2.myOrder);
    TEST_ASSERT_EQUAL_INT(2, comp3.myOrder);
}

void test_subclass_render_can_call_base() {
    // A Screen subclass that calls the base render and adds its own logic
    class CompositeScreen : public Screen {
    public:
        int customRenderCount = 0;
        void render(Adafruit_SSD1306& display) override {
            Screen::render(display); // Call base to render children
            customRenderCount++;
        }
    };

    CompositeScreen screen;
    MockComponent comp;
    screen.addChild(&comp);

    Adafruit_SSD1306 display;
    screen.render(display);

    TEST_ASSERT_EQUAL_INT(1, comp.renderCallCount);
    TEST_ASSERT_EQUAL_INT(1, screen.customRenderCount);
}

// --- Runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    // Dirty flag
    RUN_TEST(test_dirty_flag_starts_true);
    RUN_TEST(test_clear_dirty_sets_false);
    RUN_TEST(test_mark_dirty_sets_true);
    RUN_TEST(test_mark_dirty_after_clear_cycle);

    // Default lifecycle (no crash)
    RUN_TEST(test_default_on_mount_does_nothing);
    RUN_TEST(test_default_on_unmount_does_nothing);
    RUN_TEST(test_default_handle_input_does_nothing);

    // Overridden lifecycle
    RUN_TEST(test_on_mount_called);
    RUN_TEST(test_on_unmount_called);
    RUN_TEST(test_render_called);
    RUN_TEST(test_handle_input_receives_event);

    // Child components
    RUN_TEST(test_add_child_returns_true);
    RUN_TEST(test_add_child_nullptr_returns_false);
    RUN_TEST(test_add_child_increments_count);
    RUN_TEST(test_add_child_nullptr_does_not_increment_count);
    RUN_TEST(test_add_child_beyond_max_returns_false);
    RUN_TEST(test_base_render_calls_children);
    RUN_TEST(test_base_render_no_children_no_crash);
    RUN_TEST(test_base_render_calls_children_in_order);
    RUN_TEST(test_subclass_render_can_call_base);

    return UNITY_END();
}
