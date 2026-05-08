#pragma once

#include <cstdint>

class MidiEngine;

/**
 * MidiClock — Gerador de MIDI Clock com tap tempo.
 *
 * MIDI Clock envia 24 pulsos por quarter note (24 PPQN).
 * O BPM é calculado a partir de 2-4 taps consecutivos.
 *
 * Mensagens enviadas:
 *   - 0xF8: Clock pulse (24 por beat)
 *   - 0xFA: Start
 *   - 0xFC: Stop
 *   - 0xFB: Continue
 *
 * Uso:
 *   clock.tap()    — registra um tap (calcula BPM)
 *   clock.start()  — inicia envio de clock
 *   clock.stop()   — para envio de clock
 *   clock.update() — chamar no loop (envia pulsos no timing correto)
 */
class MidiClock {
public:
  static constexpr uint8_t PPQN = 24; // Pulsos por quarter note
  static constexpr uint16_t MIN_BPM = 30;
  static constexpr uint16_t MAX_BPM = 300;
  static constexpr uint8_t TAP_HISTORY = 4; // Número de taps para média

  MidiClock(MidiEngine *engine);

  /// Registra um tap. Calcula BPM a partir dos intervalos.
  void tap();

  /// Inicia envio de clock (envia Start + pulsos)
  void start();

  /// Para envio de clock (envia Stop)
  void stop();

  /// Continua envio de clock (envia Continue + pulsos)
  void resume();

  /// Envia pulsos no timing correto. Chamar no loop().
  void update();

  /// Retorna o BPM atual (0 se não calculado)
  uint16_t getBPM() const;

  /// Define BPM manualmente
  void setBPM(uint16_t bpm);

  /// Retorna true se o clock está rodando
  bool isRunning() const;

  /// Habilita/desabilita o clock
  void setEnabled(bool enabled);
  bool isEnabled() const;

private:
  MidiEngine *_engine;
  bool _enabled = false;
  bool _running = false;
  uint16_t _bpm = 120;

  // Timing
  uint32_t _lastPulseTime = 0;
  uint32_t _pulseIntervalUs = 0; // Microsegundos entre pulsos

  // Tap tempo
  uint32_t _tapTimes[TAP_HISTORY];
  uint8_t _tapCount = 0;
  uint32_t _lastTapTime = 0;
  static constexpr uint32_t TAP_TIMEOUT_MS = 2000; // Reset se > 2s entre taps

  void calcularIntervalo();
  void enviarPulse();
  void enviarStart();
  void enviarStop();
  void enviarContinue();
};
