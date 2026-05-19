/**
 * Property-based tests for BleProtocol.
 *
 * Uses Rapidcheck with Unity test framework.
 * Tag: "Feature: bluetooth-remote-control"
 */

#include <rapidcheck.h>
#include <unity.h>
#include <vector>

#include "ble/BleProtocol.h"

void setUp() {}
void tearDown() {}

/**
 * Property 1: Protocol message round-trip
 *
 * For any valid CCMessage (channel 1–16, controller 0–127, value 0–127),
 * serializing the message to a 3-byte buffer and then parsing that buffer
 * back should produce a CCMessage identical to the original.
 *
 * **Validates: Requirements 5.4, 3.1, 5.1**
 */
void test_property1_protocol_message_roundtrip() {
  int iterations = 0;
  const auto result = rc::check(
      "Feature: bluetooth-remote-control, Property 1: Protocol message "
      "round-trip",
      [&iterations]() {
        iterations++;

        // Generate a valid CCMessage with constrained ranges
        const uint8_t channel =
            *rc::gen::inRange<uint8_t>(1, 17); // 1-16 inclusive
        const uint8_t controller =
            *rc::gen::inRange<uint8_t>(0, 128); // 0-127 inclusive
        const uint8_t value =
            *rc::gen::inRange<uint8_t>(0, 128); // 0-127 inclusive

        CCMessage original;
        original.channel = channel;
        original.controller = controller;
        original.value = value;

        // Serialize to 3-byte buffer
        uint8_t buffer[3];
        BleProtocol::serialize(original, buffer);

        // Parse back from buffer
        CCMessage parsed;
        ParseResult parseResult = BleProtocol::parse(buffer, 3, parsed);

        // Assert parse succeeds
        RC_ASSERT(parseResult == ParseResult::OK);

        // Assert all fields match the original
        RC_ASSERT(parsed.channel == original.channel);
        RC_ASSERT(parsed.controller == original.controller);
        RC_ASSERT(parsed.value == original.value);
      });

  TEST_ASSERT_TRUE_MESSAGE(result,
                           "Property 1: Protocol message round-trip failed");
  TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(
      100, iterations, "Property test should run at least 100 iterations");
}

/**
 * Property 5: Invalid CC write rejection
 *
 * For any 3-byte message with at least one field outside its valid range
 * (channel == 0 or > 16, controller > 127, value > 127),
 * parse() returns a non-OK ParseResult.
 *
 * **Validates: Requirements 3.4, 3.5, 3.6, 3.7, 5.7**
 */
void test_property5_invalid_cc_write_rejection() {
  const auto result = rc::check(
      "Feature: bluetooth-remote-control, Property 5: Invalid CC write "
      "rejection",
      []() {
        // Generate all three bytes across the full uint8_t range [0, 255]
        const auto channel = *rc::gen::inRange(0, 256);
        const auto controller = *rc::gen::inRange(0, 256);
        const auto value = *rc::gen::inRange(0, 256);

        // Valid ranges: channel 1-16, controller 0-127, value 0-127
        bool channelValid = (channel >= 1 && channel <= 16);
        bool controllerValid = (controller <= 127);
        bool valueValid = (value <= 127);

        // Discard if all fields are valid (we only want invalid messages)
        RC_PRE(!channelValid || !controllerValid || !valueValid);

        uint8_t data[3] = {static_cast<uint8_t>(channel),
                           static_cast<uint8_t>(controller),
                           static_cast<uint8_t>(value)};
        CCMessage msg = {};
        ParseResult parseResult = BleProtocol::parse(data, 3, msg);

        // parse() must return a non-OK error result
        RC_ASSERT(parseResult != ParseResult::OK);

        // Verify the specific error matches the first invalid field
        // (parse checks in order: channel, controller, value)
        if (!channelValid) {
          RC_ASSERT(parseResult == ParseResult::ERROR_INVALID_CHANNEL);
        } else if (!controllerValid) {
          RC_ASSERT(parseResult == ParseResult::ERROR_INVALID_CONTROLLER);
        } else {
          RC_ASSERT(parseResult == ParseResult::ERROR_INVALID_VALUE);
        }
      });

  TEST_ASSERT_TRUE_MESSAGE(result,
                           "Property 5: Invalid CC write rejection failed");
}

/**
 * Property 6: Extra bytes tolerance
 *
 * For any byte sequence longer than 3 bytes where the first 3 bytes form a
 * valid message, parse() produces the same CCMessage as parsing only the first
 * 3 bytes.
 *
 * **Validates: Requirements 5.6**
 */
void test_property6_extra_bytes_tolerance() {
  const auto result = rc::check(
      "Feature: bluetooth-remote-control, Property 6: Extra bytes tolerance",
      []() {
        // Generate valid first 3 bytes: channel 1-16, controller 0-127, value
        // 0-127
        const auto channel =
            *rc::gen::inRange<uint8_t>(1, 17); // 1 to 16 inclusive
        const auto controller =
            *rc::gen::inRange<uint8_t>(0, 128); // 0 to 127 inclusive
        const auto value =
            *rc::gen::inRange<uint8_t>(0, 128); // 0 to 127 inclusive

        // Generate 1-10 random extra bytes
        const auto extraCount = *rc::gen::inRange<size_t>(1, 11);
        const auto extraBytes = *rc::gen::container<std::vector<uint8_t>>(
            extraCount, rc::gen::arbitrary<uint8_t>());

        // Build the full buffer: valid 3 bytes + extra bytes
        std::vector<uint8_t> fullBuffer;
        fullBuffer.push_back(channel);
        fullBuffer.push_back(controller);
        fullBuffer.push_back(value);
        fullBuffer.insert(fullBuffer.end(), extraBytes.begin(),
                          extraBytes.end());

        // Build the 3-byte-only buffer
        uint8_t shortBuffer[3] = {channel, controller, value};

        // Parse the full buffer (length > 3)
        CCMessage fullMsg{};
        ParseResult fullResult =
            BleProtocol::parse(fullBuffer.data(), fullBuffer.size(), fullMsg);

        // Parse only the first 3 bytes
        CCMessage shortMsg{};
        ParseResult shortResult = BleProtocol::parse(shortBuffer, 3, shortMsg);

        // Both should succeed with ParseResult::OK
        RC_ASSERT(fullResult == ParseResult::OK);
        RC_ASSERT(shortResult == ParseResult::OK);

        // Both should produce identical CCMessage fields
        RC_ASSERT(fullMsg.channel == shortMsg.channel);
        RC_ASSERT(fullMsg.controller == shortMsg.controller);
        RC_ASSERT(fullMsg.value == shortMsg.value);
      });

  TEST_ASSERT_TRUE_MESSAGE(result, "Property 6: Extra bytes tolerance failed");
}

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_property1_protocol_message_roundtrip);
  RUN_TEST(test_property5_invalid_cc_write_rejection);
  RUN_TEST(test_property6_extra_bytes_tolerance);

  return UNITY_END();
}
