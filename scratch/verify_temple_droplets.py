#!/usr/bin/env python3
"""
scratch/verify_temple_droplets.py
Generates temple_of_droplets_showcase.png showcasing:
1. Chamber 0: Glacial Entrance & Frozen Ice Floes (Minish Link, ice sliding physics)
2. Chamber 1: Sunbeam Skylight & Optical Prism Puzzle (Melting small key ice block)
3. Chamber 2: Dark Cavern & Flame Lantern 4-Torch Puzzle (Unlocking Boss Key)
4. Chamber 5: Big Octorok Boss Fight & Sacred Water Element Thaw (Spike deflection, burning tail, Water Element)
"""

import math
import os
import shutil
from PIL import Image, ImageDraw, ImageFont

W, H = 480, 320
IMG_W, IMG_H = W * 2, H * 2
canvas = Image.new("RGBA", (IMG_W, IMG_H), (10, 15, 24, 255))
draw = ImageDraw.Draw(canvas)

try:
    font_title = ImageFont.truetype("arial.ttf", 15)
    font_body = ImageFont.truetype("arial.ttf", 12)
    font_small = ImageFont.truetype("arial.ttf", 11)
except:
    font_title = font_body = font_small = ImageFont.load_default()

def draw_hud(pdraw, ox, oy, title, status_text, keys=1, boss_key=False, water_element=False):
    # Top HUD
    pdraw.rectangle([ox, oy, ox + W, oy + 24], fill=(12, 28, 44, 255))
    # Hearts
    for i in range(6):
        hx = ox + 8 + i * 13
        hy = oy + 6
        pdraw.polygon([(hx+4, hy+1), (hx+1, hy+4), (hx+1, hy+6), (hx+4, hy+9),
                       (hx+7, hy+6), (hx+7, hy+4)], fill=(239, 68, 68, 255))
        pdraw.polygon([(hx+7, hy+6), (hx+10, hy+4), (hx+10, hy+1), (hx+7, hy+1)], fill=(239, 68, 68, 255))
    # Rupees
    rx = ox + 95
    pdraw.polygon([(rx+4, oy+4), (rx+8, oy+9), (rx+4, oy+15), (rx, oy+9)], fill=(56, 189, 248, 255))
    pdraw.text((rx + 12, oy + 4), "350", fill=(255, 255, 255, 255), font=font_small)

    # Subweapon Slot B (Flame Lantern)
    bx = ox + 145
    pdraw.rectangle([bx, oy + 3, bx + 18, oy + 21], fill=(8, 47, 73, 255), outline=(56, 189, 248, 255))
    pdraw.text((bx + 2, oy + 4), "B", fill=(253, 224, 71, 255), font=font_small)
    # Lantern flame icon
    pdraw.ellipse([bx + 11, oy + 8, bx + 15, oy + 15], fill=(249, 115, 22, 255))

    # Small keys
    kx = ox + 175
    if keys > 0:
        pdraw.rectangle([kx, oy + 7, kx + 8, oy + 9], fill=(245, 158, 11, 255))
        pdraw.rectangle([kx + 6, oy + 9, kx + 8, oy + 13], fill=(245, 158, 11, 255))
        pdraw.text((kx + 11, oy + 4), f"x{keys}", fill=(254, 240, 138, 255), font=font_small)

    # Boss key
    if boss_key:
        bkx = ox + 205
        pdraw.rectangle([bkx, oy + 6, bkx + 9, oy + 15], fill=(217, 119, 6, 255), outline=(254, 240, 138, 255))

    # Water Element Icon
    if water_element:
        wex = ox + 225
        pdraw.ellipse([wex, oy + 6, wex + 10, oy + 16], fill=(6, 182, 212, 255), outline=(224, 242, 254, 255))

    # Room title
    pdraw.text((ox + 245, oy + 4), title, fill=(224, 242, 254, 255), font=font_small)

    # Bottom status bar
    pdraw.rectangle([ox + 25, oy + H - 24, ox + W - 25, oy + H - 6], fill=(8, 30, 50, 235), outline=(14, 165, 233, 255))
    pdraw.text((ox + 35, oy + H - 22), status_text, fill=(224, 242, 254, 255), font=font_small)

# -------------------------------------------------------------
# PANEL 1: Glacial Entrance & Frozen Ice Floes (Minish Link)
# -------------------------------------------------------------
p1 = Image.new("RGBA", (W, H), (14, 30, 48, 255))
d1 = ImageDraw.Draw(p1)

# Water floor with icy gradient
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (20, 45, 75) if ((x//16 + y//16) % 2 == 0) else (15, 38, 65)
        d1.rectangle([x, y, x + 16, y + 16], fill=c)
        d1.line([(x, y + 8), (x + 16, y + 8)], fill=(30, 60, 95))

# Perimeter Ice Walls
for y in range(0, H, 16):
    for x in range(0, W, 16):
        if x < 32 or x >= W - 32 or y < 32 or y >= H - 32:
            d1.rectangle([x, y, x + 16, y + 16], fill=(35, 75, 115), outline=(70, 130, 180))

# Southern Entryway open to Lake Hylia
d1.rectangle([W//2 - 32, H - 32, W//2 + 32, H], fill=(15, 38, 65))

# Floating Ice Floes (translucent cyan ice slabs)
ice_slabs = [
    (80, 80, 200, 150),
    (240, 110, 390, 210),
    (140, 180, 260, 260)
]
for x1, y1, x2, y2 in ice_slabs:
    d1.rectangle([x1, y1, x2, y2], fill=(125, 211, 252, 210), outline=(224, 242, 254, 255), width=2)
    # Subtle ice texture cracks
    d1.line([(x1 + 20, y1 + 15), (x1 + 60, y1 + 45)], fill=(186, 230, 253, 200))
    d1.line([(x1 + 60, y1 + 45), (x1 + 90, y1 + 35)], fill=(186, 230, 253, 200))

# Minish Link sliding on ice
mx, my = 310, 150
# Slide speed trail (momentum blur)
for s in range(5, 0, -1):
    d1.ellipse([mx - s * 8 - 4, my - 4, mx - s * 8 + 4, my + 4], fill=(186, 230, 253, 80 - s * 12))
# Minish Link sprite (tiny 8x8 representation)
d1.ellipse([mx - 6, my - 6, mx + 6, my + 6], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255))
d1.polygon([(mx + 6, my), (mx, my - 9), (mx, my + 9)], fill=(22, 101, 52, 255)) # Green cap facing right
d1.rectangle([mx - 2, my - 2, mx + 2, my + 2], fill=(254, 240, 138, 255))

# Ice Physics Vector HUD annotation
d1.line([(mx, my), (mx + 30, my)], fill=(253, 224, 71, 255), width=2)
d1.polygon([(mx + 35, my), (mx + 28, my - 4), (mx + 28, my + 4)], fill=(253, 224, 71, 255))
d1.text((mx + 10, my - 18), "Vx = 2.4 (Inercia Gelo)", fill=(253, 224, 71, 255), font=font_small)

draw_hud(d1, 0, 0, "TEMPLE OF DROPLETS - SALA 0: ENTRADA GLACIAL",
         "Minish Link infiltra o Templo: deslizamento com atrito reduzido (vx *= 0.94f) em Ice Floes")
canvas.paste(p1, (0, 0))

# -------------------------------------------------------------
# PANEL 2: Chamber 1 - Sunbeam Skylight & Optical Prism Redirect
# -------------------------------------------------------------
p2 = Image.new("RGBA", (W, H), (20, 35, 55, 255))
d2 = ImageDraw.Draw(p2)

# Chamber floor
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (28, 55, 85) if ((x//16 + y//16) % 2 == 0) else (22, 45, 70)
        d2.rectangle([x, y, x + 16, y + 16], fill=c)

# Walls
for y in range(0, H, 16):
    for x in range(0, W, 16):
        if x < 32 or x >= W - 32 or y < 32 or y >= H - 32:
            d2.rectangle([x, y, x + 16, y + 16], fill=(40, 85, 125), outline=(75, 140, 190))

# Large Central Ice Sheet
d2.rectangle([64, 64, W - 64, H - 64], fill=(125, 211, 252, 180), outline=(224, 242, 254, 255), width=2)

# Sunbeam skylight beam shining diagonally from top ceiling
beam_poly = [(160, 0), (220, 0), (260, 170), (210, 170)]
d2.polygon(beam_poly, fill=(254, 240, 138, 90))
d2.polygon([(180, 0), (200, 0), (245, 170), (225, 170)], fill=(255, 255, 255, 130))

# Optical Rotatable Prism Lever at (235, 170)
px, py = 235, 170
d2.polygon([(px, py - 14), (px - 12, py + 10), (px + 12, py + 10)], fill=(245, 158, 11, 255), outline=(254, 240, 138, 255), width=2)
d2.ellipse([px - 4, py - 4, px + 4, py + 4], fill=(255, 255, 255, 255))

# Redirected concentrated beam to east ice block
d2.polygon([(px + 6, py - 2), (360, 120), (360, 150), (px + 6, py + 6)], fill=(254, 240, 138, 160))
d2.line([(px + 6, py + 2), (360, 135)], fill=(255, 255, 255, 255), width=3)

# Melting Ice Block containing Small Key at (360, 135)
ix, iy = 360, 135
d2.rectangle([ix - 16, iy - 16, ix + 16, iy + 16], fill=(186, 230, 253, 160), outline=(254, 240, 138, 255), width=2)
# Golden Key shining inside
d2.rectangle([ix - 4, iy - 6, ix + 6, iy - 3], fill=(245, 158, 11, 255))
d2.rectangle([ix + 4, iy - 3, ix + 6, iy + 6], fill=(245, 158, 11, 255))
# Steam particles
d2.ellipse([ix - 10, iy - 26, ix - 4, iy - 20], fill=(224, 242, 254, 180))
d2.ellipse([ix + 4, iy - 30, ix + 12, iy - 22], fill=(224, 242, 254, 180))

# Link standing near lever
lx, ly = 210, 185
d2.ellipse([lx - 6, ly - 6, lx + 6, ly + 6], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255))
d2.polygon([(lx, ly - 10), (lx - 5, ly - 3), (lx + 5, ly - 3)], fill=(22, 101, 52, 255))

draw_hud(d2, W, 0, "TEMPLE OF DROPLETS - SALA 1: FACHO SOLAR & PRISMA",
         "Prisma solar girado com [A]: Facho solar concentrado derrete o gelo e libera a Chave Pequena!", keys=1)
canvas.paste(p2, (W, 0))

# -------------------------------------------------------------
# PANEL 3: Chamber 2 - Pitch-Black Cavern & Flame Lantern 4-Torch Puzzle
# -------------------------------------------------------------
p3_base = Image.new("RGBA", (W, H), (10, 15, 22, 255))
d3_base = ImageDraw.Draw(p3_base)

# Cavern floor
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (25, 30, 40) if ((x//16 + y//16) % 2 == 0) else (18, 22, 30)
        d3_base.rectangle([x, y, x + 16, y + 16], fill=c)

# Perimeter walls
for y in range(0, H, 16):
    for x in range(0, W, 16):
        if x < 32 or x >= W - 32 or y < 32 or y >= H - 32:
            d3_base.rectangle([x, y, x + 16, y + 16], fill=(20, 25, 35), outline=(35, 45, 60))

# 4 Stone Torches at coordinates:
# Top-Left (90, 80), Top-Right (390, 80), Bottom-Left (90, 240), Bottom-Right (390, 240)
torches = [(90, 80), (390, 80), (90, 240), (390, 240)]
for tx, ty in torches:
    # Stone pedestal
    d3_base.rectangle([tx - 10, ty - 6, tx + 10, ty + 14], fill=(71, 85, 105), outline=(100, 116, 139))
    # Roaring orange fire
    d3_base.polygon([(tx, ty - 20), (tx - 8, ty - 6), (tx + 8, ty - 6)], fill=(239, 68, 68, 255))
    d3_base.polygon([(tx, ty - 16), (tx - 5, ty - 6), (tx + 5, ty - 6)], fill=(249, 115, 22, 255))
    d3_base.ellipse([tx - 3, ty - 12, tx + 3, ty - 6], fill=(254, 240, 138, 255))

# North Iron Gate unsealed & Boss Key Chest revealed
d3_base.rectangle([W//2 - 28, 30, W//2 + 28, 48], fill=(15, 23, 42), outline=(56, 189, 248, 255), width=2)
# Ornate Gold Boss Key Chest
cx, cy = W//2, 85
d3_base.rectangle([cx - 14, cy - 10, cx + 14, cy + 10], fill=(217, 119, 6, 255), outline=(254, 240, 138, 255), width=2)
d3_base.ellipse([cx - 3, cy - 2, cx + 3, cy + 4], fill=(15, 23, 42, 255))

# Link standing in center holding Flame Lantern
lx, ly = 240, 180
d3_base.ellipse([lx - 7, ly - 7, lx + 7, ly + 7], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255))
d3_base.polygon([(lx, ly - 12), (lx - 6, ly - 3), (lx + 6, ly - 3)], fill=(22, 101, 52, 255))
# Flame Lantern in Link's hand
d3_base.rectangle([lx + 6, ly - 2, lx + 14, ly + 8], fill=(180, 83, 9, 255))
d3_base.ellipse([lx + 8, ly + 1, lx + 12, ly + 6], fill=(254, 240, 138, 255))

# Apply Dynamic Lighting Mask (Overlay torch lights + Link's lantern)
mask3 = Image.new("L", (W, H), int(255 * 0.10)) # 10% ambient dark
dmask3 = ImageDraw.Draw(mask3)

# Radial gradient from Link's lantern
for r in range(90, 0, -2):
    f = 1.0 - (r / 90.0)
    v = int(255 * (0.10 + 0.90 * (f * f)))
    dmask3.ellipse([lx - r, ly - r, lx + r, ly + r], fill=v)

# Radial gradients from all 4 lit torches
for tx, ty in torches:
    for r in range(75, 0, -3):
        f = 1.0 - (r / 75.0)
        v = int(255 * (0.10 + 0.90 * (f * f)))
        # Paint max illumination
        cur = mask3.getpixel((tx, ty))
        dmask3.ellipse([tx - r, ty - r, tx + r, ty + r], fill=max(v, cur))

# Composite lit scene
p3 = Image.new("RGBA", (W, H), (0, 0, 0, 255))
for y in range(H):
    for x in range(W):
        m = mask3.getpixel((x, y)) / 255.0
        r, g, b, a = p3_base.getpixel((x, y))
        # Warm amber tint for lit areas
        nr = min(255, int(r * m * 1.15))
        ng = min(255, int(g * m * 1.05))
        nb = min(255, int(b * m * 0.90))
        p3.putpixel((x, y), (nr, ng, nb, 255))

d3 = ImageDraw.Draw(p3)
draw_hud(d3, 0, H, "TEMPLE OF DROPLETS - SALA 2: CAVERNA ESCURA & 4 TOCHAS",
         "4 Tochas acesas com a Flame Lantern: Portao do Mestre aberto e BOSS KEY conquistada!", boss_key=True)
canvas.paste(p3, (0, H))

# -------------------------------------------------------------
# PANEL 4: Chamber 5 - Glacial Boss Arena: Big Octorok Battle & Water Element Thaw
# -------------------------------------------------------------
p4 = Image.new("RGBA", (W, H), (16, 32, 54, 255))
d4 = ImageDraw.Draw(p4)

# Pure crystalline ice floor
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (30, 65, 100) if ((x//16 + y//16) % 2 == 0) else (24, 52, 85)
        d4.rectangle([x, y, x + 16, y + 16], fill=c)
        d4.line([(x, y), (x + 16, y + 16)], fill=(50, 95, 140))

# Glacial walls with stalagmites
for y in range(0, H, 16):
    for x in range(0, W, 16):
        if x < 32 or x >= W - 32 or y < 32 or y >= H - 32:
            d4.rectangle([x, y, x + 16, y + 16], fill=(45, 95, 145), outline=(125, 211, 252))

# Big Octorok Boss at (240, 130)
bx, by = 240, 130
# Massive circular body (Blue glacial octopus)
d4.ellipse([bx - 36, by - 36, bx + 36, by + 36], fill=(30, 58, 138, 255), outline=(56, 189, 248, 255), width=3)
# Underbelly / mouth snout
d4.ellipse([bx - 14, by + 12, bx + 14, by + 34], fill=(15, 23, 42, 255), outline=(125, 211, 252, 255), width=2)
# Glowing angry red eyes
d4.ellipse([bx - 20, by - 12, bx - 8, by], fill=(239, 68, 68, 255), outline=(255, 255, 255, 255))
d4.ellipse([bx + 8, by - 12, bx + 20, by], fill=(239, 68, 68, 255), outline=(255, 255, 255, 255))
d4.ellipse([bx - 16, by - 8, bx - 12, by - 4], fill=(255, 255, 255, 255))
d4.ellipse([bx + 12, by - 8, bx + 16, by - 4], fill=(255, 255, 255, 255))

# Tail behind Big Octorok: Burning flame phase!
tx, ty = bx, by - 42
for fi in range(7):
    fx = tx - 18 + fi * 6
    d4.polygon([(fx, ty - 18), (fx - 6, ty + 6), (fx + 6, ty + 6)], fill=(239, 68, 68, 255))
    d4.polygon([(fx, ty - 12), (fx - 4, ty + 6), (fx + 4, ty + 6)], fill=(249, 115, 22, 255))
    d4.ellipse([fx - 3, ty - 6, fx + 3, ty + 2], fill=(254, 240, 138, 255))
d4.text((tx - 40, ty - 32), "[TAIL ON FIRE!]", fill=(253, 224, 71, 255), font=font_small)

# Reflected Rock moving back toward Big Octorok's snout
rx, ry = 240, 195
d4.ellipse([rx - 8, ry - 8, rx + 8, ry + 8], fill=(254, 240, 138, 255), outline=(245, 158, 11, 255), width=2)
d4.line([(rx, ry + 12), (rx, ry + 35)], fill=(253, 224, 71, 255), width=2)
d4.text((rx + 12, ry - 6), "Pedra Rebatida (Deflected) -> Stuns Boss", fill=(253, 224, 71, 255), font=font_small)

# Link slashing with White Sword
lx, ly = 240, 245
d4.ellipse([lx - 8, lx - 8, lx + 8, lx + 8], fill=(34, 197, 94, 255)) # Cap
# Sword swing arc
d4.arc([lx - 25, ly - 25, lx + 25, ly + 25], start=210, end=330, fill=(255, 255, 255, 255), width=3)
d4.text((lx - 35, ly + 14), "Link (White Sword)", fill=(224, 242, 254, 255), font=font_small)

# Floating Sacred Rewards in Center-North: Heart Container & Glowing Water Element
w_cx, w_cy = 100, 80
# Heart container
d4.polygon([(w_cx, w_cy - 12), (w_cx - 14, w_cy - 2), (w_cx, w_cy + 14), (w_cx + 14, w_cy - 2)], fill=(239, 68, 68, 255), outline=(254, 240, 138, 255), width=2)
d4.text((w_cx - 40, w_cy + 18), "Heart Container (+1)", fill=(254, 240, 138, 255), font=font_small)

# Sacred Water Element (Deep sapphire drop with cyan aura)
el_x, el_y = 380, 80
for ra in range(24, 0, -2):
    fa = 1.0 - (ra / 24.0)
    d4.ellipse([el_x - ra, el_y - ra, el_x + ra, el_y + ra], fill=(6, 182, 212, int(160 * fa)))
# Water droplet silhouette
d4.polygon([(el_x, el_y - 18), (el_x - 12, el_y + 4), (el_x, el_y + 16), (el_x + 12, el_y + 4)], fill=(14, 165, 233, 255), outline=(224, 242, 254, 255), width=2)
d4.ellipse([el_x - 5, el_y - 2, el_x + 1, el_y + 6], fill=(255, 255, 255, 200))
d4.text((el_x - 42, el_y + 20), "WATER ELEMENT!", fill=(56, 189, 248, 255), font=font_small)

draw_hud(d4, W, H, "TEMPLE OF DROPLETS - SALA 5: BIG OCTOROK & ELEMENTO AGUA",
         "Big Octorok derrotado! Recipiente de Coracao e o sagrado ELEMENTO DA AGUA conquistados!",
         keys=0, boss_key=True, water_element=True)
canvas.paste(p4, (W, H))

# Save output
out_scratch = "scratch/temple_of_droplets_showcase.png"
canvas.save(out_scratch)
print(f"[OK] Showcase gerada com sucesso em: {out_scratch}")

artifact_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
if os.path.isdir(artifact_dir):
    out_artifact = os.path.join(artifact_dir, "temple_of_droplets_showcase.png")
    shutil.copyfile(out_scratch, out_artifact)
    print(f"[OK] Showcase copiada para o diretorio de artefatos: {out_artifact}")
