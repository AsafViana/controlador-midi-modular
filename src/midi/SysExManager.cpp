#include "midi/SysExManager.h"
#include "hardware/HardwareMap.h"
#include "midi/MidiEngine.h"
#include "storage/Storage.h"

#ifdef ARDUINO
#include <Control_Surface.h>
#endif

SysExManager::SysExManager(MidiEngine *engine, Storage *storage)
    : _engine(engine), _storage(storage) {}

uint8_t SysExManager::checksum(const uint8_t *data, uint16_t length) {
  uint8_t cs = 0;
  for (uint16_t i = 0; i < length; i++) {
    cs ^= data[i];
  }
  return cs & 0x7F; // Garante 7-bit
}

void SysExManager::sendDump() {
#ifdef ARDUINO
  if (_engine == nullptr || _storage == nullptr)
    return;

  // Monta payload: schema, canal, oitava, vel, numControles, cc[0..N-1],
  // checksum
  const uint8_t numControles = HardwareMap::NUM_CONTROLES;
  const uint16_t payloadSize =
      5 + numControles + 1; // header(5) + ccs + checksum
  const uint16_t totalSize =
      4 + payloadSize; // F0 + mfr + dev + cmd + payload + F7

  uint8_t msg[64]; // Buffer suficiente para até 32 controles
  uint16_t idx = 0;

  // Header SysEx
  msg[idx++] = 0xF0;
  msg[idx++] = MANUFACTURER_ID;
  msg[idx++] = DEVICE_ID;
  msg[idx++] = CMD_DUMP_DATA;

  // Payload
  uint16_t payloadStart = idx;
  msg[idx++] = Storage::NVS_SCHEMA_VERSION & 0x7F;
  msg[idx++] = _storage->getCanalMidi() & 0x7F;
  msg[idx++] = _storage->getOitava() & 0x7F;
  msg[idx++] = _storage->getVelocidade() & 0x7F;
  msg[idx++] = numControles & 0x7F;

  for (uint8_t i = 0; i < numControles; i++) {
    msg[idx++] = _storage->getControladorCC(i) & 0x7F;
  }

  // Checksum (sobre o payload, sem F0/header/F7)
  msg[idx] = checksum(&msg[payloadStart], idx - payloadStart);
  idx++;

  // End
  msg[idx++] = 0xF7;

  // Envia via USB MIDI (SysEx)
  // Control_Surface usa sendSysEx com array
  USBMIDI_Interface &midi = _engine->getUsbMidi();
  midi.sendSysEx(msg, idx);
#endif
}

bool SysExManager::processReceived(const uint8_t *data, uint16_t length) {
  _lastLoadOk = false;

  // Validação mínima de formato
  // Espera: [mfr] [dev] [cmd] [payload...] (sem F0/F7, já removidos pelo
  // parser)
  if (length < 4)
    return false;

  if (data[0] != MANUFACTURER_ID)
    return false;
  if (data[1] != DEVICE_ID)
    return false;

  uint8_t cmd = data[2];

  if (cmd == CMD_DUMP_REQUEST) {
    // Host pediu dump — envia
    sendDump();
    return true;
  }

  if (cmd == CMD_DUMP_DATA) {
    // Recebendo config do host
    // Payload começa em data[3]
    const uint8_t *payload = &data[3];
    uint16_t payloadLen = length - 3;

    // Mínimo: schema(1) + canal(1) + oitava(1) + vel(1) + numCtrl(1) +
    // checksum(1) = 6
    if (payloadLen < 6)
      return false;

    uint8_t numControles = payload[4];
    uint16_t expectedLen = 5 + numControles + 1; // header + ccs + checksum

    if (payloadLen < expectedLen)
      return false;

    // Verifica checksum
    uint8_t cs = checksum(payload, expectedLen - 1);
    if (cs != payload[expectedLen - 1])
      return false;

    // Aplica configuração
    uint8_t canal = payload[1];
    uint8_t oitava = payload[2];
    uint8_t vel = payload[3];

    if (_storage == nullptr)
      return false;

    _storage->setCanalMidi(canal);
    _storage->setOitava(oitava);
    _storage->setVelocidade(vel);

    uint8_t maxCtrl = (numControles < HardwareMap::NUM_CONTROLES)
                          ? numControles
                          : HardwareMap::NUM_CONTROLES;
    for (uint8_t i = 0; i < maxCtrl; i++) {
      _storage->setControladorCC(i, payload[5 + i]);
    }

    _lastLoadOk = true;
    return true;
  }

  return false;
}

bool SysExManager::lastLoadSuccess() const { return _lastLoadOk; }
