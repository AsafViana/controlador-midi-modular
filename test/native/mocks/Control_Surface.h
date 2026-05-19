/**
 * Mock Control_Surface.h for native testing environment.
 * Provides minimal stubs for types used by MidiEngine.h and MidiCC.h.
 */
#pragma once

#include <cstdint>

// Stub types that MidiEngine.h references
class USBMIDI_Interface {};
class HardwareSerialMIDI_Interface {
public:
  HardwareSerialMIDI_Interface(...) {}
};
