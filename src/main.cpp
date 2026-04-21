#include "midi/MidiEngine.h"
#include "config.h"
#include "button/Button.h"
#include "ui/OledApp.h"
#include "ui/Screen.h"
#include "ui/components/TextComponent.h"

// ── Pinos ────────────────────────────────────────────────
#define BUTTON_PIN 8   // BOOT button do ESP32-S3 (ativo em LOW)
#define LED_PIN    0   // LED interno (se disponível)

// ── MIDI ─────────────────────────────────────────────────
MidiEngine engine;

MidiNote doMedio (MIDI_Notes::C(4));
MidiNote miMedio (MIDI_Notes::E(4));
MidiNote solMedio(MIDI_Notes::G(4));

// ── HomeScreen ───────────────────────────────────────────
// Tela inicial que exibe o nome do instrumento e reage ao
// botão BOOT para enviar notas MIDI.
class HomeScreen : public Screen {
public:
    HomeScreen()
        : _title(0, 10, USB_MIDI_DEVICE_NAME, 2)
        , _subtitle(0, 40, "Pressione o botao!", 1)
    {
        addChild(&_title);
        addChild(&_subtitle);
    }

    void handleInput(ButtonEvent event) override {
        if (event == ButtonEvent::PRESSED) {
            engine.sendNoteOn(doMedio);
            Serial.println(">>> BOTÃO PRESSIONADO! Sinal enviado. <<<");
            digitalWrite(LED_PIN, HIGH);
            _subtitle.setText("Nota ON");
            markDirty();
        } else if (event == ButtonEvent::RELEASED) {
            engine.sendNoteOff(doMedio);
            Serial.println("--- Botão solto.");
            digitalWrite(LED_PIN, LOW);
            _subtitle.setText("Pressione o botao!");
            markDirty();
        }
    }

private:
    TextComponent _title;
    TextComponent _subtitle;
};

// ── Instâncias globais ───────────────────────────────────
OledApp    app;
Button     bootButton(BUTTON_PIN, true);  // pull-up, ativo em LOW
HomeScreen homeScreen;

// ── setup() ──────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    engine.begin();

    pinMode(LED_PIN, OUTPUT);

    // Inicializa o framework OLED
    if (!app.begin(DISPLAY_I2C_ADDRESS)) {
        Serial.println("Falha ao inicializar display OLED!");
    }

    // Registra o botão BOOT no framework
    bootButton.begin();
    app.addButton(&bootButton);

    // Empilha a tela inicial no Router
    app.getRouter().push(&homeScreen);

    Serial.println("Sistema iniciado. Pressione o botão!");
}

// ── loop() ───────────────────────────────────────────────
void loop() {
    app.update();
}
