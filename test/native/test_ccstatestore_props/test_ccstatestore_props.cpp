/**
 * Property-based tests for CCStateStore.
 *
 * Uses Rapidcheck with Unity test framework.
 * Tag: "Feature: bluetooth-remote-control"
 */

#include <rapidcheck.h>
#include <unity.h>

#include "ble/CCStateStore.h"

void setUp() {}
void tearDown() {}

/**
 * Property 2: Round trip consistency
 *
 * For any valid channel (1–16), controller number (0–127),
 * and value (0–127), writing the value to the CCStateStore via set()
 * and then reading it back via get() should return the exact value
 * that was written.
 *
 * **Validates: Requirements 2.1, 2.5**
 */
void test_property2_round_trip_consistency() {
  const auto result = rc::check(
      "Feature: bluetooth-remote-control, Property 2: Round trip consistency",
      []() {
        // Generate random valid inputs
        const auto channel = *rc::gen::inRange<uint8_t>(1, 17);     // 1–16
        const auto controller = *rc::gen::inRange<uint8_t>(0, 128); // 0–127
        const auto value = *rc::gen::inRange<uint8_t>(0, 128);      // 0–127

        // Fresh store for each test case
        CCStateStore store;
        store.begin();

        // Write the value
        bool setResult = store.set(channel, controller, value);
        RC_ASSERT(setResult == true);

        // Read it back
        int16_t readBack = store.get(channel, controller);

        // Must match the written value
        RC_ASSERT(readBack == static_cast<int16_t>(value));
      });

  TEST_ASSERT_TRUE_MESSAGE(result, "Property 2 failed: set() followed by get() "
                                   "did not return the written value");
}

/**
 * Property 3: Invalid coordinates rejection
 *
 * For any channel outside 1–16 or controller outside 0–127,
 * set() returns false and get() returns -1 without modifying
 * stored values.
 *
 * **Validates: Requirements 2.6, 4.5, 9.4**
 */
void test_property3_invalid_coordinates_rejection() {
  const auto result = rc::check(
      "Feature: bluetooth-remote-control, Property 3: Invalid coordinates "
      "rejection",
      []() {
        // Generate a valid value to attempt writing
        const auto value = *rc::gen::inRange<uint8_t>(0, 128); // 0–127

        // Generate an invalid channel (0 or 17–255) OR invalid controller
        // (128–255). We use a boolean to decide which dimension is invalid.
        const auto invalidateChannel = *rc::gen::arbitrary<bool>();

        uint8_t channel;
        uint8_t controller;

        if (invalidateChannel) {
          // Invalid channel: 0 or 17–255
          // Generate using uint16_t to avoid overflow, then cast
          const auto ch =
              *rc::gen::oneOf(rc::gen::just<uint16_t>(0),
                              rc::gen::inRange<uint16_t>(17, 256) // 17–255
              );
          channel = static_cast<uint8_t>(ch);
          // Controller can be anything (valid or invalid)
          controller = *rc::gen::arbitrary<uint8_t>();
        } else {
          // Valid channel, but invalid controller: 128–255
          channel = *rc::gen::inRange<uint8_t>(1, 17); // 1–16 (valid)
          const auto ctrl = *rc::gen::inRange<uint16_t>(128, 256); // 128–255
          controller = static_cast<uint8_t>(ctrl);
        }

        // Fresh store for each test case
        CCStateStore store;
        store.begin();

        // Pre-populate with a known valid value to verify no modification
        // Use a valid coordinate to set a sentinel value
        const uint8_t sentinelChannel = 1;
        const uint8_t sentinelController = 0;
        const uint8_t sentinelValue = 42;
        bool sentinelSet =
            store.set(sentinelChannel, sentinelController, sentinelValue);
        RC_ASSERT(sentinelSet);

        // Attempt set() with invalid coordinates — must return false
        bool setResult = store.set(channel, controller, value);
        RC_ASSERT(setResult == false);

        // Attempt get() with invalid coordinates — must return -1
        int16_t getResult = store.get(channel, controller);
        RC_ASSERT(getResult == -1);

        // Verify the sentinel value was not modified
        int16_t sentinelRead = store.get(sentinelChannel, sentinelController);
        RC_ASSERT(sentinelRead == static_cast<int16_t>(sentinelValue));
      });

  TEST_ASSERT_TRUE_MESSAGE(result,
                           "Property 3: Invalid coordinates rejection failed");
}

/**
 * Property 9: Bulk read ordered snapshot
 *
 * For any valid channel (1–16) and any state of the CCStateStore,
 * getChannelSnapshot() returns 128 bytes where byte[N] equals the
 * stored CC value for controller N on that channel, in ascending
 * controller-number order.
 *
 * **Validates: Requirements 9.3**
 */
void test_property9_bulk_read_ordered_snapshot() {
  const auto result = rc::check(
      "Feature: bluetooth-remote-control, Property 9: Bulk read ordered "
      "snapshot",
      []() {
        // Generate a random valid channel (1–16)
        const auto channel = *rc::gen::inRange<uint8_t>(1, 17); // [1, 16]

        // Generate a random array of 128 values (each 0–127)
        const auto values = *rc::gen::container<std::vector<uint8_t>>(
            128, rc::gen::inRange<uint8_t>(0, 128)); // [0, 127]

        // Fresh store for each iteration
        CCStateStore store;
        store.begin();

        // Set all 128 controllers for that channel to the generated values
        for (uint8_t ctrl = 0; ctrl < 128; ctrl++) {
          bool ok = store.set(channel, ctrl, values[ctrl]);
          RC_ASSERT(ok);
        }

        // Call getChannelSnapshot and verify
        uint8_t snapshot[128] = {};
        bool snapshotOk = store.getChannelSnapshot(channel, snapshot);
        RC_ASSERT(snapshotOk);

        // Assert each byte matches the corresponding set value
        for (uint8_t n = 0; n < 128; n++) {
          RC_ASSERT(snapshot[n] == values[n]);
        }
      });

  TEST_ASSERT_TRUE_MESSAGE(result,
                           "Property 9: Bulk read ordered snapshot failed");
}

/**
 * Property 8: Last-write-wins concurrency
 *
 * For any sequence of two writes to the same (channel, controller) with
 * values A then B, get() returns B.
 *
 * **Validates: Requirements 7.4**
 */
void test_property8_last_write_wins_concurrency() {
  const auto result = rc::check(
      "Feature: bluetooth-remote-control, Property 8: Last-write-wins "
      "concurrency",
      []() {
        // Generate valid channel (1–16), controller (0–127), and two values
        const auto channel =
            *rc::gen::inRange<uint8_t>(1, 17); // 1 to 16 inclusive
        const auto controller =
            *rc::gen::inRange<uint8_t>(0, 128); // 0 to 127 inclusive
        const auto valueA =
            *rc::gen::inRange<uint8_t>(0, 128); // 0 to 127 inclusive
        const auto valueB =
            *rc::gen::inRange<uint8_t>(0, 128); // 0 to 127 inclusive

        // Ensure A and B are different to make the test meaningful
        RC_PRE(valueA != valueB);

        CCStateStore store;
        store.begin();

        // Write A then B to the same (channel, controller)
        bool setA = store.set(channel, controller, valueA);
        RC_ASSERT(setA);

        bool setB = store.set(channel, controller, valueB);
        RC_ASSERT(setB);

        // get() must return B (last write wins)
        int16_t result = store.get(channel, controller);
        RC_ASSERT(result == static_cast<int16_t>(valueB));
      });

  TEST_ASSERT_TRUE_MESSAGE(result,
                           "Property 8: Last-write-wins concurrency failed");
}

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_property2_round_trip_consistency);
  RUN_TEST(test_property3_invalid_coordinates_rejection);
  RUN_TEST(test_property9_bulk_read_ordered_snapshot);
  RUN_TEST(test_property8_last_write_wins_concurrency);

  return UNITY_END();
}
