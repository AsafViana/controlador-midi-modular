# Checklist de Validação — Controlador MIDI Modular

> [!info] Objetivo
> Mapa do que está sólido, o que precisa de atenção e o que testar antes de considerar o sistema pronto.

---

## ✅ O que já funciona (módulo principal sozinho)

- [x] Leitura dos potenciômetros locais e envio de CC
- [x] Configuração de CC por controle pela tela (`CCMapScreen`)
- [x] Persistência no NVS — sobrevive a reboot
- [x] Navegação na interface (esquema: `SINGLE_CLICK` navega, `DOUBLE_CLICK` confirma, `LONG_PRESS` volta)
- [x] Habilitar/desabilitar controles individuais
- [x] Canal MIDI configurável (1-16)
- [x] Layout do display respeitando header amarelo (Y 0-15) e conteúdo azul (Y 16-63)

---

## ⚠️ Pontos de atenção para expansão I2C

### 1. Protocolo I2C do escravo

O mestre espera um protocolo específico: primeiro lê um **descritor** (número de controles, tipos, labels), depois lê **valores**. O firmware do escravo precisa implementar exatamente esse protocolo. Se o formato dos bytes não bater, o sistema vai ler lixo ou travar a comunicação.

### 2. Timing do I2C

O `periodicScan()` e o `readValues()` rodam no loop principal junto com a UI e a leitura dos controles locais. Se a comunicação I2C demorar (escravo lento, cabo longo, ruído), pode causar **lag na interface** ou perda de leituras.

> [!tip] Recomendação
> Use pull-ups de **4.7kΩ** nas linhas SDA/SCL e mantenha os cabos curtos.

### 3. Hot-plug (conectar/desconectar em uso)

O sistema detecta módulos entrando e saindo, mas o `rebuild()` roda a cada iteração do loop. Se um módulo desconectar **no meio de uma leitura I2C**, o `readValues` pode falhar. O código trata isso (mantém o último valor), mas vale testar desconectar e reconectar fisicamente para ver se recupera limpo.

### 4. Limite de endereços e controles

| Recurso | Limite |
|---|---|
| Módulos escravos | 8 (endereços `0x20` a `0x27`) |
| Controles por módulo | 16 |
| Total de configurações remotas | 128 (8 × 16) |

Se precisar de mais, será necessário expandir o `Storage` e a `UnifiedControlList`.

### 5. Conflito de I2C com o display

O display OLED usa endereço `0x3C` — não conflita com o range `0x20-0x27` dos módulos. Mas se algum escravo tiver endereço fora do range esperado, pode interferir. Garanta que os escravos estejam configurados corretamente.

### 6. Primeiro boot com módulo novo

Quando um módulo é conectado pela primeira vez, o `Storage` não tem configuração salva para ele. O CC default será **0**. O controle remoto vai enviar CC 0 até ser configurado pela tela.

> [!warning] Atenção
> Isso pode ser confuso se você não souber que precisa configurar o CC do módulo novo pela interface.

### 7. Zona morta dos remotos

A zona morta (`ZONA_MORTA = 1`) é a mesma para locais e remotos. Se os potenciômetros do módulo escravo tiverem mais ruído (ADC diferente, cabo longo), pode precisar de um valor maior para remotos.

---

## 🧪 Testes recomendados — Módulo principal

- [ ] Configurar CC de cada potenciômetro pela tela e verificar que o valor **persiste após reboot**
- [ ] Desabilitar um controle e confirmar que ele **para de enviar CC**
- [ ] Mudar o canal MIDI e verificar que **todos os controles usam o novo canal**
- [ ] Navegar por todas as telas sem travamento
- [ ] Verificar que a zona morta está filtrando ruído **sem engolir movimentos reais** dos pots
- [ ] Testar os 3 gestos de botão em cada tela (clique, duplo clique, segurar)

---

## 🧪 Testes recomendados — Com segundo módulo

- [ ] Conectar módulo escravo e verificar que aparece na lista de controles (`CCMapScreen`)
- [ ] Controles remotos aparecem com prefixo `[XX]` (endereço hex)
- [ ] Configurar CC de um controle remoto e verificar persistência após reboot
- [ ] Desconectar o módulo e verificar que o sistema **não trava**
- [ ] Reconectar o módulo e verificar que a **configuração foi mantida**
- [ ] Mover potenciômetro remoto e verificar que o CC é enviado corretamente
- [ ] Testar com cabo I2C de diferentes comprimentos para avaliar estabilidade

---

## 📐 Esquema de navegação atual

| Gesto | Ação |
|---|---|
| **Clique simples** (`SINGLE_CLICK`) | Navegar para baixo / incrementar valor |
| **Duplo clique** (`DOUBLE_CLICK`) | Confirmar / entrar na opção |
| **Segurar** (`LONG_PRESS`) | Voltar / subir na lista (MenuScreen) / decrementar (edição CC) |

---

## 📁 Arquivos-chave do sistema

| Arquivo | Responsabilidade |
|---|---|
| `src/hardware/HardwareMap.h` | Definição dos controles físicos (GPIOs, tipos, CC padrão) |
| `src/storage/Storage.cpp` | Persistência NVS (CC, canal, habilitado, remotos) |
| `src/hardware/ControlReader.cpp` | Leitura automática de controles + envio de CC |
| `src/hardware/UnifiedControlList.cpp` | Lista unificada (locais + remotos) |
| `src/i2c/I2CScanner.cpp` | Descoberta e comunicação com módulos I2C |
| `src/screens/CCMapScreen.cpp` | Tela de configuração de CC por controle |
| `src/config.h` | Constantes do display e layout |
