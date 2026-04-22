# Relatório Técnico — Controlador MIDI Modular

> [!info] Objetivo
> Documento de referência completo do sistema. Cobre hardware, software, protocolos, conexões, lógica e arquitetura. Use como guia durante todo o desenvolvimento.

---

## 1. Hardware

### 1.1 Microcontrolador principal

| Item | Valor |
|---|---|
| Chip | ESP32-S3 |
| Board | esp32-s3-devkitc-1 |
| Flash | 16MB (QIO) |
| PSRAM | Sim (OPI) |
| Framework | Arduino |
| USB | CDC on boot (modo dispositivo USB nativo) |

### 1.2 Display OLED

| Item | Valor |
|---|---|
| Controlador | SSD1306 |
| Resolução | 128 × 64 pixels |
| Tipo | Bicolor (amarelo 0-15px / azul 16-63px) |
| Interface | I2C |
| Endereço | `0x3C` |
| Biblioteca | Adafruit SSD1306 + Adafruit GFX |

### 1.3 Pinagem do módulo principal

| Pino GPIO | Função | Tipo |
|---|---|---|
| 5 | I2C SDA (display) | I2C |
| 4 | I2C SCL (display) | I2C |
| 1 | I2C SDA (barramento expansão) | I2C |
| 2 | I2C SCL (barramento expansão) | I2C |
| 17 | MIDI DIN TX (serial 31250 baud) | UART TX |
| 11 | Botão UP | Digital, INPUT_PULLUP |
| 12 | Botão DOWN | Digital, INPUT_PULLUP |
| 13 | Botão SELECT | Digital, INPUT_PULLUP |
| 0 | LED indicador | Digital, OUTPUT |
| 1, 2, 3 | Potenciômetros (configurável) | Analógico |
| 6 | Sensor de luz (configurável) | Analógico |

> [!warning] Atenção
> Os GPIOs 1 e 2 aparecem tanto no barramento I2C de expansão (`WireI2CBus`) quanto como potenciômetros no `HardwareMap`. Verifique se há conflito na sua montagem real. O display usa GPIOs 5/4 (separado).

### 1.4 Controles MIDI locais (definidos em `HardwareMap.h`)

| Label | GPIO | Tipo | CC Padrão | Invertido |
|---|---|---|---|---|
| Pot Volume | 1 | Potenciômetro | 7 | Não |
| Pot Pan | 2 | Potenciômetro | 10 | Não |
| Pot Modulacao | 3 | Potenciômetro | 1 | Não |
| Sensor Luz | 6 | Sensor | 11 | Sim |

> [!tip] Para adicionar um novo controle
> Basta adicionar uma linha no array `CONTROLES[]` em `src/hardware/HardwareMap.h`. O `NUM_CONTROLES` se recalcula automaticamente e todo o sistema (Storage, ControlReader, CCMapScreen) se adapta.

---

## 2. Protocolos de comunicação

### 2.1 USB MIDI

| Item | Valor |
|---|---|
| Protocolo | USB MIDI 1.0 (classe USB nativa) |
| Biblioteca | Control Surface v2.1.0 |
| Interface | `USBMIDI_Interface` (USB CDC nativo do ESP32-S3) |
| Device name | "Controlador MIDI" |
| Direção | Somente saída (device → host) |

### 2.2 MIDI DIN (serial 5 pinos)

| Item | Valor |
|---|---|
| Protocolo | MIDI 1.0 serial (31250 baud, 8N1) |
| Pino TX | GPIO 17 |
| UART | Serial1 do ESP32-S3 |
| Direção | Somente saída (TX only) |
| Implementação | Bytes MIDI enviados diretamente via Serial1.write() |
| Conector | DIN 5 pinos fêmea (pinos 4=TX, 5=GND, 2=shield) |

> [!tip] Circuito MIDI DIN TX
> GPIO 17 → resistor 220Ω → pino 5 do DIN. 3.3V → resistor 220Ω → pino 4 do DIN. Pino 2 do DIN → GND (shield).

**Mensagens suportadas (enviadas por USB e DIN simultaneamente):**

- **Note On** — `0x90 + canal`, nota, velocidade
- **Note Off** — `0x80 + canal`, nota, 0
- **Control Change (CC)** — `0xB0 + canal`, controlador, valor

### 2.2 I2C — Display OLED

| Item | Valor |
|---|---|
| Barramento | Wire (pinos SDA=5, SCL=4) |
| Endereço | `0x3C` |
| Velocidade | Padrão (100kHz) |
| Uso | Somente escrita (envio de framebuffer) |

### 2.3 I2C — Barramento de expansão modular

| Item | Valor |
|---|---|
| Barramento | Wire (pinos SDA=1, SCL=2) |
| Range de endereços | `0x20` a `0x27` (8 slots) |
| Endereço excluído | `0x3C` (reservado para OLED) |
| Velocidade | Padrão (100kHz) |
| Timeout por transação | 50ms |
| Pull-ups recomendados | 4.7kΩ em SDA e SCL |

---

## 3. Protocolo I2C modular (mestre ↔ escravo)

### 3.1 Comandos

| Comando | Código | Direção | Descrição |
|---|---|---|---|
| `CMD_DESCRIPTOR` | `0x01` | Mestre → Escravo → Mestre | Solicita descritor do módulo |
| `CMD_READ_VALUES` | `0x02` | Mestre → Escravo → Mestre | Solicita valores atuais dos controles |

### 3.2 Formato do descritor (`CMD_DESCRIPTOR`)

```
Byte 0:       numControles (1-16)
Para cada controle (14 bytes):
  Byte 0:     tipo (0=BOTAO, 1=POTENCIOMETRO, 2=SENSOR, 3=ENCODER)
  Bytes 1-12: label (12 chars, null-padded)
  Byte 13:    valor atual (0-127)

Tamanho total: 1 + (numControles × 14) bytes
Máximo: 1 + (16 × 14) = 225 bytes
```

### 3.3 Formato da leitura de valores (`CMD_READ_VALUES`)

```
N bytes, um por controle (0-127)
N = numControles reportado no descritor
```

### 3.4 Fluxo de comunicação

```
1. Mestre envia 1 byte: CMD_DESCRIPTOR (0x01)
2. Mestre lê resposta: descritor completo
3. (periodicamente) Mestre envia 1 byte: CMD_READ_VALUES (0x02)
4. Mestre lê resposta: N bytes de valores
```

### 3.5 Resiliência

| Parâmetro | Valor |
|---|---|
| Máximo de falhas consecutivas | 3 (`MAX_FAIL_COUNT`) |
| Intervalo de rescan | 5 segundos (`RESCAN_INTERVAL_MS`) |
| Comportamento em falha | Mantém último valor, não envia CC |
| Após 3 falhas | Módulo marcado como desconectado |
| Reconexão | Automática no próximo rescan |

---

## 4. MIDI — Lógica de envio

### 4.1 Controles analógicos (CC)

```
loop() → ControlReader::update()
  ├── Para cada controle local habilitado:
  │     1. analogRead(gpio) → 0-4095
  │     2. Converte para 0-127 (÷32)
  │     3. Inverte se necessário
  │     4. Aplica zona morta (diferença > 1)
  │     5. Busca CC no Storage
  │     6. MidiEngine::sendCC(cc, valor, canal)
  │
  └── Para cada controle remoto habilitado:
        1. I2CScanner::readValues() → 0-127
        2. Aplica zona morta
        3. Busca CC no Storage (chave: endereço + índice)
        4. MidiEngine::sendCC(cc, valor, canal)
```

| Parâmetro | Valor |
|---|---|
| Resolução ADC | 12 bits (0-4095) |
| Range MIDI | 0-127 |
| Zona morta | 1 (diferença mínima para enviar) |
| Intervalo de leitura | 10ms (`INTERVALO_MS`) |
| Canal MIDI | Configurável (1-16), compartilhado por todos os controles |

### 4.2 Notas (teclado)

```
Botão PRESSED  → sendNoteOn(nota, velocidade, canal)
Botão RELEASED → sendNoteOff(nota, 0, canal)
```

| Parâmetro | Valor padrão |
|---|---|
| Velocidade | 100 |
| Oitava | 4 |
| Canal | 1 (configurável) |
| Nota | C da oitava configurada |
| Habilitável | Sim (toggle ON/OFF na tela de configurações) |

---

## 5. Armazenamento persistente (NVS)

### 5.1 Tecnologia

| Item | Valor |
|---|---|
| Backend | ESP32 Preferences (NVS — Non-Volatile Storage) |
| Namespace | `midi_cfg` |
| Sobrevive a | Reboot, power cycle, upload de firmware (se partição NVS não for apagada) |

### 5.2 Chaves armazenadas

**Configurações globais:**

| Chave | Tipo | Default | Descrição |
|---|---|---|---|
| `canal` | uint8 | 1 | Canal MIDI (1-16) |
| `oitava` | uint8 | 4 | Oitava do teclado (0-8) |
| `vel` | uint8 | 100 | Velocidade das notas (0-127) |
| `teclado` | bool | true | Teclado habilitado |

**Controles locais (por índice `i`):**

| Chave | Tipo | Default | Descrição |
|---|---|---|---|
| `cc{i}` | uint8 | `HardwareMap::ccPadrao[i]` | Número CC do controle |
| `en{i}` | bool | true | Controle habilitado |

**Controles remotos (por endereço `XX` hex + índice `YY` hex):**

| Chave | Tipo | Default | Descrição |
|---|---|---|---|
| `rcc{XX}{YY}` | uint8 | 0 | Número CC do controle remoto |
| `ren{XX}{YY}` | bool | true | Controle remoto habilitado |

> [!tip] Exemplo
> Módulo no endereço `0x20`, controle índice 1: chaves `rcc2001` e `ren2001`.

---

## 6. Interface de usuário (OLED)

### 6.1 Layout do display

```
┌──────────────────────────────┐
│  HEADER (amarelo) Y: 0-15    │  ← Título da tela + indicador MIDI
├──────────────────────────────┤
│                              │
│  CONTEÚDO (azul) Y: 16-63   │  ← Listas, valores, barras
│                              │
│                              │
└──────────────────────────────┘
   128px largura × 64px altura
```

| Constante | Valor | Definida em |
|---|---|---|
| `HEADER_HEIGHT` | 16px | `config.h` |
| `CONTENT_Y` | 16px | `config.h` |
| `CONTENT_HEIGHT` | 48px | `config.h` |

### 6.2 Navegação por botões

| Gesto | Evento | Ação |
|---|---|---|
| Clique curto | `SINGLE_CLICK` | Navegar para baixo / incrementar |
| Dois cliques rápidos | `DOUBLE_CLICK` | Confirmar / entrar |
| Segurar (>800ms) | `LONG_PRESS` | Voltar / subir / decrementar |
| Pressionar | `PRESSED` | Note On (só na tela Performance) |
| Soltar | `RELEASED` | Note Off (só na tela Performance) |

**Parâmetros do botão:**

| Parâmetro | Valor |
|---|---|
| Debounce | 50ms |
| Long press | 800ms |
| Double click window | 300ms |

### 6.3 Mapa de telas

```
MenuScreen (raiz)
├── PerformanceScreen
│     └── (LONG_PRESS volta)
├── ConfigScreen
│     ├── CCMapScreen
│     │     └── Modo edição: CC → ON/OFF → sai
│     ├── CanalScreen
│     │     └── Incrementa canal, DOUBLE_CLICK salva
│     └── Toggle Teclado ON/OFF
└── Sobre (futuro)
```

### 6.4 Componentes UI disponíveis

| Componente | Descrição |
|---|---|
| `TextComponent` | Texto com posição, tamanho e cor configuráveis. Trunca automaticamente. |
| `ListComponent` | Lista selecionável com scroll automático e highlight invertido. |
| `ProgressBarComponent` | Barra horizontal com borda e preenchimento proporcional (0-100). |
| `IconComponent` | Bitmap monocromático (PROGMEM). |
| `MidiActivityComponent` | Quadrado que pisca por 100ms a cada envio MIDI. |

### 6.5 Arquitetura UI

| Classe | Responsabilidade |
|---|---|
| `OledApp` | Fachada principal. Gerencia display, botões, router e render loop (30fps). |
| `Router` | Pilha LIFO de telas. Push/pop/navigateTo. Encaminha eventos de botão. |
| `Screen` | Classe base. Ciclo de vida (onMount/onUnmount), dirty flag, filhos. |
| `UIComponent` | Interface abstrata para componentes visuais. |
| `State<T>` | Template reativo. Marca a tela como dirty quando o valor muda. |

**Render loop:**

```
OledApp::update() — a cada 33ms (30fps):
  1. Atualiza cada botão → gera eventos
  2. Encaminha eventos para Router → Screen atual
  3. Se screen.isDirty() ou MIDI ativo:
     a. clearDisplay()
     b. screen.render(display)
     c. midiActivity.render(display)
     d. display.display()
     e. screen.clearDirty()
```

---

## 7. Expansão modular — Visão geral

### 7.1 Arquitetura

```
┌─────────────────┐     I2C (SDA=1, SCL=2)     ┌──────────────┐
│  Módulo          │◄──────────────────────────►│  Escravo 1   │
│  Principal       │                             │  Addr: 0x20  │
│  (ESP32-S3)      │◄──────────────────────────►│  Escravo 2   │
│                  │                             │  Addr: 0x21  │
│  I2C (SDA=5,     │     ┌──────────────┐       └──────────────┘
│   SCL=4)         │◄───►│  Display OLED│
│                  │     │  Addr: 0x3C  │
│  USB ────────────│────►│  Host (DAW)  │
└─────────────────┘     └──────────────┘
```

### 7.2 Limites

| Recurso | Limite |
|---|---|
| Módulos escravos | 8 (endereços `0x20`-`0x27`) |
| Controles por módulo | 16 |
| Total de controles (locais + remotos) | 32 (`MAX_TOTAL_CONTROLS`) |
| Configurações remotas em cache | 128 (8 × 16) |
| Label máximo | 12 caracteres |

### 7.3 Fluxo de descoberta

```
1. scan() no setup() — varredura completa 0x20-0x27
2. periodicScan() no loop() — a cada 5s:
   a. Verifica módulos conhecidos (probe) → detecta desconexões
   b. Procura novos endereços → detecta conexões
3. rebuild() no loop() — reconstrói lista unificada
4. CCMapScreen mostra locais + remotos juntos
```

### 7.4 Classes envolvidas

| Classe | Arquivo | Responsabilidade |
|---|---|---|
| `I2CBus` | `src/i2c/I2CBus.h` | Interface abstrata I2C (permite mock) |
| `WireI2CBus` | `src/i2c/WireI2CBus.cpp` | Implementação real com Wire |
| `I2CScanner` | `src/i2c/I2CScanner.cpp` | Descoberta, probe, leitura de módulos |
| `ModuleDescriptor` | `src/i2c/ModuleDescriptor.cpp` | Serialização/desserialização do descritor |
| `UnifiedControlList` | `src/hardware/UnifiedControlList.cpp` | Lista unificada locais + remotos |
| `ControlReader` | `src/hardware/ControlReader.cpp` | Leitura automática + envio CC |
| `Storage` | `src/storage/Storage.cpp` | Persistência de configurações remotas |

---

## 8. Dependências e build

### 8.1 Bibliotecas

| Biblioteca | Versão | Uso |
|---|---|---|
| Control Surface | ^2.1.0 | USB MIDI (USBMIDI_Interface) |
| MIDI Library | latest | Dependência do Control Surface |
| Adafruit SSD1306 | latest | Driver do display OLED |
| Adafruit GFX | latest | Primitivas gráficas (texto, retângulos, bitmaps) |

### 8.2 Ambientes PlatformIO

| Ambiente | Plataforma | Uso |
|---|---|---|
| `esp32-s3-n16r8` | espressif32 | Build real para o hardware |
| `native` | native | Testes unitários no PC (mocks) |

### 8.3 Flags de build (ESP32-S3)

| Flag | Efeito |
|---|---|
| `BOARD_HAS_PSRAM` | Habilita PSRAM |
| `ARDUINO_USB_MODE=0` | USB OTG (não CDC nativo do chip) |
| `ARDUINO_USB_CDC_ON_BOOT=1` | Serial via USB CDC |

### 8.4 Testes

| Framework | Unity (C/C++) |
|---|---|
| Ambiente | `native` (compila com gcc/g++ no PC) |
| Mocks | `test/mocks/` (Arduino, Wire, SSD1306, Control_Surface, Serial) |
| Total de suítes | 26 |

---

## 9. Estrutura de arquivos

```
src/
├── main.cpp                    # Setup e loop principal
├── config.h                    # Constantes globais (display, MIDI, layout)
├── button/
│   ├── Button.h/cpp            # Debounce, detecção de gestos
├── midi/
│   ├── MidiEngine.h/cpp        # Envio USB MIDI (Note, CC)
│   ├── MidiCC.h                # Struct de mensagem CC
│   ├── MidiNote.h              # Struct de mensagem Note
├── hardware/
│   ├── HardwareMap.h           # Definição de pinos e controles físicos
│   ├── ControlReader.h/cpp     # Leitura automática de analógicos + envio CC
│   ├── UnifiedControlList.h/cpp # Lista unificada (locais + remotos)
├── storage/
│   ├── Storage.h/cpp           # Persistência NVS (CC, canal, habilitado)
├── i2c/
│   ├── I2CBus.h                # Interface abstrata I2C
│   ├── WireI2CBus.h/cpp        # Implementação real (Wire)
│   ├── I2CScanner.h/cpp        # Descoberta e leitura de módulos
│   ├── ModuleDescriptor.h/cpp  # Protocolo de descritor (serialize/deserialize)
├── ui/
│   ├── OledApp.h/cpp           # Fachada principal (display + botões + router)
│   ├── Router.h/cpp            # Pilha de navegação entre telas
│   ├── Screen.h/cpp            # Classe base de tela
│   ├── UIComponent.h           # Interface base de componente visual
│   ├── State.h                 # Template reativo
│   ├── components/
│   │   ├── TextComponent       # Texto
│   │   ├── ListComponent       # Lista selecionável com scroll
│   │   ├── ProgressBarComponent # Barra de progresso
│   │   ├── IconComponent       # Bitmap/ícone
│   │   ├── MidiActivityComponent # Indicador de atividade MIDI
├── screens/
│   ├── MenuScreen              # Menu principal
│   ├── PerformanceScreen       # Tela de performance (notas + CC visual)
│   ├── ConfigScreen            # Lista de configurações
│   ├── CCMapScreen             # Endereçamento CC por controle
│   ├── CanalScreen             # Seleção de canal MIDI
```

---

## 10. Loop principal — Ordem de execução

```cpp
void loop() {
    controlReader.update();   // 1. Lê analógicos, envia CC
    scanner.periodicScan();   // 2. Verifica módulos I2C (a cada 5s)
    ucl.rebuild();            // 3. Reconstrói lista unificada
    app.update();             // 4. Atualiza botões, UI, render (30fps)
}
```

| Etapa | Frequência | Bloqueante |
|---|---|---|
| ControlReader | A cada 10ms | Não (leitura rápida) |
| periodicScan | A cada 5s | Potencialmente (I2C timeout 50ms) |
| rebuild | Toda iteração | Não (operação em memória) |
| OledApp update | A cada 33ms | Não (buffer + display) |
