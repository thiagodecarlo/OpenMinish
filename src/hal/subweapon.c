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

#define MAX_ACTIVE_BOMBS 4

typedef struct {
    bool  is_active;
    float x;
    float y;
    int   fuse_timer;       // 110 frames (1.8s)
    bool  is_exploding;
    int   explosion_timer;  // 18 frames
    float radius;
} ActiveBomb;

static ActiveBomb s_bombs[MAX_ACTIVE_BOMBS] = { 0 };
static int s_bomb_count = 10;
static int s_max_bombs = 10;
static int s_bomb_screen_shake = 0;

void subweapon_init(void) {
    s_current_item = ITEM_BOOMERANG;
    memset(&s_boomerang, 0, sizeof(s_boomerang));
    memset(&s_gust, 0, sizeof(s_gust));
    memset(s_bombs, 0, sizeof(s_bombs));
    s_bomb_count = 10;
    s_max_bombs = 10;
    s_bomb_screen_shake = 0;
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
        case ITEM_BOMBS:         return "Bolsa de Bombas (Bombs)";
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
    } else if (s_current_item == ITEM_BOMBS) {
        if (s_bomb_count > 0) {
            for (int i = 0; i < MAX_ACTIVE_BOMBS; i++) {
                if (!s_bombs[i].is_active) {
                    ActiveBomb* b = &s_bombs[i];
                    float bx = link_x + 4.0f;
                    float by = link_y + 8.0f;
                    if (dir == DIR_DOWN)  by += 12.0f;
                    if (dir == DIR_UP)    by -= 10.0f;
                    if (dir == DIR_LEFT)  bx -= 12.0f;
                    if (dir == DIR_RIGHT) bx += 12.0f;

                    b->is_active = true;
                    b->x = bx;
                    b->y = by;
                    b->fuse_timer = 110;
                    b->is_exploding = false;
                    b->explosion_timer = 0;
                    b->radius = 32.0f;
                    s_bomb_count--;

                    hal_audio_play_sound(SOUND_BOMB_FUSE, 0.85f, 1.0f);
                    printf("[BOMB] Bomba armada em (%.1f, %.1f)! Pavio aceso, estoque: %d\n", bx, by, s_bomb_count);
                    break;
                }
            }
        } else {
            hal_audio_play_sound(SOUND_SWITCH_CLICK, 0.7f, 0.8f);
            printf("[BOMB] Bolsa de bombas vazia!\n");
        }
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

    // ------------------------------------------------------------------------
    // 3. ATUALIZAÇÃO DAS BOMBAS (BOMB FUSE, EXPLOSION & SCREEN SHAKE)
    // ------------------------------------------------------------------------
    if (s_bomb_screen_shake > 0) s_bomb_screen_shake--;

    for (int i = 0; i < MAX_ACTIVE_BOMBS; i++) {
        ActiveBomb* b = &s_bombs[i];
        if (!b->is_active) continue;

        if (!b->is_exploding) {
            b->fuse_timer--;

            // Chiado do pavio ardendo a cada 20 frames
            if (b->fuse_timer % 20 == 0) {
                hal_audio_play_sound(SOUND_BOMB_FUSE, 0.70f, 1.15f);
            }

            if (b->fuse_timer <= 0) {
                // DETONAÇÃO EXPLOSIVA!
                b->is_exploding = true;
                b->explosion_timer = 18;
                s_bomb_screen_shake = 8;
                hal_audio_play_sound(SOUND_BOMB_EXPLODE, 1.0f, 1.0f);

                // 1. Dano em área em todos os monstros
                entity_check_bomb_explosion(b->x + 4.0f, b->y + 4.0f, b->radius, 4);

                // 2. Destruição do cenário (paredes rachadas, rochas quebradiças, arbustos)
                if (map) {
                    map_interact_bomb(map, b->x + 4.0f, b->y + 4.0f, b->radius);
                }

                // 3. Autodano se o herói estiver muito perto do epicentro da explosão
                float ldx = (link_x + 8.0f) - (b->x + 4.0f);
                float ldy = (link_y + 12.0f) - (b->y + 4.0f);
                float ldist = sqrtf(ldx * ldx + ldy * ldy);
                if (ldist < 22.0f && link_hearts && *link_hearts > 0) {
                    (*link_hearts)--;
                    hal_audio_play_sound(SOUND_HEART_BEEP, 1.0f, 1.0f);
                    printf("[BOMB SELF-DAMAGE] Link foi pego pela explosao da propria bomba!\n");
                }
            }
        } else {
            b->explosion_timer--;
            if (b->explosion_timer <= 0) {
                b->is_active = false;
                b->is_exploding = false;
            }
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

    // 4. Renderiza as Bombas ativas e Explosões
    for (int i = 0; i < MAX_ACTIVE_BOMBS; i++) {
        ActiveBomb* b = &s_bombs[i];
        if (!b->is_active) continue;

        int bx, by;
        map_world_to_screen(cam, b->x, b->y, &bx, &by);

        if (!b->is_exploding) {
            // Pavio e corpo da bomba esférica
            u32 col_bomb = (b->fuse_timer < 32 && ((b->fuse_timer / 4) % 2 == 0)) ? 0xEF4444FF : 0x1E293BFF;
            u32 col_highlight = 0x64748BFF;
            u32 col_cap = 0xD97706FF;

            // Gargalo metálico
            hal_video_put_pixel(bx + 3, by, col_cap);
            hal_video_put_pixel(bx + 4, by, col_cap);

            // Faísca do pavio queimando
            int spark_oy = (b->fuse_timer % 3) + 1;
            int spark_ox = ((b->fuse_timer % 5) - 2);
            hal_video_put_pixel(bx + 3 + spark_ox, by - spark_oy, 0xFDE047FF);
            hal_video_put_pixel(bx + 4 + spark_ox, by - spark_oy, 0xFFFFFFFF);

            // Corpo da bomba (círculo 8x8)
            for (int dy = 0; dy < 8; dy++) {
                for (int dx = 0; dx < 8; dx++) {
                    if ((dx == 0 || dx == 7) && (dy == 0 || dy == 7)) continue;
                    u32 c = col_bomb;
                    if (dx == 2 && dy == 2) c = col_highlight;
                    hal_video_put_pixel(bx + dx, by + 1 + dy, c);
                }
            }
        } else {
            // DETONAÇÃO EXPLOSIVA COM ONDA DE CHOQUE E LABAREDAS
            int exp_frame = 18 - b->explosion_timer;
            float blast_r = 7.0f + (float)exp_frame * 1.5f;
            int cx = bx + 4;
            int cy = by + 5;

            // Pétalas de labaredas e anel de fumaça (16 raios angulares)
            for (int a = 0; a < 16; a++) {
                float ang = (float)a * (2.0f * PI_F / 16.0f) + (float)exp_frame * 0.15f;
                int px1 = cx + (int)(cosf(ang) * (blast_r * 0.55f));
                int py1 = cy + (int)(sinf(ang) * (blast_r * 0.55f));
                int px2 = cx + (int)(cosf(ang) * blast_r);
                int py2 = cy + (int)(sinf(ang) * blast_r);

                u32 c_inner = (exp_frame < 8) ? 0xFFFFFFFF : 0xFDE047FF;
                u32 c_mid   = (exp_frame < 12) ? 0xEA580CFF : 0xDC2626FF;
                u32 c_smoke = 0x64748BCC;

                hal_video_put_pixel(px1, py1, c_inner);
                hal_video_put_pixel(px1 + 1, py1, c_inner);
                hal_video_put_pixel(px2, py2, (exp_frame > 11) ? c_smoke : c_mid);
                hal_video_put_pixel(px2, py2 - 1, (exp_frame > 11) ? c_smoke : c_mid);
            }

            // Núcleo incandescente da detonação
            int core_r = (int)(blast_r * 0.40f);
            for (int y = -core_r; y <= core_r; y++) {
                for (int x = -core_r; x <= core_r; x++) {
                    if (x * x + y * y <= core_r * core_r) {
                        u32 col = (x * x + y * y <= (core_r / 2) * (core_r / 2)) ? 0xFFFFFFFF : 0xFDE047FF;
                        hal_video_put_pixel(cx + x, cy + y, col);
                    }
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
    } else if (s_current_item == ITEM_BOMBS) {
        // Mini Bomba escura com brilho e pavio
        for (int dy = 4; dy <= 7; dy++) {
            for (int dx = 7; dx <= 10; dx++) {
                if ((dx == 7 || dx == 10) && (dy == 4 || dy == 7)) continue;
                hal_video_put_pixel(x + dx, y + dy, 0x1E293BFF);
            }
        }
        hal_video_put_pixel(x + 8, y + 5, 0x94A3B8FF); // Brilho
        hal_video_put_pixel(x + 8, y + 3, 0xD97706FF); // Gargalo
        hal_video_put_pixel(x + 9, y + 2, 0xFDE047FF); // Faísca do pavio
    }
}

int subweapon_get_bomb_count(void) {
    return s_bomb_count;
}

int subweapon_get_max_bombs(void) {
    return s_max_bombs;
}

void subweapon_add_bombs(int count) {
    s_bomb_count += count;
    if (s_bomb_count > s_max_bombs) s_bomb_count = s_max_bombs;
}

void subweapon_get_screen_shake(int* out_ox, int* out_oy) {
    if (!out_ox || !out_oy) return;
    if (s_bomb_screen_shake > 0) {
        *out_ox = ((rand() % 5) - 2);
        *out_oy = ((rand() % 5) - 2);
    } else {
        *out_ox = 0;
        *out_oy = 0;
    }
}
