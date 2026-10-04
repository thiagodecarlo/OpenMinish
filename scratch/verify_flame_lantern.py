#!/usr/bin/env python3
"""
scratch/verify_flame_lantern.py
Generates flame_lantern_showcase.png showcasing:
1. Dynamic Radial Lighting in Dark Cavern / Pitch-Black Dungeon
2. Igniting Stone Torches & Multi-Source Overlapping Illumination
3. Melting Massive Ice Blocks with Thermal Heat
4. Combat Flame Swing & Burning Cobwebs / Enemies
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

def draw_hud(pdraw, ox, oy, title, status_text, has_lantern=True, lantern_lit=True):
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
    pdraw.text((rx + 14, oy + 4), "215", fill=(255, 255, 255, 255), font=font_small)

    # Item Slot [B] - Flame Lantern Icon
    bx = ox + 140
    pdraw.rectangle([bx, oy + 3, bx + 18, oy + 21], fill=(14, 38, 20, 255), outline=(212, 175, 55, 255))
    pdraw.text((bx + 2, oy + 4), "B", fill=(253, 224, 71, 255), font=font_small)
    # Mini lantern icon in slot
    pdraw.rectangle([bx + 11, oy + 8, bx + 15, oy + 16], fill=(120, 53, 15, 255))
    pdraw.ellipse([bx + 12, oy + 10, bx + 14, oy + 14], fill=(254, 240, 138, 255) if lantern_lit else (60, 60, 60, 255))

    # Title
    pdraw.text((ox + 175, oy + 4), title, fill=(253, 224, 71, 255), font=font_small)

    # Bottom banner
    pdraw.rectangle([ox + 30, oy + H - 24, ox + W - 30, oy + H - 6], fill=(20, 15, 10, 230), outline=(249, 115, 22, 255))
    pdraw.text((ox + 40, oy + H - 22), status_text, fill=(254, 243, 199, 255), font=font_small)

# -------------------------------------------------------------
# PANEL 1: Dynamic Radial Lighting in Dark Cavern / Pitch-Black Dungeon
# -------------------------------------------------------------
p1x, p1y = 0, 0
# Create base scene: Dark stone dungeon floor & brick walls
p1_img = Image.new("RGBA", (W, H), (45, 45, 55, 255))
p1_draw = ImageDraw.Draw(p1_img)
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (55, 50, 60) if ((x//16 + y//16) % 2 == 0) else (40, 38, 48)
        p1_draw.rectangle([x, y, x + 16, y + 16], fill=c)
        p1_draw.line([(x, y), (x + 16, y)], fill=(30, 28, 38))
        p1_draw.line([(x, y), (x, y + 16)], fill=(30, 28, 38))

# Link standing at center
lx, ly = 240, 160
# Green tunic Link
p1_draw.ellipse([lx - 10, ly - 10, lx + 10, ly + 10], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255), width=2)
p1_draw.polygon([(lx, ly - 16), (lx - 6, ly - 6), (lx + 6, ly - 6)], fill=(22, 101, 52, 255))
# Brass lantern held in Link's hand
p1_draw.rectangle([lx + 10, ly - 4, lx + 18, ly + 8], fill=(180, 83, 9, 255), outline=(245, 158, 11, 255))
p1_draw.ellipse([lx + 12, ly - 1, lx + 16, ly + 5], fill=(254, 240, 138, 255))

# Dark lighting mask: radial illumination around Link
# Ambient darkness: 12%
mask_img = Image.new("L", (W, H), int(255 * 0.12))
mask_draw = ImageDraw.Draw(mask_img)
# Draw smooth radial gradient around Link
rad_out = 80
rad_in = 28
for r in range(rad_out, 0, -2):
    if r > rad_in:
        factor = 1.0 - (r - rad_in) / float(rad_out - rad_in)
        val = int(255 * (0.12 + 0.88 * (factor * factor)))
    else:
        val = 255
    mask_draw.ellipse([lx - r, ly - r, lx + r, ly + r], fill=val)

# Apply mask to p1_img
p1_final = Image.new("RGBA", (W, H), (0, 0, 0, 255))
# Blend: multiply RGB by normalized mask value
p1_data = p1_img.load()
mask_data = mask_img.load()
for py in range(H):
    for px in range(W):
        r, g, b, a = p1_data[px, py]
        mv = mask_data[px, py] / 255.0
        # Warm amber tint where lit
        r_boost = 1.0 + 0.14 * (mv - 0.12) / 0.88 if mv > 0.12 else 1.0
        g_boost = 1.0 + 0.05 * (mv - 0.12) / 0.88 if mv > 0.12 else 1.0
        b_boost = 1.0 - 0.08 * (mv - 0.12) / 0.88 if mv > 0.12 else 1.0
        nr = min(255, int(r * mv * r_boost))
        ng = min(255, int(g * mv * g_boost))
        nb = min(255, int(b * mv * b_boost))
        p1_data[px, py] = (nr, ng, nb, 255)

canvas.paste(p1_img, (p1x, p1y))
draw_hud(draw, p1x, p1y, "DARK CAVERN - ILUMINACAO DINAMICA", "FLAME LANTERN ACESA: FEIXE CIRCULAR SUAVE COM VIGNETTE RETRO")

# -------------------------------------------------------------
# PANEL 2: Igniting Stone Torches & Multi-Source Overlapping Illumination
# -------------------------------------------------------------
p2x, p2y = W, 0
p2_img = Image.new("RGBA", (W, H), (45, 45, 55, 255))
p2_draw = ImageDraw.Draw(p2_img)
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (55, 50, 60) if ((x//16 + y//16) % 2 == 0) else (40, 38, 48)
        p2_draw.rectangle([x, y, x + 16, y + 16], fill=c)
        p2_draw.line([(x, y), (x + 16, y)], fill=(30, 28, 38))
        p2_draw.line([(x, y), (x, y + 16)], fill=(30, 28, 38))

# Link at (140, 160)
lx2, ly2 = 140, 160
p2_draw.ellipse([lx2 - 10, ly2 - 10, lx2 + 10, ly2 + 10], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255), width=2)
p2_draw.polygon([(lx2, ly2 - 16), (lx2 - 6, ly2 - 6), (lx2 + 6, ly2 - 6)], fill=(22, 101, 52, 255))

# Flame swing towards right torch
p2_draw.arc([lx2 + 8, ly2 - 12, lx2 + 36, ly2 + 16], 300, 60, fill=(249, 115, 22, 255), width=4)
p2_draw.arc([lx2 + 12, ly2 - 8, lx2 + 32, ly2 + 12], 310, 50, fill=(254, 240, 138, 255), width=2)

# Two Stone Torches:
# Torch 1 (left/middle at 190, 160): Just ignited!
tx1, ty1 = 190, 160
p2_draw.rectangle([tx1 - 6, ty1 + 4, tx1 + 6, ty1 + 16], fill=(100, 116, 139, 255), outline=(51, 65, 85, 255))
p2_draw.ellipse([tx1 - 8, ty1 - 10, tx1 + 8, ty1 + 6], fill=(239, 68, 68, 255))
p2_draw.ellipse([tx1 - 5, ty1 - 7, tx1 + 5, ty1 + 3], fill=(249, 115, 22, 255))
p2_draw.ellipse([tx1 - 2, ty1 - 4, tx1 + 2, ty1 + 1], fill=(254, 240, 138, 255))

# Torch 2 (further right at 340, 130): Second roaring torch!
tx2, ty2 = 340, 130
p2_draw.rectangle([tx2 - 6, ty2 + 4, tx2 + 6, ty2 + 16], fill=(100, 116, 139, 255), outline=(51, 65, 85, 255))
p2_draw.ellipse([tx2 - 8, ty2 - 10, tx2 + 8, ty2 + 6], fill=(239, 68, 68, 255))
p2_draw.ellipse([tx2 - 5, ty2 - 7, tx2 + 5, ty2 + 3], fill=(249, 115, 22, 255))
p2_draw.ellipse([tx2 - 2, ty2 - 4, tx2 + 2, ty2 + 1], fill=(254, 240, 138, 255))

# Unlit Torch 3 (bottom right at 370, 240)
tx3, ty3 = 370, 240
p2_draw.rectangle([tx3 - 6, ty3 + 4, tx3 + 6, ty3 + 16], fill=(71, 85, 105, 255), outline=(30, 41, 59, 255))
p2_draw.rectangle([tx3 - 5, ty3 - 2, tx3 + 5, ty3 + 4], fill=(39, 39, 42, 255)) # Extinguished coal

# Multi-source Lighting calculation
p2_data = p2_img.load()
for py in range(H):
    for px in range(W):
        # Calc distance to Link, Torch 1, Torch 2
        d_link = math.hypot(px - lx2, py - ly2)
        d_t1   = math.hypot(px - tx1, py - ty1)
        d_t2   = math.hypot(px - tx2, py - ty2)

        def calc_light(d, r_out, r_in):
            if d <= r_in: return 1.0
            if d < r_out:
                f = 1.0 - (d - r_in) / float(r_out - r_in)
                return f * f
            return 0.0

        l1 = calc_light(d_link, 75, 25)
        l2 = calc_light(d_t1, 65, 20)
        l3 = calc_light(d_t2, 70, 22)
        total_l = max(l1, l2, l3)
        mv = 0.12 + 0.88 * total_l

        r, g, b, a = p2_data[px, py]
        r_boost = 1.0 + 0.15 * total_l
        g_boost = 1.0 + 0.05 * total_l
        b_boost = 1.0 - 0.08 * total_l
        p2_data[px, py] = (min(255, int(r * mv * r_boost)),
                           min(255, int(g * mv * g_boost)),
                           min(255, int(b * mv * b_boost)), 255)

canvas.paste(p2_img, (p2x, p2y))
draw_hud(draw, p2x, p2y, "SISTEMA MULTI-FONTE DE LUZ", "TOCHAS ACESAS: FUSAO DINAMICA DE FEIXES DE LUZ NO FRAMEBUFFER")

# -------------------------------------------------------------
# PANEL 3: Melting Massive Ice Blocks with Thermal Heat
# -------------------------------------------------------------
p3x, p3y = 0, H
# Glacial Cavern Floor: frosty blue ice tiles
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (186, 230, 253) if ((x//16 + y//16) % 2 == 0) else (224, 242, 254)
        draw.rectangle([p3x + x, p3y + y, p3x + x + 16, p3y + y + 16], fill=c)
        draw.line([(p3x + x, p3y + y), (p3x + x + 16, p3y + y + 16)], fill=(125, 211, 252, 100), width=1)

# Left & Right ice walls
draw.rectangle([p3x, p3y, p3x + 100, p3y + H], fill=(147, 197, 253, 255), outline=(59, 130, 246, 255), width=3)
draw.rectangle([p3x + W - 100, p3y, p3x + W, p3y + H], fill=(147, 197, 253, 255), outline=(59, 130, 246, 255), width=3)

# Link facing right at (160, 160)
lx3, ly3 = p3x + 160, p3y + 160
draw.ellipse([lx3 - 10, ly3 - 10, lx3 + 10, ly3 + 10], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255), width=2)
draw.polygon([(lx3, ly3 - 16), (lx3 - 6, ly3 - 6), (lx3 + 6, ly3 - 6)], fill=(22, 101, 52, 255))

# Blazing flame spray hitting the ice block!
draw.polygon([(lx3 + 12, ly3 - 6), (lx3 + 65, ly3 - 22), (lx3 + 68, ly3 + 22), (lx3 + 12, ly3 + 6)], fill=(249, 115, 22, 200))
draw.polygon([(lx3 + 14, ly3 - 3), (lx3 + 52, ly3 - 14), (lx3 + 54, ly3 + 14), (lx3 + 14, ly3 + 3)], fill=(254, 240, 138, 240))
# Heat sparks
for sp in [(lx3 + 40, ly3 - 18), (lx3 + 55, ly3 + 12), (lx3 + 70, ly3 - 4), (lx3 + 50, ly3 + 26)]:
    draw.ellipse([sp[0] - 2, sp[1] - 2, sp[0] + 2, sp[1] + 2], fill=(239, 68, 68, 255))

# Massive Ice Block 1: Currently MELTING into water and steam!
ibx, iby = p3x + 235, p3y + 140
# Water puddle under melting ice
draw.ellipse([ibx - 8, iby + 24, ibx + 48, iby + 42], fill=(2, 132, 199, 180), outline=(56, 189, 248, 255))
# Shrunk melting ice cube with dripping droplets
draw.rectangle([ibx, iby, ibx + 36, iby + 30], fill=(125, 211, 252, 220), outline=(224, 242, 254, 255), width=2)
draw.line([(ibx + 4, iby + 4), (ibx + 32, iby + 26)], fill=(255, 255, 255, 200), width=2)
# Steam rising
for st in [(ibx + 10, iby - 12), (ibx + 24, iby - 20), (ibx + 18, iby - 32)]:
    draw.ellipse([st[0] - 6, st[1] - 4, st[0] + 6, st[1] + 4], fill=(241, 245, 249, 160))

# Massive Ice Block 2: Intact ice block further up
draw.rectangle([p3x + 230, p3y + 70, p3x + 276, p3y + 116], fill=(56, 189, 248, 240), outline=(224, 242, 254, 255), width=3)
draw.line([(p3x + 235, p3y + 75), (p3x + 271, p3y + 111)], fill=(255, 255, 255, 220), width=2)
draw.text((p3x + 285, p3y + 86), "GELO SOLIDO", fill=(30, 58, 138, 255), font=font_small)
draw.text((p3x + 285, p3y + 150), "DERRETENDO...", fill=(185, 28, 28, 255), font=font_small)

draw_hud(draw, p3x, p3y, "TEMPLE OF DROPLETS PATH - DEGELO", "FLAME LANTERN DERRETE BLOCOS DE GELO MACICOS LIBERANDO PASSAGEM")

# -------------------------------------------------------------
# PANEL 4: Combat & Burning Obstacles (Cobwebs & Enemies)
# -------------------------------------------------------------
p4x, p4y = W, H
# Dungeon floor
for y in range(0, H, 16):
    for x in range(0, W, 16):
        c = (60, 50, 40) if ((x//16 + y//16) % 2 == 0) else (45, 38, 30)
        draw.rectangle([p4x + x, p4y + y, p4x + x + 16, p4y + y + 16], fill=c)

# Stone archway with thick burning cobweb
aw_x, aw_y = p4x + 60, p4y + 80
draw.rectangle([aw_x, aw_y, aw_x + 110, aw_y + 140], fill=(25, 20, 18, 255), outline=(120, 113, 108, 255), width=4)
# Cobweb burning away!
for r in [15, 30, 45]:
    draw.arc([aw_x + 55 - r, aw_y + 70 - r, aw_x + 55 + r, aw_y + 70 + r], 0, 360, fill=(245, 245, 245, 140), width=1)
# Fire eating the cobweb
draw.polygon([(aw_x + 35, aw_y + 55), (aw_x + 65, aw_y + 35), (aw_x + 55, aw_y + 75)], fill=(239, 68, 68, 240))
draw.polygon([(aw_x + 40, aw_y + 60), (aw_x + 60, aw_y + 45), (aw_x + 50, aw_y + 70)], fill=(254, 240, 138, 240))
draw.text((aw_x + 10, aw_y + 148), "TEIA EM CHAMAS!", fill=(249, 115, 22, 255), font=font_small)

# Link attacking monster with flame arc
lx4, ly4 = p4x + 220, p4y + 150
draw.ellipse([lx4 - 10, ly4 - 10, lx4 + 10, ly4 + 10], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255), width=2)
draw.polygon([(lx4, ly4 - 16), (lx4 - 6, ly4 - 6), (lx4 + 6, ly4 - 6)], fill=(22, 101, 52, 255))

# Flame arc striking Keese / ChuChu
draw.arc([lx4 + 8, ly4 - 16, lx4 + 48, ly4 + 20], 310, 50, fill=(239, 68, 68, 255), width=5)
draw.arc([lx4 + 12, ly4 - 12, lx4 + 44, ly4 + 16], 320, 40, fill=(254, 240, 138, 255), width=3)

# Monster: Burning Keese flapping in panic
kx, ky = p4x + 310, p4y + 145
# Bat body
draw.ellipse([kx - 8, ky - 8, kx + 8, ky + 8], fill=(30, 27, 75, 255))
# Wings flapping
draw.polygon([(kx - 8, ky), (kx - 24, ky - 14), (kx - 16, ky + 8)], fill=(49, 46, 129, 255))
draw.polygon([(kx + 8, ky), (kx + 24, ky - 14), (kx + 16, ky + 8)], fill=(49, 46, 129, 255))
# On fire!
draw.ellipse([kx - 6, ky - 12, kx + 6, ky], fill=(249, 115, 22, 220))
draw.ellipse([kx - 3, ky - 9, kx + 3, ky - 2], fill=(254, 240, 138, 240))
draw.text((kx - 20, ky - 26), "DANO DE FOGO: 2 HP", fill=(239, 68, 68, 255), font=font_small)

# Commemorative Banner at bottom center of panel
bw_x, bw_y = p4x + 30, p4y + 220
draw.rectangle([bw_x, bw_y, bw_x + W - 60, bw_y + 55], fill=(30, 13, 3, 240), outline=(249, 115, 22, 255), width=2)
draw.text((bw_x + 50, bw_y + 8), "FLAME LANTERN ADQUIRIDA!", fill=(253, 224, 71, 255), font=font_title)
draw.text((bw_x + 18, bw_y + 32), "Chama Eterna: Ilumina a escuridao e derrete o gelo!", fill=(255, 237, 213, 255), font=font_body)

draw_hud(draw, p4x, p4y, "COMBATE IGNEO & OBSTACULOS", "FLAME LANTERN QUEIMA TEIAS DE ARANHA E CAUSA DANO DE FOGO")

# Grid borders
draw.line([(W, 0), (W, IMG_H)], fill=(249, 115, 22, 255), width=3)
draw.line([(0, H), (IMG_W, H)], fill=(249, 115, 22, 255), width=3)

# Save image
out_local = "scratch/flame_lantern_showcase.png"
artifact_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
out_artifact = os.path.join(artifact_dir, "flame_lantern_showcase.png")

canvas.save(out_local)
shutil.copyfile(out_local, out_artifact)
print(f"[OK] Showcase saved to {out_local} and {out_artifact}")
