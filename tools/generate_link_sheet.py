#!/usr/bin/env python3
"""
tools/generate_link_sheet.py - Canonical GBA Link Sprite Sheet Generator

Extracts authentic Link animations from the canonical Minish Cap sprite sheets
and packages them into a standardized 32-bit RGBA BMP sheet (320x320)
containing 10 rows x 10 columns of 32x32 cells:
- Row 0: Idle (Down, Right, Up, Left)
- Row 1: Walk Down (10 frames)
- Row 2: Walk Right (10 frames)
- Row 3: Walk Up (10 frames)
- Row 4: Link Minish (10 frames: 4 idle + 6 walk frames)
- Row 5: Speech Bubble Beacons (4 frames: Down, Right, Up, Left)
- Row 6: Sword Slashes (Down x3, Right x3, Up x3, Spin Charge)
- Row 7: Somersault Roll (8 frames) + Spin Attack 360 (2 frames)
- Row 8: Airborne Jump & Roc's Cape + Lifting/Carrying + Mole Mitts (10 frames)
- Row 9: Damage / Hurt / Knockback + Swimming & Diving (10 frames)
"""

import os
import sys
import struct
from PIL import Image

def save_bmp_32(filepath, img):
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    w, h = img.size
    img = img.convert('RGBA')
    img_data = img.tobytes()
    bgra = bytearray(w * h * 4)
    for i in range(w * h):
        r = img_data[i*4 + 0]
        g = img_data[i*4 + 1]
        b = img_data[i*4 + 2]
        a = img_data[i*4 + 3]
        bgra[i*4 + 0] = b
        bgra[i*4 + 1] = g
        bgra[i*4 + 2] = r
        bgra[i*4 + 3] = a

    header_size = 54
    img_size = w * h * 4
    file_size = header_size + img_size

    file_hdr = struct.pack('<2sIHHI', b'BM', file_size, 0, 0, header_size)
    info_hdr = struct.pack('<IiiHHIIiiII', 40, w, -h, 1, 32, 0, img_size, 0, 0, 0, 0)

    with open(filepath, 'wb') as f:
        f.write(file_hdr)
        f.write(info_hdr)
        f.write(bgra)
    print(f"  -> Salvo: {filepath} ({w}x{h}, {file_size} bytes)")

def generate_link_sheet(source_sheet_path):
    if not os.path.exists(source_sheet_path):
        print(f"Erro: Arquivo fonte nao encontrado: {source_sheet_path}")
        return False

    img = Image.open(source_sheet_path)
    print(f"Processando folha de sprites: {source_sheet_path} ({img.size[0]}x{img.size[1]})")

    CELL_W = 32
    CELL_H = 32
    NUM_COLS = 10
    NUM_ROWS = 10

    sheet = Image.new('RGBA', (CELL_W * NUM_COLS, CELL_H * NUM_ROWS), (0, 0, 0, 0))

    def extract_cell(center_x, bottom_y):
        src_x1 = center_x - CELL_W // 2
        src_y1 = bottom_y - 25
        src_x2 = src_x1 + CELL_W
        src_y2 = src_y1 + CELL_H
        c = img.crop((src_x1, src_y1, src_x2, src_y2)).convert('RGBA')
        orig = img.crop((src_x1, src_y1, src_x2, src_y2))
        new_data = [(0, 0, 0, 0) if o == 0 else (r, g, b, 255) for (r, g, b, a), o in zip(c.getdata(), orig.getdata())]
        c.putdata(new_data)
        return c

    def extract_sprite_auto(x1, y1, x2, y2, baseline_y=26, flip=False):
        crop = img.crop((x1, y1, x2, y2))
        bbox = crop.getbbox()
        if not bbox:
            return Image.new('RGBA', (CELL_W, CELL_H), (0, 0, 0, 0))
        sprite = crop.crop(bbox)
        if flip:
            sprite = sprite.transpose(Image.FLIP_LEFT_RIGHT)
        sw, sh = sprite.size
        cell = Image.new('RGBA', (CELL_W, CELL_H), (0, 0, 0, 0))
        paste_x = (CELL_W - sw) // 2
        paste_y = baseline_y - sh
        if paste_y < 0: paste_y = 0
        s_rgba = sprite.convert('RGBA')
        data = [(0, 0, 0, 0) if o == 0 else (r, g, b, 255) for (r, g, b, a), o in zip(s_rgba.getdata(), sprite.getdata())]
        s_rgba.putdata(data)
        cell.paste(s_rgba, (paste_x, paste_y), s_rgba)
        return cell

    # -------------------------------------------------------------------------
    # Row 0: Idle (Down, Right, Up, Left)
    # -------------------------------------------------------------------------
    idle_down = extract_cell(24, 31)
    idle_left = extract_cell(60, 31)
    idle_right = idle_left.transpose(Image.FLIP_LEFT_RIGHT)
    idle_up = extract_cell(88, 30)

    sheet.paste(idle_down, (0 * CELL_W, 0 * CELL_H))
    sheet.paste(idle_right, (1 * CELL_W, 0 * CELL_H))
    sheet.paste(idle_up, (2 * CELL_W, 0 * CELL_H))
    sheet.paste(idle_left, (3 * CELL_W, 0 * CELL_H))

    # -------------------------------------------------------------------------
    # Row 1: Walk Down (10 frames)
    # -------------------------------------------------------------------------
    for k in range(10):
        cx = 15 + k * 32 + 9
        frame = extract_cell(cx, 99)
        sheet.paste(frame, (k * CELL_W, 1 * CELL_H))

    # -------------------------------------------------------------------------
    # Row 2: Walk Right (10 frames)
    # -------------------------------------------------------------------------
    right_xs = [350, 380, 412, 445, 476, 510, 540, 573, 609, 636]
    for k, x in enumerate(right_xs):
        cx = x + 11
        frame = extract_cell(cx, 99).transpose(Image.FLIP_LEFT_RIGHT)
        sheet.paste(frame, (k * CELL_W, 2 * CELL_H))

    # -------------------------------------------------------------------------
    # Row 3: Walk Up (10 frames)
    # -------------------------------------------------------------------------
    for k in range(10):
        cx = 683 + k * 32 + 9
        frame = extract_cell(cx, 99)
        sheet.paste(frame, (k * CELL_W, 3 * CELL_H))

    # -------------------------------------------------------------------------
    # Rows 4 & 5: Link Minish & Speech Bubble Beacons
    # -------------------------------------------------------------------------
    brain_scratch = r'C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be\scratch'
    minish_sheet_path = os.path.join(brain_scratch, 'link_minish.png')
    if os.path.exists(minish_sheet_path):
        m_img = Image.open(minish_sheet_path)
        pink = (248, 192, 240)

        def extract_minish_8x8(col, row_y):
            x1 = col * 9 + 1
            y1 = row_y + 1
            c = m_img.crop((x1, y1, x1 + 8, y1 + 8)).convert('RGBA')
            pix = c.load()
            for y in range(8):
                for x in range(8):
                    if pix[x, y][:3] == pink:
                        pix[x, y] = (0, 0, 0, 0)
            return c

        def extract_bubble_16(x1, y1):
            c = m_img.crop((x1, y1, x1 + 16, y1 + 16)).convert('RGBA')
            pix = c.load()
            for y in range(16):
                for x in range(16):
                    if pix[x, y][:3] == pink:
                        pix[x, y] = (0, 0, 0, 0)
            return c

        m_down_idle     = extract_minish_8x8(3, 305)
        m_down_a        = extract_minish_8x8(4, 305)
        m_down_b        = extract_minish_8x8(6, 305)

        m_side_raw_idle = extract_minish_8x8(0, 314)
        m_side_raw_a    = extract_minish_8x8(1, 314)
        m_side_raw_b    = extract_minish_8x8(3, 314)

        m_side_r_idle   = m_side_raw_idle.transpose(Image.FLIP_LEFT_RIGHT)
        m_side_r_a      = m_side_raw_a.transpose(Image.FLIP_LEFT_RIGHT)
        m_side_r_b      = m_side_raw_b.transpose(Image.FLIP_LEFT_RIGHT)
        m_side_l_idle   = m_side_raw_idle

        m_up_idle       = extract_minish_8x8(4, 332)
        m_up_a          = extract_minish_8x8(5, 332)
        m_up_b          = extract_minish_8x8(7, 332)

        sprites_m = [
            m_down_idle, m_side_r_idle, m_up_idle, m_side_l_idle,
            m_down_a, m_down_b, m_side_r_a, m_side_r_b, m_up_a, m_up_b
        ]

        # Linha 4: Sprites do Link Minish
        for col, s in enumerate(sprites_m):
            sheet.paste(s, (col * CELL_W + 12, 4 * CELL_H + 18), s)

        # Linha 5: Balões Indicadores Canônicos (Speech Bubble Beacons, 16x16)
        b_down  = extract_bubble_16(1, 1)
        b_left  = extract_bubble_16(1, 18)
        b_right = b_left.transpose(Image.FLIP_LEFT_RIGHT)
        b_up    = extract_bubble_16(52, 1)

        bubbles = [b_down, b_right, b_up, b_left]
        for col, b in enumerate(bubbles):
            sheet.paste(b, (col * CELL_W + 8, 5 * CELL_H + 8), b)

    # -------------------------------------------------------------------------
    # Row 6: Sword Slashes & Spin Charge
    # Col 0..2: Down Slash (windup, strike, followthrough)
    # Col 3..5: Right Slash (windup, strike, followthrough)
    # Col 6..8: Up Slash (windup, strike, followthrough)
    # Col 9: Spin Charge Pose
    # -------------------------------------------------------------------------
    # Helpers to composite authentic sword blades
    def draw_sword_v(cell, hx, hy, length=12, facing_down=True):
        pix = cell.load()
        c_white = (255, 255, 255, 255)
        c_steel = (207, 226, 243, 255)
        c_dark  = (100, 116, 139, 255)
        c_gold  = (245, 197, 24, 255)
        for gx in range(hx - 3, hx + 4):
            if 0 <= gx < 32 and 0 <= hy < 32:
                pix[gx, hy] = c_gold
        if 0 <= hx < 32 and 0 <= hy < 32:
            pix[hx, hy] = (239, 68, 68, 255)
        step = 1 if facing_down else -1
        for i in range(1, length):
            by = hy + i * step
            if 0 <= by < 32:
                if 0 <= hx - 1 < 32: pix[hx - 1, by] = c_dark
                if 0 <= hx < 32:     pix[hx, by]     = c_white
                if 0 <= hx + 1 < 32: pix[hx + 1, by] = c_steel
        tip_y = hy + length * step
        if 0 <= tip_y < 32 and 0 <= hx < 32:
            pix[hx, tip_y] = c_white

    def draw_sword_h(cell, hx, hy, length=12, facing_right=True):
        pix = cell.load()
        c_white = (255, 255, 255, 255)
        c_steel = (207, 226, 243, 255)
        c_dark  = (100, 116, 139, 255)
        c_gold  = (245, 197, 24, 255)
        for gy in range(hy - 3, hy + 4):
            if 0 <= hx < 32 and 0 <= gy < 32:
                pix[hx, gy] = c_gold
        if 0 <= hx < 32 and 0 <= hy < 32:
            pix[hx, hy] = (239, 68, 68, 255)
        step = 1 if facing_right else -1
        for i in range(1, length):
            bx = hx + i * step
            if 0 <= bx < 32:
                if 0 <= hy - 1 < 32: pix[bx, hy - 1] = c_white
                if 0 <= hy < 32:     pix[bx, hy]     = c_steel
                if 0 <= hy + 1 < 32: pix[bx, hy + 1] = c_dark
        tip_x = hx + length * step
        if 0 <= tip_x < 32 and 0 <= hy < 32:
            pix[tip_x, hy] = c_white

    # Down Slashes (Row 62: Y=1892..1917)
    d0 = extract_sprite_auto(15, 1892, 38, 1917, baseline_y=26)
    draw_sword_v(d0, 20, 12, length=9, facing_down=False)
    sheet.paste(d0, (0 * CELL_W, 6 * CELL_H))

    d1 = extract_sprite_auto(47, 1892, 69, 1917, baseline_y=26)
    draw_sword_v(d1, 15, 17, length=12, facing_down=True)
    sheet.paste(d1, (1 * CELL_W, 6 * CELL_H))

    d2 = extract_sprite_auto(78, 1892, 98, 1917, baseline_y=26)
    draw_sword_v(d2, 13, 18, length=10, facing_down=True)
    sheet.paste(d2, (2 * CELL_W, 6 * CELL_H))

    # Right Slashes (Row 6: Y=1924..1949 - facing RIGHT)
    r0 = extract_sprite_auto(42, 1924, 65, 1949, baseline_y=26, flip=True)
    draw_sword_v(r0, 24, 11, length=9, facing_down=False)
    sheet.paste(r0, (3 * CELL_W, 6 * CELL_H))

    r1 = extract_sprite_auto(79, 1924, 101, 1949, baseline_y=26, flip=True)
    draw_sword_h(r1, 20, 14, length=11, facing_right=True)
    sheet.paste(r1, (4 * CELL_W, 6 * CELL_H))

    r2 = extract_sprite_auto(110, 1924, 133, 1949, baseline_y=26, flip=True)
    draw_sword_h(r2, 21, 15, length=10, facing_right=True)
    sheet.paste(r2, (5 * CELL_W, 6 * CELL_H))

    # Up Slashes (Row 64: Y=1963..1988)
    u0 = extract_sprite_auto(15, 1963, 38, 1988, baseline_y=26)
    draw_sword_v(u0, 11, 17, length=9, facing_down=True)
    sheet.paste(u0, (6 * CELL_W, 6 * CELL_H))

    u1 = extract_sprite_auto(43, 1963, 65, 1988, baseline_y=26)
    draw_sword_v(u1, 15, 12, length=11, facing_down=False)
    sheet.paste(u1, (7 * CELL_W, 6 * CELL_H))

    u2 = extract_sprite_auto(78, 1963, 97, 1988, baseline_y=26)
    draw_sword_v(u2, 16, 11, length=10, facing_down=False)
    sheet.paste(u2, (8 * CELL_W, 6 * CELL_H))

    # Spin Charge Pose (Col 9)
    sc = extract_sprite_auto(175, 1892, 197, 1917, baseline_y=26)
    draw_sword_v(sc, 11, 14, length=10, facing_down=False)
    sheet.paste(sc, (9 * CELL_W, 6 * CELL_H))

    # -------------------------------------------------------------------------
    # Row 7: Somersault Roll (8 frames) & Spin 360 Turns (2 frames)
    # -------------------------------------------------------------------------
    roll_ranges = [
        (255, 280), (288, 312), (320, 344), (352, 376),
        (384, 408), (418, 442), (452, 476), (485, 510)
    ]
    for i, (rx1, rx2) in enumerate(roll_ranges):
        frame = extract_sprite_auto(rx1, 1100, rx2, 1135, baseline_y=26)
        sheet.paste(frame, (i * CELL_W, 7 * CELL_H))

    # Spin 360 rapid turns (Down, Right)
    spin_turn_d = extract_cell(24, 31)
    spin_turn_r = extract_cell(60, 31).transpose(Image.FLIP_LEFT_RIGHT)
    sheet.paste(spin_turn_d, (8 * CELL_W, 7 * CELL_H))
    sheet.paste(spin_turn_r, (9 * CELL_W, 7 * CELL_H))

    # -------------------------------------------------------------------------
    # Row 8: Airborne Jump & Roc's Cape + Lifting/Carrying + Mole Mitts
    # Col 0: Jump Liftoff
    # Col 1: Airborne Gliding Apex
    # Col 2: Landing Squat
    # Col 3..5: Carry Idle, Walk 1, Walk 2
    # Col 6..7: Push/Pull Block
    # Col 8..9: Mole Mitts Digging
    # -------------------------------------------------------------------------
    jump_1 = extract_sprite_auto(20, 855, 45, 885, baseline_y=26)
    jump_2 = extract_sprite_auto(65, 855, 90, 885, baseline_y=26)
    jump_3 = extract_sprite_auto(115, 855, 140, 885, baseline_y=26)
    sheet.paste(jump_1, (0 * CELL_W, 8 * CELL_H))
    sheet.paste(jump_2, (1 * CELL_W, 8 * CELL_H))
    sheet.paste(jump_3, (2 * CELL_W, 8 * CELL_H))

    # Carry poses (Y=1400..1440)
    sheet.paste(extract_sprite_auto(12, 1410, 36, 1445, baseline_y=26), (3 * CELL_W, 8 * CELL_H))
    sheet.paste(extract_sprite_auto(44, 1410, 68, 1445, baseline_y=26), (4 * CELL_W, 8 * CELL_H))
    sheet.paste(extract_sprite_auto(76, 1410, 100, 1445, baseline_y=26), (5 * CELL_W, 8 * CELL_H))

    # Push/Pull (Y=600..635)
    sheet.paste(extract_sprite_auto(12, 605, 36, 635, baseline_y=26), (6 * CELL_W, 8 * CELL_H))
    sheet.paste(extract_sprite_auto(44, 605, 68, 635, baseline_y=26), (7 * CELL_W, 8 * CELL_H))

    # Mole Mitts (Y=2020..2050)
    sheet.paste(extract_sprite_auto(12, 2020, 36, 2050, baseline_y=26), (8 * CELL_W, 8 * CELL_H))
    sheet.paste(extract_sprite_auto(44, 2020, 68, 2050, baseline_y=26), (9 * CELL_W, 8 * CELL_H))

    # -------------------------------------------------------------------------
    # Row 9: Damage / Hurt / Knockback + Swimming & Diving
    # Col 0: Hurt Down
    # Col 1: Hurt Side
    # Col 2: Hurt Up
    # Col 3: Electrocuted / Shock
    # Col 4: Mud Sinking
    # Col 5..6: Swim Down 1, 2
    # Col 7..8: Swim Side 1, 2
    # Col 9: Dive Submerged
    # -------------------------------------------------------------------------
    # Hurt poses from authentic Minish Cap reaction row (Y=186..207)
    sheet.paste(extract_sprite_auto(14, 186, 34, 206, baseline_y=26), (0 * CELL_W, 9 * CELL_H))
    sheet.paste(extract_sprite_auto(162, 186, 180, 206, baseline_y=26, flip=True), (1 * CELL_W, 9 * CELL_H))
    sheet.paste(extract_sprite_auto(271, 186, 289, 205, baseline_y=26), (2 * CELL_W, 9 * CELL_H))

    # Electrocuted (Y=890..925)
    sheet.paste(extract_sprite_auto(36, 895, 60, 925, baseline_y=26), (3 * CELL_W, 9 * CELL_H))

    # Mud Sinking (crouched)
    sheet.paste(extract_sprite_auto(12, 495, 36, 525, baseline_y=26), (4 * CELL_W, 9 * CELL_H))

    # Swimming Down (Y=710..745)
    sheet.paste(extract_sprite_auto(12, 715, 36, 745, baseline_y=26), (5 * CELL_W, 9 * CELL_H))
    sheet.paste(extract_sprite_auto(44, 715, 68, 745, baseline_y=26), (6 * CELL_W, 9 * CELL_H))

    # Swimming Side (Y=710..745)
    sheet.paste(extract_sprite_auto(235, 715, 260, 745, baseline_y=26), (7 * CELL_W, 9 * CELL_H))
    sheet.paste(extract_sprite_auto(265, 715, 290, 745, baseline_y=26), (8 * CELL_W, 9 * CELL_H))

    # Submerged Dive (Y=710..745)
    sheet.paste(extract_sprite_auto(455, 715, 485, 745, baseline_y=26), (9 * CELL_W, 9 * CELL_H))

    # -------------------------------------------------------------------------
    # Save sheets to assets/regions/
    # -------------------------------------------------------------------------
    save_bmp_32('assets/regions/link_master.bmp', sheet)
    save_bmp_32('assets/regions/usa/link.bmp', sheet)
    save_bmp_32('assets/regions/eur/link.bmp', sheet)
    save_bmp_32('assets/regions/jpn/link.bmp', sheet)
    return True

if __name__ == '__main__':
    src = sys.argv[1] if len(sys.argv) > 1 else 'scratch/spriters_link.png'
    if not os.path.exists(src):
        brain_scratch = r'C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be\scratch\spriters_link.png'
        if os.path.exists(brain_scratch):
            src = brain_scratch
    generate_link_sheet(src)
