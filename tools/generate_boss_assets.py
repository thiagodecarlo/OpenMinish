#!/usr/bin/env python3
"""
tools/generate_boss_assets.py - Canonical Dungeon Bosses Sprite Atlas Generator
Generates 1:1 authentic GBA style sprites for major dungeon bosses:
  - Gleerok (Cave of Flames)
  - Mazaal (Fortress of Winds)
  - Big Octorok (Temple of Droplets)
  - Gyorg Pair (Palace of Winds)

Output:
  - assets/regions/bosses_master.bmp (256x180 32-bit RGBA BMP)
  - assets/regions/{usa,eur,jpn}/bosses.bmp
"""

import os
import struct

WIDTH = 256
HEIGHT = 196

def create_bmp_rgba(width, height):
    return [[(0, 0, 0, 0) for _ in range(width)] for _ in range(height)]

def put_px(buf, x, y, color):
    if 0 <= x < WIDTH and 0 <= y < HEIGHT:
        buf[y][x] = color

def fill_rect(buf, x, y, w, h, color):
    for r in range(y, y + h):
        for c in range(x, x + w):
            put_px(buf, c, r, color)

def fill_ellipse(buf, cx, cy, rx, ry, color):
    for r in range(cy - ry, cy + ry + 1):
        for c in range(cx - rx, cx + rx + 1):
            if rx > 0 and ry > 0:
                dx = (c - cx) / rx
                dy = (r - cy) / ry
                if dx*dx + dy*dy <= 1.0:
                    put_px(buf, c, r, color)

def save_bmp_rgba32(buf, filepath):
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    height = len(buf)
    width = len(buf[0])
    
    row_bytes = width * 4
    image_size = row_bytes * height
    file_size = 14 + 108 + image_size  # BITMAPV4HEADER
    
    file_header = struct.pack(
        '<2sIHHI',
        b'BM',
        file_size,
        0, 0,
        14 + 108
    )
    
    dib_header = struct.pack(
        '<IIIHHIIIIII',
        108,
        width,
        height,
        1,
        32,
        3,  # BI_BITFIELDS
        image_size,
        2835, 2835,
        0, 0
    )
    
    masks = struct.pack(
        '<IIII',
        0x00FF0000,  # Red
        0x0000FF00,  # Green
        0x000000FF,  # Blue
        0xFF000000   # Alpha
    )
    
    cs_type = b'sRGB'
    endpoints = b'\x00' * 36
    gamma = struct.pack('<III', 0, 0, 0)
    v4_tail = cs_type + endpoints + gamma
    
    with open(filepath, 'wb') as f:
        f.write(file_header)
        f.write(dib_header)
        f.write(masks)
        f.write(v4_tail)
        
        for r in range(height - 1, -1, -1):
            row_data = bytearray()
            for c in range(width):
                red, green, blue, alpha = buf[r][c]
                row_data.extend(struct.pack('<BBBB', blue, green, red, alpha))
            f.write(row_data)

# Palettes
C_TRANS = (0, 0, 0, 0)

# Gleerok
C_BASALT_DARK  = (28, 25, 23, 255)
C_BASALT_MID   = (41, 37, 36, 255)
C_BASALT_LIGHT = (68, 64, 60, 255)
C_MAGMA_SEAM   = (220, 38, 38, 255)
C_MAGMA_ORANGE = (249, 115, 22, 255)
C_MAGMA_YELLOW = (253, 224, 71, 255)
C_GLEEROK_HORN = (217, 119, 6, 255)
C_RUBY_DARK    = (153, 27, 27, 255)
C_RUBY_BRIGHT  = (239, 68, 68, 255)
C_RUBY_SHINE   = (255, 255, 255, 255)

# Mazaal
C_MAZAAL_DARK  = (51, 65, 85, 255)
C_MAZAAL_SLATE = (71, 85, 105, 255)
C_MAZAAL_LIGHT = (100, 116, 139, 255)
C_MAZAAL_BRASS = (217, 119, 6, 255)
C_MAZAAL_GOLD  = (245, 158, 11, 255)
C_MAZAAL_EYE   = (253, 224, 71, 255)
C_MAZAAL_CYAN  = (56, 189, 248, 255)
C_MAZAAL_RED   = (239, 68, 68, 255)

# Big Octorok
C_OCTO_ICE_DARK  = (2, 132, 199, 255)
C_OCTO_ICE_MID   = (56, 189, 248, 255)
C_OCTO_ICE_LIGHT = (186, 230, 253, 255)
C_OCTO_FROST     = (224, 242, 254, 255)
C_OCTO_BURN_DARK = (194, 65, 12, 255)
C_OCTO_BURN_MID  = (249, 115, 22, 255)
C_OCTO_BURN_GLOW = (253, 224, 71, 255)
C_OCTO_EYE       = (15, 23, 42, 255)
C_OCTO_SNOUT     = (14, 165, 233, 255)

# Gyorg Pair
C_GYORG_BLUE_DARK  = (30, 58, 138, 255)
C_GYORG_BLUE_MID   = (37, 99, 235, 255)
C_GYORG_BLUE_LIGHT = (96, 165, 250, 255)
C_GYORG_BLUE_AERO  = (147, 197, 253, 255)

C_GYORG_RED_DARK   = (153, 27, 27, 255)
C_GYORG_RED_MID    = (220, 38, 38, 255)
C_GYORG_RED_LIGHT  = (248, 113, 113, 255)
C_GYORG_RED_AERO   = (254, 202, 202, 255)

C_GYORG_EYE_RED    = (239, 68, 68, 255)
C_GYORG_EYE_GOLD   = (253, 224, 71, 255)
C_GYORG_PUPIL      = (69, 10, 10, 255)
C_WIND_BALL        = (56, 189, 248, 255)
C_WIND_CORE        = (255, 255, 255, 255)

def draw_gleerok(buf):
    # 1. Carapaça Ativa (64x32) at (0, 0)
    x0, y0 = 0, 0
    # Outer basalt shell rim
    fill_ellipse(buf, x0 + 32, y0 + 16, 28, 14, C_BASALT_DARK)
    fill_ellipse(buf, x0 + 32, y0 + 16, 25, 12, C_BASALT_MID)
    fill_ellipse(buf, x0 + 32, y0 + 16, 21, 9, C_BASALT_LIGHT)
    # Spines along shell edges
    for sx in [8, 20, 32, 44, 56]:
        fill_rect(buf, x0 + sx - 2, y0 + 2, 4, 4, C_GLEEROK_HORN)
        fill_rect(buf, x0 + sx - 2, y0 + 26, 4, 4, C_GLEEROK_HORN)
    # Magma fissures
    for fx in [18, 30, 42]:
        fill_rect(buf, x0 + fx - 2, y0 + 8, 4, 16, C_MAGMA_SEAM)
        fill_rect(buf, x0 + fx - 1, y0 + 10, 2, 12, C_MAGMA_ORANGE)
        put_px(buf, x0 + fx, y0 + 15, C_MAGMA_YELLOW)
        put_px(buf, x0 + fx, y0 + 16, C_RUBY_SHINE)

    # 2. Cabeça & Pescoço Ativo (32x32) at (64, 0)
    x1, y1 = 64, 0
    # Neck segments
    fill_ellipse(buf, x1 + 16, y1 + 8, 8, 6, C_BASALT_MID)
    fill_rect(buf, x1 + 14, y1 + 4, 4, 6, C_MAGMA_SEAM)
    # Head cranium
    fill_ellipse(buf, x1 + 16, y1 + 18, 11, 9, C_BASALT_DARK)
    fill_ellipse(buf, x1 + 16, y1 + 18, 9, 7, C_BASALT_MID)
    # Dragon horns
    fill_rect(buf, x1 + 7, y1 + 11, 4, 7, C_GLEEROK_HORN)
    fill_rect(buf, x1 + 21, y1 + 11, 4, 7, C_GLEEROK_HORN)
    put_px(buf, x1 + 6, y1 + 10, C_MAGMA_YELLOW)
    put_px(buf, x1 + 25, y1 + 10, C_MAGMA_YELLOW)
    # Eyes
    fill_rect(buf, x1 + 10, y1 + 18, 3, 3, C_MAGMA_YELLOW)
    fill_rect(buf, x1 + 19, y1 + 18, 3, 3, C_MAGMA_YELLOW)
    put_px(buf, x1 + 11, y1 + 19, C_MAGMA_SEAM)
    put_px(buf, x1 + 20, y1 + 19, C_MAGMA_SEAM)
    # Snout
    fill_rect(buf, x1 + 12, y1 + 23, 8, 6, C_BASALT_LIGHT)
    put_px(buf, x1 + 13, y1 + 27, C_BASALT_DARK)
    put_px(buf, x1 + 18, y1 + 27, C_BASALT_DARK)

    # 3. Focinho Cuspindo Fogo (32x32) at (96, 0)
    x2, y2 = 96, 0
    # Copy head
    fill_ellipse(buf, x2 + 16, y2 + 16, 11, 9, C_BASALT_DARK)
    fill_ellipse(buf, x2 + 16, y2 + 16, 9, 7, C_BASALT_MID)
    fill_rect(buf, x2 + 7, y2 + 9, 4, 7, C_GLEEROK_HORN)
    fill_rect(buf, x2 + 21, y2 + 9, 4, 7, C_GLEEROK_HORN)
    fill_rect(buf, x2 + 10, y2 + 16, 3, 3, C_MAGMA_YELLOW)
    fill_rect(buf, x2 + 19, y2 + 16, 3, 3, C_MAGMA_YELLOW)
    # Open burning maw
    fill_ellipse(buf, x2 + 16, y2 + 25, 7, 5, C_MAGMA_SEAM)
    fill_ellipse(buf, x2 + 16, y2 + 25, 5, 3, C_MAGMA_ORANGE)
    fill_ellipse(buf, x2 + 16, y2 + 25, 3, 2, C_MAGMA_YELLOW)
    put_px(buf, x2 + 16, y2 + 25, C_RUBY_SHINE)

    # 4. Carapaça Invertida com Núcleo de Rubi (64x32) at (128, 0)
    x3, y3 = 128, 0
    fill_ellipse(buf, x3 + 32, y3 + 16, 28, 14, C_BASALT_DARK)
    fill_ellipse(buf, x3 + 32, y3 + 16, 25, 12, C_BASALT_MID)
    # Glowing Ruby Core cavity
    fill_ellipse(buf, x3 + 32, y3 + 16, 16, 9, C_RUBY_DARK)
    fill_ellipse(buf, x3 + 32, y3 + 16, 12, 7, C_RUBY_BRIGHT)
    fill_ellipse(buf, x3 + 32, y3 + 16, 8, 4, C_MAGMA_ORANGE)
    fill_ellipse(buf, x3 + 32, y3 + 16, 4, 2, C_MAGMA_YELLOW)
    put_px(buf, x3 + 30, y3 + 14, C_RUBY_SHINE)
    put_px(buf, x3 + 31, y3 + 14, C_RUBY_SHINE)

    # 5. Cabeça & Pescoço Atordoado / Rampa (32x32) at (192, 0)
    x4, y4 = 192, 0
    # Flat ramp neck
    fill_rect(buf, x4 + 10, y4 + 4, 12, 14, C_BASALT_DARK)
    fill_rect(buf, x4 + 12, y4 + 6, 8, 10, C_BASALT_LIGHT)
    # Scaled footsteps on ramp
    for ry in [7, 10, 13]:
        fill_rect(buf, x4 + 13, y4 + ry, 6, 2, C_GLEEROK_HORN)
    # Stunned head resting
    fill_ellipse(buf, x4 + 16, y4 + 23, 10, 7, C_BASALT_MID)
    # Dizzy/closed eye
    fill_rect(buf, x4 + 12, y4 + 22, 3, 2, (30, 41, 59, 255))
    fill_rect(buf, x4 + 17, y4 + 22, 3, 2, (30, 41, 59, 255))

    # 6. Magma Fireball (16x16) at (224, 0)
    x5, y5 = 224, 0
    fill_ellipse(buf, x5 + 8, y5 + 8, 6, 6, C_MAGMA_SEAM)
    fill_ellipse(buf, x5 + 8, y5 + 8, 4, 4, C_MAGMA_ORANGE)
    fill_ellipse(buf, x5 + 8, y5 + 8, 2, 2, C_MAGMA_YELLOW)
    put_px(buf, x5 + 8, y5 + 8, C_RUBY_SHINE)

def draw_mazaal(buf):
    # Y base = 32
    # 1. Cabeça de Mazaal - Normal (48x36) at (0, 32)
    x0, y0 = 0, 32
    # Heavy stone mask
    fill_rect(buf, x0 + 4, y0 + 2, 40, 32, C_MAZAAL_DARK)
    fill_rect(buf, x0 + 6, y0 + 4, 36, 28, C_MAZAAL_SLATE)
    fill_rect(buf, x0 + 8, y0 + 6, 32, 24, C_MAZAAL_LIGHT)
    # Brass engravings & Wind crest
    fill_rect(buf, x0 + 18, y0 + 4, 12, 4, C_MAZAAL_GOLD)
    fill_rect(buf, x0 + 22, y0 + 2, 4, 8, C_MAZAAL_BRASS)
    # Eyes
    fill_rect(buf, x0 + 12, y0 + 14, 7, 5, C_MAZAAL_DARK)
    fill_rect(buf, x0 + 29, y0 + 14, 7, 5, C_MAZAAL_DARK)
    fill_rect(buf, x0 + 13, y0 + 15, 5, 3, C_MAZAAL_EYE)
    fill_rect(buf, x0 + 30, y0 + 15, 5, 3, C_MAZAAL_EYE)
    put_px(buf, x0 + 15, y0 + 16, C_RUBY_SHINE)
    put_px(buf, x0 + 32, y0 + 16, C_RUBY_SHINE)
    # Stone mouth slit
    fill_rect(buf, x0 + 16, y0 + 24, 16, 4, C_MAZAAL_DARK)

    # 2. Cabeça de Mazaal - Boca Aberta / Atordoado (48x36) at (48, 32)
    x1, y1 = 48, 32
    fill_rect(buf, x1 + 4, y1 + 2, 40, 32, C_MAZAAL_DARK)
    fill_rect(buf, x1 + 6, y1 + 4, 36, 28, C_MAZAAL_SLATE)
    fill_rect(buf, x1 + 8, y1 + 6, 32, 24, C_MAZAAL_LIGHT)
    # Dimmed eyes
    fill_rect(buf, x1 + 13, y1 + 15, 5, 2, (71, 85, 105, 255))
    fill_rect(buf, x1 + 30, y1 + 15, 5, 2, (71, 85, 105, 255))
    # Big Open Mouth Gap (Minish Portal entrance)
    fill_rect(buf, x1 + 12, y1 + 20, 24, 12, (15, 23, 42, 255))
    fill_rect(buf, x1 + 14, y1 + 22, 20, 8, (2, 6, 23, 255))
    # Glowing circuits inside
    fill_rect(buf, x1 + 18, y1 + 24, 4, 4, C_MAZAAL_CYAN)
    fill_rect(buf, x1 + 26, y1 + 24, 4, 4, C_MAZAAL_RED)

    # 3. Núcleo Interno Minish (32x32) at (96, 32)
    x2, y2 = 96, 32
    fill_rect(buf, x2 + 4, y2 + 2, 24, 28, C_MAZAAL_DARK)
    fill_rect(buf, x2 + 6, y2 + 4, 20, 24, (15, 23, 42, 255))
    # Circuit pillars
    fill_rect(buf, x2 + 8, y2 + 6, 4, 20, C_MAZAAL_CYAN)
    fill_rect(buf, x2 + 20, y2 + 6, 4, 20, C_MAZAAL_CYAN)
    # Core Eye Pillar
    fill_ellipse(buf, x2 + 16, y2 + 16, 6, 6, C_MAZAAL_RED)
    fill_ellipse(buf, x2 + 16, y2 + 16, 4, 4, C_MAZAAL_GOLD)
    put_px(buf, x2 + 15, y2 + 15, C_RUBY_SHINE)

    # 4. Mão - Aberta com Olho (24x24) at (128, 32)
    x3, y3 = 128, 32
    fill_ellipse(buf, x3 + 12, y3 + 12, 10, 10, C_MAZAAL_DARK)
    fill_ellipse(buf, x3 + 12, y3 + 12, 8, 8, C_MAZAAL_BRASS)
    # Fingers
    for fx in [5, 9, 15, 19]:
        fill_rect(buf, x3 + fx, y3 + 1, 3, 5, C_MAZAAL_SLATE)
    # Palm Eye
    fill_ellipse(buf, x3 + 12, y3 + 13, 5, 4, C_MAZAAL_RED)
    fill_ellipse(buf, x3 + 12, y3 + 13, 3, 2, C_MAZAAL_EYE)
    put_px(buf, x3 + 12, y3 + 13, C_RUBY_SHINE)

    # 5. Mão - Fechada Slam (24x24) at (152, 32)
    x4, y4 = 152, 32
    fill_ellipse(buf, x4 + 12, y4 + 12, 10, 9, C_MAZAAL_DARK)
    fill_ellipse(buf, x4 + 12, y4 + 12, 8, 7, C_MAZAAL_SLATE)
    # Knuckles
    for kx in [6, 10, 14, 18]:
        fill_rect(buf, x4 + kx, y4 + 6, 3, 4, C_MAZAAL_BRASS)

    # 6. Mão - Desativada / Danificada (24x24) at (176, 32)
    x5, y5 = 176, 32
    fill_ellipse(buf, x5 + 12, y5 + 13, 10, 8, (30, 41, 59, 255))
    fill_ellipse(buf, x5 + 12, y5 + 13, 8, 6, (51, 65, 85, 255))
    # Eye extinguished
    fill_ellipse(buf, x5 + 12, y5 + 13, 4, 3, (15, 23, 42, 255))
    # Sparks
    put_px(buf, x5 + 8, y5 + 6, C_MAZAAL_CYAN)
    put_px(buf, x5 + 9, y5 + 7, C_RUBY_SHINE)
    put_px(buf, x5 + 16, y5 + 8, C_MAZAAL_CYAN)

def draw_big_octorok(buf):
    # Y base = 80
    # 1. Big Octorok - Fase Congelada (32x32) at (0, 80)
    x0, y0 = 0, 80
    fill_ellipse(buf, x0 + 16, y0 + 16, 14, 13, C_OCTO_ICE_DARK)
    fill_ellipse(buf, x0 + 16, y0 + 16, 12, 11, C_OCTO_ICE_MID)
    fill_ellipse(buf, x0 + 16, y0 + 14, 9, 8, C_OCTO_ICE_LIGHT)
    # Frost crust
    for f in [7, 12, 17, 22]:
        put_px(buf, x0 + f, y0 + 6, C_OCTO_FROST)
        put_px(buf, x0 + f + 1, y0 + 7, C_OCTO_FROST)
    # Eyes
    fill_rect(buf, x0 + 8, y0 + 15, 3, 4, C_OCTO_EYE)
    fill_rect(buf, x0 + 21, y0 + 15, 3, 4, C_OCTO_EYE)
    put_px(buf, x0 + 9, y0 + 16, C_RUBY_SHINE)
    put_px(buf, x0 + 22, y0 + 16, C_RUBY_SHINE)
    # Snout
    fill_ellipse(buf, x0 + 16, y0 + 22, 6, 5, C_OCTO_ICE_DARK)
    fill_ellipse(buf, x0 + 16, y0 + 22, 4, 3, C_OCTO_SNOUT)
    fill_rect(buf, x0 + 15, y0 + 22, 2, 2, C_OCTO_EYE)

    # 2. Big Octorok - Fase Atordoada (32x32) at (32, 80)
    x1, y1 = 32, 80
    fill_ellipse(buf, x1 + 16, y1 + 16, 14, 13, C_OCTO_ICE_DARK)
    fill_ellipse(buf, x1 + 16, y1 + 16, 12, 11, C_OCTO_ICE_MID)
    # Dizzy spiral eyes
    fill_rect(buf, x1 + 8, y1 + 15, 4, 3, (253, 224, 71, 255))
    fill_rect(buf, x1 + 20, y1 + 15, 4, 3, (253, 224, 71, 255))
    put_px(buf, x1 + 9, y1 + 16, C_OCTO_EYE)
    put_px(buf, x1 + 21, y1 + 16, C_OCTO_EYE)
    # Stunned yellowish bruised snout
    fill_ellipse(buf, x1 + 16, y1 + 22, 6, 5, (202, 138, 4, 255))
    fill_ellipse(buf, x1 + 16, y1 + 22, 4, 3, (250, 204, 21, 255))

    # 3. Big Octorok - Fase Em Chamas (32x32) at (64, 80)
    x2, y2 = 64, 80
    fill_ellipse(buf, x2 + 16, y2 + 16, 14, 13, C_OCTO_BURN_DARK)
    fill_ellipse(buf, x2 + 16, y2 + 16, 12, 11, C_OCTO_BURN_MID)
    fill_ellipse(buf, x2 + 16, y2 + 14, 9, 8, C_OCTO_BURN_GLOW)
    # Panic eyes
    fill_rect(buf, x2 + 8, y2 + 14, 4, 5, C_RUBY_SHINE)
    fill_rect(buf, x2 + 20, y2 + 14, 4, 5, C_RUBY_SHINE)
    fill_rect(buf, x2 + 9, y2 + 16, 2, 2, C_OCTO_EYE)
    fill_rect(buf, x2 + 21, y2 + 16, 2, 2, C_OCTO_EYE)
    # Screaming snout
    fill_ellipse(buf, x2 + 16, y2 + 23, 6, 5, C_OCTO_BURN_DARK)
    fill_ellipse(buf, x2 + 16, y2 + 23, 4, 3, (220, 38, 38, 255))

    # 4. Cauda Congelada (16x16) at (96, 80)
    x3, y3 = 96, 80
    fill_ellipse(buf, x3 + 8, y3 + 8, 6, 5, C_OCTO_ICE_DARK)
    fill_ellipse(buf, x3 + 8, y3 + 8, 4, 3, C_OCTO_ICE_LIGHT)
    put_px(buf, x3 + 8, y3 + 3, C_OCTO_FROST)

    # 5. Cauda Em Chamas (16x16) at (112, 80)
    x4, y4 = 112, 80
    fill_ellipse(buf, x4 + 8, y4 + 9, 6, 5, C_OCTO_BURN_DARK)
    # Fire flames leaping from tail
    fill_ellipse(buf, x4 + 8, y4 + 6, 5, 5, C_OCTO_BURN_MID)
    fill_ellipse(buf, x4 + 8, y4 + 5, 3, 4, C_OCTO_BURN_GLOW)
    put_px(buf, x4 + 8, y4 + 3, C_RUBY_SHINE)

    # 6. Octo Rock Glacial & Refletida (8x8 cada) at (128, 80)
    x5, y5 = 128, 80
    # Normal rock
    fill_ellipse(buf, x5 + 4, y5 + 4, 3, 3, (71, 85, 105, 255))
    put_px(buf, x5 + 3, y5 + 3, (148, 163, 184, 255))
    # Reflected rock at (x5 + 8, y5)
    fill_ellipse(buf, x5 + 12, y5 + 4, 3, 3, (245, 158, 11, 255))
    fill_ellipse(buf, x5 + 12, y5 + 4, 2, 2, (253, 224, 71, 255))
    put_px(buf, x5 + 12, y5 + 3, C_RUBY_SHINE)

def draw_gyorg_pair(buf):
    # Y base = 120
    # 1. Grande Arraia Fêmea Azul (64x36) at (0, 120)
    x0, y0 = 0, 120
    # Huge aerodynamic wing span
    fill_ellipse(buf, x0 + 32, y0 + 18, 29, 15, C_GYORG_BLUE_DARK)
    fill_ellipse(buf, x0 + 32, y0 + 18, 26, 12, C_GYORG_BLUE_MID)
    fill_ellipse(buf, x0 + 32, y0 + 18, 20, 8, C_GYORG_BLUE_LIGHT)
    # Wing tips aero edge
    fill_ellipse(buf, x0 + 8, y0 + 18, 6, 3, C_GYORG_BLUE_AERO)
    fill_ellipse(buf, x0 + 56, y0 + 18, 6, 3, C_GYORG_BLUE_AERO)
    # 4 Eye nodes on back
    eye_positions = [(x0 + 20, y0 + 14), (x0 + 20, y0 + 22), (x0 + 44, y0 + 14), (x0 + 44, y0 + 22)]
    for ex, ey in eye_positions:
        fill_ellipse(buf, ex, ey, 4, 3, C_GYORG_BLUE_DARK)
        fill_ellipse(buf, ex, ey, 2, 2, C_GYORG_EYE_RED)
        put_px(buf, ex, ey, C_GYORG_EYE_GOLD)

    # 2. Olhos de Gyorg Azul (16x16) at (64, 120)
    x1, y1 = 64, 120
    # Top: Eye Open
    fill_ellipse(buf, x1 + 8, y1 + 4, 5, 3, C_GYORG_EYE_RED)
    fill_ellipse(buf, x1 + 8, y1 + 4, 3, 2, C_GYORG_EYE_GOLD)
    put_px(buf, x1 + 8, y1 + 4, C_GYORG_PUPIL)
    # Bottom: Eye Closed
    fill_ellipse(buf, x1 + 8, y1 + 12, 5, 2, C_GYORG_BLUE_DARK)
    fill_rect(buf, x1 + 5, y1 + 12, 6, 1, C_GYORG_BLUE_LIGHT)

    # 3. Arraia Macho Vermelha (48x28) at (80, 120)
    x2, y2 = 80, 120
    fill_ellipse(buf, x2 + 24, y2 + 14, 21, 11, C_GYORG_RED_DARK)
    fill_ellipse(buf, x2 + 24, y2 + 14, 18, 9, C_GYORG_RED_MID)
    fill_ellipse(buf, x2 + 24, y2 + 14, 13, 6, C_GYORG_RED_LIGHT)
    # Wing tips
    fill_ellipse(buf, x2 + 6, y2 + 14, 4, 2, C_GYORG_RED_AERO)
    fill_ellipse(buf, x2 + 42, y2 + 14, 4, 2, C_GYORG_RED_AERO)
    # Center Optic Eye
    fill_ellipse(buf, x2 + 24, y2 + 14, 4, 4, C_GYORG_EYE_GOLD)
    fill_ellipse(buf, x2 + 24, y2 + 14, 2, 2, C_GYORG_EYE_RED)
    put_px(buf, x2 + 24, y2 + 14, C_RUBY_SHINE)

    # 4. Cauda & Esfera de Vento (16x16) at (128, 120)
    x3, y3 = 128, 120
    # Tail
    for t in range(8):
        put_px(buf, x3 + 2 + t, y3 + 4 + (t % 3), C_GYORG_RED_DARK)
    # Wind Sphere
    fill_ellipse(buf, x3 + 10, y3 + 10, 5, 5, C_WIND_BALL)
    fill_ellipse(buf, x3 + 10, y3 + 10, 3, 3, (125, 211, 252, 255))
    put_px(buf, x3 + 10, y3 + 10, C_WIND_CORE)

def draw_vaati_and_relics(buf):
    C_VAATI_ROBE      = (88, 28, 135, 255)
    C_VAATI_ROBE_DK   = (59, 7, 100, 255)
    C_VAATI_TRIM      = (245, 158, 11, 255)
    C_VAATI_SKIN      = (237, 233, 254, 255)
    C_VAATI_HAIR      = (216, 180, 254, 255)
    C_VAATI_HAT       = (107, 33, 168, 255)
    C_VAATI_EYE_RED   = (220, 38, 38, 255)
    C_VAATI_MALICE    = (192, 132, 252, 255)
    C_CHEST_WOOD      = (180, 83, 9, 255)
    C_CHEST_DK        = (120, 53, 15, 255)
    C_CHEST_GOLD      = (245, 158, 11, 255)
    C_SWORD_BLADE     = (56, 189, 248, 255)
    C_SWORD_CORE      = (240, 249, 255, 255)

    base_y = 160

    # 1. Vaati Idle Floating at (0, 160)
    vx0 = 0
    # Shadow
    fill_ellipse(buf, vx0 + 16, base_y + 30, 8, 2, (15, 23, 42, 120))
    # Robe / Cape
    fill_rect(buf, vx0 + 10, base_y + 12, 12, 16, C_VAATI_ROBE)
    fill_rect(buf, vx0 + 8, base_y + 16, 16, 12, C_VAATI_ROBE)
    fill_rect(buf, vx0 + 7, base_y + 24, 18, 5, C_VAATI_ROBE_DK)
    # Gold trim on hem
    fill_rect(buf, vx0 + 7, base_y + 28, 18, 1, C_VAATI_TRIM)
    # Head & Lavender Hair
    fill_rect(buf, vx0 + 11, base_y + 6, 10, 8, C_VAATI_SKIN)
    fill_rect(buf, vx0 + 9, base_y + 6, 3, 10, C_VAATI_HAIR)
    fill_rect(buf, vx0 + 20, base_y + 6, 3, 10, C_VAATI_HAIR)
    # Pointed Wizard Hat
    fill_rect(buf, vx0 + 9, base_y + 4, 14, 3, C_VAATI_HAT)
    fill_rect(buf, vx0 + 11, base_y + 1, 10, 3, C_VAATI_HAT)
    put_px(buf, vx0 + 13, base_y + 0, C_VAATI_HAT)
    put_px(buf, vx0 + 14, base_y + 0, C_VAATI_HAT)
    put_px(buf, vx0 + 15, base_y + 3, C_VAATI_TRIM) # Hat gem
    # Eyes
    put_px(buf, vx0 + 13, base_y + 9, C_VAATI_EYE_RED)
    put_px(buf, vx0 + 17, base_y + 9, C_VAATI_EYE_RED)

    # 2. Vaati Casting Dark Spell at (32, 160)
    vx1 = 32
    fill_ellipse(buf, vx1 + 16, base_y + 30, 8, 2, (15, 23, 42, 120))
    # Robe blowing
    fill_rect(buf, vx1 + 10, base_y + 12, 12, 16, C_VAATI_ROBE)
    fill_rect(buf, vx1 + 6, base_y + 16, 20, 12, C_VAATI_ROBE)
    fill_rect(buf, vx1 + 5, base_y + 24, 22, 5, C_VAATI_ROBE_DK)
    fill_rect(buf, vx1 + 5, base_y + 28, 22, 1, C_VAATI_TRIM)
    # Raised arms with malice orbs
    fill_rect(buf, vx1 + 4, base_y + 10, 6, 3, C_VAATI_SKIN)
    fill_rect(buf, vx1 + 22, base_y + 10, 6, 3, C_VAATI_SKIN)
    fill_ellipse(buf, vx1 + 4, base_y + 10, 4, 4, C_VAATI_MALICE)
    fill_ellipse(buf, vx1 + 27, base_y + 10, 4, 4, C_VAATI_MALICE)
    put_px(buf, vx1 + 4, base_y + 10, (255, 255, 255, 255))
    put_px(buf, vx1 + 27, base_y + 10, (255, 255, 255, 255))
    # Head & Hat
    fill_rect(buf, vx1 + 11, base_y + 6, 10, 8, C_VAATI_SKIN)
    fill_rect(buf, vx1 + 9, base_y + 5, 14, 3, C_VAATI_HAT)
    fill_rect(buf, vx1 + 11, base_y + 1, 10, 4, C_VAATI_HAT)
    put_px(buf, vx1 + 13, base_y + 8, C_VAATI_EYE_RED)
    put_px(buf, vx1 + 14, base_y + 8, C_VAATI_EYE_RED)
    put_px(buf, vx1 + 17, base_y + 8, C_VAATI_EYE_RED)
    put_px(buf, vx1 + 18, base_y + 8, C_VAATI_EYE_RED)

    # 3. Vaati Laughing / Smirking at (64, 160)
    vx2 = 64
    fill_ellipse(buf, vx2 + 16, base_y + 30, 8, 2, (15, 23, 42, 120))
    fill_rect(buf, vx2 + 10, base_y + 12, 12, 16, C_VAATI_ROBE)
    fill_rect(buf, vx2 + 8, base_y + 16, 16, 12, C_VAATI_ROBE)
    fill_rect(buf, vx2 + 7, base_y + 24, 18, 5, C_VAATI_ROBE_DK)
    fill_rect(buf, vx2 + 7, base_y + 28, 18, 1, C_VAATI_TRIM)
    # Head tilted back
    fill_rect(buf, vx2 + 11, base_y + 5, 10, 8, C_VAATI_SKIN)
    fill_rect(buf, vx2 + 9, base_y + 2, 14, 3, C_VAATI_HAT)
    fill_rect(buf, vx2 + 10, base_y - 1, 8, 3, C_VAATI_HAT)
    put_px(buf, vx2 + 14, base_y + 8, C_VAATI_EYE_RED)
    put_px(buf, vx2 + 18, base_y + 8, C_VAATI_EYE_RED)
    # Smirking mouth
    fill_rect(buf, vx2 + 14, base_y + 11, 4, 1, (120, 53, 15, 255))
    put_px(buf, vx2 + 17, base_y + 10, (120, 53, 15, 255))

    # 4. Vaati Dark Teleport / Sphere at (96, 160)
    vx3 = 96
    fill_ellipse(buf, vx3 + 16, base_y + 16, 14, 14, C_VAATI_ROBE_DK)
    fill_ellipse(buf, vx3 + 16, base_y + 16, 11, 11, C_VAATI_HAT)
    fill_ellipse(buf, vx3 + 16, base_y + 16, 8, 8, (15, 23, 42, 255))
    fill_ellipse(buf, vx3 + 16, base_y + 16, 4, 4, C_VAATI_MALICE)
    # Demonic Eye
    put_px(buf, vx3 + 15, base_y + 16, C_VAATI_EYE_RED)
    put_px(buf, vx3 + 16, base_y + 16, (255, 255, 255, 255))

    # 5. The Bound Chest Intact with Picori Blade at (128, 160)
    cx0 = 128
    # Stone pedestal step
    fill_rect(buf, cx0 + 2, base_y + 28, 28, 4, (100, 116, 139, 255))
    fill_rect(buf, cx0 + 4, base_y + 26, 24, 2, (148, 163, 184, 255))
    # Chest Box
    fill_rect(buf, cx0 + 5, base_y + 14, 22, 12, C_CHEST_WOOD)
    fill_rect(buf, cx0 + 4, base_y + 12, 24, 4, C_CHEST_DK) # Lid
    # Gold bands and lock
    fill_rect(buf, cx0 + 8, base_y + 12, 2, 14, C_CHEST_GOLD)
    fill_rect(buf, cx0 + 22, base_y + 12, 2, 14, C_CHEST_GOLD)
    fill_rect(buf, cx0 + 13, base_y + 15, 6, 5, C_CHEST_GOLD)
    put_px(buf, cx0 + 15, base_y + 18, (0, 0, 0, 255)) # Keyhole
    # Picori Blade sealing the chest
    fill_rect(buf, cx0 + 15, base_y + 6, 2, 10, C_SWORD_BLADE)
    put_px(buf, cx0 + 15, base_y + 7, C_SWORD_CORE)
    put_px(buf, cx0 + 15, base_y + 10, C_SWORD_CORE)
    fill_rect(buf, cx0 + 12, base_y + 5, 8, 2, C_CHEST_GOLD) # Guard
    fill_rect(buf, cx0 + 15, base_y + 1, 2, 4, (34, 197, 94, 255)) # Green grip
    put_px(buf, cx0 + 15, base_y + 0, C_CHEST_GOLD) # Pommel

    # 6. Broken Picori Blade & Opened Chest at (160, 160)
    cx1 = 160
    fill_rect(buf, cx1 + 2, base_y + 28, 28, 4, (100, 116, 139, 255))
    # Chest Open
    fill_rect(buf, cx1 + 5, base_y + 16, 22, 12, C_CHEST_WOOD)
    fill_rect(buf, cx1 + 6, base_y + 16, 20, 6, (15, 23, 42, 255)) # Dark void inside
    # Lid open back
    fill_rect(buf, cx1 + 3, base_y + 8, 26, 6, C_CHEST_DK)
    fill_rect(buf, cx1 + 7, base_y + 8, 2, 6, C_CHEST_GOLD)
    fill_rect(buf, cx1 + 23, base_y + 8, 2, 6, C_CHEST_GOLD)
    # Malice smoke leaking
    fill_ellipse(buf, cx1 + 16, base_y + 12, 6, 4, C_VAATI_MALICE)
    fill_ellipse(buf, cx1 + 20, base_y + 9, 4, 3, C_VAATI_ROBE_DK)
    # Broken Sword - Lower stump
    fill_rect(buf, cx1 + 15, base_y + 15, 2, 4, C_SWORD_BLADE)
    # Broken Sword - Shattered top piece flying
    fill_rect(buf, cx1 + 18, base_y + 3, 2, 5, C_SWORD_BLADE)
    fill_rect(buf, cx1 + 15, base_y + 2, 8, 2, C_CHEST_GOLD)
    put_px(buf, cx1 + 19, base_y + 0, C_CHEST_GOLD)
    # Sparks
    put_px(buf, cx1 + 13, base_y + 10, (254, 240, 138, 255))
    put_px(buf, cx1 + 17, base_y + 11, (56, 189, 248, 255))
    put_px(buf, cx1 + 23, base_y + 13, (254, 240, 138, 255))

def generate_all():
    buf = create_bmp_rgba(WIDTH, HEIGHT)
    draw_gleerok(buf)
    draw_mazaal(buf)
    draw_big_octorok(buf)
    draw_gyorg_pair(buf)
    draw_vaati_and_relics(buf)

    master_path = "assets/regions/bosses_master.bmp"
    save_bmp_rgba32(buf, master_path)
    print(f"[OK] Generated {master_path} ({WIDTH}x{HEIGHT})")

    for reg in ["usa", "eur", "jpn"]:
        reg_path = f"assets/regions/{reg}/bosses.bmp"
        save_bmp_rgba32(buf, reg_path)
        print(f"[OK] Generated {reg_path}")

if __name__ == "__main__":
    generate_all()
