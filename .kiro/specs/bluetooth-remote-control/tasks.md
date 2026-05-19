# Implementation Plan: Bluetooth Remote Control

## Overview

This plan implements BLE remote control for the Modular MIDI Controller. The implementation follows a bottom-up approach: first the data layer (CCStateStore), then the protocol utilities (BleProtocol), then the BLE server (BleServer), and finally integration with existing modules (ControlReader, MidiEngine, main.cpp). Property-based tests validate correctness properties defined in the design using Rapidcheck in the native PlatformIO environment.

## Tasks

- [x] 1. Set up project structure and core interfaces
  - [x] 1.1 Create BLE module directory and header files
    - Create `src/ble/` directory
    - Create `src/ble/CCStateStore.h` with the `CCStateStore` class declaration as defined in the design
    - Create `src/ble/CCStateStore.cpp` with empty method stubs
    - Create `src/ble/BleProtocol.h` with the `BleProtocol` class, `CCMessage` struct, and `ParseResult` enum declarations
    - Create `src/ble/BleProtocol.cpp` with empty method stubs
    - Create `src/ble/BleServer.h` with the `BleServer` class declaration
    - Create `src/ble/BleServer.cpp` with empty method stubs
    - _Requirements: 1.2, 2.1, 5.1_

  - [x] 1.2 Set up native test environment with Rapidcheck
    - Add a `[env:native]` section to `platformio.ini` for host-based testing
    - Add Rapidcheck as a library dependency for the native environment
    - Create `test/native/` directory for property-based and unit tests
    - Create a minimal test file `test/native/test_main.cpp` that verifies the test framework runs
    - _Requirements: (testing infrastructure)_

- [x] 2. Implement CCStateStore
  - [x] 2.1 Implement CCStateStore core logic
    - Implement `begin()` to create the FreeRTOS mutex and zero-initialize `_values[16][128]`
    - Implement `set(channel, controller, value)` with mutex protection and range validation (channel 1–16, controller 0–127, value 0–127)
    - Implement `get(channel, controller)` with mutex protection and range validation, returning -1 for invalid coordinates
    - Implement `getChannelSnapshot(channel, outBuffer)` with mutex protection, copying 128 bytes atomically
    - Implement `onChange(callback)` to register the change callback, invoked inside `set()` after a successful write
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 2.6_

  - [x] 2.2 Write property test: CC State Store round-trip
    - **Property 2: Round trip consistency**
    - For any valid (channel, controller, value), `set()` followed by `get()` returns the written value
    - Create `test/native/test_ccstatestore_props.cpp`
    - **Validates: Requirements 2.1, 2.5**

  - [x] 2.3 Write property test: Invalid coordinates rejection
    - **Property 3: Invalid coordinates rejection**
    - For any channel outside 1–16 or controller outside 0–127, `set()` returns false and `get()` returns -1 without modifying stored values
    - **Validates: Requirements 2.6, 4.5, 9.4**

  - [x] 2.4 Write property test: Last-write-wins concurrency
    - **Property 8: Last-write-wins concurrency**
    - For any sequence of two writes to the same (channel, controller) with values A then B, `get()` returns B
    - **Validates: Requirements 7.4**

  - [x] 2.5 Write property test: Bulk read ordered snapshot
    - **Property 9: Bulk read ordered snapshot**
    - For any valid channel and any state, `getChannelSnapshot()` returns 128 bytes where byte[N] equals the stored value for controller N
    - **Validates: Requirements 9.3**

- [x] 3. Implement BleProtocol
  - [x] 3.1 Implement BleProtocol parse and serialize logic
    - Implement `parse(data, length, out)` — reject if length < 3, validate ranges, extract fields; if length > 3 process only first 3 bytes
    - Implement `serialize(msg, outBuffer)` — write channel, controller, value to 3-byte buffer
    - Implement `isValid(msg)` — check channel 1–16, controller 0–127, value 0–127
    - Create `src/ble/BleProtocol.cpp` with full implementation
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 5.7_

  - [x] 3.2 Write property test: Protocol message round-trip
    - **Property 1: Protocol message round-trip**
    - For any valid CCMessage, `serialize()` then `parse()` produces an identical CCMessage
    - Create `test/native/test_bleprotocol_props.cpp`
    - **Validates: Requirements 5.4, 3.1, 5.1**

  - [x] 3.3 Write property test: Invalid CC write rejection
    - **Property 5: Invalid CC write rejection**
    - For any 3-byte message with at least one field outside valid range, `parse()` returns an error ParseResult
    - **Validates: Requirements 3.4, 3.5, 3.6, 3.7, 5.7**

  - [x] 3.4 Write property test: Extra bytes tolerance
    - **Property 6: Extra bytes tolerance**
    - For any byte sequence longer than 3 bytes where first 3 form a valid message, `parse()` produces the same CCMessage as parsing only the first 3 bytes
    - **Validates: Requirements 5.6**

- [x] 4. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 5. Implement BleServer
  - [x] 5.1 Implement BLE initialization and advertising
    - Implement `begin()` using ESP-IDF NimBLE API: initialize BLE stack, create GATT service with UUID `0000ff00-...`, create CC Read/Write characteristic (`0000ff01-...`) with Read+Write+Notify properties, create Bulk Read characteristic (`0000ff02-...`) with Read property, add CCCD descriptor, start advertising with device name "Controlador MIDI BLE"
    - Implement retry logic: up to 3 attempts with 1000ms delay, report failure to display system on exhaustion
    - Implement `stop()` to tear down BLE resources
    - _Requirements: 1.1, 1.2, 1.3, 1.6, 1.7_

  - [x] 5.2 Implement connection management
    - Implement connection callback: stop advertising on connect, report to display with client address
    - Implement disconnection callback: resume advertising within 500ms, report to display
    - Implement single-client enforcement: reject second connection attempts
    - Implement `isConnected()` accessor
    - Implement `onConnectionChange(callback)` for external state reporting
    - _Requirements: 1.4, 1.5, 6.1, 6.2, 6.3, 6.6, 6.7_

  - [x] 5.3 Implement GATT write handler (Remote CC Write)
    - Implement `handleCCWrite(data, length)`: use `BleProtocol::parse()` to validate, update `CCStateStore`, call `MidiEngine::sendCC()`, return appropriate BLE ATT error codes on validation failure
    - Enforce 20ms timeout: discard command if processing exceeds deadline
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6, 3.7, 8.1, 8.6_

  - [x] 5.4 Implement GATT read handler and notifications (Remote CC Read)
    - Implement `handleCCRead(channel, controller, outBuffer, outLength)`: read from `CCStateStore`, serialize 3-byte response, return ATT error for invalid coordinates
    - Implement `notifyCC(channel, controller, value)`: serialize and send BLE notification if client is subscribed, non-blocking, discard if no client or not subscribed
    - Track subscription state via CCCD write callback
    - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5, 4.6_

  - [x] 5.5 Implement bulk read handler
    - Implement `handleBulkRead(channel, outBuffer, outLength)`: validate channel (1–16), call `CCStateStore::getChannelSnapshot()`, return 128 bytes via ATT Long Read (Read Blob), return ATT error for invalid channel
    - Ensure snapshot isolation: capture state at start of read
    - _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5_

  - [x] 5.6 Configure BLE connection parameters
    - Request connection interval between 7.5ms and 30ms during connection parameter negotiation
    - _Requirements: 8.3_

- [x] 6. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 7. Integration with existing modules
  - [x] 7.1 Integrate CCStateStore with ControlReader
    - Modify `src/hardware/ControlReader.cpp` to update `CCStateStore` when a local potentiometer or button generates a CC change (before sending via MidiEngine)
    - Ensure the update happens at the same scan rate with no additional latency
    - _Requirements: 2.3, 7.1, 7.2_

  - [x] 7.2 Integrate CCStateStore with MidiEngine (MIDI IN)
    - Modify `src/midi/MidiEngine.cpp` to update `CCStateStore` when a CC message is received via MIDI IN
    - Ensure update occurs within 1ms of reception
    - _Requirements: 2.4, 7.3_

  - [x] 7.3 Wire BleServer change callback for notifications
    - Register `CCStateStore::onChange` callback that calls `BleServer::notifyCC()` for every CC value change (local, MIDI IN, or BLE write)
    - Ensure notification is sent within 20ms of the store update
    - _Requirements: 4.2, 7.2, 8.2_

  - [x] 7.4 Integrate BleServer into main.cpp boot sequence
    - Instantiate `CCStateStore` and `BleServer` as globals in `main.cpp`
    - Call `CCStateStore::begin()` early in `setup()`
    - Call `BleServer::setCCStateStore()` and `BleServer::setMidiEngine()`
    - Call `BleServer::begin()` after LED init, before display/app init
    - Register `BleServer::onConnectionChange()` callback to update display/StatusLed
    - Ensure BLE does not block the main MIDI processing loop
    - _Requirements: 1.1, 6.4, 6.5, 6.6, 6.7, 8.4_

  - [x] 7.5 Write property test: Valid CC write propagation
    - **Property 4: Valid CC write propagation**
    - For any valid 3-byte BLE write, processing results in CCStateStore updated AND MidiEngine::sendCC called with matching parameters
    - Create `test/native/test_integration_props.cpp` using mocked MidiEngine
    - **Validates: Requirements 3.2, 3.3, 7.3**

  - [x] 7.6 Write property test: CC change notification format
    - **Property 7: CC change notification format**
    - For any CC value change while client is subscribed, the notification payload equals `BleProtocol::serialize()` output for that CCMessage
    - **Validates: Requirements 4.2, 5.3**

  - [x] 7.7 Write property test: Bulk read snapshot isolation
    - **Property 10: Bulk read snapshot isolation**
    - For any concurrent modification after a bulk read snapshot is taken, the response reflects the pre-modification values
    - **Validates: Requirements 9.5**

- [x] 8. Unit tests for connection and initialization logic
  - [x] 8.1 Write unit tests for BLE initialization and retry logic
    - Test successful init on first attempt
    - Test retry on failure (1, 2, 3 attempts) with 1000ms delays
    - Test persistent error reporting after 3 failures
    - Test controller continues MIDI operation after BLE failure
    - Create `test/native/test_bleserver_unit.cpp` with mocked BLE stack
    - _Requirements: 1.1, 1.6, 1.7_

  - [x] 8.2 Write unit tests for connection state management
    - Test single client connection and disconnection
    - Test second client rejection
    - Test advertising resumes within 500ms after disconnect
    - Test display callbacks on connect/disconnect
    - Test notification discarded when no client connected
    - _Requirements: 6.1, 6.2, 6.3, 6.6, 6.7, 4.6_

- [x] 9. Final checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation
- Property tests validate universal correctness properties from the design document
- Unit tests validate specific examples and edge cases
- All tests run in the `native` PlatformIO environment with mocked hardware dependencies
- The BLE stack runs on its own FreeRTOS task; integration must not block the main MIDI loop
- Rapidcheck is used for property-based testing in C++

## Task Dependency Graph

```json
{
  "waves": [
    { "id": 0, "tasks": ["1.1", "1.2"] },
    { "id": 1, "tasks": ["2.1", "3.1"] },
    { "id": 2, "tasks": ["2.2", "2.3", "2.4", "2.5", "3.2", "3.3", "3.4"] },
    { "id": 3, "tasks": ["5.1"] },
    { "id": 4, "tasks": ["5.2", "5.3", "5.4", "5.5", "5.6"] },
    { "id": 5, "tasks": ["7.1", "7.2"] },
    { "id": 6, "tasks": ["7.3", "7.4"] },
    { "id": 7, "tasks": ["7.5", "7.6", "7.7", "8.1", "8.2"] }
  ]
}
```
