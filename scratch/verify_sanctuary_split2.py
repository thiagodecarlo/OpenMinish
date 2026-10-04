import sys
import os
import math
from PIL import Image, ImageDraw

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "elemental_sanctuary_split2_showcase.png")
    scratch_path = r"d:\REPOS\TLoZ-MC\scratch\elemental_sanctuary_split2_showcase.png"

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (10, 14, 24, 255))
    draw = ImageDraw.Draw(img)

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(15, 23, 42, 255))
    draw.line([0, 48, img_w, 48], fill=(56, 189, 248, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Elemental Sanctuary & Four Sword 2-Clone Split", fill=(254, 240, 138, 255))
    draw.text((660, 16), "Item 9: feature/elemental-sanctuary-split2", fill=(56, 189, 248, 255))

    panels = [
        ("1. ELEMENTAL SANCTUARY PEDESTAL & SACRED ALTAR", 20, 60, 460, 330),
        ("2. INFUSION CUTSCENE: EARTH + FIRE = WHITE SWORD (TWO ELEMENTS)", 500, 60, 940, 330),
        ("3. FOUR SWORD CHARGE & 2-CLONE SPLIT FLOOR PADS", 20, 350, 460, 620),
        ("4. SYNCHRONIZED DUAL SWITCHES & SANCTUARY GATE", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(15, 20, 32, 255), outline=(40, 55, 80, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(24, 34, 54, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(56, 189, 248, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(125, 211, 252, 255))

    # Helper function to draw Sanctuary Architecture
    def draw_sanctuary_room(px, py, gate_open=False):
        # Polished marble checkered floor
        for ry in range(14):
            for rx in range(26):
                lx = px + rx * 16
                ly = py + ry * 16
                col = (226, 232, 240, 255) if (rx + ry) % 2 == 0 else (203, 213, 225, 255)
                draw.rectangle([lx, ly, lx + 15, ly + 15], fill=col)
                draw.line([lx, ly + 15, lx + 15, ly + 15], fill=(148, 163, 184, 255))
                draw.line([lx + 15, ly, lx + 15, ly + 15], fill=(148, 163, 184, 255))

        # Stained glass light projections
        for vy, color in [(py + 40, (56, 189, 248, 50)), (py + 90, (34, 197, 94, 50)), (py + 140, (239, 68, 68, 50))]:
            draw.rectangle([px + 32, vy, px + 96, vy + 32], fill=color)
            draw.rectangle([px + 320, vy, px + 384, vy + 32], fill=color)

        # Sanctuary perimeter walls
        for rx in range(26):
            draw.rectangle([px + rx * 16, py, px + rx * 16 + 15, py + 16], fill=(30, 41, 59, 255))
            draw.line([px + rx * 16, py + 4, px + rx * 16 + 15, py + 4], fill=(51, 65, 85, 255), width=2)
            draw.line([px + rx * 16, py + 15, px + rx * 16 + 15, py + 15], fill=(245, 158, 11, 255))

        # Altar Pedestal
        ax, ay = px + 208, py + 36
        draw.rectangle([ax - 20, ay - 12, ax + 20, ay + 16], fill=(71, 85, 105, 255))
        draw.rectangle([ax - 16, ay - 8, ax + 16, ay + 12], fill=(100, 116, 139, 255))
        draw.line([ax - 20, ay + 14, ax + 20, ay + 14], fill=(212, 175, 55, 255), width=2)

        # Earth Element Pedestal (Left)
        draw.rectangle([ax - 60, ay - 4, ax - 40, ay + 14], fill=(71, 85, 105, 255))
        draw.ellipse([ax - 56, ay - 16, ax - 44, ay - 4], fill=(34, 197, 94, 255), outline=(134, 239, 172, 255))

        # Fire Element Pedestal (Right)
        draw.rectangle([ax + 40, ay - 4, ax + 60, ay + 14], fill=(71, 85, 105, 255))
        draw.ellipse([ax + 44, ay - 16, ax + 56, ay - 4], fill=(239, 68, 68, 255), outline=(254, 240, 138, 255))

        # Inner Sanctuary Gate (y=py + 64)
        gx = px + 128
        gy = py + 64
        if not gate_open:
            draw.rectangle([gx, gy, gx + 160, gy + 12], fill=(100, 116, 139, 255))
            for b in range(16):
                draw.line([gx + b * 10, gy, gx + b * 10, gy + 12], fill=(30, 41, 59, 255), width=2)
            draw.rectangle([gx + 72, gy + 2, gx + 88, gy + 10], fill=(245, 158, 11, 255))
        else:
            draw.rectangle([gx, gy, gx + 160, gy + 3], fill=(100, 116, 139, 255))

    # Helper function to draw Split Pads
    def draw_split_pads(px, py, lit1=False, lit2=False):
        pad1_x, pad1_y = px + 150, py + 110
        pad2_x, pad2_y = px + 250, py + 110

        for pdx, pdy, is_lit in [(pad1_x, pad1_y, lit1), (pad2_x, pad2_y, lit2)]:
            border_c = (255, 255, 255, 255) if is_lit else (2, 132, 199, 255)
            inner_c = (56, 189, 248, 255) if is_lit else (3, 105, 161, 255)
            draw.rectangle([pdx, pdy, pdx + 24, pdy + 24], fill=inner_c, outline=border_c, width=2)
            # Four Sword Runes
            rune_c = (255, 255, 255, 255) if is_lit else (56, 189, 248, 255)
            draw.rectangle([pdx + 10, pdy + 4, pdx + 14, pdy + 20], fill=rune_c)
            draw.rectangle([pdx + 4, pdy + 10, pdx + 20, pdy + 14], fill=rune_c)
            if is_lit:
                draw.ellipse([pdx - 8, pdy - 8, pdx + 32, pdy + 32], fill=(56, 189, 248, 60))

    # Helper function to draw Dual Floor Switches
    def draw_dual_switches(px, py, sw1_down=False, sw2_down=False):
        sw1_x, sw1_y = px + 152, py + 170
        sw2_x, sw2_y = px + 252, py + 170

        for swx, swy, is_down in [(sw1_x, sw1_y, sw1_down), (sw2_x, sw2_y, sw2_down)]:
            draw.rectangle([swx, swy, swx + 20, swy + 20], fill=(51, 65, 85, 255))
            plate_c = (34, 197, 94, 255) if is_down else (217, 119, 6, 255)
            draw.rectangle([swx + 3, swy + 3, swx + 17, swy + 17], fill=plate_c)

    # Helper function to draw Link / Clone
    def draw_hero(lx, ly, is_clone=False, action="stand", dir="up"):
        if is_clone:
            # Aura Ciano Four Sword
            draw.ellipse([lx - 4, ly - 4, lx + 20, ly + 28], fill=(56, 189, 248, 90))

        # Green Tunic
        draw.rectangle([lx + 4, ly + 2, lx + 12, ly + 8], fill=(34, 197, 94, 255)) # Cap
        draw.rectangle([lx + 4, ly + 8, lx + 12, ly + 14], fill=(253, 232, 205, 255)) # Face
        draw.rectangle([lx + 2, ly + 14, lx + 14, ly + 24], fill=(22, 163, 74, 255)) # Tunic
        draw.rectangle([lx + 4, ly + 24, lx + 12, ly + 28], fill=(255, 255, 255, 255)) # Tights
        draw.rectangle([lx + 4, ly + 28, lx + 12, ly + 32], fill=(180, 83, 9, 255)) # Boots

        if action == "charge":
            # Charging White Sword with Earth (green) and Fire (red) sparks
            draw.line([lx + 8, ly + 6, lx + 8, ly - 12], fill=(255, 255, 255, 255), width=3)
            draw.ellipse([lx + 2, ly - 16, lx + 14, ly - 4], fill=(56, 189, 248, 120))
            draw.point((lx + 4, ly - 14), fill=(34, 197, 94, 255)) # Earth spark
            draw.point((lx + 12, ly - 10), fill=(239, 68, 68, 255)) # Fire spark
        elif action == "slash":
            # Slashed sword
            draw.line([lx + 8, ly + 6, lx + 8, ly - 14], fill=(255, 255, 255, 255), width=4)
            draw.line([lx + 9, ly + 6, lx + 9, ly - 14], fill=(186, 230, 253, 255), width=2)
            draw.rectangle([lx + 4, ly + 4, lx + 12, ly + 7], fill=(250, 204, 21, 255))

    # =============================================================
    # PANEL 1: ELEMENTAL SANCTUARY & SACRED ALTAR
    # =============================================================
    p1_x, p1_y = 30, 94
    draw_sanctuary_room(p1_x, p1_y, gate_open=False)
    draw_split_pads(p1_x, p1_y, lit1=False, lit2=False)
    draw_dual_switches(p1_x, p1_y, sw1_down=False, sw2_down=False)

    # Link standing near pedestal
    draw_hero(p1_x + 200, p1_y + 46, is_clone=False, action="stand")
    draw.text((p1_x + 10, p1_y + 205), "Santuario Elemental no coracao do Castelo de Hyrule.\nAltar sagrado aguarda a White Sword e os 2 Elementos!", fill=(254, 240, 138, 255))

    # =============================================================
    # PANEL 2: INFUSION CUTSCENE
    # =============================================================
    p2_x, p2_y = 510, 94
    draw_sanctuary_room(p2_x, p2_y, gate_open=False)

    # Infusion light vortex around pedestal
    ax, ay = p2_x + 208, p2_y + 36
    draw.ellipse([ax - 40, ay - 30, ax + 40, ay + 30], fill=(255, 255, 255, 120))
    # Earth orb swirling
    draw.ellipse([ax - 26, ay - 24, ax - 10, ay - 8], fill=(34, 197, 94, 255), outline=(134, 239, 172, 255))
    draw.line([ax - 50, ay - 4, ax - 18, ay - 16], fill=(34, 197, 94, 180), width=2)
    # Fire orb swirling
    draw.ellipse([ax + 10, ay + 8, ax + 26, ay + 24], fill=(239, 68, 68, 255), outline=(254, 240, 138, 255))
    draw.line([ax + 50, ay - 4, ax + 18, ay + 16], fill=(239, 68, 68, 180), width=2)

    # Infused Sword with dual glow
    draw.line([ax, ay - 24, ax, ay - 4], fill=(255, 255, 255, 255), width=3)
    draw.point((ax - 2, ay - 18), fill=(34, 197, 94, 255))
    draw.point((ax + 2, ay - 14), fill=(239, 68, 68, 255))

    # Banner
    draw.rectangle([p2_x + 40, p2_y + 90, p2_x + 380, p2_y + 126], fill=(10, 37, 64, 240), outline=(56, 189, 248, 255))
    draw.text((p2_x + 50, p2_y + 96), "ESPADA BRANCA (2 ELEMENTOS) FORJADA!", fill=(255, 255, 255, 255))
    draw.text((p2_x + 50, p2_y + 110), "Habilidade Desbloqueada: Divisao em 2 Clones!", fill=(56, 189, 248, 255))
    draw.text((p2_x + 10, p2_y + 205), "Terra e Fogo despertam a magia quadrupla na lamina!\nA White Sword agora gera um Clone em pisos emissores.", fill=(125, 211, 252, 255))

    # =============================================================
    # PANEL 3: FOUR SWORD CHARGE & 2-CLONE SPLIT
    # =============================================================
    p3_x, p3_y = 30, 384
    draw_sanctuary_room(p3_x, p3_y, gate_open=False)
    draw_split_pads(p3_x, p3_y, lit1=True, lit2=True)
    draw_dual_switches(p3_x, p3_y, sw1_down=False, sw2_down=False)

    # Link on Pad 1 charging
    draw_hero(p3_x + 154, p3_y + 105, is_clone=False, action="charge")
    # 2nd Link (Clone) spawned on Pad 2!
    draw_hero(p3_x + 254, p3_y + 105, is_clone=True, action="charge")

    # Split connection beam between the two
    draw.line([p3_x + 168, p3_y + 118, p3_x + 254, p3_y + 118], fill=(56, 189, 248, 160), width=2)
    draw.text((p3_x + 10, p3_y + 205), "Segurando [A], a lamina carrega energia elemental!\nPisando nos 2 pads emissores: O Herói se divide em 2!", fill=(56, 189, 248, 255))

    # =============================================================
    # PANEL 4: DUAL SWITCHES & INNER GATE ELEVATION
    # =============================================================
    p4_x, p4_y = 510, 384
    draw_sanctuary_room(p4_x, p4_y, gate_open=True) # Gate elevated!
    draw_split_pads(p4_x, p4_y, lit1=False, lit2=False)
    draw_dual_switches(p4_x, p4_y, sw1_down=True, sw2_down=True)

    # Link on Switch 1 slashing
    draw_hero(p4_x + 154, p4_y + 165, is_clone=False, action="slash")
    # Clone on Switch 2 slashing simultaneously!
    draw_hero(p4_x + 254, p4_y + 165, is_clone=True, action="slash")

    draw.text((p4_x + 150, p4_y + 148), "SWITCH 1", fill=(34, 197, 94, 255))
    draw.text((p4_x + 250, p4_y + 148), "SWITCH 2", fill=(34, 197, 94, 255))
    draw.text((p4_x + 160, p4_y + 50), "PORTAO SAGRADO ELEVADO!", fill=(254, 240, 138, 255))

    # Banner notification at bottom
    draw.rectangle([p4_x + 20, p4_y + 200, p4_x + 400, p4_y + 224], fill=(10, 37, 64, 240), outline=(56, 189, 248, 255))
    draw.text((p4_x + 30, p4_y + 206), "QUEBRA-CABECA RESOLVIDO COM 2 CLONES! Avanco para Castor Wilds.", fill=(254, 240, 138, 255))

    # Save images
    os.makedirs(os.path.dirname(scratch_path), exist_ok=True)
    img.save(scratch_path)
    img.save(out_path)
    print(f"Showcase gerado com sucesso em: {scratch_path} e {out_path}")

if __name__ == "__main__":
    main()
