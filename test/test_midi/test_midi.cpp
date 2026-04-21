#include <unity.h>
#include "Arduino.h"
#include "Control_Surface.h"
#include "midi/MidiNote.h"
#include "midi/MidiCC.h"
#include "midi/MidiEngine.h"

void setUp(void) {
    mock::reset();
    mock_midi::reset();
}

void tearDown(void) {}

// ============================================================
// Testes de MidiNote
// ============================================================

void test_midi_note_default_velocity(void) {
    MidiNote note(60);
    TEST_ASSERT_EQUAL(60, note.nota);
    TEST_ASSERT_EQUAL(MIDI_DEFAULT_VELOCITY, note.velocidade);
    TEST_ASSERT_EQUAL(MIDI_DEFAULT_CHANNEL, note.canal);
}

void test_midi_note_custom_values(void) {
    MidiNote note(48, 80, 3);
    TEST_ASSERT_EQUAL(48, note.nota);
    TEST_ASSERT_EQUAL(80, note.velocidade);
    TEST_ASSERT_EQUAL(3, note.canal);
}

void test_midi_note_with_midi_notes_helper(void) {
    MidiNote note(MIDI_Notes::C(4));
    TEST_ASSERT_EQUAL(48, note.nota); // C4 = 12*4 = 48
}

void test_midi_note_e4(void) {
    MidiNote note(MIDI_Notes::E(4));
    TEST_ASSERT_EQUAL(52, note.nota); // E4 = 12*4 + 4 = 52
}

void test_midi_note_g4(void) {
    MidiNote note(MIDI_Notes::G(4));
    TEST_ASSERT_EQUAL(55, note.nota); // G4 = 12*4 + 7 = 55
}

// ============================================================
// Testes de MidiEngine::sendNoteOn
// ============================================================

void test_send_note_on(void) {
    MidiEngine engine;
    MidiNote note(60, 100, 1);

    engine.sendNoteOn(note);

    TEST_ASSERT_EQUAL(1, mock_midi::messageCount);
    TEST_ASSERT_TRUE(mock_midi::lastMessage.isNoteOn);
    TEST_ASSERT_EQUAL(60, mock_midi::lastMessage.note);
    TEST_ASSERT_EQUAL(1, mock_midi::lastMessage.channel);
    TEST_ASSERT_EQUAL(100, mock_midi::lastMessage.velocity);
}

// ============================================================
// Testes de MidiEngine::sendNoteOff
// ============================================================

void test_send_note_off(void) {
    MidiEngine engine;
    MidiNote note(60, 100, 1);

    engine.sendNoteOff(note);

    TEST_ASSERT_EQUAL(1, mock_midi::messageCount);
    TEST_ASSERT_FALSE(mock_midi::lastMessage.isNoteOn);
    TEST_ASSERT_EQUAL(60, mock_midi::lastMessage.note);
    TEST_ASSERT_EQUAL(0, mock_midi::lastMessage.velocity);
}

// ============================================================
// Testes de MidiEngine::sendNoteOnOff
// ============================================================

void test_send_note_on_off(void) {
    MidiEngine engine;
    MidiNote note(60, 100, 1);

    engine.sendNoteOnOff(note, 200);

    // Deve ter enviado 2 mensagens (NoteOn + NoteOff)
    TEST_ASSERT_EQUAL(2, mock_midi::messageCount);
    // Última mensagem deve ser NoteOff
    TEST_ASSERT_FALSE(mock_midi::lastMessage.isNoteOn);
    // delay deve ter sido chamado com 200ms
    TEST_ASSERT_EQUAL(200, mock::delayCalledMs);
}

void test_send_note_on_off_advances_time(void) {
    MidiEngine engine;
    MidiNote note(60, 100, 1);

    uint32_t before = millis();
    engine.sendNoteOnOff(note, 500);
    uint32_t after = millis();

    TEST_ASSERT_EQUAL(500, after - before);
}

// ============================================================
// Testes de múltiplas notas
// ============================================================

void test_multiple_notes(void) {
    MidiEngine engine;
    MidiNote c4(MIDI_Notes::C(4));
    MidiNote e4(MIDI_Notes::E(4));

    engine.sendNoteOn(c4);
    engine.sendNoteOn(e4);

    TEST_ASSERT_EQUAL(2, mock_midi::messageCount);
    // Última mensagem deve ser E4
    TEST_ASSERT_EQUAL(MIDI_Notes::E(4), mock_midi::lastMessage.note);
}

// ============================================================
// Teste de canal diferente
// ============================================================

void test_different_channel(void) {
    MidiEngine engine;
    MidiNote note(60, 127, 10);

    engine.sendNoteOn(note);

    TEST_ASSERT_EQUAL(10, mock_midi::lastMessage.channel);
    TEST_ASSERT_EQUAL(127, mock_midi::lastMessage.velocity);
}

// ============================================================
// Testes de MidiCC
// ============================================================

void test_midi_cc_default_value(void) {
    MidiCC cc(10);
    TEST_ASSERT_EQUAL(10, cc.controlador);
    TEST_ASSERT_EQUAL(0, cc.valor);
    TEST_ASSERT_EQUAL(1, cc.canal);
}

void test_midi_cc_custom_values(void) {
    MidiCC cc(7, 100, 3);
    TEST_ASSERT_EQUAL(7, cc.controlador);
    TEST_ASSERT_EQUAL(100, cc.valor);
    TEST_ASSERT_EQUAL(3, cc.canal);
}

// ============================================================
// Testes de MidiEngine CC
// ============================================================

void test_send_cc(void) {
    MidiEngine engine;
    MidiCC cc(1, 64, 1);

    engine.sendCC(cc);

    TEST_ASSERT_EQUAL(1, mock_midi::lastMessage.controller);
    TEST_ASSERT_EQUAL(64, mock_midi::lastMessage.ccValue);
    TEST_ASSERT_EQUAL(1, mock_midi::lastMessage.channel);
    TEST_ASSERT_TRUE(mock_midi::lastMessage.isCC);
    TEST_ASSERT_EQUAL(1, mock_midi::messageCount);
}

void test_send_cc_different_channel(void) {
    MidiEngine engine;
    MidiCC cc(7, 127, 10);

    engine.sendCC(cc);

    TEST_ASSERT_EQUAL(7, mock_midi::lastMessage.controller);
    TEST_ASSERT_EQUAL(127, mock_midi::lastMessage.ccValue);
    TEST_ASSERT_EQUAL(10, mock_midi::lastMessage.channel);
    TEST_ASSERT_TRUE(mock_midi::lastMessage.isCC);
}

void test_send_cc_max_values(void) {
    MidiEngine engine;
    MidiCC cc(127, 127, 16);

    engine.sendCC(cc);

    TEST_ASSERT_EQUAL(127, mock_midi::lastMessage.controller);
    TEST_ASSERT_EQUAL(127, mock_midi::lastMessage.ccValue);
    TEST_ASSERT_EQUAL(16, mock_midi::lastMessage.channel);
    TEST_ASSERT_TRUE(mock_midi::lastMessage.isCC);
}

void test_send_cc_min_values(void) {
    MidiEngine engine;
    MidiCC cc(0, 0, 1);

    engine.sendCC(cc);

    TEST_ASSERT_EQUAL(0, mock_midi::lastMessage.controller);
    TEST_ASSERT_EQUAL(0, mock_midi::lastMessage.ccValue);
    TEST_ASSERT_EQUAL(1, mock_midi::lastMessage.channel);
    TEST_ASSERT_TRUE(mock_midi::lastMessage.isCC);
}

void test_send_note_then_cc(void) {
    MidiEngine engine;
    MidiNote note(60, 100, 1);
    MidiCC cc(1, 64, 1);

    engine.sendNoteOn(note);
    engine.sendCC(cc);

    TEST_ASSERT_EQUAL(2, mock_midi::messageCount);
    TEST_ASSERT_TRUE(mock_midi::lastMessage.isCC);
}

void test_send_cc_then_note(void) {
    MidiEngine engine;
    MidiCC cc(1, 64, 1);
    MidiNote note(60, 100, 1);

    engine.sendCC(cc);
    engine.sendNoteOn(note);

    TEST_ASSERT_EQUAL(2, mock_midi::messageCount);
    TEST_ASSERT_TRUE(mock_midi::lastMessage.isNoteOn);
}

// ============================================================
// main
// ============================================================

int main(int argc, char** argv) {
    UNITY_BEGIN();

    // MidiNote
    RUN_TEST(test_midi_note_default_velocity);
    RUN_TEST(test_midi_note_custom_values);
    RUN_TEST(test_midi_note_with_midi_notes_helper);
    RUN_TEST(test_midi_note_e4);
    RUN_TEST(test_midi_note_g4);

    // MidiEngine
    RUN_TEST(test_send_note_on);
    RUN_TEST(test_send_note_off);
    RUN_TEST(test_send_note_on_off);
    RUN_TEST(test_send_note_on_off_advances_time);
    RUN_TEST(test_multiple_notes);
    RUN_TEST(test_different_channel);

    // MidiCC
    RUN_TEST(test_midi_cc_default_value);
    RUN_TEST(test_midi_cc_custom_values);

    // MidiEngine CC
    RUN_TEST(test_send_cc);
    RUN_TEST(test_send_cc_different_channel);
    RUN_TEST(test_send_cc_max_values);
    RUN_TEST(test_send_cc_min_values);
    RUN_TEST(test_send_note_then_cc);
    RUN_TEST(test_send_cc_then_note);

    return UNITY_END();
}
