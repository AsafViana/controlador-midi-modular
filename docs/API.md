# Documentação da API — Controlador MIDI Modular

Referência completa de todos os módulos, suas funções e exemplos de uso.

---

## Sumário

- [Configuração (config.h)](#configuração-configh)
- [Módulo MIDI](#módulo-midi)
  - [MidiNote](#midinote)
  - [MidiCC](#midicc)
  - [MidiEngine](#midiengine)
- [Módulo Button](#módulo-button)
  - [ButtonEvent](#buttonevent)
  - [Button](#button)
- [Módulo UI](#módulo-ui)
  - [NavInput](#navinput)
  - [OledApp](#oledapp)
  - [Router](#router)
  - [Screen](#screen)
  - [State\<T\>](#statet)
  - [UIComponent](#uicomponent)
  - [TextComponent](#textcomponent)
  - [IconComponent](#iconcomponent)
  - [ListComponent](#listcomponent)
  - [ProgressBarComponent](#progressbarcomponent)
  - [MidiActivityComponent](#midiactivitycomponent)
- [Módulo Storage](#módulo-storage)
- [Módulo Hardware](#módulo-hardware)
  - [CCActivityInfo](#ccactivityinfo)
  - [ControlReader](#controlreader)
- [Módulo I2C](#módulo-i2c)
  - [I2CBus](#i2cbus)
  - [I2CScanner](#i2cscanner)
  - [ModuleDescriptor](#moduledescriptor)
  - [UnifiedControlList](#unifiedcontrollist)
- [Telas](#telas)
- [Exemplo Completo](#exemplo-completo)

---

## Configuração (config.h)

Constantes globais usadas por todos os módulos. Edite `src/config.h` para ajustar.

| Constante | Valor Padrão | Descrição |
|-----------|-------------|-----------|
| `USB_MIDI_DEVICE_NAME` | `"Controlador MIDI"` | Nome do dispositivo USB MIDI |
| `MIDI_DEFAULT_VELOCITY` | `100` | Velocidade padrão das notas |
| `MIDI_DEFAULT_CHANNEL` | `1` | Canal MIDI padrão (1-16) |
| `MIDI_DEFAULT_CC_VALUE` | `0` | Valor padrão para mensagens CC |
| `DISPLAY_I2C_ADDRESS` | `0x3C` | Endereço I2C do display OLED |
| `OLED_WIDTH` | `128` | Largura do display em pixels |
| `OLED_HEIGHT` | `64` | Altura do display em pixels |
| `HEADER_HEIGHT` | `16` | Altura da faixa amarela do display (Y 0-15) |
| `CONTENT_Y` | `16` | Início da área de conteúdo azul |
| `CONTENT_HEIGHT` | `48` | Altura da área de conteúdo (64 - 16) |

---

## Módulo MIDI

Arquivos: `src/midi/MidiNote.h`, `src/midi/MidiCC.h`, `src/midi/MidiEngine.h/.cpp`

### MidiNote

Struct que representa uma nota MIDI.

**Campos:**

| Campo | Tipo | Intervalo | Descrição |
|-------|------|-----------|-----------|
| `nota` | `uint8_t` | 0-127 | Número da nota MIDI |
| `velocidade` | `uint8_t` | 0-127 | Velocidade (intensidade) |
| `canal` | `uint8_t` | 1-16 | Canal MIDI |

**Construtor:**

```cpp
MidiNote(uint8_t nota,
         uint8_t vel   = MIDI_DEFAULT_VELOCITY,  // 100
         uint8_t canal = MIDI_DEFAULT_CHANNEL)    // 1
```

**Exemplos:**

```cpp
#include "midi/MidiNote.h"

// Nota Dó central (C4) com velocidade e canal padrão
MidiNote doMedio(60);
// doMedio.nota = 60, doMedio.velocidade = 100, doMedio.canal = 1

// Nota com valores customizados
MidiNote minhaNota(48, 80, 3);
// minhaNota.nota = 48, minhaNota.velocidade = 80, minhaNota.canal = 3

// Usando helpers de nota (da biblioteca Control_Surface)
MidiNote c4(MIDI_Notes::C(4));   // Dó na 4ª oitava (nota 48)
MidiNote e4(MIDI_Notes::E(4));   // Mi na 4ª oitava (nota 52)
MidiNote g4(MIDI_Notes::G(4));   // Sol na 4ª oitava (nota 55)
```

---

### MidiCC

Struct que representa uma mensagem MIDI Control Change.

**Campos:**

| Campo | Tipo | Intervalo | Descrição |
|-------|------|-----------|-----------|
| `controlador` | `uint8_t` | 0-127 | Número do controlador CC |
| `valor` | `uint8_t` | 0-127 | Valor do controlador |
| `canal` | `uint8_t` | 1-16 | Canal MIDI |

**Construtor:**

```cpp
MidiCC(uint8_t controlador,
       uint8_t valor = MIDI_DEFAULT_CC_VALUE,  // 0
       uint8_t canal = MIDI_DEFAULT_CHANNEL)   // 1
```

**Controladores CC comuns:**

| Nome | Número | Descrição |
|------|--------|-----------|
| Modulation | 1 | Roda de modulação |
| Volume | 7 | Volume do canal |
| Pan | 10 | Panorâmica esquerda/direita |
| Expression | 11 | Expressão |
| Sustain | 64 | Pedal de sustain (on/off) |
| All Notes Off | 123 | Desliga todas as notas |

**Exemplos:**

```cpp
#include "midi/MidiCC.h"

// Modulação com valor padrão (0) no canal padrão (1)
MidiCC modulacao(1);
// modulacao.controlador = 1, modulacao.valor = 0, modulacao.canal = 1

// Volume no máximo, canal 1
MidiCC volume(7, 127, 1);
// volume.controlador = 7, volume.valor = 127, volume.canal = 1

// Pan centralizado (64 = centro)
MidiCC pan(10, 64);
// pan.controlador = 10, pan.valor = 64, pan.canal = 1

// Sustain ligado (valor >= 64 = on)
MidiCC sustainOn(64, 127);

// Sustain desligado (valor < 64 = off)
MidiCC sustainOff(64, 0);
```

---

### MidiEngine

Classe principal para envio de mensagens MIDI via USB e DIN (Serial1 a 31250 baud). Todas as mensagens são enviadas simultaneamente por ambas as interfaces.

**Incluir:**

```cpp
#include "midi/MidiEngine.h"
```

#### `void begin()`

Inicializa as interfaces USB MIDI e MIDI DIN (Serial1 a 31250 baud). Deve ser chamado uma vez no `setup()`.

```cpp
MidiEngine engine;

void setup() {
    engine.begin();
}
```

#### `void sendNoteOn(const MidiNote& note)`

Envia uma mensagem Note On (nota ligada).

```cpp
MidiEngine engine;
MidiNote nota(60, 100, 1);

engine.sendNoteOn(nota);  // Liga a nota 60 com velocidade 100 no canal 1
```

#### `void sendNoteOff(const MidiNote& note)`

Envia uma mensagem Note Off (nota desligada). A velocidade enviada é sempre 0.

```cpp
engine.sendNoteOff(nota);  // Desliga a nota 60 no canal 1
```

#### `void sendNoteOnOff(const MidiNote& note, uint16_t duracaoMs)`

Envia Note On, aguarda `duracaoMs` milissegundos, e envia Note Off. **Bloqueante** — usa `delay()` internamente.

```cpp
// Toca a nota por 200ms e desliga automaticamente
engine.sendNoteOnOff(nota, 200);
```

#### `void sendCC(const MidiCC& cc)`

Envia uma mensagem MIDI Control Change.

```cpp
// Enviar modulação com valor 64
MidiCC modulacao(1, 64);
engine.sendCC(modulacao);

// Enviar volume máximo no canal 2
MidiCC volume(7, 127, 2);
engine.sendCC(volume);
```

**Exemplo completo — Notas e CC juntos:**

```cpp
MidiEngine engine;

void setup() {
    engine.begin();
}

void loop() {
    // Toca um acorde Dó maior com volume alto
    MidiCC volume(7, 120);
    engine.sendCC(volume);

    MidiNote c4(MIDI_Notes::C(4));
    MidiNote e4(MIDI_Notes::E(4));
    MidiNote g4(MIDI_Notes::G(4));

    engine.sendNoteOn(c4);
    engine.sendNoteOn(e4);
    engine.sendNoteOn(g4);

    delay(1000);

    engine.sendNoteOff(c4);
    engine.sendNoteOff(e4);
    engine.sendNoteOff(g4);

    delay(500);
}
```

---

## Módulo Button

Arquivos: `src/button/Button.h`, `src/button/Button.cpp`

### ButtonEvent

Enum com os tipos de evento que um botão pode gerar.

| Evento | Descrição |
|--------|-----------|
| `NONE` | Nenhum evento |
| `PRESSED` | Botão pressionado (borda de descida) |
| `RELEASED` | Botão solto (borda de subida) |
| `SINGLE_CLICK` | Clique único (solto antes do tempo de long press) |
| `LONG_PRESS` | Mantido pressionado por 800ms |
| `DOUBLE_CLICK` | Dois cliques rápidos (dentro de 300ms) |

### Button

Classe para leitura de botão físico com debounce, detecção de long press e double click.

**Constantes de tempo:**

| Constante | Valor | Descrição |
|-----------|-------|-----------|
| `DEBOUNCE_MS` | 50ms | Tempo de debounce |
| `LONG_PRESS_MS` | 800ms | Tempo para detectar long press |
| `DOUBLE_CLICK_MS` | 300ms | Janela para double click |

#### Construtor

```cpp
Button(uint8_t pin, bool activeLow = true)
```

- `pin`: GPIO do botão
- `activeLow`: `true` se o botão usa pull-up (liga ao GND quando pressionado)

#### `void begin()`

Configura o pino como entrada. Chamar no `setup()`.

#### `ButtonEvent update()`

Lê o estado do botão e retorna o evento atual. Chamar no `loop()` a cada iteração.

#### `bool isHeld() const`

Retorna `true` enquanto o botão está pressionado.

#### `uint32_t heldDuration() const`

Retorna há quantos milissegundos o botão está pressionado. Retorna 0 se não está pressionado.

**Exemplo:**

```cpp
#include "button/Button.h"

Button botao(8, true);  // GPIO 8, pull-up (ativo em LOW)

void setup() {
    botao.begin();
}

void loop() {
    ButtonEvent evento = botao.update();

    switch (evento) {
        case ButtonEvent::PRESSED:
            Serial.println("Pressionado!");
            break;
        case ButtonEvent::RELEASED:
            Serial.println("Solto!");
            break;
        case ButtonEvent::SINGLE_CLICK:
            Serial.println("Clique simples");
            break;
        case ButtonEvent::LONG_PRESS:
            Serial.println("Pressionamento longo");
            break;
        case ButtonEvent::DOUBLE_CLICK:
            Serial.println("Clique duplo");
            break;
        default:
            break;
    }

    // Verificar se está sendo segurado
    if (botao.isHeld()) {
        Serial.print("Segurado por ");
        Serial.print(botao.heldDuration());
        Serial.println("ms");
    }
}
```

---

## Módulo UI

### NavInput

Enum que identifica qual botão físico foi pressionado. Usado pelas Screens para processar navegação.

**Incluir:**

```cpp
#include "ui/NavInput.h"
```

| Valor | Descrição |
|-------|-----------|
| `NONE` | Nenhum botão |
| `UP` | Botão de cima — sobe na lista / incrementa valor |
| `DOWN` | Botão de baixo — desce na lista / decrementa valor |
| `SELECT` | Botão de seleção — confirma / entra |
| `LONG_UP` | Botão de cima mantido — incremento rápido (+5 em valores, +2/+3 em ranges pequenos) |
| `LONG_DOWN` | Botão de baixo mantido — decremento rápido (-5 em valores, -2/-3 em ranges pequenos) |

---

### OledApp

Fachada principal do framework de UI OLED. Ponto de entrada para o desenvolvedor.

**Incluir:**

```cpp
#include "ui/OledApp.h"
```

#### `bool begin(uint8_t i2cAddress = 0x3C)`

Inicializa o display SSD1306 via I2C. Retorna `true` se bem-sucedido.

#### `void showSplash(const char* nome, const char* versao, uint16_t duracaoMs = 1500)`

Exibe uma tela de splash com o nome do produto e versão do firmware, centralizado no display. Bloqueia por `duracaoMs` milissegundos. Chamar logo após `begin()` no `setup()`.

```cpp
app.begin(0x3C);
app.showSplash("MIDI Ctrl", "v1.0.0");  // Exibe por 1.5s
```

#### `void showSaveConfirm()`

Feedback visual rápido de "salvo" — inverte o display por 150ms e volta ao normal. Chamar antes de `router.pop()` nas telas de configuração.

```cpp
_storage->setCanalMidi(_canal);
_app->showSaveConfirm();  // Flash visual
_app->getRouter().pop();
```

#### `void update()`

Loop principal — consulta botões, encaminha eventos e redesenha se necessário. Chamar no `loop()`. Roda a ~30 FPS (intervalo de 33ms entre frames).

#### `void setButtonUp(App::Button* btn)`

Registra o botão de navegação para cima.

#### `void setButtonDown(App::Button* btn)`

Registra o botão de navegação para baixo.

#### `void setButtonSelect(App::Button* btn)`

Registra o botão de seleção/confirmação.

#### `Router& getRouter()`

Retorna referência ao Router interno para navegação de telas.

#### `MidiActivityComponent& getMidiActivity()`

Retorna referência ao indicador de atividade MIDI (renderizado automaticamente em todas as telas).

**Exemplo:**

```cpp
#include "ui/OledApp.h"
#include "button/Button.h"

OledApp app;
App::Button btnUp(11, true);
App::Button btnDown(12, true);
App::Button btnSelect(13, true);

void setup() {
    if (!app.begin(0x3C)) {
        Serial.println("Falha no display!");
    }
    btnUp.begin();
    btnDown.begin();
    btnSelect.begin();
    app.setButtonUp(&btnUp);
    app.setButtonDown(&btnDown);
    app.setButtonSelect(&btnSelect);
}

void loop() {
    app.update();  // Lê botões, processa eventos, redesenha
}
```

---

### Router

Gerencia navegação por pilha estática de Screens (LIFO). Máximo de 8 telas na pilha.

**Incluir:**

```cpp
#include "ui/Router.h"
```

#### `void push(Screen* screen)`

Empilha uma nova tela no topo. Chama `onUnmount()` na tela anterior e `onMount()` na nova. Ignora se a pilha está cheia ou o ponteiro é `nullptr`.

#### `void pop()`

Remove a tela do topo e volta para a anterior. Chama `onUnmount()` na tela removida e `onMount()` na que ficou no topo. Ignora se há apenas 1 tela na pilha.

#### `void navigateTo(Screen* screen)`

Substitui toda a pilha por uma única tela. Útil para "resetar" a navegação.

#### `Screen* currentScreen() const`

Retorna ponteiro para a tela ativa (topo da pilha), ou `nullptr` se a pilha está vazia.

#### `void handleInput(NavInput input)`

Encaminha o evento de navegação para a tela ativa.

**Exemplo — Navegação entre telas:**

```cpp
#include "ui/OledApp.h"
#include "ui/Screen.h"
#include "ui/components/TextComponent.h"

class TelaInicial : public Screen {
public:
    TelaInicial(OledApp* app)
        : _app(app)
        , _texto(0, 20, "Tela Inicial", 2)
    {
        addChild(&_texto);
    }

    void handleInput(NavInput input) override {
        if (input == NavInput::SELECT) {
            _app->getRouter().push(&telaConfig);
        }
    }

private:
    OledApp* _app;
    TextComponent _texto;
};

class TelaConfig : public Screen {
public:
    TelaConfig(OledApp* app)
        : _app(app)
        , _texto(0, 20, "Configuracoes", 2)
    {
        addChild(&_texto);
    }

    void handleInput(NavInput input) override {
        if (input == NavInput::SELECT) {
            _app->getRouter().pop();  // Volta para a tela anterior
        }
    }

private:
    OledApp* _app;
    TextComponent _texto;
};
```

---

### Screen

Classe base para todas as telas. Cada tela herda de `Screen` e compõe `UIComponent`s como filhos.

**Incluir:**

```cpp
#include "ui/Screen.h"
```

#### `virtual void onMount()`

Chamado pelo Router ao entrar na tela (antes do primeiro render). Sobrescreva para inicializar recursos.

#### `virtual void onUnmount()`

Chamado pelo Router ao sair da tela. Sobrescreva para liberar recursos.

#### `virtual void render(Adafruit_SSD1306& display)`

Desenha a tela. A implementação base itera sobre os filhos e chama `render()` de cada um. Sobrescreva para lógica customizada.

#### `virtual void handleInput(NavInput input)`

Processa um evento de navegação. Sobrescreva para reagir a interações.

#### `void addChild(UIComponent* child)`

Adiciona um componente filho à lista de renderização. Máximo de 8 filhos. Ignora silenciosamente se o limite foi atingido.

#### `void markDirty()`

Marca a tela como suja — o próximo `update()` irá redesenhá-la. Chame sempre que alterar algo visual.

#### `bool isDirty() const`

Retorna `true` se a tela precisa ser redesenhada.

#### `void clearDirty()`

Limpa o dirty flag após o redesenho (chamado automaticamente pelo `OledApp`).

**Exemplo — Tela customizada:**

```cpp
class MinhaTela : public Screen {
public:
    MinhaTela()
        : _titulo(0, 0, "Minha Tela", 2)
        , _info(0, 30, "Info aqui", 1)
    {
        addChild(&_titulo);
        addChild(&_info);
    }

    void onMount() override {
        Serial.println("Tela montada!");
    }

    void onUnmount() override {
        Serial.println("Tela desmontada!");
    }

    void handleInput(NavInput input) override {
        if (input == NavInput::SELECT) {
            _info.setText("Botao pressionado!");
            markDirty();  // Solicita redesenho
        }
    }

private:
    TextComponent _titulo;
    TextComponent _info;
};
```

---

### State\<T\>

Template de estado reativo. Quando o valor muda, marca a Screen dona como dirty automaticamente.

**Incluir:**

```cpp
#include "ui/State.h"
```

#### Construtor

```cpp
State(Screen* owner, T initialValue)
```

#### `const T& get() const`

Retorna o valor atual (somente leitura).

#### `void set(const T& newValue)`

Atualiza o valor. Marca a Screen como dirty apenas se o valor realmente mudou.

**Exemplo:**

```cpp
#include "ui/State.h"

class TelaContador : public Screen {
public:
    TelaContador()
        : _contador(this, 0)  // Estado reativo, valor inicial 0
        , _texto(0, 20, "0", 2)
    {
        addChild(&_texto);
    }

    void handleInput(NavInput input) override {
        if (input == NavInput::UP) {
            _contador.set(_contador.get() + 1);
            // markDirty() é chamado automaticamente pelo State!

            // Atualizar o texto (precisa de buffer estático)
            snprintf(_buf, sizeof(_buf), "%d", _contador.get());
            _texto.setText(_buf);
        }
    }

private:
    State<int> _contador;
    TextComponent _texto;
    char _buf[16];
};
```

---

### UIComponent

Classe base abstrata para todos os componentes visuais.

**Incluir:**

```cpp
#include "ui/UIComponent.h"
```

#### `virtual void render(Adafruit_SSD1306& display) = 0`

Desenha o componente no buffer do display. Método puro — deve ser implementado por subclasses.

#### `virtual bool handleInput(ButtonEvent event)`

Processa um evento de botão. Retorna `true` se o evento foi consumido. Implementação padrão retorna `false`.

---

### TextComponent

Componente de texto. Renderiza uma string na posição (x, y) com tamanho de fonte configurável. Trunca automaticamente texto que ultrapassa a largura do display (128px).

**Incluir:**

```cpp
#include "ui/components/TextComponent.h"
```

#### Construtor

```cpp
TextComponent(int16_t x, int16_t y, const char* text,
              uint8_t fontSize = 1, uint16_t color = SSD1306_WHITE)
```

- `x`, `y`: posição em pixels
- `text`: texto a renderizar (`nullptr` tratado como string vazia)
- `fontSize`: 1, 2 ou 3 (cada unidade = 8px de altura)
- `color`: `SSD1306_WHITE` (1) ou `SSD1306_BLACK` (0)

#### `void setText(const char* text)`

Atualiza o texto exibido. O ponteiro deve ter lifetime maior que o componente.

#### `void setPosition(int16_t x, int16_t y)`

Atualiza a posição do componente.

**Exemplo:**

```cpp
#include "ui/components/TextComponent.h"

// Texto grande centralizado
TextComponent titulo(10, 0, "MIDI Ctrl", 2);

// Texto pequeno
TextComponent rodape(0, 56, "v1.0", 1);

// Atualizar texto dinamicamente
titulo.setText("Novo Titulo");
titulo.setPosition(5, 10);
```

---

### IconComponent

Componente de ícone (bitmap monocromático). Renderiza um bitmap em PROGMEM na posição (x, y).

**Incluir:**

```cpp
#include "ui/components/IconComponent.h"
```

#### Construtor

```cpp
IconComponent(int16_t x, int16_t y,
              const uint8_t* bitmap, uint8_t w, uint8_t h,
              uint16_t color = SSD1306_WHITE)
```

- `bitmap`: ponteiro para dados em PROGMEM (`nullptr` = nada renderizado)
- `w`, `h`: dimensões do bitmap em pixels

**Exemplo:**

```cpp
#include "ui/components/IconComponent.h"

// Bitmap 16x16 de um coração (exemplo)
static const uint8_t PROGMEM iconeCoracao[] = {
    0x00, 0x00, 0x36, 0x6C, 0x7F, 0xFE, 0x7F, 0xFE,
    0x3F, 0xFC, 0x1F, 0xF8, 0x0F, 0xF0, 0x07, 0xE0,
    0x03, 0xC0, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

IconComponent coracao(56, 24, iconeCoracao, 16, 16);
```

---

### ListComponent

Componente de lista selecionável com scroll automático e indicador visual (inversão) no item selecionado.

**Incluir:**

```cpp
#include "ui/components/ListComponent.h"
```

#### Construtor

```cpp
ListComponent(int16_t x, int16_t y, int16_t w, int16_t h,
              uint8_t fontSize = 1)
```

- `x`, `y`: posição em pixels
- `w`, `h`: dimensões da área da lista em pixels
- `fontSize`: 1, 2 ou 3

#### `void setItems(const char** items, uint8_t count)`

Define os itens da lista. Os ponteiros devem ter lifetime maior que o componente.

#### `uint8_t getSelectedIndex() const`

Retorna o índice do item selecionado.

#### `void setUpButton(ButtonEvent upEvent)`

Define qual evento de botão navega para cima.

#### `void setDownButton(ButtonEvent downEvent)`

Define qual evento de botão navega para baixo.

#### `void onSelectionChanged(OnSelectionChanged callback)`

Registra callback chamado quando a seleção muda. Assinatura: `void callback(uint8_t index)`.

#### `bool handleInput(ButtonEvent event)`

Processa navegação. Retorna `true` se o evento foi consumido.

**Exemplo:**

```cpp
#include "ui/components/ListComponent.h"

static const char* opcoes[] = {
    "Volume",
    "Pan",
    "Modulacao",
    "Reverb",
    "Chorus"
};

ListComponent lista(0, 0, 128, 64, 1);

void setup() {
    lista.setItems(opcoes, 5);
    lista.setUpButton(ButtonEvent::SINGLE_CLICK);
    lista.setDownButton(ButtonEvent::DOUBLE_CLICK);

    lista.onSelectionChanged([](uint8_t index) {
        Serial.print("Selecionado: ");
        Serial.println(opcoes[index]);
    });
}
```

---

### ProgressBarComponent

Componente de barra de progresso horizontal com borda de 1px e preenchimento proporcional.

**Incluir:**

```cpp
#include "ui/components/ProgressBarComponent.h"
```

#### Construtor

```cpp
ProgressBarComponent(int16_t x, int16_t y, int16_t w, int16_t h)
```

#### `void setValue(uint8_t value)`

Define o valor da barra (0-100). Valores acima de 100 são limitados a 100.

#### `uint8_t getValue() const`

Retorna o valor atual.

**Exemplo:**

```cpp
#include "ui/components/ProgressBarComponent.h"

ProgressBarComponent barra(10, 50, 108, 10);

barra.setValue(75);  // 75% preenchido

// Usar com MidiCC para mostrar valor de um controlador
MidiCC volume(7, 100);
engine.sendCC(volume);
barra.setValue((volume.valor * 100) / 127);  // Converte 0-127 para 0-100
```

---

### MidiActivityComponent

Componente visual que indica atividade MIDI. Exibe um pequeno indicador no canto superior direito do display quando uma mensagem MIDI é enviada.

**Incluir:**

```cpp
#include "ui/components/MidiActivityComponent.h"
```

#### Construtor

```cpp
MidiActivityComponent(int16_t x, int16_t y, uint8_t size)
```

#### `void trigger()`

Ativa o indicador visual. Chamado automaticamente pelo callback `onActivity` do MidiEngine.

#### `bool isActive() const`

Retorna `true` se o indicador está visível (dentro da janela de tempo).

O `OledApp` renderiza este componente automaticamente em todas as telas.

---

## Módulo Storage

Arquivo: `src/storage/Storage.h`, `src/storage/Storage.cpp`

Persistência de configurações na flash do ESP32 via NVS (Non-Volatile Storage). Sobrevive a reinicializações.

#### `void begin()`

Inicializa o NVS e carrega configurações salvas. Chamar no `setup()`.

#### Configurações locais

| Método | Descrição |
|--------|-----------|
| `getCanalMidi()` / `setCanalMidi(canal)` | Canal MIDI (1-16) |
| `getOitava()` / `setOitava(oitava)` | Oitava do teclado (0-8) |
| `getVelocidade()` / `setVelocidade(vel)` | Velocidade das notas (0-127) |
| `isTecladoHabilitado()` / `setTecladoHabilitado(hab)` | Teclado on/off |
| `getControladorCC(indice)` / `setControladorCC(indice, cc)` | CC de cada controle local |
| `isControleHabilitado(indice)` / `setControleHabilitado(indice, hab)` | Habilitar/desabilitar controle local |

#### `void factoryReset()`

Restaura todas as configurações para os valores padrão de fábrica. Limpa o NVS e recarrega defaults do HardwareMap. Canal volta para 1, oitava para 4, velocidade para 100, todos os controles habilitados com CC padrão.

```cpp
_storage->factoryReset();  // Tudo volta ao padrão
```

#### Configurações remotas (módulos I2C)

| Método | Descrição |
|--------|-----------|
| `getRemoteCC(addr, idx)` / `setRemoteCC(addr, idx, cc)` | CC de controle remoto |
| `isRemoteEnabled(addr, idx)` / `setRemoteEnabled(addr, idx, hab)` | Habilitar/desabilitar controle remoto |
| `loadRemoteConfig(addr, idx, cc, enabled)` | Carrega config salva de módulo redescoberto |

Chaves NVS para remotos: `rccAAII` (CC) e `renAAII` (habilitado), onde AA = endereço hex, II = índice hex.

---

## Módulo Hardware

Arquivos: `src/hardware/CCActivityInfo.h`, `src/hardware/ControlReader.h/.cpp`, `src/hardware/HardwareMap.h`, `src/hardware/UnifiedControlList.h/.cpp`

### CCActivityInfo

Struct com dados completos do último CC enviado. Usada para feedback visual na PerformanceScreen.

**Incluir:**

```cpp
#include "hardware/CCActivityInfo.h"
```

**Campos:**

| Campo | Tipo | Descrição |
|-------|------|-----------|
| `label` | `const char*` | Nome do controle (ex: "Pot Volume", "Pot1") |
| `cc` | `uint8_t` | Número do CC enviado (0-127) |
| `valor` | `uint8_t` | Valor do CC (0-127) |
| `canal` | `uint8_t` | Canal MIDI (1-16) |
| `isRemoto` | `bool` | `true` se vem de módulo externo |
| `moduleAddress` | `uint8_t` | Endereço I2C (só para remotos, ex: 0x20) |

**Callback:**

```cpp
using CCActivityCallback = void (*)(const CCActivityInfo& info);
```

### ControlReader

Leitura automática de controles analógicos (potenciômetros e sensores). Lê GPIOs, converte para 0-127, aplica zona morta e envia CC via MidiEngine.

**Incluir:**

```cpp
#include "hardware/ControlReader.h"
```

| Constante | Valor | Descrição |
|-----------|-------|-----------|
| `ZONA_MORTA` | 1 | Diferença mínima para enviar CC |
| `INTERVALO_MS` | 10 | Intervalo entre leituras (ms) |

#### `void begin()`

Inicializa pinos analógicos. Chamar no `setup()`.

#### `void update()`

Lê todos os controles e envia CC se necessário. Chamar no `loop()`.

#### `void onCCActivity(CCActivityCallback callback)`

Registra callback chamado a cada envio de CC com dados completos do controle. Usado para alimentar o monitor de CC na PerformanceScreen.

```cpp
void onCCActivity(const CCActivityInfo& info) {
    perfScreen->atualizarCCInfo(info);
}

controlReader->onCCActivity(onCCActivity);
```

---

## Módulo I2C

Arquivos: `src/i2c/`

### I2CBus

Interface abstrata para comunicação I2C. Implementações: `WireI2CBus` (hardware real) e `MockI2CBus` (testes).

| Método | Descrição |
|--------|-----------|
| `probe(address)` | Verifica se há dispositivo no endereço |
| `write(address, data, length)` | Envia dados para o dispositivo |
| `requestFrom(address, buffer, length)` | Solicita dados do dispositivo |

### I2CScanner

Descobre e gerencia módulos externos no barramento I2C (endereços 0x20-0x27).

| Constante | Valor | Descrição |
|-----------|-------|-----------|
| `MAX_MODULES` | 8 | Máximo de módulos simultâneos |
| `MAX_FAIL_COUNT` | 3 | Falhas consecutivas para desconexão |
| `RESCAN_INTERVAL_MS` | 5000 | Intervalo entre rescans (ms) |

| Método | Descrição |
|--------|-----------|
| `scan()` | Varredura completa do barramento |
| `periodicScan()` | Rescan periódico (chamar no loop) |
| `readValues(moduleIndex, values, maxLen)` | Lê valores atuais dos controles de um módulo |
| `getModuleCount()` | Número de módulos conectados |
| `getModule(index)` | Informações de um módulo (ModuleInfo) |

### ModuleDescriptor

Protocolo binário para comunicação com módulos escravos.

| Comando | Código | Descrição |
|---------|--------|-----------|
| `CMD_DESCRIPTOR` | `0x01` | Solicita descritor do módulo |
| `CMD_READ_VALUES` | `0x02` | Solicita valores dos controles |

Formato do descritor: `[numControles:1][tipo:1][label:12][valor:1] × N`

### UnifiedControlList

Fachada que combina controles locais (HardwareMap) e remotos (I2CScanner) numa lista unificada. Índices 0..N-1 = locais, N..M = remotos. Máximo de 32 controles.

| Método | Descrição |
|--------|-----------|
| `rebuild()` | Reconstrói a lista a partir do HardwareMap + módulos |
| `getNumControles()` | Total de controles (locais + remotos) |
| `getNumLocais()` | Número de controles locais |
| `isRemoto(index)` | Verifica se o controle é remoto |
| `getRemoteInfo(index, addr, ctrlIdx)` | Endereço I2C e índice no módulo |

---

## Telas

Todas as telas herdam de `Screen` e seguem o padrão: `onMount()` inicializa, `render()` desenha, `handleInput(NavInput)` processa navegação.

| Tela | Arquivo | Descrição |
|------|---------|-----------|
| `MenuScreen` | `screens/MenuScreen` | Menu principal com status no rodapé (Ch, Oit, Vel) |
| `PerformanceScreen` | `screens/PerformanceScreen` | Monitor CC em tempo real (label, valor, módulo) + ajuste de oitava |
| `ConfigScreen` | `screens/ConfigScreen` | Menu de configurações (CC Map, Canal, Oitava, Velocidade, Restaurar) |
| `CCMapScreen` | `screens/CCMapScreen` | Mapear CC para cada controle (local e remoto), com aceleração |
| `CanalScreen` | `screens/CanalScreen` | Selecionar canal MIDI (1-16), com aceleração |
| `OitavaScreen` | `screens/OitavaScreen` | Selecionar oitava (0-8), com aceleração |
| `VelocidadeScreen` | `screens/VelocidadeScreen` | Selecionar velocidade MIDI (1-127), com aceleração |
| `TecladoScreen` | `screens/TecladoScreen` | Habilitar/desabilitar teclado (ON/OFF) — escondida do menu |
| `SobreScreen` | `screens/SobreScreen` | Versão, canal MIDI, número de controles |

---

## Exemplo Completo

Um controlador MIDI com display OLED, 3 botões de navegação, notas e CC:

```cpp
#include "midi/MidiEngine.h"
#include "midi/MidiCC.h"
#include "midi/MidiNote.h"
#include "config.h"
#include "button/Button.h"
#include "ui/OledApp.h"
#include "ui/Screen.h"
#include "ui/components/TextComponent.h"
#include "ui/components/ProgressBarComponent.h"

MidiEngine engine;
OledApp app;
App::Button btnUp(11, true);
App::Button btnDown(12, true);
App::Button btnSelect(13, true);

class HomeScreen : public Screen {
public:
    HomeScreen()
        : _titulo(0, 0, "MIDI Controller", 2)
        , _status(0, 30, "Pronto", 1)
        , _barra(0, 50, 128, 10)
        , _ccValor(0)
    {
        addChild(&_titulo);
        addChild(&_status);
        addChild(&_barra);
    }

    void handleInput(NavInput input) override {
        if (input == NavInput::UP) {
            // Incrementa modulação
            _ccValor = (_ccValor + 16 > 127) ? 127 : _ccValor + 16;
            MidiCC modulacao(1, _ccValor);
            engine.sendCC(modulacao);
            _barra.setValue((_ccValor * 100) / 127);
            markDirty();
        }
        else if (input == NavInput::DOWN) {
            // Decrementa modulação
            _ccValor = (_ccValor < 16) ? 0 : _ccValor - 16;
            MidiCC modulacao(1, _ccValor);
            engine.sendCC(modulacao);
            _barra.setValue((_ccValor * 100) / 127);
            markDirty();
        }
        else if (input == NavInput::SELECT) {
            // Toca nota Dó por 200ms
            MidiNote nota(MIDI_Notes::C(4));
            engine.sendNoteOnOff(nota, 200);
            _status.setText("Nota!");
            markDirty();
        }
    }

private:
    TextComponent _titulo;
    TextComponent _status;
    ProgressBarComponent _barra;
    uint8_t _ccValor;
};

HomeScreen homeScreen;

void setup() {
    Serial.begin(115200);
    engine.begin();

    if (!app.begin(DISPLAY_I2C_ADDRESS)) {
        Serial.println("Falha no display!");
    }

    btnUp.begin();
    btnDown.begin();
    btnSelect.begin();
    app.setButtonUp(&btnUp);
    app.setButtonDown(&btnDown);
    app.setButtonSelect(&btnSelect);
    app.getRouter().push(&homeScreen);
}

void loop() {
    app.update();
}
```
