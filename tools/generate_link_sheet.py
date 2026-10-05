#!/usr/bin/env python3
"""
tools/generate_link_sheet.py - Canonical GBA Link Sprite Sheet Generator

Extracts authentic Link idle and walk animation cycles from the canonical Minish Cap
sprite sheet and packages them into a standardized 32-bit RGBA BMP sheet (320x128)
containing 32x32 cells:
- Row 0: Idle (Down, Right, Up, Left)
- Row 1: Walk Down (10 frames)
- Row 2: Walk Right (10 frames)
- Row 3: Walk Up (10 frames)
"""

import os
import sys
import struct
from PIL import Image

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

def generate_link_sheet(source_sheet_path):
    if not os.path.exists(source_sheet_path):
        print(f"Erro: Arquivo fonte nao encontrado: {source_sheet_path}")
        return False

    img = Image.open(source_sheet_path)
    print(f"Processando folha de sprites: {source_sheet_path} ({img.size[0]}x{img.size[1]})")

    CELL_W = 32
    CELL_H = 32
    NUM_COLS = 10
    NUM_ROWS = 6

    sheet = Image.new('RGBA', (CELL_W * NUM_COLS, CELL_H * NUM_ROWS), (0, 0, 0, 0))

    def extract_cell(center_x, bottom_y):
        src_x1 = center_x - CELL_W // 2
        src_y1 = bottom_y - 25
        src_x2 = src_x1 + CELL_W
        src_y2 = src_y1 + CELL_H
        c = img.crop((src_x1, src_y1, src_x2, src_y2)).convert('RGBA')
        orig = img.crop((src_x1, src_y1, src_x2, src_y2))
        new_data = [(0, 0, 0, 0) if o == 0 else (r, g, b, 255) for (r, g, b, a), o in zip(c.getdata(), orig.getdata())]
        c.putdata(new_data)
        return c

    # Row 0: Idle
    idle_down = extract_cell(24, 31)
    # O sprite na posição (60, 31) na folha original está virado para a ESQUERDA (nariz à esquerda)
    idle_left = extract_cell(60, 31)
    idle_right = idle_left.transpose(Image.FLIP_LEFT_RIGHT)
    idle_up = extract_cell(88, 30)

    sheet.paste(idle_down, (0 * CELL_W, 0 * CELL_H))
    sheet.paste(idle_right, (1 * CELL_W, 0 * CELL_H))
    sheet.paste(idle_up, (2 * CELL_W, 0 * CELL_H))
    sheet.paste(idle_left, (3 * CELL_W, 0 * CELL_H))

    # Row 1: Walk Down (10 frames)
    for k in range(10):
        cx = 15 + k * 32 + 9
        frame = extract_cell(cx, 99)
        sheet.paste(frame, (k * CELL_W, 1 * CELL_H))

    # Row 2: Walk Right (10 frames)
    # Na folha original, os sprites estão caminhando para a ESQUERDA.
    # Fazemos a inversão horizontal para que a Linha 2 represente a caminhada para a DIREITA.
    right_xs = [350, 380, 412, 445, 476, 510, 540, 573, 609, 636]
    for k, x in enumerate(right_xs):
        cx = x + 11
        frame = extract_cell(cx, 99).transpose(Image.FLIP_LEFT_RIGHT)
        sheet.paste(frame, (k * CELL_W, 2 * CELL_H))

    # Row 3: Walk Up (10 frames)
    for k in range(10):
        cx = 683 + k * 32 + 9
        frame = extract_cell(cx, 99)
        sheet.paste(frame, (k * CELL_W, 3 * CELL_H))

    # Carrega e integra os sprites do Link Minish e Balões Beacons (Linhas 4 e 5)
    brain_scratch = r'C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be\scratch'
    minish_sheet_path = os.path.join(brain_scratch, 'link_minish.png')
    if os.path.exists(minish_sheet_path):
        m_img = Image.open(minish_sheet_path)
        pink = (248, 192, 240)

        def extract_minish_8x8(col, row_y):
            x1 = col * 9 + 1
            y1 = row_y + 1
            c = m_img.crop((x1, y1, x1 + 8, y1 + 8)).convert('RGBA')
            pix = c.load()
            for y in range(8):
                for x in range(8):
                    if pix[x, y][:3] == pink:
                        pix[x, y] = (0, 0, 0, 0)
            return c

        def extract_bubble_16(x1, y1):
            c = m_img.crop((x1, y1, x1 + 16, y1 + 16)).convert('RGBA')
            pix = c.load()
            for y in range(16):
                for x in range(16):
                    if pix[x, y][:3] == pink:
                        pix[x, y] = (0, 0, 0, 0)
            return c

        m_down_idle     = extract_minish_8x8(3, 305)
        m_down_a        = extract_minish_8x8(4, 305)
        m_down_b        = extract_minish_8x8(6, 305)

        m_side_raw_idle = extract_minish_8x8(0, 314)
        m_side_raw_a    = extract_minish_8x8(1, 314)
        m_side_raw_b    = extract_minish_8x8(3, 314)

        m_side_r_idle   = m_side_raw_idle.transpose(Image.FLIP_LEFT_RIGHT)
        m_side_r_a      = m_side_raw_a.transpose(Image.FLIP_LEFT_RIGHT)
        m_side_r_b      = m_side_raw_b.transpose(Image.FLIP_LEFT_RIGHT)
        m_side_l_idle   = m_side_raw_idle

        m_up_idle       = extract_minish_8x8(4, 332)
        m_up_a          = extract_minish_8x8(5, 332)
        m_up_b          = extract_minish_8x8(7, 332)

        sprites_m = [
            m_down_idle, m_side_r_idle, m_up_idle, m_side_l_idle,
            m_down_a, m_down_b, m_side_r_a, m_side_r_b, m_up_a, m_up_b
        ]

        # Linha 4: Sprites do Link Minish (posicionados com os pés no nível y=25 do tile 32x32)
        for col, s in enumerate(sprites_m):
            sheet.paste(s, (col * CELL_W + 12, 4 * CELL_H + 18), s)

        # Linha 5: Balões Indicadores Canônicos (Speech Bubble Beacons, 16x16)
        b_down  = extract_bubble_16(1, 1)
        b_left  = extract_bubble_16(1, 18)
        b_right = b_left.transpose(Image.FLIP_LEFT_RIGHT)
        b_up    = extract_bubble_16(52, 1)

        bubbles = [b_down, b_right, b_up, b_left]
        for col, b in enumerate(bubbles):
            sheet.paste(b, (col * CELL_W + 8, 5 * CELL_H + 8), b)

    save_bmp_32('assets/regions/link_master.bmp', sheet)
    save_bmp_32('assets/regions/usa/link.bmp', sheet)
    save_bmp_32('assets/regions/eur/link.bmp', sheet)
    save_bmp_32('assets/regions/jpn/link.bmp', sheet)
    return True

if __name__ == '__main__':
    src = sys.argv[1] if len(sys.argv) > 1 else 'scratch/spriters_link.png'
    if not os.path.exists(src):
        # Tenta no brain scratch
        brain_scratch = r'C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be\scratch\spriters_link.png'
        if os.path.exists(brain_scratch):
            src = brain_scratch
    generate_link_sheet(src)
