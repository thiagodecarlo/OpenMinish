import sys
import os
import math
from PIL import Image, ImageDraw

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "fortress_of_winds_showcase.png")
    scratch_path = r"d:\REPOS\TLoZ-MC\scratch\fortress_of_winds_showcase.png"

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (10, 15, 24, 255))
    draw = ImageDraw.Draw(img)

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(15, 23, 42, 255))
    draw.line([0, 48, img_w, 48], fill=(56, 189, 248, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Fortress of Winds, Boss Mazaal & Ocarina of Wind", fill=(224, 242, 254, 255))
    draw.text((640, 16), "Item 3 of 10: feature/dungeon-fortress-of-winds", fill=(56, 189, 248, 255))

    panels = [
        ("1. FORTRESS INTERIOR: DIGGING MAZE & EYE STATUES", 20, 60, 460, 330),
        ("2. MINISH PEDESTAL, CRACKS & BOSS KEY ROOM", 500, 60, 940, 330),
        ("3. BOSS BATTLE: MAZAAL (ROBOTIC GUARDIAN)", 20, 350, 460, 620),
        ("4. DUNGEON VICTORY: OCARINA OF WIND & WARP", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(15, 23, 42, 255), outline=(30, 41, 59, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(30, 41, 59, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(56, 189, 248, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(186, 230, 253, 255))

    # Color Palette definitions
    C_WALL_TOP     = (71, 85, 105, 255)
    C_WALL_FACE    = (30, 41, 59, 255)
    C_WALL_DARK    = (15, 23, 42, 255)
    C_FLOOR_BASE   = (51, 65, 85, 255)
    C_FLOOR_CRK    = (30, 58, 82, 255)
    C_PIT          = (5, 8, 17, 255)
    C_WIND_CYAN    = (56, 189, 248, 255)
    C_GOLD         = (245, 158, 11, 255)
    C_EYE_CYAN     = (6, 182, 212, 255)
    C_MAZAAL_HEAD  = (100, 116, 139, 255)
    C_MAZAAL_BRASS = (217, 119, 6, 255)
    C_DIRT_WALL    = (130, 76, 37, 255)
    C_DIRT_DARK    = (84, 47, 21, 255)

    def draw_fortress_floor(x, y):
        draw.rectangle([x, y, x + 15, y + 15], fill=C_FLOOR_BASE)
        draw.line([x + 2, y + 2, x + 13, y + 2], fill=C_FLOOR_CRK)
        draw.line([x + 2, y + 2, x + 2, y + 13], fill=C_FLOOR_CRK)
        draw.point((x + 7, y + 7), fill=(71, 85, 105, 255))

    def draw_fortress_wall(x, y):
        draw.rectangle([x, y, x + 15, y + 4], fill=C_WALL_TOP)
        draw.rectangle([x, y + 4, x + 15, y + 15], fill=C_WALL_FACE)
        draw.rectangle([x + 4, y + 6, x + 11, y + 13], outline=C_WALL_DARK)

    def draw_fortress_pit(x, y):
        draw.rectangle([x, y, x + 15, y + 15], fill=C_PIT)
        # Wind stream lines
        draw.line([x + 4, y + 12, x + 11, y + 4], fill=(56, 189, 248, 120))

    def draw_minish_pedestal(x, y):
        draw.rectangle([x + 2, y + 4, x + 13, y + 13], fill=(71, 85, 105, 255))
        draw.rectangle([x + 3, y + 5, x + 12, y + 12], fill=(100, 116, 139, 255))
        draw.rectangle([x + 5, y + 7, x + 10, y + 10], fill=C_WIND_CYAN)

    def draw_link_human(x, y, facing="down"):
        # Hat & tunic
        draw.rectangle([x + 4, y + 2, x + 11, y + 6], fill=(34, 197, 94, 255))
        draw.rectangle([x + 5, y + 6, x + 10, y + 10], fill=(253, 224, 71, 255))
        draw.rectangle([x + 4, y + 10, x + 11, y + 15], fill=(34, 197, 94, 255))
        draw.rectangle([x + 5, y + 15, x + 10, y + 18], fill=(255, 255, 255, 255))
        draw.rectangle([x + 5, y + 18, x + 10, y + 21], fill=(161, 98, 7, 255))

    def draw_link_minish(x, y):
        # 4x6 pixel minish Link
        draw.rectangle([x, y, x + 3, y + 1], fill=(34, 197, 94, 255))
        draw.rectangle([x, y + 2, x + 3, y + 3], fill=(253, 224, 71, 255))
        draw.rectangle([x, y + 4, x + 3, y + 5], fill=(34, 197, 94, 255))

    # --- PANEL 1: Interior, Digging Maze & Eye Statues ---
    p1_x, p1_y = 35, 95
    # Render floor grid
    for row in range(12):
        for col in range(25):
            gx = p1_x + col * 16
            gy = p1_y + row * 16
            if row == 0 or row == 11 or col == 0 or col == 24:
                draw_fortress_wall(gx, gy)
            elif 3 <= col <= 9 and 2 <= row <= 9:
                # Digging sand blocks
                draw.rectangle([gx, gy, gx + 15, gy + 15], fill=C_DIRT_WALL, outline=C_DIRT_DARK)
                draw.point((gx + 4, gy + 6), fill=(180, 110, 60, 255))
            elif col == 14 and (row == 3 or row == 8):
                # Eye statues
                draw_fortress_wall(gx, gy)
                # Eye closed/open
                if row == 3:
                    draw.rectangle([gx + 3, gy + 5, gx + 12, gy + 12], fill=C_EYE_CYAN)
                    draw.ellipse([gx + 5, gy + 6, gx + 10, gy + 11], fill=(255, 255, 255, 255))
                    draw.ellipse([gx + 7, gy + 7, gx + 8, gy + 10], fill=(0, 0, 0, 255))
                else:
                    draw.rectangle([gx + 3, gy + 5, gx + 12, gy + 12], fill=(15, 23, 42, 255))
                    draw.line([gx + 4, gy + 9, gx + 11, gy + 9], fill=C_EYE_CYAN, width=2)
            else:
                draw_fortress_floor(gx, gy)

    # Link with Mole Mitts digging
    draw_link_human(p1_x + 100, p1_y + 80)
    # Digging claws effect
    draw.arc([p1_x + 94, p1_y + 84, p1_x + 104, p1_y + 96], 0, 360, fill=(245, 158, 11, 255), width=2)
    # Arrow shot towards bottom eye statue
    draw.line([p1_x + 130, p1_y + 136, p1_x + 215, p1_y + 136], fill=(245, 158, 11, 255), width=2)
    draw.polygon([(p1_x + 215, p1_y + 134), (p1_x + 223, p1_y + 136), (p1_x + 215, p1_y + 138)], fill=(255, 255, 255, 255))

    draw.text((p1_x + 5, p1_y + 198), "Mole Mitts burrow through sand maze; Bow activates switch eyes", fill=(148, 163, 184, 255))

    # --- PANEL 2: Minish Pedestal, Cracks & Boss Key ---
    p2_x, p2_y = 515, 95
    for row in range(12):
        for col in range(25):
            gx = p2_x + col * 16
            gy = p2_y + row * 16
            if row == 0 or row == 11 or col == 0 or col == 24:
                draw_fortress_wall(gx, gy)
            elif col == 12:
                # Wall with tiny Minish Crack
                if row == 5:
                    draw_fortress_floor(gx, gy)
                    draw.rectangle([gx + 6, gy + 2, gx + 9, gy + 13], fill=C_PIT)
                else:
                    draw_fortress_wall(gx, gy)
            else:
                draw_fortress_floor(gx, gy)

    # Minish Pedestal
    draw_minish_pedestal(p2_x + 48, p2_y + 80)
    # Minish Link walking towards crack
    draw_link_minish(p2_x + 130, p2_y + 86)
    # Big Key Chest in locked chamber
    bx = p2_x + 280
    by = p2_y + 75
    draw.rectangle([bx, by, bx + 24, by + 20], fill=C_GOLD, outline=(180, 83, 9, 255), width=2)
    draw.rectangle([bx + 8, by + 8, bx + 16, by + 16], fill=(255, 255, 255, 255))
    draw.text((p2_x + 250, p2_y + 105), "BIG KEY CHEST", fill=(253, 224, 71, 255))

    draw.text((p2_x + 15, p2_y + 198), "Link shrinks at altar, traverses wall crack & unlocks Big Key", fill=(148, 163, 184, 255))

    # --- PANEL 3: Boss Battle - Mazaal ---
    p3_x, p3_y = 35, 385
    for row in range(12):
        for col in range(25):
            gx = p3_x + col * 16
            gy = p3_y + row * 16
            if row == 0 or row == 11 or col == 0 or col == 24:
                draw_fortress_wall(gx, gy)
            elif (col == 12 or col == 13) and row == 11:
                draw.rectangle([gx, gy, gx + 15, gy + 15], fill=C_GOLD) # Boss door locked
            else:
                draw_fortress_floor(gx, gy)

    # Mazaal Head (Huge 48x40 ancient brass/slate mechanism)
    hx = p3_x + 176
    hy = p3_y + 30
    draw.rectangle([hx, hy, hx + 48, hy + 36], fill=C_MAZAAL_HEAD, outline=C_MAZAAL_BRASS, width=3)
    # Glowing sensor eyes
    draw.ellipse([hx + 10, hy + 12, hx + 18, hy + 20], fill=(239, 68, 68, 255))
    draw.ellipse([hx + 30, hy + 12, hx + 38, hy + 20], fill=(239, 68, 68, 255))
    # Head crown vents
    for i in range(4):
        draw.rectangle([hx + 8 + i * 8, hy - 6, hx + 13 + i * 8, hy], fill=C_MAZAAL_BRASS)

    # Left hand (active / slamming down)
    lh_x = p3_x + 90
    lh_y = p3_y + 80
    draw.rectangle([lh_x, lh_y, lh_x + 32, lh_y + 32], fill=C_MAZAAL_HEAD, outline=C_MAZAAL_BRASS, width=2)
    draw.ellipse([lh_x + 8, lh_y + 8, lh_x + 24, lh_y + 24], fill=C_EYE_CYAN, outline=(255, 255, 255, 255))

    # Right hand (stunned / eye closed with stars)
    rh_x = p3_x + 280
    rh_y = p3_y + 80
    draw.rectangle([rh_x, rh_y, rh_x + 32, rh_y + 32], fill=(51, 65, 85, 255), outline=(71, 85, 105, 255), width=2)
    draw.line([rh_x + 6, rh_y + 16, rh_x + 26, rh_y + 16], fill=(148, 163, 184, 255), width=3)
    # Stun stars
    draw.text((rh_x + 6, rh_y - 14), "* * *", fill=(253, 224, 71, 255))

    # Link shooting Bow at left hand
    draw_link_human(p3_x + 140, p3_y + 120)
    draw.line([p3_x + 140, p3_y + 128, lh_x + 20, lh_y + 20], fill=(245, 158, 11, 255), width=2)

    # Minish Pedestal in corner
    draw_minish_pedestal(p3_x + 24, p3_y + 140)

    draw.text((p3_x + 5, p3_y + 198), "Stun hands with Bow, enter head as Minish to strike core pillars!", fill=(248, 113, 113, 255))

    # --- PANEL 4: Victory, Ocarina of Wind & Fast Travel Preview ---
    p4_x, p4_y = 515, 385
    for row in range(12):
        for col in range(25):
            gx = p4_x + col * 16
            gy = p4_y + row * 16
            if row == 0 or row == 11 or col == 0 or col == 24:
                draw_fortress_wall(gx, gy)
            else:
                draw_fortress_floor(gx, gy)

    # Warp Portal in center
    wpx = p4_x + 180
    wpy = p4_y + 80
    for r in range(24, 0, -4):
        draw.ellipse([wpx - r, wpy - r, wpx + r, wpy + r], outline=C_WIND_CYAN, width=2)
    draw.text((wpx - 34, wpy + 28), "WARP PORTAL", fill=C_WIND_CYAN)

    # Link holding Ocarina of Wind triumphantly
    lx = p4_x + 70
    ly = p4_y + 70
    draw_link_human(lx, ly)
    # Ocarina sprite in hand
    ox = lx + 4
    oy = ly - 14
    draw.ellipse([ox, oy, ox + 14, oy + 8], fill=C_WIND_CYAN, outline=(255, 255, 255, 255), width=2)
    # Sparkles
    draw.text((ox - 8, oy - 10), "+", fill=(255, 255, 255, 255))
    draw.text((ox + 16, oy - 8), "+", fill=(254, 240, 138, 255))

    # Celebration Banner
    bx1 = p4_x + 120
    by1 = p4_y + 20
    draw.rectangle([bx1, by1, bx1 + 240, by1 + 44], fill=(15, 23, 42, 230), outline=C_WIND_CYAN, width=2)
    draw.text((bx1 + 10, by1 + 6), "YOU GOT THE OCARINA OF WIND!", fill=(254, 240, 138, 255))
    draw.text((bx1 + 10, by1 + 24), "Call the wind bird Zeffa to travel!", fill=(186, 230, 253, 255))

    # Heart Container in room
    hc_x = p4_x + 320
    hc_y = p4_y + 80
    draw.polygon([(hc_x, hc_y + 4), (hc_x + 6, hc_y - 2), (hc_x + 12, hc_y + 4), (hc_x + 6, hc_y + 12)], fill=(239, 68, 68, 255))

    draw.text((p4_x + 5, p4_y + 198), "Fortress Cleared: Ocarina unlocks flight with Zeffa fast travel", fill=(56, 189, 248, 255))

    img.save(out_path)
    img.save(scratch_path)
    print(f"Showcase successfully written to:\n- {out_path}\n- {scratch_path}")

if __name__ == "__main__":
    main()
