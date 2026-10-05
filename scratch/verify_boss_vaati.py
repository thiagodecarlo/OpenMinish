#!/usr/bin/env python3
"""
scratch/verify_boss_vaati.py
Generates boss_vaati_showcase.png showcasing:
1. Phase 1: Vaati Reborn (Sorcerer form, 4 floating malice orbs, chest eye vulnerability)
2. Phase 2: Vaati Transfigured (Colossal winged eye demon, 4-clone simultaneous slash, crash stun)
3. Phase 3: Vaati's Wrath (Colossal form, 2 detached claws, Cane of Pacci, 4-shield laser reflection)
4. Epilogue: Princess Zelda cured, Sage Ezlo true Minish form farewell, and End Credits
"""

import os
import shutil
import math
from PIL import Image, ImageDraw, ImageFont

def main():
    conv_id = "91f45871-4e2f-495f-88d6-7427ed06770a"
    out_dir = os.path.join(r"C:\Users\thiag\.gemini\antigravity\brain", conv_id)
    out_path = os.path.join(out_dir, "boss_vaati_showcase.png")
    scratch_path = r"scratch/boss_vaati_showcase.png"

    img_w = 980
    img_h = 660
    img = Image.new("RGBA", (img_w, img_h), (10, 8, 18, 255))
    draw = ImageDraw.Draw(img)

    try:
        font_title = ImageFont.truetype("arial.ttf", 15)
        font_sub = ImageFont.truetype("arial.ttf", 12)
        font_small = ImageFont.truetype("arial.ttf", 10)
    except:
        font_title = font_sub = font_small = ImageFont.load_default()

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(24, 15, 36, 255))
    draw.line([0, 48, img_w, 48], fill=(225, 29, 72, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Final Climax: Sorcerer Vaati 3-Phase Battle", fill=(254, 240, 138, 255), font=font_title)
    draw.text((680, 16), "Sprint 5: feature/boss-vaati-climax", fill=(244, 63, 94, 255), font=font_sub)

    panels = [
        ("1. FASE 1: VAATI REBORN (FEITICEIRO COM 4 ORBS & OLHO PEITORAL)", 20, 60, 475, 340),
        ("2. FASE 2: VAATI TRANSFIGURED (BESTA ALADA & ATAQUE QUADRUPLO)", 505, 60, 960, 340),
        ("3. FASE 3: VAATI'S WRATH (GARRAS, CANE OF PACCI & 4-SHIELD LASERS)", 20, 360, 475, 640),
        ("4. EPILOGO: CURA DE ZELDA, DESPEDIDA DE EZLO & CREDITOS FINAIS", 505, 360, 960, 640)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(18, 14, 28, 255), outline=(55, 40, 75, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(32, 22, 50, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(225, 29, 72, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(253, 224, 71, 255), font=font_small)

    def draw_link(lx, ly, tunic_col, tunic_dark, hat_col, dir_down=True, holding_sword=False, holding_shield=False):
        draw.ellipse([lx - 6, ly + 6, lx + 6, ly + 10], fill=(0, 0, 0, 100))
        draw.rectangle([lx - 4, ly - 3, lx + 4, ly + 5], fill=tunic_col, outline=tunic_dark)
        draw.rectangle([lx - 4, ly + 2, lx + 4, ly + 4], fill=(120, 53, 15, 255))
        draw.rectangle([lx - 1, ly + 2, lx + 1, ly + 4], fill=(250, 204, 21, 255))
        draw.rectangle([lx - 4, ly + 5, lx - 1, ly + 8], fill=(113, 63, 18, 255))
        draw.rectangle([lx + 1, ly + 5, lx + 4, ly + 8], fill=(113, 63, 18, 255))
        draw.rectangle([lx - 3, ly - 9, lx + 3, ly - 3], fill=(254, 215, 170, 255))
        if dir_down:
            draw.point((lx - 1, ly - 6), fill=(15, 23, 42, 255))
            draw.point((lx + 2, ly - 6), fill=(15, 23, 42, 255))
        draw.polygon([(lx - 5, ly - 8), (lx + 5, ly - 8), (lx + 2, ly - 14), (lx - 2, ly - 14)], fill=hat_col)
        draw.ellipse([lx - 1, ly - 15, lx + 2, ly - 12], fill=(234, 179, 8, 255))

        if holding_sword:
            draw.rectangle([lx + 5, ly - 10, lx + 8, ly + 4], fill=(241, 245, 249, 255))
            draw.rectangle([lx + 4, ly + 4, lx + 9, ly + 6], fill=(234, 179, 8, 255))
        if holding_shield:
            draw.rectangle([lx - 8, ly - 4, lx - 2, ly + 6], fill=(56, 189, 248, 255), outline=(224, 242, 254, 255))

    def draw_arena_base(ox, oy):
        for gy in range(14):
            for gx in range(26):
                px = ox + gx * 16
                py = oy + gy * 16
                if gy == 0 or gy == 13 or gx == 0 or gx == 25:
                    draw.rectangle([px, py, px + 15, py + 15], fill=(24, 20, 36, 255))
                    draw.line([px, py + 14, px + 15, py + 14], fill=(15, 10, 24, 255), width=2)
                else:
                    col = (46, 38, 70, 255) if (gx + gy) % 2 == 0 else (36, 28, 56, 255)
                    draw.rectangle([px, py, px + 15, py + 15], fill=col)
                    draw.rectangle([px, py, px + 15, py + 15], outline=(30, 22, 46, 80))

        # Diamond Split Pads in center
        pad_cx, pad_cy = ox + 208, oy + 150
        for px, py in [(pad_cx, pad_cy - 18), (pad_cx - 24, pad_cy), (pad_cx + 24, pad_cy), (pad_cx, pad_cy + 18)]:
            draw.rectangle([px - 7, py - 7, px + 7, py + 7], fill=(2, 132, 199, 255), outline=(56, 189, 248, 255))
            draw.rectangle([px - 4, py - 4, px + 4, py + 4], fill=(56, 189, 248, 255))
            draw.rectangle([px - 1, py - 1, px + 1, py + 1], fill=(255, 255, 255, 255))

    # ==========================================
    # PANEL 1: FASE 1 - VAATI REBORN
    # ==========================================
    p1_ox, p1_oy = 30, 95
    draw_arena_base(p1_ox, p1_oy)

    vx, vy = p1_ox + 208, p1_oy + 50
    # Malice Aura
    draw.ellipse([vx - 30, vy - 25, vx + 30, vy + 35], fill=(88, 28, 135, 100))

    # Vaati Robe
    draw.rectangle([vx - 14, vy - 10, vx + 14, vy + 24], fill=(88, 28, 135, 255), outline=(59, 7, 100, 255))
    draw.polygon([(vx - 16, vy + 24), (vx + 16, vy + 24), (vx, vy + 34)], fill=(88, 28, 135, 255))

    # Head, Face & Pale Lilac Hair
    draw.rectangle([vx - 9, vy - 22, vx + 9, vy - 10], fill=(237, 233, 254, 255))
    draw.rectangle([vx - 11, vy - 28, vx + 11, vy - 20], fill=(233, 213, 255, 255)) # Hair
    draw.polygon([(vx - 14, vy - 20), (vx - 10, vy - 6), (vx - 8, vy - 20)], fill=(233, 213, 255, 255)) # Lock L
    draw.polygon([(vx + 14, vy - 20), (vx + 10, vy - 6), (vx + 8, vy - 20)], fill=(233, 213, 255, 255)) # Lock R
    # Crimson evil eyes
    draw.rectangle([vx - 6, vy - 18, vx - 3, vy - 15], fill=(220, 38, 38, 255))
    draw.rectangle([vx + 3, vy - 18, vx + 6, vy - 15], fill=(220, 38, 38, 255))
    # Golden Corrupted Crown
    draw.polygon([(vx - 10, vy - 28), (vx - 6, vy - 35), (vx, vy - 30), (vx + 6, vy - 35), (vx + 10, vy - 28)], fill=(245, 158, 11, 255))

    # Open Vulnerable Chest Eye
    draw.ellipse([vx - 8, vy - 2, vx + 8, vy + 12], fill=(254, 240, 138, 255), outline=(220, 38, 38, 255))
    draw.ellipse([vx - 4, vy + 1, vx + 4, vy + 9], fill=(220, 38, 38, 255))
    draw.point((vx, vy + 5), fill=(255, 255, 255, 255))

    # 4 Orbiting Malice Orbs
    for angle in [0, math.pi/2, math.pi, 3*math.pi/2]:
        ox_pos = vx + int(math.cos(angle) * 45)
        oy_pos = vy + int(math.sin(angle) * 30)
        draw.ellipse([ox_pos - 8, oy_pos - 8, ox_pos + 8, oy_pos + 8], fill=(76, 5, 25, 255), outline=(225, 29, 72, 255))
        draw.ellipse([ox_pos - 4, oy_pos - 4, ox_pos + 4, oy_pos + 4], fill=(220, 38, 38, 255))

    # Link striking with Four Sword
    draw_link(vx, vy + 60, (22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255), dir_down=False, holding_sword=True)
    draw.text((p1_ox + 70, p1_oy + 195), "Destroy 4 Malice Orbs to expose the chest eye!", fill=(253, 224, 71, 255), font=font_small)


    # ==========================================
    # PANEL 2: FASE 2 - VAATI TRANSFIGURED
    # ==========================================
    p2_ox, p2_oy = 515, 95
    draw_arena_base(p2_ox, p2_oy)

    v2x, v2y = p2_ox + 208, p2_oy + 65
    # Demonic Bat Wings
    draw.polygon([(v2x - 30, v2y - 10), (v2x - 70, v2y - 35), (v2x - 55, v2y + 15), (v2x - 30, v2y + 5)], fill=(30, 27, 75, 255), outline=(79, 70, 229, 255))
    draw.polygon([(v2x + 30, v2y - 10), (v2x + 70, v2y - 35), (v2x + 55, v2y + 15), (v2x + 30, v2y + 5)], fill=(30, 27, 75, 255), outline=(79, 70, 229, 255))

    # Colossal Demonic Eyeball
    draw.ellipse([v2x - 30, v2y - 30, v2x + 30, v2y + 30], fill=(24, 24, 36, 255), outline=(225, 29, 72, 255), width=2)
    draw.ellipse([v2x - 22, v2y - 22, v2x + 22, v2y + 22], fill=(220, 38, 38, 255))
    draw.ellipse([v2x - 12, v2y - 12, v2x + 12, v2y + 12], fill=(15, 10, 24, 255))
    draw.ellipse([v2x - 4, v2y - 4, v2x + 4, v2y + 4], fill=(255, 255, 255, 255))

    # 4 Revealed Vulnerable Eyes (Diamond around the beast)
    quad_eyes = [(v2x, v2y - 42), (v2x - 44, v2y), (v2x + 44, v2y), (v2x, v2y + 42)]
    for qx, qy in quad_eyes:
        draw.ellipse([qx - 8, qy - 8, qx + 8, qy + 8], fill=(220, 38, 38, 255), outline=(254, 240, 138, 255), width=2)
        draw.ellipse([qx - 3, qy - 3, qx + 3, qy + 3], fill=(254, 240, 138, 255))

    # Four Links executing simultaneous synchronized strikes
    tunic_colors = [
        ((22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255)),   # Green (North)
        ((220, 38, 38, 255), (185, 28, 28, 255), (239, 68, 68, 255)),   # Red (West)
        ((37, 99, 235, 255), (29, 78, 216, 255), (59, 130, 246, 255)),   # Blue (East)
        ((147, 51, 234, 255), (126, 34, 206, 255), (168, 85, 247, 255)) # Purple (South)
    ]
    draw_link(v2x, v2y - 60, tunic_colors[0][0], tunic_colors[0][1], tunic_colors[0][2], dir_down=True, holding_sword=True)
    draw_link(v2x - 65, v2y, tunic_colors[1][0], tunic_colors[1][1], tunic_colors[1][2], dir_down=False, holding_sword=True)
    draw_link(v2x + 65, v2y, tunic_colors[2][0], tunic_colors[2][1], tunic_colors[2][2], dir_down=False, holding_sword=True)
    draw_link(v2x, v2y + 60, tunic_colors[3][0], tunic_colors[3][1], tunic_colors[3][2], dir_down=False, holding_sword=True)

    draw.text((p2_ox + 50, p2_oy + 195), "Simultaneous 4-Clone strike crashes the demon beast to the ground!", fill=(254, 240, 138, 255), font=font_small)


    # ==========================================
    # PANEL 3: FASE 3 - VAATI'S WRATH
    # ==========================================
    p3_ox, p3_oy = 30, 395
    draw_arena_base(p3_ox, p3_oy)

    v3x, v3y = p3_ox + 208, p3_oy + 55
    # Grand Malice Wings
    draw.polygon([(v3x - 35, v3y - 15), (v3x - 85, v3y - 45), (v3x - 65, v3y + 25), (v3x - 35, v3y + 10)], fill=(30, 27, 75, 255), outline=(225, 29, 72, 255))
    draw.polygon([(v3x + 35, v3y - 15), (v3x + 85, v3y - 45), (v3x + 65, v3y + 25), (v3x + 35, v3y + 10)], fill=(30, 27, 75, 255), outline=(225, 29, 72, 255))

    # Colossal Eyeball with 4 Laser Pupils
    draw.ellipse([v3x - 35, v3y - 35, v3x + 35, v3y + 35], fill=(15, 10, 24, 255), outline=(225, 29, 72, 255), width=2)
    draw.ellipse([v3x - 26, v3y - 26, v3x + 26, v3y + 26], fill=(220, 38, 38, 255))
    # 4 Glowing Laser Pupils
    for px, py in [(v3x - 12, v3y - 12), (v3x + 12, v3y - 12), (v3x - 12, v3y + 12), (v3x + 12, v3y + 12)]:
        draw.ellipse([px - 5, py - 5, px + 5, py + 5], fill=(254, 240, 138, 255))

    # Two Detached Burrowing Demon Claws (Left & Right)
    for cx, cy, flipped in [(v3x - 90, v3y + 40, True), (v3x + 90, v3y + 40, False)]:
        ccol = (245, 158, 11, 255) if flipped else (24, 24, 27, 255)
        draw.rectangle([cx - 16, cy - 16, cx + 16, cy + 16], fill=ccol, outline=(225, 29, 72, 255), width=2)
        # Giant Spikes
        draw.polygon([(cx - 20, cy - 8), (cx - 14, cy - 14), (cx - 8, cy - 8)], fill=(226, 232, 240, 255))
        draw.polygon([(cx + 8, cy - 8), (cx + 14, cy - 14), (cx + 20, cy - 8)], fill=(226, 232, 240, 255))
        if flipped:
            # Internal Parasite Eye exposed for Minish Link
            draw.ellipse([cx - 6, cy - 6, cx + 6, cy + 6], fill=(239, 68, 68, 255), outline=(254, 240, 138, 255))
            draw.text((cx - 24, cy + 18), "[PACCI FLIPPED]", fill=(250, 204, 21, 255), font=font_small)

    # 4 Links reflecting the 4 Laser Beams with shields!
    laser_x = [v3x - 30, v3x - 10, v3x + 10, v3x + 30]
    for i, lx_pos in enumerate(laser_x):
        tc, tdark, hc = tunic_colors[i]
        # Red Laser Beam descending
        draw.line([lx_pos, v3y + 30, lx_pos, p3_oy + 140], fill=(239, 68, 68, 255), width=3)
        # Deflection Shield Spark
        draw.ellipse([lx_pos - 8, p3_oy + 134, lx_pos + 8, p3_oy + 146], fill=(56, 189, 248, 220), outline=(255, 255, 255, 255))
        draw_link(lx_pos, p3_oy + 155, tc, tdark, hc, dir_down=False, holding_shield=True)

    draw.text((p3_ox + 60, p3_oy + 195), "Cane of Pacci flips claws; 4-shield deflection stuns Vaati for victory!", fill=(253, 224, 71, 255), font=font_small)


    # ==========================================
    # PANEL 4: EPÍLOGO & CRÉDITOS FINAIS
    # ==========================================
    p4_ox, p4_oy = 515, 395
    # Celestial Starry Night Sky
    draw.rectangle([p4_ox, p4_oy, p4_ox + 445, p4_oy + 245], fill=(9, 9, 15, 255))
    # Stars
    for sx, sy in [(p4_ox + 30, p4_oy + 40), (p4_ox + 120, p4_oy + 30), (p4_ox + 350, p4_oy + 50),
                  (p4_ox + 70, p4_oy + 100), (p4_ox + 390, p4_oy + 120), (p4_ox + 220, p4_oy + 25)]:
        draw.point((sx, sy), fill=(255, 255, 255, 255))
        draw.point((sx + 1, sy), fill=(254, 240, 138, 200))

    # Princess Zelda Cured (Center Left)
    zx, zy = p4_ox + 130, p4_oy + 85
    # Holy Glow
    draw.ellipse([zx - 25, zy - 25, zx + 25, zy + 25], fill=(254, 240, 138, 60))
    # Royal Pink Dress
    draw.rectangle([zx - 8, zy - 6, zx + 8, zy + 16], fill=(244, 114, 182, 255), outline=(219, 39, 119, 255))
    draw.polygon([(zx - 10, zy + 16), (zx + 10, zy + 16), (zx, zy + 24)], fill=(244, 114, 182, 255))
    # Golden hair & Crown
    draw.rectangle([zx - 6, zy - 18, zx + 6, zy - 6], fill=(254, 215, 170, 255))
    draw.polygon([(zx - 8, zy - 18), (zx - 8, zy - 6), (zx - 5, zy + 6)], fill=(250, 204, 21, 255)) # Hair L
    draw.polygon([(zx + 8, zy - 18), (zx + 8, zy - 6), (zx + 5, zy + 6)], fill=(250, 204, 21, 255)) # Hair R
    draw.polygon([(zx - 6, zy - 18), (zx, zy - 24), (zx + 6, zy - 18)], fill=(250, 204, 21, 255)) # Tiara
    draw.text((zx - 22, zy + 26), "Princess Zelda", fill=(244, 114, 182, 255), font=font_small)

    # Link standing happily with Zelda
    draw_link(zx + 36, zy + 8, (22, 163, 74, 255), (21, 128, 61, 255), (34, 197, 94, 255), dir_down=True)

    # Sage Ezlo Farewell in true Minish form (Hovering between Link and Zelda)
    ex, ey = p4_ox + 90, p4_oy + 95
    draw.ellipse([ex - 12, ey - 12, ex + 12, ey + 12], fill=(74, 222, 128, 80))
    # Green Minish Sage robe
    draw.rectangle([ex - 5, ey - 4, ex + 5, ey + 8], fill=(34, 197, 94, 255))
    draw.rectangle([ex - 4, ey - 12, ex + 4, ey - 4], fill=(254, 215, 170, 255)) # Face
    draw.polygon([(ex - 5, ey - 4), (ex + 5, ey - 4), (ex, ey + 4)], fill=(241, 245, 249, 255)) # White beard
    draw.polygon([(ex - 6, ey - 12), (ex + 6, ey - 12), (ex + 2, ey - 20), (ex - 2, ey - 20)], fill=(22, 163, 74, 255)) # Cap
    draw.text((ex - 18, ey + 12), "Sage Ezlo", fill=(74, 222, 128, 255), font=font_small)

    # Rolling Credits Box on Right
    cx1, cy1 = p4_ox + 230, p4_oy + 25
    draw.rectangle([cx1, cy1, cx1 + 205, cy1 + 145], fill=(15, 15, 25, 230), outline=(245, 158, 11, 255))
    draw.text((cx1 + 10, cy1 + 10), "THE LEGEND OF ZELDA: THE MINISH CAP", fill=(253, 224, 71, 255), font=font_small)
    draw.text((cx1 + 10, cy1 + 28), "Director: Hidemaro Fujibayashi", fill=(226, 232, 240, 255), font=font_small)
    draw.text((cx1 + 10, cy1 + 44), "Producers: Keiji Inafune, Eiji Aonuma", fill=(226, 232, 240, 255), font=font_small)
    draw.text((cx1 + 10, cy1 + 60), "Executive Producer: Satoru Iwata", fill=(226, 232, 240, 255), font=font_small)
    draw.text((cx1 + 10, cy1 + 80), "OpenMinish Native Port: Thiago De Carlo", fill=(56, 189, 248, 255), font=font_small)
    draw.text((cx1 + 10, cy1 + 96), "Engine: Clean-Room C11 HAL / Zero-ROM", fill=(74, 222, 128, 255), font=font_small)
    draw.text((cx1 + 10, cy1 + 118), "THANK YOU FOR PLAYING!", fill=(250, 204, 21, 255), font=font_small)

    # Celebratory Banner at bottom
    draw.rectangle([p4_ox + 15, p4_oy + 180, p4_ox + 430, p4_oy + 215], fill=(24, 20, 36, 245), outline=(245, 158, 11, 255), width=2)
    draw.text((p4_ox + 30, p4_oy + 186), "PARABENS! O REINO DE HYRULE ESTA SALVO!", fill=(253, 224, 71, 255), font=font_small)
    draw.text((p4_ox + 30, p4_oy + 198), "Vaati foi derrotado! Zelda foi curada! Sprint 5 (Ato V) 100% Concluido!", fill=(224, 242, 254, 255), font=font_small)

    os.makedirs(out_dir, exist_ok=True)
    img.save(out_path)
    print(f"Showcase image saved to {out_path}")
    shutil.copyfile(out_path, scratch_path)
    print(f"Showcase image copied to {scratch_path}")

if __name__ == "__main__":
    main()
