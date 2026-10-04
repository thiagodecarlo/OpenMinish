import sys
import os
import math
from PIL import Image, ImageDraw

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "mole_mitts_showcase.png")
    scratch_path = r"d:\REPOS\TLoZ-MC\scratch\mole_mitts_showcase.png"

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (14, 10, 8, 255))
    draw = ImageDraw.Draw(img)

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(24, 16, 12, 255))
    draw.line([0, 48, img_w, 48], fill=(217, 119, 6, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Mole Mitts, Digging Mechanics & Mole Cave Caverns", fill=(254, 240, 138, 255))
    draw.text((640, 16), "Item 1 of 10: feature/mole-mitts-digging", fill=(245, 158, 11, 255))

    panels = [
        ("1. MOLE CAVE CAVERN & PACKED DIRT LABYRINTH", 20, 60, 460, 330),
        ("2. DIGGING DYNAMICS, PARTICLE PHYSICS & DROPS", 500, 60, 940, 330),
        ("3. MOLE MITTS PEDESTAL & CELEBRATION BANNER", 20, 350, 460, 620),
        ("4. INVENTORY PAUSE SCREEN & HUD SLOT [B] INTEGRATION", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(20, 14, 10, 255), outline=(58, 38, 26, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(32, 20, 14, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(217, 119, 6, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(253, 224, 71, 255))

    # Color Palette definitions
    C_CAVE_ROCK     = (42, 36, 32, 255)
    C_CAVE_FLOOR    = (60, 48, 38, 255)
    C_DIRT_WALL     = (130, 76, 37, 255)
    C_DIRT_DARK     = (84, 47, 21, 255)
    C_DIRT_LIGHT    = (175, 115, 65, 255)
    C_TUNNEL_FLOOR  = (78, 58, 44, 255)
    C_CLAW_SCRATCH  = (105, 80, 62, 255)
    C_MOUND_BASE    = (140, 85, 45, 255)
    C_MOUND_DARK    = (70, 38, 18, 255)
    C_GOLD_CHEST    = (245, 158, 11, 255)

    def draw_dirt_wall(x, y):
        draw.rectangle([x, y, x + 15, y + 15], fill=C_DIRT_WALL)
        draw.rectangle([x, y, x + 15, y + 1], fill=C_DIRT_LIGHT)
        draw.rectangle([x, y + 14, x + 15, y + 15], fill=C_DIRT_DARK)
        # Texture marks
        draw.point((x + 4, y + 5), fill=C_DIRT_DARK)
        draw.point((x + 11, y + 6), fill=C_DIRT_LIGHT)
        draw.point((x + 7, y + 10), fill=C_DIRT_DARK)
        draw.line([x + 3, y + 11, x + 5, y + 11], fill=C_DIRT_LIGHT)
        draw.line([x + 10, y + 12, x + 12, y + 12], fill=C_DIRT_DARK)

    def draw_tunnel_floor(x, y):
        draw.rectangle([x, y, x + 15, y + 15], fill=C_TUNNEL_FLOOR)
        # Claw marks on floor
        draw.line([x + 3, y + 4, x + 6, y + 8], fill=C_CLAW_SCRATCH)
        draw.line([x + 5, y + 4, x + 8, y + 8], fill=C_CLAW_SCRATCH)
        draw.line([x + 7, y + 4, x + 10, y + 8], fill=C_CLAW_SCRATCH)

    def draw_dirt_mound(x, y):
        draw.rectangle([x, y, x + 15, y + 15], fill=C_CAVE_FLOOR)
        draw.ellipse([x + 2, y + 3, x + 13, y + 12], fill=C_MOUND_BASE, outline=C_MOUND_DARK)
        draw.line([x + 5, y + 5, x + 10, y + 5], fill=C_DIRT_LIGHT)
        draw.point((x + 7, y + 7), fill=C_DIRT_LIGHT)

    def draw_chest(x, y):
        draw.rectangle([x + 2, y + 3, x + 13, y + 13], fill=C_GOLD_CHEST, outline=(146, 64, 14, 255))
        draw.rectangle([x + 6, y + 7, x + 9, y + 10], fill=(254, 240, 138, 255))
        draw.point((x + 7, y + 8), fill=(30, 20, 10, 255))

    def draw_link_sprite(lx, ly, dir="DOWN", digging=False):
        # Shadow
        draw.ellipse([lx + 2, ly + 14, lx + 14, ly + 18], fill=(10, 8, 6, 160))
        # Cap
        draw.rectangle([lx + 3, ly, lx + 12, ly + 5], fill=(34, 197, 94, 255))
        if dir == "LEFT":
            draw.rectangle([lx + 11, ly + 2, lx + 14, ly + 6], fill=(34, 197, 94, 255))
        elif dir == "RIGHT":
            draw.rectangle([lx + 1, ly + 2, lx + 4, ly + 6], fill=(34, 197, 94, 255))
        # Face
        draw.rectangle([lx + 4, ly + 6, lx + 11, ly + 9], fill=(245, 203, 167, 255))
        if dir == "DOWN":
            draw.point((lx + 5, ly + 7), fill=(17, 17, 17, 255))
            draw.point((lx + 9, ly + 7), fill=(17, 17, 17, 255))
        elif dir == "LEFT":
            draw.point((lx + 4, ly + 7), fill=(17, 17, 17, 255))
        elif dir == "RIGHT":
            draw.point((lx + 10, ly + 7), fill=(17, 17, 17, 255))
        # Tunic
        draw.rectangle([lx + 3, ly + 10, lx + 12, ly + 14], fill=(22, 101, 52, 255))
        # Belt
        draw.line([lx + 4, ly + 13, lx + 11, ly + 13], fill=(139, 69, 19, 255))
        draw.point((lx + 7, ly + 13), fill=(245, 158, 11, 255))
        # Boots
        draw.rectangle([lx + 4, ly + 15, lx + 6, ly + 17], fill=(210, 105, 30, 255))
        draw.rectangle([lx + 9, ly + 15, lx + 11, ly + 17], fill=(210, 105, 30, 255))

        if digging:
            # Brown leather mitts with sharp curved steel claws
            c_leather = (120, 53, 15, 255)
            c_claw = (226, 232, 240, 255)
            c_tip = (255, 255, 255, 255)
            if dir == "DOWN":
                draw.rectangle([lx + 2, ly + 12, lx + 5, ly + 14], fill=c_leather)
                draw.rectangle([lx + 10, ly + 12, lx + 13, ly + 14], fill=c_leather)
                draw.line([lx + 3, ly + 15, lx + 3, ly + 18], fill=c_claw)
                draw.line([lx + 5, ly + 15, lx + 5, ly + 18], fill=c_claw)
                draw.line([lx + 10, ly + 15, lx + 10, ly + 18], fill=c_claw)
                draw.line([lx + 12, ly + 15, lx + 12, ly + 18], fill=c_claw)
                draw.point((lx + 3, ly + 19), fill=c_tip)
                draw.point((lx + 5, ly + 19), fill=c_tip)
                draw.point((lx + 10, ly + 19), fill=c_tip)
                draw.point((lx + 12, ly + 19), fill=c_tip)
            elif dir == "RIGHT":
                draw.rectangle([lx + 13, ly + 8, lx + 16, ly + 12], fill=c_leather)
                draw.line([lx + 17, ly + 9, lx + 20, ly + 9], fill=c_claw)
                draw.line([lx + 17, ly + 11, lx + 20, ly + 11], fill=c_claw)
                draw.point((lx + 21, ly + 9), fill=c_tip)
                draw.point((lx + 21, ly + 11), fill=c_tip)

    # -------------------------------------------------------------------------
    # PANEL 1: MOLE CAVE CAVERN & PACKED DIRT LABYRINTH
    # -------------------------------------------------------------------------
    p1_x = 35
    p1_y = 95
    # Render mini cavern grid (25 x 14 tiles)
    for ry in range(14):
        for rx in range(25):
            tx = p1_x + rx * 16
            ty = p1_y + ry * 16
            # Border rock walls
            if rx == 0 or rx == 24 or ry == 0 or ry == 13:
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=C_CAVE_ROCK)
            elif (rx in (7, 8, 9, 10, 11, 16, 17, 18, 19) and ry in (2, 3, 4, 7, 8, 9, 10)):
                draw_dirt_wall(tx, ty)
            elif (rx in (5, 6) and ry == 3) or (rx == 10 and ry == 5):
                draw_tunnel_floor(tx, ty)
            elif (rx == 4 and ry == 8) or (rx == 21 and ry == 4):
                draw_dirt_mound(tx, ty)
            elif (rx == 2 and ry == 2) or (rx == 22 and ry == 2):
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=C_CAVE_FLOOR)
                draw_chest(tx, ty)
            else:
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=C_CAVE_FLOOR)

    # Place Link in Cavern
    draw_link_sprite(p1_x + 6 * 16, p1_y + 3 * 16, dir="RIGHT", digging=False)
    # Altar chamber north
    draw.rectangle([p1_x + 11 * 16, p1_y + 1 * 16, p1_x + 14 * 16, p1_y + 3 * 16], fill=(45, 30, 20, 255), outline=(217, 119, 6, 255))
    draw.text((p1_x + 11 * 16 + 4, p1_y + 1 * 16 + 6), "ALTAR", fill=(253, 224, 71, 255))

    draw.text((p1_x, p1_y + 14 * 16 + 6), "Packed soft dirt blocks (TILE_DIRT_WALL) seal off secret chambers and chests", fill=(217, 119, 6, 255))

    # -------------------------------------------------------------------------
    # PANEL 2: DIGGING DYNAMICS, PARTICLE PHYSICS & DROPS
    # -------------------------------------------------------------------------
    p2_x = 520
    p2_y = 95
    # Draw floor
    for ry in range(13):
        for rx in range(25):
            tx = p2_x + rx * 16
            ty = p2_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=C_CAVE_FLOOR)

    # Dirt wall section being dug out
    for ry in range(2, 9):
        for rx in range(12, 17):
            tx = p2_x + rx * 16
            ty = p2_y + ry * 16
            if rx == 12 and ry in (4, 5):
                draw_tunnel_floor(tx, ty)
            else:
                draw_dirt_wall(tx, ty)

    # Link digging into wall
    dig_lx = p2_x + 10 * 16
    dig_ly = p2_y + 5 * 16
    draw_link_sprite(dig_lx, dig_ly, dir="RIGHT", digging=True)

    # Flying dirt particles
    import random
    random.seed(42)
    for _ in range(18):
        prx = dig_lx + 20 + random.randint(-4, 16)
        pry = dig_ly + 6 + random.randint(-12, 12)
        psize = random.choice([2, 3])
        pcolor = random.choice([(130, 76, 37, 255), (84, 47, 21, 255), (175, 115, 65, 255)])
        draw.rectangle([prx, pry, prx + psize, pry + psize], fill=pcolor)

    # Dug out drop: Green Rupee (+5) popping out
    rx_drop = dig_lx + 36
    ry_drop = dig_ly - 8
    # Rupee hexagon
    draw.polygon([(rx_drop + 3, ry_drop), (rx_drop + 7, ry_drop), (rx_drop + 10, ry_drop + 4),
                  (rx_drop + 7, ry_drop + 9), (rx_drop + 3, ry_drop + 9), (rx_drop, ry_drop + 4)], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255))
    draw.text((rx_drop + 14, ry_drop), "+5 Rupee!", fill=(74, 222, 128, 255))

    draw.text((p2_x, p2_y + 13 * 16 + 4), "Claw slash shatters dirt wall -> TILE_DIRT_WALL_TUNNEL with Rupee/Heart drops", fill=(251, 191, 36, 255))

    # -------------------------------------------------------------------------
    # PANEL 3: MOLE MITTS PEDESTAL & CELEBRATION BANNER
    # -------------------------------------------------------------------------
    p3_x = 35
    p3_y = 390
    # Room backdrop
    draw.rectangle([p3_x, p3_y, p3_x + 400, p3_y + 120], fill=(28, 20, 16, 255), outline=(78, 52, 34, 255))
    # Stone pedestal
    ped_x = p3_x + 80
    ped_y = p3_y + 45
    draw.rectangle([ped_x - 12, ped_y + 20, ped_x + 28, ped_y + 35], fill=(71, 85, 105, 255), outline=(30, 41, 59, 255))
    draw.rectangle([ped_x - 8, ped_y + 10, ped_x + 24, ped_y + 20], fill=(100, 116, 139, 255))

    # Mystical amber aura
    draw.ellipse([ped_x - 16, ped_y - 12, ped_x + 32, ped_y + 24], fill=(217, 119, 6, 60))
    draw.ellipse([ped_x - 10, ped_y - 6, ped_x + 26, ped_y + 18], fill=(245, 158, 11, 80))

    # Floating Mole Mitts sprite
    mitt_x = ped_x
    mitt_y = ped_y - 4
    # Golden cuff
    draw.rectangle([mitt_x - 6, mitt_y + 8, mitt_x + 20, mitt_y + 12], fill=(245, 158, 11, 255))
    # Leather glove
    draw.rectangle([mitt_x - 8, mitt_y, mitt_x + 22, mitt_y + 8], fill=(120, 53, 15, 255), outline=(69, 26, 3, 255))
    draw.line([mitt_x - 6, mitt_y + 1, mitt_x + 20, mitt_y + 1], fill=(146, 64, 14, 255))
    # 3 Curved steel claws
    draw.line([mitt_x - 4, mitt_y - 6, mitt_x - 4, mitt_y], fill=(226, 232, 240, 255), width=2)
    draw.line([mitt_x + 7, mitt_y - 10, mitt_x + 7, mitt_y], fill=(226, 232, 240, 255), width=2)
    draw.line([mitt_x + 18, mitt_y - 6, mitt_x + 18, mitt_y], fill=(226, 232, 240, 255), width=2)
    draw.point((mitt_x - 4, mitt_y - 7), fill=(255, 255, 255, 255))
    draw.point((mitt_x + 7, mitt_y - 11), fill=(255, 255, 255, 255))
    draw.point((mitt_x + 18, mitt_y - 7), fill=(255, 255, 255, 255))

    # Sparkles
    draw.point((mitt_x - 12, mitt_y - 4), fill=(254, 240, 138, 255))
    draw.point((mitt_x + 28, mitt_y + 2), fill=(254, 240, 138, 255))

    # Celebratory Banner mockup
    ban_x = p3_x + 150
    ban_y = p3_y + 20
    draw.rectangle([ban_x, ban_y, ban_x + 230, ban_y + 44], fill=(42, 24, 10, 240), outline=(217, 119, 6, 255), width=2)
    draw.text((ban_x + 12, ban_y + 6), "LUVAS DE TOUPEIRA OBTIDAS!", fill=(245, 158, 11, 255))
    draw.text((ban_x + 12, ban_y + 24), "Escave Paredes de Terra e Segredos!", fill=(254, 240, 138, 255))

    draw.text((p3_x, p3_y + 128), "Pedestal sagrado na camara norte com pickup automatico e fanfare lendario", fill=(217, 119, 6, 255))

    # -------------------------------------------------------------------------
    # PANEL 4: INVENTORY PAUSE SCREEN & HUD SLOT [B] INTEGRATION
    # -------------------------------------------------------------------------
    p4_x = 520
    p4_y = 390
    # Pause screen mini frame
    draw.rectangle([p4_x, p4_y, p4_x + 240, p4_y + 120], fill=(12, 18, 28, 240), outline=(37, 99, 235, 255), width=2)
    draw.rectangle([p4_x, p4_y, p4_x + 240, p4_y + 20], fill=(20, 32, 52, 255))
    draw.text((p4_x + 8, p4_y + 4), "PAUSE - INVENTARIO", fill=(147, 197, 253, 255))

    # Grid of items (4 cols x 3 rows)
    for row in range(3):
        for col in range(4):
            gx = p4_x + 14 + col * 28
            gy = p4_y + 26 + row * 24
            is_mole = (row == 2 and col == 0)
            fill_c = (30, 41, 59, 255) if not is_mole else (69, 26, 3, 255)
            outline_c = (217, 119, 6, 255) if is_mole else (51, 65, 85, 255)
            draw.rectangle([gx, gy, gx + 22, gy + 20], fill=fill_c, outline=outline_c, width=2 if is_mole else 1)
            if is_mole:
                # Mole mitts mini icon
                draw.rectangle([gx + 4, gy + 9, gx + 17, gy + 15], fill=(120, 53, 15, 255))
                draw.rectangle([gx + 4, gy + 14, gx + 17, gy + 17], fill=(245, 158, 11, 255))
                draw.line([gx + 6, gy + 5, gx + 6, gy + 8], fill=(226, 232, 240, 255))
                draw.line([gx + 10, gy + 4, gx + 10, gy + 8], fill=(226, 232, 240, 255))
                draw.line([gx + 14, gy + 5, gx + 14, gy + 8], fill=(226, 232, 240, 255))

    # Description box below grid
    draw.text((p4_x + 8, p4_y + 102), "Luvas de Toupeira: Escavam terra macia.", fill=(253, 224, 71, 255))

    # HUD Slot [B] Mockup
    hud_x = p4_x + 260
    hud_y = p4_y + 20
    draw.rectangle([hud_x, hud_y, hud_x + 140, hud_y + 80], fill=(18, 14, 10, 255), outline=(217, 119, 6, 255), width=2)
    draw.text((hud_x + 8, hud_y + 6), "HUD SLOTS [A] & [B]", fill=(245, 158, 11, 255))

    # Slot A (Sword)
    draw.rectangle([hud_x + 12, hud_y + 30, hud_x + 42, hud_y + 60], fill=(20, 30, 24, 255), outline=(34, 197, 94, 255), width=2)
    draw.text((hud_x + 18, hud_y + 62), "[A]", fill=(34, 197, 94, 255))
    draw.line([hud_x + 20, hud_y + 50, hud_x + 34, hud_y + 36], fill=(255, 255, 255, 255), width=2)

    # Slot B (Mole Mitts equipped)
    draw.rectangle([hud_x + 55, hud_y + 30, hud_x + 85, hud_y + 60], fill=(42, 24, 10, 255), outline=(245, 158, 11, 255), width=2)
    draw.text((hud_x + 61, hud_y + 62), "[B]", fill=(245, 158, 11, 255))
    # Mitts inside slot B
    draw.rectangle([hud_x + 61, hud_y + 44, hud_x + 79, hud_y + 52], fill=(120, 53, 15, 255))
    draw.rectangle([hud_x + 61, hud_y + 51, hud_x + 79, hud_y + 54], fill=(245, 158, 11, 255))
    draw.line([hud_x + 64, hud_y + 38, hud_x + 64, hud_y + 43], fill=(226, 232, 240, 255))
    draw.line([hud_x + 70, hud_y + 36, hud_x + 70, hud_y + 43], fill=(226, 232, 240, 255))
    draw.line([hud_x + 76, hud_y + 38, hud_x + 76, hud_y + 43], fill=(226, 232, 240, 255))

    draw.text((p4_x, p4_y + 128), "Slot [B] equipa ITEM_MOLE_MITTS com escavacao dinamica via tecla [X]", fill=(147, 197, 253, 255))

    img.save(out_path)
    img.save(scratch_path)
    print(f"Artifact successfully saved to {out_path}")

if __name__ == "__main__":
    main()
