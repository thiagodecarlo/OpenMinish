#!/usr/bin/env python3
"""
scratch/verify_palace_of_winds.py
Generates palace_of_winds_showcase.png showcasing:
1. Panel 1: Vestíbulo dos Céus & Turbinas Eólicas (Chamber 0: Wind Turbine, Pinwheel Switch, Portcullis)
2. Panel 2: Abismos Celestiais & Travessia com Roc's Cape (Chamber 1: Bottomless Sky Pit, Roc's Cape Leaping/Gliding, Small Key Chest)
3. Panel 3: Salão dos Interruptores Four Sword & Torre dos Wizzrobes (Chambers 2 & 3: 3-Clone Switches, Wizzrobe Sorcerers, Big Key Chest)
4. Panel 4: Arena Aérea dos Céus: Chefe GYORG PAIR & Elemento do Vento (Chamber 5: Giant Blue/Red Gyorg Boss Fight, Heart Container & Wind Element)
"""

import math
import os
import shutil
from PIL import Image, ImageDraw, ImageFont

W, H = 480, 320
IMG_W, IMG_H = W * 2, H * 2
canvas = Image.new("RGBA", (IMG_W, IMG_H), (15, 23, 42, 255))
draw = ImageDraw.Draw(canvas)

try:
    font_title = ImageFont.truetype("arial.ttf", 15)
    font_body = ImageFont.truetype("arial.ttf", 12)
    font_small = ImageFont.truetype("arial.ttf", 11)
except:
    font_title = font_body = font_small = ImageFont.load_default()

def draw_hud(pdraw, ox, oy, title, status_text, keys=1, boss_key=False, wind_element=False):
    # Top HUD
    pdraw.rectangle([ox, oy, ox + W, oy + 24], fill=(15, 23, 42, 255))
    # Hearts (8 hearts)
    for i in range(8):
        hx = ox + 8 + i * 13
        hy = oy + 6
        pdraw.polygon([(hx+4, hy+1), (hx+1, hy+4), (hx+1, hy+6), (hx+4, hy+9),
                       (hx+7, hy+6), (hx+7, hy+4)], fill=(239, 68, 68, 255))
        pdraw.polygon([(hx+7, hy+6), (hx+10, hy+4), (hx+10, hy+1), (hx+7, hy+1)], fill=(239, 68, 68, 255))
    # Rupees
    rx = ox + 120
    pdraw.polygon([(rx+4, oy+4), (rx+8, oy+9), (rx+4, oy+15), (rx, oy+9)], fill=(56, 189, 248, 255))
    pdraw.text((rx + 12, oy + 4), "480", fill=(255, 255, 255, 255), font=font_small)

    # Subweapon Slot B (Roc's Cape)
    bx = ox + 175
    pdraw.rectangle([bx, oy + 3, bx + 18, oy + 21], fill=(127, 29, 29, 255), outline=(239, 68, 68, 255))
    pdraw.text((bx + 2, oy + 4), "B", fill=(253, 224, 71, 255), font=font_small)
    # Wing icon
    pdraw.polygon([(bx+8, oy+16), (bx+12, oy+8), (bx+16, oy+14)], fill=(254, 226, 226, 255))

    # Small keys
    kx = ox + 205
    if keys > 0:
        pdraw.rectangle([kx, oy + 7, kx + 8, oy + 9], fill=(245, 158, 11, 255))
        pdraw.rectangle([kx + 6, oy + 9, kx + 8, oy + 13], fill=(245, 158, 11, 255))
        pdraw.text((kx + 11, oy + 4), f"x{keys}", fill=(254, 240, 138, 255), font=font_small)

    # Boss key
    if boss_key:
        bkx = ox + 235
        pdraw.rectangle([bkx, oy + 6, bkx + 9, oy + 15], fill=(217, 119, 6, 255), outline=(254, 240, 138, 255))
        pdraw.ellipse([bkx + 3, oy + 8, bkx + 6, oy + 11], fill=(239, 68, 68, 255))

    # Elements collected (Earth, Fire, Water, Wind)
    ex = ox + 270
    pdraw.rectangle([ex, oy + 6, ex + 6, oy + 12], fill=(34, 197, 94, 255))       # Earth
    pdraw.rectangle([ex + 8, oy + 6, ex + 14, oy + 12], fill=(239, 68, 68, 255))   # Fire
    pdraw.rectangle([ex + 16, oy + 6, ex + 22, oy + 12], fill=(56, 189, 248, 255)) # Water
    if wind_element:
        pdraw.rectangle([ex + 24, oy + 6, ex + 30, oy + 12], fill=(16, 185, 129, 255), outline=(254, 240, 138, 255)) # Wind!

    # Title & Subtitle banner at bottom
    pdraw.rectangle([ox, oy + H - 36, ox + W, oy + H], fill=(15, 23, 42, 240))
    pdraw.text((ox + 10, oy + H - 32), title, fill=(253, 224, 71, 255), font=font_title)
    pdraw.text((ox + 10, oy + H - 16), status_text, fill=(226, 232, 240, 255), font=font_body)

def draw_palace_floor(pdraw, x1, y1, x2, y2):
    pdraw.rectangle([x1, y1, x2, y2], fill=(241, 245, 249, 255), outline=(203, 213, 225, 255))
    # Motivo de nuvens / vento no mármore
    for y in range(y1 + 16, y2, 32):
        for x in range(x1 + 16, x2, 32):
            pdraw.ellipse([x - 6, y - 6, x + 6, y + 6], outline=(186, 230, 253, 160))
            pdraw.point((x, y), fill=(251, 191, 36, 255))

def draw_palace_wall(pdraw, x1, y1, x2, y2):
    pdraw.rectangle([x1, y1, x2, y2], fill=(30, 41, 59, 255))
    # Moldura dourada
    pdraw.rectangle([x1, y1, x2, y1 + 4], fill=(251, 191, 36, 255))
    pdraw.rectangle([x1, y1 + 4, x2, y1 + 6], fill=(217, 119, 6, 255))

def draw_sky_abyss(pdraw, x1, y1, x2, y2):
    pdraw.rectangle([x1, y1, x2, y2], fill=(2, 132, 199, 255))
    # Nuvens flutuando
    for cy in range(y1 + 10, y2, 24):
        for cx in range(x1 + 10, x2, 48):
            pdraw.ellipse([cx, cy, cx + 36, cy + 16], fill=(255, 255, 255, 120))
            pdraw.ellipse([cx + 10, cy - 4, cx + 32, cy + 14], fill=(255, 255, 255, 140))

def draw_palace_grate(pdraw, x1, y1, x2, y2):
    draw_sky_abyss(pdraw, x1, y1, x2, y2)
    for x in range(x1, x2, 8):
        pdraw.line([(x, y1), (x, y2)], fill=(71, 85, 105, 255), width=2)
    for y in range(y1, y2, 8):
        pdraw.line([(x1, y), (x2, y)], fill=(71, 85, 105, 255), width=2)

def draw_link_sprite(pdraw, lx, ly, has_cape=False, is_jumping=False):
    # Sombra
    shadow_w = 16 if not is_jumping else 10
    sy = ly + 14 if not is_jumping else ly + 28
    pdraw.ellipse([lx - shadow_w//2, sy - 2, lx + shadow_w//2, sy + 4], fill=(15, 23, 42, 120))

    # Capa de Roc (se equipada e saltando/planando)
    if has_cape:
        cape_y = ly - 2
        pdraw.polygon([(lx - 12, cape_y + 16), (lx, cape_y - 2), (lx + 12, cape_y + 16),
                       (lx + 6, cape_y + 20), (lx - 6, cape_y + 20)], fill=(220, 38, 38, 255))
        pdraw.ellipse([lx - 3, cape_y - 2, lx + 3, cape_y + 4], fill=(251, 191, 36, 255)) # Broche de ouro

    # Túnica verde clássica
    pdraw.rectangle([lx - 6, ly + 2, lx + 6, ly + 14], fill=(34, 197, 94, 255))
    # Cinto marrom
    pdraw.rectangle([lx - 6, ly + 8, lx + 6, ly + 10], fill=(120, 53, 15, 255))
    # Rosto
    pdraw.rectangle([lx - 4, ly - 4, lx + 4, ly + 2], fill=(254, 215, 170, 255))
    # Gorro verde Minish
    pdraw.polygon([(lx - 6, ly - 3), (lx, ly - 12), (lx + 6, ly - 3)], fill=(22, 163, 74, 255))
    pdraw.ellipse([lx - 1, ly - 13, lx + 1, ly - 11], fill=(253, 224, 71, 255))
    # Botas
    pdraw.rectangle([lx - 5, ly + 14, lx - 1, ly + 18], fill=(146, 64, 14, 255))
    pdraw.rectangle([lx + 1, ly + 14, lx + 5, ly + 18], fill=(146, 64, 14, 255))

# =============================================================================
# PAINEL 1: SALA 0 - VESTÍBULO DOS CÉUS & TURBINAS EÓLICAS
# =============================================================================
def render_panel_1(ox, oy):
    # Fundo de paredes celestes
    draw_palace_wall(draw, ox, oy + 24, ox + W, oy + H - 36)
    # Piso central de mármore
    draw_palace_floor(draw, ox + 32, oy + 56, ox + W - 32, oy + H - 56)
    # Abismos celestes laterais
    draw_sky_abyss(draw, ox + 32, oy + 56, ox + 80, oy + H - 56)
    draw_sky_abyss(draw, ox + W - 80, oy + 56, ox + W - 32, oy + H - 56)
    # Passarelas de grades
    draw_palace_grate(draw, ox + 80, oy + 120, ox + 140, oy + 180)
    draw_palace_grate(draw, ox + W - 140, oy + 120, ox + W - 80, oy + 180)

    # Turbina eólica ancestral (Palace Fan)
    fx, fy = ox + W // 2, oy + 110
    draw.ellipse([fx - 24, fy - 24, fx + 24, fy + 24], fill=(30, 41, 59, 255), outline=(180, 83, 9, 255), width=3)
    # Pás giratórias
    for ang in [0, math.pi/2, math.pi, 3*math.pi/2]:
        px = fx + int(math.cos(ang + 0.3) * 18)
        py = fy + int(math.sin(ang + 0.3) * 18)
        draw.line([(fx, fy), (px, py)], fill=(251, 191, 36, 255), width=4)
    # Vórtice de ar ascendente
    for i in range(12):
        wy = fy - 10 - i * 6
        wx = fx + int(math.sin(i * 0.8) * 14)
        draw.line([(wx - 8, wy), (wx + 8, wy)], fill=(186, 230, 253, 180), width=2)

    # Cata-vento ativador (Pinwheel Switch)
    px, py = ox + 150, oy + 150
    draw.rectangle([px - 2, py, px + 2, py + 20], fill=(148, 163, 184, 255))
    draw.ellipse([px - 10, py - 10, px + 10, py + 10], fill=(34, 197, 94, 255), outline=(254, 240, 138, 255), width=2)
    draw.text((px - 25, py + 22), "CATA-VENTO ATIVO", fill=(134, 239, 172, 255), font=font_small)

    # Portão Norte Aberto
    gx1, gx2 = ox + W // 2 - 24, ox + W // 2 + 24
    draw.rectangle([gx1, oy + 32, gx2, oy + 56], fill=(15, 23, 42, 255), outline=(251, 191, 36, 255))
    draw.text((gx1 - 10, oy + 40), "ACESSO ABERTO", fill=(254, 240, 138, 255), font=font_small)

    # Link caminhando no mármore
    draw_link_sprite(draw, ox + W // 2, oy + 190, has_cape=True, is_jumping=False)

    draw_hud(draw, ox, oy, "1. SALA 0: VESTIBULO DOS CEUS & TURBINAS",
             "Turbinas eolicas ancestrais e ativacao de cata-vento para destrancar portoes celestiais.",
             keys=0, boss_key=False, wind_element=False)

# =============================================================================
# PAINEL 2: SALA 1 - ABISMOS CELESTIAIS & TRAVESSIA COM ROC'S CAPE
# =============================================================================
def render_panel_2(ox, oy):
    # Paredes celestes de contorno
    draw_palace_wall(draw, ox, oy + 24, ox + W, oy + H - 36)
    # Grande Abismo Celestial
    draw_sky_abyss(draw, ox + 24, oy + 48, ox + W - 24, oy + H - 48)

    # Ilhas flutuantes separadas
    # Ilha 1 (Entrada Sul)
    draw_palace_floor(draw, ox + W // 2 - 40, oy + H - 90, ox + W // 2 + 40, oy + H - 48)
    # Ilha 2 (Centro - Plataforma de Pouso)
    draw_palace_floor(draw, ox + W // 2 - 30, oy + 120, ox + W // 2 + 30, oy + 170)
    # Ilha 3 (Esquerda - Baú da Chave)
    draw_palace_floor(draw, ox + 60, oy + 100, ox + 130, oy + 160)
    # Ilha 4 (Direita - Porta Trancada)
    draw_palace_floor(draw, ox + W - 120, oy + 110, ox + W - 40, oy + 170)

    # Baú dourado na Ilha 3
    bx, by = ox + 95, oy + 130
    draw.rectangle([bx - 10, by - 8, bx + 10, by + 8], fill=(245, 158, 11, 255), outline=(254, 240, 138, 255), width=2)
    draw.rectangle([bx - 2, by - 2, bx + 2, by + 2], fill=(255, 255, 255, 255))
    draw.text((bx - 28, by + 12), "BAU DA CHAVE #1", fill=(253, 224, 71, 255), font=font_small)

    # Porta trancada com fechadura de ferro na Ilha 4
    dx, dy = ox + W - 48, oy + 140
    draw.rectangle([dx - 8, dy - 16, dx + 8, dy + 16], fill=(30, 41, 59, 255), outline=(245, 158, 11, 255), width=2)
    draw.ellipse([dx - 3, dy - 4, dx + 3, dy + 2], fill=(245, 158, 11, 255))

    # Link em pleno ar saltando e planando com Roc's Cape sobre o abismo!
    lx, ly = ox + 175, oy + 125
    draw_link_sprite(draw, lx, ly, has_cape=True, is_jumping=True)
    # Rastro aerodinâmico de vento
    for t in range(4):
        draw.line([(lx - 25 - t*10, ly + 8 + t*2), (lx - 12 - t*10, ly + 8 + t*2)], fill=(255, 255, 255, 180 - t*35), width=2)
    draw.text((lx - 40, ly - 28), "SALTO & PLANEIO NO AR!", fill=(253, 224, 71, 255), font=font_small)

    draw_hud(draw, ox, oy, "2. SALA 1: ABISMOS CELESTIAIS & ROC'S CAPE",
             "Fisica de salto acrobatico e planeio sobre o vacuo com a Capa de Roc para coletar a Chave #1.",
             keys=1, boss_key=False, wind_element=False)

# =============================================================================
# PAINEL 3: SALAS 2 & 3 - INTERRUPTORES FOUR SWORD & TORRE WIZZROBES
# =============================================================================
def render_panel_3(ox, oy):
    # Parede e piso
    draw_palace_wall(draw, ox, oy + 24, ox + W, oy + H - 36)
    draw_palace_floor(draw, ox + 32, oy + 56, ox + W - 32, oy + H - 56)

    # 3 Interruptores Four Sword
    sw_x = [ox + 100, ox + 160, ox + 220]
    sw_y = oy + 120
    for i, sx in enumerate(sw_x):
        draw.rectangle([sx - 12, sw_y - 12, sx + 12, sw_y + 12], fill=(71, 85, 105, 255), outline=(34, 197, 94, 255), width=2)
        draw.ellipse([sx - 6, sw_y - 6, sx + 6, sw_y + 6], fill=(34, 197, 94, 255))
        # Clone sobre o interruptor
        draw_link_sprite(draw, sx, sw_y, has_cape=False, is_jumping=False)
    draw.text((ox + 90, sw_y + 18), "DIVISAO EM 3 CLONES (FOUR SWORD)", fill=(134, 239, 172, 255), font=font_small)

    # Divisória de grade entre Sala 2 e Sala 3
    draw.line([(ox + 270, oy + 56), (ox + 270, oy + H - 56)], fill=(251, 191, 36, 255), width=3)

    # Câmara dos Wizzrobes e Grande Baú da Boss Key
    wx, wy = ox + 370, oy + 110
    # Wizzrobe do Vento flutuando
    draw.rectangle([wx - 8, wy - 16, wx + 8, wy + 16], fill=(2, 132, 199, 255))
    draw.polygon([(wx - 8, wy - 16), (wx, wy - 24), (wx + 8, wy - 16)], fill=(251, 191, 36, 255)) # Chapéu cônico
    draw.point((wx - 3, wy - 6), fill=(253, 224, 71, 255))
    draw.point((wx + 3, wy - 6), fill=(253, 224, 71, 255))
    draw.text((wx - 30, wy + 20), "WIZZROBE CELESTE", fill=(186, 230, 253, 255), font=font_small)

    # Grande Baú da Boss Key (Big Key)
    bx, by = ox + 370, oy + 200
    draw.rectangle([bx - 14, by - 12, bx + 14, by + 12], fill=(245, 158, 11, 255), outline=(254, 240, 138, 255), width=3)
    draw.rectangle([bx - 4, by - 4, bx + 4, by + 4], fill=(239, 68, 68, 255)) # Grande Joia Rubelita
    draw.text((bx - 42, by + 16), "GRANDE BAU: BOSS KEY", fill=(253, 224, 71, 255), font=font_small)

    draw_hud(draw, ox, oy, "3. SALAS 2 & 3: ENIGMA FOUR SWORD & WIZZROBES",
             "Pisos de clones acionados simultaneamente e conquista da monumental Chave do Mestre (Boss Key).",
             keys=0, boss_key=True, wind_element=False)

# =============================================================================
# PAINEL 4: SALA 5 - CHEFE GYORG PAIR & SAGRADO ELEMENTO DO VENTO
# =============================================================================
def render_panel_4(ox, oy):
    # Vácuo da estratosfera e nuvens cúmulos animadas
    draw_sky_abyss(draw, ox, oy + 24, ox + W, oy + H - 36)

    # 1. GRANDE ARRAIA AZUL (Fêmea) - Central
    bx, by = ox + 180, oy + 120
    # Asas gigantes
    draw.ellipse([bx - 80, by - 36, bx + 80, by + 36], fill=(37, 99, 235, 255), outline=(96, 165, 250, 255), width=3)
    # 3 Olhos na Arraia Azul
    eye_x = [bx - 40, bx, bx + 40]
    for ex in eye_x:
        draw.ellipse([ex - 7, by - 7, ex + 7, by + 7], fill=(251, 191, 36, 255), outline=(153, 27, 27, 255), width=2)
        draw.ellipse([ex - 3, by - 3, ex + 3, by + 3], fill=(153, 27, 27, 255))
    draw.text((bx - 45, by + 42), "ARRAIA AZUL (FEMEA)", fill=(147, 197, 253, 255), font=font_small)

    # Link montado no dorso atacando os olhos
    draw_link_sprite(draw, bx, by + 15, has_cape=True, is_jumping=False)

    # 2. ARRAIA VERMELHA (Macho) - Rápido voando ao redor
    rx, ry = ox + 360, oy + 80
    draw.ellipse([rx - 44, ry - 22, rx + 44, ry + 22], fill=(220, 38, 38, 255), outline=(248, 113, 113, 255), width=2)
    # Olho central
    draw.ellipse([rx - 6, ry - 6, rx + 6, ry + 6], fill=(251, 191, 36, 255), outline=(153, 27, 27, 255), width=2)
    # Cauda chicoteante
    draw.line([(rx + 40, ry), (rx + 75, ry - 18)], fill=(127, 29, 29, 255), width=3)
    draw.text((rx - 45, ry + 26), "ARRAIA VERMELHA (MACHO)", fill=(252, 165, 165, 255), font=font_small)

    # Esferas de energia de vento
    for bx_pos in [(rx - 60, ry + 20), (rx - 100, ry + 40)]:
        draw.ellipse([bx_pos[0] - 5, bx_pos[1] - 5, bx_pos[0] + 5, bx_pos[1] + 5], fill=(56, 189, 248, 255), outline=(255, 255, 255, 255))

    # 3. RECOMPENSAS SAGRADAS PÓS-VITÓRIA
    # Recipiente de Coração permanente
    hx, hy = ox + 320, oy + 190
    draw.polygon([(hx+8, hy+2), (hx+2, hy+8), (hx+2, hy+12), (hx+8, hy+18),
                   (hx+14, hy+12), (hx+14, hy+8)], fill=(239, 68, 68, 255), outline=(254, 202, 202, 255))
    draw.polygon([(hx+14, hy+12), (hx+20, hy+8), (hx+20, hy+2), (hx+14, hy+2)], fill=(239, 68, 68, 255), outline=(254, 202, 202, 255))
    draw.text((hx - 36, hy + 22), "HEART CONTAINER", fill=(252, 165, 165, 255), font=font_small)

    # SAGRADO ELEMENTO DO VENTO (Wind Element)
    wx, wy = ox + 410, oy + 190
    draw.ellipse([wx - 14, wy - 14, wx + 14, wy + 14], fill=(253, 224, 71, 90)) # Aura de vento
    draw.polygon([(wx, wy - 10), (wx + 10, wy), (wx, wy + 10), (wx - 10, wy)], fill=(16, 185, 129, 255), outline=(110, 231, 183, 255), width=2)
    draw.ellipse([wx - 2, wy - 2, wx + 2, wy + 2], fill=(255, 255, 255, 255))
    draw.text((wx - 48, wy + 22), "ELEMENTO DO VENTO!", fill=(110, 231, 183, 255), font=font_small)

    draw_hud(draw, ox, oy, "4. SALA 5: ARENA DOS CEUS - GYORG PAIR",
             "Batalha aerea epica pulando entre as arraias com Roc's Cape! Recompensa: O Sagrado Elemento do Vento!",
             keys=0, boss_key=True, wind_element=True)

# Renderiza os 4 painéis
render_panel_1(0, 0)
render_panel_2(W, 0)
render_panel_3(0, H)
render_panel_4(W, H)

# Salva nos destinos obrigatórios
os.makedirs("scratch", exist_ok=True)
output_local = "scratch/palace_of_winds_showcase.png"
canvas.save(output_local, "PNG")

conv_dir = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a"
os.makedirs(conv_dir, exist_ok=True)
output_brain = os.path.join(conv_dir, "palace_of_winds_showcase.png")
canvas.save(output_brain, "PNG")

print(f"[OK] Showcase gerado com sucesso em '{output_local}' e '{output_brain}'!")
