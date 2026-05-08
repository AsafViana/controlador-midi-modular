# Melhorias para Produto Final — Controlador MIDI Modular

> Documento de referência para evolução do firmware até nível de produto comercial.
> Baseado no hardware atual: ESP32-S3 N16R8, OLED SSD1306 128x64, 4 botões (UP/DOWN/SELECT/BACK),
> potenciômetros analógicos, MIDI USB + DIN, barramento I2C para módulos remotos.

---

## Hardware considerado

- ESP32-S3-DevKitC-1 (N16R8) — 16MB flash, PSRAM, WiFi/BLE nativo
- Display OLED SSD1306 128x64 monocromático, I2C, sem touch
- 4 botões físicos: UP, DOWN, SELECT, BACK (BACK a ser adicionado)
- 1 potenciômetro ativo (GPIO 7), 5 GPIOs analógicos livres (1, 2, 3, 6, 8)
- MIDI DIN (Serial1, GPIO 9/10) + USB MIDI (Control_Surface)
- LED indicador (GPIO 0)
- Barramento I2C compartilhado (GPIO 4/5) — display + módulos remotos (0x20-0x27)
- GPIOs digitais livres para expansão: 14, 15, 16, 17, 18, 21, 38-42

---

## Princípios de UX para este hardware

1. Tela mostra no máximo 4-5 linhas úteis de conteúdo (fonte 6x8, header consome 2 linhas)
2. Texto sempre mais legível que ícones nessa resolução
3. Hierarquia de navegação máxima: 3 níveis (Menu → Submenu → Edição)
4. BACK sempre volta um nível, sem exceção
5. Feedback visual deve ser textual e temporário (1-2s)
6. Transições instantâneas (sem animação — corte limpo é mais legível que fade pixelado)

---

## 🔴 PRIORIDADE ALTA — Essencial para lançamento

### 1. Botão BACK dedicado

**Objetivo:** Dar ao usuário uma saída óbvia e consistente de qualquer tela, sem precisar procurar "Voltar" numa lista ou depender de gestos ocultos.

**Situação atual:** Não existe botão BACK. Para voltar, o usuário precisa navegar até a opção "Voltar" na lista (ConfigScreen) ou pressionar SELECT que salva e volta (telas de edição). Não há como cancelar uma edição.

**Implementação:**

- Adicionar 1 push button em GPIO livre (ex: GPIO 14)
- Adicionar `BACK` ao enum `NavInput`
- No `OledApp::update()`, ler o novo botão e emitir `NavInput::BACK`
- Comportamento padrão no Router: se a tela não trata BACK, faz `pop()` automaticamente
- Remover a opção "Voltar" das listas (ou manter como redundância acessível)

**Hardware necessário:** 1 push button + 1 resistor pull-up (ou usar INPUT_PULLUP interno)

---

### 2. Cancelar edição sem salvar

**Objetivo:** Permitir que o usuário explore valores sem medo de alterar a configuração acidentalmente. BACK sai sem salvar, SELECT confirma e salva.

**Situação atual:** Em CanalScreen, OitavaScreen e VelocidadeScreen, SELECT é a única saída e sempre salva. O usuário não tem como desistir de uma alteração.

**Implementação:**

- Cada tela de edição guarda o valor original no `onMount()`
- BACK restaura o valor original e faz `router.pop()`
- SELECT salva no Storage, mostra feedback "Salvo!", e faz `router.pop()`
- Exibir na tela: "SEL=Salvar" e "◀=Cancelar" (ou texto equivalente curto)

**Hardware necessário:** Nenhum (depende do item 1)

---

### 3. Watchdog Timer (WDT)

**Objetivo:** Garantir que o produto nunca trave permanentemente. Se o loop principal parar de responder por mais de 5 segundos, o ESP32 reinicia sozinho.

**Situação atual:** Não há watchdog. Se ocorrer um deadlock no I2C, loop infinito, ou bug inesperado, o produto fica congelado até o usuário desligar e religar manualmente.

**Implementação:**

- Habilitar o Task Watchdog Timer do ESP-IDF (`esp_task_wdt_init()`)
- Alimentar o WDT no final de cada iteração do `loop()`
- Timeout de 5 segundos (suficiente para operações I2C lentas)
- Após reset por WDT, o sistema reinicia normalmente com última config salva

**Hardware necessário:** Nenhum (recurso nativo do ESP32)

---

### 4. Timeout de inatividade → volta ao Performance

**Objetivo:** Se o usuário deixar o controlador parado num menu de configuração e começar a tocar, a tela volta sozinha para a tela útil (Performance) onde ele vê o feedback dos CCs em tempo real.

**Situação atual:** A tela fica parada onde o usuário deixou. Se ele esqueceu no menu de config, não vê nenhum feedback visual enquanto toca.

**Implementação:**

- Contador de inatividade resetado a cada `NavInput` recebido
- Após 60 segundos sem input, `router.navigateTo(perfScreen)` (limpa a stack)
- Não aplicar timeout se já estiver na PerformanceScreen ou MenuScreen (nível raiz)
- Configurável no menu: 30s / 60s / 120s / Desligado

**Hardware necessário:** Nenhum

---

### 5. Feedback "Salvo!" textual

**Objetivo:** Confirmar visualmente ao usuário que a ação foi registrada, de forma inequívoca numa tela de 128x64 pixels.

**Situação atual:** `showSaveConfirm()` faz inversão rápida do display (150ms). Numa OLED pequena, isso pode parecer um glitch ao invés de confirmação intencional.

**Implementação:**

- Substituir a inversão por overlay centralizado: texto "Salvo!" em fonte tamanho 2 (12x16px), fundo preto, por 800ms
- Após o timeout, redesenha a tela anterior normalmente
- Não bloquear o loop — usar flag + timestamp para controlar duração

**Hardware necessário:** Nenhum

---

### 6. Proteção contra corrupção de NVS

**Objetivo:** Garantir que a configuração do usuário sobreviva a quedas de energia durante gravação, sem corromper o armazenamento.

**Situação atual:** Cada `set*()` grava diretamente no NVS. Se a energia cair no meio da escrita, o valor pode ficar inconsistente. O schema version detecta mudanças de firmware, mas não corrupção parcial.

**Implementação:**

- Estratégia de double-buffer: manter dois slots no NVS ("cfg_a" e "cfg_b")
- Gravar sempre no slot inativo, depois trocar o ponteiro de slot ativo
- Na leitura, verificar integridade (CRC8 simples) e usar o slot válido mais recente
- Se ambos estiverem corrompidos, aplicar factory reset automático

**Hardware necessário:** Nenhum

---

### 7. Modo headless (display falhou)

**Objetivo:** Se o display OLED falhar (mau contato, defeito), o controlador MIDI deve continuar funcionando normalmente com a última configuração salva. O produto não pode "morrer" por causa da tela.

**Situação atual:** Se `app->begin()` falha, o LED pisca 10 vezes e o código continua. Mas o loop ainda tenta atualizar o display, e os botões de navegação ficam inúteis sem feedback visual.

**Implementação:**

- Se display falhar no boot: pular toda a lógica de UI (não chamar `app->update()`)
- ControlReader continua operando normalmente (lê pots, envia CC)
- LED pisca em padrão reconhecível: 2 piscadas curtas a cada 3 segundos = "modo headless"
- Botões ficam inativos (sem tela, não há o que navegar)
- Ao reconectar o display (hot-plug I2C), tentar reinicializar

**Hardware necessário:** Nenhum

---

### 8. Sistema de presets (mínimo 4 slots)

**Objetivo:** Permitir que o músico salve e carregue configurações completas para diferentes contextos (músicas, instrumentos, setups de palco) sem reconfigurar tudo manualmente.

**Situação atual:** Existe apenas uma configuração ativa. Para trocar de setup, o usuário precisa entrar em cada submenu e alterar canal, oitava, velocidade e CC map individualmente.

**Implementação:**

- Cada preset armazena: canal MIDI, oitava, velocidade, mapa de CC completo (local + remoto)
- 4 slots nomeáveis (nome curto de até 8 caracteres, ex: "Synth", "Drums", "Live", "Studio")
- Tela de seleção de preset acessível do menu principal
- Opções por preset: Carregar / Salvar / Renomear
- Preset ativo indicado na status bar do menu: "P1:Synth Ch:1"
- Armazenamento no NVS com prefixo por slot ("p0_canal", "p0_cc0", etc.)

**Hardware necessário:** Nenhum (usa NVS existente, 16MB flash tem espaço de sobra)

---

## 🟡 PRIORIDADE MÉDIA — Diferencial competitivo

### 9. Program Change

**Objetivo:** Permitir que o controlador troque patches/presets no sintetizador ou DAW conectado. É funcionalidade esperada em qualquer controlador MIDI de mercado.

**Situação atual:** O firmware só envia Note On/Off e Control Change. Não há suporte a Program Change.

**Implementação:**

- Adicionar `MidiEngine::sendProgramChange(uint8_t program, uint8_t canal)`
- Enviar via USB e DIN simultaneamente (mesmo padrão do sendCC)
- Tela dedicada para envio manual: UP/DOWN seleciona programa (0-127), SELECT envia
- Opcionalmente: associar Program Change ao carregamento de preset (item 8)

**Hardware necessário:** Nenhum

---

### 10. MIDI IN (receber mensagens)

**Objetivo:** Permitir que o controlador receba dados MIDI do computador/DAW para sincronização, feedback visual, e MIDI Thru.

**Situação atual:** O MidiEngine só envia. A porta MIDI DIN RX (GPIO 10) está configurada mas não é lida. USB MIDI também não processa mensagens recebidas.

**Implementação:**

- No loop, verificar `Control_Surface` e `Serial1` por mensagens recebidas
- Processar CC recebido: atualizar barra de progresso na PerformanceScreen (feedback visual)
- MIDI Thru: reenviar mensagens recebidas pela outra interface (USB→DIN e DIN→USB)
- Filtro configurável: aceitar apenas do canal ativo ou todos os canais

**Hardware necessário:** Nenhum (circuito MIDI IN com 6N138 já está documentado como opcional)

---

### 11. Calibração de potenciômetros pelo menu

**Objetivo:** Compensar variação entre componentes baratos. Cada pot tem range real diferente — sem calibração, os extremos (0 e 127) podem ser inalcançáveis.

**Situação atual:** `ADC_MIN=100` e `ADC_MAX=3900` são hardcoded para todos os pots. Se um pot específico só vai de 200 a 3600, o usuário nunca atinge 0 ou 127.

**Implementação:**

- Tela de calibração: "Gire [Pot Extra] até o mínimo → SELECT → Gire até o máximo → SELECT → Pronto!"
- Salvar min/max por controle no NVS
- ControlReader usa os valores calibrados ao invés dos hardcoded
- Opção de recalibrar a qualquer momento no menu de config
- Factory reset restaura valores padrão (100/3900)

**Hardware necessário:** Nenhum

---

### 12. Curvas de resposta configuráveis

**Objetivo:** Adaptar a resposta do potenciômetro ao tipo de controle. Volume precisa de curva logarítmica (ouvido humano), filtro funciona melhor linear, expressão pode ser exponencial.

**Situação atual:** Todos os controles usam mapeamento linear (ADC → 0-127 proporcional). Um pot de volume linear soa "estranho" porque o ouvido percebe volume de forma logarítmica.

**Implementação:**

- 3 curvas: Linear (padrão), Logarítmica, Exponencial
- Seleção por controle no CCMapScreen: "Curva: Lin" → UP/DOWN alterna → SELECT confirma
- Lookup table de 128 bytes por curva (pré-calculada, armazenada em PROGMEM)
- Aplicar após a conversão ADC→0-127 no ControlReader
- Salvar escolha no NVS por controle

**Hardware necessário:** Nenhum

---

### 13. Screensaver (dim + desligar display)

**Objetivo:** Prolongar a vida útil do OLED (pixels orgânicos degradam com uso contínuo) e reduzir consumo em modo idle.

**Situação atual:** Display fica ligado 100% do tempo com brilho máximo, mesmo quando ninguém está olhando.

**Implementação:**

- Após 2 minutos sem input: reduzir contraste para 10% (`ssd1306_command(SSD1306_SETCONTRAST, 25)`)
- Após 5 minutos sem input: desligar display (`ssd1306_command(SSD1306_DISPLAYOFF)`)
- Qualquer botão pressionado: acordar display com contraste normal, consumir o evento (não navegar)
- Atividade MIDI (CC enviado) NÃO acorda o display (senão nunca desliga durante performance)
- Tempos configuráveis no menu: 1/2/5/10 min ou Desligado

**Hardware necessário:** Nenhum

---

### 14. Indicador de scroll (setas ▲▼)

**Objetivo:** Informar ao usuário que existem mais itens acima ou abaixo da área visível da lista. Sem isso, ele pode não saber que há mais opções.

**Situação atual:** ListComponent faz scroll automático, mas não há indicação visual de que a lista continua além do visível.

**Implementação:**

- Se `scrollOffset > 0`: desenhar "▲" no canto superior direito da lista
- Se `scrollOffset + visibleItems < totalItems`: desenhar "▼" no canto inferior direito
- Usar caractere de 1 byte (triângulo) — cabe em 6px de largura
- Não consumir linha extra — sobrepor no último pixel da margem direita

**Hardware necessário:** Nenhum

---

### 15. Ativar GPIOs analógicos livres

**Objetivo:** Utilizar todo o potencial do hardware. O ESP32-S3 tem 5 GPIOs analógicos livres que já estão documentados mas não configurados no firmware.

**Situação atual:** Apenas GPIO 7 ("Pot Extra") está ativo. GPIOs 1, 2, 3, 6, 8 estão livres e documentados no `lista-hardware-minimo.txt` para Volume, Pan, Modulação e Sensor de Luz.

**Implementação:**

- Adicionar ao array `CONTROLES[]` em HardwareMap.h:
  - GPIO 1: "Volume" (CC 7)
  - GPIO 2: "Pan" (CC 10)
  - GPIO 3: "Modulacao" (CC 1)
  - GPIO 6: "Sensor Luz" (CC 74, invertido)
  - GPIO 8: livre para expansão futura
- O sistema de auto-assign já resolve conflitos de CC
- ControlReader já itera sobre todos os controles do array — basta adicionar

**Hardware necessário:** Potenciômetros/sensores nos GPIOs (já documentados na lista de compras)

---

### 16. Suporte a botões MIDI (momentâneo e toggle)

**Objetivo:** Expandir o controlador além de potenciômetros. Botões MIDI são usados para: disparar notas (pads), ligar/desligar efeitos (toggle CC 0↔127), tap tempo, transport (play/stop).

**Situação atual:** O firmware só lê controles analógicos. Botões existem apenas para navegação (UP/DOWN/SELECT). Não há infraestrutura para botões que enviam MIDI.

**Implementação:**

- Adicionar `TipoControle::BOTAO_MIDI_MOMENTANEO` e `BOTAO_MIDI_TOGGLE` ao enum
- Botão momentâneo: press → CC 127, release → CC 0 (ou Note On/Off)
- Botão toggle: press → alterna entre CC 0 e CC 127 a cada clique
- Configurável no CCMapScreen: tipo de mensagem (CC/Note), comportamento (momentâneo/toggle)
- Usar GPIOs digitais livres (14, 15, 16, 17, 18)

**Hardware necessário:** Push buttons nos GPIOs livres + resistores pull-up

---

### 17. SysEx backup/restore

**Objetivo:** Permitir que o usuário exporte toda a configuração do controlador via MIDI e importe de volta. Útil para: backup antes de atualização, clonar config entre unidades, compartilhar setups.

**Situação atual:** A configuração só existe no NVS interno. Se o NVS corromper ou o usuário quiser copiar para outro controlador, não há como.

**Implementação:**

- Definir formato SysEx proprietário: `F0 7D [device_id] [command] [data...] F7`
- Comando DUMP: envia toda a config (presets, CC maps, calibração) como sequência de mensagens SysEx
- Comando LOAD: recebe e aplica config completa
- Acionado pelo menu: "Config → Backup → Enviar" / "Config → Backup → Receber"
- Compatível com qualquer software que grave SysEx (MIDI-OX, SysEx Librarian)

**Hardware necessário:** Nenhum

---

### 18. First-run wizard

**Objetivo:** Guiar o usuário leigo na primeira configuração do produto, sem precisar ler manual. Reduz chamados de suporte e melhora a primeira impressão.

**Situação atual:** O produto liga direto no menu principal. O usuário precisa descobrir sozinho onde configurar canal, como funciona a navegação, etc.

**Implementação:**

- Detectar primeiro boot (flag no NVS: `wizard_done = false`)
- Sequência de 3-4 telas simples:
  1. "Bem-vindo! Use UP/DOWN para navegar e SELECT para confirmar."
  2. "Canal MIDI:" → UP/DOWN para escolher → SELECT
  3. "Calibrar pots? SELECT=Sim BACK=Pular"
  4. "Pronto! Bom som!"
- Após completar, gravar `wizard_done = true`
- Acessível novamente via menu: "Config → Assistente inicial"

**Hardware necessário:** Nenhum

---

## 🟢 PRIORIDADE BAIXA — Polish e futuro

### 19. OTA Update (Over-The-Air)

**Objetivo:** Permitir atualização de firmware sem abrir o produto ou conectar cabo USB ao computador. Essencial para suporte pós-venda em escala.

**Situação atual:** Atualização requer conexão USB + PlatformIO/esptool. Inviável para usuário final.

**Implementação:**

- Usar WiFi do ESP32-S3 (já disponível no hardware)
- Modo de atualização acionado pelo menu: "Config → Atualizar firmware"
- ESP32 cria AP temporário ou conecta em rede configurada
- Interface web mínima para upload do .bin (AsyncWebServer + OTA)
- Partição OTA já suportada pelo partition scheme de 16MB
- Após atualização, reinicia com novo firmware; rollback automático se falhar

**Hardware necessário:** Nenhum (WiFi é nativo do ESP32-S3)

---

### 20. LED RGB de status

**Objetivo:** Dar feedback visual sem precisar olhar a tela. Útil em palco escuro ou quando o display está em screensaver.

**Situação atual:** LED simples no GPIO 0, usado apenas para indicar falha de display (10 piscadas no boot).

**Implementação:**

- Substituir LED simples por NeoPixel (WS2812B) ou LED RGB comum
- Padrões de cor:
  - Verde fixo: sistema OK, idle
  - Azul piscando: atividade MIDI (CC sendo enviado)
  - Vermelho fixo: erro (display falhou, NVS corrompido)
  - Roxo: modo OTA ativo
- Brilho baixo para não distrair (10-20%)

**Hardware necessário:** 1 LED RGB (NeoPixel WS2812B) ou 1 LED RGB catodo comum + 3 resistores. 1 GPIO (pode reusar GPIO 0).

---

### 21. Idioma configurável (PT/EN)

**Objetivo:** Abrir mercado internacional. Português para Brasil, inglês para exportação.

**Situação atual:** Todas as strings estão hardcoded em português nos arquivos .cpp.

**Implementação:**

- Tabela de strings indexada: `const char* STR[NUM_STRINGS][2]` (PT=0, EN=1)
- Macro ou função: `STR(ID_MENU_PRINCIPAL)` retorna string no idioma ativo
- Seleção no menu: "Config → Idioma → PT / EN"
- Salvar escolha no NVS
- Strings curtas (máximo 20 chars) para caber na tela

**Hardware necessário:** Nenhum

---

### 22. Contraste ajustável

**Objetivo:** Adaptar visibilidade da tela ao ambiente. Palco escuro precisa de brilho baixo (não ofuscar), ambiente claro precisa de brilho máximo.

**Situação atual:** Display opera com contraste fixo (padrão do driver SSD1306).

**Implementação:**

- Menu: "Config → Contraste" → UP/DOWN ajusta (Baixo / Médio / Alto)
- Mapear para valores do SSD1306: 0x00 (mínimo), 0x7F (médio), 0xFF (máximo)
- Salvar no NVS
- Aplicar no boot e após carregar preset

**Hardware necessário:** Nenhum

---

### 23. MIDI Clock output

**Objetivo:** Gerar clock MIDI para sincronizar equipamentos externos (delays, arpeggiators, sequenciadores) ao tempo definido pelo músico via tap tempo.

**Situação atual:** Não há suporte a MIDI Clock.

**Implementação:**

- Tap tempo: botão dedicado ou combinação (ex: BACK + SELECT simultâneo)
- Calcular BPM a partir de 2-4 taps consecutivos
- Enviar 24 pulsos por quarter note (padrão MIDI Clock)
- Exibir BPM na PerformanceScreen: "120 BPM"
- Start/Stop/Continue messages
- Configurável: Clock ON/OFF no menu

**Hardware necessário:** Nenhum (pode usar combinação de botões existentes ou botão MIDI dedicado do item 16)

---

### 24. Versionamento semântico + changelog

**Objetivo:** Permitir que o suporte identifique rapidamente qual versão o cliente tem, e que o cliente saiba o que mudou em cada atualização.

**Situação atual:** `version_script.py` já gera `FIRMWARE_VERSION` e a tela Sobre exibe. Mas não há changelog formal nem numeração semântica.

**Implementação:**

- Adotar SemVer: MAJOR.MINOR.PATCH (ex: 1.0.0)
- Manter CHANGELOG.md na raiz do projeto
- Exibir versão na splash screen e tela Sobre (já feito)
- Incluir data de build na tela Sobre: "v1.2.0 (2026-05-08)"

**Hardware necessário:** Nenhum

---

### 25. Testes de integração automatizados

**Objetivo:** Prevenir regressões ao adicionar features. Garantir que navegação, salvamento, envio de CC e lógica de presets funcionam corretamente após cada mudança.

**Situação atual:** Existe `env:native` com Unity no platformio.ini, mas a cobertura real dos fluxos é limitada. Mocks existem para I2C e Arduino.

**Implementação:**

- Testes de fluxo: navegar Menu → Config → Canal → alterar → salvar → verificar Storage
- Testes de ControlReader: simular ADC → verificar CC enviado
- Testes de Router: push/pop/navigateTo com verificação de lifecycle
- Testes de presets: salvar/carregar/verificar integridade
- Rodar no CI (GitHub Actions) a cada push

**Hardware necessário:** Nenhum (roda no env:native)

---

## Resumo de dependências entre itens

```
Item 1 (BACK) ──→ Item 2 (Cancelar edição)
Item 6 (NVS seguro) ──→ Item 8 (Presets) ──→ Item 17 (SysEx backup)
Item 10 (MIDI IN) ──→ Item 17 (SysEx restore)
Item 11 (Calibração) ──→ Item 18 (First-run wizard)
Item 15 (GPIOs) ──→ Item 16 (Botões MIDI)
Item 19 (OTA) ──→ Item 24 (Versionamento)
```

---

## Roadmap sugerido

**v1.0 — Base sólida (items 1-8)**
Navegação robusta, proteção contra falhas, presets básicos. Produto funcional e confiável.

**v1.5 — MIDI completo (items 9-12, 15)**
Program Change, MIDI IN, calibração, curvas. Competitivo com controladores de mercado.

**v2.0 — Experiência premium (items 13, 16-19, 20)**
Screensaver, botões MIDI, SysEx, wizard, OTA, LED RGB. Diferencial de produto.

**v2.5 — Internacional (items 21-25)**
Idioma, contraste, clock, versionamento formal, testes completos. Pronto para escala.
