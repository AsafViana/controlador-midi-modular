# Potentiometer Signal Fix — Bugfix Design

## Overview

The `lerControle()` function in `ControlReader.cpp` has two defects that make the MIDI controller unusable for precise performance:

1. **Range mapping**: The simple `leitura / 32` conversion assumes the potentiometer uses the full 0-4095 ADC range, but physical pots have dead zones at both extremes. This wastes physical travel and prevents reaching MIDI 0 and 127.

2. **Signal instability**: No smoothing filter is applied, and `ZONA_MORTA = 1` is far too small for the ESP32-S3's noisy 12-bit ADC. The result is ±5 value oscillations that flood the MIDI bus with unintended CC messages.

The fix introduces configurable ADC calibration bounds, an exponential moving average (EMA) filter, and an appropriately sized dead zone threshold.

## Glossary

- **Bug_Condition (C)**: The condition that triggers the bug — ADC readings at physical extremes don't map to 0/127, and small ADC noise causes CC value changes
- **Property (P)**: The desired behavior — full physical travel maps to full 0-127 range, and noise is filtered out
- **Preservation**: Existing behavior that must remain unchanged — proportional tracking, inversion logic, remote control processing, dead zone gating
- **lerControle()**: The function in `src/hardware/ControlReader.cpp` that reads a potentiometer GPIO and converts the 12-bit ADC value to a 0-127 MIDI value
- **ZONA_MORTA**: The dead zone threshold constant in `ControlReader` that gates CC message sending — only sends if the value changed by more than this amount
- **EMA (Exponential Moving Average)**: A low-pass filter where `filtered = alpha * new_sample + (1 - alpha) * previous_filtered`. Smooths noise while preserving intentional movement.
- **ADC_MIN / ADC_MAX**: Configurable calibration values representing the actual usable ADC range of the potentiometer (excluding physical dead zones)

## Bug Details

### Bug Condition

The bug manifests in two ways: (1) when the potentiometer is at its physical extremes, the ADC readings fall in dead zones that don't map to MIDI 0 or 127; (2) when the pot is held steady, ADC noise of ±30-50 LSBs on the 12-bit ESP32-S3 ADC causes the simple `/32` conversion to oscillate by ±1-5 MIDI values, and `ZONA_MORTA = 1` is insufficient to suppress this.

**Formal Specification:**

```
FUNCTION isBugCondition(input)
  INPUT: input of type { rawADC: uint16, previousRawADC: uint16, previousOutput: uint8 }
  OUTPUT: boolean
  
  // Case 1: Range mapping bug — physical extremes don't reach MIDI 0/127
  rangeIssue := (input.rawADC <= ADC_MIN AND expectedOutput != 0)
                OR (input.rawADC >= ADC_MAX AND expectedOutput != 127)
  
  // Case 2: Noise bug — small ADC fluctuations cause output changes
  noiseIssue := abs(input.rawADC - input.previousRawADC) <= ADC_NOISE_AMPLITUDE
                AND outputChanged(input)
  
  RETURN rangeIssue OR noiseIssue
END FUNCTION
```

### Examples

- **Physical start**: Pot at 0% travel → ADC reads ~120 → `120/32 = 3` → MIDI outputs 3 instead of 0
- **Physical end**: Pot at 100% travel → ADC reads ~3900 → `3900/32 = 121` → MIDI outputs 121 instead of 127
- **Noise at rest**: Pot held steady at mid-range → ADC oscillates 2048±40 → output oscillates 64±1, sending continuous CC messages
- **Edge case**: Pot at exact mid-range → ADC reads 2048 → `2048/32 = 64` → correct output (no bug in mid-range mapping, only noise)

## Expected Behavior

### Preservation Requirements

**Unchanged Behaviors:**

- Intentional potentiometer movement through its range must continue to produce proportional CC values from 0 to 127
- Controls marked as `isInvertido` must continue to invert the output value (`127 - valor`)
- Intentional value changes must still be sent promptly without perceptible latency
- Remote controls received via I2C must continue to be processed with the same dead zone logic
- Disabled controls must continue to be skipped
- The `update()` loop timing (`INTERVALO_MS = 10`) must remain unchanged
- The CC activity callback must continue to fire with correct info on value changes

**Scope:**
All inputs where the potentiometer is being intentionally moved through the mid-range (not at physical extremes) and the movement exceeds the noise threshold should be completely unaffected by this fix. This includes:

- Normal performance gestures (sweeps, fades)
- Rapid movements between positions
- Inverted control behavior
- Remote module value processing

## Hypothesized Root Cause

Based on the bug description and code analysis, the issues are:

1. **No ADC Calibration**: `lerControle()` divides the raw ADC value by 32, assuming the pot spans 0-4095. Physical potentiometers typically have dead zones at both ends (e.g., usable range is ~120-3900), meaning the first and last ~3% of physical travel produces no useful change.

2. **Integer Division Truncation**: `leitura / 32` maps 0-4095 to 0-127 but with truncation. Values 0-31 all map to 0, and only value 4064+ maps to 127. Combined with dead zones, the effective MIDI range is reduced.

3. **No Smoothing Filter**: The ESP32-S3 ADC is known to have significant noise (±30-50 LSBs on 12-bit readings). Without any filtering, every noise spike becomes a potential CC change.

4. **Insufficient Dead Zone**: `ZONA_MORTA = 1` means any change of 2+ in the 0-127 output triggers a send. Since ADC noise of ±40 translates to ±1-2 in MIDI values after `/32`, the dead zone fails to suppress noise-induced changes.

## Correctness Properties

Property 1: Bug Condition - Full Range Mapping

_For any_ ADC reading at or below the calibrated minimum (`ADC_MIN`), the fixed `lerControle()` function SHALL return MIDI value 0; and _for any_ ADC reading at or above the calibrated maximum (`ADC_MAX`), it SHALL return MIDI value 127. The mapping between `ADC_MIN` and `ADC_MAX` SHALL be linear and cover the full 0-127 range.

**Validates: Requirements 2.1, 2.2**

Property 2: Bug Condition - Signal Stability

_For any_ sequence of ADC readings where the variation is within the noise threshold (no intentional movement), the fixed `lerControle()` combined with the dead zone logic SHALL NOT produce a change in the output CC value, maintaining signal stability.

**Validates: Requirements 2.3, 2.4**

Property 3: Preservation - Proportional Tracking

_For any_ intentional potentiometer movement (ADC change exceeding the noise threshold) within the calibrated range, the fixed function SHALL produce output that tracks the movement proportionally from 0 to 127, matching the behavior of the original function for mid-range inputs (adjusted for the new calibration mapping).

**Validates: Requirements 3.1, 3.3**

Property 4: Preservation - Inversion and Control Logic

_For any_ control marked as inverted, the fixed function SHALL continue to invert the output (`127 - valor`). Remote controls, disabled controls, and callback notifications SHALL continue to behave identically to the original implementation.

**Validates: Requirements 3.2, 3.4, 3.5**

## Fix Implementation

### Changes Required

Assuming our root cause analysis is correct:

**File**: `src/hardware/ControlReader.h`

**Changes**:

1. **Add calibration constants**: Define `ADC_MIN` and `ADC_MAX` as configurable constants (default ~100 and ~3900)
2. **Increase ZONA_MORTA**: Change from 1 to 2-3 (appropriate for filtered 7-bit output)
3. **Add EMA state array**: Add `float _emaValue[NUM_CONTROLES]` to store the filtered ADC value per control
4. **Add EMA alpha constant**: Define smoothing factor (e.g., `EMA_ALPHA = 0.1` for strong smoothing, or `0.2` for moderate)

**File**: `src/hardware/ControlReader.cpp`

**Function**: `lerControle(uint8_t indice)`

**Specific Changes**:

1. **Apply EMA filter**: Before conversion, smooth the raw ADC reading:

   ```
   _emaValue[indice] = EMA_ALPHA * rawReading + (1.0 - EMA_ALPHA) * _emaValue[indice]
   ```

2. **Clamp to calibration range**: Constrain the filtered value to `[ADC_MIN, ADC_MAX]`

3. **Map with calibration**: Replace `leitura / 32` with:

   ```
   valor = map(clampedValue, ADC_MIN, ADC_MAX, 0, 127)
   ```

   Or equivalent: `(clamped - ADC_MIN) * 127 / (ADC_MAX - ADC_MIN)`

4. **Initialize EMA state**: In the constructor or `begin()`, initialize `_emaValue[]` with the first ADC reading (or -1 sentinel for lazy init)

5. **Keep inversion logic unchanged**: The `isInvertido` check remains after the new conversion

## Testing Strategy

### Validation Approach

The testing strategy follows a two-phase approach: first, surface counterexamples that demonstrate the bug on unfixed code, then verify the fix works correctly and preserves existing behavior.

### Exploratory Bug Condition Checking

**Goal**: Surface counterexamples that demonstrate the bug BEFORE implementing the fix. Confirm or refute the root cause analysis.

**Test Plan**: Write unit tests that call `lerControle()` with mocked `analogRead()` values at the extremes and with noise patterns. Run on the UNFIXED code to observe failures.

**Test Cases**:

1. **Low Extreme Test**: Mock `analogRead()` returning 100 (physical start) — expect MIDI 0, will get ~3 (fails on unfixed code)
2. **High Extreme Test**: Mock `analogRead()` returning 3900 (physical end) — expect MIDI 127, will get ~121 (fails on unfixed code)
3. **Noise Stability Test**: Mock `analogRead()` returning 2048±40 in sequence — expect stable output, will get oscillating values (fails on unfixed code)
4. **Dead Zone Bypass Test**: Mock `analogRead()` changing by 33 (1 MIDI step) — expect no CC send with ZONA_MORTA=1, will see CC sent (fails on unfixed code)

**Expected Counterexamples**:

- `lerControle()` returns 3 when pot is at physical 0% (ADC ~100)
- `lerControle()` returns 121 when pot is at physical 100% (ADC ~3900)
- Output oscillates between 63-65 when ADC oscillates 2020-2080

### Fix Checking

**Goal**: Verify that for all inputs where the bug condition holds, the fixed function produces the expected behavior.

**Pseudocode:**

```
FOR ALL rawADC WHERE rawADC <= ADC_MIN DO
  result := lerControle_fixed(rawADC)
  ASSERT result = 0
END FOR

FOR ALL rawADC WHERE rawADC >= ADC_MAX DO
  result := lerControle_fixed(rawADC)
  ASSERT result = 127
END FOR

FOR ALL sequence WHERE max(sequence) - min(sequence) <= NOISE_AMPLITUDE DO
  results := [lerControle_fixed(s) for s in sequence]
  ASSERT max(results) - min(results) <= 0  // stable output
END FOR
```

### Preservation Checking

**Goal**: Verify that for all inputs where the bug condition does NOT hold, the fixed function produces the same proportional result as the original function.

**Pseudocode:**

```
FOR ALL rawADC WHERE ADC_MIN < rawADC < ADC_MAX DO
  // Both functions should produce proportional output in mid-range
  result_fixed := lerControle_fixed(rawADC)
  expected := (rawADC - ADC_MIN) * 127 / (ADC_MAX - ADC_MIN)
  ASSERT abs(result_fixed - expected) <= 1  // within rounding tolerance
END FOR
```

**Testing Approach**: Property-based testing is recommended for preservation checking because:

- It generates many ADC values across the full 0-4095 range automatically
- It catches edge cases at calibration boundaries that manual tests might miss
- It provides strong guarantees that proportional behavior is maintained

**Test Plan**: Observe behavior on UNFIXED code for mid-range values, then write property-based tests verifying the fixed code maintains proportionality.

**Test Cases**:

1. **Mid-Range Proportionality**: Generate random ADC values in [ADC_MIN, ADC_MAX], verify output is proportional to position within range
2. **Inversion Preservation**: Generate random values with inverted flag, verify `127 - valor` still applies
3. **Monotonicity**: Generate increasing ADC sequences, verify output is non-decreasing (after filtering settles)
4. **Boundary Continuity**: Verify no discontinuity at ADC_MIN and ADC_MAX boundaries

### Unit Tests

- Test `lerControle()` with ADC values at 0, ADC_MIN, mid-range, ADC_MAX, 4095
- Test EMA filter convergence with step input
- Test EMA filter response to noise (random ±40 around a fixed value)
- Test inversion logic with calibrated values
- Test boundary clamping (values below ADC_MIN → 0, above ADC_MAX → 127)

### Property-Based Tests

- Generate random ADC values in [0, 4095] and verify output is always in [0, 127]
- Generate random ADC sequences with bounded noise and verify output stability
- Generate random ADC values in [ADC_MIN, ADC_MAX] and verify linear proportionality
- Generate random increasing sequences and verify monotonic output (after EMA settles)

### Integration Tests

- Test full `update()` cycle with mocked `analogRead()` returning calibrated extremes
- Test that CC messages are NOT sent when ADC noise is below threshold
- Test that CC messages ARE sent promptly when intentional movement occurs
- Test interaction between EMA filter and ZONA_MORTA (combined noise rejection)
