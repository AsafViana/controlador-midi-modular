#include "MidiEngine.h"
#ifdef ARDUINO
#include "hardware/HardwareMap.h"
#endif

void MidiEngine::begin() {
  Control_Surface.begin();
#ifdef ARDUINO
  Serial1.begin(31250, SERIAL_8N1, HardwareMap::PIN_MIDI_RX,
                HardwareMap::PIN_MIDI_TX);
#endif
}

void MidiEngine::onActivity(MidiActivityCallback callback) {
  _activityCallback = callback;
}

void MidiEngine::notifyActivity() {
  if (_activityCallback) {
    _activityCallback();
  }
}

void MidiEngine::sendNoteOn(const MidiNote &note) {
  _midi.sendNoteOn(MIDIAddress(note.nota, Channel(note.canal)),
                   note.velocidade);
#ifdef ARDUINO
  _midiDIN.sendNoteOn(MIDIAddress(note.nota, Channel(note.canal)),
                      note.velocidade);
#endif
  notifyActivity();
}

void MidiEngine::sendNoteOff(const MidiNote &note) {
  _midi.sendNoteOff(MIDIAddress(note.nota, Channel(note.canal)), 0);
#ifdef ARDUINO
  _midiDIN.sendNoteOff(MIDIAddress(note.nota, Channel(note.canal)), 0);
#endif
  notifyActivity();
}

void MidiEngine::sendNoteOnOff(const MidiNote &note, uint16_t duracaoMs) {
  sendNoteOn(note);
  delay(duracaoMs);
  sendNoteOff(note);
}

void MidiEngine::sendCC(const MidiCC &cc) {
  _midi.sendControlChange(MIDIAddress(cc.controlador, Channel(cc.canal)),
                          cc.valor);
#ifdef ARDUINO
  _midiDIN.sendControlChange(MIDIAddress(cc.controlador, Channel(cc.canal)),
                             cc.valor);
#endif
  notifyActivity();
}

void MidiEngine::sendProgramChange(uint8_t program, uint8_t canal) {
  if (program > 127)
    program = 127;
  if (canal < 1)
    canal = 1;
  if (canal > 16)
    canal = 16;

  _midi.sendProgramChange(MIDIAddress(program, Channel(canal)));
#ifdef ARDUINO
  _midiDIN.sendProgramChange(MIDIAddress(program, Channel(canal)));
#endif
  notifyActivity();
}

void MidiEngine::onCCReceived(MidiCCReceivedCallback callback) {
  _ccReceivedCallback = callback;
}

void MidiEngine::onSysExReceived(SysExReceivedCallback callback) {
  _sysExReceivedCallback = callback;
}

USBMIDI_Interface &MidiEngine::getUsbMidi() { return _midi; }

void MidiEngine::setMidiThru(bool enabled) { _midiThruEnabled = enabled; }

void MidiEngine::setReceiveChannel(uint8_t canal) {
  _receiveChannel = (canal > 16) ? 0 : canal;
}

void MidiEngine::update() {
#ifdef ARDUINO
  // Lê mensagens MIDI recebidas via USB
  MIDIReadEvent usbEvent = _midi.read();
  if (usbEvent != MIDIReadEvent::NO_MESSAGE) {
    ChannelMessage msg = _midi.getChannelMessage();
    uint8_t msgCanal = msg.getChannel().getRaw() + 1; // 1-based

    // Filtro de canal
    if (_receiveChannel == 0 || msgCanal == _receiveChannel) {
      // Processa CC recebido
      if (msg.getMessageType() == MIDIMessageType::ControlChange) {
        uint8_t cc = msg.getData1();
        uint8_t valor = msg.getData2();
        if (_ccReceivedCallback) {
          _ccReceivedCallback(cc, valor, msgCanal);
        }
      }

      // MIDI Thru: USB → DIN
      if (_midiThruEnabled) {
        _midiDIN.send(msg);
      }
    }
  }

  // Lê mensagens MIDI recebidas via DIN
  MIDIReadEvent dinEvent = _midiDIN.read();
  if (dinEvent != MIDIReadEvent::NO_MESSAGE) {
    ChannelMessage msg = _midiDIN.getChannelMessage();
    uint8_t msgCanal = msg.getChannel().getRaw() + 1;

    if (_receiveChannel == 0 || msgCanal == _receiveChannel) {
      if (msg.getMessageType() == MIDIMessageType::ControlChange) {
        uint8_t cc = msg.getData1();
        uint8_t valor = msg.getData2();
        if (_ccReceivedCallback) {
          _ccReceivedCallback(cc, valor, msgCanal);
        }
      }

      // MIDI Thru: DIN → USB
      if (_midiThruEnabled) {
        _midi.send(msg);
      }
    }
  }
#endif
}
