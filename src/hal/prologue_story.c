/*
 * ============================================================================
 * src/hal/prologue_story.c - Prólogo Histórico da Lenda dos Picori (Storybook)
 * ============================================================================
 * Reproduz o prólogo cinematográfico canônico de The Legend of Zelda: The Minish Cap.
 * Exibe as tapeçarias em vitrais históricos góticos com narração lírica, efeito de
 * digitação typewriter, partículas de luz (motes), feixes solares e transições suaves.
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
#include <math.h>

#define SCREEN_W 240
#define SCREEN_H 160
#define TOTAL_ACTS 6
#define FADE_DURATION 24     // ~0.4s de transição suave a 60 FPS
#define AUTO_READ_DELAY 180  // ~3.0s de leitura após texto completo antes de avançar
#define MAX_MOTES 24

typedef struct {
    int         panel_index;  // 0: Trevas, 1: Descida, 2: Herói & Baú, 3: Festival
    const char* title;
    const char* text;
    SoundEffect trigger_sfx;
    float       sfx_pitch;
    bool        is_dark_climax;
} PrologueAct;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float phase;
    float size;
} LightMote;

typedef struct {
    bool        active;
    int         current_act;
    int         visible_chars;
    int         char_timer;
    int         total_chars;
    int         read_timer;
    float       fade_alpha;
    bool        is_fading_in;
    bool        is_fading_out;
    int         act_timer;
    LightMote   motes[MAX_MOTES];
} PrologueState;

static PrologueState s_prologue = { 0 };
static Texture* s_prologue_tex = NULL;

static const PrologueAct s_acts[TOTAL_ACTS] = {
    // Ato 0: As Trevas sobre Hyrule
    {
        .panel_index = 0,
        .title = "A LENDA DOS PICORI: AS TREVAS",
        .text = "Ha muito tempo, o mundo\nquase sucumbiu nas trevas.",
        .trigger_sfx = SOUND_SECRET,
        .sfx_pitch = 0.85f,
        .is_dark_climax = false
    },
    // Ato 1: A Descida dos Picori
    {
        .panel_index = 1,
        .title = "A DESCIDA DOS PICORI",
        .text = "Do ceu desceram os Picori,\ncom espada e Luz Dourada.",
        .trigger_sfx = SOUND_KINSTONE_FUSION,
        .sfx_pitch = 1.0f,
        .is_dark_climax = false
    },
    // Ato 2: O Herói e a Espada Sagrada
    {
        .panel_index = 2,
        .title = "O HEROI E A ESPADA SAGRADA",
        .text = "Com coragem e sabedoria,\no heroi baniu as trevas.",
        .trigger_sfx = SOUND_SWORD_SLASH,
        .sfx_pitch = 0.95f,
        .is_dark_climax = false
    },
    // Ato 3: O Selamento do Baú Sagrado
    {
        .panel_index = 2,
        .title = "O SELAMENTO DO BAU SAGRADO",
        .text = "A paz voltou e o heroi selou\no mal no sagrado Bau!",
        .trigger_sfx = SOUND_SWITCH_CLICK,
        .sfx_pitch = 1.0f,
        .is_dark_climax = false
    },
    // Ato 4: O Festival Secular e a Luz Dourada
    {
        .panel_index = 3,
        .title = "O FESTIVAL SECULAR DE HYRULE",
        .text = "A Luz brilha na Princesa e\nHyrule celebra o festival.",
        .trigger_sfx = SOUND_SECRET,
        .sfx_pitch = 1.15f,
        .is_dark_climax = false
    },
    // Ato 5: O Clímax Sombrio - O Mago Vaati nas Sombras
    {
        .panel_index = 3,
        .title = "UMA SOMBRA ESCOLHE SEU MOMENTO...",
        .text = "Heh heh heh...\nEntao o segredo e esse...",
        .trigger_sfx = SOUND_TEXT_ADVANCE,
        .sfx_pitch = 0.65f,
        .is_dark_climax = true
    }
};

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

static int count_chars_utf8(const char* text) {
    if (!text) return 0;
    int count = 0;
    int len = (int)strlen(text);
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)text[i];
        if (c == 0xC3 && i + 1 < len) {
            count++;
            i++;
        } else {
            count++;
        }
    }
    return count;
}

static void copy_chars_utf8(char* dest, int max_dest, const char* src, int count) {
    if (!dest || max_dest <= 0) return;
    dest[0] = '\0';
    if (!src || count <= 0) return;

    int cur_chars = 0;
    int src_len = (int)strlen(src);
    int dst_pos = 0;

    for (int i = 0; i < src_len && cur_chars < count && dst_pos < max_dest - 2; i++) {
        unsigned char c = (unsigned char)src[i];
        if (c == 0xC3 && i + 1 < src_len) {
            dest[dst_pos++] = src[i];
            dest[dst_pos++] = src[i + 1];
            i++;
            cur_chars++;
        } else {
            dest[dst_pos++] = src[i];
            cur_chars++;
        }
    }
    dest[dst_pos] = '\0';
}

static void init_motes(void) {
    for (int i = 0; i < MAX_MOTES; i++) {
        s_prologue.motes[i].x = (float)(rand() % 220 + 10);
        s_prologue.motes[i].y = (float)(rand() % 120 + 15);
        s_prologue.motes[i].vx = ((float)(rand() % 40) - 20.0f) / 100.0f;
        s_prologue.motes[i].vy = -((float)(rand() % 35 + 15) / 100.0f);
        s_prologue.motes[i].phase = ((float)(rand() % 628)) / 100.0f;
        s_prologue.motes[i].size = (rand() % 2 == 0) ? 2.0f : 1.0f;
    }
}

static void update_motes(void) {
    for (int i = 0; i < MAX_MOTES; i++) {
        LightMote* m = &s_prologue.motes[i];
        m->y += m->vy;
        m->phase += 0.04f;
        m->x += m->vx + sinf(m->phase) * 0.22f;

        if (m->y < 12.0f) {
            m->y = 135.0f;
            m->x = (float)(rand() % 220 + 10);
        } else if (m->y > 140.0f) {
            m->y = 14.0f;
            m->x = (float)(rand() % 220 + 10);
        }
        if (m->x < 10.0f) m->x = 230.0f;
        if (m->x > 230.0f) m->x = 10.0f;
    }
}

static void render_motes(int dest_x, int dest_y, int panel_w, int panel_h, bool is_dark) {
    for (int i = 0; i < MAX_MOTES; i++) {
        LightMote* m = &s_prologue.motes[i];
        int px = dest_x + (int)m->x;
        int py = dest_y + (int)m->y;

        if (px >= dest_x + 8 && px < dest_x + panel_w - 8 &&
            py >= dest_y + 8 && py < dest_y + panel_h - 45) {
            float pulse = 0.5f + 0.5f * sinf(m->phase * 2.0f);
            u8 alpha = (u8)(pulse * 170.0f + 65.0f);
            u32 color = is_dark ? ((0xC0 << 24) | (0x84 << 16) | (0xFC << 8) | alpha)   // Violeta malícia
                                : ((0xFF << 24) | (0xEA << 16) | (0x70 << 8) | alpha);  // Dourado celestial
            draw_box(px, py, (int)m->size, (int)m->size, color);
            if (m->size > 1.5f) {
                draw_box(px, py, 1, 1, 0xFFFFFFFF);
            }
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

    init_motes();

    if (s_prologue_tex) {
        printf("[PROLOGUE] Storybook Cinematic inicializado com sucesso (%dx%d).\n",
               s_prologue_tex->width, s_prologue_tex->height);
    } else {
        printf("[PROLOGUE] Aviso: Painéis não encontrados. Ativando fallback procedural.\n");
    }
}

void prologue_story_start(void) {
    s_prologue.active = true;
    s_prologue.current_act = 0;
    s_prologue.visible_chars = 0;
    s_prologue.char_timer = 0;
    s_prologue.read_timer = 0;
    s_prologue.act_timer = 0;
    s_prologue.fade_alpha = 1.0f;
    s_prologue.is_fading_in = true;
    s_prologue.is_fading_out = false;
    s_prologue.total_chars = count_chars_utf8(s_acts[0].text);

    init_motes();
    hal_audio_play_bgm(BGM_ELEMENTAL_SANCTUARY);
    hal_audio_play_sound(s_acts[0].trigger_sfx, 0.70f, s_acts[0].sfx_pitch);

    printf("[PROLOGUE] Storybook Cinematic iniciado: A Lenda dos Picori (The Minish Cap).\n");
}

bool prologue_story_is_active(void) {
    return s_prologue.active;
}

void prologue_story_skip(void) {
    s_prologue.active = false;
    hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.75f, 1.0f);
    printf("[PROLOGUE] Prólogo avançado pelo jogador para a Tela de Título.\n");
}

static void advance_to_next_act(void) {
    s_prologue.current_act++;
    if (s_prologue.current_act >= TOTAL_ACTS) {
        // Fim da lenda -> transição para o título
        s_prologue.active = false;
        return;
    }

    s_prologue.visible_chars = 0;
    s_prologue.char_timer = 0;
    s_prologue.read_timer = 0;
    s_prologue.act_timer = 0;
    s_prologue.total_chars = count_chars_utf8(s_acts[s_prologue.current_act].text);
    s_prologue.is_fading_in = true;
    s_prologue.is_fading_out = false;
    s_prologue.fade_alpha = 0.8f;

    // Dispara o SFX característico do ato
    hal_audio_play_sound(s_acts[s_prologue.current_act].trigger_sfx, 0.75f,
                         s_acts[s_prologue.current_act].sfx_pitch);
}

void prologue_story_update(void) {
    if (!s_prologue.active) return;

    s_prologue.act_timer++;
    update_motes();

    // Botão START ou B para pular direto para a tela de título
    if (hal_input_is_pressed(KEY_START) || hal_input_is_pressed(KEY_B)) {
        prologue_story_skip();
        return;
    }

    // Botão A para acelerar digitação ou avançar
    if (hal_input_is_pressed(KEY_A) && !s_prologue.is_fading_out) {
        if (s_prologue.visible_chars < s_prologue.total_chars) {
            // Completa o texto imediatamente
            s_prologue.visible_chars = s_prologue.total_chars;
            hal_audio_play_sound(SOUND_TEXT_BLIP, 0.40f, 1.20f);
        } else {
            // Avança para o próximo ato
            s_prologue.is_fading_out = true;
            s_prologue.is_fading_in = false;
            hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.80f, 1.0f);
        }
    }

    // Gerenciamento de Fades
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
            advance_to_next_act();
            return;
        }
    } else {
        // Digitação typewriter do texto (1 caractere a cada 2 frames a 60 FPS)
        if (s_prologue.visible_chars < s_prologue.total_chars) {
            s_prologue.char_timer++;
            if (s_prologue.char_timer >= 2) {
                s_prologue.char_timer = 0;
                s_prologue.visible_chars++;
                if (s_prologue.visible_chars % 3 == 0) {
                    hal_audio_play_sound(SOUND_TEXT_BLIP, 0.25f, 1.30f);
                }
            }
        } else {
            // Texto completo: aguarda o tempo de leitura antes de avançar automaticamente
            s_prologue.read_timer++;
            if (s_prologue.read_timer >= AUTO_READ_DELAY) {
                s_prologue.is_fading_out = true;
            }
        }
    }
}

void prologue_story_render(void) {
    if (!s_prologue.active) return;

    const HalVideoContext* ctx = hal_video_get_context();
    int W = ctx ? ctx->render_width : SCREEN_W;
    int H = ctx ? ctx->render_height : SCREEN_H;

    // Fundo preto puro / cantaria de catedral
    draw_box(0, 0, W, H, 0x0A0807FF);

    int panel_w = 240;
    int panel_h = 160;
    int dest_x = (W - panel_w) / 2;
    int dest_y = (H - panel_h) / 2;

    const PrologueAct* act = &s_acts[s_prologue.current_act];

    int col = act->panel_index % 2;
    int row = act->panel_index / 2;
    int src_x = col * 240;
    int src_y = row * 160;

    // Micro-pan cinematográfico (deslocamento sutil de 2 pixels)
    int pan_y = (s_prologue.act_timer / 120) % 2;

    // 1. Renderiza a tapeçaria de vitral histórico
    if (s_prologue_tex && s_prologue_tex->pixels) {
        texture_draw(s_prologue_tex, src_x, src_y, panel_w, panel_h, dest_x, dest_y + pan_y);
    } else {
        draw_box(dest_x, dest_y, panel_w, panel_h, 0x2A2218FF);
        draw_box(dest_x + 4, dest_y + 4, panel_w - 8, panel_h - 8, 0xD4AF3788);
    }

    // Clímax de Vaati (Ato 5): Vinheta sombria violeta sobre o vitral
    if (act->is_dark_climax) {
        draw_box(dest_x, dest_y, panel_w, panel_h, 0x3B07644D); // Sombra profunda de malícia
    }

    // 2. Partículas douradas em suspensão (Motes of Light)
    render_motes(dest_x, dest_y, panel_w, panel_h, act->is_dark_climax);

    // 3. Caixa de Texto Narrativa estilo Pergaminho Imperial
    int box_w = W - 20;
    int box_h = 40;
    int box_x = 10;
    int box_y = H - 45;

    // Moldura ornamentada dourada e interior translúcido
    draw_box(box_x - 1, box_y - 1, box_w + 2, box_h + 2, 0xD4AF37FF);
    draw_box(box_x, box_y, box_w, box_h, 0x140E0AEE);
    draw_box(box_x + 2, box_y + 2, box_w - 4, box_h - 4, 0x2C1D1188);

    // Título em ouro luminoso
    u32 title_color = act->is_dark_climax ? 0xC084FCFF : 0xFDE047FF;
    font_draw_text(box_x + 8, box_y + 4, act->title, title_color, false);

    // Texto typewriter com quebra de linha multilinhas
    char display_buf[128] = { 0 };
    copy_chars_utf8(display_buf, sizeof(display_buf), act->text, s_prologue.visible_chars);
    font_draw_text_multiline(box_x + 8, box_y + 16, box_w - 16, 11, display_buf, 0xF8FAFCFF, false);

    // Indicador [▼] piscante quando a digitação termina
    if (s_prologue.visible_chars >= s_prologue.total_chars) {
        if ((s_prologue.act_timer / 15) % 2 == 0) {
            font_draw_text(box_x + box_w - 14, box_y + box_h - 12, "\x03", 0xFDE047FF, false);
        }
    }

    // Indicador sutil de pular com START em cápsula translúcida com borda dourada
    int badge_w = 98;
    int badge_h = 14;
    int badge_x = W - 114;
    int badge_y = 10;
    draw_box(badge_x - 1, badge_y - 1, badge_w + 2, badge_h + 2, 0xD4AF3744); // Borda dourada sutil
    draw_box(badge_x, badge_y, badge_w, badge_h, 0x0F172ACC);                  // Fundo escuro translúcido
    font_draw_text(badge_x + 4, badge_y + 3, "[START] Pular", 0xFDE047FF, false);

    // 4. Efeito de Fade suave na transição
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
