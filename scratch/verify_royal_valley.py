#!/usr/bin/env python3
"""
scratch/verify_royal_valley.py
Generates royal_valley_showcase.png showcasing:
1. Royal Valley Entrance & Spectral Mist Maze (Lost Woods navigation, volumetric fog)
2. Gravedigger Dampé's Cabin, Locked Iron Gate & Takkar Crow with Graveyard Key
3. Hyrule Graveyard, Floating Ghinis/Poes & 4-Link Diamond Split opening King Gustaf's Monumental Tomb
4. Subterranean Royal Crypt: Lantern-lit Torches, 4 Heavy Switches & King Gustaf Spirit (Royal Golden Kinstone)
"""

import os
import shutil
import math
from PIL import Image, ImageDraw, ImageFont

def main():
    conv_id = "91f45871-4e2f-495f-88d6-7427ed06770a"
    out_dir = os.path.join(r"C:\Users\thiag\.gemini\antigravity\brain", conv_id)
    out_path = os.path.join(out_dir, "royal_valley_showcase.png")
    scratch_path = r"scratch/royal_valley_showcase.png"

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
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Royal Valley & Hyrule Graveyard (Ato V)", fill=(254, 240, 138, 255), font=font_title)
    draw.text((700, 16), "Sprint 5: feature/royal-valley-graveyard", fill=(56, 189, 248, 255), font=font_sub)

    panels = [
        ("1. ROYAL VALLEY ENTRANCE & SPECTRAL MIST MAZE (6-STEP LOST WOODS NAVIGATION)", 20, 60, 475, 340),
        ("2. GRAVEDIGGER DAMPÉ'S CABIN, LOCKED IRON GATE & TAKKAR CROW ENCOUNTER", 505, 60, 960, 340),
        ("3. HYRULE GRAVEYARD, GHINIS & 4-LINK DIAMOND SPLIT KING GUSTAF TOMB", 20, 360, 475, 640),
        ("4. ROYAL CRYPT: LANTERN TORCHES, 4-SWITCHES & KING GUSTAF SPIRIT", 505, 360, 960, 640)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(15, 20, 32, 255), outline=(40, 55, 80, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(24, 34, 54, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(245, 158, 11, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(253, 224, 71, 255), font=font_small)

    def draw_link(lx, ly, tunic_col, tunic_dark, hat_col, dir_down=True, holding_item=None, pushing=False):
        # Shadow
        draw.ellipse([lx - 6, ly + 6, lx + 6, ly + 10], fill=(0, 0, 0, 100))
        # Body
        draw.rectangle([lx - 4, ly - 3, lx + 4, ly + 5], fill=tunic_col, outline=tunic_dark)
        # Belt
        draw.rectangle([lx - 4, ly + 2, lx + 4, ly + 4], fill=(120, 53, 15, 255))
        draw.rectangle([lx - 1, ly + 2, lx + 1, ly + 4], fill=(250, 204, 21, 255))
        # Boots
        draw.rectangle([lx - 4, ly + 5, lx - 1, ly + 8], fill=(113, 63, 18, 255))
        draw.rectangle([lx + 1, ly + 5, lx + 4, ly + 8], fill=(113, 63, 18, 255))
        # Face / Head
        draw.rectangle([lx - 3, ly - 9, lx + 3, ly - 3], fill=(254, 215, 170, 255))
        if dir_down:
            draw.point((lx - 1, ly - 6), fill=(15, 23, 42, 255))
            draw.point((lx + 2, ly - 6), fill=(15, 23, 42, 255))
        # Cap
        draw.polygon([(lx - 5, ly - 8), (lx + 5, ly - 8), (lx + 2, ly - 14), (lx - 2, ly - 14)], fill=hat_col)
        # Ezlo cap crest
        draw.ellipse([lx - 1, ly - 15, lx + 2, ly - 12], fill=(234, 179, 8, 255))

        if pushing:
            # Hands reaching forward
            draw.rectangle([lx - 6, ly - 2, lx - 4, ly + 1], fill=(254, 215, 170, 255))
            draw.rectangle([lx + 4, ly - 2, lx + 6, ly + 1], fill=(254, 215, 170, 255))

        if holding_item == "lantern":
            # Flame Lantern in Link's hand
            draw.rectangle([lx + 5, ly - 2, lx + 9, ly + 4], fill=(180, 83, 9, 255))
            draw.ellipse([lx + 6, ly - 5, lx + 8, ly - 1], fill=(251, 146, 60, 255))
            draw.ellipse([lx + 4, ly - 7, lx + 10, ly + 6], fill=(251, 191, 36, 60))

    def draw_gnarled_tree(tx, ty):
        # Dark spooky gnarled tree
        draw.rectangle([tx - 4, ty, tx + 4, ty + 24], fill=(39, 39, 42, 255))
        # Twisted branches
        draw.line([tx - 3, ty + 12, tx - 12, ty + 4], fill=(39, 39, 42, 255), width=3)
        draw.line([tx + 3, ty + 8, tx + 14, ty - 2], fill=(39, 39, 42, 255), width=3)
        draw.line([tx - 12, ty + 4, tx - 16, ty - 4], fill=(39, 39, 42, 255), width=2)
        # Mossy leaves
        draw.ellipse([tx - 18, ty - 16, tx + 18, ty + 6], fill=(20, 83, 45, 230), outline=(5, 46, 22, 255))
        draw.ellipse([tx - 12, ty - 22, tx + 8, ty - 6], fill=(22, 101, 52, 230))

    # ==========================================
    # PANEL 1: ROYAL VALLEY ENTRANCE & MIST MAZE
    # ==========================================
    p1_ox, p1_oy = 30, 95
    # Dark grass ground
    for gy in range(14):
        for gx in range(26):
            col = (15, 23, 42, 255) if (gx + gy) % 2 == 0 else (30, 41, 59, 255)
            draw.rectangle([p1_ox + gx * 16, p1_oy + gy * 16, p1_ox + gx * 16 + 15, p1_oy + gy * 16 + 15], fill=col)

    # Dark stream flowing across the valley
    for wy in range(14):
        wx = p1_ox + 48 + int(math.sin(wy * 0.5) * 8)
        draw.rectangle([wx, p1_oy + wy * 16, wx + 20, p1_oy + wy * 16 + 15], fill=(8, 47, 73, 255))
        draw.line([wx + 2, p1_oy + wy * 16 + 8, wx + 18, p1_oy + wy * 16 + 8], fill=(56, 189, 248, 120))

    # Path from Sanctuary
    draw.rectangle([p1_ox + 180, p1_oy, p1_ox + 236, p1_oy + 224], fill=(24, 24, 27, 255))
    draw.line([p1_ox + 180, p1_oy, p1_ox + 180, p1_oy + 224], fill=(51, 65, 85, 255), width=1)
    draw.line([p1_ox + 236, p1_oy, p1_ox + 236, p1_oy + 224], fill=(51, 65, 85, 255), width=1)

    # Gnarled gothic trees
    for tx, ty in [(p1_ox + 20, p1_oy + 20), (p1_ox + 30, p1_oy + 140), (p1_ox + 110, p1_oy + 80),
                  (p1_ox + 320, p1_oy + 30), (p1_ox + 390, p1_oy + 110), (p1_ox + 290, p1_oy + 170)]:
        draw_gnarled_tree(tx, ty)

    # Ancient warning stone tablets
    draw.rectangle([p1_ox + 150, p1_oy + 60, p1_ox + 166, p1_oy + 78], fill=(100, 116, 139, 255), outline=(51, 65, 85, 255))
    draw.rectangle([p1_ox + 152, p1_oy + 64, p1_ox + 164, p1_oy + 66], fill=(30, 41, 59, 255))
    draw.rectangle([p1_ox + 152, p1_oy + 70, p1_ox + 160, p1_oy + 72], fill=(30, 41, 59, 255))

    # Link walking into the mist
    draw_link(p1_ox + 208, p1_oy + 130, (22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255), dir_down=True)

    # Volumetric spectral mist layers (translucent white/cyan drifting clouds)
    overlay = Image.new("RGBA", (img_w, img_h), (0, 0, 0, 0))
    ov_draw = ImageDraw.Draw(overlay)
    for fx, fy, fw, fh in [
        (p1_ox + 10, p1_oy + 20, 140, 50),
        (p1_ox + 100, p1_oy + 90, 200, 65),
        (p1_ox + 240, p1_oy + 150, 160, 55),
        (p1_ox + 50, p1_oy + 170, 180, 50),
        (p1_ox + 260, p1_oy + 40, 140, 45)
    ]:
        ov_draw.ellipse([fx, fy, fx + fw, fy + fh], fill=(224, 242, 254, 45))
    img = Image.alpha_composite(img, overlay)
    draw = ImageDraw.Draw(img)

    # Compass Navigation Guide Overlay
    draw.rectangle([p1_ox + 240, p1_oy + 10, p1_ox + 400, p1_oy + 75], fill=(15, 23, 42, 230), outline=(56, 189, 248, 255))
    draw.text((p1_ox + 246, p1_oy + 14), "MIST MAZE NAVIGATION (6-STEP)", fill=(56, 189, 248, 255), font=font_small)
    draw.text((p1_ox + 246, p1_oy + 28), "Seq: DOWN, LEFT, LEFT, UP, RIGHT, UP", fill=(254, 240, 138, 255), font=font_small)
    draw.text((p1_ox + 246, p1_oy + 42), "Current Progress: [Step 3/6 - LEFT OK]", fill=(74, 222, 128, 255), font=font_small)
    draw.text((p1_ox + 246, p1_oy + 56), "Wrong Path: Spectral winds reset entrance", fill=(248, 113, 113, 255), font=font_small)


    # ==========================================
    # PANEL 2: DAMPÉ'S CABIN, GATE & TAKKAR CROW
    # ==========================================
    p2_ox, p2_oy = 515, 95
    # Dark grass ground
    for gy in range(14):
        for gx in range(26):
            col = (15, 23, 42, 255) if (gx + gy) % 2 == 0 else (30, 41, 59, 255)
            draw.rectangle([p2_ox + gx * 16, p2_oy + gy * 16, p2_ox + gx * 16 + 15, p2_oy + gy * 16 + 15], fill=col)

    # Dampé's Spooky Cabin (Left side)
    cx, cy = p2_ox + 20, p2_oy + 30
    draw.rectangle([cx, cy + 20, cx + 110, cy + 100], fill=(87, 83, 78, 255), outline=(41, 37, 36, 255), width=2)
    # Roof
    draw.polygon([(cx - 10, cy + 20), (cx + 55, cy - 15), (cx + 120, cy + 20)], fill=(120, 53, 15, 255), outline=(69, 26, 3, 255))
    # Chimney with smoke
    draw.rectangle([cx + 80, cy - 25, cx + 96, cy + 5], fill=(68, 64, 60, 255))
    draw.ellipse([cx + 82, cy - 35, cx + 94, cy - 27], fill=(214, 211, 209, 120))
    draw.ellipse([cx + 86, cy - 45, cx + 102, cy - 34], fill=(214, 211, 209, 80))
    # Cabin door and warm window
    draw.rectangle([cx + 40, cy + 55, cx + 70, cy + 100], fill=(69, 26, 3, 255))
    draw.rectangle([cx + 12, cy + 45, cx + 32, cy + 65], fill=(251, 191, 36, 220), outline=(69, 26, 3, 255))
    draw.line([cx + 22, cy + 45, cx + 22, cy + 65], fill=(69, 26, 3, 255))
    draw.line([cx + 12, cy + 55, cx + 32, cy + 55], fill=(69, 26, 3, 255))

    # Dampé the gravedigger NPC outside his cabin
    dampe_x, dampe_y = cx + 85, cy + 120
    draw.ellipse([dampe_x - 7, dampe_y + 8, dampe_x + 7, dampe_y + 12], fill=(0, 0, 0, 100))
    draw.rectangle([dampe_x - 6, dampe_y - 4, dampe_x + 6, dampe_y + 7], fill=(161, 98, 7, 255)) # Brown ragged coat
    draw.ellipse([dampe_x - 5, dampe_y - 12, dampe_x + 5, dampe_y - 3], fill=(254, 215, 170, 255)) # Hunchback head
    draw.ellipse([dampe_x + 6, dampe_y, dampe_x + 12, dampe_y + 8], fill=(250, 204, 21, 240)) # Holding lantern
    draw.text((dampe_x - 18, dampe_y + 14), "Dampé", fill=(253, 224, 71, 255), font=font_small)

    # Wrought-Iron Cemetery Gate (Center-Right)
    gate_x, gate_y = p2_ox + 190, p2_oy + 50
    # Stone pillars
    draw.rectangle([gate_x - 12, gate_y, gate_x, gate_y + 90], fill=(71, 85, 105, 255), outline=(30, 41, 59, 255))
    draw.rectangle([gate_x + 90, gate_y, gate_x + 102, gate_y + 90], fill=(71, 85, 105, 255), outline=(30, 41, 59, 255))
    # Iron spikes top
    draw.polygon([(gate_x - 14, gate_y), (gate_x - 6, gate_y - 12), (gate_x + 2, gate_y)], fill=(30, 41, 59, 255))
    draw.polygon([(gate_x + 88, gate_y), (gate_x + 96, gate_y - 12), (gate_x + 104, gate_y)], fill=(30, 41, 59, 255))
    # Iron bars & padlocked chain
    for bar_x in range(gate_x + 6, gate_x + 86, 8):
        draw.line([bar_x, gate_y + 6, bar_x, gate_y + 86], fill=(51, 65, 85, 255), width=2)
        draw.polygon([(bar_x - 2, gate_y + 6), (bar_x, gate_y - 2), (bar_x + 2, gate_y + 6)], fill=(30, 41, 59, 255))
    draw.rectangle([gate_x + 40, gate_y + 40, gate_x + 52, gate_y + 54], fill=(245, 158, 11, 255), outline=(120, 53, 15, 255)) # Keyhole padlock

    # Gnarled tree on right
    draw_gnarled_tree(p2_ox + 360, p2_oy + 90)

    # Takkar Crow flying / perched holding the Graveyard Key!
    crow_x, crow_y = p2_ox + 335, p2_oy + 40
    # Crow body & wings
    draw.ellipse([crow_x - 8, crow_y - 5, crow_x + 8, crow_y + 5], fill=(24, 24, 27, 255))
    draw.polygon([(crow_x - 4, crow_y - 2), (crow_x - 16, crow_y - 14), (crow_x, crow_y - 4)], fill=(39, 39, 42, 255)) # Wing L
    draw.polygon([(crow_x + 4, crow_y - 2), (crow_x + 16, crow_y - 14), (crow_x, crow_y - 4)], fill=(39, 39, 42, 255)) # Wing R
    draw.polygon([(crow_x - 8, crow_y), (crow_x - 14, crow_y + 1), (crow_x - 8, crow_y + 3)], fill=(234, 179, 8, 255)) # Yellow beak
    # Graveyard Key held in beak!
    draw.ellipse([crow_x - 17, crow_y + 3, crow_x - 11, crow_y + 9], outline=(250, 204, 21, 255), width=2)
    draw.line([crow_x - 14, crow_y + 9, crow_x - 14, crow_y + 18], fill=(250, 204, 21, 255), width=2)
    draw.line([crow_x - 14, crow_y + 15, crow_x - 10, crow_y + 15], fill=(250, 204, 21, 255), width=2)
    draw.text((crow_x - 30, crow_y - 22), "Takkar Crow", fill=(248, 113, 113, 255), font=font_small)

    # Link aiming bow / sword at Takkar
    draw_link(p2_ox + 260, p2_oy + 140, (22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255), dir_down=False)
    # Bow and arrow projectile striking crow
    draw.line([p2_ox + 265, p2_oy + 130, crow_x - 10, crow_y + 10], fill=(254, 240, 138, 255), width=2)
    draw.ellipse([crow_x - 12, crow_y + 8, crow_x + 4, crow_y + 24], fill=(253, 224, 71, 90)) # Sparkle hit!

    draw.text((p2_ox + 160, p2_oy + 185), "Arrow strike drops Graveyard Key to unlock gate!", fill=(254, 240, 138, 255), font=font_small)


    # ==========================================
    # PANEL 3: HYRULE GRAVEYARD & KING GUSTAF TOMB
    # ==========================================
    p3_ox, p3_oy = 30, 395
    # Dark gloomy grass
    for gy in range(14):
        for gx in range(26):
            col = (15, 23, 42, 255) if (gx + gy) % 2 == 0 else (30, 41, 59, 255)
            draw.rectangle([p3_ox + gx * 16, p3_oy + gy * 16, p3_ox + gx * 16 + 15, p3_oy + gy * 16 + 15], fill=col)

    # Ancient tombstones scattered
    for gbx, gby in [(p3_ox + 35, p3_oy + 40), (p3_ox + 100, p3_oy + 130), (p3_ox + 330, p3_oy + 45), (p3_ox + 380, p3_oy + 140)]:
        draw.rectangle([gbx, gby, gbx + 18, gby + 24], fill=(148, 163, 184, 255), outline=(71, 85, 105, 255))
        draw.line([gbx + 4, gby + 6, gbx + 14, gby + 6], fill=(51, 65, 85, 255), width=2)
        draw.line([gbx + 9, gby + 6, gbx + 9, gby + 16], fill=(51, 65, 85, 255), width=2)

    # Ghinis and Poes floating (spectral blue/purple ghosts)
    for ghx, ghy, col_ghost in [(p3_ox + 60, p3_oy + 90, (56, 189, 248, 200)), (p3_ox + 350, p3_oy + 100, (192, 132, 252, 200))]:
        # Spectral aura
        draw.ellipse([ghx - 14, ghy - 14, ghx + 14, ghy + 14], fill=(56, 189, 248, 40))
        # Ghost body
        draw.ellipse([ghx - 9, ghy - 10, ghx + 9, ghy + 8], fill=col_ghost)
        # Big spooky single eye (Ghini)
        draw.ellipse([ghx - 4, ghy - 4, ghx + 4, ghy + 2], fill=(255, 255, 255, 255))
        draw.ellipse([ghx - 1, ghy - 2, ghx + 2, ghy + 1], fill=(220, 38, 38, 255))
        # Tail
        draw.polygon([(ghx - 7, ghy + 6), (ghx + 7, ghy + 6), (ghx, ghy + 14)], fill=col_ghost)

    # Monumental Tomb of King Gustaf (Center)
    tx, ty = p3_ox + 165, p3_oy + 25
    # Tomb stone base
    draw.rectangle([tx, ty + 10, tx + 85, ty + 80], fill=(51, 65, 85, 255), outline=(30, 41, 59, 255), width=2)
    # Royal crest engraved on tomb
    draw.polygon([(tx + 32, ty + 40), (tx + 52, ty + 40), (tx + 42, ty + 25)], fill=(245, 158, 11, 255)) # Triforce top
    draw.polygon([(tx + 22, ty + 55), (tx + 42, ty + 55), (tx + 32, ty + 40)], fill=(245, 158, 11, 255)) # Triforce L
    draw.polygon([(tx + 42, ty + 55), (tx + 62, ty + 55), (tx + 52, ty + 40)], fill=(245, 158, 11, 255)) # Triforce R

    # The heavy stone lid has slid backward (revealing the crypt stairs!)
    draw.rectangle([tx + 10, ty - 8, tx + 75, ty + 20], fill=(100, 116, 139, 255), outline=(30, 41, 59, 255), width=2) # Slid lid
    # Dark open stairwell down into the Royal Crypt!
    draw.rectangle([tx + 15, ty + 58, tx + 70, ty + 78], fill=(10, 15, 25, 255))
    for step_i in range(4):
        draw.line([tx + 20, ty + 62 + step_i * 4, tx + 65, ty + 62 + step_i * 4], fill=(71, 85, 105, 255), width=2)

    # 4-Link Diamond Split Formation pushing the Tomb!
    # Diamond Pad positions:
    pad_center_x, pad_center_y = tx + 42, ty + 125
    pads = [
        (pad_center_x, pad_center_y - 24, "UP"),
        (pad_center_x - 30, pad_center_y, "LEFT"),
        (pad_center_x + 30, pad_center_y, "RIGHT"),
        (pad_center_x, pad_center_y + 24, "DOWN")
    ]
    for px, py, label in pads:
        # Diamond split pad
        draw.rectangle([px - 10, py - 10, px + 10, py + 10], fill=(2, 132, 199, 255), outline=(56, 189, 248, 255))
        draw.rectangle([px - 6, py - 6, px + 6, py + 6], fill=(56, 189, 248, 255))
        draw.rectangle([px - 2, py - 2, px + 2, py + 2], fill=(255, 255, 255, 255))

    # Four Links actively pushing King Gustaf's tomb together:
    # 1. Green Link (Original)
    draw_link(tx + 25, ty + 88, (22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255), dir_down=False, pushing=True)
    # 2. Red Link
    draw_link(tx + 40, ty + 88, (220, 38, 38, 255), (185, 28, 28, 255), (239, 68, 68, 255), dir_down=False, pushing=True)
    # 3. Blue Link
    draw_link(tx + 55, ty + 88, (37, 99, 235, 255), (29, 78, 216, 255), (59, 130, 246, 255), dir_down=False, pushing=True)
    # 4. Purple Link
    draw_link(tx + 70, ty + 88, (147, 51, 234, 255), (126, 34, 206, 255), (168, 85, 247, 255), dir_down=False, pushing=True)

    # Action caption
    draw.text((p3_ox + 90, p3_oy + 195), "Combined 4-Link hero strength slides King Gustaf's tomb lid!", fill=(253, 224, 71, 255), font=font_small)


    # ==========================================
    # PANEL 4: ROYAL CRYPT & KING GUSTAF SPIRIT
    # ==========================================
    p4_ox, p4_oy = 515, 395
    # Ancient subterranean stone crypt tiles
    for gy in range(14):
        for gx in range(26):
            col = (30, 41, 59, 255) if (gx + gy) % 2 == 0 else (15, 23, 42, 255)
            draw.rectangle([p4_ox + gx * 16, p4_oy + gy * 16, p4_ox + gx * 16 + 15, p4_oy + gy * 16 + 15], fill=col)
            draw.rectangle([p4_ox + gx * 16, p4_oy + gy * 16, p4_ox + gx * 16 + 15, p4_oy + gy * 16 + 15], outline=(51, 65, 85, 80))

    # Crypt perimeter stone walls
    draw.rectangle([p4_ox, p4_oy, p4_ox + 416, p4_oy + 24], fill=(15, 23, 42, 255), outline=(51, 65, 85, 255))
    draw.line([p4_ox, p4_oy + 24, p4_ox + 416, p4_oy + 24], fill=(245, 158, 11, 255))

    # Torches lit with Flame Lantern
    torch_positions = [(p4_ox + 50, p4_oy + 45), (p4_ox + 360, p4_oy + 45), (p4_ox + 50, p4_oy + 150), (p4_ox + 360, p4_oy + 150)]
    for tox, toy in torch_positions:
        # Stone sconce
        draw.rectangle([tox - 4, toy, tox + 4, toy + 12], fill=(71, 85, 105, 255))
        # Warm firelight glow aura
        draw.ellipse([tox - 22, toy - 22, tox + 22, toy + 18], fill=(251, 146, 60, 40))
        # Animated flame
        draw.polygon([(tox - 4, toy), (tox, toy - 10), (tox + 4, toy)], fill=(239, 68, 68, 255))
        draw.polygon([(tox - 2, toy), (tox, toy - 7), (tox + 2, toy)], fill=(254, 240, 138, 255))

    # 4 Heavy Floor Switches pressed simultaneously
    switches = [
        (p4_ox + 120, p4_oy + 140),
        (p4_ox + 150, p4_oy + 140),
        (p4_ox + 180, p4_oy + 140),
        (p4_ox + 210, p4_oy + 140)
    ]
    tunic_colors = [
        ((22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255)),
        ((220, 38, 38, 255), (185, 28, 28, 255), (239, 68, 68, 255)),
        ((37, 99, 235, 255), (29, 78, 216, 255), (59, 130, 246, 255)),
        ((147, 51, 234, 255), (126, 34, 206, 255), (168, 85, 247, 255))
    ]
    for i, (swx, swy) in enumerate(switches):
        # Pressed switch base (yellow active)
        draw.rectangle([swx - 9, swy - 9, swx + 9, swy + 9], fill=(234, 179, 8, 255), outline=(161, 98, 7, 255), width=2)
        draw.rectangle([swx - 6, swy - 6, swx + 6, swy + 6], fill=(250, 204, 21, 255))
        # Clone standing on switch
        tc, tdark, hc = tunic_colors[i]
        draw_link(swx, swy - 2, tc, tdark, hc, dir_down=False, holding_item=("lantern" if i == 0 else None))

    # Crypt Iron Gate Opened (receded into ceiling)
    draw.rectangle([p4_ox + 130, p4_oy + 24, p4_ox + 200, p4_oy + 30], fill=(71, 85, 105, 255))
    draw.text((p4_ox + 135, p4_oy + 32), "[CRYPT GATE OPEN]", fill=(74, 222, 128, 255), font=font_small)

    # King Gustaf's Spectral Ghost (Center Top)
    kg_x, kg_y = p4_ox + 208, p4_oy + 65
    # Grand golden spectral aura
    draw.ellipse([kg_x - 35, kg_y - 35, kg_x + 35, kg_y + 35], fill=(253, 224, 71, 55))
    draw.ellipse([kg_x - 22, kg_y - 22, kg_x + 22, kg_y + 22], fill=(254, 240, 138, 90))
    # King Gustaf ethereal regal robe
    draw.rectangle([kg_x - 12, kg_y - 6, kg_x + 12, kg_y + 16], fill=(245, 158, 11, 230), outline=(217, 119, 6, 255))
    draw.polygon([(kg_x - 14, kg_y + 16), (kg_x + 14, kg_y + 16), (kg_x, kg_y + 26)], fill=(245, 158, 11, 230))
    # Spectral beard & face
    draw.rectangle([kg_x - 8, kg_y - 18, kg_x + 8, kg_y - 6], fill=(254, 243, 199, 240))
    draw.polygon([(kg_x - 7, kg_y - 8), (kg_x + 7, kg_y - 8), (kg_x, kg_y + 4)], fill=(241, 245, 249, 255)) # Long white ghostly beard
    # Ancient Hyrulean Crown
    draw.polygon([(kg_x - 9, kg_y - 18), (kg_x - 9, kg_y - 26), (kg_x - 4, kg_y - 21),
                  (kg_x, kg_y - 28), (kg_x + 4, kg_y - 21), (kg_x + 9, kg_y - 26), (kg_x + 9, kg_y - 18)], fill=(250, 204, 21, 255))
    draw.point((kg_x, kg_y - 23), fill=(220, 38, 38, 255)) # Crown ruby

    # Bestowal of the Royal Golden Kinstone!
    draw.ellipse([kg_x - 6, kg_y + 24, kg_x + 6, kg_y + 36], fill=(250, 204, 21, 255), outline=(254, 240, 138, 255), width=2)
    draw.line([kg_x - 3, kg_y + 27, kg_x + 3, kg_y + 33], fill=(161, 98, 7, 255), width=2) # Kinstone cleft
    draw.text((kg_x + 14, kg_y + 24), "Royal Golden Kinstone", fill=(254, 240, 138, 255), font=font_small)

    # Celebratory Banner at bottom
    draw.rectangle([p4_ox + 20, p4_oy + 180, p4_ox + 396, p4_oy + 215], fill=(15, 23, 42, 245), outline=(245, 158, 11, 255), width=2)
    draw.text((p4_ox + 32, p4_oy + 186), "KING GUSTAF'S SACRED BLESSING ACQUIRED!", fill=(253, 224, 71, 255), font=font_small)
    draw.text((p4_ox + 32, p4_oy + 198), "Received Royal Golden Kinstone! The path to Cloud Tops is open!", fill=(224, 242, 254, 255), font=font_small)

    # Save to brain directory and scratch
    os.makedirs(out_dir, exist_ok=True)
    img.save(out_path)
    print(f"Showcase image saved to {out_path}")
    shutil.copyfile(out_path, scratch_path)
    print(f"Showcase image copied to {scratch_path}")

if __name__ == "__main__":
    main()
