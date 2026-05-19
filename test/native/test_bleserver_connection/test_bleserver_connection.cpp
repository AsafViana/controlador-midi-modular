/**
 * Unit tests for BLE connection state management.
 *
 * Tests the BleServer connection lifecycle: single-client enforcement,
 * disconnect handling, advertising resume, display callbacks, and
 * notification behavior when no client is connected.
 *
 * Since NimBLE is not available in native tests, we use a TestBleServer
 * that replicates the connection management logic from BleServer with
 * controllable mock behavior.
 *
 * Requirements: 6.1, 6.2, 6.3, 6.6, 6.7, 4.6
 */

#include <unity.h>

#include "ble/BleProtocol.h"
#include "ble/CCStateStore.h"

#include <cstring>

// ═══════════════════════════════════════════════════════════════════════════════
// Mock infrastructure for connection state testing
// ═══════════════════════════════════════════════════════════════════════════════

// Track advertising state
static bool g_advertisingActive = false;
static int g_advertisingStartCount = 0;
static int g_advertisingStopCount = 0;

// Track disconnect calls (for second client rejection)
static int g_disconnectCallCount = 0;
static uint16_t g_lastDisconnectedHandle = 0;

// Track connection callback invocations
static int g_connectionCallbackCount = 0;
static bool g_lastCallbackConnected = false;
static char g_lastCallbackAddress[64] = "";

// Track connection parameter update calls
static int g_updateConnParamsCount = 0;

// Simulated time for advertising resume timing
static uint32_t g_currentTimeMs = 0;
static uint32_t g_advertisingResumedAtMs = 0;

// Reset all mock state
static void resetMocks() {
  g_advertisingActive = false;
  g_advertisingStartCount = 0;
  g_advertisingStopCount = 0;
  g_disconnectCallCount = 0;
  g_lastDisconnectedHandle = 0;
  g_connectionCallbackCount = 0;
  g_lastCallbackConnected = false;
  memset(g_lastCallbackAddress, 0, sizeof(g_lastCallbackAddress));
  g_updateConnParamsCount = 0;
  g_currentTimeMs = 0;
  g_advertisingResumedAtMs = 0;
}

// ═══════════════════════════════════════════════════════════════════════════════
// TestBleServer: Replicates BleServer connection management logic
// with mock BLE stack operations.
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * TestBleServer mirrors the connection management logic from BleServer:
 *
 * - _handleConnect(): single-client enforcement, stop advertising, callback
 * - _handleDisconnect(): reset state, callback, resume advertising
 * - isConnected(): accessor
 * - notifyCC(): only sends if connected and subscribed
 * - onConnectionChange(): register callback
 *
 * This mirrors the production code in BleServer.cpp.
 */
class TestBleServer {
public:
  using ConnectionCallback = void (*)(bool connected, const char *address);

  void begin() {
    _connected = false;
    _clientSubscribed = false;
    _connectedClientHandle = 0;
    _connectionCallback = nullptr;
    // Start advertising
    startAdvertising();
  }

  bool isConnected() const { return _connected; }

  void onConnectionChange(ConnectionCallback callback) {
    _connectionCallback = callback;
  }

  /// Mirrors BleServer::_handleConnect
  void handleConnect(uint16_t connHandle, const char *address) {
    // Single-client enforcement: reject second connection if already connected
    if (_connected) {
      // Disconnect the new (second) client immediately
      mockDisconnect(connHandle);
      return;
    }

    _connected = true;
    _connectedClientHandle = connHandle;
    stopAdvertising();

    // Request optimized connection parameters
    g_updateConnParamsCount++;

    if (_connectionCallback) {
      _connectionCallback(true, address);
    }
  }

  /// Mirrors BleServer::_handleDisconnect
  void handleDisconnect() {
    _connected = false;
    _clientSubscribed = false;
    _connectedClientHandle = 0;

    if (_connectionCallback) {
      _connectionCallback(false, "");
    }

    // Resume advertising immediately after disconnection
    // (well within 500ms requirement)
    startAdvertising();
    g_advertisingResumedAtMs = g_currentTimeMs;
  }

  /// Mirrors BleServer::_handleSubscribe
  void handleSubscribe(bool subscribed) { _clientSubscribed = subscribed; }

  /// Mirrors BleServer::notifyCC
  /// Returns false if no client connected or not subscribed
  bool notifyCC(uint8_t channel, uint8_t controller, uint8_t value) {
    if (!_connected || !_clientSubscribed) {
      return false;
    }

    // In production, this serializes and sends via BLE notification.
    // Here we just verify the preconditions are met.
    CCMessage msg = {channel, controller, value};
    uint8_t buffer[3];
    BleProtocol::serialize(msg, buffer);
    _lastNotifyChannel = channel;
    _lastNotifyController = controller;
    _lastNotifyValue = value;
    _notifyCount++;
    return true;
  }

  // Test helpers
  bool isSubscribed() const { return _clientSubscribed; }
  uint16_t getConnectedHandle() const { return _connectedClientHandle; }
  int getNotifyCount() const { return _notifyCount; }
  uint8_t getLastNotifyChannel() const { return _lastNotifyChannel; }
  uint8_t getLastNotifyController() const { return _lastNotifyController; }
  uint8_t getLastNotifyValue() const { return _lastNotifyValue; }

private:
  bool _connected = false;
  bool _clientSubscribed = false;
  uint16_t _connectedClientHandle = 0;
  ConnectionCallback _connectionCallback = nullptr;

  // Notification tracking
  int _notifyCount = 0;
  uint8_t _lastNotifyChannel = 0;
  uint8_t _lastNotifyController = 0;
  uint8_t _lastNotifyValue = 0;

  void startAdvertising() {
    g_advertisingActive = true;
    g_advertisingStartCount++;
  }

  void stopAdvertising() {
    g_advertisingActive = false;
    g_advertisingStopCount++;
  }

  void mockDisconnect(uint16_t handle) {
    g_disconnectCallCount++;
    g_lastDisconnectedHandle = handle;
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// Connection callback for tests
// ═══════════════════════════════════════════════════════════════════════════════

static void testConnectionCallback(bool connected, const char *address) {
  g_connectionCallbackCount++;
  g_lastCallbackConnected = connected;
  strncpy(g_lastCallbackAddress, address, sizeof(g_lastCallbackAddress) - 1);
  g_lastCallbackAddress[sizeof(g_lastCallbackAddress) - 1] = '\0';
}

// ═══════════════════════════════════════════════════════════════════════════════
// Test setup/teardown
// ═══════════════════════════════════════════════════════════════════════════════

void setUp() { resetMocks(); }
void tearDown() {}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Single client connection and disconnection
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Test: Single client connects successfully.
 *
 * WHEN a Remote_Client connects,
 * THEN isConnected() returns true,
 * AND advertising is stopped,
 * AND the connection handle is stored.
 *
 * Validates: Requirement 6.1
 */
void test_single_client_connects_successfully() {
  TestBleServer server;
  server.begin();

  TEST_ASSERT_FALSE_MESSAGE(server.isConnected(),
                            "Should not be connected initially");
  TEST_ASSERT_TRUE_MESSAGE(g_advertisingActive,
                           "Should be advertising after begin()");

  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");

  TEST_ASSERT_TRUE_MESSAGE(server.isConnected(),
                           "Should be connected after handleConnect");
  TEST_ASSERT_FALSE_MESSAGE(g_advertisingActive,
                            "Advertising should stop on connect");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(1, server.getConnectedHandle(),
                                   "Connection handle should be stored");
}

/**
 * Test: Single client disconnects successfully.
 *
 * WHEN a connected Remote_Client disconnects,
 * THEN isConnected() returns false,
 * AND the connection handle is cleared,
 * AND subscription state is cleared.
 *
 * Validates: Requirement 6.1
 */
void test_single_client_disconnects_successfully() {
  TestBleServer server;
  server.begin();

  // Connect and subscribe
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  server.handleSubscribe(true);
  TEST_ASSERT_TRUE(server.isConnected());
  TEST_ASSERT_TRUE(server.isSubscribed());

  // Disconnect
  server.handleDisconnect();

  TEST_ASSERT_FALSE_MESSAGE(server.isConnected(),
                            "Should not be connected after disconnect");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(0, server.getConnectedHandle(),
                                   "Connection handle should be cleared");
  TEST_ASSERT_FALSE_MESSAGE(server.isSubscribed(),
                            "Subscription should be cleared on disconnect");
}

/**
 * Test: Client can reconnect after disconnection.
 *
 * WHEN a client disconnects and a new client connects,
 * THEN the new connection is accepted,
 * AND isConnected() returns true.
 *
 * Validates: Requirement 6.1
 */
void test_client_reconnects_after_disconnect() {
  TestBleServer server;
  server.begin();

  // First connection
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  TEST_ASSERT_TRUE(server.isConnected());

  // Disconnect
  server.handleDisconnect();
  TEST_ASSERT_FALSE(server.isConnected());

  // Second connection (different client)
  server.handleConnect(2, "11:22:33:44:55:66");
  TEST_ASSERT_TRUE_MESSAGE(server.isConnected(),
                           "Should accept new connection after disconnect");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(2, server.getConnectedHandle(),
                                   "Should store new connection handle");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Second client rejection
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Test: Second client is rejected while first is connected.
 *
 * IF a second Remote_Client attempts to connect while a Remote_Client is
 * already connected, THEN THE BLE_Server SHALL reject the incoming connection
 * and maintain the existing connection unchanged.
 *
 * Validates: Requirement 6.2
 */
void test_second_client_rejected() {
  TestBleServer server;
  server.begin();

  // First client connects
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  TEST_ASSERT_TRUE(server.isConnected());
  TEST_ASSERT_EQUAL_UINT16(1, server.getConnectedHandle());

  // Second client attempts to connect
  server.handleConnect(2, "11:22:33:44:55:66");

  // First connection should remain unchanged
  TEST_ASSERT_TRUE_MESSAGE(server.isConnected(),
                           "First connection should remain active");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      1, server.getConnectedHandle(),
      "Original connection handle should be unchanged");

  // Second client should have been disconnected
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      1, g_disconnectCallCount,
      "disconnect() should be called for second client");
  TEST_ASSERT_EQUAL_UINT16_MESSAGE(
      2, g_lastDisconnectedHandle,
      "The second client's handle should be disconnected");
}

/**
 * Test: Multiple second client attempts are all rejected.
 *
 * WHEN multiple clients attempt to connect while one is already connected,
 * THEN all are rejected and the original connection remains.
 *
 * Validates: Requirement 6.2
 */
void test_multiple_second_clients_rejected() {
  TestBleServer server;
  server.begin();

  // First client connects
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");

  // Multiple second clients attempt to connect
  server.handleConnect(2, "11:22:33:44:55:66");
  server.handleConnect(3, "22:33:44:55:66:77");
  server.handleConnect(4, "33:44:55:66:77:88");

  // Original connection unchanged
  TEST_ASSERT_TRUE(server.isConnected());
  TEST_ASSERT_EQUAL_UINT16(1, server.getConnectedHandle());

  // All second clients were disconnected
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      3, g_disconnectCallCount,
      "All 3 second clients should have been disconnected");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Advertising resumes within 500ms after disconnect
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Test: Advertising resumes immediately after disconnect.
 *
 * WHEN the BLE connection is lost unexpectedly,
 * THE BLE_Server SHALL resume advertising within 500ms.
 *
 * The implementation resumes advertising immediately in the disconnect handler,
 * which is well within the 500ms requirement.
 *
 * Validates: Requirement 6.3
 */
void test_advertising_resumes_after_disconnect() {
  TestBleServer server;
  server.begin();

  int startCountBefore = g_advertisingStartCount;

  // Connect (stops advertising)
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  TEST_ASSERT_FALSE_MESSAGE(g_advertisingActive,
                            "Advertising should be stopped while connected");

  // Simulate time passing
  g_currentTimeMs = 100;

  // Disconnect
  server.handleDisconnect();

  TEST_ASSERT_TRUE_MESSAGE(g_advertisingActive,
                           "Advertising should resume after disconnect");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      startCountBefore + 1, g_advertisingStartCount,
      "startAdvertising should be called on disconnect");

  // Verify it resumed within 500ms (it resumes immediately at current time)
  TEST_ASSERT_TRUE_MESSAGE(
      (g_advertisingResumedAtMs - 100) < 500,
      "Advertising should resume within 500ms of disconnect");
}

/**
 * Test: Advertising resumes at time 0 (immediate).
 *
 * Verifies that the advertising resume happens synchronously in the
 * disconnect handler, meaning 0ms delay from the disconnect event.
 *
 * Validates: Requirement 6.3
 */
void test_advertising_resumes_immediately() {
  TestBleServer server;
  server.begin();

  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");

  // Set current time to simulate when disconnect happens
  g_currentTimeMs = 5000;

  server.handleDisconnect();

  // Advertising resumed at the exact same time as disconnect
  TEST_ASSERT_EQUAL_UINT32_MESSAGE(
      5000, g_advertisingResumedAtMs,
      "Advertising should resume at the same time as disconnect (0ms delay)");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Display callbacks on connect/disconnect
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Test: Connection callback invoked on connect with address.
 *
 * WHEN a Remote_Client connects,
 * THE BLE_Server SHALL report the connected state and the client device
 * address to the display system for UI indication.
 *
 * Validates: Requirement 6.6
 */
void test_display_callback_on_connect() {
  TestBleServer server;
  server.begin();
  server.onConnectionChange(testConnectionCallback);

  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");

  TEST_ASSERT_EQUAL_INT_MESSAGE(1, g_connectionCallbackCount,
                                "Callback should be invoked once on connect");
  TEST_ASSERT_TRUE_MESSAGE(g_lastCallbackConnected,
                           "Callback should report connected=true");
  TEST_ASSERT_EQUAL_STRING_MESSAGE("AA:BB:CC:DD:EE:FF", g_lastCallbackAddress,
                                   "Callback should report the client address");
}

/**
 * Test: Connection callback invoked on disconnect.
 *
 * WHEN a Remote_Client disconnects,
 * THE BLE_Server SHALL report the disconnected state to the display system
 * for UI indication.
 *
 * Validates: Requirement 6.7
 */
void test_display_callback_on_disconnect() {
  TestBleServer server;
  server.begin();
  server.onConnectionChange(testConnectionCallback);

  // Connect first
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  g_connectionCallbackCount = 0; // Reset to isolate disconnect callback

  // Disconnect
  server.handleDisconnect();

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      1, g_connectionCallbackCount,
      "Callback should be invoked once on disconnect");
  TEST_ASSERT_FALSE_MESSAGE(g_lastCallbackConnected,
                            "Callback should report connected=false");
  TEST_ASSERT_EQUAL_STRING_MESSAGE(
      "", g_lastCallbackAddress,
      "Callback should report empty address on disconnect");
}

/**
 * Test: No callback invoked when no callback is registered.
 *
 * WHEN no connection callback is registered,
 * THEN connect/disconnect should not crash.
 *
 * Validates: Requirement 6.6, 6.7 (robustness)
 */
void test_no_crash_without_callback() {
  TestBleServer server;
  server.begin();
  // No callback registered

  // Should not crash
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  server.handleDisconnect();

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      0, g_connectionCallbackCount,
      "No callback should be invoked when none is registered");
}

/**
 * Test: Callback not invoked for rejected second client.
 *
 * WHEN a second client is rejected,
 * THEN the connection callback should NOT be invoked
 * (the existing connection state is unchanged).
 *
 * Validates: Requirement 6.2, 6.6
 */
void test_no_callback_for_rejected_client() {
  TestBleServer server;
  server.begin();
  server.onConnectionChange(testConnectionCallback);

  // First client connects
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  TEST_ASSERT_EQUAL_INT(1, g_connectionCallbackCount);

  // Reset counter
  g_connectionCallbackCount = 0;

  // Second client rejected — no callback should fire
  server.handleConnect(2, "11:22:33:44:55:66");

  TEST_ASSERT_EQUAL_INT_MESSAGE(
      0, g_connectionCallbackCount,
      "Callback should NOT be invoked for rejected second client");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tests: Notification discarded when no client connected
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Test: Notification discarded when no client is connected.
 *
 * IF a CC_Value changes while no Remote_Client is connected,
 * THEN THE BLE_Server SHALL discard the notification and not queue it.
 *
 * Validates: Requirement 4.6
 */
void test_notification_discarded_when_not_connected() {
  TestBleServer server;
  server.begin();

  // No client connected — notification should be discarded
  bool result = server.notifyCC(1, 64, 100);

  TEST_ASSERT_FALSE_MESSAGE(result,
                            "notifyCC should return false when not connected");
  TEST_ASSERT_EQUAL_INT_MESSAGE(0, server.getNotifyCount(),
                                "No notification should be sent");
}

/**
 * Test: Notification discarded when connected but not subscribed.
 *
 * IF a CC_Value changes while a Remote_Client is connected but not subscribed,
 * THEN THE BLE_Server SHALL discard the notification.
 *
 * Validates: Requirement 4.6
 */
void test_notification_discarded_when_not_subscribed() {
  TestBleServer server;
  server.begin();

  // Client connected but NOT subscribed
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  TEST_ASSERT_TRUE(server.isConnected());
  TEST_ASSERT_FALSE(server.isSubscribed());

  bool result = server.notifyCC(1, 64, 100);

  TEST_ASSERT_FALSE_MESSAGE(result,
                            "notifyCC should return false when not subscribed");
  TEST_ASSERT_EQUAL_INT_MESSAGE(0, server.getNotifyCount(),
                                "No notification should be sent");
}

/**
 * Test: Notification sent when connected and subscribed.
 *
 * WHILE a Remote_Client is connected and subscribed to notifications,
 * WHEN a CC_Value changes,
 * THEN THE BLE_Server SHALL notify the Remote_Client.
 *
 * Validates: Requirement 4.6 (positive case)
 */
void test_notification_sent_when_connected_and_subscribed() {
  TestBleServer server;
  server.begin();

  // Client connected and subscribed
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  server.handleSubscribe(true);

  bool result = server.notifyCC(5, 74, 127);

  TEST_ASSERT_TRUE_MESSAGE(
      result, "notifyCC should return true when connected and subscribed");
  TEST_ASSERT_EQUAL_INT_MESSAGE(1, server.getNotifyCount(),
                                "One notification should be sent");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(5, server.getLastNotifyChannel(),
                                  "Notification channel should match");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(74, server.getLastNotifyController(),
                                  "Notification controller should match");
  TEST_ASSERT_EQUAL_UINT8_MESSAGE(127, server.getLastNotifyValue(),
                                  "Notification value should match");
}

/**
 * Test: Notification discarded after client disconnects.
 *
 * WHEN a subscribed client disconnects,
 * THEN subsequent notifications should be discarded.
 *
 * Validates: Requirement 4.6
 */
void test_notification_discarded_after_disconnect() {
  TestBleServer server;
  server.begin();

  // Connect and subscribe
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  server.handleSubscribe(true);

  // Verify notification works while connected
  TEST_ASSERT_TRUE(server.notifyCC(1, 0, 64));

  // Disconnect
  server.handleDisconnect();

  // Notification should now be discarded
  bool result = server.notifyCC(1, 0, 64);
  TEST_ASSERT_FALSE_MESSAGE(result,
                            "notifyCC should return false after disconnect");
  TEST_ASSERT_EQUAL_INT_MESSAGE(
      1, server.getNotifyCount(),
      "Only the pre-disconnect notification should have been sent");
}

/**
 * Test: Notification discarded after client unsubscribes.
 *
 * WHEN a connected client unsubscribes from notifications,
 * THEN subsequent notifications should be discarded.
 *
 * Validates: Requirement 4.6
 */
void test_notification_discarded_after_unsubscribe() {
  TestBleServer server;
  server.begin();

  // Connect and subscribe
  server.handleConnect(1, "AA:BB:CC:DD:EE:FF");
  server.handleSubscribe(true);

  // Verify notification works while subscribed
  TEST_ASSERT_TRUE(server.notifyCC(1, 0, 64));

  // Unsubscribe
  server.handleSubscribe(false);

  // Notification should now be discarded
  bool result = server.notifyCC(1, 0, 64);
  TEST_ASSERT_FALSE_MESSAGE(result,
                            "notifyCC should return false after unsubscribe");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Main
// ═══════════════════════════════════════════════════════════════════════════════

int main() {
  UNITY_BEGIN();

  // Single client connection and disconnection
  RUN_TEST(test_single_client_connects_successfully);
  RUN_TEST(test_single_client_disconnects_successfully);
  RUN_TEST(test_client_reconnects_after_disconnect);

  // Second client rejection
  RUN_TEST(test_second_client_rejected);
  RUN_TEST(test_multiple_second_clients_rejected);

  // Advertising resumes within 500ms after disconnect
  RUN_TEST(test_advertising_resumes_after_disconnect);
  RUN_TEST(test_advertising_resumes_immediately);

  // Display callbacks on connect/disconnect
  RUN_TEST(test_display_callback_on_connect);
  RUN_TEST(test_display_callback_on_disconnect);
  RUN_TEST(test_no_crash_without_callback);
  RUN_TEST(test_no_callback_for_rejected_client);

  // Notification discarded when no client connected
  RUN_TEST(test_notification_discarded_when_not_connected);
  RUN_TEST(test_notification_discarded_when_not_subscribed);
  RUN_TEST(test_notification_sent_when_connected_and_subscribed);
  RUN_TEST(test_notification_discarded_after_disconnect);
  RUN_TEST(test_notification_discarded_after_unsubscribe);

  return UNITY_END();
}
