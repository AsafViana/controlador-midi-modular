/**
 * NimBLE mock for native testing environment.
 *
 * Provides minimal stubs for NimBLE types used by BleServer.
 * Only enough to compile BleServer.cpp in native tests.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>


// ── NimBLE Property flags ────────────────────────────────────────────────────
namespace NIMBLE_PROPERTY {
constexpr uint8_t READ = 0x02;
constexpr uint8_t WRITE = 0x08;
constexpr uint8_t NOTIFY = 0x10;
} // namespace NIMBLE_PROPERTY

// ── Forward declarations ─────────────────────────────────────────────────────
class NimBLEServer;
class NimBLEService;
class NimBLECharacteristic;
class NimBLEAdvertising;

// ── NimBLEAddress ────────────────────────────────────────────────────────────
class NimBLEAddress {
public:
  std::string toString() const { return "00:00:00:00:00:00"; }
};

// ── NimBLEConnInfo ───────────────────────────────────────────────────────────
class NimBLEConnInfo {
public:
  uint16_t getConnHandle() const { return 0; }
  NimBLEAddress getAddress() const { return NimBLEAddress(); }
};

// ── NimBLEAttValue ───────────────────────────────────────────────────────────
class NimBLEAttValue {
public:
  NimBLEAttValue() : _size(0) { memset(_data, 0, sizeof(_data)); }
  const uint8_t *data() const { return _data; }
  size_t size() const { return _size; }
  void setValue(const uint8_t *d, size_t len) {
    _size = len < sizeof(_data) ? len : sizeof(_data);
    memcpy(_data, d, _size);
  }

private:
  uint8_t _data[256];
  size_t _size;
};

// ── NimBLECharacteristicCallbacks ────────────────────────────────────────────
class NimBLECharacteristicCallbacks {
public:
  virtual ~NimBLECharacteristicCallbacks() = default;
  virtual void onWrite(NimBLECharacteristic *, NimBLEConnInfo &) {}
  virtual void onRead(NimBLECharacteristic *, NimBLEConnInfo &) {}
  virtual void onSubscribe(NimBLECharacteristic *, NimBLEConnInfo &, uint16_t) {
  }
};

// ── NimBLECharacteristic ─────────────────────────────────────────────────────
class NimBLECharacteristic {
public:
  NimBLEAttValue getValue() { return _value; }
  void setValue(const uint8_t *data, size_t len) { _value.setValue(data, len); }
  void setCallbacks(NimBLECharacteristicCallbacks *) {}
  void notify() {}

private:
  NimBLEAttValue _value;
};

// ── NimBLEServerCallbacks ────────────────────────────────────────────────────
class NimBLEServerCallbacks {
public:
  virtual ~NimBLEServerCallbacks() = default;
  virtual void onConnect(NimBLEServer *, NimBLEConnInfo &) {}
  virtual void onDisconnect(NimBLEServer *, NimBLEConnInfo &, int) {}
};

// ── NimBLEService ────────────────────────────────────────────────────────────
class NimBLEService {
public:
  NimBLECharacteristic *createCharacteristic(const char *, uint8_t) {
    return &_char;
  }
  bool start() { return true; }

private:
  NimBLECharacteristic _char;
};

// ── NimBLEServer ─────────────────────────────────────────────────────────────
class NimBLEServer {
public:
  void setCallbacks(NimBLEServerCallbacks *) {}
  NimBLEService *createService(const char *) { return &_service; }
  int getConnectedCount() { return 0; }
  void disconnect(uint16_t) {}
  void updateConnParams(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t) {}

private:
  NimBLEService _service;
};

// ── NimBLEAdvertising ────────────────────────────────────────────────────────
class NimBLEAdvertising {
public:
  void setName(const char *) {}
  void addServiceUUID(const char *) {}
  void enableScanResponse(bool) {}
  void start() {}
  void stop() {}
};

// ── NimBLEDevice ─────────────────────────────────────────────────────────────
class NimBLEDevice {
public:
  static void init(const char *) {}
  static void deinit(bool) {}
  static NimBLEServer *createServer() {
    static NimBLEServer server;
    return &server;
  }
  static NimBLEAdvertising *getAdvertising() {
    static NimBLEAdvertising adv;
    return &adv;
  }
  static void stopAdvertising() {}
};
