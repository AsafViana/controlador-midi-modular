#include "BleServer.h"
#include "../config.h"
#include "../midi/MidiEngine.h"
#include "BleProtocol.h"
#include "CCStateStore.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

// ── UUIDs do serviço GATT MIDI CC ────────────────────────────────────────────
static const char *SERVICE_UUID = "0000ff00-0000-1000-8000-00805f9b34fb";
static const char *CC_CHAR_UUID = "0000ff01-0000-1000-8000-00805f9b34fb";
static const char *BULK_CHAR_UUID = "0000ff02-0000-1000-8000-00805f9b34fb";

// ── Constantes de inicialização ──────────────────────────────────────────────
static constexpr uint8_t MAX_INIT_RETRIES = 3;
static constexpr uint32_t RETRY_DELAY_MS = 1000;
static constexpr uint32_t CC_WRITE_TIMEOUT_MS = 20;
static const char *DEVICE_NAME = BLE_DEVICE_NAME;

// ── Ponteiro global para acesso nas callbacks estáticas ──────────────────────
static BleServer *g_instance = nullptr;
static NimBLEServer *g_pServer = nullptr;
static NimBLECharacteristic *g_pCCChar = nullptr;
static NimBLECharacteristic *g_pBulkChar = nullptr;

// ═══════════════════════════════════════════════════════════════════════════════
// Callbacks do servidor BLE
// ═══════════════════════════════════════════════════════════════════════════════

class BleServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
    if (!g_instance)
      return;
    g_instance->_handleConnect(connInfo);
  }

  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo,
                    int reason) override {
    if (!g_instance)
      return;
    g_instance->_handleDisconnect();
  }
};

static BleServerCallbacks serverCallbacks;

// ═══════════════════════════════════════════════════════════════════════════════
// Callbacks da characteristic CC (Read/Write/Notify)
// ═══════════════════════════════════════════════════════════════════════════════

class CCCharCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic,
               NimBLEConnInfo &connInfo) override {
    if (!g_instance)
      return;
    NimBLEAttValue val = pCharacteristic->getValue();
    g_instance->handleCCWrite(val.data(), val.size());
  }

  void onRead(NimBLECharacteristic *pCharacteristic,
              NimBLEConnInfo &connInfo) override {
    if (!g_instance)
      return;

    // The client sets the characteristic value with [channel, controller]
    // before reading. We use those bytes to look up the CC value.
    NimBLEAttValue val = pCharacteristic->getValue();
    if (val.size() < 2)
      return;

    uint8_t channel = val.data()[0];
    uint8_t controller = val.data()[1];
    uint8_t response[3];
    size_t outLength = 0;

    g_instance->handleCCRead(channel, controller, response, &outLength);

    if (outLength > 0) {
      pCharacteristic->setValue(response, outLength);
    }
  }

  void onSubscribe(NimBLECharacteristic *pCharacteristic,
                   NimBLEConnInfo &connInfo, uint16_t subValue) override {
    if (!g_instance)
      return;
    // subValue: 0 = unsubscribed, 1 = notifications, 2 = indications, 3 = both
    g_instance->_handleSubscribe(subValue != 0);
  }
};

static CCCharCallbacks ccCharCallbacks;

// ═══════════════════════════════════════════════════════════════════════════════
// Callbacks da characteristic Bulk Read
// ═══════════════════════════════════════════════════════════════════════════════

class BulkCharCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *pCharacteristic,
               NimBLEConnInfo &connInfo) override {
    if (!g_instance || !g_instance->_getStore())
      return;

    // O cliente escreve o canal desejado (1 byte).
    // Capturamos o snapshot imediatamente (snapshot isolation: estado no início
    // do read).
    NimBLEAttValue val = pCharacteristic->getValue();
    if (val.size() < 1)
      return;

    uint8_t channel = val.data()[0];
    uint8_t snapshot[128];
    size_t outLength = 0;
    g_instance->handleBulkRead(channel, snapshot, &outLength);

    if (outLength > 0) {
      // Define o valor da characteristic com o snapshot de 128 bytes.
      // O cliente pode então ler via ATT Long Read (Read Blob).
      pCharacteristic->setValue(snapshot, outLength);
    } else {
      // Canal inválido — define valor vazio para indicar erro.
      // NimBLE retornará um valor de comprimento 0 ao cliente.
      uint8_t empty = 0;
      pCharacteristic->setValue(&empty, 0);
    }
  }

  void onRead(NimBLECharacteristic *pCharacteristic,
              NimBLEConnInfo &connInfo) override {
    // O snapshot já foi capturado no onWrite (snapshot isolation).
    // O valor da characteristic já contém os 128 bytes ou está vazio.
    // NimBLE gerencia automaticamente o ATT Long Read (Read Blob) para
    // payloads > MTU.
  }
};

static BulkCharCallbacks bulkCharCallbacks;

// ═══════════════════════════════════════════════════════════════════════════════
// Implementação BleServer
// ═══════════════════════════════════════════════════════════════════════════════

bool BleServer::begin() {
  g_instance = this;
  _retryCount = 0;

  for (uint8_t attempt = 0; attempt < MAX_INIT_RETRIES; attempt++) {
    _retryCount = attempt + 1;

    if (_initBle()) {
      return true;
    }

    // Falha na inicialização — aguarda antes de tentar novamente
    if (attempt < MAX_INIT_RETRIES - 1) {
      delay(RETRY_DELAY_MS);
    }
  }

  // Todas as tentativas falharam — reporta erro ao sistema de display
  // (A integração com display será feita na task 7.4)
  log_e("BLE: Falha na inicializacao apos %d tentativas", MAX_INIT_RETRIES);
  return false;
}

void BleServer::stop() {
  _connected = false;
  _clientSubscribed = false;
  _connectedClientHandle = 0;

  if (g_pServer != nullptr) {
    // Desconecta clientes ativos
    if (g_pServer->getConnectedCount() > 0) {
      g_pServer->disconnect(0);
    }
    NimBLEDevice::stopAdvertising();
  }

  NimBLEDevice::deinit(true);

  g_pServer = nullptr;
  g_pCCChar = nullptr;
  g_pBulkChar = nullptr;
  g_instance = nullptr;
}

bool BleServer::isConnected() const { return _connected; }

bool BleServer::notifyCC(uint8_t channel, uint8_t controller, uint8_t value) {
  if (!_connected || !_clientSubscribed || g_pCCChar == nullptr) {
    return false;
  }

  CCMessage msg = {channel, controller, value};
  uint8_t buffer[3];
  BleProtocol::serialize(msg, buffer);

  g_pCCChar->setValue(buffer, 3);
  g_pCCChar->notify();
  return true;
}

void BleServer::setCCStateStore(CCStateStore *store) { _store = store; }

void BleServer::setMidiEngine(MidiEngine *engine) { _engine = engine; }

void BleServer::onConnectionChange(ConnectionCallback callback) {
  _connectionCallback = callback;
}

uint8_t BleServer::getRetryCount() const { return _retryCount; }

// ── Handlers GATT ────────────────────────────────────────────────────────────

void BleServer::handleCCWrite(const uint8_t *data, size_t length) {
  // Captura timestamp de início para enforcement do timeout de 20ms
  const uint32_t startMs = millis();

  // Validação de dependências
  if (!_store || !_engine) {
    return;
  }

  // Parse da mensagem usando BleProtocol
  CCMessage msg;
  ParseResult result = BleProtocol::parse(data, length, msg);

  // Se parsing falhou, descarta o comando (mensagem inválida)
  if (result != ParseResult::OK) {
    // Log do erro para diagnóstico
    log_w("BLE CC Write rejeitado: parse error %d", static_cast<int>(result));
    return;
  }

  // Verifica timeout antes de prosseguir com operações mais custosas
  if ((millis() - startMs) >= CC_WRITE_TIMEOUT_MS) {
    log_w("BLE CC Write descartado: timeout antes de atualizar store");
    return;
  }

  // Atualiza o CCStateStore com o novo valor
  if (!_store->set(msg.channel, msg.controller, msg.value)) {
    // set() retorna false se coordenadas inválidas (proteção extra)
    log_w("BLE CC Write rejeitado: store->set() falhou ch=%d cc=%d val=%d",
          msg.channel, msg.controller, msg.value);
    return;
  }

  // Verifica timeout antes de enviar MIDI
  if ((millis() - startMs) >= CC_WRITE_TIMEOUT_MS) {
    log_w("BLE CC Write descartado: timeout antes de enviar MIDI");
    return;
  }

  // Envia a mensagem MIDI CC via USB e DIN
  MidiCC midiMsg(msg.controller, msg.value, msg.channel);
  _engine->sendCC(midiMsg);
}

void BleServer::handleCCRead(uint8_t channel, uint8_t controller,
                             uint8_t *outBuffer, size_t *outLength) {
  if (!_store) {
    *outLength = 0;
    return;
  }

  int16_t value = _store->get(channel, controller);
  if (value < 0) {
    // Invalid coordinates — signal ATT error by returning 0 length
    *outLength = 0;
    return;
  }

  CCMessage msg = {channel, controller, static_cast<uint8_t>(value)};
  BleProtocol::serialize(msg, outBuffer);
  *outLength = 3;
}

void BleServer::handleBulkRead(uint8_t channel, uint8_t *outBuffer,
                               size_t *outLength) {
  if (!_store) {
    *outLength = 0;
    return;
  }

  if (!_store->getChannelSnapshot(channel, outBuffer)) {
    *outLength = 0;
    return;
  }

  *outLength = 128;
}

// ── Advertising ──────────────────────────────────────────────────────────────

void BleServer::startAdvertising() {
  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setName(DEVICE_NAME);
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->enableScanResponse(true);
  pAdvertising->start();
}

void BleServer::stopAdvertising() { NimBLEDevice::stopAdvertising(); }

// ── Métodos internos (acessados pelas callbacks) ─────────────────────────────

bool BleServer::_initBle() {
  // Inicializa o stack NimBLE
  NimBLEDevice::init(DEVICE_NAME);

  // Cria o servidor
  g_pServer = NimBLEDevice::createServer();
  if (!g_pServer) {
    NimBLEDevice::deinit(true);
    return false;
  }
  g_pServer->setCallbacks(&serverCallbacks);

  // Cria o serviço MIDI CC
  NimBLEService *pService = g_pServer->createService(SERVICE_UUID);
  if (!pService) {
    NimBLEDevice::deinit(true);
    g_pServer = nullptr;
    return false;
  }

  // Cria a characteristic CC Read/Write/Notify (0000ff01-...)
  g_pCCChar = pService->createCharacteristic(
      CC_CHAR_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY);
  if (!g_pCCChar) {
    NimBLEDevice::deinit(true);
    g_pServer = nullptr;
    return false;
  }
  g_pCCChar->setCallbacks(&ccCharCallbacks);

  // Cria a characteristic Bulk Read (0000ff02-...)
  // WRITE para o cliente definir o canal, READ para ler o snapshot via Long
  // Read
  g_pBulkChar = pService->createCharacteristic(
      BULK_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  if (!g_pBulkChar) {
    NimBLEDevice::deinit(true);
    g_pServer = nullptr;
    g_pCCChar = nullptr;
    return false;
  }
  g_pBulkChar->setCallbacks(&bulkCharCallbacks);

  // Inicia o serviço
  if (!pService->start()) {
    NimBLEDevice::deinit(true);
    g_pServer = nullptr;
    g_pCCChar = nullptr;
    g_pBulkChar = nullptr;
    return false;
  }

  // Inicia advertising
  startAdvertising();

  return true;
}

void BleServer::_handleConnect(NimBLEConnInfo &connInfo) {
  // Single-client enforcement: reject second connection if already connected
  if (_connected) {
    // Disconnect the new (second) client immediately
    g_pServer->disconnect(connInfo.getConnHandle());
    return;
  }

  _connected = true;
  _connectedClientHandle = connInfo.getConnHandle();
  stopAdvertising();

  // Request optimized connection parameters for low-latency CC control.
  // Interval is in units of 1.25ms:
  //   Min: 7.5ms / 1.25ms = 6
  //   Max: 30ms / 1.25ms = 24
  // Latency: 0 (no skipped events)
  // Supervision timeout: 400 (4000ms in 10ms units)
  g_pServer->updateConnParams(connInfo.getConnHandle(), 6, 24, 0, 400);

  if (_connectionCallback) {
    _connectionCallback(true, connInfo.getAddress().toString().c_str());
  }
}

void BleServer::_handleDisconnect() {
  _connected = false;
  _clientSubscribed = false;
  _connectedClientHandle = 0;

  if (_connectionCallback) {
    _connectionCallback(false, "");
  }

  // Resume advertising immediately after disconnection (well within 500ms
  // requirement)
  startAdvertising();
}

void BleServer::_handleSubscribe(bool subscribed) {
  _clientSubscribed = subscribed;
}

CCStateStore *BleServer::_getStore() const { return _store; }
