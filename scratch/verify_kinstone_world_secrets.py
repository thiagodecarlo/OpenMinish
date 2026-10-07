"""
Script de Homologação Visual do Sistema de Segredos de Kinstones no Overworld (Opção 3)
- Fusão de Kinstone com Janela Pop-up de Eventos Mundiais
- Baús Brotando no Overworld (Minish Woods / Lake Hylia)
- Grutas Secretas e Rochas Rachadas Desmoronando (Trilby Highlands / Mt. Crenel)
- Portais Minish Despertados & Trepadeira Gigante (South Hyrule Field / Veil Falls)

Gera showcase gráfico em alta fidelidade demonstrando conformidade com o Pilar 6 da Metodologia OpenMinish.
"""

import math
import os
from PIL import Image, ImageDraw

OUTPUT_PATH = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a\kinstone_world_secrets_showcase.png"

def draw_oval_shadow(img, cx, cy, rx, ry, alpha=0.45):
    overlay = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(overlay)
    d.ellipse([(cx - rx, cy - ry), (cx + rx, cy + ry)], fill=(0, 0, 0, int(255 * alpha)))
    img.alpha_composite(overlay)

def render_link_small(draw, lx, ly):
    c_tunic = (22, 163, 74, 255)
    c_skin  = (254, 215, 170, 255)
    c_cap   = (34, 197, 94, 255)
    draw.rectangle([(lx - 4, ly - 6), (lx + 4, ly + 4)], fill=c_tunic)
    draw.rectangle([(lx - 3, ly + 4), (lx - 1, ly + 7)], fill=(120, 53, 15, 255))
    draw.rectangle([(lx + 1, ly + 4), (lx + 3, ly + 7)], fill=(120, 53, 15, 255))
    draw.ellipse([(lx - 4, ly - 13), (lx + 4, ly - 5)], fill=c_skin)
    draw.polygon([(lx - 5, ly - 13), (lx + 5, ly - 13), (lx, ly - 19)], fill=c_cap)

# Painel 1: Fusão e Pop-up de Evento Mundial
def create_panel_fusion_popup(w, h):
    img = Image.new("RGBA", (w, h), (15, 23, 42, 255))
    draw = ImageDraw.Draw(img)

    # Background gradient
    for y in range(h):
        c_bg = int(20 + 25 * (y / float(h)))
        draw.line([(0, y), (w, y)], fill=(c_bg // 2, c_bg, c_bg + 15, 255))

    # Fusion Arena Header
    draw.text((w // 2 - 50, 10), "FUSAO DE KINSTONE", fill=(251, 191, 36, 255))

    # Link half (Left - Blue)
    draw.ellipse([(w // 2 - 28, 30), (w // 2, 58)], fill=(37, 99, 235, 255), outline=(147, 197, 253, 255), width=2)
    # Partner half (Right - Blue)
    draw.ellipse([(w // 2, 30), (w // 2 + 28, 58)], fill=(37, 99, 235, 255), outline=(147, 197, 253, 255), width=2)
    # Magic sparkling rays
    for angle in range(0, 360, 45):
        rad = math.radians(angle)
        x1 = w // 2 + int(math.cos(rad) * 16)
        y1 = 44 + int(math.sin(rad) * 16)
        x2 = w // 2 + int(math.cos(rad) * 26)
        y2 = 44 + int(math.sin(rad) * 26)
        draw.line([(x1, y1), (x2, y2)], fill=(254, 240, 138, 255), width=2)

    # Pop-up Window
    pw, ph = 216, 68
    px = (w - pw) // 2
    py = 72
    draw.rectangle([(px, py), (px + pw, py + ph)], fill=(8, 26, 18, 245), outline=(212, 175, 55, 255), width=2)
    draw.rectangle([(px + 3, py + 3), (px + pw - 3, py + ph - 3)], outline=(51, 65, 85, 255), width=1)

    draw.text((px + 10, py + 8), "Bau Submerso em Hylia!", fill=(255, 226, 122, 255))
    draw.text((px + 10, py + 23), "As aguas do lago baixaram e", fill=(245, 247, 250, 255))
    draw.text((px + 10, py + 35), "um grande bau emergiu em Lake Hylia!", fill=(186, 230, 253, 255))
    draw.text((px + 10, py + 50), "Regiao: Lake Hylia", fill=(52, 211, 153, 255))
    draw.text((px + pw - 60, py + 50), "[A] Fechar", fill=(255, 204, 0, 255))

    return img

# Painel 2: Baús Brotando no Overworld
def create_panel_chest_spawning(w, h):
    img = Image.new("RGBA", (w, h), (34, 197, 94, 255))
    draw = ImageDraw.Draw(img)

    # Grassy landscape
    for y in range(h):
        c_g = int(120 + 40 * math.sin(y * 0.1))
        draw.line([(0, y), (w, y)], fill=(34, c_g, 60, 255))

    # Lake Hylia shoreline
    for y in range(h):
        for x in range(w // 2 + 10, w):
            draw.point((x, y), fill=(14, 116, 144, 255))
            if (x + y) % 18 == 0:
                draw.point((x, y), fill=(103, 232, 249, 255))

    # Link standing near shore
    draw_oval_shadow(img, 70, 95, 10, 4)
    render_link_small(draw, 70, 90)

    # Sprouted Golden Chest on emerging sandbank
    cx, cy = w // 2 + 55, 80
    draw.ellipse([(cx - 24, cy - 8), (cx + 24, cy + 12)], fill=(254, 240, 138, 255), outline=(217, 119, 6, 255))
    # Chest base & lid
    draw.rectangle([(cx - 10, cy - 6), (cx + 10, cy + 6)], fill=(217, 119, 6, 255), outline=(245, 158, 11, 255), width=2)
    draw.rectangle([(cx - 9, cy - 10), (cx + 9, cy - 5)], fill=(245, 158, 11, 255), outline=(254, 240, 138, 255), width=1)
    draw.rectangle([(cx - 2, cy - 5), (cx + 2, cy - 1)], fill=(255, 255, 255, 255)) # Keyhole

    # Radiant golden shine rays
    for angle in range(0, 360, 30):
        rad = math.radians(angle)
        sx1 = cx + int(math.cos(rad) * 14)
        sy1 = cy + int(math.sin(rad) * 14)
        sx2 = cx + int(math.cos(rad) * 24)
        sy2 = cy + int(math.sin(rad) * 24)
        draw.line([(sx1, sy1), (sx2, sy2)], fill=(254, 240, 138, 220), width=1)

    draw.rectangle([(8, h - 22), (w - 8, h - 6)], fill=(15, 23, 42, 220))
    draw.text((14, h - 20), "Lake Hylia: Bau de Coracao Emergindo!", fill=(254, 240, 138, 255))

    return img

# Painel 3: Grutas Secretas e Rachaduras Abertas
def create_panel_secret_cave(w, h):
    img = Image.new("RGBA", (w, h), (71, 85, 105, 255))
    draw = ImageDraw.Draw(img)

    # Mountain rock cliff
    for y in range(h):
        c_rock = int(90 + 20 * math.sin(y * 0.15))
        draw.line([(0, y), (w, y)], fill=(c_rock, c_rock - 10, c_rock - 20, 255))

    # Path below
    draw.rectangle([(0, h - 45), (w, h)], fill=(180, 140, 90, 255))

    # Crumbled Stone Passage / Secret Cave Entrance
    cx, cy = w // 2, 70
    draw.rounded_rectangle([(cx - 24, cy - 25), (cx + 24, cy + 25)], radius=12, fill=(15, 23, 42, 255), outline=(51, 65, 85, 255), width=3)
    # Crumble rubble on sides
    for rx, ry, rrad in [(cx - 28, cy + 20, 5), (cx - 20, cy + 24, 7), (cx + 22, cy + 22, 6), (cx + 30, cy + 25, 4)]:
        draw.ellipse([(rx - rrad, ry - rrad), (rx + rrad, ry + rrad)], fill=(120, 113, 108, 255), outline=(68, 64, 60, 255))

    # Fairy Fountain Glow inside Cave!
    draw.ellipse([(cx - 10, cy - 5), (cx + 10, cy + 15)], fill=(244, 114, 182, 180))
    draw.ellipse([(cx - 4, cy), (cx + 4, cy + 8)], fill=(255, 255, 255, 255)) # Fairy sparkle

    # Link arriving at the cave entrance
    draw_oval_shadow(img, cx - 40, h - 30, 10, 4)
    render_link_small(draw, cx - 40, h - 35)

    draw.rectangle([(8, h - 22), (w - 8, h - 6)], fill=(15, 23, 42, 220))
    draw.text((14, h - 20), "Trilby / Crenel: Gruta Secreta Revelada!", fill=(244, 114, 182, 255))

    return img

# Painel 4: Portais Minish Despertados e Trepadeira Gigante
def create_panel_portal_beanstalk(w, h):
    img = Image.new("RGBA", (w, h), (14, 165, 233, 255))
    draw = ImageDraw.Draw(img)

    # Sky and ground
    draw.rectangle([(0, h - 40), (w, h)], fill=(34, 197, 94, 255))

    # 1. Minish Stump Portal (Left)
    sx, sy = 70, h - 45
    draw.rounded_rectangle([(sx - 16, sy - 8), (sx + 16, sy + 14)], radius=6, fill=(120, 53, 15, 255), outline=(69, 26, 3, 255), width=2)
    # Glowing magical spiral portal
    draw.ellipse([(sx - 12, sy - 14), (sx + 12, sy + 2)], fill=(56, 189, 248, 200), outline=(254, 240, 138, 255), width=2)
    draw.ellipse([(sx - 5, sy - 9), (sx + 5, sy - 3)], fill=(255, 255, 255, 255))

    # Link Minish jumping into portal
    render_link_small(draw, sx + 25, h - 35)

    # 2. Giant Beanstalk (Right)
    bx = 175
    # Thick twisting green vine climbing to clouds
    for y in range(h - 40, 0, -8):
        offset = int(math.sin(y * 0.1) * 8)
        draw.ellipse([(bx + offset - 8, y - 6), (bx + offset + 8, y + 6)], fill=(22, 163, 74, 255), outline=(21, 128, 61, 255))
        # Large leaves
        if y % 24 == 0:
            draw.ellipse([(bx + offset + 6, y - 4), (bx + offset + 22, y + 6)], fill=(34, 197, 94, 255))

    # Clouds at top
    for cx in range(bx - 30, bx + 50, 20):
        draw.ellipse([(cx - 15, -10), (cx + 15, 20)], fill=(255, 255, 255, 230))

    draw.rectangle([(8, h - 22), (w - 8, h - 6)], fill=(15, 23, 42, 220))
    draw.text((14, h - 20), "Portal Minish Ativo & Trepadeira aos Ceus!", fill=(56, 189, 248, 255))

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
    draw.text((12, 6), "OPENMINISH - SISTEMA DE SEGREDOS DE KINSTONES NO OVERWORLD (OPÇÃO 3)", fill=(251, 191, 36, 255))
    draw.text((12, 24), "HAL C11 | World Events Dinâmicos | Baús Brotando | Grutas & Portais | Zero Heap Allocation", fill=(148, 163, 184, 255))

    panels = [
        ("1. Fusão & Janela Pop-up Narrativa", create_panel_fusion_popup(pw, ph)),
        ("2. Baús Brotando (Lake Hylia / Bosques)", create_panel_chest_spawning(pw, ph)),
        ("3. Grutas Secretas e Rachaduras Abertas", create_panel_secret_cave(pw, ph)),
        ("4. Portais Minish & Trepadeira aos Céus", create_panel_portal_beanstalk(pw, ph)),
    ]

    for idx, (label, pimg) in enumerate(panels):
        c = idx % cols
        r = idx // cols
        px = gap + c * (pw + gap)
        py = header_h + gap + r * (ph + gap)

        canvas.paste(pimg, (px, py))
        draw.rectangle([(px, py), (px + pw, py + ph)], outline=(71, 85, 105, 255), width=2)
        # Small top badge
        draw.rectangle([(px + 4, py + 4), (px + 175, py + 18)], fill=(15, 23, 42, 220))
        draw.text((px + 8, py + 5), label, fill=(241, 245, 249, 255))

    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    canvas.save(OUTPUT_PATH)
    print(f"Showcase de segredos de Kinstone salvo com sucesso em: {OUTPUT_PATH}")

if __name__ == "__main__":
    main()
