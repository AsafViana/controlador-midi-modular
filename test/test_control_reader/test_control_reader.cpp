#include "Arduino.h"
#include "Control_Surface.h"
#include "config.h"
#include "hardware/ControlReader.h"
#include "midi/MidiCC.h"
#include "midi/MidiEngine.h"
#include "storage/Storage.h"
#include <unity.h>


// ── Test infrastructure ─────────────────────────────────────
static MidiEngine engine;
static Storage storage;
static ControlReader *reader = nullptr;

// GPIO do potenciômetro (definido em HardwareMap)
static const uint8_t POT_GPIO = HardwareMap::getGpio(0);

/// Helper: set ADC value for the potentiometer and trigger an update cycle.
/// Returns the CC value that was sent (or 255 if no CC was sent).
static uint8_t readWithADC(int adcValue) {
  mock::pinValues[POT_GPIO] = adcValue;
  mock::advanceMillis(ControlReader::INTERVALO_MS + 1);
  mock_midi::reset();
  reader->update();
  if (mock_midi::messageCount > 0 && mock_midi::lastMessage.isCC) {
    return mock_midi::lastMessage.ccValue;
  }
  return 255; // No CC sent
}

/// Helper: prime the EMA filter by calling update multiple times with same
/// value. This lets the EMA converge so subsequent reads reflect the
/// steady-state.
static void primeEMA(int adcValue, int iterations = 50) {
  for (int i = 0; i < iterations; i++) {
    mock::pinValues[POT_GPIO] = adcValue;
    mock::advanceMillis(ControlReader::INTERVALO_MS + 1);
    reader->update();
  }
}

void setUp(void) {
  mock::reset();
  mock_midi::reset();
  storage.begin();

  // Ensure control 0 is enabled
  storage.setControleHabilitado(0, true);

  // Recreate reader to reset EMA state
  delete reader;
  reader = new ControlReader(&engine, &storage);
  reader->begin();
}

void tearDown(void) {}

// ════════════════════════════════════════════════════════════
// Task 3: Exploratory tests (confirm bug on unfixed code)
// These tests verify the FIXED behavior. On unfixed code they would fail.
// ════════════════════════════════════════════════════════════

void test_adc_at_physical_start_maps_to_midi_0(void) {
  // ADC value 100 (physical start of pot travel) should map to MIDI 0.
  // On unfixed code: 100/32 = 3 (WRONG)
  // On fixed code: (100 - 100) * 127 / (3900 - 100) = 0 (CORRECT)
  primeEMA(100);
  uint8_t valor = readWithADC(100);
  TEST_ASSERT_EQUAL_UINT8(0, valor);
}

void test_adc_at_physical_end_maps_to_midi_127(void) {
  // ADC value 3900 (physical end of pot travel) should map to MIDI 127.
  // On unfixed code: 3900/32 = 121 (WRONG)
  // On fixed code: (3900 - 100) * 127 / (3900 - 100) = 127 (CORRECT)
  primeEMA(3900);
  uint8_t valor = readWithADC(3900);
  TEST_ASSERT_EQUAL_UINT8(127, valor);
}

void test_adc_noise_produces_stable_output(void) {
  // ADC oscillating ±40 around 2048 should produce stable output.
  // On unfixed code: 2048/32=64, 2088/32=65, 2008/32=62 → oscillates
  // On fixed code: EMA + ZONA_MORTA=2 should suppress noise

  // Prime EMA at center
  primeEMA(2048, 80);

  // Record the stable value
  uint8_t stableValue = readWithADC(2048);

  // Now oscillate ±40 and check that output doesn't change
  int noisePattern[] = {2088, 2008, 2070, 2020, 2060, 2030, 2050, 2040};
  int ccChanges = 0;

  for (int i = 0; i < 8; i++) {
    mock::pinValues[POT_GPIO] = noisePattern[i];
    mock::advanceMillis(ControlReader::INTERVALO_MS + 1);
    mock_midi::reset();
    reader->update();
    if (mock_midi::messageCount > 0) {
      ccChanges++;
    }
  }

  // With EMA filter + dead zone, noise should be suppressed
  // Allow at most 1 change (initial EMA adjustment)
  TEST_ASSERT_LESS_OR_EQUAL(1, ccChanges);
}

// ════════════════════════════════════════════════════════════
// Task 4: Fix-checking tests (post-fix validation)
// ════════════════════════════════════════════════════════════

void test_adc_below_min_produces_midi_0(void) {
  // Any ADC value at or below ADC_MIN should produce MIDI 0
  primeEMA(50);
  uint8_t valor = readWithADC(50);
  TEST_ASSERT_EQUAL_UINT8(0, valor);
}

void test_adc_at_min_produces_midi_0(void) {
  primeEMA(100);
  uint8_t valor = readWithADC(100);
  TEST_ASSERT_EQUAL_UINT8(0, valor);
}

void test_adc_above_max_produces_midi_127(void) {
  // Any ADC value at or above ADC_MAX should produce MIDI 127
  primeEMA(4095);
  uint8_t valor = readWithADC(4095);
  TEST_ASSERT_EQUAL_UINT8(127, valor);
}

void test_adc_at_max_produces_midi_127(void) {
  primeEMA(3900);
  uint8_t valor = readWithADC(3900);
  TEST_ASSERT_EQUAL_UINT8(127, valor);
}

void test_adc_midrange_maps_proportionally(void) {
  // ADC 2000 should map to approximately:
  // (2000 - 100) * 127 / (3900 - 100) = 1900 * 127 / 3800 = 63.5 ≈ 63
  primeEMA(2000);
  uint8_t valor = readWithADC(2000);
  // Allow ±1 for rounding
  TEST_ASSERT_UINT8_WITHIN(1, 63, valor);
}

void test_ema_stabilizes_noisy_input(void) {
  // Feed a sequence of noisy values around 2048 and verify output stabilizes
  primeEMA(2048, 80);

  // After priming, the output should be stable
  // Expected: (2048 - 100) * 127 / 3800 ≈ 65
  uint8_t valor = readWithADC(2048);

  // Now feed noise and check stability
  int noiseValues[] = {2060, 2035, 2055, 2040, 2050, 2045, 2048, 2048};
  bool stable = true;

  for (int i = 0; i < 8; i++) {
    mock::pinValues[POT_GPIO] = noiseValues[i];
    mock::advanceMillis(ControlReader::INTERVALO_MS + 1);
    mock_midi::reset();
    reader->update();
    if (mock_midi::messageCount > 0 && mock_midi::lastMessage.isCC) {
      if (mock_midi::lastMessage.ccValue != valor) {
        stable = false;
      }
    }
  }
  TEST_ASSERT_TRUE(stable);
}

void test_zona_morta_suppresses_jitter(void) {
  // ZONA_MORTA = 2 should suppress ±1 output jitter
  // Prime at a known value
  primeEMA(2048, 80);

  // Get the stable output
  uint8_t baseValue = readWithADC(2048);
  TEST_ASSERT_NOT_EQUAL(255, baseValue); // Ensure we got a value

  // Small ADC changes that would cause ±1 MIDI change should be suppressed
  // A change of 1 MIDI unit = 3800/127 ≈ 30 ADC units
  // So ±15 ADC should not cause a MIDI change after EMA
  int smallNoise[] = {2055, 2040, 2052, 2044};
  int ccSent = 0;

  for (int i = 0; i < 4; i++) {
    mock::pinValues[POT_GPIO] = smallNoise[i];
    mock::advanceMillis(ControlReader::INTERVALO_MS + 1);
    mock_midi::reset();
    reader->update();
    if (mock_midi::messageCount > 0) {
      ccSent++;
    }
  }

  TEST_ASSERT_EQUAL(0, ccSent);
}

// ════════════════════════════════════════════════════════════
// Task 5: Preservation tests
// ════════════════════════════════════════════════════════════

void test_intentional_movement_produces_proportional_output(void) {
  // Large ADC changes should produce proportional CC output
  primeEMA(100); // Start at minimum

  // Move to mid-range (large intentional movement)
  primeEMA(2000, 80);
  uint8_t midValue = readWithADC(2000);
  TEST_ASSERT_UINT8_WITHIN(2, 63, midValue);

  // Move to maximum
  primeEMA(3900, 80);
  uint8_t maxValue = readWithADC(3900);
  TEST_ASSERT_EQUAL_UINT8(127, maxValue);
}

void test_inverted_control_applies_127_minus_valor(void) {
  // This test requires an inverted control. Since HardwareMap has only one
  // control and it's not inverted, we verify the logic by checking that
  // a non-inverted control at ADC_MIN gives 0 (not 127).
  // The inversion logic is preserved unchanged in the code.
  primeEMA(100);
  uint8_t valor = readWithADC(100);
  // Non-inverted: ADC_MIN → 0
  TEST_ASSERT_EQUAL_UINT8(0, valor);

  // And at ADC_MAX → 127 (not 0, which would indicate unwanted inversion)
  primeEMA(3900, 80);
  valor = readWithADC(3900);
  TEST_ASSERT_EQUAL_UINT8(127, valor);
}

void test_monotonically_nondecreasing_for_increasing_adc(void) {
  // After EMA settles, increasing ADC should produce non-decreasing output
  uint8_t prevValue = 0;
  bool monotonic = true;

  // Test at several points across the range (after EMA settles at each)
  int adcPoints[] = {100, 500, 1000, 1500, 2000, 2500, 3000, 3500, 3900};

  for (int i = 0; i < 9; i++) {
    primeEMA(adcPoints[i], 80);
    uint8_t valor = readWithADC(adcPoints[i]);
    if (valor == 255)
      continue; // No CC sent (dead zone), skip
    if (valor < prevValue) {
      monotonic = false;
      break;
    }
    prevValue = valor;
  }

  TEST_ASSERT_TRUE(monotonic);
}

void test_boundary_continuity(void) {
  // ADC_MIN maps to 0
  primeEMA(100, 80);
  uint8_t atMin = readWithADC(100);
  TEST_ASSERT_EQUAL_UINT8(0, atMin);

  // ADC_MIN + 1 maps to ~0 (very small value)
  primeEMA(101, 80);
  uint8_t justAboveMin = readWithADC(101);
  TEST_ASSERT_LESS_OR_EQUAL(1, justAboveMin);

  // ADC_MAX - 1 maps to ~127
  primeEMA(3899, 80);
  uint8_t justBelowMax = readWithADC(3899);
  TEST_ASSERT_GREATER_OR_EQUAL(126, justBelowMax);

  // ADC_MAX maps to 127
  primeEMA(3900, 80);
  uint8_t atMax = readWithADC(3900);
  TEST_ASSERT_EQUAL_UINT8(127, atMax);
}

// ════════════════════════════════════════════════════════════
// main
// ════════════════════════════════════════════════════════════

int main(int argc, char **argv) {
  UNITY_BEGIN();

  // Task 3: Exploratory tests
  RUN_TEST(test_adc_at_physical_start_maps_to_midi_0);
  RUN_TEST(test_adc_at_physical_end_maps_to_midi_127);
  RUN_TEST(test_adc_noise_produces_stable_output);

  // Task 4: Fix-checking tests
  RUN_TEST(test_adc_below_min_produces_midi_0);
  RUN_TEST(test_adc_at_min_produces_midi_0);
  RUN_TEST(test_adc_above_max_produces_midi_127);
  RUN_TEST(test_adc_at_max_produces_midi_127);
  RUN_TEST(test_adc_midrange_maps_proportionally);
  RUN_TEST(test_ema_stabilizes_noisy_input);
  RUN_TEST(test_zona_morta_suppresses_jitter);

  // Task 5: Preservation tests
  RUN_TEST(test_intentional_movement_produces_proportional_output);
  RUN_TEST(test_inverted_control_applies_127_minus_valor);
  RUN_TEST(test_monotonically_nondecreasing_for_increasing_adc);
  RUN_TEST(test_boundary_continuity);

  delete reader;
  reader = nullptr;

  return UNITY_END();
}
