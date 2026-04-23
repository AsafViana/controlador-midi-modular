# Conector DB9 — Barramento I2C Modular

> Especificação da pinagem do conector DB9 usado para interconexão entre o módulo principal (mestre) e os módulos escravos via I2C.

---

## 1. Pinagem

```
  DB9 Fêmea (vista frontal)
  ┌───────────────────────┐
  │ 1   2   3   4   5     │
  │   6   7   8   9       │
  └───────────────────────┘
```

| Pino | Função       | Descrição                              |
|------|-------------|----------------------------------------|
| 1    | VCC         | Alimentação +5V (ou +3.3V)             |
| 2    | VCC         | Alimentação +5V (ou +3.3V)             |
| 3    | VCC         | Alimentação +5V (ou +3.3V)             |
| 4    | GND         | Terra (alimentação)                    |
| 5    | GND         | Terra (alimentação)                    |
| 6    | SDA         | I2C Data                               |
| 7    | SCL         | I2C Clock                              |
| 8    | GND         | Terra (referência I2C)                 |
| 9    | Reservado   | Uso futuro (não conectar)              |

---

## 2. Agrupamento de pinos

### Alimentação (VCC)

Pinos 1, 2 e 3 em paralelo. Cada pino de um conector DB9 suporta aproximadamente 1A, totalizando ~3A de capacidade confortável para alimentar o módulo escravo e seus periféricos (sensores, LEDs, etc.).

### Terra (GND)

Pinos 4 e 5 em paralelo para retorno de corrente da alimentação (~2A). Pino 8 é um terra separado dedicado à referência do barramento I2C, reduzindo ruído nos sinais de dados.

### I2C

Pinos 6 (SDA) e 7 (SCL) carregam o barramento I2C. Pull-ups de 4.7kΩ devem estar no módulo principal (mestre). Os módulos escravos não devem adicionar pull-ups próprios para evitar carga excessiva no barramento.

### Reservado

Pino 9 reservado para uso futuro (ex: sinal de interrupção, reset, identificação de módulo). Deve ser deixado desconectado em ambos os lados.

---

## 3. Diagrama de conexão

```
  Módulo Principal (Mestre)              Módulo Escravo
  ┌──────────────────────┐              ┌──────────────────────┐
  │                      │   DB9 cabo   │                      │
  │  3.3V/5V ──┬─┬─┬────┼──1,2,3──────┼────┬─┬─┬── VCC      │
  │            └─┘─┘     │              │    └─┘─┘             │
  │                      │              │                      │
  │  GND ─────┬─┬───────┼──4,5────────┼────┬─┬─── GND       │
  │           └─┘        │              │    └─┘               │
  │                      │              │                      │
  │  GPIO 1 (SDA) ──4k7─┼──6──────────┼──── SDA              │
  │  GPIO 2 (SCL) ──4k7─┼──7──────────┼──── SCL              │
  │                      │              │                      │
  │  GND ────────────────┼──8──────────┼──── GND (ref I2C)    │
  │                      │              │                      │
  │              (n/c) ──┼──9──────────┼──── (n/c)            │
  └──────────────────────┘              └──────────────────────┘

  4k7 = pull-up 4.7kΩ para VCC (somente no mestre)
```

---

## 4. Protocolo I2C

| Parâmetro                  | Valor                          |
|---------------------------|--------------------------------|
| Barramento                | Wire (SDA=GPIO 1, SCL=GPIO 2)  |
| Velocidade                | 100 kHz (Standard Mode)        |
| Range de endereços        | `0x20` a `0x27` (8 módulos)    |
| Endereço reservado        | `0x3C` (display OLED, outro barramento) |
| Pull-ups                  | 4.7kΩ no mestre                |
| Timeout por transação     | 50ms                           |

### Comandos

| Comando           | Código | Direção                    | Descrição                        |
|-------------------|--------|----------------------------|----------------------------------|
| `CMD_DESCRIPTOR`  | `0x01` | Mestre → Escravo → Mestre  | Solicita descritor do módulo     |
| `CMD_READ_VALUES` | `0x02` | Mestre → Escravo → Mestre  | Solicita valores dos controles   |

### Formato do descritor (resposta a `CMD_DESCRIPTOR`)

```
Byte 0:       numControles (1-16)
Para cada controle (14 bytes):
  Byte 0:     tipo (0=BOTAO, 1=POTENCIOMETRO, 2=SENSOR, 3=ENCODER)
  Bytes 1-12: label (12 chars, null-padded)
  Byte 13:    valor atual (0-127)

Tamanho máximo: 1 + (16 × 14) = 225 bytes
```

### Formato da leitura de valores (resposta a `CMD_READ_VALUES`)

```
N bytes, um por controle (0-127)
N = numControles reportado no descritor
```

### Fluxo de comunicação

```
1. Mestre envia 1 byte: CMD_DESCRIPTOR (0x01)
2. Mestre lê resposta: descritor completo
3. (periodicamente) Mestre envia 1 byte: CMD_READ_VALUES (0x02)
4. Mestre lê resposta: N bytes de valores
```

---

## 5. Resiliência

| Parâmetro                    | Valor                                  |
|-----------------------------|----------------------------------------|
| Máximo de falhas consecutivas | 3 (`MAX_FAIL_COUNT`)                  |
| Intervalo de rescan          | 5 segundos (`RESCAN_INTERVAL_MS`)     |
| Comportamento em falha       | Mantém último valor, não envia CC     |
| Após 3 falhas                | Módulo marcado como desconectado      |
| Reconexão                    | Automática no próximo rescan          |

---

## 6. Notas de montagem

- Usar cabo DB9 macho-fêmea blindado, comprimento máximo recomendado de 1 metro para I2C a 100kHz.
- A blindagem do cabo deve ser conectada ao GND em apenas um dos lados (mestre) para evitar loop de terra.
- Cada módulo escravo deve ter um endereço I2C único configurado por hardware (jumpers ou resistores) no range `0x20`-`0x27`.
- O pino 9 (reservado) não deve ser conectado a nada. Futuramente pode ser usado para sinal de interrupção do escravo para o mestre.
- Se a alimentação for 5V e o microcontrolador do escravo operar a 3.3V, incluir regulador de tensão no módulo escravo.
