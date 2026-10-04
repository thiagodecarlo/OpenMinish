import sys
import os
import math
from PIL import Image, ImageDraw

def main():
    out_dir = r"C:\Users\thiag\.gemini\antigravity\brain\63e5c37b-a5b5-41d8-bfe9-4100a1f4e6be"
    out_path = os.path.join(out_dir, "ocarina_fast_travel_showcase.png")
    scratch_path = r"d:\REPOS\TLoZ-MC\scratch\ocarina_fast_travel_showcase.png"

    img_w = 960
    img_h = 640
    img = Image.new("RGBA", (img_w, img_h), (8, 14, 22, 255))
    draw = ImageDraw.Draw(img)

    # Title Banner
    draw.rectangle([0, 0, img_w, 48], fill=(15, 23, 42, 255))
    draw.line([0, 48, img_w, 48], fill=(56, 189, 248, 255), width=2)
    draw.text((24, 14), "The Legend of Zelda: The Minish Cap - Ocarina of Wind, Zeffa & Wind Crest Fast Travel", fill=(224, 242, 254, 255))
    draw.text((640, 16), "Item 4 of 10: feature/ocarina-fast-travel", fill=(56, 189, 248, 255))

    panels = [
        ("1. OCARINA OF WIND & SONG OF THE WIND HARMONY", 20, 60, 460, 330),
        ("2. ZEFFA SWOOPS DOWN FROM THE HEAVENS", 500, 60, 940, 330),
        ("3. INTERACTIVE OVERWORLD FAST TRAVEL MAP", 20, 350, 460, 620),
        ("4. SKY FLIGHT & TOUCHDOWN ON WIND CREST", 500, 350, 940, 620)
    ]

    for title, x1, y1, x2, y2 in panels:
        draw.rectangle([x1, y1, x2, y2], fill=(15, 23, 42, 255), outline=(30, 41, 59, 255), width=2)
        draw.rectangle([x1, y1, x2, y1 + 24], fill=(30, 41, 59, 255))
        draw.line([x1, y1 + 24, x2, y1 + 24], fill=(56, 189, 248, 255), width=1)
        draw.text((x1 + 10, y1 + 6), title, fill=(186, 230, 253, 255))

    # Palettes
    C_WHITE     = (248, 250, 252, 255)
    C_SHADE     = (203, 213, 225, 255)
    C_GOLD      = (245, 158, 11, 255)
    C_CYAN      = (56, 189, 248, 255)
    C_TALON     = (217, 119, 6, 255)
    C_RUBY      = (220, 38, 38, 255)
    C_GRASS     = (34, 197, 94, 255)
    C_DIRT      = (180, 83, 9, 255)

    def draw_link_human(x, y):
        # Hat & tunic
        draw.rectangle([x + 4, y + 2, x + 11, y + 6], fill=(34, 197, 94, 255))
        draw.rectangle([x + 5, y + 6, x + 10, y + 10], fill=(253, 224, 71, 255))
        draw.rectangle([x + 4, y + 10, x + 11, y + 15], fill=(34, 197, 94, 255))
        draw.rectangle([x + 5, y + 15, x + 10, y + 18], fill=(255, 255, 255, 255))
        draw.rectangle([x + 5, y + 18, x + 10, y + 21], fill=(161, 98, 7, 255))

    def draw_zeffa(x, y, wing_up=True, carrying=False):
        # Body
        draw.rectangle([x + 10, y + 6, x + 22, y + 16], fill=C_WHITE)
        draw.rectangle([x + 11, y + 8, x + 21, y + 14], fill=C_SHADE)
        # Tail
        draw.rectangle([x + 14, y + 16, x + 18, y + 22], fill=C_WHITE)
        draw.rectangle([x + 13, y + 21, x + 19, y + 23], fill=C_CYAN)
        # Head & Crest
        draw.rectangle([x + 12, y, x + 20, y + 7], fill=C_WHITE)
        draw.rectangle([x + 14, y - 3, x + 18, y + 1], fill=C_GOLD)
        draw.rectangle([x + 15, y - 5, x + 17, y - 2], fill=C_GOLD)
        # Beak & Eyes
        draw.rectangle([x + 14, y + 6, x + 18, y + 9], fill=C_GOLD)
        draw.point((x + 13, y + 2), fill=C_RUBY)
        draw.point((x + 17, y + 2), fill=C_RUBY)

        # Wings
        if wing_up:
            draw.polygon([(x, y - 4), (x + 11, y + 4), (x + 1, y - 8)], fill=C_WHITE)
            draw.polygon([(x + 21, y + 4), (x + 32, y - 4), (x + 31, y - 8)], fill=C_WHITE)
            draw.line([x, y - 6, x + 4, y - 7], fill=C_CYAN, width=2)
            draw.line([x + 28, y - 7, x + 32, y - 6], fill=C_CYAN, width=2)
        else:
            draw.polygon([(x - 2, y + 4), (x + 11, y + 12), (x - 4, y + 10)], fill=C_WHITE)
            draw.polygon([(x + 21, y + 12), (x + 34, y + 4), (x + 36, y + 10)], fill=C_WHITE)
            draw.line([x - 4, y + 8, x - 1, y + 11], fill=C_CYAN, width=2)
            draw.line([x + 33, y + 11, x + 36, y + 8], fill=C_CYAN, width=2)

        # Talons
        draw.rectangle([x + 11, y + 15, x + 14, y + 19], fill=C_TALON)
        draw.rectangle([x + 18, y + 15, x + 21, y + 19], fill=C_TALON)

        if carrying:
            draw_link_human(x + 8, y + 18)

    def draw_wind_crest(x, y):
        # Stone slab with cyan swirling spiral
        draw.ellipse([x + 1, y + 1, x + 15, y + 15], fill=(71, 85, 105, 255), outline=(30, 41, 59, 255))
        draw.ellipse([x + 3, y + 3, x + 13, y + 13], fill=(30, 41, 59, 255))
        draw.arc([x + 4, y + 4, x + 12, y + 12], 0, 270, fill=C_CYAN, width=2)
        draw.ellipse([x + 7, y + 7, x + 9, y + 9], fill=(224, 242, 254, 255))

    # --- PANEL 1: Ocarina & Song of the Wind ---
    p1_x, p1_y = 35, 95
    # Grass terrain
    for row in range(12):
        for col in range(25):
            gx = p1_x + col * 16
            gy = p1_y + row * 16
            draw.rectangle([gx, gy, gx + 15, gy + 15], fill=(22, 101, 52, 255), outline=(20, 83, 45, 255))
            if (col + row) % 5 == 0:
                draw.point((gx + 4, gy + 6), fill=(74, 222, 128, 255))

    # Link playing Ocarina in the center
    lx = p1_x + 180
    ly = p1_y + 110
    draw_link_human(lx, ly)
    # Blue Ocarina held in front
    draw.ellipse([lx + 6, ly + 8, lx + 16, ly + 14], fill=C_CYAN, outline=(255, 255, 255, 255))
    draw.point((lx + 9, ly + 11), fill=(15, 23, 42, 255))
    draw.point((lx + 13, ly + 11), fill=(15, 23, 42, 255))

    # Floating musical notes drifting upwards
    notes = [
        (lx - 20, ly - 20, (56, 189, 248, 255)),
        (lx + 10, ly - 35, (254, 240, 138, 255)),
        (lx + 35, ly - 15, (52, 211, 153, 255)),
        (lx - 10, ly - 55, (244, 114, 182, 255)),
        (lx + 25, ly - 65, (56, 189, 248, 255))
    ]
    for nx, ny, ncol in notes:
        draw.ellipse([nx, ny + 4, nx + 6, ny + 8], fill=ncol)
        draw.line([nx + 5, ny, nx + 5, ny + 6], fill=ncol, width=2)
        draw.line([nx + 5, ny, nx + 10, ny + 2], fill=ncol, width=2)

    # Wind swirls around Link
    for r in range(15, 45, 10):
        draw.arc([lx - r, ly - r + 10, lx + 16 + r, ly + 16 + r], 0, 180, fill=(186, 230, 253, 140), width=1)

    draw.text((p1_x + 10, p1_y + 198), "Playing Ocarina plays Song of Wind; celestial harmony summons Zeffa", fill=(148, 163, 184, 255))

    # --- PANEL 2: Zeffa Swoops Down ---
    p2_x, p2_y = 515, 95
    for row in range(12):
        for col in range(25):
            gx = p2_x + col * 16
            gy = p2_y + row * 16
            draw.rectangle([gx, gy, gx + 15, gy + 15], fill=(22, 101, 52, 255), outline=(20, 83, 45, 255))

    # Wind gusts descending
    draw.line([p2_x + 80, p2_y + 20, p2_x + 180, p2_y + 100], fill=(56, 189, 248, 120), width=2)
    draw.line([p2_x + 280, p2_y + 20, p2_x + 220, p2_y + 100], fill=(56, 189, 248, 120), width=2)

    # Zeffa swooping in mid-air
    zx = p2_x + 175
    zy = p2_y + 60
    draw_zeffa(zx, zy, wing_up=False, carrying=False)

    # Link looking up in awe
    draw_link_human(p2_x + 185, p2_y + 120)

    # Sound wave arc
    draw.arc([zx - 10, zy - 10, zx + 42, zy + 30], 180, 360, fill=(254, 240, 138, 255), width=2)

    draw.text((p2_x + 15, p2_y + 198), "Zeffa hears the sacred song, sweeps down & grasps Link with talons", fill=(148, 163, 184, 255))

    # --- PANEL 3: World Map Fast Travel UI ---
    p3_x, p3_y = 35, 385
    # Map Window
    mx = p3_x + 20
    my = p3_y + 10
    mw = 360
    mh = 190
    draw.rectangle([mx, my, mx + mw, my + mh], fill=(23, 37, 84, 255), outline=(71, 85, 105, 255), width=2)
    # Title bar
    draw.rectangle([mx, my, mx + mw, my + 24], fill=(30, 41, 59, 255))
    draw.line([mx, my + 24, mx + mw, my + 24], fill=C_CYAN, width=1)
    draw.text((mx + 10, my + 6), "MAPA DE HYRULE - VOO DE ZEFFA", fill=(186, 230, 253, 255))

    # Kingdom landmasses
    draw.rectangle([mx + 30, my + 35, mx + 320, my + 150], fill=(20, 83, 45, 255)) # Hyrule continent
    draw.rectangle([mx + 50, my + 45, mx + 130, my + 95], fill=(120, 53, 15, 255)) # Mt Crenel
    draw.rectangle([mx + 40, my + 110, mx + 120, my + 145], fill=(30, 58, 30, 255)) # Castor Wilds
    draw.rectangle([mx + 230, my + 70, mx + 290, my + 130], fill=(3, 105, 161, 255)) # Lake Hylia
    draw.rectangle([mx + 140, my + 80, mx + 195, my + 120], fill=(217, 119, 6, 255)) # Hyrule Town

    # Crests plotted
    crests = [
        ("Town Center", mx + 165, my + 95, True, True),
        ("Minish Woods", mx + 250, my + 140, True, False),
        ("Mt. Crenel", mx + 90, my + 65, True, False),
        ("Castor Wilds", mx + 75, my + 125, True, False),
        ("Wind Ruins", mx + 85, my + 145, True, False),
        ("Lake Hylia", mx + 260, my + 95, True, False),
    ]

    for cname, cx, cy, unl, is_sel in crests:
        draw.ellipse([cx - 4, cy - 4, cx + 4, cy + 4], fill=C_CYAN, outline=(255, 255, 255, 255))
        if is_sel:
            # Pulsing selection ring
            draw.ellipse([cx - 8, cy - 8, cx + 8, cy + 8], outline=(253, 224, 71, 255), width=2)
            draw.polygon([(cx - 12, cy), (cx - 9, cy - 3), (cx - 9, cy + 3)], fill=(253, 224, 71, 255))

    # Bottom selection bar
    draw.rectangle([mx, my + mh - 26, mx + mw, my + mh], fill=(15, 23, 42, 240))
    draw.line([mx, my + mh - 26, mx + mw, my + mh - 26], fill=(245, 158, 11, 255), width=1)
    draw.text((mx + 10, my + mh - 20), "Cidade de Hyrule (Town Center)", fill=(253, 224, 71, 255))
    draw.text((mx + 220, my + mh - 18), "[A] VOAR   [B] CANCELAR", fill=(148, 163, 184, 255))

    # --- PANEL 4: Flight & Landing on Wind Crest ---
    p4_x, p4_y = 515, 385
    for row in range(12):
        for col in range(25):
            gx = p4_x + col * 16
            gy = p4_y + row * 16
            draw.rectangle([gx, gy, gx + 15, gy + 15], fill=(22, 101, 52, 255), outline=(20, 83, 45, 255))

    # Wind Crest tile on ground
    wc_x = p4_x + 180
    wc_y = p4_y + 115
    draw_wind_crest(wc_x, wc_y)

    # Zeffa landing right on the crest carrying Link
    draw_zeffa(wc_x - 8, wc_y - 20, wing_up=True, carrying=True)

    # Wind swirl & dust puffs at landing point
    draw.arc([wc_x - 12, wc_y - 4, wc_x + 28, wc_y + 24], 0, 360, fill=C_CYAN, width=2)
    draw.ellipse([wc_x - 14, wc_y + 10, wc_x - 6, wc_y + 16], fill=(226, 232, 240, 180))
    draw.ellipse([wc_x + 22, wc_y + 10, wc_x + 30, wc_y + 16], fill=(226, 232, 240, 180))

    # Sparkle stars
    draw.text((wc_x - 22, wc_y - 10), "+", fill=C_CYAN)
    draw.text((wc_x + 36, wc_y - 12), "+", fill=(254, 240, 138, 255))

    draw.text((p4_x + 15, p4_y + 198), "Zeffa glides across Hyrule and touches down gracefully on Wind Crest", fill=(56, 189, 248, 255))

    img.save(out_path)
    img.save(scratch_path)
    print(f"Showcase successfully written to:\n- {out_path}\n- {scratch_path}")

if __name__ == "__main__":
    main()
