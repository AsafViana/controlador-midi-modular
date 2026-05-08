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
  void sendProgramChange(uint8_t program, uint8_t canal);

  /// Processa mensagens MIDI recebidas (USB e DIN). Chamar no loop().
  void update();

  /// Registra callback chamado a cada mensagem MIDI enviada.
  void onActivity(MidiActivityCallback callback);

  /// Callback para CC recebido externamente (MIDI IN)
  using MidiCCReceivedCallback = void (*)(uint8_t cc, uint8_t valor,
                                          uint8_t canal);
  void onCCReceived(MidiCCReceivedCallback callback);

  /// Callback para SysEx recebido
  using SysExReceivedCallback = void (*)(const uint8_t *data, uint16_t length);
  void onSysExReceived(SysExReceivedCallback callback);

  /// Acesso à interface USB MIDI (para SysEx)
  USBMIDI_Interface &getUsbMidi();

  /// Habilita/desabilita MIDI Thru (reenvio entre interfaces)
  void setMidiThru(bool enabled);

  /// Define filtro de canal para MIDI IN (0 = aceitar todos)
  void setReceiveChannel(uint8_t canal);

private:
  USBMIDI_Interface _midi;
#ifdef ARDUINO
  HardwareSerialMIDI_Interface _midiDIN{Serial1};
#endif
  MidiActivityCallback _activityCallback = nullptr;
  MidiCCReceivedCallback _ccReceivedCallback = nullptr;
  SysExReceivedCallback _sysExReceivedCallback = nullptr;
  bool _midiThruEnabled = true;
  uint8_t _receiveChannel = 0; // 0 = todos

  /// Notifica o callback de atividade (se registrado).
  void notifyActivity();
};
