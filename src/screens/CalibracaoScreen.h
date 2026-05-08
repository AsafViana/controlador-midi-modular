#pragma once

#include "ui/Screen.h"
#include "ui/components/TextComponent.h"

class OledApp;
class Storage;

/**
 * CalibracaoScreen — Tela de calibração individual de potenciômetros.
 *
 * Fluxo:
 *   1. "Gire até o MÍNIMO" → SELECT captura valor mínimo
 *   2. "Gire até o MÁXIMO" → SELECT captura valor máximo
 *   3. "Calibrado!" → salva e volta
 *
 * BACK cancela em qualquer etapa.
 */
class CalibracaoScreen : public Screen {
public:
  CalibracaoScreen(Storage *storage);

  void setApp(OledApp *app);

  /// Define qual controle será calibrado (índice no HardwareMap)
  void setControleIndex(uint8_t index);

  void handleInput(NavInput input) override;
  bool handleBack() override;
  void onMount() override;
  void render(Adafruit_SSD1306 &display) override;

private:
  Storage *_storage;
  OledApp *_app = nullptr;
  TextComponent _titulo;

  uint8_t _controleIndex = 0;

  enum class Etapa : uint8_t { MINIMO, MAXIMO, CONCLUIDO };
  Etapa _etapa = Etapa::MINIMO;

  uint16_t _capturedMin = 0;
  uint16_t _capturedMax = 4095;

  char _bufInfo[24];

  /// Lê o valor bruto atual do ADC para o controle selecionado
  uint16_t lerADCBruto() const;
};
