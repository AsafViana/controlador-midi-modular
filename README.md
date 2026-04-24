# Controlador MIDI Modular

Controlador MIDI USB baseado em ESP32-S3 com display OLED, interface de navegação por botões e suporte a módulos de expansão via I2C.

## Funcionalidades

- Envio de MIDI CC via USB e MIDI DIN a partir de potenciômetros e sensores analógicos
- Display OLED SSD1306 128×64 com interface de navegação
- Splash screen no boot com nome do produto e versão do firmware
- Monitor de CC em tempo real na tela Performance (label, valor, módulo de origem)
- Configuração de CC por controle, canal MIDI, oitava e velocidade
- Aceleração na edição de valores (long press = incremento rápido)
- Feedback visual ao salvar configurações (inversão do display)
- Factory reset — restaura todas as configurações para os padrões de fábrica
- Persistência de configurações na flash (NVS) — sobrevive a reinicializações
- Expansão modular via barramento I2C (até 8 módulos escravos, conector DB-9)
- Descoberta automática de módulos com hot-plug e resiliência a falhas
- Indicador LED de erro se o display falhar na inicialização

## Hardware

| Componente | Descrição |
|---|---|
| MCU | ESP32-S3-DevKitC-1 (16MB flash, 8MB PSRAM) |
| Display | OLED SSD1306 128×64 (I2C, endereço 0x3C) |
| Botões | 3 botões de navegação (UP, DOWN, SELECT) |
| Controles | Potenciômetros e sensores analógicos |
| Expansão | Módulos I2C via conector DB-9 (endereços 0x20-0x27) |

## Estrutura do projeto

```
src/
├── main.cpp              # Ponto de entrada (setup/loop)
├── config.h              # Constantes globais
├── button/               # Debounce e detecção de gestos
├── hardware/             # HardwareMap, ControlReader, UnifiedControlList
├── i2c/                  # I2CBus, I2CScanner, ModuleDescriptor, WireI2CBus
├── midi/                 # MidiEngine, MidiNote, MidiCC
├── screens/              # Telas da interface (Menu, Performance, Config, etc.)
├── storage/              # Persistência NVS
└── ui/                   # Framework UI (OledApp, Router, Screen, State, componentes)

test/                     # Testes unitários e PBT (ambiente native)
docs/                     # Documentação (API, checklists, conector DB-9)
```

## Compilar e enviar

Requer [PlatformIO](https://platformio.org/).

```bash
# Compilar
pio run -e esp32-s3-n16r8

# Enviar para a placa
pio run -e esp32-s3-n16r8 -t upload

# Monitor serial
pio device monitor
```

## Testes

Os testes rodam no ambiente `native` (PC, sem placa). Requer GCC/G++ instalado no sistema.

```bash
# Rodar todos os testes
pio test -e native

# Rodar uma suíte específica
pio test -e native -f test_midi
```

## Documentação

- [API completa](docs/API.md)
- [TODO — Produto final](docs/TODO-produto-final.md)
- [Checklist de validação](docs/checklist-validacao.md)
- [Checklist de expansão modular](docs/checklist-expansao-modular.md)
- [Conector DB-9 I2C](docs/conector-db9-i2c.md)
- [Guia de aprendizado](docs/checklist-aprendizado.md)
