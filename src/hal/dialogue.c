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
static bool s_swiftblade_reward_pending = false;
static bool s_melari_reward_pending = false;

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

static void render_portrait_swiftblade(int px, int py, bool is_talking) {
    // Fundo escuro do dojo
    draw_rect_blend(px, py, 32, 32, 0x16120DFF);

    u32 c_fur       = 0x7A6A5AFF; // Pelo canino marrom-acinzentado do mestre
    u32 c_fur_dk    = 0x544638FF; // Sombra do pelo
    u32 c_muzzle    = 0xC2B2A0FF; // Focinho canino claro
    u32 c_muzzle_dk = 0x9E8F7EFF; // Sombra do focinho
    u32 c_nose      = 0x1C140FFF; // Focinho/nariz preto
    u32 c_band      = 0xDC2626FF; // Faixa vermelha marcial
    u32 c_band_dk   = 0x991B1BFF; // Sombra da faixa
    u32 c_white     = 0xFFFFFFFF; // Olhos / gola do kimono
    u32 c_black     = 0x111111FF; // Pupilas / sobrancelhas
    u32 c_gi        = 0x1E3324FF; // Kimono verde musgo escuro do mestre
    u32 c_gi_trim   = 0xE5E7EBFF; // Detalhe branco da gola cruzada

    int talk_offset = (is_talking && ((s_anim_counter / 5) % 2 == 1)) ? 1 : 0;

    // 1. Orelhas caninas pontudas de guerreiro (topo da cabeça)
    // Orelha esquerda
    for (int y = 2; y <= 8; y++) {
        for (int x = 6; x <= 11; x++) {
            if (x >= 12 - (y - 1) && x <= 11) {
                hal_video_put_pixel(px + x, py + y, (x == 11 || y == 8) ? c_fur_dk : c_fur);
            }
        }
    }
    // Interior orelha esq
    hal_video_put_pixel(px + 9, py + 5, 0x9C7C6EFF);
    hal_video_put_pixel(px + 10, py + 6, 0x9C7C6EFF);

    // Orelha direita
    for (int y = 2; y <= 8; y++) {
        for (int x = 20; x <= 25; x++) {
            if (x <= 19 + (y - 1) && x >= 20) {
                hal_video_put_pixel(px + x, py + y, (x == 20 || y == 8) ? c_fur_dk : c_fur);
            }
        }
    }
    // Interior orelha dir
    hal_video_put_pixel(px + 21, py + 6, 0x9C7C6EFF);
    hal_video_put_pixel(px + 22, py + 5, 0x9C7C6EFF);

    // 2. Cabeça canina (formato arredondado)
    for (int y = 7; y <= 23; y++) {
        for (int x = 7; x <= 24; x++) {
            float dx = (float)(x - 16) / 8.5f;
            float dy = (float)(y - 15) / 8.0f;
            if (dx * dx + dy * dy <= 1.0f) {
                u32 col = (x < 10 || x > 21 || y > 20) ? c_fur_dk : c_fur;
                hal_video_put_pixel(px + x, py + y, col);
            }
        }
    }

    // 3. Faixa marcial vermelha (Hachimaki) amarrada na testa
    for (int y = 9; y <= 12; y++) {
        for (int x = 8; x <= 23; x++) {
            u32 col = (y == 12) ? c_band_dk : c_band;
            hal_video_put_pixel(px + x, py + y, col);
        }
    }
    // Nó da faixa e fitas pendentes no lado direito
    draw_rect_blend(px + 24, py + 10, 3, 3, c_band);
    for (int y = 12; y <= 21; y++) {
        int trail_x = 25 + (int)(sinf((float)(y + s_anim_counter * 0.1f)) * 1.2f);
        hal_video_put_pixel(px + trail_x, py + y, c_band);
        hal_video_put_pixel(px + trail_x + 1, py + y, c_band_dk);
    }

    // 4. Sobrancelhas resolutas e olhos de mestre espadachim
    draw_rect_blend(px + 10, py + 13, 4, 1, c_black);
    draw_rect_blend(px + 18, py + 13, 4, 1, c_black);

    // Esclera branca e pupilas determinadas
    draw_rect_blend(px + 10, py + 14, 4, 3, c_white);
    draw_rect_blend(px + 18, py + 14, 4, 3, c_white);
    draw_rect_blend(px + 12, py + 15, 2, 2, c_black);
    draw_rect_blend(px + 18, py + 15, 2, 2, c_black);
    hal_video_put_pixel(px + 11, py + 14, c_white); // Brilho nos olhos
    hal_video_put_pixel(px + 19, py + 14, c_white);

    // 5. Focinho canino e bigodes de mestre
    int snout_y = 17;
    for (int y = 0; y <= 5; y++) {
        int w = 8 - y;
        if (w < 4) w = 4;
        for (int x = 16 - w / 2; x <= 16 + w / 2; x++) {
            u32 col = (y >= 4) ? c_muzzle_dk : c_muzzle;
            hal_video_put_pixel(px + x, py + snout_y + y, col);
        }
    }

    // Nariz preto
    draw_rect_blend(px + 14, py + 17, 4, 2, c_nose);
    hal_video_put_pixel(px + 15, py + 17, 0x443328FF); // Brilho no nariz

    // Boca que move ao falar
    if (talk_offset > 0) {
        draw_rect_blend(px + 14, py + 21, 4, 2, 0x331111FF);
        hal_video_put_pixel(px + 15, py + 22, 0xFF4444FF); // Língua
    } else {
        hal_video_put_pixel(px + 14, py + 21, c_nose);
        hal_video_put_pixel(px + 15, py + 21, c_nose);
        hal_video_put_pixel(px + 16, py + 21, c_nose);
        hal_video_put_pixel(px + 17, py + 21, c_nose);
    }

    // 6. Kimono/Gi do Mestre de Espadas com gola cruzada
    for (int y = 24; y <= 31; y++) {
        for (int x = 5; x <= 26; x++) {
            hal_video_put_pixel(px + x, py + y, c_gi);
        }
    }
    // Gola branca cruzada (estilo dojo tradicional)
    for (int i = 0; i <= 6; i++) {
        hal_video_put_pixel(px + 12 + i, py + 24 + i, c_gi_trim);
        hal_video_put_pixel(px + 13 + i, py + 24 + i, c_gi_trim);
        hal_video_put_pixel(px + 19 - i, py + 24 + i, c_gi_trim);
    }
}

static void render_portrait_shopkeeper(int px, int py, bool is_talking) {
    draw_rect_blend(px, py, 32, 32, 0x1B140EFF);

    u32 c_cap       = 0x4A148CFF; // Boina roxa mercantil
    u32 c_cap_dk    = 0x310D5CFF;
    u32 c_cap_trim  = 0xF1C40FFF; // Fita dourada na boina
    u32 c_skin      = 0xFDE8CDFF; // Pele clara
    u32 c_skin_dk   = 0xE0BA94FF;
    u32 c_hair      = 0x5D4037FF; // Cabelos castanhos
    u32 c_glasses   = 0xF59E0BFF; // Aro dourado dos óculos
    u32 c_lens      = 0xE0F2FEFF; // Lentes brilhantes
    u32 c_pupil     = 0x1E293BFF; // Olhos atentos
    u32 c_mustache  = 0x4E342EFF; // Bigode
    u32 c_shirt     = 0xFFFFFFFF; // Colarinho branco
    u32 c_tie       = 0xDC2626FF; // Gravata borboleta
    u32 c_apron     = 0x059669FF; // Avental esmeralda

    int talk_offset = (is_talking && ((s_anim_counter / 5) % 2 == 1)) ? 1 : 0;

    // 1. Boina roxa inclinada com fita dourada
    for (int y = 2; y <= 9; y++) {
        int w = 18 - (y - 2);
        for (int x = 16 - w / 2; x <= 16 + w / 2 + 3; x++) {
            u32 c = (x > 18 || y > 7) ? c_cap_dk : c_cap;
            hal_video_put_pixel(px + x, py + y - talk_offset, c);
        }
    }
    for (int x = 8; x <= 24; x++) {
        hal_video_put_pixel(px + x, py + 9 - talk_offset, c_cap_trim);
    }

    // 2. Cabelo e orelhas
    draw_rect_blend(px + 6, py + 10, 3, 7, c_hair);
    draw_rect_blend(px + 23, py + 10, 3, 7, c_hair);
    hal_video_put_pixel(px + 5, py + 15, c_skin_dk);
    hal_video_put_pixel(px + 26, py + 15, c_skin_dk);

    // 3. Rosto redondo e simpático
    for (int y = 10; y <= 22; y++) {
        for (int x = 8; x <= 23; x++) {
            float dx = (float)(x - 16) / 7.5f;
            float dy = (float)(y - 16) / 6.5f;
            if (dx * dx + dy * dy <= 1.0f) {
                u32 col = (y > 19) ? c_skin_dk : c_skin;
                hal_video_put_pixel(px + x, py + y, col);
            }
        }
    }

    // 4. Óculos redondos de Stockwell
    draw_rect_blend(px + 10, py + 13, 4, 4, c_glasses);
    draw_rect_blend(px + 11, py + 14, 2, 2, c_lens);
    hal_video_put_pixel(px + 12, py + 14, c_pupil);
    hal_video_put_pixel(px + 11, py + 13, 0xFFFFFFFF);

    draw_rect_blend(px + 18, py + 13, 4, 4, c_glasses);
    draw_rect_blend(px + 19, py + 14, 2, 2, c_lens);
    hal_video_put_pixel(px + 19, py + 14, c_pupil);
    hal_video_put_pixel(px + 20, py + 13, 0xFFFFFFFF);

    draw_rect_blend(px + 14, py + 14, 4, 1, c_glasses);

    // 5. Bigodinho e boca
    draw_rect_blend(px + 14, py + 18, 4, 2, c_mustache);
    if (talk_offset > 0) {
        draw_rect_blend(px + 15, py + 20, 2, 2, 0x450A0AFF);
    } else {
        hal_video_put_pixel(px + 15, py + 20, 0x8A4520FF);
        hal_video_put_pixel(px + 16, py + 20, 0x8A4520FF);
    }

    // 6. Colarinho, gravata e avental
    draw_rect_blend(px + 12, py + 23, 8, 3, c_shirt);
    draw_rect_blend(px + 14, py + 23, 4, 2, c_tie);
    hal_video_put_pixel(px + 15, py + 23, 0xFF6B6BFF);
    draw_rect_blend(px + 6, py + 25, 20, 7, c_apron);
    draw_rect_blend(px + 8, py + 24, 2, 8, 0x047857FF);
    draw_rect_blend(px + 22, py + 24, 2, 8, 0x047857FF);
}

static void render_portrait_town_citizen(int px, int py, bool is_talking) {
    draw_rect_blend(px, py, 32, 32, 0x152834FF);

    u32 c_bonnet    = 0xEC4899FF; // Touca rosa
    u32 c_bonnet_dk = 0xBE185DFF;
    u32 c_bonnet_lt = 0xFBCFE8FF;
    u32 c_skin      = 0xFDE8CDFF; // Pele clara
    u32 c_hair      = 0xD97706FF; // Cabelos castanho-dourados
    u32 c_blush     = 0xFCA5A5FF; // Bochechas rosadas
    u32 c_eye       = 0x1E1B4BFF; // Olhos
    u32 c_dress     = 0x0284C7FF; // Vestido azul celeste
    u32 c_collar    = 0xFFFFFFFF; // Gola branca

    int talk_offset = (is_talking && ((s_anim_counter / 5) % 2 == 1)) ? 1 : 0;

    // Touca / Bonnet
    for (int y = 2; y <= 11; y++) {
        for (int x = 6; x <= 25; x++) {
            float dx = (float)(x - 16) / 9.5f;
            float dy = (float)(y - 7) / 5.5f;
            if (dx * dx + dy * dy <= 1.0f) {
                u32 col = (y > 8) ? c_bonnet_dk : c_bonnet;
                hal_video_put_pixel(px + x, py + y, col);
            }
        }
    }
    for (int x = 5; x <= 26; x += 2) {
        hal_video_put_pixel(px + x, py + 11, c_bonnet_lt);
        hal_video_put_pixel(px + x + 1, py + 12, 0xFFFFFFFF);
    }

    // Cabelos
    draw_rect_blend(px + 7, py + 12, 3, 8, c_hair);
    draw_rect_blend(px + 22, py + 12, 3, 8, c_hair);
    hal_video_put_pixel(px + 9, py + 13, 0xF59E0BFF);
    hal_video_put_pixel(px + 22, py + 13, 0xF59E0BFF);

    // Rosto
    for (int y = 12; y <= 23; y++) {
        for (int x = 9; x <= 22; x++) {
            float dx = (float)(x - 16) / 6.5f;
            float dy = (float)(y - 17) / 5.5f;
            if (dx * dx + dy * dy <= 1.0f) {
                hal_video_put_pixel(px + x, py + y, c_skin);
            }
        }
    }

    // Olhos e bochechas
    draw_rect_blend(px + 11, py + 16, 2, 3, c_eye);
    draw_rect_blend(px + 19, py + 16, 2, 3, c_eye);
    hal_video_put_pixel(px + 11, py + 16, 0xFFFFFFFF);
    hal_video_put_pixel(px + 19, py + 16, 0xFFFFFFFF);
    draw_rect_blend(px + 9, py + 18, 3, 2, c_blush);
    draw_rect_blend(px + 20, py + 18, 3, 2, c_blush);

    if (talk_offset > 0) {
        draw_rect_blend(px + 15, py + 21, 2, 2, 0xBE123CFF);
    } else {
        hal_video_put_pixel(px + 14, py + 21, 0xE11D48FF);
        hal_video_put_pixel(px + 15, py + 21, 0xE11D48FF);
        hal_video_put_pixel(px + 16, py + 21, 0xE11D48FF);
    }

    // Vestido azul celeste
    draw_rect_blend(px + 6, py + 25, 20, 7, c_dress);
    draw_rect_blend(px + 12, py + 24, 8, 3, c_collar);
    hal_video_put_pixel(px + 15, py + 26, c_bonnet);
}

static void render_portrait_town_guard(int px, int py) {
    draw_rect_blend(px, py, 32, 32, 0x1A202CFF);

    u32 c_steel     = 0xD1D5DBFF; // Aço brilhante
    u32 c_steel_dk  = 0x6B7280FF;
    u32 c_steel_hi  = 0xFFFFFFFF;
    u32 c_plume     = 0xDC2626FF; // Pluma vermelha
    u32 c_plume_dk  = 0x991B1BFF;
    u32 c_visor     = 0x111827FF;
    u32 c_eye_glow  = 0x60A5FAFF;
    u32 c_tunic     = 0x1E3A8AFF; // Manto azul real
    u32 c_gold      = 0xF59E0BFF;

    // Pluma vermelha
    for (int y = 2; y <= 8; y++) {
        int w = 4 + (y - 2);
        for (int x = 16 - w / 2; x <= 16 + w / 2; x++) {
            u32 col = (y > 5) ? c_plume_dk : c_plume;
            hal_video_put_pixel(px + x, py + y, col);
        }
    }
    draw_rect_blend(px + 15, py + 1, 3, 2, c_plume);

    // Elmo de aço
    for (int y = 7; y <= 21; y++) {
        for (int x = 8; x <= 24; x++) {
            float dx = (float)(x - 16) / 7.5f;
            float dy = (float)(y - 14) / 7.0f;
            if (dx * dx + dy * dy <= 1.0f) {
                u32 col = (x > 18 || y > 18) ? c_steel_dk : c_steel;
                hal_video_put_pixel(px + x, py + y, col);
            }
        }
    }
    for (int y = 7; y <= 13; y++) {
        hal_video_put_pixel(px + 16, py + y, c_steel_hi);
    }

    // Viseira
    draw_rect_blend(px + 10, py + 14, 13, 4, c_visor);
    draw_rect_blend(px + 12, py + 15, 3, 2, c_eye_glow);
    draw_rect_blend(px + 18, py + 15, 3, 2, c_eye_glow);
    hal_video_put_pixel(px + 13, py + 15, 0xFFFFFFFF);
    hal_video_put_pixel(px + 19, py + 15, 0xFFFFFFFF);

    for (int x = 12; x <= 20; x += 2) {
        hal_video_put_pixel(px + x, py + 19, c_visor);
        hal_video_put_pixel(px + x, py + 20, c_visor);
    }

    // Peitoral e manto real
    draw_rect_blend(px + 6, py + 22, 20, 10, c_tunic);
    draw_rect_blend(px + 10, py + 22, 12, 10, c_steel);
    draw_rect_blend(px + 12, py + 23, 8, 8, c_steel_hi);
    hal_video_put_pixel(px + 15, py + 25, c_gold);
    hal_video_put_pixel(px + 16, py + 25, c_gold);
    hal_video_put_pixel(px + 14, py + 26, c_gold);
    hal_video_put_pixel(px + 15, py + 26, c_gold);
    hal_video_put_pixel(px + 16, py + 26, c_gold);
    hal_video_put_pixel(px + 17, py + 26, c_gold);
}

static void render_portrait_gentari(int px, int py, bool is_talking) {
    draw_rect_blend(px, py, 32, 32, 0x160C28FF);

    u32 c_mitre     = 0xF59E0BFF; // Mitra sagrada dourada
    u32 c_mitre_dk  = 0xB45309FF; // Sombra da mitra
    u32 c_gem       = 0x10B981FF; // Joia esmeralda
    u32 c_skin      = 0xFDE8CDFF; // Pele clara
    u32 c_beard     = 0xFFFFFFFF; // Longa barba branca do anciao
    u32 c_beard_dk  = 0xE2E8F0FF; // Sombra da barba
    u32 c_robe      = 0x7C3AEDFF; // Manto violeta
    u32 c_robe_dk   = 0x5B21B6FF;
    u32 c_gold      = 0xFDE047FF; // Borda dourada

    int talk_offset = (is_talking && ((s_anim_counter / 5) % 2 == 1)) ? 1 : 0;

    // Mitra alta ornamental
    for (int y = 1; y <= 13; y++) {
        int w = 4 + (y * 2) / 3;
        if (w > 12) w = 12;
        int sx = 16 - w / 2;
        for (int x = sx; x <= sx + w; x++) {
            hal_video_put_pixel(px + x, py + y, (x == sx || x == sx + w) ? c_mitre_dk : c_mitre);
        }
    }
    // Gema esmeralda central na mitra
    draw_rect_blend(px + 14, py + 4, 4, 4, c_gem);
    hal_video_put_pixel(px + 15, py + 5, 0xFFFFFFFF);

    // Orelhas pontudas Minish
    hal_video_put_pixel(px + 6, py + 15, c_skin);
    hal_video_put_pixel(px + 7, py + 16, c_skin);
    hal_video_put_pixel(px + 25, py + 15, c_skin);
    hal_video_put_pixel(px + 24, py + 16, c_skin);

    // Rosto sábio do ancião
    for (int y = 13; y <= 20; y++) {
        for (int x = 9; x <= 22; x++) {
            hal_video_put_pixel(px + x, py + y, c_skin);
        }
    }

    // Olhos serenos
    hal_video_put_pixel(px + 12, py + 16, 0x1E1B4BFF);
    hal_video_put_pixel(px + 19, py + 16, 0x1E1B4BFF);
    hal_video_put_pixel(px + 11, py + 15, 0xFFFFFFFF); // Sobrancelha branca esq
    hal_video_put_pixel(px + 12, py + 15, 0xFFFFFFFF);
    hal_video_put_pixel(px + 19, py + 15, 0xFFFFFFFF); // Sobrancelha branca dir
    hal_video_put_pixel(px + 20, py + 15, 0xFFFFFFFF);

    // Longa barba branca majestosa descendo
    for (int y = 18; y <= 27 + talk_offset; y++) {
        int bw = 12 - (y - 18);
        if (bw < 4) bw = 4;
        int bx = 16 - bw / 2;
        for (int x = bx; x <= bx + bw; x++) {
            hal_video_put_pixel(px + x, py + y, (y > 23 || x == bx || x == bx + bw) ? c_beard_dk : c_beard);
        }
    }

    // Manto violeta no peito e ombros
    draw_rect_blend(px + 6, py + 23, 7, 8, c_robe);
    draw_rect_blend(px + 19, py + 23, 7, 8, c_robe);
    draw_rect_blend(px + 8, py + 24, 2, 8, c_gold);
    draw_rect_blend(px + 22, py + 24, 2, 8, c_gold);
}

static void render_portrait_festari(int px, int py, bool is_talking) {
    draw_rect_blend(px, py, 32, 32, 0x0F172AFF);

    u32 c_cowl      = 0x2563EBFF; // Capuz azul de sacerdote
    u32 c_cowl_dk   = 0x1D4ED8FF;
    u32 c_cowl_trim = 0xE0E7FFFF; // Borda branca do capuz
    u32 c_skin      = 0xFDE8CDFF; // Pele clara
    u32 c_eye       = 0x0F172AFF; // Olhos
    u32 c_robe      = 0x1E40AFFF; // Manto monástico
    u32 c_stole     = 0xF59E0BFF; // Estola dourada

    int talk_offset = (is_talking && ((s_anim_counter / 5) % 2 == 1)) ? 1 : 0;

    // Capuz monástico
    for (int y = 2; y <= 16; y++) {
        int w = 6 + (y * 2) / 3;
        if (w > 16) w = 16;
        int sx = 16 - w / 2;
        for (int x = sx; x <= sx + w; x++) {
            hal_video_put_pixel(px + x, py + y, (x == sx || x == sx + w || y == 2) ? c_cowl_dk : c_cowl);
        }
    }
    // Borda clara do capuz ao redor da face
    for (int x = 8; x <= 23; x++) {
        hal_video_put_pixel(px + x, py + 12, c_cowl_trim);
    }

    // Orelhas pontudas Minish saindo do capuz
    hal_video_put_pixel(px + 6, py + 16, c_skin);
    hal_video_put_pixel(px + 7, py + 17, c_skin);
    hal_video_put_pixel(px + 25, py + 16, c_skin);
    hal_video_put_pixel(px + 24, py + 17, c_skin);

    // Rosto sereno
    for (int y = 14; y <= 23; y++) {
        for (int x = 9; x <= 22; x++) {
            hal_video_put_pixel(px + x, py + y, c_skin);
        }
    }

    // Olhos
    hal_video_put_pixel(px + 12, py + 17, c_eye);
    hal_video_put_pixel(px + 19, py + 17, c_eye);

    // Boca
    if (talk_offset > 0) {
        draw_rect_blend(px + 15, py + 21, 2, 2, 0x78350FFF);
    } else {
        hal_video_put_pixel(px + 15, py + 21, 0x8A4520FF);
        hal_video_put_pixel(px + 16, py + 21, 0x8A4520FF);
    }

    // Manto azul e estola dourada Picori
    draw_rect_blend(px + 6, py + 24, 20, 8, c_robe);
    draw_rect_blend(px + 13, py + 24, 6, 8, c_stole);
    hal_video_put_pixel(px + 15, py + 26, 0xFFFFFFFF);
    hal_video_put_pixel(px + 16, py + 26, 0xFFFFFFFF);
}

static void render_portrait_village_minish(int px, int py) {
    draw_rect_blend(px, py, 32, 32, 0x0A2612FF);

    u32 c_hat   = 0x10B981FF; // Gorro verde esmeralda
    u32 c_pom   = 0xFDE047FF; // Pom-pom dourado
    u32 c_skin  = 0xFDE8CDFF; // Pele clara Minish
    u32 c_hair  = 0xFFF2DEFF; // Cabelos felpudos
    u32 c_blush = 0xFCA5A5FF; // Bochechas rosadas
    u32 c_eyes  = 0x111111FF; // Olhos
    u32 c_tunic = 0xD97706FF; // Túnica terracota

    // Gorro pontudo Minish verde
    for (int y = 3; y <= 16; y++) {
        int w = 3 + (y - 3);
        if (w > 14) w = 14;
        for (int x = 16 - w / 2; x <= 16 + w / 2; x++) {
            hal_video_put_pixel(px + x, py + y, c_hat);
        }
    }
    // Pom-pom dourado no topo
    draw_rect_blend(px + 14, py + 2, 4, 3, c_pom);

    // Orelhas pontudas
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

    // Olhos pretos alegres
    hal_video_put_pixel(px + 12, py + 18, c_eyes);
    hal_video_put_pixel(px + 20, py + 18, c_eyes);

    // Bochechas rosadas
    hal_video_put_pixel(px + 10, py + 20, c_blush);
    hal_video_put_pixel(px + 22, py + 20, c_blush);

    // Sorriso alegre
    hal_video_put_pixel(px + 15, py + 21, 0x8A4520FF);
    hal_video_put_pixel(px + 16, py + 22, 0x8A4520FF);
    hal_video_put_pixel(px + 17, py + 21, 0x8A4520FF);

    // Túnica terracota
    draw_rect_blend(px + 10, py + 25, 13, 6, c_tunic);
}

static void render_portrait_malon(int px, int py, bool is_talking) {
    draw_rect_blend(px, py, 32, 32, 0x0A2612FF);

    u32 c_hair    = 0xEA580CFF;
    u32 c_bandana = 0xFACC15FF;
    u32 c_skin    = 0xFDE8CDFF;
    u32 c_bodice  = 0x3B82F6FF;
    u32 c_apron   = 0xFEF3C7FF;
    u32 c_cheek   = 0xFB7185FF;
    u32 c_eyes    = 0x1E293BFF;

    // Cabelo ruivo volumoso
    draw_rect_blend(px + 4, py + 2, 24, 10, c_hair);
    draw_rect_blend(px + 3, py + 6, 26, 18, c_hair);
    // Faixa amarela (bandana)
    draw_rect_blend(px + 7, py + 4, 18, 3, c_bandana);

    // Rosto
    draw_rect_blend(px + 8, py + 9, 16, 12, c_skin);
    // Olhos
    hal_video_put_pixel(px + 12, py + 13, c_eyes);
    hal_video_put_pixel(px + 19, py + 13, c_eyes);
    // Bochechas rosadas
    hal_video_put_pixel(px + 10, py + 16, c_cheek);
    hal_video_put_pixel(px + 21, py + 16, c_cheek);

    // Boca (animada se falando)
    if (is_talking && ((s_char_progress / 3) % 2 == 1)) {
        draw_rect_blend(px + 15, py + 17, 3, 2, 0x881337FF);
    } else {
        hal_video_put_pixel(px + 15, py + 17, 0x881337FF);
        hal_video_put_pixel(px + 16, py + 18, 0x881337FF);
    }

    // Colete azul e avental
    draw_rect_blend(px + 8, py + 22, 16, 8, c_bodice);
    draw_rect_blend(px + 12, py + 23, 8, 7, c_apron);
}

static void render_portrait_business_scrub(int px, int py) {
    draw_rect_blend(px, py, 32, 32, 0x1E1208FF);

    u32 c_wood   = 0x78350FFF;
    u32 c_bark   = 0x451A03FF;
    u32 c_snout  = 0x9A3412FF;
    u32 c_leaves = 0x16A34AFF;
    u32 c_eyes   = 0xFDE047FF;

    // Folhagem verde no topo da cabeça
    for (int dy = 2; dy <= 8; dy++) {
        draw_rect_blend(px + 8, py + dy, 16, 2, c_leaves);
    }
    draw_rect_blend(px + 13, py + 1, 6, 3, 0x22C55EFF);

    // Corpo e cabeça de casca de madeira
    draw_rect_blend(px + 8, py + 9, 16, 15, c_wood);
    draw_rect_blend(px + 6, py + 12, 20, 10, c_wood);

    // Olhos amarelos brilhantes
    hal_video_put_pixel(px + 10, py + 12, c_eyes);
    hal_video_put_pixel(px + 11, py + 12, c_eyes);
    hal_video_put_pixel(px + 20, py + 12, c_eyes);
    hal_video_put_pixel(px + 21, py + 12, c_eyes);

    // Focinho tubular de madeira (para cuspir nozes)
    draw_rect_blend(px + 12, py + 15, 8, 6, c_snout);
    draw_rect_blend(px + 14, py + 17, 4, 3, c_bark);

    // Colar de folhas
    draw_rect_blend(px + 7, py + 24, 18, 5, c_leaves);
}

static void render_portrait_melari(int px, int py, bool is_talking) {
    draw_rect_blend(px, py, 32, 32, 0x18100AFF);

    u32 c_skin   = 0xFDE8CDFF;
    u32 c_hair   = 0xE2E8F0FF; // Cabelos e barba grisalha de ferreiro
    u32 c_goggle = 0x92400EFF; // Armação dos óculos de proteção
    u32 c_lens   = 0xF97316FF; // Lentes âmbar brilhantes
    u32 c_apron  = 0x78350FFF; // Avental de couro
    u32 c_shirt  = 0xDC2626FF; // Túnica vermelha dos ferreiros Minish
    u32 c_eyes   = 0x0F172AFF;

    // Cabelo grisalho robusto
    draw_rect_blend(px + 6, py + 3, 20, 8, c_hair);
    // Óculos de ferreiro levantados na testa
    draw_rect_blend(px + 8, py + 5, 16, 4, c_goggle);
    draw_rect_blend(px + 10, py + 6, 4, 3, c_lens);
    draw_rect_blend(px + 18, py + 6, 4, 3, c_lens);

    // Rosto
    draw_rect_blend(px + 8, py + 9, 16, 10, c_skin);
    // Sobrancelhas grisalhas franzidas e olhos
    draw_rect_blend(px + 10, py + 10, 4, 1, c_hair);
    draw_rect_blend(px + 18, py + 10, 4, 1, c_hair);
    hal_video_put_pixel(px + 11, py + 12, c_eyes);
    hal_video_put_pixel(px + 20, py + 12, c_eyes);

    // Nariz arredondado
    draw_rect_blend(px + 15, py + 13, 2, 2, 0xFCA5A5FF);

    // Barba volumosa de ferreiro cobrindo queixo
    draw_rect_blend(px + 7, py + 15, 18, 9, c_hair);
    draw_rect_blend(px + 10, py + 23, 12, 3, c_hair);
    draw_rect_blend(px + 13, py + 25, 6, 2, c_hair);

    // Boca aberta ao falar no meio da barba
    if (is_talking && ((s_char_progress / 3) % 2 == 1)) {
        draw_rect_blend(px + 14, py + 17, 4, 2, 0x450A0AFF);
    }

    // Túnica vermelha e avental de ferreiro
    draw_rect_blend(px + 6, py + 24, 20, 7, c_shirt);
    draw_rect_blend(px + 11, py + 24, 10, 7, c_apron);
}

static void render_portrait_mountain_minish(int px, int py) {
    draw_rect_blend(px, py, 32, 32, 0x111827FF);

    u32 c_helmet = 0xFACC15FF; // Capacete amarelo de minerador
    u32 c_lamp   = 0xFFFFFFFF; // Lanterna do capacete
    u32 c_beam   = 0xFEF08AFF; // Brilho da luz
    u32 c_skin   = 0xFDE8CDFF;
    u32 c_eyes   = 0x1E293BFF;
    u32 c_cloth  = 0x2563EBFF; // Macacão azul
    u32 c_cheek  = 0xFCA5A5FF;

    // Capacete de minerador amarelo
    draw_rect_blend(px + 7, py + 3, 18, 8, c_helmet);
    draw_rect_blend(px + 5, py + 9, 22, 2, 0xCA8A04FF); // Aba
    // Lanterna central
    draw_rect_blend(px + 14, py + 5, 4, 3, c_lamp);
    hal_video_put_pixel(px + 15, py + 4, c_beam);
    hal_video_put_pixel(px + 16, py + 4, c_beam);

    // Rosto do Minish
    draw_rect_blend(px + 8, py + 11, 16, 11, c_skin);
    // Olhos brilhantes
    hal_video_put_pixel(px + 11, py + 14, c_eyes);
    hal_video_put_pixel(px + 20, py + 14, c_eyes);
    // Bochechas rosadas
    hal_video_put_pixel(px + 9, py + 16, c_cheek);
    hal_video_put_pixel(px + 22, py + 16, c_cheek);
    // Sorriso
    draw_rect_blend(px + 14, py + 18, 4, 1, 0x991B1BFF);

    // Lenço no pescoço e macacão azul de minerador
    draw_rect_blend(px + 9, py + 22, 14, 2, 0xDC2626FF);
    draw_rect_blend(px + 7, py + 24, 18, 7, c_cloth);
}

static void render_portrait_smith(int px, int py, bool is_talking) {
    // Fundo quente de forja / carvalho rústico
    draw_rect_blend(px, py, 32, 32, 0x1A1009FF);

    u32 c_skin    = 0xFDE8CDFF; // Pele de ferreiro curtida
    u32 c_skin_dk = 0xE2B991FF; // Sombra da pele
    u32 c_hair    = 0xF8FAFCFF; // Cabelos e barba branca reluzente
    u32 c_hair_dk = 0xCBD5E1FF; // Sombra dos fios brancos
    u32 c_shirt   = 0xB91C1CFF; // Túnica vermelha de ferreiro
    u32 c_strap   = 0x78350FFF; // Alças de couro
    u32 c_gold    = 0xD4AF37FF; // Fivelas das alças
    u32 c_black   = 0x111111FF; // Olhos

    int talk_offset = (is_talking && ((s_anim_counter / 6) % 2 == 1)) ? 1 : 0;

    // Cabelos brancos volumosos nas têmporas
    draw_rect_blend(px + 5, py + 8, 4, 11, c_hair);
    draw_rect_blend(px + 23, py + 8, 4, 11, c_hair);
    draw_rect_blend(px + 4, py + 10, 2, 7, c_hair_dk);
    draw_rect_blend(px + 26, py + 10, 2, 7, c_hair_dk);

    // Cabeça e testa
    draw_rect_blend(px + 8, py + 5, 16, 12, c_skin);
    draw_rect_blend(px + 9, py + 4, 14, 2, c_skin_dk);

    // Sobrancelhas grossas brancas de ferreiro
    draw_rect_blend(px + 8, py + 8, 6, 2, c_hair);
    draw_rect_blend(px + 18, py + 8, 6, 2, c_hair);
    hal_video_put_pixel(px + 7, py + 9, c_hair_dk);
    hal_video_put_pixel(px + 24, py + 9, c_hair_dk);

    // Olhos escuros determinados
    draw_rect_blend(px + 10, py + 11, 3, 2, c_black);
    draw_rect_blend(px + 19, py + 11, 3, 2, c_black);
    hal_video_put_pixel(px + 10, py + 11, 0xFFFFFFFF);
    hal_video_put_pixel(px + 19, py + 11, 0xFFFFFFFF);

    // Nariz forte
    draw_rect_blend(px + 14, py + 11, 4, 4, c_skin);
    draw_rect_blend(px + 13, py + 14, 6, 2, c_skin_dk);

    // Bigode volumoso
    draw_rect_blend(px + 9, py + 16, 14, 3, c_hair);
    draw_rect_blend(px + 11, py + 18, 10, 2, c_hair_dk);

    // Boca abrindo e fechando ao falar
    if (talk_offset) {
        draw_rect_blend(px + 14, py + 18, 4, 2, 0x450A0AFF);
    }

    // Barba branca majestosa cobrindo o queixo e peito
    draw_rect_blend(px + 7, py + 19, 18, 7, c_hair);
    draw_rect_blend(px + 9, py + 26, 14, 3, c_hair);
    draw_rect_blend(px + 11, py + 29, 10, 2, c_hair_dk);
    draw_rect_blend(px + 8, py + 22, 2, 5, c_hair_dk);
    draw_rect_blend(px + 22, py + 22, 2, 5, c_hair_dk);

    // Túnica vermelha de ferreiro e avental de couro
    draw_rect_blend(px + 4, py + 25, 4, 6, c_shirt);
    draw_rect_blend(px + 24, py + 25, 4, 6, c_shirt);
    draw_rect_blend(px + 7, py + 26, 3, 5, c_strap);
    draw_rect_blend(px + 22, py + 26, 3, 5, c_strap);
    hal_video_put_pixel(px + 8, py + 27, c_gold);
    hal_video_put_pixel(px + 23, py + 27, c_gold);
}

static void render_portrait_mayor_hagen(int px, int py, bool is_talking) {
    // Fundo azul municipal de Hyrule
    draw_rect_blend(px, py, 32, 32, 0x0F172AFF);

    u32 c_skin     = 0xFDE8CDFF; // Pele distinta
    u32 c_skin_dk  = 0xE2B991FF; // Sombra
    u32 c_hat      = 0x1D4ED8FF; // Cartola azul cerúleo
    u32 c_hat_dk   = 0x1E3A8AFF; // Sombra da cartola
    u32 c_ribbon   = 0xFBBF24FF; // Fita dourada
    u32 c_feather  = 0xDC2626FF; // Pluma vermelha
    u32 c_monocle  = 0xFDE047FF; // Monóculo dourado
    u32 c_glass    = 0x7DD3FCFF; // Lente do monóculo
    u32 c_mustache = 0x78350FFF; // Bigode castanho vitoriano
    u32 c_coat     = 0x1E3A8AFF; // Casaco nobre
    u32 c_vest     = 0xD97706FF; // Colete dourado
    u32 c_cravat   = 0xF8FAFCFF; // Lenço branco
    u32 c_black    = 0x111111FF;

    int talk_offset = (is_talking && ((s_anim_counter / 6) % 2 == 1)) ? 1 : 0;

    // Cartola azul do Prefeito (copa alta)
    draw_rect_blend(px + 10, py + 2, 12, 9, c_hat);
    draw_rect_blend(px + 11, py + 2, 10, 2, 0x3B82F6FF);
    // Fita dourada da cartola
    draw_rect_blend(px + 10, py + 9, 12, 2, c_ribbon);
    // Aba larga da cartola
    draw_rect_blend(px + 6, py + 11, 20, 2, c_hat_dk);
    // Pluma vermelha pomposa
    draw_rect_blend(px + 7, py + 3, 3, 8, c_feather);
    hal_video_put_pixel(px + 8, py + 2, 0xF87171FF);

    // Rosto distinto
    draw_rect_blend(px + 9, py + 13, 14, 11, c_skin);
    draw_rect_blend(px + 8, py + 14, 2, 6, c_skin_dk);
    draw_rect_blend(px + 22, py + 14, 2, 6, c_skin_dk);

    // Olho direito normal
    draw_rect_blend(px + 11, py + 15, 2, 2, c_black);
    hal_video_put_pixel(px + 11, py + 15, 0xFFFFFFFF);

    // Olho esquerdo com Monóculo Dourado
    draw_rect_blend(px + 17, py + 14, 5, 5, c_monocle);
    draw_rect_blend(px + 18, py + 15, 3, 3, c_glass);
    hal_video_put_pixel(px + 18, py + 15, 0xFFFFFFFF);
    hal_video_put_pixel(px + 19, py + 16, c_black);
    // Correntinha dourada do monóculo
    hal_video_put_pixel(px + 22, py + 17, c_monocle);
    hal_video_put_pixel(px + 22, py + 19, c_monocle);
    hal_video_put_pixel(px + 21, py + 21, c_monocle);

    // Nariz ilustre
    draw_rect_blend(px + 14, py + 15, 3, 3, c_skin_dk);

    // Bigode aristocrático curvado
    draw_rect_blend(px + 11, py + 19, 10, 2, c_mustache);
    hal_video_put_pixel(px + 10, py + 18, c_mustache);
    hal_video_put_pixel(px + 21, py + 18, c_mustache);

    // Boca ao falar
    if (talk_offset) {
        draw_rect_blend(px + 14, py + 21, 4, 1, 0x881337FF);
    }

    // Queixo
    draw_rect_blend(px + 13, py + 22, 6, 2, c_skin);

    // Cravat / Lenço nobre branco e jaqueta cerúlea com colete
    draw_rect_blend(px + 6, py + 24, 20, 7, c_coat);
    draw_rect_blend(px + 11, py + 24, 10, 7, c_vest);
    draw_rect_blend(px + 13, py + 24, 6, 5, c_cravat);
    hal_video_put_pixel(px + 15, py + 27, c_ribbon);
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

void dialogue_trigger_ezlo_minish_hint(void) {
    hal_audio_play_sound(SOUND_EZLO_ALERT, 0.95f, 1.0f);

    static const char* minish_hint[] = {
        "Incrível, Link! Você agora e do\ntamanho exato de um Minish!",
        "Procure pelo Tronco Oco a leste do\ntoco! So nesse tamanho podemos passar!",
        "Para voltar ao tamanho humano,\nbasta subir novamente no Toco Minish!"
    };

    dialogue_show(SPEAKER_EZLO, "Ezlo", minish_hint, 3);
}

void dialogue_trigger_minish_talk(void) {
    static const char* minish_speech[] = {
        "Ola, nobre heroi de verde!\nEu sou um Minish dos bosques!",
        "Ha muitas eras, nosso povo forjou\na Espada Sagrada que salvou Hyrule.",
        "Que a brisa da floresta guie seus\npassos com coragem e sabedoria!"
    };
    dialogue_show(SPEAKER_FOREST_MINISH, "Minish", minish_speech, 3);
}

void dialogue_trigger_swiftblade_talk(bool already_learned) {
    if (!already_learned) {
        s_swiftblade_reward_pending = true;
        static const char* swiftblade_training[] = {
            "Saudacoes, heroi!\nSou Mestre Swiftblade!",
            "Treine a nobre arte:\no lendario Spin Attack!",
            "Segure [A] para focar\ne solte para o Giro!",
            "Tome este Pergaminho!\nDomine o corte circular!"
        };
        dialogue_show(SPEAKER_SWIFTBLADE, "Swiftblade", swiftblade_training, 4);
    } else {
        static const char* swiftblade_reminder[] = {
            "Treine com afinco!\nFoque a sua energia.",
            "O corte em 360 graus\ne temido em Hyrule!"
        };
        dialogue_show(SPEAKER_SWIFTBLADE, "Swiftblade", swiftblade_reminder, 2);
    }
}

void dialogue_trigger_smith_talk(void) {
    static const char* pages[] = {
        "Link, meu rapaz!\nForjei a espada real.",
        "Honre nossa ferraria!\nQue a coragem te guie."
    };
    dialogue_show(SPEAKER_SMITH, "Mestre Smith", pages, 2);
    hal_audio_play_sound(SOUND_SWITCH_CLICK, 0.9f, 1.1f);
}

void dialogue_trigger_mayor_hagen_talk(void) {
    static const char* pages[] = {
        "Bem-vindo a Hyrule!\nSou o Prefeito Hagen.",
        "O festival centenario\ntraz alegria ao reino!"
    };
    dialogue_show(SPEAKER_MAYOR_HAGEN, "Prefeito Hagen", pages, 2);
    hal_audio_play_sound(SOUND_SECRET, 0.8f, 1.2f);
}

bool dialogue_is_swiftblade_reward_pending(void) {
    return s_swiftblade_reward_pending;
}

void dialogue_clear_swiftblade_reward(void) {
    s_swiftblade_reward_pending = false;
}

void dialogue_trigger_shopkeeper_talk(int link_rupees) {
    static char buf[256];
    snprintf(buf, sizeof(buf),
             "Voce possui %d Rupees na sacola!\nAproxime-se do balcao e aperte [A]\npara comprar quando quiser!",
             link_rupees);

    static const char* pages[3];
    pages[0] = "Bem-vindo a Loja de Stockwell!\nTrabalho duro para trazer as melhores\nmercadorias de toda a terra de Hyrule!";
    pages[1] = "Meus produtos de hoje:\n- Pocao Vermelha (Cura Total): 30 R\n- Pedaco de Coracao (+1 Max HP): 80 R\n- Bolsa de Bombas: 50 R";
    pages[2] = buf;

    dialogue_show(SPEAKER_SHOPKEEPER, "Stockwell", pages, 3);
}

void dialogue_trigger_town_citizen_talk(void) {
    static const char* citizen_speech[] = {
        "Ola, rapazinho! O sol brilha radiante\nsobre a nossa querida Cidade de Hyrule!",
        "Ouvi dizer que o grande Torneio da\nEspada trara guerreiros de toda parte!",
        "Se encontrar um pedaco azul de Kinstone,\npressione [L] para unirmos nossa sorte!"
    };
    dialogue_show(SPEAKER_TOWN_CITIZEN, "Cidada", citizen_speech, 3);
}

void dialogue_trigger_town_guard_talk(void) {
    static const char* guard_speech[] = {
        "Alto la! Este e o Portao Real que da\nacesso ao Castelo de Hyrule!",
        "Sua Majestade, o Rei Daltus, ordenou\nvigilancia redobrada nas muralhas.",
        "Mantenha sua espada afiada e que as\nDeusas de Hyrule iluminem seu caminho!"
    };
    dialogue_show(SPEAKER_TOWN_GUARD, "Guarda Real", guard_speech, 3);
}

void dialogue_trigger_gentari_talk(void) {
    hal_audio_play_sound(SOUND_SECRET, 0.85f, 1.25f);
    static const char* gentari_speech[] = {
        "Bem-vindo a Vila dos Minish,\njovem heroi da superficie!",
        "A lendaria Espada Picori foi\npartida pelo maligno feiticeiro Vaati...",
        "Para restaura-la ao seu poder divino,\nvoce devera reunir os Quatro Elementos!",
        "O primeiro, o Elemento da Terra,\nrepousa no Santuario Deepwood.",
        "Va falar com Festari na ermida a\nnoroeste para que ele abra o caminho!"
    };
    dialogue_show(SPEAKER_GENTARI, "Anciao Gentari", gentari_speech, 5);
}

void dialogue_trigger_festari_talk(void) {
    hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.90f, 1.10f);
    static const char* festari_speech[] = {
        "Saudacoes, valoroso heroi de verde.\nEu sou Festari, guardiao do templo.",
        "O caminho para o Deepwood Shrine\nesta liberado para a sua nobre jornada!",
        "Va com fe e empunhe sua espada...\nO sagrado Elemento da Terra o aguarda!"
    };
    dialogue_show(SPEAKER_FESTARI, "Festari", festari_speech, 3);
}

void dialogue_trigger_village_minish_talk(void) {
    hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.85f, 1.20f);
    static const char* villager_speech[] = {
        "Ola! E tao bom ver um humano do\nnosso tamanho passeando pela vila!",
        "Nossas casas de cogumelo e bolota\nsao muito aconchegantes e acolhedoras.",
        "Se encontrar fragmentos de Kinstone,\nvenha unir a sua sorte com a nossa!"
    };
    dialogue_show(SPEAKER_VILLAGE_MINISH, "Picori", villager_speech, 3);
}

void dialogue_trigger_malon_talk(void) {
    hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.85f, 1.25f);
    static const char* malon_speech[] = {
        "Ola viajante! Sou a Malon da Fazenda\nLon Lon. Que dia lindo nos campos!",
        "Meu pai Talon perdeu a chave do nosso\nportao quando foi para a cidade...",
        "Eu tenho uma Kinstone azul especial!\nSe voce tiver a metade compativel,\ntalvez o portao possa ser aberto!"
    };
    dialogue_show(SPEAKER_MALON, "Malon", malon_speech, 3);
}

void dialogue_trigger_crenel_sign_talk(void) {
    hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.85f, 0.90f);
    static const char* sign_speech[] = {
        "<- Monte Crenel e Minas Melari a Oeste.\n^ Castelo Real de Hyrule ao Norte.\nv Cidade de Hyrule ao Sul."
    };
    dialogue_show(SPEAKER_SIGNPOST, "Placa de Estrada", sign_speech, 1);
}

void dialogue_trigger_business_scrub_talk(int link_rupees, bool has_grip_ring) {
    hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.85f, 1.10f);
    if (has_grip_ring) {
        static const char* bought_speech[] = {
            "Aproveite o Grip Ring!\nEle escala paredoes.",
            "Volte sempre para mais\nnegocios em Hyrule!"
        };
        dialogue_show(SPEAKER_BUSINESS_SCRUB, "Business Scrub", bought_speech, 2);
    } else if (link_rupees >= 40) {
        static const char* sell_speech[] = {
            "Psst! Bravo viajante!\nTrago itens raros!",
            "Vendo o Grip Ring por\napenas 40 Rupees!",
            "Aperte [A] para fechar\no melhor negocio!"
        };
        dialogue_show(SPEAKER_BUSINESS_SCRUB, "Business Scrub", sell_speech, 3);
    } else {
        static const char* poor_speech[] = {
            "O Grip Ring custa\napenas 40 Rupees!",
            "Junte moedas e volte\npara negociar comigo!"
        };
        dialogue_show(SPEAKER_BUSINESS_SCRUB, "Business Scrub", poor_speech, 2);
    }
}

void dialogue_trigger_melari_talk(bool has_white_sword) {
    if (!has_white_sword) {
        s_melari_reward_pending = true;
        static const char* melari_forge[] = {
            "Ora, veja so quem temos aqui!\nUm jovem rapaz em busca do Mestre\nFerreiro Melari do Monte Crenel!",
            "O que e isso em suas maos?!\nEsta lamina despedacada... e a\nsagrada Lamina Picori forjada pelos\nnossos ancestrais ha seculos!",
            "Rapazes! Apaguem as conversas e\nacendam o fole da forja com toda\na forca! Temos um trabalho lendario!",
            "*CLANG! CLANG! CLANG!*\nO fogo e o martelo unem o aco sagrado\nem uma lamina branca reluzente!",
            "Pronto! Contemple a ESPADA BRANCA\n(White Sword)! Ela corta com o dobro\nde poder de uma lamina comum!",
            "Mas preste atencao: para restaurar\nseu poder divino total, voce deve\ninfundi-la com os Quatro Elementos!",
            "Passe pela porta ao norte para entrar\nna Caverna das Chamas e encontrar o\nsegundo elemento: o Elemento Fogo!"
        };
        dialogue_show(SPEAKER_MELARI, "Mestre Melari", melari_forge, 7);
    } else {
        static const char* melari_guidance[] = {
            "A Espada Branca e um trabalho-prima!\nCuide bem dela, meu jovem!",
            "Siga pela passagem ao norte e entre\nna Caverna das Chamas. O Elemento\nFogo aguarda no coracao da montanha!"
        };
        dialogue_show(SPEAKER_MELARI, "Mestre Melari", melari_guidance, 2);
    }
}

void dialogue_trigger_mountain_minish_talk(void) {
    static const char* minish_miner_speech[] = {
        "Trabalhamos dia e noite nestas minas!\nO magma ardente do Monte Crenel mantem\nnossa forja sempre aquecida!",
        "Tenha muito cuidado ao caminhar perto\ndos canais de lava! Um passo em falso\ne o calor vai chamuscar suas botas!",
        "Nosso Mestre Melari e o maior ferreiro\nde todo o continente! Nao ha aco ou\nminerio que resista ao seu martelo!"
    };
    dialogue_show(SPEAKER_MOUNTAIN_MINISH, "Minerador Minish", minish_miner_speech, 3);
}

bool dialogue_is_melari_reward_pending(void) {
    return s_melari_reward_pending;
}

void dialogue_clear_melari_reward(void) {
    s_melari_reward_pending = false;
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
        u32 name_col = (s_speaker == SPEAKER_EZLO) ? 0xFFE27AFF :
                       (s_speaker == SPEAKER_SWIFTBLADE) ? 0xFF6B6BFF :
                       (s_speaker == SPEAKER_SHOPKEEPER) ? 0xFDE047FF :
                       (s_speaker == SPEAKER_TOWN_CITIZEN) ? 0xF472B6FF :
                       (s_speaker == SPEAKER_TOWN_GUARD) ? 0x60A5FAFF :
                       (s_speaker == SPEAKER_GENTARI) ? 0xFDE047FF :
                       (s_speaker == SPEAKER_FESTARI) ? 0x93C5FDFF :
                       (s_speaker == SPEAKER_VILLAGE_MINISH) ? 0x34D399FF :
                       (s_speaker == SPEAKER_MALON) ? 0xFB923CFF :
                       (s_speaker == SPEAKER_BUSINESS_SCRUB) ? 0xF59E0BFF :
                       (s_speaker == SPEAKER_MELARI) ? 0xF97316FF :
                       (s_speaker == SPEAKER_MOUNTAIN_MINISH) ? 0xFDE047FF :
                       (s_speaker == SPEAKER_SMITH) ? 0xF87171FF :
                       (s_speaker == SPEAKER_MAYOR_HAGEN) ? 0x60A5FAFF : 0x77FF99FF;
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
    } else if (s_speaker == SPEAKER_SWIFTBLADE) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_swiftblade(port_x, port_y, is_talking);
    } else if (s_speaker == SPEAKER_SHOPKEEPER) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_shopkeeper(port_x, port_y, is_talking);
    } else if (s_speaker == SPEAKER_TOWN_CITIZEN) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_town_citizen(port_x, port_y, is_talking);
    } else if (s_speaker == SPEAKER_TOWN_GUARD) {
        render_portrait_town_guard(port_x, port_y);
    } else if (s_speaker == SPEAKER_GENTARI) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_gentari(port_x, port_y, is_talking);
    } else if (s_speaker == SPEAKER_FESTARI) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_festari(port_x, port_y, is_talking);
    } else if (s_speaker == SPEAKER_VILLAGE_MINISH) {
        render_portrait_village_minish(port_x, port_y);
    } else if (s_speaker == SPEAKER_MALON) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_malon(port_x, port_y, is_talking);
    } else if (s_speaker == SPEAKER_BUSINESS_SCRUB) {
        render_portrait_business_scrub(port_x, port_y);
    } else if (s_speaker == SPEAKER_MELARI) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_melari(port_x, port_y, is_talking);
    } else if (s_speaker == SPEAKER_MOUNTAIN_MINISH) {
        render_portrait_mountain_minish(port_x, port_y);
    } else if (s_speaker == SPEAKER_SMITH) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_smith(port_x, port_y, is_talking);
    } else if (s_speaker == SPEAKER_MAYOR_HAGEN) {
        bool is_talking = (s_state == DIALOGUE_STATE_TYPING);
        render_portrait_mayor_hagen(port_x, port_y, is_talking);
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
