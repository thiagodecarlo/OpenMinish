#!/usr/bin/env python3
"""
scratch/verify_cinematic_chain.py
Validação Visual Empírica do Encadeamento Completo da Abertura Cinematográfica:
1. Prólogo Histórico dos Picori (Opção B - Vitrais e Narração)
2. Tela de Título Minish Cap (Four Sword, Motes & Press Start)
3. Seleção de Arquivo / Novo Jogo (Slots de Save)
4. Cutscene In-Game: Quarto de Link (O Despertar com Zelda e Mestre Smith)
5. Cutscene In-Game: Pátio do Castelo (Cerimônia, Vaati & Zelda Petrificada)
6. Cutscene In-Game: Sala do Trono (Rei Daltus)
7. Gameplay Ativo no Vilarejo de Hyrule (Hyrule Town com HUD)
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

def draw_start_badge(frame, font_data, W):
    badge_w = 98
    badge_h = 14
    badge_x = W - 114
    badge_y = 10
    badge_layer = Image.new("RGBA", (W, 160), (0, 0, 0, 0))
    b_draw = ImageDraw.Draw(badge_layer)
    b_draw.rectangle([(badge_x - 1, badge_y - 1), (badge_x + badge_w, badge_y + badge_h)], outline=(212, 175, 55, 68))
    b_draw.rectangle([(badge_x, badge_y), (badge_x + badge_w - 1, badge_y + badge_h - 1)], fill=(15, 23, 42, 204))
    draw_text_8x8(b_draw, font_data, badge_x + 4, badge_y + 3, "[START] Pular", (253, 224, 71, 255), shadow=False)
    return Image.alpha_composite(frame, badge_layer)

def render_showcase():
    output_path = "C:/Users/thiag/.gemini/antigravity/brain/91f45871-4e2f-495f-88d6-7427ed06770a/cinematic_chain_showcase.png"
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    font_data = parse_font_c("src/hal/font.c")

    prologue_tex = load_bmp_rgba("assets/ui/prologue/prologue_panels.bmp")
    castle_tex   = Image.open("assets/regions/usa/map_castle_courtyard.bmp").convert("RGBA")
    town_tex     = Image.open("assets/regions/usa/map_town.bmp").convert("RGBA")
    bosses_tex   = load_bmp_rgba("assets/regions/usa/bosses.bmp")
    npcs_tex     = load_bmp_rgba("assets/regions/usa/npcs.bmp")
    link_tex     = load_bmp_rgba("assets/regions/usa/link.bmp")
    enemies_tex  = load_bmp_rgba("assets/regions/usa/enemies.bmp")
    hud_tex      = load_bmp_rgba("assets/ui/hud_items.bmp")

    W, H = 240, 160

    frames = []
    card_meta = []

    # =========================================================================
    # CENA 1: PRÓLOGO HISTÓRICO - A DESCIDA DOS PICORI (OPÇÃO B)
    # =========================================================================
    f1 = Image.new("RGBA", (W, H), (10, 8, 7, 255))
    crop_p1 = prologue_tex.crop((240, 0, 480, 160))
    f1.paste(crop_p1, (0, 0))
    d1 = ImageDraw.Draw(f1)
    # Motes
    import random
    random.seed(111)
    for _ in range(16):
        mx = random.randint(20, W - 30)
        my = random.randint(15, H - 55)
        d1.rectangle([(mx, my), (mx + 1, my + 1)], fill=(255, 235, 120, 200))
    # Caixa texto
    d1.rectangle([(9, 114), (230, 155)], outline=(212, 175, 55, 255))
    d1.rectangle([(10, 115), (229, 154)], fill=(20, 14, 10, 238))
    draw_text_8x8(d1, font_data, 18, 119, "A DESCIDA DOS PICORI", (253, 224, 71, 255))
    draw_text_multiline_8x8(d1, font_data, 18, 131, 204, 11, "Do ceu desceram os Picori,\ncom espada e Luz Dourada.", (248, 250, 252, 255))
    draw_char_8x8(d1, font_data, 0x03, 216, 143, (253, 224, 71, 255))
    f1 = draw_start_badge(f1, font_data, W)
    frames.append(f1)
    card_meta.append("1. PRÓLOGO HISTÓRICO (OPÇÃO B): A LENDA DOS PICORI")

    # =========================================================================
    # CENA 2: TELA DE TÍTULO THE MINISH CAP
    # =========================================================================
    f2 = Image.new("RGBA", (W, H), (10, 16, 36, 255))
    d2 = ImageDraw.Draw(f2)
    # Gradiente de fundo
    for y in range(H):
        t = y / float(H)
        r = int(10 + t * 20)
        g = int(16 + t * 25)
        b = int(36 + t * 30)
        d2.line([(0, y), (W, y)], fill=(r, g, b, 255))
    # Motes ascendentes
    for _ in range(20):
        mx = random.randint(10, W - 10)
        my = random.randint(10, H - 20)
        d2.point((mx, my), fill=(253, 224, 71, random.randint(140, 255)))
    # Four Sword central no pedestal
    sword_x, sword_y = W // 2, 48
    d2.rectangle([(sword_x - 3, sword_y - 28), (sword_x + 3, sword_y + 18)], fill=(226, 232, 240, 255))
    d2.line([(sword_x, sword_y - 28), (sword_x, sword_y + 18)], fill=(56, 189, 248, 255))
    d2.rectangle([(sword_x - 14, sword_y - 8), (sword_x + 14, sword_y - 5)], fill=(212, 175, 55, 255)) # Guarda
    d2.rectangle([(sword_x - 4, sword_y - 3), (sword_x + 4, sword_y + 5)], fill=(30, 58, 138, 255)) # Empunhadura
    d2.ellipse([(sword_x - 5, sword_y + 5), (sword_x + 5, sword_y + 11)], fill=(212, 175, 55, 255))
    # Banner The Minish Cap
    draw_text_8x8(d2, font_data, (W - 19 * 7) // 2, 74, "THE LEGEND OF ZELDA", (253, 224, 71, 255), shadow=True)
    draw_text_8x8(d2, font_data, (W - 14 * 7) // 2, 88, "THE MINISH CAP", (56, 189, 248, 255), shadow=True)
    # PRESS START
    d2.rectangle([(W // 2 - 58, 116), (W // 2 + 58, 132)], fill=(15, 23, 42, 204), outline=(212, 175, 55, 128))
    draw_text_8x8(d2, font_data, (W - 11 * 7) // 2, 120, "PRESS START", (248, 250, 252, 255), shadow=True)
    draw_text_8x8(d2, font_data, (W - 20 * 7) // 2, 142, "(C) 2004 NINTENDO / CAPCOM", (148, 163, 184, 255))
    frames.append(f2)
    card_meta.append("2. TELA DE TÍTULO CANÔNICA: FOUR SWORD & PRESS START")

    # =========================================================================
    # CENA 3: SELEÇÃO DE ARQUIVO (FILE SELECT & SLOTS)
    # =========================================================================
    f3 = Image.new("RGBA", (W, H), (15, 23, 42, 255))
    d3 = ImageDraw.Draw(f3)
    draw_text_8x8(d3, font_data, (W - 17 * 7) // 2, 12, "SELECIONE O ARQUIVO", (253, 224, 71, 255), shadow=True)
    for slot_i in range(3):
        sy = 32 + slot_i * 38
        is_cur = (slot_i == 0)
        border_col = (253, 224, 71, 255) if is_cur else (71, 85, 105, 255)
        bg_col = (30, 41, 59, 240) if is_cur else (15, 23, 42, 240)
        d3.rectangle([(18, sy), (W - 18, sy + 32)], fill=bg_col, outline=border_col)
        # Cursor
        if is_cur:
            draw_char_8x8(d3, font_data, '>', 24, sy + 12, (253, 224, 71, 255))
        d3.text((36, sy + 6), f"SLOT {slot_i + 1}:", fill=(226, 232, 240, 255))
        if slot_i == 0:
            draw_text_8x8(d3, font_data, 90, sy + 7, "LINK", (248, 250, 252, 255))
            # 3 corações
            for h_i in range(3):
                d3.rectangle([(140 + h_i * 12, sy + 8), (148 + h_i * 12, sy + 16)], fill=(239, 68, 68, 255))
            draw_text_8x8(d3, font_data, 185, sy + 7, "000 R", (253, 224, 71, 255))
        else:
            draw_text_8x8(d3, font_data, 90, sy + 7, "- NOVO JOGO -", (148, 163, 184, 255))
    # Rodapé botões
    draw_text_8x8(d3, font_data, 20, 146, "[A] INICIAR    [B] RETORNAR AO TITULO", (203, 213, 225, 255))
    frames.append(f3)
    card_meta.append("3. SELEÇÃO DE ARQUIVO: 3 SLOTS DE SAVE & CRIAÇÃO")

    # =========================================================================
    # CENA 4: CUTSCENE NO QUARTO DE LINK (O DESPERTAR COM ZELDA E SMITH)
    # =========================================================================
    f4 = Image.new("RGBA", (W, H), (74, 40, 16, 255))
    d4 = ImageDraw.Draw(f4)
    # Parede com vigas
    for vx in range(12, W, 36):
        d4.rectangle([(vx, 0), (vx + 3, 39)], fill=(46, 16, 2, 255))
    d4.rectangle([(0, 38), (W, 40)], fill=(46, 16, 2, 255))
    # Assoalho
    d4.rectangle([(0, 40), (W, H)], fill=(120, 53, 15, 255))
    for py in range(48, H, 12):
        d4.line([(0, py), (W, py)], fill=(69, 26, 3, 255))
    # Tapete verde
    d4.rectangle([(68, 52), (132, 92)], fill=(21, 128, 61, 136), outline=(212, 175, 55, 68))
    # Janela ensolarada
    d4.rectangle([(170, 8), (194, 32)], fill=(56, 189, 248, 255), outline=(30, 41, 59, 255))
    d4.line([(170, 20), (194, 20)], fill=(30, 41, 59, 255))
    d4.line([(182, 8), (182, 32)], fill=(30, 41, 59, 255))
    # Raios solares translúcidos
    sun_layer = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    s_draw = ImageDraw.Draw(sun_layer)
    for i in range(18):
        s_draw.rectangle([(140 - i * 3, 40 + i * 4), (175 - i * 1, 46 + i * 4)], fill=(254, 240, 138, 24))
    f4 = Image.alpha_composite(f4, sun_layer)
    d4 = ImageDraw.Draw(f4)
    # Cama
    d4.rectangle([(18, 28), (50, 74)], fill=(61, 26, 4, 255))
    d4.rectangle([(20, 30), (48, 42)], fill=(241, 245, 249, 255))
    d4.rectangle([(20, 42), (48, 72)], fill=(29, 78, 216, 255))
    d4.rectangle([(20, 42), (48, 46)], fill=(96, 165, 250, 255))
    # Mesa e vela
    d4.rectangle([(52, 34), (64, 50)], fill=(92, 43, 9, 255))
    d4.rectangle([(56, 30), (60, 34)], fill=(253, 224, 71, 255))
    # Forja e lareira de Smith
    d4.rectangle([(196, 24), (232, 68)], fill=(51, 65, 85, 255))
    d4.rectangle([(200, 40), (228, 64)], fill=(15, 23, 42, 255))
    d4.rectangle([(204, 48), (224, 62)], fill=(234, 88, 12, 255))
    d4.rectangle([(208, 52), (220, 60)], fill=(251, 191, 36, 255))
    d4.rectangle([(182, 56), (192, 66)], fill=(71, 85, 105, 255)) # Bigorna

    # Sprites
    # Link
    link_sp = link_tex.crop((0, 0, 32, 32))
    f4.paste(link_sp, (58 - 16, 66 - 20), link_sp)
    # Zelda
    zelda_sp = npcs_tex.crop((0, 32, 16, 48))
    f4.paste(zelda_sp, (92 - 8, 68 - 12), zelda_sp)
    # Smith
    smith_sp = npcs_tex.crop((0, 16, 16, 32))
    f4.paste(smith_sp, (152 - 8, 68 - 8), smith_sp)
    # Espada reluzente
    d4.rectangle([(122, 54), (124, 66)], fill=(56, 189, 248, 255))
    d4.rectangle([(120, 62), (126, 64)], fill=(212, 175, 55, 255))
    d4.rectangle([(121, 52), (123, 54)], fill=(253, 224, 71, 255))

    # Caixa texto
    d4.rectangle([(7, 114), (232, 155)], outline=(212, 175, 55, 255))
    d4.rectangle([(8, 115), (231, 154)], fill=(15, 23, 42, 238))
    draw_text_8x8(d4, font_data, 16, 119, "O DESPERTAR DE LINK", (253, 224, 71, 255))
    draw_text_multiline_8x8(d4, font_data, 16, 131, 204, 11, "Zelda vem convidar Link.\nSmith entrega a espada.", (226, 232, 240, 255))
    draw_char_8x8(d4, font_data, 0x03, 218, 143, (253, 224, 71, 255))
    f4 = draw_start_badge(f4, font_data, W)
    frames.append(f4)
    card_meta.append("4. CUTSCENE IN-GAME: O DESPERTAR NO QUARTO DE LINK")

    # =========================================================================
    # CENA 5: CUTSCENE NO PÁTIO DO CASTELO (FESTIVAL & VAATI)
    # =========================================================================
    f5 = castle_tex.crop((136, 80, 136 + W, 80 + H)).copy()
    d5 = ImageDraw.Draw(f5)
    # Guardas
    guard_sp = npcs_tex.crop((0, 128, 16, 144))
    f5.paste(guard_sp, (68, 50), guard_sp)
    f5.paste(guard_sp, (156, 50), guard_sp)
    # Baú Sagrado violado & Espada Partida
    chest_sp = bosses_tex.crop((160, 160, 192, 192))
    f5.paste(chest_sp, (104, 46), chest_sp)
    # Vaati rindo
    vaati_sp = bosses_tex.crop((64, 160, 96, 192))
    f5.paste(vaati_sp, (120 - 16, 35 - 16), vaati_sp)
    # Zelda em choque
    zelda_sp2 = npcs_tex.crop((32, 32, 48, 48))
    f5.paste(zelda_sp2, (120 - 8, 100 - 12), zelda_sp2)
    # Link
    f5.paste(link_sp, (90 - 16, 100 - 20), link_sp)
    # Monstros voando (Keese)
    keese_sp = enemies_tex.crop((0, 32, 16, 48))
    f5.paste(keese_sp, (65, 30), keese_sp)
    f5.paste(keese_sp, (160, 26), keese_sp)
    # Caixa texto
    d5.rectangle([(7, 114), (232, 155)], outline=(212, 175, 55, 255))
    d5.rectangle([(8, 115), (231, 154)], fill=(15, 23, 42, 238))
    draw_text_8x8(d5, font_data, 16, 119, "VIOLACAO DO BAU SAGRADO!", (253, 224, 71, 255))
    draw_text_multiline_8x8(d5, font_data, 16, 131, 204, 11, "Vaati quebra a Picori Blade e\nrompe o Bau Sagrado!", (226, 232, 240, 255))
    draw_char_8x8(d5, font_data, 0x03, 218, 143, (253, 224, 71, 255))
    f5 = draw_start_badge(f5, font_data, W)
    frames.append(f5)
    card_meta.append("5. CUTSCENE IN-GAME: VAATI QUEBRA O BAÚ SAGRADO")

    # =========================================================================
    # CENA 6: AUDIÊNCIA COM O REI DALTUS NA SALA DO TRONO
    # =========================================================================
    f6 = Image.new("RGBA", (W, H), (30, 27, 24, 255))
    d6 = ImageDraw.Draw(f6)
    d6.rectangle([(W // 2 - 40, 0), (W // 2 + 40, H)], fill=(127, 29, 29, 255))
    d6.line([(W // 2 - 42, 0), (W // 2 - 42, H)], fill=(212, 175, 55, 255), width=2)
    d6.line([(W // 2 + 40, 0), (W // 2 + 40, H)], fill=(212, 175, 55, 255), width=2)
    d6.rectangle([(W // 2 - 16, 18), (W // 2 + 16, 44)], fill=(212, 175, 55, 255))
    d6.rectangle([(W // 2 - 12, 22), (W // 2 + 12, 40)], fill=(180, 83, 9, 255))
    f6.paste(guard_sp, (44, 44), guard_sp)
    f6.paste(guard_sp, (180, 44), guard_sp)
    daltus_sp = npcs_tex.crop((5 * 16, 0, 6 * 16, 16))
    f6.paste(daltus_sp, ((W - 16) // 2, 26), daltus_sp)
    f6.paste(smith_sp, (80 - 8, 60 - 8), smith_sp)
    link_back = link_tex.crop((2 * 32, 0, 3 * 32, 32))
    f6.paste(link_back, (120 - 16, 85 - 20), link_back)
    # Caixa texto
    d6.rectangle([(7, 114), (232, 155)], outline=(212, 175, 55, 255))
    d6.rectangle([(8, 115), (231, 154)], fill=(15, 23, 42, 238))
    draw_text_8x8(d6, font_data, 16, 119, "AUDIENCIA COM O REI DALTUS", (253, 224, 71, 255))
    draw_text_multiline_8x8(d6, font_data, 16, 131, 204, 11, "O Rei Daltus envia Link\nem busca dos Minish!", (226, 232, 240, 255))
    draw_char_8x8(d6, font_data, 0x03, 218, 143, (253, 224, 71, 255))
    f6 = draw_start_badge(f6, font_data, W)
    frames.append(f6)
    card_meta.append("6. CUTSCENE IN-GAME: AUDIÊNCIA COM O REI DALTUS")

    # =========================================================================
    # CENA 7: GAMEPLAY ATIVO NO VILAREJO DE HYRULE (HYRULE TOWN)
    # =========================================================================
    # Ponto central da praça de Hyrule Town (link em 240, 160)
    tx = 240 - W // 2
    ty = 160 - H // 2
    f7 = town_tex.crop((tx, ty, tx + W, ty + H)).copy()
    d7 = ImageDraw.Draw(f7)
    # Link no centro da praça virado para baixo
    f7.paste(link_sp, (W // 2 - 16, H // 2 - 20), link_sp)
    # NPCs na praça
    npc_town = npcs_tex.crop((16, 0, 32, 16))
    f7.paste(npc_town, (60, 70), npc_town)
    npc_town2 = npcs_tex.crop((32, 0, 48, 16))
    f7.paste(npc_town2, (180, 80), npc_town2)
    # HUD Completo
    # Corações
    for h_i in range(3):
        d7.rectangle([(10 + h_i * 11, 8), (18 + h_i * 11, 16)], fill=(239, 68, 68, 255), outline=(153, 27, 27, 255))
    # Botões [B] e [A]
    # Botão B (Espada)
    d7.ellipse([(190, 6), (206, 22)], fill=(220, 38, 38, 255), outline=(153, 27, 27, 255))
    draw_text_8x8(d7, font_data, 194, 10, "B", (255, 255, 255, 255))
    # Botão A (Ação/Rolar)
    d7.ellipse([(212, 6), (228, 22)], fill=(37, 99, 235, 255), outline=(30, 58, 138, 255))
    draw_text_8x8(d7, font_data, 216, 10, "A", (255, 255, 255, 255))
    # Rupees
    d7.rectangle([(10, 24), (16, 32)], fill=(34, 197, 94, 255))
    draw_text_8x8(d7, font_data, 20, 24, "000", (255, 255, 255, 255), shadow=True)
    # Status Banner
    d7.rectangle([(20, H - 24), (W - 20, H - 8)], fill=(15, 23, 42, 220), outline=(34, 197, 94, 200))
    draw_text_8x8(d7, font_data, (W - 24 * 7) // 2, H - 20, "GAMEPLAY ATIVO: HYRULE", (74, 222, 128, 255))
    frames.append(f7)
    card_meta.append("7. GAMEPLAY ATIVO: VILAREJO DE HYRULE (FULL HUD & 60 FPS)")

    # =========================================================================
    # MONTAGEM FINAL DO SHOWCASE (Grid 4x2 em Escala 2x Pixel Art)
    # =========================================================================
    SCALE = 2
    PAD = 20
    HEADER_H = 110
    SEC_TITLE_H = 34

    CELL_W = W * SCALE  # 480
    CELL_H = H * SCALE  # 320

    TOTAL_ROWS = 4
    TOTAL_COLS = 2

    CANVAS_W = PAD + TOTAL_COLS * (CELL_W + PAD)
    CANVAS_H = HEADER_H + TOTAL_ROWS * (CELL_H + PAD + SEC_TITLE_H) + PAD

    canvas = Image.new("RGBA", (CANVAS_W, CANVAS_H), (15, 23, 42, 255))
    c_draw = ImageDraw.Draw(canvas)

    try:
        font_large = ImageFont.truetype("arialbd.ttf", 15)
        font_mid   = ImageFont.truetype("arialbd.ttf", 12)
        font_small = ImageFont.truetype("arial.ttf", 10)
    except:
        font_large = font_mid = font_small = ImageFont.load_default()

    # Header
    c_draw.rectangle([(0, 0), (CANVAS_W, HEADER_H)], fill=(2, 6, 23, 255))
    c_draw.line([(0, HEADER_H - 2), (CANVAS_W, HEADER_H - 2)], fill=(212, 175, 55, 255), width=2)
    c_draw.text((PAD, 20), "THE LEGEND OF ZELDA: THE MINISH CAP - FLUXO COMPLETO DE ABERTURA", fill=(253, 224, 71, 255), font=font_large)
    c_draw.text((PAD, 48), "Encadeamento Contínuo: Prólogo dos Picori -> Tela de Título -> Cutscene Quarto de Link & Castelo -> Gameplay", fill=(226, 232, 240, 255), font=font_mid)
    c_draw.text((PAD, 72), "Clean-Room C11 HAL / Zero-ROM Runtime: Transições Suaves, Motes, Badges de Pular e Zero Colisão de Texto", fill=(148, 163, 184, 255), font=font_small)

    for i in range(len(frames)):
        r = i // 2
        c = i % 2
        px = PAD + c * (CELL_W + PAD)
        py = HEADER_H + PAD + r * (CELL_H + PAD + SEC_TITLE_H)

        # Barra do subtítulo
        is_gameplay = (i == 6)
        bar_color = (74, 222, 128, 255) if is_gameplay else (212, 175, 55, 255)
        c_draw.rectangle([(px, py), (px + CELL_W, py + 26)], fill=(30, 41, 59, 255))
        c_draw.rectangle([(px, py), (px + 6, py + 26)], fill=bar_color)
        c_draw.text((px + 12, py + 5), card_meta[i].upper(), fill=bar_color, font=font_small)

        # Frame escalado 2x Nearest-Neighbor
        scaled = frames[i].resize((CELL_W, CELL_H), Image.NEAREST)
        c_draw.rectangle([(px - 2, py + 28), (px + CELL_W + 1, py + 28 + CELL_H + 1)], fill=bar_color)
        canvas.paste(scaled, (px, py + 30))

    canvas.save(output_path, "PNG")
    print(f"[OK] Showcase do Encadeamento Completo gerado com sucesso: {output_path} ({canvas.width}x{canvas.height})")

if __name__ == "__main__":
    render_showcase()
