# Checklist de Aprendizado — Controlador MIDI Modular

> Guia passo a passo para entender, manter e expandir o projeto.
> Pensado para quem está começando em C/C++ e sistemas embarcados.
> Vá ponto a ponto — cada item constrói sobre o anterior.

---

## Fase 1 — Fundamentos de C/C++ usados no projeto

Antes de mexer no código, você precisa entender as peças da linguagem que o projeto usa.

### 1.1 Variáveis, tipos e constantes

- [ ] Tipos inteiros com tamanho fixo: `uint8_t`, `uint16_t`, `uint32_t` (por que usar eles em vez de `int`)
- [ ] `const` e `constexpr` — valores que nunca mudam (ex: `OLED_WIDTH`, `MAX_CHILDREN`)
- [ ] `#define` vs `constexpr` — o projeto usa os dois, entender a diferença
- [ ] `bool`, `true`, `false`

### 1.2 Structs

- [ ] O que é uma `struct` e como agrupar dados relacionados
- [ ] Exemplos no projeto: `MidiNote`, `MidiCC`, `ControleHW`, `ControlInfo`
- [ ] Como acessar campos: `nota.canal`, `cc.valor`

### 1.3 Enums

- [ ] `enum class` — constantes nomeadas com segurança de tipo
- [ ] Exemplos: `ButtonEvent` (NONE, PRESSED, SINGLE_CLICK...), `TipoControle` (BOTAO, POTENCIOMETRO...)
- [ ] Por que `enum class` é melhor que `enum` simples

### 1.4 Arrays e tamanhos fixos

- [ ] Arrays de tamanho fixo: `Screen* _stack[8]`
- [ ] `sizeof(array) / sizeof(array[0])` para calcular tamanho automaticamente (usado em `NUM_CONTROLES`)
- [ ] Por que o projeto evita alocação dinâmica (`new`/`delete`) — estabilidade em embarcados

### 1.5 Ponteiros

- [ ] O que é um ponteiro (`*`) e referência (`&`)
- [ ] Ponteiro para objeto: `Screen* screen` — usado em todo o Router e OledApp
- [ ] `nullptr` — ponteiro que não aponta pra nada
- [ ] Arrays de ponteiros: `Screen* _stack[MAX_STACK_SIZE]`
- [ ] Quando usar ponteiro vs referência no projeto

### 1.6 Classes e objetos

- [ ] Classe = struct com funções e controle de acesso (`public`, `private`)
- [ ] Construtor — inicialização do objeto (ex: `OledApp()`, `Button(gpio, activeLow)`)
- [ ] Métodos — funções que pertencem à classe (ex: `engine.sendCC(cc)`)
- [ ] `this` — ponteiro para o próprio objeto
- [ ] Membros `static` e `constexpr` dentro de classes

### 1.7 Herança e métodos virtuais

- [ ] Herança: `class MenuScreen : public Screen` — MenuScreen É uma Screen
- [ ] `virtual` — permite que classes filhas substituam o comportamento
- [ ] `override` — marca que o método está substituindo o da classe pai
- [ ] `= 0` (virtual puro) — obriga a classe filha a implementar (ex: `I2CBus`)
- [ ] `virtual ~Destrutor() = default` — destrutor virtual para herança segura

### 1.8 Templates (básico)

- [ ] O que é um template: `State<T>` funciona com qualquer tipo (uint8_t, bool, etc.)
- [ ] Como usar: `State<uint8_t> _canal{1}` — estado reativo que guarda um uint8_t
- [ ] Você não precisa criar templates novos, só usar os que existem

### 1.9 Namespaces

- [ ] `namespace App { class Button; }` — evita conflito de nomes
- [ ] `namespace HardwareMap { ... }` — agrupa constantes de hardware
- [ ] Como acessar: `App::Button`, `HardwareMap::PIN_BTN_UP`

### 1.10 Includes e organização de arquivos

- [ ] `.h` (header) = declaração — diz O QUE existe
- [ ] `.cpp` (source) = implementação — diz COMO funciona
- [ ] `#pragma once` — evita incluir o mesmo header duas vezes
- [ ] `#include "arquivo.h"` (local) vs `#include <biblioteca>` (externa)
- [ ] Forward declaration: `class Router;` — declara que a classe existe sem incluir o header inteiro

---

## Fase 2 — Ferramentas e ambiente

### 2.1 PlatformIO

- [ ] O que é PlatformIO e por que é usado em vez da Arduino IDE
- [ ] Estrutura do projeto: `src/`, `lib/`, `include/`, `test/`, `platformio.ini`
- [ ] O arquivo `platformio.ini` — dois ambientes: `esp32-s3-n16r8` (hardware) e `native` (testes no PC)
- [ ] Comandos essenciais:
  - [ ] `pio run` — compilar para o ESP32
  - [ ] `pio run -t upload` — enviar para a placa
  - [ ] `pio test -e native` — rodar testes no PC
  - [ ] `pio device monitor` — ver Serial do ESP32

### 2.2 Bibliotecas externas (lib_deps)

- [ ] **Control Surface** (v2.1.0) — USB MIDI nativo
- [ ] **Adafruit SSD1306** — driver do display OLED
- [ ] **Adafruit GFX** — primitivas gráficas (texto, linhas, retângulos)
- [ ] **MIDI Library** — dependência do Control Surface

### 2.3 Testes

- [ ] Framework Unity — testes unitários em C/C++
- [ ] Ambiente `native` — compila e roda no PC, sem precisar da placa
- [ ] Mocks em `test/mocks/` — simulam Arduino, Wire, Display, MIDI
- [ ] Testes PBT (property-based testing) — testes com valores aleatórios

---

## Fase 3 — Arquitetura do sistema (visão geral)

### 3.1 Fluxo principal (main.cpp)

- [ ] `setup()` — roda uma vez: inicializa tudo (MIDI, Storage, I2C, Display, Splash, Telas, Botões)
- [ ] `loop()` — roda infinitamente: atualiza UI (`app.update()`) e lê controles (`controlReader.update()`)
- [ ] Ordem de inicialização importa — entender por que cada coisa é inicializada naquela ordem
- [ ] Splash screen — `app.showSplash()` exibe nome e versão por 1.5s no boot
- [ ] Fallback LED — se o display falhar, o LED pisca rapidamente para indicar erro

### 3.2 Camadas do sistema

- [ ] Entender o diagrama de camadas (de cima para baixo):

  ```
  Telas (MenuScreen, ConfigScreen, CCMapScreen, CanalScreen, PerformanceScreen)
      ↓
  Framework UI (OledApp, Router, Screen, State<T>, UIComponents)
      ↓
  Hardware (HardwareMap, ControlReader, UnifiedControlList)
      ↓
  MIDI (MidiEngine, MidiNote, MidiCC)
      ↓
  I2C (I2CBus, WireI2CBus, I2CScanner, ModuleDescriptor)
      ↓
  Storage (NVS — persistência em flash)
      ↓
  Botões (Button — debounce, long press, double click)
  ```

### 3.3 Como as peças se conectam

- [ ] Botão pressionado → OledApp converte para NavInput (UP/DOWN/SELECT) → Router repassa para Screen ativa
- [ ] Botão mantido (long press) → OledApp envia LONG_UP/LONG_DOWN → telas de edição incrementam com passo maior
- [ ] Screen muda State → marca dirty → próximo frame redesenha
- [ ] ControlReader lê potenciômetros → converte para 0-127 → MidiEngine envia CC → callback notifica PerformanceScreen
- [ ] I2CScanner descobre módulos → UnifiedControlList junta locais + remotos
- [ ] Storage salva configurações na flash → sobrevive a reinicializações

---

## Fase 4 — Framework de UI (o coração visual)

### 4.1 OledApp (fachada)

- [ ] `begin()` — inicializa display e limpa tela
- [ ] `showSplash(nome, versao)` — exibe tela de splash no boot (nome do produto + versão)
- [ ] `showSaveConfirm()` — feedback visual rápido (inversão do display por 150ms) ao salvar configurações
- [ ] `update()` — ciclo principal: lê botões, renderiza se dirty, 30 FPS (~33ms por frame)
- [ ] Detecção de LONG_PRESS → envia `NavInput::LONG_UP` / `LONG_DOWN` para aceleração
- [ ] `setButtonUp()`, `setButtonDown()`, `setButtonSelect()` — registra os 3 botões de navegação
- [ ] `getRouter()` — acesso ao sistema de navegação
- [ ] `getMidiActivity()` — acesso ao indicador visual de atividade MIDI

### 4.2 Router (navegação por pilha)

- [ ] Pilha LIFO de Screens (máximo 8)
- [ ] `push(screen)` — empilha nova tela (chama `onMount`)
- [ ] `pop()` — volta para tela anterior (chama `onUnmount` / `onMount`)
- [ ] `navigateTo(screen)` — substitui tela atual
- [ ] `handleInput(NavInput)` — encaminha evento de navegação para tela do topo

### 4.3 Screen (classe base das telas)

- [ ] Ciclo de vida: `onMount()` → `render()` (repetido) → `onUnmount()`
- [ ] `handleInput(NavInput)` — cada tela decide o que fazer com cada botão (UP, DOWN, SELECT)
- [ ] `addChild(component)` — composição de componentes visuais
- [ ] `markDirty()` / `isDirty()` — controle de quando redesenhar
- [ ] `render(display)` — base itera filhos; subclasses podem sobrescrever

### 4.4 State<T> (estado reativo)

- [ ] `State<uint8_t> _canal{1}` — cria estado com valor inicial 1
- [ ] `_canal.get()` — lê o valor
- [ ] `_canal.set(5)` — muda o valor E marca a tela como dirty automaticamente
- [ ] Precisa chamar `_canal.attachScreen(this)` no `onMount()` para funcionar

### 4.5 Componentes visuais (UIComponent)

- [ ] **TextComponent** — texto com posição (x,y), tamanho de fonte (1-3), cor
- [ ] **ListComponent** — lista selecionável com scroll e callback de seleção
- [ ] **ProgressBarComponent** — barra horizontal (0-100%)
- [ ] **IconComponent** — bitmap armazenado em PROGMEM (flash)
- [ ] **MidiActivityComponent** — indicador visual de atividade MIDI

---

## Fase 5 — Sistema MIDI

### 5.1 Conceitos MIDI básicos

- [ ] O que é MIDI — protocolo de comunicação musical
- [ ] Canal MIDI (1-16) — como endereçar instrumentos diferentes
- [ ] Note On / Note Off — tocar e soltar uma nota
- [ ] Control Change (CC) — controles contínuos (volume, pan, modulação, etc.)
- [ ] Valores MIDI: 0-127 (7 bits)

### 5.2 MidiEngine

- [ ] `begin()` — inicializa USB MIDI e MIDI DIN (Serial1 a 31250 baud)
- [ ] `sendNoteOn(note)` / `sendNoteOff(note)` — envia nota por USB e DIN
- [ ] `sendCC(cc)` — envia Control Change por USB e DIN
- [ ] `onActivity(callback)` — registra função chamada a cada envio (para o indicador visual)

### 5.3 Structs MIDI

- [ ] `MidiNote { nota, velocidade, canal }` — representa uma nota
- [ ] `MidiCC { controlador, valor, canal }` — representa um CC

---

## Fase 6 — Hardware e controles

### 6.1 HardwareMap (mapa de hardware)

- [ ] Definição centralizada de todos os pinos e controles
- [ ] Array `CONTROLES[]` — cada linha é um controle físico (label, gpio, tipo, ccPadrão, invertido)
- [ ] `NUM_CONTROLES` — calculado automaticamente pelo tamanho do array
- [ ] **Como adicionar um novo controle**: basta adicionar uma linha no array

### 6.2 ControlReader (leitura automática)

- [ ] Lê potenciômetros e sensores a cada 10ms
- [ ] Converte leitura analógica (0-4095) para valor MIDI (0-127)
- [ ] Aplica inversão se configurado
- [ ] Aplica dead zone (ignora variações menores que 2) — evita ruído
- [ ] Consulta Storage para saber qual CC enviar
- [ ] Também lê controles remotos via I2C
- [ ] `onCCActivity(callback)` — notifica a cada envio de CC com dados completos (`CCActivityInfo`)
- [ ] `CCActivityInfo` — struct com label, CC, valor, canal, módulo de origem (local ou remoto)

### 6.3 Button (sistema de botões)

- [ ] Debounce de 50ms — filtra ruído elétrico
- [ ] Eventos: PRESSED, RELEASED, SINGLE_CLICK, LONG_PRESS (800ms), DOUBLE_CLICK (300ms)
- [ ] `update()` — deve ser chamado no loop para detectar eventos
- [ ] `isHeld()` / `heldDuration()` — para ações contínuas

---

## Fase 7 — Expansão I2C (módulos remotos)

### 7.1 Conceitos I2C

- [ ] Barramento com 2 fios: SDA (dados) e SCL (clock)
- [ ] Master (ESP32) e Slaves (módulos de expansão)
- [ ] Endereços: cada dispositivo tem um endereço único (0x20-0x27 para módulos)
- [ ] Pull-ups de 4.7kΩ nos fios SDA e SCL

### 7.2 I2CBus (interface abstrata)

- [ ] `probe(address)` — verifica se há dispositivo no endereço
- [ ] `write(address, data, length)` — envia dados para o dispositivo
- [ ] `requestFrom(address, buffer, length)` — solicita dados do dispositivo
- [ ] `WireI2CBus` — implementação real (hardware)
- [ ] `MockI2CBus` — implementação fake (testes)

### 7.3 I2CScanner (descoberta de módulos)

- [ ] `scan()` — varredura completa dos endereços 0x20-0x27
- [ ] `periodicScan()` — re-scan a cada 5 segundos (detecta conexão/desconexão)
- [ ] Resiliência: 3 falhas consecutivas → marca módulo como desconectado
- [ ] Reconexão automática no próximo scan

### 7.4 ModuleDescriptor (protocolo binário)

- [ ] Comando `0x01` — solicita descritor do módulo (quantos controles, tipos, labels)
- [ ] Comando `0x02` — solicita valores atuais dos controles
- [ ] Formato binário: 1 byte (numControles) + 14 bytes por controle

### 7.5 UnifiedControlList (lista unificada)

- [ ] Combina controles locais (HardwareMap) + remotos (I2CScanner)
- [ ] Índices 0..N-1 = locais, N..M = remotos
- [ ] Máximo 32 controles no total
- [ ] `rebuild()` — reconstrói a lista quando módulos mudam

---

## Fase 8 — Persistência (Storage)

### 8.1 NVS do ESP32

- [ ] Non-Volatile Storage — dados salvos na flash que sobrevivem a reinicializações
- [ ] Funciona como um dicionário chave-valor
- [ ] `begin()` — abre o namespace NVS

### 8.2 O que é salvo

- [ ] Canal MIDI (1-16)
- [ ] Oitava do teclado (0-8)
- [ ] Velocidade das notas (0-127)
- [ ] Teclado habilitado (sim/não)
- [ ] CC de cada controle local (ex: `cc0` = 7, `cc1` = 10)
- [ ] Habilitado/desabilitado de cada controle local
- [ ] CC e habilitado de cada controle remoto (por endereço I2C + índice)

### 8.3 API do Storage

- [ ] `getCanalMidi()` / `setCanalMidi(canal)`
- [ ] `getControladorCC(indice)` / `setControladorCC(indice, cc)`
- [ ] `isControleHabilitado(indice)` / `setControleHabilitado(indice, hab)`
- [ ] `getRemoteCC(addr, idx)` / `setRemoteCC(addr, idx, cc)`
- [ ] `factoryReset()` — restaura todas as configurações para os valores padrão de fábrica

---

## Fase 9 — Telas do sistema

### 9.1 Hierarquia de navegação

- [ ] **MenuScreen** (raiz) → Performance | Configurações | Sobre — com status no rodapé (Ch, Oit, Vel)
- [ ] **PerformanceScreen** → monitor CC em tempo real (label, CC, valor, módulo) + ajuste de oitava
- [ ] **ConfigScreen** → menu de configurações (CC Map, Canal, Oitava, Velocidade, Restaurar Padrões)
- [ ] **CCMapScreen** → mapear CC para cada controle (local e remoto), com aceleração no long press
- [ ] **CanalScreen** → selecionar canal MIDI (1-16), com aceleração
- [ ] **OitavaScreen** → selecionar oitava (0-8), com aceleração
- [ ] **VelocidadeScreen** → selecionar velocidade MIDI (1-127), com aceleração
- [ ] **TecladoScreen** → habilitar/desabilitar teclado (ON/OFF) — escondida do menu por enquanto
- [ ] **SobreScreen** → versão do firmware, canal MIDI, número de controles

### 9.2 Padrões de navegação

- [ ] 3 botões físicos: UP (GPIO 11), DOWN (GPIO 12), SELECT (GPIO 13)
- [ ] UP → subir na lista / incrementar valor
- [ ] DOWN → descer na lista / decrementar valor
- [ ] SELECT → confirmar / entrar no submenu / voltar (depende da tela)
- [ ] Cada botão gera `PRESSED` → OledApp converte para `NavInput::UP/DOWN/SELECT`
- [ ] Long press em UP/DOWN → OledApp envia `NavInput::LONG_UP/LONG_DOWN` → incremento rápido (+5 em velocidade/CC, +3 em canal, +2 em oitava)
- [ ] Feedback visual "Salvo" — inversão do display por 150ms ao confirmar com SELECT nas telas de configuração
- [ ] Factory reset — opção "Restaurar" no ConfigScreen com tela de confirmação (SELECT confirma, UP/DOWN cancela)

### 9.3 Como criar uma nova tela

- [ ] Criar `NovaTela.h` e `NovaTela.cpp` em `src/screens/`
- [ ] Herdar de `Screen`
- [ ] Implementar `onMount()`, `render()`, `handleInput()`
- [ ] Usar `State<T>` para dados reativos
- [ ] Compor com UIComponents (TextComponent, ListComponent, etc.)
- [ ] Registrar no Router via `push()` a partir de outra tela

---

## Fase 10 — Testes e manutenção

### 10.1 Rodando testes

- [ ] `pio test -e native` — roda todos os testes no PC
- [ ] `pio test -e native -f test_button` — roda só os testes de botão
- [ ] Entender a saída: `PASS` / `FAIL` / `IGNORED`

### 10.2 Escrevendo um teste simples

- [ ] Criar pasta `test/test_novatela/`
- [ ] Criar `test_novatela.cpp` com `#include <unity.h>`
- [ ] Funções `void test_xxx()` com `TEST_ASSERT_*`
- [ ] `void setUp()` / `void tearDown()` — preparação e limpeza
- [ ] `int main()` com `UNITY_BEGIN()` / `RUN_TEST()` / `UNITY_END()`

### 10.3 Mocks

- [ ] Entender que os mocks simulam o hardware no PC
- [ ] `mock_millis` — simula o tempo do Arduino
- [ ] `mock_analogRead` — simula leitura de potenciômetro
- [ ] `MockI2CBus` — simula comunicação I2C

### 10.4 Compilação e upload

- [ ] `pio run` — compilar (verificar erros antes de enviar)
- [ ] `pio run -t upload` — enviar para o ESP32
- [ ] `pio device monitor` — ver mensagens Serial para debug

---

## Fase 11 — Tarefas práticas (exercícios sugeridos)

Depois de estudar os pontos acima, tente estas tarefas para fixar o conhecimento:

### Nível 1 — Observação

- [ ] Ler `main.cpp` e identificar cada módulo sendo inicializado
- [ ] Ler `MenuScreen.cpp` e entender como os botões navegam entre telas
- [ ] Rodar `pio test -e native` e ver todos os testes passando

### Nível 2 — Modificação simples

- [ ] Mudar o texto de uma tela (ex: título do MenuScreen)
- [ ] Adicionar um novo controle no `HardwareMap.h` (ex: novo potenciômetro)
- [ ] Mudar o valor padrão de velocidade MIDI em `config.h`

### Nível 3 — Criação

- [ ] Criar uma nova tela de diagnóstico (DiagScreen) que mostra módulos I2C conectados
- [ ] Adicionar a tela de diagnóstico no menu de configurações
- [ ] Escrever um teste unitário para a nova tela

### Nível 4 — Integração

- [ ] Entender o fluxo completo: potenciômetro → analogRead → ControlReader → MidiEngine → USB + DIN
- [ ] Entender o fluxo I2C: scan → descriptor → values → CC
- [ ] Modificar o ControlReader para adicionar um novo tipo de controle

---

## Referências rápidas

| Preciso de... | Arquivo |
|---|---|
| Constantes globais | `src/config.h` |
| Mapa de hardware | `src/hardware/HardwareMap.h` |
| Ponto de entrada | `src/main.cpp` |
| API completa | `docs/API.md` |
| Conector DB-9 | `docs/conector-db9-i2c.md` |
| Specs de features | `.kiro/specs/` |

---

> **Dica**: Vá fase por fase. Não tente entender tudo de uma vez.
> Quando estiver pronto para começar um ponto, me chame que a gente trabalha junto.
