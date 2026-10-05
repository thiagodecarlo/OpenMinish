#!/usr/bin/env python3
"""
scratch/verify_graphics_pipeline.py - Visual verification showcase for Authentic Graphics Pipeline

Composites a 4-panel visual comparison and feature showcase:
- Panel 1: Link Expanded 10-Row Master Sprite Sheet (320x320)
- Panel 2: Authentic GBA HUD & Item Icons Atlas (160x80 scaled)
- Panel 3: In-Game Action Simulation (Roll Somesault & Sword Slash with authentic HUD)
- Panel 4: Pause Menu Inventory 4x3 Grid with authentic 16x16 GBA item icons
"""

import os
from PIL import Image, ImageDraw, ImageFont

def render_showcase():
    link_sheet_path = 'assets/regions/link_master.bmp'
    hud_items_path  = 'assets/ui/hud_items.bmp'

    link_sheet = Image.open(link_sheet_path).convert('RGBA')
    hud_items  = Image.open(hud_items_path).convert('RGBA')

    W, H = 960, 720
    canvas = Image.new('RGBA', (W, H), (15, 23, 42, 255))
    draw = ImageDraw.Draw(canvas)

    # Header
    draw.rectangle([0, 0, W, 40], fill=(30, 41, 59, 255))
    draw.text((16, 10), "THE LEGEND OF ZELDA: THE MINISH CAP - AUTHENTIC GRAPHICS PIPELINE", fill=(245, 158, 11, 255))
    draw.text((700, 10), "GBA C11 Native HAL (SDL3)", fill=(56, 189, 248, 255))

    # Panel 1: Link 10-Row Master Sprite Sheet (Left, 440x330)
    p1_x, p1_y, p1_w, p1_h = 20, 55, 440, 310
    draw.rectangle([p1_x, p1_y, p1_x + p1_w, p1_y + p1_h], fill=(10, 15, 26, 255), outline=(51, 65, 85, 255))
    draw.text((p1_x + 10, p1_y + 8), "PANEL 1: LINK 10-ROW MASTER SHEET (320x320)", fill=(16, 185, 129, 255))
    # Subtitle for rows
    draw.text((p1_x + 10, p1_y + 24), "Rows: 0-3 Walk/Idle | 4 Minish | 5 Beacons | 6 Slashes | 7 Rolls | 8 Jumps | 9 Hurt/Swim", fill=(148, 163, 184, 255))
    # Paste scaled sheet
    sheet_scaled = link_sheet.resize((260, 260), Image.NEAREST)
    canvas.paste(sheet_scaled, (p1_x + 90, p1_y + 45), sheet_scaled)

    # Panel 2: Authentic HUD & Item Atlas (Right, 460x310)
    p2_x, p2_y, p2_w, p2_h = 480, 55, 460, 310
    draw.rectangle([p2_x, p2_y, p2_x + p2_w, p2_y + p2_h], fill=(10, 15, 26, 255), outline=(51, 65, 85, 255))
    draw.text((p2_x + 10, p2_y + 8), "PANEL 2: GBA HUD & ITEM ICONS ATLAS (16x16)", fill=(56, 189, 248, 255))
    draw.text((p2_x + 10, p2_y + 24), "Hearts (4 Quarters) | Rupees | Badges [A]/[B] | Swords | Shields | Dungeon & Tools", fill=(148, 163, 184, 255))
    # Scale atlas 3x
    hud_scaled = hud_items.resize((160 * 2, 80 * 2), Image.NEAREST)
    canvas.paste(hud_scaled, (p2_x + 70, p2_y + 60), hud_scaled)

    # Panel 3: In-Game Action Simulation (Bottom-Left, 440x320)
    p3_x, p3_y, p3_w, p3_h = 20, 385, 440, 315
    draw.rectangle([p3_x, p3_y, p3_x + p3_w, p3_y + p3_h], fill=(10, 15, 26, 255), outline=(51, 65, 85, 255))
    draw.text((p3_x + 10, p3_y + 8), "PANEL 3: IN-GAME COMBAT & ACTION RENDER (240x160)", fill=(245, 158, 11, 255))

    # Mini game screen 384x256 (1.6x GBA)
    sim_w, sim_h = 400, 260
    sim = Image.new('RGBA', (sim_w, sim_h), (34, 139, 34, 255)) # Green grass
    sdraw = ImageDraw.Draw(sim)
    # Background dirt path
    sdraw.rectangle([100, 0, 300, sim_h], fill=(194, 155, 96, 255))

    # Authentic HUD at top of game screen
    sdraw.rectangle([0, 0, sim_w, 24], fill=(12, 28, 13, 255))
    # Hearts
    for h in range(4):
        h_icon = hud_items.crop((0, 0, 16, 16)).resize((18, 18), Image.NEAREST)
        sim.paste(h_icon, (6 + h * 18, 3), h_icon)
    # Rupee
    r_icon = hud_items.crop((5 * 16, 0, 6 * 16, 16)).resize((18, 18), Image.NEAREST)
    sim.paste(r_icon, (88, 3), r_icon)
    sdraw.text((108, 6), "150", fill=(0, 255, 136, 255))
    # [A] Badge + Sword
    a_badge = hud_items.crop((8 * 16, 0, 9 * 16, 16)).resize((18, 18), Image.NEAREST)
    sword_icon = hud_items.crop((1 * 16, 1 * 16, 2 * 16, 2 * 16)).resize((18, 18), Image.NEAREST)
    sim.paste(a_badge, (280, 3), a_badge)
    sim.paste(sword_icon, (298, 3), sword_icon)
    # [B] Badge + Bomb
    b_badge = hud_items.crop((9 * 16, 0, 10 * 16, 16)).resize((18, 18), Image.NEAREST)
    bomb_icon = hud_items.crop((2 * 16, 2 * 16, 3 * 16, 3 * 16)).resize((18, 18), Image.NEAREST)
    sim.paste(b_badge, (334, 3), b_badge)
    sim.paste(bomb_icon, (352, 3), bomb_icon)

    # Action 1: Link Somersault Roll
    sdraw.text((20, 45), "[R] SOMERSAULT ROLL", fill=(255, 255, 255, 255))
    for step in range(5):
        rf = link_sheet.crop((step * 32, 7 * 32, (step + 1) * 32, 8 * 32)).resize((36, 36), Image.NEAREST)
        sim.paste(rf, (20 + step * 36, 65), rf)

    # Action 2: Link Sword Slash Down
    sdraw.text((20, 120), "[A] SWORD SLASH DOWN", fill=(255, 255, 255, 255))
    for step in range(3):
        sf = link_sheet.crop((step * 32, 6 * 32, (step + 1) * 32, 7 * 32)).resize((44, 44), Image.NEAREST)
        sim.paste(sf, (20 + step * 50, 140), sf)

    # Action 3: Link Sword Slash Right
    sdraw.text((220, 120), "[A] SWORD SLASH RIGHT", fill=(255, 255, 255, 255))
    for step in range(3):
        sf = link_sheet.crop(((3 + step) * 32, 6 * 32, (4 + step) * 32, 7 * 32)).resize((44, 44), Image.NEAREST)
        sim.paste(sf, (220 + step * 50, 140), sf)

    # Action 4: Hurt / Damage reaction
    sdraw.text((20, 205), "HURT / KNOCKBACK", fill=(239, 68, 68, 255))
    hf = link_sheet.crop((0, 9 * 32, 32, 10 * 32)).resize((36, 36), Image.NEAREST)
    sim.paste(hf, (160, 200), hf)

    # Action 5: Roc's Cape Airborne Jump
    sdraw.text((220, 205), "ROC'S CAPE GLIDE", fill=(56, 189, 248, 255))
    jf = link_sheet.crop((1 * 32, 8 * 32, 2 * 32, 9 * 32)).resize((36, 36), Image.NEAREST)
    sim.paste(jf, (350, 200), jf)

    canvas.paste(sim, (p3_x + 20, p3_y + 40))

    # Panel 4: Pause Menu Inventory Grid 4x3 (Bottom-Right, 460x320)
    p4_x, p4_y, p4_w, p4_h = 480, 385, 460, 315
    draw.rectangle([p4_x, p4_y, p4_x + p4_w, p4_y + p4_h], fill=(10, 15, 26, 255), outline=(51, 65, 85, 255))
    draw.text((p4_x + 10, p4_y + 8), "PANEL 4: PAUSE MENU INVENTORY 4x3 GRID", fill=(234, 179, 8, 255))

    # Mini pause menu box
    menu_w, menu_h = 420, 260
    menu = Image.new('RGBA', (menu_w, menu_h), (10, 24, 15, 255))
    mdraw = ImageDraw.Draw(menu)
    mdraw.rectangle([0, 0, menu_w - 1, menu_h - 1], outline=(212, 175, 55, 255), width=2)
    mdraw.rectangle([4, 4, menu_w - 5, menu_h - 5], outline=(30, 58, 36, 255))
    mdraw.text((16, 10), "ITENS", fill=(245, 158, 11, 255))
    mdraw.text((320, 10), "150 R", fill=(0, 255, 136, 255))

    # 4x3 Grid slots
    grid_items = [
        # Col, Row in atlas
        ((1, 1), "White Sword"),
        ((0, 2), "Boomerang"),
        ((6, 2), "Gust Jar"),
        ((2, 2), "Bolsa Bombas"),
        ((7, 2), "Pegasus Boots"),
        ((8, 2), "Flippers"),
        ((3, 3), "Cane of Pacci"),
        ((0, 3), "Mole Mitts"),
        ((2, 3), "Flame Lantern"),
        ((1, 3), "Roc's Cape"),
        ((4, 2), "Arco e Flechas"),
        ((4, 3), "Ocarina do Vento")
    ]
    slot_w, slot_h = 70, 48
    start_gx, start_gy = 30, 38
    for idx, ((c, r), name) in enumerate(grid_items):
        gx = idx % 4
        gy = idx // 4
        sx = start_gx + gx * (slot_w + 14)
        sy = start_gy + gy * (slot_h + 10)
        # Slot frame
        is_sel = (idx == 0)
        border_col = (255, 226, 122, 255) if is_sel else (30, 58, 36, 255)
        mdraw.rectangle([sx, sy, sx + slot_w, sy + slot_h], fill=(14, 38, 20, 255), outline=border_col, width=2 if is_sel else 1)
        # Paste 2x icon
        icon = hud_items.crop((c * 16, r * 16, (c + 1) * 16, (r + 1) * 16)).resize((28, 28), Image.NEAREST)
        menu.paste(icon, (sx + 21, sy + 6), icon)
        # Badge on equipped slots
        if idx == 0:
            a_ic = hud_items.crop((8 * 16, 0, 9 * 16, 16)).resize((14, 14), Image.NEAREST)
            menu.paste(a_ic, (sx + 4, sy + 4), a_ic)
        elif idx == 3:
            b_ic = hud_items.crop((9 * 16, 0, 10 * 16, 16)).resize((14, 14), Image.NEAREST)
            menu.paste(b_ic, (sx + 4, sy + 4), b_ic)

    # Item description footer
    mdraw.rectangle([20, 205, menu_w - 20, 245], fill=(7, 21, 32, 240), outline=(56, 189, 248, 255))
    mdraw.text((28, 210), "Espada Branca", fill=(255, 226, 122, 255))
    mdraw.text((28, 225), "Forjada com 2 Elementos no Santuario dos Elementos. Dano dobrado!", fill=(226, 232, 240, 255))

    canvas.paste(menu, (p4_x + 20, p4_y + 40))

    # Save showcase
    out_path = 'scratch/graphics_pipeline_showcase.png'
    canvas.save(out_path)
    print(f"Showcase gerado com sucesso: {out_path}")
    # Also save to artifact directory
    artifact_path = 'C:/Users/thiag/.gemini/antigravity/brain/91f45871-4e2f-495f-88d6-7427ed06770a/graphics_pipeline_showcase.png'
    canvas.save(artifact_path)
    print(f"Artifact salvo com sucesso: {artifact_path}")

if __name__ == '__main__':
    render_showcase()
