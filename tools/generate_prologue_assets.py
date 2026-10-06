#!/usr/bin/env python3
"""
tools/generate_prologue_assets.py - Canonical GBA The Minish Cap Storybook Prologue Generator

Generates a 480x320 32-bit RGBA BMP containing 4 authentic storybook panels (240x160 each):
- Panel 0 (0, 0):     "The Darkness in the World" - Demonic shadows creeping over Hyrule mountains
- Panel 1 (240, 0):   "The Descent of the Picori" - Tiny folk descending from golden clouds with Force & Picori Blade
- Panel 2 (0, 160):   "The Hero & Bound Chest"    - Hero sealing the monsters into the Bound Chest with Picori Blade
- Panel 3 (240, 160): "The Picori Festival"       - Hyrule in peace, royal banners and celebration of the centennial festival
"""

import os
import struct
import math
from PIL import Image, ImageDraw, ImageFont

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

def draw_parchment_border(draw, w, h):
    # Antique ornamental gold & leather border
    # Outer dark rim
    draw.rectangle([0, 0, w - 1, h - 1], outline=(58, 42, 28, 255), width=2)
    draw.rectangle([3, 3, w - 4, h - 4], outline=(180, 140, 60, 255), width=1)
    draw.rectangle([5, 5, w - 6, h - 6], outline=(100, 75, 45, 255), width=1)
    # Corner ornaments
    corners = [(4, 4), (w - 12, 4), (4, h - 12), (w - 12, h - 12)]
    for cx, cy in corners:
        draw.rectangle([cx, cy, cx + 7, cy + 7], fill=(218, 165, 32, 255), outline=(90, 60, 20, 255))
        draw.point((cx + 3, cy + 3), fill=(255, 255, 255, 255))

def create_panel_0_darkness():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (38, 28, 44, 255)) # Dark plum / antique parchment base
    draw = ImageDraw.Draw(img)

    # Aged parchment background gradient
    for y in range(h):
        grad = y / h
        r = int(38 + grad * 20)
        g = int(24 + grad * 15)
        b = int(40 + grad * 18)
        draw.line([0, y, w, y], fill=(r, g, b, 255))

    # Blood moon / eclipse sun
    draw.ellipse([w//2 - 28, 22, w//2 + 28, 78], fill=(168, 40, 40, 255), outline=(100, 20, 20, 255))
    draw.ellipse([w//2 - 20, 30, w//2 + 20, 70], fill=(200, 60, 60, 255))

    # Menacing storm clouds in background
    for cx, cy, rad in [(40, 30, 35), (90, 20, 45), (150, 25, 40), (200, 32, 35)]:
        draw.ellipse([cx - rad, cy - rad, cx + rad, cy + rad], fill=(24, 16, 32, 220))

    # Jagged mountain peaks (Hyrule shrouded in peril)
    mountains = [
        [(0, 160), (30, 85), (65, 120), (105, 70), (145, 115), (185, 75), (215, 110), (240, 80), (240, 160)]
    ]
    for pts in mountains:
        draw.polygon(pts, fill=(18, 12, 24, 255))
        draw.line(pts, fill=(45, 30, 55, 255), width=2)

    # Menacing monster silhouettes & glowing red eyes rising from shadows
    # Silhouette 1 (Moblin-like horns left)
    draw.polygon([(25, 140), (35, 105), (32, 95), (38, 102), (48, 100), (52, 92), (50, 105), (60, 140)], fill=(8, 6, 12, 255))
    draw.point((38, 110), fill=(255, 30, 30, 255))
    draw.point((46, 110), fill=(255, 30, 30, 255))

    # Silhouette 2 (Giant demon in center with claw hands)
    draw.polygon([(100, 160), (105, 95), (95, 80), (110, 88), (115, 72), (125, 72), (130, 88), (145, 80), (135, 95), (140, 160)], fill=(8, 6, 12, 255))
    draw.ellipse([116, 92, 120, 96], fill=(255, 60, 60, 255))
    draw.ellipse([124, 92, 128, 96], fill=(255, 60, 60, 255))

    # Silhouette 3 (Bat/Keese swarm right)
    for bx, by in [(175, 95), (205, 85), (190, 110), (160, 80), (220, 100)]:
        draw.polygon([(bx, by), (bx - 8, by - 6), (bx - 4, by), (bx, by + 4), (bx + 4, by), (bx + 8, by - 6)], fill=(12, 8, 16, 255))
        draw.point((bx - 1, by + 1), fill=(255, 50, 50, 255))
        draw.point((bx + 1, by + 1), fill=(255, 50, 50, 255))

    draw_parchment_border(draw, w, h)
    return img

def create_panel_1_picori_descent():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (235, 218, 185, 255)) # Antique warm golden parchment
    draw = ImageDraw.Draw(img)

    # Golden holy light rays radiating from center top
    center_x, center_y = w // 2, 20
    for deg in range(0, 181, 6):
        rad = math.radians(deg)
        x2 = center_x + math.cos(rad) * 260
        y2 = center_y + math.sin(rad) * 200
        beam_col = (255, 245, 180, 75) if (deg // 6) % 2 == 0 else (245, 210, 120, 40)
        draw.polygon([(center_x - 4, center_y), (center_x + 4, center_y), (x2 + 8, y2), (x2 - 8, y2)], fill=beam_col)

    # Celestial golden clouds at the top
    for cx, cy, rad in [(30, 15, 30), (80, 10, 40), (130, 8, 42), (180, 12, 38), (220, 15, 28)]:
        draw.ellipse([cx - rad, cy - rad, cx + rad, cy + rad], fill=(255, 250, 220, 230), outline=(230, 195, 110, 255))

    # The Golden Light (Sacred Force) radiating at center
    draw.polygon([(center_x, center_y + 10), (center_x - 14, center_y + 35), (center_x + 14, center_y + 35)], fill=(255, 240, 100, 255), outline=(220, 160, 20, 255))
    draw.polygon([(center_x - 14, center_y + 35), (center_x - 28, center_y + 60), (center_x, center_y + 60)], fill=(255, 240, 100, 255), outline=(220, 160, 20, 255))
    draw.polygon([(center_x + 14, center_y + 35), (center_x, center_y + 60), (center_x + 28, center_y + 60)], fill=(255, 240, 100, 255), outline=(220, 160, 20, 255))

    # The Sacred Picori Blade descending vertically beneath the Force
    blade_x = center_x
    blade_top = center_y + 68
    draw.line([blade_x, blade_top, blade_x, blade_top + 38], fill=(160, 230, 255, 255), width=3) # Blade
    draw.line([blade_x, blade_top, blade_x, blade_top + 38], fill=(255, 255, 255, 255), width=1) # Specular core
    draw.line([blade_x - 8, blade_top + 8, blade_x + 8, blade_top + 8], fill=(245, 190, 30, 255), width=2) # Gold crossguard
    draw.line([blade_x, blade_top + 2, blade_x, blade_top + 8], fill=(80, 120, 180, 255), width=2) # Blue hilt
    draw.point((blade_x, blade_top + 1), fill=(255, 215, 0, 255)) # Pommel

    # Tiny magical Picori folk flying / descending alongside the gifts
    picori_positions = [
        (center_x - 55, 65, (220, 50, 50, 255)),   # Red cap
        (center_x - 30, 95, (40, 160, 70, 255)),   # Green tunic
        (center_x + 35, 90, (50, 120, 220, 255)),  # Blue cloak
        (center_x + 60, 68, (230, 180, 30, 255)),  # Yellow cap
    ]
    for px, py, tunic_col in picori_positions:
        # Glow halo
        draw.ellipse([px - 10, py - 10, px + 10, py + 10], fill=(255, 255, 200, 80))
        # Body / tunic
        draw.polygon([(px, py - 6), (px - 5, py + 4), (px + 5, py + 4)], fill=tunic_col)
        # Head & face
        draw.ellipse([px - 3, py - 8, px + 3, py - 3], fill=(255, 220, 190, 255))
        # Pointed Minish Cap
        draw.polygon([(px - 4, py - 7), (px + 4, py - 7), (px - 1, py - 14)], fill=(220, 40, 40, 255))
        draw.point((px - 1, py - 14), fill=(255, 255, 255, 255)) # White pom-pom
        # Sparkle trail
        draw.point((px + 2, py + 7), fill=(255, 255, 150, 255))
        draw.point((px - 3, py + 10), fill=(255, 255, 200, 255))

    # Ancient mountains in distant relief at bottom
    draw.polygon([(0, 160), (40, 135), (90, 145), (140, 130), (190, 142), (240, 128), (240, 160)], fill=(195, 175, 140, 255))

    draw_parchment_border(draw, w, h)
    return img

def create_panel_2_hero_bound_chest():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (225, 210, 175, 255))
    draw = ImageDraw.Draw(img)

    # Dramatic spotlight / divine halo behind the hero
    draw.ellipse([20, 20, 130, 130], fill=(255, 245, 200, 160))

    # The Hero of Men (left side in noble stance, holding blade)
    hx, hy = 75, 95
    # Green tunic & cap
    draw.polygon([(hx - 10, hy + 20), (hx + 10, hy + 20), (hx + 6, hy - 8), (hx - 6, hy - 8)], fill=(34, 150, 72, 255))
    draw.polygon([(hx - 5, hy - 10), (hx + 5, hy - 10), (hx - 12, hy - 20)], fill=(34, 150, 72, 255)) # Cap
    draw.ellipse([hx - 5, hy - 12, hx + 5, hy - 4], fill=(255, 218, 185, 255)) # Head
    draw.line([hx - 4, hy + 20, hx - 6, hy + 35], fill=(225, 220, 210, 255), width=3) # White leggings
    draw.line([hx + 4, hy + 20, hx + 8, hy + 35], fill=(225, 220, 210, 255), width=3)
    draw.rectangle([hx - 9, hy + 32, hx - 3, hy + 36], fill=(139, 69, 19, 255)) # Boots
    draw.rectangle([hx + 5, hy + 32, hx + 11, hy + 36], fill=(139, 69, 19, 255))

    # Hero's outstretched arm directing the Picori Blade into the Bound Chest
    draw.line([hx + 6, hy - 2, hx + 35, hy + 2], fill=(255, 218, 185, 255), width=3)

    # The Bound Chest on altar (right side)
    cx, cy = 160, 95
    # Stone altar pedestal
    draw.rectangle([cx - 30, cy + 18, cx + 30, cy + 40], fill=(160, 150, 135, 255), outline=(100, 90, 80, 255))
    draw.line([cx - 30, cy + 18, cx + 30, cy + 18], fill=(200, 190, 175, 255))

    # The Chest body (mahogany with gold bands)
    draw.rectangle([cx - 22, cy - 8, cx + 22, cy + 18], fill=(120, 50, 20, 255), outline=(60, 25, 10, 255))
    draw.arc([cx - 22, cy - 20, cx + 22, cy + 4], start=180, end=0, fill=(140, 60, 25, 255), width=10) # Rounded lid
    # Golden reinforced bands & runes
    draw.line([cx - 14, cy - 14, cx - 14, cy + 18], fill=(245, 197, 24, 255), width=2)
    draw.line([cx + 14, cy - 14, cx + 14, cy + 18], fill=(245, 197, 24, 255), width=2)
    # Sacred lock with gold emblem
    draw.rectangle([cx - 6, cy - 2, cx + 6, cy + 10], fill=(245, 197, 24, 255), outline=(160, 120, 10, 255))

    # The Picori Blade plunged into the chest lock!
    draw.line([cx, cy - 26, cx, cy], fill=(180, 240, 255, 255), width=3)
    draw.line([cx, cy - 26, cx, cy], fill=(255, 255, 255, 255), width=1) # Specular gleam
    draw.line([cx - 7, cy - 20, cx + 7, cy - 20], fill=(245, 197, 24, 255), width=2) # Guard
    draw.point((cx, cy - 28), fill=(245, 197, 24, 255))

    # Golden chains wrapping the Bound Chest
    draw.line([cx - 24, cy, cx - 10, cy + 12], fill=(250, 215, 60, 255), width=2)
    draw.line([cx + 10, cy + 12, cx + 24, cy], fill=(250, 215, 60, 255), width=2)

    # Defeated monsters being dragged into the chest in vortex of purple mist
    for vx, vy in [(140, 45), (175, 40), (200, 60), (185, 75)]:
        draw.arc([vx - 12, vy - 12, vx + 12, vy + 12], start=45, end=270, fill=(130, 40, 160, 180), width=3)
        draw.point((vx - 2, vy), fill=(255, 50, 50, 255))

    draw_parchment_border(draw, w, h)
    return img

def create_panel_3_picori_festival():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (220, 235, 245, 255)) # Soft azure sky
    draw = ImageDraw.Draw(img)

    # Bright sunny morning sky
    for y in range(80):
        grad = y / 80.0
        r = int(140 + grad * 80)
        g = int(200 + grad * 35)
        b = int(250 - grad * 10)
        draw.line([0, y, w, y], fill=(r, g, b, 255))

    # Green rolling hills of Hyrule
    draw.polygon([(0, 160), (0, 95), (60, 85), (130, 95), (190, 88), (240, 92), (240, 160)], fill=(72, 180, 80, 255))

    # Hyrule Castle silhouette in background
    cast_x = w // 2
    # Central keep & towers
    draw.rectangle([cast_x - 35, 45, cast_x + 35, 95], fill=(240, 242, 248, 255), outline=(180, 185, 195, 255))
    draw.polygon([(cast_x - 42, 45), (cast_x - 28, 45), (cast_x - 35, 22)], fill=(40, 100, 180, 255)) # Left blue spire
    draw.polygon([(cast_x + 28, 45), (cast_x + 42, 45), (cast_x + 35, 22)], fill=(40, 100, 180, 255)) # Right blue spire
    draw.polygon([(cast_x - 12, 45), (cast_x + 12, 45), (cast_x, 15)], fill=(40, 100, 180, 255))     # Center grand spire
    draw.line([cast_x, 15, cast_x, 8], fill=(245, 197, 24, 255), width=1) # Gold flagpole
    draw.polygon([(cast_x, 8), (cast_x + 8, 11), (cast_x, 14)], fill=(239, 68, 68, 255)) # Royal red banner

    # Festive bunting & pennants fluttering across the sky
    bunting_points = [(15, 65), (55, 78), (95, 68), (145, 78), (195, 68), (230, 72)]
    for i in range(len(bunting_points) - 1):
        x1, y1 = bunting_points[i]
        x2, y2 = bunting_points[i+1]
        draw.line([x1, y1, x2, y2], fill=(120, 100, 80, 255), width=1)
        # Small colored triangular flags
        mid_x = (x1 + x2) // 2
        mid_y = (y1 + y2) // 2 + 1
        col = [(239, 68, 68, 255), (59, 130, 246, 255), (250, 204, 21, 255), (34, 197, 94, 255)][i % 4]
        draw.polygon([(mid_x - 4, mid_y), (mid_x + 4, mid_y), (mid_x, mid_y + 8)], fill=col)

    # Cheering townsfolk & festive crowd silhouettes in foreground
    for fx in range(15, 230, 18):
        c_h = 20 + (fx % 5) * 3
        draw.polygon([(fx - 4, 160), (fx + 4, 160), (fx + 2, 160 - c_h), (fx - 2, 160 - c_h)], fill=(45, 80, 50, 255))
        draw.ellipse([fx - 3, 160 - c_h - 6, fx + 3, 160 - c_h], fill=(245, 210, 180, 255)) # Head
        if fx % 36 == 0:
            # Waving arm with flag
            draw.line([fx + 2, 160 - c_h + 4, fx + 8, 160 - c_h - 6], fill=(245, 210, 180, 255), width=2)
            draw.polygon([(fx + 8, 160 - c_h - 10), (fx + 16, 160 - c_h - 6), (fx + 8, 160 - c_h - 2)], fill=(250, 204, 21, 255))

    # The Bound Chest on high dais honored during festival
    draw.rectangle([cast_x - 12, 120, cast_x + 12, 142], fill=(160, 140, 110, 255), outline=(90, 75, 55, 255))
    draw.rectangle([cast_x - 8, 126, cast_x + 8, 138], fill=(120, 50, 20, 255), outline=(245, 197, 24, 255))
    # Glowing Picori Blade emblem on chest
    draw.line([cast_x, 123, cast_x, 134], fill=(180, 240, 255, 255), width=2)

    # Confetti specks in the breeze
    confetti_cols = [(255, 215, 0, 255), (239, 68, 68, 255), (59, 130, 246, 255), (255, 255, 255, 255), (34, 197, 94, 255)]
    for i, (cx, cy) in enumerate([(35, 45), (65, 30), (105, 55), (140, 35), (170, 50), (210, 38), (85, 80), (185, 75)]):
        draw.rectangle([cx, cy, cx + 2, cy + 2], fill=confetti_cols[i % len(confetti_cols)])

    draw_parchment_border(draw, w, h)
    return img

def build_prologue_atlas():
    atlas_w = 480
    atlas_h = 320
    atlas = Image.new('RGBA', (atlas_w, atlas_h), (0, 0, 0, 255))

    p0 = create_panel_0_darkness()
    p1 = create_panel_1_picori_descent()
    p2 = create_panel_2_hero_bound_chest()
    p3 = create_panel_3_picori_festival()

    atlas.paste(p0, (0, 0))
    atlas.paste(p1, (240, 0))
    atlas.paste(p2, (0, 160))
    atlas.paste(p3, (240, 160))

    paths = [
        'assets/ui/prologue/prologue_panels.bmp',
        'assets/regions/prologue_panels_master.bmp',
        'assets/regions/usa/prologue_panels.bmp',
        'assets/regions/eur/prologue_panels.bmp',
        'assets/regions/jpn/prologue_panels.bmp'
    ]
    for p in paths:
        save_bmp_32(p, atlas)
    print("[SUCESSO] Atlas de Prólogo da Lenda dos Picori gerado com êxito!")

if __name__ == '__main__':
    build_prologue_atlas()
