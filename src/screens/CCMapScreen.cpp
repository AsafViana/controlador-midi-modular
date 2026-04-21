#include "screens/CCMapScreen.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include "hardware/UnifiedControlList.h"
#include "Adafruit_SSD1306.h"
#include <cstdio>

CCMapScreen::CCMapScreen(Storage* storage, UnifiedControlList* ucl)
    : _storage(storage)
    , _app(nullptr)
    , _ucl(ucl)
    , _titulo(0, 0, "Endereco CC", 1)
{
    addChild(&_titulo);
}

// ── Helpers privados ────────────────────────────────────

uint8_t CCMapScreen::getTotalControles() const {
    if (_ucl) return _ucl->getNumControles();
    return HardwareMap::NUM_CONTROLES;
}

uint8_t CCMapScreen::getNumLocais() const {
    if (_ucl) return _ucl->getNumLocais();
    return HardwareMap::NUM_CONTROLES;
}

bool CCMapScreen::isRemoto(uint8_t idx) const {
    if (_ucl) return _ucl->isRemoto(idx);
    return false;
}

const char* CCMapScreen::getBaseLabel(uint8_t idx) const {
    if (_ucl) return _ucl->getLabel(idx);
    return HardwareMap::getLabel(idx);
}

const char* CCMapScreen::formatLabel(uint8_t idx, char* buf, uint8_t bufSize) const {
    if (isRemoto(idx)) {
        uint8_t addr = 0, ctrlIdx = 0;
        _ucl->getRemoteInfo(idx, addr, ctrlIdx);
        snprintf(buf, bufSize, "[%02X]%s", addr, getBaseLabel(idx));
        return buf;
    }
    // Para locais, retorna o label diretamente (sem cópia desnecessária)
    const char* label = getBaseLabel(idx);
    snprintf(buf, bufSize, "%s", label);
    return buf;
}

uint8_t CCMapScreen::getCC(uint8_t idx) const {
    if (isRemoto(idx)) {
        uint8_t addr = 0, ctrlIdx = 0;
        _ucl->getRemoteInfo(idx, addr, ctrlIdx);
        return _storage->getRemoteCC(addr, ctrlIdx);
    }
    return _storage->getControladorCC(idx);
}

void CCMapScreen::setCC(uint8_t idx, uint8_t cc) {
    if (isRemoto(idx)) {
        uint8_t addr = 0, ctrlIdx = 0;
        _ucl->getRemoteInfo(idx, addr, ctrlIdx);
        _storage->setRemoteCC(addr, ctrlIdx, cc);
        return;
    }
    _storage->setControladorCC(idx, cc);
}

bool CCMapScreen::isHabilitado(uint8_t idx) const {
    if (isRemoto(idx)) {
        uint8_t addr = 0, ctrlIdx = 0;
        _ucl->getRemoteInfo(idx, addr, ctrlIdx);
        return _storage->isRemoteEnabled(addr, ctrlIdx);
    }
    return _storage->isControleHabilitado(idx);
}

void CCMapScreen::setHabilitado(uint8_t idx, bool habilitado) {
    if (isRemoto(idx)) {
        uint8_t addr = 0, ctrlIdx = 0;
        _ucl->getRemoteInfo(idx, addr, ctrlIdx);
        _storage->setRemoteEnabled(addr, ctrlIdx, habilitado);
        return;
    }
    _storage->setControleHabilitado(idx, habilitado);
}

// ── Ciclo de vida ───────────────────────────────────────

void CCMapScreen::onMount() {
    _indice = 0;
    _modo = ModoEdicao::NENHUM;
    markDirty();
}

// ── Input ───────────────────────────────────────────────

void CCMapScreen::handleInput(ButtonEvent event) {
    const uint8_t total = getTotalControles();

    if (_modo == ModoEdicao::NENHUM) {
        // ── Modo navegação ──────────────────────────────
        if (event == ButtonEvent::DOUBLE_CLICK) {
            if (_app) _app->getRouter().pop();
            return;
        }
        if (event == ButtonEvent::PRESSED) {
            if (_indice < total - 1) _indice++;
            markDirty();
        }
        else if (event == ButtonEvent::LONG_PRESS) {
            if (_indice > 0) _indice--;
            markDirty();
        }
        else if (event == ButtonEvent::SINGLE_CLICK) {
            _ccTemp = getCC(_indice);
            _modo = ModoEdicao::EDITAR_CC;
            markDirty();
        }
    }
    else if (_modo == ModoEdicao::EDITAR_CC) {
        // ── Editando CC ─────────────────────────────────
        if (event == ButtonEvent::PRESSED) {
            if (_ccTemp < 127) _ccTemp++;
            markDirty();
        }
        else if (event == ButtonEvent::LONG_PRESS) {
            if (_ccTemp > 0) _ccTemp--;
            markDirty();
        }
        else if (event == ButtonEvent::SINGLE_CLICK) {
            setCC(_indice, _ccTemp);
            _onOffTemp = isHabilitado(_indice);
            _modo = ModoEdicao::EDITAR_ONOFF;
            markDirty();
        }
    }
    else if (_modo == ModoEdicao::EDITAR_ONOFF) {
        // ── Editando ON/OFF ─────────────────────────────
        if (event == ButtonEvent::PRESSED || event == ButtonEvent::LONG_PRESS) {
            _onOffTemp = !_onOffTemp;
            markDirty();
        }
        else if (event == ButtonEvent::SINGLE_CLICK) {
            setHabilitado(_indice, _onOffTemp);
            _modo = ModoEdicao::NENHUM;
            markDirty();
        }
    }
}

// ── Render ──────────────────────────────────────────────

void CCMapScreen::render(Adafruit_SSD1306& display) {
    Screen::render(display);

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    const uint8_t total = getTotalControles();
    char labelBuf[24];

    if (_modo == ModoEdicao::EDITAR_CC) {
        display.setCursor(0, 14);
        display.print("Editando CC:");

        display.setTextSize(2);
        display.setCursor(0, 26);
        display.print(formatLabel(_indice, labelBuf, sizeof(labelBuf)));

        snprintf(_lineBuf[0], sizeof(_lineBuf[0]), "CC: %d", _ccTemp);
        display.setCursor(0, 46);
        display.print(_lineBuf[0]);

    } else if (_modo == ModoEdicao::EDITAR_ONOFF) {
        display.setCursor(0, 14);
        display.print("Habilitar controle:");

        display.setTextSize(2);
        display.setCursor(0, 26);
        display.print(formatLabel(_indice, labelBuf, sizeof(labelBuf)));

        display.setCursor(0, 46);
        display.print(_onOffTemp ? "[ ON  ]" : "[ OFF ]");

    } else {
        // ── Lista de controles ──────────────────────────
        uint8_t startIdx = 0;
        if (_indice >= 4) startIdx = _indice - 3;

        uint8_t itemH = 10;
        uint8_t y0 = 14;

        for (uint8_t i = 0; i < 4 && (startIdx + i) < total; i++) {
            uint8_t idx = startIdx + i;
            uint8_t cc = getCC(idx);
            bool on = isHabilitado(idx);

            snprintf(_lineBuf[i], sizeof(_lineBuf[i]),
                     "%s %s CC%d", on ? "*" : " ",
                     formatLabel(idx, labelBuf, sizeof(labelBuf)), cc);

            int16_t y = y0 + i * (itemH + 2);

            if (idx == _indice) {
                display.fillRect(0, y, 128, itemH, SSD1306_WHITE);
                display.setTextColor(SSD1306_BLACK);
            } else {
                display.setTextColor(SSD1306_WHITE);
            }

            display.setCursor(2, y + 1);
            display.print(_lineBuf[i]);
        }
    }
}
