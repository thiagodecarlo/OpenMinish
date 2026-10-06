#!/usr/bin/env python3
"""
tools/generate_overworld_tilesets.py - Canonical GBA Overworld Tilesets & Pre-rendered Regional Maps Generator

Generates authentic pixel-perfect 16x16 metatiles and overworld maps for:
1. Hyrule Town Hub (Cobblestone, Timber/Stucco walls, Terracotta/Blue roofs, Fountain, Stalls, Barrels)
2. Mount Crenel (Volcanic gravel, Basalt cliffs, Grip Ring climbing walls, Mineral hot springs, Magic bean)
3. Castor Wilds (Deep swamp mud, Murky water, Olive marsh moss, Hollow logs, Ancient eye statues, Mole dirt)
4. Hyrule Castle Courtyard (Royal marble pavement, Topiary hedges, White stone walls, Triforce mosaic, Royal roses)

Outputs:
- assets/regions/tileset_overworld_master.bmp (256x256 32-bit RGBA BMP)
- assets/regions/{usa,eur,jpn}/tileset_overworld.bmp
- Pre-rendered overworld maps and collision matrices:
  - map_town.bmp & map_town_collision.bin (576x448 = 36x28 metatiles)
  - map_crenel.bmp & map_crenel_collision.bin (512x384 = 32x24 metatiles)
  - map_castor.bmp & map_castor_collision.bin (576x448 = 36x28 metatiles)
  - map_castle_courtyard.bmp & map_castle_courtyard_collision.bin (512x384 = 32x24 metatiles)
"""

import os
import struct
import math
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

def save_binary_collision(filepath, width_tiles, height_tiles, collision_matrix):
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    data = bytearray(collision_matrix)
    with open(filepath, 'wb') as f:
        f.write(data)
    print(f"  -> Matriz de colisao salva: {filepath} ({width_tiles}x{height_tiles} = {len(data)} bytes)")

# -----------------------------------------------------------------------------
# Metatile Generator Functions (16x16 RGBA)
# -----------------------------------------------------------------------------
def make_cell():
    return Image.new('RGBA', (16, 16), (0, 0, 0, 0))

# OVERWORLD BASE
def tile_grass():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(65, 180, 139, 255))
    # Delicate grass blades
    blades_light = [(3, 4), (11, 9), (7, 13), (1, 11), (13, 2)]
    blades_dark  = [(4, 5), (12, 10), (8, 14), (2, 12), (14, 3)]
    for bx, by in blades_light: c.putpixel((bx, by), (96, 218, 172, 255))
    for bx, by in blades_dark:  c.putpixel((bx, by), (42, 142, 106, 255))
    return c

def tile_dirt_path():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(218, 180, 132, 255))
    pebbles = [(2, 3), (13, 11), (5, 8), (10, 4), (7, 14)]
    for px, py in pebbles:
        c.putpixel((px, py), (242, 216, 178, 255))
        c.putpixel((px + 1, py), (174, 136, 92, 255))
    return c

def tile_water():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(52, 138, 218, 255))
    # Soft wave ripples
    for y in range(16):
        for x in range(16):
            if (x + y * 2) % 7 == 0:
                c.putpixel((x, y), (102, 188, 248, 255))
            elif (x + y * 2) % 14 == 0:
                c.putpixel((x, y), (210, 240, 255, 255))
    return c

def tile_stone_wall():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(136, 144, 154, 255))
    # Stone brick mortar lines
    d.line([0, 0, 15, 0], fill=(78, 86, 96, 255))
    d.line([0, 7, 15, 7], fill=(78, 86, 96, 255))
    d.line([0, 8, 15, 8], fill=(178, 186, 196, 255))
    d.line([0, 15, 15, 15], fill=(78, 86, 96, 255))
    # Vertical seams
    d.line([7, 1, 7, 6], fill=(78, 86, 96, 255))
    d.line([12, 9, 12, 14], fill=(78, 86, 96, 255))
    d.line([3, 9, 3, 14], fill=(78, 86, 96, 255))
    return c

def tile_bush():
    c = tile_grass()
    d = ImageDraw.Draw(c)
    # Circular rounded bush with shading
    for y in range(1, 15):
        for x in range(1, 15):
            dist = math.hypot(x - 7.5, y - 7.5)
            if dist < 6.8:
                col = (34, 150, 72, 255)
                if dist < 3.8 and y < 7: col = (76, 196, 112, 255)
                elif dist > 5.2 or y > 10: col = (20, 102, 48, 255)
                c.putpixel((x, y), col)
    return c

def tile_flower_red():
    c = tile_grass()
    red = (239, 68, 68, 255)
    yel = (250, 204, 21, 255)
    for fx, fy in [(7, 6), (9, 6), (7, 8), (9, 8)]: c.putpixel((fx, fy), red)
    c.putpixel((8, 7), yel)
    return c

def tile_flower_yellow():
    c = tile_grass()
    yel = (250, 204, 21, 255)
    for fx, fy in [(6, 7), (8, 7), (7, 6), (7, 8)]: c.putpixel((fx, fy), yel)
    c.putpixel((7, 7), (255, 255, 255, 255))
    return c

# HYRULE TOWN TILES
def tile_cobblestone():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(196, 192, 180, 255))
    # Irregular interlocking cobblestone joints
    seams = [
        (0, 5, 15, 5), (0, 10, 15, 10),
        (5, 0, 5, 5), (11, 0, 11, 5),
        (3, 5, 3, 10), (8, 5, 8, 10), (14, 5, 14, 10),
        (6, 10, 6, 15), (12, 10, 12, 15)
    ]
    for x1, y1, x2, y2 in seams:
        d.line([x1, y1, x2, y2], fill=(142, 138, 126, 255))
    # Highlight dots
    for hx, hy in [(2, 2), (8, 2), (13, 2), (1, 7), (5, 7), (10, 7), (3, 12), (9, 12)]:
        c.putpixel((hx, hy), (224, 222, 212, 255))
    return c

def tile_town_wall():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(234, 224, 200, 255)) # Cream stucco
    # Half-timbered oak wood frame
    d.line([0, 0, 15, 0], fill=(120, 72, 38, 255), width=2)
    d.line([0, 14, 15, 14], fill=(120, 72, 38, 255), width=2)
    d.line([0, 0, 0, 15], fill=(100, 58, 28, 255), width=2)
    d.line([14, 0, 14, 15], fill=(100, 58, 28, 255), width=2)
    # Diagonal timber strut
    d.line([2, 2, 13, 13], fill=(136, 84, 46, 255))
    return c

def tile_roof_red():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(204, 58, 38, 255))
    # Terracotta tiles overlapping horizontal ridges
    d.line([0, 0, 15, 0], fill=(240, 102, 78, 255))
    d.line([0, 4, 15, 4], fill=(140, 32, 20, 255))
    d.line([0, 5, 15, 5], fill=(240, 102, 78, 255))
    d.line([0, 9, 15, 9], fill=(140, 32, 20, 255))
    d.line([0, 10, 15, 10], fill=(240, 102, 78, 255))
    d.line([0, 14, 15, 14], fill=(140, 32, 20, 255))
    d.line([0, 15, 15, 15], fill=(240, 102, 78, 255))
    # Vertical overlaps
    d.line([4, 0, 4, 4], fill=(140, 32, 20, 255))
    d.line([10, 0, 10, 4], fill=(140, 32, 20, 255))
    d.line([7, 5, 7, 9], fill=(140, 32, 20, 255))
    d.line([13, 5, 13, 9], fill=(140, 32, 20, 255))
    return c

def tile_roof_blue():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(42, 102, 182, 255))
    # Stockwell Shop cobalt blue tiles
    d.line([0, 0, 15, 0], fill=(86, 154, 234, 255))
    d.line([0, 5, 15, 5], fill=(24, 60, 116, 255))
    d.line([0, 6, 15, 6], fill=(86, 154, 234, 255))
    d.line([0, 10, 15, 10], fill=(24, 60, 116, 255))
    d.line([0, 11, 15, 11], fill=(86, 154, 234, 255))
    d.line([0, 15, 15, 15], fill=(24, 60, 116, 255))
    return c

def tile_town_door():
    c = tile_town_wall()
    d = ImageDraw.Draw(c)
    # Heavy oak door
    d.rounded_rectangle([3, 2, 12, 15], radius=3, fill=(112, 66, 32, 255), outline=(70, 38, 16, 255))
    # Vertical wooden planks
    d.line([6, 5, 6, 14], fill=(84, 48, 22, 255))
    d.line([9, 5, 9, 14], fill=(84, 48, 22, 255))
    # Brass doorknob
    c.putpixel((11, 9), (250, 204, 21, 255))
    c.putpixel((11, 10), (196, 148, 16, 255))
    return c

def tile_town_window():
    c = tile_town_wall()
    d = ImageDraw.Draw(c)
    # Glass window pane with shutters
    d.rectangle([3, 3, 12, 11], fill=(132, 208, 236, 255), outline=(78, 44, 20, 255))
    d.line([3, 7, 12, 7], fill=(78, 44, 20, 255))
    d.line([7, 3, 7, 11], fill=(78, 44, 20, 255))
    # Diagonal glass reflection
    c.putpixel((5, 5), (255, 255, 255, 255))
    c.putpixel((10, 9), (255, 255, 255, 255))
    return c

def tile_fountain_edge():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(196, 192, 180, 255)) # Cobblestone base
    # Carved marble rim
    d.arc([0, 0, 15, 15], start=0, end=360, fill=(236, 236, 244, 255), width=3)
    # Water interior
    d.ellipse([3, 3, 12, 12], fill=(54, 168, 224, 255))
    c.putpixel((7, 7), (255, 255, 255, 255)) # Water droplet splash
    return c

def tile_market_stall():
    c = tile_cobblestone()
    d = ImageDraw.Draw(c)
    # Red and white striped canopy
    for x in range(16):
        col = (220, 38, 38, 255) if (x // 2) % 2 == 0 else (248, 248, 252, 255)
        d.line([x, 0, x, 7], fill=col)
    d.line([0, 7, 15, 7], fill=(180, 28, 28, 255))
    # Wooden counter
    d.rectangle([1, 8, 14, 15], fill=(160, 108, 62, 255), outline=(108, 68, 36, 255))
    return c

def tile_barrel_crate():
    c = tile_cobblestone()
    d = ImageDraw.Draw(c)
    # Wooden barrel on left, crate on right
    d.rounded_rectangle([1, 3, 7, 14], radius=2, fill=(136, 88, 48, 255), outline=(82, 48, 24, 255))
    d.line([1, 6, 7, 6], fill=(70, 74, 82, 255)) # Iron hoop
    d.line([1, 11, 7, 11], fill=(70, 74, 82, 255))
    # Crate
    d.rectangle([8, 4, 14, 14], fill=(164, 112, 66, 255), outline=(96, 62, 32, 255))
    d.line([8, 4, 14, 14], fill=(96, 62, 32, 255)) # Diagonal cross brace
    d.line([8, 14, 14, 4], fill=(96, 62, 32, 255))
    return c

# MOUNT CRENEL TILES
def tile_crenel_gravel():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(116, 84, 66, 255))
    # Volcanic pebbles
    darks  = [(3, 4), (12, 8), (7, 12), (1, 14), (14, 2), (9, 3)]
    lights = [(4, 3), (11, 7), (8, 11), (2, 13), (13, 1), (10, 2)]
    for dx, dy in darks:  c.putpixel((dx, dy), (68, 46, 34, 255))
    for lx, ly in lights: c.putpixel((lx, ly), (156, 118, 96, 255))
    return c

def tile_crenel_cliff():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(76, 60, 54, 255))
    # Jagged horizontal basalt strata
    d.line([0, 1, 15, 1], fill=(122, 102, 92, 255))
    d.line([0, 6, 15, 6], fill=(42, 32, 28, 255))
    d.line([0, 7, 15, 7], fill=(112, 94, 84, 255))
    d.line([0, 12, 15, 12], fill=(42, 32, 28, 255))
    d.line([0, 13, 15, 13], fill=(98, 80, 72, 255))
    # Vertical crags
    for cx in [4, 9, 13]: d.line([cx, 7, cx, 12], fill=(42, 32, 28, 255))
    return c

def tile_climbable_wall():
    c = tile_crenel_cliff()
    d = ImageDraw.Draw(c)
    # Handholds and climbing crevices for Grip Ring
    grips = [(3, 4), (9, 3), (5, 9), (12, 10), (7, 14)]
    for gx, gy in grips:
        d.rectangle([gx, gy, gx + 2, gy + 1], fill=(192, 168, 144, 255))
        d.line([gx, gy + 2, gx + 2, gy + 2], fill=(30, 20, 16, 255))
    return c

def tile_mineral_water():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(54, 196, 138, 255)) # Thermal emerald water
    # Effervescent bubbles
    bubbles = [(4, 3), (12, 6), (7, 9), (2, 11), (10, 13), (13, 2)]
    for bx, by in bubbles:
        c.putpixel((bx, by), (218, 250, 234, 255))
        c.putpixel((bx + 1, by), (128, 234, 186, 255))
    return c

def tile_beanstalk():
    c = tile_crenel_gravel()
    d = ImageDraw.Draw(c)
    # Giant twisting bean vine
    d.line([7, 0, 7, 15], fill=(34, 168, 48, 255), width=4)
    d.line([8, 0, 8, 15], fill=(86, 214, 102, 255))
    # Leaves curling off the stem
    d.ellipse([2, 3, 6, 6], fill=(34, 168, 48, 255))
    d.ellipse([9, 9, 13, 12], fill=(34, 168, 48, 255))
    return c

# CASTOR WILDS SWAMP TILES
def tile_swamp_mud():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(60, 52, 40, 255)) # Dark viscous bog mud
    # Mud ripple circles
    d.ellipse([2, 3, 7, 6], outline=(78, 68, 52, 255))
    d.ellipse([9, 9, 14, 13], outline=(78, 68, 52, 255))
    # Methane bubbles
    c.putpixel((4, 4), (112, 100, 78, 255))
    c.putpixel((11, 10), (112, 100, 78, 255))
    return c

def tile_swamp_water():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(42, 68, 62, 255)) # Murky greenish-slate water
    # Murky algae swirls
    for y in range(16):
        for x in range(16):
            if (x * 2 + y) % 9 == 0:
                c.putpixel((x, y), (64, 98, 88, 255))
    return c

def tile_swamp_grass():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(82, 112, 52, 255)) # Olive marsh grass
    reeds = [(3, 2), (4, 4), (11, 5), (12, 8), (7, 11), (8, 13)]
    for rx, ry in reeds:
        c.putpixel((rx, ry), (124, 158, 74, 255))
        c.putpixel((rx, ry + 1), (54, 78, 32, 255))
    return c

def tile_swamp_log():
    c = tile_swamp_mud()
    d = ImageDraw.Draw(c)
    # Hollow fallen log
    d.rounded_rectangle([2, 1, 13, 14], radius=3, fill=(108, 72, 42, 255), outline=(62, 40, 22, 255))
    # Hollow core
    d.ellipse([4, 4, 11, 11], fill=(28, 18, 10, 255))
    # Moss patch on bark
    d.rectangle([3, 1, 6, 3], fill=(82, 118, 52, 255))
    return c

def tile_eye_statue(opened=False):
    c = tile_swamp_mud()
    d = ImageDraw.Draw(c)
    # Ancient weathered stone pillar with carved eye
    d.rounded_rectangle([2, 1, 13, 14], radius=2, fill=(128, 134, 142, 255), outline=(72, 78, 86, 255))
    # Eye shape in center
    d.ellipse([4, 5, 11, 10], fill=(40, 44, 50, 255), outline=(200, 204, 212, 255))
    if opened:
        # Glowing emerald eye
        d.ellipse([6, 6, 9, 9], fill=(16, 240, 140, 255))
        c.putpixel((7, 7), (255, 255, 255, 255))
    else:
        # Closed eye slit
        d.line([5, 8, 10, 8], fill=(20, 22, 26, 255))
    return c

def tile_dirt_wall():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(142, 88, 52, 255)) # Soft diggable dirt
    # Soil granules and Mole Mitts scratch textures
    d.line([2, 4, 7, 9], fill=(92, 54, 30, 255))
    d.line([9, 3, 14, 8], fill=(92, 54, 30, 255))
    d.line([3, 11, 8, 15], fill=(92, 54, 30, 255))
    highlights = [(4, 2), (11, 2), (6, 7), (13, 12), (2, 13)]
    for hx, hy in highlights: c.putpixel((hx, hy), (188, 126, 84, 255))
    return c

# HYRULE CASTLE COURTYARD TILES
def tile_castle_marble():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(236, 238, 244, 255)) # Polished white royal marble
    # Subtle grout lines
    d.line([0, 0, 15, 0], fill=(196, 200, 210, 255))
    d.line([0, 0, 0, 15], fill=(196, 200, 210, 255))
    # Marble veining and specular sheen
    c.putpixel((2, 2), (255, 255, 255, 255))
    c.putpixel((8, 8), (218, 222, 232, 255))
    c.putpixel((12, 4), (255, 255, 255, 255))
    return c

def tile_castle_hedge():
    c = tile_grass()
    d = ImageDraw.Draw(c)
    # Manicured rectangular topiary hedge
    d.rounded_rectangle([1, 1, 14, 14], radius=3, fill=(38, 140, 54, 255), outline=(20, 84, 32, 255))
    # Leafy highlights on top surface
    d.line([3, 3, 12, 3], fill=(82, 196, 102, 255))
    d.line([3, 4, 12, 4], fill=(58, 172, 78, 255))
    return c

def tile_castle_wall():
    c = make_cell()
    d = ImageDraw.Draw(c)
    d.rectangle([0, 0, 15, 15], fill=(222, 226, 234, 255)) # Royal white ashlar masonry
    # Mortar joints with golden filigree edge
    d.line([0, 0, 15, 0], fill=(218, 178, 54, 255)) # Gold trim on top
    d.line([0, 7, 15, 7], fill=(170, 174, 186, 255))
    d.line([0, 15, 15, 15], fill=(170, 174, 186, 255))
    d.line([7, 1, 7, 6], fill=(170, 174, 186, 255))
    d.line([12, 8, 12, 14], fill=(170, 174, 186, 255))
    return c

def tile_castle_battlement():
    c = make_cell()
    d = ImageDraw.Draw(c)
    # Crenellation parapet with defensive notch
    d.rectangle([0, 4, 6, 15], fill=(222, 226, 234, 255), outline=(170, 174, 186, 255))
    d.rectangle([9, 4, 15, 15], fill=(222, 226, 234, 255), outline=(170, 174, 186, 255))
    d.line([0, 4, 6, 4], fill=(218, 178, 54, 255))
    d.line([9, 4, 15, 4], fill=(218, 178, 54, 255))
    return c

def tile_castle_gate():
    c = tile_castle_marble()
    d = ImageDraw.Draw(c)
    # Wrought iron royal gate with gold spearheads
    for x in [3, 7, 11]:
        d.line([x, 2, x, 15], fill=(42, 46, 52, 255), width=2)
        c.putpixel((x, 1), (250, 204, 21, 255)) # Gold spearhead
    d.line([1, 6, 14, 6], fill=(42, 46, 52, 255))
    d.line([1, 11, 14, 11], fill=(42, 46, 52, 255))
    return c

def tile_castle_flower_royal():
    c = tile_grass()
    d = ImageDraw.Draw(c)
    # Royal red rose with deep crimson shading
    d.ellipse([4, 4, 11, 11], fill=(216, 28, 48, 255))
    d.ellipse([6, 6, 9, 9], fill=(248, 88, 108, 255))
    c.putpixel((7, 7), (255, 255, 255, 255))
    return c

def tile_castle_triforce():
    c = tile_castle_marble()
    d = ImageDraw.Draw(c)
    # Sacred Triforce golden inlay
    gold = (250, 204, 21, 255)
    dark_gold = (180, 140, 16, 255)
    # Top triangle
    d.polygon([(7, 3), (4, 8), (10, 8)], fill=gold, outline=dark_gold)
    # Bottom-left triangle
    d.polygon([(4, 8), (1, 13), (7, 13)], fill=gold, outline=dark_gold)
    # Bottom-right triangle
    d.polygon([(10, 8), (7, 13), (13, 13)], fill=gold, outline=dark_gold)
    return c

# -----------------------------------------------------------------------------
# Atlas Composer: 256x256 RGBA (16 cols x 16 rows of 16x16 metatiles)
# -----------------------------------------------------------------------------
def build_overworld_atlas():
    atlas_w = 256
    atlas_h = 256
    atlas = Image.new('RGBA', (atlas_w, atlas_h), (0, 0, 0, 0))

    def paste_tile(col, row, tile_img):
        atlas.paste(tile_img, (col * 16, row * 16))

    # ROW 0: OVERWORLD BASE
    paste_tile(0, 0, tile_grass())
    paste_tile(1, 0, tile_dirt_path())
    paste_tile(2, 0, tile_water())
    paste_tile(3, 0, tile_stone_wall())
    paste_tile(4, 0, tile_bush())
    paste_tile(5, 0, tile_flower_red())
    paste_tile(6, 0, tile_flower_yellow())
    paste_tile(7, 0, tile_bush()) # Tree top placeholder
    paste_tile(8, 0, tile_barrel_crate()) # Tree trunk placeholder
    paste_tile(9, 0, tile_barrel_crate())
    paste_tile(10, 0, tile_barrel_crate())
    paste_tile(11, 0, tile_castle_gate())

    # ROW 1: HYRULE TOWN
    paste_tile(0, 1, tile_cobblestone())
    paste_tile(1, 1, tile_town_wall())
    paste_tile(2, 1, tile_roof_red())
    paste_tile(3, 1, tile_roof_blue())
    paste_tile(4, 1, tile_town_door())
    paste_tile(5, 1, tile_town_window())
    paste_tile(6, 1, tile_fountain_edge())
    paste_tile(7, 1, tile_market_stall())
    paste_tile(8, 1, tile_barrel_crate())
    paste_tile(9, 1, tile_cobblestone())
    paste_tile(10, 1, tile_town_wall())
    paste_tile(11, 1, tile_roof_red())

    # ROW 2: MOUNT CRENEL
    paste_tile(0, 2, tile_crenel_gravel())
    paste_tile(1, 2, tile_crenel_cliff())
    paste_tile(2, 2, tile_climbable_wall())
    paste_tile(3, 2, tile_mineral_water())
    paste_tile(4, 2, tile_beanstalk())
    paste_tile(5, 2, tile_climbable_wall())
    paste_tile(6, 2, tile_crenel_cliff())
    paste_tile(7, 2, tile_crenel_gravel())

    # ROW 3: CASTOR WILDS SWAMP
    paste_tile(0, 3, tile_swamp_mud())
    paste_tile(1, 3, tile_swamp_water())
    paste_tile(2, 3, tile_swamp_grass())
    paste_tile(3, 3, tile_swamp_log())
    paste_tile(4, 3, tile_eye_statue(opened=False))
    paste_tile(5, 3, tile_eye_statue(opened=True))
    paste_tile(6, 3, tile_dirt_wall())
    paste_tile(7, 3, tile_dirt_wall())

    # ROW 4: HYRULE CASTLE COURTYARD
    paste_tile(0, 4, tile_castle_marble())
    paste_tile(1, 4, tile_castle_hedge())
    paste_tile(2, 4, tile_castle_wall())
    paste_tile(3, 4, tile_castle_battlement())
    paste_tile(4, 4, tile_castle_gate())
    paste_tile(5, 4, tile_castle_flower_royal())
    paste_tile(6, 4, tile_castle_triforce())
    paste_tile(7, 4, tile_castle_marble())

    return atlas

# -----------------------------------------------------------------------------
# Pre-rendered Regional Overworld Maps
# -----------------------------------------------------------------------------
def render_full_map_town():
    w, h = 36, 28
    pw, ph = w * 16, h * 16
    img = Image.new('RGBA', (pw, ph), (0, 0, 0, 255))
    collision = [0] * (w * h)

    t_cobble = tile_cobblestone()
    t_wall   = tile_stone_wall()
    t_water  = tile_water()
    t_dirt   = tile_dirt_path()
    t_r_red  = tile_roof_red()
    t_r_blue = tile_roof_blue()
    t_door   = tile_town_door()
    t_fount  = tile_fountain_edge()
    t_stall  = tile_market_stall()
    t_barrel = tile_barrel_crate()
    t_flower = tile_flower_red()
    t_bush   = tile_bush()

    # Fill base cobblestone
    for y in range(h):
        for x in range(w):
            img.paste(t_cobble, (x * 16, y * 16))

    # Outer fortress walls (North, South, East, West)
    for x in range(w):
        if not (16 <= x <= 19): # Castle gate
            img.paste(t_wall, (x * 16, 0))
            img.paste(t_wall, (x * 16, 16))
            collision[0 * w + x] = 1
            collision[1 * w + x] = 1
        if not (16 <= x <= 19): # South gate
            img.paste(t_wall, (x * 16, (h - 2) * 16))
            img.paste(t_wall, (x * 16, (h - 1) * 16))
            collision[(h - 2) * w + x] = 1
            collision[(h - 1) * w + x] = 1

    for y in range(h):
        img.paste(t_wall, (0, y * 16))
        img.paste(t_wall, ((w - 1) * 16, y * 16))
        collision[y * w + 0] = 1
        collision[y * w + (w - 1)] = 1

    # West ornamental water canal
    for y in range(1, h - 2):
        if y not in [8, 14]: # Stone bridges
            img.paste(t_water, (1 * 16, y * 16))
            collision[y * w + 1] = 1

    # Central Grand Fountain (x=16..19, y=12..15)
    for fy in range(12, 16):
        for fx in range(16, 20):
            img.paste(t_fount, (fx * 16, fy * 16))
            collision[fy * w + fx] = 1

    # Stockwell Shop (Blue roof) in North-West: x=4..9, y=3..7
    for by in range(3, 8):
        for bx in range(4, 10):
            tile = t_door if (by == 7 and bx == 6) else t_r_blue
            img.paste(tile, (bx * 16, by * 16))
            if not (by == 7 and bx == 6):
                collision[by * w + bx] = 1

    # Town Houses (Red roof) in North-East: x=24..29, y=3..7
    for by in range(3, 8):
        for bx in range(24, 30):
            tile = t_door if (by == 7 and bx == 27) else t_r_red
            img.paste(tile, (bx * 16, by * 16))
            if not (by == 7 and bx == 27):
                collision[by * w + bx] = 1

    # Market Stalls (South-West: x=5..8, y=18..20)
    for sy in range(18, 21):
        for sx in range(5, 9):
            img.paste(t_stall, (sx * 16, sy * 16))
            collision[sy * w + sx] = 1

    # Bakery / Tavern (South-East: x=24..29, y=17..21)
    for by in range(17, 22):
        for bx in range(24, 30):
            tile = t_door if (by == 21 and bx == 26) else t_r_red
            img.paste(tile, (bx * 16, by * 16))
            if not (by == 21 and bx == 26):
                collision[by * w + bx] = 1

    # Barrels & Crates
    for bx, by in [(10, 7), (10, 8), (23, 7), (4, 18), (9, 18)]:
        img.paste(t_barrel, (bx * 16, by * 16))
        collision[by * w + bx] = 1

    # Ornamental flowers & bushes
    for fx, fy in [(14, 12), (14, 15), (21, 12), (21, 15)]:
        img.paste(t_flower, (fx * 16, fy * 16))
    for bx, by in [(15, 11), (20, 11), (15, 16), (20, 16)]:
        img.paste(t_bush, (bx * 16, by * 16))
        collision[by * w + bx] = 1

    return img, w, h, collision

def render_full_map_crenel():
    w, h = 32, 24
    pw, ph = w * 16, h * 16
    img = Image.new('RGBA', (pw, ph), (0, 0, 0, 255))
    collision = [0] * (w * h)

    t_gravel = tile_crenel_gravel()
    t_cliff  = tile_crenel_cliff()
    t_climb  = tile_climbable_wall()
    t_water  = tile_mineral_water()
    t_bean   = tile_beanstalk()

    # Fill volcanic gravel
    for y in range(h):
        for x in range(w):
            img.paste(t_gravel, (x * 16, y * 16))

    # Northern & Western sheer cliff wall
    for x in range(w):
        for y in range(5):
            img.paste(t_cliff, (x * 16, y * 16))
            collision[y * w + x] = 1

    for y in range(h):
        for x in range(4):
            img.paste(t_cliff, (x * 16, y * 16))
            collision[y * w + x] = 1

    # Climbable Grip Ring section in cliff (x=12..15, y=0..4)
    for y in range(5):
        for x in range(12, 16):
            img.paste(t_climb, (x * 16, y * 16))
            collision[y * w + x] = 0 # Climbable with Grip Ring!

    # Thermal mineral water pool (x=20..25, y=10..14)
    for y in range(10, 15):
        for x in range(20, 26):
            img.paste(t_water, (x * 16, y * 16))
            collision[y * w + x] = 1

    # Magic beanstalk vine (x=8, y=5..15)
    for y in range(5, 16):
        img.paste(t_bean, (8 * 16, y * 16))

    # Border boundaries
    for y in range(h):
        collision[y * w + 0] = 1
        collision[y * w + (w - 1)] = 1
    for x in range(w):
        collision[(h - 1) * w + x] = 1

    return img, w, h, collision

def render_full_map_castor():
    w, h = 36, 28
    pw, ph = w * 16, h * 16
    img = Image.new('RGBA', (pw, ph), (0, 0, 0, 255))
    collision = [0] * (w * h)

    t_mud   = tile_swamp_mud()
    t_water = tile_swamp_water()
    t_grass = tile_swamp_grass()
    t_log   = tile_swamp_log()
    t_eye_c = tile_eye_statue(opened=False)
    t_eye_o = tile_eye_statue(opened=True)
    t_dirt  = tile_dirt_wall()

    # Fill base swamp grass
    for y in range(h):
        for x in range(w):
            img.paste(t_grass, (x * 16, y * 16))

    # Deep swamp mud pits (x=6..28, y=6..22)
    for y in range(6, 23):
        for x in range(6, 29):
            img.paste(t_mud, (x * 16, y * 16))

    # Dark swamp river channels
    for x in range(w):
        for y in range(2):
            img.paste(t_water, (x * 16, y * 16))
            collision[y * w + x] = 1

    # Hollow log bridges crossing the mud (x=16..20, y=14)
    for x in range(16, 21):
        img.paste(t_log, (x * 16, 14 * 16))

    # Ancient eye statues
    img.paste(t_eye_c, (10 * 16, 10 * 16))
    collision[10 * w + 10] = 1
    img.paste(t_eye_o, (24 * 16, 10 * 16))
    collision[10 * w + 24] = 1

    # Mole Mitts soft dirt wall blocking passage to Wind Ruins (x=30..35, y=10..18)
    for y in range(10, 19):
        for x in range(30, 36):
            img.paste(t_dirt, (x * 16, y * 16))
            collision[y * w + x] = 1

    # Boundary collisions
    for y in range(h):
        collision[y * w + 0] = 1
        collision[y * w + (w - 1)] = 1
    for x in range(w):
        collision[(h - 1) * w + x] = 1

    return img, w, h, collision

def render_full_map_castle_courtyard():
    w, h = 32, 24
    pw, ph = w * 16, h * 16
    img = Image.new('RGBA', (pw, ph), (0, 0, 0, 255))
    collision = [0] * (w * h)

    t_marble  = tile_castle_marble()
    t_hedge   = tile_castle_hedge()
    t_wall    = tile_castle_wall()
    t_battle  = tile_castle_battlement()
    t_gate    = tile_castle_gate()
    t_rose    = tile_castle_flower_royal()
    t_triforce= tile_castle_triforce()
    t_fount   = tile_fountain_edge()

    # Fill base royal marble
    for y in range(h):
        for x in range(w):
            img.paste(t_marble, (x * 16, y * 16))

    # Castle Fortress Wall along the North (Palace facade: y=0..3)
    for x in range(w):
        for y in range(3):
            tile = t_battle if y == 0 else t_wall
            if y == 2 and (14 <= x <= 17): # Royal palace entrance gate
                tile = t_gate
            img.paste(tile, (x * 16, y * 16))
            if not (y == 2 and 14 <= x <= 17):
                collision[y * w + x] = 1

    # Boundary fortress walls on East & West
    for y in range(h):
        img.paste(t_wall, (0, y * 16))
        img.paste(t_wall, ((w - 1) * 16, y * 16))
        collision[y * w + 0] = 1
        collision[y * w + (w - 1)] = 1

    # Royal Hedge Labyrinth (Topiary walls)
    # West wing hedge garden
    for y in range(6, 18):
        for x in [5, 9]:
            img.paste(t_hedge, (x * 16, y * 16))
            collision[y * w + x] = 1
    for x in range(5, 10):
        img.paste(t_hedge, (x * 16, 6 * 16))
        collision[6 * w + x] = 1

    # East wing hedge garden
    for y in range(6, 18):
        for x in [22, 26]:
            img.paste(t_hedge, (x * 16, y * 16))
            collision[y * w + x] = 1
    for x in range(22, 27):
        img.paste(t_hedge, (x * 16, 6 * 16))
        collision[6 * w + x] = 1

    # Sacred Triforce Mosaic in the grand courtyard center (x=15..16, y=10..11)
    for ty in range(10, 12):
        for tx in range(15, 17):
            img.paste(t_triforce, (tx * 16, ty * 16))

    # Royal Red Roses framing the central path
    for y in range(7, 19, 2):
        img.paste(t_rose, (13 * 16, y * 16))
        img.paste(t_rose, (18 * 16, y * 16))

    # Southern Castle Courtyard Portals (Connecting to North Hyrule Field)
    for x in range(w):
        if not (14 <= x <= 17): # Main gate open
            img.paste(t_wall, (x * 16, (h - 1) * 16))
            collision[(h - 1) * w + x] = 1

    return img, w, h, collision

# -----------------------------------------------------------------------------
# Main Execution Pipeline
# -----------------------------------------------------------------------------
def main():
    print("====================================================================")
    print("  Gerador de Tilesets de Overworld & Mapas Regionais Canônicos GBA  ")
    print("  The Legend of Zelda: The Minish Cap - Native Port (OpenMinish)    ")
    print("====================================================================\n")

    # 1. Gera Atlas Mestre de Tilesets de Overworld (256x256)
    print("[1/5] Gerando Atlas de Metatiles de Overworld (256x256)...")
    atlas = build_overworld_atlas()
    save_bmp_32('assets/regions/tileset_overworld_master.bmp', atlas)
    for reg in ['usa', 'eur', 'jpn']:
        save_bmp_32(f'assets/regions/{reg}/tileset_overworld.bmp', atlas)

    # 2. Gera Mapa e Colisão de Hyrule Town (576x448)
    print("\n[2/5] Gerando Mapa de Hyrule Town (576x448)...")
    m_town, tw_w, tw_h, col_town = render_full_map_town()
    save_bmp_32('assets/regions/map_town_master.bmp', m_town)
    save_binary_collision('assets/regions/map_town_collision_master.bin', tw_w, tw_h, col_town)
    for reg in ['usa', 'eur', 'jpn']:
        save_bmp_32(f'assets/regions/{reg}/map_town.bmp', m_town)
        save_binary_collision(f'assets/regions/{reg}/map_town_collision.bin', tw_w, tw_h, col_town)

    # 3. Gera Mapa e Colisão de Mount Crenel (512x384)
    print("\n[3/5] Gerando Mapa de Mount Crenel Base (512x384)...")
    m_crenel, cr_w, cr_h, col_crenel = render_full_map_crenel()
    save_bmp_32('assets/regions/map_crenel_master.bmp', m_crenel)
    save_binary_collision('assets/regions/map_crenel_collision_master.bin', cr_w, cr_h, col_crenel)
    for reg in ['usa', 'eur', 'jpn']:
        save_bmp_32(f'assets/regions/{reg}/map_crenel.bmp', m_crenel)
        save_binary_collision(f'assets/regions/{reg}/map_crenel_collision.bin', cr_w, cr_h, col_crenel)

    # 4. Gera Mapa e Colisão de Castor Wilds (576x448)
    print("\n[4/5] Gerando Mapa de Castor Wilds (576x448)...")
    m_castor, cs_w, cs_h, col_castor = render_full_map_castor()
    save_bmp_32('assets/regions/map_castor_master.bmp', m_castor)
    save_binary_collision('assets/regions/map_castor_collision_master.bin', cs_w, cs_h, col_castor)
    for reg in ['usa', 'eur', 'jpn']:
        save_bmp_32(f'assets/regions/{reg}/map_castor.bmp', m_castor)
        save_binary_collision(f'assets/regions/{reg}/map_castor_collision.bin', cs_w, cs_h, col_castor)

    # 5. Gera Mapa e Colisão de Hyrule Castle Courtyard (512x384)
    print("\n[5/5] Gerando Mapa de Hyrule Castle Courtyard (512x384)...")
    m_castle, cc_w, cc_h, col_castle = render_full_map_castle_courtyard()
    save_bmp_32('assets/regions/map_castle_courtyard_master.bmp', m_castle)
    save_binary_collision('assets/regions/map_castle_courtyard_collision_master.bin', cc_w, cc_h, col_castle)
    for reg in ['usa', 'eur', 'jpn']:
        save_bmp_32(f'assets/regions/{reg}/map_castle_courtyard.bmp', m_castle)
        save_binary_collision(f'assets/regions/{reg}/map_castle_courtyard_collision.bin', cc_w, cc_h, col_castle)

    print("\n>>> Pipeline de Overworld & Tilesets Regionais concluído com sucesso!")

if __name__ == '__main__':
    main()
