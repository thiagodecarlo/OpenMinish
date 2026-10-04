import sys
import os
import math
from PIL import Image, ImageDraw

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "cave_of_flames_showcase.png")

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (12, 10, 16, 255))
    draw = ImageDraw.Draw(img)

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(24, 18, 20, 255))
    draw.line([0, 48, img_w, 48], fill=(239, 68, 68, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Cave of Flames (Dungeon 2) Showcase", fill=(254, 240, 138, 255))
    draw.text((680, 16), "Item 7: feature/dungeon-cave-of-flames", fill=(248, 113, 113, 255))

    panels = [
        ("1. ENTRANCE VESTIBULE & LAVA CHANNELS", 20, 60, 460, 330),
        ("2. MINECART RAILWAY & CANE OF PACCI INVERSION", 500, 60, 940, 330),
        ("3. ROLLING SPIKED BARREL CORRIDOR", 20, 350, 460, 620),
        ("4. COMBAT CRUCIBLE, FIRE KEESE & BOSS DOOR", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(18, 16, 20, 255), outline=(60, 50, 55, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(35, 24, 28, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(220, 38, 38, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(251, 146, 60, 255))

    # =============================================================
    # PANEL 1: ENTRANCE VESTIBULE & LAVA CHANNELS
    # =============================================================
    p1_x, p1_y = 30, 94
    # Volcanic basalt floor
    for ry in range(14):
        for rx in range(26):
            tx = p1_x + rx * 16
            ty = p1_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(41, 37, 36, 255))
            draw.line([tx, ty + 15, tx + 15, ty + 15], fill=(68, 64, 60, 255))
            draw.line([tx + 15, ty, tx + 15, ty + 15], fill=(68, 64, 60, 255))

    # Molten lava channels on West (cols 1..3) and East (cols 22..24)
    for ry in range(1, 13):
        for rx in [2, 3, 22, 23]:
            lx = p1_x + rx * 16
            ly = p1_y + ry * 16
            draw.rectangle([lx, ly, lx + 15, ly + 15], fill=(245, 158, 11, 255))
            draw.rectangle([lx + 3, ly + 3, lx + 12, ly + 12], fill=(254, 240, 138, 255))
            draw.point((lx + 6, ly + 6), fill=(220, 38, 38, 255))

    # Stone pillars with glowing fire braziers
    for bx, by in [(p1_x + 6 * 16, p1_y + 3 * 16), (p1_x + 18 * 16, p1_y + 3 * 16)]:
        # Pedestal
        draw.rectangle([bx + 4, by + 8, bx + 12, by + 20], fill=(82, 82, 91, 255))
        draw.rectangle([bx + 2, by + 18, bx + 14, by + 22], fill=(39, 39, 42, 255))
        # Fire
        draw.rectangle([bx + 3, by + 1, bx + 13, by + 8], fill=(239, 68, 68, 255))
        draw.rectangle([bx + 5, by + 2, bx + 11, by + 6], fill=(249, 115, 22, 255))
        draw.rectangle([bx + 6, by, bx + 10, by + 4], fill=(253, 224, 71, 255))

    # Floor switch (Iron pressure plate)
    sw_x = p1_x + 7 * 16
    sw_y = p1_y + 8 * 16
    draw.rectangle([sw_x + 2, sw_y + 2, sw_x + 14, sw_y + 14], fill=(24, 24, 27, 255))
    draw.rectangle([sw_x + 4, sw_y + 4, sw_x + 12, sw_y + 12], fill=(16, 185, 129, 255)) # Activated green
    draw.text((sw_x - 14, sw_y + 16), "Switch [ON]", fill=(52, 211, 153, 255))

    # North Shutter Door (Opened)
    gate_x = p1_x + 11 * 16
    gate_y = p1_y
    draw.rectangle([gate_x, gate_y, gate_x + 36, gate_y + 18], fill=(12, 10, 9, 255))
    draw.rectangle([gate_x, gate_y, gate_x + 36, gate_y + 4], fill=(100, 116, 139, 255)) # Raised iron grate

    # Link standing near center
    lk_x = p1_x + 12 * 16
    lk_y = p1_y + 7 * 16
    draw.rectangle([lk_x + 3, lk_y, lk_x + 13, lk_y + 14], fill=(34, 197, 94, 255))
    draw.rectangle([lk_x + 5, lk_y + 4, lk_x + 11, lk_y + 9], fill=(254, 215, 170, 255))
    draw.rectangle([lk_x + 4, lk_y + 13, lk_x + 12, lk_y + 18], fill=(245, 158, 11, 255))
    draw.text((p1_x + 10, p1_y + 204), "Chamber 0: Melari's Mines connection & Shutter Gate", fill=(203, 213, 225, 255))

    # =============================================================
    # PANEL 2: MINECART RAILWAY & CANE OF PACCI INVERSION
    # =============================================================
    p2_x, p2_y = 510, 94
    # Volcanic floor platforms
    for ry in range(14):
        for rx in range(26):
            tx = p2_x + rx * 16
            ty = p2_y + ry * 16
            if 4 <= ry <= 9:
                # Boiling Lava Lake
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(220, 38, 38, 255))
                draw.rectangle([tx + 2, ty + 2, tx + 13, ty + 13], fill=(245, 158, 11, 255))
                if (rx + ry) % 4 == 0:
                    draw.rectangle([tx + 5, ty + 5, tx + 10, ty + 10], fill=(254, 240, 138, 255))
            else:
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(41, 37, 36, 255))
                draw.line([tx, ty + 15, tx + 15, ty + 15], fill=(68, 64, 60, 255))
                draw.line([tx + 15, ty, tx + 15, ty + 15], fill=(68, 64, 60, 255))

    # Railway Tracks crossing Lava Lake
    track_x = p2_x + 12 * 16
    for ry in range(2, 12):
        ty = p2_y + ry * 16
        # Wooden ties
        draw.rectangle([track_x - 8, ty + 4, track_x + 20, ty + 8], fill=(120, 53, 15, 255))
        # Iron rails
        draw.rectangle([track_x - 3, ty, track_x - 1, ty + 15], fill=(148, 163, 184, 255))
        draw.rectangle([track_x + 13, ty, track_x + 15, ty + 15], fill=(148, 163, 184, 255))

    # 1. Capsized Minecart (Flipped) on Left Side with Pacci Magic Beam
    mc1_x = p2_x + 4 * 16
    mc1_y = p2_y + 6 * 16
    # Flipped cart body (curved iron upside down)
    draw.rectangle([mc1_x, mc1_y + 4, mc1_x + 24, mc1_y + 16], fill=(71, 85, 105, 255))
    draw.rectangle([mc1_x + 2, mc1_y + 1, mc1_x + 22, mc1_y + 4], fill=(217, 119, 6, 255))
    draw.rectangle([mc1_x + 2, mc1_y - 2, mc1_x + 6, mc1_y + 2], fill=(30, 41, 59, 255)) # Wheels up
    draw.rectangle([mc1_x + 18, mc1_y - 2, mc1_x + 22, mc1_y + 2], fill=(30, 41, 59, 255))
    draw.text((mc1_x - 10, mc1_y + 20), "1. Capsized Cart", fill=(248, 113, 113, 255))

    # Cane of Pacci Cyan Energy Beam striking cart
    beam_x = mc1_x - 28
    beam_y = mc1_y + 6
    draw.line([beam_x, beam_y, mc1_x, beam_y], fill=(56, 189, 248, 255), width=3)
    draw.ellipse([mc1_x - 4, beam_y - 4, mc1_x + 4, beam_y + 4], fill=(186, 230, 253, 255))
    draw.text((beam_x - 16, beam_y - 14), "Pacci Zap!", fill=(56, 189, 248, 255))

    # 2. Upright Minecart on Track (Riding across lava)
    mc2_x = track_x - 6
    mc2_y = p2_y + 6 * 16
    # Upright cart
    draw.rectangle([mc2_x, mc2_y + 2, mc2_x + 24, mc2_y + 18], fill=(71, 85, 105, 255))
    draw.rectangle([mc2_x + 1, mc2_y + 1, mc2_x + 23, mc2_y + 4], fill=(217, 119, 6, 255))
    draw.rectangle([mc2_x + 4, mc2_y + 5, mc2_x + 20, mc2_y + 13], fill=(15, 23, 42, 255))
    # Link sitting inside cart
    draw.rectangle([mc2_x + 7, mc2_y - 2, mc2_x + 17, mc2_y + 6], fill=(34, 197, 94, 255))
    draw.rectangle([mc2_x + 9, mc2_y + 1, mc2_x + 15, mc2_y + 6], fill=(254, 215, 170, 255))
    # Sparks from wheels
    draw.point((mc2_x - 2, mc2_y + 18), fill=(253, 224, 71, 255))
    draw.point((mc2_x + 25, mc2_y + 18), fill=(253, 224, 71, 255))
    draw.text((mc2_x - 10, mc2_y + 22), "2. Link Riding Cart", fill=(74, 222, 128, 255))

    draw.text((p2_x + 10, p2_y + 204), "Chamber 1: Pacci flips minecart upright; Link rides across lava", fill=(203, 213, 225, 255))

    # =============================================================
    # PANEL 3: ROLLING SPIKED BARREL CORRIDOR
    # =============================================================
    p3_x, p3_y = 30, 384
    # Sunken corridor floor
    for ry in range(14):
        for rx in range(26):
            tx = p3_x + rx * 16
            ty = p3_y + ry * 16
            if 4 <= ry <= 8:
                # Sunken dark stone track
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(28, 25, 23, 255))
                draw.line([tx, ty + 15, tx + 15, ty + 15], fill=(41, 37, 36, 255))
            else:
                # Solid stone platforms (safe alcoves)
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(41, 37, 36, 255))
                draw.line([tx, ty + 15, tx + 15, ty + 15], fill=(68, 64, 60, 255))

    # Rolling Spiked Barrel (patrolling horizontally)
    rb_x = p3_x + 10 * 16
    rb_y = p3_y + 5 * 16
    # Steel cylinder body
    draw.rectangle([rb_x, rb_y + 4, rb_x + 72, rb_y + 24], fill=(51, 65, 85, 255))
    draw.rectangle([rb_x + 2, rb_y + 6, rb_x + 70, rb_y + 10], fill=(100, 116, 139, 255)) # Metallic sheen
    draw.rectangle([rb_x + 2, rb_y + 18, rb_x + 70, rb_y + 23], fill=(15, 23, 42, 255)) # Shadow
    # Sharp triangular steel spikes
    for s in range(7):
        sp_x = rb_x + 6 + s * 10
        # Top spikes
        draw.polygon([(sp_x, rb_y + 4), (sp_x + 3, rb_y - 3), (sp_x + 6, rb_y + 4)], fill=(226, 232, 240, 255))
        # Bottom spikes
        draw.polygon([(sp_x, rb_y + 24), (sp_x + 3, rb_y + 31), (sp_x + 6, rb_y + 24)], fill=(226, 232, 240, 255))

    # Oscillation direction arrows
    draw.line([rb_x - 18, rb_y + 14, rb_x - 6, rb_y + 14], fill=(239, 68, 68, 255), width=2)
    draw.line([rb_x + 78, rb_y + 14, rb_x + 90, rb_y + 14], fill=(239, 68, 68, 255), width=2)

    # Safe alcove Link
    lk_x2 = p3_x + 4 * 16
    lk_y2 = p3_y + 2 * 16
    draw.rectangle([lk_x2 + 3, lk_y2, lk_x2 + 13, lk_y2 + 14], fill=(34, 197, 94, 255))
    draw.rectangle([lk_x2 + 5, lk_y2 + 4, lk_x2 + 11, lk_y2 + 9], fill=(254, 215, 170, 255))
    draw.text((lk_x2 - 12, lk_y2 - 12), "Safe Alcove", fill=(52, 211, 153, 255))

    draw.text((p3_x + 10, p3_y + 204), "Chamber 2: Rolling Spiked Barrel obstacle with timing evasion", fill=(203, 213, 225, 255))

    # =============================================================
    # PANEL 4: COMBAT CRUCIBLE, FIRE KEESE & BOSS DOOR
    # =============================================================
    p4_x, p4_y = 510, 384
    # Volcanic arena floor
    for ry in range(14):
        for rx in range(26):
            tx = p4_x + rx * 16
            ty = p4_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(41, 37, 36, 255))
            draw.line([tx, ty + 15, tx + 15, ty + 15], fill=(68, 64, 60, 255))
            draw.line([tx + 15, ty, tx + 15, ty + 15], fill=(68, 64, 60, 255))

    # 1. Fire Keese Enemies
    for fk_x, fk_y in [(p4_x + 4 * 16, p4_y + 5 * 16), (p4_x + 16 * 16, p4_y + 6 * 16)]:
        # Shadow
        draw.ellipse([fk_x + 2, fk_y + 20, fk_x + 18, fk_y + 26], fill=(5, 16, 7, 100))
        # Body
        draw.rectangle([fk_x + 6, fk_y + 4, fk_x + 14, fk_y + 14], fill=(127, 29, 29, 255))
        # Fiery wings
        draw.polygon([(fk_x, fk_y), (fk_x + 6, fk_y + 6), (fk_x, fk_y + 12)], fill=(234, 88, 12, 255))
        draw.polygon([(fk_x + 20, fk_y), (fk_x + 14, fk_y + 6), (fk_x + 20, fk_y + 12)], fill=(234, 88, 12, 255))
        # Glowing yellow eyes & flame sparks
        draw.point((fk_x + 8, fk_y + 7), fill=(254, 240, 138, 255))
        draw.point((fk_x + 12, fk_y + 7), fill=(254, 240, 138, 255))
        draw.point((fk_x + 2, fk_y - 2), fill=(253, 224, 71, 255))
        draw.point((fk_x + 18, fk_y - 2), fill=(239, 68, 68, 255))

    draw.text((p4_x + 2 * 16, p4_y + 7 * 16 + 8), "Fire Keese (Flaming Bat)", fill=(251, 146, 60, 255))

    # 2. Golden Small Key Drop (Center)
    key_x = p4_x + 9 * 16
    key_y = p4_y + 8 * 16
    draw.ellipse([key_x - 6, key_y - 6, key_x + 18, key_y + 18], fill=(253, 224, 71, 80))
    draw.rectangle([key_x + 2, key_y, key_x + 10, key_y + 6], fill=(251, 191, 36, 255))
    draw.rectangle([key_x + 5, key_y + 6, key_x + 7, key_y + 14], fill=(251, 191, 36, 255))
    draw.rectangle([key_x + 7, key_y + 10, key_x + 10, key_y + 12], fill=(251, 191, 36, 255))
    draw.text((key_x - 18, key_y + 16), "Small Key Drop", fill=(253, 224, 71, 255))

    # 3. Massive Boss Door at North
    bd_x = p4_x + 17 * 16
    bd_y = p4_y + 1 * 16
    # Carved door frame
    draw.rectangle([bd_x, bd_y, bd_x + 48, bd_y + 36], fill=(69, 26, 3, 255))
    draw.rectangle([bd_x + 3, bd_y + 3, bd_x + 45, bd_y + 33], fill=(120, 53, 15, 255))
    # Flame emblem & skull relief
    draw.polygon([(bd_x + 24, bd_y + 6), (bd_x + 16, bd_y + 20), (bd_x + 32, bd_y + 20)], fill=(249, 115, 22, 255))
    draw.rectangle([bd_x + 21, bd_y + 14, bd_x + 27, bd_y + 20], fill=(253, 224, 71, 255))
    # Golden Padlock
    draw.rectangle([bd_x + 20, bd_y + 24, bd_x + 28, bd_y + 31], fill=(251, 191, 36, 255))
    draw.rectangle([bd_x + 22, bd_y + 21, bd_x + 26, bd_y + 24], fill=(148, 163, 184, 255))
    draw.text((bd_x - 8, bd_y + 40), "Boss Door (Gleerok)", fill=(248, 113, 113, 255))

    # Mini HUD Preview
    draw.rectangle([p4_x + 2, p4_y + 2, p4_x + 100, p4_y + 16], fill=(12, 28, 13, 255))
    draw.text((p4_x + 6, p4_y + 4), "HUD: [B] PACCI   KEY: x1", fill=(254, 240, 138, 255))

    draw.text((p4_x + 10, p4_y + 204), "Chamber 3 & 4: Defeat Fire Keese -> Collect Small Key -> Unlock Boss Door", fill=(203, 213, 225, 255))

    # Save artifact
    os.makedirs(out_dir, exist_ok=True)
    img.save(out_path)
    print(f"Showcase image successfully generated at: {out_path}")

if __name__ == "__main__":
    main()
