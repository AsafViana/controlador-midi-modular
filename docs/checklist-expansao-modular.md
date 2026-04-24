# Checklist de Testes — Expansão Modular I2C

> [!info] Quando usar
> Este checklist só pode ser executado quando você tiver **pelo menos 2 módulos** (principal + 1 escravo). Os testes com 3 módulos estão na seção final.

---

## 🔌 1. Descoberta do primeiro módulo escravo

- [ ] Ligar o módulo principal **sem** o escravo conectado
- [ ] Verificar que a tela `Endereco CC` mostra **apenas** os controles locais
- [ ] Conectar o módulo escravo no barramento I2C (SDA, SCL, GND)
- [ ] Aguardar até 5 segundos (intervalo do `periodicScan`)
- [ ] Verificar que os controles do escravo **aparecem** na lista de `Endereco CC`
- [ ] Verificar que os controles remotos mostram o prefixo `[XX]` com o endereço I2C hex (ex: `[20]Pot1`)
- [ ] Verificar que os labels dos controles remotos correspondem ao que o escravo reporta no descritor

---

## 📡 2. Protocolo I2C — Descritor

> [!warning] Crítico
> Se o descritor não for lido corretamente, nenhum controle remoto vai aparecer.

- [ ] O escravo responde ao comando `0x01` (`CMD_DESCRIPTOR`) com o formato correto:
  - Byte 0: número de controles (1-16)
  - Para cada controle: `[tipo:1][label:12][valor:1]` = 14 bytes
- [ ] Tipos válidos: `0` = BOTAO, `1` = POTENCIOMETRO, `2` = SENSOR, `3` = ENCODER
- [ ] Labels com menos de 12 caracteres são preenchidos com `\0` (null-padded)
- [ ] O número total de bytes da resposta bate: `1 + (numControles × 14)`
- [ ] Se o escravo reportar 0 controles ou mais de 16, o mestre **ignora** o módulo

---

## 📡 3. Protocolo I2C — Leitura de valores

- [ ] O escravo responde ao comando `0x02` (`CMD_READ_VALUES`) com N bytes (um por controle, 0-127)
- [ ] Mover um potenciômetro no escravo e verificar que o valor MIDI CC muda no host (DAW/monitor MIDI)
- [ ] Verificar que valores acima de 127 são clampados para 127
- [ ] Verificar que a zona morta está funcionando (movimentos muito pequenos não geram CC)

---

## ⚙️ 4. Configuração de CC por controle remoto

- [ ] Na tela `Endereco CC`, selecionar um controle remoto
- [ ] Editar o número CC (ex: mudar de 0 para 74)
- [ ] Confirmar com `SELECT`
- [ ] Mover o potenciômetro remoto e verificar que o CC enviado é o **novo valor** (74)
- [ ] Habilitar/desabilitar o controle remoto e verificar que ele **para/volta** a enviar CC

---

## 💾 5. Persistência de configurações remotas

- [ ] Configurar CC de um controle remoto (ex: CC 74)
- [ ] **Desligar e religar** o módulo principal (power cycle)
- [ ] Verificar que o CC configurado **persiste** (ainda é 74, não voltou para 0)
- [ ] Verificar que o estado habilitado/desabilitado também persiste
- [ ] Desconectar o escravo, religar o principal, reconectar o escravo — a configuração deve ser **mantida**

> [!tip] Como funciona
> O `Storage` salva no NVS com chaves baseadas no endereço I2C + índice do controle (ex: `rcc2001` para endereço `0x20`, controle 1). Enquanto o escravo mantiver o mesmo endereço, a configuração é recuperada.

---

## 🔄 6. Hot-plug — Desconexão e reconexão

- [ ] Com o escravo conectado e funcionando, **desconectar o cabo I2C**
- [ ] Verificar que o sistema **não trava** (a UI continua respondendo)
- [ ] Verificar que após ~15 segundos (3 falhas × 5s de intervalo) o módulo é marcado como desconectado
- [ ] Os controles remotos **desaparecem** da lista de `Endereco CC`
- [ ] **Reconectar** o cabo I2C
- [ ] Verificar que os controles **reaparecem** na lista (dentro de 5 segundos)
- [ ] Verificar que as configurações de CC salvas são **restauradas** (não voltam para 0)
- [ ] Mover um potenciômetro remoto e verificar que o CC é enviado normalmente

---

## ⚡ 7. Resiliência a falhas de comunicação

- [ ] Desconectar brevemente o cabo I2C (menos de 1 segundo) e reconectar
- [ ] Verificar que o sistema se recupera sem precisar de reboot
- [ ] Testar com cabo I2C longo (~50cm) e verificar se há erros de leitura
- [ ] Se houver erros frequentes, testar com pull-ups de 4.7kΩ nas linhas SDA/SCL
- [ ] Verificar que leituras falhadas **não enviam CC** (o sistema mantém o último valor válido)

---

## 🎹 8. Funcionamento MIDI com controles remotos

- [ ] Conectar o controlador a uma DAW ou monitor MIDI
- [ ] Mover um potenciômetro **local** — verificar CC correto no canal configurado
- [ ] Mover um potenciômetro **remoto** — verificar CC correto no canal configurado
- [ ] Ambos devem usar o **mesmo canal MIDI** (configurado na tela `Canal MIDI`)
- [ ] Mudar o canal MIDI pela interface e verificar que **todos** os controles (locais e remotos) passam a usar o novo canal
- [ ] Desabilitar um controle remoto e verificar que ele **para** de enviar CC
- [ ] Verificar que não há flood de mensagens MIDI (zona morta funcionando)

---

## 🖥️ 9. Interface — Tela CCMapScreen com remotos

- [ ] A lista mostra primeiro os controles locais, depois os remotos
- [ ] Controles remotos têm prefixo `[XX]` (ex: `[20]Pot1`)
- [ ] O scroll funciona corretamente quando a lista tem mais de 4 itens (locais + remotos)
- [ ] Navegar até um controle remoto e entrar no modo edição funciona normalmente
- [ ] O fluxo de edição (CC → ON/OFF → sair) funciona igual para locais e remotos

---

## 👥 10. Testes com 3 módulos (principal + 2 escravos)

> [!note] Pré-requisito
> Os escravos devem ter endereços I2C **diferentes** (ex: `0x20` e `0x21`).

### Descoberta simultânea

- [ ] Ligar o principal com os 2 escravos já conectados
- [ ] Verificar que **ambos** os módulos são descobertos no `scan()` inicial
- [ ] Todos os controles de ambos os módulos aparecem na lista de `Endereco CC`
- [ ] Os prefixos diferenciam os módulos: `[20]Pot1` vs `[21]Pot1`

### Configuração independente

- [ ] Configurar CC diferente para controles de cada módulo (ex: módulo 0x20 → CC 74, módulo 0x21 → CC 75)
- [ ] Verificar que cada controle envia o CC **correto** e independente
- [ ] Desabilitar um controle do módulo 0x20 — verificar que os controles do 0x21 **continuam funcionando**

### Hot-plug parcial

- [ ] Desconectar **apenas** o módulo 0x21
- [ ] Verificar que os controles do 0x20 **continuam funcionando** normalmente
- [ ] Os controles do 0x21 desaparecem da lista
- [ ] Reconectar o 0x21 — verificar que reaparece com configurações mantidas

### Persistência com múltiplos módulos

- [ ] Configurar CC para controles de ambos os módulos
- [ ] Power cycle do principal
- [ ] Reconectar ambos os escravos
- [ ] Verificar que **todas** as configurações de ambos os módulos foram mantidas

### Limite de controles

- [ ] Se cada escravo tem muitos controles, verificar que o total não ultrapassa 32 (`MAX_TOTAL_CONTROLS`)
- [ ] Se ultrapassar, verificar que o sistema **não trava** (apenas ignora os excedentes)

---

## 🐛 Problemas conhecidos para ficar atento

| Problema | Sintoma | Causa provável |
|---|---|---|
| Controles remotos não aparecem | Lista mostra só locais | Escravo não responde ao `CMD_DESCRIPTOR` ou formato errado |
| CC sempre 0 nos remotos | Potenciômetro move mas CC é 0 | Escravo não responde ao `CMD_READ_VALUES` |
| Configuração não persiste | CC volta para 0 após reboot | Endereço I2C do escravo mudou entre boots |
| UI trava ao conectar escravo | Tela congela | Comunicação I2C bloqueante (escravo não responde, sem timeout) |
| Flood de CC | DAW recebe mensagens sem parar | Zona morta insuficiente para o ruído do ADC remoto |
| Controle remoto com label `???` | Label errado na tela | Label no descritor não está null-terminated ou tem mais de 12 chars |

---

## 📋 Resumo rápido

| Teste | Módulos necessários |
|---|---|
| Descoberta e protocolo | 1 + 1 escravo |
| Configuração e persistência | 1 + 1 escravo |
| Hot-plug e resiliência | 1 + 1 escravo |
| Funcionamento MIDI remoto | 1 + 1 escravo |
| Múltiplos módulos simultâneos | 1 + 2 escravos |
| Hot-plug parcial | 1 + 2 escravos |
| Limite de controles | 1 + 2 escravos (com muitos controles) |
