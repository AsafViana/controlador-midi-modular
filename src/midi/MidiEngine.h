#pragma once
#include "MidiCC.h"
#include "MidiNote.h"
#include <Control_Surface.h>

/// Tipo do callback chamado a cada envio MIDI.
using MidiActivityCallback = void (*)();

/**
 * MidiEngine — Envio de mensagens MIDI via USB e DIN (Serial1).
 *
 * Todas as mensagens são enviadas simultaneamente por ambas as
 * interfaces (USB e MIDI DIN a 31250 baud).
 */
class MidiEngine {
public:
  void begin();
  void sendNoteOn(const MidiNote &note);
  void sendNoteOff(const MidiNote &note);
  void sendNoteOnOff(const MidiNote &note, uint16_t duracaoMs);
  void sendCC(const MidiCC &cc);

  /// Registra callback chamado a cada mensagem MIDI enviada.
  void onActivity(MidiActivityCallback callback);

private:
  USBMIDI_Interface _midi;
#ifdef ARDUINO
  HardwareSerialMIDI_Interface _midiDIN{Serial1};
#endif
  MidiActivityCallback _activityCallback = nullptr;

  /// Notifica o callback de atividade (se registrado).
  void notifyActivity();
};
