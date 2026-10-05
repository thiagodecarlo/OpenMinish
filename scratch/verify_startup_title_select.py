#!/usr/bin/env python3
"""
scratch/verify_startup_title_select.py
Generates startup_title_select_showcase.png showcasing:
1. Panel 1: Capcom & Nintendo Logo Screens (Clean-room brand boot sequence)
2. Panel 2: Title Screen & "Press Start" (Four Sword in pedestal, radiant aura, flashing prompt)
3. Panel 3: File Select Screen (3 Save Slots with Link stats, hearts, rupees & elements)
4. Panel 4: Name Entry Screen (6-character name display with virtual minish keyboard grid)
"""

import math
import os
import shutil
from PIL import Image, ImageDraw, ImageFont

W, H = 480, 320
IMG_W, IMG_H = W * 2, H * 2
canvas = Image.new("RGBA", (IMG_W, IMG_H), (10, 15, 24, 255))

try:
    font_title = ImageFont.truetype("arial.ttf", 16)
    font_bold  = ImageFont.truetype("arialbd.ttf", 13)
    font_large = ImageFont.truetype("arialbd.ttf", 20)
    font_body  = ImageFont.truetype("arial.ttf", 12)
    font_small = ImageFont.truetype("arial.ttf", 11)
except:
    font_title = font_bold = font_large = font_body = font_small = ImageFont.load_default()

def draw_heart(pdraw, hx, hy, filled=True):
    red   = (239, 68, 68, 255) if filled else (71, 85, 105, 255)
    dark  = (153, 27, 27, 255) if filled else (30, 41, 59, 255)
    white = (254, 226, 226, 255) if filled else (100, 116, 139, 255)

    pdraw.polygon([(hx+4, hy+1), (hx+1, hy+4), (hx+1, hy+6), (hx+4, hy+9),
                   (hx+7, hy+6), (hx+7, hy+4)], fill=red)
    pdraw.polygon([(hx+7, hy+6), (hx+10, hy+4), (hx+10, hy+1), (hx+7, hy+1)], fill=red)
    pdraw.point((hx+2, hy+3), fill=white)
    pdraw.line([(hx+5, hy+2), (hx+5, hy+4)], fill=dark)

# ============================================================================
# PANEL 1: LOGOS DA CAPCOM & NINTENDO
# ============================================================================
p1 = Image.new("RGBA", (W, H), (255, 255, 255, 255))
d1 = ImageDraw.Draw(p1)

# Divide into two display cards: Capcom on left, Nintendo on right
# Left Half: CAPCOM Logo
d1.rectangle([20, 30, 220, 290], fill=(248, 250, 252, 255), outline=(203, 213, 225, 255), width=2)
d1.rectangle([30, 40, 210, 60], fill=(0, 58, 140, 255))
d1.text((45, 42), "BOOT 1: CAPCOM", fill=(255, 255, 255, 255), font=font_bold)

# Large Capcom Logo Text (Blue with yellow outline)
cap_cx, cap_cy = 120, 135
d1.rectangle([38, 115, 202, 160], fill=(0, 58, 140, 255), outline=(255, 204, 0, 255), width=3)
d1.text((54, 126), "C A P C O M", fill=(255, 204, 0, 255), font=font_large)

d1.text((46, 185), "SOUND_CAPCOM_CHIME", fill=(14, 116, 144, 255), font=font_small)
d1.text((36, 205), "(C) CAPCOM CO., LTD. 2004", fill=(100, 116, 139, 255), font=font_small)
d1.text((36, 220), "ALL RIGHTS RESERVED.", fill=(100, 116, 139, 255), font=font_small)
d1.text((44, 255), "[Qualquer tecla para pular]", fill=(148, 163, 184, 255), font=font_small)

# Right Half: NINTENDO Logo
d1.rectangle([260, 30, 460, 290], fill=(248, 250, 252, 255), outline=(203, 213, 225, 255), width=2)
d1.rectangle([270, 40, 450, 60], fill=(230, 0, 18, 255))
d1.text((280, 42), "BOOT 2: NINTENDO", fill=(255, 255, 255, 255), font=font_bold)

# Red rounded pill capsule
nin_x, nin_y = 285, 125
d1.rounded_rectangle([nin_x, nin_y, nin_x + 150, nin_y + 44], radius=22, fill=(230, 0, 18, 255), outline=(180, 0, 14, 255), width=3)
d1.text((nin_x + 24, nin_y + 11), "Nintendo", fill=(255, 255, 255, 255), font=font_large)

d1.text((290, 185), "SOUND_SECRET (Chime)", fill=(230, 0, 18, 255), font=font_small)
d1.text((285, 205), "Licenciado pela Nintendo", fill=(100, 116, 139, 255), font=font_small)
d1.text((285, 220), "Transição suave com Fade-In/Out", fill=(100, 116, 139, 255), font=font_small)
d1.text((284, 255), "[Qualquer tecla para pular]", fill=(148, 163, 184, 255), font=font_small)

# Bottom indicator
d1.rectangle([0, H - 24, W, H], fill=(15, 23, 42, 255))
d1.text((12, H - 18), "FASE DE INICIALIZACAO: LOGOTIPOS COMERCIAIS OFICIAIS (FADE 60 FPS)", fill=(224, 242, 254, 255), font=font_small)

canvas.paste(p1, (0, 0))

# ============================================================================
# PANEL 2: TELA DE TÍTULO & "PRESS START"
# ============================================================================
p2 = Image.new("RGBA", (W, H), (15, 23, 42, 255))
d2 = ImageDraw.Draw(p2)

# Deep Twilight Gradient Background
for y in range(H):
    t = y / float(H)
    r = int(12 + t * 24)
    g = int(20 + t * 30)
    b = int(48 + t * 40)
    d2.line([(0, y), (W, y)], fill=(r, g, b, 255))

# Floating Golden Minish Dust Motes
for i in range(24):
    mx = int((math.sin(i * 1.7) * 0.5 + 0.5) * W)
    my = int((math.cos(i * 2.3) * 0.5 + 0.5) * H)
    mr = 2 + (i % 3)
    d2.ellipse([mx - mr, my - mr, mx + mr, my + mr], fill=(253, 224, 71, 160))

# Stone Pedestal of the Four Sword (Center Altar)
ped_x = W // 2
ped_y = 230

# Stone base tiers
d2.polygon([(ped_x - 110, ped_y + 40), (ped_x + 110, ped_y + 40),
            (ped_x + 85, ped_y + 15), (ped_x - 85, ped_y + 15)], fill=(30, 41, 59, 255))
d2.polygon([(ped_x - 80, ped_y + 15), (ped_x + 80, ped_y + 15),
            (ped_x + 60, ped_y - 10), (ped_x - 60, ped_y - 10)], fill=(51, 65, 85, 255))
d2.polygon([(ped_x - 55, ped_y - 10), (ped_x + 55, ped_y - 10),
            (ped_x + 40, ped_y - 25), (ped_x - 40, ped_y - 25)], fill=(71, 85, 105, 255))

# Four Sword thrust into the stone
# Gleaming silver blade
d2.polygon([(ped_x - 4, ped_y - 25), (ped_x + 4, ped_y - 25),
            (ped_x + 4, ped_y - 70), (ped_x - 4, ped_y - 70)], fill=(248, 250, 252, 255))
d2.line([(ped_x, ped_y - 25), (ped_x, ped_y - 70)], fill=(56, 189, 248, 255), width=2)

# Golden winged crossguard
d2.polygon([(ped_x - 22, ped_y - 70), (ped_x + 22, ped_y - 70),
            (ped_x + 16, ped_y - 76), (ped_x - 16, ped_y - 76)], fill=(251, 191, 36, 255))
# Cyan gem in center
d2.ellipse([ped_x - 4, ped_y - 75, ped_x + 4, ped_y - 67], fill=(6, 182, 212, 255), outline=(255, 255, 255, 255))
# Grip and golden pommel
d2.rectangle([ped_x - 3, ped_y - 90, ped_x + 3, ped_y - 76], fill=(21, 128, 61, 255))
d2.ellipse([ped_x - 5, ped_y - 96, ped_x + 5, ped_y - 88], fill=(251, 191, 36, 255))

# Radiant Light Ring Pulsing from Blade Base
for r_glow in [14, 26, 38]:
    d2.ellipse([ped_x - r_glow, ped_y - 25 - r_glow//2, ped_x + r_glow, ped_y - 25 + r_glow//2],
               outline=(253, 224, 71, max(30, 220 - r_glow * 5)), width=2)

# Title Logo: THE LEGEND OF ZELDA: The Minish Cap
d2.text((ped_x - 72, 30), "THE LEGEND OF", fill=(251, 191, 36, 255), font=font_bold)

# Monumental ZELDA
d2.text((ped_x - 100, 48), "Z E L D A", fill=(253, 224, 71, 255), font=font_large)
d2.text((ped_x - 98, 48), "Z E L D A", fill=(220, 38, 38, 255), font=font_large)

# The Minish Cap Green Banner
d2.rectangle([ped_x - 85, 82, ped_x + 85, 106], fill=(5, 46, 22, 235), outline=(34, 197, 94, 255), width=2)
d2.text((ped_x - 68, 86), "The Minish Cap", fill=(220, 252, 231, 255), font=font_bold)

# Flashing "PRESS START" Box
d2.rectangle([ped_x - 70, 155, ped_x + 70, 180], fill=(15, 23, 42, 240), outline=(253, 224, 71, 255), width=2)
d2.text((ped_x - 52, 160), "PRESS START", fill=(254, 240, 138, 255), font=font_bold)

# BGM & Sound note
d2.text((12, H - 20), "BGM: TITLE THEME (CHIPTUNE) | [START]: SOUND_TITLE_SWORD & FLASH", fill=(224, 242, 254, 255), font=font_small)

canvas.paste(p2, (W, 0))

# ============================================================================
# PANEL 3: TELA DE SELEÇÃO DE ARQUIVO (FILE SELECT)
# ============================================================================
p3 = Image.new("RGBA", (W, H), (15, 23, 42, 255))
d3 = ImageDraw.Draw(p3)

# Royal Dark Tapestry Texture
for y in range(0, H, 4):
    c_tap = (15, 23, 42, 255) if (y % 8 == 0) else (30, 41, 59, 255)
    d3.line([(0, y), (W, y)], fill=c_tap)

# Ornate Header
d3.rectangle([80, 10, W - 80, 36], fill=(2, 6, 23, 240), outline=(251, 191, 36, 255), width=2)
d3.text((135, 15), "SELECAO DE ARQUIVO (CHOOSE A FILE)", fill=(253, 224, 71, 255), font=font_bold)

# 3 File Slot Cards
slots_data = [
    { "num": 1, "name": "LINK", "hearts": 14, "max": 14, "rup": 560, "active": True, "elements": [True, True, True, False] },
    { "num": 2, "name": "ZELDA", "hearts": 8,  "max": 10, "rup": 220, "active": True, "elements": [True, True, False, False] },
    { "num": 3, "name": "- NO DATA -", "hearts": 3, "max": 3, "rup": 0, "active": False, "elements": [] }
]

for idx, s in enumerate(slots_data):
    card_y = 52 + idx * 72
    is_selected = (idx == 0)

    # Card outline & background
    border_color = (251, 191, 36, 255) if is_selected else (71, 85, 105, 255)
    card_fill    = (30, 41, 59, 255) if is_selected else (15, 23, 42, 240)
    d3.rectangle([45, card_y, W - 45, card_y + 60], fill=card_fill, outline=border_color, width=2 if is_selected else 1)

    # Bouncing cursor on selected slot
    if is_selected:
        draw_heart(d3, 22, card_y + 24, filled=True)
        d3.polygon([(36, card_y + 28), (42, card_y + 31), (36, card_y + 34)], fill=(251, 191, 36, 255))

    # File number pill
    d3.rectangle([55, card_y + 14, 95, card_y + 46], fill=(15, 23, 42, 255), outline=(251, 191, 36, 255))
    d3.text((64, card_y + 22), f"Slot {s['num']}", fill=(253, 224, 71, 255), font=font_bold)

    # Slot details
    if s["active"]:
        # Name
        d3.text((110, card_y + 12), s["name"], fill=(255, 255, 255, 255), font=font_bold)
        # Rupees
        d3.polygon([(110, card_y + 38), (114, card_y + 33), (118, card_y + 38), (114, card_y + 43)], fill=(34, 197, 94, 255))
        d3.text((122, card_y + 32), f"{s['rup']} Rupees", fill=(134, 239, 172, 255), font=font_small)

        # Heart containers
        for h in range(s["max"]):
            hx = 205 + (h % 8) * 13
            hy = card_y + 12 + (h // 8) * 14
            draw_heart(d3, hx, hy, filled=(h < s["hearts"]))

        # Conquered Element Gems
        d3.text((325, card_y + 12), "Elementos:", fill=(148, 163, 184, 255), font=font_small)
        elem_colors = [(34, 197, 94, 255), (239, 68, 68, 255), (56, 189, 248, 255), (251, 191, 36, 255)]
        elem_names = ["Terra", "Fogo", "Agua", "Vento"]
        for e_i in range(4):
            ex = 325 + e_i * 24
            ey = card_y + 28
            active_e = s["elements"][e_i] if e_i < len(s["elements"]) else False
            fill_c = elem_colors[e_i] if active_e else (51, 65, 85, 255)
            d3.ellipse([ex, ey, ex + 14, ey + 14], fill=fill_c, outline=(255, 255, 255, 120))
            if active_e:
                d3.point((ex + 4, ey + 4), fill=(255, 255, 255, 255))
    else:
        # Empty File / New Game
        d3.text((115, card_y + 22), "- NO DATA (NOVO JOGO) -", fill=(148, 163, 184, 255), font=font_bold)
        for h in range(3):
            draw_heart(d3, 310 + h * 14, card_y + 22, filled=True)

# Action navigation bar
d3.rectangle([20, H - 36, W - 20, H - 10], fill=(2, 6, 23, 240), outline=(56, 189, 248, 255))
d3.text((35, H - 28), "[A] ESCOLHER   |   [L] COPIAR   |   [R] APAGAR   |   [B] VOLTAR AO TITULO",
        fill=(224, 242, 254, 255), font=font_small)

canvas.paste(p3, (0, H))

# ============================================================================
# PANEL 4: REGISTRO DE NOME DO HERÓI (NAME ENTRY)
# ============================================================================
p4 = Image.new("RGBA", (W, H), (15, 23, 42, 255))
d4 = ImageDraw.Draw(p4)

# Royal dark slate background
for y in range(0, H, 8):
    d4.line([(0, y), (W, y)], fill=(20, 30, 50, 255))

# Header Box
d4.rectangle([100, 10, W - 100, 36], fill=(2, 6, 23, 240), outline=(251, 191, 36, 255), width=2)
d4.text((140, 15), "REGISTRO DE NOME DO HEROI", fill=(253, 224, 71, 255), font=font_bold)

# 6 Character Display Box
disp_x = (W - 240) // 2
disp_y = 48
d4.rectangle([disp_x, disp_y, disp_x + 240, disp_y + 40], fill=(30, 41, 59, 255), outline=(56, 189, 248, 255), width=2)

current_name = "LINK"
for i in range(6):
    box_ix = disp_x + 18 + i * 36
    d4.rectangle([box_ix, disp_y + 6, box_ix + 26, disp_y + 34], fill=(15, 23, 42, 255), outline=(71, 85, 105, 255))
    if i < len(current_name):
        d4.text((box_ix + 7, disp_y + 11), current_name[i], fill=(255, 255, 255, 255), font=font_bold)
    elif i == len(current_name):
        # Active cursor underline
        d4.rectangle([box_ix + 4, disp_y + 28, box_ix + 22, disp_y + 30], fill=(253, 224, 71, 255))

# Virtual Minish Keyboard Grid (7 Rows x 10 Columns)
k_grid = [
    "ABCDEFGHIJ",
    "KLMNOPQRST",
    "UVWXYZ.-!?",
    "abcdefghij",
    "klmnopqrst",
    "uvwxyz0123",
    "456789    "
]

grid_x = 75
grid_y = 105
key_w, key_h = 32, 24

for r in range(len(k_grid)):
    for c in range(10):
        kx = grid_x + c * key_w
        ky = grid_y + r * key_h
        ch = k_grid[r][c]
        if ch == ' ':
            continue

        is_active_key = (r == 0 and c == 3) # Highlighting 'D' as example
        key_bg = (251, 191, 36, 255) if is_active_key else (30, 41, 59, 255)
        text_c = (15, 23, 42, 255) if is_active_key else (241, 245, 249, 255)

        d4.rectangle([kx, ky, kx + key_w - 4, ky + key_h - 4], fill=key_bg, outline=(71, 85, 105, 255))
        d4.text((kx + 9, ky + 2), ch, fill=text_c, font=font_bold)

# DEL and OK Action Buttons on the bottom right of the keyboard
del_x = grid_x + 6 * key_w
del_y = grid_y + 6 * key_h
d4.rectangle([del_x, del_y, del_x + 58, del_y + key_h - 4], fill=(185, 28, 28, 255), outline=(239, 68, 68, 255))
d4.text((del_x + 12, del_y + 2), "[ DEL ]", fill=(254, 226, 226, 255), font=font_bold)

ok_x = grid_x + 8 * key_w
ok_y = grid_y + 6 * key_h
d4.rectangle([ok_x, ok_y, ok_x + 58, ok_y + key_h - 4], fill=(21, 128, 61, 255), outline=(34, 197, 94, 255))
d4.text((ok_x + 12, ok_y + 2), "[ FIM ]", fill=(220, 252, 231, 255), font=font_bold)

# Footer instructions
d4.rectangle([20, H - 32, W - 20, H - 8], fill=(2, 6, 23, 240), outline=(251, 191, 36, 255))
d4.text((45, H - 24), "[A] INSERIR   |   [B] APAGAR   |   [START / FIM] CONFIRMAR & INICIAR",
        fill=(254, 240, 138, 255), font=font_small)

canvas.paste(p4, (W, H))

# ============================================================================
# Save showcase image to scratch and artifact directory
# ============================================================================
scratch_path = os.path.join("d:/REPOS/TLoZ-MC/scratch", "startup_title_select_showcase.png")
artifact_dir = "C:/Users/thiag/.gemini/antigravity/brain/63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
artifact_path = os.path.join(artifact_dir, "startup_title_select_showcase.png")

canvas.save(scratch_path)
print(f"[OK] Saved to scratch: {scratch_path}")

os.makedirs(artifact_dir, exist_ok=True)
shutil.copyfile(scratch_path, artifact_path)
print(f"[OK] Saved to artifacts: {artifact_path}")
