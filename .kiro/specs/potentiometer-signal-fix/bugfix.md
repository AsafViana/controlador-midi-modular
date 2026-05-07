# Bugfix Requirements Document

## Introduction

O potenciômetro apresenta dois problemas que comprometem a usabilidade do controlador MIDI:

1. **Mapeamento incorreto do range**: O curso físico completo do potenciômetro não corresponde ao range MIDI completo (0-127). O valor 0 só aparece após algum percurso inicial, e o valor 127 é atingido bem antes do final do curso físico. As zonas mortas nas extremidades do pot são desperdiçadas.

2. **Instabilidade do sinal / oscilação**: O sinal não é estável — qualquer toque leve causa oscilações de ±5 valores, fazendo o output oscilar constantemente sem movimento intencional.

A causa raiz está no método `lerControle()` em `ControlReader.cpp`:

- A conversão `leitura / 32` assume que o pot usa o range ADC completo (0-4095), mas potenciômetros físicos têm zonas mortas nas extremidades.
- `ZONA_MORTA = 1` é insuficiente para o ADC de 12 bits do ESP32-S3, que é reconhecidamente ruidoso.
- Nenhum filtro de suavização (média móvel, filtro exponencial, etc.) é aplicado.

## Bug Analysis

### Current Behavior (Defect)

1.1 WHEN the potentiometer is at the physical start of its travel THEN the system reports a MIDI value greater than 0 (dead zone at the start is not compensated)

1.2 WHEN the potentiometer is at the physical end of its travel THEN the system reports a MIDI value less than 127 (dead zone at the end is not compensated)

1.3 WHEN the potentiometer is held steady at any position THEN the system sends fluctuating CC values (±5 jitter) due to ADC noise with no smoothing applied

1.4 WHEN the ADC reading fluctuates by 1-2 LSBs due to electrical noise THEN the system sends new CC messages because ZONA_MORTA = 1 is too small to filter noise on a 12-bit ADC

### Expected Behavior (Correct)

2.1 WHEN the potentiometer is at the physical start of its travel THEN the system SHALL report MIDI value 0

2.2 WHEN the potentiometer is at the physical end of its travel THEN the system SHALL report MIDI value 127

2.3 WHEN the potentiometer is held steady at any position THEN the system SHALL maintain a stable CC value without sending fluctuating messages

2.4 WHEN the ADC reading fluctuates due to electrical noise (small variations without intentional movement) THEN the system SHALL filter out the noise and NOT send new CC messages

### Unchanged Behavior (Regression Prevention)

3.1 WHEN the potentiometer is moved intentionally through its range THEN the system SHALL CONTINUE TO send CC values that track the movement proportionally from 0 to 127

3.2 WHEN a control is marked as inverted (isInvertido) THEN the system SHALL CONTINUE TO invert the output value (127 - valor)

3.3 WHEN the potentiometer value changes by more than the noise threshold THEN the system SHALL CONTINUE TO send the updated CC message promptly without excessive latency

3.4 WHEN remote controls send values via I2C THEN the system SHALL CONTINUE TO process them with the same dead zone logic

3.5 WHEN a control is disabled in Storage THEN the system SHALL CONTINUE TO skip reading and sending CC for that control

---

## Bug Condition (Formal)

### Bug Condition Function

```pascal
FUNCTION isBugCondition(X)
  INPUT: X of type ADCReading (raw 12-bit value from potentiometer)
  OUTPUT: boolean
  
  // The bug manifests in two cases:
  // Case 1: Range mapping - readings near physical extremes don't map to 0 or 127
  // Case 2: Noise - small ADC fluctuations (≤ noise threshold) cause CC changes
  
  RETURN (X.rawValue < POT_MIN_USABLE OR X.rawValue > POT_MAX_USABLE)
         OR (abs(X.rawValue - X.previousRawValue) <= NOISE_THRESHOLD)
END FUNCTION
```

### Property: Fix Checking — Range Mapping

```pascal
// Property: Full physical travel maps to full MIDI range
FOR ALL X WHERE X.rawValue <= POT_MIN_USABLE DO
  result ← lerControle'(X)
  ASSERT result = 0
END FOR

FOR ALL X WHERE X.rawValue >= POT_MAX_USABLE DO
  result ← lerControle'(X)
  ASSERT result = 127
END FOR
```

### Property: Fix Checking — Signal Stability

```pascal
// Property: Noise below threshold does not produce CC changes
FOR ALL X WHERE abs(X.currentReading - X.previousReading) <= NOISE_THRESHOLD DO
  result ← lerControle'(X)
  ASSERT result = previousOutput (no change emitted)
END FOR
```

### Property: Preservation Checking

```pascal
// Property: Intentional movements still produce correct, proportional output
FOR ALL X WHERE NOT isBugCondition(X) DO
  ASSERT F(X) ≈ F'(X)  // Output remains proportional for mid-range intentional movements
END FOR
```
