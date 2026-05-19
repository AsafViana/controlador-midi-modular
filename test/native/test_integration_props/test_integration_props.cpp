/**
 * Property-based integration tests for Bluetooth Remote Control.
 *
 * Tests the integration logic between BleProtocol, CCStateStore, and
 * MidiEngine by exercising the same logic flow as BleServer::handleCCWrite
 * and BleServer::notifyCC.
 *
 * Uses Rapidcheck with Unity test framework.
 * Tag: "Feature: bluetooth-remote-control"
 */

#include <cstdint>
#include <rapidcheck.h>
#include <unity.h>
#include <vector>

#include "ble/BleProtocol.h"
#include "ble/CCStateStore.h"

// ── Mock MidiEngine ──────────────────────────────────────────────────────────

/**
 * MockMidiEngine — Records sendCC calls for verification.
 * Mimics the interface used by BleServer::handleCCWrite.
 */
struct MockMidiCC {
  uint8_t controlador;
  uint8_t valor;
  uint8_t canal;
};

class MockMidiEngine {
public:
  void sendCC(const MockMidiCC &cc) { _calls.push_back(cc); }

  size_t getSendCCCount() const { return _calls.size(); }

  const MockMidiCC &getLastCall() const { return _calls.back(); }

  void reset() { _calls.clear(); }

private:
  std::vector<MockMidiCC> _calls;
};

// ── Integration logic under test ─────────────────────────────────────────────

/**
 * Replicates the core logic of BleServer::handleCCWrite:
 *   1. Parse the BLE message using BleProtocol
 *   2. If valid, update CCStateStore
 *   3. If store update succeeds, call MidiEngine::sendCC
 */
static ParseResult handleCCWriteIntegration(const uint8_t *data, size_t length,
                                            CCStateStore &store,
                                            MockMidiEngine &engine) {
  CCMessage msg;
  ParseResult result = BleProtocol::parse(data, length, msg);

  if (result != ParseResult::OK) {
    return result;
  }

  if (!store.set(msg.channel, msg.controller, msg.value)) {
    return ParseResult::ERROR_INVALID_CHANNEL;
  }

  MockMidiCC midiCC;
  midiCC.controlador = msg.controller;
  midiCC.valor = msg.value;
  midiCC.canal = msg.channel;
  engine.sendCC(midiCC);

  return ParseResult::OK;
}

// ── Test setup/teardown ──────────────────────────────────────────────────────

void setUp() {}
void tearDown() {}

// ── Property Tests ───────────────────────────────────────────────────────────

/**
 * Property 4: Valid CC write propagation
 *
 * For any valid 3-byte BLE write message (channel 1–16, controller 0–127,
 * value 0–127), processing the write should result in BOTH:
 *   - The CCStateStore being updated with the new value
 *   - The MidiEngine receiving a sendCC call with the corresponding
 *     channel, controller, and value
 *
 * **Validates: Requirements 3.2, 3.3, 7.3**
 */
void test_property4_valid_cc_write_propagation() {
  const auto result =
      rc::check("Feature: bluetooth-remote-control, Property 4: Valid CC write "
                "propagation",
                []() {
                  // Generate valid CC parameters
                  const auto channel =
                      *rc::gen::inRange<uint8_t>(1, 17); // 1–16 inclusive
                  const auto controller =
                      *rc::gen::inRange<uint8_t>(0, 128); // 0–127 inclusive
                  const auto value =
                      *rc::gen::inRange<uint8_t>(0, 128); // 0–127 inclusive

                  // Serialize into a 3-byte BLE write payload
                  CCMessage originalMsg;
                  originalMsg.channel = channel;
                  originalMsg.controller = controller;
                  originalMsg.value = value;

                  uint8_t writeData[3];
                  BleProtocol::serialize(originalMsg, writeData);

                  // Set up fresh store and mock engine
                  CCStateStore store;
                  store.begin();
                  MockMidiEngine engine;

                  // Process the BLE write (same logic as
                  // BleServer::handleCCWrite)
                  ParseResult parseResult =
                      handleCCWriteIntegration(writeData, 3, store, engine);

                  // ── Assertion 1: Parse succeeded ──
                  RC_ASSERT(parseResult == ParseResult::OK);

                  // ── Assertion 2: CCStateStore was updated with the correct
                  // value ──
                  int16_t storedValue = store.get(channel, controller);
                  RC_ASSERT(storedValue == static_cast<int16_t>(value));

                  // ── Assertion 3: MidiEngine::sendCC was called exactly once
                  // ──
                  RC_ASSERT(engine.getSendCCCount() == 1);

                  // ── Assertion 4: sendCC parameters match the original
                  // message ──
                  const auto &call = engine.getLastCall();
                  RC_ASSERT(call.canal == channel);
                  RC_ASSERT(call.controlador == controller);
                  RC_ASSERT(call.valor == value);
                });

  TEST_ASSERT_TRUE_MESSAGE(result,
                           "Property 4: Valid CC write propagation failed");
}

/**
 * Property 7: CC change notification format
 *
 * For any CC value change while client is subscribed, the notification payload
 * equals BleProtocol::serialize() output for that CCMessage.
 *
 * We verify at the BleProtocol level: for any valid CCMessage,
 * serialize() produces a 3-byte buffer where:
 *   - byte[0] == channel  (1–16)
 *   - byte[1] == controller (0–127)
 *   - byte[2] == value (0–127)
 *
 * This is the exact format used by BleServer::notifyCC() when sending
 * notifications to subscribed clients.
 *
 * **Validates: Requirements 4.2, 5.3**
 */
void test_property7_cc_change_notification_format() {
  const auto result = rc::check(
      "Feature: bluetooth-remote-control, Property 7: CC change notification "
      "format",
      []() {
        // Generate a valid CCMessage representing a CC value change
        const uint8_t channel =
            *rc::gen::inRange<uint8_t>(1, 17); // 1-16 inclusive
        const uint8_t controller =
            *rc::gen::inRange<uint8_t>(0, 128); // 0-127 inclusive
        const uint8_t value =
            *rc::gen::inRange<uint8_t>(0, 128); // 0-127 inclusive

        CCMessage msg;
        msg.channel = channel;
        msg.controller = controller;
        msg.value = value;

        // Serialize the message (this is what notifyCC uses to build the
        // notification payload)
        uint8_t buffer[3] = {0xFF, 0xFF, 0xFF}; // Initialize to sentinel
        BleProtocol::serialize(msg, buffer);

        // The notification payload must be exactly 3 bytes with:
        //   byte[0] = channel
        //   byte[1] = controller
        //   byte[2] = value
        RC_ASSERT(buffer[0] == channel);
        RC_ASSERT(buffer[1] == controller);
        RC_ASSERT(buffer[2] == value);

        // Additionally verify the serialized payload can be parsed back
        // to the same message (ensuring the notification format is valid
        // for the client to decode)
        CCMessage parsed;
        ParseResult parseResult = BleProtocol::parse(buffer, 3, parsed);
        RC_ASSERT(parseResult == ParseResult::OK);
        RC_ASSERT(parsed.channel == msg.channel);
        RC_ASSERT(parsed.controller == msg.controller);
        RC_ASSERT(parsed.value == msg.value);
      });

  TEST_ASSERT_TRUE_MESSAGE(result,
                           "Property 7: CC change notification format failed");
}

/**
 * Property 10: Bulk read snapshot isolation
 *
 * For any concurrent modification to the CCStateStore that occurs after
 * a bulk read snapshot is taken (via getChannelSnapshot()), the snapshot
 * response should still reflect the values captured at the start of the
 * read, not the modified values.
 *
 * This verifies that getChannelSnapshot() returns a copy of the data,
 * so subsequent set() calls do not alter the already-returned buffer.
 *
 * **Validates: Requirements 9.5**
 */
void test_property10_bulk_read_snapshot_isolation() {
  const auto result = rc::check(
      "Feature: bluetooth-remote-control, Property 10: Bulk read snapshot "
      "isolation",
      []() {
        // Generate a valid channel (1–16)
        const auto channel = *rc::gen::inRange<uint8_t>(1, 17); // 1–16

        // Generate initial values for all 128 controllers (0–127 each)
        const auto initialValues = *rc::gen::container<std::vector<uint8_t>>(
            128, rc::gen::inRange<uint8_t>(0, 128)); // [0, 127]

        // Generate a set of modifications: random controller indices and new
        // values
        const auto numModifications =
            *rc::gen::inRange<size_t>(1, 65); // 1–64 modifications

        struct Modification {
          uint8_t controller;
          uint8_t newValue;
        };

        std::vector<Modification> modifications;
        for (size_t i = 0; i < numModifications; i++) {
          Modification mod;
          mod.controller = *rc::gen::inRange<uint8_t>(0, 128); // 0–127
          mod.newValue = *rc::gen::inRange<uint8_t>(0, 128);   // 0–127
          modifications.push_back(mod);
        }

        // Set up the store with initial values
        CCStateStore store;
        store.begin();

        for (uint8_t ctrl = 0; ctrl < 128; ctrl++) {
          bool ok = store.set(channel, ctrl, initialValues[ctrl]);
          RC_ASSERT(ok);
        }

        // Take the snapshot (simulates the start of a bulk read)
        uint8_t snapshot[128] = {};
        bool snapshotOk = store.getChannelSnapshot(channel, snapshot);
        RC_ASSERT(snapshotOk);

        // Now modify the store AFTER the snapshot was taken
        // (simulates concurrent modifications during a bulk read response)
        for (const auto &mod : modifications) {
          store.set(channel, mod.controller, mod.newValue);
        }

        // ── Assertion: The snapshot still reflects the ORIGINAL values ──
        // The snapshot buffer must NOT have been affected by the post-snapshot
        // modifications.
        for (uint8_t n = 0; n < 128; n++) {
          RC_ASSERT(snapshot[n] == initialValues[n]);
        }

        // ── Verify the store itself DID change (sanity check) ──
        // At least one modification should have changed a value
        // (unless the new value happened to be the same as the old one)
        bool anyDifference = false;
        for (const auto &mod : modifications) {
          int16_t currentVal = store.get(channel, mod.controller);
          if (currentVal !=
              static_cast<int16_t>(initialValues[mod.controller])) {
            anyDifference = true;
            break;
          }
        }
        // If modifications had different values, the store should reflect them
        if (anyDifference) {
          // Verify at least one store value differs from the snapshot
          bool snapshotDiffers = false;
          for (uint8_t n = 0; n < 128; n++) {
            int16_t currentVal = store.get(channel, n);
            if (currentVal != static_cast<int16_t>(snapshot[n])) {
              snapshotDiffers = true;
              break;
            }
          }
          RC_ASSERT(snapshotDiffers);
        }
      });

  TEST_ASSERT_TRUE_MESSAGE(result,
                           "Property 10: Bulk read snapshot isolation failed");
}

// ── Main ─────────────────────────────────────────────────────────────────────

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_property4_valid_cc_write_propagation);
  RUN_TEST(test_property7_cc_change_notification_format);
  RUN_TEST(test_property10_bulk_read_snapshot_isolation);

  return UNITY_END();
}
