import sys
import os
import math
from PIL import Image, ImageDraw, ImageFont

def create_showcase():
    W = 720
    H = 480
    img = Image.new('RGB', (W, H), (15, 23, 42))
    draw = ImageDraw.Draw(img)

    # Header Banner
    draw.rectangle([0, 0, W, 48], fill=(30, 41, 59))
    draw.line([0, 48, W, 48], fill=(217, 119, 6), width=2)
    draw.text((20, 14), "OpenMinish - Feature: Hyrule Field Expansion (South & North Fields, Moblin, Peahat, Malon)", fill=(248, 250, 252))

    # --- PANEL 1: SOUTH HYRULE FIELD (LON LON RANCH & MALON) ---
    p1_x, p1_y, p1_w, p1_h = 20, 65, 335, 220
    draw.rectangle([p1_x, p1_y, p1_x + p1_w, p1_y + p1_h], fill=(34, 197, 94)) # Grass base
    draw.rectangle([p1_x, p1_y, p1_x + p1_w, p1_y + p1_h], outline=(20, 83, 45), width=2)

    # Dirt paths (North-South & East towards Minish Woods)
    draw.rectangle([p1_x + 140, p1_y, p1_x + 180, p1_y + p1_h], fill=(217, 119, 6))
    draw.rectangle([p1_x + 180, p1_y + 90, p1_x + p1_w, p1_y + 130], fill=(217, 119, 6))

    # Lon Lon Ranch Wooden Fence & Gate
    fence_x = p1_x + 190
    fence_y = p1_y + 130
    draw.rectangle([fence_x, fence_y, fence_x + 130, fence_y + 80], outline=(120, 53, 15), width=4)
    # Fence Gate opening
    draw.rectangle([fence_x + 30, fence_y, fence_x + 60, fence_y + 4], fill=(217, 119, 6))
    draw.text((fence_x + 10, fence_y + 10), "Fazenda Lon Lon", fill=(69, 26, 3))

    # Malon NPC at fence gate
    mx = fence_x + 40
    my = fence_y + 20
    # Red hair & yellow bandana
    draw.ellipse([mx - 8, my - 12, mx + 8, my + 4], fill=(234, 88, 12))
    draw.rectangle([mx - 6, my - 10, mx + 6, my - 6], fill=(250, 204, 21)) # Bandana
    # Face
    draw.ellipse([mx - 6, my - 6, mx + 6, my + 4], fill=(253, 232, 205))
    # Dress & apron
    draw.rectangle([mx - 7, my + 4, mx + 7, my + 18], fill=(59, 130, 246))
    draw.rectangle([mx - 4, my + 7, mx + 4, my + 16], fill=(254, 243, 199))
    # Kinstone Blue bubble above Malon
    draw.ellipse([mx - 8, my - 28, mx + 8, my - 14], fill=(255, 255, 255), outline=(212, 175, 55), width=2)
    draw.ellipse([mx - 5, my - 25, mx + 5, my - 17], fill=(59, 130, 246))
    draw.text((mx - 15, my + 22), "Malon [A]", fill=(255, 255, 255))

    # Moblin enemy patrolling plains
    mox = p1_x + 80
    moy = p1_y + 110
    # Pig face & helm
    draw.rectangle([mox - 8, moy - 10, mox + 8, moy - 4], fill=(120, 53, 15)) # Helm
    draw.rectangle([mox - 9, moy - 4, mox + 9, moy + 6], fill=(251, 146, 60)) # Pig skin
    draw.rectangle([mox - 4, moy, mox + 4, moy + 4], fill=(244, 63, 94)) # Snout
    # Leather Armor
    draw.rectangle([mox - 8, moy + 6, mox + 8, moy + 16], fill=(180, 83, 9))
    # Spear in hand
    draw.line([mox + 10, moy - 8, mox + 10, moy + 22], fill=(120, 53, 15), width=2)
    draw.polygon([(mox + 10, moy - 14), (mox + 7, moy - 8), (mox + 13, moy - 8)], fill=(226, 232, 240))
    draw.text((mox - 18, moy + 20), "Moblin (Lanca)", fill=(248, 250, 252))

    # Peahat flying in the air
    px = p1_x + 90
    py = p1_y + 40
    # Shadow on ground
    draw.ellipse([px - 8, py + 18, px + 8, py + 24], fill=(0, 0, 0, 100))
    # Rotating leaves
    draw.ellipse([px - 14, py - 6, px + 14, py + 6], fill=(34, 197, 94), outline=(21, 128, 61))
    draw.ellipse([px - 6, py - 14, px + 6, py + 14], fill=(34, 197, 94), outline=(21, 128, 61))
    draw.ellipse([px - 7, py - 7, px + 7, py + 7], fill=(234, 179, 8))
    draw.text((px - 16, py - 26), "Peahat (Voo)", fill=(254, 240, 138))

    # Link in South Field
    lx = p1_x + 160
    ly = p1_y + 110
    draw.rectangle([lx - 6, ly - 8, lx + 6, ly - 2], fill=(22, 163, 74)) # Hat
    draw.ellipse([lx - 5, ly - 4, lx + 5, ly + 2], fill=(254, 215, 170)) # Face
    draw.rectangle([lx - 6, ly + 2, lx + 6, ly + 12], fill=(22, 163, 74)) # Tunic
    draw.rectangle([lx - 4, ly + 12, lx + 4, ly + 16], fill=(245, 158, 11))

    draw.text((p1_x + 10, p1_y + 8), "South Hyrule Field (Campos Sul)", fill=(255, 255, 255))

    # --- PANEL 2: NORTH HYRULE FIELD (ROAD TO MT CRENEL & CASTLE) ---
    p2_x, p2_y, p2_w, p2_h = 370, 65, 335, 220
    draw.rectangle([p2_x, p2_y, p2_x + p2_w, p2_y + p2_h], fill=(34, 197, 94))
    draw.rectangle([p2_x, p2_y, p2_x + p2_w, p2_y + p2_h], outline=(20, 83, 45), width=2)

    # Mountain crags on West border
    draw.rectangle([p2_x, p2_y, p2_x + 40, p2_y + 80], fill=(100, 116, 139))
    draw.rectangle([p2_x, p2_y + 130, p2_x + 40, p2_y + p2_h], fill=(100, 116, 139))
    # Dirt path to Mt Crenel (West opening)
    draw.rectangle([p2_x, p2_y + 80, p2_x + 170, p2_y + 130], fill=(217, 119, 6))
    # North-South main road to Hyrule Castle
    draw.rectangle([p2_x + 140, p2_y, p2_x + 180, p2_y + p2_h], fill=(217, 119, 6))

    # Crenel Road Sign
    sx = p2_x + 55
    sy = p2_y + 70
    draw.rectangle([sx + 6, sy + 10, sx + 10, sy + 22], fill=(120, 53, 15)) # Post
    draw.rectangle([sx, sy, sx + 24, sy + 12], fill=(217, 119, 6), outline=(120, 53, 15), width=2)
    draw.polygon([(sx + 5, sy + 6), (sx + 10, sy + 3), (sx + 10, sy + 9)], fill=(69, 26, 3)) # Arrow pointing west
    draw.text((sx - 15, sy - 15), "Placa Crenel", fill=(255, 255, 255))

    # Secret Entrance / Cracked Wall
    cx = p2_x + 80
    cy = p2_y + 10
    draw.rectangle([cx, cy, cx + 40, cy + 24], fill=(100, 116, 139), outline=(51, 65, 85), width=2)
    # Cracks
    draw.line([cx + 15, cy + 5, cx + 22, cy + 18], fill=(30, 41, 59), width=2)
    draw.line([cx + 22, cy + 18, cx + 28, cy + 10], fill=(30, 41, 59), width=2)
    draw.text((cx - 10, cy + 26), "Parede Rachada [Bomba]", fill=(226, 232, 240))

    # Two Moblins on North Field
    for i, (mx_pos, my_pos) in enumerate([(p2_x + 230, p2_y + 70), (p2_x + 230, p2_y + 150)]):
        draw.rectangle([mx_pos - 8, my_pos - 10, mx_pos + 8, my_pos - 4], fill=(120, 53, 15))
        draw.rectangle([mx_pos - 9, my_pos - 4, mx_pos + 9, my_pos + 6], fill=(251, 146, 60))
        draw.rectangle([mx_pos - 8, my_pos + 6, mx_pos + 8, my_pos + 16], fill=(180, 83, 9))
        draw.line([mx_pos + 10, my_pos - 8, mx_pos + 10, my_pos + 22], fill=(120, 53, 15), width=2)
        draw.polygon([(mx_pos + 10, my_pos - 14), (mx_pos + 7, my_pos - 8), (mx_pos + 13, my_pos - 8)], fill=(226, 232, 240))
        draw.text((mx_pos - 15, my_pos + 20), f"Moblin #{i+1}", fill=(248, 250, 252))

    draw.text((p2_x + 10, p2_y + 8), "North Hyrule Field (Campos Norte)", fill=(255, 255, 255))

    # --- PANEL 3: AUTHENTIC MALON DIALOGUE & PORTRAIT (GBA TMC STYLE) ---
    p3_x, p3_y, p3_w, p3_h = 20, 305, 685, 155
    # Green translucent dialogue box with gold border
    draw.rectangle([p3_x, p3_y, p3_x + p3_w, p3_y + p3_h], fill=(10, 38, 18), outline=(212, 175, 55), width=3)
    draw.rectangle([p3_x + 4, p3_y + 4, p3_x + p3_w - 4, p3_y + p3_h - 4], outline=(245, 158, 11), width=1)

    # Portrait Frame
    port_x = p3_x + 16
    port_y = p3_y + 18
    port_s = 96
    draw.rectangle([port_x, port_y, port_x + port_s, port_y + port_s], fill=(5, 16, 8), outline=(212, 175, 55), width=2)

    # Large Malon Portrait
    # Red hair
    draw.ellipse([port_x + 10, port_y + 8, port_x + port_s - 10, port_y + port_s - 15], fill=(234, 88, 12))
    # Yellow Bandana
    draw.rectangle([port_x + 22, port_y + 16, port_x + port_s - 22, port_y + 26], fill=(250, 204, 21))
    # Soft Peach Face
    draw.ellipse([port_x + 24, port_y + 26, port_x + port_s - 24, port_y + port_s - 25], fill=(253, 232, 205))
    # Cute Blue Eyes
    draw.ellipse([port_x + 36, port_y + 40, port_x + 44, port_y + 52], fill=(30, 41, 59))
    draw.ellipse([port_x + 56, port_y + 40, port_x + 64, port_y + 52], fill=(30, 41, 59))
    draw.point((port_x + 38, port_y + 43), fill=(255, 255, 255))
    draw.point((port_x + 58, port_y + 43), fill=(255, 255, 255))
    # Blushing pink cheeks
    draw.ellipse([port_x + 30, port_y + 52, port_x + 40, port_y + 58], fill=(251, 113, 133))
    draw.ellipse([port_x + 60, port_y + 52, port_x + 70, port_y + 58], fill=(251, 113, 133))
    # Smiling mouth
    draw.arc([port_x + 44, port_y + 54, port_x + 56, port_y + 64], start=0, end=180, fill=(136, 19, 55), width=2)
    # Blue Bodice & White Scarf
    draw.rectangle([port_x + 20, port_y + port_s - 25, port_x + port_s - 20, port_y + port_s], fill=(59, 130, 246))
    draw.polygon([(port_x + 38, port_y + port_s - 25), (port_x + 48, port_y + port_s - 10), (port_x + 58, port_y + port_s - 25)], fill=(254, 243, 199))

    # Name Tag Badge
    badge_x = port_x + port_s + 20
    badge_y = p3_y + 14
    draw.rectangle([badge_x, badge_y, badge_x + 90, badge_y + 24], fill=(20, 83, 45), outline=(212, 175, 55), width=2)
    draw.text((badge_x + 14, badge_y + 4), "Malon", fill=(251, 146, 60))

    # Dialogue text
    txt_x = badge_x
    txt_y = badge_y + 36
    draw.text((txt_x, txt_y), '"Ola viajante! Sou a Malon da Fazenda Lon Lon. Que dia lindo nos campos!"', fill=(248, 250, 252))
    draw.text((txt_x, txt_y + 22), '"Meu pai Talon perdeu a chave do nosso portao quando foi para a cidade..."', fill=(248, 250, 252))
    draw.text((txt_x, txt_y + 44), '"Eu tenho uma Kinstone azul especial! Se unir com a sua, quem sabe encontramos a chave!"', fill=(254, 240, 138))

    # Kinstone Indicator Icon
    draw.ellipse([p3_x + p3_w - 48, p3_y + 20, p3_x + p3_w - 18, p3_y + 50], fill=(59, 130, 246), outline=(212, 175, 55), width=2)
    draw.text((p3_x + p3_w - 42, p3_y + 26), "K", fill=(255, 255, 255))
    draw.text((p3_x + p3_w - 85, p3_y + 56), "[L] Kinstone", fill=(96, 165, 250))

    out_path = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be\hyrule_field_expansion_showcase.png"
    img.save(out_path)
    print(f"[OK] Showcase saved to {out_path}")

if __name__ == '__main__':
    create_showcase()
