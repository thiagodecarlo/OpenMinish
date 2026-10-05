#!/usr/bin/env python3
"""
scratch/verify_four_sword_forged.py
Generates four_sword_forged_showcase.png showcasing:
1. Elemental Sanctuary: 4 Sacred Orbs & Four Sword Infusion Ceremony (Earth, Fire, Water, Wind)
2. Diamond Split Pads & 4-Clone Division (Green, Red, Blue, Purple Link)
3. Radiant Sword Beams Projectiles with Full Health
4. Quadruple Floor Switches & Unsealing the Royal Valley North Passage (Ato V)
"""

import os
import shutil
import math
from PIL import Image, ImageDraw, ImageFont

def main():
    conv_id = "91f45871-4e2f-495f-88d6-7427ed06770a"
    out_dir = os.path.join(r"C:\Users\thiag\.gemini\antigravity\brain", conv_id)
    out_path = os.path.join(out_dir, "four_sword_forged_showcase.png")
    scratch_path = r"scratch/four_sword_forged_showcase.png"

    img_w = 980
    img_h = 660
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
    draw.line([0, 48, img_w, 48], fill=(245, 158, 11, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Four Sword Forged & 4-Clone Division", fill=(254, 240, 138, 255), font=font_title)
    draw.text((700, 16), "Sprint 4: feature/four-sword-forged", fill=(56, 189, 248, 255), font=font_sub)

    panels = [
        ("1. ELEMENTAL SANCTUARY: 4 SACRED ORBS & FOUR SWORD CEREMONY", 20, 60, 475, 340),
        ("2. DIAMOND SPLIT PADS & 4-CLONE FORMATION (GREEN, RED, BLUE, PURPLE)", 505, 60, 960, 340),
        ("3. FOUR SWORD RADIANT SWORD BEAMS (FULL HEALTH PROJECTILE)", 20, 360, 475, 640),
        ("4. QUADRUPLE FLOOR SWITCHES & UNSEALED ROYAL VALLEY PASSAGE", 505, 360, 960, 640)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(15, 20, 32, 255), outline=(40, 55, 80, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(24, 34, 54, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(245, 158, 11, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(253, 224, 71, 255), font=font_small)

    def draw_sanctuary_base(px, py, north_passage_open=False, gate_open=False, block_pushed=False):
        # Polished marble checkered floor
        for ry in range(14):
            for rx in range(27):
                lx = px + rx * 16
                ly = py + ry * 16
                col = (226, 232, 240, 255) if (rx + ry) % 2 == 0 else (203, 213, 225, 255)
                draw.rectangle([lx, ly, lx + 15, ly + 15], fill=col)
                draw.line([lx, ly + 15, lx + 15, ly + 15], fill=(148, 163, 184, 255))
                draw.line([lx + 15, ly, lx + 15, ly + 15], fill=(148, 163, 184, 255))

        # Stained glass colored ambient beams
        for vy, color in [(py + 40, (6, 182, 212, 35)), (py + 90, (34, 197, 94, 35)), (py + 140, (239, 68, 68, 35)), (py + 190, (16, 185, 129, 35))]:
            draw.rectangle([px + 32, vy, px + 96, vy + 28], fill=color)
            draw.rectangle([px + 336, vy, px + 400, vy + 28], fill=color)

        # Sanctuary perimeter walls
        for rx in range(27):
            # Check if this column is part of the north secret passage
            if north_passage_open and rx in (12, 13, 14):
                # Unsealed royal passage
                draw.rectangle([px + rx * 16, py, px + rx * 16 + 15, py + 16], fill=(15, 23, 42, 255))
                draw.rectangle([px + rx * 16, py, px + rx * 16 + 15, py + 16], fill=(253, 224, 71, 70))
            else:
                draw.rectangle([px + rx * 16, py, px + rx * 16 + 15, py + 16], fill=(30, 41, 59, 255))
                draw.line([px + rx * 16, py + 4, px + rx * 16 + 15, py + 4], fill=(51, 65, 85, 255), width=2)
                draw.line([px + rx * 16, py + 15, px + rx * 16 + 15, py + 15], fill=(245, 158, 11, 255))

        # Altar Pedestal (Center)
        ax, ay = px + 216, py + 36
        draw.rectangle([ax - 20, ay - 12, ax + 20, ay + 16], fill=(71, 85, 105, 255))
        draw.rectangle([ax - 16, ay - 8, ax + 16, ay + 12], fill=(100, 116, 139, 255))
        draw.line([ax - 20, ay + 14, ax + 20, ay + 14], fill=(245, 158, 11, 255), width=2)

        # 4 Sacred Element Pedestals:
        # 1. Water Element (Far Left)
        draw.rectangle([ax - 125, ay - 4, ax - 105, ay + 14], fill=(71, 85, 105, 255))
        draw.ellipse([ax - 121, ay - 16, ax - 109, ay - 4], fill=(6, 182, 212, 255), outline=(224, 242, 254, 255))

        # 2. Earth Element (Mid Left)
        draw.rectangle([ax - 65, ay - 4, ax - 45, ay + 14], fill=(71, 85, 105, 255))
        draw.ellipse([ax - 61, ay - 16, ax - 49, ay - 4], fill=(34, 197, 94, 255), outline=(134, 239, 172, 255))

        # 3. Fire Element (Mid Right)
        draw.rectangle([ax + 45, ay - 4, ax + 65, ay + 14], fill=(71, 85, 105, 255))
        draw.ellipse([ax + 49, ay - 16, ax + 61, ay - 4], fill=(239, 68, 68, 255), outline=(254, 240, 138, 255))

        # 4. Wind Element (Far Right - from Palace of Winds!)
        draw.rectangle([ax + 105, ay - 4, ax + 125, ay + 14], fill=(71, 85, 105, 255))
        draw.ellipse([ax + 109, ay - 16, ax + 121, ay - 4], fill=(16, 185, 129, 255), outline=(254, 240, 138, 255))

        # Gate (y = py + 64)
        gx = px + 120
        gy = py + 64
        if not gate_open:
            draw.rectangle([gx, gy, gx + 192, gy + 12], fill=(100, 116, 139, 255))
            for b in range(20):
                draw.line([gx + b * 10, gy, gx + b * 10, gy + 12], fill=(30, 41, 59, 255), width=2)
            draw.rectangle([gx + 88, gy + 2, gx + 104, gy + 10], fill=(245, 158, 11, 255))
        else:
            draw.rectangle([gx, gy, gx + 192, gy + 4], fill=(100, 116, 139, 255))

        # Heavy Push Block (col 12-14, row 4)
        bx = px + 192
        by = py + (42 if block_pushed else 66)
        draw.rectangle([bx, by, bx + 48, by + 20], fill=(71, 85, 105, 255), outline=(100, 116, 139, 255), width=2)
        draw.rectangle([bx + 16, by + 4, bx + 32, by + 16], fill=(245, 158, 11, 255))
        draw.text((bx + 18, by + 5), "IV", fill=(255, 255, 255, 255), font=font_small)

    def draw_link_hero(lx, ly, tunic_c, cap_c, aura_c=None, attacking=False, dir_up=False):
        # Shadow
        draw.ellipse([lx - 8, ly + 8, lx + 8, ly + 14], fill=(5, 16, 7, 90))
        # Aura if present
        if aura_c:
            draw.ellipse([lx - 14, ly - 14, lx + 14, ly + 14], fill=aura_c)

        # Hero Body
        draw.rectangle([lx - 5, ly - 10, lx + 5, ly - 3], fill=cap_c) # Cap
        draw.rectangle([lx - 4, ly - 3, lx + 4, ly + 3], fill=(253, 232, 205, 255)) # Face
        draw.rectangle([lx - 5, ly + 3, lx + 5, ly + 9], fill=tunic_c) # Tunic
        draw.rectangle([lx - 4, ly + 9, lx + 4, ly + 12], fill=(255, 255, 255, 255)) # Pants
        draw.rectangle([lx - 4, ly + 12, lx + 4, ly + 15], fill=(180, 83, 9, 255)) # Boots

        if not dir_up:
            draw.rectangle([lx - 3, ly - 1, lx - 1, ly + 1], fill=(30, 41, 59, 255))
            draw.rectangle([lx + 1, ly - 1, lx + 3, ly + 1], fill=(30, 41, 59, 255))

        # Sword
        if attacking:
            if dir_up:
                draw.rectangle([lx + 6, ly - 14, lx + 8, ly], fill=(255, 255, 255, 255))
                draw.rectangle([lx + 4, ly - 2, lx + 10, ly], fill=(245, 158, 11, 255))
            else:
                draw.rectangle([lx + 7, ly, lx + 19, ly + 3], fill=(255, 255, 255, 255))
                draw.rectangle([lx + 7, ly - 2, lx + 9, ly + 5], fill=(245, 158, 11, 255))

    # -------------------------------------------------------------
    # PANEL 1: ELEMENTAL SANCTUARY ALTAR & 4-ELEMENT INFUSION CEREMONY
    # -------------------------------------------------------------
    p1_x, p1_y = 30, 90
    draw_sanctuary_base(p1_x, p1_y, north_passage_open=False, gate_open=False, block_pushed=False)

    # 4 Celestial Elements spiraling in orbital vortex
    ax, ay = p1_x + 216, p1_y + 36
    draw.ellipse([ax - 42, ay - 32, ax + 42, ay + 20], fill=(254, 240, 138, 45))
    draw.ellipse([ax - 28, ay - 24, ax + 28, ay + 12], fill=(255, 255, 255, 70))

    # Orbiting Orbs
    angles = [0.0, 1.571, 3.142, 4.712]
    colors = [
        ((34, 197, 94, 255), (134, 239, 172, 255), "Earth"),
        ((239, 68, 68, 255), (254, 240, 138, 255), "Fire"),
        ((6, 182, 212, 255), (224, 242, 254, 255), "Water"),
        ((16, 185, 129, 255), (254, 240, 138, 255), "Wind")
    ]
    for i, a in enumerate(angles):
        ox = ax + int(math.cos(a + 0.5) * 36.0)
        oy = ay + int(math.sin(a + 0.5) * 22.0) - 8
        c_fill, c_out, name = colors[i]
        draw.ellipse([ox - 7, oy - 7, ox + 7, oy + 7], fill=c_fill, outline=c_out, width=2)
        draw.ellipse([ox - 12, oy - 12, ox + 12, oy + 12], fill=(c_fill[0], c_fill[1], c_fill[2], 50))

    # Four Sword infused blade in pedestal
    draw.rectangle([ax - 2, ay - 26, ax + 2, ay - 8], fill=(255, 255, 255, 255))
    draw.rectangle([ax - 7, ay - 10, ax + 7, ay - 7], fill=(250, 204, 21, 255))
    # 4 colored gems on crossguard
    draw.point([ax - 5, ay - 9], fill=(34, 197, 94, 255))
    draw.point([ax - 2, ay - 9], fill=(239, 68, 68, 255))
    draw.point([ax + 2, ay - 9], fill=(6, 182, 212, 255))
    draw.point([ax + 5, ay - 9], fill=(16, 185, 129, 255))

    # Link praying at the altar
    draw_link_hero(ax, ay + 28, (34, 197, 94, 255), (22, 163, 74, 255), dir_up=True)

    # Info card
    draw.rectangle([p1_x + 10, p1_y + 160, p1_x + 420, p1_y + 235], fill=(15, 23, 42, 230), outline=(56, 189, 248, 255))
    draw.text((p1_x + 18, p1_y + 166), "CERIMONIA DOS 4 ELEMENTOS: FOUR SWORD FORJADA", fill=(253, 224, 71, 255), font=font_sub)
    draw.text((p1_x + 18, p1_y + 184), "* Infusao Sagrada: Terra, Fogo, Agua e Vento (Palace of Winds).", fill=(226, 232, 240, 255), font=font_small)
    draw.text((p1_x + 18, p1_y + 198), "* A White Sword transforma-se na definitiva FOUR SWORD COMPLETA!", fill=(125, 211, 252, 255), font=font_small)
    draw.text((p1_x + 18, p1_y + 212), "* Desbloqueia 4 Herois simultaneos, Sword Beams e a passagem norte.", fill=(167, 243, 208, 255), font=font_small)

    # -------------------------------------------------------------
    # PANEL 2: DIAMOND SPLIT PADS & 4-CLONE FORMATION
    # -------------------------------------------------------------
    p2_x, p2_y = 515, 90
    draw_sanctuary_base(p2_x, p2_y, north_passage_open=False, gate_open=True, block_pushed=False)

    # 4 Split Pads in Diamond Formation
    # Pad 0: Top, Pad 1: Left, Pad 2: Right, Pad 3: Bottom
    pad_positions = [
        (p2_x + 216, p2_y + 70,  "Pad 0 (Norte)"),
        (p2_x + 180, p2_y + 90,  "Pad 1 (Oeste)"),
        (p2_x + 252, p2_y + 90,  "Pad 2 (Leste)"),
        (p2_x + 216, p2_y + 110, "Pad 3 (Sul)")
    ]
    for px, py_pos, label in pad_positions:
        draw.rectangle([px - 9, py_pos - 9, px + 9, py_pos + 9], fill=(2, 132, 199, 255), outline=(255, 255, 255, 255), width=2)
        draw.rectangle([px - 6, py_pos - 6, px + 6, py_pos + 6], fill=(56, 189, 248, 255))
        draw.ellipse([px - 14, py_pos - 14, px + 14, py_pos + 14], fill=(56, 189, 248, 50))

    # 4 Heroes marching downward in diamond formation
    # Green Link (Bottom/Leader)
    draw_link_hero(p2_x + 216, p2_y + 155, (34, 197, 94, 255), (22, 163, 74, 255))
    draw.text((p2_x + 225, p2_y + 150), "Green Link", fill=(134, 239, 172, 255), font=font_small)

    # Red Link (Top)
    draw_link_hero(p2_x + 216, p2_y + 115, (220, 38, 38, 255), (239, 68, 68, 255), aura_c=(248, 113, 113, 60))
    draw.text((p2_x + 225, p2_y + 110), "Red Link", fill=(248, 113, 113, 255), font=font_small)

    # Blue Link (Left)
    draw_link_hero(p2_x + 175, p2_y + 135, (2, 132, 199, 255), (56, 189, 248, 255), aura_c=(56, 189, 248, 60))
    draw.text((p2_x + 115, p2_y + 130), "Blue Link", fill=(125, 211, 252, 255), font=font_small)

    # Purple Link (Right - Canon Violet Four Sword!)
    draw_link_hero(p2_x + 257, p2_y + 135, (126, 34, 206, 255), (168, 85, 247, 255), aura_c=(192, 132, 252, 60))
    draw.text((p2_x + 270, p2_y + 130), "Purple Link", fill=(216, 180, 254, 255), font=font_small)

    # Celebratory Banner UI rendering
    ban_x = p2_x + 85
    ban_y = p2_y + 185
    draw.rectangle([ban_x, ban_y, ban_x + 260, ban_y + 36], fill=(15, 23, 42, 250), outline=(245, 158, 11, 255), width=2)
    # 4 hero icons in banner
    draw.rectangle([ban_x + 6, ban_y + 10, ban_x + 12, ban_y + 26], fill=(34, 197, 94, 255))
    draw.rectangle([ban_x + 14, ban_y + 10, ban_x + 20, ban_y + 26], fill=(220, 38, 38, 255))
    draw.rectangle([ban_x + 22, ban_y + 10, ban_x + 28, ban_y + 26], fill=(2, 132, 199, 255))
    draw.rectangle([ban_x + 30, ban_y + 10, ban_x + 36, ban_y + 26], fill=(126, 34, 206, 255))
    draw.text((ban_x + 44, ban_y + 7), "FOUR SWORD COMPLETA FORJADA!", fill=(253, 224, 71, 255), font=font_small)
    draw.text((ban_x + 44, ban_y + 20), "4 HEROIS & SWORD BEAMS ATIVOS!", fill=(56, 189, 248, 255), font=font_small)

    # -------------------------------------------------------------
    # PANEL 3: FOUR SWORD RADIANT SWORD BEAMS (FULL HP PROJECTILE)
    # -------------------------------------------------------------
    p3_x, p3_y = 30, 390
    draw_sanctuary_base(p3_x, p3_y, north_passage_open=False, gate_open=True, block_pushed=True)

    # Green Link attacking forward
    draw_link_hero(p3_x + 120, p3_y + 110, (34, 197, 94, 255), (22, 163, 74, 255), attacking=True)

    # 3 Clones attacking in unison!
    draw_link_hero(p3_x + 120, p3_y + 80, (220, 38, 38, 255), (239, 68, 68, 255), aura_c=(248, 113, 113, 60), attacking=True)
    draw_link_hero(p3_x + 85,  p3_y + 95, (2, 132, 199, 255), (56, 189, 248, 255), aura_c=(56, 189, 248, 60), attacking=True)
    draw_link_hero(p3_x + 155, p3_y + 95, (126, 34, 206, 255), (168, 85, 247, 255), aura_c=(192, 132, 252, 60), attacking=True)

    # Radiant Sword Beam projectile speeding horizontally to the right
    bm_x, bm_y = p3_x + 220, p3_y + 112
    draw.rectangle([bm_x - 12, bm_y - 8, bm_x + 24, bm_y + 8], fill=(253, 224, 71, 90)) # Gold aura
    draw.rectangle([bm_x - 8, bm_y - 4, bm_x + 20, bm_y + 4], fill=(56, 189, 248, 255)) # Cyan plasma core
    draw.rectangle([bm_x + 4, bm_y - 2, bm_x + 16, bm_y + 2], fill=(255, 255, 255, 255)) # Brilliant tip
    # Sparkles
    for sp_x, sp_y in [(bm_x - 6, bm_y - 10), (bm_x + 10, bm_y + 9), (bm_x - 14, bm_y + 7)]:
        draw.rectangle([sp_x, sp_y, sp_x + 3, sp_y + 3], fill=(254, 240, 138, 255))

    # Enemy Darknut / Moblin taking hit impact
    en_x, en_y = p3_x + 320, p3_y + 112
    draw.rectangle([en_x - 12, en_y - 14, en_x + 12, en_y + 14], fill=(185, 28, 28, 255), outline=(254, 202, 202, 255), width=2)
    draw.text((en_x - 10, en_y - 8), "MOB", fill=(255, 255, 255, 255), font=font_small)

    # Impact burst & Damage Float
    draw.ellipse([en_x - 22, en_y - 22, en_x + 22, en_y + 22], fill=(250, 204, 21, 100))
    draw.text((en_x - 8, en_y - 32), "-3 HP", fill=(239, 68, 68, 255), font=font_sub)

    # Hearts indicator in HUD showing Full Health condition
    draw.rectangle([p3_x + 10, p3_y + 175, p3_x + 420, p3_y + 235], fill=(15, 23, 42, 230), outline=(245, 158, 11, 255))
    draw.text((p3_x + 18, p3_y + 182), "SWORD BEAMS: ATIVADOS COM VIDA CHEIA (100% HP)", fill=(253, 224, 71, 255), font=font_sub)
    draw.text((p3_x + 18, p3_y + 200), "* Dano: 3 HP (Lâmina Quádrupla Suprema). Velocidade: 4.4 px/frame.", fill=(226, 232, 240, 255), font=font_small)
    draw.text((p3_x + 18, p3_y + 214), "* Dissipa ao colidir em paredes e corta inimigos à longa distância.", fill=(167, 243, 208, 255), font=font_small)

    # -------------------------------------------------------------
    # PANEL 4: QUADRUPLE FLOOR SWITCHES & UNSEALED ROYAL VALLEY PASSAGE
    # -------------------------------------------------------------
    p4_x, p4_y = 505, 390
    draw_sanctuary_base(p4_x, p4_y, north_passage_open=True, gate_open=True, block_pushed=True)

    # 4 Floor Switches at y = p4_y + 135
    sw_x_coords = [p4_x + 90, p4_x + 155, p4_x + 245, p4_x + 310]
    for sx in sw_x_coords:
        draw.rectangle([sx - 10, p4_y + 125, sx + 10, p4_y + 145], fill=(51, 65, 85, 255))
        draw.rectangle([sx - 7, p4_y + 128, sx + 7, p4_y + 142], fill=(34, 197, 94, 255)) # Green = Pressed down!

    # 4 Heroes standing on the 4 switches!
    draw_link_hero(sw_x_coords[0], p4_y + 130, (2, 132, 199, 255), (56, 189, 248, 255), aura_c=(56, 189, 248, 60))   # Blue
    draw_link_hero(sw_x_coords[1], p4_y + 130, (34, 197, 94, 255), (22, 163, 74, 255))                                 # Green
    draw_link_hero(sw_x_coords[2], p4_y + 130, (220, 38, 38, 255), (239, 68, 68, 255), aura_c=(248, 113, 113, 60))   # Red
    draw_link_hero(sw_x_coords[3], p4_y + 130, (126, 34, 206, 255), (168, 85, 247, 255), aura_c=(192, 132, 252, 60)) # Purple

    # Golden light rays pouring from the unsealed north portal
    nx = p4_x + 216
    draw.polygon([(nx - 24, p4_y + 16), (nx + 24, p4_y + 16), (nx + 50, p4_y + 70), (nx - 50, p4_y + 70)], fill=(253, 224, 71, 55))
    draw.text((nx - 60, p4_y + 4), "ROYAL VALLEY PASSAGE", fill=(254, 240, 138, 255), font=font_small)

    # Info card for panel 4
    draw.rectangle([p4_x + 10, p4_y + 175, p4_x + 440, p4_y + 235], fill=(15, 23, 42, 230), outline=(245, 158, 11, 255))
    draw.text((p4_x + 18, p4_y + 182), "PASSAGEM SECRETA PARA O ROYAL VALLEY (ATO V)", fill=(253, 224, 71, 255), font=font_sub)
    draw.text((p4_x + 18, p4_y + 200), "* 4 interruptores pesados acionados simultaneamente pela equipe.", fill=(226, 232, 240, 255), font=font_small)
    draw.text((p4_x + 18, p4_y + 214), "* Altar abre rota sagrada ao norte: Tumba Real de Gustaf & Dark Hyrule!", fill=(167, 243, 208, 255), font=font_small)

    # Save showcase image
    os.makedirs(out_dir, exist_ok=True)
    img.save(out_path, "PNG")
    img.save(scratch_path, "PNG")
    print(f"[VERIFY] Showcase gerado com sucesso em: {out_path}")
    print(f"[VERIFY] Copia em scratch salva em: {scratch_path}")

if __name__ == "__main__":
    main()
