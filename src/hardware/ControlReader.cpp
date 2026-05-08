#include "hardware/ControlReader.h"
#include "hardware/ResponseCurve.h"
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
    _ultimoValor[i] = 255;
    _emaInitialized[i] = false;
    _emaValue[i] = 0.0f;
  }
  for (uint8_t i = 0; i < MAX_REMOTE_CONTROLS; i++) {
    _ultimoValorRemoto[i] = 255;
  }
}

void ControlReader::begin() {
#ifdef ARDUINO
  analogReadResolution(12);

  for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
    if (HardwareMap::isAnalogico(i)) {
      pinMode(HardwareMap::getGpio(i), INPUT);
    } else if (HardwareMap::isBotaoMidi(i)) {
      pinMode(HardwareMap::getGpio(i), INPUT_PULLUP);
    }
  }
#endif
}

void ControlReader::onCCActivity(CCActivityCallback callback) {
  _ccActivityCallback = callback;
}

uint8_t ControlReader::lerControle(uint8_t indice) {
  int leitura = analogRead(HardwareMap::getGpio(indice));

  if (!_emaInitialized[indice]) {
    _emaValue[indice] = static_cast<float>(leitura);
    _emaInitialized[indice] = true;
  } else {
    _emaValue[indice] = EMA_ALPHA * static_cast<float>(leitura) +
                        (1.0f - EMA_ALPHA) * _emaValue[indice];
  }

  float filtered = _emaValue[indice];

  // Usa calibração individual se disponível, senão usa defaults
  uint16_t adcMin = _storage->hasCalibration(indice)
                        ? _storage->getCalibMin(indice)
                        : ADC_MIN;
  uint16_t adcMax = _storage->hasCalibration(indice)
                        ? _storage->getCalibMax(indice)
                        : ADC_MAX;

  if (filtered < static_cast<float>(adcMin))
    filtered = static_cast<float>(adcMin);
  if (filtered > static_cast<float>(adcMax))
    filtered = static_cast<float>(adcMax);

  uint8_t valor =
      static_cast<uint8_t>((filtered - static_cast<float>(adcMin)) * 127.0f /
                           static_cast<float>(adcMax - adcMin));
  if (valor > 127)
    valor = 127;

  if (HardwareMap::isInvertido(indice)) {
    valor = 127 - valor;
  }

  // Aplica curva de resposta configurada
  CurvaResposta curva = static_cast<CurvaResposta>(_storage->getCurva(indice));
  valor = ResponseCurve::aplicar(curva, valor);

  return valor;
}

void ControlReader::update() {
  uint32_t now = millis();
  if (now - _ultimaLeitura < INTERVALO_MS) {
    return;
  }
  _ultimaLeitura = now;

  uint8_t canal = _storage->getCanalMidi();

  // ── Controles locais ─────────────────────────────────────────────────────
  for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
    if (!HardwareMap::isAnalogico(i))
      continue;

    // Todos os controles locais são sempre ativos — sem flag de habilitado.
    uint8_t valor = lerControle(i);

    uint8_t ultimo = _ultimoValor[i];
    if (ultimo != 255) {
      int diff = (int)valor - (int)ultimo;
      if (diff < 0)
        diff = -diff;
      if (diff <= ZONA_MORTA)
        continue;
    }

    uint8_t cc = _storage->getControladorCC(i);
    MidiCC msg(cc, valor, canal);
    _engine->sendCC(msg);

    _ultimoValor[i] = valor;

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

  // ── Botões MIDI ──────────────────────────────────────────────────────────
  processarBotoesMidi(canal);

  // ── Controles remotos ────────────────────────────────────────────────────
  if (_ucl == nullptr || _scanner == nullptr)
    return;

  uint8_t numLocais = _ucl->getNumLocais();
  uint8_t numTotal = _ucl->getNumControles();

  uint8_t moduleValues[I2CProtocol::MAX_CONTROLES_POR_MODULO];

  uint8_t moduleCount = _scanner->getModuleCount();
  for (uint8_t m = 0; m < moduleCount; m++) {
    const ModuleInfo *mod = _scanner->getModule(m);
    if (mod == nullptr || !mod->connected)
      continue;

    uint8_t numCtrl = mod->descriptor.numControles;
    bool readOk = _scanner->readValues(m, moduleValues, numCtrl);

    for (uint8_t c = 0; c < numCtrl; c++) {
      for (uint8_t idx = numLocais; idx < numTotal; idx++) {
        uint8_t addr, ctrlIdx;
        if (!_ucl->getRemoteInfo(idx, addr, ctrlIdx))
          continue;
        if (addr != mod->address || ctrlIdx != c)
          continue;

        if (!_ucl->isAnalogico(idx))
          break;

        // Controles remotos também sempre ativos.
        uint8_t remoteIdx = idx - numLocais;
        if (remoteIdx >= MAX_REMOTE_CONTROLS)
          break;

        if (!readOk)
          break;

        uint8_t valor = moduleValues[c];
        if (valor > 127)
          valor = 127;

        uint8_t ultimo = _ultimoValorRemoto[remoteIdx];
        if (ultimo != 255) {
          int diff = (int)valor - (int)ultimo;
          if (diff < 0)
            diff = -diff;
          if (diff <= ZONA_MORTA)
            break;
        }

        uint8_t cc = _storage->getRemoteCC(addr, ctrlIdx);
        MidiCC msg(cc, valor, canal);
        _engine->sendCC(msg);

        _ultimoValorRemoto[remoteIdx] = valor;

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

void ControlReader::processarBotoesMidi(uint8_t canal) {
  uint32_t now = millis();

  for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
    if (!HardwareMap::isBotaoMidi(i))
      continue;

    // Lê estado do botão (active low com pull-up)
    bool pressed = (digitalRead(HardwareMap::getGpio(i)) == 0);

    // Debounce
    if (pressed != _btnMidiState[i].lastState) {
      if (now - _btnMidiState[i].lastChange < BTN_DEBOUNCE_MS)
        continue;
      _btnMidiState[i].lastChange = now;
      _btnMidiState[i].lastState = pressed;

      uint8_t cc = _storage->getControladorCC(i);
      uint8_t valor = 0;

      TipoControle tipo = HardwareMap::getTipo(i);

      if (tipo == TipoControle::BOTAO_MIDI_MOMENTANEO) {
        // Momentâneo: press=127, release=0
        valor = pressed ? 127 : 0;
        MidiCC msg(cc, valor, canal);
        _engine->sendCC(msg);
      } else if (tipo == TipoControle::BOTAO_MIDI_TOGGLE) {
        // Toggle: alterna a cada press (ignora release)
        if (pressed) {
          _btnMidiState[i].toggleValue = !_btnMidiState[i].toggleValue;
          valor = _btnMidiState[i].toggleValue ? 127 : 0;
          MidiCC msg(cc, valor, canal);
          _engine->sendCC(msg);
        } else {
          continue; // Release não envia nada no toggle
        }
      }

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
  }
}
