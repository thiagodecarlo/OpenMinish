"""
Script de Homologação Visual dos 4 Grandes Chefes de Masmorras (Opção 1)
- Gleerok (Cave of Flames)
- Mazaal (Fortress of Winds)
- Big Octo (Temple of Droplets)
- Gyorg Pair (Palace of Winds)

Gera showcase em alta fidelidade demonstrando mecânicas, HUDs canônicos GBA e táticas.
"""

import math
import os
from PIL import Image, ImageDraw, ImageFont

OUTPUT_PATH = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a\all_boss_battles_showcase.png"

def draw_oval_shadow(img, cx, cy, rx, ry, alpha=0.45):
    overlay = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(overlay)
    d.ellipse([(cx - rx, cy - ry), (cx + rx, cy + ry)], fill=(0, 0, 0, int(255 * alpha)))
    img.alpha_composite(overlay)

def render_link(draw, lx, ly, item="sword"):
    c_tunic = (22, 163, 74, 255)
    c_cap   = (34, 197, 94, 255)
    c_skin  = (254, 215, 170, 255)
    c_hair  = (202, 138, 4, 255)
    c_sword = (226, 232, 240, 255)
    c_hilt  = (234, 179, 8, 255)

    # Link body
    draw.rectangle([(lx - 5, ly - 8), (lx + 5, ly + 4)], fill=c_tunic)
    draw.rectangle([(lx - 4, ly + 4), (lx - 1, ly + 8)], fill=(120, 53, 15, 255))
    draw.rectangle([(lx + 1, ly + 4), (lx + 4, ly + 8)], fill=(120, 53, 15, 255))

    # Head & Cap
    draw.ellipse([(lx - 5, ly - 16), (lx + 5, ly - 7)], fill=c_skin)
    draw.polygon([(lx - 6, ly - 16), (lx + 6, ly - 16), (lx, ly - 23)], fill=c_cap)
    draw.ellipse([(lx + 2, ly - 24), (lx + 6, ly - 20)], fill=(250, 204, 21, 255)) # Ezlo beak
    draw.rectangle([(lx - 5, ly - 12), (lx - 2, ly - 9)], fill=c_hair)

    # Item / Weapon in hand
    if item == "sword":
        draw.line([(lx + 5, ly - 2), (lx + 14, ly - 10)], fill=c_sword, width=2)
        draw.rectangle([(lx + 4, ly - 3), (lx + 7, ly - 1)], fill=c_hilt)
    elif item == "pacci":
        draw.line([(lx + 5, ly + 2), (lx + 14, ly - 8)], fill=(168, 85, 247, 255), width=3)
        draw.ellipse([(lx + 12, ly - 12), (lx + 17, ly - 7)], fill=(56, 189, 248, 255))
    elif item == "lantern":
        draw.rectangle([(lx + 5, ly - 2), (lx + 11, ly + 6)], fill=(180, 83, 9, 255))
        draw.ellipse([(lx + 6, ly - 5), (lx + 10, ly)], fill=(245, 158, 11, 255))
    elif item == "cape":
        draw.polygon([(lx - 12, ly - 10), (lx - 4, ly - 6), (lx - 10, ly + 6)], fill=(185, 28, 28, 255))
        draw.polygon([(lx + 12, ly - 10), (lx + 4, ly - 6), (lx + 10, ly + 6)], fill=(185, 28, 28, 255))

def create_panel_gleerok(w, h):
    img = Image.new("RGBA", (w, h), (40, 15, 10, 255))
    draw = ImageDraw.Draw(img)

    # Lava floor with glowing heat waves
    for y in range(h):
        lava_glow = int(180 + 40 * math.sin(y * 0.1))
        draw.line([(0, y), (w, y)], fill=(lava_glow, int(lava_glow * 0.3), 10, 255))

    # Central volcanic rock island
    draw.ellipse([(30, 45), (w - 30, h - 25)], fill=(55, 45, 40, 255), outline=(120, 60, 30, 255), width=3)
    draw.ellipse([(45, 55), (w - 45, h - 35)], fill=(75, 65, 55, 255))

    # Gleerok (Colossal Lava Dragon)
    gx, gy = w // 2, 78
    # Massive neck & head
    draw.rounded_rectangle([(gx - 30, gy - 20), (gx + 30, gy + 35)], radius=12, fill=(185, 28, 28, 255), outline=(127, 29, 29, 255), width=3)
    # Volcanic Shell (Overturned / Inverted!)
    draw.ellipse([(gx - 38, gy + 10), (gx + 38, gy + 45)], fill=(68, 64, 60, 255), outline=(234, 88, 12, 255), width=3)
    # Pulsing Radiant Back Jewel (Vulnerable!)
    draw.ellipse([(gx - 12, gy + 20), (gx + 12, gy + 38)], fill=(239, 68, 68, 255), outline=(254, 240, 138, 255), width=2)
    draw.ellipse([(gx - 6, gy + 24), (gx + 6, gy + 34)], fill=(255, 255, 255, 255))

    # Dragon Head & Horns
    draw.ellipse([(gx - 22, gy - 38), (gx + 22, gy - 10)], fill=(220, 38, 38, 255), outline=(153, 27, 27, 255), width=2)
    draw.polygon([(gx - 18, gy - 34), (gx - 28, gy - 52), (gx - 12, gy - 36)], fill=(245, 158, 11, 255))
    draw.polygon([(gx + 18, gy - 34), (gx + 28, gy - 52), (gx + 12, gy - 36)], fill=(245, 158, 11, 255))
    # Glowing Eyes
    draw.ellipse([(gx - 14, gy - 26), (gx - 6, gy - 18)], fill=(254, 240, 138, 255))
    draw.ellipse([(gx + 6, gy - 26), (gx + 14, gy - 18)], fill=(254, 240, 138, 255))

    # Falling volcanic rocks
    for rx, ry, rrad in [(70, 75, 6), (200, 65, 8), (140, 45, 5)]:
        draw.ellipse([(rx - rrad, ry - rrad), (rx + rrad, ry + rrad)], fill=(68, 64, 60, 255), outline=(234, 88, 12, 255), width=2)
        draw.line([(rx, ry - rrad), (rx - 4, ry - rrad - 10)], fill=(249, 115, 22, 200), width=2)

    # Link with Cane of Pacci & Sword
    draw_oval_shadow(img, 90, 128, 12, 5)
    render_link(draw, 90, 122, item="pacci")

    # BOSS HUD: Moldura Vulcânica 3D de Bronze e Basalto
    bx, by, bw, bh = (w - 120) // 2, 10, 120, 8
    draw.rectangle([(bx - 3, by - 2), (bx + bw + 3, by + bh + 2)], fill=(28, 25, 23, 255))
    draw.rectangle([(bx - 2, by - 1), (bx + bw + 2, by + bh + 1)], fill=(217, 119, 6, 255))
    draw.rectangle([(bx, by + 1), (bx + bw, by + bh - 1)], fill=(15, 23, 42, 255))
    # Ruby gems on sides
    draw.rectangle([(bx - 6, by), (bx - 2, by + bh)], fill=(220, 38, 38, 255), outline=(254, 240, 138, 255))
    draw.rectangle([(bx + bw + 2, by), (bx + bw + 6, by + bh)], fill=(220, 38, 38, 255), outline=(254, 240, 138, 255))
    # 10 Pips (6 full fire pips)
    for p in range(10):
        px = bx + 3 + p * 11
        if p < 6:
            draw.rectangle([(px, by + 1), (px + 9, by + 3)], fill=(254, 240, 138, 255))
            draw.rectangle([(px, by + 4), (px + 9, by + bh - 2)], fill=(234, 88, 12, 255))
        else:
            draw.rectangle([(px, by + 2), (px + 9, by + bh - 2)], fill=(41, 37, 36, 255))

    draw.text((bx + 10, by - 8), "GLEEROK - DRAGAO DE FOGO", fill=(251, 191, 36, 255))
    draw.text((bx - 18, by + bh + 3), "* VULNERAVEL! ATAQUE A JOIA NO DORSO! *", fill=(254, 240, 138, 255))

    return img

def create_panel_mazaal(w, h):
    img = Image.new("RGBA", (w, h), (30, 41, 59, 255))
    draw = ImageDraw.Draw(img)

    # Fortress of Winds stone tiled floor
    for ty in range(0, h, 16):
        for tx in range(0, w, 16):
            c_tile = (51, 65, 85, 255) if ((tx + ty) // 16) % 2 == 0 else (71, 85, 105, 255)
            draw.rectangle([(tx, ty), (tx + 15, ty + 15)], fill=c_tile, outline=(30, 41, 59, 255))

    # Mazaal Colossal Ancient Stone Head
    mx, my = w // 2, 70
    draw.rounded_rectangle([(mx - 32, my - 24), (mx + 32, my + 24)], radius=8, fill=(148, 163, 184, 255), outline=(71, 85, 105, 255), width=3)
    # Ancient Carved Nose & Forehead Emblem
    draw.polygon([(mx - 8, my - 16), (mx + 8, my - 16), (mx, my - 4)], fill=(217, 119, 6, 255))
    draw.rectangle([(mx - 4, my - 4), (mx + 4, my + 10)], fill=(100, 116, 139, 255))

    # Mazaal Floating Mechanical Hands
    # Left Hand (Damaged/Exposed Palm Eye)
    hx1, hy1 = mx - 60, my + 12
    draw.rounded_rectangle([(hx1 - 18, hy1 - 16), (hx1 + 18, hy1 + 16)], radius=6, fill=(148, 163, 184, 255), outline=(71, 85, 105, 255), width=2)
    draw.ellipse([(hx1 - 8, hy1 - 8), (hx1 + 8, hy1 + 8)], fill=(239, 68, 68, 255), outline=(254, 240, 138, 255), width=2)
    draw.ellipse([(hx1 - 3, hy1 - 3), (hx1 + 3, hy1 + 3)], fill=(255, 255, 255, 255))

    # Right Hand (Slamming down!)
    hx2, hy2 = mx + 60, my + 20
    draw.rounded_rectangle([(hx2 - 18, hy2 - 16), (hx2 + 18, hy2 + 16)], radius=6, fill=(148, 163, 184, 255), outline=(71, 85, 105, 255), width=2)
    draw.ellipse([(hx2 - 8, hy2 - 8), (hx2 + 8, hy2 + 8)], fill=(34, 197, 94, 255), outline=(22, 101, 52, 255), width=2)

    # Link firing Bow at Left Hand Eye
    draw_oval_shadow(img, 110, 132, 10, 4)
    render_link(draw, 110, 126, item="sword")
    draw.line([(116, 122), (hx1 + 6, hy1 + 6)], fill=(234, 179, 8, 255), width=2) # Arrow trail

    # BOSS HUD: Moldura de Bronze Ancestral com Sensores Ópticos
    bx, by, bw, bh = (w - 120) // 2, 10, 120, 8
    draw.rectangle([(bx - 3, by - 2), (bx + bw + 3, by + bh + 2)], fill=(15, 23, 42, 255))
    draw.rectangle([(bx - 2, by - 1), (bx + bw + 2, by + bh + 1)], fill=(217, 119, 6, 255))
    draw.rectangle([(bx, by + 1), (bx + bw, by + bh - 1)], fill=(30, 41, 59, 255))
    # Optical sensors
    draw.rectangle([(bx - 18, by), (bx - 4, by + bh)], fill=(239, 68, 68, 255)) # E:OFF
    draw.text((bx - 17, by - 1), "E", fill=(255, 255, 255, 255))
    draw.rectangle([(bx + bw + 4, by), (bx + bw + 18, by + bh)], fill=(34, 197, 94, 255)) # D:ON
    draw.text((bx + bw + 7, by - 1), "D", fill=(255, 255, 255, 255))
    # Core Health Pips (4 Pips)
    for p in range(4):
        px = bx + 6 + p * 28
        draw.rectangle([(px, by + 1), (px + 22, by + bh - 2)], fill=(245, 158, 11, 255), outline=(254, 240, 138, 255))

    draw.text((bx + 14, by - 8), "MAZAAL - GUARDIAO ANCESTRAL", fill=(251, 191, 36, 255))
    draw.text((bx - 22, by + bh + 3), "* DERRUBE AS MAOS E ENCOLHA COMO MINISH! *", fill=(253, 224, 71, 255))

    return img

def create_panel_big_octo(w, h):
    img = Image.new("RGBA", (w, h), (15, 23, 42, 255))
    draw = ImageDraw.Draw(img)

    # Temple of Droplets Glacial Ice Floor
    for y in range(h):
        c_ice = int(186 + 30 * math.sin(y * 0.08))
        draw.line([(0, y), (w, y)], fill=(int(c_ice * 0.7), int(c_ice * 0.9), c_ice, 255))

    # Giant Big Octo
    ox, oy = w // 2, 75
    # Rounded colossal red octopus head
    draw.ellipse([(ox - 36, oy - 26), (ox + 36, oy + 26)], fill=(225, 29, 72, 255), outline=(159, 18, 57, 255), width=3)
    # Giant Spout / Snout
    draw.ellipse([(ox - 12, oy + 12), (ox + 12, oy + 32)], fill=(190, 18, 60, 255), outline=(136, 19, 55, 255), width=2)
    # Huge Glacial Octo Eyes
    draw.ellipse([(ox - 24, oy - 14), (ox - 10, oy)], fill=(255, 255, 255, 255), outline=(15, 23, 42, 255))
    draw.ellipse([(ox - 18, oy - 10), (ox - 12, oy - 4)], fill=(15, 23, 42, 255))
    draw.ellipse([(ox + 10, oy - 14), (ox + 24, oy)], fill=(255, 255, 255, 255), outline=(15, 23, 42, 255))
    draw.ellipse([(ox + 12, oy - 10), (ox + 18, oy - 4)], fill=(15, 23, 42, 255))

    # Freezing Spikes / Tentacles on sides
    draw.polygon([(ox - 36, oy), (ox - 56, oy + 18), (ox - 32, oy + 18)], fill=(159, 18, 57, 255))
    draw.polygon([(ox + 36, oy), (ox + 56, oy + 18), (ox + 32, oy + 18)], fill=(159, 18, 57, 255))

    # Giant Tail burning with Flame Lantern!
    tx, ty = ox + 28, oy - 22
    draw.ellipse([(tx - 10, ty - 10), (tx + 10, ty + 10)], fill=(234, 88, 12, 255), outline=(249, 115, 22, 255), width=2)
    draw.polygon([(tx - 8, ty - 4), (tx + 8, ty - 4), (tx, ty - 18)], fill=(254, 240, 138, 255))

    # Link with Flame Lantern charging tail
    draw_oval_shadow(img, 175, 125, 10, 4)
    render_link(draw, 175, 120, item="lantern")

    # BOSS HUD: Glacial com Safiras e 12 Pips de Vida
    bx, by, bw, bh = (w - 110) // 2, 10, 110, 8
    draw.rectangle([(bx - 3, by - 2), (bx + bw + 3, by + bh + 2)], fill=(15, 23, 42, 255))
    draw.rectangle([(bx - 2, by - 1), (bx + bw + 2, by + bh + 1)], fill=(56, 189, 248, 255))
    draw.rectangle([(bx, by + 1), (bx + bw, by + bh - 1)], fill=(8, 47, 73, 255))
    # Sapphire gems
    draw.rectangle([(bx - 6, by), (bx - 2, by + bh)], fill=(2, 132, 199, 255), outline=(186, 230, 253, 255))
    draw.rectangle([(bx + bw + 2, by), (bx + bw + 6, by + bh)], fill=(2, 132, 199, 255), outline=(186, 230, 253, 255))
    # 12 Pips (8 active with fire/ice gradient)
    for p in range(12):
        px = bx + 3 + p * 8
        if p < 8:
            draw.rectangle([(px, by + 1), (px + 6, by + 3)], fill=(253, 224, 71, 255))
            draw.rectangle([(px, by + 4), (px + 6, by + bh - 2)], fill=(234, 88, 12, 255))
        else:
            draw.rectangle([(px, by + 2), (px + 6, by + bh - 2)], fill=(30, 41, 59, 255))

    draw.text((bx + 14, by - 8), "BIG OCTO (EM CHAMAS)", fill=(248, 113, 113, 255))
    draw.text((bx - 16, by + bh + 3), "* CAUDA EM CHAMAS! ATAQUE COM A ESPADA! *", fill=(252, 165, 165, 255))

    return img

def create_panel_gyorg_pair(w, h):
    img = Image.new("RGBA", (w, h), (3, 105, 161, 255))
    draw = ImageDraw.Draw(img)

    # Cloud Tops / Palace of Winds Sky
    for c in [(30, 40, 40), (100, 30, 60), (180, 50, 50), (60, 130, 70), (190, 120, 60)]:
        cx, cy, cr = c
        draw.ellipse([(cx - cr, cy - cr // 2), (cx + cr, cy + cr // 2)], fill=(240, 249, 255, 180))

    # Female Blue Gyorg (Large Flying Ray)
    gx, gy = w // 2 - 25, 75
    # Wings
    draw.polygon([(gx, gy - 20), (gx - 55, gy - 4), (gx, gy + 20)], fill=(37, 99, 235, 255), outline=(30, 64, 175, 255))
    draw.polygon([(gx, gy - 20), (gx + 55, gy - 4), (gx, gy + 20)], fill=(37, 99, 235, 255), outline=(30, 64, 175, 255))
    # 3 Eyes on back (Vulnerable to 3 Clones)
    for ey in [-10, 0, 10]:
        draw.ellipse([(gx - 4, gy + ey - 4), (gx + 4, gy + ey + 4)], fill=(251, 191, 36, 255), outline=(153, 27, 27, 255), width=2)
        draw.ellipse([(gx - 2, gy + ey - 2), (gx + 2, gy + ey + 2)], fill=(185, 28, 28, 255))

    # Male Red Gyorg (Orbiting fast)
    rx, ry = w // 2 + 55, 60
    draw.polygon([(rx, ry - 14), (rx - 32, ry - 2), (rx, ry + 14)], fill=(220, 38, 38, 255), outline=(153, 27, 27, 255))
    draw.polygon([(rx, ry - 14), (rx + 32, ry - 2), (rx, ry + 14)], fill=(220, 38, 38, 255), outline=(153, 27, 27, 255))
    # Long curling tail
    draw.arc([(rx + 15, ry), (rx + 35, ry + 25)], start=0, end=180, fill=(185, 28, 28, 255), width=4)

    # Link with Roc's Cape & Minish Clones jumping between rays
    render_link(draw, gx - 14, gy + 4, item="sword")
    render_link(draw, gx, gy + 14, item="sword")
    render_link(draw, gx + 14, gy + 4, item="sword")

    # BOSS HUD: Duplo com 8 Pips Azuis + 8 Pips Vermelhos
    bx, by, bw, bh = (w - 114) // 2, 10, 114, 8
    draw.rectangle([(bx - 3, by - 2), (bx + bw + 3, by + bh + 2)], fill=(15, 23, 42, 255))
    draw.rectangle([(bx - 2, by - 1), (bx + bw + 2, by + bh + 1)], fill=(56, 189, 248, 255))
    draw.rectangle([(bx, by + 1), (bx + bw, by + bh - 1)], fill=(12, 19, 34, 255))
    # Gems (Left Blue, Right Red)
    draw.rectangle([(bx - 6, by), (bx - 2, by + bh)], fill=(37, 99, 235, 255))
    draw.rectangle([(bx + bw + 2, by), (bx + bw + 6, by + bh)], fill=(220, 38, 38, 255))
    # Separator
    draw.line([(bx + bw // 2, by + 1), (bx + bw // 2, by + bh - 1)], fill=(100, 116, 139, 255), width=2)

    # 8 Blue Pips (5 left)
    for p in range(8):
        px = bx + 2 + p * 6
        if p < 5:
            draw.rectangle([(px, by + 1), (px + 4, by + bh - 2)], fill=(37, 99, 235, 255), outline=(186, 230, 253, 255))
        else:
            draw.rectangle([(px, by + 2), (px + 4, by + bh - 2)], fill=(30, 41, 59, 255))

    # 8 Red Pips (4 left)
    for p in range(8):
        px = bx + (bw // 2) + 3 + p * 6
        if p < 4:
            draw.rectangle([(px, by + 1), (px + 4, by + bh - 2)], fill=(220, 38, 38, 255), outline=(254, 202, 202, 255))
        else:
            draw.rectangle([(px, by + 2), (px + 4, by + bh - 2)], fill=(30, 41, 59, 255))

    draw.text((bx + 8, by - 8), "GYORG PAIR - ARRAIAS DOS CEUS", fill=(253, 224, 71, 255))
    draw.text((bx - 16, by + bh + 3), "* CRIE 3 CLONES E GOLPEIE OS 3 OLHOS JUNTOS! *", fill=(56, 189, 248, 255))

    return img

def main():
    pw, ph = 240, 160
    header_h = 44
    cols, rows = 2, 2
    gap = 8

    total_w = cols * pw + (cols + 1) * gap
    total_h = header_h + rows * ph + (rows + 1) * gap

    canvas = Image.new("RGBA", (total_w, total_h), (10, 15, 26, 255))
    draw = ImageDraw.Draw(canvas)

    # Master Header
    draw.rectangle([(0, 0), (total_w, header_h)], fill=(15, 23, 42, 255))
    draw.line([(0, header_h), (total_w, header_h)], fill=(56, 189, 248, 255), width=2)
    draw.text((12, 6), "OPENMINISH - EXPANSÃO DO CICLO CANÔNICO DOS GRANDES CHEFES (OPÇÃO 1)", fill=(251, 191, 36, 255))
    draw.text((12, 24), "HAL C11 | HUDs Canônicos GBA | Câmera Cinemática Centróide | Zero Heap Allocation | 60 FPS", fill=(148, 163, 184, 255))

    panels = [
        ("Masmorra 2: Gleerok (Cave of Flames)", create_panel_gleerok(pw, ph)),
        ("Masmorra 3: Mazaal (Fortress of Winds)", create_panel_mazaal(pw, ph)),
        ("Masmorra 4: Big Octo (Temple of Droplets)", create_panel_big_octo(pw, ph)),
        ("Masmorra 5: Gyorg Pair (Palace of Winds)", create_panel_gyorg_pair(pw, ph)),
    ]

    for idx, (label, pimg) in enumerate(panels):
        c = idx % cols
        r = idx // cols
        px = gap + c * (pw + gap)
        py = header_h + gap + r * (ph + gap)

        canvas.paste(pimg, (px, py))
        draw.rectangle([(px, py), (px + pw, py + ph)], outline=(71, 85, 105, 255), width=2)
        # Label banner
        draw.rectangle([(px + 4, py + ph - 16), (px + pw - 4, py + ph - 2)], fill=(15, 23, 42, 220))
        draw.text((px + 8, py + ph - 15), label, fill=(241, 245, 249, 255))

    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    canvas.save(OUTPUT_PATH)
    print(f"Showcase salvo com sucesso em: {OUTPUT_PATH}")

if __name__ == "__main__":
    main()
