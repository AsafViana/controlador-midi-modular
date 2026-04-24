#include "screens/ConfigScreen.h"
#include "config.h"
#include "screens/CCMapScreen.h"
#include "screens/CanalScreen.h"
#include "screens/OitavaScreen.h"
#include "screens/VelocidadeScreen.h"
#include "storage/Storage.h"
#include "ui/OledApp.h"

const char *ConfigScreen::_opcoes[] = {"Mapa CC",    "Canal MIDI", "Oitava",
                                       "Velocidade", "Restaurar",  "Voltar"};

ConfigScreen::ConfigScreen(OledApp *app, Storage *storage, CCMapScreen *ccMap,
                           CanalScreen *canal, OitavaScreen *oitava,
                           VelocidadeScreen *velocidade)
    : _app(app), _storage(storage), _ccMap(ccMap), _canal(canal),
      _oitava(oitava), _velocidade(velocidade),
      _titulo(0, 0, "Configuracoes", 1),
      _voltar(OLED_WIDTH - 48, 4, "<Voltar", 1),
      _lista(0, CONTENT_Y, OLED_WIDTH, CONTENT_HEIGHT, 1),
      _confirmandoReset(false) {
  addChild(&_titulo);
  addChild(&_voltar);
  addChild(&_lista);
  _lista.setItems(_opcoes, NUM_OPCOES);
  _lista.setUpButton(ButtonEvent::LONG_PRESS);
  _lista.setDownButton(ButtonEvent::SINGLE_CLICK);
}

void ConfigScreen::onMount() {
  _confirmandoReset = false;
  markDirty();
}

void ConfigScreen::render(Adafruit_SSD1306 &display) {
  if (_confirmandoReset) {
    // Tela de confirmação de factory reset
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Restaurar Padroes?");

    display.setCursor(0, CONTENT_Y + 4);
    display.print("Todas as configs");
    display.setCursor(0, CONTENT_Y + 14);
    display.print("serao perdidas!");

    display.setCursor(0, CONTENT_Y + 30);
    display.print("SELECT = Confirmar");
    display.setCursor(0, CONTENT_Y + 40);
    display.print("UP/DOWN = Cancelar");
  } else {
    renderChildren(display);
  }
}

void ConfigScreen::handleInput(NavInput input) {
  if (_confirmandoReset) {
    switch (input) {
    case NavInput::SELECT:
      // Confirma factory reset
      _storage->factoryReset();
      _confirmandoReset = false;
      if (_app)
        _app->showSaveConfirm();
      markDirty();
      break;
    case NavInput::UP:
    case NavInput::DOWN:
      // Cancela
      _confirmandoReset = false;
      markDirty();
      break;
    default:
      break;
    }
    return;
  }

  Router &router = _app->getRouter();

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
    switch (_lista.getSelectedIndex()) {
    case 0:
      if (_ccMap)
        router.push(_ccMap);
      break;
    case 1:
      if (_canal)
        router.push(_canal);
      break;
    case 2:
      if (_oitava)
        router.push(_oitava);
      break;
    case 3:
      if (_velocidade)
        router.push(_velocidade);
      break;
    case 4:
      // Factory reset — pede confirmação
      _confirmandoReset = true;
      markDirty();
      break;
    case 5:
      router.pop();
      break;
    }
    break;
  default:
    break;
  }
}
