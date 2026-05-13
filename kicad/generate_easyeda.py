#!/usr/bin/env python3
"""
Gera esquematico no formato EasyEDA (JSON) para o modulo principal
do controlador MIDI modular.

O arquivo gerado pode ser importado no EasyEDA via:
  File -> Open -> EasyEDA Source (JSON)

Execucao:
    python generate_easyeda.py
"""
import json
import os

# Contadores globais para IDs unicos
_gid = [1000]
def gid():
    _gid[0] += 1
    return f"gge{_gid[0]}"


def make_resistor(x, y, ref, value, rotation=0):
    """Gera um resistor no formato EasyEDA."""
    shapes = []
    if rotation == 0:  # horizontal
        # Corpo do resistor
        shapes.append(f"R~{x-15}~{y-5}~2~2~30~10~#8B4513~1~0~none~{gid()}~{rotation}~")
        # Pinos
        shapes.append(f"P~show~0~1~{x-25}~{y}~0~{gid()}~0^^{x-25}~{y}^^M{x-25},{y}h10~#880000^^1~{x-22}~{y-6}~0~{ref}~start~~~#0000FF^^1~{x-22}~{y+8}~0~1~end~~~#0000FF^^0~{x-22}~{y}^^0~M {x-19} {y-3} L {x-22} {y} L {x-19} {y+3}")
        shapes.append(f"P~show~0~2~{x+25}~{y}~180~{gid()}~0^^{x+25}~{y}^^M{x+25},{y}h-10~#880000^^1~{x+22}~{y-6}~0~{value}~end~~~#0000FF^^1~{x+22}~{y+8}~0~2~start~~~#0000FF^^0~{x+22}~{y}^^0~M {x+19} {y-3} L {x+22} {y} L {x+19} {y+3}")
    else:  # vertical
        shapes.append(f"R~{x-5}~{y-15}~2~2~10~30~#8B4513~1~0~none~{gid()}~0~")
        shapes.append(f"P~show~0~1~{x}~{y-25}~270~{gid()}~0^^{x}~{y-25}^^M{x},{y-25}v10~#880000^^1~{x+6}~{y-22}~0~{ref}~start~~~#0000FF^^1~{x-8}~{y-22}~0~1~end~~~#0000FF^^0~{x}~{y-22}^^0~M {x-3} {y-19} L {x} {y-22} L {x+3} {y-19}")
        shapes.append(f"P~show~0~2~{x}~{y+25}~90~{gid()}~0^^{x}~{y+25}^^M{x},{y+25}v-10~#880000^^1~{x+6}~{y+22}~0~{value}~end~~~#0000FF^^1~{x-8}~{y+22}~0~2~start~~~#0000FF^^0~{x}~{y+22}^^0~M {x-3} {y+19} L {x} {y+22} L {x+3} {y+19}")
    return shapes


def make_capacitor(x, y, ref, value, rotation=0):
    """Gera um capacitor no formato EasyEDA."""
    shapes = []
    if rotation == 0:  # vertical (padrao para cap)
        shapes.append(f"L~{x-8}~{y-2}~{x+8}~{y-2}~#0000FF~2~0~none~{gid()}~")
        shapes.append(f"L~{x-8}~{y+2}~{x+8}~{y+2}~#0000FF~2~0~none~{gid()}~")
        shapes.append(f"P~show~0~1~{x}~{y-12}~270~{gid()}~0^^{x}~{y-12}^^M{x},{y-12}v10~#880000^^1~{x+6}~{y-10}~0~{ref}~start~~~#0000FF^^1~{x-8}~{y-10}~0~1~end~~~#0000FF^^0~{x}~{y-9}^^0~M {x-3} {y-6} L {x} {y-9} L {x+3} {y-6}")
        shapes.append(f"P~show~0~2~{x}~{y+12}~90~{gid()}~0^^{x}~{y+12}^^M{x},{y+12}v-10~#880000^^1~{x+6}~{y+10}~0~{value}~end~~~#0000FF^^1~{x-8}~{y+10}~0~2~start~~~#0000FF^^0~{x}~{y+9}^^0~M {x-3} {y+6} L {x} {y+9} L {x+3} {y+6}")
    return shapes


def make_switch(x, y, ref, label):
    """Gera um botao (switch push) no formato EasyEDA."""
    shapes = []
    shapes.append(f"R~{x-10}~{y-8}~2~2~20~16~#333333~1~0~none~{gid()}~0~")
    shapes.append(f"P~show~0~1~{x-20}~{y}~0~{gid()}~0^^{x-20}~{y}^^M{x-20},{y}h10~#880000^^1~{x-17}~{y-10}~0~{ref}~start~~~#0000FF^^1~{x-17}~{y+10}~0~1~end~~~#0000FF^^0~{x-17}~{y}^^0~M {x-14} {y-3} L {x-17} {y} L {x-14} {y+3}")
    shapes.append(f"P~show~0~2~{x+20}~{y}~180~{gid()}~0^^{x+20}~{y}^^M{x+20},{y}h-10~#880000^^1~{x+17}~{y-10}~0~{label}~end~~~#0000FF^^1~{x+17}~{y+10}~0~2~start~~~#0000FF^^0~{x+17}~{y}^^0~M {x+14} {y-3} L {x+17} {y} L {x+14} {y+3}")
    return shapes


def make_wire(x1, y1, x2, y2):
    """Gera um fio (wire)."""
    return f"W~{x1} {y1} {x2} {y2}~#008800~1~0~none~{gid()}~0"


def make_netlabel(x, y, name, rotation=0):
    """Gera um net label."""
    return f"N~{x}~{y}~{rotation}~#FF0000~{name}~{gid()}~default~7pt~comment~{name}~pinmark~0~{x}~{y}~0~end~{gid()}"


def make_netflag(x, y, name, net_type="power"):
    """Gera um net flag (VCC, GND, etc)."""
    if name == "GND":
        return f"F~part_netLabel_gnD~{x}~{y}~270~#000000~GND~{gid()}~0~0~GND~{x}~{y}~0~end~{gid()}"
    else:
        return f"F~part_netLabel_VCC~{x}~{y}~90~#FF0000~{name}~{gid()}~0~0~{name}~{x}~{y}~0~end~{gid()}"


def make_text(x, y, text, size=8, color="#333333"):
    """Gera texto anotativo."""
    return f"T~{x}~{y}~0~{color}~{size}pt~~~~comment~{text}~{gid()}~start~0"


# ============================================================
# CONSTRUCAO DO ESQUEMATICO
# ============================================================

shapes = []

# --- Titulo ---
shapes.append(make_text(50, 30, "MODULO PRINCIPAL - CONTROLADOR MIDI MODULAR", 14, "#000000"))
shapes.append(make_text(50, 50, "ESP32-S3-WROOM-1-N16R8 | Rev 1.0", 10, "#555555"))
shapes.append(make_text(50, 65, "USB MIDI + I2C Modular + MIDI DIN | Sem LEDs | Sem UART debug", 8, "#555555"))

# --- BLOCO: Regulador AMS1117 (simplificado como retangulo com pinos) ---
bx, by = 150, 150
shapes.append(make_text(bx, by-20, "=== REGULADOR 5V -> 3.3V ===", 10, "#CC0000"))
shapes.append(f"R~{bx}~{by}~2~2~80~40~#CC0000~1.5~0~none~{gid()}~0~")
shapes.append(make_text(bx+5, by+15, "U3: AMS1117-3.3", 8, "#000000"))
shapes.append(make_text(bx+5, by+30, "VIN=5V  VOUT=3V3", 7, "#333333"))

# Capacitores do regulador
shapes.extend(make_capacitor(bx-30, by+20, "C1", "10uF"))
shapes.extend(make_capacitor(bx+110, by+10, "C2", "10uF"))
shapes.extend(make_capacitor(bx+140, by+10, "C3", "100nF"))

# Net labels regulador
shapes.append(make_netlabel(bx-10, by-5, "VBUS_5V"))
shapes.append(make_netlabel(bx+85, by-5, "3V3"))

# --- BLOCO: USB-C ---
ux, uy = 150, 280
shapes.append(make_text(ux, uy-20, "=== USB-C (J1) ===", 10, "#006600"))
shapes.append(f"R~{ux}~{uy}~2~2~60~100~#006600~1.5~0~none~{gid()}~0~")
shapes.append(make_text(ux+5, uy+12, "J1", 9, "#000000"))
shapes.append(make_text(ux+5, uy+25, "USB-C", 8, "#000000"))
shapes.append(make_text(ux+5, uy+38, "VBUS", 7, "#FF0000"))
shapes.append(make_text(ux+5, uy+50, "D-", 7, "#0000FF"))
shapes.append(make_text(ux+5, uy+62, "D+", 7, "#0000FF"))
shapes.append(make_text(ux+5, uy+74, "CC1/CC2", 7, "#333333"))
shapes.append(make_text(ux+5, uy+86, "GND", 7, "#000000"))

# Resistores CC1/CC2
shapes.extend(make_resistor(ux+100, uy+74, "R1", "5.1k"))
shapes.extend(make_resistor(ux+100, uy+90, "R2", "5.1k"))
shapes.append(make_text(ux+130, uy+80, "-> GND", 7, "#333333"))
shapes.append(make_text(ux+130, uy+96, "-> GND", 7, "#333333"))

# ESD
shapes.append(f"R~{ux+80}~{uy+40}~2~2~60~30~#880088~1~0~none~{gid()}~0~")
shapes.append(make_text(ux+85, uy+55, "U2: USBLC6-2SC6", 7, "#880088"))
shapes.append(make_text(ux+85, uy+67, "ESD D+/D-", 6, "#880088"))

# Net labels USB
shapes.append(make_netlabel(ux+65, uy+38, "VBUS_5V"))
shapes.append(make_netlabel(ux+145, uy+45, "USB_D-"))
shapes.append(make_netlabel(ux+145, uy+55, "USB_D+"))

# --- BLOCO: Boot/Reset ---
bx2, by2 = 150, 430
shapes.append(make_text(bx2, by2-15, "=== BOOT / RESET ===", 10, "#333333"))
shapes.extend(make_switch(bx2+30, by2+10, "SW1", "RESET"))
shapes.extend(make_switch(bx2+30, by2+40, "SW2", "BOOT"))
shapes.extend(make_resistor(bx2+100, by2+40, "R3", "10k"))
shapes.extend(make_capacitor(bx2+100, by2+10, "C6", "100nF"))
shapes.append(make_text(bx2+5, by2+60, "SW1: EN->GND | SW2: GPIO0->GND", 7, "#333333"))
shapes.append(make_text(bx2+5, by2+72, "R3: 10k pull-up GPIO0->3V3", 7, "#333333"))
shapes.append(make_text(bx2+5, by2+84, "C6: 100nF debounce EN", 7, "#333333"))

# --- BLOCO: Display OLED ---
dx, dy = 550, 150
shapes.append(make_text(dx, dy-20, "=== DISPLAY OLED I2C (J2) ===", 10, "#0000CC"))
shapes.append(f"R~{dx}~{dy}~2~2~80~60~#0000CC~1.5~0~none~{gid()}~0~")
shapes.append(make_text(dx+5, dy+12, "J2: Header 1x4", 8, "#000000"))
shapes.append(make_text(dx+5, dy+25, "1:VCC 2:GND", 7, "#333333"))
shapes.append(make_text(dx+5, dy+37, "3:SDA 4:SCL", 7, "#333333"))
shapes.append(make_text(dx+5, dy+50, "SSD1306 0x3C", 7, "#0000CC"))

# Pull-ups I2C
shapes.extend(make_resistor(dx+110, dy+20, "R4", "4.7k", 90))
shapes.extend(make_resistor(dx+140, dy+20, "R5", "4.7k", 90))
shapes.append(make_text(dx+105, dy+55, "SDA", 6, "#0000CC"))
shapes.append(make_text(dx+135, dy+55, "SCL", 6, "#0000CC"))

# Net labels I2C
shapes.append(make_netlabel(dx+85, dy+30, "I2C_SDA"))
shapes.append(make_netlabel(dx+85, dy+42, "I2C_SCL"))

# --- BLOCO: Barramento Modular I2C ---
mx, my = 550, 260
shapes.append(make_text(mx, my-15, "=== BARRAMENTO MODULAR I2C ===", 10, "#0000CC"))
shapes.append(f"R~{mx}~{my}~2~2~80~40~#0000CC~1~0~none~{gid()}~0~")
shapes.append(make_text(mx+5, my+12, "J3: JST-SH 4p", 7, "#000000"))
shapes.append(make_text(mx+5, my+25, "3V3 GND SDA SCL", 7, "#333333"))
shapes.append(f"R~{mx+100}~{my}~2~2~80~40~#0000CC~1~0~none~{gid()}~0~")
shapes.append(make_text(mx+105, my+12, "J4: JST-SH 4p", 7, "#000000"))
shapes.append(make_text(mx+105, my+25, "3V3 GND SDA SCL", 7, "#333333"))
shapes.append(make_text(mx, my+45, "Mesmo bus I2C: GPIO4(SCL) GPIO5(SDA)", 7, "#0000CC"))

# --- BLOCO: MIDI DIN ---
midx, midy = 550, 350
shapes.append(make_text(midx, midy-15, "=== MIDI DIN ===", 10, "#880000"))

# MIDI OUT
shapes.append(f"R~{midx}~{midy}~2~2~100~60~#880000~1~0~none~{gid()}~0~")
shapes.append(make_text(midx+5, midy+12, "J5: DIN-5 MIDI OUT", 8, "#000000"))
shapes.append(make_text(midx+5, midy+25, "Pin2:GND Pin4:3V3/R6", 7, "#333333"))
shapes.append(make_text(midx+5, midy+37, "Pin5:GPIO9/R7", 7, "#333333"))
shapes.append(make_text(midx+5, midy+50, "R6,R7: 33R serie", 7, "#333333"))

# MIDI IN
shapes.append(f"R~{midx}~{midy+75}~2~2~100~80~#880000~1~0~none~{gid()}~0~")
shapes.append(make_text(midx+5, midy+87, "J6: DIN-5 MIDI IN", 8, "#000000"))
shapes.append(make_text(midx+5, midy+100, "U4: 6N138 optoacoplador", 7, "#333333"))
shapes.append(make_text(midx+5, midy+112, "R8: 220R entrada", 7, "#333333"))
shapes.append(make_text(midx+5, midy+124, "R9: 10k pull-up VO", 7, "#333333"))
shapes.append(make_text(midx+5, midy+136, "D1: 1N4148 protecao", 7, "#333333"))
shapes.append(make_text(midx+5, midy+148, "Saida -> GPIO10", 7, "#CC0000"))

# Net labels MIDI
shapes.append(make_netlabel(midx+105, midy+30, "MIDI_TX"))
shapes.append(make_netlabel(midx+105, midy+120, "MIDI_RX"))

# --- BLOCO: Botoes ---
btx, bty = 550, 530
shapes.append(make_text(btx, bty-15, "=== BOTOES NAVEGACAO ===", 10, "#333333"))
shapes.extend(make_switch(btx+40, bty+10, "SW3", "UP"))
shapes.extend(make_switch(btx+40, bty+35, "SW4", "DOWN"))
shapes.extend(make_switch(btx+40, bty+60, "SW5", "SEL"))
shapes.extend(make_switch(btx+40, bty+85, "SW6", "BACK"))
shapes.append(make_text(btx, bty+105, "GPIO11-14 -> SW -> GND", 7, "#333333"))
shapes.append(make_text(btx, bty+117, "Pull-up interno (INPUT_PULLUP)", 7, "#333333"))

# Net labels botoes
shapes.append(make_netlabel(btx-5, bty+10, "GPIO11"))
shapes.append(make_netlabel(btx-5, bty+35, "GPIO12"))
shapes.append(make_netlabel(btx-5, bty+60, "GPIO13"))
shapes.append(make_netlabel(btx-5, bty+85, "GPIO14"))

# --- BLOCO: Potenciometro ---
px, py = 150, 560
shapes.append(make_text(px, py-15, "=== POTENCIOMETRO ===", 10, "#333333"))
shapes.append(f"R~{px}~{py}~2~2~60~50~#8B4513~1~0~none~{gid()}~0~")
shapes.append(make_text(px+5, py+12, "RV1: 10k linear", 8, "#000000"))
shapes.append(make_text(px+5, py+25, "1:3V3 2:GPIO7 3:GND", 7, "#333333"))
shapes.extend(make_capacitor(px+80, py+25, "C7", "100nF"))
shapes.append(make_text(px+70, py+50, "Filtro wiper", 6, "#333333"))
shapes.append(make_netlabel(px+65, py+10, "POT_GPIO7"))

# --- BLOCO: ESP32-S3 (referencia central) ---
# Nota: O usuario ja tem o simbolo do ESP32-S3 no EasyEDA
# Aqui colocamos apenas a referencia de conexoes
ex, ey = 350, 430
shapes.append(make_text(ex, ey-15, "=== ESP32-S3 (U1) - CONEXOES ===", 10, "#000088"))
shapes.append(f"R~{ex}~{ey}~2~2~160~200~#000088~2~0~none~{gid()}~0~")
shapes.append(make_text(ex+5, ey+15, "U1: ESP32-S3-WROOM-1-N16R8", 9, "#000000"))
shapes.append(make_text(ex+5, ey+30, "GPIO0  = BOOT", 7, "#333333"))
shapes.append(make_text(ex+5, ey+42, "GPIO4  = I2C_SCL", 7, "#0000CC"))
shapes.append(make_text(ex+5, ey+54, "GPIO5  = I2C_SDA", 7, "#0000CC"))
shapes.append(make_text(ex+5, ey+66, "GPIO7  = POT (ADC)", 7, "#8B4513"))
shapes.append(make_text(ex+5, ey+78, "GPIO9  = MIDI_TX", 7, "#880000"))
shapes.append(make_text(ex+5, ey+90, "GPIO10 = MIDI_RX", 7, "#880000"))
shapes.append(make_text(ex+5, ey+102, "GPIO11 = BTN_UP", 7, "#333333"))
shapes.append(make_text(ex+5, ey+114, "GPIO12 = BTN_DOWN", 7, "#333333"))
shapes.append(make_text(ex+5, ey+126, "GPIO13 = BTN_SELECT", 7, "#333333"))
shapes.append(make_text(ex+5, ey+138, "GPIO14 = BTN_BACK", 7, "#333333"))
shapes.append(make_text(ex+5, ey+152, "GPIO19 = USB_D-", 7, "#006600"))
shapes.append(make_text(ex+5, ey+164, "GPIO20 = USB_D+", 7, "#006600"))
shapes.append(make_text(ex+5, ey+178, "EN     = RESET", 7, "#333333"))
shapes.append(make_text(ex+5, ey+190, "IO35/36/37 = PROIBIDOS", 7, "#FF0000"))

# Desacoplamento ESP32
shapes.extend(make_capacitor(ex+170, ey+30, "C4", "100nF"))
shapes.extend(make_capacitor(ex+200, ey+30, "C5", "1uF"))
shapes.append(make_text(ex+165, ey+60, "Bypass", 6, "#333333"))

# --- Wires conectando blocos ---
# VBUS -> Regulador
shapes.append(make_wire(210, 318, 210, 150))
# 3V3 rail
shapes.append(make_wire(230, 150, 230, 130))
shapes.append(make_wire(230, 130, 700, 130))
# I2C bus
shapes.append(make_wire(630, 210, 630, 260))
# MIDI
shapes.append(make_wire(650, 380, 650, 425))

# --- Legenda ---
lx, ly = 750, 430
shapes.append(make_text(lx, ly, "=== RESUMO DE NETS ===", 10, "#000000"))
shapes.append(make_text(lx, ly+18, "VBUS_5V: J1.VBUS -> U3.VIN", 7, "#333333"))
shapes.append(make_text(lx, ly+30, "3V3: U3.OUT -> U1/J2/J3/J4/R3-R6/R9/RV1", 7, "#333333"))
shapes.append(make_text(lx, ly+42, "GND: Comum a todos", 7, "#333333"))
shapes.append(make_text(lx, ly+54, "USB_D-: J1 -> U2 -> U1.GPIO19", 7, "#333333"))
shapes.append(make_text(lx, ly+66, "USB_D+: J1 -> U2 -> U1.GPIO20", 7, "#333333"))
shapes.append(make_text(lx, ly+78, "I2C_SDA: U1.GPIO5 -> R4 -> J2/J3/J4", 7, "#333333"))
shapes.append(make_text(lx, ly+90, "I2C_SCL: U1.GPIO4 -> R5 -> J2/J3/J4", 7, "#333333"))
shapes.append(make_text(lx, ly+102, "MIDI_TX: U1.GPIO9 -> R7 -> J5.Pin5", 7, "#333333"))
shapes.append(make_text(lx, ly+114, "MIDI_RX: U4.VO -> R9 -> U1.GPIO10", 7, "#333333"))
shapes.append(make_text(lx, ly+126, "BOOT: 3V3->R3->GPIO0->SW2->GND", 7, "#333333"))
shapes.append(make_text(lx, ly+138, "RESET: EN->C6->GND | EN->SW1->GND", 7, "#333333"))
shapes.append(make_text(lx, ly+150, "BTN: GPIO11-14 -> SW3-6 -> GND", 7, "#333333"))
shapes.append(make_text(lx, ly+162, "POT: 3V3->RV1.1 | RV1.2->GPIO7 | RV1.3->GND", 7, "#333333"))

# GPIOs livres
shapes.append(make_text(lx, ly+185, "=== GPIOs LIVRES ===", 10, "#006600"))
shapes.append(make_text(lx, ly+200, "1, 2, 3, 6, 8, 15, 16, 17, 18, 21", 7, "#006600"))
shapes.append(make_text(lx, ly+212, "38, 39, 40, 41, 42, 43, 44, 47, 48", 7, "#006600"))


# ============================================================
# MONTAGEM DO JSON EASYEDA
# ============================================================

easyeda_doc = {
    "head": {
        "docType": "2",
        "editorVersion": "6.5.54",
        "x": 400,
        "y": 400,
        "c_para": {},
        "importFlag": 0,
        "uuid": "midi-controller-main-module-schematic",
        "utime": 1749804679
    },
    "canvas": "CA~1000~800~#FFFFFF~yes~#CCCCCC~5~1000~800~line~5~pixel~5~400~400",
    "shape": shapes,
    "BBox": {
        "x": 30,
        "y": 20,
        "width": 950,
        "height": 750
    }
}

# Salvar
output_path = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "modulo-principal-esquematico.json"
)

with open(output_path, "w", encoding="utf-8") as f:
    json.dump(easyeda_doc, f, indent=2, ensure_ascii=False)

print(f"Esquematico EasyEDA gerado: {output_path}")
print(f"Tamanho: {os.path.getsize(output_path)} bytes")
print()
print("Para importar no EasyEDA:")
print("  1. Abra o EasyEDA (easyeda.com)")
print("  2. File -> Open -> EasyEDA Source...")
print("  3. Selecione o arquivo .json gerado")
print("  4. O esquematico aparecera com todos os blocos")
print()
print("NOTA: Este arquivo contem o diagrama de blocos com anotacoes.")
print("Voce precisara substituir os retangulos pelos simbolos reais")
print("da biblioteca LCSC/EasyEDA e fazer as conexoes com wires.")
