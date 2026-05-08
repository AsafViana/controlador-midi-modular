#pragma once

#include "hardware/CCActivityInfo.h"
#include "hardware/HardwareMap.h"
#include "hardware/UnifiedControlList.h"
#include <cstdint>

class MidiEngine;
class Storage;
class I2CScanner;

/**
 * ControlReader — Leitura automática de controles analógicos.
 *
 * Varre todos os controles do HardwareMap que são analógicos
 * (POTENCIOMETRO ou SENSOR), lê o GPIO, converte para 0-127,
 * e envia o CC configurado no Storage — tudo automático.
 *
 * Quando uma UnifiedControlList e I2CScanner são fornecidos,
 * também lê controles remotos de módulos I2C e aplica a mesma
 * lógica de zona morta e envio de CC.
 *
 * Respeita o flag de habilitado/desabilitado do Storage.
 * Inclui filtro de ruído (zona morta) para evitar flood.
 *
 * Parâmetros de tuning:
 *   EMA_ALPHA  — peso da leitura atual no filtro (0=congela, 1=sem filtro).
 *                0.30 é o ponto ideal para pot físico: suaviza ruído ADC
 *                sem atrasar a resposta ao movimento do usuário.
 *                Valores abaixo de 0.20 causam resposta lenta demais.
 *
 *   ZONA_MORTA — diferença mínima para disparar envio de CC.
 *                Deve ser pequena (1-2) quando EMA_ALPHA >= 0.25,
 *                pois o filtro já elimina micro-oscilações do ADC.
 *                Com ALPHA baixo, a zona morta precisaria ser maior
 *                para compensar — o que mascara movimento real.
 */
class ControlReader {
public:
  /// Zona morta: só envia CC se a diferença for maior que este valor.
  static constexpr uint8_t ZONA_MORTA = 2;

  /// Limites de calibração do ADC (zona morta física do potenciômetro).
  static constexpr uint16_t ADC_MIN = 100;
  static constexpr uint16_t ADC_MAX = 3900;

  /// Fator de suavização do filtro EMA.
  /// 0.30 = resposta rápida ao toque + ruído ADC eliminado.
  /// Era 0.15 — muito lento: valor filtrado não alcançava o real
  /// a tempo, fazendo a zona morta bloquear envios legítimos.
  static constexpr float EMA_ALPHA = 0.30f;

  /// Intervalo mínimo entre leituras (ms).
  static constexpr uint32_t INTERVALO_MS = 10;

  /// Máximo de controles remotos suportados.
  static constexpr uint8_t MAX_REMOTE_CONTROLS =
      UnifiedControlList::MAX_TOTAL_CONTROLS - HardwareMap::NUM_CONTROLES;

  /**
   * @param engine  Ponteiro para o MidiEngine (envio de CC)
   * @param storage Ponteiro para o Storage (CC map + habilitado)
   * @param ucl     Ponteiro para a UnifiedControlList (nullptr = modo
   * standalone)
   * @param scanner Ponteiro para o I2CScanner (nullptr = modo standalone)
   */
  ControlReader(MidiEngine *engine, Storage *storage,
                UnifiedControlList *ucl = nullptr,
                I2CScanner *scanner = nullptr);

  /// Inicializa os pinos analógicos. Chamar no setup().
  void begin();

  /// Lê todos os controles e envia CC se necessário. Chamar no loop().
  void update();

  /// Registra callback chamado a cada envio de CC com dados completos.
  void onCCActivity(CCActivityCallback callback);

private:
  MidiEngine *_engine;
  Storage *_storage;
  UnifiedControlList *_ucl;
  I2CScanner *_scanner;

  uint8_t _ultimoValor[HardwareMap::NUM_CONTROLES];
  uint8_t _ultimoValorRemoto[MAX_REMOTE_CONTROLS];
  uint32_t _ultimaLeitura = 0;

  float _emaValue[HardwareMap::NUM_CONTROLES];
  bool _emaInitialized[HardwareMap::NUM_CONTROLES];

  CCActivityCallback _ccActivityCallback = nullptr;

  /// Estado dos botões MIDI (para debounce e toggle)
  struct MidiButtonState {
    bool lastState = false;   // último estado lido (após debounce)
    bool toggleValue = false; // valor atual do toggle (0 ou 127)
    uint32_t lastChange = 0;  // timestamp da última mudança
  };
  MidiButtonState _btnMidiState[HardwareMap::NUM_CONTROLES];
  static constexpr uint32_t BTN_DEBOUNCE_MS = 50;

  /// Lê um controle analógico local e retorna valor 0-127.
  uint8_t lerControle(uint8_t indice);

  /// Processa botões MIDI (momentâneo e toggle).
  void processarBotoesMidi(uint8_t canal);
};
