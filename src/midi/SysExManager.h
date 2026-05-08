#pragma once

#include <cstdint>

class MidiEngine;
class Storage;

/**
 * SysExManager — Backup e restore de configuração via MIDI SysEx.
 *
 * Formato SysEx proprietário:
 *   F0 7D 01 [cmd] [data...] F7
 *
 *   - F0: SysEx Start
 *   - 7D: Non-commercial/educational use (manufacturer ID)
 *   - 01: Device ID (fixo v1)
 *   - cmd: Comando (DUMP_REQUEST=0x01, DUMP_DATA=0x02)
 *   - data: Payload (7-bit encoded)
 *   - F7: SysEx End
 *
 * Comandos:
 *   DUMP_REQUEST (0x01): Solicita dump da config (recebido do host)
 *   DUMP_DATA    (0x02): Dados de configuração (enviado/recebido)
 *
 * Payload do DUMP_DATA:
 *   [schema_version] [canal] [oitava] [velocidade] [num_controles]
 *   [cc0] [cc1] ... [ccN-1]
 *   [checksum]
 *
 * Todos os bytes são 7-bit (0-127) para compatibilidade SysEx.
 */
class SysExManager {
public:
  static constexpr uint8_t MANUFACTURER_ID = 0x7D;
  static constexpr uint8_t DEVICE_ID = 0x01;
  static constexpr uint8_t CMD_DUMP_REQUEST = 0x01;
  static constexpr uint8_t CMD_DUMP_DATA = 0x02;

  SysExManager(MidiEngine *engine, Storage *storage);

  /// Envia dump completo da configuração via SysEx
  void sendDump();

  /// Processa mensagem SysEx recebida. Retorna true se foi tratada.
  bool processReceived(const uint8_t *data, uint16_t length);

  /// Indica se o último load foi bem-sucedido
  bool lastLoadSuccess() const;

private:
  MidiEngine *_engine;
  Storage *_storage;
  bool _lastLoadOk = false;

  /// Calcula checksum simples (XOR de todos os bytes)
  static uint8_t checksum(const uint8_t *data, uint16_t length);
};
