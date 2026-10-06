#!/usr/bin/env python3
"""
scratch/verify_overworld_and_hud.py - Verification and Showcase for Overworld Tilesets & Authentic GBA HUD
"""

import os
from PIL import Image, ImageDraw, ImageFont

def main():
    print("[VERIFY] Carregando assets de Overworld e HUD...")
    tileset_path = "assets/regions/tileset_overworld_master.bmp"
    hud_path     = "assets/regions/hud_items_master.bmp"
    town_path    = "assets/regions/usa/map_town.bmp"
    crenel_path  = "assets/regions/usa/map_crenel.bmp"
    castor_path  = "assets/regions/usa/map_castor.bmp"
    castle_path  = "assets/regions/usa/map_castle_courtyard.bmp"

    tileset = Image.open(tileset_path).convert("RGBA")
    hud     = Image.open(hud_path).convert("RGBA")
    map_town   = Image.open(town_path).convert("RGBA")
    map_crenel = Image.open(crenel_path).convert("RGBA")
    map_castor = Image.open(castor_path).convert("RGBA")
    map_castle = Image.open(castle_path).convert("RGBA")

    print(f"  Tileset: {tileset.size}")
    print(f"  HUD:     {hud.size}")
    print(f"  Town:    {map_town.size}")
    print(f"  Crenel:  {map_crenel.size}")
    print(f"  Castor:  {map_castor.size}")
    print(f"  Castle:  {map_castle.size}")

    # Criando o painel de montagem do showcase
    # Layout:
    # Top banner
    # Section 1: Showcase dos 4 Mapas Regionais Pré-renderizados (576x448, 512x384 redimensionados em escala limpa)
    # Section 2: Amostra detalhada dos metatiles 16x16 em zoom 3x
    # Section 3: HUD e Barra de Magia em zoom 3x (Corações 1/4, 2/4, 3/4, 4/4, Vazio, Magic Meter, Badges [A]/[B])
    # Section 4: 2 Simulações de Tela em Formato GBA Widescreen (284x160 a 2x = 568x320)

    canvas_w = 1200
    canvas_h = 1380
    showcase = Image.new("RGBA", (canvas_w, canvas_h), (18, 22, 28, 255))
    draw = ImageDraw.Draw(showcase)

    # Título do Showcase
    draw.rectangle([0, 0, canvas_w, 48], fill=(12, 16, 22, 255))
    draw.line([0, 48, canvas_w, 48], fill=(74, 222, 128, 255), width=2)
    draw.text((24, 14), "OPENMINISH - PIPELINE GBA OVERWORLD & SISTEMA DE HUD E MOLDURAS AUTENTICAS", fill=(240, 246, 252, 255))

    # --- SEÇÃO 1: OS 4 MAPAS REGIONAIS ---
    draw.text((24, 60), "1. CENARIOS OVERWORLD REGIONAIS PRE-RENDERIZADOS A 60 FPS", fill=(250, 204, 21, 255))
    
    # 4 quadros de mapas: Town, Crenel, Castor, Castle Courtyard
    # Miniaturas com proporção adequada
    thumb_w, thumb_h = 270, 210
    
    maps_info = [
        ("Hyrule Town Hub (36x28 Tiles)", map_town),
        ("Mount Crenel Base (32x24 Tiles)", map_crenel),
        ("Castor Wilds Swamp (36x28 Tiles)", map_castor),
        ("Castle Courtyard (32x24 Tiles)", map_castle)
    ]
    
    for idx, (title, m_img) in enumerate(maps_info):
        mx = 24 + idx * 290
        my = 85
        thumb = m_img.resize((thumb_w, thumb_h), Image.Resampling.NEAREST)
        draw.rectangle([mx - 2, my - 2, mx + thumb_w + 1, my + thumb_h + 1], outline=(56, 189, 248, 255), width=2)
        showcase.paste(thumb, (mx, my))
        draw.rectangle([mx, my + thumb_h - 22, mx + thumb_w, my + thumb_h], fill=(10, 14, 20, 220))
        draw.text((mx + 6, my + thumb_h - 18), title, fill=(240, 246, 252, 255))

    # --- SEÇÃO 2: METATILES DE OVERWORLD (ZOOM 3X) ---
    draw.text((24, 320), "2. ATLAS DE METATILES 16x16 DO OVERWORLD GBA (ZOOM 3X)", fill=(250, 204, 21, 255))
    
    tile_categories = [
        ("Base Hyrule", [(0, 0, "Grama"), (1, 0, "Trilha"), (2, 0, "Agua"), (3, 0, "Muro"), (4, 0, "Arbusto"), (5, 0, "Rosa"), (6, 0, "Margarida")]),
        ("Hyrule Town", [(0, 1, "Calcamento"), (1, 1, "Parede"), (2, 1, "Telha Verm."), (3, 1, "Telha Azul"), (4, 1, "Porta"), (5, 1, "Janela"), (6, 1, "Fonte")]),
        ("Mount Crenel", [(0, 2, "Cascalho"), (1, 2, "Penhasco"), (2, 2, "Escalada"), (3, 2, "Agua Termal"), (4, 2, "Feijao"), (5, 2, "Pacci"), (6, 2, "Placa")]),
        ("Castor Wilds", [(0, 3, "Lodo Movedico"), (1, 3, "Agua Pantano"), (2, 3, "Musgo"), (3, 3, "Tronco Oco"), (4, 3, "Est. Olho"), (5, 3, "Olho Aberto"), (6, 3, "Terra Fofa")]),
        ("Castle Courtyard", [(0, 4, "Marmore Real"), (1, 4, "Cerca Viva"), (2, 4, "Muralha"), (3, 4, "Ameias"), (4, 4, "Portao Dourado"), (5, 4, "Rosa Real"), (6, 4, "Triforce")])
    ]

    ty = 350
    for cat_name, items in tile_categories:
        draw.text((24, ty + 12), cat_name, fill=(148, 163, 184, 255))
        tx = 160
        for col, row, label in items:
            tile_crop = tileset.crop((col * 16, row * 16, col * 16 + 16, row * 16 + 16))
            tile_zoom = tile_crop.resize((48, 48), Image.Resampling.NEAREST)
            draw.rectangle([tx - 1, ty - 1, tx + 48, ty + 48], outline=(71, 85, 105, 255))
            showcase.paste(tile_zoom, (tx, ty))
            tx += 56
        ty += 56

    # --- SEÇÃO 3: HUD E MOLDURAS AUTÊNTICAS GBA (ZOOM 3X) ---
    hud_y = 650
    draw.text((24, hud_y), "3. SISTEMA DE HUD GBA 1:1 (CORACOES 3D, QUARTOS, MAGIC METER, BADGES [A]/[B])", fill=(250, 204, 21, 255))

    # Linha 1 de HUD: Corações
    draw.text((24, hud_y + 36), "Corações 3D:", fill=(148, 163, 184, 255))
    heart_items = [
        (0, 0, "4/4 (Cheio)"),
        (1, 0, "3/4"),
        (2, 0, "2/4 (Metade)"),
        (3, 0, "1/4"),
        (4, 0, "Vazio (Recipiente)")
    ]
    hx = 160
    for col, row, lbl in heart_items:
        h_crop = hud.crop((col * 16, row * 16, col * 16 + 16, row * 16 + 16))
        h_zoom = h_crop.resize((48, 48), Image.Resampling.NEAREST)
        draw.rectangle([hx - 1, hud_y + 24 - 1, hx + 48, hud_y + 24 + 48], outline=(185, 28, 28, 255))
        showcase.paste(h_zoom, (hx, hud_y + 24))
        draw.text((hx, hud_y + 76), lbl, fill=(203, 213, 225, 255))
        hx += 100

    # Barra de Magia (Magic Meter)
    draw.text((24, hud_y + 110), "Magic Meter:", fill=(148, 163, 184, 255))
    meter_items = [
        (0, 5, "Ponta Rubi"),
        (1, 5, "Barra Cheia"),
        (2, 5, "Barra Vazia"),
        (3, 5, "Ponta Ouro"),
        (4, 5, "Frasco Magico")
    ]
    mx = 160
    for col, row, lbl in meter_items:
        m_crop = hud.crop((col * 16, row * 16, col * 16 + 16, row * 16 + 16))
        m_zoom = m_crop.resize((48, 48), Image.Resampling.NEAREST)
        draw.rectangle([mx - 1, hud_y + 100 - 1, mx + 48, hud_y + 100 + 48], outline=(5, 150, 105, 255))
        showcase.paste(m_zoom, (mx, hud_y + 100))
        draw.text((mx, hud_y + 152), lbl, fill=(203, 213, 225, 255))
        mx += 100

    # Barra Montada em Zoom 3x:
    cap_l = hud.crop((0 * 16, 5 * 16, 1 * 16, 6 * 16))
    bar_f = hud.crop((1 * 16, 5 * 16, 2 * 16, 6 * 16))
    bar_e = hud.crop((2 * 16, 5 * 16, 3 * 16, 6 * 16))
    cap_r = hud.crop((3 * 16, 5 * 16, 4 * 16, 6 * 16))
    assembled_meter = Image.new("RGBA", (16 + 16 + 16 + 16, 16), (0, 0, 0, 0))
    assembled_meter.paste(cap_l, (0, 0))
    assembled_meter.paste(bar_f, (16, 0))
    assembled_meter.paste(bar_f, (32, 0))
    assembled_meter.paste(cap_r, (48, 0))
    assembled_zoom = assembled_meter.resize((64 * 3, 16 * 3), Image.Resampling.NEAREST)
    showcase.paste(assembled_zoom, (700, hud_y + 100))
    draw.text((700, hud_y + 152), "Barra de Magia Montada (1:1 GBA)", fill=(52, 211, 153, 255))

    # Badges de Ação [A] e [B]
    draw.text((24, hud_y + 185), "Badges [A] / [B]:", fill=(148, 163, 184, 255))
    badge_a = hud.crop((8 * 16, 0, 9 * 16, 16)).resize((48, 48), Image.Resampling.NEAREST)
    badge_b = hud.crop((9 * 16, 0, 10 * 16, 16)).resize((48, 48), Image.Resampling.NEAREST)
    sword_0 = hud.crop((0, 16, 16, 32)).resize((48, 48), Image.Resampling.NEAREST)
    sub_0   = hud.crop((4 * 16, 32, 5 * 16, 48)).resize((48, 48), Image.Resampling.NEAREST)

    showcase.paste(badge_a, (160, hud_y + 175))
    showcase.paste(sword_0, (212, hud_y + 175))
    draw.text((160, hud_y + 228), "[A] Espada (Safira)", fill=(96, 165, 250, 255))

    showcase.paste(badge_b, (300, hud_y + 175))
    showcase.paste(sub_0,   (352, hud_y + 175))
    draw.text((300, hud_y + 228), "[B] Arco (Carmim)", fill=(248, 113, 113, 255))

    # --- SEÇÃO 4: TELAS COMPLETAS SIMULADAS DE JOGO (GBA WIDESCREEN 284x160 a 2X = 568x320) ---
    sim_y = 920
    draw.text((24, sim_y), "4. TELAS DE JOGO COMPLETAS IN-GAME (VIRTUAL FRAMEBUFFER WIDESCREEN 16:9 + HUD)", fill=(250, 204, 21, 255))

    def make_screen_mockup(map_img, cam_x, cam_y, hearts_q, magic_val, sword_idx, sub_col, sub_row):
        vw, vh = 284, 160
        scr = Image.new("RGBA", (vw, vh), (0, 0, 0, 255))
        # Crop world
        view_crop = map_img.crop((cam_x, cam_y, cam_x + vw, cam_y + vh))
        scr.paste(view_crop, (0, 0))

        # HUD Top Bar
        hud_bar = Image.new("RGBA", (vw, 14), (12, 28, 13, 255))
        scr.paste(hud_bar, (0, 0))

        # Hearts
        for h in range(3):
            q = hearts_q - (h * 4)
            h_col = 4
            if q >= 4:      h_col = 0
            elif q == 3:    h_col = 1
            elif q == 2:    h_col = 2
            elif q == 1:    h_col = 3
            h_cell = hud.crop((h_col * 16, 0, (h_col + 1) * 16, 16))
            scr.paste(h_cell, (2 + h * 11, -1), h_cell)

        # Magic Meter
        scr.paste(cap_l, (36, -1), cap_l)
        s1 = bar_f if magic_val >= 50 else bar_e
        scr.paste(s1, (44, -1), s1)
        s2 = bar_f if magic_val >= 100 else bar_e
        scr.paste(s2, (52, -1), s2)
        scr.paste(cap_r, (60, -1), cap_r)

        # Rupees
        rupee_icon = hud.crop((5 * 16, 0, 6 * 16, 16))
        scr.paste(rupee_icon, (70, -1), rupee_icon)
        d_scr = ImageDraw.Draw(scr)
        d_scr.text((83, 3), "99", fill=(0, 255, 136, 255))

        # BGM note
        d_scr.rectangle([106, 5, 108, 9], fill=(0, 229, 255, 255))

        # Badge [A] + Sword
        b_a = hud.crop((8 * 16, 0, 9 * 16, 16))
        scr.paste(b_a, (118, -1), b_a)
        sw = hud.crop((sword_idx * 16, 16, (sword_idx + 1) * 16, 32))
        scr.paste(sw, (129, -1), sw)

        # Badge [B] + Subweapon
        b_b = hud.crop((9 * 16, 0, 10 * 16, 16))
        scr.paste(b_b, (146, -1), b_b)
        subw = hud.crop((sub_col * 16, sub_row * 16, (sub_col + 1) * 16, (sub_row + 1) * 16))
        scr.paste(subw, (157, -1), subw)

        # Region Badge USA
        d_scr.rectangle([vw - 32, 2, vw - 4, 11], fill=(66, 135, 245, 255))
        d_scr.text((vw - 28, 2), "USA", fill=(255, 255, 255, 255))

        return scr

    # Tela 1: Castle Courtyard com 2 corações e 3/4 (11 quartos), magia 100%, Four Sword
    scr1 = make_screen_mockup(map_castle, 114, 112, 11, 100, 2, 4, 2)
    scr1_zoom = scr1.resize((568, 320), Image.Resampling.NEAREST)
    draw.rectangle([24 - 2, sim_y + 30 - 2, 24 + 568 + 1, sim_y + 30 + 320 + 1], outline=(234, 179, 8, 255), width=2)
    showcase.paste(scr1_zoom, (24, sim_y + 30))
    draw.text((24, sim_y + 356), "Hyrule Castle Courtyard - Brasao da Triforce, Coracoes 2 3/4 HP e Magic Meter Cheio", fill=(250, 204, 21, 255))

    # Tela 2: Hyrule Town com 1 coração e meio (6 quartos), magia 50%, Boomerang
    scr2 = make_screen_mockup(map_town, 146, 144, 6, 50, 1, 0, 2)
    scr2_zoom = scr2.resize((568, 320), Image.Resampling.NEAREST)
    draw.rectangle([608 - 2, sim_y + 30 - 2, 608 + 568 + 1, sim_y + 30 + 320 + 1], outline=(56, 189, 248, 255), width=2)
    showcase.paste(scr2_zoom, (608, sim_y + 30))
    draw.text((608, sim_y + 356), "Hyrule Town Square - Chafariz de Marmore, Coracoes 1 1/2 HP e Magic Meter a 50%", fill=(56, 189, 248, 255))

    # Salva o showcase
    output_path = "C:/Users/thiag/.gemini/antigravity/brain/91f45871-4e2f-495f-88d6-7427ed06770a/overworld_and_hud_showcase.png"
    showcase.save(output_path)
    print(f"[SUCESSO] Showcase gerado com exito em: {output_path}")

if __name__ == "__main__":
    main()
