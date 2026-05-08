# Requirements Document

## Introduction

Simplificação do fluxo de navegação da tela de mapeamento CC (CCMapScreen) no controlador MIDI. O objetivo é reduzir etapas desnecessárias na configuração de controles, removendo a tela de habilitação ON/OFF, removendo a tela de lista de componentes, e adicionando uma opção de "voltar" na tela de escuta de controles.

## Glossary

- **CCMapScreen**: Tela de configuração de mapeamento CC que permite ao usuário associar valores CC MIDI a controles físicos
- **ModoEdicao**: Máquina de estados interna da CCMapScreen que controla qual etapa do fluxo está ativa
- **AGUARDANDO_CONTROLE**: Estado da CCMapScreen onde o sistema escuta movimentação de controles físicos (MIDI Learn)
- **EDITAR_CC**: Estado da CCMapScreen onde o usuário ajusta o valor CC (0-127) para o controle selecionado
- **Router**: Componente de navegação que gerencia a pilha de telas (push/pop)
- **NavInput**: Enumeração de entradas de navegação do usuário (UP, DOWN, SELECT, LONG_UP, LONG_DOWN)
- **Storage**: Camada de persistência que armazena configurações de CC e habilitação dos controles
- **OledApp**: Aplicação principal que gerencia display, router e feedback visual

## Requirements

### Requirement 1: Remover etapa de habilitação ON/OFF

**User Story:** Como usuário, eu quero que ao confirmar o valor CC o sistema salve imediatamente e retorne à escuta, para que eu configure controles mais rapidamente sem passar pela tela de ON/OFF.

#### Acceptance Criteria

1. WHEN o usuário pressiona SELECT no modo EDITAR_CC, THE CCMapScreen SHALL salvar o valor CC no Storage e retornar ao modo AGUARDANDO_CONTROLE
2. WHEN o usuário pressiona SELECT no modo EDITAR_CC, THE CCMapScreen SHALL exibir o feedback visual de confirmação (showSaveConfirm) antes de retornar ao modo AGUARDANDO_CONTROLE
3. THE CCMapScreen SHALL manter o estado de habilitação existente de cada controle inalterado durante a edição de CC (preservar valor atual no Storage)

### Requirement 2: Adicionar opção de voltar na tela de escuta

**User Story:** Como usuário, eu quero poder voltar ao menu de configurações a partir da tela de escuta de controles, para que eu possa sair sem precisar esperar ou mover um controle.

#### Acceptance Criteria

1. WHILE no modo AGUARDANDO_CONTROLE, THE CCMapScreen SHALL exibir instrução indicando que SELECT permite voltar ao menu anterior
2. WHEN o usuário pressiona SELECT no modo AGUARDANDO_CONTROLE, THE CCMapScreen SHALL executar router.pop() para retornar à tela anterior (ConfigScreen)
3. THE CCMapScreen SHALL exibir texto informativo na tela indicando a função do botão SELECT como "voltar" (ex: "SELECT = Voltar")

### Requirement 3: Remover tela de lista de componentes

**User Story:** Como usuário, eu quero que após configurar o CC de um controle o sistema volte diretamente à escuta, para que eu possa configurar outro controle imediatamente sem navegar por uma lista.

#### Acceptance Criteria

1. THE CCMapScreen SHALL iniciar no modo AGUARDANDO_CONTROLE ao ser montada (onMount)
2. WHEN a configuração de CC é confirmada, THE CCMapScreen SHALL retornar ao modo AGUARDANDO_CONTROLE em vez de exibir uma lista de componentes
3. THE CCMapScreen SHALL remover o modo NENHUM (lista de componentes) da máquina de estados ModoEdicao

### Requirement 4: Fluxo simplificado completo

**User Story:** Como usuário, eu quero um fluxo de configuração CC com apenas duas etapas (escuta → edição CC → escuta), para que a experiência seja direta e sem telas intermediárias.

#### Acceptance Criteria

1. THE CCMapScreen SHALL suportar apenas dois modos de edição: AGUARDANDO_CONTROLE e EDITAR_CC
2. WHEN um controle físico é movido no modo AGUARDANDO_CONTROLE, THE CCMapScreen SHALL transicionar para o modo EDITAR_CC com o valor CC atual do controle detectado
3. WHEN o valor CC é confirmado no modo EDITAR_CC, THE CCMapScreen SHALL salvar o valor, exibir feedback visual e retornar ao modo AGUARDANDO_CONTROLE
4. WHILE no modo EDITAR_CC, THE CCMapScreen SHALL permitir ajuste do valor CC entre 0 e 127 usando UP/DOWN (passo 1) e LONG_UP/LONG_DOWN (passo 5)
