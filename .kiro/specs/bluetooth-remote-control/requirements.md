# Requirements Document

## Introduction

This feature adds Bluetooth Low Energy (BLE) remote control capability to the Modular MIDI Controller. External systems (initially an Android app) will be able to connect to the controller via BLE and remotely control all MIDI CC parameters. The remote client can read and write CC values across all 16 MIDI channels, effectively turning a smartphone into a wireless control surface for the controller.

## Glossary

- **BLE_Server**: The Bluetooth Low Energy GATT server running on the ESP32-S3, responsible for advertising, accepting connections, and exposing MIDI CC control characteristics.
- **Remote_Client**: An external device (initially an Android app) that connects to the BLE_Server to read and write MIDI CC parameters.
- **CC_Parameter**: A MIDI Control Change parameter identified by a controller number (0-127) and a MIDI channel (1-16).
- **CC_Value**: The value of a MIDI CC parameter, ranging from 0 to 127.
- **GATT_Service**: A BLE Generic Attribute Profile service that groups related characteristics for MIDI CC remote control.
- **GATT_Characteristic**: A BLE data endpoint within a GATT_Service that represents a readable/writable piece of data.
- **Connection_State**: The current state of the BLE link between the BLE_Server and a Remote_Client (disconnected, connected, authenticated).
- **MidiEngine**: The existing module responsible for sending MIDI messages via USB and DIN interfaces.
- **CC_State_Store**: An internal data structure that holds the current value of all CC parameters (128 controllers × 16 channels).

## Requirements

### Requirement 1: BLE Server Initialization

**User Story:** As a user, I want the controller to advertise itself via Bluetooth, so that my Android phone can discover and connect to it.

#### Acceptance Criteria

1. WHEN the controller powers on, THE BLE_Server SHALL initialize and begin advertising with the device name "Controlador MIDI BLE" within 2000ms of power-on.
2. THE BLE_Server SHALL expose a custom GATT_Service dedicated to MIDI CC remote control.
3. WHILE the BLE_Server is advertising, THE BLE_Server SHALL remain discoverable by any BLE-capable Remote_Client.
4. WHEN a Remote_Client connects, THE BLE_Server SHALL stop advertising to other devices.
5. WHEN the connected Remote_Client disconnects, THE BLE_Server SHALL resume advertising within 500ms.
6. IF BLE initialization fails during power-on, THEN THE BLE_Server SHALL report the failure to the display system and retry initialization up to 3 times with a 1000ms delay between attempts.
7. IF all BLE initialization retries are exhausted, THEN THE BLE_Server SHALL report a persistent error to the display system and THE controller SHALL continue normal MIDI operation without BLE functionality.

---

### Requirement 2: CC State Store

**User Story:** As a user, I want the controller to maintain the current state of all CC parameters, so that the remote app can read the current values at any time.

#### Acceptance Criteria

1. THE CC_State_Store SHALL maintain the current CC_Value (0–127) for all 128 CC_Parameters (numbered 0–127) across all 16 MIDI channels (numbered 1–16), totaling 2048 entries.
2. WHEN the controller powers on, THE CC_State_Store SHALL initialize all 2048 CC_Values to 0.
3. WHEN a CC message is sent locally (via potentiometer or button), THE CC_State_Store SHALL update the corresponding CC_Value before the MIDI message is transmitted on any output port.
4. WHEN a CC message is received via MIDI IN, THE CC_State_Store SHALL update the corresponding CC_Value within 1 ms of message reception.
5. WHEN a read is requested with a valid controller number (0–127) and a valid channel (1–16), THE CC_State_Store SHALL return the current CC_Value for that entry.
6. IF a read is requested with a controller number outside 0–127 or a channel outside 1–16, THEN THE CC_State_Store SHALL reject the request and return an error indication without modifying any stored value.

---

### Requirement 3: Remote CC Write (Client to Controller)

**User Story:** As a user, I want to send CC values from my phone to the controller, so that I can remotely control MIDI parameters.

#### Acceptance Criteria

1. WHEN the Remote_Client writes a CC_Parameter value via the GATT_Characteristic, THE BLE_Server SHALL parse the 3-byte message and extract the MIDI channel (byte 0), controller number (byte 1), and CC_Value (byte 2).
2. WHEN a valid CC write is received from the Remote_Client, THE MidiEngine SHALL send the corresponding MIDI CC message via USB and DIN interfaces.
3. WHEN a valid CC write is received from the Remote_Client, THE CC_State_Store SHALL update the corresponding entry.
4. IF the Remote_Client writes a CC_Value outside the range 0-127, THEN THE BLE_Server SHALL reject the write, return an error response, and leave the CC_State_Store unchanged.
5. IF the Remote_Client writes a controller number outside the range 0-127, THEN THE BLE_Server SHALL reject the write, return an error response, and leave the CC_State_Store unchanged.
6. IF the Remote_Client writes a MIDI channel outside the range 1-16, THEN THE BLE_Server SHALL reject the write, return an error response, and leave the CC_State_Store unchanged.
7. IF the BLE_Server rejects a CC write due to invalid values, THEN THE MidiEngine SHALL NOT send any MIDI message for that write.

---

### Requirement 4: Remote CC Read (Controller to Client)

**User Story:** As a user, I want to read the current CC values from my phone, so that I can see the current state of all parameters.

#### Acceptance Criteria

1. WHEN the Remote_Client reads the CC state GATT_Characteristic with a valid channel (1-16) and controller number (0-127), THE BLE_Server SHALL return the current CC_Value for the requested CC_Parameter from the CC_State_Store using the 3-byte binary format.
2. WHILE a Remote_Client is connected and subscribed to notifications on the CC state characteristic, WHEN a CC_Value changes locally or via MIDI IN, THE BLE_Server SHALL notify the Remote_Client of the updated value via BLE notification using the 3-byte binary format.
3. THE BLE_Server SHALL support reading any CC_Parameter across all 16 MIDI channels (128 controllers × 16 channels = 2048 total values).
4. WHEN the Remote_Client subscribes to notifications on the CC state characteristic, THE BLE_Server SHALL send change notifications for all subsequent CC_Value updates until the client unsubscribes or disconnects.
5. IF the Remote_Client reads the CC state GATT_Characteristic with a controller number outside 0-127 or a channel outside 1-16, THEN THE BLE_Server SHALL reject the read and return an error response.
6. IF a CC_Value changes while no Remote_Client is connected or subscribed, THEN THE BLE_Server SHALL discard the notification and not queue it for later delivery.

---

### Requirement 5: BLE Communication Protocol

**User Story:** As a developer, I want a well-defined binary protocol for BLE messages, so that the Android app can communicate efficiently with the controller.

#### Acceptance Criteria

1. THE BLE_Server SHALL use a binary message format of 3 bytes for CC operations: [channel (1 byte, valid range: 1–16), controller_number (1 byte, valid range: 0–127), cc_value (1 byte, valid range: 0–127)].
2. THE BLE_Server SHALL use the same 3-byte format for both write commands and read responses.
3. THE BLE_Server SHALL use the same 3-byte format for BLE notifications of CC_Value changes.
4. THE BLE_Server SHALL guarantee that for all messages with byte values within valid ranges, parsing then serializing produces an identical 3-byte sequence (round-trip property).
5. WHEN a message with fewer than 3 bytes is received, THE BLE_Server SHALL discard the message without modifying CC state and SHALL return a BLE error response indicating invalid length.
6. WHEN a message with more than 3 bytes is received, THE BLE_Server SHALL process only the first 3 bytes and ignore the remainder.
7. WHEN a 3-byte message is received with any byte value outside its valid range (channel not in 1–16, controller_number not in 0–127, or cc_value not in 0–127), THE BLE_Server SHALL discard the message without modifying CC state and SHALL return a BLE error response indicating invalid value.

---

### Requirement 6: Connection Management

**User Story:** As a user, I want the Bluetooth connection to be reliable, so that I can control my MIDI parameters without interruptions.

#### Acceptance Criteria

1. THE BLE_Server SHALL support exactly one connected Remote_Client at a time.
2. IF a second Remote_Client attempts to connect while a Remote_Client is already connected, THEN THE BLE_Server SHALL reject the incoming connection and maintain the existing connection unchanged.
3. WHEN the BLE connection is lost unexpectedly, THE BLE_Server SHALL resume advertising within 500ms.
4. WHILE no Remote_Client is connected, THE BLE_Server SHALL continue normal MIDI controller operation with no additional latency on local control processing or MIDI message output.
5. WHILE a Remote_Client is connected, THE BLE_Server SHALL continue normal MIDI controller operation with no additional latency on local control processing or MIDI message output.
6. WHEN a Remote_Client connects, THE BLE_Server SHALL report the connected state and the client device address to the display system for UI indication.
7. WHEN a Remote_Client disconnects, THE BLE_Server SHALL report the disconnected state to the display system for UI indication.

---

### Requirement 7: BLE and Local Control Coexistence

**User Story:** As a user, I want both the physical controls and the remote app to work simultaneously, so that I can use whichever is more convenient.

#### Acceptance Criteria

1. WHILE a Remote_Client is connected, THE controller SHALL continue processing local potentiometer and button inputs at the same scan rate and responsiveness as when no Remote_Client is connected.
2. WHEN a local control changes a CC_Value, THE CC_State_Store SHALL update and THE BLE_Server SHALL notify the Remote_Client with the updated value within 20ms.
3. WHEN the Remote_Client changes a CC_Value, THE CC_State_Store SHALL update and THE MidiEngine SHALL send the corresponding MIDI CC message via USB and DIN interfaces.
4. IF both a local control and the Remote_Client modify the same CC_Parameter within the same processing cycle, THEN THE CC_State_Store SHALL apply the value that was received last (last-write-wins) and THE BLE_Server SHALL notify the Remote_Client of the final stored value.
5. IF the BLE_Server fails to notify the Remote_Client of a local CC_Value change, THEN THE CC_State_Store SHALL retain the updated value and THE controller SHALL continue normal MIDI operation without blocking.

---

### Requirement 8: Performance and Resource Constraints

**User Story:** As a user, I want the Bluetooth feature to not impact the controller's responsiveness, so that my live performance is not affected.

#### Acceptance Criteria

1. WHEN a CC write command is received from the Remote_Client, THE BLE_Server SHALL complete the CC_State_Store update and trigger the MidiEngine send within 20ms of the GATT write callback.
2. WHEN a CC_Value changes locally or via MIDI IN, THE BLE_Server SHALL send the BLE notification to the connected Remote_Client within 20ms of the CC_State_Store update.
3. THE BLE_Server SHALL request a BLE connection interval between 7.5ms and 30ms from the Remote_Client during connection parameter negotiation.
4. WHILE the BLE_Server is active, THE controller SHALL maintain its MIDI processing loop cycle time within 1ms of the cycle time measured with BLE_Server inactive.
5. THE BLE_Server SHALL consume no more than 10KB of RAM for its application-level operation (excluding the CC_State_Store and the underlying ESP-IDF BLE stack).
6. IF the BLE_Server cannot complete a CC write command within 20ms, THEN THE BLE_Server SHALL discard the command and continue processing subsequent commands without blocking the MIDI processing loop.

---

### Requirement 9: Bulk CC State Transfer

**User Story:** As a user, I want to quickly sync all CC values when my phone connects, so that the app immediately shows the current state.

#### Acceptance Criteria

1. WHEN a Remote_Client connects and requests a full state sync, THE BLE_Server SHALL provide all current CC_Values for a requested MIDI channel (128 values) within 200ms of the read request.
2. THE BLE_Server SHALL expose a dedicated GATT_Characteristic for bulk read operations that returns all 128 CC_Values for a specified channel using the BLE ATT Long Read mechanism (Read Blob) when the response exceeds the negotiated MTU size.
3. WHEN the Remote_Client reads the bulk characteristic with a channel byte (value 1-16), THE BLE_Server SHALL return 128 bytes representing CC_Values 0-127 in ascending controller-number order for that channel, captured as an atomic snapshot of the CC_State_Store at the time the read begins.
4. IF the Remote_Client requests a bulk read with an invalid channel (outside 1-16), THEN THE BLE_Server SHALL return a BLE ATT error response indicating an invalid parameter.
5. WHILE a bulk read is in progress, THE BLE_Server SHALL return the snapshot values captured at the start of the read, even if CC_Values are modified concurrently by local controls or MIDI IN.
