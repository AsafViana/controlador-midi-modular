#pragma once

#include "hardware/CCActivityInfo.h"
#include "midi/MidiEngine.h"
#include "ui/Screen.h"
#include "ui/components/ProgressBarComponent.h"
#include "ui/components/TextComponent.h"

class OledApp;
class Storage;

/**
 * PerformanceScreen — Tela de performance ao vivo.
 *
 * Exibe informações do estado MIDI (canal, oitava, velocidade)
 * e monitor em tempo real do último CC enviado, mostrando:
 * - Nome do controle (label)
 * - Número do CC e valor atual
 * - Módulo de origem (Local ou endereço I2C)
 * - Barra de progresso visual
 *
 * Navegação:
 *   UP     — incrementa oitava (0-8), salva no Storage
 *   DOWN   — decrementa oitava (0-8), salva no Storage
 *   SELECT — volta ao menu anterior
 */
class PerformanceScreen : public Screen {
public:
  PerformanceScreen(MidiEngine *engine, Storage *storage);

  void setApp(OledApp *app);

  void handleInput(NavInput input) override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

  /// Chamado pelo callback do ControlReader com dados completos do CC.
  void atualizarCCInfo(const CCActivityInfo &info);

  /// Chamado externamente para atualizar feedback visual de CC (legado).
  void atualizarCC(uint8_t valor);

private:
  MidiEngine *_engine;
  Storage *_storage;
  OledApp *_app = nullptr;

  // Componentes visuais — coluna esquerda (estado MIDI)
  TextComponent _titulo;
  TextComponent _voltar;
  TextComponent _infoCanal;

  // Componentes visuais — monitor CC (metade inferior)
  TextComponent _monitorLabel;  // Nome do controle
  TextComponent _monitorCC;     // CC: XX  Val: YYY
  TextComponent _monitorModulo; // Local / [XX] Remoto
  ProgressBarComponent _barraCC;

  // Estado
  uint8_t _ultimoCC = 0;

  // Buffers para texto dinâmico
  char _bufCanal[16];
  char _bufMonLabel[24];
  char _bufMonCC[22];
  char _bufMonModulo[20];

  void atualizarTextos();
};
