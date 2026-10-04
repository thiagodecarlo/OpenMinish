#!/usr/bin/env python3
"""
scratch/verify_lake_library.py
Generates lake_hylia_library_showcase.png showcasing:
1. Royal Hyrule Library & 3-Book Staircase for Minish Link
2. Elder Librari atop Bookshelf & Temple Passage Unlock
3. Lake Hylia shoreline, Mayor Hagen's Cabin, Island with Stump & Wind Crest
4. Frozen Cavern Entrance to Temple of Droplets
"""

import math
import os
import shutil
from PIL import Image, ImageDraw, ImageFont

W, H = 480, 320
IMG_W, IMG_H = W * 2, H * 2
canvas = Image.new("RGBA", (IMG_W, IMG_H), (10, 10, 14, 255))
draw = ImageDraw.Draw(canvas)

# Fonts
try:
    font_title = ImageFont.truetype("arial.ttf", 16)
    font_body = ImageFont.truetype("arial.ttf", 13)
    font_small = ImageFont.truetype("arial.ttf", 11)
except:
    font_title = font_body = font_small = ImageFont.load_default()

def draw_hud(pdraw, ox, oy, title, status_text):
    # Top HUD bar
    pdraw.rectangle([ox, oy, ox + W, oy + 24], fill=(16, 24, 20, 255))
    # Hearts
    for i in range(5):
        hx = ox + 8 + i * 14
        hy = oy + 6
        pdraw.polygon([(hx+5, hy+1), (hx+2, hy+4), (hx+2, hy+7), (hx+5, hy+10),
                       (hx+8, hy+7), (hx+8, hy+4)], fill=(239, 68, 68, 255))
        pdraw.polygon([(hx+8, hy+7), (hx+11, hy+4), (hx+11, hy+1), (hx+8, hy+1)],
                      fill=(239, 68, 68, 255))
    # Rupees
    rx = ox + 90
    pdraw.polygon([(rx+5, oy+4), (rx+9, oy+10), (rx+5, oy+16), (rx+1, oy+10)], fill=(34, 197, 94, 255))
    pdraw.text((rx + 14, oy + 4), "185", fill=(255, 255, 255, 255), font=font_small)

    # Title & Subtitle
    pdraw.text((ox + 160, oy + 4), title, fill=(253, 224, 71, 255), font=font_small)
    # Bottom banner
    pdraw.rectangle([ox + 40, oy + H - 24, ox + W - 40, oy + H - 6], fill=(20, 20, 30, 220), outline=(212, 175, 55, 255))
    pdraw.text((ox + 50, oy + H - 22), status_text, fill=(240, 240, 240, 255), font=font_small)

# -------------------------------------------------------------
# PANEL 1: Royal Hyrule Library & Giant Book Staircase
# -------------------------------------------------------------
p1x, p1y = 0, 0
# Parquet floor
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (146, 92, 42) if ((x//16 + y//16) % 2 == 0) else (130, 80, 34)
        draw.rectangle([p1x + x, p1y + y, p1x + x + 16, p1y + y + 16], fill=c)
        draw.line([(p1x + x, p1y + y), (p1x + x + 16, p1y + y)], fill=(110, 65, 25))

# Central ornate carpet
draw.rectangle([p1x + 80, p1y + 60, p1x + 400, p1y + 260], fill=(139, 26, 26, 255), outline=(212, 175, 55, 255), width=3)
draw.rectangle([p1x + 88, p1y + 68, p1x + 392, p1y + 252], fill=(160, 36, 36, 255), outline=(190, 150, 45, 255), width=2)

# Grand Bookshelves along back wall
for bx in range(32, W - 32, 64):
    draw.rectangle([p1x + bx, p1y + 30, p1x + bx + 56, p1y + 90], fill=(85, 45, 18, 255), outline=(40, 20, 8, 255), width=2)
    # Shelf rows with books
    for sy in [48, 66, 84]:
        draw.line([(p1x + bx + 2, p1y + sy), (p1x + bx + 54, p1y + sy)], fill=(50, 25, 10, 255), width=2)
        # Colorful books
        book_colors = [(190, 40, 40), (40, 120, 190), (40, 160, 70), (200, 160, 40), (140, 50, 160)]
        for k, c in enumerate(book_colors):
            draw.rectangle([p1x + bx + 4 + k * 10, p1y + sy - 14, p1x + bx + 12 + k * 10, p1y + sy], fill=c)

# 3 Colossal Books stacked as Staircase for Minish Link
# Book 1: Red (A Hyrulean Bestiary) - Base step
draw.rectangle([p1x + 220, p1y + 130, p1x + 280, p1y + 155], fill=(185, 28, 28, 255), outline=(245, 158, 11, 255), width=2)
draw.text((p1x + 226, p1y + 136), "BESTIARY", fill=(255, 255, 255, 255), font=font_small)

# Book 2: Blue (Legend of the Picori) - Middle step
draw.rectangle([p1x + 235, p1y + 105, p1x + 295, p1y + 130], fill=(29, 78, 216, 255), outline=(147, 197, 253, 255), width=2)
draw.text((p1x + 242, p1y + 111), "PICORI", fill=(255, 255, 255, 255), font=font_small)

# Book 3: Green (History of Masks) - Top step reaching shelf
draw.rectangle([p1x + 250, p1y + 80, p1x + 310, p1y + 105], fill=(21, 128, 61, 255), outline=(134, 239, 172, 255), width=2)
draw.text((p1x + 258, p1y + 86), "MASKS", fill=(255, 255, 255, 255), font=font_small)

# Minish Link climbing the staircase! (tiny 8x8 sprite)
lx, ly = p1x + 265, p1y + 70
draw.ellipse([lx, ly, lx + 8, ly + 8], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255))
draw.polygon([(lx + 4, ly - 5), (lx + 1, ly + 1), (lx + 7, ly + 1)], fill=(22, 101, 52, 255))

# Elder Librari on top of the shelf
ex, ey = p1x + 290, p1y + 40
draw.ellipse([ex, ey, ex + 14, ey + 14], fill=(245, 158, 11, 255))
draw.polygon([(ex + 7, ey - 8), (ex + 1, ey + 2), (ex + 13, ey + 2)], fill=(185, 28, 28, 255))
draw.rectangle([ex + 2, ey + 4, ex + 12, ey + 8], fill=(255, 255, 255, 255)) # Beard & Glasses

draw_hud(draw, p1x, p1y, "HYRULE ROYAL LIBRARY", "ESCADA DE 3 LIVROS FORMADA: LINK MINISH ALCANCE O SABIO LIBRARI")

# -------------------------------------------------------------
# PANEL 2: Elder Librari Dialogue & Secret Passage
# -------------------------------------------------------------
p2x, p2y = W, 0
# Background: Shelf top close-up with open manuscripts & dust motes
draw.rectangle([p2x, p2y, p2x + W, p2y + H], fill=(42, 26, 16, 255))
# Huge open tome
draw.polygon([(p2x + 120, p2y + 60), (p2x + 240, p2y + 80), (p2x + 240, p2y + 190), (p2x + 120, p2y + 170)], fill=(245, 240, 220, 255), outline=(120, 90, 50, 255), width=2)
draw.polygon([(p2x + 360, p2y + 60), (p2x + 240, p2y + 80), (p2x + 240, p2y + 190), (p2x + 360, p2y + 170)], fill=(250, 245, 230, 255), outline=(120, 90, 50, 255), width=2)
# Ancient Picori runes
for ry in range(85, 160, 12):
    draw.line([(p2x + 140, p2y + ry), (p2x + 225, p2y + ry + 3)], fill=(100, 80, 60, 255), width=2)
    draw.line([(p2x + 255, p2y + ry + 3), (p2x + 340, p2y + ry)], fill=(100, 80, 60, 255), width=2)

# Elder Librari Detailed Pixel Portrait
px, py = p2x + 50, p2y + 60
draw.rectangle([px, py, px + 56, py + 70], fill=(28, 18, 12, 255), outline=(212, 175, 55, 255), width=2)
# Picori red wizard hat
draw.polygon([(px + 28, py + 4), (px + 10, py + 26), (px + 46, py + 26)], fill=(220, 38, 38, 255))
# Round head & spectacles
draw.ellipse([px + 16, py + 24, px + 40, py + 48], fill=(254, 215, 170, 255))
draw.ellipse([px + 20, py + 30, px + 27, py + 37], outline=(202, 138, 4, 255), width=2)
draw.ellipse([px + 29, py + 30, px + 36, py + 37], outline=(202, 138, 4, 255), width=2)
draw.line([(px + 27, py + 33), (px + 29, py + 33)], fill=(202, 138, 4, 255), width=2)
# White sage beard
draw.polygon([(px + 18, py + 40), (px + 38, py + 40), (px + 28, py + 64)], fill=(245, 245, 245, 255))

# Dialogue Box
draw.rectangle([p2x + 30, p2y + 200, p2x + W - 30, p2y + 280], fill=(15, 23, 42, 240), outline=(212, 175, 55, 255), width=2)
draw.text((p2x + 45, p2y + 208), "ANCIAO LIBRARI:", fill=(253, 224, 71, 255), font=font_title)
draw.text((p2x + 45, p2y + 230), "\"Ho ho! Excelente trabalho devolvendo os 3 livros atrasados!\"", fill=(241, 245, 249, 255), font=font_body)
draw.text((p2x + 45, p2y + 250), "\"O Elemento da Agua jaz no Templo das Gotas no Lago Hylia!\"", fill=(147, 197, 253, 255), font=font_body)

draw_hud(draw, p2x, p2y, "SABEDORIA MINISH", "LIBRARI REVELOU A PASSAGEM SECRETA PARA O TEMPLO DAS GOTAS")

# -------------------------------------------------------------
# PANEL 3: Lake Hylia Overworld (Shoreline, Cabin & Island)
# -------------------------------------------------------------
p3x, p3y = 0, H
# Deep Water gradient
for y in range(0, H, 8):
    c = (14, int(116 + y*0.2), int(180 + y*0.15))
    draw.rectangle([p3x, p3y + y, p3x + W, p3y + y + 8], fill=c)

# Shoreline / Grass left side
draw.polygon([(p3x, p3y), (p3x + 120, p3y), (p3x + 100, p3y + 140), (p3x + 140, p3y + 240), (p3x + 90, p3y + H), (p3x, p3y + H)], fill=(74, 150, 48, 255))
draw.line([(p3x + 120, p3y), (p3x + 100, p3y + 140), (p3x + 140, p3y + 240), (p3x + 90, p3y + H)], fill=(217, 185, 120, 255), width=4)

# Wooden Dock
draw.rectangle([p3x + 90, p3y + 110, p3x + 160, p3y + 135], fill=(138, 85, 38, 255), outline=(78, 45, 18, 255), width=2)
for px in range(95, 155, 8):
    draw.line([(p3x + px, p3y + 110), (p3x + px, p3y + 135)], fill=(78, 45, 18, 255))

# Mayor Hagen's Lakeside Cabin
draw.rectangle([p3x + 16, p3y + 180, p3x + 80, p3y + 240], fill=(160, 100, 50, 255), outline=(70, 38, 15, 255), width=2)
draw.polygon([(p3x + 8, p3y + 180), (p3x + 48, p3y + 145), (p3x + 88, p3y + 180)], fill=(185, 55, 38, 255), outline=(100, 25, 15, 255), width=2)
draw.rectangle([p3x + 38, p3y + 210, p3x + 58, p3y + 240], fill=(60, 30, 10, 255)) # Door
draw.text((p3x + 18, p3y + 246), "CABANA DE HAGEN", fill=(255, 255, 255, 255), font=font_small)

# Central Island
draw.ellipse([p3x + 230, p3y + 120, p3x + 370, p3y + 230], fill=(86, 170, 58, 255), outline=(217, 185, 120, 255), width=4)
# Minish Stump on island
draw.ellipse([p3x + 260, p3y + 155, p3x + 290, p3y + 185], fill=(130, 80, 34, 255), outline=(70, 40, 15, 255), width=2)
draw.ellipse([p3x + 266, p3y + 161, p3x + 284, p3y + 179], fill=(180, 120, 60, 255))
# Glowing Wind Crest on island
cx, cy = p3x + 325, p3y + 170
for r in [18, 14, 10, 6]:
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], outline=(6, 182, 212, 200), width=2)
# Swirling wind particles
draw.arc([cx - 16, cy - 16, cx + 16, cy + 16], 45, 220, fill=(165, 243, 252, 255), width=2)
draw.arc([cx - 16, cy - 16, cx + 16, cy + 16], 225, 400, fill=(165, 243, 252, 255), width=2)

# Link swimming in the deep water with Zora's Flippers
sx, sy = p3x + 190, p3y + 80
draw.ellipse([sx - 12, sy - 8, sx + 12, sy + 8], fill=(56, 189, 248, 180)) # Ripple
draw.ellipse([sx - 6, sy - 6, sx + 6, sy + 6], fill=(34, 197, 94, 255)) # Green tunic
draw.ellipse([sx - 3, sy - 10, sx + 3, sy - 4], fill=(254, 215, 170, 255)) # Link head

draw_hud(draw, p3x, p3y, "LAKE HYLIA - GRANDE LAGO", "CRISTA DE VENTO DO LAGO, TOCO MINISH & CABANA DO PREFEITO HAGEN")

# -------------------------------------------------------------
# PANEL 4: Frozen Cavern Entrance to Temple of Droplets
# -------------------------------------------------------------
p4x, p4y = W, H
# Icy Frozen Lake surface
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (186, 230, 253) if ((x//16 + y//16) % 2 == 0) else (224, 242, 254)
        draw.rectangle([p4x + x, p4y + y, p4x + x + 16, p4y + y + 16], fill=c)
        draw.line([(p4x + x, p4y + y), (p4x + x + 16, p4y + y + 16)], fill=(125, 211, 252, 100), width=1)

# Massive Ice Cliff Face at top
draw.rectangle([p4x, p4y, p4x + W, p4y + 110], fill=(147, 197, 253, 255), outline=(59, 130, 246, 255), width=3)
# Icicles hanging down
for ix in range(16, W, 24):
    ih = 20 + int(15 * math.sin(ix * 0.1))
    draw.polygon([(p4x + ix, p4y + 110), (p4x + ix + 10, p4y + 110), (p4x + ix + 5, p4y + 110 + ih)], fill=(240, 249, 255, 255), outline=(96, 165, 250, 255))

# Frozen Cavern Archway (Temple Entrance)
ax, ay = p4x + 200, p4y + 40
draw.ellipse([ax - 40, ay - 10, ax + 40, ay + 70], fill=(30, 58, 138, 255), outline=(191, 219, 254, 255), width=4)
draw.rectangle([ax - 40, ay + 30, ax + 40, ay + 70], fill=(15, 23, 42, 255))

# Shimmering Water Element Crest above cavern
draw.ellipse([ax - 14, ay - 24, ax + 14, ay + 4], fill=(14, 165, 233, 255), outline=(224, 242, 254, 255), width=2)
# Water drop icon
draw.polygon([(ax, ay - 22), (ax - 8, ay - 10), (ax, ay - 4), (ax + 8, ay - 10)], fill=(255, 255, 255, 255))

# Frost sparkles and unlocked entrance glow
for sp in [(ax - 60, ay + 20), (ax + 60, ay + 20), (ax - 20, ay + 90), (ax + 20, ay + 90)]:
    draw.line([(sp[0] - 6, sp[1]), (sp[0] + 6, sp[1])], fill=(255, 255, 255, 255), width=2)
    draw.line([(sp[0], sp[1] - 6), (sp[0], sp[1] + 6)], fill=(255, 255, 255, 255), width=2)

# Minish Link standing before the monumental frozen gate
mlx, mly = ax - 4, ay + 95
draw.ellipse([mlx, mly, mlx + 10, mly + 10], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255))
draw.polygon([(mlx + 5, mly - 7), (mlx + 1, mly + 1), (mlx + 9, mly + 1)], fill=(22, 101, 52, 255))

# Status Box
draw.rectangle([p4x + 60, p4y + 200, p4x + W - 60, p4y + 270], fill=(15, 23, 42, 240), outline=(56, 189, 248, 255), width=2)
draw.text((p4x + 75, p4y + 210), "TEMPLE OF DROPLETS (TEMPLO DAS GOTAS)", fill=(56, 189, 248, 255), font=font_title)
draw.text((p4x + 75, p4y + 232), "Entrada glida destravada pelo Ancio Librari!", fill=(241, 245, 249, 255), font=font_body)
draw.text((p4x + 75, p4y + 250), "Requer: Flame Lantern (Lanterna) para derreter o gelo!", fill=(253, 224, 71, 255), font=font_small)

draw_hud(draw, p4x, p4y, "PORTAL GLACIAL - DUNGEON 4", "TEMPLE OF DROPLETS PRONTO PARA EXPLORACAO COM FLAME LANTERN")

# Grid borders
draw.line([(W, 0), (W, IMG_H)], fill=(212, 175, 55, 255), width=3)
draw.line([(0, H), (IMG_W, H)], fill=(212, 175, 55, 255), width=3)

# Save image
out_local = "scratch/lake_hylia_library_showcase.png"
artifact_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
out_artifact = os.path.join(artifact_dir, "lake_hylia_library_showcase.png")

canvas.save(out_local)
shutil.copyfile(out_local, out_artifact)
print(f"[OK] Showcase saved to {out_local} and {out_artifact}")
