#!/usr/bin/env python3
"""
scratch/verify_veil_clouds.py
Generates veil_falls_cloud_tops_showcase.png showcasing:
1. Scene 0: Veil Falls Base (Slate cliffs, torrential waterfall, foam spray, Grip Ring cliff, Wind Crest & Zeffa)
2. Scene 1: Veil Falls Summit (High peaks, cliffside cave entrance, giant skyward updraft whirlwind)
3. Scene 2: Cloud Tops Lower (Fluffy cloud platforms, Mole Mitts storm cloud digging, updraft catapult flight)
4. Scene 3: Wind Tribe Sanctuary (Wind Tribe elders, 5 Golden Kinstones, Great Tornado to Palace of Winds)
"""

import math
import os
import shutil
from PIL import Image, ImageDraw, ImageFont

W, H = 480, 320
IMG_W, IMG_H = W * 2, H * 2
canvas = Image.new("RGBA", (IMG_W, IMG_H), (10, 15, 24, 255))
draw = ImageDraw.Draw(canvas)

try:
    font_title = ImageFont.truetype("arial.ttf", 15)
    font_body = ImageFont.truetype("arial.ttf", 12)
    font_small = ImageFont.truetype("arial.ttf", 11)
except:
    font_title = font_body = font_small = ImageFont.load_default()

def draw_hud(pdraw, ox, oy, title, status_text, kinstones=0, tornado=False, sub_icon="mitts"):
    # Top HUD
    pdraw.rectangle([ox, oy, ox + W, oy + 24], fill=(15, 23, 42, 255))
    # Hearts
    for i in range(8):
        hx = ox + 8 + i * 12
        hy = oy + 6
        pdraw.polygon([(hx+4, hy+1), (hx+1, hy+4), (hx+1, hy+6), (hx+4, hy+9),
                       (hx+7, hy+6), (hx+7, hy+4)], fill=(239, 68, 68, 255))
        pdraw.polygon([(hx+7, hy+6), (hx+10, hy+4), (hx+10, hy+1), (hx+7, hy+1)], fill=(239, 68, 68, 255))
    # Rupees
    rx = ox + 115
    pdraw.polygon([(rx+4, oy+4), (rx+8, oy+9), (rx+4, oy+15), (rx, oy+9)], fill=(34, 197, 94, 255))
    pdraw.text((rx + 12, oy + 4), "480", fill=(255, 255, 255, 255), font=font_small)

    # Subweapon Slot B
    bx = ox + 165
    pdraw.rectangle([bx, oy + 3, bx + 18, oy + 21], fill=(30, 41, 59, 255), outline=(56, 189, 248, 255))
    pdraw.text((bx + 2, oy + 4), "B", fill=(253, 224, 71, 255), font=font_small)
    if sub_icon == "mitts":
        # Mole Mitts icon (brown claw)
        pdraw.rectangle([bx + 9, oy + 7, bx + 15, oy + 16], fill=(180, 83, 9, 255))
        pdraw.polygon([(bx + 9, oy + 7), (bx + 11, oy + 5), (bx + 13, oy + 5), (bx + 15, oy + 7)], fill=(245, 158, 11, 255))
    elif sub_icon == "ocarina":
        # Ocarina icon
        pdraw.ellipse([bx + 8, oy + 8, bx + 16, oy + 16], fill=(56, 189, 248, 255))

    # Golden Kinstones indicator
    kx = ox + 195
    pdraw.polygon([(kx+4, oy+5), (kx+8, oy+10), (kx+4, oy+15), (kx, oy+10)], fill=(245, 158, 11, 255))
    pdraw.text((kx + 11, oy + 4), f"x{kinstones}/5", fill=(254, 240, 138, 255), font=font_small)

    if tornado:
        tx = ox + 245
        pdraw.text((tx, oy + 4), "[TORNADO ATIVO!]", fill=(56, 189, 248, 255), font=font_small)

    # Room title
    pdraw.text((ox + 270 if not tornado else ox + 355, oy + 4), title, fill=(224, 242, 254, 255), font=font_small)

    # Bottom status bar
    pdraw.rectangle([ox + 20, oy + H - 24, ox + W - 20, oy + H - 6], fill=(15, 23, 42, 240), outline=(56, 189, 248, 255))
    pdraw.text((ox + 30, oy + H - 22), status_text, fill=(224, 242, 254, 255), font=font_small)

# -------------------------------------------------------------
# PANEL 1: Scene 0 - Veil Falls Base (Quedas do Véu - Base)
# -------------------------------------------------------------
p1 = Image.new("RGBA", (W, H), (30, 41, 59, 255))
d1 = ImageDraw.Draw(p1)

# Slate cliff rock background
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (51, 65, 85) if ((x//16 + y//16) % 2 == 0) else (71, 85, 105)
        d1.rectangle([x, y, x + 16, y + 16], fill=c)
        if (x + y) % 32 == 0:
            d1.line([(x, y + 4), (x + 12, y + 12)], fill=(30, 41, 59))

# High mountain rock walls on sides
d1.rectangle([0, 0, 80, H], fill=(30, 41, 59))
d1.rectangle([W - 80, 0, W, H], fill=(30, 41, 59))

# Torrential Waterfall in center (x=160..320)
for y in range(0, 220):
    flow_offset = int(math.sin(y * 0.15) * 4)
    d1.rectangle([170 + flow_offset, y, 310 + flow_offset, y + 1], fill=(56, 189, 248, 230))
    if y % 8 == 0:
        d1.line([(190 + flow_offset, y), (290 + flow_offset, y)], fill=(224, 242, 254, 255), width=2)

# Deep pool basin at the foot of the falls (y=210..270)
d1.rectangle([110, 210, 370, 270], fill=(3, 105, 161, 240))
for i in range(120, 360, 20):
    d1.arc([i, 220, i + 30, 240], 0, 180, fill=(224, 242, 254), width=2)

# Water spray & foam mist particles
for px, py in [(150, 215), (180, 205), (220, 210), (260, 208), (300, 215), (330, 220), (200, 180), (280, 175)]:
    d1.ellipse([px - 8, py - 8, px + 8, py + 8], fill=(224, 242, 254, 180))

# Grip Ring climbable wall face on the right (x=330..390, y=50..220)
d1.rectangle([330, 50, 390, 220], fill=(47, 59, 76), outline=(212, 175, 55), width=2)
for gy in range(60, 210, 18):
    d1.line([(340, gy), (380, gy)], fill=(253, 224, 71), width=2)
d1.text((334, 34), "PAREDAO GRIP RING", fill=(253, 224, 71), font=font_small)

# Grassy riverbank & pathway at bottom (y=270..H)
d1.rectangle([0, 270, W, H], fill=(22, 101, 52))
d1.rectangle([W//2 - 40, 270, W//2 + 40, H], fill=(133, 77, 14)) # dirt path leading south

# Wind Crest Cresta do Vento (pedestal circular on grass, x=130, y=280)
wc_x, wc_y = 130, 290
d1.ellipse([wc_x - 18, wc_y - 10, wc_x + 18, wc_y + 10], fill=(100, 116, 139), outline=(56, 189, 248), width=2)
d1.ellipse([wc_x - 10, wc_y - 5, wc_x + 10, wc_y + 5], fill=(30, 58, 138))
# Zeffa Bird hovering above crest
d1.polygon([(wc_x, wc_y - 32), (wc_x - 14, wc_y - 42), (wc_x + 14, wc_y - 42)], fill=(239, 68, 68))
d1.polygon([(wc_x - 22, wc_y - 38), (wc_x - 10, wc_y - 40), (wc_x - 12, wc_y - 30)], fill=(245, 158, 11))
d1.polygon([(wc_x + 22, wc_y - 38), (wc_x + 10, wc_y - 40), (wc_x + 12, wc_y - 30)], fill=(245, 158, 11))
d1.text((wc_x - 30, wc_y - 56), "ZEFFA & WIND CREST", fill=(56, 189, 248), font=font_small)

# Link at bottom center
lx, ly = 240, 280
d1.rectangle([lx - 6, ly - 10, lx + 6, ly + 6], fill=(34, 197, 94))
d1.rectangle([lx - 4, ly - 16, lx + 4, ly - 10], fill=(254, 215, 170))
d1.polygon([(lx - 5, ly - 16), (lx, ly - 24), (lx + 5, ly - 16)], fill=(34, 197, 94))
# White sword on back
d1.line([(lx + 6, ly - 14), (lx + 10, ly + 2)], fill=(255, 255, 255), width=2)

draw_hud(d1, 0, 0, "VEIL FALLS - BASE", "Base das Quedas: Paredao escalavel (Grip Ring), Cachoeiras e Crista de Vento Zeffa", sub_icon="ocarina")

# -------------------------------------------------------------
# PANEL 2: Scene 1 - Veil Falls Summit (Quedas do Véu - Cume)
# -------------------------------------------------------------
p2 = Image.new("RGBA", (W, H), (15, 23, 42, 255))
d2 = ImageDraw.Draw(p2)

# High mountain summit background with cloudy sky
for y in range(0, 120):
    c = (2 + y // 3, 132 - y // 2, 199 - y // 2)
    d2.line([(0, y), (W, y)], fill=c)

# Mountain peak ridge line
d2.polygon([(0, 140), (100, 100), (220, 130), (340, 90), (480, 120), (480, H), (0, H)], fill=(51, 65, 85))
d2.polygon([(40, 150), (140, 120), (260, 145), (380, 110), (480, 135), (480, H), (0, H)], fill=(71, 85, 105))

# Dark slate platform
d2.rectangle([80, 160, 400, 290], fill=(30, 41, 59), outline=(100, 116, 139), width=2)

# Cave entrance archway on the left (access from base)
d2.rectangle([110, 170, 150, 220], fill=(15, 23, 42), outline=(148, 163, 184), width=3)
d2.ellipse([110, 155, 150, 185], fill=(15, 23, 42), outline=(148, 163, 184), width=3)
d2.text((95, 140), "CAVERNA DO CUME", fill=(224, 242, 254), font=font_small)

# Giant Updraft Whirlwind (Redemoinho Colossal do Cume) in center-right (x=270, y=210)
whirl_x, whirl_y = 280, 210
for r in range(45, 10, -6):
    angle_offset = r * 0.4
    d2.arc([whirl_x - r, whirl_y - r//2, whirl_x + r, whirl_y + r//2], 0, 360, fill=(253, 224, 71, 200), width=3)
    d2.arc([whirl_x - r + 3, whirl_y - r//2 - 4, whirl_x + r - 3, whirl_y + r//2 - 4], 45, 225, fill=(239, 68, 68, 220), width=2)

# Upward swirling vortex wind ribbons shooting up to the sky
for wy in range(whirl_y - 20, 30, -15):
    span = int(18 + (whirl_y - wy) * 0.25)
    d2.line([(whirl_x - span, wy), (whirl_x + span, wy - 8)], fill=(254, 240, 138, 180), width=3)
    d2.line([(whirl_x + span - 5, wy - 4), (whirl_x - span + 5, wy - 12)], fill=(224, 242, 254, 180), width=2)
d2.text((whirl_x - 65, 30), "VORTICE ASCENSIONAL PARA AS NUVENS!", fill=(253, 224, 71), font=font_small)

# Link approaching the summit whirlwind
lx, ly = 200, 220
d2.rectangle([lx - 6, ly - 10, lx + 6, ly + 6], fill=(34, 197, 94))
d2.rectangle([lx - 4, ly - 16, lx + 4, ly - 10], fill=(254, 215, 170))
d2.polygon([(lx - 5, ly - 16), (lx, ly - 24), (lx + 5, ly - 16)], fill=(34, 197, 94))

draw_hud(d2, 0, 0, "VEIL FALLS - CUME", "Cume das Quedas: Grande Redemoinho Colossal que lanca Link ao Topo das Nuvens!", sub_icon="ocarina")

# -------------------------------------------------------------
# PANEL 3: Scene 2 - Cloud Tops Lower (Topo das Nuvens - Baixas)
# -------------------------------------------------------------
p3 = Image.new("RGBA", (W, H), (2, 132, 199, 255)) # Stratosphere cyan blue
d3 = ImageDraw.Draw(p3)

# Distant cloud layers in horizon
for cy in range(40, 100, 15):
    for cx in range(0, W, 60):
        d3.ellipse([cx, cy, cx + 80, cy + 35], fill=(241, 245, 249, 140))

# Cloud Islands (Main platforms)
# Platform 1 (Left): x=40..200, y=140..280
for cx, cy in [(60, 160), (110, 150), (160, 160), (80, 200), (140, 210), (100, 250)]:
    d3.ellipse([cx - 40, cy - 25, cx + 40, cy + 25], fill=(248, 250, 252))
    d3.ellipse([cx - 36, cy - 10, cx + 36, cy + 28], fill=(203, 213, 225))

# Platform 2 (Right): x=280..440, y=130..270
for cx, cy in [(310, 150), (360, 140), (410, 150), (330, 200), (390, 200), (360, 240)]:
    d3.ellipse([cx - 40, cy - 25, cx + 40, cy + 25], fill=(248, 250, 252))
    d3.ellipse([cx - 36, cy - 10, cx + 36, cy + 28], fill=(203, 213, 225))

# Dark Storm Clouds escaváveis com Mole Mitts (blocking path on Platform 2)
for bx, by in [(310, 180), (340, 180), (340, 210), (370, 210)]:
    d3.rectangle([bx, by, bx + 28, by + 28], fill=(100, 116, 139), outline=(71, 85, 105), width=2)
    d3.line([(bx + 4, by + 6), (bx + 24, by + 22)], fill=(51, 65, 85), width=2)
d3.text((305, 160), "NUVENS DE TEMPESTADE (MOLE MITTS)", fill=(254, 240, 138), font=font_small)

# Yellow Updraft Whirlpool on Left platform (x=160, y=190)
d3.ellipse([145, 180, 175, 200], fill=(253, 224, 71, 220), outline=(245, 158, 11), width=2)
d3.arc([148, 182, 172, 198], 0, 360, fill=(239, 68, 68), width=2)

# Red Super-Updraft Whirlpool on Right platform (x=390, y=170)
d3.ellipse([375, 160, 405, 180], fill=(239, 68, 68, 230), outline=(254, 240, 138), width=2)

# Parabolic Jump Arc between platforms
pts = []
for t in range(25):
    prog = t / 24.0
    ax = 160 + prog * (320 - 160)
    ay = 190 - math.sin(prog * math.pi) * 85
    pts.append((ax, ay))
for i in range(len(pts) - 1):
    d3.line([pts[i], pts[i+1]], fill=(253, 224, 71), width=2)

# Link in mid-flight on the trajectory
lx, ly = int(pts[13][0]), int(pts[13][1])
d3.ellipse([lx - 12, ly - 8, lx + 12, ly + 8], fill=(254, 240, 138, 160)) # wind glow
d3.rectangle([lx - 5, ly - 8, lx + 5, ly + 5], fill=(34, 197, 94))
d3.rectangle([lx - 3, ly - 13, lx + 3, ly - 8], fill=(254, 215, 170))
d3.polygon([(lx - 4, ly - 13), (lx, ly - 20), (lx + 4, ly - 13)], fill=(34, 197, 94))

draw_hud(d3, 0, 0, "CLOUD TOPS - NUVENS BAIXAS", "Voo parabolico via Redemoinho de Impulso & Escavacao de tempestade com Mole Mitts", sub_icon="mitts")

# -------------------------------------------------------------
# PANEL 4: Scene 3 - Wind Tribe Sanctuary & Great Tornado
# -------------------------------------------------------------
p4 = Image.new("RGBA", (W, H), (15, 23, 42, 255))
d4 = ImageDraw.Draw(p4)

# Golden celestial sunset stratosphere background
for y in range(0, H):
    r = int(15 + (y / H) * 25)
    g = int(35 + (y / H) * 45)
    b = int(70 + (y / H) * 70)
    d4.line([(0, y), (W, y)], fill=(r, g, b))

# Sacred golden cloud floor of the Sanctuary
for cx, cy in [(100, 220), (180, 210), (240, 205), (300, 210), (380, 220), (140, 260), (240, 265), (340, 260)]:
    d4.ellipse([cx - 65, cy - 35, cx + 65, cy + 35], fill=(254, 243, 199))
    d4.ellipse([cx - 60, cy - 20, cx + 60, cy + 38], fill=(253, 230, 138))

# Wind Tribe Ancient Columns (Pillars)
for col_x in [60, 420]:
    d4.rectangle([col_x - 12, 100, col_x + 12, 240], fill=(226, 232, 240), outline=(148, 163, 184), width=2)
    d4.rectangle([col_x - 18, 90, col_x + 18, 100], fill=(203, 213, 225))
    d4.rectangle([col_x - 18, 235, col_x + 18, 245], fill=(203, 213, 225))

# 5 Golden Kinstone Pedestals arranged in arc
ped_coords = [
    (110, 180),
    (170, 150),
    (240, 135),
    (310, 150),
    (370, 180)
]

for i, (kpx, kpy) in enumerate(ped_coords):
    # Pedestal base
    d4.rectangle([kpx - 14, kpy + 10, kpx + 14, kpy + 26], fill=(148, 163, 184), outline=(100, 116, 139), width=2)
    # Golden Kinstone disc
    d4.ellipse([kpx - 12, kpy - 6, kpx + 12, kpy + 10], fill=(245, 158, 11), outline=(254, 240, 138), width=2)
    # Sacred glow
    d4.arc([kpx - 16, kpy - 10, kpx + 16, kpy + 14], 0, 360, fill=(253, 224, 71), width=2)
    d4.text((kpx - 6, kpy + 30), f"K{i+1}", fill=(254, 240, 138), font=font_small)

# Wind Tribe Elders (Hailey & Gregal in blue/white robes)
for ex, ey, name in [(140, 175, "Hailey"), (340, 175, "Gregal")]:
    d4.rectangle([ex - 7, ey - 14, ex + 7, ey + 10], fill=(2, 132, 199)) # blue robe
    d4.ellipse([ex - 5, ey - 22, ex + 5, ey - 14], fill=(254, 215, 170)) # face
    d4.polygon([(ex - 6, ey - 22), (ex, ey - 30), (ex + 6, ey - 22)], fill=(241, 245, 249)) # white hair/cowl
    d4.text((ex - 14, ey + 12), name, fill=(186, 230, 253), font=font_small)

# Colossal Great Updraft Tornado (Tornado do Palace of Winds) in center
tor_x = 240
for ty in range(200, 30, -6):
    width_span = int(12 + (200 - ty) * 0.35)
    sway = int(math.sin(ty * 0.1) * 8)
    d4.ellipse([tor_x - width_span + sway, ty - 6, tor_x + width_span + sway, ty + 6],
               fill=(56, 189, 248, 160), outline=(254, 240, 138, 220), width=2)

d4.text((tor_x - 110, 24), "GRANDE TORNADO CONVOCADO! (PALACE OF WINDS)", fill=(253, 224, 71), font=font_small)

# Link standing at the center before the tornado
lx, ly = 240, 235
d4.rectangle([lx - 6, ly - 10, lx + 6, ly + 6], fill=(34, 197, 94))
d4.rectangle([lx - 4, ly - 16, lx + 4, ly - 10], fill=(254, 215, 170))
d4.polygon([(lx - 5, ly - 16), (lx, ly - 24), (lx + 5, ly - 16)], fill=(34, 197, 94))

draw_hud(d4, 0, 0, "SANTUARIO DA TRIBO DO VENTO", "5 Kinstones Douradas fundidas com os Anciaos! Grande Tornado para Palace of Winds desperto!", kinstones=5, tornado=True, sub_icon="mitts")

# -------------------------------------------------------------
# Paste 4 panels into 2x2 Grid canvas
# -------------------------------------------------------------
canvas.paste(p1, (0, 0))
canvas.paste(p2, (W, 0))
canvas.paste(p3, (0, H))
canvas.paste(p4, (W, H))

# Border dividers
draw.line([(W, 0), (W, IMG_H)], fill=(56, 189, 248, 255), width=3)
draw.line([(0, H), (IMG_W, H)], fill=(56, 189, 248, 255), width=3)

# Global Title Badge in center
title_box = [IMG_W // 2 - 280, H - 20, IMG_W // 2 + 280, H + 20]
draw.rectangle(title_box, fill=(15, 23, 42, 245), outline=(253, 224, 71, 255), width=2)
draw.text((IMG_W // 2 - 260, H - 10), "VEIL FALLS & CLOUD TOPS - MOUNTAINS, SKY & WIND TRIBE", fill=(253, 224, 71, 255), font=font_title)

# Save artifact
artifact_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
out_path = os.path.join(artifact_dir, "veil_falls_cloud_tops_showcase.png")
canvas.save(out_path, "PNG")
print(f"[SUCCESS] Showcase image successfully generated at: {out_path}")
