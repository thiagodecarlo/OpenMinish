import sys
import os
import math
from PIL import Image, ImageDraw

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "cane_of_pacci_showcase.png")

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (12, 10, 16, 255))
    draw = ImageDraw.Draw(img)

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(20, 28, 36, 255))
    draw.line([0, 48, img_w, 48], fill=(56, 189, 248, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Cane of Pacci Showcase", fill=(253, 224, 71, 255))
    draw.text((700, 16), "Item 5: feature/cane-of-pacci", fill=(148, 163, 184, 255))

    panels = [
        ("1. CANE OF PACCI: MAGICAL ENERGY PROJECTILE & HUD", 20, 60, 460, 330),
        ("2. PACCI HOLES: PIT ENERGIZATION & CHARGED VORTEX", 500, 60, 940, 330),
        ("3. SUPER VERTICAL CATAPULT JUMP PHYSICS (Z-AXIS)", 20, 350, 460, 620),
        ("4. FLIPPING SPINY BEETLES & MINECARTS ON RAILS", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(18, 20, 28, 255), outline=(50, 60, 75, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(28, 36, 48, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(2, 132, 199, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(125, 211, 252, 255))

    # =============================================================
    # PANEL 1: CANE OF PACCI FIRING & HUD ICON
    # =============================================================
    p1_x, p1_y = 30, 94
    # Gravel terrain background
    for ry in range(14):
        for rx in range(26):
            tx = p1_x + rx * 16
            ty = p1_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(74, 55, 45, 255))
            if (rx + ry) % 4 == 0:
                draw.point((tx + 4, ty + 4), fill=(100, 75, 60, 255))
            if (rx * 2 + ry) % 7 == 0:
                draw.point((tx + 11, ty + 10), fill=(45, 32, 24, 255))

    # Link firing Cane of Pacci towards the right
    lx = p1_x + 60
    ly = p1_y + 90
    # Link sprite
    draw.rectangle([lx + 3, ly + 0, lx + 13, ly + 3], fill=(50, 205, 50, 255))
    draw.rectangle([lx + 2, ly + 3, lx + 14, ly + 6], fill=(50, 205, 50, 255))
    draw.rectangle([lx + 4, ly + 6, lx + 12, ly + 10], fill=(245, 203, 167, 255))
    draw.point((lx + 10, ly + 7), fill=(17, 17, 17, 255)) # eyes facing right
    draw.rectangle([lx + 3, ly + 10, lx + 13, ly + 15], fill=(34, 139, 34, 255)) # tunic
    draw.rectangle([lx + 4, ly + 13, lx + 12, ly + 15], fill=(139, 69, 19, 255)) # belt
    draw.rectangle([lx + 4, ly + 15, lx + 7, ly + 18], fill=(210, 105, 30, 255)) # boots
    draw.rectangle([lx + 9, ly + 15, lx + 12, ly + 18], fill=(210, 105, 30, 255))

    # Cane held in hand extended forward
    cx = lx + 14
    cy = ly + 8
    # Emerald/wooden shaft
    draw.line([cx, cy + 4, cx + 10, cy], fill=(22, 101, 52, 255), width=2)
    # Golden curved hook
    draw.arc([cx + 8, cy - 6, cx + 16, cy + 2], start=200, end=40, fill=(245, 158, 11, 255), width=2)
    # Cyan glowing orb at tip
    draw.ellipse([cx + 12, cy - 4, cx + 16, cy], fill=(56, 189, 248, 255), outline=(255, 255, 255, 255))

    # Magical Pacci energy projectile flying in mid-air
    proj_x = lx + 80
    proj_y = ly + 6
    # Outer cyan/azure aura
    draw.ellipse([proj_x - 10, proj_y - 10, proj_x + 10, proj_y + 10], fill=(2, 132, 199, 140))
    draw.ellipse([proj_x - 7, proj_y - 7, proj_x + 7, proj_y + 7], fill=(56, 189, 248, 200))
    draw.ellipse([proj_x - 4, proj_y - 4, proj_x + 4, proj_y + 4], fill=(255, 255, 255, 255)) # core
    # Swirling orbital sparks
    for a in range(6):
        ang = a * (math.pi / 3.0) + 0.4
        sx = proj_x + int(math.cos(ang) * 12)
        sy = proj_y + int(math.sin(ang) * 12)
        draw.rectangle([sx - 1, sy - 1, sx + 1, sy + 1], fill=(253, 224, 71, 255))

    # Sparkle dust trail behind projectile
    for t in range(5):
        tx = proj_x - (t + 1) * 12 + ((t % 2) * 4)
        ty = proj_y + ((t * 3) % 7 - 3)
        draw.ellipse([tx - 2, ty - 2, tx + 2, ty + 2], fill=(125, 211, 252, 180))

    # HUD Box in top-left of panel
    hx, hy = p1_x + 6, p1_y + 8
    # Hearts
    for h in range(3):
        draw.rectangle([hx + h * 12, hy, hx + h * 12 + 8, hy + 8], fill=(220, 38, 38, 255))
    # Button B HUD Slot with Cane of Pacci icon
    bx, by = p1_x + 6, p1_y + 22
    draw.rectangle([bx, by, bx + 28, by + 18], fill=(14, 38, 20, 255), outline=(212, 175, 55, 255), width=2)
    draw.text((bx + 3, by + 2), "B", fill=(255, 226, 122, 255))
    # Mini Cane icon inside HUD
    draw.line([bx + 16, by + 14, bx + 22, by + 8], fill=(22, 101, 52, 255), width=2)
    draw.point((bx + 24, by + 6), fill=(245, 158, 11, 255))
    draw.point((bx + 23, by + 5), fill=(245, 158, 11, 255))
    draw.rectangle([bx + 20, by + 5, bx + 22, by + 7], fill=(56, 189, 248, 255))

    draw.text((p1_x + 40, p1_y + 26), "ITEM_CANE_OF_PACCI (B)", fill=(253, 224, 71, 255))
    draw.text((p1_x + 20, p1_y + 190), "Disparo balistico veloz (speed 3.4 px/frame, alcance 115 px)", fill=(203, 213, 225, 255))
    draw.text((p1_x + 20, p1_y + 205), "Energiza buracos, desvira carrinhos e vira monstros", fill=(148, 163, 184, 255))

    # =============================================================
    # PANEL 2: PACCI HOLES: PIT ENERGIZATION & CHARGED VORTEX
    # =============================================================
    p2_x, p2_y = 510, 94
    for ry in range(14):
        for rx in range(26):
            tx = p2_x + rx * 16
            ty = p2_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(74, 55, 45, 255))
            if (rx + ry) % 5 == 0:
                draw.point((tx + 3, ty + 3), fill=(100, 75, 60, 255))

    # Left: Normal Hole (TILE_PACCI_HOLE_NORMAL)
    h1_cx = p2_x + 90
    h1_cy = p2_y + 90
    # Crater rim
    draw.ellipse([h1_cx - 24, h1_cy - 20, h1_cx + 24, h1_cy + 20], fill=(45, 27, 15, 255), outline=(63, 35, 18, 255), width=2)
    draw.ellipse([h1_cx - 18, h1_cy - 14, h1_cx + 18, h1_cy + 14], fill=(21, 12, 8, 255))
    draw.ellipse([h1_cx - 12, h1_cy - 8, h1_cx + 12, h1_cy + 8], fill=(10, 6, 4, 255))
    draw.text((h1_cx - 45, h1_cy + 28), "TILE_PACCI_HOLE_NORMAL", fill=(203, 213, 225, 255))
    draw.text((h1_cx - 30, h1_cy + 42), "(Buraco Normal)", fill=(148, 163, 184, 255))

    # Arrow transition
    draw.line([p2_x + 175, p2_y + 90, p2_x + 225, p2_y + 90], fill=(56, 189, 248, 255), width=3)
    draw.polygon([(p2_x + 230, p2_y + 90), (p2_x + 220, p2_y + 84), (p2_x + 220, p2_y + 96)], fill=(56, 189, 248, 255))
    draw.text((p2_x + 180, p2_y + 68), "Cane Hit", fill=(253, 224, 71, 255))

    # Right: Charged Hole (TILE_PACCI_HOLE_CHARGED)
    h2_cx = p2_x + 310
    h2_cy = p2_y + 90
    # Outer crater rim
    draw.ellipse([h2_cx - 24, h2_cy - 20, h2_cx + 24, h2_cy + 20], fill=(45, 27, 15, 255))
    # Swirling iridescent cyan energy vortex
    draw.ellipse([h2_cx - 20, h2_cy - 16, h2_cx + 20, h2_cy + 16], fill=(3, 105, 161, 255))
    draw.ellipse([h2_cx - 15, h2_cy - 12, h2_cx + 15, h2_cy + 12], fill=(2, 132, 199, 255))
    draw.ellipse([h2_cx - 10, h2_cy - 8, h2_cx + 10, h2_cy + 8], fill=(56, 189, 248, 255))
    draw.ellipse([h2_cx - 5, h2_cy - 4, h2_cx + 5, h2_cy + 4], fill=(255, 255, 255, 255)) # core
    # Upward energy rays / sparks
    for s in range(5):
        sx = h2_cx - 12 + s * 6
        sy = h2_cy - 10 - (s % 3) * 6
        draw.line([sx, h2_cy, sx, sy], fill=(253, 224, 71, 255), width=2)
        draw.rectangle([sx - 1, sy - 2, sx + 1, sy], fill=(255, 255, 255, 255))

    draw.text((h2_cx - 50, h2_cy + 28), "TILE_PACCI_HOLE_CHARGED", fill=(56, 189, 248, 255))
    draw.text((h2_cx - 36, h2_cy + 42), "(Vortice Energizado)", fill=(125, 211, 252, 255))
    draw.text((p2_x + 40, p2_y + 190), "Armazena energia magica para catapultar Link aos ceus", fill=(203, 213, 225, 255))

    # =============================================================
    # PANEL 3: SUPER VERTICAL CATAPULT JUMP PHYSICS (Z-AXIS)
    # =============================================================
    p3_x, p3_y = 30, 384
    # Draw craggy terrain with elevated cliff plateau
    for ry in range(14):
        for rx in range(26):
            tx = p3_x + rx * 16
            ty = p3_y + ry * 16
            # Lower valley (ry >= 8)
            if ry >= 8:
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(74, 55, 45, 255))
            # Plateau wall (ry == 6 or ry == 7)
            elif ry >= 6:
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(55, 40, 32, 255))
                draw.rectangle([tx, ty, tx + 15, ty + 2], fill=(85, 62, 48, 255))
            # Upper plateau (ry < 6)
            else:
                draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(95, 68, 52, 255))
                if (rx + ry) % 3 == 0:
                    draw.point((tx + 6, ty + 6), fill=(120, 90, 70, 255))

    # Charged hole in the valley
    ch_x = p3_x + 70
    ch_y = p3_y + 160
    draw.ellipse([ch_x - 16, ch_y - 8, ch_x + 16, ch_y + 8], fill=(2, 132, 199, 255))
    draw.ellipse([ch_x - 8, ch_y - 4, ch_x + 8, ch_y + 4], fill=(255, 255, 255, 255))

    # Parabolic trajectory arc
    arc_points = []
    for step in range(30):
        t = step / 29.0
        # Horizontal from ch_x to plateau (x = ch_x + 180)
        ax = ch_x + t * 190
        # Parabolic height reaching z = 55px
        h = math.sin(t * math.pi) * 65.0
        ay = (ch_y - t * 110) - h
        arc_points.append((ax, ay))

    for i in range(len(arc_points) - 1):
        draw.line([arc_points[i], arc_points[i+1]], fill=(56, 189, 248, 220), width=2)
        if i % 4 == 0:
            draw.rectangle([arc_points[i][0] - 1, arc_points[i][1] - 1, arc_points[i][0] + 1, arc_points[i][1] + 1], fill=(253, 224, 71, 255))

    # Link at apex of the jump! (t = 0.5)
    apex_x = ch_x + 0.5 * 190
    apex_y = (ch_y - 0.5 * 110) - 65.0
    # Ground shadow directly below Link
    shadow_y = ch_y - 0.5 * 110
    draw.ellipse([apex_x - 10, shadow_y - 4, apex_x + 10, shadow_y + 4], fill=(10, 8, 6, 160))

    # Link in mid-air with tuck jump posture
    draw.rectangle([apex_x - 5, apex_y - 9, apex_x + 5, apex_y - 5], fill=(50, 205, 50, 255)) # hat
    draw.rectangle([apex_x - 4, apex_y - 5, apex_x + 4, apex_y - 1], fill=(245, 203, 167, 255)) # face
    draw.rectangle([apex_x - 6, apex_y - 1, apex_x + 6, apex_y + 4], fill=(34, 139, 34, 255)) # tunic
    draw.rectangle([apex_x - 5, apex_y + 4, apex_x + 5, apex_y + 8], fill=(210, 105, 30, 255)) # boots tucked

    draw.text((apex_x - 36, apex_y - 24), "link.vz = 5.2f", fill=(253, 224, 71, 255))
    draw.text((apex_x - 30, apex_y - 12), "(Altitude Z)", fill=(125, 211, 252, 255))

    # Landing target on upper plateau
    land_x = ch_x + 190
    land_y = ch_y - 110
    draw.ellipse([land_x - 8, land_y - 4, land_x + 8, land_y + 4], fill=(34, 197, 94, 200))
    draw.text((land_x - 30, land_y + 8), "Plato Superior!", fill=(74, 222, 128, 255))

    draw.text((p3_x + 20, p3_y + 190), "Bypassa colisao com paredoes durante o voo (z > 4.0f)", fill=(203, 213, 225, 255))
    draw.text((p3_x + 20, p3_y + 205), "Permite subir patamares e cruzar abismos rochosos", fill=(148, 163, 184, 255))

    # =============================================================
    # PANEL 4: FLIPPING SPINY BEETLES & MINECARTS ON RAILS
    # =============================================================
    p4_x, p4_y = 510, 384
    for ry in range(14):
        for rx in range(26):
            tx = p4_x + rx * 16
            ty = p4_y + ry * 16
            draw.rectangle([tx, ty, tx + 15, ty + 15], fill=(74, 55, 45, 255))

    # Section A: Spiny Beetle Flipped
    sb_y = p4_y + 40
    # Left: Upright Spiny Beetle
    sb1_x = p4_x + 80
    # Rocky shell
    draw.rectangle([sb1_x - 10, sb_y - 8, sb1_x + 10, sb_y + 6], fill=(100, 116, 139, 255))
    draw.rectangle([sb1_x - 6, sb_y - 11, sb1_x + 6, sb_y - 8], fill=(203, 213, 225, 255)) # spikes
    # Red menacing eyes
    draw.point((sb1_x - 4, sb_y + 4), fill=(239, 68, 68, 255))
    draw.point((sb1_x + 4, sb_y + 4), fill=(239, 68, 68, 255))
    draw.text((sb1_x - 30, sb_y + 14), "1. Casca Dura", fill=(203, 213, 225, 255))
    draw.text((sb1_x - 36, sb_y + 26), "(Invulneravel)", fill=(239, 68, 68, 255))

    # Pacci Bolt Hit Arrow
    draw.line([p4_x + 130, sb_y, p4_x + 170, sb_y], fill=(56, 189, 248, 255), width=2)
    draw.polygon([(p4_x + 175, sb_y), (p4_x + 168, sb_y - 4), (p4_x + 168, sb_y + 4)], fill=(56, 189, 248, 255))

    # Right: Flipped Spiny Beetle (action == 3)
    sb2_x = p4_x + 220
    # Inverted rock shell at bottom
    draw.rectangle([sb2_x - 10, sb_y + 2, sb2_x + 10, sb_y + 10], fill=(100, 116, 139, 255))
    # Soft beige exposed underbelly facing up
    draw.rectangle([sb2_x - 8, sb_y - 4, sb2_x + 8, sb_y + 2], fill=(253, 230, 138, 255))
    draw.line([sb2_x - 3, sb_y - 1, sb2_x + 3, sb_y - 1], fill=(217, 119, 6, 255), width=1)
    # Legs wiggling up in the air!
    draw.line([sb2_x - 8, sb_y - 4, sb2_x - 11, sb_y - 10], fill=(30, 41, 59, 255), width=2)
    draw.line([sb2_x - 4, sb_y - 4, sb2_x - 5, sb_y - 11], fill=(30, 41, 59, 255), width=2)
    draw.line([sb2_x + 4, sb_y - 4, sb2_x + 5, sb_y - 11], fill=(30, 41, 59, 255), width=2)
    draw.line([sb2_x + 8, sb_y - 4, sb2_x + 11, sb_y - 10], fill=(30, 41, 59, 255), width=2)
    # Dizzy stars
    draw.text((sb2_x - 6, sb_y - 20), "*", fill=(253, 224, 71, 255))
    draw.text((sb2_x - 36, sb_y + 14), "2. Virado de Costas!", fill=(74, 222, 128, 255))
    draw.text((sb2_x - 36, sb_y + 26), "* 1-Hit Sword Kill *", fill=(253, 224, 71, 255))

    # Section B: Minecart on Rails
    mc_y = p4_y + 130
    # Railway tracks across the bottom
    for rx in range(24):
        tx = p4_x + 16 + rx * 16
        draw.line([tx, mc_y + 14, tx + 16, mc_y + 14], fill=(71, 85, 105, 255), width=2)
        if rx % 2 == 0:
            draw.rectangle([tx + 4, mc_y + 12, tx + 8, mc_y + 16], fill=(120, 53, 15, 255))

    # Left: Upside-down Minecart (TILE_MINECART_UPSIDE_DOWN)
    mc1_x = p4_x + 90
    draw.rectangle([mc1_x - 14, mc_y + 2, mc1_x + 14, mc_y + 12], fill=(100, 116, 139, 255), outline=(51, 65, 85, 255))
    draw.ellipse([mc1_x - 12, mc_y - 2, mc1_x - 6, mc_y + 4], fill=(148, 163, 184, 255)) # wheels up
    draw.ellipse([mc1_x + 6, mc_y - 2, mc1_x + 12, mc_y + 4], fill=(148, 163, 184, 255))
    draw.text((mc1_x - 42, mc_y + 20), "Carrinho Tombado", fill=(239, 68, 68, 255))

    # Arrow
    draw.line([p4_x + 170, mc_y + 6, p4_x + 210, mc_y + 6], fill=(56, 189, 248, 255), width=2)
    draw.polygon([(p4_x + 215, mc_y + 6), (p4_x + 208, mc_y + 2), (p4_x + 208, mc_y + 10)], fill=(56, 189, 248, 255))

    # Right: Upright Minecart (TILE_MINECART)
    mc2_x = p4_x + 280
    draw.rectangle([mc2_x - 14, mc_y - 2, mc2_x + 14, mc_y + 8], fill=(100, 116, 139, 255), outline=(51, 65, 85, 255))
    draw.rectangle([mc2_x - 10, mc_y - 1, mc2_x + 10, mc_y + 4], fill=(30, 41, 59, 255)) # interior bucket
    draw.ellipse([mc2_x - 12, mc_y + 8, mc2_x - 6, mc_y + 14], fill=(148, 163, 184, 255)) # wheels down
    draw.ellipse([mc2_x + 6, mc_y + 8, mc2_x + 12, mc_y + 14], fill=(148, 163, 184, 255))
    draw.text((mc2_x - 40, mc_y + 20), "Carrinho Desvirado!", fill=(74, 222, 128, 255))

    draw.text((p4_x + 30, p4_y + 205), "Inversao de orientacao canonica autentica de The Minish Cap", fill=(148, 163, 184, 255))

    # Save showcase image
    img.save(out_path)
    print(f"[OK] Cane of Pacci showcase gerado em: {out_path}")

if __name__ == "__main__":
    main()
