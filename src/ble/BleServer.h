#pragma once

#include <cstddef>
#include <cstdint>

// Forward declarations
class CCStateStore;
class MidiEngine;
class NimBLEConnInfo;

/**
 * BleServer — Gerencia o servidor BLE GATT, características e interação com o
 * cliente.
 *
 * Responsável por:
 *   - Inicialização e advertising BLE
 *   - Gerenciamento de conexão (single-client)
 *   - Leitura/escrita de CC via GATT characteristics
 *   - Notificações de mudança de CC para o cliente conectado
 *   - Bulk read de canal completo
 */
class BleServer {
public:
  /// Inicializa o stack BLE e começa advertising.
  /// Retorna true em sucesso, false em falha.
  bool begin();

  /// Para o servidor BLE e libera recursos.
  void stop();

  /// Verifica se um cliente está conectado.
  bool isConnected() const;

  /// Envia notificação de mudança de CC para o cliente conectado (se inscrito).
  /// Non-blocking. Retorna false se não há cliente ou não está inscrito.
  bool notifyCC(uint8_t channel, uint8_t controller, uint8_t value);

  /// Define a referência ao CC State Store para operações de leitura/escrita.
  void setCCStateStore(CCStateStore *store);

  /// Define a referência ao MidiEngine para encaminhar writes BLE para MIDI.
  void setMidiEngine(MidiEngine *engine);

  /// Tipo de callback para mudança de estado de conexão.
  using ConnectionCallback = void (*)(bool connected, const char *address);

  /// Registra callback para mudanças de conexão.
  void onConnectionChange(ConnectionCallback callback);

  /// Retorna o número de tentativas de inicialização (para diagnóstico).
  uint8_t getRetryCount() const;

  // ── Métodos internos acessados pelas callbacks NimBLE (friend-like) ────────
  void _handleConnect(NimBLEConnInfo &connInfo);
  void _handleDisconnect();
  void _handleSubscribe(bool subscribed);
  CCStateStore *_getStore() const;

  /// Handler de callback GATT write
  void handleCCWrite(const uint8_t *data, size_t length);

  /// Handler de callback GATT read
  void handleCCRead(uint8_t channel, uint8_t controller, uint8_t *outBuffer,
                    size_t *outLength);

  /// Handler de callback GATT bulk read
  void handleBulkRead(uint8_t channel, uint8_t *outBuffer, size_t *outLength);

private:
  CCStateStore *_store = nullptr;
  MidiEngine *_engine = nullptr;
  ConnectionCallback _connectionCallback = nullptr;
  bool _connected = false;
  bool _clientSubscribed = false;
  uint8_t _retryCount = 0;
  uint16_t _connectedClientHandle =
      0; // BLE connection handle of the active client

  /// Inicializa o stack BLE (chamado internamente por begin())
  bool _initBle();

  /// Inicia advertising BLE
  void startAdvertising();

  /// Para advertising BLE
  void stopAdvertising();
};
