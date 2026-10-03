/*
 * ============================================================================
 * src/hal/dialogue.c - Motor de Diálogos, Retratos e Companheiro Ezlo
 * ============================================================================
 */

#include "hal/dialogue.h"
#include "hal/font.h"
#include "hal/video.h"
#include "hal/audio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static DialogueState   s_state = DIALOGUE_STATE_CLOSED;
static DialogueSpeaker s_speaker = SPEAKER_EZLO;
static char            s_speaker_name[32] = { 0 };

static char s_pages[MAX_DIALOGUE_PAGES][MAX_PAGE_LENGTH];
static int  s_page_count = 0;
static int  s_current_page = 0;

static int  s_char_progress = 0;
static int  s_type_timer = 0;
static int  s_anim_counter = 0;
static int  s_current_hint_idx = 0;

// Alpha Blending para o fundo esmeralda do balão
static inline u32 blend_colors(u32 dst, u32 src) {
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

static void draw_rect_blend(int rx, int ry, int rw, int rh, u32 color) {
    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer) return;

    for (int y = ry; y < ry + rh; y++) {
        if (y < 0 || y >= ctx->render_height) continue;
        for (int x = rx; x < rx + rw; x++) {
            if (x < 0 || x >= ctx->render_width) continue;
            int idx = y * ctx->render_width + x;
            ctx->framebuffer[idx] = blend_colors(ctx->framebuffer[idx], color);
        }
    }
}

// ----------------------------------------------------------------------------
// RENDERIZAÇÃO DOS RETRATOS DE PERSONAGENS (PORTRAITS 32x32)
// ----------------------------------------------------------------------------

static void render_portrait_ezlo(int px, int py, bool is_talking) {
    // Fundo da moldura do retrato
    draw_rect_blend(px, py, 32, 32, 0x0A2012FF);

    // Cores de Ezlo (O Gorro Pássaro Verde)
    u32 c_body      = 0x2BAE47FF; // Verde penas do corpo
    u32 c_body_dark = 0x1A6A2AFF; // Sombra verde
    u32 c_crest     = 0xE63946FF; // Crista vermelha de penas
    u32 c_crest_dk  = 0x9B1B25FF; // Sombra da crista
    u32 c_beak      = 0xF5C518FF; // Bico amarelo
    u32 c_beak_dk   = 0xC49000FF; // Sombra do bico
    u32 c_white     = 0xFFFFFFFF; // Olhos
    u32 c_black     = 0x111111FF; // Pupilas

    // Animação de fala: bico e crista movem levemente
    int talk_offset = (is_talking && ((s_anim_counter / 5) % 2 == 1)) ? 1 : 0;

    // Crista vermelha (penas no topo do gorro)
    for (int y = 2; y <= 8; y++) {
        for (int x = 8; x <= 18; x++) {
            if ((x - 13) * (x - 13) + (y - 5) * (y - 5) <= 18) {
                hal_video_put_pixel(px + x, py + y - talk_offset, c_crest);
            }
        }
    }
    // Destaque na ponta da crista
    hal_video_put_pixel(px + 7, py + 4 - talk_offset, c_crest_dk);
    hal_video_put_pixel(px + 8, py + 3 - talk_offset, c_crest_dk);

    // Corpo do chapéu/gorro verde (cabeça do Ezlo)
    for (int y = 8; y <= 27; y++) {
        for (int x = 5; x <= 27; x++) {
            float dx = (float)(x - 16) / 10.0f;
            float dy = (float)(y - 18) / 9.0f;
            if (dx * dx + dy * dy <= 1.0f) {
                u32 col = (x > 18 || y > 23) ? c_body_dark : c_body;
                hal_video_put_pixel(px + x, py + y, col);
            }
        }
    }

    // Grandes olhos expressivos de pássaro
    bool blinking = (!is_talking && (s_anim_counter % 120 > 112));

    if (blinking) {
        // Pálpebras fechadas
        draw_rect_blend(px + 10, py + 14, 5, 2, c_body_dark);
        draw_rect_blend(px + 19, py + 14, 5, 2, c_body_dark);
    } else {
        // Esclera branca
        for (int y = 11; y <= 16; y++) {
            for (int x = 9; x <= 14; x++) hal_video_put_pixel(px + x, py + y, c_white);
            for (int x = 18; x <= 23; x++) hal_video_put_pixel(px + x, py + y, c_white);
        }
        // Pupilas pretas olhando com curiosidade
        draw_rect_blend(px + 12, py + 13, 2, 3, c_black);
        draw_rect_blend(px + 20, py + 13, 2, 3, c_black);
        // Brilho nos olhos
        hal_video_put_pixel(px + 11, py + 12, c_white);
        hal_video_put_pixel(px + 19, py + 12, c_white);
    }

    // Grande bico curvo de madeira/pássaro
    int beak_y = 17 + talk_offset;
    for (int y = 0; y <= 8; y++) {
        int w = 12 - (y * y) / 7;
        if (w < 2) w = 2;
        for (int x = 16 - w / 2; x <= 16 + w / 2; x++) {
            u32 col = (y > 4) ? c_beak_dk : c_beak;
            hal_video_put_pixel(px + x, py + beak_y + y, col);
        }
    }
    // Linha do bico
    draw_rect_blend(px + 12, py + beak_y + 4, 9, 1, 0x5C3800FF);
}

static void render_portrait_minish(int px, int py) {
    // Fundo da moldura
    draw_rect_blend(px, py, 32, 32, 0x122416FF);

    u32 c_hat   = 0xC93B2BFF; // Gorro vermelho de bolota
    u32 c_skin  = 0xFDE8CDFF; // Pele clara Minish
    u32 c_hair  = 0xFFF2DEFF; // Cabelos felpudos
    u32 c_blush = 0xFF9999FF; // Bochechas rosadas
    u32 c_eyes  = 0x221105FF; // Olhos
    u32 c_tunic = 0x2A6F97FF; // Túnica azul Minish

    // Gorro pontudo Minish
    for (int y = 3; y <= 16; y++) {
        int w = 3 + (y - 3);
        if (w > 14) w = 14;
        for (int x = 16 - w / 2; x <= 16 + w / 2; x++) {
            hal_video_put_pixel(px + x, py + y, c_hat);
        }
    }
    // Pom-pom branco na ponta do gorro
    draw_rect_blend(px + 14, py + 2, 4, 3, 0xFFFFFFFF);

    // Orelhas pontudas Minish
    hal_video_put_pixel(px + 6, py + 16, c_skin);
    hal_video_put_pixel(px + 7, py + 17, c_skin);
    hal_video_put_pixel(px + 25, py + 16, c_skin);
    hal_video_put_pixel(px + 24, py + 17, c_skin);

    // Rosto redondo Minish
    for (int y = 14; y <= 24; y++) {
        for (int x = 9; x <= 23; x++) {
            hal_video_put_pixel(px + x, py + y, c_skin);
        }
    }

    // Cabelo branco/creme ao redor do gorro
    draw_rect_blend(px + 8, py + 14, 3, 4, c_hair);
    draw_rect_blend(px + 21, py + 14, 3, 4, c_hair);

    // Olhos pretos pequenos
    hal_video_put_pixel(px + 12, py + 18, c_eyes);
    hal_video_put_pixel(px + 20, py + 18, c_eyes);

    // Bochechas rosadas fofas
    hal_video_put_pixel(px + 10, py + 20, c_blush);
    hal_video_put_pixel(px + 22, py + 20, c_blush);

    // Sorriso meigo
    hal_video_put_pixel(px + 15, py + 21, 0x8A4520FF);
    hal_video_put_pixel(px + 16, py + 22, 0x8A4520FF);
    hal_video_put_pixel(px + 17, py + 21, 0x8A4520FF);

    // Túnica azul no peito
    draw_rect_blend(px + 10, py + 25, 13, 6, c_tunic);
}

// ----------------------------------------------------------------------------
// INTERFACE PÚBLICA DO SISTEMA DE DIÁLOGO
// ----------------------------------------------------------------------------

void dialogue_init(void) {
    s_state = DIALOGUE_STATE_CLOSED;
    s_speaker = SPEAKER_EZLO;
    s_speaker_name[0] = '\0';
    s_page_count = 0;
    s_current_page = 0;
    s_char_progress = 0;
    s_type_timer = 0;
    s_anim_counter = 0;
    s_current_hint_idx = 0;
}

void dialogue_show(DialogueSpeaker speaker, const char* name, const char* const* pages, int page_count) {
    if (!pages || page_count <= 0) return;

    s_speaker = speaker;
    if (name) {
        strncpy(s_speaker_name, name, sizeof(s_speaker_name) - 1);
        s_speaker_name[sizeof(s_speaker_name) - 1] = '\0';
    } else {
        s_speaker_name[0] = '\0';
    }

    s_page_count = (page_count > MAX_DIALOGUE_PAGES) ? MAX_DIALOGUE_PAGES : page_count;
    for (int i = 0; i < s_page_count; i++) {
        if (pages[i]) {
            strncpy(s_pages[i], pages[i], MAX_PAGE_LENGTH - 1);
            s_pages[i][MAX_PAGE_LENGTH - 1] = '\0';
        } else {
            s_pages[i][0] = '\0';
        }
    }

    s_current_page = 0;
    s_char_progress = 0;
    s_type_timer = 0;
    s_state = DIALOGUE_STATE_TYPING;

    hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.75f, 1.0f);
}

void dialogue_trigger_ezlo_hint(void) {
    hal_audio_play_sound(SOUND_EZLO_ALERT, 0.95f, 1.0f);

    // Banco de dicas do Ezlo que giram a cada chamada
    static const char* hint0[] = {
        "Ei, Link! Esta e a misteriosa\nFloresta dos Minish (Deepwood)!",
        "Mantenha os olhos bem abertos!\nHa segredos e lendas por aqui."
    };
    static const char* hint1[] = {
        "Cuidado com os Octoroks vermelhos!\nEles cospem pedras velozes.",
        "Mas se você golpear com a espada\nno timing certo, podera rebate-las!"
    };
    static const char* hint2[] = {
        "Corte os arbustos com [A]!\nEles escondem Rupees e Corações!",
        "E lembre-se: segure [B] para dar\num Dash rapido pelas trilhas!"
    };
    static const char* hint3[] = {
        "Aperte [T] para alternar entre as\ntrilhas sonoras chiptune do GBA!",
        "E aperte [W] para ver o campo de\nvisao expandido em Widescreen 16:9!"
    };
    static const char* hint4[] = {
        "Dizem que os pequenos Minish so\npodem ser vistos por crianças!",
        "Procure por tocos magicos para\nencontrar o caminho sagrado!"
    };

    const char* const* chosen_pages = NULL;
    int page_num = 2;

    switch (s_current_hint_idx % 5) {
        case 0: chosen_pages = hint0; break;
        case 1: chosen_pages = hint1; break;
        case 2: chosen_pages = hint2; break;
        case 3: chosen_pages = hint3; break;
        case 4: chosen_pages = hint4; break;
    }
    s_current_hint_idx++;

    dialogue_show(SPEAKER_EZLO, "Ezlo", chosen_pages, page_num);
}

void dialogue_trigger_minish_talk(void) {
    static const char* minish_speech[] = {
        "Ola, nobre heroi de verde!\nEu sou um Minish dos bosques!",
        "Ha muitas eras, nosso povo forjou\na Espada Sagrada que salvou Hyrule.",
        "Que a brisa da floresta guie seus\npassos com coragem e sabedoria!"
    };
    dialogue_show(SPEAKER_FOREST_MINISH, "Minish", minish_speech, 3);
}

void dialogue_update(void) {
    s_anim_counter++;

    if (s_state != DIALOGUE_STATE_TYPING) return;

    s_type_timer++;
    if (s_type_timer >= 2) { // 1 caractere a cada 2 frames (~30 chars/segundo)
        s_type_timer = 0;

        int page_len = (int)strlen(s_pages[s_current_page]);
        if (s_char_progress < page_len) {
            s_char_progress++;

            // Som typewriter a cada 2 caracteres não-espaço
            char cur_c = s_pages[s_current_page][s_char_progress - 1];
            if (cur_c != ' ' && cur_c != '\n' && (s_char_progress % 2 == 0)) {
                float pitch = 1.0f + ((float)(rand() % 9) - 4.0f) * 0.02f;
                hal_audio_play_sound(SOUND_TEXT_BLIP, 0.40f, pitch);
            }
        } else {
            s_state = DIALOGUE_STATE_WAITING;
        }
    }
}

void dialogue_advance(void) {
    if (s_state == DIALOGUE_STATE_CLOSED) return;

    if (s_state == DIALOGUE_STATE_TYPING) {
        // Revela a página inteira imediatamente ao pressionar botão
        s_char_progress = (int)strlen(s_pages[s_current_page]);
        s_state = DIALOGUE_STATE_WAITING;
        return;
    }

    if (s_state == DIALOGUE_STATE_WAITING) {
        if (s_current_page + 1 < s_page_count) {
            s_current_page++;
            s_char_progress = 0;
            s_type_timer = 0;
            s_state = DIALOGUE_STATE_TYPING;
            hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.70f, 1.0f);
        } else {
            // Fim do diálogo
            s_state = DIALOGUE_STATE_CLOSED;
            hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.70f, 1.15f);
        }
    }
}

void dialogue_fast_forward(void) {
    if (s_state == DIALOGUE_STATE_TYPING) {
        // Acelera avanço de caractere quando segurado o botão B
        int page_len = (int)strlen(s_pages[s_current_page]);
        if (s_char_progress + 2 <= page_len) {
            s_char_progress += 2;
        } else {
            s_char_progress = page_len;
            s_state = DIALOGUE_STATE_WAITING;
        }
    }
}

bool dialogue_is_active(void) {
    return (s_state != DIALOGUE_STATE_CLOSED);
}

void dialogue_render(void) {
    if (s_state == DIALOGUE_STATE_CLOSED) return;

    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx) return;

    int screen_w = ctx->render_width;
    int screen_h = ctx->render_height;

    // Dimensões do Balão de Diálogo (Posicionado na parte inferior da tela)
    int margin_x = (ctx->widescreen) ? 14 : 8;
    int box_x = margin_x;
    int box_y = screen_h - 56;
    int box_w = screen_w - (margin_x * 2);
    int box_h = 50;

    // 1. Sombra projetada do balão de diálogo (2px offset)
    draw_rect_blend(box_x + 2, box_y + 2, box_w, box_h, 0x030805AA);

    // 2. Fundo esmeralda semi-transparente clássico
    draw_rect_blend(box_x, box_y, box_w, box_h, 0x081C10E8);

    // 3. Moldura decorada externa (Dourada / Bronze)
    u32 gold_outer = 0xD4AF37FF;
    u32 gold_inner = 0x8C7322FF;
    u32 gold_hi    = 0xFFE27AFF;

    // Bordas horizontais e verticais
    draw_rect_blend(box_x + 2, box_y, box_w - 4, 1, gold_hi);
    draw_rect_blend(box_x + 2, box_y + box_h - 1, box_w - 4, 1, gold_outer);
    draw_rect_blend(box_x, box_y + 2, 1, box_h - 4, gold_outer);
    draw_rect_blend(box_x + box_w - 1, box_y + 2, 1, box_h - 4, gold_outer);

    // Cantos chanfrados com detalhe ornamental
    hal_video_put_pixel(box_x + 1, box_y + 1, gold_hi);
    hal_video_put_pixel(box_x + box_w - 2, box_y + 1, gold_hi);
    hal_video_put_pixel(box_x + 1, box_y + box_h - 2, gold_inner);
    hal_video_put_pixel(box_x + box_w - 2, box_y + box_h - 2, gold_inner);

    // 4. Badge com o Nome do Orador no topo do balão
    if (s_speaker_name[0] != '\0') {
        int badge_w = font_get_text_width(s_speaker_name) + 12;
        int badge_x = box_x + 10;
        int badge_y = box_y - 6;
        int badge_h = 9;

        // Fundo do badge
        draw_rect_blend(badge_x, badge_y, badge_w, badge_h, 0x1A4022FF);
        // Borda dourada
        draw_rect_blend(badge_x, badge_y, badge_w, 1, gold_hi);
        draw_rect_blend(badge_x, badge_y + badge_h - 1, badge_w, 1, gold_outer);
        draw_rect_blend(badge_x, badge_y, 1, badge_h, gold_outer);
        draw_rect_blend(badge_x + badge_w - 1, badge_y, 1, badge_h, gold_outer);

        // Texto do nome
        u32 name_col = (s_speaker == SPEAKER_EZLO) ? 0xFFE27AFF : 0x77FF99FF;
        font_draw_text(badge_x + 6, badge_y + 1, s_speaker_name, name_col, true);
    }

    // 5. Retrato do Personagem (32x32) na lateral esquerda
    int port_x = box_x + 6;
    int port_y = box_y + 9;

    // Moldura do retrato
    draw_rect_blend(port_x - 1, port_y - 1, 34, 34, gold_inner);
    draw_rect_blend(port_x, port_y, 32, 32, 0x051008FF);

    if (s_speaker == SPEAKER_EZLO) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_ezlo(port_x, port_y, is_talking);
    } else if (s_speaker == SPEAKER_FOREST_MINISH) {
        render_portrait_minish(port_x, port_y);
    }

    // 6. Área de Texto com quebra de linhas (\n)
    int text_x = box_x + 44;
    int text_y = box_y + 9;
    int chars_left = s_char_progress;

    const char* p = s_pages[s_current_page];
    int line = 0;
    char line_buf[128];
    int line_idx = 0;

    for (int i = 0; p[i] != '\0' && chars_left > 0; i++) {
        if (p[i] == '\n') {
            line_buf[line_idx] = '\0';
            font_draw_text(text_x, text_y + (line * 12), line_buf, 0xF5F7FAFF, true);
            line++;
            line_idx = 0;
            chars_left--; // consome o '\n'
            continue;
        }

        line_buf[line_idx++] = p[i];
        chars_left--;

        if (chars_left == 0 || p[i + 1] == '\0') {
            line_buf[line_idx] = '\0';
            font_draw_text(text_x, text_y + (line * 12), line_buf, 0xF5F7FAFF, true);
        }
    }

    // 7. Indicador de Próxima Página (Seta Vermelha/Dourada saltitante ▼)
    if (s_state == DIALOGUE_STATE_WAITING) {
        int arrow_x = box_x + box_w - 14;
        int bounce = ((s_anim_counter / 12) % 2 == 1) ? 2 : 0;
        int arrow_y = box_y + box_h - 13 + bounce;

        // Desenha a setinha animada
        font_draw_char(arrow_x, arrow_y, FONT_CHAR_ARROW_DOWN, 0xFFCC00FF, true);
    }
}
