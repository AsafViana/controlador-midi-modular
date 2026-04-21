# Documento de Requisitos — OLED UI Framework

## Introdução

Framework de interface gráfica para display OLED SSD1306 (I2C) no projeto de instrumento musical baseado em ESP32-S3. A abstração segue paradigmas inspirados em React — componentes declarativos, estado reativo e sistema de páginas/navegação — para que um desenvolvedor familiarizado com React consiga criar telas de forma intuitiva. O framework deve integrar-se ao sistema de botões (`Button`) já existente no projeto e operar dentro das restrições de memória e processamento de um microcontrolador.

## Glossário

- **Display**: Instância do hardware SSD1306 OLED 128×64 conectado via I2C, gerenciado pela biblioteca Adafruit SSD1306.
- **Renderer**: Módulo responsável por enviar o buffer de pixels ao Display físico.
- **Component**: Unidade visual reutilizável (texto, ícone, barra de progresso, lista, etc.) que declara o que deve ser desenhado.
- **Screen**: Composição de Components que representa uma tela completa visível ao usuário.
- **Router**: Módulo que gerencia a pilha de Screens e controla a navegação entre elas.
- **State**: Dados mutáveis associados a uma Screen ou Component que, ao serem alterados, disparam uma nova renderização.
- **Props**: Dados imutáveis passados de um Component pai para um Component filho no momento da criação.
- **Hook**: Função utilitária que encapsula lógica reutilizável de State e efeitos colaterais, inspirada nos hooks do React.
- **InputEvent**: Evento gerado pelo sistema de botões (`ButtonEvent`) já existente no projeto, consumido pelo Router ou pela Screen ativa.
- **RenderLoop**: Ciclo principal que verifica mudanças de State e, quando necessário, redesenha a Screen ativa no Display.

## Requisitos

### Requisito 1: Inicialização do Display

**User Story:** Como desenvolvedor, quero inicializar o display OLED com uma única chamada, para que a configuração de hardware fique encapsulada.

#### Critérios de Aceitação

1. WHEN `begin()` é chamado, THE Renderer SHALL inicializar a comunicação I2C e configurar o Display SSD1306 128×64.
2. IF a inicialização do Display falhar, THEN THE Renderer SHALL retornar um código de erro e registrar uma mensagem via `Serial`.
3. WHEN `begin()` é concluído com sucesso, THE Renderer SHALL limpar o Display e deixá-lo pronto para receber comandos de desenho.

---

### Requisito 2: Sistema de Componentes Declarativos

**User Story:** Como desenvolvedor React, quero criar componentes visuais de forma declarativa, para que eu possa compor interfaces OLED da mesma forma que componho JSX.

#### Critérios de Aceitação

1. THE Component SHALL expor um método `render()` que desenha o Component no buffer do Display sem enviá-lo à tela.
2. WHEN um Component é criado, THE Component SHALL aceitar Props que definam posição (x, y), dimensões e conteúdo.
3. THE Framework SHALL fornecer Components embutidos para: texto (`TextComponent`), ícone bitmap (`IconComponent`), barra de progresso (`ProgressBarComponent`) e lista selecionável (`ListComponent`).
4. WHEN um Component pai contém Components filhos, THE Component pai SHALL invocar `render()` de cada filho na ordem de adição.

---

### Requisito 3: Gerenciamento de Estado Reativo

**User Story:** Como desenvolvedor, quero que a tela se atualize automaticamente quando o estado mudar, para que eu não precise gerenciar redesenhos manualmente.

#### Critérios de Aceitação

1. WHEN o valor de um State é alterado via `setState()`, THE RenderLoop SHALL marcar a Screen ativa como "suja" (dirty flag).
2. WHILE a Screen ativa estiver marcada como suja, THE RenderLoop SHALL redesenhar a Screen na próxima iteração do `loop()`.
3. WHILE a Screen ativa NÃO estiver marcada como suja, THE RenderLoop SHALL pular o redesenho para economizar ciclos de CPU.
4. THE State SHALL ser tipado via templates C++ para suportar tipos primitivos (`int`, `bool`, `float`) e structs definidos pelo usuário.

---

### Requisito 4: Sistema de Telas (Screens)

**User Story:** Como desenvolvedor, quero organizar a interface em telas independentes, para que cada funcionalidade do instrumento tenha sua própria página.

#### Critérios de Aceitação

1. THE Screen SHALL expor métodos de ciclo de vida: `onMount()` (ao entrar na tela), `onUnmount()` (ao sair da tela) e `render()`.
2. WHEN uma Screen é montada pelo Router, THE Router SHALL invocar `onMount()` antes do primeiro `render()`.
3. WHEN uma Screen é desmontada pelo Router, THE Router SHALL invocar `onUnmount()` antes de montar a próxima Screen.
4. THE Screen SHALL receber InputEvents do Router e processá-los em um método `handleInput(InputEvent)`.

---

### Requisito 5: Navegação entre Telas (Router)

**User Story:** Como desenvolvedor, quero navegar entre telas com comandos simples como `push` e `pop`, para que a navegação funcione como uma pilha de rotas.

#### Critérios de Aceitação

1. THE Router SHALL manter uma pilha (stack) de Screens, onde a Screen no topo é a Screen ativa.
2. WHEN `push(screen)` é chamado, THE Router SHALL empilhar a nova Screen, invocar `onUnmount()` da Screen anterior e `onMount()` da nova Screen.
3. WHEN `pop()` é chamado, THE Router SHALL desempilhar a Screen ativa, invocar `onUnmount()` dela e `onMount()` da Screen que ficou no topo.
4. IF `pop()` é chamado e a pilha contém apenas uma Screen, THEN THE Router SHALL manter a Screen atual e ignorar a operação.
5. WHEN `navigateTo(screen)` é chamado, THE Router SHALL substituir toda a pilha por uma única Screen, invocando `onUnmount()` da Screen anterior e `onMount()` da nova Screen.
6. THE Router SHALL expor um método `currentScreen()` que retorna um ponteiro para a Screen ativa.

---

### Requisito 6: Integração com Sistema de Botões

**User Story:** Como desenvolvedor, quero que os eventos de botão sejam automaticamente encaminhados para a tela ativa, para que cada tela possa reagir à entrada do usuário de forma independente.

#### Critérios de Aceitação

1. WHEN um `ButtonEvent` diferente de `NONE` é detectado, THE Router SHALL encaminhar o InputEvent para o método `handleInput()` da Screen ativa.
2. THE Framework SHALL aceitar um ou mais objetos `Button` registrados via `addButton()` e consultá-los a cada iteração do RenderLoop.
3. WHILE nenhum Button estiver registrado, THE Framework SHALL operar normalmente sem processar InputEvents.

---

### Requisito 7: Ciclo Principal (RenderLoop)

**User Story:** Como desenvolvedor, quero uma única chamada `update()` no `loop()` do Arduino, para que o framework gerencie internamente botões, estado e renderização.

#### Critérios de Aceitação

1. THE RenderLoop SHALL expor um método `update()` que o desenvolvedor chama dentro do `loop()` do Arduino.
2. WHEN `update()` é chamado, THE RenderLoop SHALL executar, nesta ordem: (a) consultar todos os Buttons registrados, (b) encaminhar InputEvents ao Router, (c) verificar o dirty flag e, se necessário, redesenhar a Screen ativa no Display.
3. THE RenderLoop SHALL limitar a taxa de redesenho a no máximo 30 quadros por segundo para evitar uso excessivo de I2C.

---

### Requisito 8: Componente de Lista Selecionável

**User Story:** Como desenvolvedor, quero um componente de lista com item selecionado e scroll, para que o usuário possa navegar por opções do instrumento musical.

#### Critérios de Aceitação

1. THE ListComponent SHALL exibir uma lista de itens de texto com um indicador visual no item atualmente selecionado.
2. WHEN o InputEvent `SINGLE_CLICK` de um botão "cima" é recebido, THE ListComponent SHALL mover a seleção para o item anterior.
3. WHEN o InputEvent `SINGLE_CLICK` de um botão "baixo" é recebido, THE ListComponent SHALL mover a seleção para o próximo item.
4. WHILE a lista contiver mais itens do que cabem na área visível, THE ListComponent SHALL aplicar scroll automático para manter o item selecionado visível.
5. WHEN o item selecionado muda, THE ListComponent SHALL marcar o State como sujo para disparar redesenho.

---

### Requisito 9: Componente de Texto

**User Story:** Como desenvolvedor, quero renderizar texto no display de forma simples, para que eu possa exibir informações como nome de notas, valores MIDI e rótulos.

#### Critérios de Aceitação

1. THE TextComponent SHALL renderizar uma string na posição (x, y) especificada via Props.
2. THE TextComponent SHALL aceitar Props para tamanho da fonte (1, 2 ou 3) e cor (branco ou preto/invertido).
3. WHEN o texto exceder a largura disponível, THE TextComponent SHALL truncar o texto no limite da área sem causar overflow no buffer.

---

### Requisito 10: Componente de Barra de Progresso

**User Story:** Como desenvolvedor, quero exibir barras de progresso, para que eu possa representar visualmente valores como velocidade MIDI ou posição de controle.

#### Critérios de Aceitação

1. THE ProgressBarComponent SHALL renderizar uma barra horizontal na posição e dimensões especificadas via Props.
2. WHEN o valor do State associado muda, THE ProgressBarComponent SHALL atualizar o preenchimento proporcional ao valor (0 a 100).
3. THE ProgressBarComponent SHALL desenhar uma borda de 1 pixel ao redor da barra e preencher o interior proporcionalmente.

---

### Requisito 11: Uso de Memória

**User Story:** Como desenvolvedor embarcado, quero que o framework consuma pouca memória RAM, para que sobre espaço para o motor MIDI e outras funcionalidades.

#### Critérios de Aceitação

1. THE Framework SHALL utilizar alocação estática ou pool de memória para Components, evitando `new`/`delete` dinâmicos durante a operação normal.
2. THE Framework SHALL manter apenas a Screen ativa e a pilha de referências (ponteiros) das Screens anteriores em memória, sem manter buffers de Screens inativas.
3. THE Renderer SHALL utilizar um único buffer de 1024 bytes (128×64 pixels / 8 bits) compartilhado com a biblioteca Adafruit SSD1306.
