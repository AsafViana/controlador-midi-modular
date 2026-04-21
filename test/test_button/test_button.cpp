#include <unity.h>
#include "Arduino.h"
#include "button/Button.h"

// Pin usado nos testes
static const uint8_t TEST_PIN = 10;

// Helper: simula pressionar o botão (activeLow = pin vai LOW)
static void pressButton() {
    mock::setDigitalRead(TEST_PIN, LOW);
}

// Helper: simula soltar o botão (activeLow = pin vai HIGH)
static void releaseButton() {
    mock::setDigitalRead(TEST_PIN, HIGH);
}

// Helper: avança tempo e chama update, retorna último evento não-NONE
static ButtonEvent pumpUntilEvent(Button& btn, uint32_t totalMs, uint32_t stepMs = 1) {
    ButtonEvent last = ButtonEvent::NONE;
    uint32_t elapsed = 0;
    while (elapsed < totalMs) {
        mock::advanceMillis(stepMs);
        elapsed += stepMs;
        ButtonEvent e = btn.update();
        if (e != ButtonEvent::NONE) last = e;
    }
    return last;
}

// Helper: avança tempo chamando update, coleta todos os eventos
static ButtonEvent pumpFor(Button& btn, uint32_t totalMs, uint32_t stepMs = 1) {
    ButtonEvent last = ButtonEvent::NONE;
    uint32_t elapsed = 0;
    while (elapsed < totalMs) {
        mock::advanceMillis(stepMs);
        elapsed += stepMs;
        ButtonEvent e = btn.update();
        if (e != ButtonEvent::NONE) last = e;
    }
    return last;
}

void setUp(void) {
    mock::reset();
    releaseButton(); // botão começa solto
}

void tearDown(void) {}

// ============================================================
// Testes de inicialização
// ============================================================

void test_begin_sets_pullup_for_active_low(void) {
    Button btn(TEST_PIN, true);
    btn.begin();
    TEST_ASSERT_EQUAL(INPUT_PULLUP, mock::pinModes[TEST_PIN]);
}

void test_begin_sets_pulldown_for_active_high(void) {
    Button btn(TEST_PIN, false);
    btn.begin();
    TEST_ASSERT_EQUAL(INPUT_PULLDOWN, mock::pinModes[TEST_PIN]);
}

// ============================================================
// Testes de debounce
// ============================================================

void test_no_event_before_debounce(void) {
    Button btn(TEST_PIN, true);
    btn.begin();

    pressButton();
    // Avança menos que DEBOUNCE_MS
    ButtonEvent evt = pumpFor(btn, Button::DEBOUNCE_MS - 10);
    TEST_ASSERT_EQUAL(ButtonEvent::NONE, evt);
}

void test_pressed_event_after_debounce(void) {
    Button btn(TEST_PIN, true);
    btn.begin();

    pressButton();
    ButtonEvent evt = pumpFor(btn, Button::DEBOUNCE_MS + 5);
    TEST_ASSERT_EQUAL(ButtonEvent::PRESSED, evt);
}

// ============================================================
// Testes de PRESSED e RELEASED
// ============================================================

void test_press_and_release_sequence(void) {
    Button btn(TEST_PIN, true);
    btn.begin();

    // Press
    pressButton();
    ButtonEvent evt = pumpFor(btn, Button::DEBOUNCE_MS + 5);
    TEST_ASSERT_EQUAL(ButtonEvent::PRESSED, evt);

    // Release
    releaseButton();
    evt = pumpFor(btn, Button::DEBOUNCE_MS + 5);
    TEST_ASSERT_EQUAL(ButtonEvent::RELEASED, evt);
}

// ============================================================
// Teste de isHeld e heldDuration
// ============================================================

void test_is_held_while_pressed(void) {
    Button btn(TEST_PIN, true);
    btn.begin();

    TEST_ASSERT_FALSE(btn.isHeld());
    TEST_ASSERT_EQUAL(0, btn.heldDuration());

    pressButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);
    TEST_ASSERT_TRUE(btn.isHeld());
    TEST_ASSERT_TRUE(btn.heldDuration() > 0);

    releaseButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);
    TEST_ASSERT_FALSE(btn.isHeld());
    TEST_ASSERT_EQUAL(0, btn.heldDuration());
}

// ============================================================
// Teste de SINGLE_CLICK
// ============================================================

void test_single_click(void) {
    Button btn(TEST_PIN, true);
    btn.begin();

    // Press
    pressButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);

    // Release (antes de LONG_PRESS)
    releaseButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);

    // Espera a janela de double-click expirar
    ButtonEvent evt = pumpFor(btn, Button::DOUBLE_CLICK_MS + 10);
    TEST_ASSERT_EQUAL(ButtonEvent::SINGLE_CLICK, evt);
}

// ============================================================
// Teste de LONG_PRESS
// ============================================================

void test_long_press(void) {
    Button btn(TEST_PIN, true);
    btn.begin();

    // Press
    pressButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);

    // Mantém pressionado até LONG_PRESS_MS
    ButtonEvent evt = pumpFor(btn, Button::LONG_PRESS_MS);
    TEST_ASSERT_EQUAL(ButtonEvent::LONG_PRESS, evt);
}

void test_long_press_does_not_generate_single_click(void) {
    Button btn(TEST_PIN, true);
    btn.begin();

    // Press
    pressButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);

    // Mantém até long press
    pumpFor(btn, Button::LONG_PRESS_MS);

    // Release
    releaseButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);

    // Espera janela de double-click — não deve gerar SINGLE_CLICK
    ButtonEvent evt = pumpFor(btn, Button::DOUBLE_CLICK_MS + 10);
    TEST_ASSERT_NOT_EQUAL(ButtonEvent::SINGLE_CLICK, evt);
}

// ============================================================
// Teste de DOUBLE_CLICK
// ============================================================

void test_double_click(void) {
    Button btn(TEST_PIN, true);
    btn.begin();

    // Primeiro clique
    pressButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);
    releaseButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);

    // Segundo clique (dentro da janela DOUBLE_CLICK_MS)
    pressButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);
    releaseButton();
    pumpFor(btn, Button::DEBOUNCE_MS + 5);

    // Espera janela expirar
    ButtonEvent evt = pumpFor(btn, Button::DOUBLE_CLICK_MS + 10);
    TEST_ASSERT_EQUAL(ButtonEvent::DOUBLE_CLICK, evt);
}

// ============================================================
// Teste: botão active-high
// ============================================================

void test_active_high_button(void) {
    Button btn(TEST_PIN, false); // active high
    btn.begin();

    // Para active-high, HIGH = pressionado
    mock::setDigitalRead(TEST_PIN, HIGH);
    ButtonEvent evt = pumpFor(btn, Button::DEBOUNCE_MS + 5);
    TEST_ASSERT_EQUAL(ButtonEvent::PRESSED, evt);

    mock::setDigitalRead(TEST_PIN, LOW);
    evt = pumpFor(btn, Button::DEBOUNCE_MS + 5);
    TEST_ASSERT_EQUAL(ButtonEvent::RELEASED, evt);
}

// ============================================================
// Teste: múltiplos updates sem mudança não geram eventos
// ============================================================

void test_no_spurious_events(void) {
    Button btn(TEST_PIN, true);
    btn.begin();

    // Botão solto, vários updates
    for (int i = 0; i < 100; i++) {
        mock::advanceMillis(10);
        TEST_ASSERT_EQUAL(ButtonEvent::NONE, btn.update());
    }
}

// ============================================================
// main
// ============================================================

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_begin_sets_pullup_for_active_low);
    RUN_TEST(test_begin_sets_pulldown_for_active_high);
    RUN_TEST(test_no_event_before_debounce);
    RUN_TEST(test_pressed_event_after_debounce);
    RUN_TEST(test_press_and_release_sequence);
    RUN_TEST(test_is_held_while_pressed);
    RUN_TEST(test_single_click);
    RUN_TEST(test_long_press);
    RUN_TEST(test_long_press_does_not_generate_single_click);
    RUN_TEST(test_double_click);
    RUN_TEST(test_active_high_button);
    RUN_TEST(test_no_spurious_events);

    return UNITY_END();
}
