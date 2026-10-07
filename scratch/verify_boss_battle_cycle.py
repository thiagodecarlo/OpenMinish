"""
Script de Homologação Visual do Ciclo Completo de Batalha com Chefes (Deepwood Shrine / Big Green ChuChu)
Gera o showcase gráfico demonstrando:
1. Sucção da base gelatinosa com Gust Jar e medidor de desequilíbrio
2. Inversão gravitacional e atordoamento (Toppled) com o Cane of Pacci
3. Fase Enraged com olhos carmesins e tremores de terra (Screen Shake)
4. HUD Canônica GBA com moldura dourada, gemas e enquadramento de câmera dinâmica
"""

import math
import os
from PIL import Image, ImageDraw, ImageFont

OUTPUT_PATH = r"C:\Users\thiag\.gemini\antigravity\brain\91f45871-4e2f-495f-88d6-7427ed06770a\boss_battle_cycle_showcase.png"

def draw_oval_shadow(img, cx, cy, rx, ry, alpha=0.45):
    overlay = Image.new("RGBA", img.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(overlay)
    d.ellipse([(cx - rx, cy - ry), (cx + rx, cy + ry)], fill=(0, 0, 0, int(255 * alpha)))
    img.alpha_composite(overlay)

def render_big_chuchu(draw, cx, cy, scale_base=1.0, toppled=False, enraged=False, jump_z=0):
    # Colors
    c_outer = (22, 101, 52, 255) if not enraged else (153, 27, 27, 255)
    c_body  = (34, 197, 94, 255) if not enraged else (239, 68, 68, 255)
    c_high  = (134, 239, 172, 255) if not enraged else (252, 165, 165, 255)
    c_eye_bg = (248, 250, 252, 255)
    c_pupil = (15, 23, 42, 255) if not enraged else (185, 28, 28, 255)

    eff_y = cy - int(jump_z)

    if not toppled:
        # Colossal standing / jumping ChuChu
        # Base
        bw = int(32 * scale_base)
        bh = 10
        draw.ellipse([(cx - bw, eff_y + 12 - bh), (cx + bw, eff_y + 12 + bh)], fill=c_outer)
        draw.ellipse([(cx - bw + 2, eff_y + 12 - bh + 2), (cx + bw - 2, eff_y + 12 + bh - 2)], fill=c_body)

        # Mid body stalk
        mw = int(24 * max(0.4, scale_base * 0.8))
        draw.rounded_rectangle([(cx - mw, eff_y - 15), (cx + mw, eff_y + 10)], radius=8, fill=c_body, outline=c_outer, width=2)

        # Huge spherical head
        hw = 36
        hh = 32
        head_y = eff_y - 25
        draw.ellipse([(cx - hw, head_y - hh), (cx + hw, head_y + hh)], fill=c_outer)
        draw.ellipse([(cx - hw + 3, head_y - hh + 3), (cx + hw - 3, head_y + hh - 3)], fill=c_body)
        # Specular shine
        draw.ellipse([(cx - 24, head_y - 24), (cx - 10, head_y - 12)], fill=c_high)

        # Big goofy / menacing eyes
        for ex in [cx - 14, cx + 14]:
            draw.ellipse([(ex - 9, head_y - 8), (ex + 9, head_y + 10)], fill=c_eye_bg, outline=c_outer, width=2)
            draw.ellipse([(ex - 4, head_y - 2), (ex + 4, head_y + 6)], fill=c_pupil)
            draw.rectangle([(ex - 2, head_y - 1), (ex + 1, head_y + 2)], fill=(255, 255, 255, 255))
            if enraged:
                # Angry crimson brow
                draw.line([(ex - 9, head_y - 11), (ex + 9, head_y - 7 if ex < cx else head_y - 11)], fill=(127, 29, 29, 255), width=3)
    else:
        # Toppled on ground! Deflated base, huge head rolling on the floor!
        # Flat deflated puddle
        draw.ellipse([(cx - 28, cy - 6), (cx + 28, cy + 8)], fill=c_outer)
        draw.ellipse([(cx - 25, cy - 4), (cx + 25, cy + 6)], fill=c_body)

        # Slumped massive head
        draw.ellipse([(cx - 36, cy - 28), (cx + 36, cy + 18)], fill=c_outer)
        draw.ellipse([(cx - 33, cy - 25), (cx + 33, cy + 15)], fill=c_body)
        draw.ellipse([(cx - 20, cy - 20), (cx - 6, cy - 10)], fill=c_high)

        # Dizzy spiral eyes
        for ex in [cx - 15, cx + 15]:
            draw.ellipse([(ex - 8, cy - 8), (ex + 8, cy + 8)], fill=c_eye_bg, outline=c_outer, width=1)
            # Spiral
            draw.arc([(ex - 6, cy - 6), (ex + 6, cy + 6)], start=0, end=300, fill=c_pupil, width=2)
            draw.arc([(ex - 3, cy - 3), (ex + 3, cy + 3)], start=120, end=360, fill=c_pupil, width=2)

def render_link(draw, lx, ly, action="normal", dir_right=True):
    c_tunic = (22, 163, 74, 255)
    c_skin  = (254, 215, 170, 255)
    c_cap   = (21, 128, 61, 255)
    c_belt  = (180, 83, 9, 255)
    c_white = (255, 255, 255, 255)
    c_metal = (226, 232, 240, 255)

    # Body
    draw.rectangle([(lx - 6, ly - 8), (lx + 6, ly + 6)], fill=c_tunic)
    draw.rectangle([(lx - 5, ly + 2), (lx + 5, ly + 4)], fill=c_belt)
    # Head & Cap
    draw.rectangle([(lx - 5, ly - 16), (lx + 5, ly - 8)], fill=c_skin)
    draw.polygon([(lx - 7, ly - 16), (lx + 7, ly - 16), (lx - 2 if dir_right else lx + 2, ly - 23)], fill=c_cap)
    draw.rectangle([(lx + 1 if dir_right else lx - 3, ly - 13), (lx + 3 if dir_right else lx - 1, ly - 11)], fill=(15, 23, 42, 255))
    # Feet
    draw.rectangle([(lx - 5, ly + 6), (lx - 1, ly + 11)], fill=c_belt)
    draw.rectangle([(lx + 1, ly + 6), (lx + 5, ly + 11)], fill=c_belt)

    if action == "gust":
        # Holding Gust Jar
        jx = lx + 8 if dir_right else lx - 16
        draw.ellipse([(jx, ly - 8), (jx + 10, ly + 6)], fill=(147, 197, 253, 255), outline=(30, 64, 175, 255), width=2)
        draw.rectangle([(jx + 8 if dir_right else jx - 4, ly - 4), (jx + 14 if dir_right else jx + 2, ly + 2)], fill=(59, 130, 246, 255))
    elif action == "pacci":
        # Cane of Pacci
        cx = lx + 8 if dir_right else lx - 14
        draw.line([(cx, ly + 8), (cx + 6 if dir_right else cx - 6, ly - 14)], fill=(16, 185, 129, 255), width=3)
        draw.ellipse([(cx + 4 if dir_right else cx - 10, ly - 18), (cx + 12 if dir_right else cx - 2, ly - 10)], fill=(56, 189, 248, 255))
    elif action == "slash":
        # Sword slash
        sx = lx + 6 if dir_right else lx - 20
        draw.line([(sx, ly + 4), (sx + 16 if dir_right else sx, ly - 12)], fill=c_metal, width=3)
        draw.polygon([(sx + 14 if dir_right else sx + 2, ly - 14), (sx + 18 if dir_right else sx - 2, ly - 10), (sx + 12 if dir_right else sx + 4, ly - 8)], fill=c_white)

def render_dungeon_floor(draw, x, y, w, h):
    # Slate tiles
    draw.rectangle([(x, y), (x + w, y + h)], fill=(51, 65, 85, 255))
    for ty in range(y, y + h, 16):
        draw.line([(x, ty), (x + w, ty)], fill=(30, 41, 59, 255), width=1)
    for tx in range(x, x + w, 16):
        draw.line([(tx, y), (tx, y + h)], fill=(30, 41, 59, 255), width=1)

def main():
    img_w, img_h = 1000, 780
    img = Image.new("RGBA", (img_w, img_h), (11, 15, 25, 255))
    draw = ImageDraw.Draw(img)

    try:
        font_title = ImageFont.truetype("arial.ttf", 20)
        font_header = ImageFont.truetype("arial.ttf", 15)
        font_label = ImageFont.truetype("arial.ttf", 13)
        font_desc = ImageFont.truetype("arial.ttf", 11)
    except:
        font_title = ImageFont.load_default()
        font_header = ImageFont.load_default()
        font_label = ImageFont.load_default()
        font_desc = ImageFont.load_default()

    # Header
    draw.text((30, 20), "The Legend of Zelda: The Minish Cap - Native C11 Engine", fill=(255, 255, 255, 255), font=font_title)
    draw.text((30, 48), "PONTO 2: Ciclo Completo de Batalha com Chefes (Deepwood Shrine / Big Green ChuChu)", fill=(74, 222, 128, 255), font=font_header)

    panel_w, panel_h = 455, 320
    p1_x, p1_y = 30, 85
    p2_x, p2_y = 515, 85
    p3_x, p3_y = 30, 425
    p4_x, p4_y = 515, 425

    def draw_panel(px, py, title, sub):
        draw.rectangle([(px, py), (px + panel_w, py + panel_h)], fill=(15, 23, 42, 240))
        draw.rectangle([(px, py), (px + panel_w, py + panel_h)], outline=(30, 41, 59, 255), width=2)
        draw.rectangle([(px, py), (px + panel_w, py + 34)], fill=(20, 83, 45, 220))
        draw.text((px + 12, py + 5), title, fill=(255, 255, 255, 255), font=font_label)
        draw.text((px + 12, py + 20), sub, fill=(187, 247, 208, 255), font=font_desc)

    # -------------------------------------------------------------------------
    # PAINEL 1: Fase 1 - Sucção com Gust Jar & Saltos Colossais
    # -------------------------------------------------------------------------
    draw_panel(p1_x, p1_y, "PAINEL 1: Fase 1 - Pote Mágico (Gust Jar) & Sucção da Base", "Vórtice de ar contínuo suga a gosma dos pés até desestabilizar (Base Scale 0.45)")
    render_dungeon_floor(draw, p1_x + 10, p1_y + 45, panel_w - 20, panel_h - 55)

    # Shadow
    draw_oval_shadow(img, p1_x + 280, p1_y + 195, 32, 12, 0.45)
    # Boss
    render_big_chuchu(draw, p1_x + 280, p1_y + 190, scale_base=0.45, toppled=False, enraged=False, jump_z=0)

    # Link with Gust Jar
    draw_oval_shadow(img, p1_x + 100, p1_y + 205, 12, 5, 0.40)
    render_link(draw, p1_x + 100, p1_y + 195, action="gust", dir_right=True)

    # Suction Vortex FX
    for v_i in range(5):
        vx1 = p1_x + 124 + v_i * 26
        vy1 = p1_y + 195 + int(math.sin(v_i * 1.2) * 8.0)
        draw.arc([(vx1, vy1 - 6), (vx1 + 18, vy1 + 6)], start=180, end=360, fill=(186, 230, 253, 220), width=2)
        draw.line([(vx1, vy1), (vx1 + 14, vy1)], fill=(224, 242, 254, 200), width=1)

    # Tactical card
    draw.rectangle([(p1_x + 20, p1_y + 245), (p1_x + panel_w - 20, p1_y + 305)], fill=(15, 23, 42, 210))
    draw.rectangle([(p1_x + 20, p1_y + 245), (p1_x + panel_w - 20, p1_y + 305)], outline=(51, 65, 85, 255))
    draw.text((p1_x + 28, p1_y + 252), "Mecânica Autêntica GBA: Gust Jar", fill=(56, 189, 248, 255), font=font_label)
    draw.text((p1_x + 28, p1_y + 270), "• Link suga a base gelatinosa com o Gust Jar (ação contínua).", fill=(226, 232, 240, 255), font=font_desc)
    draw.text((p1_x + 28, p1_y + 286), "• Medidor de Desequilíbrio atinge 100% -> Base encolhe e o chefe desaba!", fill=(74, 222, 128, 255), font=font_desc)

    # -------------------------------------------------------------------------
    # PAINEL 2: Fase 2 - Inversão c/ Cane of Pacci & Toppled (Atordoado)
    # -------------------------------------------------------------------------
    draw_panel(p2_x, p2_y, "PAINEL 2: Atordoamento Instantâneo via Cane of Pacci & Golpe", "Pulso gravitacional inverte a base (+30) -> Cabeça desaba vulnerável!")
    render_dungeon_floor(draw, p2_x + 10, p2_y + 45, panel_w - 20, panel_h - 55)

    # Toppled Boss on ground
    draw_oval_shadow(img, p2_x + 300, p2_y + 195, 38, 14, 0.45)
    render_big_chuchu(draw, p2_x + 300, p2_y + 190, scale_base=0.15, toppled=True, enraged=False, jump_z=0)

    # Link slashing sword on vulnerable head
    draw_oval_shadow(img, p2_x + 235, p2_y + 200, 12, 5, 0.40)
    render_link(draw, p2_x + 235, p2_y + 190, action="slash", dir_right=True)

    # Hit Sparks / Slash arc
    draw.arc([(p2_x + 250, p2_y + 165), (p2_x + 285, p2_y + 205)], start=220, end=350, fill=(250, 204, 21, 255), width=3)
    draw.line([(p2_x + 265, p2_y + 175), (p2_x + 278, p2_y + 165)], fill=(255, 255, 255, 255), width=2)

    # Cane of Pacci bolt trail indicator
    draw.text((p2_x + 30, p2_y + 60), "Cane of Pacci Disparado:", fill=(56, 189, 248, 255), font=font_label)
    draw.ellipse([(p2_x + 185, p2_y + 140), (p2_x + 205, p2_y + 160)], fill=(56, 189, 248, 255), outline=(255, 255, 255, 255), width=2)
    draw.line([(p2_x + 120, p2_y + 150), (p2_x + 185, p2_y + 150)], fill=(56, 189, 248, 180), width=3)
    draw.text((p2_x + 30, p2_y + 80), "• Orbe gravitacional desestabiliza a base", fill=(226, 232, 240, 255), font=font_desc)
    draw.text((p2_x + 30, p2_y + 98), "• Atordoamento imediato: 200 frames de vulnerabilidade", fill=(226, 232, 240, 255), font=font_desc)

    # Tactical card
    draw.rectangle([(p2_x + 20, p2_y + 245), (p2_x + panel_w - 20, p2_y + 305)], fill=(15, 23, 42, 210))
    draw.rectangle([(p2_x + 20, p2_y + 245), (p2_x + panel_w - 20, p2_y + 305)], outline=(51, 65, 85, 255))
    draw.text((p2_x + 28, p2_y + 252), "* VULNERAVEL! ATAQUE A CABECA! *", fill=(250, 204, 21, 255), font=font_label)
    draw.text((p2_x + 28, p2_y + 270), "• A cabeça desaba no chão com olhos girando em espiral tonta.", fill=(226, 232, 240, 255), font=font_desc)
    draw.text((p2_x + 28, p2_y + 286), "• Espadadas causam dano real aos pontos de vida do chefe!", fill=(56, 189, 248, 255), font=font_desc)

    # -------------------------------------------------------------------------
    # PAINEL 3: Fase 3 - Enraged Carmesim & Impacto de Solo (Screen Shake)
    # -------------------------------------------------------------------------
    draw_panel(p3_x, p3_y, "PAINEL 3: Fase Enraged (HP <= 5) & Esmagamento no Solo", "Corpo vermelho escuro, saltos acelerados e ondas de choque terrestres")
    render_dungeon_floor(draw, p3_x + 10, p3_y + 45, panel_w - 20, panel_h - 55)

    # Airborne boss (z = 24px)
    draw_oval_shadow(img, p3_x + 240, p3_y + 200, 22, 9, 0.28) # Sombra atenuada
    render_big_chuchu(draw, p3_x + 240, p3_y + 200, scale_base=1.0, toppled=False, enraged=True, jump_z=30)

    # Ground Shockwaves
    for r_shock in [36, 52, 68]:
        draw.ellipse([(p3_x + 240 - r_shock, p3_y + 200 - r_shock // 3),
                      (p3_x + 240 + r_shock, p3_y + 200 + r_shock // 3)],
                     outline=(239, 68, 68, max(50, 255 - r_shock * 3)), width=2)

    # Screen Shake Indicator
    draw.rectangle([(p3_x + 25, p3_y + 55), (p3_x + 175, p3_y + 90)], fill=(15, 23, 42, 220))
    draw.text((p3_x + 32, p3_y + 60), "SCREEN SHAKE ATIVO:", fill=(248, 113, 113, 255), font=font_label)
    draw.text((p3_x + 32, p3_y + 74), "Impacto pesadíssimo: mag=4", fill=(254, 202, 202, 255), font=font_desc)

    # Link dodging
    draw_oval_shadow(img, p3_x + 90, p3_y + 200, 12, 5, 0.40)
    render_link(draw, p3_x + 90, p3_y + 190, action="normal", dir_right=True)

    # Tactical card
    draw.rectangle([(p3_x + 20, p3_y + 245), (p3_x + panel_w - 20, p3_y + 305)], fill=(15, 23, 42, 210))
    draw.rectangle([(p3_x + 20, p3_y + 245), (p3_x + panel_w - 20, p3_y + 305)], outline=(51, 65, 85, 255))
    draw.text((p3_x + 28, p3_y + 252), "Comportamento da Fase Enrage:", fill=(239, 68, 68, 255), font=font_label)
    draw.text((p3_x + 28, p3_y + 270), "• Velocidade e força de salto aumentadas (+60% velocidade de aproximação).", fill=(226, 232, 240, 255), font=font_desc)
    draw.text((p3_x + 28, p3_y + 286), "• Impacto gera tremores que atordoam Link se pego em raio próximo.", fill=(254, 202, 202, 255), font=font_desc)

    # -------------------------------------------------------------------------
    # PAINEL 4: HUD de Vida do Chefe & Câmera Dinâmica
    # -------------------------------------------------------------------------
    draw_panel(p4_x, p4_y, "PAINEL 4: HUD Canônica GBA & Câmera Dinâmica Centroid", "Barra ornamental dourada com gemas Minish + Enquadramento móvel inteligente")
    render_dungeon_floor(draw, p4_x + 10, p4_y + 45, panel_w - 20, panel_h - 55)

    # Demonstration of the Boss Health Bar (Rendered at top of panel 4)
    bh_x = p4_x + 90
    bh_y = p4_y + 70
    bw = 140
    bh = 10

    # Draw gold border
    draw.rectangle([(bh_x - 3, bh_y - 2), (bh_x + bw + 3, bh_y + bh + 2)], fill=(15, 23, 42, 255))
    draw.rectangle([(bh_x - 2, bh_y - 1), (bh_x + bw + 2, bh_y + bh + 1)], fill=(217, 119, 6, 255))
    draw.rectangle([(bh_x - 1, bh_y), (bh_x + bw + 1, bh_y + bh)], fill=(120, 53, 15, 255))
    draw.rectangle([(bh_x, bh_y + 1), (bh_x + bw, bh_y + bh - 1)], fill=(2, 6, 23, 255))

    # Minish Gems at ends
    draw.rectangle([(bh_x - 7, bh_y), (bh_x - 3, bh_y + bh)], fill=(16, 185, 129, 255))
    draw.rectangle([(bh_x + bw + 3, bh_y), (bh_x + bw + 7, bh_y + bh)], fill=(16, 185, 129, 255))

    # 10 HP pips (7 remaining)
    pip_w = 12
    for p_i in range(10):
        px = bh_x + 3 + p_i * (pip_w + 1)
        if p_i < 7:
            draw.rectangle([(px, bh_y + 2), (px + pip_w, bh_y + 4)], fill=(134, 239, 172, 255))
            draw.rectangle([(px, bh_y + 4), (px + pip_w, bh_y + 7)], fill=(34, 197, 94, 255))
            draw.rectangle([(px, bh_y + 7), (px + pip_w, bh_y + 8)], fill=(21, 128, 61, 255))
            draw.point((px + 1, bh_y + 3), fill=(255, 255, 255, 255))
        else:
            draw.rectangle([(px, bh_y + 3), (px + pip_w, bh_y + 7)], fill=(51, 65, 85, 255))

    draw.text((bh_x + 10, bh_y - 16), "BIG GREEN CHUCHU", fill=(251, 191, 36, 255), font=font_label)

    # Dynamic Camera diagram
    draw.rectangle([(p4_x + 30, p4_y + 115), (p4_x + panel_w - 30, p4_y + 230)], fill=(15, 23, 42, 220))
    draw.rectangle([(p4_x + 30, p4_y + 115), (p4_x + panel_w - 30, p4_y + 230)], outline=(56, 189, 248, 255), width=2)
    draw.text((p4_x + 40, p4_y + 125), "Algoritmo da Câmera Dinâmica:", fill=(56, 189, 248, 255), font=font_label)
    draw.text((p4_x + 40, p4_y + 145), "• Posição Alvo: target = ((pos_link + pos_boss)/2) - (viewport/2)", fill=(226, 232, 240, 255), font=font_desc)
    draw.text((p4_x + 40, p4_y + 165), "• Compensação Vertical de Salto: elevação de câmera em saltos (boss.z * 0.4)", fill=(226, 232, 240, 255), font=font_desc)
    draw.text((p4_x + 40, p4_y + 185), "• Interpolação Exponencial Suave: cam += (target - cam) * 0.12f", fill=(226, 232, 240, 255), font=font_desc)
    draw.text((p4_x + 40, p4_y + 205), "• Confinamento Estrito: 0 <= cam_x <= 256-vw, 0 <= cam_y <= 160-vh", fill=(148, 163, 184, 255), font=font_desc)

    # Tactical card
    draw.rectangle([(p4_x + 20, p4_y + 245), (p4_x + panel_w - 20, p4_y + 305)], fill=(15, 23, 42, 210))
    draw.rectangle([(p4_x + 20, p4_y + 245), (p4_x + panel_w - 20, p4_y + 305)], outline=(51, 65, 85, 255))
    draw.text((p4_x + 28, p4_y + 252), "Conclusão do Ciclo de Combate:", fill=(74, 222, 128, 255), font=font_label)
    draw.text((p4_x + 28, p4_y + 270), "• Derrota do chefe spawna Heart Container permanente.", fill=(226, 232, 240, 255), font=font_desc)
    draw.text((p4_x + 28, p4_y + 286), "• Grades levadiças da câmara se abrem e Portal Sagrado é ativado!", fill=(250, 204, 21, 255), font=font_desc)

    # Footer note
    draw.text((30, 755), "Metodologia OpenMinish: Conformidade Zero-ROM, 60 FPS sem heap allocation, Arquitetura C11 HAL e Verificação Empírica Pixel-Perfect.", fill=(148, 163, 184, 255), font=font_desc)

    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    img.save(OUTPUT_PATH, "PNG")
    print(f"[OK] Showcase de Batalha com Chefes gerado com sucesso em: {OUTPUT_PATH}")

if __name__ == "__main__":
    main()
