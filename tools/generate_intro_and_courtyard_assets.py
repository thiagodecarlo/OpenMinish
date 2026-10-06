#!/usr/bin/env python3
"""
tools/generate_intro_and_courtyard_assets.py - Master Pipeline de Assets Autênticos
Gera os assets pixel-perfect para a abertura de The Legend of Zelda: The Minish Cap:
1. assets/regions/{usa,eur,jpn}/map_castle_courtyard.bmp (512x384) & master
2. assets/regions/map_castle_courtyard_collision_master.bin & regionais (32x24)
3. assets/ui/prologue/prologue_panels.bmp (480x320) & master
"""

import os
import struct
import math
from PIL import Image, ImageDraw

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

def save_binary_collision(filepath, w, h, col_bytes):
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, 'wb') as f:
        f.write(bytes(col_bytes))
    print(f"  -> Colisao salva: {filepath} ({w}x{h}, {len(col_bytes)} bytes)")

# ============================================================================
# 1. PÁTIO DO CASTELO DE HYRULE (HYRULE CASTLE COURTYARD - 512x384)
# ============================================================================
def render_authentic_castle_courtyard():
    w, h = 512, 384
    img = Image.new('RGBA', (w, h), (72, 168, 64, 255)) # Base gramado esmeralda real
    draw = ImageDraw.Draw(img)

    # 1.1 Gramado do Pátio com textura rica em padrão de pixel art
    for y in range(h):
        for x in range(w):
            # Variação sutil de grama em 3 tons GBA
            v = ((x ^ (y * 3)) % 7)
            if v == 0:
                draw.point((x, y), fill=(64, 152, 56, 255))
            elif v == 5:
                draw.point((x, y), fill=(80, 184, 72, 255))

    # 1.2 Fachada Monumental Norte do Castelo de Hyrule (Y: 0 a 110)
    # Paredão de cantaria cinza esculpida
    draw.rectangle([0, 0, w - 1, 98], fill=(136, 142, 154, 255), outline=(72, 76, 88, 255))
    # Sombreamento superior da muralha
    draw.rectangle([0, 0, w - 1, 14], fill=(112, 118, 130, 255))
    # Linhas de tijolos de cantaria com chanfros
    for by in range(16, 98, 12):
        draw.line([0, by, w - 1, by], fill=(92, 98, 110, 255), width=2)
        # Juntas verticais alternadas
        shift = 16 if ((by // 12) % 2 == 0) else 0
        for bx in range(shift, w, 32):
            draw.line([bx, by, bx, by + 12], fill=(92, 98, 110, 255), width=1)
            draw.line([bx + 1, by + 1, bx + 1, by + 12], fill=(160, 166, 178, 255), width=1) # Destaque de luz

    # Parapeitos e Ameias (Battlements) no topo da muralha
    for cx in range(0, w, 24):
        draw.rectangle([cx, 0, cx + 12, 14], fill=(160, 166, 178, 255), outline=(72, 76, 88, 255))
        draw.rectangle([cx + 2, 2, cx + 10, 12], fill=(144, 150, 162, 255))

    # Torres Cilíndricas Flanqueadoras (Esquerda: X: 48..112, Direita: X: 400..464)
    for tx in [80, 432]:
        # Corpo da torre
        draw.rectangle([tx - 28, 0, tx + 28, 98], fill=(144, 150, 162, 255), outline=(64, 68, 80, 255))
        # Sombreamento curvo cilíndrico
        draw.rectangle([tx - 26, 0, tx - 14, 98], fill=(112, 118, 130, 255))
        draw.rectangle([tx + 14, 0, tx + 26, 98], fill=(168, 174, 186, 255))
        # Fresta de flecheiro (Embrasura)
        draw.rectangle([tx - 3, 35, tx + 3, 65], fill=(32, 34, 42, 255), outline=(72, 76, 88, 255))
        # Telhado Cônico de Ardósia Azul Real
        roof_pts = [(tx - 34, 0), (tx + 34, 0), (tx, -48)]
        draw.polygon(roof_pts, fill=(40, 96, 184, 255), outline=(24, 60, 120, 255))
        draw.polygon([(tx - 32, 0), (tx, 0), (tx, -46)], fill=(28, 72, 144, 255)) # Sombra esquerda
        # Flâmula dourada no pináculo
        draw.line([tx, -48, tx, -58], fill=(215, 175, 45, 255), width=2)
        draw.polygon([(tx, -58), (tx + 12, -54), (tx, -50)], fill=(224, 48, 48, 255))

    # Grande Portal Gótico com Grade Levadiça (Portcullis) no centro (X: 208 a 304, Y: 24 a 98)
    gx1, gx2 = 216, 296
    # Moldura monumental de pedra polida
    draw.rectangle([gx1 - 8, 20, gx2 + 8, 98], fill=(168, 174, 186, 255), outline=(64, 68, 80, 255))
    # Arco ogival superior
    draw.chord([gx1, 10, gx2, 70], start=180, end=0, fill=(168, 174, 186, 255), outline=(64, 68, 80, 255))
    # Interior do portal (Escuridão e profundidade)
    draw.rectangle([gx1, 40, gx2, 98], fill=(24, 26, 32, 255))
    draw.chord([gx1 + 4, 16, gx2 - 4, 66], start=180, end=0, fill=(24, 26, 32, 255))
    # Grade de ferro forjado (Portcullis)
    for ix in range(gx1 + 8, gx2, 10):
        draw.line([ix, 32, ix, 98], fill=(96, 104, 116, 255), width=2)
        draw.line([ix, 32, ix, 98], fill=(176, 184, 196, 255), width=1)
    for iy in range(40, 98, 12):
        draw.line([gx1 + 4, iy, gx2 - 4, iy], fill=(96, 104, 116, 255), width=2)

    # Estandartes Reais Vermelho & Ouro de Hyrule ladeando o portal
    for bx in [184, 328]:
        # Haste de latão dourado
        draw.line([bx - 12, 28, bx + 12, 28], fill=(225, 185, 45, 255), width=2)
        draw.point((bx - 13, 28), fill=(255, 220, 90, 255))
        draw.point((bx + 13, 28), fill=(255, 220, 90, 255))
        # Tecido carmesim com ponta em cauda de andorinha
        banner_pts = [(bx - 10, 29), (bx + 10, 29), (bx + 10, 80), (bx, 68), (bx - 10, 80)]
        draw.polygon(banner_pts, fill=(185, 28, 36, 255), outline=(120, 16, 22, 255))
        draw.polygon([(bx - 8, 31), (bx + 8, 31), (bx + 8, 76), (bx, 65), (bx - 8, 76)], fill=(210, 36, 44, 255))
        # Brasão dourado da Triforce no estandarte
        draw.polygon([(bx, 42), (bx - 5, 52), (bx + 5, 52)], fill=(245, 205, 45, 255), outline=(160, 120, 20, 255))

    # 1.3 O Pódio Sagrado & Altar do Baú (Y: 104 a 144)
    # Plataforma de mármore elevado com degraus
    dais_x1, dais_x2 = 180, 332
    # Degrau inferior
    draw.rectangle([dais_x1 - 10, 104, dais_x2 + 10, 146], fill=(160, 155, 145, 255), outline=(90, 85, 75, 255))
    # Degrau médio
    draw.rectangle([dais_x1 - 4, 108, dais_x2 + 4, 142], fill=(185, 180, 170, 255), outline=(110, 105, 95, 255))
    # Plataforma principal de mármore real
    draw.rectangle([dais_x1, 112, dais_x2, 138], fill=(215, 210, 200, 255), outline=(135, 130, 120, 255))
    # Mosaico da Triforce dourada cravada no piso do pódio
    tf_cx, tf_cy = 256, 125
    draw.polygon([(tf_cx, tf_cy - 8), (tf_cx - 9, tf_cy + 7), (tf_cx + 9, tf_cy + 7)], fill=(235, 195, 45, 255), outline=(160, 120, 20, 255))
    draw.polygon([(tf_cx, tf_cy + 7), (tf_cx - 4, tf_cy), (tf_cx + 4, tf_cy)], fill=(215, 210, 200, 255))

    # Balustrada de pedra ornamentada flanqueando o pódio
    for lx in range(dais_x1 + 6, 225, 10):
        draw.rectangle([lx, 102, lx + 5, 114], fill=(225, 220, 210, 255), outline=(110, 105, 95, 255))
    draw.line([dais_x1 + 4, 102, 228, 102], fill=(235, 230, 220, 255), width=3)

    for rx in range(286, dais_x2 - 5, 10):
        draw.rectangle([rx, 102, rx + 5, 114], fill=(225, 220, 210, 255), outline=(110, 105, 95, 255))
    draw.line([284, 102, dais_x2 - 4, 102], fill=(235, 230, 220, 255), width=3)

    # Escadaria Cerimonial Central (Y: 138 a 156, X: 228 a 284)
    stairs_x1, stairs_x2 = 224, 288
    for step_y in range(138, 156, 4):
        draw.rectangle([stairs_x1, step_y, stairs_x2, step_y + 4], fill=(195, 190, 180, 255), outline=(120, 115, 105, 255))
        draw.line([stairs_x1 + 1, step_y + 1, stairs_x2 - 1, step_y + 1], fill=(230, 225, 215, 255))

    # 1.4 Promenade Central Pavimentada com Lajes de Cantaria (Y: 156 a 384, X: 216 a 296)
    prom_x1, prom_x2 = 216, 296
    draw.rectangle([prom_x1, 156, prom_x2, h - 1], fill=(185, 178, 165, 255), outline=(115, 108, 98, 255))
    # Guias de pedra cinza polida nas bordas
    draw.line([prom_x1, 156, prom_x1, h - 1], fill=(215, 208, 195, 255), width=3)
    draw.line([prom_x2, 156, prom_x2, h - 1], fill=(125, 118, 108, 255), width=3)
    # Lajes de cantaria retangulares
    for py in range(158, h, 16):
        draw.line([prom_x1 + 3, py, prom_x2 - 3, py], fill=(145, 138, 126, 255), width=1)
        shift = 18 if ((py // 16) % 2 == 0) else 0
        for px in range(prom_x1 + 4 + shift, prom_x2 - 4, 24):
            draw.line([px, py, px, py + 16], fill=(145, 138, 126, 255), width=1)

    # 1.5 Jardins Reais e Parterres Florais (Esquerda: X: 32 a 196, Direita: X: 316 a 480)
    # Sebes vivas de buxinho (Hedges) estruturadas
    for hx1, hx2 in [(32, 196), (316, 480)]:
        # Sebe Norte
        draw.rectangle([hx1, 120, hx2, 134], fill=(32, 105, 42, 255), outline=(16, 60, 24, 255))
        # Textura arbustiva da sebe
        for sx in range(hx1 + 2, hx2 - 2, 6):
            draw.ellipse([sx, 118, sx + 6, 126], fill=(48, 136, 56, 255))
        # Sebe Externa lateral
        edge_x = hx1 if (hx1 < 256) else (hx2 - 14)
        draw.rectangle([edge_x, 134, edge_x + 14, 340], fill=(32, 105, 42, 255), outline=(16, 60, 24, 255))
        for sy in range(136, 338, 6):
            draw.ellipse([edge_x - 1, sy, edge_x + 15, sy + 6], fill=(48, 136, 56, 255))
        # Sebe Interna junto à promenade
        inner_x = (hx2 - 14) if (hx1 < 256) else hx1
        draw.rectangle([inner_x, 156, inner_x + 14, 340], fill=(32, 105, 42, 255), outline=(16, 60, 24, 255))
        for sy in range(158, 338, 6):
            draw.ellipse([inner_x - 1, sy, inner_x + 15, sy + 6], fill=(48, 136, 56, 255))

    # Canteiros Florais Ornamentais em Mosaico Francês (Parterres)
    # Canteiro Esquerdo (Centro: 114, 230), Canteiro Direito (Centro: 398, 230)
    for fx_c in [114, 398]:
        # Moldura de tijolos ornamentais de barro cozido
        draw.rectangle([fx_c - 46, 160, fx_c + 46, 310], fill=(165, 85, 45, 255), outline=(95, 45, 20, 255))
        draw.rectangle([fx_c - 42, 164, fx_c + 42, 306], fill=(92, 54, 32, 255)) # Terra fértil
        # Roseta Central de Flores Vermelhas e Brancas
        draw.ellipse([fx_c - 24, 215, fx_c + 24, 255], fill=(215, 35, 45, 255))
        draw.ellipse([fx_c - 16, 221, fx_c + 16, 249], fill=(255, 245, 245, 255))
        draw.ellipse([fx_c - 8, 227, fx_c + 8, 243], fill=(245, 215, 45, 255)) # Miolo dourado

        # Bordadura de flores amarelas e azuis
        for bx in range(fx_c - 36, fx_c + 38, 10):
            draw.ellipse([bx - 3, 170, bx + 3, 176], fill=(245, 205, 35, 255))
            draw.ellipse([bx - 3, 294, bx + 3, 300], fill=(245, 205, 35, 255))
        for by in range(180, 290, 10):
            draw.ellipse([fx_c - 36, by - 3, fx_c - 30, by + 3], fill=(45, 135, 245, 255))
            draw.ellipse([fx_c + 30, by - 3, fx_c + 36, by + 3], fill=(45, 135, 245, 255))

    # Fontes Ornamentais de Água Cristalina nos Jardins
    for f_center in [(114, 345), (398, 345)]:
        fcx, fcy = f_center
        # Bacia octogonal de pedra talhada
        draw.ellipse([fcx - 22, fcy - 14, fcx + 22, fcy + 14], fill=(160, 155, 145, 255), outline=(80, 75, 68, 255))
        draw.ellipse([fcx - 18, fcy - 10, fcx + 18, fcy + 10], fill=(40, 140, 215, 255), outline=(25, 95, 155, 255))
        # Espelho d'água com reflexos celestes
        draw.ellipse([fcx - 14, fcy - 7, fcx + 14, fcy + 7], fill=(70, 185, 245, 255))
        draw.point((fcx - 4, fcy - 3), fill=(255, 255, 255, 255))
        draw.point((fcx + 5, fcy + 2), fill=(255, 255, 255, 255))
        # Chafariz central de pedra
        draw.rectangle([fcx - 3, fcy - 8, fcx + 3, fcy + 2], fill=(195, 190, 180, 255), outline=(100, 95, 88, 255))

    # Postes de Iluminação Real de Ferro Forjado ao longo da promenade
    for ly in [175, 235, 295]:
        for lx in [prom_x1 - 8, prom_x2 + 8]:
            # Base de pedra
            draw.rectangle([lx - 3, ly + 8, lx + 3, ly + 14], fill=(140, 135, 125, 255), outline=(80, 75, 68, 255))
            # Haste de ferro
            draw.line([lx, ly - 8, lx, ly + 8], fill=(45, 48, 55, 255), width=2)
            # Lanterna dourada
            draw.rectangle([lx - 4, ly - 14, lx + 4, ly - 8], fill=(245, 205, 45, 255), outline=(45, 48, 55, 255))
            draw.point((lx, ly - 11), fill=(255, 255, 255, 255))

    # Matriz de Colisão Binária (32x24 tiles = 768 bytes: 0 livre, 1 sólido)
    mw, mh = 32, 24
    col_map = bytearray(mw * mh)
    for ty in range(mh):
        for tx in range(mw):
            is_solid = False
            # Muralha norte sólida (tiles 0..6)
            if ty <= 6:
                is_solid = True
            # Bordas laterais do castelo
            elif tx <= 1 or tx >= 30:
                is_solid = True
            # Sebes densas sólidas
            elif (2 <= tx <= 12 or 19 <= tx <= 29) and (7 <= ty <= 8 or 20 <= ty <= 21):
                is_solid = True
            elif (tx in [2, 12, 19, 29]) and (7 <= ty <= 21):
                is_solid = True
            # Fontes de água sólidas
            elif (6 <= tx <= 8 or 23 <= tx <= 25) and (21 <= ty <= 22):
                is_solid = True
            col_map[ty * mw + tx] = 1 if is_solid else 0

    return img, mw, mh, col_map

def create_stained_glass_border(draw, w, h, inner_w, inner_h):
    # Authentic Gothic Stained Glass Cathedral Frame
    draw.rectangle([0, 0, w - 1, h - 1], outline=(35, 28, 24, 255), width=3)
    draw.rectangle([3, 3, w - 4, h - 4], outline=(75, 60, 48, 255), width=2)
    draw.rectangle([5, 5, w - 6, h - 6], outline=(195, 155, 65, 255), width=1)
    draw.rectangle([7, 7, w - 8, h - 8], outline=(25, 20, 18, 255), width=2)
    
    corners = [(4, 4), (w - 14, 4), (4, h - 14), (w - 14, h - 14)]
    for cx, cy in corners:
        draw.rectangle([cx, cy, cx + 9, cy + 9], fill=(212, 175, 55, 255), outline=(60, 45, 25, 255))
        draw.ellipse([cx + 2, cy + 2, cx + 7, cy + 7], fill=(180, 40, 40, 255))
        draw.point((cx + 4, cy + 4), fill=(255, 240, 180, 255))

def render_panel_0_darkness():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (18, 12, 26, 255))
    draw = ImageDraw.Draw(img)

    for y in range(h):
        fac = y / h
        r = int(24 + fac * 35 + math.sin(y * 0.15) * 8)
        g = int(14 + fac * 15)
        b = int(32 + fac * 25 + math.cos(y * 0.1) * 10)
        draw.line([0, y, w, y], fill=(r, g, b, 255))

    moon_x, moon_y = w // 2, 42
    for r in range(32, 0, -1):
        cr = min(255, 180 + (32 - r) * 2)
        cg = max(0, 40 - (32 - r))
        cb = max(0, 30 - (32 - r))
        draw.ellipse([moon_x - r, moon_y - r, moon_x + r, moon_y + r], fill=(cr, cg, cb, 255))
    draw.ellipse([moon_x - 14, moon_y - 14, moon_x + 14, moon_y + 14], fill=(18, 10, 22, 255), outline=(140, 20, 20, 255))

    for cx, cy, rad, color in [
        (45, 30, 40, (35, 18, 48, 180)),
        (195, 35, 42, (40, 20, 52, 180)),
        (85, 20, 48, (28, 14, 40, 180)),
        (155, 22, 45, (30, 15, 42, 180))
    ]:
        draw.ellipse([cx - rad, cy - rad, cx + rad, cy + rad], fill=color)

    bg_peaks = [(0, 120), (35, 65), (75, 95), (120, 55), (165, 88), (205, 60), (240, 85), (240, 160), (0, 160)]
    draw.polygon(bg_peaks, fill=(28, 18, 38, 255))
    draw.line(bg_peaks, fill=(65, 40, 80, 255), width=2)

    mg_peaks = [(0, 135), (45, 82), (90, 110), (135, 75), (180, 105), (225, 78), (240, 95), (240, 160), (0, 160)]
    draw.polygon(mg_peaks, fill=(18, 10, 26, 255))
    draw.line(mg_peaks, fill=(45, 25, 60, 255), width=2)

    fissures = [
        [(55, 90), (62, 105), (58, 120), (65, 145)],
        [(170, 85), (165, 102), (172, 118), (168, 140)],
        [(115, 80), (120, 98), (112, 125)]
    ]
    for pts in fissures:
        draw.line(pts, fill=(245, 85, 25, 255), width=2)
        draw.line(pts, fill=(255, 200, 50, 255), width=1)

    cx = w // 2
    demon_pts = [
        (cx - 36, 160), (cx - 32, 115), (cx - 48, 105), (cx - 28, 92),
        (cx - 42, 60), (cx - 24, 75), (cx - 15, 62), (cx, 68),
        (cx + 15, 62), (cx + 24, 75), (cx + 42, 60), (cx + 28, 92),
        (cx + 48, 105), (cx + 32, 115), (cx + 36, 160)
    ]
    draw.polygon(demon_pts, fill=(10, 6, 14, 255))
    draw.line(demon_pts, fill=(75, 20, 90, 255), width=2)

    draw.polygon([(cx - 18, 80), (cx - 8, 82), (cx - 14, 87)], fill=(255, 30, 30, 255))
    draw.point((cx - 13, 83), fill=(255, 240, 80, 255))
    draw.polygon([(cx + 18, 80), (cx + 8, 82), (cx + 14, 87)], fill=(255, 30, 30, 255))
    draw.point((cx + 13, 83), fill=(255, 240, 80, 255))

    draw.polygon([(cx - 12, 94), (cx + 12, 94), (cx + 8, 104), (cx - 8, 104)], fill=(180, 20, 30, 255))
    for fx in [-9, -4, 0, 4, 8]:
        draw.line([(cx + fx, 94), (cx + fx, 98)], fill=(255, 255, 255, 255))
        draw.line([(cx + fx, 104), (cx + fx, 101)], fill=(255, 255, 255, 255))

    draw.polygon([(cx - 48, 105), (cx - 68, 95), (cx - 78, 85), (cx - 65, 105), (cx - 52, 118)], fill=(12, 8, 16, 255))
    draw.line([(cx - 78, 85), (cx - 84, 80)], fill=(240, 40, 40, 255), width=2)
    draw.polygon([(cx + 48, 105), (cx + 68, 95), (cx + 78, 85), (cx + 65, 105), (cx + 52, 118)], fill=(12, 8, 16, 255))
    draw.line([(cx + 78, 85), (cx + 84, 80)], fill=(240, 40, 40, 255), width=2)

    fiends = [(48, 65), (72, 48), (95, 70), (145, 52), (170, 72), (198, 55)]
    for bx, by in fiends:
        draw.polygon([(bx, by), (bx - 9, by - 6), (bx - 4, by), (bx, by + 4), (bx + 4, by), (bx + 9, by - 6)], fill=(10, 6, 12, 255))
        draw.point((bx - 2, by - 1), fill=(255, 50, 50, 255))
        draw.point((bx + 2, by - 1), fill=(255, 50, 50, 255))

    create_stained_glass_border(draw, w, h, w - 16, h - 16)
    return img

def render_panel_1_descent():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (245, 235, 210, 255))
    draw = ImageDraw.Draw(img)

    cx, cy = w // 2, 48

    for i in range(24):
        angle = (i / 24.0) * math.pi
        x1 = cx + math.cos(angle) * 190
        y1 = 8 + math.sin(angle) * 140
        fill_col = (255, 248, 190, 160) if (i % 2 == 0) else (240, 215, 140, 140)
        draw.polygon([(cx - 10, 8), (cx + 10, 8), (int(x1), int(y1))], fill=fill_col)

    clouds = [
        (25, 20, 32), (65, 15, 38), (110, 10, 42), (160, 12, 40), (210, 18, 34),
        (40, 35, 28), (85, 28, 32), (145, 26, 34), (195, 32, 30)
    ]
    for c_x, c_y, rad in clouds:
        draw.ellipse([c_x - rad, c_y - rad, c_x + rad, c_y + rad], fill=(255, 252, 235, 240), outline=(225, 195, 120, 255))

    for r in range(36, 12, -3):
        draw.ellipse([cx - r, cy - r + 8, cx + r, cy + r + 8], fill=(255, 245, 120, 35))

    tf_w, tf_h = 32, 28
    top_pt = (cx, cy - 8)
    left_pt = (cx - tf_w // 2, cy + tf_h - 8)
    right_pt = (cx + tf_w // 2, cy + tf_h - 8)
    mid_left = ((cx + left_pt[0]) // 2, (top_pt[1] + left_pt[1]) // 2)
    mid_right = ((cx + right_pt[0]) // 2, (top_pt[1] + right_pt[1]) // 2)
    mid_bot = (cx, left_pt[1])

    draw.polygon([top_pt, mid_left, mid_right], fill=(255, 235, 60, 255), outline=(195, 145, 20, 255))
    draw.polygon([mid_left, left_pt, mid_bot], fill=(255, 235, 60, 255), outline=(195, 145, 20, 255))
    draw.polygon([mid_right, mid_bot, right_pt], fill=(255, 235, 60, 255), outline=(195, 145, 20, 255))

    draw.polygon([(cx, cy + 2), (cx - 4, cy + 6), (cx, cy + 10), (cx + 4, cy + 6)], fill=(255, 255, 255, 255))

    blade_x = cx
    blade_top = cy + 28
    blade_len = 54
    draw.line([blade_x, blade_top, blade_x, blade_top + blade_len], fill=(180, 240, 255, 180), width=5)
    draw.line([blade_x, blade_top, blade_x, blade_top + blade_len], fill=(210, 245, 255, 255), width=3)
    draw.line([blade_x, blade_top + 4, blade_x, blade_top + blade_len - 2], fill=(255, 255, 255, 255), width=1)
    draw.rectangle([blade_x - 12, blade_top + 6, blade_x + 12, blade_top + 10], fill=(245, 195, 35, 255), outline=(160, 110, 15, 255))
    draw.ellipse([blade_x - 14, blade_top + 5, blade_x - 10, blade_top + 11], fill=(215, 160, 25, 255))
    draw.ellipse([blade_x + 10, blade_top + 5, blade_x + 14, blade_top + 11], fill=(215, 160, 25, 255))
    draw.rectangle([blade_x - 3, blade_top + 6, blade_x + 3, blade_top + 10], fill=(45, 120, 240, 255))
    draw.line([blade_x, blade_top - 2, blade_x, blade_top + 6], fill=(30, 60, 130, 255), width=3)
    draw.ellipse([blade_x - 4, blade_top - 6, blade_x + 4, blade_top - 1], fill=(245, 195, 35, 255), outline=(160, 110, 15, 255))

    picori_data = [
        (cx - 58, 62, (220, 45, 45, 255), "red"),
        (cx - 32, 94, (45, 165, 75, 255), "green"),
        (cx + 34, 90, (40, 110, 225, 255), "blue"),
        (cx + 62, 65, (235, 175, 30, 255), "gold")
    ]
    for px, py, tcol, cap_type in picori_data:
        draw.ellipse([px - 11, py - 8, px - 1, py + 4], fill=(240, 250, 255, 180), outline=(180, 220, 255, 220))
        draw.ellipse([px + 1, py - 8, px + 11, py + 4], fill=(240, 250, 255, 180), outline=(180, 220, 255, 220))
        draw.polygon([(px, py - 3), (px - 5, py + 8), (px + 5, py + 8)], fill=tcol, outline=(30, 20, 15, 255))
        draw.line([px - 3, py + 7, px - 8, py + 12], fill=(255, 250, 240, 255), width=1)
        draw.ellipse([px - 4, py - 8, px + 4, py - 2], fill=(255, 225, 195, 255), outline=(50, 35, 25, 255))
        draw.point((px - 2, py - 5), fill=(40, 30, 20, 255))
        draw.point((px + 2, py - 5), fill=(40, 30, 20, 255))
        draw.polygon([(px - 4, py - 7), (px + 4, py - 7), (px - 2, py - 18)], fill=tcol, outline=(30, 20, 15, 255))
        draw.ellipse([px - 4, py - 20, px, py - 16], fill=(255, 255, 255, 255))

    mounts = [(0, 160), (35, 138), (80, 146), (135, 132), (185, 144), (240, 135), (240, 160)]
    draw.polygon(mounts, fill=(185, 160, 120, 255))
    draw.line(mounts, fill=(140, 115, 80, 255), width=2)

    create_stained_glass_border(draw, w, h, w - 16, h - 16)
    return img

def render_panel_2_hero_and_chest():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (230, 218, 190, 255))
    draw = ImageDraw.Draw(img)

    hx, hy = 76, 92
    cx, cy = 162, 92

    for r in range(54, 10, -5):
        draw.ellipse([hx - r, hy - r + 10, hx + r, hy + r + 10], fill=(255, 250, 210, 28))

    draw.rectangle([cx - 36, cy + 20, cx + 36, cy + 45], fill=(145, 138, 125, 255), outline=(75, 68, 60, 255))
    draw.rectangle([cx - 32, cy + 24, cx + 32, cy + 42], fill=(168, 160, 146, 255), outline=(95, 88, 80, 255))
    draw.polygon([(cx, cy + 28), (cx - 7, cy + 39), (cx + 7, cy + 39)], fill=(215, 175, 55, 255), outline=(95, 75, 20, 255))

    draw.rectangle([cx - 26, cy - 6, cx + 26, cy + 20], fill=(110, 48, 18, 255), outline=(48, 20, 8, 255))
    draw.chord([cx - 26, cy - 22, cx + 26, cy + 4], start=180, end=0, fill=(135, 60, 22, 255), outline=(48, 20, 8, 255))

    for bx in [-18, 18]:
        draw.line([cx + bx, cy - 14, cx + bx, cy + 20], fill=(235, 185, 30, 255), width=3)
        draw.line([cx + bx, cy - 14, cx + bx, cy + 20], fill=(255, 240, 150, 255), width=1)
    draw.rectangle([cx - 8, cy - 2, cx + 8, cy + 12], fill=(245, 195, 30, 255), outline=(130, 90, 10, 255))
    draw.ellipse([cx - 3, cy + 2, cx + 3, cy + 8], fill=(40, 20, 10, 255))

    draw.line([cx - 28, cy + 2, cx - 12, cy + 16], fill=(245, 205, 50, 255), width=2)
    draw.line([cx + 12, cy + 16, cx + 28, cy + 2], fill=(245, 205, 50, 255), width=2)

    draw.line([cx, cy - 32, cx, cy + 2], fill=(190, 245, 255, 255), width=3)
    draw.line([cx, cy - 32, cx, cy + 2], fill=(255, 255, 255, 255), width=1)
    draw.rectangle([cx - 9, cy - 24, cx + 9, cy - 20], fill=(245, 195, 30, 255), outline=(140, 95, 10, 255))
    draw.point((cx, cy - 34), fill=(245, 195, 30, 255))

    spirits = [
        [(cx + 38, cy - 42), (cx + 24, cy - 25), (cx + 14, cy - 8), (cx + 2, cy - 2)],
        [(cx - 15, cy - 46), (cx - 6, cy - 28), (cx - 2, cy - 10)],
        [(cx + 48, cy - 18), (cx + 32, cy - 6), (cx + 12, cy + 2)]
    ]
    for s_pts in spirits:
        draw.line(s_pts, fill=(120, 35, 150, 200), width=4)
        draw.line(s_pts, fill=(185, 80, 230, 240), width=2)
        draw.point((s_pts[0][0] - 2, s_pts[0][1]), fill=(255, 40, 40, 255))
        draw.point((s_pts[0][0] + 2, s_pts[0][1]), fill=(255, 40, 40, 255))

    draw.rectangle([hx - 12, hy + 35, hx - 5, hy + 40], fill=(120, 60, 20, 255))
    draw.rectangle([hx + 4, hy + 35, hx + 11, hy + 40], fill=(120, 60, 20, 255))
    draw.line([hx - 8, hy + 20, hx - 9, hy + 36], fill=(235, 230, 220, 255), width=4)
    draw.line([hx + 7, hy + 20, hx + 8, hy + 36], fill=(235, 230, 220, 255), width=4)

    draw.polygon([(hx - 14, hy + 22), (hx + 14, hy + 22), (hx + 8, hy - 6), (hx - 8, hy - 6)], fill=(28, 140, 58, 255), outline=(15, 75, 30, 255))
    draw.rectangle([hx - 11, hy + 12, hx + 11, hy + 16], fill=(95, 45, 15, 255))
    draw.rectangle([hx - 3, hy + 11, hx + 3, hy + 17], fill=(245, 200, 40, 255))

    draw.ellipse([hx - 7, hy - 14, hx + 7, hy - 2], fill=(255, 225, 190, 255), outline=(50, 35, 25, 255))
    draw.line([hx - 6, hy - 10, hx - 6, hy - 4], fill=(245, 195, 45, 255), width=2)
    draw.polygon([(hx - 8, hy - 12), (hx + 8, hy - 12), (hx - 18, hy - 22)], fill=(28, 140, 58, 255), outline=(15, 75, 30, 255))

    draw.line([hx + 8, hy - 2, cx - 10, cy - 22], fill=(255, 225, 190, 255), width=4)
    draw.line([hx + 8, hy - 2, cx - 10, cy - 22], fill=(255, 255, 255, 255), width=1)
    draw.polygon([(hx - 16, hy - 2), (hx - 8, hy - 12), (hx - 8, hy + 16), (hx - 16, hy + 8)], fill=(35, 85, 175, 255), outline=(195, 195, 205, 255))

    create_stained_glass_border(draw, w, h, w - 16, h - 16)
    return img

def render_panel_3_centennial_festival():
    w, h = 240, 160
    img = Image.new('RGBA', (w, h), (215, 235, 250, 255))
    draw = ImageDraw.Draw(img)

    cast_x = w // 2 - 15
    for y in range(75):
        fac = y / 75.0
        r = int(120 + fac * 90)
        g = int(185 + fac * 50)
        b = int(248 - fac * 15)
        draw.line([0, y, w, y], fill=(r, g, b, 255))

    draw.rectangle([cast_x - 48, 38, cast_x + 48, 88], fill=(228, 232, 240, 255), outline=(145, 155, 170, 255))
    for sy in [48, 58, 68, 78]:
        draw.line([cast_x - 48, sy, cast_x + 48, sy], fill=(200, 208, 220, 255))
    for bx in range(cast_x - 48, cast_x + 44, 12):
        draw.rectangle([bx, 32, bx + 6, 38], fill=(228, 232, 240, 255), outline=(145, 155, 170, 255))

    draw.polygon([(cast_x - 56, 38), (cast_x - 36, 38), (cast_x - 46, 12)], fill=(38, 92, 178, 255), outline=(18, 48, 105, 255))
    draw.polygon([(cast_x + 36, 38), (cast_x + 56, 38), (cast_x + 46, 12)], fill=(38, 92, 178, 255), outline=(18, 48, 105, 255))
    draw.polygon([(cast_x - 16, 34), (cast_x + 16, 34), (cast_x, 4)], fill=(32, 80, 160, 255), outline=(18, 48, 105, 255))
    draw.line([cast_x, 4, cast_x, -4], fill=(245, 205, 35, 255), width=2)
    draw.polygon([(cast_x, -4), (cast_x + 16, 0), (cast_x, 4)], fill=(225, 45, 45, 255))

    draw.polygon([(0, 160), (0, 88), (65, 82), (140, 88), (200, 80), (240, 85), (240, 160)], fill=(58, 165, 68, 255))
    flowers = [
        (35, 105, (245, 60, 60)), (55, 112, (255, 240, 80)), (75, 102, (255, 255, 255)),
        (165, 108, (245, 60, 60)), (185, 100, (255, 240, 80)), (210, 110, (255, 255, 255))
    ]
    for fx, fy, fcol in flowers:
        draw.ellipse([fx - 3, fy - 3, fx + 3, fy + 3], fill=fcol)
        draw.point((fx, fy), fill=(255, 215, 0))

    buntings = [
        ((12, 58), (55, 68), (235, 55, 55)),
        ((55, 68), (105, 60), (245, 215, 40)),
        ((105, 60), (155, 68), (45, 130, 245)),
        ((155, 68), (198, 60), (50, 185, 75)),
        ((198, 60), (232, 64), (235, 55, 55))
    ]
    for p1, p2, pcol in buntings:
        draw.line([p1, p2], fill=(85, 65, 45, 255), width=1)
        mx = (p1[0] + p2[0]) // 2
        my = (p1[1] + p2[1]) // 2
        draw.polygon([(mx - 5, my), (mx + 5, my), (mx, my + 8)], fill=pcol)

    confetti = [
        (45, 42, (245, 50, 50)), (85, 35, (255, 220, 40)), (130, 44, (50, 150, 255)),
        (175, 38, (60, 210, 80)), (210, 48, (240, 80, 200)), (150, 52, (255, 160, 30))
    ]
    for kx, ky, kcol in confetti:
        draw.rectangle([kx, ky, kx + 2, ky + 2], fill=kcol)

    zx, zy = w - 55, 102
    for r in range(32, 8, -4):
        draw.ellipse([zx - r, zy - r, zx + r, zy + r], fill=(255, 245, 140, 35))

    draw.polygon([(zx - 12, zy + 38), (zx + 12, zy + 38), (zx + 5, zy + 8), (zx - 5, zy + 8)], fill=(250, 248, 252, 255), outline=(180, 170, 190, 255))
    draw.polygon([(zx - 5, zy + 12), (zx + 5, zy + 12), (zx + 4, zy + 38), (zx - 4, zy + 38)], fill=(155, 48, 145, 255))
    draw.rectangle([zx - 6, zy + 8, zx + 6, zy + 12], fill=(245, 200, 40, 255))
    draw.polygon([(zx, zy + 10), (zx - 3, zy + 15), (zx + 3, zy + 15)], fill=(255, 230, 80, 255))

    draw.ellipse([zx - 6, zy - 4, zx + 6, zy + 7], fill=(255, 225, 195, 255), outline=(60, 40, 30, 255))
    draw.polygon([(zx - 8, zy - 2), (zx - 10, zy + 18), (zx - 5, zy + 18), (zx - 5, zy + 4)], fill=(245, 198, 55, 255))
    draw.polygon([(zx + 8, zy - 2), (zx + 10, zy + 18), (zx + 5, zy + 18), (zx + 5, zy + 4)], fill=(245, 198, 55, 255))
    draw.polygon([(zx - 6, zy - 3), (zx + 6, zy - 3), (zx + 4, zy - 7), (zx, zy - 9), (zx - 4, zy - 7)], fill=(245, 205, 40, 255))
    draw.point((zx, zy - 6), fill=(225, 35, 35, 255))

    draw.ellipse([zx - 3, zy + 8, zx + 3, zy + 13], fill=(255, 235, 205, 255))
    draw.point((zx, zy + 6), fill=(255, 255, 255, 255))

    crowd_x = 75
    draw.polygon([(crowd_x - 22, 142), (crowd_x - 12, 142), (crowd_x - 14, 118), (crowd_x - 20, 118)], fill=(45, 100, 185, 255))
    draw.ellipse([crowd_x - 20, 110, crowd_x - 14, 118], fill=(255, 220, 185, 255))
    draw.polygon([(crowd_x - 6, 140), (crowd_x + 4, 140), (crowd_x + 2, 116), (crowd_x - 4, 116)], fill=(210, 50, 50, 255))
    draw.ellipse([crowd_x - 4, 108, crowd_x + 2, 116], fill=(255, 220, 185, 255))
    draw.line([crowd_x + 2, 118, crowd_x + 8, 108], fill=(255, 220, 185, 255), width=2)
    gx = crowd_x + 22
    draw.polygon([(gx - 5, 142), (gx + 5, 142), (gx + 4, 115), (gx - 4, 115)], fill=(120, 125, 135, 255))
    draw.ellipse([gx - 4, 106, gx + 4, 114], fill=(185, 190, 200, 255))
    draw.line([gx + 6, 92, gx + 6, 145], fill=(95, 70, 40, 255), width=2)
    draw.polygon([(gx + 4, 92), (gx + 8, 92), (gx + 6, 82)], fill=(230, 235, 245, 255))

    create_stained_glass_border(draw, w, h, w - 16, h - 16)
    return img

# ============================================================================
# 2. PÁINEIS DE VITRAL DO PRÓLOGO NARRATIVO (PROLOGUE PANELS - 480x320)
# ============================================================================
def render_master_prologue_panels():
    p0 = render_panel_0_darkness()
    p1 = render_panel_1_descent()
    p2 = render_panel_2_hero_and_chest()
    p3 = render_panel_3_centennial_festival()

    master = Image.new('RGBA', (480, 320))
    master.paste(p0, (0, 0))
    master.paste(p1, (240, 0))
    master.paste(p2, (0, 160))
    master.paste(p3, (240, 160))
    return master

def main():
    print("====================================================================")
    print("   Gerador de Assets Autênticos de Abertura (Minish Cap GBA)        ")
    print("====================================================================")

    # 1. Pátio do Castelo de Hyrule (512x384)
    print("\n[1/2] Gerando Pátio Autêntico do Castelo de Hyrule (512x384)...")
    courtyard_img, cw, ch, col_bytes = render_authentic_castle_courtyard()
    save_bmp_32('assets/regions/map_castle_courtyard_master.bmp', courtyard_img)
    save_binary_collision('assets/regions/map_castle_courtyard_collision_master.bin', cw, ch, col_bytes)

    for reg in ['usa', 'eur', 'jpn']:
        save_bmp_32(f'assets/regions/{reg}/map_castle_courtyard.bmp', courtyard_img)
        save_binary_collision(f'assets/regions/{reg}/map_castle_courtyard_collision.bin', cw, ch, col_bytes)

    # 2. Painéis do Prólogo dos Picori em Vitral (480x320)
    print("\n[2/2] Gerando Painéis de Vitral do Prólogo dos Picori (480x320)...")
    prologue_master = render_master_prologue_panels()
    save_bmp_32('assets/regions/prologue_panels_master.bmp', prologue_master)
    save_bmp_32('assets/ui/prologue/prologue_panels.bmp', prologue_master)
    for reg in ['usa', 'eur', 'jpn']:
        save_bmp_32(f'assets/regions/{reg}/prologue_panels.bmp', prologue_master)

    print("\n>>> Todos os assets autênticos de abertura foram gerados com sucesso!")

if __name__ == '__main__':
    main()
