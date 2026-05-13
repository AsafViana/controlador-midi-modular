# Relatório técnico — Módulo Principal do Controlador MIDI Modular (ESP32-S3)

## Visão geral

Módulo principal de um controlador MIDI modular baseado em ESP32-S3-WROOM-1-N16R8. Funciona como o cérebro do sistema: concentra comunicação USB MIDI, barramento I2C para módulos periféricos, display OLED, saída/entrada MIDI DIN física e controles de navegação locais.

## Plataforma

- **MCU:** ESP32-S3-WROOM-1-N16R8 (16 MB flash, 8 MB PSRAM Octal)
- **Alimentação:** 3,3 V (regulado a partir de 5 V USB)
- **USB:** Nativo full-speed (GPIO19/20) — MIDI USB + programação
- **Comunicação modular:** I2C (GPIO4/5) — display e módulos no mesmo barramento
- **MIDI físico:** DIN 5 pinos via UART Serial1 (GPIO9/10)

## Pinagem consolidada

| Função | GPIO | Notas |
|--------|------|-------|
| USB D- | 19 | Reservado — USB nativo |
| USB D+ | 20 | Reservado — USB nativo |
| BOOT | 0 | Strapping — botão físico para modo download |
| RESET | EN | Botão físico |
| I2C SDA | 5 | OLED + barramento modular I2C |
| I2C SCL | 4 | OLED + barramento modular I2C |
| Botão UP | 11 | Navegação (não MIDI) |
| Botão DOWN | 12 | Navegação (não MIDI) |
| Botão SELECT | 13 | Navegação (não MIDI) |
| Botão BACK | 14 | Navegação (não MIDI) |
| MIDI DIN TX | 9 | Serial1 — saída MIDI física |
| MIDI DIN RX | 10 | Serial1 — entrada MIDI física |
| Potenciômetro | 7 | ADC — CC auto-assign |

### Pinos proibidos

| GPIO | Motivo |
|------|--------|
| 35, 36, 37 | Ocupados pela PSRAM Octal (N16R8) |
| 19, 20 | Reservados para USB nativo |
| 0 | Strapping (BOOT) — não usar para periféricos |

### Pinos livres para expansão futura

1, 2, 3, 6, 8, 15, 16, 17, 18, 21, 38, 39, 40, 41, 42, 43, 44, 47, 48

## Blocos funcionais da PCB

### 1. Entrada USB-C

- Conector USB-C receptáculo (device)
- Resistores 5,1 kΩ em CC1 e CC2 (pull-down para negociação USB-C)
- Proteção ESD/TVS de baixa capacitância em D+ e D-
- Proteção em VBUS
- Trilhas D+/D- roteadas como par diferencial curto até GPIO19/20

### 2. Regulação de alimentação

- Regulador LDO 5 V → 3,3 V (margem mínima 500 mA recomendada)
- Capacitor de entrada (10 µF cerâmico)
- Capacitor de saída (10 µF + 100 nF cerâmico)
- Capacitores de desacoplamento próximos ao ESP32-S3 (100 nF + 1 µF)

### 3. ESP32-S3-WROOM-1-N16R8

- Footprint conforme datasheet do módulo
- EPAD (exposed pad) conectado ao GND
- Plano de terra sólido sob o módulo
- Keepout na área da antena (sem cobre, componentes ou trilhas)

### 4. Boot e Reset

- Botão RESET: conecta EN ao GND (com capacitor de debounce 100 nF)
- Botão BOOT: conecta GPIO0 ao GND (com pull-up 10 kΩ para 3V3)
- Circuito garante SPI Boot normal; pressionar BOOT durante RESET entra em download

### 5. Display OLED (I2C)

- Header 4 pinos: VCC (3V3), GND, SDA (GPIO5), SCL (GPIO4)
- Resistores pull-up 4,7 kΩ em SDA e SCL (para 3V3)
- Display SSD1306 ou compatível, endereço I2C 0x3C

### 6. Barramento modular I2C

- Conector(es) de expansão com: 3V3, GND, SDA (GPIO5), SCL (GPIO4)
- Compartilha o mesmo barramento I2C do display
- Cada módulo periférico deve ter endereço I2C único
- Pull-ups compartilhados (já no bloco do display)
- Recomendação: conector JST-SH 4 pinos ou equivalente polarizado

### 7. MIDI DIN

- Conector DIN 5 pinos para MIDI OUT:
  - Pino 2: GND (blindagem)
  - Pino 4: +3,3V via resistor 33Ω (ou 5V via 220Ω conforme spec clássica)
  - Pino 5: TX (GPIO9) via resistor 33Ω (ou 220Ω)
- Conector DIN 5 pinos para MIDI IN:
  - Optoacoplador (6N138 ou equivalente) para isolamento galvânico
  - Resistor de entrada 220Ω
  - Diodo de proteção reversa no optoacoplador
  - Saída do optoacoplador → GPIO10 (RX)
- Circuito conforme especificação MIDI elétrica padrão

### 8. Botões de navegação

- 4 botões táteis (momentâneos, normalmente aberto)
- Conexão: GPIO → botão → GND
- Pull-up interno do ESP32-S3 habilitado via firmware (INPUT_PULLUP)
- Debounce tratado em software
- GPIOs: 11 (UP), 12 (DOWN), 13 (SELECT), 14 (BACK)

### 9. Potenciômetro

- Potenciômetro linear (10 kΩ recomendado)
- Conexão: VCC (3V3) — wiper (GPIO7) — GND
- Leitura via ADC do ESP32-S3
- Capacitor de filtro 100 nF entre wiper e GND (opcional, reduz ruído)

## Requisitos de layout

- Par diferencial USB D+/D- curto e equilibrado, próximo à borda
- ESD/TVS imediatamente após o conector USB
- Regulador e capacitores com loop de corrente curto
- Plano de GND sólido, especialmente sob ESP32-S3 e USB
- Keepout na área da antena do módulo (sem cobre/componentes)
- Conectores modulares em posição compatível com encaixe entre módulos
- Serigrafia clara em todos os conectores com indicação de pino 1

## Estratégia de alimentação para módulos

- Módulo principal distribui 3,3 V de baixa corrente via barramento I2C
- Módulos periféricos devem consumir < 50 mA cada (lógica apenas)
- Se módulos precisarem de mais corrente (LEDs, motores), alimentação dedicada futura

## Sequência de validação (bring-up)

1. Verificar ausência de curto entre 5V/GND e 3V3/GND
2. Alimentar via USB, medir VBUS (5V) e saída do regulador (3,3V)
3. Verificar botões BOOT/RESET funcionais
4. Gravar firmware de teste via USB
5. Confirmar enumeração USB MIDI no host
6. Testar display OLED via I2C
7. Testar leitura do potenciômetro (ADC)
8. Testar MIDI DIN OUT com equipamento externo
9. Conectar módulo periférico I2C e verificar comunicação

## Decisões tomadas

- ✅ Comunicação modular via I2C (não UART)
- ✅ Sem LEDs indicadores na revisão 1
- ✅ Sem UART de debug (usa USB Serial/JTAG nativo)
- ✅ MIDI DIN presente no módulo principal
- ✅ USB-C como conector principal
- ✅ Display OLED no mesmo barramento I2C dos módulos
