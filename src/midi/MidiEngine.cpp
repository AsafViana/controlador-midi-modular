#include "MidiEngine.h"

void MidiEngine::begin() {
    Control_Surface.begin();
}

void MidiEngine::onActivity(MidiActivityCallback callback) {
    _activityCallback = callback;
}

void MidiEngine::notifyActivity() {
    if (_activityCallback) {
        _activityCallback();
    }
}

void MidiEngine::sendNoteOn(const MidiNote& note) {
    _midi.sendNoteOn(
        MIDIAddress(note.nota, Channel(note.canal)),
        note.velocidade
    );
    notifyActivity();
}

void MidiEngine::sendNoteOff(const MidiNote& note) {
    _midi.sendNoteOff(
        MIDIAddress(note.nota, Channel(note.canal)),
        0
    );
    notifyActivity();
}

void MidiEngine::sendNoteOnOff(const MidiNote& note, uint16_t duracaoMs) {
    sendNoteOn(note);
    delay(duracaoMs);
    sendNoteOff(note);
}

void MidiEngine::sendCC(const MidiCC& cc) {
    _midi.sendControlChange(
        MIDIAddress(cc.controlador, Channel(cc.canal)),
        cc.valor
    );
    notifyActivity();
}
