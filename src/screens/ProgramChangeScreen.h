#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"

class OledApp;
class Storage;
class MidiEngine;

/**
 * ProgramChangeScreen — Tela para envio manual de Program Change.
 *
 * UP/DOWN seleciona o número do programa (0-127).
 * LONG_UP/LONG_DOWN incrementa/decrementa em passos de 10.
 * SELECT envia o Program Change no canal MIDI ativo.
 * BACK volta ao menu anterior.
 */
class ProgramChangeScreen : public Screen {
public:
  ProgramChangeScreen(MidiEngine *engine, Storage *storage);

  void setApp(OledApp *app);

  void handleInput(NavInput input) override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

private:
  MidiEngine *_engine;
  Storage *_storage;
  OledApp *_app = nullptr;
  TextComponent _titulo;
  TextComponent _hint;
  TextComponent _valorComp;
  TextComponent _labelEnviado;

  uint8_t _program = 0;
  bool _enviado = false;

  char _bufValor[8];
  char _bufEnviado[20];

  void atualizarValor();
};
