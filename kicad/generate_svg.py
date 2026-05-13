#!/usr/bin/env python3
"""
Gera esquematico SVG do modulo principal do controlador MIDI modular.
Cada bloco funcional eh desenhado com componentes, pinos e conexoes.

Execucao:
    python generate_svg.py

Saida: modulo-principal-esquematico.svg
"""

SVG_WIDTH = 1600
SVG_HEIGHT = 1100

lines = []

def start_svg():
    lines.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {SVG_WIDTH} {SVG_HEIGHT}" width="{SVG_WIDTH}" height="{SVG_HEIGHT}" font-family="monospace">')
    lines.append('<style>')
    lines.append('  .title { font-size: 14px; font-weight: bold; fill: #333; }')
    lines.append('  .subtitle { font-size: 11px; fill: #555; }')
    lines.append('  .comp { font-size: 10px; fill: #000; }')
    lines.append('  .pin { font-size: 9px; fill: #0066cc; }')
    lines.append('  .net { font-size: 9px; fill: #cc0000; font-weight: bold; }')
    lines.append('  .note { font-size: 8px; fill: #666; font-style: italic; }')
    lines.append('  .wire { stroke: #006600; stroke-width: 1.5; fill: none; }')
    lines.append('  .bus { stroke: #006600; stroke-width: 3; fill: none; }')
    lines.append('  .box { stroke: #333; stroke-width: 1.5; fill: #f9f9f9; rx: 5; }')
    lines.append('  .ic-box { stroke: #000; stroke-width: 2; fill: #ffffee; }')
    lines.append('  .conn-box { stroke: #000; stroke-width: 1.5; fill: #eeffee; }')
    lines.append('  .pwr-box { stroke: #cc0000; stroke-width: 1.5; fill: #fff0f0; }')
    lines.append('  .dot { fill: #006600; }')
    lines.append('</style>')
    # Background
    lines.append(f'<rect width="{SVG_WIDTH}" height="{SVG_HEIGHT}" fill="white"/>')
    # Title block
    lines.append('<rect x="10" y="10" width="1580" height="50" fill="#f0f0f0" stroke="#333" stroke-width="1"/>')
    lines.append('<text x="20" y="35" class="title" style="font-size:16px">MODULO PRINCIPAL - CONTROLADOR MIDI MODULAR</text>')
    lines.append('<text x="20" y="50" class="subtitle">ESP32-S3-WROOM-1-N16R8 | Rev 1.0 | USB MIDI + I2C Modular + MIDI DIN</text>')
    lines.append('<text x="1200" y="35" class="subtitle">Alimentacao: USB-C 5V -> LDO 3.3V</text>')
    lines.append('<text x="1200" y="50" class="subtitle">Comunicacao modular: I2C (GPIO4/5)</text>')


def end_svg():
    lines.append('</svg>')


def draw_box(x, y, w, h, cls="box", label=""):
    lines.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" class="{cls}"/>')
    if label:
        lines.append(f'<text x="{x+5}" y="{y+14}" class="title">{label}</text>')


def draw_ic(x, y, w, h, ref, value, pin_left=None, pin_right=None):
    """Desenha um IC com pinos a esquerda e direita."""
    lines.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" class="ic-box"/>')
    lines.append(f'<text x="{x + w//2}" y="{y + 14}" text-anchor="middle" class="comp" style="font-weight:bold">{ref}</text>')
    lines.append(f'<text x="{x + w//2}" y="{y + 26}" text-anchor="middle" class="comp">{value}</text>')
    py = y + 40
    if pin_left:
        for pin in pin_left:
            lines.append(f'<text x="{x+4}" y="{py}" class="pin">{pin}</text>')
            lines.append(f'<line x1="{x-10}" y1="{py-3}" x2="{x}" y2="{py-3}" class="wire"/>')
            py += 16
    py = y + 40
    if pin_right:
        for pin in pin_right:
            lines.append(f'<text x="{x+w-4}" y="{py}" text-anchor="end" class="pin">{pin}</text>')
            lines.append(f'<line x1="{x+w}" y1="{py-3}" x2="{x+w+10}" y2="{py-3}" class="wire"/>')
            py += 16


def draw_connector(x, y, ref, pins, w=100, h=None):
    """Desenha um conector com lista de pinos."""
    if h is None:
        h = 20 + len(pins) * 14
    lines.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" class="conn-box"/>')
    lines.append(f'<text x="{x + w//2}" y="{y+14}" text-anchor="middle" class="comp" style="font-weight:bold">{ref}</text>')
    py = y + 30
    for pin in pins:
        lines.append(f'<text x="{x+5}" y="{py}" class="pin">{pin}</text>')
        py += 14


def draw_passive(x, y, ref, value, orient="h"):
    """Desenha um resistor/capacitor simples."""
    if orient == "h":
        lines.append(f'<rect x="{x}" y="{y}" width="40" height="12" fill="#ffe0b0" stroke="#333" stroke-width="1"/>')
        lines.append(f'<text x="{x+20}" y="{y-2}" text-anchor="middle" class="comp">{ref} {value}</text>')
    else:
        lines.append(f'<rect x="{x}" y="{y}" width="12" height="40" fill="#ffe0b0" stroke="#333" stroke-width="1"/>')
        lines.append(f'<text x="{x+18}" y="{y+20}" class="comp">{ref} {value}</text>')


def draw_wire(x1, y1, x2, y2):
    lines.append(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" class="wire"/>')


def draw_net_label(x, y, name):
    lines.append(f'<text x="{x}" y="{y}" class="net">{name}</text>')


def draw_junction(x, y):
    lines.append(f'<circle cx="{x}" cy="{y}" r="3" class="dot"/>')


def draw_text(x, y, text, cls="note"):
    lines.append(f'<text x="{x}" y="{y}" class="{cls}">{text}</text>')


# ============================================================
# DESENHO DO ESQUEMATICO
# ============================================================

start_svg()

# ============ BLOCO 1: USB-C ============
draw_box(20, 70, 300, 200, "box", "BLOCO 1: USB-C + Protecao")

draw_connector(40, 95, "J1: USB-C", [
    "VBUS (5V)",
    "D- (dados)",
    "D+ (dados)",
    "CC1",
    "CC2",
    "GND",
    "SHIELD"
], w=120)

# Resistores CC
draw_passive(180, 150, "R1", "5.1k")
draw_text(180, 175, "CC1 -> GND")
draw_passive(180, 185, "R2", "5.1k")
draw_text(180, 210, "CC2 -> GND")

# ESD
draw_ic(180, 220, 110, 40, "U2", "USBLC6-2SC6",
        pin_left=None, pin_right=None)
draw_text(185, 250, "ESD D+/D- (baixa cap.)")

# ============ BLOCO 2: REGULADOR ============
draw_box(20, 280, 300, 150, "pwr-box", "BLOCO 2: Regulador 5V -> 3.3V")

draw_ic(50, 320, 130, 60, "U3", "AMS1117-3.3",
        pin_left=["VIN (5V)", "GND"],
        pin_right=["VOUT (3V3)"])

draw_passive(200, 310, "C1", "10uF")
draw_text(200, 305, "Entrada")
draw_passive(200, 340, "C2", "10uF")
draw_text(200, 335, "Saida")
draw_passive(200, 370, "C3", "100nF")
draw_text(200, 365, "Saida HF")

draw_net_label(50, 315, "VBUS_5V")
draw_net_label(200, 400, "3V3")

# ============ BLOCO 3: ESP32-S3 ============
draw_box(340, 70, 380, 500, "box", "BLOCO 3: ESP32-S3-WROOM-1-N16R8")

draw_ic(370, 100, 320, 440, "U1", "ESP32-S3-WROOM-1-N16R8",
        pin_left=[
            "3V3 (VDD)",
            "EN (RESET)",
            "GPIO0 (BOOT)",
            "GPIO4 (I2C_SCL)",
            "GPIO5 (I2C_SDA)",
            "GPIO7 (POT/ADC)",
            "GPIO9 (MIDI_TX)",
            "GPIO10 (MIDI_RX)",
            "GPIO11 (BTN_UP)",
            "GPIO12 (BTN_DOWN)",
            "GPIO13 (BTN_SEL)",
            "GPIO14 (BTN_BACK)",
            "GND",
        ],
        pin_right=[
            "GPIO19 (USB_D-)",
            "GPIO20 (USB_D+)",
            "GPIO35 [PSRAM]",
            "GPIO36 [PSRAM]",
            "GPIO37 [PSRAM]",
            "GPIO43 (livre)",
            "GPIO44 (livre)",
            "GPIO48 (livre)",
            "EPAD -> GND",
        ])

# Desacoplamento
draw_passive(380, 545, "C4", "100nF", "h")
draw_passive(440, 545, "C5", "1uF", "h")
draw_text(380, 540, "Bypass 3V3")

# ============ BLOCO 4: BOOT/RESET ============
draw_box(340, 580, 380, 100, "box", "BLOCO 4: Boot e Reset")

draw_text(360, 610, "SW1: RESET -> EN -> GND (N.O.)", "comp")
draw_text(360, 625, "C6: 100nF (EN -> GND debounce)", "comp")
draw_text(360, 645, "SW2: BOOT -> GPIO0 -> GND (N.O.)", "comp")
draw_text(360, 660, "R3: 10k pull-up (GPIO0 -> 3V3)", "comp")

# ============ BLOCO 5: DISPLAY OLED ============
draw_box(740, 70, 250, 130, "box", "BLOCO 5: Display OLED I2C")

draw_connector(760, 95, "J2: Header 1x4", [
    "1: VCC (3V3)",
    "2: GND",
    "3: SDA (GPIO5)",
    "4: SCL (GPIO4)"
], w=140)

draw_passive(920, 110, "R4", "4.7k", "v")
draw_text(940, 125, "SDA pull-up")
draw_passive(920, 155, "R5", "4.7k", "v")
draw_text(940, 170, "SCL pull-up")

draw_text(760, 195, "SSD1306 128x64, addr 0x3C", "note")

# ============ BLOCO 6: BARRAMENTO I2C ============
draw_box(740, 210, 250, 110, "box", "BLOCO 6: Barramento Modular I2C")

draw_connector(760, 235, "J3: JST-SH 4p", [
    "1: 3V3",
    "2: GND",
    "3: SDA",
    "4: SCL"
], w=100)

draw_connector(875, 235, "J4: JST-SH 4p", [
    "1: 3V3",
    "2: GND",
    "3: SDA",
    "4: SCL"
], w=100)

draw_text(760, 315, "Mesmo bus I2C do display (GPIO4/5)", "note")

# ============ BLOCO 7: MIDI DIN ============
draw_box(740, 330, 400, 250, "box", "BLOCO 7: MIDI DIN (5 pinos)")

# MIDI OUT
draw_text(760, 360, "--- MIDI OUT ---", "subtitle")
draw_connector(760, 370, "J5: DIN-5 OUT", [
    "Pin 2: GND",
    "Pin 4: corrente (+)",
    "Pin 5: corrente (-)",
], w=120)

draw_text(900, 390, "3V3 -> R6 (33R) -> Pin 4", "comp")
draw_text(900, 405, "GPIO9 -> R7 (33R) -> Pin 5", "comp")
draw_passive(900, 410, "R6", "33R")
draw_passive(900, 435, "R7", "33R")

# MIDI IN
draw_text(760, 470, "--- MIDI IN ---", "subtitle")
draw_connector(760, 480, "J6: DIN-5 IN", [
    "Pin 4: sinal (+)",
    "Pin 5: sinal (-)",
], w=120)

draw_ic(900, 475, 120, 80, "U4", "6N138",
        pin_left=["Anode (2)", "Cathode (3)"],
        pin_right=["VCC (7)", "VO (6)", "GND (5)"])

draw_text(900, 560, "R8: 220R (entrada)", "comp")
draw_text(900, 573, "R9: 10k (pull-up VO -> 3V3)", "comp")
draw_text(760, 573, "D1: 1N4148 (protecao reversa)", "comp")
draw_text(1030, 510, "-> GPIO10", "net")

# ============ BLOCO 8: BOTOES ============
draw_box(1010, 70, 220, 130, "box", "BLOCO 8: Botoes Navegacao")

draw_text(1030, 100, "SW3: GPIO11 -> GND  [UP]", "comp")
draw_text(1030, 118, "SW4: GPIO12 -> GND  [DOWN]", "comp")
draw_text(1030, 136, "SW5: GPIO13 -> GND  [SELECT]", "comp")
draw_text(1030, 154, "SW6: GPIO14 -> GND  [BACK]", "comp")
draw_text(1030, 175, "Pull-up interno (INPUT_PULLUP)", "note")
draw_text(1030, 188, "Debounce em software", "note")

# ============ BLOCO 9: POTENCIOMETRO ============
draw_box(1010, 210, 220, 100, "box", "BLOCO 9: Potenciometro")

draw_text(1030, 240, "RV1: 10k linear", "comp")
draw_text(1030, 258, "  Pino 1: 3V3", "comp")
draw_text(1030, 274, "  Pino 2 (wiper): GPIO7", "comp")
draw_text(1030, 290, "  Pino 3: GND", "comp")
draw_text(1030, 305, "C7: 100nF (wiper -> GND)", "note")

# ============ CONEXOES PRINCIPAIS (wires) ============
# USB -> ESD -> ESP32
draw_wire(160, 115, 360, 115)  # VBUS
draw_net_label(250, 112, "VBUS_5V")

draw_wire(160, 129, 360, 165)  # D-
draw_net_label(250, 140, "USB_D-")

draw_wire(160, 143, 360, 181)  # D+
draw_net_label(250, 155, "USB_D+")

# Regulador -> ESP32 3V3
draw_wire(320, 355, 360, 137)
draw_net_label(325, 350, "3V3")

# I2C -> Display e Barramento
draw_wire(690, 197, 760, 165)
draw_net_label(700, 190, "I2C_SDA")
draw_wire(690, 213, 760, 179)
draw_net_label(700, 208, "I2C_SCL")

# MIDI TX
draw_wire(690, 245, 900, 395)
draw_net_label(750, 350, "MIDI_TX (GPIO9)")

# Botoes
draw_wire(690, 277, 1010, 100)
draw_wire(690, 293, 1010, 118)
draw_wire(690, 309, 1010, 136)
draw_wire(690, 325, 1010, 154)

# ============ LEGENDA ============
draw_box(1250, 70, 330, 250, "box", "LEGENDA / NOTAS")
draw_text(1270, 100, "Alimentacao: USB-C 5V -> AMS1117 -> 3.3V", "comp")
draw_text(1270, 118, "USB: Nativo GPIO19/20 (MIDI USB)", "comp")
draw_text(1270, 136, "I2C: GPIO4 (SCL) + GPIO5 (SDA)", "comp")
draw_text(1270, 154, "  -> OLED + Modulos perifericos", "comp")
draw_text(1270, 172, "MIDI DIN: GPIO9 (TX) + GPIO10 (RX)", "comp")
draw_text(1270, 190, "Botoes: GPIO11-14 (pull-up interno)", "comp")
draw_text(1270, 208, "Pot: GPIO7 (ADC)", "comp")
draw_text(1270, 230, "PROIBIDOS: IO35, IO36, IO37 (PSRAM)", "net")
draw_text(1270, 248, "BOOT: GPIO0 | RESET: EN", "comp")
draw_text(1270, 270, "Conector modular: JST-SH 4p", "comp")
draw_text(1270, 288, "  (3V3, GND, SDA, SCL)", "comp")
draw_text(1270, 310, "Todos capacitores: ceramico SMD 0805", "note")

# Pinos livres
draw_box(1250, 330, 330, 100, "box", "GPIOs LIVRES (expansao futura)")
draw_text(1270, 360, "1, 2, 3, 6, 8, 15, 16, 17, 18, 21", "comp")
draw_text(1270, 378, "38, 39, 40, 41, 42, 43, 44, 47, 48", "comp")
draw_text(1270, 400, "Total: 19 GPIOs disponiveis", "note")
draw_text(1270, 418, "ADC disponiveis: 1, 2, 3, 6, 8, 15, 16, 17, 18", "note")

end_svg()

# Escreve o SVG
import os
output_path = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "modulo-principal-esquematico.svg"
)

with open(output_path, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))

print(f"SVG gerado: {output_path}")
print(f"Tamanho: {os.path.getsize(output_path)} bytes")
print("Abra no navegador ou qualquer visualizador SVG.")
