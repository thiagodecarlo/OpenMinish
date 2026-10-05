/*
 * ============================================================================
 * src/hal/sword_dojo.c - Dojos de Esgrima & 8 Pergaminhos do Tigre (Ato VI)
 * ============================================================================
 * Implementação clean-room C11 do sistema de técnicas marciais e mestres de Hyrule.
 */

#include "hal/sword_dojo.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include <stdio.h>
#include <string.h>

#define SCREEN_W 256
#define SCREEN_H 160

static const TigerScrollInfo s_scroll_catalog[TOTAL_TIGER_SCROLLS] = {
    {
        SCROLL_SPIN_ATTACK,
        "Ataque Giratorio",
        "Mestre Swiftblade",
        "Hyrule Town",
        "Segure o botao de ataque para carregar energia,",
        "solte para desferir um corte devastador de 360°!",
        0xDC2626FF // Fita Escarlate
    },
    {
        SCROLL_ROLL_ATTACK,
        "Estocada Rolando",
        "Mestre Grayblade",
        "Monte Crenel",
        "Pressione o botao de espada logo apos rolar para",
        "desferir um estocada fulminante com alcance duplo!",
        0x64748BFF // Fita de Aco Cinzento
    },
    {
        SCROLL_DASH_ATTACK,
        "Investida com Espada",
        "Mestre Swiftblade",
        "Hyrule Town",
        "Corra com as Botas de Pegaso mantendo a lamina empunhada;",
        "empala inimigos e destroi obstaculos no caminho!",
        0x2563EBFF // Fita Azul Cobalto
    },
    {
        SCROLL_ROCK_BREAKER,
        "Quebra-Pedras",
        "Mestre Swiftblade",
        "Hyrule Town",
        "Canaliza forca no fio da lamina; permite cortar rochas,",
        "potes e blocos sem precisar recorrer a bombas!",
        0xD97706FF // Fita Ambar
    },
    {
        SCROLL_SWORD_BEAM,
        "Raio de Espada Sagrado",
        "Mestre Grimblade",
        "Jardim do Castelo",
        "Quando sua energia vital (HP) estiver totalmente cheia,",
        "cada golpe dispara uma lamina de luz cortante a distancia!",
        0xFBBF24FF // Fita Dourada Celestial
    },
    {
        SCROLL_DOWN_THRUST,
        "Estocada Aerea",
        "Mestre Waveblade",
        "Lago Hylia",
        "Ao saltar com a Capa de Roc ou pelo ar, golpeie para baixo;",
        "cai como um meteoro quebrando escudos e armaduras!",
        0x06B6D4FF // Fita Ciano Aquatico
    },
    {
        SCROLL_PERIL_BEAM,
        "Raio de Desespero",
        "Mestre Splitblade",
        "Quedas do Veu",
        "Quando Link estiver a beira da derrota (1 coracao ou menos),",
        "a espada pulsa com furor e dispara raios vermelhos de perigo!",
        0x991B1BFF // Fita Carmesim
    },
    {
        SCROLL_GREAT_SPIN,
        "Grande Furacao",
        "Mestre Swiftblade I",
        "Castor Wilds",
        "Durante o Spin Attack, tecle ataque continuamente para expandir",
        "o giro em um colossal tufao movel de 5 voltas continuas!",
        0x059669FF // Fita Esmeralda Nobre
    }
};

static u8  s_unlocked_mask = 0;
static int s_banner_timer = 0;
static TigerScrollId s_active_banner_scroll = SCROLL_SPIN_ATTACK;

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

static void draw_filled_rect(int x, int y, int w, int h, u32 color) {
    int x1 = (x < 0) ? 0 : x;
    int y1 = (y < 0) ? 0 : y;
    int x2 = (x + w > SCREEN_W) ? SCREEN_W : (x + w);
    int y2 = (y + h > SCREEN_H) ? SCREEN_H : (y + h);

    for (int py = y1; py < y2; py++) {
        for (int px = x1; px < x2; px++) {
            u32 current = hal_video_get_pixel(px, py);
            hal_video_put_pixel(px, py, blend_colors(current, color));
        }
    }
}

void sword_dojo_init(void) {
    s_unlocked_mask = (1 << SCROLL_SPIN_ATTACK); // Spin Attack ja inicia desbloqueado
    s_banner_timer = 0;
    s_active_banner_scroll = SCROLL_SPIN_ATTACK;
}

void sword_dojo_unlock_scroll(TigerScrollId scroll) {
    if (scroll < 0 || scroll >= TOTAL_TIGER_SCROLLS) return;

    if (!(s_unlocked_mask & (1 << scroll))) {
        s_unlocked_mask |= (1 << scroll);
        sword_dojo_trigger_banner(scroll);
        hal_audio_play_sound(SOUND_TIGER_SCROLL, 1.0f, 1.0f);
        printf("[DOJO] PERGAMINHO DO TIGRE DESBLOQUEADO: '%s' ensinado por %s!\n",
               s_scroll_catalog[scroll].name, s_scroll_catalog[scroll].master_name);
    }
}

bool sword_dojo_has_scroll(TigerScrollId scroll) {
    if (scroll < 0 || scroll >= TOTAL_TIGER_SCROLLS) return false;
    return (s_unlocked_mask & (1 << scroll)) != 0;
}

u8 sword_dojo_get_unlocked_mask(void) {
    return s_unlocked_mask;
}

int sword_dojo_get_unlocked_count(void) {
    int count = 0;
    for (int i = 0; i < TOTAL_TIGER_SCROLLS; i++) {
        if (s_unlocked_mask & (1 << i)) count++;
    }
    return count;
}

const TigerScrollInfo* sword_dojo_get_info(TigerScrollId scroll) {
    if (scroll < 0 || scroll >= TOTAL_TIGER_SCROLLS) return NULL;
    return &s_scroll_catalog[scroll];
}

bool sword_dojo_can_fire_beam(int hearts, int max_hearts, bool* out_is_peril) {
    if (sword_dojo_has_scroll(SCROLL_SWORD_BEAM) && hearts >= max_hearts) {
        if (out_is_peril) *out_is_peril = false;
        return true;
    }
    if (sword_dojo_has_scroll(SCROLL_PERIL_BEAM) && hearts <= 4) {
        if (out_is_peril) *out_is_peril = true;
        return true;
    }
    return false;
}

bool sword_dojo_can_break_rocks(void) {
    return sword_dojo_has_scroll(SCROLL_ROCK_BREAKER);
}

bool sword_dojo_can_roll_attack(void) {
    return sword_dojo_has_scroll(SCROLL_ROLL_ATTACK);
}

bool sword_dojo_can_dash_attack(void) {
    return sword_dojo_has_scroll(SCROLL_DASH_ATTACK);
}

bool sword_dojo_can_down_thrust(void) {
    return sword_dojo_has_scroll(SCROLL_DOWN_THRUST);
}

bool sword_dojo_can_great_spin(void) {
    return sword_dojo_has_scroll(SCROLL_GREAT_SPIN);
}

void sword_dojo_trigger_banner(TigerScrollId scroll) {
    s_active_banner_scroll = scroll;
    s_banner_timer = 240; // 4 segundos de duracao
}

void sword_dojo_update(void) {
    if (s_banner_timer > 0) {
        s_banner_timer--;
    }
}

void sword_dojo_render_banner(void) {
    if (s_banner_timer <= 0) return;

    const TigerScrollInfo* info = &s_scroll_catalog[s_active_banner_scroll];

    int ban_w = 236;
    int ban_h = 36;
    int ban_x = (SCREEN_W - ban_w) / 2;
    int ban_y = 60;

    // Moldura Dourada do Pergaminho
    draw_filled_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x0A0806EE);
    draw_filled_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0xD4AF37FF);
    draw_filled_rect(ban_x, ban_y, ban_w, ban_h, 0x1A120EFF);
    draw_filled_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x2A1A12EE);

    // Ícone do Pergaminho com fita colorida de acordo com o mestre
    draw_filled_rect(ban_x + 8, ban_y + 8, 12, 20, 0xFEF08AFF);
    draw_filled_rect(ban_x + 6, ban_y + 6, 16, 3, 0xD97706FF);
    draw_filled_rect(ban_x + 6, ban_y + 27, 16, 3, 0xD97706FF);
    draw_filled_rect(ban_x + 8, ban_y + 15, 12, 6, info->ribbon_color); // Fita específica

    char title_buf[48];
    snprintf(title_buf, sizeof(title_buf), "PERGAMINHO DO TIGRE Nº %d!", (int)info->id + 1);
    font_draw_text(ban_x + 28, ban_y + 6, title_buf, 0xFDE047FF, true);

    char desc_buf[64];
    snprintf(desc_buf, sizeof(desc_buf), "%s (%s)", info->name, info->master_name);
    font_draw_text(ban_x + 28, ban_y + 18, desc_buf, 0x38BDF8FF, true);
}

void sword_dojo_export(u8* out_mask) {
    if (out_mask) *out_mask = s_unlocked_mask;
}

void sword_dojo_restore(u8 mask) {
    s_unlocked_mask = mask ? mask : (1 << SCROLL_SPIN_ATTACK);
    printf("[DOJO] Estado restaurado: 0x%02X (%d/8 Pergaminhos do Tigre aprendidos)\n",
           s_unlocked_mask, sword_dojo_get_unlocked_count());
}
