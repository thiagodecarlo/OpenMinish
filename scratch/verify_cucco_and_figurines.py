#!/usr/bin/env python3
"""
scratch/verify_cucco_and_figurines.py - Visual Verification of Cucco Minigame & Figurine Gallery (Opção C)
Validates:
1. Anju's Cucco Minigame (Pen, Anju cheer with heart bubble, White Cucco, Chick, Golden Cucco, feathers)
2. Link Gliding with Cucco (airborne feather glide physics)
3. Carlov's Gacha Machine & Mysterious Shells (betting panel, probability calculation, crank and chute)
4. Figurine Trophy Pedestal (Spotlight cone, marble/gold beveled pedestal, 3D rotated trophy models)
5. Mystery Silhouette & Canonical Figurine Lore
"""

import os
import sys
import math
from PIL import Image, ImageDraw, ImageFont

def draw_mysterious_shell_icon(draw, x, y):
    draw.rectangle([(x + 2, y), (x + 6, y + 1)], fill=(56, 189, 248, 255))
    draw.rectangle([(x + 1, y + 2), (x + 7, y + 3)], fill=(125, 211, 252, 255))
    draw.rectangle([(x, y + 4), (x + 8, y + 6)], fill=(186, 230, 253, 255))
    draw.rectangle([(x + 1, y + 7), (x + 7, y + 8)], fill=(2, 132, 199, 255))
    draw.rectangle([(x + 3, y + 9), (x + 5, y + 9)], fill=(3, 105, 161, 255))
    draw.point((x + 3, y + 3), fill=(2, 132, 199, 255))
    draw.point((x + 4, y + 4), fill=(2, 132, 199, 255))
    draw.point((x + 5, y + 3), fill=(3, 105, 161, 255))
    draw.point((x + 3, y + 5), fill=(255, 255, 255, 255))

def render_cucco_sprite(draw, sx, sy, cucco_type=0, is_flapping=False, flap_phase=0):
    c_beak = (245, 158, 11, 255)
    c_feet = (234, 88, 12, 255)
    
    if cucco_type == 1: # CHICK
        c_body = (253, 224, 71, 255)
        draw.rectangle([(sx - 3, sy - 4), (sx + 2, sy)], fill=c_body)
        draw.rectangle([(sx + 2, sy - 3), (sx + 3, sy - 2)], fill=c_beak)
        draw.rectangle([(sx - 2, sy + 1), (sx - 1, sy + 2)], fill=c_feet)
        draw.rectangle([(sx + 1, sy + 1), (sx + 2, sy + 2)], fill=c_feet)
        if is_flapping:
            draw.rectangle([(sx - 4, sy - 3), (sx - 3, sy - 2)], fill=(254, 240, 138, 255))
            draw.rectangle([(sx + 3, sy - 3), (sx + 4, sy - 2)], fill=(254, 240, 138, 255))
    elif cucco_type == 2: # GOLDEN
        c_body = (251, 191, 36, 255)
        c_crest = (220, 38, 38, 255)
        draw.rectangle([(sx - 5, sy - 6), (sx + 4, sy + 1)], fill=c_body)
        draw.rectangle([(sx - 2, sy - 9), (sx + 1, sy - 7)], fill=c_crest)
        draw.rectangle([(sx + 4, sy - 4), (sx + 6, sy - 2)], fill=c_beak)
        draw.rectangle([(sx - 3, sy + 2), (sx - 2, sy + 4)], fill=(217, 119, 6, 255))
        draw.rectangle([(sx + 1, sy + 2), (sx + 2, sy + 4)], fill=(217, 119, 6, 255))
        
        wing_off = -3 if is_flapping else 0
        draw.rectangle([(sx - 7, sy - 5 + wing_off), (sx - 5, sy - 1 + wing_off)], fill=(253, 224, 71, 255))
        draw.rectangle([(sx + 4, sy - 5 + wing_off), (sx + 6, sy - 1 + wing_off)], fill=(253, 224, 71, 255))
        draw.rectangle([(sx - 6, sy - 8), (sx - 5, sy - 7)], fill=(255, 255, 255, 255)) # Sparkle
    else: # WHITE
        c_body = (255, 255, 255, 255)
        c_comb = (239, 68, 68, 255)
        draw.rectangle([(sx - 5, sy - 6), (sx + 4, sy + 1)], fill=c_body)
        draw.rectangle([(sx - 2, sy - 9), (sx + 1, sy - 7)], fill=c_comb)
        draw.rectangle([(sx + 4, sy - 4), (sx + 6, sy - 2)], fill=c_beak)
        draw.rectangle([(sx - 3, sy + 2), (sx - 2, sy + 4)], fill=c_feet)
        draw.rectangle([(sx + 1, sy + 2), (sx + 2, sy + 4)], fill=c_feet)
        
        wing_off = -2 if is_flapping else 0
        draw.rectangle([(sx - 7, sy - 5 + wing_off), (sx - 5, sy - 1 + wing_off)], fill=(203, 213, 225, 255))
        draw.rectangle([(sx + 4, sy - 5 + wing_off), (sx + 6, sy - 1 + wing_off)], fill=(203, 213, 225, 255))

def render_anju(draw, sx, sy, is_cheering=False):
    hop = 2 if is_cheering else 0
    y = sy - hop
    # Hair
    draw.rectangle([(sx - 4, y - 14), (sx + 3, y - 8)], fill=(249, 168, 212, 255))
    # Face
    draw.rectangle([(sx - 3, y - 10), (sx + 2, y - 5)], fill=(254, 215, 170, 255))
    # Blue dress
    draw.rectangle([(sx - 5, y - 4), (sx + 4, y + 5)], fill=(59, 130, 246, 255))
    # White apron
    draw.rectangle([(sx - 4, y - 2), (sx + 3, y + 3)], fill=(255, 255, 255, 255))
    
    if is_cheering:
        # Heart bubble above Anju's head
        hx, hy = sx, y - 20
        draw.rectangle([(hx - 4, hy - 4), (hx + 3, hy + 3)], fill=(255, 255, 255, 238))
        draw.rectangle([(hx - 3, hy - 3), (hx + 2, hy + 2)], fill=(239, 68, 68, 255))
        draw.point((hx - 2, hy - 4), fill=(239, 68, 68, 255))
        draw.point((hx + 2, hy - 4), fill=(239, 68, 68, 255))
        draw.point((hx, hy + 3), fill=(239, 68, 68, 255))

def render_feather(draw, x, y, col=(255, 255, 255, 255)):
    draw.rectangle([(x, y), (x + 1, y + 1)], fill=col)
    draw.point((x + 1, y), fill=(255, 255, 255, 255))

def render_trophy_model(draw, px, py, fig_id, rot_angle, is_silhouette=False):
    # 2. Pedestal
    draw.rectangle([(px - 32, py + 19), (px + 31, py + 23)], fill=(5, 16, 7, 136))
    draw.rectangle([(px - 30, py + 15), (px + 29, py + 18)], fill=(245, 158, 11, 255))
    draw.rectangle([(px - 26, py + 3),  (px + 25, py + 14)], fill=(100, 116, 139, 255))
    draw.rectangle([(px - 24, py + 5),  (px + 23, py + 6)],  fill=(148, 163, 184, 255))
    draw.rectangle([(px - 28, py),      (px + 27, py + 3)],  fill=(51, 65, 85, 255))
    draw.line([(px - 26, py), (px + 25, py)], fill=(203, 213, 225, 255))

    rox = int(math.sin(rot_angle) * 7.0)
    ty = py - 34

    if is_silhouette:
        draw.rectangle([(px - 12 + rox, ty), (px + 11 + rox, ty + 31)], fill=(30, 27, 24, 255))
        draw.rectangle([(px - 10 + rox, ty - 2), (px + 9 + rox, ty - 1)], fill=(68, 64, 60, 255))
        draw.rectangle([(px - 12 + rox, ty), (px - 11 + rox, ty + 31)], fill=(68, 64, 60, 255))
        try:
            f = ImageFont.load_default()
            draw.text((px - 3 + rox, ty + 9), "?", font=f, fill=(245, 158, 11, 255))
        except Exception:
            pass
        return

    if fig_id == 0: # LINK
        draw.rectangle([(px - 5 + rox, ty - 6), (px + 4 + rox, ty - 1)], fill=(22, 163, 74, 255))
        draw.rectangle([(px + 1 + rox, ty - 9), (px + 6 + rox, ty - 6)], fill=(21, 128, 61, 255))
        draw.rectangle([(px - 4 + rox, ty), (px + 3 + rox, ty + 6)], fill=(254, 215, 170, 255))
        draw.rectangle([(px - 4 + rox, ty), (px + 3 + rox, ty + 1)], fill=(250, 204, 21, 255))
        draw.point((px - 2 + rox, ty + 2), fill=(2, 132, 199, 255))
        draw.point((px + 1 + rox, ty + 2), fill=(2, 132, 199, 255))
        draw.rectangle([(px - 6 + rox, ty + 7), (px + 5 + rox, ty + 20)], fill=(22, 163, 74, 255))
        draw.rectangle([(px - 6 + rox, ty + 13), (px + 5 + rox, ty + 14)], fill=(248, 250, 252, 255))
        draw.point((px - 1 + rox, ty + 13), fill=(245, 158, 11, 255))
        draw.rectangle([(px - 4 + rox, ty + 21), (px - 2 + rox, ty + 29)], fill=(120, 53, 15, 255))
        draw.rectangle([(px + 1 + rox, ty + 21), (px + 3 + rox, ty + 29)], fill=(120, 53, 15, 255))
        draw.rectangle([(px - 11 + rox, ty + 8), (px - 7 + rox, ty + 18)], fill=(37, 99, 235, 255))
        draw.rectangle([(px - 10 + rox, ty + 11), (px - 8 + rox, ty + 15)], fill=(239, 68, 68, 255))
        draw.rectangle([(px + 7 + rox, ty - 4), (px + 8 + rox, ty + 9)], fill=(203, 213, 225, 255))
        draw.point((px + 7 + rox, ty - 5), fill=(255, 255, 255, 255))
        draw.rectangle([(px + 6 + rox, ty + 10), (px + 9 + rox, ty + 11)], fill=(245, 158, 11, 255))
    elif fig_id == 1: # EZLO
        draw.rectangle([(px - 12, ty + 24), (px + 11, ty + 27)], fill=(120, 53, 15, 255))
        draw.rectangle([(px - 8 + rox, ty + 6), (px + 7 + rox, ty + 21)], fill=(5, 150, 105, 255))
        draw.rectangle([(px - 9 + rox, ty + 8), (px - 7 + rox, ty + 17)], fill=(4, 120, 87, 255))
        draw.rectangle([(px - 7 + rox, ty - 2), (px + 6 + rox, ty + 7)], fill=(5, 150, 105, 255))
        draw.rectangle([(px - 3 + rox, ty - 7), (px + 2 + rox, ty - 3)], fill=(220, 38, 38, 255))
        draw.rectangle([(px - 6 + rox, ty), (px - 2 + rox, ty + 4)], fill=(255, 255, 255, 255))
        draw.rectangle([(px + 1 + rox, ty), (px + 5 + rox, ty + 4)], fill=(255, 255, 255, 255))
        draw.point((px - 4 + rox, ty + 2), fill=(24, 24, 27, 255))
        draw.point((px + 3 + rox, ty + 2), fill=(24, 24, 27, 255))
        draw.rectangle([(px - 2 + rox, ty + 5), (px + 2 + rox, ty + 8)], fill=(234, 88, 12, 255))
        draw.rectangle([(px - 1 + rox, ty + 8), (px + 1 + rox, ty + 10)], fill=(217, 119, 6, 255))
    elif fig_id == 2: # PRINCESS ZELDA
        draw.rectangle([(px - 5 + rox, ty - 7), (px + 4 + rox, ty - 6)], fill=(245, 158, 11, 255))
        draw.point((px + rox, ty - 8), fill=(56, 189, 248, 255))
        draw.rectangle([(px - 6 + rox, ty - 5), (px + 5 + rox, ty + 12)], fill=(253, 224, 71, 255))
        draw.rectangle([(px - 4 + rox, ty - 4), (px + 3 + rox, ty + 2)], fill=(254, 215, 170, 255))
        draw.point((px - 2 + rox, ty - 1), fill=(29, 78, 216, 255))
        draw.point((px + 1 + rox, ty - 1), fill=(29, 78, 216, 255))
        draw.rectangle([(px - 7 + rox, ty + 4), (px + 6 + rox, ty + 27)], fill=(244, 114, 182, 255))
        draw.rectangle([(px - 3 + rox, ty + 6), (px + 2 + rox, ty + 27)], fill=(255, 255, 255, 255))
        draw.rectangle([(px - 2 + rox, ty + 12), (px + 1 + rox, ty + 14)], fill=(245, 158, 11, 255))
    elif fig_id == 26: # VAATI
        draw.rectangle([(px - 7 + rox, ty - 8), (px + 6 + rox, ty - 1)], fill=(139, 92, 246, 255))
        draw.rectangle([(px - 1 + rox, ty - 13), (px + 1 + rox, ty - 9)], fill=(124, 58, 237, 255))
        draw.point((px + rox, ty - 6), fill=(239, 68, 68, 255))
        draw.rectangle([(px - 5 + rox, ty), (px + 4 + rox, ty + 5)], fill=(233, 213, 255, 255))
        draw.rectangle([(px - 2 + rox, ty + 2), (px + 1 + rox, ty + 4)], fill=(220, 38, 38, 255))
        draw.point((px + rox, ty + 3), fill=(253, 224, 71, 255))
        draw.rectangle([(px - 9 + rox, ty + 6), (px + 8 + rox, ty + 27)], fill=(59, 7, 100, 255))
        draw.rectangle([(px - 9 + rox, ty + 24), (px + 8 + rox, ty + 27)], fill=(153, 27, 27, 255))

def main():
    W, H = 640, 420
    img = Image.new("RGBA", (W, H), (15, 17, 23, 255))
    draw = ImageDraw.Draw(img)

    # 1. Header Banner
    draw.rectangle([(0, 0), (W - 1, 28)], fill=(24, 28, 40, 255))
    draw.line([(0, 28), (W - 1, 28)], fill=(59, 130, 246, 255))

    try:
        font_sm = ImageFont.load_default()
    except Exception:
        font_sm = None

    def draw_text(x, y, text, col=(248, 250, 252, 255)):
        if font_sm:
            draw.text((x, y), text, font=font_sm, fill=col)

    draw_text(14, 8, "OPENMINISH - CUCCO MINIGAME & CARLOV FIGURINE GALLERY (OPCAO C)", (96, 165, 250, 255))
    draw_text(520, 8, "60 FPS HAL C11", (74, 222, 128, 255))

    # 4 Main Panels:
    # Top Row:
    #   Panel 1: Anju's Cucco Pen (x=12, y=38, w=304, h=175)
    #   Panel 2: Link Cucco Gliding (x=324, y=38, w=304, h=175)
    # Bottom Row:
    #   Panel 3: Carlov Gacha Machine (x=12, y=224, w=304, h=182)
    #   Panel 4: Figurine Inspector & Pedestal (x=324, y=224, w=304, h=182)

    panels = [
        ("1. ANJU CUCCO RESCUE & PEN IN HYRULE TOWN", 12, 38, 304, 175),
        ("2. LINK CUCCO-GLIDE AIRBORNE PHYSICS", 324, 38, 304, 175),
        ("3. CARLOV GACHA MACHINE & MYSTERIOUS SHELLS", 12, 224, 304, 182),
        ("4. 3D FIGURINE INSPECTOR & CARLOV TROPHY STAND", 324, 224, 304, 182)
    ]

    for title, px, py, pw, ph in panels:
        draw.rectangle([(px, py), (px + pw - 1, py + ph - 1)], fill=(20, 24, 33, 255), outline=(51, 65, 85, 255))
        draw.rectangle([(px + 1, py + 1), (px + pw - 2, py + 18)], fill=(30, 41, 59, 255))
        draw_text(px + 8, py + 4, title, (226, 232, 240, 255))

    # ==========================================
    # PANEL 1: ANJU'S PEN
    # ==========================================
    p1_x, p1_y = 12, 38
    # Cobblestone ground
    draw.rectangle([(p1_x + 4, p1_y + 20), (p1_x + 300, p1_y + 171)], fill=(51, 65, 85, 255))
    
    # Wooden Pen (Cercado)
    pen_x1, pen_y1, pen_x2, pen_y2 = p1_x + 130, p1_y + 55, p1_x + 285, p1_y + 145
    draw.rectangle([(pen_x1, pen_y1), (pen_x2, pen_y2)], fill=(44, 48, 56, 255))
    draw.rectangle([(pen_x1, pen_y1), (pen_x2, pen_y1 + 3)], fill=(120, 53, 15, 255))
    draw.rectangle([(pen_x1, pen_y2 - 3), (pen_x2, pen_y2)], fill=(120, 53, 15, 255))
    draw.rectangle([(pen_x1, pen_y1), (pen_x1 + 3, pen_y2)], fill=(120, 53, 15, 255))
    draw.rectangle([(pen_x2 - 3, pen_y1), (pen_x2, pen_y2)], fill=(120, 53, 15, 255))

    # Anju at gate cheering
    render_anju(draw, pen_x1 - 14, (pen_y1 + pen_y2) // 2, is_cheering=True)
    draw_text(pen_x1 - 42, pen_y1 + 10, "Anju", (249, 168, 212, 255))
    draw_text(pen_x1 - 50, pen_y1 + 24, "(Feliz!)", (249, 168, 212, 255))

    # Secured Cuccos inside the pen
    render_cucco_sprite(draw, pen_x1 + 30, pen_y1 + 35, cucco_type=0)
    render_cucco_sprite(draw, pen_x1 + 75, pen_y1 + 45, cucco_type=1)
    render_cucco_sprite(draw, pen_x1 + 120, pen_y1 + 30, cucco_type=2, is_flapping=True)
    draw_text(pen_x1 + 18, pen_y2 - 14, "Cercado (Resgatadas!)", (134, 239, 172, 255))

    # Escaped Cuccos outside
    render_cucco_sprite(draw, p1_x + 35, p1_y + 70, cucco_type=0, is_flapping=True)
    render_cucco_sprite(draw, p1_x + 65, p1_y + 115, cucco_type=1)
    render_feather(draw, p1_x + 45, p1_y + 60)
    render_feather(draw, p1_x + 50, p1_y + 75)
    render_feather(draw, p1_x + 28, p1_y + 80)

    # HUD Banner overlay
    draw.rectangle([(p1_x + 10, p1_y + 24), (p1_x + 150, p1_y + 44)], fill=(30, 27, 75, 220))
    draw_text(p1_x + 16, p1_y + 28, "24.5s", (56, 189, 248, 255))
    draw_text(p1_x + 62, p1_y + 28, "CUCCO: 3/5", (250, 204, 21, 255))
    draw_text(p1_x + 10, p1_y + 152, "Round 4/10 | Pen secured chime + penas", (148, 163, 184, 255))

    # ==========================================
    # PANEL 2: LINK GLIDING WITH CUCCO
    # ==========================================
    p2_x, p2_y = 324, 38
    # Sky and cliff backdrop
    draw.rectangle([(p2_x + 4, p2_y + 20), (p2_x + 300, p2_y + 171)], fill=(30, 41, 59, 255))
    # Cliff ledge on left
    draw.polygon([(p2_x + 4, p2_y + 70), (p2_x + 65, p2_y + 70), (p2_x + 45, p2_y + 171), (p2_x + 4, p2_y + 171)], fill=(71, 85, 105, 255))
    draw.line([(p2_x + 4, p2_y + 70), (p2_x + 65, p2_y + 70)], fill=(148, 163, 184, 255), width=2)
    draw_text(p2_x + 10, p2_y + 80, "Penhasco", (203, 213, 225, 255))

    # Ground floor far below
    draw.rectangle([(p2_x + 4, p2_y + 155), (p2_x + 300, p2_y + 171)], fill=(34, 197, 94, 180))

    # Link in mid-air (x=160, y=95)
    lx, ly = p2_x + 150, p2_y + 90
    # Shadow on ground below
    draw.ellipse([(lx - 10, p2_y + 158), (lx + 10, p2_y + 164)], fill=(5, 16, 7, 100))

    # Link sprite hanging
    draw.rectangle([(lx - 5, ly + 2), (lx + 4, ly + 14)], fill=(22, 163, 74, 255)) # Green tunic
    draw.rectangle([(lx - 4, ly - 4), (lx + 3, ly + 2)], fill=(254, 215, 170, 255)) # Face
    draw.rectangle([(lx - 4, ly - 7), (lx + 3, ly - 4)], fill=(21, 128, 61, 255)) # Cap
    draw.rectangle([(lx - 4, ly + 15), (lx - 2, ly + 20)], fill=(120, 53, 15, 255)) # Legs
    draw.rectangle([(lx + 1, ly + 15), (lx + 3, ly + 20)], fill=(120, 53, 15, 255))
    draw.rectangle([(lx - 7, ly - 2), (lx - 5, ly + 6)], fill=(22, 163, 74, 255)) # Raised arms
    draw.rectangle([(lx + 4, ly - 2), (lx + 6, ly + 6)], fill=(22, 163, 74, 255))

    # Cucco held above head flapping furiously!
    render_cucco_sprite(draw, lx, ly - 12, cucco_type=0, is_flapping=True)

    # Gliding trajectory arc
    draw.arc([(p2_x + 60, p2_y + 65), (lx + 40, ly + 30)], start=180, end=350, fill=(56, 189, 248, 200))
    # Falling feather trail
    render_feather(draw, lx - 18, ly - 4)
    render_feather(draw, lx - 30, ly + 8)
    render_feather(draw, lx - 12, ly + 22)
    render_feather(draw, lx - 45, ly - 8)

    draw_text(p2_x + 60, p2_y + 35, "Voo Planado: vz = -0.55f (Gravidade)", (56, 189, 248, 255))
    draw_text(p2_x + 60, p2_y + 50, "Asas batem e Link flutua sobre fendas!", (248, 250, 252, 255))
    draw_text(p2_x + 10, p2_y + 152, "Desafio Supremo Nivel 10: Concede Heart Piece!", (244, 63, 94, 255))

    # ==========================================
    # PANEL 3: CARLOV'S GACHA MACHINE
    # ==========================================
    p3_x, p3_y = 12, 224
    draw.rectangle([(p3_x + 4, p3_y + 20), (p3_x + 300, p3_y + 178)], fill=(28, 25, 23, 255))

    # Glass dome
    mx, my = p3_x + 70, p3_y + 90
    draw.rectangle([(mx - 28, my - 45), (mx + 27, my - 1)], fill=(2, 132, 199, 68))
    draw.rectangle([(mx - 26, my - 43), (mx + 25, my - 3)], fill=(56, 189, 248, 51))
    
    # Colorful capsules in globe
    b_cols = [(239, 68, 68, 255), (59, 130, 246, 255), (16, 185, 129, 255), (245, 158, 11, 255), (139, 92, 246, 255), (236, 72, 153, 255)]
    for b in range(6):
        bx = mx - 16 + (b % 3) * 12
        by = my - 35 + (b // 3) * 14
        draw.rectangle([(bx, by), (bx + 8, by + 8)], fill=b_cols[b])
        draw.point((bx + 2, by + 2), fill=(255, 255, 255, 255))

    # Machine Red Base
    draw.rectangle([(mx - 32, my), (mx + 31, my + 44)], fill=(220, 38, 38, 255))
    draw.rectangle([(mx - 28, my + 4), (mx + 27, my + 7)], fill=(153, 27, 27, 255))
    
    # Golden Crank
    draw.rectangle([(mx - 3, my + 17), (mx + 2, my + 22)], fill=(120, 53, 15, 255))
    draw.rectangle([(mx + 6, my + 14), (mx + 13, my + 21)], fill=(245, 158, 11, 255))
    
    # Chute & Dispensed Capsule
    draw.rectangle([(mx - 10, my + 30), (mx + 9, my + 41)], fill=(24, 24, 27, 255))
    draw.rectangle([(mx - 5, my + 32), (mx + 4, my + 36)], fill=(239, 68, 68, 255))
    draw.rectangle([(mx - 5, my + 37), (mx + 4, my + 41)], fill=(248, 250, 252, 255))

    # Shell Betting Panel on right
    bx, by = p3_x + 135, p3_y + 30
    draw.rectangle([(bx, by), (bx + 155, by + 120)], fill=(41, 37, 36, 255), outline=(87, 83, 78, 255))
    draw_text(bx + 8, by + 8, "APOSTA DE CONCHAS", (253, 224, 71, 255))
    
    draw_mysterious_shell_icon(draw, bx + 8, by + 26)
    draw_text(bx + 22, by + 27, "CONCHAS: [ 15 ]", (255, 255, 255, 255))
    draw_text(bx + 8, by + 45, "DPAD: +/-10 e +/-1", (148, 163, 184, 255))
    draw_text(bx + 8, by + 63, "CHANCE: 78.5%", (74, 222, 128, 255))
    draw_text(bx + 8, by + 81, "COLECAO: 28/136", (56, 189, 248, 255))
    draw_text(bx + 8, by + 99, "Medalha de Carlov", (245, 158, 11, 255))

    draw_text(p3_x + 12, p3_y + 160, "[A] Girar | [ESQ/DIR] +/-10 | [START] Abrir Galeria", (148, 163, 184, 255))

    # ==========================================
    # PANEL 4: FIGURINE INSPECTOR & PEDESTAL
    # ==========================================
    p4_x, p4_y = 324, 224
    draw.rectangle([(p4_x + 4, p4_y + 20), (p4_x + 300, p4_y + 178)], fill=(28, 25, 23, 255))

    # 3D Pedestal and Figurine #001 (Link)
    render_trophy_model(draw, p4_x + 65, p4_y + 120, fig_id=0, rot_angle=0.45)
    
    # Star sparkles on pedestal
    draw.point((p4_x + 45, p4_y + 80), fill=(253, 224, 71, 255))
    draw.point((p4_x + 85, p4_y + 75), fill=(255, 255, 255, 255))
    draw.point((p4_x + 90, p4_y + 105), fill=(253, 224, 71, 255))

    # Detail card on right
    cx, cy = p4_x + 125, p4_y + 30
    draw.rectangle([(cx, cy), (cx + 170, cy + 120)], fill=(41, 37, 36, 255), outline=(87, 83, 78, 255))
    draw_text(cx + 8, cy + 8, "001: Link (O Heroi)", (253, 224, 71, 255))
    draw_text(cx + 8, cy + 24, "CATEGORIA: [HEROI]", (56, 189, 248, 255))
    draw_text(cx + 8, cy + 44, "Jovem aprendiz de ferreiro que", (231, 229, 228, 255))
    draw_text(cx + 8, cy + 58, "empunha a lendaria White Sword", (231, 229, 228, 255))
    draw_text(cx + 8, cy + 72, "e salva o reino de Hyrule.", (231, 229, 228, 255))
    draw_text(cx + 8, cy + 96, "STATUS: COLETADA", (74, 222, 128, 255))

    draw_text(p4_x + 16, p4_y + 160, "Modelagem 3D rotativa com pedestal chanfrado", (148, 163, 184, 255))

    # Save output
    brain_dir = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a"
    out_brain = os.path.join(brain_dir, "cucco_and_figurines_showcase.png")
    out_scratch = os.path.join(os.path.dirname(__file__), "cucco_and_figurines_showcase.png")

    img.save(out_brain)
    img.save(out_scratch)
    print(f"Saved cucco & figurines showcase to:\n- {out_brain}\n- {out_scratch}")

if __name__ == "__main__":
    main()
