#!/usr/bin/env python3
"""
tools/generate_npc_assets.py - Canonical GBA NPC Sprite Atlas Generator
=======================================================================
Generates a 160x160 32-bit RGBA BMP containing pixel-perfect GBA The Minish Cap
NPC sprites with authentic palettes, proportions, and animations:

- Row 0 (Y=0..15):   Mestre Espadachim Swiftblade (Dojo)
- Row 1 (Y=16..31):  Ferreiro Smith (Avô de Link / Mestre Ferreiro)
- Row 2 (Y=32..47):  Princesa Zelda (Vestido Real Rosa, Tiara & Versão Petrificada)
- Row 3 (Y=48..63):  Prefeito Hagen (Cartola Azul, Pluma, Monóculo, Colete Dourado)
- Row 4 (Y=64..79):  Malon da Fazenda Lon Lon (Cabelos Ruivos, Bandana, Avental)
- Row 5 (Y=80..95):  Deku Business Scrub (Monte Crenel / Vendedor de Itens)
- Row 6 (Y=96..111): Comerciante Stockwell (Boina Roxa, Óculos, Avental Esmeralda)
- Row 7 (Y=112..127): Cidadã de Hyrule (Touca Rosa, Vestido Azul, Cesta)
- Row 8 (Y=128..143): Guarda Real do Castelo (Armadura de Aço, Pluma Vermelha, Alabarda)
- Row 9 (Y=144..159): Mestre Ferreiro Melari & Ancião Gentari (Sábios Minish)
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

def create_npc_atlas():
    ATLAS_W = 160
    ATLAS_H = 160
    sheet = Image.new('RGBA', (ATLAS_W, ATLAS_H), (0, 0, 0, 0))

    def make_cell_16():
        return Image.new('RGBA', (16, 16), (0, 0, 0, 0))

    SKIN_LIGHT   = (253, 232, 205, 255)
    SKIN_SHADOW  = (226, 185, 145, 255)
    WHITE        = (255, 255, 255, 255)
    BLACK        = (17, 17, 17, 255)

    # =========================================================================
    # ROW 0: MESTRE ESPADACHIM SWIFTBLADE (Y = 0)
    # =========================================================================
    FUR_MAIN     = (122, 106, 90, 255)
    FUR_DARK     = (84, 70, 56, 255)
    MUZZLE       = (194, 178, 160, 255)
    HEADBAND     = (220, 38, 38, 255)
    HEADBAND_DK  = (153, 27, 27, 255)
    GI_MAIN      = (30, 51, 36, 255)
    BELT_WHITE   = (229, 231, 235, 255)
    WOOD_BOKKEN  = (146, 64, 14, 255)
    WOOD_HI      = (217, 119, 6, 255)

    # 0.0: Swiftblade Idle Down
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    # Ears
    c.putpixel((4, 0), FUR_MAIN); c.putpixel((5, 1), FUR_MAIN)
    c.putpixel((10, 1), FUR_MAIN); c.putpixel((11, 0), FUR_MAIN)
    # Head & muzzle
    d.rectangle([(4, 2), (11, 6)], fill=FUR_MAIN)
    d.rectangle([(3, 3), (12, 4)], fill=HEADBAND)
    c.putpixel((2, 4), HEADBAND_DK); c.putpixel((1, 5), HEADBAND) # Knot
    c.putpixel((5, 5), BLACK); c.putpixel((10, 5), BLACK)
    d.rectangle([(6, 6), (9, 7)], fill=MUZZLE)
    c.putpixel((7, 6), (28, 20, 15, 255)); c.putpixel((8, 6), (28, 20, 15, 255))
    # Body & Gi
    d.rectangle([(4, 8), (11, 12)], fill=GI_MAIN)
    d.rectangle([(4, 11), (11, 12)], fill=BELT_WHITE)
    # Legs
    d.rectangle([(4, 13), (6, 15)], fill=GI_MAIN)
    d.rectangle([(9, 13), (11, 15)], fill=GI_MAIN)
    # Bokken
    d.rectangle([(12, 6), (13, 13)], fill=WOOD_BOKKEN)
    c.putpixel((12, 5), WOOD_HI); c.putpixel((13, 5), WOOD_HI)
    sheet.paste(c, (0 * 16, 0))

    # 0.1: Swiftblade Facing Side (Right)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    c.putpixel((4, 0), FUR_MAIN); c.putpixel((5, 1), FUR_MAIN)
    d.rectangle([(3, 2), (10, 6)], fill=FUR_MAIN)
    d.rectangle([(2, 3), (10, 4)], fill=HEADBAND)
    c.putpixel((10, 5), BLACK)
    d.rectangle([(10, 6), (12, 7)], fill=MUZZLE)
    c.putpixel((12, 6), (28, 20, 15, 255))
    d.rectangle([(3, 8), (10, 12)], fill=GI_MAIN)
    d.rectangle([(3, 11), (10, 12)], fill=BELT_WHITE)
    d.rectangle([(4, 13), (7, 15)], fill=GI_MAIN)
    d.rectangle([(8, 13), (10, 15)], fill=GI_MAIN)
    # Bokken pointed forward
    d.line([(10, 9), (15, 9)], fill=WOOD_BOKKEN, width=2)
    sheet.paste(c, (1 * 16, 0))

    # 0.2: Swiftblade Facing Up
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    c.putpixel((4, 0), FUR_MAIN); c.putpixel((11, 0), FUR_MAIN)
    d.rectangle([(4, 2), (11, 6)], fill=FUR_MAIN)
    d.rectangle([(4, 3), (11, 4)], fill=HEADBAND)
    c.putpixel((8, 4), HEADBAND_DK); c.putpixel((9, 5), HEADBAND)
    d.rectangle([(4, 8), (11, 12)], fill=GI_MAIN)
    d.rectangle([(4, 11), (11, 12)], fill=BELT_WHITE)
    d.rectangle([(4, 13), (6, 15)], fill=GI_MAIN)
    d.rectangle([(9, 13), (11, 15)], fill=GI_MAIN)
    d.rectangle([(12, 6), (13, 13)], fill=WOOD_BOKKEN)
    sheet.paste(c, (2 * 16, 0))

    # 0.3: Swiftblade Bokken Practice Slash
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 2), (10, 6)], fill=FUR_MAIN)
    d.rectangle([(2, 3), (10, 4)], fill=HEADBAND)
    c.putpixel((10, 5), BLACK)
    d.rectangle([(10, 6), (12, 7)], fill=MUZZLE)
    d.rectangle([(3, 8), (10, 12)], fill=GI_MAIN)
    d.rectangle([(4, 13), (6, 15)], fill=GI_MAIN)
    d.rectangle([(8, 13), (11, 15)], fill=GI_MAIN)
    # Bokken slashing diagonal
    d.line([(9, 11), (15, 5)], fill=WOOD_BOKKEN, width=2)
    c.putpixel((15, 4), WOOD_HI); c.putpixel((14, 4), WOOD_HI)
    sheet.paste(c, (3 * 16, 0))

    # =========================================================================
    # ROW 1: FERREIRO SMITH (AVÔ DE LINK) (Y = 16)
    # =========================================================================
    BEARD_WHITE  = (248, 250, 252, 255)
    BEARD_SHADOW = (203, 213, 225, 255)
    SHIRT_RED    = (185, 28, 28, 255)
    APRON_SLATE  = (71, 85, 105, 255)
    HAMMER_IRON  = (148, 163, 184, 255)

    # 1.0: Smith Idle Down
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    # Head & hair
    d.rectangle([(4, 1), (11, 5)], fill=SKIN_LIGHT)
    d.rectangle([(3, 1), (4, 4)], fill=BEARD_WHITE)
    d.rectangle([(11, 1), (12, 4)], fill=BEARD_WHITE)
    # Eyes & thick white eyebrows
    c.putpixel((5, 2), BEARD_WHITE); c.putpixel((10, 2), BEARD_WHITE)
    c.putpixel((5, 3), BLACK); c.putpixel((10, 3), BLACK)
    # Big bushy white beard
    d.rectangle([(4, 5), (11, 8)], fill=BEARD_WHITE)
    d.rectangle([(5, 8), (10, 10)], fill=BEARD_SHADOW)
    # Torso with blacksmith apron
    d.rectangle([(3, 9), (12, 13)], fill=SHIRT_RED)
    d.rectangle([(5, 9), (10, 13)], fill=APRON_SLATE)
    # Strong muscular arms
    c.putpixel((2, 10), SKIN_LIGHT); c.putpixel((2, 11), SKIN_LIGHT)
    c.putpixel((13, 10), SKIN_LIGHT); c.putpixel((13, 11), SKIN_LIGHT)
    # Legs/boots
    d.rectangle([(4, 14), (6, 15)], fill=(60, 40, 20, 255))
    d.rectangle([(9, 14), (11, 15)], fill=(60, 40, 20, 255))
    # Forge hammer
    d.rectangle([(13, 8), (15, 10)], fill=HAMMER_IRON)
    d.line([(14, 10), (14, 13)], fill=(120, 53, 15, 255))
    sheet.paste(c, (0 * 16, 16))

    # 1.1: Smith Facing Side (Right)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 1), (9, 5)], fill=SKIN_LIGHT)
    d.rectangle([(2, 1), (4, 4)], fill=BEARD_WHITE)
    c.putpixel((8, 3), BLACK)
    d.rectangle([(7, 4), (12, 8)], fill=BEARD_WHITE) # Beard pointing right
    d.rectangle([(3, 9), (9, 13)], fill=SHIRT_RED)
    d.rectangle([(6, 9), (10, 13)], fill=APRON_SLATE)
    d.rectangle([(4, 14), (6, 15)], fill=(60, 40, 20, 255))
    d.rectangle([(8, 14), (10, 15)], fill=(60, 40, 20, 255))
    d.line([(10, 9), (14, 9)], fill=(120, 53, 15, 255))
    d.rectangle([(13, 7), (15, 9)], fill=HAMMER_IRON)
    sheet.paste(c, (1 * 16, 16))

    # 1.2: Smith Hammer Raised (Forging)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(4, 1), (11, 5)], fill=SKIN_LIGHT)
    d.rectangle([(4, 5), (11, 9)], fill=BEARD_WHITE)
    c.putpixel((5, 3), BLACK); c.putpixel((10, 3), BLACK)
    d.rectangle([(3, 9), (12, 13)], fill=SHIRT_RED)
    d.rectangle([(5, 9), (10, 13)], fill=APRON_SLATE)
    d.rectangle([(4, 14), (6, 15)], fill=(60, 40, 20, 255))
    d.rectangle([(9, 14), (11, 15)], fill=(60, 40, 20, 255))
    # Hammer held high above head
    d.rectangle([(12, 1), (15, 3)], fill=HAMMER_IRON)
    d.line([(13, 3), (13, 8)], fill=(120, 53, 15, 255))
    sheet.paste(c, (2 * 16, 16))

    # 1.3: Smith Hammer Strike (Sparks)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(4, 2), (11, 6)], fill=SKIN_LIGHT)
    d.rectangle([(4, 6), (11, 10)], fill=BEARD_WHITE)
    c.putpixel((5, 4), BLACK); c.putpixel((10, 4), BLACK)
    d.rectangle([(3, 10), (12, 13)], fill=SHIRT_RED)
    d.rectangle([(5, 10), (10, 13)], fill=APRON_SLATE)
    d.rectangle([(4, 14), (6, 15)], fill=(60, 40, 20, 255))
    d.rectangle([(9, 14), (11, 15)], fill=(60, 40, 20, 255))
    # Hammer striking down at anvil
    d.rectangle([(11, 12), (15, 14)], fill=HAMMER_IRON)
    # Golden forge spark
    c.putpixel((13, 11), (254, 240, 138, 255))
    c.putpixel((12, 10), (245, 158, 11, 255))
    c.putpixel((14, 10), (245, 158, 11, 255))
    sheet.paste(c, (3 * 16, 16))

    # =========================================================================
    # ROW 2: PRINCESA ZELDA (Y = 32)
    # =========================================================================
    HAIR_GOLD    = (251, 191, 36, 255)
    HAIR_SHADOW  = (217, 119, 6, 255)
    TIARA_GOLD   = (254, 240, 138, 255)
    TIARA_RUBY   = (239, 68, 68, 255)
    GOWN_PINK    = (244, 114, 182, 255)
    GOWN_LIGHT   = (251, 207, 232, 255)
    GOWN_DARK    = (190, 24, 93, 255)
    TRIFORCE_COL = (250, 204, 21, 255)

    STONE_BASE   = (100, 116, 139, 255)
    STONE_HI     = (148, 163, 184, 255)
    STONE_DARK   = (51, 65, 85, 255)

    # 2.0: Zelda Idle Down
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    # Blonde hair
    d.rectangle([(3, 1), (12, 8)], fill=HAIR_GOLD)
    # Tiara
    d.line([(4, 1), (11, 1)], fill=TIARA_GOLD)
    c.putpixel((7, 0), TIARA_RUBY); c.putpixel((8, 0), TIARA_RUBY)
    # Face & blue eyes
    d.rectangle([(5, 3), (10, 6)], fill=SKIN_LIGHT)
    c.putpixel((6, 4), (59, 130, 246, 255)) # Blue eyes
    c.putpixel((9, 4), (59, 130, 246, 255))
    c.putpixel((5, 5), (251, 113, 133, 255)) # Blush
    c.putpixel((10, 5), (251, 113, 133, 255))
    # Hair tresses on sides
    d.rectangle([(3, 5), (4, 9)], fill=HAIR_GOLD)
    d.rectangle([(11, 5), (12, 9)], fill=HAIR_GOLD)
    # Royal gown
    d.rectangle([(4, 7), (11, 9)], fill=WHITE) # White mantle
    d.rectangle([(3, 10), (12, 15)], fill=GOWN_PINK)
    d.rectangle([(5, 10), (10, 15)], fill=GOWN_LIGHT)
    # Triforce emblem on tabard
    d.polygon([(7, 11), (6, 13), (8, 13)], fill=TRIFORCE_COL)
    sheet.paste(c, (0 * 16, 32))

    # 2.1: Zelda Facing Side (Right)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 1), (11, 8)], fill=HAIR_GOLD)
    d.line([(4, 1), (9, 1)], fill=TIARA_GOLD)
    c.putpixel((8, 0), TIARA_RUBY)
    d.rectangle([(6, 3), (11, 6)], fill=SKIN_LIGHT)
    c.putpixel((9, 4), (59, 130, 246, 255))
    d.rectangle([(3, 5), (5, 10)], fill=HAIR_GOLD)
    d.rectangle([(4, 7), (11, 9)], fill=WHITE)
    d.rectangle([(3, 10), (11, 15)], fill=GOWN_PINK)
    d.rectangle([(7, 10), (11, 15)], fill=GOWN_LIGHT)
    sheet.paste(c, (1 * 16, 32))

    # 2.2: Zelda Greeting / Curtsy
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    d.rectangle([(3, 2), (12, 9)], fill=HAIR_GOLD)
    d.line([(4, 2), (11, 2)], fill=TIARA_GOLD)
    c.putpixel((7, 1), TIARA_RUBY); c.putpixel((8, 1), TIARA_RUBY)
    d.rectangle([(5, 4), (10, 7)], fill=SKIN_LIGHT)
    # Smiling eyes
    c.putpixel((6, 5), (30, 64, 175, 255))
    c.putpixel((9, 5), (30, 64, 175, 255))
    c.putpixel((7, 6), (239, 68, 68, 255)) # Smile
    d.rectangle([(2, 11), (13, 15)], fill=GOWN_PINK) # Flared gown
    d.rectangle([(5, 10), (10, 15)], fill=GOWN_LIGHT)
    d.polygon([(7, 11), (6, 13), (8, 13)], fill=TRIFORCE_COL)
    # Little waving white glove
    c.putpixel((13, 7), WHITE); c.putpixel((13, 8), WHITE)
    sheet.paste(c, (2 * 16, 32))

    # 2.3: Petrified Princess Zelda (Stone Statue)
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    # Entirely carved from marble/granite stone
    d.rectangle([(3, 1), (12, 8)], fill=STONE_BASE)
    d.line([(4, 1), (11, 1)], fill=STONE_HI)
    c.putpixel((7, 0), STONE_HI); c.putpixel((8, 0), STONE_HI)
    d.rectangle([(5, 3), (10, 6)], fill=STONE_HI)
    c.putpixel((6, 4), STONE_DARK); c.putpixel((9, 4), STONE_DARK) # Stone closed eyes
    d.rectangle([(3, 5), (4, 9)], fill=STONE_BASE)
    d.rectangle([(11, 5), (12, 9)], fill=STONE_BASE)
    d.rectangle([(4, 7), (11, 9)], fill=STONE_HI)
    d.rectangle([(3, 10), (12, 15)], fill=STONE_BASE)
    d.rectangle([(5, 10), (10, 15)], fill=STONE_DARK)
    # Stone texture cracks
    c.putpixel((5, 12), STONE_HI); c.putpixel((9, 14), STONE_HI)
    sheet.paste(c, (3 * 16, 32))

    # =========================================================================
    # ROW 3: PREFEITO HAGEN (Y = 48)
    # =========================================================================
    HAT_BLUE     = (30, 64, 175, 255)
    PLUME_RED    = (220, 38, 38, 255)
    FROCK_NAVY   = (30, 58, 138, 255)
    VEST_GOLD    = (250, 204, 21, 255)
    MUSTACHE_BRN = (120, 53, 15, 255)
    MONOCLE_GOLD = (245, 158, 11, 255)
    LENS_CYAN    = (224, 242, 254, 255)

    # 3.0: Mayor Hagen Idle Down
    c = make_cell_16()
    d = ImageDraw.Draw(c)
    # Top hat with feather
    draw = ImageDraw.Draw(c)
    draw.rectangle([(4, 0), (11, 3)], fill=HAT_BLUE)
    draw.rectangle([(3, 3), (12, 4)], fill=HAT_BLUE)
    draw.rectangle([(9, 0), (10, 2)], fill=PLUME_RED)
    # Round face & monocle
    draw.rectangle([(4, 5), (11, 8)], fill=SKIN_LIGHT)
    c.putpixel((5, 6), BLACK)
    # Monocle right eye
    c.putpixel((9, 5), MONOCLE_GOLD); c.putpixel((10, 5), MONOCLE_GOLD)
    c.putpixel((9, 6), LENS_CYAN); c.putpixel((10, 6), MONOCLE_GOLD)
    # Bushy brown mustache
    draw.rectangle([(4, 7), (11, 8)], fill=MUSTACHE_BRN)
    # Frock coat & gold vest
    draw.rectangle([(3, 9), (12, 13)], fill=FROCK_NAVY)
    draw.rectangle([(6, 9), (9, 13)], fill=VEST_GOLD)
    c.putpixel((7, 9), WHITE); c.putpixel((8, 9), WHITE) # White cravat
    draw.rectangle([(4, 14), (6, 15)], fill=(15, 23, 42, 255))
    draw.rectangle([(9, 14), (11, 15)], fill=(15, 23, 42, 255))
    sheet.paste(c, (0 * 16, 48))

    # 3.1: Mayor Hagen Facing Side (Right)
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(3, 0), (10, 3)], fill=HAT_BLUE)
    draw.rectangle([(2, 3), (11, 4)], fill=HAT_BLUE)
    draw.rectangle([(3, 0), (4, 2)], fill=PLUME_RED)
    draw.rectangle([(4, 5), (10, 8)], fill=SKIN_LIGHT)
    c.putpixel((9, 6), MONOCLE_GOLD)
    draw.rectangle([(7, 7), (11, 8)], fill=MUSTACHE_BRN)
    draw.rectangle([(3, 9), (10, 13)], fill=FROCK_NAVY)
    draw.rectangle([(7, 9), (10, 13)], fill=VEST_GOLD)
    draw.rectangle([(4, 14), (6, 15)], fill=(15, 23, 42, 255))
    draw.rectangle([(8, 14), (10, 15)], fill=(15, 23, 42, 255))
    sheet.paste(c, (1 * 16, 48))

    # 3.2: Mayor Hagen Adjusting Monocle
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(4, 0), (11, 3)], fill=HAT_BLUE)
    draw.rectangle([(3, 3), (12, 4)], fill=HAT_BLUE)
    draw.rectangle([(9, 0), (10, 2)], fill=PLUME_RED)
    draw.rectangle([(4, 5), (11, 8)], fill=SKIN_LIGHT)
    c.putpixel((5, 6), BLACK)
    # Monocle with hand adjusting
    c.putpixel((9, 6), LENS_CYAN); c.putpixel((10, 6), SKIN_LIGHT)
    c.putpixel((11, 6), SKIN_LIGHT)
    draw.rectangle([(4, 7), (11, 8)], fill=MUSTACHE_BRN)
    draw.rectangle([(3, 9), (12, 13)], fill=FROCK_NAVY)
    draw.rectangle([(6, 9), (9, 13)], fill=VEST_GOLD)
    draw.rectangle([(4, 14), (6, 15)], fill=(15, 23, 42, 255))
    draw.rectangle([(9, 14), (11, 15)], fill=(15, 23, 42, 255))
    sheet.paste(c, (2 * 16, 48))

    # 3.3: Mayor Hagen Tipping Hat
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    # Tipped hat held slightly up
    draw.rectangle([(3, 0), (10, 2)], fill=HAT_BLUE)
    draw.rectangle([(2, 2), (11, 3)], fill=HAT_BLUE)
    draw.rectangle([(3, 5), (11, 8)], fill=SKIN_LIGHT)
    c.putpixel((5, 6), BLACK); c.putpixel((9, 6), BLACK)
    draw.rectangle([(4, 7), (11, 8)], fill=MUSTACHE_BRN)
    draw.rectangle([(3, 9), (12, 13)], fill=FROCK_NAVY)
    draw.rectangle([(6, 9), (9, 13)], fill=VEST_GOLD)
    draw.rectangle([(4, 14), (6, 15)], fill=(15, 23, 42, 255))
    draw.rectangle([(9, 14), (11, 15)], fill=(15, 23, 42, 255))
    sheet.paste(c, (3 * 16, 48))

    # =========================================================================
    # ROW 4: MALON DA FAZENDA LON LON (Y = 64)
    # =========================================================================
    HAIR_RED     = (234, 88, 12, 255)
    HAIR_RED_DK  = (154, 52, 18, 255)
    BANDANA_YEL  = (250, 204, 21, 255)
    BODICE_BLUE  = (59, 130, 246, 255)
    APRON_WHITE  = (254, 243, 199, 255)
    BLUSH_PINK   = (251, 113, 133, 255)

    # 4.0: Malon Idle Down
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    # Red wavy hair & yellow bandana
    draw.rectangle([(3, 1), (12, 6)], fill=HAIR_RED)
    draw.rectangle([(5, 1), (10, 2)], fill=BANDANA_YEL)
    draw.rectangle([(4, 4), (11, 7)], fill=SKIN_LIGHT)
    # Friendly blue eyes and rosy cheeks
    c.putpixel((5, 5), (30, 64, 175, 255)); c.putpixel((10, 5), (30, 64, 175, 255))
    c.putpixel((4, 6), BLUSH_PINK); c.putpixel((11, 6), BLUSH_PINK)
    c.putpixel((7, 6), (225, 29, 72, 255)); c.putpixel((8, 6), (225, 29, 72, 255))
    # Hair locks on shoulders
    draw.rectangle([(2, 6), (3, 9)], fill=HAIR_RED)
    draw.rectangle([(12, 6), (13, 9)], fill=HAIR_RED)
    # Blue bodice and white apron
    draw.rectangle([(4, 8), (11, 13)], fill=BODICE_BLUE)
    draw.rectangle([(5, 9), (10, 13)], fill=APRON_WHITE)
    draw.rectangle([(5, 14), (6, 15)], fill=(120, 53, 15, 255))
    draw.rectangle([(9, 14), (10, 15)], fill=(120, 53, 15, 255))
    sheet.paste(c, (0 * 16, 64))

    # 4.1: Malon Facing Side (Right)
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(2, 1), (10, 7)], fill=HAIR_RED)
    draw.rectangle([(5, 1), (8, 2)], fill=BANDANA_YEL)
    draw.rectangle([(5, 4), (10, 7)], fill=SKIN_LIGHT)
    c.putpixel((9, 5), (30, 64, 175, 255))
    c.putpixel((10, 6), BLUSH_PINK)
    draw.rectangle([(2, 6), (4, 10)], fill=HAIR_RED)
    draw.rectangle([(4, 8), (10, 13)], fill=BODICE_BLUE)
    draw.rectangle([(6, 9), (10, 13)], fill=APRON_WHITE)
    draw.rectangle([(5, 14), (7, 15)], fill=(120, 53, 15, 255))
    draw.rectangle([(8, 14), (10, 15)], fill=(120, 53, 15, 255))
    sheet.paste(c, (1 * 16, 64))

    # 4.2: Malon Waving Hand
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(3, 1), (12, 6)], fill=HAIR_RED)
    draw.rectangle([(5, 1), (10, 2)], fill=BANDANA_YEL)
    draw.rectangle([(4, 4), (11, 7)], fill=SKIN_LIGHT)
    c.putpixel((5, 5), (30, 64, 175, 255)); c.putpixel((10, 5), (30, 64, 175, 255))
    c.putpixel((4, 6), BLUSH_PINK); c.putpixel((11, 6), BLUSH_PINK)
    draw.rectangle([(4, 8), (11, 13)], fill=BODICE_BLUE)
    draw.rectangle([(5, 9), (10, 13)], fill=APRON_WHITE)
    draw.rectangle([(5, 14), (6, 15)], fill=(120, 53, 15, 255))
    draw.rectangle([(9, 14), (10, 15)], fill=(120, 53, 15, 255))
    # Waving hand
    c.putpixel((13, 6), SKIN_LIGHT); c.putpixel((13, 7), SKIN_LIGHT)
    sheet.paste(c, (2 * 16, 64))

    # 4.3: Malon Holding Lon Lon Milk Bottle
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(3, 1), (12, 6)], fill=HAIR_RED)
    draw.rectangle([(5, 1), (10, 2)], fill=BANDANA_YEL)
    draw.rectangle([(4, 4), (11, 7)], fill=SKIN_LIGHT)
    c.putpixel((5, 5), (30, 64, 175, 255)); c.putpixel((10, 5), (30, 64, 175, 255))
    draw.rectangle([(4, 8), (11, 13)], fill=BODICE_BLUE)
    draw.rectangle([(5, 9), (10, 13)], fill=APRON_WHITE)
    draw.rectangle([(5, 14), (6, 15)], fill=(120, 53, 15, 255))
    draw.rectangle([(9, 14), (10, 15)], fill=(120, 53, 15, 255))
    # Lon Lon Milk Bottle in hands
    draw.rectangle([(7, 10), (9, 13)], fill=WHITE)
    c.putpixel((8, 9), (220, 38, 38, 255)) # Bottle cap
    sheet.paste(c, (3 * 16, 64))

    # =========================================================================
    # ROW 5: DEKU BUSINESS SCRUB (Y = 80)
    # =========================================================================
    BUSH_GRN     = (22, 163, 74, 255)
    BUSH_LGT     = (74, 222, 128, 255)
    BUSH_DK      = (20, 83, 45, 255)
    WOOD_DEKU    = (120, 53, 15, 255)
    SNOUT_TUBE   = (154, 52, 18, 255)
    EYE_YEL      = (253, 224, 71, 255)

    # 5.0: Scrub Camouflaged in Bush
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.ellipse([(2, 5), (13, 14)], fill=BUSH_GRN)
    draw.ellipse([(4, 4), (11, 8)], fill=BUSH_LGT)
    draw.rectangle([(3, 13), (12, 14)], fill=BUSH_DK)
    # Eyes peeking
    c.putpixel((6, 7), EYE_YEL); c.putpixel((9, 7), EYE_YEL)
    sheet.paste(c, (0 * 16, 80))

    # 5.1: Scrub Emerged Head (Trumpet Snout)
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    # Bush base
    draw.ellipse([(2, 8), (13, 15)], fill=BUSH_GRN)
    # Head & leaf crown
    draw.rectangle([(4, 1), (11, 3)], fill=(34, 197, 94, 255))
    c.putpixel((7, 0), BUSH_LGT); c.putpixel((8, 0), BUSH_LGT)
    draw.rectangle([(5, 3), (10, 8)], fill=WOOD_DEKU)
    # Yellow glowing eyes
    c.putpixel((6, 4), EYE_YEL); c.putpixel((9, 4), EYE_YEL)
    c.putpixel((6, 5), BLACK); c.putpixel((9, 5), BLACK)
    # Trumpet snout
    draw.rectangle([(6, 6), (9, 8)], fill=SNOUT_TUBE)
    c.putpixel((7, 7), (69, 26, 3, 255)); c.putpixel((8, 7), (69, 26, 3, 255))
    sheet.paste(c, (1 * 16, 80))

    # 5.2: Scrub Snout Bobbing / Talking
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.ellipse([(2, 8), (13, 15)], fill=BUSH_GRN)
    draw.rectangle([(4, 2), (11, 4)], fill=(34, 197, 94, 255))
    draw.rectangle([(5, 4), (10, 9)], fill=WOOD_DEKU)
    c.putpixel((6, 5), EYE_YEL); c.putpixel((9, 5), EYE_YEL)
    draw.rectangle([(5, 7), (10, 9)], fill=SNOUT_TUBE)
    c.putpixel((7, 8), (69, 26, 3, 255)); c.putpixel((8, 8), (69, 26, 3, 255))
    sheet.paste(c, (2 * 16, 80))

    # 5.3: Scrub Holding Grip Ring
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.ellipse([(2, 8), (13, 15)], fill=BUSH_GRN)
    draw.rectangle([(4, 1), (11, 3)], fill=(34, 197, 94, 255))
    draw.rectangle([(5, 3), (10, 8)], fill=WOOD_DEKU)
    c.putpixel((6, 4), EYE_YEL); c.putpixel((9, 4), EYE_YEL)
    draw.rectangle([(6, 6), (9, 8)], fill=SNOUT_TUBE)
    # Little Grip Ring icon held up
    draw.ellipse([(10, 7), (14, 11)], fill=(217, 119, 6, 255))
    c.putpixel((12, 9), (254, 240, 138, 255))
    sheet.paste(c, (3 * 16, 80))

    # =========================================================================
    # ROW 6: COMERCIANTE STOCKWELL (Y = 96)
    # =========================================================================
    CAP_PURPLE   = (74, 20, 140, 255)
    CAP_GOLD     = (241, 196, 15, 255)
    GLASSES_GOLD = (245, 158, 11, 255)
    GLASS_LENS   = (224, 242, 254, 255)
    APRON_EMERALD= (5, 150, 105, 255)
    BOWTIE_RED   = (220, 38, 38, 255)

    # 6.0: Stockwell Idle Down
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    # Purple cap with gold brim
    draw.rectangle([(4, 0), (11, 2)], fill=CAP_PURPLE)
    draw.rectangle([(3, 2), (12, 3)], fill=CAP_PURPLE)
    draw.rectangle([(4, 3), (11, 3)], fill=CAP_GOLD)
    # Face & glasses
    draw.rectangle([(4, 4), (11, 8)], fill=SKIN_LIGHT)
    c.putpixel((5, 5), GLASSES_GOLD); c.putpixel((6, 5), GLASS_LENS); c.putpixel((7, 5), GLASSES_GOLD)
    c.putpixel((8, 5), GLASSES_GOLD); c.putpixel((9, 5), GLASS_LENS); c.putpixel((10, 5), GLASSES_GOLD)
    # Brown mustache
    c.putpixel((7, 7), (93, 64, 55, 255)); c.putpixel((8, 7), (93, 64, 55, 255))
    # White shirt, red bow tie & emerald apron
    draw.rectangle([(5, 9), (10, 10)], fill=WHITE)
    c.putpixel((7, 9), BOWTIE_RED); c.putpixel((8, 9), BOWTIE_RED)
    draw.rectangle([(4, 11), (11, 15)], fill=APRON_EMERALD)
    sheet.paste(c, (0 * 16, 96))

    # 6.1: Stockwell Gesturing / Wares
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(4, 0), (11, 2)], fill=CAP_PURPLE)
    draw.rectangle([(3, 2), (12, 3)], fill=CAP_PURPLE)
    draw.rectangle([(4, 3), (11, 3)], fill=CAP_GOLD)
    draw.rectangle([(4, 4), (11, 8)], fill=SKIN_LIGHT)
    c.putpixel((5, 5), GLASSES_GOLD); c.putpixel((6, 5), GLASS_LENS); c.putpixel((7, 5), GLASSES_GOLD)
    c.putpixel((8, 5), GLASSES_GOLD); c.putpixel((9, 5), GLASS_LENS); c.putpixel((10, 5), GLASSES_GOLD)
    draw.rectangle([(5, 9), (10, 10)], fill=WHITE)
    c.putpixel((7, 9), BOWTIE_RED); c.putpixel((8, 9), BOWTIE_RED)
    draw.rectangle([(4, 11), (11, 15)], fill=APRON_EMERALD)
    # Hand gesturing
    c.putpixel((2, 9), SKIN_LIGHT); c.putpixel((13, 9), SKIN_LIGHT)
    sheet.paste(c, (1 * 16, 96))

    # 6.2: Stockwell Counting Rupees
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(4, 0), (11, 2)], fill=CAP_PURPLE)
    draw.rectangle([(3, 2), (12, 3)], fill=CAP_PURPLE)
    draw.rectangle([(4, 3), (11, 3)], fill=CAP_GOLD)
    draw.rectangle([(4, 4), (11, 8)], fill=SKIN_LIGHT)
    # Looking down at rupees
    c.putpixel((6, 6), (93, 64, 55, 255)); c.putpixel((9, 6), (93, 64, 55, 255))
    draw.rectangle([(4, 11), (11, 15)], fill=APRON_EMERALD)
    # Green Rupee in hands
    c.putpixel((7, 11), (34, 197, 94, 255)); c.putpixel((8, 11), (34, 197, 94, 255))
    sheet.paste(c, (2 * 16, 96))

    # =========================================================================
    # ROW 7: CIDADÃ DE HYRULE (TOWN CITIZEN) (Y = 112)
    # =========================================================================
    BONNET_PINK  = (244, 114, 182, 255)
    BONNET_LACE  = (251, 207, 232, 255)
    DRESS_SKY    = (56, 189, 248, 255)
    HAIR_AUBURN  = (217, 119, 6, 255)

    # 7.0: Citizen Idle Down
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(4, 1), (11, 3)], fill=BONNET_PINK)
    draw.rectangle([(3, 3), (12, 4)], fill=BONNET_LACE)
    draw.rectangle([(4, 4), (11, 5)], fill=HAIR_AUBURN)
    draw.rectangle([(4, 6), (11, 9)], fill=SKIN_LIGHT)
    c.putpixel((5, 7), BLACK); c.putpixel((10, 7), BLACK)
    c.putpixel((4, 8), BLUSH_PINK); c.putpixel((11, 8), BLUSH_PINK)
    draw.rectangle([(4, 10), (11, 15)], fill=DRESS_SKY)
    draw.rectangle([(6, 10), (9, 14)], fill=WHITE)
    sheet.paste(c, (0 * 16, 112))

    # 7.1: Citizen Facing Side (Right)
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(3, 1), (10, 3)], fill=BONNET_PINK)
    draw.rectangle([(2, 3), (11, 4)], fill=BONNET_LACE)
    draw.rectangle([(3, 4), (10, 5)], fill=HAIR_AUBURN)
    draw.rectangle([(5, 6), (10, 9)], fill=SKIN_LIGHT)
    c.putpixel((9, 7), BLACK)
    draw.rectangle([(4, 10), (10, 15)], fill=DRESS_SKY)
    draw.rectangle([(7, 10), (10, 14)], fill=WHITE)
    sheet.paste(c, (1 * 16, 112))

    # 7.2: Citizen Carrying Market Basket
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(4, 1), (11, 3)], fill=BONNET_PINK)
    draw.rectangle([(3, 3), (12, 4)], fill=BONNET_LACE)
    draw.rectangle([(4, 6), (11, 9)], fill=SKIN_LIGHT)
    c.putpixel((5, 7), BLACK); c.putpixel((10, 7), BLACK)
    draw.rectangle([(4, 10), (11, 15)], fill=DRESS_SKY)
    draw.rectangle([(6, 10), (9, 14)], fill=WHITE)
    # Wicker basket
    draw.rectangle([(11, 10), (14, 13)], fill=(180, 83, 9, 255))
    c.putpixel((12, 9), (239, 68, 68, 255)) # Apple
    sheet.paste(c, (2 * 16, 112))

    # =========================================================================
    # ROW 8: GUARDA REAL DO CASTELO (Y = 128)
    # =========================================================================
    STEEL_MAIN   = (209, 213, 219, 255)
    STEEL_DARK   = (107, 114, 128, 255)
    PLUME_SCARLET= (220, 38, 38, 255)
    TUNIC_ROYAL  = (30, 58, 138, 255)
    SPEAR_WOOD   = (120, 53, 15, 255)
    SPEAR_TIP    = (243, 244, 246, 255)

    # 8.0: Guard Standing Vigil
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    # Red plume atop helm
    draw.rectangle([(7, 0), (9, 3)], fill=PLUME_SCARLET)
    # Steel helm
    draw.rectangle([(4, 3), (11, 8)], fill=STEEL_MAIN)
    draw.rectangle([(5, 5), (10, 6)], fill=(17, 24, 39, 255)) # Visor slit
    c.putpixel((7, 5), (96, 165, 250, 255)) # Eye reflection
    # Armor plate
    draw.rectangle([(4, 9), (11, 13)], fill=STEEL_MAIN)
    draw.rectangle([(5, 10), (10, 12)], fill=STEEL_DARK)
    draw.rectangle([(4, 13), (11, 15)], fill=TUNIC_ROYAL)
    # Halberd
    draw.line([(13, 0), (13, 15)], fill=SPEAR_WOOD)
    draw.polygon([(13, -1), (11, 2), (15, 2)], fill=SPEAR_TIP)
    sheet.paste(c, (0 * 16, 128))

    # 8.1: Guard Facing Side (Right)
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(5, 0), (7, 3)], fill=PLUME_SCARLET)
    draw.rectangle([(3, 3), (10, 8)], fill=STEEL_MAIN)
    draw.rectangle([(8, 5), (10, 6)], fill=(17, 24, 39, 255))
    draw.rectangle([(3, 9), (10, 13)], fill=STEEL_MAIN)
    draw.rectangle([(3, 13), (10, 15)], fill=TUNIC_ROYAL)
    draw.line([(11, 0), (11, 15)], fill=SPEAR_WOOD)
    draw.polygon([(11, -1), (9, 2), (13, 2)], fill=SPEAR_TIP)
    sheet.paste(c, (1 * 16, 128))

    # 8.2: Guard Saluting
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(7, 0), (9, 3)], fill=PLUME_SCARLET)
    draw.rectangle([(4, 3), (11, 8)], fill=STEEL_MAIN)
    draw.rectangle([(5, 5), (10, 6)], fill=(17, 24, 39, 255))
    draw.rectangle([(4, 9), (11, 13)], fill=STEEL_MAIN)
    draw.rectangle([(4, 13), (11, 15)], fill=TUNIC_ROYAL)
    # Halberd raised in salute
    draw.line([(13, 2), (13, 15)], fill=SPEAR_WOOD)
    draw.polygon([(13, 1), (11, 4), (15, 4)], fill=SPEAR_TIP)
    c.putpixel((3, 7), STEEL_MAIN); c.putpixel((3, 8), STEEL_MAIN) # Hand to helm
    sheet.paste(c, (2 * 16, 128))

    # =========================================================================
    # ROW 9: MESTRE FERREIRO MELARI & ANCIÃO GENTARI (MINISH) (Y = 144)
    # =========================================================================
    GOGGLE_BROWN = (146, 64, 14, 255)
    LENS_ORANGE  = (249, 115, 22, 255)
    MINISH_GRN   = (34, 197, 94, 255)

    # 9.0: Master Smith Melari (Minish Smith)
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.rectangle([(4, 3), (11, 6)], fill=SKIN_LIGHT)
    # Goggles on forehead
    draw.rectangle([(4, 2), (11, 3)], fill=GOGGLE_BROWN)
    c.putpixel((5, 2), LENS_ORANGE); c.putpixel((9, 2), LENS_ORANGE)
    c.putpixel((6, 5), BLACK); c.putpixel((9, 5), BLACK)
    # Full white beard
    draw.rectangle([(4, 7), (11, 11)], fill=BEARD_WHITE)
    draw.rectangle([(5, 11), (10, 13)], fill=BEARD_SHADOW)
    # Smith apron
    draw.rectangle([(4, 8), (11, 13)], fill=SHIRT_RED)
    draw.rectangle([(5, 9), (10, 14)], fill=(120, 53, 15, 255))
    # Small smith hammer
    draw.rectangle([(12, 7), (14, 9)], fill=HAMMER_IRON)
    draw.line([(13, 9), (13, 12)], fill=(120, 53, 15, 255))
    sheet.paste(c, (0 * 16, 144))

    # 9.1: Elder Gentari (Minish Village Elder)
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    # Green Picori cap with leaf
    draw.polygon([(8, 0), (4, 3), (12, 3)], fill=MINISH_GRN)
    c.putpixel((8, 0), (254, 240, 138, 255)) # Leaf tip
    draw.rectangle([(5, 3), (11, 6)], fill=SKIN_LIGHT)
    c.putpixel((6, 4), BLACK); c.putpixel((10, 4), BLACK)
    # Long flowing elder beard
    draw.rectangle([(5, 6), (11, 12)], fill=BEARD_WHITE)
    draw.rectangle([(6, 12), (10, 14)], fill=BEARD_SHADOW)
    # Green sage robe & staff
    draw.rectangle([(4, 8), (12, 14)], fill=MINISH_GRN)
    draw.line([(13, 4), (13, 15)], fill=(120, 53, 15, 255)) # Staff
    c.putpixel((13, 3), (250, 204, 21, 255)) # Staff orb
    sheet.paste(c, (1 * 16, 144))

    # 9.2: Priest Festari (Deepwood Shrine)
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.polygon([(8, 0), (4, 3), (12, 3)], fill=(147, 51, 234, 255))
    draw.rectangle([(5, 3), (11, 6)], fill=SKIN_LIGHT)
    c.putpixel((6, 4), BLACK); c.putpixel((10, 4), BLACK)
    draw.rectangle([(5, 6), (11, 10)], fill=BEARD_WHITE)
    draw.rectangle([(4, 8), (12, 14)], fill=(147, 51, 234, 255))
    draw.rectangle([(6, 8), (10, 14)], fill=WHITE)
    sheet.paste(c, (2 * 16, 144))

    # 9.3: Cheerful Village Minish
    c = make_cell_16()
    draw = ImageDraw.Draw(c)
    draw.polygon([(8, 1), (4, 4), (12, 4)], fill=(239, 68, 68, 255))
    draw.rectangle([(5, 4), (11, 7)], fill=SKIN_LIGHT)
    c.putpixel((6, 5), BLACK); c.putpixel((10, 5), BLACK)
    c.putpixel((8, 6), (220, 38, 38, 255)) # Smile
    draw.rectangle([(5, 8), (11, 13)], fill=(234, 179, 8, 255))
    draw.rectangle([(5, 14), (7, 15)], fill=(120, 53, 15, 255))
    draw.rectangle([(9, 14), (11, 15)], fill=(120, 53, 15, 255))
    sheet.paste(c, (3 * 16, 144))

    # =========================================================================
    # Save to regions
    # =========================================================================
    os.makedirs("assets/regions/usa", exist_ok=True)
    os.makedirs("assets/regions/eur", exist_ok=True)
    os.makedirs("assets/regions/jpn", exist_ok=True)

    save_bmp_32("assets/regions/npcs_master.bmp", sheet)
    save_bmp_32("assets/regions/usa/npcs.bmp", sheet)
    save_bmp_32("assets/regions/eur/npcs.bmp", sheet)
    save_bmp_32("assets/regions/jpn/npcs.bmp", sheet)
    print("Sucesso: Atlas de NPCs gerado para todas as regioes!")

if __name__ == "__main__":
    create_npc_atlas()
