#pragma once
#include <Control_Surface.h>
#include "MidiNote.h"
#include "MidiCC.h"

/// Tipo do callback chamado a cada envio MIDI.
using MidiActivityCallback = void(*)();

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
    USBMIDI_Interface _midi;
    MidiActivityCallback _activityCallback = nullptr;

    /// Notifica o callback de atividade (se registrado).
    void notifyActivity();
};
