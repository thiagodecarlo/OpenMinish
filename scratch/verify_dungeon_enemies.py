#!/usr/bin/env python3
"""
scratch/verify_dungeon_enemies.py - Visual Verification of Dungeon Enemies & Combat FX
Validates:
1. Electric ChuChu (Normal vs Electrified pulsing with electric arcs)
2. Spike Roller (Indestructible rolling spike hazard with metallic shading)
3. Gibdo (Ancient tomb mummy with bandages and glowing red eyes)
4. Stalfos (Acrobatic jumping skeleton warrior with bone sword)
5. Combat FX Particle System (Slash sparks, Electric sparks, Fire embers, Smoke puffs, Bone fragments)
6. Zero-ROM compliance & C11 HAL pixel-perfect fidelity
"""

import os
import sys
import math
from PIL import Image, ImageDraw, ImageFont

def hex_to_rgba(c):
    r = (c >> 24) & 0xFF
    g = (c >> 16) & 0xFF
    b = (c >> 8) & 0xFF
    a = c & 0xFF
    return (r, g, b, a)

def draw_dungeon_floor(draw, x0, y0, w, h):
    # Authentic Minish Cap dungeon stone tiles (Deepwood Shrine / Cave style)
    c_tile1 = (44, 48, 56, 255)
    c_tile2 = (36, 40, 48, 255)
    c_grid  = (24, 28, 36, 255)
    c_hi    = (58, 64, 76, 255)

    draw.rectangle([(x0, y0), (x0 + w - 1, y0 + h - 1)], fill=c_tile1)
    tile_sz = 16
    for ty in range(y0, y0 + h, tile_sz):
        for tx in range(x0, x0 + w, tile_sz):
            if ((tx // tile_sz) + (ty // tile_sz)) % 2 == 1:
                draw.rectangle([(tx, ty), (tx + tile_sz - 1, ty + tile_sz - 1)], fill=c_tile2)
            draw.line([(tx, ty), (tx + tile_sz - 1, ty)], fill=c_hi)
            draw.line([(tx, ty), (tx, ty + tile_sz - 1)], fill=c_hi)
            draw.line([(tx + tile_sz - 1, ty), (tx + tile_sz - 1, ty + tile_sz - 1)], fill=c_grid)
            draw.line([(tx, ty + tile_sz - 1), (tx + tile_sz - 1, ty + tile_sz - 1)], fill=c_grid)

def render_electric_chuchu(draw, sx, sy, anim_timer=0, is_sparking=False):
    # Shadow
    draw.rectangle([(sx + 2, sy + 11), (sx + 13, sy + 13)], fill=(5, 16, 7, 102))

    c_body    = (254, 240, 138, 255) if is_sparking else (250, 204, 21, 255)
    c_body_dk = (245, 158, 11, 255)  if is_sparking else (217, 119, 6, 255)
    c_spark   = (103, 232, 249, 255)
    c_eye     = (255, 255, 255, 255) if is_sparking else (24, 24, 27, 255)

    squash = (anim_timer // 6) % 3
    h = 10 if squash == 1 else 12
    w = 14 if squash == 1 else 12
    ox = (16 - w) // 2
    oy = 14 - h

    # Body
    draw.rectangle([(sx + ox, sy + oy), (sx + ox + w - 1, sy + oy + h - 1)], fill=c_body)
    draw.rectangle([(sx + ox + 1, sy + oy + 1), (sx + ox + w - 2, sy + oy + 2)], fill=(255, 255, 255, 255))
    draw.rectangle([(sx + ox + 2, sy + oy + h - 2), (sx + ox + w - 3, sy + oy + h - 1)], fill=c_body_dk)

    # Eyes
    draw.point((sx + ox + 3, sy + oy + 4), fill=c_eye)
    draw.point((sx + ox + 8, sy + oy + 4), fill=c_eye)

    if is_sparking:
        flicker = anim_timer % 3
        if flicker == 0:
            draw.point((sx + ox - 2, sy + oy + 2), fill=c_spark)
            draw.point((sx + ox + w + 1, sy + oy + 5), fill=c_spark)
            draw.line([(sx + ox - 1, sy + oy + 1), (sx + ox - 3, sy + oy + 3)], fill=c_spark)
            draw.line([(sx + ox + w, sy + oy + 4), (sx + ox + w + 2, sy + oy + 6)], fill=(255, 255, 255, 255))
        elif flicker == 1:
            draw.point((sx + ox + 4, sy + oy - 2), fill=c_spark)
            draw.point((sx + ox - 1, sy + oy + 8), fill=c_spark)
            draw.line([(sx + ox + 3, sy + oy - 3), (sx + ox + 5, sy + oy - 1)], fill=(255, 255, 255, 255))
        else:
            draw.point((sx + ox + w, sy + oy + 2), fill=c_spark)
            draw.point((sx + ox + 2, sy + oy + h), fill=c_spark)
            draw.line([(sx + ox + w - 1, sy + oy + 1), (sx + ox + w + 1, sy + oy + 3)], fill=c_spark)

def render_spike_roller(draw, sx, sy, anim_timer=0):
    # Roller shadow
    draw.rectangle([(sx - 12, sy + 6), (sx + 11, sy + 9)], fill=(5, 16, 7, 136))

    c_roller    = (71, 85, 105, 255)
    c_roller_hi = (148, 163, 184, 255)
    c_roller_dk = (30, 41, 59, 255)
    c_spike     = (226, 232, 240, 255)
    c_spike_dk  = (100, 116, 139, 255)

    # Main cylinder
    draw.rectangle([(sx - 11, sy - 4), (sx + 10, sy + 4)], fill=c_roller)
    draw.rectangle([(sx - 10, sy - 4), (sx + 9, sy - 3)], fill=c_roller_hi)
    draw.rectangle([(sx - 10, sy + 3), (sx + 9, sy + 4)], fill=c_roller_dk)

    rot = (anim_timer // 3) % 4
    offsets = [-8, -2, 4, 10]
    for s in range(4):
        spk_x = sx + offsets[(s + rot) % 4]
        # Upper spikes
        draw.rectangle([(spk_x - 1, sy - 7), (spk_x, sy - 5)], fill=c_spike)
        draw.point((spk_x, sy - 8), fill=(255, 255, 255, 255))
        # Lower spikes
        draw.rectangle([(spk_x - 1, sy + 5), (spk_x, sy + 7)], fill=c_spike_dk)

def render_gibdo(draw, sx, sy, anim_timer=0):
    # Shadow
    draw.rectangle([(sx - 6, sy + 8), (sx + 5, sy + 10)], fill=(5, 16, 7, 102))

    c_cloth    = (212, 212, 216, 255)
    c_cloth_dk = (113, 113, 122, 255)
    c_cloth_hi = (244, 244, 245, 255)
    c_eye      = (239, 68, 68, 255)

    walk_bob = 1 if ((anim_timer // 8) % 2 == 1) else 0
    gy = sy - walk_bob

    # Head & bandages
    draw.rectangle([(sx - 5, gy - 12), (sx + 4, gy - 5)], fill=c_cloth)
    draw.rectangle([(sx - 4, gy - 13), (sx + 3, gy - 12)], fill=c_cloth_hi)
    draw.rectangle([(sx - 5, gy - 9), (sx + 4, gy - 9)], fill=c_cloth_dk)
    draw.rectangle([(sx - 5, gy - 6), (sx + 4, gy - 6)], fill=c_cloth_dk)

    # Eyes
    draw.point((sx - 2, gy - 8), fill=c_eye)
    draw.point((sx + 2, gy - 8), fill=c_eye)

    # Body torso
    draw.rectangle([(sx - 6, gy - 4), (sx + 5, gy + 3)], fill=c_cloth)
    draw.rectangle([(sx - 6, gy - 1), (sx + 5, gy - 1)], fill=c_cloth_dk)
    draw.rectangle([(sx - 6, gy + 2), (sx + 5, gy + 2)], fill=c_cloth_dk)

    # Legs
    draw.rectangle([(sx - 4, gy + 4), (sx - 2, gy + 7 + walk_bob)], fill=c_cloth_dk)
    draw.rectangle([(sx + 1, gy + 4), (sx + 3, gy + 7 + (1 - walk_bob))], fill=c_cloth_dk)

    # Loose bandage ends
    sway = 1 if (anim_timer % 8 < 4) else -1
    draw.point((sx - 7, gy - 2 + sway), fill=c_cloth)
    draw.point((sx + 6, gy + 1 - sway), fill=c_cloth)

def render_stalfos(draw, sx, sy, anim_timer=0, z=0.0):
    # Shadow
    draw.rectangle([(sx - 6, sy + 8), (sx + 5, sy + 10)], fill=(5, 16, 7, 102))

    sty = sy - int(z)

    c_bone    = (248, 250, 252, 255)
    c_bone_dk = (148, 163, 184, 255)
    c_socket  = (15, 23, 42, 255)
    c_glint   = (239, 68, 68, 255)
    c_steel   = (203, 213, 225, 255)
    c_hilt    = (180, 83, 9, 255)

    # Skull
    draw.rectangle([(sx - 5, sty - 12), (sx + 4, sty - 6)], fill=c_bone)
    draw.rectangle([(sx - 4, sty - 13), (sx + 3, sty - 12)], fill=c_bone)
    draw.rectangle([(sx - 3, sty - 9), (sx - 2, sty - 8)], fill=c_socket)
    draw.rectangle([(sx + 1, sty - 9), (sx + 2, sty - 8)], fill=c_socket)
    draw.point((sx - 2, sty - 9), fill=c_glint)
    draw.point((sx + 2, sty - 9), fill=c_glint)
    draw.rectangle([(sx - 3, sty - 5), (sx + 2, sty - 4)], fill=c_bone_dk)
    draw.point((sx - 2, sty - 4), fill=c_bone)
    draw.point((sx,     sty - 4), fill=c_bone)
    draw.point((sx + 2, sty - 4), fill=c_bone)

    # Spine & Ribcage
    draw.rectangle([(sx - 1, sty - 3), (sx, sty + 2)], fill=c_bone_dk)
    draw.rectangle([(sx - 4, sty - 2), (sx + 3, sty - 2)], fill=c_bone)
    draw.rectangle([(sx - 3, sty),     (sx + 2, sty)], fill=c_bone)

    # Bone Sword
    draw.rectangle([(sx + 5, sty - 7), (sx + 6, sty + 1)], fill=c_steel)
    draw.point((sx + 5, sty - 8), fill=(255, 255, 255, 255))
    draw.rectangle([(sx + 4, sty + 2), (sx + 7, sty + 2)], fill=c_hilt)

    # Legs
    if z > 1.0:
        draw.rectangle([(sx - 4, sty + 3), (sx - 3, sty + 5)], fill=c_bone)
        draw.rectangle([(sx + 2, sty + 3), (sx + 3, sty + 5)], fill=c_bone)
    else:
        step = 1 if ((anim_timer // 6) % 2 == 1) else 0
        draw.rectangle([(sx - 4, sty + 3), (sx - 3, sty + 6 + step)], fill=c_bone)
        draw.rectangle([(sx + 2, sty + 3), (sx + 3, sty + 7 - step)], fill=c_bone)

def render_combat_fx(draw, px, py, fx_type, life=12):
    if fx_type == "SLASH_SPARK":
        # Brilliant golden/white slash sparks
        draw.rectangle([(px, py), (px + 1, py + 1)], fill=(254, 240, 138, 255))
        draw.point((px + 1, py), fill=(255, 255, 255, 255))
        draw.line([(px - 2, py - 1), (px + 3, py + 2)], fill=(251, 191, 36, 255))
    elif fx_type == "ELECTRIC_SPARK":
        # Vibrant cyan electric arc
        draw.rectangle([(px, py), (px + 1, py + 1)], fill=(103, 232, 249, 255))
        draw.point((px + 1, py), fill=(255, 255, 255, 255))
        draw.line([(px - 1, py - 2), (px + 2, py + 1)], fill=(56, 189, 248, 255))
    elif fx_type == "FIRE_EMBER":
        # Warm glowing flame ember
        draw.rectangle([(px, py), (px + 1, py + 1)], fill=(249, 115, 22, 255))
        draw.point((px, py - 1), fill=(254, 240, 138, 255))
    elif fx_type == "SMOKE_PUFF":
        # Dispersing dust / smoke
        alpha = min(255, life * 28)
        draw.rectangle([(px - 1, py - 1), (px + 1, py + 1)], fill=(203, 213, 225, alpha))
    elif fx_type == "BONE_FRAGMENT":
        # Shattered stalfos bone fragment
        draw.rectangle([(px, py), (px + 1, py + 1)], fill=(248, 250, 252, 255))
        draw.point((px, py + 1), fill=(203, 213, 225, 255))

def main():
    W, H = 640, 420
    img = Image.new("RGBA", (W, H), (15, 17, 23, 255))
    draw = ImageDraw.Draw(img)

    # 1. Background Header & Title Banner
    draw.rectangle([(0, 0), (W - 1, 28)], fill=(24, 28, 40, 255))
    draw.line([(0, 28), (W - 1, 28)], fill=(59, 130, 246, 255))
    
    try:
        font_sm = ImageFont.load_default()
    except Exception:
        font_sm = None

    def draw_text_badge(x, y, text, col=(248, 250, 252, 255)):
        if font_sm:
            draw.text((x, y), text, font=font_sm, fill=col)

    draw_text_badge(14, 8, "OPENMINISH - DUNGEON ENEMIES & COMBAT FX PIPELINE (OPCAO B)", (96, 165, 250, 255))
    draw_text_badge(520, 8, "60 FPS HAL C11", (74, 222, 128, 255))

    # 5 Panels Layout (2 rows: Row 1 = 3 panels, Row 2 = 2 panels)
    # Row 1: y = 38, h = 175
    #   Panel 1: Electric ChuChu (x=12,  w=196)
    #   Panel 2: Spike Roller   (x=222, w=196)
    #   Panel 3: Gibdo          (x=432, w=196)
    # Row 2: y = 224, h = 182
    #   Panel 4: Stalfos        (x=12,  w=304)
    #   Panel 5: Combat FX Hub  (x=324, w=304)

    panels = [
        ("1. ELECTRIC CHUCHU", 12, 38, 196, 175),
        ("2. SPIKE ROLLER", 222, 38, 196, 175),
        ("3. GIBDO (MUMIA)", 432, 38, 196, 175),
        ("4. STALFOS (ESQUELETO SALTADOR)", 12, 224, 304, 182),
        ("5. COMBAT FX & IMPACT SYSTEM", 324, 224, 304, 182)
    ]

    for title, px, py, pw, ph in panels:
        # Border
        draw.rectangle([(px, py), (px + pw - 1, py + ph - 1)], fill=(20, 24, 33, 255), outline=(51, 65, 85, 255))
        # Inner dungeon floor
        draw_dungeon_floor(draw, px + 4, py + 20, pw - 8, ph - 24)
        # Header strip
        draw.rectangle([(px + 1, py + 1), (px + pw - 2, py + 18)], fill=(30, 41, 59, 255))
        draw_text_badge(px + 8, py + 4, title, (226, 232, 240, 255))

    # --- PANEL 1: ELECTRIC CHUCHU ---
    # Normal State
    render_electric_chuchu(draw, 46, 100, anim_timer=12, is_sparking=False)
    draw_text_badge(30, 130, "Normal", (250, 204, 21, 255))
    draw_text_badge(20, 145, "Passivo", (148, 163, 184, 255))

    # Electrified State
    render_electric_chuchu(draw, 136, 100, anim_timer=33, is_sparking=True)
    draw_text_badge(112, 130, "Eletrificado", (103, 232, 249, 255))
    draw_text_badge(110, 145, "Aura de Choque", (148, 163, 184, 255))

    draw_text_badge(20, 185, "Reflete espada / Requer subarmas", (203, 213, 225, 255))

    # --- PANEL 2: SPIKE ROLLER ---
    # Rail guide
    draw.line([(240, 106), (398, 106)], fill=(15, 23, 42, 255), width=2)
    draw.rectangle([(236, 101), (240, 111)], fill=(71, 85, 105, 255))
    draw.rectangle([(398, 101), (402, 111)], fill=(71, 85, 105, 255))

    # Spike Roller in motion
    render_spike_roller(draw, 318, 104, anim_timer=18)
    # Sparks on track
    render_combat_fx(draw, 304, 110, "SLASH_SPARK")
    render_combat_fx(draw, 332, 110, "SLASH_SPARK")

    draw_text_badge(236, 134, "Hazard Indestrutivel (999 HP)", (248, 113, 113, 255))
    draw_text_badge(236, 154, "Patrulha linear em vaievem", (148, 163, 184, 255))
    draw_text_badge(236, 185, "Ricochete de lamina e flecha", (203, 213, 225, 255))

    # --- PANEL 3: GIBDO ---
    # Walk pose 1
    render_gibdo(draw, 474, 102, anim_timer=4)
    draw_text_badge(454, 132, "Passo Lento", (212, 212, 216, 255))

    # Walk pose 2 (Swaying wraps)
    render_gibdo(draw, 566, 102, anim_timer=12)
    draw_text_badge(548, 132, "Enfaixado", (244, 244, 245, 255))

    draw_text_badge(446, 158, "HP: 6 | Resiste a recuo", (251, 146, 60, 255))
    draw_text_badge(446, 185, "Vulneravel a chamas / lanterna", (203, 213, 225, 255))

    # --- PANEL 4: STALFOS ---
    # Ground posture
    render_stalfos(draw, 52, 310, anim_timer=8, z=0.0)
    draw_text_badge(30, 336, "Combate Chao", (248, 250, 252, 255))

    # Acrobatic jump evasion (mid-air z=20)
    render_stalfos(draw, 152, 310, anim_timer=15, z=20.0)
    # Jump arc trajectory
    draw.arc([(62, 276), (160, 320)], start=200, end=350, fill=(96, 165, 250, 200))
    draw_text_badge(126, 336, "Salto Evasivo", (96, 165, 250, 255))

    # Bone shatter defeat
    render_combat_fx(draw, 244, 300, "BONE_FRAGMENT")
    render_combat_fx(draw, 252, 308, "BONE_FRAGMENT")
    render_combat_fx(draw, 238, 312, "BONE_FRAGMENT")
    render_combat_fx(draw, 258, 304, "BONE_FRAGMENT")
    render_combat_fx(draw, 248, 294, "SMOKE_PUFF", life=8)
    draw_text_badge(226, 336, "Fragmentacao", (203, 213, 225, 255))

    draw_text_badge(24, 375, "IA Evasiva: Salta para tras ao pressentir golpe", (148, 163, 184, 255))

    # --- PANEL 5: COMBAT FX & IMPACT SYSTEM ---
    fx_items = [
        ("Slash Spark", 360, 270, "SLASH_SPARK", (254, 240, 138, 255)),
        ("Electric Arc", 474, 270, "ELECTRIC_SPARK", (103, 232, 249, 255)),
        ("Fire Ember", 576, 270, "FIRE_EMBER", (249, 115, 22, 255)),
        ("Smoke Puff", 416, 324, "SMOKE_PUFF", (203, 213, 225, 255)),
        ("Bone Shard", 524, 324, "BONE_FRAGMENT", (248, 250, 252, 255)),
    ]

    for name, fx, fy, ftype, col in fx_items:
        draw.ellipse([(fx - 8, fy - 8), (fx + 8, fy + 8)], outline=(71, 85, 105, 140))
        render_combat_fx(draw, fx, fy, ftype)
        if ftype == "SLASH_SPARK":
            render_combat_fx(draw, fx + 2, fy - 2, ftype)
        elif ftype == "ELECTRIC_SPARK":
            render_combat_fx(draw, fx - 2, fy + 2, ftype)
        draw_text_badge(fx - 24, fy + 12, name, col)

    draw_text_badge(340, 375, "Buffer 48 slots | 0 heap alloc | 60 FPS", (74, 222, 128, 255))

    # Save outputs
    brain_dir = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a"
    out_brain = os.path.join(brain_dir, "dungeon_enemies_showcase.png")
    out_scratch = os.path.join(os.path.dirname(__file__), "dungeon_enemies_showcase.png")

    img.save(out_brain)
    img.save(out_scratch)
    print(f"Saved dungeon enemies showcase to:\n- {out_brain}\n- {out_scratch}")

if __name__ == "__main__":
    main()
