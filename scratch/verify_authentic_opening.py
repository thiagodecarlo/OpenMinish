#!/usr/bin/env python3
"""
scratch/verify_authentic_opening.py - Visual Validation of Authentic Opening & Cutscenes
Generates a 2x scaled composite showcase of all 6 finalized opening screens:
1. Capcom Authentic Logo
2. Nintendo Authentic Logo
3. Title Screen (Monumental Zelda logo, Triforce, Four Sword in pedestal, "PRESS START")
4. File Select Screen (Save cards, Link's data, Hearts, Rupees, Elements)
5. Link's House Cutscene (Bedroom, Smith's forge, Link, Zelda, Smith, sparkling sword)
6. Throne Room Cutscene (King Daltus, dais, Royal Guards, Link, Smith)
"""

import os
import sys
import struct
from PIL import Image, ImageDraw, ImageFont

ARTIFACT_DIR = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a"
SHOWCASE_PATH = os.path.join(ARTIFACT_DIR, "authentic_opening_showcase.png")

def get_font(size):
    try:
        for font_name in ["arialbd.ttf", "arial.ttf", "calibri.ttf"]:
            p = os.path.join(os.environ.get("WINDIR", "C:\\Windows"), "Fonts", font_name)
            if os.path.exists(p):
                return ImageFont.truetype(p, size)
    except Exception:
        pass
    return ImageFont.load_default()

def load_or_create(path):
    if os.path.exists(path):
        try:
            with open(path, 'rb') as f:
                f.seek(10)
                off = struct.unpack('<I', f.read(4))[0]
                f.seek(18)
                w, h = struct.unpack('<ii', f.read(8))
                top_down = (h < 0)
                h = abs(h)
                f.seek(28)
                bpp = struct.unpack('<H', f.read(2))[0]
                if bpp == 32:
                    f.seek(off)
                    raw = f.read(w * h * 4)
                    rgba = bytearray(w * h * 4)
                    for i in range(w * h):
                        b = raw[i*4 + 0]
                        g = raw[i*4 + 1]
                        r = raw[i*4 + 2]
                        a = raw[i*4 + 3]
                        rgba[i*4 + 0] = r
                        rgba[i*4 + 1] = g
                        rgba[i*4 + 2] = b
                        rgba[i*4 + 3] = a
                    img = Image.frombytes('RGBA', (w, h), bytes(rgba))
                    if not top_down:
                        img = img.transpose(Image.FLIP_TOP_BOTTOM)
                    return img
        except Exception:
            pass
        return Image.open(path).convert('RGBA')
    print(f"Aviso: Imagem {path} nao encontrada.")
    return Image.new('RGBA', (240, 160), (0, 0, 0, 255))

def simulate_title_screen(bg_img):
    img = bg_img.copy()
    draw = ImageDraw.Draw(img)
    font = get_font(10)
    # PRESS START
    txt = "PRESS START"
    bbox = draw.textbbox((0, 0), txt, font=font)
    tw = bbox[2] - bbox[0]
    draw.text(((240 - tw) // 2 + 1, 137), txt, font=font, fill=(15, 23, 42, 255))
    draw.text(((240 - tw) // 2, 136), txt, font=font, fill=(254, 240, 138, 255))
    # Copyright
    foot = "(C) 2004 Nintendo / CAPCOM Co., Ltd."
    fbox = draw.textbbox((0, 0), foot, font=font)
    fw = fbox[2] - fbox[0]
    draw.text(((240 - fw) // 2, 149), foot, font=font, fill=(148, 163, 184, 255))
    return img

def simulate_file_select(bg_img):
    img = bg_img.copy()
    draw = ImageDraw.Draw(img)
    font_bold = get_font(10)
    font_small = get_font(9)

    # Slot 1: Link (Endgame save)
    # Card 1 active highlight & heart cursor
    draw.rectangle([11, 26, 11 + 216, 26 + 32], outline=(245, 158, 11, 255), width=1)
    # Heart cursor at left
    draw.polygon([(4, 38), (8, 34), (12, 38), (8, 44)], fill=(239, 68, 68, 255))
    draw.text((52, 32), "LINK", font=font_bold, fill=(255, 255, 255, 255))
    # Hearts
    for h in range(12):
        hx = 98 + (h % 8) * 8
        hy = 31 + (h // 8) * 7
        draw.rectangle([hx, hy, hx + 5, hy + 5], fill=(239, 68, 68, 255))
    # Rupees
    draw.text((168, 32), "999", font=font_small, fill=(134, 239, 172, 255))
    # 4 Elements
    draw.rectangle([200, 31, 204, 35], fill=(34, 197, 94, 255))   # Earth
    draw.rectangle([206, 31, 210, 35], fill=(239, 68, 68, 255))   # Fire
    draw.rectangle([200, 37, 204, 41], fill=(56, 189, 248, 255))  # Water
    draw.rectangle([206, 37, 210, 41], fill=(16, 185, 129, 255))  # Wind

    # Slot 2: ZELDA (Midgame save)
    draw.text((52, 68), "ZELDA", font=font_bold, fill=(241, 245, 249, 255))
    for h in range(6):
        hx = 98 + h * 8
        hy = 67
        draw.rectangle([hx, hy, hx + 5, hy + 5], fill=(239, 68, 68, 255))
    draw.text((168, 68), "250", font=font_small, fill=(134, 239, 172, 255))
    draw.rectangle([200, 67, 204, 71], fill=(34, 197, 94, 255))   # Earth
    draw.rectangle([206, 67, 210, 71], fill=(239, 68, 68, 255))   # Fire

    # Slot 3: New Game
    draw.text((52, 104), "- NOVO JOGO -", font=font_small, fill=(148, 163, 184, 255))
    for h in range(3):
        hx = 150 + h * 8
        hy = 103
        draw.rectangle([hx, hy, hx + 5, hy + 5], fill=(239, 68, 68, 255))

    return img

def simulate_link_house(bg_img, link_sheet, npcs_sheet):
    img = bg_img.copy()
    # Overlay Link, Zelda, Smith, and dialogue banner
    # Link at (58, 66)
    if link_sheet:
        link_sprite = link_sheet.crop((0, 0, 32, 32))
        img.alpha_composite(link_sprite, (58 - 16, 66 - 20))
    # Zelda at (92, 68)
    if npcs_sheet:
        zelda_sprite = npcs_sheet.crop((0, 32, 16, 48))
        img.alpha_composite(zelda_sprite, (92 - 8, 68 - 12))
    # Smith at (152, 68)
    if npcs_sheet:
        smith_sprite = npcs_sheet.crop((0, 16, 16, 32))
        img.alpha_composite(smith_sprite, (152 - 8, 68 - 8))

    draw = ImageDraw.Draw(img)
    # Sparkling sword between Link and Smith
    draw.line([122, 54, 122, 66], fill=(56, 189, 248, 255), width=2)
    draw.line([119, 62, 125, 62], fill=(245, 158, 11, 255), width=2)
    draw.point((122, 52), fill=(254, 240, 138, 255))

    # Cinematic Dialogue Banner at bottom
    draw.rectangle([8, 116, 232, 155], fill=(15, 23, 42, 230), outline=(245, 158, 11, 255))
    font_hud_title = get_font(10)
    font_hud_txt = get_font(9)
    draw.text((16, 119), "O DESPERTAR DE LINK", font=font_hud_title, fill=(253, 224, 71, 255))
    draw.text((16, 132), "Zelda vem convidar Link.\nSmith entrega a espada do torneio!", font=font_hud_txt, fill=(226, 232, 240, 255))

    return img

def simulate_throne_room(bg_img, link_sheet, npcs_sheet):
    img = bg_img.copy()
    # King Daltus on throne at (120, 26)
    if npcs_sheet:
        king_sprite = npcs_sheet.crop((80, 0, 96, 16))
        img.alpha_composite(king_sprite, (120 - 8, 24))
    # Guards flanking at (44, 44) and (180, 44)
    if npcs_sheet:
        guard_sprite = npcs_sheet.crop((0, 128, 16, 144))
        img.alpha_composite(guard_sprite, (44, 44))
        img.alpha_composite(guard_sprite, (180, 44))
    # Smith at (152, 68)
    if npcs_sheet:
        smith_sprite = npcs_sheet.crop((0, 16, 16, 32))
        img.alpha_composite(smith_sprite, (152 - 8, 68 - 8))
    # Link at (120, 50) facing North
    if link_sheet:
        link_north = link_sheet.crop((64, 0, 96, 32))
        img.alpha_composite(link_north, (120 - 16, 70 - 20))

    draw = ImageDraw.Draw(img)
    # Cinematic Dialogue Banner at bottom
    draw.rectangle([8, 116, 232, 155], fill=(15, 23, 42, 230), outline=(245, 158, 11, 255))
    font_hud_title = get_font(10)
    font_hud_txt = get_font(9)
    draw.text((16, 119), "AUDIENCIA COM O REI DALTUS", font=font_hud_title, fill=(253, 224, 71, 255))
    draw.text((16, 132), "O Rei Daltus confia a Link o destino do reino:\nencontre o povo Minish e restaure a lamina!", font=font_hud_txt, fill=(226, 232, 240, 255))

    return img

def main():
    print("[VERIFICACAO] Carregando assets e montando showcase...")
    capcom_img = load_or_create("assets/ui/startup/capcom_logo.bmp")
    nintendo_img = load_or_create("assets/ui/startup/nintendo_logo.bmp")
    title_bg = load_or_create("assets/ui/startup/title_screen_bg.bmp")
    file_bg = load_or_create("assets/ui/startup/file_select_bg.bmp")
    house_bg = load_or_create("assets/regions/map_link_house.bmp")
    throne_bg = load_or_create("assets/regions/map_throne_room.bmp")

    link_sheet = load_or_create("assets/regions/usa/link.bmp")
    npcs_sheet = load_or_create("assets/regions/usa/npcs.bmp")

    title_img = simulate_title_screen(title_bg)
    file_img = simulate_file_select(file_bg)
    house_img = simulate_link_house(house_bg, link_sheet, npcs_sheet)
    throne_img = simulate_throne_room(throne_bg, link_sheet, npcs_sheet)

    # Create 3x2 Grid Showcase (Scaled 2x for crisp pixel art evaluation)
    # Canvas: 3 columns x 2 rows
    # Each panel: 240x160 -> 480x320 scaled
    panel_w, panel_h = 240, 160
    scale = 2
    pw_sc, ph_sc = panel_w * scale, panel_h * scale

    cols, rows = 3, 2
    margin_x = 24
    margin_y = 48
    header_h = 60

    total_w = cols * pw_sc + (cols + 1) * margin_x
    total_h = rows * ph_sc + (rows + 1) * margin_y + header_h

    showcase = Image.new('RGBA', (total_w, total_h), (11, 15, 25, 255))
    draw = ImageDraw.Draw(showcase)

    # Master Header
    font_main = get_font(22)
    font_sub = get_font(12)
    draw.text((margin_x, 14), "OpenMinish - Showcase de Abertura & Cutscenes Autenticas", font=font_main, fill=(251, 191, 36, 255))
    draw.text((margin_x, 40), "Eliminacao de Graficos Procedurais: Logos Capcom & Nintendo, Tela de Titulo, Selecao de Save, Quarto de Link e Sala do Trono", font=font_sub, fill=(148, 163, 184, 255))

    panels = [
        ("1. Logo Autentico da Capcom", capcom_img),
        ("2. Logo Autentico da Nintendo", nintendo_img),
        ("3. Tela de Titulo Autentica", title_img),
        ("4. Tela de Selecao de Arquivo Autentica", file_img),
        ("5. Cutscene: Quarto de Link & Forja", house_img),
        ("6. Cutscene: Sala do Trono do Rei Daltus", throne_img)
    ]

    font_label = get_font(13)

    for idx, (label, pimg) in enumerate(panels):
        c = idx % cols
        r = idx // cols
        px = margin_x + c * (pw_sc + margin_x)
        py = header_h + margin_y + r * (ph_sc + margin_y)

        # Draw panel title
        draw.text((px, py - 20), label, font=font_label, fill=(245, 158, 11, 255))

        # Scale 2x nearest-neighbor
        p_scaled = pimg.resize((pw_sc, ph_sc), Image.NEAREST)
        showcase.paste(p_scaled, (px, py))

        # Border around panel
        draw.rectangle([px - 2, py - 2, px + pw_sc + 1, py + ph_sc + 1], outline=(71, 85, 105, 255), width=2)

    os.makedirs(ARTIFACT_DIR, exist_ok=True)
    showcase.save(SHOWCASE_PATH)
    print(f"[SUCESSO] Showcase gerado com exito em: {SHOWCASE_PATH}")

if __name__ == '__main__':
    main()
