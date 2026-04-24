#include "hardware/ControlReader.h"
#include "i2c/I2CScanner.h"
#include "midi/MidiCC.h"
#include "midi/MidiEngine.h"
#include "storage/Storage.h"


#ifdef ARDUINO
#include <Arduino.h>
#else
#include "Arduino.h"
#endif

ControlReader::ControlReader(MidiEngine *engine, Storage *storage,
                             UnifiedControlList *ucl, I2CScanner *scanner)
    : _engine(engine), _storage(storage), _ucl(ucl), _scanner(scanner) {
  for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
    _ultimoValor[i] = 255; // Valor impossível para forçar primeiro envio
  }
  for (uint8_t i = 0; i < MAX_REMOTE_CONTROLS; i++) {
    _ultimoValorRemoto[i] = 255;
  }
}

void ControlReader::begin() {
#ifdef ARDUINO
  analogReadResolution(12); // 0-4095 no ESP32-S3

  for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
    if (HardwareMap::isAnalogico(i)) {
      pinMode(HardwareMap::getGpio(i), INPUT);
    }
  }
#endif
}

void ControlReader::onCCActivity(CCActivityCallback callback) {
  _ccActivityCallback = callback;
}

uint8_t ControlReader::lerControle(uint8_t indice) {
  int leitura = analogRead(HardwareMap::getGpio(indice));

  // Converte 0-4095 para 0-127
  uint8_t valor = leitura / 32;
  if (valor > 127)
    valor = 127;

  // Inverte se necessário (ex: LDR — mais luz = menos valor)
  if (HardwareMap::isInvertido(indice)) {
    valor = 127 - valor;
  }

  return valor;
}

void ControlReader::update() {
  uint32_t now = millis();
  if (now - _ultimaLeitura < INTERVALO_MS) {
    return;
  }
  _ultimaLeitura = now;

  uint8_t canal = _storage->getCanalMidi();

  // ── Controles locais (sempre processados) ────────────
  for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
    // Pula controles não-analógicos
    if (!HardwareMap::isAnalogico(i))
      continue;

    // Pula controles desabilitados
    if (!_storage->isControleHabilitado(i))
      continue;

    uint8_t valor = lerControle(i);

    // Zona morta: só envia se mudou significativamente
    uint8_t ultimo = _ultimoValor[i];
    if (ultimo != 255) { // 255 = primeiro envio, sempre envia
      int diff = (int)valor - (int)ultimo;
      if (diff < 0)
        diff = -diff;
      if (diff <= ZONA_MORTA)
        continue;
    }

    // Envia CC com o controlador configurado no Storage
    uint8_t cc = _storage->getControladorCC(i);
    MidiCC msg(cc, valor, canal);
    _engine->sendCC(msg);

    _ultimoValor[i] = valor;

    // Notifica callback com dados completos do controle
    if (_ccActivityCallback) {
      CCActivityInfo info;
      info.label = HardwareMap::getLabel(i);
      info.cc = cc;
      info.valor = valor;
      info.canal = canal;
      info.isRemoto = false;
      info.moduleAddress = 0;
      _ccActivityCallback(info);
    }
  }

  // ── Controles remotos (somente se UCL e scanner disponíveis) ──
  if (_ucl == nullptr || _scanner == nullptr)
    return;

  uint8_t numLocais = _ucl->getNumLocais();
  uint8_t numTotal = _ucl->getNumControles();

  // Buffer para valores lidos de um módulo
  uint8_t moduleValues[I2CProtocol::MAX_CONTROLES_POR_MODULO];

  // Iterar por cada módulo conectado e ler seus valores de uma vez
  uint8_t moduleCount = _scanner->getModuleCount();
  for (uint8_t m = 0; m < moduleCount; m++) {
    const ModuleInfo *mod = _scanner->getModule(m);
    if (mod == nullptr || !mod->connected)
      continue;

    uint8_t numCtrl = mod->descriptor.numControles;
    bool readOk = _scanner->readValues(m, moduleValues, numCtrl);

    // Para cada controle deste módulo, encontrar na lista unificada
    for (uint8_t c = 0; c < numCtrl; c++) {
      for (uint8_t idx = numLocais; idx < numTotal; idx++) {
        uint8_t addr, ctrlIdx;
        if (!_ucl->getRemoteInfo(idx, addr, ctrlIdx))
          continue;
        if (addr != mod->address || ctrlIdx != c)
          continue;

        // Encontrou o controle na lista unificada
        if (!_ucl->isAnalogico(idx))
          break;

        // Verificar se está habilitado no Storage
        if (!_storage->isRemoteEnabled(addr, ctrlIdx))
          break;

        uint8_t remoteIdx = idx - numLocais;
        if (remoteIdx >= MAX_REMOTE_CONTROLS)
          break;

        // Obter valor: se leitura falhou, manter último valor
        uint8_t valor;
        if (readOk) {
          valor = moduleValues[c];
          if (valor > 127)
            valor = 127;
        } else {
          break;
        }

        // Zona morta: só envia se mudou significativamente
        uint8_t ultimo = _ultimoValorRemoto[remoteIdx];
        if (ultimo != 255) {
          int diff = (int)valor - (int)ultimo;
          if (diff < 0)
            diff = -diff;
          if (diff <= ZONA_MORTA)
            break;
        }

        // Envia CC com o controlador configurado no Storage (remoto)
        uint8_t cc = _storage->getRemoteCC(addr, ctrlIdx);
        MidiCC msg(cc, valor, canal);
        _engine->sendCC(msg);

        _ultimoValorRemoto[remoteIdx] = valor;

        // Notifica callback com dados completos do controle remoto
        if (_ccActivityCallback) {
          CCActivityInfo info;
          info.label = _ucl->getLabel(idx);
          info.cc = cc;
          info.valor = valor;
          info.canal = canal;
          info.isRemoto = true;
          info.moduleAddress = addr;
          _ccActivityCallback(info);
        }

        break;
      }
    }
  }
}
