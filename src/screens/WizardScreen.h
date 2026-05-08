#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"

class OledApp;
class Storage;

/**
 * WizardScreen — Assistente de primeira configuração.
 *
 * Detecta primeiro boot (flag NVS) e guia o usuário:
 *   1. Boas-vindas + instruções de navegação
 *   2. Seleção de canal MIDI
 *   3. Conclusão
 *
 * Após completar, grava flag no Storage e não aparece mais.
 * Acessível novamente via menu Config > Assistente.
 */
class WizardScreen : public Screen {
public:
  WizardScreen(OledApp *app, Storage *storage);

  void handleInput(NavInput input) override;
  bool handleBack() override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

  /// Verifica se o wizard deve ser exibido (primeiro boot)
  static bool deveExibir(Storage *storage);

  /// Marca o wizard como concluído
  static void marcarConcluido(Storage *storage);

private:
  OledApp *_app;
  Storage *_storage;
  TextComponent _titulo;

  enum class Etapa : uint8_t { BOAS_VINDAS, CANAL, CONCLUIDO };
  Etapa _etapa = Etapa::BOAS_VINDAS;

  uint8_t _canal = 1;
  char _bufCanal[4];
};
