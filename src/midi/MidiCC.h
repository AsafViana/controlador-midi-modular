#pragma once
#include <Control_Surface.h>
#include "config.h"

struct MidiCC {
    uint8_t  controlador;  // Número do controlador CC (0-127)
    uint8_t  valor;        // Valor do CC (0-127)
    uint8_t  canal;        // Canal MIDI (1-16)

    MidiCC(uint8_t controlador,
           uint8_t valor = MIDI_DEFAULT_CC_VALUE,
           uint8_t canal = MIDI_DEFAULT_CHANNEL)
        : controlador(controlador), valor(valor), canal(canal) {}
};
