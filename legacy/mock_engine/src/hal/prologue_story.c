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
#define TOTAL_ACTS 7
#define FADE_DURATION 24     // ~0.4s de transição suave a 60 FPS
#define AUTO_READ_DELAY 180  // ~3.0s de leitura após texto completo antes de avançar

typedef struct {
    int         panel_index;  // 0..6: índice do vitral na folha máster de 8 vitrais
    bool        is_side_layout; // true: vitral na esquerda, texto na direita
    const char* text;
    SoundEffect trigger_sfx;
    float       sfx_pitch;
    bool        is_dark_climax;
} PrologueAct;

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
} PrologueState;

static PrologueState s_prologue = { 0 };
static Texture* s_prologue_tex = NULL;

static const PrologueAct s_acts[TOTAL_ACTS] = {
    // Ato 0: O Bosque Sagrado / Abertura da Lenda
    {
        .panel_index = 0,
        .is_side_layout = false,
        .text = "A long, long time ago...",
        .trigger_sfx = SOUND_SECRET,
        .sfx_pitch = 0.85f,
        .is_dark_climax = false
    },
    // Ato 1: As Trevas e o Ataque dos Monstros
    {
        .panel_index = 1,
        .is_side_layout = false,
        .text = "when the world was on the verge of\nbeing swallowed by shadow...",
        .trigger_sfx = SOUND_KINSTONE_FUSION,
        .sfx_pitch = 0.90f,
        .is_dark_climax = false
    },
    // Ato 2: A Descida dos Picori com a Espada e a Luz
    {
        .panel_index = 2,
        .is_side_layout = true,
        .text = "The tiny Picori\nappeared from the\nsky, bringing the\nhero of men a sword\nand a golden light.",
        .trigger_sfx = SOUND_SECRET,
        .sfx_pitch = 1.0f,
        .is_dark_climax = false
    },
    // Ato 3: O Herói Baniu as Trevas com Sabedoria e Coragem
    {
        .panel_index = 3,
        .is_side_layout = true,
        .text = "With wisdom and\ncourage, the hero\ndrove out the\ndarkness.",
        .trigger_sfx = SOUND_SWORD_SLASH,
        .sfx_pitch = 0.95f,
        .is_dark_climax = false
    },
    // Ato 4: A Paz Retornou e a Lâmina foi Selada no Baú
    {
        .panel_index = 4,
        .is_side_layout = false,
        .text = "When peace had been restored, the\npeople enshrined that blade with care.",
        .trigger_sfx = SOUND_SWITCH_CLICK,
        .sfx_pitch = 1.0f,
        .is_dark_climax = false
    },
    // Ato 5: A Força da Luz Dourada na Princesa de Hyrule
    {
        .panel_index = 5,
        .is_side_layout = false,
        .text = "And the force of the golden light,\nembodied in Hyrule's princess,\nshone forth upon the lands.",
        .trigger_sfx = SOUND_SECRET,
        .sfx_pitch = 1.15f,
        .is_dark_climax = false
    },
    // Ato 6: O Clímax Sombrio - O Mago Vaati Revelado
    {
        .panel_index = 6,
        .is_side_layout = false,
        .text = "Heh heh heh...\nSo that's what it means...",
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

static void draw_prologue_text(const PrologueAct* act, int visible_chars, int W, int H) {
    (void)H;
    if (!act || !act->text || visible_chars <= 0) return;

    // Buffer de cópia para separação por linhas sem alterar o original
    char text_copy[256];
    strncpy(text_copy, act->text, sizeof(text_copy) - 1);
    text_copy[sizeof(text_copy) - 1] = '\0';

    char* lines[8];
    int line_count = 0;
    char* cur = text_copy;
    lines[line_count++] = cur;

    while (*cur && line_count < 8) {
        if (*cur == '\n') {
            *cur = '\0';
            lines[line_count++] = cur + 1;
        }
        cur++;
    }

    int remaining_chars = visible_chars;

    if (act->is_side_layout) {
        // Layout lateral direito (Atos 2 e 3): Vitral na esquerda (0..119), texto à direita (x=126)
        int start_y = (line_count <= 4) ? 42 : 36;
        for (int i = 0; i < line_count && remaining_chars > 0; i++) {
            int line_len = (int)strlen(lines[i]);
            int draw_len = (remaining_chars < line_len) ? remaining_chars : line_len;

            char line_buf[64];
            strncpy(line_buf, lines[i], draw_len);
            line_buf[draw_len] = '\0';

            font_draw_text(126, start_y + i * 15, line_buf, 0xFFFFFFFF, true);
            remaining_chars -= (line_len + 1);
        }
    } else {
        // Layout inferior centralizado (Atos 0, 1, 4, 5, 6): Vitral no topo, texto abaixo
        int base_y = 120;
        int line_spacing = 14;
        if (line_count == 1) {
            base_y = 126;
        } else if (line_count == 3) {
            base_y = 116;
            line_spacing = 12;
        }

        for (int i = 0; i < line_count && remaining_chars > 0; i++) {
            int line_len = (int)strlen(lines[i]);
            int draw_len = (remaining_chars < line_len) ? remaining_chars : line_len;

            char line_buf[64];
            strncpy(line_buf, lines[i], draw_len);
            line_buf[draw_len] = '\0';

            int full_w = font_get_text_width(lines[i]);
            int line_x = (W - full_w) / 2;

            u32 col = act->is_dark_climax ? 0xE9D5FFFF : 0xFFFFFFFF;
            font_draw_text(line_x, base_y + i * line_spacing, line_buf, col, true);
            remaining_chars -= (line_len + 1);
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

    // 1. Fundo preto puro canônico (Zero caixas de pergaminho ou molduras douradas)
    draw_box(0, 0, W, H, 0x000000FF);

    int panel_w = 240;
    int panel_h = 160;
    int dest_x = (W - panel_w) / 2;
    int dest_y = (H - panel_h) / 2;

    const PrologueAct* act = &s_acts[s_prologue.current_act];

    int col = act->panel_index % 2;
    int row = act->panel_index / 2;
    int src_x = col * 240;
    int src_y = row * 160;

    // 2. Renderiza a tapeçaria de vitral histórico autêntico da ROM
    if (s_prologue_tex && s_prologue_tex->pixels) {
        texture_draw(s_prologue_tex, src_x, src_y, panel_w, panel_h, dest_x, dest_y);
    }

    // Clímax de Vaati (Ato 6): Vinheta sombria violeta sobre o vitral
    if (act->is_dark_climax) {
        draw_box(dest_x, dest_y, panel_w, panel_h, 0x3B07644D);
    }

    // Scrim sutil para máxima legibilidade nos atos em que o vitral ocupa a área inferior
    if (act->panel_index == 5) {
        draw_box(0, 112, W, 46, 0x000000CC);
    } else if (act->panel_index == 6) {
        draw_box(0, 114, W, 44, 0x000000D4);
    }

    // 3. Renderiza o texto autêntico canônico em branco com drop shadow preta de 1px
    draw_prologue_text(act, s_prologue.visible_chars, W, H);

    // 4. Indicador [▼] piscante quando a digitação termina
    if (s_prologue.visible_chars >= s_prologue.total_chars) {
        if ((s_prologue.act_timer / 15) % 2 == 0) {
            if (act->is_side_layout) {
                font_draw_text(224, 142, "\x03", 0xFDE047FF, false);
            } else {
                font_draw_text(dest_x + panel_w - 18, 142, "\x03", 0xFDE047FF, false);
            }
        }
    }

    // 5. Efeito de Fade suave na transição
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
