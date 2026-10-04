import sys
import os
import math
from PIL import Image, ImageDraw

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "melari_mines_white_sword_showcase.png")

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (12, 10, 16, 255))
    draw = ImageDraw.Draw(img)

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(20, 24, 32, 255))
    draw.line([0, 48, img_w, 48], fill=(249, 115, 22, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Melari's Mines & White Sword Showcase", fill=(253, 224, 71, 255))
    draw.text((700, 16), "Item 6: feature/melari-mines-white-sword", fill=(148, 163, 184, 255))

    panels = [
        ("1. MELARI'S MINES: SUBTERRANEAN FORGE & LAVA CHANNELS", 20, 60, 460, 330),
        ("2. MASTER SMITH MELARI & MOUNTAIN MINISH MINERS", 500, 60, 940, 330),
        ("3. REFORGING PICORI BLADE: DIALOGUE & ACQUISITION", 20, 350, 460, 620),
        ("4. WHITE SWORD COMBAT (2 HP) & PAUSE INVENTORY", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(16, 18, 24, 255), outline=(50, 55, 70, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(28, 32, 42, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(234, 88, 12, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(251, 146, 60, 255))

    # =============================================================
    # PANEL 1: MELARI'S MINES MAP & ENVIRONMENT
    # =============================================================
    p1_x, p1_y = 30, 94
    # Slate stone floor
    for ry in range(14):
        for rx in range(26):
            tx = p1_x + rx * 16
            ty = p1_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(30, 41, 59, 255))
            draw.line([tx, ty + 15, tx + 15, ty + 15], fill=(15, 23, 42, 255))
            draw.line([tx + 15, ty, tx + 15, ty + 15], fill=(15, 23, 42, 255))
            if (rx + ry) % 3 == 0:
                draw.point((tx + 4, ty + 4), fill=(51, 65, 85, 255))

    # Cavern cliff walls on edges with ore veins
    for ry in range(14):
        # West cliff
        wx = p1_x
        wy = p1_y + ry * 16
        draw.rectangle([wx, wy, wx + 31, wy + 15], fill=(28, 25, 23, 255))
        if ry % 3 == 0:
            # Ore vein with gems
            draw.point((wx + 8, wy + 5), fill=(16, 185, 129, 255)) # emerald
            draw.point((wx + 9, wy + 5), fill=(52, 211, 153, 255))
            draw.point((wx + 18, wy + 10), fill=(6, 182, 212, 255)) # sapphire
            draw.point((wx + 24, wy + 7), fill=(239, 68, 68, 255)) # ruby
        # East cliff
        ex = p1_x + 24 * 16
        draw.rectangle([ex, wy, ex + 31, wy + 15], fill=(28, 25, 23, 255))
        if ry % 3 == 1:
            draw.point((ex + 10, wy + 6), fill=(250, 204, 21, 255)) # gold
            draw.point((ex + 16, wy + 11), fill=(6, 182, 212, 255))

    # Northern Cave Archway (Cave of Flames entrance)
    arch_x = p1_x + 11 * 16
    arch_y = p1_y
    draw.rectangle([arch_x, arch_y, arch_x + 63, arch_y + 31], fill=(51, 65, 85, 255))
    draw.rectangle([arch_x + 12, arch_y + 8, arch_x + 51, arch_y + 31], fill=(2, 6, 23, 255)) # dark opening
    # Entrance Torches
    draw.rectangle([arch_x + 4, arch_y + 12, arch_x + 8, arch_y + 20], fill=(120, 53, 15, 255))
    draw.rectangle([arch_x + 4, arch_y + 8, arch_x + 8, arch_y + 11], fill=(249, 115, 22, 255))
    draw.rectangle([arch_x + 55, arch_y + 12, arch_x + 59, arch_y + 20], fill=(120, 53, 15, 255))
    draw.rectangle([arch_x + 55, arch_y + 8, arch_x + 59, arch_y + 11], fill=(249, 115, 22, 255))

    # Molten Lava Channels (West and East trenches)
    for ry in [5, 6]:
        # West lava channel
        for rx in range(3, 10):
            lx = p1_x + rx * 16
            ly = p1_y + ry * 16
            draw.rectangle([lx, ly, lx + 15, ly + 15], fill=(220, 38, 38, 255))
            draw.rectangle([lx + 2, ly + 2, lx + 13, ly + 13], fill=(234, 88, 12, 255))
            draw.point((lx + 6, ly + 7), fill=(251, 191, 36, 255)) # bubbling heat
            draw.point((lx + 10, ly + 9), fill=(253, 224, 71, 255))
        # East lava channel
        for rx in range(16, 23):
            lx = p1_x + rx * 16
            ly = p1_y + ry * 16
            draw.rectangle([lx, ly, lx + 15, ly + 15], fill=(220, 38, 38, 255))
            draw.rectangle([lx + 2, ly + 2, lx + 13, ly + 13], fill=(234, 88, 12, 255))
            draw.point((lx + 5, ly + 6), fill=(251, 191, 36, 255))
            draw.point((lx + 11, ly + 8), fill=(253, 224, 71, 255))

    # Mining Tracks (rails & ties)
    for ry in range(8, 14):
        rx = 13
        tx = p1_x + rx * 16
        ty = p1_y + ry * 16
        draw.rectangle([tx + 1, ty + 3, tx + 14, ty + 5], fill=(120, 53, 15, 255))
        draw.rectangle([tx + 1, ty + 11, tx + 14, ty + 13], fill=(120, 53, 15, 255))
        draw.rectangle([tx + 3, ty, tx + 4, ty + 15], fill=(148, 163, 184, 255))
        draw.rectangle([tx + 11, ty, tx + 12, ty + 15], fill=(148, 163, 184, 255))

    # Minecart on tracks
    mc_x = p1_x + 13 * 16
    mc_y = p1_y + 11 * 16
    draw.rectangle([mc_x + 2, mc_y + 3, mc_x + 13, mc_y + 11], fill=(100, 116, 139, 255))
    draw.rectangle([mc_x + 4, mc_y + 4, mc_x + 11, mc_y + 8], fill=(30, 41, 59, 255))
    draw.rectangle([mc_x + 3, mc_y + 12, mc_x + 5, ty + 14], fill=(71, 85, 105, 255))
    draw.rectangle([mc_x + 10, mc_y + 12, mc_x + 12, ty + 14], fill=(71, 85, 105, 255))

    draw.text((p1_x + 8, p1_y + 205), "Trilhos, canais de lava incandescente e veios de minerio", fill=(203, 213, 225, 255))

    # =============================================================
    # PANEL 2: MASTER SMITH MELARI & MOUNTAIN MINISH
    # =============================================================
    p2_x, p2_y = 510, 94
    # Slate floor
    for ry in range(14):
        for rx in range(26):
            tx = p2_x + rx * 16
            ty = p2_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(30, 41, 59, 255))
            draw.line([tx, ty + 15, tx + 15, ty + 15], fill=(15, 23, 42, 255))
            draw.line([tx + 15, ty, tx + 15, ty + 15], fill=(15, 23, 42, 255))

    # Central Forge Platform with Glowing Embers Hearth
    fx = p2_x + 10 * 16
    fy = p2_y + 4 * 16
    draw.rectangle([fx - 16, fy, fx + 47, fy + 47], fill=(51, 65, 85, 255), outline=(71, 85, 105, 255))
    # Glowing hearth embers
    draw.rectangle([fx + 4, fy + 24, fx + 27, fy + 36], fill=(234, 88, 12, 255))
    draw.rectangle([fx + 8, fy + 26, fx + 23, fy + 34], fill=(249, 115, 22, 255))
    draw.point((fx + 12, fy + 28), fill=(254, 240, 138, 255))
    draw.point((fx + 18, fy + 30), fill=(254, 240, 138, 255))

    # Massive Steel Anvil
    ax = fx + 6
    ay = fy + 8
    draw.rectangle([ax + 2, ay + 6, ax + 18, ay + 16], fill=(100, 116, 139, 255)) # anvil body
    draw.rectangle([ax + 4, ay + 2, ax + 20, ay + 6], fill=(203, 213, 225, 255)) # face
    draw.rectangle([ax - 2, ay + 2, ax + 4, ay + 5], fill=(148, 163, 184, 255))  # horn
    draw.rectangle([ax + 6, ay + 16, ax + 14, ay + 20], fill=(71, 85, 105, 255)) # base

    # Master Smith Melari NPC at the anvil
    mx = ax + 26
    my = ay + 2
    draw.ellipse([mx + 2, my + 14, mx + 16, my + 19], fill=(5, 16, 7, 100)) # shadow
    # Beard
    draw.rectangle([mx + 4, my + 7, mx + 12, my + 14], fill=(226, 232, 240, 255))
    # Face & Goggles
    draw.rectangle([mx + 5, my + 4, mx + 11, my + 8], fill=(253, 232, 205, 255))
    draw.rectangle([mx + 4, my + 2, mx + 12, my + 4], fill=(146, 64, 14, 255)) # goggles
    draw.point((mx + 6, my + 3), fill=(249, 115, 22, 255)) # lens
    draw.point((mx + 10, my + 3), fill=(249, 115, 22, 255))
    draw.point((mx + 6, my + 6), fill=(15, 23, 42, 255)) # eye
    draw.point((mx + 9, my + 6), fill=(15, 23, 42, 255))
    # Red tunic & leather apron
    draw.rectangle([mx + 4, my + 9, mx + 12, my + 15], fill=(220, 38, 38, 255))
    draw.rectangle([mx + 5, my + 10, mx + 11, my + 15], fill=(120, 53, 15, 255))
    # Hammer striking anvil!
    draw.rectangle([mx - 6, my + 2, mx - 2, my + 6], fill=(100, 116, 139, 255))
    draw.rectangle([mx - 4, my + 5, mx + 1, my + 10], fill=(120, 53, 15, 255))
    # Striking sparks
    draw.point((ax + 18, ay + 3), fill=(250, 204, 21, 255))
    draw.point((ax + 20, ay + 1), fill=(255, 255, 255, 255))
    draw.point((ax + 17, ay - 1), fill=(249, 115, 22, 255))

    # [A] Melari prompt
    draw.rectangle([mx - 18, my - 16, mx + 38, my - 4], fill=(26, 15, 6, 240), outline=(249, 115, 22, 255))
    draw.text((mx - 14, my - 15), "[A] Melari", fill=(253, 224, 71, 255))

    # Mountain Minish Miner 1 (West ore vein)
    m1_x = p2_x + 40
    m1_y = p2_y + 120
    draw.rectangle([m1_x + 4, m1_y + 2, m1_x + 12, m1_y + 6], fill=(250, 204, 21, 255)) # yellow helmet
    draw.point((m1_x + 8, m1_y + 3), fill=(255, 255, 255, 255)) # headlamp
    draw.rectangle([m1_x + 4, m1_y + 6, m1_x + 12, m1_y + 9], fill=(253, 232, 205, 255)) # face
    draw.rectangle([m1_x + 4, m1_y + 9, m1_x + 12, m1_y + 15], fill=(37, 99, 235, 255)) # blue denim
    # Pickaxe
    draw.rectangle([m1_x + 13, m1_y + 5, m1_x + 17, m1_y + 8], fill=(100, 116, 139, 255))
    draw.rectangle([m1_x + 11, m1_y + 7, m1_x + 13, m1_y + 14], fill=(120, 53, 15, 255))

    # Mountain Minish Miner 2 (East cart)
    m2_x = p2_x + 320
    m2_y = p2_y + 140
    draw.rectangle([m2_x + 4, m2_x % 2 + m2_y + 2, m2_x + 12, m2_y + 6], fill=(250, 204, 21, 255))
    draw.point((m2_x + 8, m2_y + 3), fill=(255, 255, 255, 255))
    draw.rectangle([m2_x + 4, m2_y + 6, m2_x + 12, m2_y + 9], fill=(253, 232, 205, 255))
    draw.rectangle([m2_x + 4, m2_y + 9, m2_x + 12, m2_y + 15], fill=(37, 99, 235, 255))

    # Link interacting with Melari
    lx = fx + 6
    ly = fy + 38
    draw.rectangle([lx + 3, ly + 0, lx + 13, ly + 3], fill=(50, 205, 50, 255))
    draw.rectangle([lx + 2, ly + 3, lx + 14, ly + 6], fill=(50, 205, 50, 255))
    draw.rectangle([lx + 4, ly + 6, lx + 12, ly + 10], fill=(245, 203, 167, 255))
    draw.point((lx + 6, ly + 7), fill=(17, 17, 17, 255))
    draw.point((lx + 9, ly + 7), fill=(17, 17, 17, 255))
    draw.rectangle([lx + 3, ly + 10, lx + 13, ly + 15], fill=(34, 139, 34, 255))
    draw.rectangle([lx + 4, ly + 15, lx + 7, ly + 18], fill=(210, 105, 30, 255))
    draw.rectangle([lx + 9, ly + 15, lx + 12, ly + 18], fill=(210, 105, 30, 255))

    draw.text((p2_x + 10, p2_y + 205), "Melari na bigorna forjando & Aprendizes Mountain Minish", fill=(203, 213, 225, 255))

    # =============================================================
    # PANEL 3: DIALOGUE BOX & CELEBRATION BANNER
    # =============================================================
    p3_x, p3_y = 30, 384
    draw.rectangle([p3_x, p3_y, p3_x + 416, p3_y + 220], fill=(15, 23, 42, 255))

    # Melari Dialogue Box
    db_x, db_y = p3_x + 10, p3_y + 10
    db_w, db_h = 396, 96
    draw.rectangle([db_x, db_y, db_x + db_w, db_y + db_h], fill=(8, 28, 16, 240), outline=(212, 175, 55, 255), width=2)
    # Speaker badge
    draw.rectangle([db_x + 12, db_y - 10, db_x + 110, db_y + 6], fill=(26, 64, 34, 255), outline=(212, 175, 55, 255))
    draw.text((db_x + 16, db_y - 9), "Mestre Melari", fill=(249, 115, 22, 255))

    # Melari 32x32 Portrait inside Dialogue Box
    pt_x = db_x + 12
    pt_y = db_y + 14
    draw.rectangle([pt_x, pt_y, pt_x + 36, pt_y + 36], fill=(24, 16, 10, 255), outline=(140, 115, 34, 255))
    # Hair & bushy beard
    draw.rectangle([pt_x + 6, pt_y + 4, pt_x + 30, pt_y + 12], fill=(226, 232, 240, 255))
    # Goggles
    draw.rectangle([pt_x + 8, pt_y + 6, pt_x + 28, pt_y + 10], fill=(146, 64, 14, 255))
    draw.rectangle([pt_x + 10, pt_y + 7, pt_x + 16, pt_y + 10], fill=(249, 115, 22, 255))
    draw.rectangle([pt_x + 20, pt_y + 7, pt_x + 26, pt_y + 10], fill=(249, 115, 22, 255))
    # Face & beard
    draw.rectangle([pt_x + 10, pt_y + 11, pt_x + 26, pt_y + 18], fill=(253, 232, 205, 255))
    draw.point((pt_x + 13, pt_y + 13), fill=(15, 23, 42, 255))
    draw.point((pt_x + 23, pt_y + 13), fill=(15, 23, 42, 255))
    draw.rectangle([pt_x + 8, pt_y + 17, pt_x + 28, pt_y + 28], fill=(226, 232, 240, 255)) # big beard
    # Apron
    draw.rectangle([pt_x + 8, pt_y + 28, pt_x + 28, pt_y + 34], fill=(120, 53, 15, 255))

    # Dialogue text
    dialogue_lines = [
        "*CLANG! CLANG! CLANG!*",
        "O fogo e o martelo unem o aco sagrado",
        "em uma lamina branca reluzente!",
        "Contemple a ESPADA BRANCA (White Sword)!",
        "Ela corta com o dobro de poder (2 HP)!"
    ]
    dy = db_y + 12
    for line in dialogue_lines:
        draw.text((db_x + 58, dy), line, fill=(255, 255, 255, 255))
        dy += 15

    # White Sword Celebration Banner
    ban_x = p3_x + 20
    ban_y = p3_y + 125
    ban_w = 376
    ban_h = 60
    draw.rectangle([ban_x, ban_y, ban_x + ban_w, ban_y + ban_h], fill=(15, 23, 42, 255), outline=(56, 189, 248, 255), width=2)
    # White Sword Icon
    draw.rectangle([ban_x + 18, ban_y + 12, ban_x + 24, ban_y + 40], fill=(255, 255, 255, 255))
    draw.rectangle([ban_x + 20, ban_y + 12, ban_x + 22, ban_y + 40], fill=(186, 230, 253, 255))
    draw.rectangle([ban_x + 12, ban_y + 32, ban_x + 30, ban_y + 36], fill=(250, 204, 21, 255)) # gold wing guard
    draw.rectangle([ban_x + 19, ban_y + 33, ban_x + 23, ban_y + 35], fill=(239, 68, 68, 255)) # ruby core
    draw.rectangle([ban_x + 18, ban_y + 38, ban_x + 24, ban_y + 46], fill=(248, 250, 252, 255)) # silver grip
    draw.rectangle([ban_x + 17, ban_y + 46, ban_x + 25, ban_y + 49], fill=(250, 204, 21, 255)) # pommel

    draw.text((ban_x + 46, ban_y + 14), "LENDARIA ESPADA BRANCA FORJADA!", fill=(255, 255, 255, 255))
    draw.text((ban_x + 46, ban_y + 32), "Dano Dobrado (2 HP) | Reforjada por Mestre Melari", fill=(250, 204, 21, 255))

    draw.text((p3_x + 10, p3_y + 198), "Retrato do Melari, dialogo de forja e banner comemorativo", fill=(148, 163, 184, 255))

    # =============================================================
    # PANEL 4: WHITE SWORD COMBAT & INVENTORY
    # =============================================================
    p4_x, p4_y = 510, 384
    draw.rectangle([p4_x, p4_y, p4_x + 416, p4_y + 220], fill=(15, 23, 42, 255))

    # Combat scene (top half of panel 4)
    # Link swinging White Sword
    sl_x = p4_x + 40
    sl_y = p4_y + 35
    # Link body
    draw.rectangle([sl_x + 3, sl_y + 0, sl_x + 13, sl_y + 3], fill=(50, 205, 50, 255))
    draw.rectangle([sl_x + 2, sl_y + 3, sl_x + 14, sl_y + 6], fill=(50, 205, 50, 255))
    draw.rectangle([sl_x + 4, sl_y + 6, sl_x + 12, sl_y + 10], fill=(245, 203, 167, 255))
    draw.point((sl_x + 10, sl_y + 7), fill=(17, 17, 17, 255)) # eyes right
    draw.rectangle([sl_x + 3, sl_y + 10, sl_x + 13, sl_y + 15], fill=(34, 139, 34, 255))
    draw.rectangle([sl_x + 4, sl_y + 15, sl_x + 7, sl_y + 18], fill=(210, 105, 30, 255))
    draw.rectangle([sl_x + 9, sl_y + 15, sl_x + 12, sl_y + 18], fill=(210, 105, 30, 255))

    # Radiant White Sword Blade extended to the right
    draw.rectangle([sl_x + 14, sl_y + 9, sl_x + 34, sl_y + 13], fill=(255, 255, 255, 255)) # silver blade
    draw.rectangle([sl_x + 16, sl_y + 10, sl_x + 32, sl_y + 12], fill=(186, 230, 253, 255)) # azure shine
    draw.rectangle([sl_x + 12, sl_y + 7, sl_x + 15, sl_y + 15], fill=(250, 204, 21, 255)) # gold guard
    draw.point((sl_x + 13, sl_y + 11), fill=(239, 68, 68, 255)) # ruby gem
    # Sparkling trail particles
    draw.point((sl_x + 24, sl_y + 3), fill=(255, 255, 255, 255))
    draw.point((sl_x + 30, sl_y + 5), fill=(56, 189, 248, 255))
    draw.point((sl_x + 26, sl_y + 18), fill=(255, 255, 255, 255))
    draw.point((sl_x + 33, sl_y + 16), fill=(56, 189, 248, 255))

    # Enemy taking 2 damage (Tektite / Spiny Beetle)
    en_x = sl_x + 46
    en_y = sl_y + 4
    draw.rectangle([en_x, en_y, en_x + 24, en_y + 16], fill=(239, 68, 68, 200), outline=(255, 255, 255, 255)) # flash
    draw.text((en_x + 28, en_y - 6), "-2 HP!", fill=(255, 255, 255, 255))
    draw.text((en_x + 28, en_y + 8), "(2x Dano)", fill=(250, 204, 21, 255))

    # Pause Menu Inventory Display (bottom half of panel 4)
    inv_x = p4_x + 12
    inv_y = p4_y + 70
    draw.rectangle([inv_x, inv_y, inv_x + 392, inv_y + 120], fill=(24, 20, 16, 250), outline=(212, 175, 55, 255), width=2)
    draw.text((inv_x + 12, inv_y + 8), "MENU DE PAUSA - INVENTARIO", fill=(253, 224, 71, 255))

    # Equipped slots
    draw.rectangle([inv_x + 260, inv_y + 6, inv_x + 310, inv_y + 26], fill=(30, 41, 59, 255), outline=(56, 189, 248, 255))
    draw.text((inv_x + 264, inv_y + 9), "[A] Espada", fill=(255, 255, 255, 255))
    draw.rectangle([inv_x + 320, inv_y + 6, inv_x + 375, inv_y + 26], fill=(30, 41, 59, 255), outline=(234, 88, 12, 255))
    draw.text((inv_x + 324, inv_y + 9), "[B] Pacci", fill=(255, 255, 255, 255))

    # Inventory Slot 0 (White Sword selected)
    s0_x = inv_x + 16
    s0_y = inv_y + 30
    draw.rectangle([s0_x, s0_y, s0_x + 28, s0_y + 28], fill=(51, 65, 85, 255), outline=(250, 204, 21, 255), width=2)
    # White Sword icon inside slot
    draw.line([s0_x + 6, s0_y + 22, s0_x + 20, s0_y + 6], fill=(255, 255, 255, 255), width=2)
    draw.line([s0_x + 5, s0_y + 22, s0_x + 19, s0_y + 6], fill=(186, 230, 253, 255), width=1)
    draw.point((s0_x + 9, s0_y + 19), fill=(250, 204, 21, 255))
    draw.point((s0_x + 10, s0_y + 18), fill=(239, 68, 68, 255))

    # Other inventory slots
    for i in range(1, 6):
        sx = s0_x + i * 36
        draw.rectangle([sx, s0_y, sx + 28, s0_y + 28], fill=(30, 41, 59, 255), outline=(71, 85, 105, 255))

    # Item details
    draw.text((inv_x + 16, inv_y + 66), "Item: Espada Branca (White Sword)", fill=(255, 255, 255, 255))
    draw.text((inv_x + 16, inv_y + 82), "Reforjada por Mestre Melari na forja subterranea.", fill=(203, 213, 225, 255))
    draw.text((inv_x + 16, inv_y + 98), "Dano: 2 HP (Dano Dobrado) | Ataque Giratorio: 4 HP", fill=(250, 204, 21, 255))

    draw.text((p4_x + 10, p4_y + 200), "Golpe com Espada Branca, dano 2x e slot no inventario", fill=(148, 163, 184, 255))

    os.makedirs(out_dir, exist_ok=True)
    img.save(out_path)
    print(f"[OK] Showcase gerado com sucesso em: {out_path}")

if __name__ == "__main__":
    main()
