#include "MidiEngine.h"

void MidiEngine::begin() {
    Control_Surface.begin();
}

void MidiEngine::sendNoteOn(const MidiNote& note) {
    _midi.sendNoteOn(
        MIDIAddress(note.nota, Channel(note.canal)),
        note.velocidade
    );
}

void MidiEngine::sendNoteOff(const MidiNote& note) {
    _midi.sendNoteOff(
        MIDIAddress(note.nota, Channel(note.canal)),
        0
    );
}

void MidiEngine::sendNoteOnOff(const MidiNote& note, uint16_t duracaoMs) {
    sendNoteOn(note);
    delay(duracaoMs);
    sendNoteOff(note);
}