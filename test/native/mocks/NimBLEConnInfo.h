/**
 * Mock NimBLEConnInfo for native testing environment.
 * Provides minimal stubs for the NimBLE connection info class
 * used by BleServer's _handleConnect method.
 */
#pragma once

#include <cstdint>
#include <cstring>
#include <string>

/// Mock BLE address class
class NimBLEAddress {
public:
  NimBLEAddress() { memset(_addr, 0, 6); }
  NimBLEAddress(const char *str) { strncpy(_str, str, sizeof(_str) - 1); }

  std::string toString() const { return std::string(_str); }

private:
  uint8_t _addr[6] = {};
  char _str[32] = "AA:BB:CC:DD:EE:FF";
};

/// Mock NimBLEConnInfo class
class NimBLEConnInfo {
public:
  NimBLEConnInfo() : _connHandle(0) {}
  NimBLEConnInfo(uint16_t handle) : _connHandle(handle) {}
  NimBLEConnInfo(uint16_t handle, const char *addr)
      : _connHandle(handle), _address(addr) {}

  uint16_t getConnHandle() const { return _connHandle; }
  NimBLEAddress getAddress() const { return _address; }

private:
  uint16_t _connHandle;
  NimBLEAddress _address;
};
