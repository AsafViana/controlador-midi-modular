#!/usr/bin/env python3
"""
Gera o esquematico KiCad 7 (.kicad_sch) valido do modulo principal.
Formato S-expression compativel com KiCad 7.0+.

Execucao:
    python generate_schematic.py
"""
import os
import uuid

def uid():
    return str(uuid.uuid4())

OUTPUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "modulo-principal")
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "modulo-principal.kicad_sch")
os.makedirs(OUTPUT_DIR, exist_ok=True)

SHEET_UUID = "e63e39d7-6ac0-4ffd-8aa3-1841a4541b55"


def text_note(text, x, y):
    """Gera uma text note valida no formato KiCad 7."""
    return (
        f'  (text "{text}"\n'
        f'    (at {x} {y} 0)\n'
        f'    (effects\n'
        f'      (font\n'
        f'        (size 1.27 1.27)\n'
        f'      )\n'
        f'    )\n'
        f'    (uuid "{uid()}")\n'
        f'  )\n'
    )


def section_title(text, x, y):
    """Gera um titulo de secao (texto maior)."""
    return (
        f'  (text "{text}"\n'
        f'    (at {x} {y} 0)\n'
        f'    (effects\n'
        f'      (font\n'
        f'        (size 2.54 2.54)\n'
        f'        (bold yes)\n'
        f'      )\n'
        f'    )\n'
        f'    (uuid "{uid()}")\n'
        f'  )\n'
    )


def global_label(name, x, y, shape="bidirectional"):
    """Gera um global label (net global)."""
    return (
        f'  (global_label "{name}"\n'
        f'    (shape {shape})\n'
        f'    (at {x} {y} 0)\n'
        f'    (effects\n'
        f'      (font\n'
        f'        (size 1.27 1.27)\n'
        f'      )\n'
        f'      (justify left)\n'
        f'    )\n'
        f'    (uuid "{uid()}")\n'
        f'    (property "Intersheetrefs" "${{INTERSHEET_REFS}}"\n'
        f'      (at 0 0 0)\n'
        f'      (effects\n'
        f'        (font\n'
        f'          (size 1.27 1.27)\n'
        f'        )\n'
        f'        (hide yes)\n'
        f'      )\n'
        f'    )\n'
        f'  )\n'
    )


# --- Construcao do esquematico ---
lines = []

# Header obrigatorio
lines.append('(kicad_sch\n')
lines.append('  (version 20230121)\n')
lines.append('  (generator "custom_generator")\n')
lines.append('  (generator_version "7.0")\n')
lines.append(f'  (uuid "{SHEET_UUID}")\n')
lines.append('  (paper "A3")\n')
lines.append('  (title_block\n')
lines.append('    (title "Modulo Principal - Controlador MIDI Modular")\n')
lines.append('    (date "2025-06-01")\n')
lines.append('    (rev "1.0")\n')
lines.append('    (comment 1 "ESP32-S3-WROOM-1-N16R8")\n')
lines.append('    (comment 2 "USB MIDI + I2C Modular + MIDI DIN")\n')
lines.append('  )\n')
lines.append('\n')
lines.append('  (lib_symbols\n')
lines.append('  )\n')
lines.append('\n')

# --- Bloco 1: USB-C ---
y_base = 30
lines.append(section_title("BLOCO 1: USB-C (Alimentacao + Dados)", 25, y_base))
lines.append(text_note("J1: USB-C Receptaculo (USB 2.0 Device)", 27, y_base + 7))
lines.append(text_note("R1: 5.1k CC1 -> GND (pull-down USB-C)", 27, y_base + 11))
lines.append(text_note("R2: 5.1k CC2 -> GND (pull-down USB-C)", 27, y_base + 15))
lines.append(text_note("U2: USBLC6-2SC6 (ESD/TVS D+/D-)", 27, y_base + 19))
lines.append(text_note("VBUS (5V) -> U3 regulador", 27, y_base + 23))
lines.append(text_note("D- -> U2 -> GPIO19 (USB_D-)", 27, y_base + 27))
lines.append(text_note("D+ -> U2 -> GPIO20 (USB_D+)", 27, y_base + 31))

# --- Bloco 2: Regulador ---
y_base = 75
lines.append(section_title("BLOCO 2: Regulador 5V -> 3.3V", 25, y_base))
lines.append(text_note("U3: AMS1117-3.3 (LDO 800mA)", 27, y_base + 7))
lines.append(text_note("C1: 10uF ceramico (VIN -> GND)", 27, y_base + 11))
lines.append(text_note("C2: 10uF ceramico (VOUT -> GND)", 27, y_base + 15))
lines.append(text_note("C3: 100nF ceramico (VOUT -> GND)", 27, y_base + 19))
lines.append(text_note("IN: VBUS 5V | OUT: 3V3 rail", 27, y_base + 23))

# --- Bloco 3: ESP32-S3 ---
y_base = 115
lines.append(section_title("BLOCO 3: ESP32-S3-WROOM-1-N16R8", 25, y_base))
lines.append(text_note("U1: ESP32-S3-WROOM-1-N16R8", 27, y_base + 7))
lines.append(text_note("C4: 100nF (3V3 -> GND, bypass)", 27, y_base + 11))
lines.append(text_note("C5: 1uF (3V3 -> GND, bulk)", 27, y_base + 15))
lines.append(text_note("Pinagem:", 27, y_base + 21))
lines.append(text_note("  GPIO0  = BOOT (strapping, pull-up 10k)", 27, y_base + 25))
lines.append(text_note("  EN     = RESET (cap 100nF + botao)", 27, y_base + 29))
lines.append(text_note("  GPIO4  = I2C_SCL", 27, y_base + 33))
lines.append(text_note("  GPIO5  = I2C_SDA", 27, y_base + 37))
lines.append(text_note("  GPIO7  = POT (ADC)", 27, y_base + 41))
lines.append(text_note("  GPIO9  = MIDI_DIN_TX (Serial1)", 27, y_base + 45))
lines.append(text_note("  GPIO10 = MIDI_DIN_RX (Serial1)", 27, y_base + 49))
lines.append(text_note("  GPIO11 = BTN_UP", 27, y_base + 53))
lines.append(text_note("  GPIO12 = BTN_DOWN", 27, y_base + 57))
lines.append(text_note("  GPIO13 = BTN_SELECT", 27, y_base + 61))
lines.append(text_note("  GPIO14 = BTN_BACK", 27, y_base + 65))
lines.append(text_note("  GPIO19 = USB_D-", 27, y_base + 69))
lines.append(text_note("  GPIO20 = USB_D+", 27, y_base + 73))
lines.append(text_note("  IO35/36/37 = PROIBIDOS (PSRAM)", 27, y_base + 77))

# --- Bloco 4: Boot/Reset ---
y_base = 210
lines.append(section_title("BLOCO 4: Boot e Reset", 25, y_base))
lines.append(text_note("SW1: RESET (EN -> GND, normalmente aberto)", 27, y_base + 7))
lines.append(text_note("C6: 100nF (EN -> GND, debounce)", 27, y_base + 11))
lines.append(text_note("SW2: BOOT (GPIO0 -> GND, normalmente aberto)", 27, y_base + 15))
lines.append(text_note("R3: 10k (GPIO0 -> 3V3, pull-up)", 27, y_base + 19))

# --- Bloco 5: Display OLED ---
y_base = 245
lines.append(section_title("BLOCO 5: Display OLED I2C", 25, y_base))
lines.append(text_note("J2: Header 1x4 pinos (VCC GND SDA SCL)", 27, y_base + 7))
lines.append(text_note("R4: 4.7k (SDA -> 3V3, pull-up)", 27, y_base + 11))
lines.append(text_note("R5: 4.7k (SCL -> 3V3, pull-up)", 27, y_base + 15))
lines.append(text_note("Display SSD1306 128x64, endereco 0x3C", 27, y_base + 19))

# --- Bloco 6: Barramento I2C modular ---
y_base = 280
lines.append(section_title("BLOCO 6: Barramento Modular I2C", 25, y_base))
lines.append(text_note("J3: JST-SH 4 pinos (3V3 GND SDA SCL) - Modulo 1", 27, y_base + 7))
lines.append(text_note("J4: JST-SH 4 pinos (3V3 GND SDA SCL) - Modulo 2", 27, y_base + 11))
lines.append(text_note("Mesmo barramento I2C do display (GPIO4/5)", 27, y_base + 15))
lines.append(text_note("Pull-ups compartilhados com R4/R5", 27, y_base + 19))

# --- Bloco 7: MIDI DIN ---
y_base = 30
x_base = 180
lines.append(section_title("BLOCO 7: MIDI DIN", x_base, y_base))
lines.append(text_note("--- MIDI OUT ---", x_base + 2, y_base + 7))
lines.append(text_note("J5: Conector DIN 5 pinos femea", x_base + 2, y_base + 11))
lines.append(text_note("  Pino 2: GND (blindagem)", x_base + 2, y_base + 15))
lines.append(text_note("  Pino 4: 3V3 -> R6 (33R) -> Pino 4", x_base + 2, y_base + 19))
lines.append(text_note("  Pino 5: GPIO9 -> R7 (33R) -> Pino 5", x_base + 2, y_base + 23))
lines.append(text_note("--- MIDI IN ---", x_base + 2, y_base + 30))
lines.append(text_note("J6: Conector DIN 5 pinos femea", x_base + 2, y_base + 34))
lines.append(text_note("  Pino 4 -> R8 (220R) -> U4 pin 2 (anode)", x_base + 2, y_base + 38))
lines.append(text_note("  Pino 5 -> U4 pin 3 (cathode)", x_base + 2, y_base + 42))
lines.append(text_note("  D1: 1N4148 (catodo no anode, protecao reversa)", x_base + 2, y_base + 46))
lines.append(text_note("U4: 6N138 Optoacoplador", x_base + 2, y_base + 52))
lines.append(text_note("  Pin 7 (VCC) -> 3V3", x_base + 2, y_base + 56))
lines.append(text_note("  Pin 5 (GND) -> GND", x_base + 2, y_base + 60))
lines.append(text_note("  Pin 6 (VO) -> R9 (10k -> 3V3) -> GPIO10", x_base + 2, y_base + 64))

# --- Bloco 8: Botoes ---
y_base = 110
lines.append(section_title("BLOCO 8: Botoes de Navegacao", x_base, y_base))
lines.append(text_note("Todos: GPIO -> SW -> GND (pull-up interno)", x_base + 2, y_base + 7))
lines.append(text_note("SW3: GPIO11 (UP)", x_base + 2, y_base + 13))
lines.append(text_note("SW4: GPIO12 (DOWN)", x_base + 2, y_base + 17))
lines.append(text_note("SW5: GPIO13 (SELECT)", x_base + 2, y_base + 21))
lines.append(text_note("SW6: GPIO14 (BACK)", x_base + 2, y_base + 25))

# --- Bloco 9: Potenciometro ---
y_base = 150
lines.append(section_title("BLOCO 9: Potenciometro", x_base, y_base))
lines.append(text_note("RV1: 10k linear", x_base + 2, y_base + 7))
lines.append(text_note("  Pino 1: 3V3", x_base + 2, y_base + 11))
lines.append(text_note("  Pino 2 (wiper): GPIO7 (ADC)", x_base + 2, y_base + 15))
lines.append(text_note("  Pino 3: GND", x_base + 2, y_base + 19))
lines.append(text_note("C7: 100nF (wiper -> GND, filtro)", x_base + 2, y_base + 23))

# --- BOM ---
y_base = 190
lines.append(section_title("BOM - Componentes", x_base, y_base))
lines.append(text_note("U1: ESP32-S3-WROOM-1-N16R8", x_base + 2, y_base + 7))
lines.append(text_note("U2: USBLC6-2SC6 (ESD)", x_base + 2, y_base + 11))
lines.append(text_note("U3: AMS1117-3.3 (LDO)", x_base + 2, y_base + 15))
lines.append(text_note("U4: 6N138 (Optoacoplador)", x_base + 2, y_base + 19))
lines.append(text_note("J1: USB-C Receptaculo", x_base + 2, y_base + 23))
lines.append(text_note("J2: Header 1x4 (OLED)", x_base + 2, y_base + 27))
lines.append(text_note("J3,J4: JST-SH 1x4 (I2C modular)", x_base + 2, y_base + 31))
lines.append(text_note("J5,J6: DIN-5 femea (MIDI)", x_base + 2, y_base + 35))
lines.append(text_note("SW1: Reset | SW2: Boot", x_base + 2, y_base + 39))
lines.append(text_note("SW3-SW6: Navegacao", x_base + 2, y_base + 43))
lines.append(text_note("RV1: Pot 10k linear", x_base + 2, y_base + 47))
lines.append(text_note("R1,R2: 5.1k | R3: 10k | R4,R5: 4.7k", x_base + 2, y_base + 51))
lines.append(text_note("R6,R7: 33R | R8: 220R | R9: 10k", x_base + 2, y_base + 55))
lines.append(text_note("C1,C2: 10uF | C3,C4,C6,C7: 100nF | C5: 1uF", x_base + 2, y_base + 59))
lines.append(text_note("D1: 1N4148", x_base + 2, y_base + 63))

# Global labels para as nets principais
y_base = 270
lines.append(section_title("Nets Globais", x_base, y_base))
lines.append(global_label("3V3", x_base + 2, y_base + 8, "passive"))
lines.append(global_label("GND", x_base + 2, y_base + 14, "passive"))
lines.append(global_label("VBUS_5V", x_base + 2, y_base + 20, "passive"))
lines.append(global_label("USB_D-", x_base + 2, y_base + 26, "bidirectional"))
lines.append(global_label("USB_D+", x_base + 2, y_base + 32, "bidirectional"))
lines.append(global_label("I2C_SDA", x_base + 2, y_base + 38, "bidirectional"))
lines.append(global_label("I2C_SCL", x_base + 2, y_base + 44, "bidirectional"))
lines.append(global_label("MIDI_TX", x_base + 2, y_base + 50, "output"))
lines.append(global_label("MIDI_RX", x_base + 2, y_base + 56, "input"))

# Fechar o arquivo
lines.append(')\n')

content = "".join(lines)

with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
    f.write(content)

print(f"Esquematico gerado: {OUTPUT_FILE}")
print(f"Tamanho: {len(content)} bytes")
print()
print("Abra modulo-principal.kicad_pro no KiCad 7+")
print("O esquematico contem notas organizadas por bloco funcional.")
print("Use como referencia para adicionar os simbolos da biblioteca.")
