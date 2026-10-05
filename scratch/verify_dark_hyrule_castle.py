#!/usr/bin/env python3
"""
scratch/verify_dark_hyrule_castle.py
Generates dark_hyrule_castle_showcase.png showcasing:
1. Corrupted Foyer & Malice Barrier (Four Sword diamond split pads, lantern torches)
2. West Wing: 4-Clone Synchronized Chasm, 4-Eye Switches & Hard Light Bridge
3. East Wing & Darknut Miniboss Duel (Heavy monolith strength, Black Knight duel)
4. Petrification Bell Tower & Vaati's Sanctum Gate (3 Bells silenced, Sanctum Gate unsealed)
"""

import os
import shutil
import math
from PIL import Image, ImageDraw, ImageFont

def main():
    conv_id = "91f45871-4e2f-495f-88d6-7427ed06770a"
    out_dir = os.path.join(r"C:\Users\thiag\.gemini\antigravity\brain", conv_id)
    out_path = os.path.join(out_dir, "dark_hyrule_castle_showcase.png")
    scratch_path = r"scratch/dark_hyrule_castle_showcase.png"

    img_w = 980
    img_h = 660
    img = Image.new("RGBA", (img_w, img_h), (12, 10, 20, 255))
    draw = ImageDraw.Draw(img)

    try:
        font_title = ImageFont.truetype("arial.ttf", 15)
        font_sub = ImageFont.truetype("arial.ttf", 12)
        font_small = ImageFont.truetype("arial.ttf", 10)
    except:
        font_title = font_sub = font_small = ImageFont.load_default()

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(24, 20, 36, 255))
    draw.line([0, 48, img_w, 48], fill=(225, 29, 72, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Dark Hyrule Castle (Dungeon 6 - Ato V)", fill=(254, 240, 138, 255), font=font_title)
    draw.text((680, 16), "Sprint 5: feature/dungeon-dark-hyrule-castle", fill=(244, 63, 94, 255), font=font_sub)

    panels = [
        ("1. CORRUPTED FOYER & MALICE BARRIER (DIAMOND SPLIT PADS & TORCHES)", 20, 60, 475, 340),
        ("2. WEST WING: 4-CLONE SYNCHRONIZED CHASM & HARD LIGHT BRIDGE", 505, 60, 960, 340),
        ("3. EAST WING & BLACK KNIGHT (DARKNUT MINIBOSS DUEL & MONOLITH)", 20, 360, 475, 640),
        ("4. PETRIFICATION BELL TOWER & VAATI'S SANCTUM GATE UNSEALED", 505, 360, 960, 640)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(18, 16, 28, 255), outline=(49, 46, 75, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(30, 27, 48, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(225, 29, 72, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(253, 224, 71, 255), font=font_small)

    def draw_link(lx, ly, tunic_col, tunic_dark, hat_col, dir_down=True, holding_sword=False, pushing=False):
        draw.ellipse([lx - 6, ly + 6, lx + 6, ly + 10], fill=(0, 0, 0, 100))
        draw.rectangle([lx - 4, ly - 3, lx + 4, ly + 5], fill=tunic_col, outline=tunic_dark)
        draw.rectangle([lx - 4, ly + 2, lx + 4, ly + 4], fill=(120, 53, 15, 255))
        draw.rectangle([lx - 1, ly + 2, lx + 1, ly + 4], fill=(250, 204, 21, 255))
        draw.rectangle([lx - 4, ly + 5, lx - 1, ly + 8], fill=(113, 63, 18, 255))
        draw.rectangle([lx + 1, ly + 5, lx + 4, ly + 8], fill=(113, 63, 18, 255))
        draw.rectangle([lx - 3, ly - 9, lx + 3, ly - 3], fill=(254, 215, 170, 255))
        if dir_down:
            draw.point((lx - 1, ly - 6), fill=(15, 23, 42, 255))
            draw.point((lx + 2, ly - 6), fill=(15, 23, 42, 255))
        draw.polygon([(lx - 5, ly - 8), (lx + 5, ly - 8), (lx + 2, ly - 14), (lx - 2, ly - 14)], fill=hat_col)
        draw.ellipse([lx - 1, ly - 15, lx + 2, ly - 12], fill=(234, 179, 8, 255))

        if pushing:
            draw.rectangle([lx - 6, ly - 2, lx - 4, ly + 1], fill=(254, 215, 170, 255))
            draw.rectangle([lx + 4, ly - 2, lx + 6, ly + 1], fill=(254, 215, 170, 255))
        if holding_sword:
            draw.rectangle([lx + 6, ly - 8, lx + 8, ly + 4], fill=(241, 245, 249, 255))
            draw.rectangle([lx + 5, ly + 4, lx + 9, ly + 6], fill=(234, 179, 8, 255))

    def draw_dark_castle_room_base(ox, oy, has_chasm=False):
        for gy in range(14):
            for gx in range(26):
                px = ox + gx * 16
                py = oy + gy * 16
                if gy == 0 or gy == 13 or gx == 0 or gx == 25:
                    draw.rectangle([px, py, px + 15, py + 15], fill=(24, 24, 36, 255))
                    draw.line([px, py + 14, px + 15, py + 14], fill=(15, 14, 23, 255), width=2)
                else:
                    if has_chasm and gy in (5, 6, 7):
                        draw.rectangle([px, py, px + 15, py + 15], fill=(8, 8, 12, 255))
                    else:
                        fcol = (46, 42, 79, 255) if (gx + gy) % 2 == 0 else (37, 34, 64, 255)
                        draw.rectangle([px, py, px + 15, py + 15], fill=fcol)
                        draw.rectangle([px, py, px + 15, py + 15], outline=(30, 27, 48, 80))

        # Central Red Malice Carpet (cols 12..13)
        if not has_chasm:
            for gy in range(1, 13):
                cx1 = ox + 12 * 16
                cx2 = ox + 14 * 16
                cy = oy + gy * 16
                draw.rectangle([cx1, cy, cx2, cy + 15], fill=(136, 19, 55, 255))
                draw.line([cx1, cy, cx1, cy + 15], fill=(217, 119, 6, 255), width=2)
                draw.line([cx2, cy, cx2, cy + 15], fill=(217, 119, 6, 255), width=2)

    # ==========================================
    # PANEL 1: CORRUPTED FOYER & MALICE BARRIER
    # ==========================================
    p1_ox, p1_oy = 30, 95
    draw_dark_castle_room_base(p1_ox, p1_oy)

    # 4 Corner Torches lit with Flame Lantern
    for tx, ty in [(p1_ox + 48, p1_oy + 40), (p1_ox + 350, p1_oy + 40),
                  (p1_ox + 48, p1_oy + 160), (p1_ox + 350, p1_oy + 160)]:
        draw.rectangle([tx - 4, ty, tx + 4, ty + 12], fill=(24, 24, 27, 255))
        draw.polygon([(tx - 5, ty), (tx, ty - 10), (tx + 5, ty)], fill=(234, 88, 12, 255))
        draw.polygon([(tx - 2, ty), (tx, ty - 6), (tx + 2, ty)], fill=(254, 240, 138, 255))

    # Four Sword Diamond Split Pads
    pad_cx, pad_cy = p1_ox + 208, p1_oy + 140
    for px, py in [(pad_cx, pad_cy - 20), (pad_cx - 24, pad_cy), (pad_cx + 24, pad_cy), (pad_cx, pad_cy + 20)]:
        draw.rectangle([px - 8, py - 8, px + 8, py + 8], fill=(2, 132, 199, 255), outline=(56, 189, 248, 255))
        draw.rectangle([px - 5, py - 5, px + 5, py + 5], fill=(56, 189, 248, 255))
        draw.rectangle([px - 2, py - 2, px + 2, py + 2], fill=(255, 255, 255, 255))

    # Pulsing Malice Barrier at North Gate
    bar_x, bar_y = p1_ox + 176, p1_oy + 16
    draw.rectangle([bar_x, bar_y, bar_x + 64, bar_y + 20], fill=(76, 5, 25, 255), outline=(225, 29, 72, 255), width=2)
    # Toxic Malice Eye
    draw.ellipse([bar_x + 22, bar_y + 3, bar_x + 42, bar_y + 17], fill=(244, 63, 94, 255), outline=(254, 205, 211, 255))
    draw.ellipse([bar_x + 29, bar_y + 6, bar_x + 35, bar_y + 14], fill=(136, 19, 55, 255))
    draw.point((bar_x + 31, bar_y + 8), fill=(255, 255, 255, 255))

    # Link standing near split pads
    draw_link(pad_cx, pad_cy - 35, (22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255), dir_down=False, holding_sword=True)

    draw.text((p1_ox + 70, p1_oy + 195), "Light 4 Torches & 4 Split Pads to dissolve Malice Barrier!", fill=(253, 224, 71, 255), font=font_small)


    # ==========================================
    # PANEL 2: WEST WING: 4-CLONE SYNCHRONIZED CHASM
    # ==========================================
    p2_ox, p2_oy = 515, 95
    draw_dark_castle_room_base(p2_ox, p2_oy, has_chasm=True)

    # 4 Simultaneous Eye Switches across the northern ledge
    eye_x = [p2_ox + 80, p2_ox + 160, p2_ox + 240, p2_ox + 320]
    eye_y = p2_oy + 45
    for ex in eye_x:
        draw.rectangle([ex - 9, eye_y - 9, ex + 9, eye_y + 9], fill=(71, 85, 105, 255))
        draw.ellipse([ex - 7, eye_y - 7, ex + 7, eye_y + 7], fill=(34, 197, 94, 255), outline=(187, 247, 208, 255)) # Green hit!
        draw.ellipse([ex - 3, eye_y - 3, ex + 3, eye_y + 3], fill=(20, 83, 45, 255))

    # Glowing Cyan Hard Light Bridge extending over the abyss
    bridge_x1 = p2_ox + 140
    bridge_x2 = p2_ox + 260
    draw.rectangle([bridge_x1, p2_oy + 75, bridge_x2, p2_oy + 130], fill=(56, 189, 248, 220), outline=(224, 242, 254, 255), width=2)
    # Bridge energy grid lines
    for by in range(p2_oy + 80, p2_oy + 130, 10):
        draw.line([bridge_x1, by, bridge_x2, by], fill=(255, 255, 255, 180), width=1)

    # Treasure Chest on far ledge
    cx, cy = p2_ox + 200, p2_oy + 40
    draw.rectangle([cx - 10, cy - 8, cx + 10, cy + 8], fill=(180, 83, 9, 255), outline=(245, 158, 11, 255))
    draw.rectangle([cx - 8, cy - 6, cx + 8, cy + 6], fill=(245, 158, 11, 255))
    draw.rectangle([cx - 2, cy - 1, cx + 2, cy + 3], fill=(120, 53, 15, 255))

    # Four Links lined up firing arrows/swords simultaneously at the 4 eyes!
    tunic_colors = [
        ((22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255)),   # Green
        ((220, 38, 38, 255), (185, 28, 28, 255), (239, 68, 68, 255)),   # Red
        ((37, 99, 235, 255), (29, 78, 216, 255), (59, 130, 246, 255)),   # Blue
        ((147, 51, 234, 255), (126, 34, 206, 255), (168, 85, 247, 255)) # Purple
    ]
    for i, ex in enumerate(eye_x):
        tc, tdark, hc = tunic_colors[i]
        draw_link(ex, p2_oy + 155, tc, tdark, hc, dir_down=False, holding_sword=True)
        # Arrow flight trail to each eye
        draw.line([ex, p2_oy + 145, ex, eye_y + 10], fill=(254, 240, 138, 255), width=2)
        draw.ellipse([ex - 4, eye_y + 6, ex + 4, eye_y + 14], fill=(250, 204, 21, 255))

    draw.text((p2_ox + 65, p2_oy + 195), "Simultaneous 4-Clone arrow strike extends Hard Light Bridge!", fill=(254, 240, 138, 255), font=font_small)


    # ==========================================
    # PANEL 3: EAST WING & BLACK KNIGHT DUEL
    # ==========================================
    p3_ox, p3_oy = 30, 395
    draw_dark_castle_room_base(p3_ox, p3_oy)

    # Heavy Obsidian Monolith Block (slid forward)
    mx, my = p3_ox + 100, p3_oy + 75
    draw.rectangle([mx - 18, my - 18, mx + 18, my + 18], fill=(24, 24, 36, 255), outline=(49, 46, 75, 255), width=2)
    draw.rectangle([mx - 14, my - 14, mx + 14, my + 14], fill=(49, 46, 129, 255))
    draw.rectangle([mx - 6, my - 6, mx + 6, my + 6], fill=(217, 119, 6, 255))

    # Four Links actively pushing the monolith together
    draw_link(mx - 12, my + 24, (22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255), dir_down=False, pushing=True)
    draw_link(mx - 4, my + 24, (220, 38, 38, 255), (185, 28, 28, 255), (239, 68, 68, 255), dir_down=False, pushing=True)
    draw_link(mx + 4, my + 24, (37, 99, 235, 255), (29, 78, 216, 255), (59, 130, 246, 255), dir_down=False, pushing=True)
    draw_link(mx + 12, my + 24, (147, 51, 234, 255), (126, 34, 206, 255), (168, 85, 247, 255), dir_down=False, pushing=True)

    # Darknut / Black Knight Miniboss Arena (Right side)
    dk_x, dk_y = p3_ox + 290, p3_oy + 90
    draw.ellipse([dk_x - 12, dk_y + 12, dk_x + 12, dk_y + 18], fill=(0, 0, 0, 100))
    # Dark Purple Cape
    draw.polygon([(dk_x - 12, dk_y - 8), (dk_x + 12, dk_y - 8), (dk_x + 18, dk_y + 18), (dk_x - 18, dk_y + 18)], fill=(107, 33, 168, 255))
    # Heavy Black Iron Armor
    draw.rectangle([dk_x - 9, dk_y - 12, dk_x + 9, dk_y + 12], fill=(24, 24, 27, 255), outline=(245, 158, 11, 255))
    # Horned Great Helmet
    draw.rectangle([dk_x - 8, dk_y - 20, dk_x + 8, dk_y - 10], fill=(24, 24, 27, 255))
    draw.polygon([(dk_x - 11, dk_y - 23), (dk_x - 7, dk_y - 19), (dk_x - 7, dk_y - 13)], fill=(245, 158, 11, 255))
    draw.polygon([(dk_x + 11, dk_y - 23), (dk_x + 7, dk_y - 19), (dk_x + 7, dk_y - 13)], fill=(245, 158, 11, 255))
    draw.rectangle([dk_x - 4, dk_y - 16, dk_x + 4, dk_y - 14], fill=(239, 68, 68, 255)) # Red glowing eye slit
    # Heavy Iron Tower Shield
    draw.rectangle([dk_x - 18, dk_y - 6, dk_x - 9, dk_y + 12], fill=(71, 85, 105, 255), outline=(245, 158, 11, 255))
    # Colossal Broadsword
    draw.rectangle([dk_x + 10, dk_y - 16, dk_x + 14, dk_y + 14], fill=(226, 232, 240, 255))
    draw.rectangle([dk_x + 8, dk_y + 2, dk_x + 16, dk_y + 5], fill=(245, 158, 11, 255))
    draw.text((dk_x - 30, dk_y - 32), "Darknut (Black Knight)", fill=(248, 113, 113, 255), font=font_small)

    # Sanctum Key dropped
    key_x, key_y = p3_ox + 290, p3_oy + 140
    draw.ellipse([key_x - 8, key_y - 8, key_x + 8, key_y + 8], fill=(253, 224, 71, 100))
    draw.ellipse([key_x - 5, key_y - 5, key_x + 5, key_y + 5], fill=(245, 158, 11, 255), outline=(254, 240, 138, 255))
    draw.line([key_x, key_y + 4, key_x, key_y + 12], fill=(245, 158, 11, 255), width=2)
    draw.text((key_x + 10, key_y + 2), "Sanctum Boss Key", fill=(254, 240, 138, 255), font=font_small)

    draw.text((p3_ox + 70, p3_oy + 195), "Flank the Darknut's heavy shield to claim the Sanctum Boss Key!", fill=(253, 224, 71, 255), font=font_small)


    # ==========================================
    # PANEL 4: PETRIFICATION BELLS & SANCTUM GATE
    # ==========================================
    p4_ox, p4_oy = 515, 395
    draw_dark_castle_room_base(p4_ox, p4_oy)

    # 3 Massive Petrification Bells suspended on dark iron chains
    bells_x = [p4_ox + 90, p4_ox + 208, p4_ox + 326]
    bell_y = p4_oy + 40
    for i, bx in enumerate(bells_x):
        # Heavy chains
        draw.line([bx, p4_oy + 16, bx, bell_y], fill=(71, 85, 105, 255), width=3)
        # Ancient Bronze Bell body
        draw.rectangle([bx - 14, bell_y, bx + 14, bell_y + 20], fill=(180, 83, 9, 255), outline=(120, 53, 15, 255), width=2)
        draw.rectangle([bx - 18, bell_y + 16, bx + 18, bell_y + 24], fill=(120, 53, 15, 255))
        # Bell clapper
        draw.ellipse([bx - 3, bell_y + 22, bx + 3, bell_y + 28], fill=(245, 158, 11, 255))
        # Silenced holy aura!
        draw.rectangle([bx - 10, bell_y + 6, bx + 10, bell_y + 12], fill=(74, 222, 128, 255))
        draw.text((bx - 22, bell_y + 28), f"Bell #{i+1} [SILENT]", fill=(74, 222, 128, 255), font=font_small)

    # Central Four Sword Elemental Altar charging
    al_x, al_y = p4_ox + 208, p4_oy + 130
    draw.rectangle([al_x - 22, al_y - 22, al_x + 22, al_y + 22], fill=(49, 46, 129, 255), outline=(67, 56, 202, 255), width=2)
    draw.rectangle([al_x - 14, al_y - 14, al_x + 14, al_y + 14], fill=(56, 189, 248, 255), outline=(255, 255, 255, 255))
    draw.polygon([(al_x, al_y - 8), (al_x - 8, al_y + 6), (al_x + 8, al_y + 6)], fill=(245, 158, 11, 255)) # Triforce radiant charge!

    # The 4 Clones channeling energy at the altar
    draw_link(al_x - 28, al_y, (22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255), dir_down=True, holding_sword=True)
    draw_link(al_x, al_y - 28, (220, 38, 38, 255), (185, 28, 28, 255), (239, 68, 68, 255), dir_down=True, holding_sword=True)
    draw_link(al_x + 28, al_y, (37, 99, 235, 255), (29, 78, 216, 255), (59, 130, 246, 255), dir_down=True, holding_sword=True)
    draw_link(al_x, al_y + 28, (147, 51, 234, 255), (126, 34, 206, 255), (168, 85, 247, 255), dir_down=False, holding_sword=True)

    # Grand Sanctum Gate Unsealed (Portal to Vaati's Climax!)
    gate_x, gate_y = p4_ox + 180, p4_oy + 16
    draw.rectangle([gate_x, gate_y, gate_x + 56, gate_y + 18], fill=(30, 27, 75, 255), outline=(245, 158, 11, 255), width=2)
    draw.rectangle([gate_x + 10, gate_y + 2, gate_x + 46, gate_y + 16], fill=(253, 224, 71, 180)) # Golden portal light
    draw.text((gate_x + 4, gate_y + 4), "[SANCTUM OPEN]", fill=(24, 24, 27, 255), font=font_small)

    # Celebratory Banner at bottom
    draw.rectangle([p4_ox + 20, p4_oy + 180, p4_ox + 396, p4_oy + 215], fill=(24, 24, 36, 245), outline=(225, 29, 72, 255), width=2)
    draw.text((p4_ox + 32, p4_oy + 186), "PORTAO DO SANTUARIO DESLACRADO!", fill=(253, 224, 71, 255), font=font_small)
    draw.text((p4_ox + 32, p4_oy + 198), "Os 3 Sinos foram silenciados! Vaati aguarda no topo para o confronto final!", fill=(254, 205, 211, 255), font=font_small)

    os.makedirs(out_dir, exist_ok=True)
    img.save(out_path)
    print(f"Showcase image saved to {out_path}")
    shutil.copyfile(out_path, scratch_path)
    print(f"Showcase image copied to {scratch_path}")

if __name__ == "__main__":
    main()
