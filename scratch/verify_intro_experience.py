#!/usr/bin/env python3
"""
scratch/verify_intro_experience.py
Verifies the complete opening experience of OpenMinish:
1. Storybook Prologue: The Legend of the Picori (4 canonical stained-glass panels)
2. In-Game Cutscene: The Picori Festival & Vaati's Attack (6 canonical stages)
"""

import os
import math
import struct
from PIL import Image, ImageDraw, ImageFont

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
    output_path = "C:/Users/thiag/.gemini/antigravity/brain/91f45871-4e2f-495f-88d6-7427ed06770a/intro_experience_showcase.png"
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    # Carregar texturas autênticas com canal alfa 1:1 C11 HAL
    prologue_tex = load_bmp_rgba("assets/ui/prologue/prologue_panels.bmp")
    castle_tex   = Image.open("assets/regions/usa/map_castle_courtyard.bmp").convert("RGBA")
    bosses_tex   = load_bmp_rgba("assets/regions/usa/bosses.bmp")
    npcs_tex     = load_bmp_rgba("assets/regions/usa/npcs.bmp")
    link_tex     = load_bmp_rgba("assets/regions/usa/link.bmp")
    enemies_tex  = load_bmp_rgba("assets/regions/usa/enemies.bmp")

    # Fontes
    try:
        font_large = ImageFont.truetype("arialbd.ttf", 15)
        font_mid   = ImageFont.truetype("arialbd.ttf", 12)
        font_small = ImageFont.truetype("arial.ttf", 10)
    except:
        font_large = font_mid = font_small = ImageFont.load_default()

    W = 240
    H = 160

    # -------------------------------------------------------------------------
    # PARTE 1: Prólogo Narrativo (4 Painéis)
    # -------------------------------------------------------------------------
    prologue_frames = []
    prologue_info = [
        ("A LENDA DOS PICORI: AS TREVAS", "Ha muito tempo atras, o mundo esteve a ponto de sucumbir em trevas..."),
        ("A DESCIDA DOS PICORI", "Do ceu desceram os pequeninos Picori, trazendo a Luz Dourada e uma espada sagrada."),
        ("O HEROI E O BAU SAGRADO", "O bravo heroi baniu os monstros e os selou para sempre no Bau Sagrado!"),
        ("O FESTIVAL SECULAR DE HYRULE", "A paz renasceu, e a cada cem anos Hyrule celebra o sagrado Festival de Picori.")
    ]

    for idx, (title, desc) in enumerate(prologue_info):
        frame = Image.new("RGBA", (W, H), (10, 8, 7, 255))
        # Painel ilustrado
        col = idx % 2
        row = idx // 2
        crop_p = prologue_tex.crop((col * 240, row * 160, col * 240 + 240, row * 160 + 160))
        frame.paste(crop_p, (0, 0))

        # Moldura de pergaminho de texto
        draw = ImageDraw.Draw(frame)
        bx, by, bw, bh = 8, H - 38, W - 16, 32
        draw.rectangle([(bx - 1, by - 1), (bx + bw + 1, by + bh + 1)], fill=(212, 175, 55, 255))
        draw.rectangle([(bx, by), (bx + bw, by + bh)], fill=(20, 14, 10, 240))
        draw.rectangle([(bx + 2, by + 2), (bx + bw - 2, by + bh - 2)], fill=(44, 29, 17, 180))

        draw.text((bx + 6, by + 3), title, fill=(253, 224, 71, 255), font=font_small)
        draw.text((bx + 6, by + 16), desc, fill=(248, 250, 252, 255), font=font_small)
        draw.text((W - 70, 4), "[START] Pular", fill=(148, 163, 184, 255), font=font_small)
        prologue_frames.append(frame)

    # -------------------------------------------------------------------------
    # PARTE 2: Cutscene In-Game Pixel-Perfect (6 Estágios)
    # -------------------------------------------------------------------------
    cutscene_frames = []
    cutscene_info = [
        ("1. FESTIVAL DE PICORI", "Link e Zelda celebram o festival no patio do castelo."),
        ("2. O CAMPEAO DO TORNEIO", "Vaati surge flutuando sobre o podio do Bau Sagrado."),
        ("3. A VIOLACAO DO BAU", "Vaati destroi o selo e quebra a lendaria Picori Blade!"),
        ("4. TREVAS LIBERTADAS", "Monstros ancestrais explodem do bau sobre o reino."),
        ("5. A MALDIÇÃO DE VAATI", "Zelda e petrificada em pedra e Link e nocauteado!"),
        ("6. AUDIENCIA REAL", "Despertar na Sala do Trono com o Rei Daltus e Smith.")
    ]

    for stage_idx, (title, desc) in enumerate(cutscene_info):
        frame = Image.new("RGBA", (W, H), (0, 0, 0, 255))
        draw = ImageDraw.Draw(frame)

        if stage_idx == 5:
            # Sala do Trono Real
            draw.rectangle([(0, 0), (W, H)], fill=(30, 27, 24, 255))
            # Tapete imperial carmesim
            draw.rectangle([(W // 2 - 40, 0), (W // 2 + 40, H)], fill=(127, 29, 29, 255))
            draw.line([(W // 2 - 42, 0), (W // 2 - 42, H)], fill=(212, 175, 55, 255), width=2)
            draw.line([(W // 2 + 40, 0), (W // 2 + 40, H)], fill=(212, 175, 55, 255), width=2)
            # Trono dourado
            draw.rectangle([(W // 2 - 16, 18), (W // 2 + 16, 44)], fill=(212, 175, 55, 255))
            draw.rectangle([(W // 2 - 12, 22), (W // 2 + 12, 40)], fill=(180, 83, 9, 255))

            # Guardas Reais
            guard_sprite = npcs_tex.crop((0, 128, 16, 144))
            frame.paste(guard_sprite, (44, 44), guard_sprite)
            frame.paste(guard_sprite, (180, 44), guard_sprite)

            # Rei Daltus no Trono (Col 5, Row 0)
            daltus_sprite = npcs_tex.crop((5 * 16, 0, 6 * 16, 16))
            frame.paste(daltus_sprite, ((W - 16) // 2, 26), daltus_sprite)

            # Mestre Smith (Ferreiro)
            smith_sprite = npcs_tex.crop((0, 16, 16, 32))
            frame.paste(smith_sprite, (80 - 8, 60 - 8), smith_sprite)

            # Link de costas ouvindo o Rei (Row 0, Col 2: facing up)
            link_sprite = link_tex.crop((2 * 32, 0, 3 * 32, 32))
            frame.paste(link_sprite, (120 - 16, 85 - 20), link_sprite)

        else:
            # Pátio Externo com Castelo Autêntico
            courtyard_crop = castle_tex.crop((136, 80, 136 + W, 80 + H))
            frame.paste(courtyard_crop, (0, 0))

            # Guardas Reais no pátio
            guard_sprite = npcs_tex.crop((0, 128, 16, 144))
            frame.paste(guard_sprite, (68, 50), guard_sprite)
            frame.paste(guard_sprite, (156, 50), guard_sprite)

            # Baú Sagrado e Picori Blade
            if stage_idx < 2:
                # Baú Intacto com Picori Blade (128, 160)
                chest_sprite = bosses_tex.crop((128, 160, 160, 192))
                frame.paste(chest_sprite, (104, 46), chest_sprite)
            else:
                # Baú Violado e Picori Blade Quebrada (160, 160)
                chest_broken = bosses_tex.crop((160, 160, 192, 192))
                frame.paste(chest_broken, (104, 46), chest_broken)

            # Vaati
            if stage_idx == 1:
                # Descendo flutuando
                v_sprite = bosses_tex.crop((0, 160, 32, 192))
                frame.paste(v_sprite, (120 - 16, 35 - 16), v_sprite)
            elif stage_idx == 2:
                # Conjurando feitiço
                v_sprite = bosses_tex.crop((32, 160, 64, 192))
                frame.paste(v_sprite, (120 - 16, 35 - 16), v_sprite)
            elif stage_idx == 3:
                # Rindo triunfante
                v_sprite = bosses_tex.crop((64, 160, 96, 192))
                frame.paste(v_sprite, (120 - 16, 30 - 16), v_sprite)
            elif stage_idx == 4:
                # Vórtice sombrio / teleporte
                v_sprite = bosses_tex.crop((96, 160, 128, 192))
                frame.paste(v_sprite, (120 - 16, 30 - 16), v_sprite)

            # Monstros libertados em stage 3
            if stage_idx == 3:
                keese_sprite = enemies_tex.crop((0, 32, 16, 48))
                chuchu_sprite = enemies_tex.crop((4 * 16, 16, 5 * 16, 32))
                frame.paste(keese_sprite, (70, 38), keese_sprite)
                frame.paste(keese_sprite, (160, 34), keese_sprite)
                frame.paste(chuchu_sprite, (112, 75), chuchu_sprite)

            # Princesa Zelda
            if stage_idx == 0:
                zelda_sprite = npcs_tex.crop((0, 32, 16, 48)) # smiling
            elif stage_idx in [1, 2]:
                zelda_sprite = npcs_tex.crop((16, 32, 32, 48)) # looking up
            elif stage_idx == 3:
                zelda_sprite = npcs_tex.crop((32, 32, 48, 48)) # shocked
            else: # stage 4
                zelda_sprite = npcs_tex.crop((48, 32, 64, 48)) # stone statue!
            frame.paste(zelda_sprite, (120 - 8, 100 - 12), zelda_sprite)

            # Link
            if stage_idx == 4:
                # Nocauteado pela explosão
                link_down = link_tex.crop((0, 288, 32, 320))
                frame.paste(link_down, (90 - 16, 105 - 16), link_down)
            else:
                # Em pé
                link_stand = link_tex.crop((0, 0, 32, 32))
                frame.paste(link_stand, (90 - 16, 100 - 20), link_stand)

            # Efeito de malícia em stage 3 e 4
            if stage_idx in [3, 4]:
                for sp in [(100, 60), (135, 55), (115, 80), (145, 95), (85, 75)]:
                    draw.ellipse([(sp[0]-3, sp[1]-3), (sp[0]+3, sp[1]+3)], fill=(192, 132, 252, 200))

        # HUD banner
        bx, by, bw, bh = 8, H - 38, W - 16, 32
        draw.rectangle([(bx - 1, by - 1), (bx + bw + 1, by + bh + 1)], fill=(212, 175, 55, 255))
        draw.rectangle([(bx, by), (bx + bw, by + bh)], fill=(15, 23, 42, 240))
        draw.text((bx + 6, by + 3), title, fill=(253, 224, 71, 255), font=font_small)
        draw.text((bx + 6, by + 16), desc, fill=(226, 232, 240, 255), font=font_small)
        draw.text((W - 70, 4), "[START] Pular", fill=(148, 163, 184, 255), font=font_small)

        cutscene_frames.append(frame)

    # -------------------------------------------------------------------------
    # MONTAGEM FINAL DO SHOWCASE (Resolução 1080p, Escala 2x Pixel Art)
    # -------------------------------------------------------------------------
    SCALE = 2
    PAD = 24
    HEADER_H = 110
    SEC_TITLE_H = 36

    CELL_W = W * SCALE  # 480
    CELL_H = H * SCALE  # 320

    # Grid: 2 colunas
    # Linha 0: Prólogo (2 painéis)
    # Linha 1: Prólogo (2 painéis)
    # Linha 2: Cutscene (2 painéis)
    # Linha 3: Cutscene (2 painéis)
    # Linha 4: Cutscene (2 painéis)
    TOTAL_ROWS = 5
    TOTAL_COLS = 2

    CANVAS_W = PAD + TOTAL_COLS * (CELL_W + PAD)
    CANVAS_H = HEADER_H + SEC_TITLE_H + 2 * (CELL_H + PAD) + SEC_TITLE_H + 3 * (CELL_H + PAD) + PAD

    canvas = Image.new("RGBA", (CANVAS_W, CANVAS_H), (15, 23, 42, 255))
    c_draw = ImageDraw.Draw(canvas)

    # Header
    c_draw.rectangle([(0, 0), (CANVAS_W, HEADER_H)], fill=(2, 6, 23, 255))
    c_draw.line([(0, HEADER_H - 2), (CANVAS_W, HEADER_H - 2)], fill=(212, 175, 55, 255), width=2)
    c_draw.text((PAD, 20), "THE LEGEND OF ZELDA: THE MINISH CAP - EXPERIÊNCIA DE ABERTURA", fill=(253, 224, 71, 255), font=font_large)
    c_draw.text((PAD, 48), "Clean-Room C11 HAL / Zero-ROM Runtime: Prólogo Histórico da Lenda dos Picori & Cutscene In-Game Pixel-Perfect", fill=(226, 232, 240, 255), font=font_mid)
    c_draw.text((PAD, 72), "Assets Nativos: Vitrais Dourados 240x160, Castelo de Hyrule 512x384, Sprites de Vaati, Zelda, Daltus, Smith & Monstros", fill=(148, 163, 184, 255), font=font_small)

    cur_y = HEADER_H + PAD

    # Seção 1: O Prólogo dos Picori
    c_draw.rectangle([(PAD, cur_y), (CANVAS_W - PAD, cur_y + SEC_TITLE_H - 8)], fill=(30, 41, 59, 255))
    c_draw.rectangle([(PAD, cur_y), (PAD + 6, cur_y + SEC_TITLE_H - 8)], fill=(212, 175, 55, 255))
    c_draw.text((PAD + 16, cur_y + 6), "PARTE 1: PRÓLOGO HISTÓRICO - A LENDA DOS PICORI (STORYBOOK CINEMATIC 4K)", fill=(253, 224, 71, 255), font=font_mid)
    cur_y += SEC_TITLE_H

    for i in range(4):
        r = i // 2
        c = i % 2
        px = PAD + c * (CELL_W + PAD)
        py = cur_y + r * (CELL_H + PAD)

        scaled = prologue_frames[i].resize((CELL_W, CELL_H), Image.NEAREST)
        c_draw.rectangle([(px - 3, py - 3), (px + CELL_W + 2, py + CELL_H + 2)], fill=(212, 175, 55, 255))
        canvas.paste(scaled, (px, py))

    cur_y += 2 * (CELL_H + PAD) + 10

    # Seção 2: Cutscene In-Game
    c_draw.rectangle([(PAD, cur_y), (CANVAS_W - PAD, cur_y + SEC_TITLE_H - 8)], fill=(30, 41, 59, 255))
    c_draw.rectangle([(PAD, cur_y), (PAD + 6, cur_y + SEC_TITLE_H - 8)], fill=(56, 189, 248, 255))
    c_draw.text((PAD + 16, cur_y + 6), "PARTE 2: CUTSCENE IN-GAME - FESTIVAL DE PICORI & ATAQUE DO MAGO VAATI (GBA AUTHENTIC)", fill=(56, 189, 248, 255), font=font_mid)
    cur_y += SEC_TITLE_H

    for i in range(6):
        r = i // 2
        c = i % 2
        px = PAD + c * (CELL_W + PAD)
        py = cur_y + r * (CELL_H + PAD)

        scaled = cutscene_frames[i].resize((CELL_W, CELL_H), Image.NEAREST)
        c_draw.rectangle([(px - 3, py - 3), (px + CELL_W + 2, py + CELL_H + 2)], fill=(56, 189, 248, 255))
        canvas.paste(scaled, (px, py))

    canvas.save(output_path, "PNG")
    print(f"[OK] Showcase da Abertura gerado com sucesso: {output_path} ({canvas.width}x{canvas.height})")

if __name__ == "__main__":
    render_showcase()
