#pragma once
#include <Control_Surface.h>
#include "MidiNote.h"

class MidiEngine {
public:
    void begin();
    void sendNoteOn(const MidiNote& note);
    void sendNoteOff(const MidiNote& note);
    void sendNoteOnOff(const MidiNote& note, uint16_t duracaoMs);

private:
    USBMIDI_Interface _midi;
};