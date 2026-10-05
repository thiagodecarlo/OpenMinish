import math
import os
from PIL import Image, ImageDraw, ImageFont

# Canvas dimensions
WIDTH = 1200
HEIGHT = 800
IMG_PATH = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a\figurine_gallery_showcase.png"

img = Image.new("RGBA", (WIDTH, HEIGHT), (18, 16, 24, 255))
draw = ImageDraw.Draw(img)

# Helper font fallback
try:
    font_title = ImageFont.truetype("arial.ttf", 26)
    font_h2 = ImageFont.truetype("arialbd.ttf", 18)
    font_body = ImageFont.truetype("arial.ttf", 13)
    font_small = ImageFont.truetype("arial.ttf", 11)
    font_large_num = ImageFont.truetype("arialbd.ttf", 32)
except:
    font_title = ImageFont.load_default()
    font_h2 = ImageFont.load_default()
    font_body = ImageFont.load_default()
    font_small = ImageFont.load_default()
    font_large_num = ImageFont.load_default()

# Header
draw.rectangle([(0, 0), (WIDTH, 70)], fill=(30, 27, 46, 255))
draw.line([(0, 70), (WIDTH, 70)], fill=(245, 158, 11, 255), width=2)
draw.text((25, 12), "OPENMINISH - SPRINT 6: ATO VI", fill=(245, 158, 11, 255), font=font_small)
draw.text((25, 28), "GALERIA DO CARLOV & SISTEMA GACHA DE ESTATUETAS", fill=(255, 255, 255, 255), font=font_title)
draw.text((WIDTH - 340, 26), "Zero-ROM C11 | 136 Trofeus 3D | Medalha Carlov", fill=(148, 163, 184, 255), font=font_body)

# 4 Panels grid
panels = [
    {"rect": (25, 85, 585, 420), "title": "1. MAQUINA GACHA & APOSTA DE CONCHAS", "badge": "MISTERIOUS SHELLS"},
    {"rect": (615, 85, 1175, 420), "title": "2. DISPENSACAO & REVELACAO DE CAPSULA", "badge": "RARIDADE & ANIMACOES"},
    {"rect": (25, 440, 585, 775), "title": "3. INSPECAO 3D & LORE DAS MINIATURAS", "badge": "136 ESTATUETAS"},
    {"rect": (615, 440, 1175, 775), "title": "4. MEDALHA DE CARLOV & SALAO DOS MESTRES", "badge": "RECOMPENSA 100%"}
]

for p in panels:
    x0, y0, x1, y1 = p["rect"]
    draw.rectangle([(x0, y0), (x1, y1)], fill=(28, 25, 38, 255), outline=(60, 55, 80, 255), width=2)
    # Header bar of panel
    draw.rectangle([(x0, y0), (x1, y0 + 35)], fill=(38, 34, 54, 255))
    draw.line([(x0, y0 + 35), (x1, y0 + 35)], fill=(75, 70, 100, 255), width=1)
    draw.text((x0 + 15, y0 + 8), p["title"], fill=(255, 255, 255, 255), font=font_h2)
    # Badge
    bw = len(p["badge"]) * 7 + 16
    draw.rectangle([(x1 - bw - 15, y0 + 7), (x1 - 15, y0 + 27)], fill=(245, 158, 11, 40), outline=(245, 158, 11, 200))
    draw.text((x1 - bw - 8, y0 + 9), p["badge"], fill=(245, 158, 11, 255), font=font_small)

# -------------------------------------------------------------
# Panel 1: Gacha Machine & Shell Betting
# -------------------------------------------------------------
p1 = panels[0]["rect"]
cx = p1[0] + 160
cy = p1[1] + 210

# Tree trunk workshop background
draw.rectangle([(p1[0] + 15, p1[1] + 45), (p1[2] - 15, p1[3] - 15)], fill=(20, 17, 28, 255))

# Gacha Glass Dome
draw.ellipse([(cx - 70, cy - 120), (cx + 70, cy + 20)], fill=(56, 189, 248, 50), outline=(56, 189, 248, 200), width=2)
# Inside colorful capsules
colors = [(239, 68, 68), (59, 130, 246), (34, 197, 94), (234, 179, 8), (168, 85, 247)]
for i, col in enumerate(colors):
    ox = cx + int(35 * math.cos(i * 1.3))
    oy = cy - 50 + int(30 * math.sin(i * 1.5))
    draw.ellipse([(ox - 14, oy - 14), (ox + 14, oy + 14)], fill=col, outline=(255, 255, 255, 180), width=1)
    draw.chord([(ox - 14, oy - 14), (ox + 14, oy + 14)], 0, 180, fill=(240, 240, 240, 220))

# Gacha Red Metal Base
draw.polygon([(cx - 85, cy + 15), (cx + 85, cy + 15), (cx + 95, cy + 120), (cx - 95, cy + 120)], fill=(220, 38, 38, 255), outline=(153, 27, 27, 255), width=2)
# Golden crank
draw.ellipse([(cx - 18, cy + 50), (cx + 18, cy + 86)], fill=(245, 158, 11, 255), outline=(217, 119, 6, 255), width=2)
draw.rectangle([(cx - 6, cy + 62), (cx + 36, cy + 74)], fill=(251, 191, 36, 255))
draw.ellipse([(cx + 32, cy + 56), (cx + 48, cy + 80)], fill=(180, 83, 9, 255))

# Capsule chute
draw.rectangle([(cx - 30, cy + 95), (cx + 30, cy + 120)], fill=(15, 23, 42, 255), outline=(220, 38, 38, 255))

# UI Right Side: Shells betting & odds
rx = p1[0] + 330
ry = p1[1] + 65
draw.rectangle([(rx, ry), (p1[2] - 25, ry + 75)], fill=(34, 30, 48, 255), outline=(75, 70, 100, 255))
draw.text((rx + 15, ry + 10), "CONCHAS DISPONIVEIS", fill=(148, 163, 184, 255), font=font_small)
draw.text((rx + 15, ry + 26), "482 / 999", fill=(56, 189, 248, 255), font=font_large_num)

ry2 = ry + 90
draw.rectangle([(rx, ry2), (p1[2] - 25, ry2 + 75)], fill=(34, 30, 48, 255), outline=(75, 70, 100, 255))
draw.text((rx + 15, ry2 + 10), "CONCHAS APOSTADAS", fill=(148, 163, 184, 255), font=font_small)
draw.text((rx + 15, ry2 + 26), "[<]  12  [>]", fill=(245, 158, 11, 255), font=font_large_num)

ry3 = ry2 + 90
draw.rectangle([(rx, ry3), (p1[2] - 25, ry3 + 75)], fill=(34, 30, 48, 255), outline=(75, 70, 100, 255))
draw.text((rx + 15, ry3 + 10), "CHANCE DE NOVA FIGURA", fill=(148, 163, 184, 255), font=font_small)
# Gauge bar
draw.rectangle([(rx + 15, ry3 + 30), (p1[2] - 40, ry3 + 48)], fill=(15, 23, 42, 255), outline=(51, 65, 85, 255))
draw.rectangle([(rx + 15, ry3 + 30), (rx + 15 + int((p1[2] - 40 - rx - 15) * 0.88), ry3 + 48)], fill=(34, 197, 94, 255))
draw.text((rx + 18, ry3 + 52), "88.0%  (Quase Garantido!)", fill=(74, 222, 128, 255), font=font_body)

# -------------------------------------------------------------
# Panel 2: Capsule Dispense & Reveal
# -------------------------------------------------------------
p2 = panels[1]["rect"]
cx2 = (p2[0] + p2[2]) // 2
cy2 = p2[1] + 195

draw.rectangle([(p2[0] + 15, p2[1] + 45), (p2[2] - 15, p2[3] - 15)], fill=(16, 14, 24, 255))

# Golden rays burst
for angle in range(0, 360, 20):
    rad = math.radians(angle)
    x_end = cx2 + int(140 * math.cos(rad))
    y_end = cy2 - 20 + int(140 * math.sin(rad))
    draw.line([(cx2, cy2 - 20), (x_end, y_end)], fill=(245, 158, 11, 70), width=2)

# Open capsule halves
draw.arc([(cx2 - 90, cy2 - 20), (cx2 - 40, cy2 + 30)], 120, 300, fill=(239, 68, 68, 255), width=6)
draw.arc([(cx2 + 40, cy2 - 20), (cx2 + 90, cy2 + 30)], 240, 60, fill=(248, 250, 252, 255), width=6)

# Glowing 3D Trophy Emerging (Link Trophy)
# Pedestal
draw.ellipse([(cx2 - 50, cy2 + 50), (cx2 + 50, cy2 + 75)], fill=(71, 85, 105, 255), outline=(148, 163, 184, 255), width=2)
draw.polygon([(cx2 - 50, cy2 + 62), (cx2 + 50, cy2 + 62), (cx2 + 40, cy2 + 85), (cx2 - 40, cy2 + 85)], fill=(51, 65, 85, 255))

# Link Figurine Mockup
draw.ellipse([(cx2 - 18, cy2 - 60), (cx2 + 18, cy2 - 24)], fill=(254, 202, 202, 255)) # Face
draw.polygon([(cx2 - 25, cy2 - 45), (cx2 + 25, cy2 - 45), (cx2 + 10, cy2 - 85), (cx2 - 5, cy2 - 75)], fill=(34, 197, 94, 255)) # Green Hat
draw.rectangle([(cx2 - 20, cy2 - 24), (cx2 + 20, cy2 + 25)], fill=(22, 163, 74, 255)) # Tunic
draw.rectangle([(cx2 + 15, cy2 - 20), (cx2 + 35, cy2 + 20)], fill=(59, 130, 246, 255)) # Shield
draw.line([(cx2 - 25, cy2 + 10), (cx2 - 35, cy2 - 50)], fill=(226, 232, 240, 255), width=4) # Sword

# Sparkles
for sp in [(cx2 - 80, cy2 - 70), (cx2 + 85, cy2 - 60), (cx2 - 60, cy2 + 20), (cx2 + 70, cy2 + 30)]:
    draw.text(sp, "*", fill=(251, 191, 36, 255), font=font_h2)

# Banner notification
draw.rectangle([(p2[0] + 60, p2[3] - 70), (p2[2] - 60, p2[3] - 25)], fill=(22, 101, 52, 220), outline=(34, 197, 94, 255), width=2)
draw.text((p2[0] + 90, p2[3] - 62), "NOVA FIGURA OBTIDA!  #001: Link (O Heroi)", fill=(240, 253, 244, 255), font=font_h2)
draw.text((p2[0] + 160, p2[3] - 42), "Adicionada com sucesso a sua colecao!", fill=(187, 247, 208, 255), font=font_small)

# -------------------------------------------------------------
# Panel 3: 3D Trophy Inspection & Lore Viewer
# -------------------------------------------------------------
p3 = panels[2]["rect"]
draw.rectangle([(p3[0] + 15, p3[1] + 45), (p3[2] - 15, p3[3] - 15)], fill=(18, 16, 26, 255))

# Trophy Showcase Left
t_cx = p3[0] + 130
t_cy = p3[1] + 190

# 3D Pedestal
draw.ellipse([(t_cx - 65, t_cy + 40), (t_cx + 65, t_cy + 75)], fill=(100, 116, 139, 255), outline=(148, 163, 184, 255), width=2)
draw.polygon([(t_cx - 65, t_cy + 57), (t_cx + 65, t_cy + 57), (t_cx + 55, t_cy + 95), (t_cx - 55, t_cy + 95)], fill=(71, 85, 105, 255))
draw.text((t_cx - 24, t_cy + 65), "CARLOV", fill=(226, 232, 240, 255), font=font_small)

# Shaded Boss Figurine (027: Feiticeiro Vaati)
draw.ellipse([(t_cx - 16, t_cy - 45), (t_cx + 16, t_cy - 15)], fill=(192, 132, 252, 255)) # Head
draw.polygon([(t_cx - 25, t_cy - 15), (t_cx + 25, t_cy - 15), (t_cx + 35, t_cy + 40), (t_cx - 35, t_cy + 40)], fill=(88, 28, 135, 255)) # Sorcerer Robe
# Malice Orbs orbiting
for orb_idx in range(4):
    o_ang = orb_idx * 1.57 + 0.4
    ox = t_cx + int(45 * math.cos(o_ang))
    oy = t_cy - 10 + int(20 * math.sin(o_ang))
    draw.ellipse([(ox - 8, oy - 8), (ox + 8, oy + 8)], fill=(239, 68, 68, 255), outline=(254, 202, 202, 255))

# Rotation indicators
draw.text((t_cx - 60, t_cy + 105), "[<-] Girar Trofeu [->]", fill=(148, 163, 184, 255), font=font_small)

# Lore Card Right
lx = p3[0] + 250
ly = p3[1] + 60
draw.rectangle([(lx, ly), (p3[2] - 25, p3[3] - 25)], fill=(30, 27, 44, 255), outline=(99, 102, 241, 200), width=1)
draw.rectangle([(lx, ly), (p3[2] - 25, ly + 35)], fill=(49, 46, 129, 255))
draw.text((lx + 15, ly + 8), "CAT: CHEFES & VILOES", fill=(199, 210, 254, 255), font=font_small)
draw.text((lx + 15, ly + 45), "#027: Feiticeiro Vaati", fill=(255, 255, 255, 255), font=font_h2)

lore_text = [
    "O jovem aprendiz Minish corrompido pela",
    "ambicao e fascinio pela malicia no coracao",
    "dos homens. Roubou o Gorro dos Desejos,",
    "amaldicoou seu mestre Ezlo e transformou",
    "a Princesa Zelda em pedra para usurpar a",
    "sagrada Forca da Luz de Hyrule.",
    "",
    "Status: DERROTADO NO SANTUARIO"
]
for idx, line in enumerate(lore_text):
    col = (244, 63, 94, 255) if "DERROTADO" in line else (203, 213, 225, 255)
    draw.text((lx + 15, ly + 80 + idx * 19), line, fill=col, font=font_body)

# -------------------------------------------------------------
# Panel 4: Carlov Medal & 100% Completion
# -------------------------------------------------------------
p4 = panels[3]["rect"]
draw.rectangle([(p4[0] + 15, p4[1] + 45), (p4[2] - 15, p4[3] - 15)], fill=(24, 20, 16, 255))

# Master Hall stats
mx = p4[0] + 35
my = p4[1] + 60
draw.text((mx, my), "PROGRESSO TOTAL DA COLECAO", fill=(245, 158, 11, 255), font=font_h2)

# Full completion bar
draw.rectangle([(mx, my + 30), (p4[2] - 35, my + 55)], fill=(41, 37, 36, 255), outline=(120, 113, 108, 255))
draw.rectangle([(mx, my + 30), (p4[2] - 35, my + 55)], fill=(245, 158, 11, 255))
draw.text((mx + 190, my + 34), "136 / 136 (100% COMPLETO!)", fill=(12, 10, 9, 255), font=font_body)

# Carlov Medal Display
med_cx = p4[0] + 140
med_cy = p4[1] + 200

# Ribbon
draw.polygon([(med_cx - 30, med_cy - 70), (med_cx - 10, med_cy - 20), (med_cx - 50, med_cy - 20)], fill=(220, 38, 38, 255))
draw.polygon([(med_cx + 30, med_cy - 70), (med_cx + 50, med_cy - 20), (med_cx + 10, med_cy - 20)], fill=(220, 38, 38, 255))

# Golden Medal
draw.ellipse([(med_cx - 45, med_cy - 25), (med_cx + 45, med_cy + 65)], fill=(245, 158, 11, 255), outline=(251, 191, 36, 255), width=4)
draw.ellipse([(med_cx - 35, med_cy - 15), (med_cx + 35, med_cy + 55)], fill=(217, 119, 6, 255))
draw.text((med_cx - 16, med_cy + 5), "★", fill=(255, 255, 255, 255), font=font_large_num)
draw.text((med_cx - 55, med_cy + 75), "MEDALHA DE CARLOV", fill=(251, 191, 36, 255), font=font_small)

# Unlocked rewards list
rx4 = p4[0] + 250
ry4 = p4[1] + 130
draw.rectangle([(rx4, ry4), (p4[2] - 30, p4[3] - 30)], fill=(41, 37, 36, 255), outline=(245, 158, 11, 150), width=1)
draw.text((rx4 + 15, ry4 + 12), "RECOMPENSAS CONCEDIDAS:", fill=(251, 191, 36, 255), font=font_h2)

rewards = [
    "✔ Chave da Casa dos Herbologistas",
    "✔ Acesso ao Tocador de Musica Secreto (Sound Test)",
    "✔ Pedaco de Coracao Lendario (+1 Coracao)",
    "✔ Trofeu Comemorativo de 100% no Perfil",
    "✔ Reconhecimento como Mestre Colecionador de Hyrule"
]
for r_i, rew in enumerate(rewards):
    draw.text((rx4 + 15, ry4 + 45 + r_i * 24), rew, fill=(245, 245, 244, 255), font=font_body)

# Save image
os.makedirs(os.path.dirname(IMG_PATH), exist_ok=True)
img.save(IMG_PATH, "PNG")
print("Visual validation showcase gerado com sucesso em:", IMG_PATH)
