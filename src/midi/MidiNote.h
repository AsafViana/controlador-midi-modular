#pragma once
#include <Control_Surface.h>
#include "config.h"

struct MidiNote {
    uint8_t  nota;
    uint8_t  velocidade;
    uint8_t  canal;

    MidiNote(uint8_t nota,
             uint8_t vel   = MIDI_DEFAULT_VELOCITY,
             uint8_t canal = MIDI_DEFAULT_CHANNEL)
        : nota(nota), velocidade(vel), canal(canal) {}
};