/**
 * Mock MidiEngine for native testing environment.
 * Provides a testable implementation that records sendCC calls.
 */
#pragma once

#include <cstdint>
#include <vector>

struct MidiCC {
  uint8_t controlador; // CC controller number (0-127)
  uint8_t valor;       // CC value (0-127)
  uint8_t canal;       // MIDI channel (1-16)

  MidiCC(uint8_t controlador, uint8_t valor = 0, uint8_t canal = 1)
      : controlador(controlador), valor(valor), canal(canal) {}
};

class CCStateStore;

/// Tipo do callback chamado a cada envio MIDI.
using MidiActivityCallback = void (*)();

/**
 * Mock MidiEngine — Records sendCC calls for verification in tests.
 */
class MidiEngine {
public:
  void begin() {}
  void sendCC(const MidiCC &cc) { _calls.push_back(cc); }
  void update() {}
  void onActivity(MidiActivityCallback) {}

  using MidiCCReceivedCallback = void (*)(uint8_t cc, uint8_t valor,
                                          uint8_t canal);
  void onCCReceived(MidiCCReceivedCallback) {}

  using SysExReceivedCallback = void (*)(const uint8_t *data, uint16_t length);
  void onSysExReceived(SysExReceivedCallback) {}

  void setMidiThru(bool) {}
  void setDinInputEnabled(bool) {}
  void setReceiveChannel(uint8_t) {}
  void setCCStateStore(CCStateStore *) {}

  // ── Test helpers ──

  /// Returns the number of sendCC calls recorded.
  size_t getSendCCCount() const { return _calls.size(); }

  /// Returns the last sendCC call recorded.
  const MidiCC &getLastCall() const { return _calls.back(); }

  /// Returns all sendCC calls recorded.
  const std::vector<MidiCC> &getAllCalls() const { return _calls; }

  /// Resets the recorded calls.
  void reset() { _calls.clear(); }

private:
  std::vector<MidiCC> _calls;
};
