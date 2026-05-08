#include "midi/MidiClock.h"
#include "midi/MidiEngine.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "Arduino.h"
#endif

MidiClock::MidiClock(MidiEngine *engine) : _engine(engine) {
  for (uint8_t i = 0; i < TAP_HISTORY; i++) {
    _tapTimes[i] = 0;
  }
  calcularIntervalo();
}

void MidiClock::setEnabled(bool enabled) {
  _enabled = enabled;
  if (!enabled && _running) {
    stop();
  }
}

bool MidiClock::isEnabled() const { return _enabled; }
bool MidiClock::isRunning() const { return _running; }
uint16_t MidiClock::getBPM() const { return _bpm; }

void MidiClock::setBPM(uint16_t bpm) {
  if (bpm < MIN_BPM)
    bpm = MIN_BPM;
  if (bpm > MAX_BPM)
    bpm = MAX_BPM;
  _bpm = bpm;
  calcularIntervalo();
}

void MidiClock::calcularIntervalo() {
  // Intervalo entre pulsos em microsegundos
  // 1 beat = 60000000us / BPM
  // 1 pulse = 1 beat / 24
  if (_bpm > 0) {
    _pulseIntervalUs = 60000000UL / (static_cast<uint32_t>(_bpm) * PPQN);
  }
}

void MidiClock::tap() {
  uint32_t now = millis();

  // Reset se muito tempo entre taps
  if (_tapCount > 0 && (now - _lastTapTime) > TAP_TIMEOUT_MS) {
    _tapCount = 0;
  }

  // Registra tap
  if (_tapCount < TAP_HISTORY) {
    _tapTimes[_tapCount] = now;
    _tapCount++;
  } else {
    // Shift e adiciona novo
    for (uint8_t i = 0; i < TAP_HISTORY - 1; i++) {
      _tapTimes[i] = _tapTimes[i + 1];
    }
    _tapTimes[TAP_HISTORY - 1] = now;
  }
  _lastTapTime = now;

  // Calcula BPM se temos pelo menos 2 taps
  if (_tapCount >= 2) {
    uint32_t totalInterval = 0;
    uint8_t numIntervals = _tapCount - 1;

    for (uint8_t i = 0; i < numIntervals; i++) {
      totalInterval += _tapTimes[i + 1] - _tapTimes[i];
    }

    uint32_t avgInterval = totalInterval / numIntervals;
    if (avgInterval > 0) {
      uint16_t newBPM = static_cast<uint16_t>(60000UL / avgInterval);
      if (newBPM >= MIN_BPM && newBPM <= MAX_BPM) {
        _bpm = newBPM;
        calcularIntervalo();
      }
    }
  }
}

void MidiClock::start() {
  if (!_enabled)
    return;
  _running = true;
  _lastPulseTime = micros();
  enviarStart();
}

void MidiClock::stop() {
  _running = false;
  enviarStop();
}

void MidiClock::resume() {
  if (!_enabled)
    return;
  _running = true;
  _lastPulseTime = micros();
  enviarContinue();
}

void MidiClock::update() {
  if (!_enabled || !_running || _engine == nullptr)
    return;

  uint32_t now = micros();
  uint32_t elapsed = now - _lastPulseTime;

  if (elapsed >= _pulseIntervalUs) {
    enviarPulse();
    _lastPulseTime += _pulseIntervalUs;

    // Evita acúmulo se perdeu pulsos
    if ((now - _lastPulseTime) > _pulseIntervalUs * 2) {
      _lastPulseTime = now;
    }
  }
}

void MidiClock::enviarPulse() {
#ifdef ARDUINO
  // 0xF8 = MIDI Clock pulse (System Real-Time)
  _engine->getUsbMidi().sendRealTime(0xF8);
#endif
}

void MidiClock::enviarStart() {
#ifdef ARDUINO
  // 0xFA = Start
  _engine->getUsbMidi().sendRealTime(0xFA);
#endif
}

void MidiClock::enviarStop() {
#ifdef ARDUINO
  // 0xFC = Stop
  _engine->getUsbMidi().sendRealTime(0xFC);
#endif
}

void MidiClock::enviarContinue() {
#ifdef ARDUINO
  // 0xFB = Continue
  _engine->getUsbMidi().sendRealTime(0xFB);
#endif
}
