/*
 * ============================================================================
 * src/hal/prologue_story.c - Prólogo Narrativo: A Lenda dos Picori
 * ============================================================================
 * Reproduz o prólogo cinematográfico canônico de The Legend of Zelda: The Minish Cap.
 * Exibe as 4 tapeçarias históricas com narração lírica e transições suaves.
 */

#include "hal/prologue_story.h"
#include "hal/video.h"
#include "hal/texture.h"
#include "hal/font.h"
#include "hal/audio.h"
#include "hal/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_W 240
#define SCREEN_H 160
#define TOTAL_PANELS 4
#define PANEL_DURATION 270 // ~4.5 segundos por painel a 60 FPS
#define FADE_DURATION  25  // ~0.4 segundos de transição suave

typedef struct {
    bool     active;
    int      current_panel;
    int      panel_timer;
    float    fade_alpha;
    bool     is_fading_out;
    bool     is_fading_in;
} PrologueState;

static PrologueState s_prologue = { 0 };
static Texture* s_prologue_tex = NULL;

static inline u32 blend_color(u32 dst, u32 src) {
    u32 sa = src & 0xFF;
    if (sa == 255) return src;
    if (sa == 0)   return dst;

    u32 sr = (src >> 24) & 0xFF;
    u32 sg = (src >> 16) & 0xFF;
    u32 sb = (src >> 8)  & 0xFF;

    u32 dr = (dst >> 24) & 0xFF;
    u32 dg = (dst >> 16) & 0xFF;
    u32 db = (dst >> 8)  & 0xFF;

    u32 r = (sr * sa + dr * (255 - sa)) / 255;
    u32 g = (sg * sa + dg * (255 - sa)) / 255;
    u32 b = (sb * sa + db * (255 - sa)) / 255;

    return (r << 24) | (g << 16) | (b << 8) | 0xFF;
}

static void draw_box(int x, int y, int w, int h, u32 color) {
    const HalVideoContext* ctx = hal_video_get_context();
    int rw = ctx ? ctx->render_width : SCREEN_W;
    int rh = ctx ? ctx->render_height : SCREEN_H;

    int x1 = (x < 0) ? 0 : x;
    int y1 = (y < 0) ? 0 : y;
    int x2 = (x + w > rw) ? rw : (x + w);
    int y2 = (y + h > rh) ? rh : (y + h);

    for (int py = y1; py < y2; py++) {
        for (int px = x1; px < x2; px++) {
            u32 cur = hal_video_get_pixel(px, py);
            hal_video_put_pixel(px, py, blend_color(cur, color));
        }
    }
}

void prologue_story_init(void) {
    memset(&s_prologue, 0, sizeof(PrologueState));
    s_prologue.active = false;

    if (!s_prologue_tex) {
        s_prologue_tex = texture_load_bmp("assets/ui/prologue/prologue_panels.bmp");
        if (!s_prologue_tex) {
            s_prologue_tex = texture_load_bmp("assets/regions/prologue_panels_master.bmp");
        }
        if (!s_prologue_tex) {
            s_prologue_tex = texture_load_bmp("assets/regions/usa/prologue_panels.bmp");
        }
    }
    if (s_prologue_tex) {
        printf("[PROLOGUE] Painéis do prólogo da Lenda dos Picori carregados (%dx%d).\n",
               s_prologue_tex->width, s_prologue_tex->height);
    } else {
        printf("[PROLOGUE] Aviso: Painéis não encontrados. Ativando fallback procedural.\n");
    }
}

void prologue_story_start(void) {
    s_prologue.active = true;
    s_prologue.current_panel = 0;
    s_prologue.panel_timer = 0;
    s_prologue.fade_alpha = 1.0f;
    s_prologue.is_fading_in = true;
    s_prologue.is_fading_out = false;

    hal_audio_play_bgm(BGM_ELEMENTAL_SANCTUARY);
    printf("[PROLOGUE] Prólogo da história iniciado: A Lenda dos Picori (The Minish Cap).\n");
}

bool prologue_story_is_active(void) {
    return s_prologue.active;
}

void prologue_story_skip(void) {
    s_prologue.active = false;
    printf("[PROLOGUE] Prólogo avançado pelo jogador para a Tela de Título.\n");
}

void prologue_story_update(void) {
    if (!s_prologue.active) return;

    // Botão START ou B para pular direto para o menu de título
    if (hal_input_is_pressed(KEY_START) || hal_input_is_pressed(KEY_B)) {
        prologue_story_skip();
        return;
    }

    // Botão A avança para o próximo painel
    if (hal_input_is_pressed(KEY_A) && !s_prologue.is_fading_out) {
        s_prologue.is_fading_out = true;
        s_prologue.is_fading_in = false;
    }

    // Gerenciamento de Fade
    if (s_prologue.is_fading_in) {
        s_prologue.fade_alpha -= 1.0f / (float)FADE_DURATION;
        if (s_prologue.fade_alpha <= 0.0f) {
            s_prologue.fade_alpha = 0.0f;
            s_prologue.is_fading_in = false;
        }
    } else if (s_prologue.is_fading_out) {
        s_prologue.fade_alpha += 1.0f / (float)FADE_DURATION;
        if (s_prologue.fade_alpha >= 1.0f) {
            s_prologue.fade_alpha = 1.0f;
            s_prologue.is_fading_out = false;
            s_prologue.current_panel++;
            s_prologue.panel_timer = 0;
            if (s_prologue.current_panel >= TOTAL_PANELS) {
                // Fim da introdução histórica -> transição para o título
                s_prologue.active = false;
                return;
            } else {
                s_prologue.is_fading_in = true;
            }
        }
    } else {
        // Exibição regular do painel ativo
        s_prologue.panel_timer++;
        if (s_prologue.panel_timer >= PANEL_DURATION - FADE_DURATION) {
            s_prologue.is_fading_out = true;
        }
    }
}

void prologue_story_render(void) {
    if (!s_prologue.active) return;

    const HalVideoContext* ctx = hal_video_get_context();
    int W = ctx ? ctx->render_width : SCREEN_W;
    int H = ctx ? ctx->render_height : SCREEN_H;

    // Fundo limpo
    draw_box(0, 0, W, H, 0x0A0807FF);

    int panel_w = 240;
    int panel_h = 160;
    int dest_x = (W - panel_w) / 2;
    int dest_y = (H - panel_h) / 2;

    int col = s_prologue.current_panel % 2;
    int row = s_prologue.current_panel / 2;
    int src_x = col * 240;
    int src_y = row * 160;

    // 1. Renderiza o painel ilustrado
    if (s_prologue_tex && s_prologue_tex->pixels) {
        texture_draw(s_prologue_tex, src_x, src_y, panel_w, panel_h, dest_x, dest_y);
    } else {
        // Fallback procedural se textura não existir
        draw_box(dest_x, dest_y, panel_w, panel_h, 0x2A2218FF);
        draw_box(dest_x + 4, dest_y + 4, panel_w - 8, panel_h - 8, 0xD4AF3788);
    }

    // 2. Caixa de Texto Narrativa estilo Pergaminho Imperial
    int box_w = W - 20;
    int box_h = 36;
    int box_x = 10;
    int box_y = H - 42;

    // Moldura ornamentada dourada e interior translúcido
    draw_box(box_x - 1, box_y - 1, box_w + 2, box_h + 2, 0xD4AF37FF);
    draw_box(box_x, box_y, box_w, box_h, 0x140E0AEE);
    draw_box(box_x + 2, box_y + 2, box_w - 4, box_h - 4, 0x2C1D1188);

    const char* title = "";
    const char* text  = "";

    switch (s_prologue.current_panel) {
        case 0:
            title = "A LENDA DOS PICORI: AS TREVAS";
            text  = "Ha muito tempo atras, o mundo esteve a ponto de sucumbir em trevas...";
            break;
        case 1:
            title = "A DESCIDA DOS PICORI";
            text  = "Do ceu desceram os pequeninos Picori, trazendo a Luz Dourada e uma espada sagrada.";
            break;
        case 2:
            title = "O HEROI E O BAU SAGRADO";
            text  = "O bravo heroi baniu os monstros e os selou para sempre no Bau Sagrado!";
            break;
        case 3:
            title = "O FESTIVAL SECULAR DE HYRULE";
            text  = "A paz renasceu, e a cada cem anos Hyrule celebra o sagrado Festival de Picori.";
            break;
        default:
            break;
    }

    font_draw_text(box_x + 8, box_y + 5, title, 0xFDE047FF, false);
    font_draw_text(box_x + 8, box_y + 19, text, 0xF8FAFCFF, false);

    // Indicador sutil de avanço [A] / [START]
    font_draw_text(W - 74, 4, "[START] Pular", 0x94A3B8FF, false);

    // 3. Efeito de Fade suave na transição
    if (s_prologue.fade_alpha > 0.0f) {
        u8 a = (u8)(s_prologue.fade_alpha * 255.0f);
        draw_box(0, 0, W, H, (0x00000000 | a));
    }
}

void prologue_story_shutdown(void) {
    if (s_prologue_tex) {
        texture_free(s_prologue_tex);
        s_prologue_tex = NULL;
    }
}
