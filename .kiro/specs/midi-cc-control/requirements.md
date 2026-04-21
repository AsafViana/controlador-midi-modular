# Requisitos: MIDI CC Control

## Requisito 1: Struct MidiCC

### Descrição

Criar uma struct `MidiCC` para representar mensagens MIDI Control Change, seguindo o mesmo padrão da `MidiNote` existente.

### Critérios de Aceitação

- 1.1 Given uma instância de `MidiCC` construída com `(controlador, valor, canal)`, When os campos são acessados, Then `controlador`, `valor` e `canal` devem ser iguais aos valores passados no construtor.
- 1.2 Given uma instância de `MidiCC` construída apenas com `controlador`, When os campos `valor` e `canal` são acessados, Then `valor` deve ser `MIDI_DEFAULT_CC_VALUE` (0) e `canal` deve ser `MIDI_DEFAULT_CHANNEL` (1).
- 1.3 Given o arquivo `config.h`, When a constante `MIDI_DEFAULT_CC_VALUE` é verificada, Then ela deve existir com valor `0`.

### Propriedades de Corretude

```
Para todo (c, v, ch) onde c ∈ [0,127], v ∈ [0,127], ch ∈ [1,16]:
  MidiCC(c, v, ch).controlador == c
  MidiCC(c, v, ch).valor == v
  MidiCC(c, v, ch).canal == ch

Para todo c ∈ [0,127]:
  MidiCC(c).valor == MIDI_DEFAULT_CC_VALUE
  MidiCC(c).canal == MIDI_DEFAULT_CHANNEL
```

---

## Requisito 2: MidiEngine::sendCC

### Descrição

Estender o `MidiEngine` com um método `sendCC()` que envia mensagens MIDI Control Change via `USBMIDI_Interface`.

### Critérios de Aceitação

- 2.1 Given um `MidiEngine` inicializado e um `MidiCC(controlador, valor, canal)`, When `sendCC()` é chamado, Then a mensagem capturada no mock deve conter `controller == controlador`, `ccValue == valor` e `channel == canal`.
- 2.2 Given um `MidiEngine` e N chamadas a `sendCC()`, When o `messageCount` é verificado, Then deve ser igual a N.
- 2.3 Given um `MidiEngine` e uma chamada a `sendCC()`, When a `lastMessage` do mock é verificada, Then `isCC` deve ser `true`.

### Propriedades de Corretude

```
Para todo MidiCC cc enviado via sendCC(cc):
  mock_midi::lastMessage.controller == cc.controlador
  mock_midi::lastMessage.ccValue == cc.valor
  mock_midi::lastMessage.channel == cc.canal
  mock_midi::lastMessage.isCC == true

Para toda sequência de N chamadas sendCC():
  mock_midi::messageCount == N (após reset)
```

---

## Requisito 3: Extensão do Sistema de Mocks

### Descrição

Estender o mock de `USBMIDI_Interface` e o namespace `mock_midi` para suportar captura e verificação de mensagens CC nos testes.

### Critérios de Aceitação

- 3.1 Given o mock `USBMIDI_Interface`, When o método `sendControlChange(MIDIAddress, uint8_t)` é chamado, Then a `lastMessage` do `mock_midi` deve ser atualizada com os dados CC.
- 3.2 Given a struct `mock_midi::MidiMessage`, When inspecionada, Then deve conter os campos `controller` (uint8_t), `ccValue` (uint8_t) e `isCC` (bool).
- 3.3 Given uma chamada a `mock_midi::reset()`, When os campos CC são verificados, Then `controller`, `ccValue` devem ser `0` e `isCC` deve ser `false`.

### Propriedades de Corretude

```
Para toda chamada sendControlChange(addr, value):
  mock_midi::lastMessage.controller == addr.note
  mock_midi::lastMessage.ccValue == value
  mock_midi::lastMessage.channel == addr.channel
  mock_midi::lastMessage.isCC == true
  mock_midi::messageCount incrementa em 1

Após mock_midi::reset():
  lastMessage.controller == 0
  lastMessage.ccValue == 0
  lastMessage.isCC == false
```

---

## Requisito 4: Coexistência Note e CC

### Descrição

Garantir que mensagens de nota (NoteOn/NoteOff) e mensagens CC coexistem corretamente no `MidiEngine`, sem interferência mútua.

### Critérios de Aceitação

- 4.1 Given um `MidiEngine`, When `sendNoteOn()` é chamado seguido de `sendCC()`, Then a `lastMessage` deve refletir a mensagem CC e `messageCount` deve ser 2.
- 4.2 Given um `MidiEngine`, When `sendCC()` é chamado seguido de `sendNoteOn()`, Then a `lastMessage` deve refletir a mensagem de nota e `messageCount` deve ser 2.

### Propriedades de Corretude

```
Para qualquer sequência intercalada de M chamadas sendNoteOn/sendNoteOff e N chamadas sendCC:
  mock_midi::messageCount == M + N (após reset)
  mock_midi::lastMessage reflete a última operação executada
```

---

## Requisito 5: Valores Limítrofes

### Descrição

Garantir que o sistema funciona corretamente com valores nos extremos dos intervalos MIDI CC.

### Critérios de Aceitação

- 5.1 Given um `MidiCC(0, 0, 1)`, When `sendCC()` é chamado, Then a mensagem deve conter `controller == 0`, `ccValue == 0`, `channel == 1`.
- 5.2 Given um `MidiCC(127, 127, 16)`, When `sendCC()` é chamado, Then a mensagem deve conter `controller == 127`, `ccValue == 127`, `channel == 16`.
