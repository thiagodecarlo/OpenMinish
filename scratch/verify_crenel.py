import sys
import os
import math
from PIL import Image, ImageDraw, ImageFont

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "mount_crenel_base_showcase.png")

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (12, 10, 16, 255))
    draw = ImageDraw.Draw(img)

    # 4 Panels:
    # Top-Left: (20, 60, 460, 320) -> Mount Crenel Base: Crags, Mineral Water & Climbable Walls
    # Top-Right: (500, 60, 940, 320) -> Enemies: Tektites & Spiny Beetles + Business Scrub
    # Bottom-Left: (20, 350, 460, 610) -> Grip Ring Climbing & Beanstalk Vine Mechanics
    # Bottom-Right: (500, 350, 940, 610) -> Business Scrub Dialogue, Shop & Inventory Item

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(30, 20, 15, 255))
    draw.line([0, 48, img_w, 48], fill=(212, 175, 55, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Mount Crenel Base & Grip Ring Showcase", fill=(253, 224, 71, 255))
    draw.text((720, 16), "Item 4: feature/mount-crenel-grip-ring", fill=(148, 163, 184, 255))

    panels = [
        ("1. MOUNT CRENEL BASE: VOLCANIC WALLS & MINERAL WATER", 20, 60, 460, 330),
        ("2. ENEMIES: TEKTITES, SPINY BEETLES & BUSINESS SCRUB", 500, 60, 940, 330),
        ("3. GRIP RING CLIMBING & MAGIC BEANSTALK MECHANICS", 20, 350, 460, 620),
        ("4. BUSINESS SCRUB DIALOGUE & GRIP RING INVENTORY", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(20, 18, 24, 255), outline=(70, 50, 40, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(38, 26, 20, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(180, 83, 9, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(251, 191, 36, 255))

    # -------------------------------------------------------------
    # PANEL 1: MOUNT CRENEL BASE OVERVIEW
    # -------------------------------------------------------------
    p1_x, p1_y = 30, 94
    # Draw craggy volcanic terrain
    for ry in range(14):
        for rx in range(26):
            tx = p1_x + rx * 16
            ty = p1_y + ry * 16
            # Gravel base
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(74, 55, 45, 255))
            draw.point((tx + 4, ty + 4), fill=(100, 75, 60, 255))
            draw.point((tx + 11, ty + 10), fill=(45, 32, 24, 255))

    # Cliff face on the west/north
    for ry in range(7):
        for rx in range(16):
            tx = p1_x + rx * 16
            ty = p1_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(55, 40, 32, 255))
            draw.rectangle([tx, ty, tx + 15, ty + 2], fill=(85, 62, 48, 255))
            draw.line([tx, ty + 8, tx + 14, ty + 12], fill=(35, 24, 18, 255))

    # Climbable wall with handholds / ledges
    for ry in range(7):
        for rx in range(6, 11):
            tx = p1_x + rx * 16
            ty = p1_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(70, 52, 40, 255))
            # Handholds
            draw.rectangle([tx + 3, ty + 4, tx + 7, ty + 7], fill=(140, 105, 75, 255))
            draw.rectangle([tx + 9, ty + 10, tx + 13, ty + 13], fill=(140, 105, 75, 255))
            draw.line([tx + 2, ty + 8, tx + 8, ty + 8], fill=(35, 24, 18, 255))

    # Mineral water pool with bubbling animation
    for ry in range(8, 12):
        for rx in range(16, 23):
            tx = p1_x + rx * 16
            ty = p1_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(20, 115, 130, 255))
            draw.rectangle([tx + 2, ty + 2, tx + 13, ty + 13], fill=(40, 160, 175, 255))
            # Bubbles
            draw.ellipse([tx + 4, ty + 4, tx + 7, ty + 7], fill=(200, 245, 255, 255))
            draw.ellipse([tx + 10, ty + 9, tx + 12, ty + 11], fill=(255, 255, 255, 255))

    # Annotations on Panel 1
    draw.text((p1_x + 10, p1_y + 190), "[Climbable Cliff Face with Handholds]", fill=(253, 224, 71, 255))
    draw.text((p1_x + 250, p1_y + 190), "[Mineral Water Springs]", fill=(56, 189, 248, 255))

    # -------------------------------------------------------------
    # PANEL 2: ENEMIES & BUSINESS SCRUB
    # -------------------------------------------------------------
    p2_x, p2_y = 510, 94

    # 1. Tektite Showcase (Ground pose & Airborne jump pose)
    draw.rectangle([p2_x + 10, p2_y + 10, p2_x + 190, p2_y + 130], fill=(28, 24, 32, 255), outline=(90, 60, 50, 255))
    draw.text((p2_x + 18, p2_y + 16), "TEKTITE (Volcanic Leaper)", fill=(239, 68, 68, 255))

    # Tektite 1: On ground
    t1_x = p2_x + 35
    t1_y = p2_y + 70
    draw.ellipse([t1_x - 10, t1_y + 14, t1_x + 10, t1_y + 20], fill=(0, 0, 0, 80)) # shadow
    # Body
    draw.rectangle([t1_x - 10, t1_y - 8, t1_x + 10, t1_y + 8], fill=(220, 38, 38, 255))
    draw.rectangle([t1_x - 8, t1_y - 12, t1_x + 8, t1_y + 12], fill=(185, 28, 28, 255))
    # Eye
    draw.rectangle([t1_x - 3, t1_y - 2, t1_x + 3, t1_y + 3], fill=(254, 240, 138, 255))
    draw.point((t1_x, t1_y + 1), fill=(17, 17, 17, 255))
    # 4 jointed legs spread
    draw.line([t1_x - 10, t1_y - 4, t1_x - 20, t1_y - 10], fill=(120, 53, 15, 255), width=2)
    draw.line([t1_x - 20, t1_y - 10, t1_x - 26, t1_y + 16], fill=(120, 53, 15, 255), width=2)
    draw.line([t1_x + 10, t1_y - 4, t1_x + 20, t1_y - 10], fill=(120, 53, 15, 255), width=2)
    draw.line([t1_x + 20, t1_y - 10, t1_x + 26, t1_y + 16], fill=(120, 53, 15, 255), width=2)
    draw.text((t1_x - 22, t1_y + 24), "Ground Stance", fill=(203, 213, 225, 255))

    # Tektite 2: Airborne leap
    t2_x = p2_x + 130
    t2_y = p2_y + 50
    draw.ellipse([t2_x - 8, p2_y + 84, t2_x + 8, p2_y + 90], fill=(0, 0, 0, 60)) # ground shadow
    draw.rectangle([t2_x - 10, t2_y - 8, t2_x + 10, t2_y + 8], fill=(220, 38, 38, 255))
    draw.rectangle([t2_x - 8, t2_y - 12, t2_x + 8, t2_y + 12], fill=(185, 28, 28, 255))
    draw.rectangle([t2_x - 3, t2_y - 2, t2_x + 3, t2_y + 3], fill=(254, 240, 138, 255))
    draw.point((t2_x, t2_y + 1), fill=(17, 17, 17, 255))
    # Bent jumping legs
    draw.line([t2_x - 8, t2_y + 4, t2_x - 16, t2_y + 14], fill=(120, 53, 15, 255), width=2)
    draw.line([t2_x - 16, t2_y + 14, t2_x - 10, t2_y + 24], fill=(120, 53, 15, 255), width=2)
    draw.line([t2_x + 8, t2_y + 4, t2_x + 16, t2_y + 14], fill=(120, 53, 15, 255), width=2)
    draw.line([t2_x + 16, t2_y + 14, t2_x + 10, t2_y + 24], fill=(120, 53, 15, 255), width=2)
    draw.text((t2_x - 22, p2_y + 96), "Airborne Arc (z)", fill=(252, 165, 165, 255))

    # 2. Spiny Beetle Showcase
    draw.rectangle([p2_x + 210, p2_y + 10, p2_x + 410, p2_y + 130], fill=(28, 24, 32, 255), outline=(90, 60, 50, 255))
    draw.text((p2_x + 220, p2_y + 16), "SPINY BEETLE (Spiked Rock)", fill=(148, 163, 184, 255))

    sb_x = p2_x + 310
    sb_y = p2_y + 68
    # Insect legs scurrying
    for leg_dx, leg_dy in [(-18, -6), (-18, 2), (-18, 10), (18, -6), (18, 2), (18, 10)]:
        draw.line([sb_x + leg_dx//2, sb_y + leg_dy, sb_x + leg_dx, sb_y + leg_dy + 4], fill=(30, 41, 59, 255), width=2)
    # Spiked craggy rock shell
    draw.rectangle([sb_x - 14, sb_y - 12, sb_x + 14, sb_y + 12], fill=(100, 116, 139, 255))
    draw.polygon([(sb_x - 2, sb_y - 20), (sb_x + 2, sb_y - 20), (sb_x + 6, sb_y - 12), (sb_x - 6, sb_y - 12)], fill=(203, 213, 225, 255))
    draw.polygon([(sb_x - 18, sb_y - 4), (sb_x - 14, sb_y - 10), (sb_x - 14, sb_y + 4)], fill=(148, 163, 184, 255))
    draw.polygon([(sb_x + 18, sb_y + 2), (sb_x + 14, sb_y - 4), (sb_x + 14, sb_y + 8)], fill=(148, 163, 184, 255))
    # Glowing eyes
    draw.point((sb_x - 6, sb_y + 13), fill=(239, 68, 68, 255))
    draw.point((sb_x + 6, sb_y + 13), fill=(239, 68, 68, 255))
    draw.text((sb_x - 45, sb_y + 24), "Camouflaged Shell", fill=(203, 213, 225, 255))

    # 3. Business Scrub (Deku Merchant)
    draw.rectangle([p2_x + 10, p2_y + 145, p2_x + 410, p2_y + 225], fill=(28, 24, 32, 255), outline=(90, 60, 50, 255))
    scrub_x = p2_x + 45
    scrub_y = p2_y + 185
    # Bush base
    draw.ellipse([scrub_x - 18, scrub_y + 2, scrub_x + 18, scrub_y + 20], fill=(22, 163, 74, 255))
    draw.ellipse([scrub_x - 12, scrub_y - 2, scrub_x + 12, scrub_y + 14], fill=(34, 197, 94, 255))
    # Scrub head
    draw.ellipse([scrub_x - 10, scrub_y - 16, scrub_x + 10, scrub_y], fill=(120, 53, 15, 255))
    # Snout
    draw.rectangle([scrub_x - 6, scrub_y - 6, scrub_x + 6, scrub_y + 2], fill=(154, 52, 18, 255))
    draw.ellipse([scrub_x - 4, scrub_y - 4, scrub_x + 4, scrub_y], fill=(69, 26, 3, 255))
    # Palm leaves crown
    draw.polygon([(scrub_x, scrub_y - 26), (scrub_x - 12, scrub_y - 16), (scrub_x + 12, scrub_y - 16)], fill=(34, 197, 94, 255))
    draw.point((scrub_x - 5, scrub_y - 10), fill=(253, 224, 71, 255))
    draw.point((scrub_x + 5, scrub_y - 10), fill=(253, 224, 71, 255))

    draw.text((p2_x + 90, p2_y + 158), "BUSINESS SCRUB (Deku Merchant)", fill=(245, 158, 11, 255))
    draw.text((p2_x + 90, p2_y + 176), "Sells Grip Ring for 40 Rupees!", fill=(253, 224, 71, 255))
    draw.text((p2_x + 90, p2_y + 194), "Enables scaling craggy walls & giant bean vines.", fill=(203, 213, 225, 255))

    # -------------------------------------------------------------
    # PANEL 3: GRIP RING CLIMBING & BEANSTALK
    # -------------------------------------------------------------
    p3_x, p3_y = 30, 384

    # Cliff wall background
    for ry in range(14):
        for rx in range(12):
            tx = p3_x + rx * 16
            ty = p3_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(70, 52, 40, 255))
            draw.rectangle([tx + 3, ty + 4, tx + 7, ty + 7], fill=(140, 105, 75, 255))
            draw.rectangle([tx + 9, ty + 10, tx + 13, ty + 13], fill=(140, 105, 75, 255))

    # Link in climbing pose on the cliff wall
    lk_x = p3_x + 96
    lk_y = p3_y + 90
    # Link body facing wall
    draw.rectangle([lk_x - 6, lk_y - 8, lk_x + 6, lk_y + 8], fill=(34, 139, 34, 255)) # green tunic
    draw.rectangle([lk_x - 5, lk_y - 14, lk_x + 5, lk_y - 8], fill=(50, 205, 50, 255)) # hat
    draw.polygon([(lk_x, lk_y - 18), (lk_x - 4, lk_y - 14), (lk_x + 4, lk_y - 14)], fill=(50, 205, 50, 255))
    draw.rectangle([lk_x - 4, lk_y + 8, lk_x - 1, lk_y + 14], fill=(210, 105, 30, 255)) # left boot climbing
    draw.rectangle([lk_x + 1, lk_y + 6, lk_x + 4, lk_y + 12], fill=(210, 105, 30, 255)) # right boot
    # Hands grabbing rock with Grip Ring golden glint
    draw.rectangle([lk_x - 10, lk_y - 14, lk_x - 6, lk_y - 10], fill=(253, 224, 71, 255)) # Gold Ring L
    draw.point((lk_x - 8, lk_y - 12), fill=(239, 68, 68, 255)) # ruby
    draw.rectangle([lk_x + 6, lk_y - 10, lk_x + 10, lk_y - 6], fill=(253, 224, 71, 255)) # Gold Ring R
    draw.point((lk_x + 8, lk_y - 8), fill=(239, 68, 68, 255)) # ruby

    draw.text((p3_x + 10, p3_y + 195), "Link Scaling Cliff (Grip Ring Active)", fill=(253, 224, 71, 255))

    # Giant Beanstalk on the right side of Panel 3
    for vy in range(14):
        vx = p3_x + 280
        vy_pos = p3_y + vy * 16
        draw.rectangle([vx - 6, vy_pos, vx + 6, vy_pos + 15], fill=(34, 197, 94, 255))
        draw.ellipse([vx - 14, vy_pos + 2, vx - 2, vy_pos + 10], fill=(22, 163, 74, 255)) # left leaf
        draw.ellipse([vx + 2, vy_pos + 6, vx + 14, vy_pos + 14], fill=(74, 222, 128, 255)) # right leaf

    draw.text((p3_x + 225, p3_y + 195), "Giant Beanstalk (Mineral Water)", fill=(74, 222, 128, 255))

    # -------------------------------------------------------------
    # PANEL 4: BUSINESS SCRUB DIALOGUE & GRIP RING INVENTORY
    # -------------------------------------------------------------
    p4_x, p4_y = 510, 384

    # Dialogue Box
    box_w = 400
    box_h = 110
    draw.rectangle([p4_x + 10, p4_y + 10, p4_x + box_w, p4_y + box_h], fill=(16, 12, 18, 250), outline=(212, 175, 55, 255), width=2)
    # Speaker badge
    draw.rectangle([p4_x + 18, p4_y + 2, p4_x + 140, p4_y + 18], fill=(30, 20, 15, 255), outline=(245, 158, 11, 255))
    draw.text((p4_x + 24, p4_y + 4), "DEKU SCRUB", fill=(245, 158, 11, 255))

    # Portrait 48x48
    port_x = p4_x + 20
    port_y = p4_y + 24
    draw.rectangle([port_x, port_y, port_x + 48, port_y + 48], fill=(30, 18, 8, 255), outline=(212, 175, 55, 255))
    # Scrub portrait
    draw.ellipse([port_x + 8, port_y + 26, port_x + 40, port_y + 46], fill=(22, 163, 74, 255))
    draw.ellipse([port_x + 12, port_y + 12, port_x + 36, port_y + 36], fill=(120, 53, 15, 255))
    draw.rectangle([port_x + 18, port_y + 22, port_x + 30, port_y + 32], fill=(154, 52, 18, 255))
    draw.point((port_x + 16, port_y + 18), fill=(253, 224, 71, 255))
    draw.point((port_x + 32, port_y + 18), fill=(253, 224, 71, 255))
    draw.polygon([(port_x + 24, port_y + 4), (port_x + 10, port_y + 14), (port_x + 38, port_y + 14)], fill=(34, 197, 94, 255))

    # Dialogue text
    draw.text((p4_x + 80, p4_y + 26), "Ola viajante! Quer subir o Monte Crenel?", fill=(254, 243, 199, 255))
    draw.text((p4_x + 80, p4_y + 42), "As encostas sao escarpadas demais a mao livre!", fill=(254, 243, 199, 255))
    draw.text((p4_x + 80, p4_y + 58), "Compre meu GRIP RING por 40 Rupees!", fill=(253, 224, 71, 255))
    draw.text((p4_x + 80, p4_y + 74), "[A] Comprar Grip Ring  [B] Recusar", fill=(74, 222, 128, 255))

    # Inventory HUD Preview Box
    draw.rectangle([p4_x + 10, p4_y + 130, p4_x + box_w, p4_y + 220], fill=(20, 24, 30, 255), outline=(70, 80, 95, 255))
    draw.text((p4_x + 20, p4_y + 136), "INVENTORY [START] - GRIP RING ACQUIRED", fill=(148, 163, 184, 255))

    # Grip ring icon large (40x40)
    gr_x = p4_x + 30
    gr_y = p4_y + 160
    draw.ellipse([gr_x, gr_y, gr_x + 36, gr_y + 36], fill=(217, 119, 6, 255))
    draw.ellipse([gr_x + 4, gr_y + 4, gr_x + 32, gr_y + 32], fill=(253, 224, 71, 255))
    draw.ellipse([gr_x + 10, gr_y + 10, gr_x + 26, gr_y + 26], fill=(20, 24, 30, 255))
    # Ruby gem at top
    draw.polygon([(gr_x + 18, gr_y - 2), (gr_x + 12, gr_y + 6), (gr_x + 24, gr_y + 6)], fill=(239, 68, 68, 255))
    draw.polygon([(gr_x + 18, gr_y + 12), (gr_x + 12, gr_y + 6), (gr_x + 24, gr_y + 6)], fill=(185, 28, 28, 255))

    draw.text((gr_x + 50, gr_y + 4), "GRIP RING (Anel de Escalada)", fill=(253, 224, 71, 255))
    draw.text((gr_x + 50, gr_y + 20), "Garras douradas que fincam em fendas de rocha.", fill=(203, 213, 225, 255))
    draw.text((gr_x + 50, gr_y + 34), "Status: ATIVO (Escalada automatica em paredoes e vinhas)", fill=(52, 211, 153, 255))

    img.save(out_path, "PNG")
    print(f"Showcase generated at {out_path}")

if __name__ == "__main__":
    main()
