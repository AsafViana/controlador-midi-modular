#include <unity.h>
#include "ui/State.h"

// --- Mock Adafruit_SSD1306 (minimal, just enough for Screen tests) ---
class Adafruit_SSD1306 {};

// --- Concrete Screen for testing dirty flag ---
class DirtyTracker : public Screen {
public:
    void render(Adafruit_SSD1306& display) override {}
};

// --- User-defined struct with operator!= ---
struct Point {
    int x;
    int y;
    bool operator!=(const Point& other) const {
        return x != other.x || y != other.y;
    }
};

// --- Construction and get() ---

void test_state_int_initial_value() {
    DirtyTracker screen;
    State<int> state(&screen, 42);
    TEST_ASSERT_EQUAL_INT(42, state.get());
}

void test_state_bool_initial_value() {
    DirtyTracker screen;
    State<bool> state(&screen, false);
    TEST_ASSERT_FALSE(state.get());
}

void test_state_float_initial_value() {
    DirtyTracker screen;
    State<float> state(&screen, 3.14f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 3.14f, state.get());
}

void test_state_struct_initial_value() {
    DirtyTracker screen;
    Point p = {10, 20};
    State<Point> state(&screen, p);
    TEST_ASSERT_EQUAL_INT(10, state.get().x);
    TEST_ASSERT_EQUAL_INT(20, state.get().y);
}

// --- set() with different value marks dirty ---

void test_set_different_int_marks_dirty() {
    DirtyTracker screen;
    State<int> state(&screen, 0);
    screen.clearDirty(); // clear initial dirty
    state.set(1);
    TEST_ASSERT_TRUE(screen.isDirty());
}

void test_set_different_bool_marks_dirty() {
    DirtyTracker screen;
    State<bool> state(&screen, false);
    screen.clearDirty();
    state.set(true);
    TEST_ASSERT_TRUE(screen.isDirty());
}

void test_set_different_float_marks_dirty() {
    DirtyTracker screen;
    State<float> state(&screen, 1.0f);
    screen.clearDirty();
    state.set(2.0f);
    TEST_ASSERT_TRUE(screen.isDirty());
}

void test_set_different_struct_marks_dirty() {
    DirtyTracker screen;
    State<Point> state(&screen, {0, 0});
    screen.clearDirty();
    state.set({5, 10});
    TEST_ASSERT_TRUE(screen.isDirty());
}

// --- set() with same value does NOT mark dirty ---

void test_set_same_int_does_not_mark_dirty() {
    DirtyTracker screen;
    State<int> state(&screen, 42);
    screen.clearDirty();
    state.set(42);
    TEST_ASSERT_FALSE(screen.isDirty());
}

void test_set_same_bool_does_not_mark_dirty() {
    DirtyTracker screen;
    State<bool> state(&screen, true);
    screen.clearDirty();
    state.set(true);
    TEST_ASSERT_FALSE(screen.isDirty());
}

void test_set_same_float_does_not_mark_dirty() {
    DirtyTracker screen;
    State<float> state(&screen, 1.5f);
    screen.clearDirty();
    state.set(1.5f);
    TEST_ASSERT_FALSE(screen.isDirty());
}

void test_set_same_struct_does_not_mark_dirty() {
    DirtyTracker screen;
    State<Point> state(&screen, {3, 7});
    screen.clearDirty();
    state.set({3, 7});
    TEST_ASSERT_FALSE(screen.isDirty());
}

// --- set() updates the stored value ---

void test_set_updates_value() {
    DirtyTracker screen;
    State<int> state(&screen, 0);
    state.set(99);
    TEST_ASSERT_EQUAL_INT(99, state.get());
}

// --- null owner is safe ---

void test_null_owner_does_not_crash() {
    State<int> state(nullptr, 10);
    state.set(20); // should not crash
    TEST_ASSERT_EQUAL_INT(20, state.get());
}

// --- Runner ---

void setUp() {}
void tearDown() {}

int main() {
    UNITY_BEGIN();

    // Initial values
    RUN_TEST(test_state_int_initial_value);
    RUN_TEST(test_state_bool_initial_value);
    RUN_TEST(test_state_float_initial_value);
    RUN_TEST(test_state_struct_initial_value);

    // Different value marks dirty
    RUN_TEST(test_set_different_int_marks_dirty);
    RUN_TEST(test_set_different_bool_marks_dirty);
    RUN_TEST(test_set_different_float_marks_dirty);
    RUN_TEST(test_set_different_struct_marks_dirty);

    // Same value does NOT mark dirty
    RUN_TEST(test_set_same_int_does_not_mark_dirty);
    RUN_TEST(test_set_same_bool_does_not_mark_dirty);
    RUN_TEST(test_set_same_float_does_not_mark_dirty);
    RUN_TEST(test_set_same_struct_does_not_mark_dirty);

    // Value update
    RUN_TEST(test_set_updates_value);

    // Null owner safety
    RUN_TEST(test_null_owner_does_not_crash);

    return UNITY_END();
}
