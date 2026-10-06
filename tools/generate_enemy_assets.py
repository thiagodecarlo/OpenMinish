#!/usr/bin/env python3
"""
tools/generate_enemy_assets.py - Canonical GBA Enemy Sprite Atlas Generator
===========================================================================
Generates a 160x160 32-bit RGBA BMP containing pixel-perfect GBA The Minish Cap
enemy sprites with authentic palettes, proportions, and animation states:

- Row 0 (Y=0..15):   Octorok (Red) & Rock Projectile
- Row 1 (Y=16..31):  Green ChuChu (Puddle, Sprout, Standing, Squash, Jump, Wobble)
- Row 2 (Y=32..47):  Keese & Fire Keese (Wings Up, Wings Down, Shadow)
- Row 3 (Y=48..63):  Moblin (Walk Down, Walk Side, Walk Up, Spear Thrust)
- Row 4 (Y=64..79):  Tektite & Peahat (Ground, Leap, Flower, Propeller Spin)
- Row 5 (Y=80..95):  Rope & Spiny Beetle (Slither, Dash, Bush Shell, Running)
- Row 6..7 (Y=96..159): Boss Big Green ChuChu (32x32 cells: Idle, Squash, Electric, Stunned)
"""

import os
import struct
from PIL import Image, ImageDraw

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

def create_enemy_atlas():
    ATLAS_W = 160
    ATLAS_H = 160
    sheet = Image.new('RGBA', (ATLAS_W, ATLAS_H), (0, 0, 0, 0))

    def make_cell_16():
        return Image.new('RGBA', (16, 16), (0, 0, 0, 0))

    def make_cell_32():
        return Image.new('RGBA', (32, 32), (0, 0, 0, 0))

    # =========================================================================
    # ROW 0: OCTOROK & ROCK PROJECTILE (Y = 0)
    # =========================================================================
    # Palette Octorok GBA
    OCTO_RED      = (222, 48, 48, 255)
    OCTO_DARK     = (153, 24, 24, 255)
    OCTO_HIGHLIGHT= (240, 112, 112, 255)
    OCTO_SHADOW   = (80, 10, 10, 255)
    WHITE         = (255, 255, 255, 255)
    BLACK         = (17, 17, 17, 255)

    # 0.0: Octorok Walk Down
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 3), (12, 11)], fill=OCTO_RED)
    d.rectangle([(4, 2), (11, 3)], fill=OCTO_HIGHLIGHT)
    d.rectangle([(3, 11), (12, 12)], fill=OCTO_DARK)
    # Eyes
    d.rectangle([(4, 5), (5, 7)], fill=WHITE)
    d.rectangle([(10, 5), (11, 7)], fill=WHITE)
    c.putpixel((5, 6), BLACK)
    c.putpixel((11, 6), BLACK)
    # Snout
    d.rectangle([(6, 8), (9, 10)], fill=OCTO_DARK)
    # Tentacles step 0
    d.rectangle([(3, 13), (5, 14)], fill=OCTO_DARK)
    d.rectangle([(10, 13), (12, 14)], fill=OCTO_DARK)
    sheet.paste(c, (0 * 16, 0))

    # 0.1: Octorok Walk Up
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 3), (12, 11)], fill=OCTO_RED)
    d.rectangle([(4, 2), (11, 3)], fill=OCTO_HIGHLIGHT)
    d.rectangle([(3, 11), (12, 12)], fill=OCTO_DARK)
    d.rectangle([(4, 4), (11, 6)], fill=OCTO_DARK)
    d.rectangle([(3, 13), (5, 14)], fill=OCTO_DARK)
    d.rectangle([(10, 13), (12, 14)], fill=OCTO_DARK)
    sheet.paste(c, (1 * 16, 0))

    # 0.2: Octorok Walk Side 0
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 3), (12, 11)], fill=OCTO_RED)
    d.rectangle([(4, 2), (11, 3)], fill=OCTO_HIGHLIGHT)
    d.rectangle([(3, 11), (12, 12)], fill=OCTO_DARK)
    d.rectangle([(9, 5), (11, 7)], fill=WHITE)
    c.putpixel((11, 6), BLACK)
    d.rectangle([(12, 6), (14, 8)], fill=OCTO_DARK)
    d.rectangle([(3, 13), (5, 14)], fill=OCTO_DARK)
    d.rectangle([(10, 13), (12, 14)], fill=OCTO_DARK)
    sheet.paste(c, (2 * 16, 0))

    # 0.3: Octorok Walk Side 1
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 3), (12, 11)], fill=OCTO_RED)
    d.rectangle([(4, 2), (11, 3)], fill=OCTO_HIGHLIGHT)
    d.rectangle([(3, 11), (12, 12)], fill=OCTO_DARK)
    d.rectangle([(9, 5), (11, 7)], fill=WHITE)
    c.putpixel((11, 6), BLACK)
    d.rectangle([(12, 6), (14, 8)], fill=OCTO_DARK)
    d.rectangle([(5, 13), (7, 14)], fill=OCTO_DARK)
    d.rectangle([(8, 13), (10, 14)], fill=OCTO_DARK)
    sheet.paste(c, (3 * 16, 0))

    # 0.4: Octorok Shoot Down (Puff)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(2, 2), (13, 11)], fill=OCTO_RED)
    d.rectangle([(3, 1), (12, 2)], fill=OCTO_HIGHLIGHT)
    d.rectangle([(2, 11), (13, 12)], fill=OCTO_DARK)
    d.rectangle([(4, 4), (5, 6)], fill=WHITE)
    d.rectangle([(10, 4), (11, 6)], fill=WHITE)
    c.putpixel((5, 5), BLACK)
    c.putpixel((11, 5), BLACK)
    d.rectangle([(5, 8), (10, 12)], fill=OCTO_DARK)
    d.rectangle([(6, 9), (9, 11)], fill=BLACK)
    sheet.paste(c, (4 * 16, 0))

    # 0.5: Octorok Shoot Side (Puff)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(2, 2), (13, 11)], fill=OCTO_RED)
    d.rectangle([(3, 1), (12, 2)], fill=OCTO_HIGHLIGHT)
    d.rectangle([(8, 4), (10, 6)], fill=WHITE)
    c.putpixel((10, 5), BLACK)
    d.rectangle([(12, 5), (15, 9)], fill=OCTO_DARK)
    d.rectangle([(13, 6), (15, 8)], fill=BLACK)
    sheet.paste(c, (5 * 16, 0))

    # 0.6: Rock Projectile (8x8 centered)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    ROCK_MAIN  = (138, 85, 40, 255)
    ROCK_DARK  = (77, 46, 20, 255)
    ROCK_LIGHT = (196, 134, 77, 255)
    d.rectangle([(5, 5), (10, 10)], fill=ROCK_MAIN)
    d.rectangle([(6, 4), (9, 4)], fill=ROCK_LIGHT)
    d.rectangle([(6, 11), (9, 11)], fill=ROCK_DARK)
    c.putpixel((6, 6), ROCK_LIGHT)
    c.putpixel((9, 9), ROCK_DARK)
    sheet.paste(c, (6 * 16, 0))

    # =========================================================================
    # ROW 1: GREEN CHUCHU (Y = 16)
    # =========================================================================
    CHU_JELLY = (38, 196, 55, 240)
    CHU_DARK  = (20, 107, 30, 255)
    CHU_SHINE = (136, 255, 170, 255)

    # 1.0: Puddle (Camouflaged)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(2, 11), (13, 14)], fill=CHU_JELLY)
    d.ellipse([(4, 11), (11, 13)], fill=CHU_SHINE)
    d.rectangle([(3, 13), (12, 14)], fill=CHU_DARK)
    sheet.paste(c, (0 * 16, 16))

    # 1.1: Emerging / Sprouting
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(2, 7), (13, 14)], fill=CHU_JELLY)
    d.ellipse([(4, 6), (11, 9)], fill=CHU_SHINE)
    d.rectangle([(3, 13), (12, 14)], fill=CHU_DARK)
    # Small nascent eyes
    c.putpixel((5, 9), WHITE)
    c.putpixel((10, 9), WHITE)
    sheet.paste(c, (1 * 16, 16))

    # 1.2: Standing Tall (Idle)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(2, 3), (13, 14)], fill=CHU_JELLY)
    d.ellipse([(4, 2), (11, 5)], fill=CHU_SHINE)
    d.rectangle([(3, 13), (12, 14)], fill=CHU_DARK)
    # Eyes
    d.rectangle([(4, 5), (6, 7)], fill=WHITE)
    d.rectangle([(9, 5), (11, 7)], fill=WHITE)
    c.putpixel((5, 6), BLACK)
    c.putpixel((10, 6), BLACK)
    sheet.paste(c, (2 * 16, 16))

    # 1.3: Squash (Preparing Jump)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(1, 6), (14, 14)], fill=CHU_JELLY)
    d.ellipse([(3, 5), (12, 8)], fill=CHU_SHINE)
    d.rectangle([(2, 13), (13, 14)], fill=CHU_DARK)
    d.rectangle([(4, 7), (6, 9)], fill=WHITE)
    d.rectangle([(9, 7), (11, 9)], fill=WHITE)
    c.putpixel((5, 8), BLACK)
    c.putpixel((10, 8), BLACK)
    sheet.paste(c, (3 * 16, 16))

    # 1.4: Stretch Jump (Mid-air)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(4, 1), (11, 14)], fill=CHU_JELLY)
    d.ellipse([(5, 1), (10, 4)], fill=CHU_SHINE)
    d.rectangle([(5, 13), (10, 14)], fill=CHU_DARK)
    d.rectangle([(5, 4), (6, 6)], fill=WHITE)
    d.rectangle([(9, 4), (10, 6)], fill=WHITE)
    c.putpixel((5, 5), BLACK)
    c.putpixel((9, 5), BLACK)
    sheet.paste(c, (4 * 16, 16))

    # 1.5: Wobble / Damage
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(2, 4), (13, 13)], fill=CHU_JELLY)
    d.ellipse([(4, 3), (11, 6)], fill=CHU_SHINE)
    d.rectangle([(4, 5), (6, 7)], fill=WHITE)
    d.rectangle([(9, 5), (11, 7)], fill=WHITE)
    c.putpixel((5, 5), BLACK)
    c.putpixel((10, 5), BLACK)
    sheet.paste(c, (5 * 16, 16))

    # =========================================================================
    # ROW 2: KEESE & FIRE KEESE (Y = 32)
    # =========================================================================
    KEESE_BODY = (42, 14, 61, 255)
    KEESE_WING = (88, 36, 125, 255)
    KEESE_RIB  = (130, 57, 181, 255)
    KEESE_EYE  = (255, 215, 0, 255)

    FIRE_BODY  = (127, 29, 29, 255)
    FIRE_WING  = (234, 88, 12, 255)
    FIRE_RIB   = (251, 191, 36, 255)
    FIRE_EYE   = (254, 240, 138, 255)

    # 2.0: Keese Wings Up
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(6, 4), (9, 9)], fill=KEESE_BODY)
    c.putpixel((6, 3), KEESE_BODY)
    c.putpixel((9, 3), KEESE_BODY)
    c.putpixel((6, 5), KEESE_EYE)
    c.putpixel((9, 5), KEESE_EYE)
    # Wings up
    d.polygon([(6, 6), (1, 1), (3, 7)], fill=KEESE_WING)
    d.polygon([(9, 6), (14, 1), (12, 7)], fill=KEESE_WING)
    d.line([(6, 6), (1, 1)], fill=KEESE_RIB)
    d.line([(9, 6), (14, 1)], fill=KEESE_RIB)
    sheet.paste(c, (0 * 16, 32))

    # 2.1: Keese Wings Down
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(6, 4), (9, 9)], fill=KEESE_BODY)
    c.putpixel((6, 3), KEESE_BODY)
    c.putpixel((9, 3), KEESE_BODY)
    c.putpixel((6, 5), KEESE_EYE)
    c.putpixel((9, 5), KEESE_EYE)
    # Wings down
    d.polygon([(6, 6), (0, 8), (4, 10)], fill=KEESE_WING)
    d.polygon([(9, 6), (15, 8), (11, 10)], fill=KEESE_WING)
    d.line([(6, 6), (0, 8)], fill=KEESE_RIB)
    d.line([(9, 6), (15, 8)], fill=KEESE_RIB)
    sheet.paste(c, (1 * 16, 32))

    # 2.2: Keese Perched
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(5, 4), (10, 11)], fill=KEESE_BODY)
    c.putpixel((6, 3), KEESE_BODY)
    c.putpixel((9, 3), KEESE_BODY)
    c.putpixel((6, 6), KEESE_EYE)
    c.putpixel((9, 6), KEESE_EYE)
    d.rectangle([(4, 6), (5, 11)], fill=KEESE_WING)
    d.rectangle([(10, 6), (11, 11)], fill=KEESE_WING)
    sheet.paste(c, (2 * 16, 32))

    # 2.3: Fire Keese Wings Up
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(6, 4), (9, 9)], fill=FIRE_BODY)
    c.putpixel((6, 3), FIRE_BODY)
    c.putpixel((9, 3), FIRE_BODY)
    c.putpixel((6, 5), FIRE_EYE)
    c.putpixel((9, 5), FIRE_EYE)
    d.polygon([(6, 6), (1, 1), (3, 7)], fill=FIRE_WING)
    d.polygon([(9, 6), (14, 1), (12, 7)], fill=FIRE_WING)
    d.line([(6, 6), (1, 1)], fill=FIRE_RIB)
    d.line([(9, 6), (14, 1)], fill=FIRE_RIB)
    c.putpixel((2, 0), (254, 240, 138, 255))
    c.putpixel((13, 0), (254, 240, 138, 255))
    sheet.paste(c, (3 * 16, 32))

    # 2.4: Fire Keese Wings Down
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(6, 4), (9, 9)], fill=FIRE_BODY)
    c.putpixel((6, 3), FIRE_BODY)
    c.putpixel((9, 3), FIRE_BODY)
    c.putpixel((6, 5), FIRE_EYE)
    c.putpixel((9, 5), FIRE_EYE)
    d.polygon([(6, 6), (0, 8), (4, 10)], fill=FIRE_WING)
    d.polygon([(9, 6), (15, 8), (11, 10)], fill=FIRE_WING)
    d.line([(6, 6), (0, 8)], fill=FIRE_RIB)
    d.line([(9, 6), (15, 8)], fill=FIRE_RIB)
    c.putpixel((1, 9), (254, 240, 138, 255))
    c.putpixel((14, 9), (254, 240, 138, 255))
    sheet.paste(c, (4 * 16, 32))

    # 2.5: Shadow
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(3, 11), (12, 14)], fill=(5, 16, 7, 100))
    sheet.paste(c, (5 * 16, 32))

    # =========================================================================
    # ROW 3: MOBLIN (Y = 48)
    # =========================================================================
    MOBLIN_PIG   = (216, 120, 112, 255)
    MOBLIN_TUNIC = (168, 48, 32, 255)
    MOBLIN_ARMOR = (104, 72, 48, 255)
    MOBLIN_SPEAR = (200, 200, 210, 255)
    MOBLIN_WOOD  = (112, 64, 24, 255)

    # 3.0: Moblin Walk Down 0
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    # Head & Snout
    d.rectangle([(4, 2), (11, 7)], fill=MOBLIN_PIG)
    d.rectangle([(5, 5), (10, 7)], fill=(240, 144, 136, 255))
    c.putpixel((5, 4), BLACK)
    c.putpixel((10, 4), BLACK)
    # Ears / Horns
    c.putpixel((3, 2), (90, 40, 20, 255))
    c.putpixel((12, 2), (90, 40, 20, 255))
    # Tunic
    d.rectangle([(4, 8), (11, 12)], fill=MOBLIN_TUNIC)
    d.rectangle([(5, 9), (10, 11)], fill=MOBLIN_ARMOR)
    # Legs
    d.rectangle([(4, 13), (6, 15)], fill=(60, 40, 20, 255))
    d.rectangle([(9, 13), (11, 15)], fill=(60, 40, 20, 255))
    # Spear
    d.line([(2, 4), (2, 14)], fill=MOBLIN_WOOD)
    d.polygon([(2, 2), (1, 4), (3, 4)], fill=MOBLIN_SPEAR)
    sheet.paste(c, (0 * 16, 48))

    # 3.1: Moblin Walk Down 1
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(4, 2), (11, 7)], fill=MOBLIN_PIG)
    d.rectangle([(5, 5), (10, 7)], fill=(240, 144, 136, 255))
    c.putpixel((5, 4), BLACK)
    c.putpixel((10, 4), BLACK)
    c.putpixel((3, 2), (90, 40, 20, 255))
    c.putpixel((12, 2), (90, 40, 20, 255))
    d.rectangle([(4, 8), (11, 12)], fill=MOBLIN_TUNIC)
    d.rectangle([(5, 9), (10, 11)], fill=MOBLIN_ARMOR)
    d.rectangle([(5, 13), (7, 15)], fill=(60, 40, 20, 255))
    d.rectangle([(8, 13), (10, 15)], fill=(60, 40, 20, 255))
    d.line([(2, 5), (2, 15)], fill=MOBLIN_WOOD)
    d.polygon([(2, 3), (1, 5), (3, 5)], fill=MOBLIN_SPEAR)
    sheet.paste(c, (1 * 16, 48))

    # 3.2: Moblin Walk Side 0
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 2), (9, 7)], fill=MOBLIN_PIG)
    d.rectangle([(9, 4), (12, 6)], fill=(240, 144, 136, 255))
    c.putpixel((8, 3), BLACK)
    c.putpixel((12, 6), WHITE) # Tusk
    d.rectangle([(3, 8), (9, 12)], fill=MOBLIN_TUNIC)
    d.rectangle([(4, 13), (6, 15)], fill=(60, 40, 20, 255))
    d.rectangle([(8, 13), (10, 15)], fill=(60, 40, 20, 255))
    # Spear horizontal
    d.line([(10, 10), (15, 10)], fill=MOBLIN_WOOD)
    d.polygon([(15, 10), (13, 9), (13, 11)], fill=MOBLIN_SPEAR)
    sheet.paste(c, (2 * 16, 48))

    # 3.3: Moblin Walk Side 1
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 2), (9, 7)], fill=MOBLIN_PIG)
    d.rectangle([(9, 4), (12, 6)], fill=(240, 144, 136, 255))
    c.putpixel((8, 3), BLACK)
    c.putpixel((12, 6), WHITE)
    d.rectangle([(3, 8), (9, 12)], fill=MOBLIN_TUNIC)
    d.rectangle([(6, 13), (8, 15)], fill=(60, 40, 20, 255))
    d.line([(10, 9), (15, 9)], fill=MOBLIN_WOOD)
    d.polygon([(15, 9), (13, 8), (13, 10)], fill=MOBLIN_SPEAR)
    sheet.paste(c, (3 * 16, 48))

    # 3.4: Moblin Spear Thrust Side
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(1, 2), (7, 7)], fill=MOBLIN_PIG)
    d.rectangle([(7, 4), (10, 6)], fill=(240, 144, 136, 255))
    c.putpixel((6, 3), BLACK)
    d.rectangle([(1, 8), (7, 12)], fill=MOBLIN_TUNIC)
    d.rectangle([(2, 13), (5, 15)], fill=(60, 40, 20, 255))
    d.line([(7, 8), (15, 8)], fill=MOBLIN_WOOD, width=2)
    d.polygon([(15, 8), (12, 6), (12, 10)], fill=MOBLIN_SPEAR)
    sheet.paste(c, (4 * 16, 48))

    # =========================================================================
    # ROW 4: TEKTITE & PEAHAT (Y = 64)
    # =========================================================================
    TEK_BODY  = (220, 70, 20, 255)
    TEK_DARK  = (140, 30, 10, 255)
    TEK_LEG   = (80, 40, 20, 255)

    PEA_BODY  = (160, 210, 50, 255)
    PEA_FLOWER= (240, 100, 140, 255)
    PEA_BLADE = (220, 240, 180, 255)

    # 4.0: Tektite Ground
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(4, 5), (11, 11)], fill=TEK_BODY)
    d.rectangle([(4, 10), (11, 11)], fill=TEK_DARK)
    # Big center eye
    d.rectangle([(6, 6), (9, 9)], fill=WHITE)
    c.putpixel((7, 7), (180, 20, 20, 255))
    # Legs folded
    d.line([(4, 8), (1, 10), (2, 14)], fill=TEK_LEG)
    d.line([(11, 8), (14, 10), (13, 14)], fill=TEK_LEG)
    sheet.paste(c, (0 * 16, 64))

    # 4.1: Tektite Leap
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(4, 2), (11, 8)], fill=TEK_BODY)
    d.rectangle([(6, 3), (9, 6)], fill=WHITE)
    c.putpixel((7, 4), (180, 20, 20, 255))
    # Legs extended downward
    d.line([(4, 5), (1, 9), (2, 14)], fill=TEK_LEG)
    d.line([(11, 5), (14, 9), (13, 14)], fill=TEK_LEG)
    sheet.paste(c, (1 * 16, 64))

    # 4.2: Peahat Flower (Resting)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(4, 6), (11, 13)], fill=PEA_BODY)
    # Petals
    d.polygon([(7, 1), (5, 6), (9, 6)], fill=PEA_FLOWER)
    d.polygon([(1, 8), (4, 7), (4, 11)], fill=PEA_FLOWER)
    d.polygon([(14, 8), (11, 7), (11, 11)], fill=PEA_FLOWER)
    c.putpixel((6, 9), BLACK)
    c.putpixel((9, 9), BLACK)
    sheet.paste(c, (2 * 16, 64))

    # 4.3: Peahat Propeller Spin
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(4, 6), (11, 13)], fill=PEA_BODY)
    # Rotor blades spinning blur
    d.ellipse([(1, 3), (14, 6)], fill=PEA_BLADE)
    d.line([(0, 4), (15, 4)], fill=WHITE)
    c.putpixel((6, 9), BLACK)
    c.putpixel((9, 9), BLACK)
    sheet.paste(c, (3 * 16, 64))

    # =========================================================================
    # ROW 5: ROPE (SNAKE) & SPINY BEETLE (Y = 80)
    # =========================================================================
    ROPE_SKIN = (210, 160, 40, 255)
    ROPE_DARK = (130, 90, 20, 255)
    ROPE_BELLY= (245, 230, 160, 255)

    BEETLE_BODY = (180, 40, 40, 255)
    BUSH_LEAF   = (34, 139, 34, 255)
    BUSH_SHADOW = (20, 80, 20, 255)

    # 5.0: Rope Slither 1
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    # S-curve snake body
    d.line([(2, 10), (6, 7), (10, 11), (13, 8)], fill=ROPE_SKIN, width=2)
    d.rectangle([(11, 6), (15, 10)], fill=ROPE_SKIN)
    c.putpixel((13, 7), (200, 20, 20, 255)) # Red eye
    c.putpixel((15, 8), (255, 50, 50, 255)) # Tongue
    sheet.paste(c, (0 * 16, 80))

    # 5.1: Rope Slither 2
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.line([(2, 8), (6, 11), (10, 7), (13, 10)], fill=ROPE_SKIN, width=2)
    d.rectangle([(11, 8), (15, 12)], fill=ROPE_SKIN)
    c.putpixel((13, 9), (200, 20, 20, 255))
    c.putpixel((15, 10), (255, 50, 50, 255))
    sheet.paste(c, (1 * 16, 80))

    # 5.2: Spiny Beetle Camouflaged (Bush Shell)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(2, 3), (13, 13)], fill=BUSH_LEAF)
    d.ellipse([(4, 4), (11, 10)], fill=(50, 180, 50, 255))
    d.rectangle([(3, 12), (12, 13)], fill=BUSH_SHADOW)
    # Little beetle legs peeking
    c.putpixel((3, 14), BLACK)
    c.putpixel((6, 14), BLACK)
    c.putpixel((9, 14), BLACK)
    c.putpixel((12, 14), BLACK)
    sheet.paste(c, (2 * 16, 80))

    # 5.3: Spiny Beetle Running (Exposed Shell)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.ellipse([(3, 4), (12, 12)], fill=BEETLE_BODY)
    d.line([(7, 4), (7, 12)], fill=BLACK)
    # Spikes
    d.line([(3, 6), (1, 5)], fill=WHITE)
    d.line([(12, 6), (14, 5)], fill=WHITE)
    d.line([(3, 10), (1, 11)], fill=WHITE)
    d.line([(12, 10), (14, 11)], fill=WHITE)
    # Eyes
    c.putpixel((6, 5), WHITE)
    c.putpixel((8, 5), WHITE)
    sheet.paste(c, (3 * 16, 80))

    # =========================================================================
    # ROW 6..7: BOSS BIG GREEN CHUCHU (32x32 cells, Y = 96)
    # =========================================================================
    # 6.0: Boss Big ChuChu Idle
    b = make_cell_32()
    d = ImageDraw.Draw(b)
    # Giant Jelly Dome
    d.ellipse([(3, 5), (28, 28)], fill=CHU_JELLY)
    d.ellipse([(6, 4), (25, 12)], fill=CHU_SHINE)
    d.rectangle([(5, 27), (26, 29)], fill=CHU_DARK)
    # Specular bubbles inside
    d.ellipse([(8, 14), (12, 18)], fill=CHU_SHINE)
    d.ellipse([(20, 16), (23, 19)], fill=CHU_SHINE)
    # Huge goofy eyes
    d.rectangle([(8, 9), (13, 15)], fill=WHITE)
    d.rectangle([(18, 9), (23, 15)], fill=WHITE)
    d.rectangle([(10, 11), (12, 14)], fill=BLACK)
    d.rectangle([(20, 11), (22, 14)], fill=BLACK)
    sheet.paste(b, (0 * 32, 96))

    # 6.1: Boss Big ChuChu Squash
    b = make_cell_32()
    d = ImageDraw.Draw(b)
    d.ellipse([(1, 11), (30, 28)], fill=CHU_JELLY)
    d.ellipse([(4, 10), (27, 16)], fill=CHU_SHINE)
    d.rectangle([(3, 27), (28, 29)], fill=CHU_DARK)
    d.rectangle([(8, 14), (13, 19)], fill=WHITE)
    d.rectangle([(18, 14), (23, 19)], fill=WHITE)
    d.rectangle([(10, 16), (12, 18)], fill=BLACK)
    d.rectangle([(20, 16), (22, 18)], fill=BLACK)
    sheet.paste(b, (1 * 32, 96))

    # 6.2: Boss Big ChuChu Electric Shock
    b = make_cell_32()
    d = ImageDraw.Draw(b)
    d.ellipse([(3, 5), (28, 28)], fill=(40, 220, 180, 240))
    d.ellipse([(6, 4), (25, 12)], fill=(200, 255, 255, 255))
    d.rectangle([(8, 9), (13, 15)], fill=WHITE)
    d.rectangle([(18, 9), (23, 15)], fill=WHITE)
    d.rectangle([(10, 11), (12, 14)], fill=BLACK)
    d.rectangle([(20, 11), (22, 14)], fill=BLACK)
    # Lightning bolts
    E_COL = (255, 255, 80, 255)
    d.line([(4, 4), (7, 9), (4, 13)], fill=E_COL, width=2)
    d.line([(27, 4), (24, 9), (27, 13)], fill=E_COL, width=2)
    d.line([(12, 1), (15, 4), (17, 1)], fill=E_COL, width=2)
    sheet.paste(b, (2 * 32, 96))

    # 6.3: Boss Big ChuChu Stunned / Melted
    b = make_cell_32()
    d = ImageDraw.Draw(b)
    d.ellipse([(2, 14), (29, 29)], fill=CHU_JELLY)
    d.rectangle([(4, 27), (27, 29)], fill=CHU_DARK)
    # Spiral dazed eyes
    d.rectangle([(9, 16), (13, 20)], fill=WHITE)
    d.rectangle([(18, 16), (22, 20)], fill=WHITE)
    d.line([(9, 16), (13, 20)], fill=BLACK)
    d.line([(9, 20), (13, 16)], fill=BLACK)
    d.line([(18, 16), (22, 20)], fill=BLACK)
    d.line([(18, 20), (22, 16)], fill=BLACK)
    sheet.paste(b, (3 * 32, 96))

    # =========================================================================
    # Save to regions
    # =========================================================================
    save_bmp_32('assets/regions/enemies_master.bmp', sheet)
    save_bmp_32('assets/regions/usa/enemies.bmp', sheet)
    save_bmp_32('assets/regions/eur/enemies.bmp', sheet)
    save_bmp_32('assets/regions/jpn/enemies.bmp', sheet)
    return True

if __name__ == '__main__':
    create_enemy_atlas()
