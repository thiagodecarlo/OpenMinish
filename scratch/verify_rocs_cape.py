#!/usr/bin/env python3
"""
scratch/verify_rocs_cape.py
Generates rocs_cape_showcase.png showcasing:
1. Panel 1: Acrobatic Jump & Dynamic Scaling Shadow (Leaping over ground obstacles)
2. Panel 2: Airborne Gliding over Endless Chasm Pits (Fortress of Winds abyss crossing)
3. Panel 3: Updraft Whirlwind & Double-Jump Flutter (Acrobatic height ascension)
4. Panel 4: Down-Thrust Sword Plunge & Ground Impact Shockwave (Area damage & block break)
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
    font_bold = ImageFont.truetype("arialbd.ttf", 12)
except:
    font_title = font_body = font_small = font_bold = ImageFont.load_default()

def draw_hud(pdraw, ox, oy, title, status_text, banner_text=None, is_gliding=False, is_down_thrust=False):
    # Top HUD background
    pdraw.rectangle([ox, oy, ox + W, oy + 24], fill=(12, 28, 13, 255))
    # Hearts
    for i in range(10):
        hx = ox + 8 + i * 11
        hy = oy + 6
        pdraw.polygon([(hx+4, hy+1), (hx+1, hy+4), (hx+1, hy+6), (hx+4, hy+9),
                       (hx+7, hy+6), (hx+7, hy+4)], fill=(239, 68, 68, 255))
        pdraw.polygon([(hx+7, hy+6), (hx+10, hy+4), (hx+10, hy+1), (hx+7, hy+1)], fill=(239, 68, 68, 255))
    # Rupees
    rx = ox + 128
    pdraw.polygon([(rx+4, oy+4), (rx+8, oy+9), (rx+4, oy+15), (rx, oy+9)], fill=(34, 197, 94, 255))
    pdraw.text((rx + 11, oy + 4), "560", fill=(255, 255, 255, 255), font=font_small)

    # Subweapon Slot B: Roc's Cape
    bx = ox + 168
    pdraw.rectangle([bx, oy + 3, bx + 20, oy + 21], fill=(30, 10, 10, 255), outline=(239, 68, 68, 255))
    pdraw.text((bx + 2, oy + 4), "B", fill=(253, 224, 71, 255), font=font_small)
    # Miniature Cape Icon: Crimson cape with gold brooch and feather trim
    pdraw.polygon([(bx + 9, oy + 7), (bx + 16, oy + 7), (bx + 18, oy + 17), (bx + 8, oy + 18)], fill=(185, 28, 28, 255))
    pdraw.polygon([(bx + 11, oy + 10), (bx + 16, oy + 10), (bx + 17, oy + 16), (bx + 10, oy + 16)], fill=(239, 68, 68, 255))
    pdraw.ellipse([bx + 11, oy + 6, bx + 14, oy + 9], fill=(234, 179, 8, 255))
    pdraw.line([(bx + 8, oy + 18), (bx + 18, oy + 17)], fill=(254, 242, 242, 255), width=2)

    # Subweapon Slot A: Sword
    ax = ox + 194
    pdraw.rectangle([ax, oy + 3, ax + 20, oy + 21], fill=(15, 23, 42, 255), outline=(56, 189, 248, 255))
    pdraw.text((ax + 2, oy + 4), "A", fill=(56, 189, 248, 255), font=font_small)
    pdraw.line([(ax + 10, oy + 17), (ax + 16, oy + 6)], fill=(224, 242, 254, 255), width=2)
    pdraw.ellipse([ax + 9, oy + 16, ax + 11, oy + 18], fill=(234, 179, 8, 255))

    # Room/Action Title in HUD
    pdraw.text((ox + 225, oy + 4), title, fill=(253, 224, 71, 255), font=font_bold)

    # Bottom status bar prompt
    pdraw.rectangle([ox + 16, oy + H - 26, ox + W - 16, oy + H - 6], fill=(30, 5, 5, 240), outline=(239, 68, 68, 255))
    pdraw.text((ox + 26, oy + H - 23), status_text, fill=(254, 240, 138, 255), font=font_small)

    # Celebratory Banner if present
    if banner_text:
        bw = 360
        bx = ox + (W - bw) // 2
        by = oy + 32
        pdraw.rectangle([bx - 4, by - 3, bx + bw + 4, by + 31], fill=(30, 5, 5, 245), outline=(239, 68, 68, 255), width=2)
        pdraw.text((bx + 16, by + 2), banner_text[0], fill=(253, 224, 71, 255), font=font_bold)
        pdraw.text((bx + 12, by + 16), banner_text[1], fill=(254, 226, 226, 255), font=font_small)

def draw_link_ground_shadow(pdraw, lx, ly, z):
    # Dynamic scaling shadow: smaller and more transparent as z increases
    shadow_w = max(6, int(18.0 - (z * 0.45)))
    shadow_h = max(3, int(8.0 - (z * 0.20)))
    alpha = max(40, int(160.0 - (z * 4.5)))
    sx = int(lx - shadow_w // 2)
    sy = int(ly + 10 - shadow_h // 2)
    pdraw.ellipse([sx, sy, sx + shadow_w, sy + shadow_h], fill=(0, 0, 0, alpha))

def draw_link_acrobatic(pdraw, lx, ly, z, dir_right=True, gliding=False, down_thrust=False, fluttering=0):
    # Link's feet position is ly, but body is elevated by z
    by = ly - z

    # 1. Cape behind body
    if gliding:
        # Wing-like wide spread cape billowing in wind
        cape_color = (220, 38, 38, 255)
        cape_light = (239, 68, 68, 255)
        # Left wing
        pdraw.polygon([(lx - 2, by - 6), (lx - 26, by - 14 + fluttering), (lx - 22, by + 4), (lx - 6, by + 2)], fill=cape_color)
        pdraw.polygon([(lx - 2, by - 6), (lx - 24, by - 12 + fluttering), (lx - 20, by + 2), (lx - 6, by + 1)], fill=cape_light)
        # Right wing
        pdraw.polygon([(lx + 6, by - 6), (lx + 28, by - 14 - fluttering), (lx + 24, by + 4), (lx + 8, by + 2)], fill=cape_color)
        pdraw.polygon([(lx + 6, by - 6), (lx + 26, by - 12 - fluttering), (lx + 22, by + 2), (lx + 8, by + 1)], fill=cape_light)
        # White feather trim along edge
        pdraw.line([(lx - 26, by - 14 + fluttering), (lx - 22, by + 4)], fill=(255, 255, 255, 255), width=2)
        pdraw.line([(lx + 28, by - 14 - fluttering), (lx + 24, by + 4)], fill=(255, 255, 255, 255), width=2)
        # Wind feather motes drifting behind
        for i in range(4):
            fx = lx - 20 - i * 8
            fy = by - 8 + int(math.sin(i * 1.5) * 6)
            pdraw.ellipse([fx, fy, fx + 3, fy + 2], fill=(254, 242, 242, 200))
    elif down_thrust:
        # Cape fluttering upwards in extreme vertical plunge
        pdraw.polygon([(lx - 4, by - 8), (lx - 12, by - 26), (lx + 12, by - 26), (lx + 4, by - 8)], fill=(185, 28, 28, 255))
        pdraw.polygon([(lx - 2, by - 8), (lx - 8, by - 24), (lx + 8, by - 24), (lx + 2, by - 8)], fill=(239, 68, 68, 255))
        pdraw.line([(lx - 12, by - 26), (lx + 12, by - 26)], fill=(255, 255, 255, 255), width=2)
    else:
        # Jumping cape tail flowing behind
        cx = lx - 10 if dir_right else lx + 10
        pdraw.polygon([(lx - 2, by - 6), (cx - 12, by + 8), (cx - 4, by + 14), (lx + 4, by - 2)], fill=(220, 38, 38, 255))
        pdraw.polygon([(lx - 2, by - 6), (cx - 10, by + 8), (cx - 4, by + 12), (lx + 2, by - 2)], fill=(239, 68, 68, 255))
        pdraw.line([(cx - 12, by + 8), (cx - 4, by + 14)], fill=(255, 255, 255, 255), width=2)

    # 2. Golden Cape Brooch/Clasp at neckline
    pdraw.ellipse([lx - 2, by - 8, lx + 4, by - 2], fill=(234, 179, 8, 255), outline=(161, 98, 7, 255))

    # 3. Link's Tunic & Body
    tunic_green = (34, 197, 94, 255)
    tunic_shadow = (22, 163, 74, 255)
    tunic_light = (74, 222, 128, 255)
    belt_brown = (146, 64, 14, 255)
    belt_gold = (234, 179, 8, 255)

    if down_thrust:
        # Body tucked in aerodynamically, plunge pose
        pdraw.rectangle([lx - 6, by - 10, lx + 6, by + 4], fill=tunic_green)
        pdraw.rectangle([lx - 5, by - 1, lx + 5, by + 2], fill=belt_brown)
        pdraw.rectangle([lx - 2, by - 1, lx + 2, by + 2], fill=belt_gold)
    else:
        pdraw.rectangle([lx - 6, by - 8, lx + 6, by + 6], fill=tunic_green)
        pdraw.rectangle([lx - 5, by + 1, lx + 5, by + 4], fill=belt_brown)
        pdraw.rectangle([lx - 2, by + 1, lx + 2, by + 4], fill=belt_gold)

    # 4. Ezlo / Green Cap
    cap_green = (22, 163, 74, 255)
    face_skin = (254, 215, 170, 255)
    hair_yellow = (234, 179, 8, 255)

    pdraw.rectangle([lx - 5, by - 17, lx + 5, by - 9], fill=face_skin)
    # Eyes
    eye_x = lx + 1 if dir_right else lx - 3
    pdraw.rectangle([eye_x, by - 14, eye_x + 2, by - 11], fill=(15, 23, 42, 255))
    # Blonde hair tufts
    pdraw.rectangle([lx - 6, by - 15, lx - 4, by - 10], fill=hair_yellow)
    pdraw.rectangle([lx + 4, by - 15, lx + 6, by - 10], fill=hair_yellow)

    # Cap & Ezlo beak
    pdraw.polygon([(lx - 6, by - 17), (lx + 6, by - 17), (lx + (4 if dir_right else -4), by - 24)], fill=cap_green)
    # Ezlo bird eyes and yellow beak
    pdraw.polygon([(lx + (6 if dir_right else -6), by - 21), (lx + (11 if dir_right else -11), by - 20), (lx + (6 if dir_right else -6), by - 18)], fill=(245, 158, 11, 255))
    pdraw.ellipse([lx + (3 if dir_right else -5), by - 23, lx + (6 if dir_right else -2), by - 20], fill=(255, 255, 255, 255))
    pdraw.ellipse([lx + (4 if dir_right else -4), by - 22, lx + (5 if dir_right else -3), by - 21], fill=(0, 0, 0, 255))

    # 5. Arms and Sword
    if down_thrust:
        # Both hands gripping sword pointing straight DOWN
        pdraw.rectangle([lx - 2, by + 4, lx + 3, by + 8], fill=face_skin)
        # Golden crossguard
        pdraw.rectangle([lx - 8, by + 8, lx + 9, by + 11], fill=(234, 179, 8, 255))
        # Gleaming silver blade pointing down
        pdraw.polygon([(lx - 3, by + 11), (lx + 4, by + 11), (lx + 1, by + 28), (lx, by + 28)], fill=(248, 250, 252, 255))
        pdraw.line([(lx, by + 11), (lx, by + 27)], fill=(56, 189, 248, 255), width=1)
        # Motion blur trailing behind blade
        for s in range(3):
            pdraw.line([(lx - 2 + s * 2, by - 4), (lx - 2 + s * 2, by + 10)], fill=(56, 189, 248, 140 - s * 30), width=1)
    else:
        # Relaxed airborne legs & boots
        boot_brown = (120, 53, 15, 255)
        pdraw.rectangle([lx - 5, by + 6, lx - 1, by + 10], fill=face_skin)
        pdraw.rectangle([lx + 1, by + 6, lx + 5, by + 10], fill=face_skin)
        pdraw.rectangle([lx - 6, by + 9, lx - 1, by + 13], fill=boot_brown)
        pdraw.rectangle([lx + 1, by + 9, lx + 6, by + 13], fill=boot_brown)

# ----------------------------------------------------------------------------
# PANEL 1: Salto Acrobático e Sombra Dinâmica (Acrobatic Jump & Scaling Shadow)
# ----------------------------------------------------------------------------
p1 = Image.new("RGBA", (W, H), (40, 80, 45, 255))
d1 = ImageDraw.Draw(p1)

# Hyrule Field grassy terrain with stone path & flower tufts
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (55, 120, 60) if ((x//16 + y//16) % 2 == 0) else (65, 135, 70)
        d1.rectangle([x, y, x + 16, y + 16], fill=c)

# Cobblestone path across the screen
for x in range(0, W, 16):
    d1.rectangle([x, 150, x + 16, 198], fill=(148, 163, 184, 255), outline=(100, 116, 139, 255))

# Wooden fence & bushes that block ground walking
for fx in [140, 160, 180, 200]:
    d1.rectangle([fx, 152, fx + 8, 196], fill=(120, 53, 15, 255), outline=(69, 26, 3, 255))
d1.rectangle([130, 160, 220, 166], fill=(146, 64, 14, 255))
d1.rectangle([130, 180, 220, 186], fill=(146, 64, 14, 255))

# Bushes on sides
for bx, by in [(60, 100), (80, 220), (360, 90), (380, 210), (400, 120)]:
    d1.ellipse([bx, by, bx + 24, by + 24], fill=(21, 128, 61, 255), outline=(22, 101, 52, 255))
    d1.ellipse([bx + 4, by + 4, bx + 16, by + 16], fill=(34, 197, 94, 255))

# Parabolic Jump trajectory curve
jump_points = []
for step in range(25):
    t = step / 24.0
    px_curve = 70 + t * 240
    pz_curve = math.sin(t * math.pi) * 22.0
    jump_points.append((px_curve, 175 - pz_curve))
    if step % 3 == 0:
        d1.ellipse([px_curve - 2, 175 - pz_curve - 2, px_curve + 2, 175 - pz_curve + 2], fill=(253, 224, 71, 180))

# Link mid-air at apex of acrobatic jump (x=175, y=175, z=22.0f)
draw_link_ground_shadow(d1, 175, 175, 22.0)
draw_link_acrobatic(d1, 175, 175, 22.0, dir_right=True, gliding=False)

# Sparkle altitude rings
d1.arc([165, 153 - 22, 185, 165 - 22], 0, 360, fill=(253, 224, 71, 220), width=2)
d1.text((195, 130), "Z = 22.0px (IMPULSO MAXIMO)", fill=(254, 240, 138, 255), font=font_bold)
d1.text((195, 144), "Salto livre sobre cercas e obstaculos!", fill=(224, 242, 254, 255), font=font_small)

# HUD & Banner
draw_hud(d1, 0, 0, "CAMPOS DE HYRULE - SALTO ACROBATICO",
         "CAPA DE ROC [B: PULAR / PLANAR] | [A: ESPADA]",
         banner_text=("CAPA DE ROC (ROC'S CAPE) ADQUIRIDA!",
                      "Salto Acrobatico: Pule, plane sobre abismos e execute o Down-Thrust!"))

canvas.paste(p1, (0, 0))

# ----------------------------------------------------------------------------
# PANEL 2: Voo Planado sobre Abismos (Gliding Traversal over Endless Pits)
# ----------------------------------------------------------------------------
p2 = Image.new("RGBA", (W, H), (20, 25, 38, 255))
d2 = ImageDraw.Draw(p2)

# Fortress of Winds stone floor on Left (x=0..140) and Right (x=340..480)
for y in range(0, H, 16):
    for x in list(range(0, 140, 16)) + list(range(340, W, 16)):
        c = (51, 65, 85) if ((x//16 + y//16) % 2 == 0) else (71, 85, 105)
        d2.rectangle([x, y, x + 16, y + 16], fill=c, outline=(30, 41, 59, 255))

# Massive Endless Pit in Center (x=140..340, y=0..H) -> TILE_FORTRESS_PIT
d2.rectangle([140, 0, 340, H], fill=(5, 8, 17, 255))
# Pit edge shading
d2.rectangle([138, 0, 144, H], fill=(15, 23, 42, 255))
d2.rectangle([336, 0, 342, H], fill=(15, 23, 42, 255))

# Rising wind streaks in abyss
for wy in range(40, H, 30):
    for wx in [180, 220, 260, 300]:
        offset = int(math.sin(wy * 0.2 + wx) * 6)
        d2.line([(wx + offset, wy), (wx + offset, wy - 22)], fill=(56, 189, 248, 160), width=2)
        d2.ellipse([wx + offset - 2, wy - 25, wx + offset + 2, wy - 21], fill=(224, 242, 254, 200))

# Gliding horizontal trajectory line with wind draft
d2.line([(130, 150), (350, 168)], fill=(239, 68, 68, 180), width=2)
for mx in range(150, 330, 24):
    d2.line([(mx, 155), (mx + 12, 157)], fill=(255, 255, 255, 220), width=2)

# Link gliding across abyss: x=240, y=170, z=14.0f
draw_link_ground_shadow(d2, 240, 170, 14.0)
draw_link_acrobatic(d2, 240, 170, 14.0, dir_right=True, gliding=True, fluttering=3)

# Floating wind feather motes & speed lines
d2.text((150, 80), "ABISMO DA FORTALEZA DOS VENTOS", fill=(148, 163, 184, 255), font=font_bold)
d2.text((150, 96), "Decaimento suave clampado: vz = -0.38 px/frame", fill=(56, 189, 248, 255), font=font_small)
d2.text((150, 110), "Velocidade horizontal aumentada em 1.45x!", fill=(254, 240, 138, 255), font=font_small)

# Pit hazard marker
d2.rectangle([210, 240, 290, 260], fill=(30, 10, 10, 220), outline=(239, 68, 68, 255))
d2.text((216, 244), "ABISMO SEM FUNDO", fill=(248, 113, 113, 255), font=font_small)

draw_hud(d2, 0, 0, "FORTALEZA DOS VENTOS - PLANANDO SOBRE O ABISMO",
         "PLANANDO NO AR (CAPA DE ROC) | [A] DOWN-THRUST | [B] MANTER VOO",
         is_gliding=True)

canvas.paste(p2, (W, 0))

# ----------------------------------------------------------------------------
# PANEL 3: Salto Duplo & Catapulta em Redemoinho (Double-Jump & Whirlwind Boost)
# ----------------------------------------------------------------------------
p3 = Image.new("RGBA", (W, H), (15, 25, 45, 255))
d3 = ImageDraw.Draw(p3)

# Cloud Tops sky background
for y in range(0, H, 8):
    c_sky = (20 + y // 10, 35 + y // 8, 70 + y // 6, 255)
    d3.line([(0, y), (W, y)], fill=c_sky)

# Fluffy cloud base at bottom (y=220..320)
for cx, cy in [(40, 240), (120, 230), (200, 245), (280, 235), (360, 240), (440, 230)]:
    d3.ellipse([cx - 50, cy - 30, cx + 50, cy + 30], fill=(224, 242, 254, 255))
    d3.ellipse([cx - 40, cy - 20, cx + 40, cy + 20], fill=(255, 255, 255, 255))

# Updraft Redemoinho / Whirlwind funnel in center (x=240, y=140..250)
for r_y in range(150, 260, 6):
    r_radius = int(8 + (r_y - 150) * 0.45)
    r_shift = int(math.sin(r_y * 0.25) * 8)
    d3.ellipse([240 - r_radius + r_shift, r_y - 4, 240 + r_radius + r_shift, r_y + 4],
               outline=(56, 189, 248, 200), width=2)
    if r_y % 12 == 0:
        d3.arc([240 - r_radius + r_shift, r_y - 5, 240 + r_radius + r_shift, r_y + 5],
               0, 180, fill=(255, 255, 255, 240), width=2)

# High floating cloud altar on right (y=110)
d3.ellipse([340, 100, 460, 130], fill=(241, 245, 249, 255), outline=(203, 213, 225, 255))
# Golden chest on high platform
d3.rectangle([390, 92, 412, 108], fill=(234, 179, 8, 255), outline=(161, 98, 7, 255))
d3.ellipse([399, 98, 403, 102], fill=(254, 240, 138, 255))

# Radiant double-jump burst flare at Link's feet
flare_cx, flare_cy = 240, 140 - 24
for ang in range(0, 360, 30):
    rad = math.radians(ang)
    fx1 = flare_cx + math.cos(rad) * 6
    fy1 = flare_cy + math.sin(rad) * 6
    fx2 = flare_cx + math.cos(rad) * 22
    fy2 = flare_cy + math.sin(rad) * 22
    d3.line([(fx1, fy1), (fx2, fy2)], fill=(253, 224, 71, 240), width=2)
d3.ellipse([flare_cx - 10, flare_cy - 10, flare_cx + 10, flare_cy + 10], fill=(255, 255, 255, 220))

# Link soaring at extreme height (x=240, y=140, z=26.0f)
draw_link_ground_shadow(d3, 240, 240, 26.0)
draw_link_acrobatic(d3, 240, 140, 26.0, dir_right=True, gliding=False)

# Double jump indicator text
d3.rectangle([40, 80, 200, 130], fill=(15, 23, 42, 230), outline=(56, 189, 248, 255))
d3.text((48, 86), "SALTO DUPLO ACROBATICO", fill=(253, 224, 71, 255), font=font_bold)
d3.text((48, 100), "Impulso no apice: +3.6 px/frame", fill=(224, 242, 254, 255), font=font_small)
d3.text((48, 114), "Alcance vertical: 26.0px (max)", fill=(56, 189, 248, 255), font=font_small)

draw_hud(d3, 0, 0, "TOPO DAS NUVENS - SALTO DUPLO E REDEMOINHO",
         "CAPA DE ROC: SALTO DUPLO ATIVADO! | ALCANCE DE ILHAS ALTAS",
         is_gliding=False)

canvas.paste(p3, (0, H))

# ----------------------------------------------------------------------------
# PANEL 4: Down-Thrust Sword Plunge & Impact Shockwave (Ataque Aéreo e Choque)
# ----------------------------------------------------------------------------
p4 = Image.new("RGBA", (W, H), (25, 20, 25, 255))
d4 = ImageDraw.Draw(p4)

# Dungeon stone floor
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (64, 45, 55) if ((x//16 + y//16) % 2 == 0) else (80, 58, 70)
        d4.rectangle([x, y, x + 16, y + 16], fill=c, outline=(40, 25, 35, 255))

# Cracked blocks on sides destroyed by the impact
for cbx, cby in [(150, 180), (310, 180)]:
    # Crumbled stone pieces
    for ox_c, oy_c in [(-8, -6), (6, -8), (-4, 8), (8, 6), (0, 0)]:
        d4.rectangle([cbx + ox_c, cby + oy_c, cbx + ox_c + 8, cby + oy_c + 8],
                     fill=(148, 163, 184, 240), outline=(71, 85, 105, 255))
    d4.text((cbx - 12, cby - 22), "ESTILHACADO!", fill=(248, 113, 113, 255), font=font_small)

# Concentric Shockwave Rings on ground (impact point: x=240, y=190)
ix, iy = 240, 190
for radius in [14, 26, 38]:
    d4.arc([ix - radius, iy - radius // 2, ix + radius, iy + radius // 2],
           0, 360, fill=(239, 68, 68, 220 - radius * 4), width=3)
    d4.arc([ix - radius + 2, iy - (radius // 2) + 1, ix + radius - 2, iy + (radius // 2) - 1],
           0, 360, fill=(253, 224, 71, 240 - radius * 4), width=2)

# Dust & rubble particles bursting outward
for i in range(12):
    p_ang = i * (math.pi / 6.0)
    p_dist = 28 + (i % 3) * 8
    px_p = ix + math.cos(p_ang) * p_dist
    py_p = iy + math.sin(p_ang) * (p_dist * 0.5)
    d4.rectangle([px_p - 2, py_p - 2, px_p + 2, py_p + 2], fill=(254, 240, 138, 255))

# Enemy ChuChus blown back by shockwave
for ex, ey, hp in [(160, 175, "4 HP!"), (320, 175, "4 HP!")]:
    # Stunned Red ChuChu
    d4.ellipse([ex - 10, ey - 14, ex + 10, ey + 4], fill=(220, 38, 38, 255), outline=(153, 27, 27, 255))
    d4.ellipse([ex - 4, ey - 8, ex - 1, ey - 4], fill=(255, 255, 255, 255))
    d4.ellipse([ex + 1, ey - 8, ex + 4, ey - 4], fill=(255, 255, 255, 255))
    # Impact stars
    d4.text((ex - 10, ey - 28), hp, fill=(253, 224, 71, 255), font=font_bold)
    d4.text((ex - 14, ey + 8), "REPELIDO!", fill=(248, 113, 113, 255), font=font_small)

# Link executing Down-Thrust plunge at touchdown (x=240, y=190, z=4.0f)
draw_link_ground_shadow(d4, 240, 190, 4.0)
draw_link_acrobatic(d4, 240, 190, 4.0, dir_right=True, gliding=False, down_thrust=True)

# Screen shake / impact rumble labels
d4.rectangle([40, 80, 220, 132], fill=(30, 5, 5, 240), outline=(239, 68, 68, 255))
d4.text((48, 86), "DOWN-THRUST (GOLPE DO ROC)", fill=(253, 224, 71, 255), font=font_bold)
d4.text((48, 100), "Velocidade vertical de queda: -8.2f px/f", fill=(254, 226, 226, 255), font=font_small)
d4.text((48, 114), "Dano em area: 4 HP | Raio: 34px", fill=(248, 113, 113, 255), font=font_small)

draw_hud(d4, 0, 0, "DOWN-THRUST - IMPACTO E ONDA DE CHOQUE",
         "DOWN-THRUST DESTRUTIVO! ONDA DE CHOQUE DE 34PX | BLOCOS DESTRUIDOS!",
         is_down_thrust=True)

canvas.paste(p4, (W, H))

# ----------------------------------------------------------------------------
# Save showcase image to scratch and artifact directory
# ----------------------------------------------------------------------------
scratch_path = os.path.join("d:\\REPOS\\TLoZ-MC\\scratch", "rocs_cape_showcase.png")
artifact_dir = "C:\\Users\\thiag\\.gemini\\antigravity\\brain\\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
artifact_path = os.path.join(artifact_dir, "rocs_cape_showcase.png")

canvas.save(scratch_path)
print(f"[OK] Saved to scratch: {scratch_path}")

os.makedirs(artifact_dir, exist_ok=True)
shutil.copyfile(scratch_path, artifact_path)
print(f"[OK] Saved to artifacts: {artifact_path}")
