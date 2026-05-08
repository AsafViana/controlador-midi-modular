#include "screens/PresetScreen.h"
#include "config.h"
#include "storage/PresetManager.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"
#include <cstdio>

const char *PresetScreen::_acoes[] = {"Carregar", "Salvar"};

PresetScreen::PresetScreen(OledApp *app, Storage *storage,
                           PresetManager *presets)
    : _app(app), _storage(storage), _presets(presets),
      _titulo(0, 0, "Presets", 1),
      _lista(0, CONTENT_Y, OLED_WIDTH, CONTENT_HEIGHT, 1) {
  addChild(&_titulo);

  for (uint8_t i = 0; i < 4; i++) {
    _slotPtrs[i] = _slotLabels[i];
  }
  _lista.setItems(_slotPtrs, 4);
  _lista.setUpButton(ButtonEvent::LONG_PRESS);
  _lista.setDownButton(ButtonEvent::SINGLE_CLICK);
}

void PresetScreen::atualizarLabels() {
  uint8_t active = _presets->getActivePreset();
  for (uint8_t i = 0; i < 4; i++) {
    const char *name = _presets->getPresetName(i);
    bool isActive = (i == active);
    bool hasData = _presets->hasData(i);

    if (isActive) {
      snprintf(_slotLabels[i], sizeof(_slotLabels[i]), ">%s%s", name,
               hasData ? "" : " (-)");
    } else {
      snprintf(_slotLabels[i], sizeof(_slotLabels[i]), " %s%s", name,
               hasData ? "" : " (-)");
    }
  }
}

void PresetScreen::onMount() {
  _modo = Modo::LISTA;
  atualizarLabels();
  markDirty();
}

bool PresetScreen::handleBack() {
  if (_modo == Modo::ACAO || _modo == Modo::CONFIRMAR_SALVAR) {
    _modo = Modo::LISTA;
    markDirty();
    return true;
  }
  return false; // Deixa Router fazer pop
}

void PresetScreen::handleInput(NavInput input) {
  if (_modo == Modo::LISTA) {
    switch (input) {
    case NavInput::UP:
      _lista.handleInput(ButtonEvent::LONG_PRESS);
      markDirty();
      break;
    case NavInput::DOWN:
      _lista.handleInput(ButtonEvent::SINGLE_CLICK);
      markDirty();
      break;
    case NavInput::SELECT:
      _selectedSlot = _lista.getSelectedIndex();
      _selectedAction = 0;
      _modo = Modo::ACAO;
      markDirty();
      break;
    default:
      break;
    }
  } else if (_modo == Modo::ACAO) {
    switch (input) {
    case NavInput::UP:
    case NavInput::DOWN:
      _selectedAction = (_selectedAction == 0) ? 1 : 0;
      markDirty();
      break;
    case NavInput::SELECT:
      if (_selectedAction == 0) {
        // Carregar
        _presets->loadPreset(_selectedSlot);
        if (_app)
          _app->showSaveConfirm();
        _modo = Modo::LISTA;
        atualizarLabels();
        markDirty();
      } else {
        // Salvar — pede confirmação
        _modo = Modo::CONFIRMAR_SALVAR;
        markDirty();
      }
      break;
    default:
      break;
    }
  } else if (_modo == Modo::CONFIRMAR_SALVAR) {
    switch (input) {
    case NavInput::SELECT:
      // Confirma salvar
      _presets->savePreset(_selectedSlot);
      if (_app)
        _app->showSaveConfirm();
      _modo = Modo::LISTA;
      atualizarLabels();
      markDirty();
      break;
    case NavInput::UP:
    case NavInput::DOWN:
    case NavInput::BACK:
      // Cancela
      _modo = Modo::ACAO;
      markDirty();
      break;
    default:
      break;
    }
  }
}

void PresetScreen::render(Adafruit_SSD1306 &display) {
  // Sempre renderiza título
  _titulo.render(display);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (_modo == Modo::LISTA) {
    _lista.render(display);
  } else if (_modo == Modo::ACAO) {
    const char *name = _presets->getPresetName(_selectedSlot);
    display.setCursor(0, CONTENT_Y);
    display.print(name);

    for (uint8_t i = 0; i < NUM_ACOES; i++) {
      int16_t y = CONTENT_Y + 14 + i * 12;
      if (i == _selectedAction) {
        display.fillRect(0, y, OLED_WIDTH, 10, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      display.setCursor(4, y + 1);
      display.print(_acoes[i]);
    }
  } else if (_modo == Modo::CONFIRMAR_SALVAR) {
    display.setCursor(0, CONTENT_Y + 4);
    display.print("Salvar config atual");
    display.setCursor(0, CONTENT_Y + 14);
    display.print("no preset?");
    display.setCursor(0, CONTENT_Y + 30);
    display.print("SELECT = Confirmar");
    display.setCursor(0, CONTENT_Y + 40);
    display.print("BACK = Cancelar");
  }
}
