#include "hardware/UnifiedControlList.h"
#include "i2c/I2CScanner.h"
#include <cstring>

// ── Construtor ───────────────────────────────────────────

UnifiedControlList::UnifiedControlList(I2CScanner* scanner)
    : _scanner(scanner), _numControles(0), _numLocais(0) {
    memset(_controls, 0, sizeof(_controls));
    memset(_remoteLabels, 0, sizeof(_remoteLabels));
}

// ── rebuild() ────────────────────────────────────────────

void UnifiedControlList::rebuild() {
    _numControles = 0;
    _numLocais = 0;

    // 1. Adicionar controles locais do HardwareMap (índices 0..N-1)
    for (uint8_t i = 0; i < HardwareMap::NUM_CONTROLES; i++) {
        if (_numControles >= MAX_TOTAL_CONTROLS) break;

        ControlInfo& ctrl = _controls[_numControles];
        ctrl.label = HardwareMap::CONTROLES[i].label;
        ctrl.tipo = HardwareMap::CONTROLES[i].tipo;
        ctrl.valor = 0;  // valor atual é lido pelo ControlReader
        ctrl.ccPadrao = HardwareMap::CONTROLES[i].ccPadrao;
        ctrl.isRemoto = false;
        ctrl.moduleAddress = 0;
        ctrl.moduleCtrlIdx = 0;

        _numControles++;
    }
    _numLocais = _numControles;

    // 2. Adicionar controles remotos dos módulos descobertos pelo scanner
    if (_scanner == nullptr) return;

    uint8_t moduleCount = _scanner->getModuleCount();
    for (uint8_t m = 0; m < moduleCount; m++) {
        const ModuleInfo* mod = _scanner->getModule(m);
        if (mod == nullptr || !mod->connected) continue;

        for (uint8_t c = 0; c < mod->descriptor.numControles; c++) {
            if (_numControles >= MAX_TOTAL_CONTROLS) return;

            const RemoteControl& rc = mod->descriptor.controles[c];

            // Copiar label para buffer local (o descritor pode ser
            // sobrescrito em rescans futuros)
            uint8_t labelIdx = _numControles;
            strncpy(_remoteLabels[labelIdx], rc.label, 12);
            _remoteLabels[labelIdx][12] = '\0';

            ControlInfo& ctrl = _controls[_numControles];
            ctrl.label = _remoteLabels[labelIdx];
            ctrl.tipo = rc.tipo;
            ctrl.valor = rc.valor;
            ctrl.ccPadrao = rc.valor;  // CC padrão baseado no descritor
            ctrl.isRemoto = true;
            ctrl.moduleAddress = mod->address;
            ctrl.moduleCtrlIdx = c;

            _numControles++;
        }
    }
}

// ── getNumControles() ────────────────────────────────────

uint8_t UnifiedControlList::getNumControles() const {
    return _numControles;
}

// ── getNumLocais() ───────────────────────────────────────

uint8_t UnifiedControlList::getNumLocais() const {
    return _numLocais;
}

// ── getControlInfo() ─────────────────────────────────────

bool UnifiedControlList::getControlInfo(uint8_t index, ControlInfo& out) const {
    if (index >= _numControles) return false;
    out = _controls[index];
    return true;
}

// ── getLabel() ───────────────────────────────────────────

const char* UnifiedControlList::getLabel(uint8_t index) const {
    if (index >= _numControles) return "???";
    return _controls[index].label;
}

// ── getTipo() ────────────────────────────────────────────

TipoControle UnifiedControlList::getTipo(uint8_t index) const {
    if (index >= _numControles) return TipoControle::BOTAO;
    return _controls[index].tipo;
}

// ── isAnalogico() ────────────────────────────────────────

bool UnifiedControlList::isAnalogico(uint8_t index) const {
    if (index >= _numControles) return false;
    TipoControle t = _controls[index].tipo;
    return t == TipoControle::POTENCIOMETRO || t == TipoControle::SENSOR;
}

// ── isRemoto() ───────────────────────────────────────────

bool UnifiedControlList::isRemoto(uint8_t index) const {
    if (index >= _numControles) return false;
    return _controls[index].isRemoto;
}

// ── getCCPadrao() ────────────────────────────────────────

uint8_t UnifiedControlList::getCCPadrao(uint8_t index) const {
    if (index >= _numControles) return 0;
    return _controls[index].ccPadrao;
}

// ── getRemoteInfo() ──────────────────────────────────────

bool UnifiedControlList::getRemoteInfo(uint8_t index, uint8_t& address, uint8_t& ctrlIdx) const {
    if (index >= _numControles) return false;
    if (!_controls[index].isRemoto) return false;

    address = _controls[index].moduleAddress;
    ctrlIdx = _controls[index].moduleCtrlIdx;
    return true;
}
