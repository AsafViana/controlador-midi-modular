#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"

class OledApp;
class Storage;
class UnifiedControlList;

/**
 * SobreScreen — Tela de informações do sistema.
 *
 * Exibe nome do produto, versão do firmware, canal MIDI atual
 * e número de controles (locais + remotos). Útil para suporte e diagnóstico.
 */
class SobreScreen : public Screen {
public:
  SobreScreen(Storage *storage, UnifiedControlList *ucl = nullptr);

  void setApp(OledApp *app);

  void handleInput(NavInput input) override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

private:
  OledApp *_app = nullptr;
  Storage *_storage;
  UnifiedControlList *_ucl;
  TextComponent _titulo;
  TextComponent _voltar;
  TextComponent _sobre;
  TextComponent _versao;
  TextComponent _infoCanal;
  TextComponent _infoControles;

  char _bufCanal[20];
  char _bufControles[24];
};
