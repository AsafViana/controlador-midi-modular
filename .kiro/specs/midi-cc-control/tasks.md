# Tarefas: MIDI CC Control

## Tarefa 1: Adicionar constante MIDI_DEFAULT_CC_VALUE ao config.h

- [x] 1.1 Adicionar `#define MIDI_DEFAULT_CC_VALUE 0` ao arquivo `src/config.h` na seção MIDI, após `MIDI_DEFAULT_CHANNEL`
- [x] 1.2 Adicionar `#define MIDI_DEFAULT_CC_VALUE 0` ao arquivo `test/mocks/config.h` na seção MIDI, após `MIDI_DEFAULT_CHANNEL`

## Tarefa 2: Criar struct MidiCC

- [x] 2.1 Criar o arquivo `src/midi/MidiCC.h` com a struct `MidiCC` contendo campos `controlador`, `valor`, `canal` e construtor com defaults de config.h, seguindo o padrão de `MidiNote.h`

## Tarefa 3: Estender o mock de Control_Surface para suportar CC

- [x] 3.1 Adicionar campos `controller` (uint8_t), `ccValue` (uint8_t) e `isCC` (bool) à struct `mock_midi::MidiMessage` em `test/mocks/Control_Surface.h`
- [x] 3.2 Adicionar método `sendControlChange(MIDIAddress addr, uint8_t value)` à classe `USBMIDI_Interface` no mock em `test/mocks/Control_Surface.h`
- [x] 3.3 Atualizar `mock_midi::reset()` em `test/mocks/Control_Surface.cpp` para zerar os novos campos CC (`controller=0`, `ccValue=0`, `isCC=false`)

## Tarefa 4: Estender MidiEngine com método sendCC

- [x] 4.1 Adicionar `#include "MidiCC.h"` e declaração `void sendCC(const MidiCC& cc);` ao arquivo `src/midi/MidiEngine.h`
- [x] 4.2 Implementar `MidiEngine::sendCC()` em `src/midi/MidiEngine.cpp`, chamando `_midi.sendControlChange(MIDIAddress(cc.controlador, Channel(cc.canal)), cc.valor)`

## Tarefa 5: Adicionar testes unitários para MidiCC e sendCC

- [x] 5.1 Adicionar teste `test_midi_cc_default_value` em `test/test_midi/test_midi.cpp`: construir `MidiCC(10)` e verificar que `valor == 0` e `canal == 1`
- [x] 5.2 Adicionar teste `test_midi_cc_custom_values` em `test/test_midi/test_midi.cpp`: construir `MidiCC(7, 100, 3)` e verificar os três campos
- [x] 5.3 Adicionar teste `test_send_cc` em `test/test_midi/test_midi.cpp`: enviar `MidiCC(1, 64, 1)` via `sendCC()` e verificar `lastMessage.controller==1`, `lastMessage.ccValue==64`, `lastMessage.channel==1`, `lastMessage.isCC==true`, `messageCount==1`
- [x] 5.4 Adicionar teste `test_send_cc_different_channel` em `test/test_midi/test_midi.cpp`: enviar `MidiCC(7, 127, 10)` e verificar `channel==10`
- [x] 5.5 Adicionar teste `test_send_cc_max_values` em `test/test_midi/test_midi.cpp`: enviar `MidiCC(127, 127, 16)` e verificar valores máximos
- [x] 5.6 Adicionar teste `test_send_cc_min_values` em `test/test_midi/test_midi.cpp`: enviar `MidiCC(0, 0, 1)` e verificar valores mínimos
- [x] 5.7 Adicionar teste `test_send_note_then_cc` em `test/test_midi/test_midi.cpp`: enviar `sendNoteOn()` seguido de `sendCC()`, verificar `messageCount==2` e `lastMessage.isCC==true`
- [x] 5.8 Adicionar teste `test_send_cc_then_note` em `test/test_midi/test_midi.cpp`: enviar `sendCC()` seguido de `sendNoteOn()`, verificar `messageCount==2` e `lastMessage.isNoteOn==true`
- [x] 5.9 Registrar todos os novos testes no `main()` de `test/test_midi/test_midi.cpp` com `RUN_TEST()`

## Tarefa 6: Compilar e executar testes

- [x] 6.1 Compilar e executar os testes MIDI com `pio test -e native -f test_midi` para verificar que todos os testes passam (existentes e novos)
