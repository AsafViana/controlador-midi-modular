#pragma once
#include <Control_Surface.h>
#include "MidiNote.h"
#include "MidiCC.h"

/// Tipo do callback chamado a cada envio MIDI.
using MidiActivityCallback = void(*)();

/**
 * MidiEngine — Envio de mensagens MIDI por USB e DIN (serial).
 *
 * USB usa a USBMIDI_Interface da Control Surface.
 * DIN usa Serial1 do ESP32-S3 a 31250 baud, enviando bytes MIDI
 * diretamente (sem depender do pipe system da Control Surface).
 */
class MidiEngine {
public:
    void begin();
    void sendNoteOn(const MidiNote& note);
    void sendNoteOff(const MidiNote& note);
    void sendNoteOnOff(const MidiNote& note, uint16_t duracaoMs);
    void sendCC(const MidiCC& cc);

    /// Registra callback chamado a cada mensagem MIDI enviada.
    void onActivity(MidiActivityCallback callback);

private:
    USBMIDI_Interface _usbMidi;
    MidiActivityCallback _activityCallback = nullptr;

    /// Notifica o callback de atividade (se registrado).
    void notifyActivity();

#ifdef ARDUINO
    /// Envia 3 bytes MIDI pela porta DIN serial.
    void dinSend(uint8_t status, uint8_t data1, uint8_t data2);
#endif
};
