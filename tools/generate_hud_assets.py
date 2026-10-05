#!/usr/bin/env python3
"""
tools/generate_hud_assets.py - Canonical GBA HUD & Item Icons Atlas Generator

Generates a 160x80 32-bit RGBA BMP (10 columns x 5 rows of 16x16 cells)
containing authentic pixel-perfect GBA The Minish Cap HUD & item sprites:
- Row 0: Full Heart, 3/4 Heart, 1/2 Heart, 1/4 Heart, Empty Heart Container,
         Green Rupee, Blue Rupee, Red Rupee, Badge [A], Badge [B]
- Row 1: Smith's Sword, White Sword, Four Sword, Small Shield, Mirror Shield,
         Tiger Scroll, Kinstone Bag, Mysterious Shell, Carlov Medal, Heart Piece
- Row 2: Boomerang, Magic Boomerang, Bombs, Remote Bombs, Bow, Light Bow,
         Gust Jar, Pegasus Boots, Flippers, Grip Ring
- Row 3: Mole Mitts, Roc's Cape, Flame Lantern, Cane of Pacci, Ocarina of Wind,
         Empty Bottle, Lon Lon Milk, Picolyte, Small Key, Big Key
- Row 4: Earth Element, Fire Element, Water Element, Wind Element, Royal Kinstone,
         Compass, Dungeon Map, Stone Tablet, Power Bracelet, Fairy
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

def create_hud_atlas():
    atlas_w = 160 # 10 cols * 16
    atlas_h = 80  # 5 rows * 16
    img = Image.new('RGBA', (atlas_w, atlas_h), (0, 0, 0, 0))

    def make_cell():
        return Image.new('RGBA', (16, 16), (0, 0, 0, 0))

    # Helper: draw heart in 16x16 with quarters
    def make_heart(fill_fraction): # 1.0, 0.75, 0.5, 0.25, 0.0
        c = make_cell()
        # GBA Minish Cap heart shape: 11x10 centered at (2, 3)
        # Red: (220, 32, 32), Dark rim: (100, 16, 16), Empty: (30, 40, 50), Highlight: (255, 180, 180)
        c_rim = (80, 10, 10, 255)
        c_red = (220, 32, 32, 255)
        c_hi  = (255, 160, 160, 255)
        c_empty = (20, 30, 40, 220)
        c_empty_hi = (45, 60, 75, 220)

        # Heart mask 11x10:
        # y=0:  .xx. .xx.   (2..3, 6..7)
        # y=1: xxxxx xxxxx  (1..4, 5..8)
        # y=2: xxxxxxxxxxx  (0..9)
        # y=3..8: tapering V
        grid = [
            "  ##   ##  ",
            " #### #### ",
            "###########",
            "###########",
            "###########",
            " ######### ",
            "  #######  ",
            "   #####   ",
            "    ###    ",
            "     #     "
        ]
        ox, oy = 2, 3
        for y, row in enumerate(grid):
            for x, ch in enumerate(row):
                if ch == '#':
                    # Determine rim or inner
                    is_rim = (y == 0 or x == 0 or x == len(row)-1 or
                              (y == 1 and x in [0, 4, 5, 9]) or
                              (y >= 5 and (x == 10-y or x == y)) or
                              y == len(grid)-1)
                    
                    # Fill logic: fill_fraction determines how much is red
                    # In MC, hearts empty from right to left or top to bottom.
                    # Standard Zelda quarter: left half (x < 5), right half (x >= 5)
                    # Quarters: 1 = full, 3/4 = right side partly empty, 1/2 = right empty, 1/4 = left bottom only
                    is_filled = False
                    if fill_fraction >= 1.0:
                        is_filled = True
                    elif fill_fraction >= 0.75:
                        is_filled = not (x >= 5 and y < 5)
                    elif fill_fraction >= 0.50:
                        is_filled = (x < 5)
                    elif fill_fraction >= 0.25:
                        is_filled = (x < 5 and y >= 4)
                    else:
                        is_filled = False

                    if is_rim:
                        c.putpixel((ox + x, oy + y), c_rim)
                    else:
                        if is_filled:
                            col = c_hi if (y == 2 and x in [2, 7]) else c_red
                            c.putpixel((ox + x, oy + y), col)
                        else:
                            col = c_empty_hi if (y == 2 and x in [2, 7]) else c_empty
                            c.putpixel((ox + x, oy + y), col)
        return c

    # Helper: draw rupee (16x16)
    def make_rupee(main_rgb, hi_rgb, dk_rgb):
        c = make_cell()
        # Rupee shape 8x12 centered at (4, 2)
        # Hexagonal gemstone
        ox, oy = 4, 2
        shape = [
            "  ####  ",
            " ###### ",
            "########",
            "########",
            "########",
            "########",
            "########",
            "########",
            "########",
            "########",
            " ###### ",
            "  ####  "
        ]
        for y, row in enumerate(shape):
            for x, ch in enumerate(row):
                if ch == '#':
                    px, py = ox + x, oy + y
                    if x == 0 or x == 7 or y == 0 or y == 11 or (y in [1, 10] and x in [1, 6]):
                        c.putpixel((px, py), (dk_rgb[0]//2, dk_rgb[1]//2, dk_rgb[2]//2, 255))
                    elif x <= 2 and y <= 5:
                        c.putpixel((px, py), (*hi_rgb, 255))
                    elif x >= 5:
                        c.putpixel((px, py), (*dk_rgb, 255))
                    else:
                        c.putpixel((px, py), (*main_rgb, 255))
        return c

    # Helper: draw button badge [A] or [B] (12x12 pill)
    def make_badge(letter, bg_rgb, text_rgb):
        c = make_cell()
        d = ImageDraw.Draw(c)
        d.rounded_rectangle([2, 2, 13, 13], radius=4, fill=(*bg_rgb, 255), outline=(255, 255, 255, 255))
        # Draw letter
        if letter == 'A':
            # 5x7 'A'
            coords = [(7, 4), (8, 4), (6, 5), (9, 5), (6, 6), (7, 6), (8, 6), (9, 6), (6, 7), (9, 7), (6, 8), (9, 8)]
            for (x, y) in coords:
                c.putpixel((x, y), (*text_rgb, 255))
        else: # 'B'
            coords = [(6, 4), (7, 4), (8, 4), (6, 5), (9, 5), (6, 6), (7, 6), (8, 6), (6, 7), (9, 7), (6, 8), (7, 8), (8, 8)]
            for (x, y) in coords:
                c.putpixel((x, y), (*text_rgb, 255))
        return c

    # Helper: draw Smith's / White / Four Sword
    def make_sword(tier): # 0: Smith, 1: White, 2: Four Sword
        c = make_cell()
        # Diagonal sword from bottom-left to top-right
        # Hilt: (3, 12), Crossguard: (4, 11), Blade: (5, 10) to (12, 3)
        hilt_col = (130, 80, 30, 255) if tier == 0 else (180, 140, 20, 255)
        guard_col = (220, 180, 50, 255) if tier != 2 else (60, 180, 240, 255)
        blade_col = (210, 225, 240, 255) if tier == 0 else (245, 250, 255, 255)
        glow_col  = (160, 180, 200, 255) if tier == 0 else ((56, 189, 248, 255) if tier == 1 else (168, 85, 247, 255))

        # Blade pixels
        for i in range(8):
            bx, by = 5 + i, 10 - i
            c.putpixel((bx, by), blade_col)
            c.putpixel((bx - 1, by), glow_col)
            c.putpixel((bx, by + 1), (glow_col[0]//2, glow_col[1]//2, glow_col[2]//2, 255))
        # Tip
        c.putpixel((13, 2), (255, 255, 255, 255))

        # Guard
        c.putpixel((4, 10), guard_col)
        c.putpixel((5, 11), guard_col)
        c.putpixel((3, 11), guard_col)
        if tier >= 1: # Ruby or jewel in crossguard
            c.putpixel((4, 11), (239, 68, 68, 255))

        # Hilt & Pommel
        c.putpixel((3, 12), hilt_col)
        c.putpixel((2, 13), guard_col)
        return c

    # Helper: draw Shield (Small / Mirror)
    def make_shield(mirror=False):
        c = make_cell()
        ox, oy = 2, 2
        shape = [
            " ########## ",
            "############",
            "############",
            "############",
            " ########## ",
            " ########## ",
            "  ########  ",
            "  ########  ",
            "   ######   ",
            "   ######   ",
            "    ####    ",
            "     ##     "
        ]
        c_wood  = (140, 85, 45, 255) if not mirror else (30, 90, 180, 255)
        c_rim   = (190, 150, 60, 255) if not mirror else (220, 230, 245, 255)
        c_crest = (240, 210, 60, 255)

        for y, row in enumerate(shape):
            for x, ch in enumerate(row):
                if ch == '#':
                    px, py = ox + x, oy + y
                    is_border = (x in [0, 1, 10, 11] or y in [0, 1, 10, 11])
                    if is_border:
                        c.putpixel((px, py), c_rim)
                    else:
                        # Draw Triforce or mirror sheen
                        if mirror and (x in [5, 6] or y in [5, 6]):
                            c.putpixel((px, py), (255, 255, 255, 255))
                        elif not mirror and (y in [4, 5, 6] and 4 <= x <= 7):
                            c.putpixel((px, py), c_crest)
                        else:
                            c.putpixel((px, py), c_wood)
        return c

    # Helper: draw Tiger Scroll
    def make_scroll():
        c = make_cell()
        d = ImageDraw.Draw(c)
        # Golden ribbon parchment roll
        d.rectangle([3, 4, 12, 11], fill=(245, 222, 179, 255), outline=(139, 69, 19, 255))
        d.line([3, 7, 12, 7], fill=(220, 38, 38, 255)) # Red tie
        d.rectangle([2, 3, 4, 12], fill=(218, 165, 32, 255)) # Roll knob
        d.rectangle([11, 3, 13, 12], fill=(218, 165, 32, 255))
        return c

    # Helper: draw Boomerang
    def make_boomerang(magical=False):
        c = make_cell()
        main_col = (180, 115, 50, 255) if not magical else (56, 189, 248, 255)
        rim_col  = (245, 200, 60, 255)
        pts = [(3, 3), (4, 3), (5, 4), (6, 5), (7, 6), (8, 7), (9, 8), (10, 9), (11, 10), (12, 11), (12, 12),
               (3, 4), (4, 5), (5, 6), (6, 7), (7, 8), (8, 9), (9, 10), (10, 11), (11, 12)]
        for (x, y) in pts:
            c.putpixel((x, y), main_col)
        c.putpixel((7, 7), (239, 68, 68, 255)) # Central red gem
        c.putpixel((3, 3), rim_col)
        c.putpixel((12, 12), rim_col)
        return c

    # Helper: draw Bomb
    def make_bomb(remote=False):
        c = make_cell()
        d = ImageDraw.Draw(c)
        body_col = (40, 50, 65, 255) if not remote else (14, 116, 144, 255)
        d.ellipse([3, 4, 12, 13], fill=body_col, outline=(15, 23, 42, 255))
        # Highlight
        c.putpixel((5, 6), (255, 255, 255, 255))
        c.putpixel((6, 6), (200, 210, 230, 255))
        # Neck & fuse
        d.rectangle([7, 3, 8, 4], fill=(180, 120, 50, 255))
        c.putpixel((8, 2), (250, 204, 21, 255))
        c.putpixel((9, 1), (239, 68, 68, 255)) # Spark
        return c

    # Helper: draw Bow
    def make_bow(light=False):
        c = make_cell()
        wood = (160, 80, 30, 255) if not light else (250, 204, 21, 255)
        string = (240, 240, 240, 255) if not light else (56, 189, 248, 255)
        # Curve
        for y in range(2, 14):
            x = 4 + abs(y - 7) // 2
            c.putpixel((x, y), wood)
            c.putpixel((4, y), string)
        # Arrow
        for x in range(3, 13):
            c.putpixel((x, 7), (230, 230, 230, 255))
        c.putpixel((12, 6), (150, 150, 150, 255))
        c.putpixel((12, 8), (150, 150, 150, 255))
        c.putpixel((13, 7), (255, 255, 255, 255))
        return c

    # Helper: draw Gust Jar
    def make_gust_jar():
        c = make_cell()
        d = ImageDraw.Draw(c)
        d.ellipse([3, 5, 12, 13], fill=(184, 92, 56, 255), outline=(100, 40, 20, 255))
        d.rectangle([6, 2, 9, 5], fill=(218, 165, 32, 255), outline=(130, 90, 10, 255))
        c.putpixel((7, 9), (56, 189, 248, 255)) # Swirl vortex
        c.putpixel((8, 8), (56, 189, 248, 255))
        return c

    # Helper: draw Pegasus Boots
    def make_boots():
        c = make_cell()
        d = ImageDraw.Draw(c)
        d.rectangle([4, 6, 11, 12], fill=(220, 38, 38, 255), outline=(127, 29, 29, 255))
        d.rectangle([7, 10, 13, 12], fill=(220, 38, 38, 255))
        # Wings
        d.polygon([(3, 3), (7, 6), (3, 8)], fill=(255, 255, 255, 255))
        return c

    # Helper: draw Flippers
    def make_flippers():
        c = make_cell()
        d = ImageDraw.Draw(c)
        d.polygon([(3, 4), (7, 13), (2, 12)], fill=(16, 185, 129, 255), outline=(5, 150, 105, 255))
        d.polygon([(9, 4), (13, 13), (8, 12)], fill=(16, 185, 129, 255), outline=(5, 150, 105, 255))
        return c

    # Helper: draw Cane of Pacci
    def make_cane():
        c = make_cell()
        for i in range(8):
            c.putpixel((3 + i, 12 - i), (22, 101, 52, 255)) # Green staff
        # Golden hook
        c.putpixel((10, 4), (245, 158, 11, 255))
        c.putpixel((11, 4), (245, 158, 11, 255))
        c.putpixel((12, 5), (245, 158, 11, 255))
        # Blue glowing orb
        c.putpixel((10, 3), (56, 189, 248, 255))
        c.putpixel((11, 3), (255, 255, 255, 255))
        return c

    # Helper: draw Mole Mitts
    def make_mole_mitts():
        c = make_cell()
        d = ImageDraw.Draw(c)
        d.rectangle([4, 6, 11, 12], fill=(120, 53, 15, 255), outline=(69, 26, 3, 255))
        # Claws
        c.putpixel((4, 4), (226, 232, 240, 255))
        c.putpixel((4, 5), (226, 232, 240, 255))
        c.putpixel((7, 4), (226, 232, 240, 255))
        c.putpixel((7, 5), (226, 232, 240, 255))
        c.putpixel((10, 4), (226, 232, 240, 255))
        c.putpixel((10, 5), (226, 232, 240, 255))
        return c

    # Helper: draw Roc's Cape
    def make_rocs_cape():
        c = make_cell()
        d = ImageDraw.Draw(c)
        d.polygon([(4, 4), (8, 2), (12, 4), (14, 12), (8, 10), (2, 12)], fill=(37, 99, 235, 255), outline=(220, 38, 38, 255))
        c.putpixel((8, 3), (245, 158, 11, 255)) # Clasp
        return c

    # Helper: draw Flame Lantern
    def make_lantern():
        c = make_cell()
        d = ImageDraw.Draw(c)
        d.rectangle([4, 4, 11, 12], fill=(217, 119, 6, 255), outline=(120, 53, 15, 255))
        d.rectangle([6, 6, 9, 10], fill=(253, 224, 71, 255)) # Flame
        c.putpixel((7, 7), (239, 68, 68, 255))
        d.line([6, 3, 9, 3], fill=(120, 53, 15, 255)) # Handle
        return c

    # Helper: draw Ocarina
    def make_ocarina():
        c = make_cell()
        d = ImageDraw.Draw(c)
        d.ellipse([3, 5, 12, 10], fill=(2, 132, 199, 255), outline=(15, 23, 42, 255))
        # Mouthpiece
        d.rectangle([2, 6, 4, 8], fill=(56, 189, 248, 255))
        # Holes
        c.putpixel((6, 7), (15, 23, 42, 255))
        c.putpixel((8, 6), (15, 23, 42, 255))
        c.putpixel((9, 8), (15, 23, 42, 255))
        c.putpixel((11, 7), (15, 23, 42, 255))
        return c

    # Helper: draw Bottle
    def make_bottle(content=None):
        c = make_cell()
        d = ImageDraw.Draw(c)
        d.rectangle([4, 5, 11, 12], fill=(200, 230, 245, 180), outline=(50, 100, 130, 255))
        d.rectangle([6, 2, 9, 4], fill=(200, 230, 245, 180), outline=(50, 100, 130, 255))
        c.putpixel((7, 1), (180, 120, 60, 255)) # Cork
        c.putpixel((8, 1), (180, 120, 60, 255))
        if content == 'milk':
            d.rectangle([5, 8, 10, 11], fill=(255, 255, 255, 255))
        elif content == 'potion':
            d.rectangle([5, 8, 10, 11], fill=(236, 72, 153, 255))
        elif content == 'fairy':
            c.putpixel((7, 8), (244, 114, 182, 255))
            c.putpixel((8, 8), (255, 255, 255, 255))
        return c

    # Helper: draw Keys
    def make_key(boss=False):
        c = make_cell()
        col = (234, 179, 8, 255) if not boss else (220, 38, 38, 255)
        d = ImageDraw.Draw(c)
        d.ellipse([4, 2, 11, 7], outline=col, fill=(0, 0, 0, 0))
        d.line([7, 7, 7, 13], fill=col)
        d.line([8, 10, 10, 10], fill=col)
        d.line([8, 12, 10, 12], fill=col)
        return c

    # Helper: draw Elements (Earth, Fire, Water, Wind)
    def make_element(elem_type):
        c = make_cell()
        d = ImageDraw.Draw(c)
        if elem_type == 0: # Earth
            d.rectangle([4, 4, 11, 11], fill=(34, 197, 94, 255), outline=(21, 128, 61, 255))
            c.putpixel((7, 7), (240, 253, 244, 255))
        elif elem_type == 1: # Fire
            d.polygon([(7, 2), (12, 11), (9, 13), (7, 10), (5, 13), (2, 11)], fill=(239, 68, 68, 255), outline=(185, 28, 28, 255))
            c.putpixel((7, 6), (254, 240, 138, 255))
        elif elem_type == 2: # Water
            d.polygon([(7, 2), (12, 9), (7, 13), (2, 9)], fill=(59, 130, 246, 255), outline=(29, 78, 216, 255))
            c.putpixel((7, 6), (219, 234, 254, 255))
        elif elem_type == 3: # Wind
            d.ellipse([3, 3, 12, 12], outline=(16, 185, 129, 255))
            d.line([4, 7, 11, 7], fill=(52, 211, 153, 255))
        return c

    # Populate Atlas:
    # Row 0: Hearts & Rupees & Badges
    img.paste(make_heart(1.00), (0 * 16, 0 * 16))
    img.paste(make_heart(0.75), (1 * 16, 0 * 16))
    img.paste(make_heart(0.50), (2 * 16, 0 * 16))
    img.paste(make_heart(0.25), (3 * 16, 0 * 16))
    img.paste(make_heart(0.00), (4 * 16, 0 * 16))
    img.paste(make_rupee((0, 255, 136), (180, 255, 200), (0, 150, 70)), (5 * 16, 0 * 16))
    img.paste(make_rupee((59, 130, 246), (191, 219, 254), (29, 78, 216)), (6 * 16, 0 * 16))
    img.paste(make_rupee((239, 68, 68), (254, 202, 202), (185, 28, 28)), (7 * 16, 0 * 16))
    img.paste(make_badge('A', (37, 99, 235), (255, 255, 255)), (8 * 16, 0 * 16))
    img.paste(make_badge('B', (234, 88, 12), (255, 255, 255)), (9 * 16, 0 * 16))

    # Row 1: Swords & Shields & Pergaminhos
    img.paste(make_sword(0), (0 * 16, 1 * 16)) # Smith's Sword
    img.paste(make_sword(1), (1 * 16, 1 * 16)) # White Sword
    img.paste(make_sword(2), (2 * 16, 1 * 16)) # Four Sword
    img.paste(make_shield(False), (3 * 16, 1 * 16)) # Small Shield
    img.paste(make_shield(True), (4 * 16, 1 * 16))  # Mirror Shield
    img.paste(make_scroll(), (5 * 16, 1 * 16))      # Tiger Scroll
    # Kinstone Bag / Pouch
    pouch = make_cell()
    ImageDraw.Draw(pouch).ellipse([3, 5, 12, 13], fill=(34, 197, 94, 255), outline=(21, 128, 61, 255))
    pouch.putpixel((7, 4), (234, 179, 8, 255))
    img.paste(pouch, (6 * 16, 1 * 16))
    # Shell
    shell = make_cell()
    ImageDraw.Draw(shell).polygon([(3, 11), (7, 3), (12, 7), (8, 12)], fill=(56, 189, 248, 255), outline=(3, 105, 161, 255))
    img.paste(shell, (7 * 16, 1 * 16))
    # Carlov Medal
    medal = make_cell()
    ImageDraw.Draw(medal).ellipse([3, 3, 12, 12], fill=(234, 179, 8, 255), outline=(161, 98, 7, 255))
    medal.putpixel((7, 7), (255, 255, 255, 255))
    img.paste(medal, (8 * 16, 1 * 16))
    # Heart Piece
    hp = make_cell()
    ImageDraw.Draw(hp).rectangle([2, 2, 13, 13], outline=(200, 220, 240, 200))
    hp.paste(make_heart(0.5).crop((2, 2, 14, 14)), (2, 2))
    img.paste(hp, (9 * 16, 1 * 16))

    # Row 2: Secondary / Combat
    img.paste(make_boomerang(False), (0 * 16, 2 * 16))
    img.paste(make_boomerang(True), (1 * 16, 2 * 16))
    img.paste(make_bomb(False), (2 * 16, 2 * 16))
    img.paste(make_bomb(True), (3 * 16, 2 * 16))
    img.paste(make_bow(False), (4 * 16, 2 * 16))
    img.paste(make_bow(True), (5 * 16, 2 * 16))
    img.paste(make_gust_jar(), (6 * 16, 2 * 16))
    img.paste(make_boots(), (7 * 16, 2 * 16))
    img.paste(make_flippers(), (8 * 16, 2 * 16))
    # Grip Ring
    ring = make_cell()
    ImageDraw.Draw(ring).ellipse([3, 3, 12, 12], outline=(234, 179, 8, 255))
    ring.putpixel((7, 3), (239, 68, 68, 255))
    img.paste(ring, (9 * 16, 2 * 16))

    # Row 3: Magic & Tools
    img.paste(make_mole_mitts(), (0 * 16, 3 * 16))
    img.paste(make_rocs_cape(), (1 * 16, 3 * 16))
    img.paste(make_lantern(), (2 * 16, 3 * 16))
    img.paste(make_cane(), (3 * 16, 3 * 16))
    img.paste(make_ocarina(), (4 * 16, 3 * 16))
    img.paste(make_bottle(None), (5 * 16, 3 * 16))
    img.paste(make_bottle('milk'), (6 * 16, 3 * 16))
    img.paste(make_bottle('potion'), (7 * 16, 3 * 16))
    img.paste(make_key(False), (8 * 16, 3 * 16))
    img.paste(make_key(True), (9 * 16, 3 * 16))

    # Row 4: Elements & Map
    img.paste(make_element(0), (0 * 16, 4 * 16))
    img.paste(make_element(1), (1 * 16, 4 * 16))
    img.paste(make_element(2), (2 * 16, 4 * 16))
    img.paste(make_element(3), (3 * 16, 4 * 16))
    # Royal Kinstone
    rk = make_cell()
    ImageDraw.Draw(rk).polygon([(7, 2), (13, 8), (7, 14), (1, 8)], fill=(234, 179, 8, 255), outline=(255, 255, 255, 255))
    img.paste(rk, (4 * 16, 4 * 16))
    # Compass
    comp = make_cell()
    ImageDraw.Draw(comp).ellipse([3, 3, 12, 12], fill=(217, 119, 6, 255), outline=(245, 158, 11, 255))
    comp.putpixel((7, 5), (239, 68, 68, 255))
    comp.putpixel((7, 9), (59, 130, 246, 255))
    img.paste(comp, (5 * 16, 4 * 16))
    # Map
    dmap = make_cell()
    ImageDraw.Draw(dmap).rectangle([3, 3, 12, 12], fill=(245, 222, 179, 255), outline=(139, 69, 19, 255))
    ImageDraw.Draw(dmap).line([5, 5, 10, 5], fill=(59, 130, 246, 255))
    ImageDraw.Draw(dmap).line([5, 7, 8, 7], fill=(239, 68, 68, 255))
    img.paste(dmap, (6 * 16, 4 * 16))
    # Stone Tablet
    tab = make_cell()
    ImageDraw.Draw(tab).rectangle([3, 3, 12, 12], fill=(148, 163, 184, 255), outline=(71, 85, 105, 255))
    img.paste(tab, (7 * 16, 4 * 16))
    # Bracelet
    brac = make_cell()
    ImageDraw.Draw(brac).ellipse([3, 4, 12, 11], fill=(234, 179, 8, 255), outline=(180, 83, 9, 255))
    img.paste(brac, (8 * 16, 4 * 16))
    # Fairy Bottle
    img.paste(make_bottle('fairy'), (9 * 16, 4 * 16))

    # Save to canonical paths
    paths = [
        'assets/ui/hud_items.bmp',
        'assets/regions/usa/hud_items.bmp',
        'assets/regions/eur/hud_items.bmp',
        'assets/regions/jpn/hud_items.bmp',
    ]
    for p in paths:
        save_bmp_32(p, img)
    return True

if __name__ == '__main__':
    create_hud_atlas()
