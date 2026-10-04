import sys
import os
import math
from PIL import Image, ImageDraw

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "castor_wilds_showcase.png")
    scratch_path = r"d:\REPOS\TLoZ-MC\scratch\castor_wilds_showcase.png"

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (8, 16, 12, 255))
    draw = ImageDraw.Draw(img)

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(12, 28, 18, 255))
    draw.line([0, 48, img_w, 48], fill=(34, 197, 94, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Castor Wilds Swamp, Pegasus Boots Mud Dash & Bow", fill=(254, 240, 138, 255))
    draw.text((640, 16), "Item 10: feature/castor-wilds-pegasus-swamp", fill=(74, 222, 128, 255))

    panels = [
        ("1. CASTOR WILDS SWAMP & EYE STATUES OVERVIEW", 20, 60, 460, 330),
        ("2. PEGASUS BOOTS MUD DASH VS SINKING PHYSICS", 500, 60, 940, 330),
        ("3. BOW & ARROW ACQUISITION & CELEBRATION BANNER", 20, 350, 460, 620),
        ("4. ARROW BALLISTICS, EYE STATUE OPEN & ROPE AI", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(14, 24, 16, 255), outline=(30, 58, 38, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(20, 38, 26, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(34, 197, 94, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(134, 239, 172, 255))

    # Color Palette definitions matching src/hal/map.c
    C_SWAMP_GRASS = (46, 74, 34, 255)
    C_SWAMP_MUD   = (42, 24, 16, 255)
    C_SWAMP_MUD_DARK = (28, 14, 7, 255)
    C_SWAMP_WATER = (18, 38, 48, 255)
    C_SWAMP_LOG   = (101, 67, 33, 255)
    C_TREE_TOP    = (21, 128, 61, 255)
    C_TREE_TRUNK  = (120, 53, 15, 255)

    def draw_swamp_background(px, py, w_tiles=26, h_tiles=14):
        for ry in range(h_tiles):
            for rx in range(w_tiles):
                lx = px + rx * 16
                ly = py + ry * 16
                draw.rectangle([lx, ly, lx + 15, ly + 15], fill=C_SWAMP_GRASS)
                if (rx + ry * 3) % 5 == 0:
                    draw.point((lx + 4, ly + 6), fill=(34, 197, 94, 255))
                    draw.point((lx + 10, ly + 12), fill=(22, 101, 52, 255))

    def draw_mud_lake(px, py, rx_start, ry_start, rx_end, ry_end):
        for ry in range(ry_start, ry_end):
            for rx in range(rx_start, rx_end):
                lx = px + rx * 16
                ly = py + ry * 16
                draw.rectangle([lx, ly, lx + 15, ly + 15], fill=C_SWAMP_MUD)
                # Mud ripples & bubbles
                if (rx + ry) % 3 == 0:
                    draw.line([lx + 2, ly + 8, lx + 13, ly + 8], fill=C_SWAMP_MUD_DARK, width=1)
                if (rx * 7 + ry * 13) % 11 == 0:
                    draw.ellipse([lx + 6, ly + 5, lx + 9, ly + 8], fill=(92, 62, 37, 255))

    def draw_swamp_log(lx, ly, length_tiles=4):
        for i in range(length_tiles):
            x = lx + i * 16
            draw.rectangle([x, ly + 2, x + 15, ly + 13], fill=C_SWAMP_LOG)
            draw.line([x, ly + 3, x + 15, ly + 3], fill=(139, 90, 43, 255))
            draw.line([x, ly + 12, x + 15, ly + 12], fill=(59, 39, 18, 255))
            draw.line([x + 15, ly + 2, x + 15, ly + 13], fill=(40, 25, 10, 255))

    def draw_eye_statue(lx, ly, is_open=False):
        # Slate monolith body
        draw.rectangle([lx + 2, ly + 1, lx + 13, ly + 14], fill=(71, 85, 105, 255), outline=(30, 41, 59, 255))
        if not is_open:
            # Closed eyelid fissure
            draw.line([lx + 4, ly + 7, lx + 11, ly + 7], fill=(15, 23, 42, 255), width=2)
            draw.point((lx + 7, ly + 6), fill=(100, 116, 139, 255))
            draw.point((lx + 8, ly + 6), fill=(100, 116, 139, 255))
        else:
            # Radiant open eye
            draw.ellipse([lx + 4, ly + 4, lx + 11, ly + 11], fill=(254, 240, 138, 255), outline=(8, 145, 178, 255))
            draw.ellipse([lx + 6, ly + 6, lx + 9, ly + 9], fill=(8, 145, 178, 255))
            draw.point((lx + 7, ly + 7), fill=(255, 255, 255, 255))
            # Magic glow
            draw.ellipse([lx + 1, ly + 1, lx + 14, ly + 14], outline=(103, 232, 249, 140))

    def draw_rope_enemy(lx, ly, direction="right", charging=False):
        # Rope body (segmented coiling snake)
        body_c = (217, 119, 6, 255) if not charging else (239, 68, 68, 255)
        belly_c = (254, 240, 138, 255)
        draw.ellipse([lx + 2, ly + 4, lx + 13, ly + 12], fill=body_c)
        draw.line([lx + 4, ly + 11, lx + 11, ly + 11], fill=belly_c, width=1)
        # Head with glaring red eyes and tongue
        if direction == "right":
            draw.ellipse([lx + 10, ly + 3, lx + 15, ly + 9], fill=body_c)
            draw.point((lx + 13, ly + 5), fill=(255, 255, 255, 255))
            draw.point((lx + 14, ly + 5), fill=(239, 68, 68, 255))
            if charging:
                draw.line([lx + 15, ly + 6, lx + 18, ly + 6], fill=(239, 68, 68, 255))
        elif direction == "left":
            draw.ellipse([lx + 1, ly + 3, lx + 6, ly + 9], fill=body_c)
            draw.point((lx + 3, ly + 5), fill=(255, 255, 255, 255))
            draw.point((lx + 2, ly + 5), fill=(239, 68, 68, 255))
            if charging:
                draw.line([lx + 1, ly + 6, lx - 2, ly + 6], fill=(239, 68, 68, 255))

    def draw_link_sprite(lx, ly, dir="down", sinking_pixels=0, dashing=False):
        draw_y = ly - sinking_pixels
        # Green tunic & hat
        draw.rectangle([lx + 4, draw_y + 4, lx + 12, draw_y + 14], fill=(34, 197, 94, 255)) # Green
        draw.rectangle([lx + 4, draw_y + 1, lx + 12, draw_y + 5], fill=(22, 101, 52, 255)) # Dark hat
        draw.rectangle([lx + 5, draw_y + 5, lx + 11, draw_y + 8], fill=(254, 226, 199, 255)) # Face
        draw.rectangle([lx + 5, draw_y + 13, lx + 11, draw_y + 16], fill=(255, 255, 255, 255)) # White undershirt
        # Boots
        if sinking_pixels < 4:
            draw.rectangle([lx + 4, draw_y + 15, lx + 7, draw_y + 18], fill=(139, 69, 19, 255))
            draw.rectangle([lx + 9, draw_y + 15, lx + 12, draw_y + 18], fill=(139, 69, 19, 255))

        # Mud cover if sinking
        if sinking_pixels > 0:
            draw.rectangle([lx + 2, ly + 16 - sinking_pixels, lx + 14, ly + 18], fill=C_SWAMP_MUD)
            draw.line([lx, ly + 16 - sinking_pixels, lx + 16, ly + 16 - sinking_pixels], fill=(92, 62, 37, 255))

        # Dash dust particles if dashing
        if dashing:
            for d in [(-6, 8), (-12, 12), (-18, 9), (-8, 14)]:
                draw.ellipse([lx + d[0], ly + d[1], lx + d[0] + 4, ly + d[1] + 4], fill=(255, 255, 255, 180))

    # -------------------------------------------------------------
    # PANEL 1: CASTOR WILDS SWAMP & EYE STATUES OVERVIEW
    # -------------------------------------------------------------
    p1_x, p1_y = 30, 95
    draw_swamp_background(p1_x, p1_y)
    # Huge Mud Lake in center
    draw_mud_lake(p1_x, p1_y, 4, 3, 18, 11)
    # Hollow log crossing the mud
    draw_swamp_log(p1_x + 6 * 16, p1_y + 6 * 16, 5)
    # Deep water swamp pool in southeast
    for ry in range(8, 13):
        for rx in range(19, 25):
            draw.rectangle([p1_x + rx * 16, p1_y + ry * 16, p1_x + rx * 16 + 15, p1_y + ry * 16 + 15], fill=C_SWAMP_WATER)
            if (rx + ry) % 4 == 0:
                draw.line([p1_x + rx * 16 + 2, p1_y + ry * 16 + 6, p1_x + rx * 16 + 12, p1_y + ry * 16 + 6], fill=(56, 189, 248, 160))

    # Eye Statues at ancient ruins
    draw_eye_statue(p1_x + 3 * 16, p1_y + 2 * 16, is_open=False)
    draw_eye_statue(p1_x + 8 * 16, p1_y + 2 * 16, is_open=False)
    # Chest on pedestal
    draw.rectangle([p1_x + 5 * 16 + 4, p1_y + 2 * 16 + 4, p1_x + 6 * 16 + 12, p1_y + 2 * 16 + 14], fill=(180, 83, 9, 255), outline=(250, 204, 21, 255))

    # Ropes slithering
    draw_rope_enemy(p1_x + 14 * 16, p1_y + 2 * 16, "left")
    draw_rope_enemy(p1_x + 19 * 16, p1_y + 5 * 16, "right")

    # Trees on perimeter
    for tx in range(0, 26, 2):
        draw.ellipse([p1_x + tx * 16, p1_y - 8, p1_x + tx * 16 + 28, p1_y + 16], fill=C_TREE_TOP)

    draw.rectangle([p1_x + 4, p1_y + 180, p1_x + 408, p1_y + 218], fill=(10, 20, 14, 220), outline=(34, 197, 94, 255))
    draw.text((p1_x + 12, p1_y + 185), "- Deep Mud Lakes (TILE_SWAMP_MUD) with sinking physics", fill=(254, 240, 138, 255))
    draw.text((p1_x + 12, p1_y + 200), "- Hollow Log Bridges (TILE_SWAMP_LOG) & Eye Statues (TILE_EYE_STATUE)", fill=(134, 239, 172, 255))

    # -------------------------------------------------------------
    # PANEL 2: PEGASUS BOOTS MUD DASH VS SINKING PHYSICS
    # -------------------------------------------------------------
    p2_x, p2_y = 510, 95
    draw_swamp_background(p2_x, p2_y)
    draw_mud_lake(p2_x, p2_y, 2, 2, 24, 12)

    # Left side: Link sinking walking normally
    draw.rectangle([p2_x + 20, p2_y + 30, p2_x + 190, p2_y + 160], fill=(20, 10, 6, 180), outline=(239, 68, 68, 255))
    draw.text((p2_x + 30, p2_y + 36), "NORMAL WALK IN MUD", fill=(239, 68, 68, 255))
    draw.text((p2_x + 30, p2_y + 50), "Speed: 0.38f (Sinks in 60 frames)", fill=(252, 165, 165, 255))
    draw_link_sprite(p2_x + 95, p2_y + 90, dir="down", sinking_pixels=5, dashing=False)
    # Bubbles of mud around Link
    draw.ellipse([p2_x + 85, p2_y + 102, p2_x + 91, p2_y + 106], fill=(92, 62, 37, 255))
    draw.ellipse([p2_x + 115, p2_y + 104, p2_x + 122, p2_y + 109], fill=(92, 62, 37, 255))
    draw.text((p2_x + 40, p2_y + 130), "Timer: 48/60 -> Hazard Damage!", fill=(254, 202, 202, 255))

    # Right side: Link dashing with Pegasus Boots
    draw.rectangle([p2_x + 215, p2_y + 30, p2_x + 395, p2_y + 160], fill=(10, 24, 14, 180), outline=(34, 197, 94, 255))
    draw.text((p2_x + 225, p2_y + 36), "PEGASUS BOOTS DASH [B]", fill=(34, 197, 94, 255))
    draw.text((p2_x + 225, p2_y + 50), "Speed: 1.85f (NO SINKING!)", fill=(134, 239, 172, 255))
    # Dashed speed lines
    for sl in range(4):
        draw.line([p2_x + 230 + sl * 15, p2_y + 105, p2_x + 250 + sl * 15, p2_y + 105], fill=(255, 255, 255, 140), width=2)
    draw_link_sprite(p2_x + 330, p2_y + 90, dir="right", sinking_pixels=0, dashing=True)
    draw.text((p2_x + 235, p2_y + 130), "mud_sink_timer = 0 | Surface Traversal", fill=(187, 247, 208, 255))

    draw.rectangle([p2_x + 4, p2_y + 180, p2_x + 408, p2_y + 218], fill=(10, 20, 14, 220), outline=(34, 197, 94, 255))
    draw.text((p2_x + 12, p2_y + 185), "- Pegasus Boots dash allows sprinting smoothly over deep swamp mud", fill=(254, 240, 138, 255))
    draw.text((p2_x + 12, p2_y + 200), "- Sinking to timer 60 inflicts 1 HP hazard damage & respawns safely", fill=(134, 239, 172, 255))

    # -------------------------------------------------------------
    # PANEL 3: BOW & ARROW ACQUISITION & CELEBRATION BANNER
    # -------------------------------------------------------------
    p3_x, p3_y = 30, 385
    draw_swamp_background(p3_x, p3_y)

    # Ancient Pedestal where Bow was resting
    draw.rectangle([p3_x + 40, p3_y + 20, p3_x + 110, p3_y + 70], fill=(51, 65, 85, 255), outline=(100, 116, 139, 255), width=2)
    draw.rectangle([p3_x + 55, p3_y + 30, p3_x + 95, p3_y + 55], fill=(30, 41, 59, 255))
    # Open Chest with Bow
    draw.rectangle([p3_x + 60, p3_y + 25, p3_x + 90, p3_y + 45], fill=(217, 119, 6, 255), outline=(250, 204, 21, 255))
    # Glowing Aura around Link
    draw.ellipse([p3_x + 175, p3_y + 20, p3_x + 225, p3_y + 70], fill=(34, 197, 94, 50), outline=(74, 222, 128, 200))
    draw_link_sprite(p3_x + 192, p3_y + 32, dir="down")

    # Celebration Banner (Matching main.c banner design!)
    bx, by, bw, bh = p3_x + 20, p3_y + 85, 380, 52
    draw.rectangle([bx - 2, by - 2, bx + bw + 4, by + bh + 4], fill=(6, 24, 6, 238))
    draw.rectangle([bx - 1, by - 1, bx + bw + 2, by + bh + 2], fill=(34, 197, 94, 255))
    draw.rectangle([bx, by, bx + bw, by + bh], fill=(10, 32, 10, 255))
    draw.rectangle([bx + 2, by + 2, bx + bw - 2, by + bh - 2], fill=(20, 53, 20, 238))

    # Bow Icon inside banner
    draw.arc([bx + 14, by + 10, bx + 36, by + 42], start=270, end=90, fill=(133, 77, 14, 255), width=3)
    draw.line([bx + 25, by + 10, bx + 25, by + 42], fill=(226, 232, 240, 255), width=2) # String
    draw.line([bx + 16, by + 26, bx + 42, by + 26], fill=(148, 163, 184, 255), width=2) # Arrow
    draw.polygon([(bx + 42, by + 23), (bx + 46, by + 26), (bx + 42, by + 29)], fill=(56, 189, 248, 255))

    draw.text((bx + 55, by + 10), "ARCO E FLECHAS OBTIDO!", fill=(253, 224, 71, 255))
    draw.text((bx + 55, by + 28), "Disparo a Distancia | 30 Flechas | Castor Wilds", fill=(52, 211, 153, 255))

    # HUD Status Bar representation
    draw.rectangle([p3_x + 20, p3_y + 155, p3_x + 400, p3_y + 185], fill=(12, 28, 13, 255), outline=(34, 197, 94, 255))
    # Hearts
    for h in range(3):
        draw.polygon([(p3_x + 30 + h * 14, p3_y + 168), (p3_x + 34 + h * 14, p3_y + 163), (p3_x + 38 + h * 14, p3_y + 168), (p3_x + 34 + h * 14, p3_y + 175)], fill=(230, 34, 34, 255))
    # Bow HUD icon [B]
    draw.rectangle([p3_x + 95, p3_y + 160, p3_x + 130, p3_y + 180], fill=(24, 40, 24, 255), outline=(74, 222, 128, 255))
    draw.text((p3_x + 100, p3_y + 164), "[B] BOW", fill=(134, 239, 172, 255))
    # Arrow Count
    draw.text((p3_x + 140, p3_y + 164), "30", fill=(56, 189, 248, 255))
    draw.text((p3_x + 170, p3_y + 164), "RUPEES: 50", fill=(74, 222, 128, 255))
    draw.rectangle([p3_x + 330, p3_y + 162, p3_x + 385, p3_y + 178], fill=(66, 135, 245, 255))
    draw.text((p3_x + 345, p3_y + 164), "USA", fill=(255, 255, 255, 255))

    draw.rectangle([p3_x + 4, p3_y + 195, p3_x + 408, p3_y + 225], fill=(10, 20, 14, 220), outline=(34, 197, 94, 255))
    draw.text((p3_x + 12, p3_y + 200), "- ITEM_BOW / ENTITY_ITEM_BOW synced with Inventory & HUD", fill=(254, 240, 138, 255))
    draw.text((p3_x + 12, p3_y + 212), "- Quiver starts with 30 active arrows, replenishable via pickups", fill=(134, 239, 172, 255))

    # -------------------------------------------------------------
    # PANEL 4: ARROW BALLISTICS, EYE STATUE OPEN & ROPE AI
    # -------------------------------------------------------------
    p4_x, p4_y = 510, 385
    draw_swamp_background(p4_x, p4_y)
    draw_mud_lake(p4_x, p4_y, 4, 3, 22, 11)

    # Link firing Bow to the right
    draw_link_sprite(p4_x + 40, p4_y + 60, dir="right")

    # Arrow 1 flying towards Eye Statue
    draw.line([p4_x + 80, p4_y + 68, p4_x + 130, p4_y + 68], fill=(148, 163, 184, 255), width=2)
    draw.polygon([(p4_x + 130, p4_y + 65), (p4_x + 135, p4_y + 68), (p4_x + 130, p4_y + 71)], fill=(56, 189, 248, 255))
    draw.text((p4_x + 85, p4_y + 50), "FLYING ARROW", fill=(56, 189, 248, 255))

    # Eye Statue struck and OPENED!
    draw_eye_statue(p4_x + 160, p4_y + 60, is_open=True)
    draw.ellipse([p4_x + 150, p4_y + 50, p4_x + 185, p4_y + 85], outline=(103, 232, 249, 200), width=2)
    draw.text((p4_x + 145, p4_y + 90), "EYE OPENED!", fill=(254, 240, 138, 255))
    draw.text((p4_x + 145, p4_y + 102), "SOUND_SECRET chime", fill=(134, 239, 172, 255))

    # Rope Enemy charging at Link!
    draw.line([p4_x + 360, p4_y + 135, p4_x + 270, p4_y + 135], fill=(239, 68, 68, 160), width=2)
    draw_rope_enemy(p4_x + 250, p4_y + 128, "left", charging=True)
    draw.text((p4_x + 270, p4_y + 115), "ROPE CHARGE!", fill=(239, 68, 68, 255))
    draw.text((p4_x + 270, p4_y + 145), "Speed: 2.8f on line-of-sight", fill=(252, 165, 165, 255))

    # Arrow 2 striking second Rope
    draw.line([p4_x + 300, p4_y + 40, p4_x + 345, p4_y + 40], fill=(148, 163, 184, 255), width=2)
    draw.polygon([(p4_x + 345, p4_y + 37), (p4_x + 350, p4_y + 40), (p4_x + 345, p4_y + 43)], fill=(56, 189, 248, 255))
    draw_rope_enemy(p4_x + 355, p4_y + 33, "left", charging=False)
    # Damage flash on Rope
    draw.ellipse([p4_x + 350, p4_y + 28, p4_x + 375, p4_y + 50], outline=(254, 240, 138, 255))
    draw.text((p4_x + 330, p4_y + 18), "2 HP DAMAGE HIT", fill=(254, 240, 138, 255))

    draw.rectangle([p4_x + 4, p4_y + 195, p4_x + 408, p4_y + 225], fill=(10, 20, 14, 220), outline=(34, 197, 94, 255))
    draw.text((p4_x + 12, p4_y + 200), "- Arrow triggers map_hit_eye_statue() converting TILE_EYE_STATUE", fill=(254, 240, 138, 255))
    draw.text((p4_x + 12, p4_y + 212), "- Rope AI stalks at 0.25f, aligns, and charges aggressively at 2.8f", fill=(134, 239, 172, 255))

    img.save(out_path)
    img.save(scratch_path)
    print(f"[SUCCESS] Verification image saved to {out_path} and {scratch_path}")

if __name__ == "__main__":
    main()
