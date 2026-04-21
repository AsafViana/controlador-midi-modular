#include "midi/MidiEngine.h"

#define BUTTON_PIN 8   // BOOT button do ESP32-S3 (ativo em LOW)
#define LED_PIN    0   // LED interno (se disponível)

unsigned long lastDebounce = 0;
const unsigned long DEBOUNCE_DELAY = 50;
bool lastButtonState = HIGH;
bool buttonState = HIGH;

MidiEngine engine;

MidiNote doMedio (MIDI_Notes::C(4));
MidiNote miMedio (MIDI_Notes::E(4));
MidiNote solMedio(MIDI_Notes::G(4));

void setup() {
    engine.begin();
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // pull-up interno
  pinMode(LED_PIN, OUTPUT);
  Serial.println("Sistema iniciado. Pressione o botão!");
}

void loop() {
    bool reading = digitalRead(BUTTON_PIN);

  // Detecta mudança de estado
  if (reading != lastButtonState) {
    lastDebounce = millis();
  }

  // Aplica debounce
  if ((millis() - lastDebounce) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        // Botão pressionado (LOW porque usa pull-up)
        engine.sendNoteOn(doMedio);
        Serial.println(">>> BOTÃO PRESSIONADO! Sinal enviado. <<<");
        digitalWrite(LED_PIN, HIGH);
      } else {
        // Botão solto
        engine.sendNoteOff(doMedio);
        Serial.println("--- Botão solto.");
        digitalWrite(LED_PIN, LOW);
      }
    }
  }

  lastButtonState = reading;
}