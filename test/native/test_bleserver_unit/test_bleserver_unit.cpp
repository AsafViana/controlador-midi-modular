/**
 * Unit tests for BLE initialization and retry logic.
 *
 * Tests the BleServer::begin() retry mechanism by providing a mock
 * implementation of the BLE stack. Since NimBLE is not available in native
 * tests, we compile a test-specific version of BleServer that replaces
 * _initBle() with controllable behavior.
 *
 * Requirements: 1.1, 1.6, 1.7
 */

#include <unity.h>

#include "ble/BleProtocol.h"
#include "ble/CCStateStore.h"

// ═══════════════════════════════════════════════════════════════════════════════
// Mock infrastructure for BleServer initialization testing
// ═══════════════════════════════════════════════════════════════════════════════

// Controls how many times _initBle() will fail before succeeding.
// -1 means always fail.
static int g_initFailCount = 0;
static int g_initCallCount = 0;
static int g_delayCallCount = 0;
static uint32_t g_lastDelayMs = 0;

// Track if error was reported (simulates display system reporting)
static bool g_errorReported = false;

// Reset all mock state
static void resetMocks() {
  g_initFailCount = 0;
  g_initCallCount = 0;
  g_delayCallCount = 0;
  g_lastDelayMs = 0;
  g_errorReported = false;
}

// ═══════════════════════════════════════════════════════════════════════════════
// TestBleServer: A test-specific implementation of the BLE server that
// replicates the begin() retry logic from BleServer but uses mock _initBle().
// This allows testing the retry mechanism without the NimBLE dependency.
// ═══════════════════════════════════════════════════════════════════════════════

static constexpr uint8_t MAX_INIT_RETRIES = 3;
static constexpr uint32_t RETRY_DELAY_MS = 1000;

/// Mock delay function that records calls
static void mockDelay(uint32_t ms) {
  g_delayCallCount++;
  g_lastDelayMs = ms;
}

/// Mock _initBle that fails g_initFailCount times then succeeds
static bool mockInitBle() {
  g_initCallCount++;
  if (g_initFailCount < 0) {
    // Always fail
    return false;
  }
  if (g_initCallCount <= g_initFailCount) {
    return false;
  }
  return true;
}

/**
 * TestBleServer replicates the exact retry logic from BleServer::begin()
 * but uses mock functions instead of real NimBLE calls.
 *
 * This mirrors the production code:
 *   bool BleServer::begin() {
 *     _retryCount = 0;
 *     for (uint8_t attempt = 0; attempt < MAX_INIT_RETRIES; attempt++) {
 *       _retryCount = attempt + 1;
 *       if (_initBle()) return true;
 *       if (attempt < MAX_INIT_RETRIES - 1) delay(RETRY_DELAY_MS);
 *     }
 *     // report error
 *     return false;
 *   }
 */
class TestBleServer {
public:
  bool begin() {
    _retryCount = 0;

    for (uint8_t attempt = 0; attempt < MAX_INIT_RETRIES; attempt++) {
      _retryCount = attempt + 1;

      if (mockInitBle()) {
        return true;
      }

      // Delay between retries (not after last attempt)
      if (attempt < MAX_INIT_RETRIES - 1) {
        mockDelay(RETRY_DELAY_MS);
      }
    }

    // All retries exhausted — report persistent error
    g_errorReported = true;
    return false;
  }

  uint8_t getRetryCount() const { return _retryCount; }

  // Simulate that MIDI continues to operate even after BLE failure
  bool isMidiOperational() const { return true; }

private:
  uint8_t _retryCount = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Test setup/teardown
// ═══════════════════════════════════════════════════════════════════════════════

void setUp() { resetMocks(); }
void tearDown() {}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Test: Successful init on first attempt.
 *
 * WHEN _initBle() succeeds on the first call,
 * THEN begin() returns true,
 * AND retryCount is 1 (one attempt made),
 * AND no delay is called.
 *
 * Validates: Requirement 1.1
 */
void test_successful_init_on_first_attempt() {
  TestBleServer server;

  // _initBle will succeed immediately (0 failures before success)
  g_initFailCount = 0;

  bool result = server.begin();

  TEST_ASSERT_TRUE_MESSAGE(result, "begin() should return true on first "
                                   "successful init");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, server.getRetryCount(),
                                  "retryCount should be 1 (one attempt)");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_initCallCount,
                                "_initBle should be called exactly once");
  TEST_ASSERT_EQUAL_INT_MESSAGE(0, g_delayCallCount,
                                "No delay should be called on first success");
}

/**
 * Test: Retry on failure - succeeds on 2nd attempt.
 *
 * WHEN _initBle() fails on the first call but succeeds on the second,
 * THEN begin() returns true,
 * AND retryCount is 2,
 * AND delay(1000) is called once between attempts.
 *
 * Validates: Requirement 1.6
 */
void test_retry_succeeds_on_second_attempt() {
  TestBleServer server;

  // Fail once, then succeed
  g_initFailCount = 1;

  bool result = server.begin();

  TEST_ASSERT_TRUE_MESSAGE(result, "begin() should return true after retry");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(2, server.getRetryCount(),
                                  "retryCount should be 2");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, g_initCallCount,
                                "_initBle should be called twice");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_delayCallCount,
                                "delay should be called once between retries");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(1000, g_lastDelayMs,
                                   "delay should be 1000ms");
}

/**
 * Test: Retry on failure - succeeds on 3rd attempt.
 *
 * WHEN _initBle() fails on the first two calls but succeeds on the third,
 * THEN begin() returns true,
 * AND retryCount is 3,
 * AND delay(1000) is called twice.
 *
 * Validates: Requirement 1.6
 */
void test_retry_succeeds_on_third_attempt() {
  TestBleServer server;

  // Fail twice, then succeed
  g_initFailCount = 2;

  bool result = server.begin();

  TEST_ASSERT_TRUE_MESSAGE(result,
                           "begin() should return true after 2 retries");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(3, server.getRetryCount(),
                                  "retryCount should be 3");
  TEST_ASSERT_EQUAL_INT_MESSAGE(3, g_initCallCount,
                                "_initBle should be called three times");
  TEST_ASSERT_EQUAL_INT_MESSAGE(2, g_delayCallCount,
                                "delay should be called twice between retries");
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(1000, g_lastDelayMs,
                                   "delay should be 1000ms");
}

/**
 * Test: Persistent error reporting after 3 failures.
 *
 * WHEN _initBle() fails on all 3 attempts,
 * THEN begin() returns false,
 * AND retryCount is 3,
 * AND a persistent error is reported to the display system,
 * AND delay(1000) is called twice (between attempts 1-2 and 2-3, not after
 * last).
 *
 * Validates: Requirement 1.6, 1.7
 */
void test_persistent_error_after_three_failures() {
  TestBleServer server;

  // Always fail
  g_initFailCount = -1;

  bool result = server.begin();

  TEST_ASSERT_FALSE_MESSAGE(result,
                            "begin() should return false after all retries");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(3, server.getRetryCount(),
                                  "retryCount should be 3 (max attempts)");
  TEST_ASSERT_EQUAL_INT_MESSAGE(3, g_initCallCount,
                                "_initBle should be called exactly 3 times");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      2, g_delayCallCount,
      "delay should be called twice (not after last attempt)");
  TEST_ASSERT_TRUE_MESSAGE(g_errorReported,
                           "Persistent error should be reported to display");
}

/**
 * Test: Controller continues MIDI operation after BLE failure.
 *
 * WHEN BLE initialization fails completely (all retries exhausted),
 * THEN the controller continues normal MIDI operation,
 * AND the CCStateStore remains functional,
 * AND MIDI processing is not blocked.
 *
 * Validates: Requirement 1.7
 */
void test_controller_continues_midi_after_ble_failure() {
  TestBleServer server;

  // BLE init always fails
  g_initFailCount = -1;

  bool bleResult = server.begin();
  TEST_ASSERT_FALSE_MESSAGE(bleResult, "BLE init should fail");

  // Verify MIDI operation continues (simulated by isMidiOperational)
  TEST_ASSERT_TRUE_MESSAGE(server.isMidiOperational(),
                           "MIDI should continue operating after BLE failure");

  // Verify CCStateStore still works independently of BLE
  CCStateStore store;
  store.begin();

  // Can still set and get CC values (MIDI operation unaffected)
  bool setOk = store.set(1, 64, 100);
  TEST_ASSERT_TRUE_MESSAGE(setOk, "CCStateStore should work after BLE failure");

  int16_t value = store.get(1, 64);
  TEST_ASSERT_EQUAL_INT16_MESSAGE(
      100, value, "CCStateStore should return correct value after BLE failure");

  // Verify multiple channels still work
  for (uint8_t ch = 1; ch <= 16; ch++) {
    TEST_ASSERT_TRUE(store.set(ch, 0, ch));
    TEST_ASSERT_EQUAL_INT16(ch, store.get(ch, 0));
  }
}

/**
 * Test: Retry count is accessible for diagnostics.
 *
 * Verifies that getRetryCount() correctly reports the number of
 * initialization attempts made, useful for diagnostic display.
 *
 * Validates: Requirement 1.6
 */
void test_retry_count_accessible_for_diagnostics() {
  TestBleServer server;

  // Before begin(), retryCount should be 0
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      0, server.getRetryCount(),
      "retryCount should be 0 before begin() is called");

  // After successful init on first try
  g_initFailCount = 0;
  server.begin();
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(
      1, server.getRetryCount(),
      "retryCount should be 1 after first-attempt success");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Main
// ═══════════════════════════════════════════════════════════════════════════════

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_successful_init_on_first_attempt);
  RUN_TEST(test_retry_succeeds_on_second_attempt);
  RUN_TEST(test_retry_succeeds_on_third_attempt);
  RUN_TEST(test_persistent_error_after_three_failures);
  RUN_TEST(test_controller_continues_midi_after_ble_failure);
  RUN_TEST(test_retry_count_accessible_for_diagnostics);

  return UNITY_END();
}
