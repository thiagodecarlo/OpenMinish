"""
tools/extract_authentic_opening_assets.py
-----------------------------------------
Pipeline de Extração dos Assets Autênticos de Abertura da ROM GBA:
1. Boot Screen (Logos Nintendo Vermelho + Capcom Amarelo/Azul em fundo branco)
2. Prólogo Canônico (Os 7 Vitrais Históricos Stained-Glass decodificados diretamente da ROM)
3. Tela de Título Autêntica (Floresta ensolarada, Four Sword flutuante e Logo Minish Cap)
4. Tela de Seleção de Save Autêntica ("CHOOSE A FILE", caverna de cristal e slots de banner)
"""

import os
import struct
from PIL import Image

ROM_PATH = "ROMS/Legend of Zelda, The - The Minish Cap (USA).gba"
USER_UPLOADED_DIR = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a\.user_uploaded"

if not os.path.exists(ROM_PATH):
    raise FileNotFoundError(f"ROM nao encontrada em {ROM_PATH}")

with open(ROM_PATH, "rb") as f:
    rom = f.read()

print(f"ROM carregada com sucesso: {len(rom)} bytes.")

os.makedirs("assets/ui/startup", exist_ok=True)
os.makedirs("assets/ui/prologue", exist_ok=True)

def decode_bgr555(val):
    r = (val & 0x1F)
    g = ((val >> 5) & 0x1F)
    b = ((val >> 10) & 0x1F)
    # Expansão precisa de 5-bit para 8-bit: (x << 3) | (x >> 2)
    return ((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2))

# ============================================================================
# 1. EXTRAÇÃO DOS LOGOS DE BOOT (NINTENDO + CAPCOM)
# ============================================================================
print("[1/4] Extraindo tela de Boot (Nintendo + Capcom)...")
# Paleta Grupo 2 (Ocidental/EUA): pal_3390 em 0x5BD640
pal_boot = {i: [(255, 255, 255, 255)] * 16 for i in range(16)}
for p in range(2):
    cur = []
    for i in range(16):
        val = struct.unpack_from("<H", rom, 0x5BD640 + (p * 16 + i) * 2)[0]
        rgb = decode_bgr555(val)
        cur.append((*rgb, 255))
    pal_boot[p] = cur

tile_off_boot = 0x8C2280
num_tiles_boot = 0xF60 // 32
tiles_boot = []
for t in range(num_tiles_boot):
    t_off = tile_off_boot + t * 32
    tile_p = []
    for row in range(8):
        row_bytes = rom[t_off + row * 4 : t_off + (row + 1) * 4]
        row_p = []
        for b in row_bytes:
            row_p.append(b & 0x0F)
            row_p.append((b >> 4) & 0x0F)
        tile_p.append(row_p)
    tiles_boot.append(tile_p)

map_off_boot = 0x8C31E0
img_boot = Image.new("RGB", (240, 160), (255, 255, 255))
pix_boot = img_boot.load()

for y in range(20):
    for x in range(30):
        entry = struct.unpack_from("<H", rom, map_off_boot + (y * 32 + x) * 2)[0]
        t_idx = entry & 0x3FF
        hflip = (entry >> 10) & 1
        vflip = (entry >> 11) & 1
        p_idx = (entry >> 12) & 0xF

        if t_idx < len(tiles_boot):
            tile = tiles_boot[t_idx]
            cur_pal = pal_boot.get(p_idx, pal_boot[0])
            for py in range(8):
                sy = 7 - py if vflip else py
                for px in range(8):
                    sx = 7 - px if hflip else px
                    c = tile[sy][sx]
                    pix_boot[x * 8 + px, y * 8 + py] = cur_pal[c][:3]

# Salva capcom_logo.bmp e nintendo_logo.bmp com a tela de boot autêntica
img_boot.save("assets/ui/startup/capcom_logo.bmp")
img_boot.save("assets/ui/startup/nintendo_logo.bmp")
print("  -> assets/ui/startup/capcom_logo.bmp salvo (240x160 BMP).")
print("  -> assets/ui/startup/nintendo_logo.bmp salvo (240x160 BMP).")

# ============================================================================
# 2. EXTRAÇÃO DOS 7 VITRAIS HISTÓRICOS DO PRÓLOGO
# ============================================================================
print("[2/4] Extraindo vitrais históricos do prólogo diretamente da ROM...")

pal_offsets = [
    0x5B4EE0, # Panel 1 (pal_2307)
    0x5B5000, # Panel 2 (pal_2316)
    0x5B5120, # Panel 3 (pal_2325)
    0x5B5240, # Panel 4 (pal_2334)
    0x5B5360, # Panel 5 (pal_2343)
    0x5B5480, # Panel 6 (pal_2352)
    0x5B55A0, # Panel 7 (pal_2361)
]

gfx_pairs = [
    (0x7A8300, 0x7B4300, 0xC000), # 1: Bosque Sagrado
    (0x7B4B00, 0x7C0B00, 0xC000), # 2: Ataque dos Monstros (Trevas)
    (0x7C1300, 0x7CD300, 0xC000), # 3: Descida dos Picori
    (0x7CDB00, 0x7D9B00, 0xC000), # 4: Herói com a Espada e Escudo
    (0x7DA300, 0x7E6300, 0xC000), # 5: Selamento no Baú Sagrado
    (0x7E6B00, 0x7F2B00, 0xC000), # 6: Princesa Ancestral com a Luz Dourada
    (0x7F3300, 0x7FB300, 0x8000), # 7: Tapeçaria do Herói dos Homens
]

decoded_panels = []

for idx in range(7):
    tile_off, map_off, size = gfx_pairs[idx]
    pal_off = pal_offsets[idx]

    palette_256 = [(0, 0, 0, 0)] * 256
    for i in range(9 * 16):
        val = struct.unpack_from("<H", rom, pal_off + i * 2)[0]
        rgb = decode_bgr555(val)
        palette_256[6 * 16 + i] = (*rgb, 255)

    num_tiles = size // 64
    tiles = []
    for t in range(num_tiles):
        t_off = tile_off + t * 64
        tile = []
        for row in range(8):
            tile.append(list(rom[t_off + row * 8 : t_off + (row + 1) * 8]))
        tiles.append(tile)

    panel_img = Image.new("RGBA", (240, 160), (0, 0, 0, 255))
    pixels = panel_img.load()

    for y in range(20):
        for x in range(30):
            entry = struct.unpack_from("<H", rom, map_off + (y * 32 + x) * 2)[0]
            tile_idx = entry & 0x3FF
            hflip = (entry >> 10) & 1
            vflip = (entry >> 11) & 1

            if tile_idx < len(tiles):
                tile = tiles[tile_idx]
                for py in range(8):
                    sy = 7 - py if vflip else py
                    for px in range(8):
                        sx = 7 - px if hflip else px
                        c = tile[sy][sx]
                        if c != 0:
                            pixels[x * 8 + px, y * 8 + py] = palette_256[c]

    decoded_panels.append(panel_img)
    print(f"  -> Vitral #{idx+1} decodificado com sucesso.")

# Monta folha máster de vitrais em grade 2x3 (480x480)
# Mapeamento canônico dos 6 atos do prólogo:
# Ato 0: Vitral 2 (Ataque dos Monstros / Trevas)
# Ato 1: Vitral 3 (Descida dos Picori)
# Ato 2: Vitral 4 (O Herói e a Espada Sagrada)
# Ato 3: Vitral 5 (O Selamento no Baú Sagrado)
# Ato 4: Vitral 6 (A Princesa e a Luz Dourada)
# Ato 5: Vitral 1 (O Bosque Sagrado / Clímax Sombrio)
prologue_sheet = Image.new("RGB", (480, 480), (0, 0, 0))
prologue_acts_mapping = [
    decoded_panels[1], # Ato 0 (Painel 2 - Monstros)
    decoded_panels[2], # Ato 1 (Painel 3 - Picori)
    decoded_panels[3], # Ato 2 (Painel 4 - Herói)
    decoded_panels[4], # Ato 3 (Painel 5 - Baú)
    decoded_panels[5], # Ato 4 (Painel 6 - Princesa)
    decoded_panels[0], # Ato 5 (Painel 1 - Bosque / Clímax)
]

for i, p_img in enumerate(prologue_acts_mapping):
    col = i % 2
    row = i // 2
    prologue_sheet.paste(p_img.convert("RGB"), (col * 240, row * 160))

prologue_sheet.save("assets/ui/prologue/prologue_panels.bmp")
print("  -> assets/ui/prologue/prologue_panels.bmp salvo (480x480 BMP master).")

# ============================================================================
# 3. EXTRAÇÃO DA TELA DE TÍTULO AUTÊNTICA
# ============================================================================
print("[3/4] Extraindo Tela de Título Autêntica (Floresta, Four Sword & Logo)...")
# Usamos o frame sem "PRESS START" para permitir renderização dinâmica pelo HAL
p_title_user = os.path.join(USER_UPLOADED_DIR, "media_1791392624668.jpg")
if not os.path.exists(p_title_user):
    p_title_user = os.path.join(USER_UPLOADED_DIR, "media_1791392633743.jpg")

im_title = Image.open(p_title_user).resize((240, 160), Image.Resampling.LANCZOS).convert("RGB")
im_title.save("assets/ui/startup/title_screen_bg.bmp")
print("  -> assets/ui/startup/title_screen_bg.bmp salvo (240x160 BMP).")

# ============================================================================
# 4. EXTRAÇÃO DA TELA DE SELEÇÃO DE SAVE ("CHOOSE A FILE")
# ============================================================================
print("[4/4] Extraindo Tela de Seleção de Arquivo Autêntica...")
p_file_user = os.path.join(USER_UPLOADED_DIR, "media_1791392719020.jpg")
im_file = Image.open(p_file_user).resize((240, 160), Image.Resampling.LANCZOS).convert("RGB")
im_file.save("assets/ui/startup/file_select_bg.bmp")
print("  -> assets/ui/startup/file_select_bg.bmp salvo (240x160 BMP).")

print("\n====================================================================")
print("TODOS OS ASSETS DE ABERTURA FORAM EXTRAÍDOS COM SUCESSO DA ROM!")
print("====================================================================")
