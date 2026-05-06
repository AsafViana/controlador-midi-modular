#include "screens/CCMapScreen.h"
#include "Adafruit_SSD1306.h"
#include "config.h"
#include "hardware/UnifiedControlList.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include <cstdio>

CCMapScreen::CCMapScreen(Storage *storage, UnifiedControlList *ucl)
    : _storage(storage), _app(nullptr), _ucl(ucl),
      _titulo(0, 0, "Endereco CC", 1),
      _voltar(OLED_WIDTH - 48, 4, "<Voltar", 1) {
  addChild(&_titulo);
  addChild(&_voltar);
}

uint8_t CCMapScreen::getTotalControles() const {
  if (_ucl)
    return _ucl->getNumControles();
  return HardwareMap::NUM_CONTROLES;
}

uint8_t CCMapScreen::getNumLocais() const {
  if (_ucl)
    return _ucl->getNumLocais();
  return HardwareMap::NUM_CONTROLES;
}

bool CCMapScreen::isRemoto(uint8_t idx) const {
  if (_ucl)
    return _ucl->isRemoto(idx);
  return false;
}

void CCMapScreen::setApp(OledApp *app) { _app = app; }

const char *CCMapScreen::getBaseLabel(uint8_t idx) const {
  if (_ucl)
    return _ucl->getLabel(idx);
  return HardwareMap::getLabel(idx);
}

const char *CCMapScreen::formatLabel(uint8_t idx, char *buf,
                                     uint8_t bufSize) const {
  if (isRemoto(idx)) {
    uint8_t addr = 0, ctrlIdx = 0;
    _ucl->getRemoteInfo(idx, addr, ctrlIdx);
    snprintf(buf, bufSize, "[%02X]%s", addr, getBaseLabel(idx));
    return buf;
  }
  snprintf(buf, bufSize, "%s", getBaseLabel(idx));
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

void CCMapScreen::onMount() {
  _indice = 0;
  _modo = ModoEdicao::AGUARDANDO_CONTROLE;
  markDirty();
}

void CCMapScreen::notifyControlMoved(uint8_t unifiedIndex) {
  if (_modo != ModoEdicao::AGUARDANDO_CONTROLE)
    return;
  if (unifiedIndex >= getTotalControles())
    return;
  _indice = unifiedIndex;
  _ccTemp = getCC(_indice);
  _modo = ModoEdicao::EDITAR_CC;
  markDirty();
}

void CCMapScreen::handleInput(NavInput input) {
  const uint8_t total = getTotalControles();

  if (_modo == ModoEdicao::AGUARDANDO_CONTROLE) {
    // SELECT cancela e volta
    if (input == NavInput::SELECT && _app) {
      _app->getRouter().pop();
    }
    return;
  }

  if (_modo == ModoEdicao::NENHUM) {
    switch (input) {
    case NavInput::UP:
      if (_indice > 0) {
        _indice--;
        markDirty();
      }
      break;
    case NavInput::DOWN:
      if (_indice < total - 1) {
        _indice++;
        markDirty();
      }
      break;
    case NavInput::SELECT:
      _ccTemp = getCC(_indice);
      _modo = ModoEdicao::EDITAR_CC;
      markDirty();
      break;
    default:
      break;
    }
  } else if (_modo == ModoEdicao::EDITAR_CC) {
    uint8_t passo = 1;
    switch (input) {
    case NavInput::LONG_UP:
      passo = 5;
      // fall through
    case NavInput::UP:
      if (_ccTemp < 127) {
        _ccTemp += passo;
        if (_ccTemp > 127)
          _ccTemp = 127;
        markDirty();
      }
      break;
    case NavInput::LONG_DOWN:
      passo = 5;
      // fall through
    case NavInput::DOWN:
      if (_ccTemp > 0) {
        if (_ccTemp <= passo)
          _ccTemp = 0;
        else
          _ccTemp -= passo;
        markDirty();
      }
      break;
    case NavInput::SELECT:
      setCC(_indice, _ccTemp);
      _onOffTemp = isHabilitado(_indice);
      _modo = ModoEdicao::EDITAR_ONOFF;
      markDirty();
      break;
    default:
      break;
    }
  } else if (_modo == ModoEdicao::EDITAR_ONOFF) {
    switch (input) {
    case NavInput::UP:
    case NavInput::DOWN:
      _onOffTemp = !_onOffTemp;
      markDirty();
      break;
    case NavInput::SELECT:
      setHabilitado(_indice, _onOffTemp);
      if (_app)
        _app->showSaveConfirm();
      _modo = ModoEdicao::NENHUM;
      markDirty();
      break;
    default:
      break;
    }
  }
}

void CCMapScreen::render(Adafruit_SSD1306 &display) {
  renderChildren(display);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  const uint8_t total = getTotalControles();
  char labelBuf[24];

  if (_modo == ModoEdicao::AGUARDANDO_CONTROLE) {
    display.setCursor(0, CONTENT_Y + 8);
    display.print("Mova um controle...");
    display.setCursor(0, CONTENT_Y + 28);
    display.print("SELECT = Cancelar");

  } else if (_modo == ModoEdicao::EDITAR_CC) {
    display.setCursor(0, CONTENT_Y);
    display.print("Editando CC:");
    display.setTextSize(2);
    display.setCursor(0, CONTENT_Y + 12);
    display.print(formatLabel(_indice, labelBuf, sizeof(labelBuf)));
    snprintf(_lineBuf[0], sizeof(_lineBuf[0]), "CC: %d", _ccTemp);
    display.setTextSize(1);
    display.setCursor(0, CONTENT_Y + 32);
    display.print(_lineBuf[0]);

  } else if (_modo == ModoEdicao::EDITAR_ONOFF) {
    display.setCursor(0, CONTENT_Y);
    display.print("Habilitar:");
    display.setTextSize(2);
    display.setCursor(0, CONTENT_Y + 12);
    display.print(formatLabel(_indice, labelBuf, sizeof(labelBuf)));
    display.setTextSize(1);
    display.setCursor(0, CONTENT_Y + 32);
    display.print(_onOffTemp ? "[ ON  ]" : "[ OFF ]");

  } else {
    uint8_t startIdx = (_indice >= 4) ? _indice - 3 : 0;
    const uint8_t itemH = 10;
    const uint8_t y0 = CONTENT_Y;

    for (uint8_t i = 0; i < 4 && (startIdx + i) < total; i++) {
      uint8_t idx = startIdx + i;
      uint8_t cc = getCC(idx);
      bool on = isHabilitado(idx);

      snprintf(_lineBuf[i], sizeof(_lineBuf[i]), "%s %s CC%d", on ? "*" : " ",
               formatLabel(idx, labelBuf, sizeof(labelBuf)), cc);

      int16_t y = y0 + i * (itemH + 2);

      if (idx == _indice) {
        display.fillRect(0, y, OLED_WIDTH, itemH, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(2, y + 1);
      display.print(_lineBuf[i]);
    }
  }
}
