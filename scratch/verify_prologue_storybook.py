#!/usr/bin/env python3
"""
scratch/verify_prologue_storybook.py
Validação Visual Empírica Pixel-Perfect do Prólogo Histórico da Lenda dos Picori
(Storybook Cinematic - Opção B)
Simula os 6 atos cinematográficos com vitrais autênticos, partículas douradas e sombrias,
caixa de texto em pergaminho imperial e o clímax da revelação de Vaati.
"""

import os
import re
import math
import struct
from PIL import Image, ImageDraw, ImageFont

def parse_font_c(font_c_path="src/hal/font.c"):
    font = [[0]*8 for _ in range(128)]
    with open(font_c_path, "r", encoding="utf-8") as f:
        content = f.read()

    pattern = re.compile(r"\[(0x[0-9a-fA-F]+|'(\\.|[^'])')\]\s*=\s*\{\s*([^}]+)\s*\}")
    for m in pattern.finditer(content):
        idx_str = m.group(1)
        bytes_str = m.group(3)
        if idx_str.startswith("0x"):
            idx = int(idx_str, 16)
        elif idx_str.startswith("'"):
            char_lit = idx_str[1:-1]
            if char_lit == "\\'": char_lit = "'"
            elif char_lit == "\\\\": char_lit = "\\"
            idx = ord(char_lit)
        else:
            continue

        raw_bytes = [int(b.strip(), 16) for b in bytes_str.split(",") if b.strip()]
        if 0 <= idx < 128 and len(raw_bytes) == 8:
            font[idx] = raw_bytes

    return font

def draw_char_8x8(img_draw, font_data, ch, x, y, color):
    ascii_code = ord(ch) if isinstance(ch, str) else ch
    if ascii_code >= 128 or ascii_code < 0:
        ascii_code = ord('?')
    glyph = font_data[ascii_code]
    for row in range(8):
        b = glyph[row]
        for col in range(8):
            if (b >> (7 - col)) & 1:
                img_draw.point((x + col, y + row), fill=color)

def draw_text_8x8(img_draw, font_data, x, y, text, color, shadow=False):
    cur_x = x
    for ch in text:
        if shadow:
            draw_char_8x8(img_draw, font_data, ch, cur_x + 1, y + 1, (0, 0, 0, 255))
        draw_char_8x8(img_draw, font_data, ch, cur_x, y, color)
        cur_x += 7

def draw_text_multiline_8x8(img_draw, font_data, x, y, max_w, line_h, text, color, shadow=False):
    lines = text.split('\n')
    cur_y = y
    for line in lines:
        draw_text_8x8(img_draw, font_data, x, cur_y, line, color, shadow)
        cur_y += line_h

def load_bmp_rgba(filepath):
    with open(filepath, 'rb') as f:
        data = f.read()
    w, h = struct.unpack('<ii', data[18:26])
    top_down = (h < 0)
    h = abs(h)
    pixels = bytearray(w * h * 4)
    off = struct.unpack('<I', data[10:14])[0]
    raw = data[off:]
    row_stride = ((w * 4 + 3) // 4) * 4
    for y in range(h):
        src_y = y if top_down else (h - 1 - y)
        row_src = src_y * row_stride
        row_dst = y * (w * 4)
        for x in range(w):
            b = raw[row_src + x*4 + 0]
            g = raw[row_src + x*4 + 1]
            r = raw[row_src + x*4 + 2]
            a = raw[row_src + x*4 + 3]
            pixels[row_dst + x*4 + 0] = r
            pixels[row_dst + x*4 + 1] = g
            pixels[row_dst + x*4 + 2] = b
            pixels[row_dst + x*4 + 3] = a
    return Image.frombytes('RGBA', (w, h), bytes(pixels))

def render_showcase():
    output_path = "C:/Users/thiag/.gemini/antigravity/brain/91f45871-4e2f-495f-88d6-7427ed06770a/prologue_storybook_showcase.png"
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    font_data = parse_font_c("src/hal/font.c")
    prologue_tex = load_bmp_rgba("assets/ui/prologue/prologue_panels.bmp")

    try:
        font_large = ImageFont.truetype("arialbd.ttf", 15)
        font_mid   = ImageFont.truetype("arialbd.ttf", 12)
        font_small = ImageFont.truetype("arial.ttf", 10)
    except:
        font_large = font_mid = font_small = ImageFont.load_default()

    W, H = 240, 160

    # Os 6 Atos Narrativos do Prólogo Histórico de Minish Cap
    acts = [
        {
            "panel": 0,
            "title": "A LENDA DOS PICORI: AS TREVAS",
            "text": "Ha muito tempo, o mundo\nquase sucumbiu nas trevas.",
            "is_dark": False,
            "subtitle": "Ato 1: O Mal Antigo e a Perdição de Hyrule"
        },
        {
            "panel": 1,
            "title": "A DESCIDA DOS PICORI",
            "text": "Do ceu desceram os Picori,\ncom espada e Luz Dourada.",
            "is_dark": False,
            "subtitle": "Ato 2: A Graça Divina do Povo Minish"
        },
        {
            "panel": 2,
            "title": "O HEROI E A ESPADA SAGRADA",
            "text": "Com coragem e sabedoria,\no heroi baniu as trevas.",
            "is_dark": False,
            "subtitle": "Ato 3: O Triunfo do Herói dos Homens"
        },
        {
            "panel": 2,
            "title": "O SELAMENTO DO BAU SAGRADO",
            "text": "A paz voltou e o heroi selou\no mal no sagrado Bau!",
            "is_dark": False,
            "subtitle": "Ato 4: O Selo Sagrado da Picori Blade"
        },
        {
            "panel": 3,
            "title": "O FESTIVAL SECULAR DE HYRULE",
            "text": "A Luz brilha na Princesa e\nHyrule celebra o festival.",
            "is_dark": False,
            "subtitle": "Ato 5: O Século de Paz e a Força da Luz"
        },
        {
            "panel": 3,
            "title": "UMA SOMBRA ESCOLHE SEU MOMENTO...",
            "text": "Heh heh heh...\nEntao o segredo e esse...",
            "is_dark": True,
            "subtitle": "Ato 6: O Mago Sombrio Vaati Espreita a Luz"
        }
    ]

    act_frames = []

    for idx, act in enumerate(acts):
        frame = Image.new("RGBA", (W, H), (10, 8, 7, 255))
        draw = ImageDraw.Draw(frame)

        # 1. Painel de vitral
        col = act["panel"] % 2
        row = act["panel"] // 2
        crop_p = prologue_tex.crop((col * 240, row * 160, col * 240 + 240, row * 160 + 160))
        frame.paste(crop_p, (0, 0))

        # 2. Clímax Sombrio (Ato 6): Vinheta violeta translúcida
        if act["is_dark"]:
            dark_overlay = Image.new("RGBA", (W, H), (59, 7, 100, 85))
            frame = Image.alpha_composite(frame, dark_overlay)
            draw = ImageDraw.Draw(frame)

        # 3. Partículas de Luz em Suspensão (Motes)
        import random
        random.seed(idx * 777 + 42)
        for _ in range(20):
            mx = random.randint(15, W - 25)
            my = random.randint(15, H - 55)
            msize = random.choice([1, 2])
            if act["is_dark"]:
                mcol = (192, 132, 252, random.randint(160, 240)) # Violeta
            else:
                mcol = (255, 235, 120, random.randint(160, 240)) # Ouro
            draw.rectangle([(mx, my), (mx + msize - 1, my + msize - 1)], fill=mcol)

        # 4. Caixa de texto em pergaminho imperial
        box_w = W - 20
        box_h = 40
        box_x = 10
        box_y = H - 45

        draw.rectangle([(box_x - 1, box_y - 1), (box_x + box_w, box_y + box_h)], outline=(212, 175, 55, 255))
        draw.rectangle([(box_x, box_y), (box_x + box_w - 1, box_y + box_h - 1)], fill=(20, 14, 10, 238))
        draw.rectangle([(box_x + 2, box_y + 2), (box_x + box_w - 3, box_y + box_h - 3)], outline=(44, 29, 17, 136))

        # Título
        tcol = (192, 132, 252, 255) if act["is_dark"] else (253, 224, 71, 255)
        draw_text_8x8(draw, font_data, box_x + 8, box_y + 4, act["title"], tcol)

        # Texto
        draw_text_multiline_8x8(draw, font_data, box_x + 8, box_y + 16, box_w - 16, 11, act["text"], (248, 250, 252, 255))

        # Indicador de avanço piscante [▼] no canto inferior direito
        draw_char_8x8(draw, font_data, 0x03, box_x + box_w - 14, box_y + box_h - 12, (253, 224, 71, 255))

        # Indicador sutil de pular com START em cápsula translúcida com borda dourada
        badge_w = 98
        badge_h = 14
        badge_x = W - 114
        badge_y = 10
        badge_layer = Image.new("RGBA", (W, H), (0, 0, 0, 0))
        b_draw = ImageDraw.Draw(badge_layer)
        b_draw.rectangle([(badge_x - 1, badge_y - 1), (badge_x + badge_w, badge_y + badge_h)], outline=(212, 175, 55, 68))
        b_draw.rectangle([(badge_x, badge_y), (badge_x + badge_w - 1, badge_y + badge_h - 1)], fill=(15, 23, 42, 204))
        draw_text_8x8(b_draw, font_data, badge_x + 4, badge_y + 3, "[START] Pular", (253, 224, 71, 255), shadow=False)
        frame = Image.alpha_composite(frame, badge_layer)

        act_frames.append(frame)

    # -------------------------------------------------------------------------
    # MONTAGEM FINAL DO SHOWCASE (Resolução 1080p, Escala 2x Pixel Art)
    # -------------------------------------------------------------------------
    SCALE = 2
    PAD = 24
    HEADER_H = 110
    SEC_TITLE_H = 36

    CELL_W = W * SCALE  # 480
    CELL_H = H * SCALE  # 320

    TOTAL_ROWS = 3
    TOTAL_COLS = 2

    CANVAS_W = PAD + TOTAL_COLS * (CELL_W + PAD)
    CANVAS_H = HEADER_H + TOTAL_ROWS * (CELL_H + PAD + SEC_TITLE_H) + PAD

    canvas = Image.new("RGBA", (CANVAS_W, CANVAS_H), (15, 23, 42, 255))
    c_draw = ImageDraw.Draw(canvas)

    # Header
    c_draw.rectangle([(0, 0), (CANVAS_W, HEADER_H)], fill=(2, 6, 23, 255))
    c_draw.line([(0, HEADER_H - 2), (CANVAS_W, HEADER_H - 2)], fill=(212, 175, 55, 255), width=2)
    c_draw.text((PAD, 20), "THE LEGEND OF ZELDA: THE MINISH CAP - STORYBOOK CINEMATIC (OPÇÃO B)", fill=(253, 224, 71, 255), font=font_large)
    c_draw.text((PAD, 48), "Clean-Room C11 HAL / Zero-ROM Runtime: Prólogo Histórico da Lenda dos Picori com 6 Atos Canônicos", fill=(226, 232, 240, 255), font=font_mid)
    c_draw.text((PAD, 72), "Vitrais Góticos, Partículas de Luz em Suspensão, Efeito Typewriter, Glifo [▼] e Clímax Revelador de Vaati", fill=(148, 163, 184, 255), font=font_small)

    for i in range(6):
        r = i // 2
        c = i % 2
        px = PAD + c * (CELL_W + PAD)
        py = HEADER_H + PAD + r * (CELL_H + PAD + SEC_TITLE_H)

        # Barra do subtítulo do Ato
        c_draw.rectangle([(px, py), (px + CELL_W, py + 26)], fill=(30, 41, 59, 255))
        act_bar_color = (192, 132, 252, 255) if acts[i]["is_dark"] else (212, 175, 55, 255)
        c_draw.rectangle([(px, py), (px + 6, py + 26)], fill=act_bar_color)
        c_draw.text((px + 12, py + 5), acts[i]["subtitle"].upper(), fill=act_bar_color, font=font_small)

        # Frame escalado 2x Nearest-Neighbor
        scaled = act_frames[i].resize((CELL_W, CELL_H), Image.NEAREST)
        c_draw.rectangle([(px - 2, py + 28), (px + CELL_W + 1, py + 28 + CELL_H + 1)], fill=act_bar_color)
        canvas.paste(scaled, (px, py + 30))

    canvas.save(output_path, "PNG")
    print(f"[OK] Showcase Storybook Cinematic gerado com sucesso: {output_path} ({canvas.width}x{canvas.height})")

if __name__ == "__main__":
    render_showcase()
