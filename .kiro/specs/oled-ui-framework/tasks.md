# Plano de Implementação: OLED UI Framework

## Visão Geral

Implementação incremental do framework de UI para display OLED SSD1306 128×64 no ESP32-S3. Cada tarefa constrói sobre as anteriores, começando pelas interfaces base e terminando com a integração completa no `main.cpp`. O código é C++ para PlatformIO/Arduino, usando a biblioteca Adafruit SSD1306/GFX como backend de renderização.

## Tarefas

- [x] 1. Configurar dependências e estrutura do projeto
  - Adicionar `adafruit/Adafruit SSD1306` e `adafruit/Adafruit GFX Library` ao `lib_deps` em `platformio.ini`
  - Criar a estrutura de diretórios `src/ui/` e `src/ui/components/`
  - Adicionar constantes de configuração do display em `src/config.h` (endereço I2C `0x3C`, largura 128, altura 64)
  - _Requisitos: 1.1, 1.3, 11.3_

- [x] 2. Implementar classes base (UIComponent e Screen)
  - [x] 2.1 Criar `UIComponent` (classe base abstrata)
    - Criar `src/ui/UIComponent.h` com métodos virtuais `render(Adafruit_SSD1306&)` e `handleInput(ButtonEvent)` conforme o design
    - _Requisitos: 2.1, 2.2_

  - [x] 2.2 Criar `Screen` (classe base abstrata)
    - Criar `src/ui/Screen.h` e `src/ui/Screen.cpp` com ciclo de vida (`onMount()`, `onUnmount()`, `render()`, `handleInput()`) e dirty flag (`markDirty()`, `isDirty()`, `clearDirty()`)
    - O dirty flag deve iniciar como `true` para garantir o primeiro render
    - _Requisitos: 3.1, 3.2, 3.3, 4.1, 4.4_

  - [x] 2.3 Criar `State<T>` (template de estado reativo)
    - Criar `src/ui/State.h` com template que encapsula valor tipado, referência à Screen dona, e `set()` que marca dirty apenas quando o valor muda (`!=`)
    - Suportar tipos primitivos (`int`, `bool`, `float`) e structs do usuário
    - _Requisitos: 3.1, 3.4_

  - [x] 2.4 Escrever teste de propriedade para State reativo
    - **Propriedade 3: Reatividade do State**
    - Gerar valores aleatórios de `int`, `bool`, `float`; verificar que `set(b)` com `b != a` marca dirty, e `set(a)` com mesmo valor não marca
    - **Valida: Requisito 3.1**

- [x] 3. Implementar Router (navegação por pilha)
  - [x] 3.1 Criar `Router`
    - Criar `src/ui/Router.h` e `src/ui/Router.cpp` com pilha estática de ponteiros (`MAX_STACK_SIZE = 8`)
    - Implementar `push()`, `pop()`, `navigateTo()`, `currentScreen()`, `handleInput()`
    - `push()` deve chamar `onUnmount()` da Screen anterior e `onMount()` da nova
    - `pop()` deve chamar `onUnmount()` da atual e `onMount()` da que ficou no topo
    - `pop()` com pilha de tamanho 1 deve ser ignorado silenciosamente
    - `push()` com pilha cheia deve ser ignorado silenciosamente
    - `navigateTo()` deve substituir toda a pilha por uma única Screen
    - `currentScreen()` retorna `nullptr` se a pilha estiver vazia
    - _Requisitos: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6_

  - [x] 3.2 Escrever teste de propriedade para modelo de pilha do Router
    - **Propriedade 1: Modelo de pilha do Router**
    - Gerar sequências aleatórias de `push`/`pop` com Screens mock; verificar que `currentScreen()` retorna o topo da pilha LIFO e que `onMount()`/`onUnmount()` são invocados na ordem correta
    - **Valida: Requisitos 5.1, 5.2, 5.3, 5.4**

  - [x] 3.3 Escrever teste de propriedade para navigateTo
    - **Propriedade 2: navigateTo substitui a pilha**
    - Gerar pilhas aleatórias + `navigateTo(screen)`; verificar que a pilha resultante tem tamanho 1 e `currentScreen()` retorna a Screen correta
    - **Valida: Requisito 5.5**

  - [x] 3.4 Escrever teste de propriedade para encaminhamento de eventos
    - **Propriedade 4: Encaminhamento de eventos**
    - Gerar `ButtonEvent`s aleatórios (excluindo `NONE`); verificar que o Router invoca `handleInput()` da Screen ativa com exatamente esse evento
    - **Valida: Requisito 6.1**

- [x] 4. Checkpoint — Verificar classes base e Router
  - Garantir que todos os testes passam, perguntar ao usuário se houver dúvidas.

- [x] 5. Implementar OledApp (fachada principal)
  - [x] 5.1 Criar `OledApp`
    - Criar `src/ui/OledApp.h` e `src/ui/OledApp.cpp`
    - Implementar `begin(uint8_t i2cAddress)` que inicializa `Adafruit_SSD1306`, retorna `false` em caso de falha e imprime erro via `Serial`
    - Implementar `addButton(Button*)` com array estático de até `MAX_BUTTONS = 4`
    - Implementar `getRouter()` que retorna referência ao Router interno
    - _Requisitos: 1.1, 1.2, 1.3, 6.2, 6.3_

  - [x] 5.2 Implementar `OledApp::update()` (RenderLoop)
    - Implementar o ciclo: (a) chamar `update()` em cada Button registrado, (b) encaminhar eventos `!= NONE` ao Router, (c) verificar dirty flag da Screen ativa, (d) se dirty: `clearDisplay()` → `render()` da Screen → `display()`
    - Implementar rate limiting de 30 FPS (`FRAME_INTERVAL_MS = 33`)
    - _Requisitos: 7.1, 7.2, 7.3, 6.1_

  - [x] 5.3 Escrever testes unitários para OledApp
    - Testar `begin()` sucesso e falha, `addButton()` com 0 e múltiplos botões, dirty flag dispara redesenho, clean flag pula redesenho, rate limiting 30 FPS
    - _Requisitos: 1.1, 1.2, 1.3, 7.1, 7.2, 7.3_

- [x] 6. Implementar TextComponent
  - [x] 6.1 Criar `TextComponent`
    - Criar `src/ui/components/TextComponent.h` e `src/ui/components/TextComponent.cpp`
    - Implementar `render()` que desenha texto na posição (x, y) com `setCursor()`, `setTextSize()`, `setTextColor()` e `print()`
    - Implementar truncamento: calcular largura do texto via `getTextBounds()` e truncar caracteres que ultrapassem 128px
    - Aceitar Props: posição (x, y), texto, tamanho da fonte (1, 2, 3), cor (SSD1306_WHITE ou SSD1306_BLACK)
    - Tratar `text == nullptr` como string vazia
    - _Requisitos: 9.1, 9.2, 9.3_

  - [x] 6.2 Escrever teste de propriedade para truncamento de texto
    - **Propriedade 6: Truncamento de texto sem overflow**
    - Gerar strings de comprimentos aleatórios e posições (x, y) dentro dos limites; verificar que nenhum pixel é desenhado fora de 128×64
    - **Valida: Requisito 9.3**

- [x] 7. Implementar ProgressBarComponent
  - [x] 7.1 Criar `ProgressBarComponent`
    - Criar `src/ui/components/ProgressBarComponent.h` e `src/ui/components/ProgressBarComponent.cpp`
    - Implementar `render()` que desenha borda de 1px com `drawRect()` e preenchimento proporcional com `fillRect()`
    - Implementar `setValue()` com clamp para [0, 100]
    - Fórmula de preenchimento: `fillWidth = (value * (width - 2)) / 100`
    - _Requisitos: 10.1, 10.2, 10.3_

  - [x] 7.2 Escrever teste de propriedade para preenchimento da barra de progresso
    - **Propriedade 7: Preenchimento proporcional da barra de progresso**
    - Gerar valores 0–100 e dimensões aleatórias; verificar que `fillWidth = (value * innerWidth) / 100` e que valor 0 → fillWidth 0, valor 100 → fillWidth == innerWidth
    - **Valida: Requisito 10.2**

- [x] 8. Implementar IconComponent
  - [x] 8.1 Criar `IconComponent`
    - Criar `src/ui/components/IconComponent.h` e `src/ui/components/IconComponent.cpp`
    - Implementar `render()` que desenha bitmap via `drawBitmap()` na posição (x, y) com dimensões (w, h)
    - Aceitar ponteiro para bitmap em PROGMEM
    - _Requisitos: 2.3_

- [x] 9. Implementar ListComponent
  - [x] 9.1 Criar `ListComponent`
    - Criar `src/ui/components/ListComponent.h` e `src/ui/components/ListComponent.cpp`
    - Implementar `render()` que desenha itens de texto com indicador visual (seta ou inversão) no item selecionado
    - Implementar `handleInput()` que processa eventos de navegação (cima/baixo) e atualiza `selectedIndex`
    - Implementar scroll automático via `scrollToSelected()` para manter o item selecionado visível
    - Implementar `setItems()`, `getSelectedIndex()`, `setUpButton()`, `setDownButton()`, callback `onSelectionChanged`
    - Ignorar eventos de navegação se a lista estiver vazia
    - Clamp de `selectedIndex` no intervalo [0, itemCount - 1]
    - _Requisitos: 8.1, 8.2, 8.3, 8.4, 8.5_

  - [x] 9.2 Escrever teste de propriedade para navegação da lista
    - **Propriedade 5: Navegação da lista**
    - Gerar listas de tamanhos aleatórios (N ≥ 1) e sequências de eventos "cima"/"baixo"; verificar que `selectedIndex` ∈ [0, N-1] e `scrollOffset` mantém o item selecionado na janela visível
    - **Valida: Requisitos 8.2, 8.3, 8.4**

- [x] 10. Checkpoint — Verificar todos os componentes
  - Garantir que todos os testes passam, perguntar ao usuário se houver dúvidas.

- [x] 11. Implementar ordem de renderização e composição de componentes
  - [x] 11.1 Adicionar suporte a componentes filhos na Screen
    - Implementar mecanismo para que uma Screen mantenha uma lista ordenada de `UIComponent*` filhos (array estático)
    - Implementar `render()` base que itera sobre os filhos na ordem de adição e chama `render()` de cada um
    - _Requisitos: 2.4_

  - [x] 11.2 Escrever teste de propriedade para ordem de renderização
    - **Propriedade 8: Ordem de renderização dos filhos**
    - Gerar listas aleatórias de componentes mock; verificar que `render()` da Screen invoca `render()` de cada filho exatamente uma vez, na ordem de adição
    - **Valida: Requisito 2.4**

- [x] 12. Integrar framework no main.cpp
  - [x] 12.1 Atualizar `main.cpp` com o framework
    - Incluir headers do framework (`OledApp.h`, `Screen.h`, componentes)
    - Criar instância de `OledApp` e chamar `begin()` no `setup()`
    - Registrar botões existentes via `addButton()`
    - Criar uma Screen de exemplo (ex: tela Home com TextComponent mostrando nome do instrumento)
    - Fazer `push()` da Screen inicial no Router
    - Chamar `OledApp::update()` no `loop()`
    - _Requisitos: 1.1, 1.3, 7.1, 7.2_

- [x] 13. Checkpoint final — Verificar integração completa
  - Garantir que todos os testes passam e que o projeto compila sem erros, perguntar ao usuário se houver dúvidas.

## Notas

- Tarefas marcadas com `*` são opcionais e podem ser puladas para um MVP mais rápido
- Cada tarefa referencia requisitos específicos para rastreabilidade
- Checkpoints garantem validação incremental
- Testes de propriedade validam propriedades universais de corretude definidas no design
- Testes unitários validam cenários específicos e edge cases
- O framework usa alocação estática para respeitar as restrições de memória do ESP32-S3 (Requisito 11)
