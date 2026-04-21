#pragma once
#include <cstdint>
#include "Arduino.h"

// --- Minimal mock for Control_Surface library ---

struct MIDIAddress {
    uint8_t note;
    uint8_t channel;
    MIDIAddress(uint8_t n, uint8_t ch) : note(n), channel(ch) {}
};

inline uint8_t Channel(uint8_t ch) { return ch; }

// Mock for tracking sent MIDI messages
namespace mock_midi {
    struct MidiMessage {
        uint8_t note;
        uint8_t channel;
        uint8_t velocity;
        bool    isNoteOn;
        // CC fields
        uint8_t controller;
        uint8_t ccValue;
        bool    isCC;
    };

    extern MidiMessage lastMessage;
    extern int         messageCount;

    void reset();
}

class USBMIDI_Interface {
public:
    void sendNoteOn(MIDIAddress addr, uint8_t velocity) {
        mock_midi::lastMessage = { addr.note, addr.channel, velocity, true, 0, 0, false };
        mock_midi::messageCount++;
    }

    void sendNoteOff(MIDIAddress addr, uint8_t velocity) {
        mock_midi::lastMessage = { addr.note, addr.channel, velocity, false, 0, 0, false };
        mock_midi::messageCount++;
    }

    void sendControlChange(MIDIAddress addr, uint8_t value) {
        mock_midi::lastMessage = { 0, addr.channel, 0, false, addr.note, value, true };
        mock_midi::messageCount++;
    }
};

// Mock for Control_Surface singleton
struct ControlSurfaceMock {
    void begin() {}
};

extern ControlSurfaceMock Control_Surface;

// MIDI note helpers
namespace MIDI_Notes {
    inline uint8_t C(uint8_t octave)  { return 12 * octave + 0; }
    inline uint8_t Db(uint8_t octave) { return 12 * octave + 1; }
    inline uint8_t D(uint8_t octave)  { return 12 * octave + 2; }
    inline uint8_t Eb(uint8_t octave) { return 12 * octave + 3; }
    inline uint8_t E(uint8_t octave)  { return 12 * octave + 4; }
    inline uint8_t F(uint8_t octave)  { return 12 * octave + 5; }
    inline uint8_t Gb(uint8_t octave) { return 12 * octave + 6; }
    inline uint8_t G(uint8_t octave)  { return 12 * octave + 7; }
    inline uint8_t Ab(uint8_t octave) { return 12 * octave + 8; }
    inline uint8_t A(uint8_t octave)  { return 12 * octave + 9; }
    inline uint8_t Bb(uint8_t octave) { return 12 * octave + 10; }
    inline uint8_t B(uint8_t octave)  { return 12 * octave + 11; }
}
