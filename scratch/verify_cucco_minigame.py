import math
import os
from PIL import Image, ImageDraw, ImageFont

WIDTH = 1200
HEIGHT = 800
IMG_PATH = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a\cucco_minigame_showcase.png"

img = Image.new("RGBA", (WIDTH, HEIGHT), (15, 23, 42, 255)) # Dark slate base
draw = ImageDraw.Draw(img)

try:
    font_title = ImageFont.truetype("arial.ttf", 26)
    font_h2 = ImageFont.truetype("arialbd.ttf", 18)
    font_body = ImageFont.truetype("arial.ttf", 13)
    font_small = ImageFont.truetype("arial.ttf", 11)
    font_huge = ImageFont.truetype("arialbd.ttf", 34)
except:
    font_title = ImageFont.load_default()
    font_h2 = ImageFont.load_default()
    font_body = ImageFont.load_default()
    font_small = ImageFont.load_default()
    font_huge = ImageFont.load_default()

# Header
draw.rectangle([(0, 0), (WIDTH, 70)], fill=(30, 41, 59, 255))
draw.line([(0, 70), (WIDTH, 70)], fill=(251, 191, 36, 255), width=2)
draw.text((25, 12), "OPENMINISH - SPRINT 6: POLIMENTO & MINIGAMES", fill=(251, 191, 36, 255), font=font_small)
draw.text((25, 28), "MINIGAME DAS GALINHAS CUCCO DE ANJU (10 NIVEIS)", fill=(255, 255, 255, 255), font=font_title)
draw.text((WIDTH - 360, 26), "Zero-ROM C11 | IA Cuccos | Heart Piece Supremo", fill=(148, 163, 184, 255), font=font_body)

# 4 Panels
panels = [
    {"rect": (25, 85, 585, 420), "title": "1. HYRULE TOWN & CERCADO DA ANJU", "badge": "10 ROUNDS ESCALONADOS"},
    {"rect": (615, 85, 1175, 420), "title": "2. FISICA DE TRANSPORTE & ARREMESSO", "badge": "PLANAR & PARABOLA"},
    {"rect": (25, 440, 585, 775), "title": "3. IA DINAMICA: BRANCA, PINTINHO & DOURADA", "badge": "FUGA & BATIMENTO DE ASAS"},
    {"rect": (615, 440, 1175, 775), "title": "4. DESAFIO SUPREMO (ROUND 10) & RECOMPENSA", "badge": "HEART PIECE 100%"}
]

for p in panels:
    x0, y0, x1, y1 = p["rect"]
    draw.rectangle([(x0, y0), (x1, y1)], fill=(23, 31, 48, 255), outline=(51, 65, 85, 255), width=2)
    # Header bar
    draw.rectangle([(x0, y0), (x1, y0 + 35)], fill=(30, 41, 59, 255))
    draw.line([(x0, y0 + 35), (x1, y0 + 35)], fill=(71, 85, 105, 255), width=1)
    draw.text((x0 + 15, y0 + 8), p["title"], fill=(255, 255, 255, 255), font=font_h2)
    # Badge
    bw = len(p["badge"]) * 7 + 16
    draw.rectangle([(x1 - bw - 15, y0 + 7), (x1 - 15, y0 + 27)], fill=(245, 158, 11, 40), outline=(245, 158, 11, 200))
    draw.text((x1 - bw - 8, y0 + 9), p["badge"], fill=(245, 158, 11, 255), font=font_small)

# -------------------------------------------------------------
# Panel 1: Hyrule Town & Anju's Pen
# -------------------------------------------------------------
p1 = panels[0]["rect"]
# Grass background inside panel
draw.rectangle([(p1[0] + 15, p1[1] + 45), (p1[2] - 15, p1[3] - 15)], fill=(34, 100, 60, 255))

# Town cobbled path
draw.rectangle([(p1[0] + 15, p1[1] + 45), (p1[0] + 180, p1[3] - 15)], fill=(120, 113, 108, 255))
for cy in range(p1[1] + 55, p1[3] - 25, 24):
    for cx in range(p1[0] + 25, p1[0] + 170, 30):
        draw.rectangle([(cx, cy), (cx + 22, cy + 18)], fill=(100, 95, 90, 255), outline=(80, 75, 70, 255))

# Anju's Pen Fences
pen_x1 = p1[0] + 250
pen_y1 = p1[1] + 70
pen_x2 = p1[2] - 30
pen_y2 = p1[3] - 40

# Pen interior (hay / earth)
draw.rectangle([(pen_x1, pen_y1), (pen_x2, pen_y2)], fill=(180, 140, 80, 255))

# Wooden Fences
draw.rectangle([(pen_x1, pen_y1), (pen_x2, pen_y1 + 8)], fill=(110, 50, 20, 255))
draw.rectangle([(pen_x1, pen_y2 - 8), (pen_x2, pen_y2)], fill=(110, 50, 20, 255))
draw.rectangle([(pen_x1, pen_y1), (pen_x1 + 8, pen_y2)], fill=(110, 50, 20, 255))
draw.rectangle([(pen_x2 - 8, pen_y1), (pen_x2, pen_y2)], fill=(110, 50, 20, 255))

# Posts
for fx in range(pen_x1, pen_x2 + 1, 35):
    draw.rectangle([(fx - 4, pen_y1 - 6), (fx + 4, pen_y1 + 12)], fill=(160, 80, 30, 255))
    draw.rectangle([(fx - 4, pen_y2 - 12), (fx + 4, pen_y2 + 6)], fill=(160, 80, 30, 255))

# Anju NPC at gate
anju_x = pen_x1 - 25
anju_y = (pen_y1 + pen_y2) // 2
draw.ellipse([(anju_x - 10, anju_y - 30), (anju_x + 10, anju_y - 12)], fill=(244, 114, 182, 255)) # Head/Ribbon
draw.rectangle([(anju_x - 12, anju_y - 12), (anju_x + 12, anju_y + 20)], fill=(59, 130, 246, 255)) # Blue dress
draw.rectangle([(anju_x - 8, anju_y - 8), (anju_x + 8, anju_y + 14)], fill=(255, 255, 255, 255)) # White apron
draw.text((anju_x - 22, anju_y + 24), "ANJU", fill=(254, 240, 138, 255), font=font_small)

# Speech Bubble
draw.rectangle([(anju_x - 120, anju_y - 80), (anju_x + 30, anju_y - 40)], fill=(255, 255, 255, 240), outline=(30, 41, 59, 255), width=2)
draw.text((anju_x - 110, anju_y - 74), "Socorro, Link!", fill=(15, 23, 42, 255), font=font_body)
draw.text((anju_x - 110, anju_y - 56), "Meus Cuccos fugiram!", fill=(185, 28, 28, 255), font=font_small)

# Rescued Cuccos inside pen
for c_i, (cx, cy) in enumerate([(pen_x1 + 40, pen_y1 + 50), (pen_x1 + 100, pen_y1 + 80), (pen_x1 + 180, pen_y1 + 45)]):
    draw.ellipse([(cx - 10, cy - 8), (cx + 10, cy + 8)], fill=(255, 255, 255, 255), outline=(203, 213, 225, 255))
    draw.polygon([(cx - 2, cy - 14), (cx + 6, cy - 14), (cx + 2, cy - 7)], fill=(239, 68, 68, 255))
    draw.polygon([(cx + 8, cy - 4), (cx + 14, cy - 1), (cx + 8, cy + 2)], fill=(245, 158, 11, 255))

# -------------------------------------------------------------
# Panel 2: Carrying Physics & Throwing Parabola
# -------------------------------------------------------------
p2 = panels[1]["rect"]
draw.rectangle([(p2[0] + 15, p2[1] + 45), (p2[2] - 15, p2[3] - 15)], fill=(20, 27, 45, 255))

# Link carrying Cucco
lx = p2[0] + 100
ly = p2[1] + 240
# Link
draw.ellipse([(lx - 12, ly - 35), (lx + 12, ly - 15)], fill=(254, 202, 202, 255))
draw.polygon([(lx - 16, ly - 30), (lx + 16, ly - 30), (lx, ly - 55)], fill=(34, 197, 94, 255))
draw.rectangle([(lx - 14, ly - 15), (lx + 14, ly + 20)], fill=(22, 163, 74, 255))
# Arms raised carrying
draw.line([(lx - 14, ly - 5), (lx - 16, ly - 45)], fill=(22, 163, 74, 255), width=4)
draw.line([(lx + 14, ly - 5), (lx + 16, ly - 45)], fill=(22, 163, 74, 255), width=4)
# Cucco above head
draw.ellipse([(lx - 14, ly - 65), (lx + 14, ly - 42)], fill=(255, 255, 255, 255), outline=(203, 213, 225, 255))
draw.polygon([(lx - 2, cy - 14), (lx + 6, ly - 72), (lx + 2, ly - 63)], fill=(239, 68, 68, 255))
draw.polygon([(lx + 12, ly - 56), (lx + 18, ly - 53), (lx + 12, ly - 50)], fill=(245, 158, 11, 255))
# Wing flapping
draw.polygon([(lx - 24, ly - 60), (lx - 14, ly - 55), (lx - 14, ly - 45)], fill=(226, 232, 240, 255))
draw.polygon([(lx + 24, ly - 60), (lx + 14, ly - 55), (lx + 14, ly - 45)], fill=(226, 232, 240, 255))
draw.text((lx - 45, ly + 30), "LINK CARREGANDO [A]", fill=(148, 163, 184, 255), font=font_small)

# Parabolic arc trajectory
start_arc = (lx + 25, ly - 55)
peak_arc = (lx + 180, ly - 130)
end_arc = (lx + 340, ly - 20)

points = []
for t_step in range(30):
    t = t_step / 29.0
    # Bezier quadratic
    bx = (1 - t)**2 * start_arc[0] + 2 * (1 - t) * t * peak_arc[0] + t**2 * end_arc[0]
    by = (1 - t)**2 * start_arc[1] + 2 * (1 - t) * t * peak_arc[1] + t**2 * end_arc[1]
    points.append((bx, by))

for i in range(len(points) - 1):
    draw.line([points[i], points[i + 1]], fill=(251, 191, 36, 200), width=3)

# Thrown Cucco flying in mid-air
fly_x, fly_y = points[15]
draw.ellipse([(fly_x - 14, fly_y - 12), (fly_x + 14, fly_y + 12)], fill=(255, 255, 255, 255), outline=(203, 213, 225, 255))
draw.polygon([(fly_x - 4, fly_y - 18), (fly_x + 6, fly_y - 18), (fly_x + 2, fly_y - 10)], fill=(239, 68, 68, 255))
draw.polygon([(fly_x + 12, fly_y - 2), (fly_x + 18, fly_y + 2), (fly_x + 12, fly_y + 6)], fill=(245, 158, 11, 255))
# Big flapping wings
draw.polygon([(fly_x - 26, fly_y - 22), (fly_x - 10, fly_y - 5), (fly_x - 12, fly_y + 8)], fill=(226, 232, 240, 255))
draw.polygon([(fly_x + 26, fly_y - 22), (fly_x + 10, fly_y - 5), (fly_x + 12, fly_y + 8)], fill=(226, 232, 240, 255))
# Feathers falling
for f_pos in [(fly_x - 20, fly_y + 20), (fly_x - 5, fly_y + 35), (fly_x + 15, fly_y + 25)]:
    draw.ellipse([(f_pos[0] - 2, f_pos[1] - 5), (f_pos[0] + 2, f_pos[1] + 5)], fill=(241, 245, 249, 200))

# Destination Pen Gate
pen_t_x = end_arc[0]
pen_t_y = end_arc[1]
draw.rectangle([(pen_t_x - 30, pen_t_y - 15), (pen_t_x + 60, pen_t_y + 25)], fill=(180, 140, 80, 255), outline=(110, 50, 20, 255), width=3)
draw.text((pen_t_x - 20, pen_t_y + 30), "CERCADO (SEGURO!)", fill=(74, 222, 128, 255), font=font_small)

# -------------------------------------------------------------
# Panel 3: Dynamic AI: White, Chick, Golden
# -------------------------------------------------------------
p3 = panels[2]["rect"]
draw.rectangle([(p3[0] + 15, p3[1] + 45), (p3[2] - 15, p3[3] - 15)], fill=(22, 30, 46, 255))

ai_cards = [
    {
        "x": p3[0] + 30, "y": p3[1] + 65, "w": 155, "h": 230,
        "title": "CUCCO ADULTA", "type": "Padrao", "speed": "1.4 px/frame",
        "desc": "Cacareja, bica o solo\ne foge batendo asas\nquando Link se aproxima.",
        "col": (255, 255, 255), "comb": (239, 68, 68), "scale": 1.0
    },
    {
        "x": p3[0] + 205, "y": p3[1] + 65, "w": 155, "h": 230,
        "title": "PINTINHO (CHICK)", "type": "Agil", "speed": "2.0 px/frame",
        "desc": "Corpinho minusculo\ne rapido. Muda de\ndirecao velozmente!",
        "col": (253, 224, 71), "comb": (234, 88, 12), "scale": 0.6
    },
    {
        "x": p3[0] + 380, "y": p3[1] + 65, "w": 155, "h": 230,
        "title": "CUCCO DOURADA", "type": "Rara / Supremo", "speed": "2.5 px/frame",
        "desc": "Penugem brilhante.\nPlana distancias longas\ne salta sobre cercas.",
        "col": (251, 191, 36), "comb": (220, 38, 38), "scale": 1.2
    }
]

for card in ai_cards:
    cx0, cy0, cw, ch = card["x"], card["y"], card["w"], card["h"]
    draw.rectangle([(cx0, cy0), (cx0 + cw, cy0 + ch)], fill=(30, 41, 59, 255), outline=(71, 85, 105, 255), width=1)
    # Header of card
    draw.rectangle([(cx0, cy0), (cx0 + cw, cy0 + 26)], fill=(15, 23, 42, 255))
    draw.text((cx0 + 10, cy0 + 6), card["title"], fill=(255, 255, 255, 255), font=font_small)

    # Bird mockup
    bird_cx = cx0 + cw // 2
    bird_cy = cy0 + 75
    sc = card["scale"]
    bw = int(14 * sc)
    bh = int(10 * sc)
    draw.ellipse([(bird_cx - bw, bird_cy - bh), (bird_cx + bw, bird_cy + bh)], fill=card["col"], outline=(150, 150, 150, 255))
    draw.polygon([(bird_cx - 2, bird_cy - bh - 6), (bird_cx + 4, bird_cy - bh - 6), (bird_cx + 1, bird_cy - bh)], fill=card["comb"])
    draw.polygon([(bird_cx + bw, bird_cy - 2), (bird_cx + bw + 6, bird_cy + 1), (bird_cx + bw, bird_cy + 4)], fill=(245, 158, 11, 255))

    if card["title"] == "CUCCO DOURADA":
        # Glint stars
        draw.text((bird_cx - 25, bird_cy - 25), "✦", fill=(254, 240, 138, 255), font=font_small)
        draw.text((bird_cx + 18, bird_cy + 12), "✦", fill=(254, 240, 138, 255), font=font_small)

    # Stats
    draw.text((cx0 + 12, cy0 + 115), "Velocidade:", fill=(148, 163, 184, 255), font=font_small)
    draw.text((cx0 + 12, cy0 + 130), card["speed"], fill=(56, 189, 248, 255), font=font_body)
    draw.text((cx0 + 12, cy0 + 155), card["desc"], fill=(226, 232, 240, 255), font=font_small)

# -------------------------------------------------------------
# Panel 4: Supreme Challenge & Heart Piece
# -------------------------------------------------------------
p4 = panels[3]["rect"]
draw.rectangle([(p4[0] + 15, p4[1] + 45), (p4[2] - 15, p4[3] - 15)], fill=(18, 26, 42, 255))

# HUD Showcase Top
hud_x = p4[0] + 60
hud_y = p4[1] + 65
draw.rectangle([(hud_x, hud_y), (p4[2] - 60, hud_y + 45)], fill=(30, 27, 75, 230), outline=(99, 102, 241, 255), width=2)
draw.text((hud_x + 20, hud_y + 12), "TEMPO: 24.3s", fill=(56, 189, 248, 255), font=font_h2)
draw.text((hud_x + 190, hud_y + 12), "CUCCOS: 10 / 10", fill=(250, 204, 21, 255), font=font_h2)
draw.text((hud_x + 360, hud_y + 12), "ROUND 10/10", fill=(244, 63, 94, 255), font=font_h2)

# Victory Trophy Banner
vic_y = hud_y + 65
draw.rectangle([(hud_x, vic_y), (p4[2] - 60, vic_y + 50)], fill=(20, 83, 45, 240), outline=(34, 197, 94, 255), width=2)
draw.text((hud_x + 50, vic_y + 8), "VITORIA SUPREMA! TODAS AS CUCCOS RESGATADAS!", fill=(220, 252, 231, 255), font=font_h2)
draw.text((hud_x + 110, vic_y + 28), "+200 Rupees  |  +100 Conchas Misteriosas", fill=(253, 224, 71, 255), font=font_body)

# Heart Piece Award Display
hp_cx = p4[0] + 130
hp_cy = p4[1] + 240

# Floating Heart Container Piece
draw.polygon([(hp_cx, hp_cy + 30), (hp_cx - 35, hp_cy - 10), (hp_cx - 35, hp_cy - 30),
              (hp_cx, hp_cy - 10), (hp_cx + 35, hp_cy - 30), (hp_cx + 35, hp_cy - 10)],
             fill=(239, 68, 68, 255), outline=(254, 202, 202, 255), width=3)
# 4th piece glowing quadrant
draw.arc([(hp_cx - 28, hp_cy - 25), (hp_cx + 28, hp_cy + 25)], 0, 90, fill=(254, 240, 138, 255), width=4)
draw.text((hp_cx - 55, hp_cy + 42), "PEDACO DE CORACAO", fill=(248, 113, 113, 255), font=font_small)

# Text List of Rewards
rw_x = p4[0] + 240
rw_y = p4[1] + 190
draw.text((rw_x, rw_y), "STATUS DO MINIGAME:", fill=(251, 191, 36, 255), font=font_h2)
records = [
    "✔ 10 de 10 Rondas Concluidas com Sucesso",
    "✔ Pedaço de Coração Canônico Permanente Obtido",
    "✔ Concedido Titulo de Mestre Cucco de Hyrule",
    "✔ Persistencia Automatica no Savegame (.dat)"
]
for r_i, rec in enumerate(records):
    draw.text((rw_x, rw_y + 26 + r_i * 20), rec, fill=(241, 245, 249, 255), font=font_body)

# Save image
os.makedirs(os.path.dirname(IMG_PATH), exist_ok=True)
img.save(IMG_PATH, "PNG")
print("Visual validation showcase gerado com sucesso em:", IMG_PATH)
