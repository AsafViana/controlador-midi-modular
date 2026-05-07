# Tasks — Potentiometer Signal Fix

## Task 1: Add calibration constants and EMA state to ControlReader.h

- [x] 1.1 Add `ADC_MIN` constant (default 100) and `ADC_MAX` constant (default 3900) as `static constexpr uint16_t`
- [x] 1.2 Add `EMA_ALPHA` constant (default 0.15f) as `static constexpr float`
- [x] 1.3 Increase `ZONA_MORTA` from 1 to 2
- [x] 1.4 Add `float _emaValue[HardwareMap::NUM_CONTROLES]` private member array for EMA filter state
- [x] 1.5 Add `bool _emaInitialized[HardwareMap::NUM_CONTROLES]` private member array for lazy initialization

## Task 2: Implement calibrated mapping and EMA filter in lerControle()

- [x] 2.1 Initialize EMA state in the constructor (set `_emaInitialized[]` to false)
- [x] 2.2 In `lerControle()`, apply EMA filter to raw ADC reading: `_emaValue[indice] = EMA_ALPHA * rawReading + (1.0f - EMA_ALPHA) * _emaValue[indice]` (with lazy init on first call)
- [x] 2.3 Clamp the filtered value to `[ADC_MIN, ADC_MAX]` range
- [x] 2.4 Replace `leitura / 32` with calibrated linear mapping: `(clamped - ADC_MIN) * 127 / (ADC_MAX - ADC_MIN)`
- [x] 2.5 Keep the existing `isInvertido` logic unchanged after the new conversion
- [x] 2.6 Ensure the output is clamped to [0, 127] after mapping

## Task 3: Write exploratory tests to confirm bug on unfixed code

- [x] 3.1 Create test file `test/test_control_reader/test_control_reader.cpp` with PlatformIO native test structure
- [x] 3.2 Mock `analogRead()` to return controllable values
- [x] 3.3 Write test: ADC value 100 (physical start) should map to MIDI 0 — expect failure on unfixed code (returns 3)
- [x] 3.4 Write test: ADC value 3900 (physical end) should map to MIDI 127 — expect failure on unfixed code (returns 121)
- [x] 3.5 Write test: ADC oscillating ±40 around 2048 should produce stable output — expect failure on unfixed code

## Task 4: Write fix-checking tests (post-fix validation)

- [x] 4.1 Test that ADC values at or below ADC_MIN produce MIDI output 0
- [x] 4.2 Test that ADC values at or above ADC_MAX produce MIDI output 127
- [x] 4.3 Test that ADC mid-range value maps proportionally (e.g., ADC 2000 → ~60)
- [x] 4.4 Test that EMA filter stabilizes noisy input (sequence of ±40 noise → stable output)
- [x] 4.5 Test that ZONA_MORTA = 2 suppresses ±1 output jitter from residual noise

## Task 5: Write preservation tests

- [x] 5.1 Test that intentional movement (large ADC change) still produces proportional CC output
- [x] 5.2 Test that inverted controls still apply `127 - valor` correctly with new mapping
- [x] 5.3 Test that output is monotonically non-decreasing for increasing ADC input (after EMA settles)
- [x] 5.4 Test boundary continuity: ADC_MIN maps to 0, ADC_MIN+1 maps to ~0, ADC_MAX-1 maps to ~127, ADC_MAX maps to 127

## Task 6: Verify build compiles and all tests pass

- [x] 6.1 Run PlatformIO build to verify no compilation errors
- [x] 6.2 Run the test suite and verify all fix-checking tests pass
- [x] 6.3 Run the test suite and verify all preservation tests pass
