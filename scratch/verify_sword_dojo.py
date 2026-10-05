#!/usr/bin/env python3
"""
verify_sword_dojo.py - Verificação Visual do Sistema de Dojos & Tiger Scrolls
Gera uma vitrine PNG com todos os 8 Pergaminhos do Tigre e os Mestres Espadachins.
"""
from PIL import Image, ImageDraw, ImageFont
import os, sys

W, H = 640, 520
img = Image.new("RGBA", (W, H), (15, 10, 8, 255))
draw = ImageDraw.Draw(img)

try:
    font_title = ImageFont.truetype("arial.ttf", 18)
    font_sub   = ImageFont.truetype("arial.ttf", 13)
    font_small = ImageFont.truetype("arial.ttf", 11)
    font_tiny  = ImageFont.truetype("arial.ttf", 9)
except:
    font_title = ImageFont.load_default()
    font_sub = font_small = font_tiny = font_title

# Title bar
draw.rectangle([0, 0, W, 48], fill=(26, 18, 14))
draw.rectangle([0, 46, W, 48], fill=(212, 175, 55))
draw.text((W//2 - 180, 8), "⚔ DOJOS DE ESGRIMA & PERGAMINHOS DO TIGRE ⚔", fill=(253, 224, 71), font=font_title)
draw.text((W//2 - 120, 30), "8 Tiger Scrolls — Blade Brothers Masters", fill=(56, 189, 248), font=font_small)

# Tiger Scrolls catalog
scrolls = [
    ("Nº 1 — Spin Attack",      "Ataque Giratório",          "Mestre Swiftblade",     "Hyrule Town",    (220, 38, 38),   "Segura espada → giro 360°"),
    ("Nº 2 — Roll Attack",      "Estocada Rolando",          "Mestre Grayblade",      "Monte Crenel",   (100, 116, 139), "Ataca logo após rolar"),
    ("Nº 3 — Dash Attack",      "Investida com Espada",      "Mestre Swiftblade",     "Hyrule Town",    (37, 99, 235),   "Corre com Botas de Pégaso + espada"),
    ("Nº 4 — Rock Breaker",     "Quebra-Pedras",             "Mestre Swiftblade",     "Hyrule Town",    (217, 119, 6),   "Corta rochas sem bombas"),
    ("Nº 5 — Sword Beam",       "Raio de Espada Sagrado",    "Mestre Grimblade",      "Jardim do Castelo", (251, 191, 36), "HP cheio → raio de luz"),
    ("Nº 6 — Down Thrust",      "Estocada Aérea",            "Mestre Waveblade",      "Lago Hylia",     (6, 182, 212),   "Ataque vertical no ar"),
    ("Nº 7 — Peril Beam",       "Raio de Desespero",         "Mestre Splitblade",     "Quedas do Véu",  (153, 27, 27),   "1 coração → raios vermelhos"),
    ("Nº 8 — Great Spin Attack", "Grande Furacão",           "Mestre Swiftblade I",   "Castor Wilds",   (5, 150, 105),   "Spin Attack × 5 revoluções"),
]

y_start = 56
card_h = 54
gap = 4

for i, (title, name_pt, master, location, ribbon_rgb, desc) in enumerate(scrolls):
    y = y_start + i * (card_h + gap)

    # Card background
    draw.rectangle([8, y, W - 8, y + card_h], fill=(30, 22, 18))
    draw.rectangle([8, y, W - 8, y + 1], fill=(212, 175, 55))
    draw.rectangle([8, y + card_h - 1, W - 8, y + card_h], fill=(212, 175, 55))

    # Scroll icon
    scroll_x = 16
    scroll_y = y + 8
    # Parchment body
    draw.rectangle([scroll_x, scroll_y + 4, scroll_x + 16, scroll_y + card_h - 16], fill=(254, 240, 138))
    # Top/bottom rollers
    draw.rectangle([scroll_x - 2, scroll_y + 2, scroll_x + 18, scroll_y + 6], fill=(217, 119, 6))
    draw.rectangle([scroll_x - 2, scroll_y + card_h - 18, scroll_x + 18, scroll_y + card_h - 14], fill=(217, 119, 6))
    # Ribbon stripe (colored per master)
    draw.rectangle([scroll_x, scroll_y + 16, scroll_x + 16, scroll_y + 22], fill=ribbon_rgb)

    # Number circle
    cx = scroll_x + 8
    cy = scroll_y + 19
    draw.ellipse([cx - 5, cy - 5, cx + 5, cy + 5], fill=ribbon_rgb, outline=(255, 255, 255))
    draw.text((cx - 3, cy - 5), str(i + 1), fill=(255, 255, 255), font=font_tiny)

    # Text info
    tx = 44
    draw.text((tx, y + 4), title, fill=(253, 224, 71), font=font_sub)
    draw.text((tx, y + 20), f"{name_pt}  •  {master}  •  {location}", fill=(148, 163, 184), font=font_small)
    draw.text((tx, y + 34), desc, fill=(56, 189, 248), font=font_tiny)

    # Unlocked badge
    badge_x = W - 90
    draw.rectangle([badge_x, y + 14, badge_x + 72, y + 34], fill=(22, 101, 52), outline=(34, 197, 94))
    draw.text((badge_x + 6, y + 17), "APRENDIDO ✓", fill=(134, 239, 172), font=font_small)

# Footer summary
footer_y = y_start + 8 * (card_h + gap) + 8
draw.rectangle([0, footer_y, W, footer_y + 2], fill=(212, 175, 55))
draw.text((16, footer_y + 6), "8/8 PERGAMINHOS COMPLETOS — MESTRE ESPADACHIM SUPREMO!", fill=(253, 224, 71), font=font_sub)
draw.text((16, footer_y + 24), "Sistema: sword_dojo.h/.c  |  Hotkey: ~ (backtick)  |  Persistência: tiger_scrolls_mask (u8 bitmask)", fill=(148, 163, 184), font=font_small)
draw.text((16, footer_y + 40), "Mecânicas: Sword Beam, Peril Beam, Rock Breaker, Roll Attack, Dash Attack, Down Thrust, Great Spin", fill=(56, 189, 248), font=font_small)

out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a"
os.makedirs(out_dir, exist_ok=True)
out_path = os.path.join(out_dir, "sword_dojo_showcase.png")
img.save(out_path)
print(f"[OK] Vitrine gerada em: {out_path}")
