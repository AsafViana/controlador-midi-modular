#include "MidiEngine.h"
#include "config.h"

void MidiEngine::begin() {
#ifdef ARDUINO
    // Inicializa Serial1 para MIDI DIN (TX only, pino configurável)
    MIDI_DIN_SERIAL.begin(MIDI_DIN_BAUD, SERIAL_8N1, -1, MIDI_DIN_TX_PIN);
#endif
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

#ifdef ARDUINO
void MidiEngine::dinSend(uint8_t status, uint8_t data1, uint8_t data2) {
    MIDI_DIN_SERIAL.write(status);
    MIDI_DIN_SERIAL.write(data1 & 0x7F);
    MIDI_DIN_SERIAL.write(data2 & 0x7F);
}
#endif

void MidiEngine::sendNoteOn(const MidiNote& note) {
    _usbMidi.sendNoteOn(
        MIDIAddress(note.nota, Channel(note.canal)),
        note.velocidade
    );
#ifdef ARDUINO
    dinSend(0x90 | ((note.canal - 1) & 0x0F), note.nota, note.velocidade);
#endif
    notifyActivity();
}

void MidiEngine::sendNoteOff(const MidiNote& note) {
    _usbMidi.sendNoteOff(
        MIDIAddress(note.nota, Channel(note.canal)),
        0
    );
#ifdef ARDUINO
    dinSend(0x80 | ((note.canal - 1) & 0x0F), note.nota, 0);
#endif
    notifyActivity();
}

void MidiEngine::sendNoteOnOff(const MidiNote& note, uint16_t duracaoMs) {
    sendNoteOn(note);
    delay(duracaoMs);
    sendNoteOff(note);
}

void MidiEngine::sendCC(const MidiCC& cc) {
    _usbMidi.sendControlChange(
        MIDIAddress(cc.controlador, Channel(cc.canal)),
        cc.valor
    );
#ifdef ARDUINO
    dinSend(0xB0 | ((cc.canal - 1) & 0x0F), cc.controlador, cc.valor);
#endif
    notifyActivity();
}
