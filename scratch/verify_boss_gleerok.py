import sys
import os
import math
from PIL import Image, ImageDraw

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "boss_gleerok_showcase.png")
    scratch_path = r"d:\REPOS\TLoZ-MC\scratch\boss_gleerok_showcase.png"

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (12, 10, 16, 255))
    draw = ImageDraw.Draw(img)

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(24, 16, 18, 255))
    draw.line([0, 48, img_w, 48], fill=(239, 68, 68, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Boss Gleerok (Cave of Flames) Showcase", fill=(254, 240, 138, 255))
    draw.text((680, 16), "Item 8: feature/boss-gleerok", fill=(248, 113, 113, 255))

    panels = [
        ("1. ARENA & GLEEROK EMERGES SPEWING FIREBALLS", 20, 60, 460, 330),
        ("2. CANE OF PACCI HIT: INVERTING ROCKY CARAPACE", 500, 60, 940, 330),
        ("3. VULNERABLE BRIDGE & WHITE SWORD ON RUBY CORE", 20, 350, 460, 620),
        ("4. DEFEAT: SACRED FIRE ELEMENT & WARP PORTAL", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(18, 16, 20, 255), outline=(60, 50, 55, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(35, 22, 26, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(220, 38, 38, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(251, 146, 60, 255))

    # Helper function to draw volcanic arena island
    def draw_arena_island(px, py):
        # Molten lava perimeter
        for ry in range(15):
            for rx in range(26):
                lx = px + rx * 16
                ly = py + ry * 16
                if rx < 3 or rx > 22 or ry < 2 or ry > 12:
                    draw.rectangle([lx, ly, lx + 15, ly + 15], fill=(220, 38, 38, 255))
                    draw.rectangle([lx + 2, ly + 2, lx + 13, ly + 13], fill=(245, 158, 11, 255))
                    draw.rectangle([lx + 5, ly + 5, lx + 10, ly + 10], fill=(254, 240, 138, 255))
                else:
                    # Basalt island
                    draw.rectangle([lx, ly, lx + 15, ly + 15], fill=(41, 37, 36, 255))
                    draw.line([lx, ly + 15, lx + 15, ly + 15], fill=(68, 64, 60, 255))
                    draw.line([lx + 15, ly, lx + 15, ly + 15], fill=(68, 64, 60, 255))

        # 4 Corner stone torches
        for tx, ty in [(px + 4 * 16, py + 3 * 16), (px + 21 * 16, py + 3 * 16),
                       (px + 4 * 16, py + 11 * 16), (px + 21 * 16, py + 11 * 16)]:
            draw.rectangle([tx + 4, ty + 8, tx + 12, ty + 18], fill=(82, 82, 91, 255))
            draw.rectangle([tx + 5, ty + 2, tx + 11, ty + 8], fill=(239, 68, 68, 255))
            draw.rectangle([tx + 6, ty + 1, tx + 10, ty + 5], fill=(253, 224, 71, 255))

    # Helper function to draw Gleerok Dragon
    def draw_gleerok(gx, gy, state="active"):
        if state == "active":
            # Rocky carapace with magma veins
            draw.rectangle([gx, gy, gx + 96, gy + 52], fill=(28, 25, 23, 255))
            draw.rectangle([gx + 4, gy + 4, gx + 92, gy + 48], fill=(41, 37, 36, 255))
            # Magma fissures
            for fx in [gx + 20, gx + 48, gx + 76]:
                draw.rectangle([fx, gy + 10, fx + 8, gy + 42], fill=(220, 38, 38, 255))
                draw.rectangle([fx + 2, gy + 14, fx + 6, gy + 38], fill=(249, 115, 22, 255))
                draw.line([fx + 4, gy + 16, fx + 4, gy + 36], fill=(254, 240, 138, 255))

            # Horn spikes
            for hx in [gx + 8, gx + 44, gx + 80]:
                draw.polygon([(hx, gy), (hx + 8, gy - 14), (hx + 16, gy)], fill=(217, 119, 6, 255))

            # Serpentine neck
            draw.rectangle([gx + 36, gy + 48, gx + 60, gy + 80], fill=(28, 25, 23, 255))
            draw.rectangle([gx + 40, gy + 50, gx + 56, gy + 78], fill=(41, 37, 36, 255))
            draw.line([gx + 48, gy + 52, gx + 48, gy + 76], fill=(220, 38, 38, 255), width=2)

            # Dragon Head
            draw.rectangle([gx + 30, gy + 76, gx + 66, gy + 104], fill=(28, 25, 23, 255))
            draw.rectangle([gx + 34, gy + 80, gx + 62, gy + 100], fill=(41, 37, 36, 255))
            # Head Horns
            draw.polygon([(gx + 30, gy + 78), (gx + 20, gy + 66), (gx + 36, gy + 80)], fill=(217, 119, 6, 255))
            draw.polygon([(gx + 66, gy + 78), (gx + 76, gy + 66), (gx + 60, gy + 80)], fill=(217, 119, 6, 255))
            # Fiery Eyes
            draw.rectangle([gx + 38, gy + 84, gx + 44, gy + 90], fill=(253, 224, 71, 255))
            draw.rectangle([gx + 52, gy + 84, gx + 58, gy + 90], fill=(253, 224, 71, 255))
            draw.point((gx + 41, gy + 87), fill=(220, 38, 38, 255))
            draw.point((gx + 55, gy + 87), fill=(220, 38, 38, 255))
            # Snout spewing fire embers
            draw.rectangle([gx + 42, gy + 96, gx + 54, gy + 102], fill=(249, 115, 22, 255))
            draw.rectangle([gx + 44, gy + 98, gx + 52, gy + 104], fill=(254, 240, 138, 255))

        elif state == "toppled":
            # Overturned carapace
            draw.rectangle([gx, gy, gx + 96, gy + 48], fill=(28, 25, 23, 255))
            draw.rectangle([gx + 4, gy + 4, gx + 92, gy + 44], fill=(41, 37, 36, 255))

            # Pulsing Ruby Core on back!
            cx, cy = gx + 32, gy + 10
            draw.rectangle([cx - 4, cy - 4, cx + 36, cy + 32], fill=(220, 38, 38, 120))
            draw.rectangle([cx, cy, cx + 32, cy + 28], fill=(239, 68, 68, 255))
            draw.rectangle([cx + 6, cy + 5, cx + 26, cy + 23], fill=(249, 115, 22, 255))
            draw.rectangle([cx + 11, cy + 9, cx + 21, cy + 19], fill=(254, 240, 138, 255))
            draw.rectangle([cx + 14, cy + 12, cx + 18, cy + 16], fill=(255, 255, 255, 255))

            # Collapsed neck forming a ramp onto the island
            draw.rectangle([gx + 36, gy + 44, gx + 60, gy + 96], fill=(28, 25, 23, 255))
            draw.rectangle([gx + 40, gy + 46, gx + 56, gy + 94], fill=(120, 53, 15, 255))

            # Head resting flat on stone floor
            draw.rectangle([gx + 30, gy + 94, gx + 66, gy + 114], fill=(28, 25, 23, 255))
            draw.rectangle([gx + 34, gy + 96, gx + 62, gy + 110], fill=(41, 37, 36, 255))
            draw.line([gx + 38, gy + 102, gx + 44, gy + 102], fill=(30, 41, 59, 255), width=2)
            draw.line([gx + 52, gy + 102, gx + 58, gy + 102], fill=(30, 41, 59, 255), width=2)

    # Helper function to draw Link
    def draw_link(lx, ly, dir="up", action="stand"):
        # Green Tunic & Cap
        draw.rectangle([lx + 4, ly + 2, lx + 12, ly + 8], fill=(34, 197, 94, 255)) # Cap
        draw.rectangle([lx + 4, ly + 8, lx + 12, ly + 14], fill=(253, 232, 205, 255)) # Face
        draw.rectangle([lx + 2, ly + 14, lx + 14, ly + 24], fill=(22, 163, 74, 255)) # Tunic
        draw.rectangle([lx + 4, ly + 24, lx + 12, ly + 28], fill=(255, 255, 255, 255)) # Tights
        draw.rectangle([lx + 4, ly + 28, lx + 12, ly + 32], fill=(180, 83, 9, 255)) # Boots
        if action == "pacci":
            # Blue Cane of Pacci
            draw.line([lx + 14, ly + 10, lx + 26, ly + 4], fill=(56, 189, 248, 255), width=3)
            draw.rectangle([lx + 24, ly + 2, lx + 28, ly + 6], fill=(254, 240, 138, 255))
        elif action == "slash":
            # White Sword slash
            draw.line([lx + 8, ly + 6, lx + 8, ly - 14], fill=(255, 255, 255, 255), width=4)
            draw.line([lx + 9, ly + 6, lx + 9, ly - 14], fill=(186, 230, 253, 255), width=2)
            draw.rectangle([lx + 4, ly + 4, lx + 12, ly + 7], fill=(250, 204, 21, 255))

    # Helper function to draw Boss Health Bar
    def draw_boss_hud(bx, by, hp=10):
        draw.rectangle([bx, by, bx + 160, by + 16], fill=(11, 15, 25, 255), outline=(68, 64, 60, 255))
        draw.text((bx + 10, by - 12), "GLEEROK - DRAGAO DE FOGO", fill=(249, 115, 22, 255))
        for p in range(10):
            col = (239, 68, 68, 255) if p < hp else (68, 64, 60, 255)
            draw.rectangle([bx + 4 + p * 15, by + 3, bx + 16 + p * 15, by + 13], fill=col)
            if p < hp:
                draw.line([bx + 5 + p * 15, by + 4, bx + 15 + p * 15, by + 4], fill=(254, 240, 138, 255))

    # =============================================================
    # PANEL 1: GLEEROK EMERGES & SPEWS FIREBALLS
    # =============================================================
    p1_x, p1_y = 30, 94
    draw_arena_island(p1_x, p1_y)
    draw_gleerok(p1_x + 160, p1_y + 10, state="active")
    draw_boss_hud(p1_x + 130, p1_y + 18, hp=10)

    # Fireballs travelling towards Link
    for fbx, fby in [(p1_x + 200, p1_y + 130), (p1_x + 180, p1_y + 160)]:
        draw.ellipse([fbx - 8, fby - 8, fbx + 8, fby + 8], fill=(220, 38, 38, 255))
        draw.ellipse([fbx - 5, fby - 5, fbx + 5, fby + 5], fill=(249, 115, 22, 255))
        draw.ellipse([fbx - 2, fby - 2, fbx + 2, fby + 2], fill=(255, 255, 255, 255))

    # Link dodging on the stone platform
    draw_link(p1_x + 160, p1_y + 185, dir="up", action="stand")
    draw.text((p1_x + 10, p1_y + 205), "Carapaca blindada de basalto resvala a espada!\nDispara salvas de Bolas de Fogo incandescentes!", fill=(254, 240, 138, 255))

    # =============================================================
    # PANEL 2: CANE OF PACCI INVERSION
    # =============================================================
    p2_x, p2_y = 510, 94
    draw_arena_island(p2_x, p2_y)
    draw_gleerok(p2_x + 160, p2_y + 10, state="active")

    # Link firing Cane of Pacci
    draw_link(p2_x + 195, p2_y + 185, dir="up", action="pacci")

    # Cyan Pacci magical energy projectile & explosion
    for step in range(5):
        ey = p2_y + 180 - step * 18
        draw.ellipse([p2_x + 205 - 6, ey - 6, p2_x + 205 + 6, ey + 6], fill=(56, 189, 248, 200))
    # Magic blast on Gleerok's shell
    bx, by = p2_x + 208, p2_y + 55
    draw.ellipse([bx - 24, by - 24, bx + 24, by + 24], fill=(56, 189, 248, 140))
    draw.ellipse([bx - 14, by - 14, bx + 14, by + 14], fill=(255, 255, 255, 220))
    draw.text((p2_x + 10, p2_y + 205), "[CANE OF PACCI] Raio magico inverte a carapaca!\nCouraça de rocha e virada e o dragao tomba ao solo!", fill=(56, 189, 248, 255))

    # =============================================================
    # PANEL 3: VULNERABLE BRIDGE & WHITE SWORD SLASH
    # =============================================================
    p3_x, p3_y = 30, 384
    draw_arena_island(p3_x, p3_y)
    draw_gleerok(p3_x + 160, p3_y + 10, state="toppled")
    draw_boss_hud(p3_x + 130, p3_y + 18, hp=4)

    # Link ascending the neck ramp and slashing Ruby Core
    draw_link(p3_x + 200, p3_y + 40, dir="up", action="slash")
    # Slash sparks & damage indicator
    draw.text((p3_x + 225, p3_y + 35), "HIT! -2 HP", fill=(254, 240, 138, 255))
    draw.ellipse([p3_x + 204, p3_y + 20, p3_x + 220, p3_y + 36], fill=(255, 255, 255, 180))
    draw.text((p3_x + 10, p3_y + 205), "Pescoco forma uma rampa acessivel ate o dorso!\nWhite Sword causa dano duplicado no Nucleo de Rubi!", fill=(248, 113, 113, 255))

    # =============================================================
    # PANEL 4: DEFEAT & SACRED FIRE ELEMENT
    # =============================================================
    p4_x, p4_y = 510, 384
    draw_arena_island(p4_x, p4_y)

    # Sinking defeat magma bubbles
    for bx, by in [(p4_x + 180, p4_y + 30), (p4_x + 210, p4_y + 40), (p4_x + 240, p4_y + 25)]:
        draw.ellipse([bx - 12, by - 12, bx + 12, by + 12], fill=(254, 240, 138, 255))
        draw.ellipse([bx - 6, by - 6, bx + 6, by + 6], fill=(255, 255, 255, 255))

    # Heart Container item
    hx, hy = p4_x + 140, p4_y + 110
    draw.polygon([(hx, hy + 4), (hx + 8, hy - 4), (hx + 16, hy + 4), (hx + 8, hy + 16)], fill=(239, 68, 68, 255))
    draw.text((hx - 15, hy + 20), "HEART CONTAINER", fill=(248, 113, 113, 255))

    # SACRED FIRE ELEMENT
    ex, ey = p4_x + 260, p4_y + 100
    draw.ellipse([ex - 22, ey - 22, ex + 22, ey + 22], fill=(249, 115, 22, 100))
    draw.rectangle([ex - 12, ey - 16, ex + 12, ey + 16], fill=(245, 158, 11, 255), outline=(120, 53, 15, 255))
    # Inner sacred flame
    draw.polygon([(ex - 6, ey + 10), (ex, ey - 12), (ex + 6, ey + 10)], fill=(220, 38, 38, 255))
    draw.polygon([(ex - 3, ey + 8), (ex, ey - 6), (ex + 3, ey + 8)], fill=(254, 240, 138, 255))
    draw.text((ex - 30, ey + 22), "FIRE ELEMENT", fill=(254, 240, 138, 255))

    # Blue Warp Portal
    wx, wy = p4_x + 200, p4_y + 155
    draw.ellipse([wx - 26, wy - 26, wx + 26, wy + 26], fill=(2, 132, 199, 100))
    draw.ellipse([wx - 18, wy - 18, wx + 18, wy + 18], fill=(56, 189, 248, 160))
    draw.ellipse([wx - 10, wy - 10, wx + 10, wy + 10], fill=(224, 242, 254, 255))
    draw.text((wx - 25, wy + 30), "WARP PORTAL", fill=(56, 189, 248, 255))

    # Banner notification at bottom
    draw.rectangle([p4_x + 20, p4_y + 200, p4_x + 390, p4_y + 224], fill=(26, 5, 5, 240), outline=(249, 115, 22, 255))
    draw.text((p4_x + 30, p4_y + 206), "ELEMENTO FOGO OBTIDO! Melari's Mines retorno ativo.", fill=(254, 240, 138, 255))

    # Save images
    os.makedirs(os.path.dirname(scratch_path), exist_ok=True)
    img.save(scratch_path)
    img.save(out_path)
    print(f"Showcase gerado com sucesso em: {scratch_path} e {out_path}")

if __name__ == "__main__":
    main()
