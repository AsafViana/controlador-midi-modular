# TODO — Módulo Principal como Produto Final

> Tarefas para deixar o módulo principal pronto para comercialização.
> Foco na experiência do cliente final que não desenvolveu o sistema.
> Escravos I2C ficam para depois.

---

## Tarefas

| # | Item | Impacto no cliente | Esforço | Status |
|---|---|---|---|---|
| 1 | Conectar feedback CC → PerformanceScreen | Parece bug (barra parada) | Pequeno | [x] |
| 2 | Monitor de CC em tempo real na PerformanceScreen | Validação visual do sistema | Médio | [x] |
| 3 | Splash screen no boot | Identidade do produto | Pequeno | [x] |
| 4 | Feedback visual "Salvo" ao confirmar | Confiança do usuário | Pequeno | [x] |
| 5 | Decidir sobre Teclado: implementar ou esconder | Feature fantasma | Médio | [x] |
| 6 | Factory reset | Essencial para suporte | Médio | [x] |
| 7 | Aceleração na edição de valores (long press = incremento rápido) | Usabilidade | Médio | [x] |
| 8 | Enriquecer tela Sobre (status, controles) | Profissionalismo | Pequeno | [x] |
| 9 | Info de status no menu ou header | Orientação do usuário | Médio | [x] |
| 10 | Bug: CanalScreen buffer local → ponteiro pendurado | Crash/lixo na tela | Pequeno | [x] |
| 11 | Aceleração no CanalScreen e OitavaScreen | UX inconsistente | Pequeno | [x] |
| 12 | Fallback LED se display falhar | Produto parece morto | Pequeno | [x] |
| 13 | Atualizar API.md com features novas | Manutenibilidade | Médio | [x] |
| 14 | Atualizar checklist-aprendizado.md | Manutenibilidade | Médio | [x] |
| 15 | Verificar testes (construtores mudaram) | Build quebrado | Médio | [x] |
| 16 | Atualizar README.md | Documentação | Pequeno | [x] |
| 9 | Info de status no menu ou header | Orientação do usuário | Médio | [x] |

---

## O que foi feito

### 1+2. Feedback CC + Monitor em tempo real (PerformanceScreen)

- Criado `CCActivityInfo` (struct com label, CC, valor, canal, módulo)
- `ControlReader` agora tem callback `onCCActivity()` que notifica a cada envio de CC
- `PerformanceScreen` redesenhada: mostra canal+oitava no topo, e na metade inferior exibe em tempo real o nome do controle, CC:valor, e módulo de origem (Local ou [XX])
- Barra de progresso visual acompanha o valor

**Arquivos:** `CCActivityInfo.h`, `ControlReader.h/.cpp`, `PerformanceScreen.h/.cpp`, `main.cpp`

### 3. Splash screen

- `OledApp::showSplash()` exibe nome do produto + versão centralizado por 1.5s
- Chamado no `setup()` logo após inicializar o display

**Arquivos:** `OledApp.h/.cpp`, `main.cpp`

### 4. Feedback visual "Salvo"

- `OledApp::showSaveConfirm()` faz inversão rápida do display (150ms) como feedback
- Chamado em CanalScreen, OitavaScreen, VelocidadeScreen e TecladoScreen ao confirmar

**Arquivos:** `OledApp.h/.cpp`, `CanalScreen.cpp`, `OitavaScreen.cpp`, `VelocidadeScreen.cpp`, `TecladoScreen.cpp`

### 5. Teclado escondido

- Opção "Teclado" removida do menu de Configurações (substituída por "Restaurar")
- A infraestrutura (TecladoScreen, Storage) permanece intacta para reativar quando houver hardware

**Arquivos:** `ConfigScreen.cpp`

### 6. Factory reset

- `Storage::factoryReset()` limpa NVS e restaura todos os defaults do HardwareMap
- Opção "Restaurar" no ConfigScreen com tela de confirmação (SELECT confirma, UP/DOWN cancela)

**Arquivos:** `Storage.h/.cpp`, `ConfigScreen.h/.cpp`

### 7. Aceleração na edição de valores

- Adicionados `NavInput::LONG_UP` e `NavInput::LONG_DOWN`
- `OledApp` envia LONG_UP/LONG_DOWN quando detecta `LONG_PRESS` nos botões UP/DOWN
- VelocidadeScreen e CCMapScreen (modo edição CC) usam passo de +5/-5 no long press

**Arquivos:** `NavInput.h`, `OledApp.cpp`, `VelocidadeScreen.cpp`, `CCMapScreen.cpp`

### 8. Tela Sobre enriquecida

- `SobreScreen` agora recebe `Storage*` e exibe: nome, versão, canal MIDI atual, número de controles
- Dados atualizados a cada `onMount()`

**Arquivos:** `SobreScreen.h/.cpp`, `main.cpp`

### 9. Status no menu principal

- `MenuScreen` agora recebe `Storage*` e exibe linha de status no rodapé: "Ch:1 Oit:4 Vel:100"
- Atualizado a cada `onMount()` (quando volta de sub-telas, reflete mudanças)

**Arquivos:** `MenuScreen.h/.cpp`, `main.cpp`
