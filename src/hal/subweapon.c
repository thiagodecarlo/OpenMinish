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
#include "hal/dungeon_flames.h"
#include "hal/dungeon_fortress.h"
#include "hal/fast_travel.h"
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

// Estrutura do Cajado de Pacci (Cane of Pacci)
typedef struct {
    bool  is_active;
    float x;
    float y;
    float vx;
    float vy;
    int   life_timer;
    int   anim_timer;
} PacciState;

static PacciState s_pacci = { 0 };

#define MAX_ACTIVE_ARROWS 4

typedef struct {
    bool      is_active;
    float     x;
    float     y;
    float     vx;
    float     vy;
    Direction dir;
    int       life_timer;
} ActiveArrow;

static ActiveArrow s_arrows[MAX_ACTIVE_ARROWS] = { 0 };
static int s_arrow_count = 30;
static int s_max_arrows = 30;

#define MAX_DIG_PARTICLES 12

typedef struct {
    bool  is_active;
    float x;
    float y;
    float vx;
    float vy;
    int   life;
    u32   color;
} DigParticle;

typedef struct {
    bool        is_digging;
    int         dig_timer;
    Direction   dig_dir;
    float       target_x;
    float       target_y;
    DigParticle particles[MAX_DIG_PARTICLES];
} MoleMittsState;

static MoleMittsState s_mitts = { 0 };

void subweapon_init(void) {
    s_current_item = ITEM_BOOMERANG;
    memset(&s_boomerang, 0, sizeof(s_boomerang));
    memset(&s_gust, 0, sizeof(s_gust));
    memset(s_bombs, 0, sizeof(s_bombs));
    memset(&s_pacci, 0, sizeof(s_pacci));
    memset(s_arrows, 0, sizeof(s_arrows));
    memset(&s_mitts, 0, sizeof(s_mitts));
    s_bomb_count = 10;
    s_max_bombs = 10;
    s_bomb_screen_shake = 0;
    s_arrow_count = 30;
    s_max_arrows = 30;
}

void subweapon_cycle(void) {
    s_current_item = (s_current_item + 1) % ITEM_COUNT;
    hal_audio_play_sound(SOUND_SECRET, 0.70f, 1.6f);
    printf("[SUBWEAPON] Item equipado: %s\n", subweapon_get_name(s_current_item));
}

SubweaponType subweapon_get_current(void) {
    return s_current_item;
}

void subweapon_set_current(SubweaponType item) {
    if (item >= 0 && item < ITEM_COUNT) {
        s_current_item = item;
    }
}

const char* subweapon_get_name(SubweaponType item) {
    switch (item) {
        case ITEM_BOOMERANG:     return "Bumerangue Magico";
        case ITEM_GUST_JAR:      return "Pote Magico (Gust Jar)";
        case ITEM_PEGASUS_BOOTS: return "Botas de Pegasus (Dash)";
        case ITEM_BOMBS:         return "Bolsa de Bombas (Bombs)";
        case ITEM_CANE_OF_PACCI: return "Cajado de Pacci (Inversao)";
        case ITEM_BOW:           return "Arco e Flechas (Bow)";
        case ITEM_MOLE_MITTS:    return "Luvas de Toupeira (Mole Mitts)";
        case ITEM_OCARINA_OF_WIND: return "Ocarina do Vento (Ocarina)";
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
    } else if (s_current_item == ITEM_CANE_OF_PACCI) {
        if (!s_pacci.is_active) {
            s_pacci.is_active = true;
            s_pacci.x = link_x + 4.0f;
            s_pacci.y = link_y + 4.0f;
            s_pacci.life_timer = 36; // Alcance balístico de ~115 pixels
            s_pacci.anim_timer = 0;

            float speed = 3.4f;
            s_pacci.vx = 0.0f;
            s_pacci.vy = 0.0f;
            if (dir == DIR_DOWN)  { s_pacci.vy = speed;  s_pacci.y += 8.0f; }
            if (dir == DIR_UP)    { s_pacci.vy = -speed; s_pacci.y -= 4.0f; }
            if (dir == DIR_LEFT)  { s_pacci.vx = -speed; s_pacci.x -= 4.0f; }
            if (dir == DIR_RIGHT) { s_pacci.vx = speed;  s_pacci.x += 8.0f; }

            hal_audio_play_sound(SOUND_SECRET, 0.90f, 1.8f);
            printf("[CANE OF PACCI] Disparo de energia magica lancado!\n");
        }
    } else if (s_current_item == ITEM_BOW) {
        if (s_arrow_count > 0) {
            for (int i = 0; i < MAX_ACTIVE_ARROWS; i++) {
                if (!s_arrows[i].is_active) {
                    ActiveArrow* a = &s_arrows[i];
                    a->is_active = true;
                    a->x = link_x + 4.0f;
                    a->y = link_y + 4.0f;
                    a->dir = dir;
                    a->life_timer = 45; // ~210 pixels de alcance balístico

                    float speed = 4.8f;
                    a->vx = 0.0f;
                    a->vy = 0.0f;
                    if (dir == DIR_DOWN)  { a->vy = speed;  a->y += 8.0f; }
                    if (dir == DIR_UP)    { a->vy = -speed; a->y -= 4.0f; }
                    if (dir == DIR_LEFT)  { a->vx = -speed; a->x -= 4.0f; }
                    if (dir == DIR_RIGHT) { a->vx = speed;  a->x += 8.0f; }

                    s_arrow_count--;
                    hal_audio_play_sound(SOUND_SWORD_SLASH, 0.9f, 1.4f);
                    printf("[BOW] Flecha disparada na direcao %d! Estoque restante: %d\n", dir, s_arrow_count);
                    break;
                }
            }
        } else {
            hal_audio_play_sound(SOUND_SWITCH_CLICK, 0.7f, 0.8f);
            printf("[BOW] Aljava de flechas vazia!\n");
        }
    } else if (s_current_item == ITEM_MOLE_MITTS) {
        s_mitts.is_digging = true;
        s_mitts.dig_timer = 12;
        s_mitts.dig_dir = dir;
        float fx = link_x + 8.0f;
        float fy = link_y + 12.0f;
        if (dir == DIR_DOWN)  fy += 12.0f;
        if (dir == DIR_UP)    fy -= 12.0f;
        if (dir == DIR_LEFT)  fx -= 12.0f;
        if (dir == DIR_RIGHT) fx += 12.0f;
        s_mitts.target_x = fx;
        s_mitts.target_y = fy;
        hal_audio_play_sound(SOUND_SWORD_SLASH, 0.85f, 1.6f);
    } else if (s_current_item == ITEM_OCARINA_OF_WIND) {
        fast_travel_start(link_x, link_y);
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
    } else if (s_current_item == ITEM_MOLE_MITTS) {
        if (!s_mitts.is_digging || s_mitts.dig_timer <= 0) {
            s_mitts.is_digging = true;
            s_mitts.dig_timer = 10;
            s_mitts.dig_dir = dir;
            float fx = link_x + 8.0f;
            float fy = link_y + 12.0f;
            if (dir == DIR_DOWN)  fy += 12.0f;
            if (dir == DIR_UP)    fy -= 12.0f;
            if (dir == DIR_LEFT)  fx -= 12.0f;
            if (dir == DIR_RIGHT) fx += 12.0f;
            s_mitts.target_x = fx;
            s_mitts.target_y = fy;
            hal_audio_play_sound(SOUND_SWORD_SLASH, 0.80f, 1.7f);
        }
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

    // ------------------------------------------------------------------------
    // 4. ATUALIZAÇÃO DO CAJADO DE PACCI (CANE OF PACCI ENERGY BOLT)
    // ------------------------------------------------------------------------
    if (s_pacci.is_active) {
        s_pacci.x += s_pacci.vx;
        s_pacci.y += s_pacci.vy;
        s_pacci.life_timer--;
        s_pacci.anim_timer++;

        // 1. Interação com o mapa ou dungeon flames (desvira carrinhos de mina)
        if (dungeon_flames_is_active() && dungeon_flames_check_pacci_hit(s_pacci.x, s_pacci.y, 8.0f, 8.0f)) {
            s_pacci.is_active = false;
        }
        else if (map && map_interact_pacci(map, s_pacci.x + 4.0f, s_pacci.y + 4.0f)) {
            s_pacci.is_active = false;
        }
        // 2. Colisão com obstáculos sólidos comuns (dissolve o projétil)
        else if (map && map_is_solid(map, s_pacci.x + 4.0f, s_pacci.y + 4.0f)) {
            s_pacci.is_active = false;
            hal_audio_play_sound(SOUND_SWORD_HIT, 0.6f, 1.6f);
        }
        // 3. Colisão com entidades (inversão de Spiny Beetles e atordoamento)
        else if (entity_check_pacci_hit(s_pacci.x, s_pacci.y, 8.0f, 8.0f)) {
            s_pacci.is_active = false;
        }

        if (s_pacci.life_timer <= 0) {
            s_pacci.is_active = false;
        }
    }

    // ------------------------------------------------------------------------
    // 5. ATUALIZAÇÃO DO ARCO E FLECHAS (ARROW BALLISTICS & EYE STATUE ACTIVATION)
    // ------------------------------------------------------------------------
    for (int i = 0; i < MAX_ACTIVE_ARROWS; i++) {
        ActiveArrow* a = &s_arrows[i];
        if (!a->is_active) continue;

        a->x += a->vx;
        a->y += a->vy;
        a->life_timer--;

        // 1. Interação com Estátua de Olho (ativação ancestral!)
        if (map && map_hit_eye_statue(map, a->x + 4.0f, a->y + 4.0f)) {
            a->is_active = false;
            continue;
        }
        if (dungeon_fortress_is_active()) {
            dungeon_fortress_shoot_arrow_hit(a->x + 4.0f, a->y + 4.0f);
        }

        // 2. Colisão com obstáculos sólidos (corta arbustos, ricocheteia ou quebra na parede)
        if (map) {
            map_interact_slash(map, a->x + 4.0f, a->y + 4.0f);
            if (map_is_solid(map, a->x + 4.0f, a->y + 4.0f)) {
                a->is_active = false;
                hal_audio_play_sound(SOUND_SWORD_HIT, 0.6f, 1.8f);
                continue;
            }
        }

        // 3. Colisão com entidades e monstros (causa 2 pontos de dano)
        int dummy = 0;
        if (entity_check_subweapon_hit(a->x, a->y, 8.0f, 8.0f, 2, &dummy)) {
            a->is_active = false;
            continue;
        }

        if (a->life_timer <= 0) {
            a->is_active = false;
        }
    }

    // ------------------------------------------------------------------------
    // 6. ATUALIZAÇÃO DA ESCAVAÇÃO COM AS LUVAS DE TOUPEIRA (MOLE MITTS)
    // ------------------------------------------------------------------------
    if (s_mitts.is_digging) {
        s_mitts.dig_timer--;
        if (s_mitts.dig_timer == 6) {
            int drop = 0;
            if (map && map_dig_tile(map, s_mitts.target_x, s_mitts.target_y, &drop)) {
                // Gera partículas de terra arremessadas para trás
                for (int p = 0; p < 4; p++) {
                    for (int i = 0; i < MAX_DIG_PARTICLES; i++) {
                        if (!s_mitts.particles[i].is_active) {
                            s_mitts.particles[i].is_active = true;
                            s_mitts.particles[i].x = s_mitts.target_x + ((rand() % 8) - 4);
                            s_mitts.particles[i].y = s_mitts.target_y + ((rand() % 8) - 4);
                            float p_spd = 1.6f + (rand() % 10) * 0.12f;
                            float p_vx = 0.0f, p_vy = 0.0f;
                            if (s_mitts.dig_dir == DIR_DOWN)  { p_vy = -p_spd; p_vx = ((rand() % 10) - 5) * 0.2f; }
                            if (s_mitts.dig_dir == DIR_UP)    { p_vy = p_spd;  p_vx = ((rand() % 10) - 5) * 0.2f; }
                            if (s_mitts.dig_dir == DIR_LEFT)  { p_vx = p_spd;  p_vy = ((rand() % 10) - 5) * 0.2f; }
                            if (s_mitts.dig_dir == DIR_RIGHT) { p_vx = -p_spd; p_vy = ((rand() % 10) - 5) * 0.2f; }
                            s_mitts.particles[i].vx = p_vx;
                            s_mitts.particles[i].vy = p_vy;
                            s_mitts.particles[i].life = 14 + (rand() % 8);
                            s_mitts.particles[i].color = (rand() % 2 == 0) ? 0x824C25FF : 0x542F15FF;
                            break;
                        }
                    }
                }

                // Processa recompensas da escavação
                if (link_rupees && (drop == 1 || drop == 2 || drop == 4)) {
                    int amt = (drop == 1) ? 1 : ((drop == 2) ? 5 : 10);
                    *link_rupees += amt;
                    hal_audio_play_sound(SOUND_SECRET, 0.7f, 1.8f);
                    printf("[MOLE MITTS] Terra escavada revelou %d Rupees!\n", amt);
                } else if (link_hearts && drop == 3) {
                    (*link_hearts)++;
                    hal_audio_play_sound(SOUND_HEART_BEEP, 0.7f, 1.3f);
                    printf("[MOLE MITTS] Terra escavada revelou Coracao de cura!\n");
                }
            }

            // Golpeia inimigos se estiverem na área da escavação
            int dummy = 0;
            entity_check_subweapon_hit(s_mitts.target_x - 6.0f, s_mitts.target_y - 6.0f, 12.0f, 12.0f, 1, &dummy);
        }

        if (s_mitts.dig_timer <= 0) {
            s_mitts.is_digging = false;
        }
    }

    // Atualização das partículas de terra escavada
    for (int i = 0; i < MAX_DIG_PARTICLES; i++) {
        DigParticle* p = &s_mitts.particles[i];
        if (!p->is_active) continue;
        p->x += p->vx;
        p->y += p->vy;
        p->vx *= 0.88f;
        p->vy *= 0.88f;
        p->life--;
        if (p->life <= 0) {
            p->is_active = false;
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

    // 5. Renderiza o Orbe Mágico do Cajado de Pacci
    if (s_pacci.is_active) {
        int px, py;
        map_world_to_screen(cam, s_pacci.x, s_pacci.y, &px, &py);

        u32 c_core  = 0xFFFFFFFF; // Núcleo brilhante branco
        u32 c_cyan1 = 0x38BDF8FF; // Azul celeste mágico
        u32 c_cyan2 = 0x0284C7FF; // Halo azul profundo
        u32 c_spark = 0xFDE047FF; // Faísca dourada de energia

        // Orbe circular com halo estelar pulsante
        int pulse = (s_pacci.anim_timer / 3) % 2;
        for (int dy = -3; dy <= 3; dy++) {
            for (int dx = -3; dx <= 3; dx++) {
                int dist_sq = dx * dx + dy * dy;
                if (dist_sq <= 3) {
                    hal_video_put_pixel(px + 4 + dx, py + 4 + dy, c_core);
                } else if (dist_sq <= 8 + pulse) {
                    hal_video_put_pixel(px + 4 + dx, py + 4 + dy, c_cyan1);
                } else if (dist_sq <= 12 + pulse) {
                    hal_video_put_pixel(px + 4 + dx, py + 4 + dy, c_cyan2);
                }
            }
        }

        // Partículas orbitais / faíscas estelares em rotação contínua
        int rot = s_pacci.anim_timer * 20;
        float rad = (float)rot * (PI_F / 180.0f);
        int sx1 = px + 4 + (int)(cosf(rad) * 6.0f);
        int sy1 = py + 4 + (int)(sinf(rad) * 6.0f);
        int sx2 = px + 4 - (int)(cosf(rad) * 6.0f);
        int sy2 = py + 4 - (int)(sinf(rad) * 6.0f);
        hal_video_put_pixel(sx1, sy1, c_spark);
        hal_video_put_pixel(sx2, sy2, c_spark);
    }

    // 6. Renderiza Flechas em voo
    for (int i = 0; i < MAX_ACTIVE_ARROWS; i++) {
        const ActiveArrow* a = &s_arrows[i];
        if (!a->is_active) continue;

        int ax, ay;
        map_world_to_screen(cam, a->x, a->y, &ax, &ay);
        u32 c_shaft  = 0x92400EFF; // Madeira da haste
        u32 c_head   = 0xF8FAFCFF; // Ponta prateada
        u32 c_fletch = 0xEF4444FF; // Penas vermelhas

        if (a->dir == DIR_RIGHT) {
            for (int dx = 0; dx < 8; dx++) hal_video_put_pixel(ax + dx, ay + 3, c_shaft);
            hal_video_put_pixel(ax + 7, ay + 2, c_head);
            hal_video_put_pixel(ax + 8, ay + 3, c_head);
            hal_video_put_pixel(ax + 7, ay + 4, c_head);
            hal_video_put_pixel(ax + 0, ay + 2, c_fletch);
            hal_video_put_pixel(ax + 0, ay + 4, c_fletch);
        } else if (a->dir == DIR_LEFT) {
            for (int dx = 1; dx < 9; dx++) hal_video_put_pixel(ax + dx, ay + 3, c_shaft);
            hal_video_put_pixel(ax + 1, ay + 2, c_head);
            hal_video_put_pixel(ax + 0, ay + 3, c_head);
            hal_video_put_pixel(ax + 1, ay + 4, c_head);
            hal_video_put_pixel(ax + 8, ay + 2, c_fletch);
            hal_video_put_pixel(ax + 8, ay + 4, c_fletch);
        } else if (a->dir == DIR_DOWN) {
            for (int dy = 0; dy < 8; dy++) hal_video_put_pixel(ax + 3, ay + dy, c_shaft);
            hal_video_put_pixel(ax + 2, ay + 7, c_head);
            hal_video_put_pixel(ax + 3, ay + 8, c_head);
            hal_video_put_pixel(ax + 4, ay + 7, c_head);
            hal_video_put_pixel(ax + 2, ay + 0, c_fletch);
            hal_video_put_pixel(ax + 4, ay + 0, c_fletch);
        } else { // DIR_UP
            for (int dy = 1; dy < 9; dy++) hal_video_put_pixel(ax + 3, ay + dy, c_shaft);
            hal_video_put_pixel(ax + 2, ay + 1, c_head);
            hal_video_put_pixel(ax + 3, ay + 0, c_head);
            hal_video_put_pixel(ax + 4, ay + 1, c_head);
            hal_video_put_pixel(ax + 2, ay + 8, c_fletch);
            hal_video_put_pixel(ax + 4, ay + 8, c_fletch);
        }
    }

    // 5. Renderiza as partículas de terra escavada pelas Mole Mitts
    for (int i = 0; i < MAX_DIG_PARTICLES; i++) {
        DigParticle* p = &s_mitts.particles[i];
        if (!p->is_active) continue;
        int sx, sy;
        map_world_to_screen(cam, p->x, p->y, &sx, &sy);
        hal_video_put_pixel(sx, sy, p->color);
        hal_video_put_pixel(sx + 1, sy, p->color);
        hal_video_put_pixel(sx, sy + 1, 0x3E2310FF);
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
    } else if (s_current_item == ITEM_CANE_OF_PACCI) {
        // Mini Cajado de Pacci: haste esmeralda, gancho dourado e orbe azul celeste
        hal_video_put_pixel(x + 7, y + 8, 0x166534FF); // Haste
        hal_video_put_pixel(x + 8, y + 7, 0x166534FF);
        hal_video_put_pixel(x + 9, y + 6, 0x15803DFF);
        hal_video_put_pixel(x + 10, y + 5, 0xF59E0BFF); // Gancho dourado
        hal_video_put_pixel(x + 10, y + 4, 0xF59E0BFF);
        hal_video_put_pixel(x + 9, y + 3, 0xF59E0BFF);
        hal_video_put_pixel(x + 8, y + 3, 0x38BDF8FF); // Orbe ciano
        hal_video_put_pixel(x + 8, y + 4, 0x0284C7FF);
    } else if (s_current_item == ITEM_BOW) {
        // Mini Arco curvado de madeira e corda
        hal_video_put_pixel(x + 7, y + 4, 0x92400EFF);
        hal_video_put_pixel(x + 8, y + 3, 0x92400EFF);
        hal_video_put_pixel(x + 9, y + 4, 0x92400EFF);
        hal_video_put_pixel(x + 9, y + 5, 0x92400EFF);
        hal_video_put_pixel(x + 9, y + 6, 0x92400EFF);
        hal_video_put_pixel(x + 8, y + 7, 0x92400EFF);
        hal_video_put_pixel(x + 7, y + 6, 0x92400EFF);
        // Corda esticada
        hal_video_put_pixel(x + 7, y + 5, 0xE2E8F0FF);
        // Flecha encaixada
        hal_video_put_pixel(x + 11, y + 5, 0xFFFFFFFF);
        hal_video_put_pixel(x + 10, y + 5, 0x92400EFF);
    } else if (s_current_item == ITEM_MOLE_MITTS) {
        // Mini Luva de Toupeira (Luva de couro marrom com 3 garras prateadas)
        hal_video_put_pixel(x + 7, y + 4, 0x78350FFF);
        hal_video_put_pixel(x + 8, y + 4, 0x92400EFF);
        hal_video_put_pixel(x + 7, y + 5, 0x78350FFF);
        hal_video_put_pixel(x + 8, y + 5, 0xB45309FF);
        hal_video_put_pixel(x + 7, y + 6, 0x78350FFF);
        hal_video_put_pixel(x + 8, y + 6, 0x92400EFF);
        // Garras afiadas
        hal_video_put_pixel(x + 9, y + 3, 0xE2E8F0FF);
        hal_video_put_pixel(x + 10, y + 4, 0xFFFFFFFF);
        hal_video_put_pixel(x + 10, y + 5, 0xFFFFFFFF);
        hal_video_put_pixel(x + 10, y + 6, 0xFFFFFFFF);
        hal_video_put_pixel(x + 9, y + 7, 0xE2E8F0FF);
    } else if (s_current_item == ITEM_OCARINA_OF_WIND) {
        // Mini Ocarina azul do Vento
        hal_video_put_pixel(x + 5, y + 5, 0x38BDF8FF); // Bocal
        for (int dx = 6; dx <= 10; dx++) {
            hal_video_put_pixel(x + dx, y + 4, 0x0284C7FF);
            hal_video_put_pixel(x + dx, y + 5, 0x0284C7FF);
            hal_video_put_pixel(x + dx, y + 6, 0x0369A1FF);
        }
        // Furos de notas
        hal_video_put_pixel(x + 7, y + 5, 0x0F172AFF);
        hal_video_put_pixel(x + 9, y + 5, 0x0F172AFF);
        hal_video_put_pixel(x + 8, y + 4, 0xBAE6FDFF); // Brilho
    }
}

bool subweapon_is_digging(void) {
    return (s_current_item == ITEM_MOLE_MITTS && s_mitts.is_digging);
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

int subweapon_get_arrow_count(void) {
    return s_arrow_count;
}

int subweapon_get_max_arrows(void) {
    return s_max_arrows;
}

void subweapon_add_arrows(int count) {
    s_arrow_count += count;
    if (s_arrow_count > s_max_arrows) s_arrow_count = s_max_arrows;
}
