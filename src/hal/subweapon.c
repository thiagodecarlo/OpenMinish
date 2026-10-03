/*
 * ============================================================================
 * src/hal/subweapon.c - Implementação do Sistema de Armas Secundárias e Itens
 * ============================================================================
 */

#include "hal/subweapon.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/entity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

static SubweaponType s_current_item = ITEM_BOOMERANG;

// Estrutura do Bumerangue Mágico
typedef struct {
    bool  is_active;
    float x;
    float y;
    float vx;
    float vy;
    int   flight_timer;
    bool  is_returning;
    int   spin_angle;
    int   carried_item; // 0=Nenhum, 1=Rupee, 2=Coração
} BoomerangState;

static BoomerangState s_boomerang = { 0 };

// Estrutura do Pote Mágico (Gust Jar)
typedef struct {
    bool      is_sucking;
    Direction suck_dir;
    float     jar_x;
    float     jar_y;
    int       suck_timer;
    bool      has_charge;
    int       wind_anim;

    // Disparo pressurizado de ar
    bool      blast_active;
    float     blast_x;
    float     blast_y;
    float     blast_vx;
    float     blast_vy;
    int       blast_timer;
} GustJarState;

static GustJarState s_gust = { 0 };

void subweapon_init(void) {
    s_current_item = ITEM_BOOMERANG;
    memset(&s_boomerang, 0, sizeof(s_boomerang));
    memset(&s_gust, 0, sizeof(s_gust));
}

void subweapon_cycle(void) {
    s_current_item = (s_current_item + 1) % ITEM_COUNT;
    hal_audio_play_sound(SOUND_SECRET, 0.70f, 1.6f);
    printf("[SUBWEAPON] Item equipado: %s\n", subweapon_get_name(s_current_item));
}

SubweaponType subweapon_get_current(void) {
    return s_current_item;
}

const char* subweapon_get_name(SubweaponType item) {
    switch (item) {
        case ITEM_BOOMERANG:     return "Bumerangue Magico";
        case ITEM_GUST_JAR:      return "Pote Magico (Gust Jar)";
        case ITEM_PEGASUS_BOOTS: return "Botas de Pegasus (Dash)";
        default:                 return "Nenhum";
    }
}

bool subweapon_is_gust_active(void) {
    return (s_current_item == ITEM_GUST_JAR && s_gust.is_sucking);
}

void subweapon_use_pressed(float link_x, float link_y, Direction dir) {
    if (s_current_item == ITEM_BOOMERANG) {
        if (!s_boomerang.is_active) {
            s_boomerang.is_active = true;
            s_boomerang.x = link_x + 6.0f;
            s_boomerang.y = link_y + 8.0f;
            s_boomerang.flight_timer = 0;
            s_boomerang.is_returning = false;
            s_boomerang.spin_angle = 0;
            s_boomerang.carried_item = 0;

            float speed = 3.6f;
            s_boomerang.vx = 0.0f;
            s_boomerang.vy = 0.0f;
            if (dir == DIR_DOWN)  s_boomerang.vy = speed;
            if (dir == DIR_UP)    s_boomerang.vy = -speed;
            if (dir == DIR_LEFT)  s_boomerang.vx = -speed;
            if (dir == DIR_RIGHT) s_boomerang.vx = speed;

            hal_audio_play_sound(SOUND_BOOMERANG_FLY, 0.90f, 1.0f);
        }
    } else if (s_current_item == ITEM_GUST_JAR) {
        s_gust.is_sucking = true;
        s_gust.suck_dir = dir;
        s_gust.suck_timer = 0;
        s_gust.jar_x = link_x + 8.0f;
        s_gust.jar_y = link_y + 8.0f;
        hal_audio_play_sound(SOUND_GUST_SUCTION, 0.85f, 1.0f);
    }
}

void subweapon_use_held(float link_x, float link_y, Direction dir) {
    if (s_current_item == ITEM_GUST_JAR) {
        s_gust.is_sucking = true;
        s_gust.suck_dir = dir;

        // Posiciona a boca do jarro à frente do herói
        float off_x = 0.0f;
        float off_y = 0.0f;
        if (dir == DIR_DOWN)  { off_x = 0.0f;  off_y = 12.0f; }
        if (dir == DIR_UP)    { off_x = 0.0f;  off_y = -8.0f; }
        if (dir == DIR_LEFT)  { off_x = -10.0f; off_y = 4.0f;  }
        if (dir == DIR_RIGHT) { off_x = 10.0f;  off_y = 4.0f;  }

        s_gust.jar_x = link_x + 8.0f + off_x;
        s_gust.jar_y = link_y + 8.0f + off_y;
    }
}

void subweapon_use_released(float link_x, float link_y, Direction dir) {
    if (s_current_item == ITEM_GUST_JAR) {
        if (s_gust.is_sucking) {
            s_gust.is_sucking = false;

            // Se sugou ar suficiente ou absorveu uma pedra, dispara a rajada de ar!
            if (s_gust.has_charge || s_gust.suck_timer >= 20) {
                s_gust.blast_active = true;
                s_gust.blast_x = s_gust.jar_x;
                s_gust.blast_y = s_gust.jar_y;
                s_gust.blast_timer = 22;
                s_gust.has_charge = false;

                float b_spd = 4.6f;
                s_gust.blast_vx = 0.0f;
                s_gust.blast_vy = 0.0f;
                if (dir == DIR_DOWN)  s_gust.blast_vy = b_spd;
                if (dir == DIR_UP)    s_gust.blast_vy = -b_spd;
                if (dir == DIR_LEFT)  s_gust.blast_vx = -b_spd;
                if (dir == DIR_RIGHT) s_gust.blast_vx = b_spd;

                hal_audio_play_sound(SOUND_GUST_BLAST, 0.95f, 1.0f);
            }
            s_gust.suck_timer = 0;
        }
    }
}

void subweapon_update(Tilemap* map, float link_x, float link_y, int* link_rupees, int* link_hearts) {
    // ------------------------------------------------------------------------
    // 1. ATUALIZAÇÃO DO BUMERANGUE MÁGICO
    // ------------------------------------------------------------------------
    if (s_boomerang.is_active) {
        s_boomerang.spin_angle = (s_boomerang.spin_angle + 1) % 4;

        if (!s_boomerang.is_returning) {
            // Fase de Ida: arremesso reto com desaceleração gradual
            s_boomerang.x += s_boomerang.vx;
            s_boomerang.y += s_boomerang.vy;
            s_boomerang.vx *= 0.94f;
            s_boomerang.vy *= 0.94f;
            s_boomerang.flight_timer++;

            // Corta arbustos no caminho
            if (map) {
                map_interact_slash(map, s_boomerang.x + 4.0f, s_boomerang.y + 4.0f);
                if (map_is_solid(map, s_boomerang.x + 4.0f, s_boomerang.y + 4.0f)) {
                    s_boomerang.is_returning = true;
                    hal_audio_play_sound(SOUND_SWORD_HIT, 0.8f, 1.5f); // Bate na parede e volta
                }
            }

            // Atinge o alcance máximo (~24 frames) e inicia retorno
            if (s_boomerang.flight_timer >= 24) {
                s_boomerang.is_returning = true;
            }
        } else {
            // Fase de Retorno Teleguiado: curva acelerada na direção atual do Link
            float target_x = link_x + 6.0f;
            float target_y = link_y + 8.0f;
            float dx = target_x - s_boomerang.x;
            float dy = target_y - s_boomerang.y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist <= 10.0f) {
                // Capturado pelo Link!
                s_boomerang.is_active = false;
                hal_audio_play_sound(SOUND_ITEM_CATCH, 0.85f, 1.0f);

                // Deposita o item carregado no inventário do Link
                if (s_boomerang.carried_item == 1 && link_rupees) {
                    *link_rupees += 5;
                } else if (s_boomerang.carried_item == 2 && link_hearts) {
                    if (*link_hearts < 3) (*link_hearts)++;
                }
                s_boomerang.carried_item = 0;
            } else {
                float ret_speed = 3.8f;
                s_boomerang.vx = (dx / dist) * ret_speed;
                s_boomerang.vy = (dy / dist) * ret_speed;
                s_boomerang.x += s_boomerang.vx;
                s_boomerang.y += s_boomerang.vy;
            }
        }

        // Colisão do Bumerangue com Inimigos e Itens
        int carried = 0;
        if (entity_check_subweapon_hit(s_boomerang.x, s_boomerang.y, 8.0f, 8.0f, 1, &carried)) {
            if (carried > 0 && s_boomerang.carried_item == 0) {
                s_boomerang.carried_item = carried;
            }
            s_boomerang.is_returning = true; // Inicia retorno imediato ao acertar
        }
    }

    // ------------------------------------------------------------------------
    // 2. ATUALIZAÇÃO DO POTE MÁGICO (GUST JAR)
    // ------------------------------------------------------------------------
    if (s_gust.is_sucking) {
        s_gust.suck_timer++;
        s_gust.wind_anim++;

        // Som contínuo de sucção de vento a cada 11 frames
        if (s_gust.suck_timer % 11 == 0) {
            hal_audio_play_sound(SOUND_GUST_SUCTION, 0.70f, 1.0f);
        }

        // Aplica o vórtice de sucção gravitacional nas entidades
        if (entity_apply_gust_suction(s_gust.jar_x, s_gust.jar_y, s_gust.suck_dir, 60.0f, 1.8f)) {
            s_gust.has_charge = true; // Carregou tiro com objeto absorvido!
        }
    }

    // Disparo de Rajada de Ar pressurizada
    if (s_gust.blast_active) {
        s_gust.blast_x += s_gust.blast_vx;
        s_gust.blast_y += s_gust.blast_vy;
        s_gust.blast_timer--;

        // Corta arbustos com a rajada de vento
        if (map) {
            map_interact_slash(map, s_gust.blast_x + 4.0f, s_gust.blast_y + 4.0f);
        }

        // Fere e arremessa inimigos atingidos pela rajada
        int dummy = 0;
        if (entity_check_subweapon_hit(s_gust.blast_x, s_gust.blast_y, 10.0f, 10.0f, 2, &dummy)) {
            s_gust.blast_active = false;
        }

        if (s_gust.blast_timer <= 0) {
            s_gust.blast_active = false;
        }
    }
}

void subweapon_render(const Camera* cam) {
    if (!cam) return;

    // 1. Renderiza o Bumerangue em voo rotativo
    if (s_boomerang.is_active) {
        int bx, by;
        map_world_to_screen(cam, s_boomerang.x, s_boomerang.y, &bx, &by);

        u32 c_gold  = 0xE5C158FF; // Lâmina de ouro
        u32 c_steel = 0xF0F4F8FF; // Aço brilhante
        u32 c_gem   = 0xE63946FF; // Gema central vermelha

        // Sprites de rotação do Bumerangue (4 ângulos)
        int ang = s_boomerang.spin_angle;
        if (ang == 0) {
            // Em cruz (+)
            for (int y = 0; y < 8; y++) hal_video_put_pixel(bx + 3, by + y, c_gold);
            for (int x = 0; x < 8; x++) hal_video_put_pixel(bx + x, by + 3, c_steel);
            hal_video_put_pixel(bx + 3, by + 3, c_gem);
        } else if (ang == 1) {
            // Diagonal (x)
            for (int i = 0; i < 7; i++) {
                hal_video_put_pixel(bx + i, by + i, c_gold);
                hal_video_put_pixel(bx + 6 - i, by + i, c_steel);
            }
            hal_video_put_pixel(bx + 3, by + 3, c_gem);
        } else if (ang == 2) {
            for (int y = 0; y < 8; y++) hal_video_put_pixel(bx + 4, by + y, c_steel);
            for (int x = 0; x < 8; x++) hal_video_put_pixel(bx + x, by + 4, c_gold);
            hal_video_put_pixel(bx + 4, by + 4, c_gem);
        } else {
            for (int i = 0; i < 7; i++) {
                hal_video_put_pixel(bx + i + 1, by + i, c_steel);
                hal_video_put_pixel(bx + 6 - i, by + i + 1, c_gold);
            }
            hal_video_put_pixel(bx + 3, by + 3, c_gem);
        }

        // Renderiza o item capturado pelo bumerangue
        if (s_boomerang.carried_item == 1) {
            // Mini Rupee verde
            for (int y = 0; y < 5; y++) {
                for (int x = 0; x < 3; x++) hal_video_put_pixel(bx + x + 2, by + y + 6, 0x00FF88FF);
            }
        } else if (s_boomerang.carried_item == 2) {
            // Mini Coração
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 5; x++) hal_video_put_pixel(bx + x + 1, by + y + 6, 0xE62222FF);
            }
        }
    }

    // 2. Renderiza o Vórtice de Vento do Pote Mágico
    if (s_gust.is_sucking) {
        int jx, jy;
        map_world_to_screen(cam, s_gust.jar_x, s_gust.jar_y, &jx, &jy);

        // Renderiza o corpo do Pote Mágico segurado pelo herói
        u32 c_clay = 0xB85C38FF; // Cerâmica terracota
        u32 c_nozzle = 0xE5C158FF; // Bocal dourado
        for (int y = -2; y <= 2; y++) {
            for (int x = -2; x <= 2; x++) hal_video_put_pixel(jx + x, jy + y, c_clay);
        }
        hal_video_put_pixel(jx, jy, c_nozzle);

        // Linhas de vórtice espirais sugadas em direção ao bocal
        for (int p = 0; p < 8; p++) {
            float t = ((float)((s_gust.wind_anim * 3 + p * 20) % 60)) / 60.0f;
            float dist = 45.0f * (1.0f - t);
            float spread = sinf(t * PI_F * 2.0f + p) * 14.0f * (1.0f - t);

            int px = jx;
            int py = jy;
            if (s_gust.suck_dir == DIR_DOWN)  { px += (int)spread; py += (int)dist; }
            if (s_gust.suck_dir == DIR_UP)    { px += (int)spread; py -= (int)dist; }
            if (s_gust.suck_dir == DIR_RIGHT) { px += (int)dist;   py += (int)spread; }
            if (s_gust.suck_dir == DIR_LEFT)  { px -= (int)dist;   py += (int)spread; }

            u32 wind_col = (p % 2 == 0) ? 0xE0F7FAFF : 0x80DEEAFF;
            hal_video_put_pixel(px, py, wind_col);
            hal_video_put_pixel(px + 1, py, wind_col);
        }
    }

    // 3. Renderiza a Rajada de Ar pressurizada
    if (s_gust.blast_active) {
        int ax, ay;
        map_world_to_screen(cam, s_gust.blast_x, s_gust.blast_y, &ax, &ay);

        u32 c_puff_wh = 0xFFFFFFFF;
        u32 c_puff_cy = 0x80DEEAFF;

        for (int y = -3; y <= 3; y++) {
            for (int x = -3; x <= 3; x++) {
                if (x * x + y * y <= 9) {
                    u32 col = (x * x + y * y <= 3) ? c_puff_wh : c_puff_cy;
                    hal_video_put_pixel(ax + x, ay + y, col);
                }
            }
        }
    }
}

void subweapon_render_hud_icon(int x, int y) {
    // Moldura do Slot de Item Secundário [B]
    // Fundo esmeralda e borda dourada
    for (int dy = 0; dy < 11; dy++) {
        for (int dx = 0; dx < 14; dx++) {
            u32 col = 0x0E2614FF;
            if (dy == 0 || dy == 10 || dx == 0 || dx == 13) col = 0xD4AF37FF;
            hal_video_put_pixel(x + dx, y + dy, col);
        }
    }

    // Letra B indicadora
    font_draw_char(x + 2, y + 2, 'B', 0xFFE27AFF, false);

    // Ícone do item no slot
    if (s_current_item == ITEM_BOOMERANG) {
        // Mini Bumerangue dourado
        hal_video_put_pixel(x + 8, y + 3, 0xFFE27AFF);
        hal_video_put_pixel(x + 9, y + 4, 0xFFE27AFF);
        hal_video_put_pixel(x + 10, y + 5, 0xE63946FF); // Gema vermelha
        hal_video_put_pixel(x + 9, y + 6, 0xFFE27AFF);
        hal_video_put_pixel(x + 8, y + 7, 0xFFE27AFF);
    } else if (s_current_item == ITEM_GUST_JAR) {
        // Mini Pote Mágico terracota com bocal
        for (int dy = 4; dy <= 7; dy++) {
            for (int dx = 8; dx <= 11; dx++) hal_video_put_pixel(x + dx, y + dy, 0xB85C38FF);
        }
        hal_video_put_pixel(x + 8, y + 3, 0xD4AF37FF); // Bocal dourado
        hal_video_put_pixel(x + 9, y + 3, 0xD4AF37FF);
    } else if (s_current_item == ITEM_PEGASUS_BOOTS) {
        // Mini Bota de Pegasus vermelha com asa branca
        for (int dy = 5; dy <= 7; dy++) {
            for (int dx = 7; dx <= 11; dx++) hal_video_put_pixel(x + dx, y + dy, 0xE63946FF);
        }
        hal_video_put_pixel(x + 8, y + 3, 0xFFFFFFFF); // Asinha branca
        hal_video_put_pixel(x + 9, y + 4, 0xFFFFFFFF);
    }
}
