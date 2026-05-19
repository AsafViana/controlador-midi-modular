#pragma once

#include <cstddef>
#include <cstdint>


/**
 * CCMessage — Representa uma mensagem CC no protocolo BLE de 3 bytes.
 */
struct CCMessage {
  uint8_t channel;    // 1-16
  uint8_t controller; // 0-127
  uint8_t value;      // 0-127
};

/**
 * ParseResult — Resultado da análise de uma mensagem BLE.
 */
enum class ParseResult {
  OK,
  ERROR_TOO_SHORT,
  ERROR_INVALID_CHANNEL,
  ERROR_INVALID_CONTROLLER,
  ERROR_INVALID_VALUE
};

/**
 * BleProtocol — Utilitário stateless para parsing e serialização
 * do protocolo binário BLE de 3 bytes.
 *
 * Formato: [channel (1 byte, 1–16), controller (1 byte, 0–127), value (1 byte,
 * 0–127)]
 */
class BleProtocol {
public:
  /// Faz parse de um buffer de bytes em um CCMessage.
  /// Se length > 3, apenas os 3 primeiros bytes são usados.
  /// Retorna ParseResult indicando sucesso ou erro específico.
  static ParseResult parse(const uint8_t *data, size_t length, CCMessage &out);

  /// Serializa um CCMessage em um buffer de 3 bytes.
  /// O chamador deve fornecer buffer de pelo menos 3 bytes.
  static void serialize(const CCMessage &msg, uint8_t *outBuffer);

  /// Valida um CCMessage (channel 1-16, controller 0-127, value 0-127).
  static bool isValid(const CCMessage &msg);
};
