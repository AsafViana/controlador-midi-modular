#include "i2c/I2CScanner.h"
#include <cstring>

#ifdef ARDUINO
#include <Arduino.h>
#else
// Em builds nativos, millis() é fornecido pelo mock (test/mocks/Arduino.h).
// O mock define mock::currentMillis e inline millis() que o retorna.
#include "Arduino.h"
#endif

// ── Construtor ───────────────────────────────────────────

I2CScanner::I2CScanner(I2CBus *bus)
    : _bus(bus), _moduleCount(0), _lastScanTime(0) {
  memset(_modules, 0, sizeof(_modules));
}

// ── scan() ───────────────────────────────────────────────

uint8_t I2CScanner::scan() {
  _moduleCount = 0;

  for (uint8_t addr = I2CProtocol::ADDR_MIN; addr <= I2CProtocol::ADDR_MAX;
       addr++) {
    // Pular endereço do OLED
    if (addr == I2CProtocol::ADDR_OLED)
      continue;

    // Limite de módulos atingido
    if (_moduleCount >= MAX_MODULES)
      break;

    ModuleDescriptor desc;
    if (probeAndRead(addr, desc)) {
      ModuleInfo &info = _modules[_moduleCount];
      info.address = addr;
      info.descriptor = desc;
      info.connected = true;
      info.failCount = 0;
      _moduleCount++;
    }
  }

  _needsRebuild = true;
  _lastScanTime = millis();
  return _moduleCount;
}

// ── periodicScan() ───────────────────────────────────────

void I2CScanner::periodicScan() {
  uint32_t now = millis();

  // Respeitar intervalo de rescan
  if (now - _lastScanTime < RESCAN_INTERVAL_MS)
    return;

  _lastScanTime = now;

  // 1. Verificar módulos já conhecidos — detectar desconexões
  for (uint8_t i = 0; i < _moduleCount; i++) {
    if (!_modules[i].connected)
      continue;

    if (!_bus->probe(_modules[i].address)) {
      bool wasConnected = _modules[i].connected;
      registerFailure(i);
      if (wasConnected && !_modules[i].connected) {
        _needsRebuild = true;
      }
    } else {
      // Módulo ainda responde — resetar contagem de falhas
      _modules[i].failCount = 0;
    }
  }

  // 2. Procurar novos módulos em endereços não ocupados
  for (uint8_t addr = I2CProtocol::ADDR_MIN; addr <= I2CProtocol::ADDR_MAX;
       addr++) {
    if (addr == I2CProtocol::ADDR_OLED)
      continue;
    if (_moduleCount >= MAX_MODULES)
      break;

    // Verificar se já temos este endereço
    bool found = false;
    for (uint8_t i = 0; i < _moduleCount; i++) {
      if (_modules[i].address == addr && _modules[i].connected) {
        found = true;
        break;
      }
    }
    if (found)
      continue;

    // Tentar descobrir novo módulo
    ModuleDescriptor desc;
    if (probeAndRead(addr, desc)) {
      _needsRebuild = true;

      // Verificar se é um módulo previamente desconectado
      bool reused = false;
      for (uint8_t i = 0; i < _moduleCount; i++) {
        if (_modules[i].address == addr && !_modules[i].connected) {
          _modules[i].descriptor = desc;
          _modules[i].connected = true;
          _modules[i].failCount = 0;
          reused = true;
          break;
        }
      }

      if (!reused) {
        ModuleInfo &info = _modules[_moduleCount];
        info.address = addr;
        info.descriptor = desc;
        info.connected = true;
        info.failCount = 0;
        _moduleCount++;
      }
    }
  }
}

// ── readValues() ─────────────────────────────────────────

bool I2CScanner::readValues(uint8_t moduleIndex, uint8_t *values,
                            uint8_t maxLen) {
  if (moduleIndex >= _moduleCount)
    return false;

  ModuleInfo &info = _modules[moduleIndex];
  if (!info.connected)
    return false;

  uint8_t numControles = info.descriptor.numControles;
  if (numControles > maxLen)
    numControles = maxLen;

  // Enviar comando CMD_READ_VALUES
  uint8_t cmd = I2CProtocol::CMD_READ_VALUES;
  if (!_bus->write(info.address, &cmd, 1)) {
    registerFailure(moduleIndex);
    return false;
  }

  // Ler valores
  uint8_t bytesRead = _bus->requestFrom(info.address, values, numControles);
  if (bytesRead != numControles) {
    registerFailure(moduleIndex);
    return false;
  }

  // Leitura bem-sucedida — resetar contagem de falhas
  info.failCount = 0;

  // Atualizar valores no descritor (cache dos últimos valores conhecidos)
  for (uint8_t i = 0; i < numControles; i++) {
    info.descriptor.controles[i].valor = values[i];
  }

  return true;
}

// ── getModuleCount() ─────────────────────────────────────

uint8_t I2CScanner::getModuleCount() const { return _moduleCount; }

// ── getModule() ──────────────────────────────────────────

const ModuleInfo *I2CScanner::getModule(uint8_t index) const {
  if (index >= _moduleCount)
    return nullptr;
  return &_modules[index];
}

// ── getTotalRemoteControls() ─────────────────────────────

uint8_t I2CScanner::getTotalRemoteControls() const {
  uint8_t total = 0;
  for (uint8_t i = 0; i < _moduleCount; i++) {
    if (_modules[i].connected) {
      total += _modules[i].descriptor.numControles;
    }
  }
  return total;
}

// ── probeAndRead() ───────────────────────────────────────

bool I2CScanner::probeAndRead(uint8_t address, ModuleDescriptor &desc) {
  // 1. Probe — verificar se há dispositivo no endereço
  if (!_bus->probe(address))
    return false;

  // 2. Enviar comando CMD_DESCRIPTOR
  uint8_t cmd = I2CProtocol::CMD_DESCRIPTOR;
  if (!_bus->write(address, &cmd, 1))
    return false;

  // 3. Ler resposta (tamanho máximo: 1 + 16*14 = 225 bytes)
  uint8_t buffer[225];
  uint8_t bytesRead =
      _bus->requestFrom(address, buffer, static_cast<uint8_t>(sizeof(buffer)));
  if (bytesRead == 0)
    return false;

  // 4. Desserializar
  if (!I2CProtocol::deserialize(buffer, bytesRead, desc))
    return false;

  return true;
}

// ── registerFailure() ────────────────────────────────────

void I2CScanner::registerFailure(uint8_t moduleIndex) {
  if (moduleIndex >= _moduleCount)
    return;

  ModuleInfo &info = _modules[moduleIndex];
  info.failCount++;

  if (info.failCount >= MAX_FAIL_COUNT) {
    info.connected = false;
  }
}

// ── needsRebuild() ──────────────────────────────────────

bool I2CScanner::needsRebuild() const { return _needsRebuild; }

// ── clearRebuildFlag() ──────────────────────────────────

void I2CScanner::clearRebuildFlag() { _needsRebuild = false; }
