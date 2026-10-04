#!/usr/bin/env python3
"""
scratch/verify_sanctuary_split3.py
Generates elemental_sanctuary_split3_showcase.png showcasing:
1. Elemental Sanctuary: Sacred Altar & 3 Elemental Orbs (Earth, Fire, Water)
2. Infusion Cutscene: Three Elements Spiral into White Sword (Three Elements)
3. Trio of Split Pads: Charging & Creation of 3 Links (Green, Red, Blue)
4. Triple Floor Switches & Colossal 3-Hero Push Block Unlocking Sacred Treasury
"""

import os
import shutil
import math
from PIL import Image, ImageDraw, ImageFont

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "elemental_sanctuary_split3_showcase.png")
    scratch_path = r"scratch/elemental_sanctuary_split3_showcase.png"

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (10, 14, 24, 255))
    draw = ImageDraw.Draw(img)

    try:
        font_title = ImageFont.truetype("arial.ttf", 15)
        font_sub = ImageFont.truetype("arial.ttf", 12)
        font_small = ImageFont.truetype("arial.ttf", 10)
    except:
        font_title = font_sub = font_small = ImageFont.load_default()

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(15, 23, 42, 255))
    draw.line([0, 48, img_w, 48], fill=(56, 189, 248, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Elemental Sanctuary & Four Sword 3-Clone Split", fill=(254, 240, 138, 255), font=font_title)
    draw.text((680, 16), "Item 8: feature/elemental-sanctuary-split3", fill=(56, 189, 248, 255), font=font_sub)

    panels = [
        ("1. ELEMENTAL SANCTUARY ALTAR & 3 SACRED ORBS (EARTH, FIRE, WATER)", 20, 60, 460, 330),
        ("2. THREE-ELEMENT INFUSION: WHITE SWORD (THREE ELEMENTS)", 500, 60, 940, 330),
        ("3. TRIO OF SPLIT PADS & 3-CLONE FORMATION (GREEN, RED, BLUE)", 20, 350, 460, 620),
        ("4. TRIPLE FLOOR SWITCHES & COLOSSAL 3-HERO PUSH BLOCK", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(15, 20, 32, 255), outline=(40, 55, 80, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(24, 34, 54, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(56, 189, 248, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(125, 211, 252, 255), font=font_small)

    def draw_sanctuary_room(px, py, gate_open=False, block_pushed=False):
        # Polished marble checkered floor
        for ry in range(14):
            for rx in range(26):
                lx = px + rx * 16
                ly = py + ry * 16
                col = (226, 232, 240, 255) if (rx + ry) % 2 == 0 else (203, 213, 225, 255)
                draw.rectangle([lx, ly, lx + 15, ly + 15], fill=col)
                draw.line([lx, ly + 15, lx + 15, ly + 15], fill=(148, 163, 184, 255))
                draw.line([lx + 15, ly, lx + 15, ly + 15], fill=(148, 163, 184, 255))

        # Stained glass light projections (Water Cyan, Earth Green, Fire Ruby)
        for vy, color in [(py + 40, (6, 182, 212, 45)), (py + 90, (34, 197, 94, 45)), (py + 140, (239, 68, 68, 45))]:
            draw.rectangle([px + 32, vy, px + 96, vy + 32], fill=color)
            draw.rectangle([px + 320, vy, px + 384, vy + 32], fill=color)

        # Sanctuary perimeter walls
        for rx in range(26):
            draw.rectangle([px + rx * 16, py, px + rx * 16 + 15, py + 16], fill=(30, 41, 59, 255))
            draw.line([px + rx * 16, py + 4, px + rx * 16 + 15, py + 4], fill=(51, 65, 85, 255), width=2)
            draw.line([px + rx * 16, py + 15, px + rx * 16 + 15, py + 15], fill=(245, 158, 11, 255))

        # Altar Pedestal (Center)
        ax, ay = px + 208, py + 36
        draw.rectangle([ax - 20, ay - 12, ax + 20, ay + 16], fill=(71, 85, 105, 255))
        draw.rectangle([ax - 16, ay - 8, ax + 16, ay + 12], fill=(100, 116, 139, 255))
        draw.line([ax - 20, ay + 14, ax + 20, ay + 14], fill=(212, 175, 55, 255), width=2)

        # Earth Element Pedestal (Left center)
        draw.rectangle([ax - 55, ay - 4, ax - 35, ay + 14], fill=(71, 85, 105, 255))
        draw.ellipse([ax - 51, ay - 16, ax - 39, ay - 4], fill=(34, 197, 94, 255), outline=(134, 239, 172, 255))

        # Fire Element Pedestal (Right center)
        draw.rectangle([ax + 35, ay - 4, ax + 55, ay + 14], fill=(71, 85, 105, 255))
        draw.ellipse([ax + 39, ay - 16, ax + 51, ay - 4], fill=(239, 68, 68, 255), outline=(254, 240, 138, 255))

        # Water Element Pedestal (Far Left - from Temple of Droplets!)
        draw.rectangle([ax - 105, ay - 4, ax - 85, ay + 14], fill=(71, 85, 105, 255))
        draw.ellipse([ax - 101, ay - 16, ax - 89, ay - 4], fill=(6, 182, 212, 255), outline=(224, 242, 254, 255))

        # Gate (y = py + 64)
        gx = px + 120
        gy = py + 64
        if not gate_open:
            draw.rectangle([gx, gy, gx + 176, gy + 12], fill=(100, 116, 139, 255))
            for b in range(18):
                draw.line([gx + b * 10, gy, gx + b * 10, gy + 12], fill=(30, 41, 59, 255), width=2)
            draw.rectangle([gx + 80, gy + 2, gx + 96, gy + 10], fill=(245, 158, 11, 255))
        else:
            draw.rectangle([gx, gy, gx + 176, gy + 3], fill=(100, 116, 139, 255))

        # Heavy 3-Hero Push Block (at x = px + 180, y = py + 64 - 16 or -40 if pushed)
        by = gy - (32 if block_pushed else 8)
        draw.rectangle([px + 188, by, px + 228, by + 16], fill=(71, 85, 105, 255), outline=(245, 158, 11, 255), width=2)
        draw.text((px + 194, by + 2), "3-HERO", fill=(254, 240, 138, 255), font=font_small)

    def draw_split_pads_trio(px, py, l1=False, l2=False, l3=False):
        pad_x = [px + 130, px + 208, px + 286]
        pad_y = py + 110
        lits = [l1, l2, l3]

        for i, pdx in enumerate(pad_x):
            is_lit = lits[i]
            border_c = (255, 255, 255, 255) if is_lit else (2, 132, 199, 255)
            inner_c = (56, 189, 248, 255) if is_lit else (3, 105, 161, 255)
            draw.rectangle([pdx, pad_y, pdx + 24, pad_y + 24], fill=inner_c, outline=border_c, width=2)
            rune_c = (255, 255, 255, 255) if is_lit else (56, 189, 248, 255)
            draw.rectangle([pdx + 10, pad_y + 4, pdx + 14, pad_y + 20], fill=rune_c)
            draw.rectangle([pdx + 4, pad_y + 10, pdx + 20, pad_y + 14], fill=rune_c)
            if is_lit:
                draw.ellipse([pdx - 8, pad_y - 8, pdx + 32, pad_y + 32], fill=(56, 189, 248, 70))

    def draw_triple_switches(px, py, s1=False, s2=False, s3=False):
        sw_x = [px + 130, px + 208, px + 286]
        sw_y = py + 160
        downs = [s1, s2, s3]

        for i, sx in enumerate(sw_x):
            draw.rectangle([sx, sw_y, sx + 24, sw_y + 24], fill=(51, 65, 85, 255), outline=(30, 41, 59, 255))
            plate_c = (34, 197, 94, 255) if downs[i] else (217, 119, 6, 255)
            draw.rectangle([sx + 4, sw_y + 4, sx + 20, sw_y + 20], fill=plate_c)

    def draw_link_hero(hx, hy, tunic_col, cap_col, aura_col=None, sword=False):
        if aura_col:
            draw.ellipse([hx - 6, hy - 6, hx + 22, hy + 26], fill=aura_col)
        # Shadow
        draw.ellipse([hx, hy + 16, hx + 16, hy + 22], fill=(5, 16, 7, 100))
        # Cap
        draw.polygon([(hx + 8, hy - 4), (hx + 2, hy + 4), (hx + 14, hy + 4)], fill=cap_col)
        # Face
        draw.rectangle([hx + 4, hy + 4, hx + 12, hy + 10], fill=(253, 232, 205, 255))
        draw.rectangle([hx + 6, hy + 6, hx + 7, hy + 8], fill=(30, 41, 59, 255))
        draw.rectangle([hx + 9, hy + 6, hx + 10, hy + 8], fill=(30, 41, 59, 255))
        # Tunic
        draw.rectangle([hx + 3, hy + 10, hx + 13, hy + 16], fill=tunic_col)
        # Boots
        draw.rectangle([hx + 4, hy + 16, hx + 7, hy + 20], fill=(180, 83, 9, 255))
        draw.rectangle([hx + 9, hy + 16, hx + 12, hy + 20], fill=(180, 83, 9, 255))

        if sword:
            draw.rectangle([hx + 14, hy + 2, hx + 17, hy + 16], fill=(255, 255, 255, 255), outline=(250, 204, 21, 255))

    # --- PANEL 1: Sanctuary Room & 3 Orbs ---
    p1x, p1y = 24, 88
    draw_sanctuary_room(p1x, p1y)
    draw_link_hero(p1x + 200, p1y + 120, (22, 163, 74, 255), (34, 197, 94, 255), sword=True)
    draw.text((p1x + 30, p1y + 180), "Altar sagrado pronto para receber a White Sword e os 3 Elementos!", fill=(224, 242, 254, 255), font=font_small)
    draw.text((p1x + 30, p1y + 196), "- Earth Element (Verde) | Fire Element (Vermelho) | Water Element (Ciano)", fill=(125, 211, 252, 255), font=font_small)

    # --- PANEL 2: Infusion Cutscene (Earth + Fire + Water -> 3 Elements) ---
    p2x, p2y = 504, 88
    draw_sanctuary_room(p2x, p2y)
    # 3 Glowing Energy Orbs in orbit around pedestal
    ax, ay = p2x + 208, p2y + 36
    draw.ellipse([ax - 40, ay - 35, ax + 40, ay + 35], fill=(56, 189, 248, 50))
    # Orbiting Orbs
    draw.ellipse([ax - 28, ay - 24, ax - 12, ay - 8], fill=(34, 197, 94, 255), outline=(255, 255, 255, 255), width=2) # Earth
    draw.ellipse([ax + 12, ay - 24, ax + 28, ay - 8], fill=(239, 68, 68, 255), outline=(255, 255, 255, 255), width=2) # Fire
    draw.ellipse([ax - 8, ay + 12, ax + 8, ay + 28], fill=(6, 182, 212, 255), outline=(255, 255, 255, 255), width=2)   # Water
    # Glowing White Sword
    draw.rectangle([ax - 2, ay - 24, ax + 2, ay], fill=(255, 255, 255, 255), outline=(253, 224, 71, 255), width=2)
    # Cutscene celebratory banner
    draw.rectangle([p2x + 40, p2y + 160, p2x + 380, p2y + 204], fill=(8, 47, 73, 230), outline=(56, 189, 248, 255), width=2)
    draw.text((p2x + 55, p2y + 168), "WHITE SWORD (THREE ELEMENTS) FORJADA!", fill=(254, 240, 138, 255), font=font_title)
    draw.text((p2x + 65, p2y + 186), "A Forca Divina desperta: Divisao em 3 Links simultaneos!", fill=(224, 242, 254, 255), font=font_small)

    # --- PANEL 3: Trio of Split Pads & 3-Clone Formation ---
    p3x, p3y = 24, 378
    draw_sanctuary_room(p3x, p3y)
    draw_split_pads_trio(p3x, p3y, l1=True, l2=True, l3=True)
    # The 3 Hero Formation!
    # Left: Red Link (Clone 1)
    draw_link_hero(p3x + 134, p3y + 112, (220, 38, 38, 255), (239, 68, 68, 255), (248, 113, 113, 70), sword=True)
    draw.text((p3x + 120, p3y + 140), "Red Link (Clone 1)", fill=(248, 113, 113, 255), font=font_small)
    # Center: Green Link (Original)
    draw_link_hero(p3x + 212, p3y + 112, (22, 163, 74, 255), (34, 197, 94, 255), sword=True)
    draw.text((p3x + 200, p3y + 140), "Green Link (Hero)", fill=(134, 239, 172, 255), font=font_small)
    # Right: Blue Link (Clone 2)
    draw_link_hero(p3x + 290, p3y + 112, (2, 132, 199, 255), (56, 189, 248, 255), (56, 189, 248, 70), sword=True)
    draw.text((p3x + 276, p3y + 140), "Blue Link (Clone 2)", fill=(125, 211, 252, 255), font=font_small)
    draw.text((p3x + 30, p3y + 196), "Carga nos 3 Pads -> Divisao em 3 Guerreiros sincronizados (Movimento & Ataque)!", fill=(224, 242, 254, 255), font=font_small)

    # --- PANEL 4: Triple Switches & Colossal Push Block ---
    p4x, p4y = 504, 378
    draw_sanctuary_room(p4x, p4y, gate_open=True, block_pushed=True)
    draw_triple_switches(p4x, p4y, s1=True, s2=True, s3=True)
    # 3 Links standing on the 3 switches
    draw_link_hero(p4x + 134, p4y + 162, (220, 38, 38, 255), (239, 68, 68, 255), (248, 113, 113, 70))
    draw_link_hero(p4x + 212, p4y + 162, (22, 163, 74, 255), (34, 197, 94, 255))
    draw_link_hero(p4x + 290, p4y + 162, (2, 132, 199, 255), (56, 189, 248, 255), (56, 189, 248, 70))
    # Unlocked Treasury Banner
    draw.rectangle([p4x + 35, p4y + 196, p4x + 395, p4y + 224], fill=(12, 74, 110, 230), outline=(14, 165, 233, 255))
    draw.text((p4x + 45, p4y + 202), "3 INTERRUPTORES ATIVADOS + BLOCO EMPURRADO -> TESOURO SAGRADO!", fill=(254, 240, 138, 255), font=font_small)

    # Save outputs
    img.save(scratch_path)
    print(f"[OK] Showcase gerada com sucesso em: {scratch_path}")
    if os.path.isdir(out_dir):
        shutil.copyfile(scratch_path, out_path)
        print(f"[OK] Showcase copiada para o diretorio de artefatos: {out_path}")

if __name__ == "__main__":
    main()
