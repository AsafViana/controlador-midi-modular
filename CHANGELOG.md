# Changelog

Todas as mudanças notáveis neste projeto serão documentadas neste arquivo.

O formato segue [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/),
e este projeto adere ao [Versionamento Semântico](https://semver.org/lang/pt-BR/).

## [1.14.0] - 2026-05-08

### Adicionado

- Contraste do display ajustável (Baixo/Médio/Alto)
- CHANGELOG.md formal

## [1.13.0] - 2026-05-08

### Adicionado

- Sistema de idioma configurável (Português/English)
- Tabela de strings i18n com 21 entradas

## [1.12.0] - 2026-05-08

### Adicionado

- LED RGB de status com padrões de cor (verde/azul/vermelho/roxo)
- Flash azul a cada envio MIDI
- Ativável via HardwareMap::RGB_LED_ENABLED

## [1.11.0] - 2026-05-08

### Adicionado

- First-run wizard para primeira configuração
- Detecção de primeiro boot via flag NVS

## [1.10.0] - 2026-05-08

### Adicionado

- SysEx backup/restore de configuração
- Formato proprietário com checksum XOR
- BackupScreen no menu de configurações

## [1.9.0] - 2026-05-08

### Adicionado

- Suporte a botões MIDI (momentâneo e toggle)
- Debounce de 50ms para botões MIDI
- Tipos BOTAO_MIDI_MOMENTANEO e BOTAO_MIDI_TOGGLE

## [1.8.0] - 2026-05-08

### Adicionado

- Indicadores de scroll (^/v) nas listas

## [1.7.0] - 2026-05-08

### Adicionado

- Screensaver com dim (2min) e desligamento (5min)
- Qualquer botão acorda o display

## [1.6.0] - 2026-05-08

### Adicionado

- GPIOs analógicos livres ativados (6 controles: Volume, Pan, Modulação, Sensor Luz, Expressão)

## [1.5.0] - 2026-05-08

### Adicionado

- Curvas de resposta configuráveis (Linear, Logarítmica, Exponencial)
- Lookup tables de 128 bytes pré-calculadas

## [1.4.0] - 2026-05-08

### Adicionado

- Calibração individual de potenciômetros
- CalibracaoScreen com fluxo guiado

## [1.3.0] - 2026-05-08

### Adicionado

- MIDI IN com Thru e filtro de canal
- Processamento de CC recebido na PerformanceScreen

## [1.2.0] - 2026-05-08

### Adicionado

- Suporte a Program Change (envio manual 0-127)
- ProgramChangeScreen no menu de configurações

## [1.1.0] - 2026-05-08

### Adicionado

- Botão BACK dedicado (GPIO 14)
- Cancelar edição sem salvar (BACK restaura valor original)
- Watchdog Timer de 5 segundos
- Timeout de inatividade (60s → PerformanceScreen)
- Feedback "Salvo!" textual (overlay 800ms)
- Proteção NVS com double-buffer e CRC8
- Modo headless (MIDI funciona sem display)
- Sistema de presets (4 slots)

## [1.0.0] - 2026-05-08

### Marco

- Baseline antes das melhorias de produto
- Firmware funcional com navegação, CC map, Performance monitor
