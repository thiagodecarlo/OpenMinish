#!/usr/bin/env python3
"""
tools/generate_startup_and_intro_assets.py - Master Opening & Intro Asset Generator
OpenMinish - Native C11 Port of The Legend of Zelda: The Minish Cap

Generates authentic, high-fidelity 32-bit BMP assets for the complete game opening:
1. assets/ui/startup/capcom_logo.bmp     - Authentic Capcom logo & copyright (240x160)
2. assets/ui/startup/nintendo_logo.bmp   - Authentic Nintendo racetrack pill logo (240x160)
3. assets/ui/startup/title_screen_bg.bmp - Monumental Zelda logo, Triforce, Four Sword pedestal (240x160)
4. assets/ui/startup/file_select_bg.bmp  - Royal tapestry, gold frames & 3 save cards (240x160)
5. assets/regions/map_link_house.bmp     - Link's bedroom & Smith's forge workshop (240x160)
6. assets/regions/map_throne_room.bmp    - King Daltus's Hyrule Castle Throne Room (240x160)
Also mirrors map assets into assets/regions/usa/.
"""

import os
import struct
import math
from PIL import Image, ImageDraw, ImageFont

def save_bmp_32(filepath, img):
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    w, h = img.size
    img = img.convert('RGBA')
    img_data = img.tobytes()
    bgra = bytearray(w * h * 4)
    for i in range(w * h):
        r = img_data[i*4 + 0]
        g = img_data[i*4 + 1]
        b = img_data[i*4 + 2]
        a = img_data[i*4 + 3]
        bgra[i*4 + 0] = b
        bgra[i*4 + 1] = g
        bgra[i*4 + 2] = r
        bgra[i*4 + 3] = a

    header_size = 54
    img_size = w * h * 4
    file_size = header_size + img_size

    file_hdr = struct.pack('<2sIHHI', b'BM', file_size, 0, 0, header_size)
    info_hdr = struct.pack('<IiiHHIIiiII', 40, w, -h, 1, 32, 0, img_size, 0, 0, 0, 0)

    with open(filepath, 'wb') as f:
        f.write(file_hdr)
        f.write(info_hdr)
        f.write(bgra)
    print(f"  -> Salvo: {filepath} ({w}x{h}, {file_size} bytes)")

def get_font(size):
    try:
        # Standard fonts on Windows
        for font_name in ["arialbd.ttf", "arial.ttf", "calibri.ttf", "segoeui.ttf"]:
            font_path = os.path.join(os.environ.get("WINDIR", "C:\\Windows"), "Fonts", font_name)
            if os.path.exists(font_path):
                return ImageFont.truetype(font_path, size)
    except Exception:
        pass
    return ImageFont.load_default()

# ============================================================================
# 1. CAPCOM LOGO (240x160)
# ============================================================================
def generate_capcom_logo():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (255, 255, 255, 255))
    draw = ImageDraw.Draw(img)

    # Letras CAPCOM em tipografia geométrica icônica
    # Cores: Borda Azul Marinho (0, 45, 114), Miolo Amarelo Ouro (255, 204, 0)
    navy = (0, 45, 114, 255)
    yellow = (255, 204, 0, 255)
    yellow_hi = (255, 230, 80, 255)
    yellow_sh = (218, 165, 0, 255)

    # Coordenadas relativas de cada letra (C, A, P, C, O, M)
    # Largura total ~144px, altura ~26px, centralizado em (48, 54)
    start_x = 44
    base_y = 54
    letter_h = 26

    # Função para desenhar bloco com contorno navy e gradiente amarelo
    def draw_letter_c(bx, by):
        # Outer border
        draw.rectangle([bx, by, bx + 19, by + letter_h], fill=navy)
        draw.rectangle([bx + 7, by + 6, bx + 19, by + letter_h - 6], fill=(255, 255, 255, 255))
        # Inner fill
        draw.rectangle([bx + 3, by + 3, bx + 16, by + letter_h - 3], fill=yellow)
        draw.rectangle([bx + 7, by + 7, bx + 19, by + letter_h - 7], fill=(255, 255, 255, 255))
        # Top/bottom serif cut
        draw.rectangle([bx + 13, by + 6, bx + 16, by + 10], fill=yellow)
        draw.rectangle([bx + 13, by + letter_h - 10, bx + 16, by + letter_h - 6], fill=yellow)

    def draw_letter_a(bx, by):
        draw.rectangle([bx, by, bx + 20, by + letter_h], fill=navy)
        draw.polygon([(bx + 5, by), (bx + 15, by), (bx + 20, by + letter_h), (bx, by + letter_h)], fill=navy)
        draw.polygon([(bx + 7, by + 3), (bx + 13, by + 3), (bx + 17, by + letter_h - 3), (bx + 3, by + letter_h - 3)], fill=yellow)
        # Inner hole
        draw.polygon([(bx + 8, by + 7), (bx + 12, by + 7), (bx + 10, by + 13)], fill=(255, 255, 255, 255))
        # Crossbar
        draw.rectangle([bx + 6, by + 16, bx + 14, by + 19], fill=yellow)
        draw.rectangle([bx + 8, by + 13, bx + 12, by + letter_h], fill=(255, 255, 255, 255))
        draw.rectangle([bx + 6, by + 15, bx + 14, by + 18], fill=yellow)

    def draw_letter_p(bx, by):
        draw.rectangle([bx, by, bx + 19, by + letter_h], fill=navy)
        draw.rectangle([bx + 3, by + 3, bx + 16, by + letter_h - 3], fill=yellow)
        # Hollow bottom right
        draw.rectangle([bx + 7, by + 15, bx + 19, by + letter_h], fill=(255, 255, 255, 255))
        draw.rectangle([bx + 3, by + 15, bx + 7, by + letter_h - 3], fill=yellow)
        # Loop inner hole
        draw.rectangle([bx + 7, by + 6, bx + 13, by + 12], fill=navy)
        draw.rectangle([bx + 8, by + 7, bx + 12, by + 11], fill=(255, 255, 255, 255))

    def draw_letter_o(bx, by):
        draw.rectangle([bx, by, bx + 20, by + letter_h], fill=navy)
        draw.rectangle([bx + 3, by + 3, bx + 17, by + letter_h - 3], fill=yellow)
        # Inner hole
        draw.rectangle([bx + 7, by + 6, bx + 13, by + letter_h - 6], fill=navy)
        draw.rectangle([bx + 8, by + 7, bx + 12, by + letter_h - 7], fill=(255, 255, 255, 255))

    def draw_letter_m(bx, by):
        draw.rectangle([bx, by, bx + 24, by + letter_h], fill=navy)
        draw.rectangle([bx + 3, by + 3, bx + 21, by + letter_h - 3], fill=yellow)
        # Central V cut
        draw.polygon([(bx + 8, by + 6), (bx + 16, by + 6), (bx + 12, by + 18)], fill=navy)
        draw.polygon([(bx + 9, by + 7), (bx + 15, by + 7), (bx + 12, by + 16)], fill=(255, 255, 255, 255))
        draw.rectangle([bx + 10, by + 17, bx + 14, by + letter_h], fill=(255, 255, 255, 255))

    # Render letras com espaçamento calibrado
    draw_letter_c(start_x, base_y)
    draw_letter_a(start_x + 24, base_y)
    draw_letter_p(start_x + 49, base_y)
    draw_letter_c(start_x + 73, base_y)
    draw_letter_o(start_x + 97, base_y)
    draw_letter_m(start_x + 122, base_y)

    # Registered trademark symbol (R)
    rx = start_x + 148
    ry = base_y + 2
    draw.ellipse([rx, ry, rx + 6, ry + 6], outline=navy, width=1)
    draw.point((rx + 2, ry + 2), fill=navy)
    draw.point((rx + 3, ry + 2), fill=navy)
    draw.point((rx + 2, ry + 3), fill=navy)
    draw.point((rx + 3, ry + 4), fill=navy)

    # Shading highlights on yellow letters (bevel look)
    for lx in [start_x, start_x + 24, start_x + 49, start_x + 73, start_x + 97, start_x + 122]:
        draw.line([lx + 3, base_y + 3, lx + 16, base_y + 3], fill=yellow_hi)

    # Legendas de Copyright e Direitos Reservados em tipografia clássica GBA
    font_sub = get_font(10)
    copy1 = "(C) CAPCOM CO., LTD. 2004"
    copy2 = "ALL RIGHTS RESERVED."
    
    # Calculate bounding boxes for centering
    bbox1 = draw.textbbox((0, 0), copy1, font=font_sub)
    w1 = bbox1[2] - bbox1[0]
    bbox2 = draw.textbbox((0, 0), copy2, font=font_sub)
    w2 = bbox2[2] - bbox2[0]

    draw.text(((w - w1) // 2, 98), copy1, font=font_sub, fill=(71, 85, 105, 255))
    draw.text(((w - w2) // 2, 112), copy2, font=font_sub, fill=(100, 116, 139, 255))

    return img

# ============================================================================
# 2. NINTENDO LOGO (240x160)
# ============================================================================
def generate_nintendo_logo():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (255, 255, 255, 255))
    draw = ImageDraw.Draw(img)

    red = (230, 0, 18, 255)
    white = (255, 255, 255, 255)

    # Racetrack badge oval pill
    badge_w = 116
    badge_h = 32
    bx = (w - badge_w) // 2
    by = (h - badge_h) // 2 - 8

    # Rounded pill badge
    r = badge_h // 2
    # Left semicircle
    draw.pieslice([bx, by, bx + badge_h, by + badge_h], 90, 270, fill=red)
    # Right semicircle
    draw.pieslice([bx + badge_w - badge_h, by, bx + badge_w, by + badge_h], 270, 90, fill=red)
    # Center rectangle
    draw.rectangle([bx + r, by, bx + badge_w - r, by + badge_h], fill=red)

    # Nintendo font text inside pill
    font_nin = get_font(17)
    text = "Nintendo"
    bbox = draw.textbbox((0, 0), text, font=font_nin)
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    tx = (w - tw) // 2
    ty = by + (badge_h - th) // 2 - 2
    draw.text((tx, ty), text, font=font_nin, fill=white)

    # Registered trademark symbol (R)
    rrx = bx + badge_w - 9
    rry = by + 6
    draw.ellipse([rrx, rry, rrx + 5, rry + 5], outline=white, width=1)

    # Subtitle Copyright
    font_sub = get_font(10)
    copy_str = "(C) 2004 Nintendo"
    cbox = draw.textbbox((0, 0), copy_str, font=font_sub)
    cw = cbox[2] - cbox[0]
    draw.text(((w - cw) // 2, 114), copy_str, font=font_sub, fill=(100, 116, 139, 255))

    return img

# ============================================================================
# 3. TITLE SCREEN (240x160)
# ============================================================================
def generate_title_screen_bg():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (10, 16, 36, 255))
    draw = ImageDraw.Draw(img)

    # 3.1 Gradiente do Céu Noturno Místico
    for y in range(h):
        t = y / h
        r = int(8 + t * 24)
        g = int(12 + t * 32)
        b = int(28 + t * 50)
        draw.line([0, y, w, y], fill=(r, g, b, 255))

    # Constelações e poeira estelar
    import random
    rng = random.Random(42)
    for _ in range(35):
        sx = rng.randint(4, w - 5)
        sy = rng.randint(4, 95)
        brightness = rng.randint(180, 255)
        draw.point((sx, sy), fill=(254, 240, 138, brightness))
        if rng.random() > 0.6:
            draw.point((sx + 1, sy), fill=(253, 224, 71, brightness // 2))

    # 3.2 Pedestal de Pedra Sagrado da Four Sword (Y: 104 a 148)
    px = w // 2
    py = 118

    # Auréola de luz divina dourada ao redor do pedestal
    for radius in range(38, 10, -3):
        alpha = int(45 * (1.0 - radius / 38.0))
        draw.ellipse([px - radius * 2, py - radius, px + radius * 2, py + radius], fill=(253, 224, 71, alpha))

    # Degraus do pedestal em blocos de pedra cinza e runas
    # Degrau inferior
    draw.rectangle([px - 44, py + 14, px + 44, py + 24], fill=(40, 52, 70, 255), outline=(25, 34, 48, 255))
    draw.line([px - 43, py + 15, px + 43, py + 15], fill=(70, 85, 110, 255))
    # Degrau intermediário
    draw.rectangle([px - 32, py + 6, px + 32, py + 14], fill=(55, 70, 92, 255), outline=(32, 42, 58, 255))
    draw.line([px - 31, py + 7, px + 31, py + 7], fill=(85, 105, 135, 255))
    # Altar superior com musgo e runas
    draw.rectangle([px - 20, py - 4, px + 20, py + 6], fill=(71, 85, 105, 255), outline=(40, 52, 70, 255))
    draw.line([px - 19, py - 3, px + 19, py - 3], fill=(110, 130, 160, 255))
    # Musgo nas juntas da pedra
    for mx, my in [(px - 26, py + 12), (px + 18, py + 13), (px - 14, py + 4), (px + 12, py + 4)]:
        draw.rectangle([mx, my, mx + 5, my + 2], fill=(34, 120, 60, 255))

    # A Four Sword Cravada na Rocha
    # Brilho místico vertical
    draw.line([px, py - 36, px, py - 4], fill=(224, 242, 254, 255), width=3)
    draw.line([px, py - 34, px, py - 4], fill=(56, 189, 248, 255), width=1)
    # Lâmina de Prata polida
    draw.polygon([(px - 2, py - 32), (px + 2, py - 32), (px + 1, py - 4), (px - 1, py - 4)], fill=(248, 250, 252, 255))
    # Guarda de Ouro Alada
    draw.rectangle([px - 10, py - 35, px + 10, py - 32], fill=(245, 158, 11, 255), outline=(180, 83, 9, 255))
    draw.line([px - 9, py - 34, px + 9, py - 34], fill=(253, 224, 71, 255))
    # Joia de Safira Mística Central
    draw.rectangle([px - 2, py - 34, px + 2, py - 32], fill=(6, 182, 212, 255))
    draw.point((px, py - 33), fill=(255, 255, 255, 255))
    # Empunhadura de Esmeralda Trançada
    draw.rectangle([px - 2, py - 43, px + 2, py - 35], fill=(21, 128, 61, 255))
    draw.line([px - 1, py - 43, px - 1, py - 35], fill=(34, 197, 94, 255))
    # Pomo de Ouro
    draw.ellipse([px - 3, py - 46, px + 3, py - 42], fill=(251, 191, 36, 255), outline=(180, 83, 9, 255))

    # 3.3 Monumental Logotipo de Zelda
    # 1. "THE LEGEND OF"
    font_small = get_font(10)
    leg_text = "THE LEGEND OF"
    lbox = draw.textbbox((0, 0), leg_text, font=font_small)
    lw = lbox[2] - lbox[0]
    lx = (w - lw) // 2
    ly = 13
    # Sombra
    draw.text((lx + 1, ly + 1), leg_text, font=font_small, fill=(30, 20, 10, 255))
    draw.text((lx, ly), leg_text, font=font_small, fill=(251, 191, 36, 255))

    # 2. Triforce Dourada no Topo
    t_center_x = w // 2
    t_top_y = 21
    # Triângulo Superior
    draw.polygon([(t_center_x, t_top_y), (t_center_x - 6, t_top_y + 8), (t_center_x + 6, t_top_y + 8)],
                 fill=(253, 224, 71, 255), outline=(180, 83, 9, 255))
    # Triângulo Inferior Esquerdo
    draw.polygon([(t_center_x - 6, t_top_y + 8), (t_center_x - 12, t_top_y + 16), (t_center_x, t_top_y + 16)],
                 fill=(251, 191, 36, 255), outline=(180, 83, 9, 255))
    # Triângulo Inferior Direito
    draw.polygon([(t_center_x + 6, t_top_y + 8), (t_center_x, t_top_y + 16), (t_center_x + 12, t_top_y + 16)],
                 fill=(251, 191, 36, 255), outline=(180, 83, 9, 255))

    # 3. "Z E L D A" Monumental
    font_zelda = get_font(27)
    z_text = "ZELDA"
    zbox = draw.textbbox((0, 0), z_text, font=font_zelda)
    zw = zbox[2] - zbox[0]
    zx = (w - zw) // 2
    zy = 27

    # Sombra de profundidade preta
    for off in range(3, 0, -1):
        draw.text((zx + off, zy + off), z_text, font=font_zelda, fill=(15, 5, 5, 255))
    # Borda dourada chanfrada
    for ox, oy in [(-1, 0), (1, 0), (0, -1), (0, 1), (-1, -1), (1, 1), (-1, 1), (1, -1)]:
        draw.text((zx + ox, zy + oy), z_text, font=font_zelda, fill=(253, 224, 71, 255))
    # Preenchimento Carmesim Imperial com chanfro de luz
    draw.text((zx, zy), z_text, font=font_zelda, fill=(220, 38, 38, 255))
    draw.text((zx, zy - 1), z_text, font=font_zelda, fill=(239, 68, 68, 255))

    # 4. Faixa de Fita Esmeralda "The Minish Cap"
    mc_w = 132
    mc_h = 16
    mc_x = (w - mc_w) // 2
    mc_y = 57

    # Rabos da fita entalhados nas pontas
    draw.polygon([(mc_x - 6, mc_y + 2), (mc_x, mc_y), (mc_x, mc_y + mc_h), (mc_x - 6, mc_y + mc_h - 2), (mc_x - 2, mc_y + mc_h // 2)], fill=(5, 46, 22, 255))
    draw.polygon([(mc_x + mc_w + 6, mc_y + 2), (mc_x + mc_w, mc_y), (mc_x + mc_w, mc_y + mc_h), (mc_x + mc_w + 6, mc_y + mc_h - 2), (mc_x + mc_w + 2, mc_y + mc_h // 2)], fill=(5, 46, 22, 255))
    # Corpo principal da fita
    draw.rectangle([mc_x, mc_y, mc_x + mc_w, mc_y + mc_h], fill=(21, 128, 61, 255), outline=(245, 158, 11, 255))
    draw.line([mc_x + 1, mc_y + 1, mc_x + mc_w - 1, mc_y + 1], fill=(74, 222, 128, 255))
    draw.line([mc_x + 1, mc_y + mc_h - 1, mc_x + mc_w - 1, mc_y + mc_h - 1], fill=(5, 46, 22, 255))

    # Texto "The Minish Cap" em tipografia elegante
    font_mc = get_font(11)
    mc_str = "The Minish Cap"
    mc_box = draw.textbbox((0, 0), mc_str, font=font_mc)
    mc_tw = mc_box[2] - mc_box[0]
    draw.text(((w - mc_tw) // 2 + 1, mc_y + 2), mc_str, font=font_mc, fill=(5, 46, 22, 255)) # Sombra
    draw.text(((w - mc_tw) // 2, mc_y + 1), mc_str, font=font_mc, fill=(240, 253, 244, 255))

    return img

# ============================================================================
# 4. FILE SELECT BACKGROUND (240x160)
# ============================================================================
def generate_file_select_bg():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (15, 23, 42, 255))
    draw = ImageDraw.Draw(img)

    # 4.1 Fundo de Tapeçaria Real com Textura Damasco em Padrão Diamante
    for y in range(h):
        for x in range(w):
            val = (x ^ y) % 16
            if val == 0:
                draw.point((x, y), fill=(30, 41, 59, 255))
            elif val == 8:
                draw.point((x, y), fill=(24, 33, 47, 255))

    # Moldura externa em latão polido
    draw.rectangle([0, 0, w - 1, h - 1], outline=(100, 116, 139, 255), width=1)
    draw.rectangle([2, 2, w - 3, h - 3], outline=(245, 158, 11, 255), width=1)

    # 4.2 Placa de Título Superior "SELEÇÃO DE ARQUIVO"
    header_w = 160
    header_h = 16
    hx = (w - header_w) // 2
    hy = 6
    draw.rectangle([hx, hy, hx + header_w, hy + header_h], fill=(30, 41, 59, 255), outline=(245, 158, 11, 255))
    draw.line([hx + 1, hy + 1, hx + header_w - 1, hy + 1], fill=(253, 224, 71, 255))
    # Cantos ornamentais
    for cx, cy in [(hx, hy), (hx + header_w - 3, hy), (hx, hy + header_h - 3), (hx + header_w - 3, hy + header_h - 3)]:
        draw.rectangle([cx, cy, cx + 2, cy + 2], fill=(217, 119, 6, 255))

    font_hdr = get_font(10)
    title_txt = "- SELECAO DE ARQUIVO -"
    tbox = draw.textbbox((0, 0), title_txt, font=font_hdr)
    tw = tbox[2] - tbox[0]
    draw.text(((w - tw) // 2, hy + 2), title_txt, font=font_hdr, fill=(254, 240, 138, 255))

    # 4.3 Os 3 Cards de Save (Slot 1, 2 e 3)
    card_w = 216
    card_h = 32
    cx = (w - card_w) // 2

    for i in range(3):
        cy = 26 + i * 36
        # Sombra projetada
        draw.rectangle([cx + 2, cy + 2, cx + card_w + 1, cy + card_h + 1], fill=(5, 10, 20, 200))
        # Moldura chanfrada de pergaminho/latão
        draw.rectangle([cx, cy, cx + card_w, cy + card_h], fill=(30, 41, 59, 255), outline=(71, 85, 105, 255))
        draw.rectangle([cx + 1, cy + 1, cx + card_w - 1, cy + card_h - 1], outline=(15, 23, 42, 255))
        # Miolo de pergaminho escuro nobre
        draw.rectangle([cx + 2, cy + 2, cx + card_w - 2, cy + card_h - 2], fill=(22, 30, 46, 255))

        # Círculo com o número do Slot à esquerda
        badge_r = 10
        bx = cx + 18
        by = cy + 16
        draw.ellipse([bx - badge_r, by - badge_r, bx + badge_r, by + badge_r], fill=(15, 23, 42, 255), outline=(245, 158, 11, 255))
        font_num = get_font(11)
        num_str = str(i + 1)
        nbox = draw.textbbox((0, 0), num_str, font=font_num)
        nw = nbox[2] - nbox[0]
        draw.text((bx - nw // 2, by - 7), num_str, font=font_num, fill=(253, 224, 71, 255))

        # Divisor vertical dourado
        draw.line([cx + 36, cy + 4, cx + 36, cy + card_h - 4], fill=(71, 85, 105, 255))
        draw.line([cx + 37, cy + 4, cx + 37, cy + card_h - 4], fill=(245, 158, 11, 160))

    # 4.4 Barra de Rodapé com Controles e Ações
    bar_y = 138
    draw.rectangle([4, bar_y, w - 5, bar_y + 18], fill=(15, 23, 42, 255), outline=(71, 85, 105, 255))
    font_bar = get_font(9)
    bar_str = "[A] Confirmar    [L] Copiar    [R] Apagar    [START] Jogar"
    bbox = draw.textbbox((0, 0), bar_str, font=font_bar)
    bw = bbox[2] - bbox[0]
    draw.text(((w - bw) // 2, bar_y + 3), bar_str, font=font_bar, fill=(148, 163, 184, 255))

    return img

# ============================================================================
# 5. LINK'S HOUSE & SMITH'S FORGE (240x160)
# ============================================================================
def generate_link_house_map():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (92, 45, 15, 255))
    draw = ImageDraw.Draw(img)

    # 5.1 Paredes de Madeira e Vigas Estruturais (Y: 0 a 38)
    draw.rectangle([0, 0, w - 1, 38], fill=(75, 38, 14, 255))
    # Vigas verticais de sustentação
    for vx in range(0, w, 30):
        draw.rectangle([vx, 0, vx + 6, 38], fill=(55, 26, 9, 255))
        draw.line([vx + 1, 0, vx + 1, 38], fill=(95, 48, 18, 255))
    # Rodapé de cantaria de pedra
    draw.rectangle([0, 34, w - 1, 38], fill=(51, 65, 85, 255), outline=(30, 41, 59, 255))

    # 5.2 Piso de Tábuas de Madeira Nobre (Y: 38 a 160)
    for y in range(38, h, 10):
        # Tábua de madeira
        draw.rectangle([0, y, w - 1, y + 9], fill=(146, 64, 14, 255))
        draw.line([0, y, w - 1, y], fill=(113, 49, 11, 255))
        # Juntas verticais das tábuas
        shift = 32 if ((y // 10) % 2 == 0) else 0
        for x in range(shift, w, 48):
            draw.line([x, y, x, y + 9], fill=(85, 36, 8, 255))
            draw.line([x + 1, y + 1, x + 1, y + 9], fill=(180, 83, 9, 255))

    # 5.3 Quarto de Link (Lado Esquerdo: X: 10 a 70)
    # Cama de madeira de Link
    bed_x, bed_y = 14, 24
    bed_w, bed_h = 36, 52
    # Estrutura de carvalho escuro
    draw.rectangle([bed_x, bed_y, bed_x + bed_w, bed_y + bed_h], fill=(61, 26, 4, 255), outline=(35, 15, 2, 255))
    # Cabeceira entalhada
    draw.rectangle([bed_x + 2, bed_y + 2, bed_x + bed_w - 2, bed_y + 8], fill=(95, 48, 18, 255))
    # Travesseiro branco macio
    draw.rectangle([bed_x + 4, bed_y + 9, bed_x + bed_w - 4, bed_y + 19], fill=(241, 245, 249, 255), outline=(203, 213, 225, 255))
    # Lençol branco dobrado
    draw.rectangle([bed_x + 4, bed_y + 20, bed_x + bed_w - 4, bed_y + 24], fill=(226, 232, 240, 255))
    # Cobertor azul real acolchoado com textura
    draw.rectangle([bed_x + 4, bed_y + 25, bed_x + bed_w - 4, bed_y + bed_h - 4], fill=(29, 78, 216, 255), outline=(30, 58, 138, 255))
    for by in range(bed_y + 28, bed_y + bed_h - 4, 6):
        draw.line([bed_x + 5, by, bed_x + bed_w - 5, by], fill=(59, 130, 246, 255))

    # Criado-mudo de cabeceira com vela e caneca
    draw.rectangle([bed_x + bed_w + 3, bed_y + 10, bed_x + bed_w + 14, bed_y + 24], fill=(85, 36, 8, 255), outline=(45, 18, 4, 255))
    # Castiçal de latão e vela acesa
    draw.rectangle([bed_x + bed_w + 6, bed_y + 4, bed_x + bed_w + 10, bed_y + 10], fill=(254, 240, 138, 255))
    draw.point((bed_x + bed_w + 8, bed_y + 2), fill=(249, 115, 22, 255))

    # 5.4 Janela com Raios Solares Matinais (Norte: X: 148 a 184)
    win_x, win_y = 152, 6
    win_w, win_h = 28, 24
    # Moldura de madeira da janela
    draw.rectangle([win_x, win_y, win_x + win_w, win_y + win_h], fill=(56, 189, 248, 255), outline=(30, 41, 59, 255), width=2)
    # Vidro com reflexo celeste
    draw.line([win_x + win_w // 2, win_y, win_x + win_w // 2, win_y + win_h], fill=(30, 41, 59, 255), width=2)
    draw.line([win_x, win_y + win_h // 2, win_x + win_w, win_y + win_h // 2], fill=(30, 41, 59, 255), width=2)
    # Cortinas verdes nas bordas
    draw.rectangle([win_x - 3, win_y, win_x + 1, win_y + win_h], fill=(22, 101, 52, 255))
    draw.rectangle([win_x + win_w - 1, win_y, win_x + win_w + 3, win_y + win_h], fill=(22, 101, 52, 255))

    # Feixes de luz solar dourada projetados diagonalmente no quarto
    for i in range(22):
        lx = win_x - 20 - i * 3
        ly = 38 + i * 4
        lw = 45 + i * 2
        draw.rectangle([lx, ly, lx + lw, ly + 4], fill=(254, 240, 138, 24))

    # 5.5 Tapete Nobre Oval no Centro da Sala
    rx, ry = 72, 54
    rw, rh = 64, 42
    draw.ellipse([rx - 2, ry - 2, rx + rw + 2, ry + rh + 2], fill=(245, 158, 11, 160))
    draw.ellipse([rx, ry, rx + rw, ry + rh], fill=(22, 101, 52, 255), outline=(180, 83, 9, 255))
    draw.ellipse([rx + 6, ry + 4, rx + rw - 6, ry + rh - 4], fill=(21, 128, 61, 255))
    # Símbolo Triforce sutil tecido no centro do tapete
    tx = rx + rw // 2
    ty = ry + rh // 2 - 4
    draw.polygon([(tx, ty), (tx - 5, ty + 8), (tx + 5, ty + 8)], fill=(251, 191, 36, 180))

    # 5.6 A Forja do Mestre Smith (Canto Direito: X: 186 a 236)
    fx, fy = 192, 14
    fw, fh = 42, 52
    # Estrutura de tijolos de cantaria refratária
    draw.rectangle([fx, fy, fx + fw, fy + fh], fill=(71, 85, 105, 255), outline=(30, 41, 59, 255))
    # Tijolos individuais com juntas
    for ty in range(fy + 4, fy + fh - 4, 8):
        draw.line([fx, ty, fx + fw, ty], fill=(51, 65, 85, 255))
    # Boca da fornalha em arco
    draw.rectangle([fx + 6, fy + 24, fx + fw - 6, fy + fh], fill=(15, 23, 42, 255), outline=(30, 41, 59, 255))
    # Brasas incandescentes e labaredas de fogo
    draw.rectangle([fx + 10, fy + 34, fx + fw - 10, fy + fh - 2], fill=(234, 88, 12, 255))
    draw.rectangle([fx + 14, fy + 38, fx + fw - 14, fy + fh - 4], fill=(251, 191, 36, 255))
    draw.point((fx + 18, fy + 30), fill=(253, 224, 71, 255))
    draw.point((fx + 24, fy + 28), fill=(249, 115, 22, 255))

    # Bigorna de Ferro Fundido
    ax, ay = 176, 68
    # Cepo de tora de carvalho
    draw.rectangle([ax + 2, ay + 6, ax + 14, ay + 18], fill=(69, 26, 3, 255), outline=(35, 15, 2, 255))
    # Corpo de aço da bigorna
    draw.rectangle([ax, ay, ax + 16, ay + 7], fill=(100, 116, 139, 255), outline=(30, 41, 59, 255))
    draw.polygon([(ax, ay + 2), (ax - 5, ay + 3), (ax, ay + 5)], fill=(148, 163, 184, 255)) # Chifre
    # Martelo de ferreiro apoiado
    draw.line([ax + 12, ay - 3, ax + 18, ay + 4], fill=(180, 83, 9, 255), width=2)
    draw.rectangle([ax + 10, ay - 5, ax + 14, ay - 1], fill=(51, 65, 85, 255))

    # Barril de resfriamento d'água ao lado da forja
    bx, by = 222, 68
    draw.rectangle([bx, by, bx + 14, by + 18], fill=(95, 48, 18, 255), outline=(51, 65, 85, 255))
    draw.line([bx, by + 4, bx + 14, by + 4], fill=(148, 163, 184, 255)) # Aro metálico
    draw.line([bx, by + 14, bx + 14, by + 14], fill=(148, 163, 184, 255))
    draw.ellipse([bx + 1, by + 1, bx + 13, by + 5], fill=(56, 189, 248, 255)) # Água limpa

    return img

# ============================================================================
# 6. HYRULE CASTLE THRONE ROOM (240x160)
# ============================================================================
def generate_throne_room_map():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (30, 27, 24, 255))
    draw = ImageDraw.Draw(img)

    # 6.1 Piso de Mármore Real Polido (Y: 0 a 160)
    for y in range(0, h, 16):
        for x in range(0, w, 16):
            # Alternância de lajotas de mármore imperial
            tile_col = (45, 40, 36, 255) if ((x // 16 + y // 16) % 2 == 0) else (38, 34, 30, 255)
            draw.rectangle([x, y, x + 15, y + 15], fill=tile_col, outline=(24, 21, 18, 255))
            draw.point((x + 1, y + 1), fill=(60, 54, 48, 255))

    # 6.2 Paredes Monumentais do Castelo com Arcos Góticos (Norte: Y: 0 a 38)
    draw.rectangle([0, 0, w - 1, 34], fill=(55, 50, 46, 255), outline=(24, 21, 18, 255))
    for ax in range(12, w - 24, 44):
        # Janelas em arco ogival
        draw.arc([ax, 6, ax + 20, 26], 180, 360, fill=(245, 158, 11, 255), width=2)
        draw.rectangle([ax, 16, ax + 20, 32], fill=(20, 25, 38, 255), outline=(245, 158, 11, 255))
        # Vidral com luz solar mística
        draw.line([ax + 10, 8, ax + 10, 32], fill=(56, 189, 248, 255))
        draw.line([ax + 2, 20, ax + 18, 20], fill=(251, 191, 36, 255))

    # 6.3 Grande Tapete Imperial Carmesim e Ouro (X: 74 a 166, Y: 0 a 160)
    carpet_x1 = 76
    carpet_x2 = 164
    carpet_w = carpet_x2 - carpet_x1

    # Borda em Ouro Polido
    draw.rectangle([carpet_x1 - 4, 0, carpet_x1 - 1, h - 1], fill=(245, 158, 11, 255))
    draw.rectangle([carpet_x2 + 1, 0, carpet_x2 + 4, h - 1], fill=(245, 158, 11, 255))
    # Tapete Carmesim Real Aveludado
    draw.rectangle([carpet_x1, 0, carpet_x2, h - 1], fill=(127, 29, 29, 255))
    # Padrão decorativo no tapete
    for y in range(0, h, 20):
        draw.rectangle([carpet_x1 + 6, y, carpet_x2 - 6, y + 2], fill=(153, 27, 27, 255))
        # Brasão dourado repetido no centro
        tx = w // 2
        draw.polygon([(tx, y + 6), (tx - 4, y + 14), (tx + 4, y + 14)], fill=(217, 119, 6, 200))

    # 6.4 Pilares e Colunas Flanqueadoras de Mármore
    for px in [36, 188]:
        # Base da coluna
        draw.rectangle([px - 10, 10, px + 10, 130], fill=(148, 163, 184, 255), outline=(51, 65, 85, 255))
        draw.rectangle([px - 8, 10, px - 2, 130], fill=(203, 213, 225, 255)) # Luz
        draw.rectangle([px + 2, 10, px + 8, 130], fill=(100, 116, 139, 255)) # Sombra
        # Capitel Dourado Coríntio
        draw.rectangle([px - 14, 10, px + 14, 20], fill=(245, 158, 11, 255), outline=(180, 83, 9, 255))
        draw.line([px - 13, 12, px + 13, 12], fill=(253, 224, 71, 255))
        # Plinto inferior
        draw.rectangle([px - 12, 126, px + 12, 136], fill=(71, 85, 105, 255), outline=(30, 41, 59, 255))

    # 6.5 O Estrado e o Trono Dourado Real do Rei Daltus (Norte: Y: 12 a 58)
    dais_w = 70
    dais_x = (w - dais_w) // 2
    dais_y = 16

    # 3 Degraus de subida do Trono
    draw.rectangle([dais_x - 10, dais_y + 24, dais_x + dais_w + 10, dais_y + 36], fill=(51, 65, 85, 255), outline=(30, 41, 59, 255))
    draw.rectangle([dais_x - 4, dais_y + 16, dais_x + dais_w + 4, dais_y + 24], fill=(71, 85, 105, 255), outline=(40, 52, 70, 255))
    draw.rectangle([dais_x, dais_y + 8, dais_x + dais_w, dais_y + 16], fill=(100, 116, 139, 255), outline=(51, 65, 85, 255))
    # Tapete sobre os degraus
    draw.rectangle([dais_x + 8, dais_y + 8, dais_x + dais_w - 8, dais_y + 36], fill=(127, 29, 29, 255), outline=(245, 158, 11, 255))

    # Trono de Ouro Maciço
    tx = w // 2
    ty = 12
    # Encosto Alto com Curvatura Imperial
    draw.rectangle([tx - 16, ty, tx + 16, ty + 28], fill=(245, 158, 11, 255), outline=(180, 83, 9, 255))
    # Crista Real Alada e Triforce no topo
    draw.polygon([(tx, ty - 8), (tx - 10, ty), (tx + 10, ty)], fill=(253, 224, 71, 255), outline=(180, 83, 9, 255))
    draw.polygon([(tx, ty - 6), (tx - 3, ty - 1), (tx + 3, ty - 1)], fill=(254, 240, 138, 255))
    # Almofada Aveludada Vermelha do Encosto
    draw.rectangle([tx - 11, ty + 4, tx + 11, ty + 20], fill=(185, 28, 28, 255), outline=(127, 29, 29, 255))
    # Assento do Trono
    draw.rectangle([tx - 14, ty + 20, tx + 14, ty + 28], fill=(217, 119, 6, 255), outline=(146, 64, 14, 255))
    draw.rectangle([tx - 12, ty + 21, tx + 12, ty + 25], fill=(220, 38, 38, 255))
    # Braços dourados do trono
    draw.rectangle([tx - 18, ty + 16, tx - 14, ty + 26], fill=(251, 191, 36, 255), outline=(180, 83, 9, 255))
    draw.rectangle([tx + 14, ty + 16, tx + 18, ty + 26], fill=(251, 191, 36, 255), outline=(180, 83, 9, 255))

    # Estandartes Reais de Hyrule Pendurados na Parede Norte
    for bx in [dais_x - 22, dais_x + dais_w + 10]:
        draw.rectangle([bx, 6, bx + 12, 42], fill=(30, 58, 138, 255), outline=(245, 158, 11, 255))
        # Ponta bifurcada
        draw.polygon([(bx, 42), (bx + 6, 48), (bx + 12, 42)], fill=(30, 58, 138, 255))
        # Símbolo da Triforce no estandarte
        stx = bx + 6
        draw.polygon([(stx, 14), (stx - 3, 20), (stx + 3, 20)], fill=(253, 224, 71, 255))

    return img

def main():
    print("[ASSETS] Gerando pacote completo de assets de abertura e cutscenes...")

    # 1. Capcom Logo
    img_capcom = generate_capcom_logo()
    save_bmp_32("assets/ui/startup/capcom_logo.bmp", img_capcom)

    # 2. Nintendo Logo
    img_nintendo = generate_nintendo_logo()
    save_bmp_32("assets/ui/startup/nintendo_logo.bmp", img_nintendo)

    # 3. Title Screen Background
    img_title = generate_title_screen_bg()
    save_bmp_32("assets/ui/startup/title_screen_bg.bmp", img_title)

    # 4. File Select Background
    img_file = generate_file_select_bg()
    save_bmp_32("assets/ui/startup/file_select_bg.bmp", img_file)

    # 5. Link's House Map
    img_house = generate_link_house_map()
    save_bmp_32("assets/regions/map_link_house.bmp", img_house)
    save_bmp_32("assets/regions/usa/map_link_house.bmp", img_house)

    # 6. Throne Room Map
    img_throne = generate_throne_room_map()
    save_bmp_32("assets/regions/map_throne_room.bmp", img_throne)
    save_bmp_32("assets/regions/usa/map_throne_room.bmp", img_throne)

    print("[ASSETS] Todos os 6 assets mestres de abertura foram gerados com sucesso!")

if __name__ == '__main__':
    main()
