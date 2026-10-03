#include "hal/entity.h"
#include "hal/dungeon.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

/*
 * ============================================================================
 * src/hal/entity.c - Gerenciador de Entidades, IA do Octorok e NPCs
 * ============================================================================
 */

static Entity s_entities[MAX_ENTITIES];
static const Texture* s_octo_tex = NULL;
static float s_last_link_x = 0.0f;
static float s_last_link_y = 0.0f;
static int   s_screen_shake_timer = 0;
static int   s_screen_shake_magnitude = 0;
static bool  s_boss_defeated = false;

static inline bool entity_is_solid(const Tilemap* map, float wx, float wy) {
    if (dungeon_is_active()) {
        return dungeon_is_solid(wx, wy);
    }
    return map_is_solid(map, wx, wy);
}

void entity_set_texture(const Texture* tex) {
    s_octo_tex = tex;
}

static inline void put_pixel_safe(int x, int y, u32 color) {
    hal_video_put_pixel(x, y, color);
}

static void draw_filled_rect(int rx, int ry, int rw, int rh, u32 color) {
    for (int y = ry; y < ry + rh; y++) {
        for (int x = rx; x < rx + rw; x++) {
            put_pixel_safe(x, y, color);
        }
    }
}

void entity_trigger_screen_shake(int duration_frames, int magnitude) {
    s_screen_shake_timer = duration_frames;
    s_screen_shake_magnitude = magnitude;
}

void entity_get_screen_shake(int* out_x, int* out_y) {
    if (s_screen_shake_timer > 0) {
        s_screen_shake_timer--;
        int mag = s_screen_shake_magnitude;
        if (mag < 1) mag = 1;
        if (out_x) *out_x = (rand() % (mag * 2 + 1)) - mag;
        if (out_y) *out_y = (rand() % (mag * 2 + 1)) - mag;
    } else {
        if (out_x) *out_x = 0;
        if (out_y) *out_y = 0;
    }
}

bool entity_is_boss_defeated(void) {
    return s_boss_defeated;
}

bool entity_is_boss_alive(void) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (s_entities[i].is_active &&
            s_entities[i].type == ENTITY_BOSS_BIG_CHUCHU &&
            s_entities[i].health > 0) {
            return true;
        }
    }
    return false;
}

void entity_manager_init(void) {
    memset(s_entities, 0, sizeof(s_entities));
    s_screen_shake_timer = 0;
    s_screen_shake_magnitude = 0;
    s_boss_defeated = false;
    printf("[HAL Entity] Gerenciador de entidades inicializado (Pool: %d slots).\n", MAX_ENTITIES);
}

int entity_count_active_enemies(void) {
    int count = 0;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!s_entities[i].is_active) continue;
        if (s_entities[i].type == ENTITY_ENEMY_OCTOROK ||
            s_entities[i].type == ENTITY_ENEMY_KEESE ||
            s_entities[i].type == ENTITY_ENEMY_CHUCHU ||
            (s_entities[i].type == ENTITY_BOSS_BIG_CHUCHU && s_entities[i].health > 0)) {
            count++;
        }
    }
    return count;
}

void entity_clear_all(void) {
    memset(s_entities, 0, sizeof(s_entities));
}

Entity* entity_spawn(EntityType type, float world_x, float world_y) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (!s_entities[i].is_active) {
            Entity* e = &s_entities[i];
            memset(e, 0, sizeof(Entity));

            e->type      = type;
            e->x         = world_x;
            e->y         = world_y;
            e->is_active = true;

            switch (type) {
                case ENTITY_ENEMY_OCTOROK:
                    e->health    = 2; // Octorok clássico precisa de 2 golpes de espada
                    e->maxHealth = 2;
                    e->damage    = 1;
                    e->dir       = DIR_DOWN;
                    e->action    = 1; // Patrulha
                    e->aiTimer   = 60 + (rand() % 60);
                    e->hitbox    = (Hitbox){ 1.0f, 1.0f, 14.0f, 14.0f };
                    break;

                case ENTITY_PROJECTILE_ROCK:
                    e->damage    = 1;
                    e->hitbox    = (Hitbox){ 1.0f, 1.0f, 6.0f, 6.0f };
                    break;

                case ENTITY_ITEM_RUPEE:
                    e->hitbox    = (Hitbox){ 1.0f, 1.0f, 8.0f, 10.0f };
                    break;

                case ENTITY_ITEM_HEART:
                    e->hitbox    = (Hitbox){ 1.0f, 1.0f, 8.0f, 8.0f };
                    break;

                case ENTITY_NPC_FOREST_MINISH:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1; // Idle/respirando
                    e->z             = 0.0f;
                    e->vz            = 0.0f;
                    e->hitbox        = (Hitbox){ 2.0f, 2.0f, 12.0f, 12.0f };
                    e->hasKinstone   = true;
                    e->kinstoneType  = 0; // KINSTONE_GREEN (Fragmento verde comum)
                    e->kinstoneFused = false;
                    e->bubbleBob     = 0.0f;
                    break;

                case ENTITY_CHEST_GOLD:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 0; // 0 = Fechado, 1 = Aberto
                    e->hitbox        = (Hitbox){ 1.0f, 1.0f, 14.0f, 14.0f };
                    break;

                case ENTITY_ENEMY_KEESE:
                    e->health    = 1;
                    e->maxHealth = 1;
                    e->damage    = 1;
                    e->dir       = DIR_DOWN;
                    e->action    = 1; // Voo de cruzeiro
                    e->z         = 10.0f;
                    e->vz        = 0.0f;
                    e->aiTimer   = 60 + (rand() % 60);
                    e->hitbox    = (Hitbox){ 1.0f, 1.0f, 14.0f, 12.0f };
                    break;

                case ENTITY_ENEMY_CHUCHU:
                    e->health    = 2;
                    e->maxHealth = 2;
                    e->damage    = 1;
                    e->dir       = DIR_DOWN;
                    e->action    = 0; // Poça de gosma camuflada no solo
                    e->z         = 0.0f;
                    e->vz        = 0.0f;
                    e->aiTimer   = 30;
                    e->hitbox    = (Hitbox){ 2.0f, 2.0f, 12.0f, 12.0f };
                    break;

                case ENTITY_BOSS_BIG_CHUCHU:
                    e->health        = 10;
                    e->maxHealth     = 10;
                    e->damage        = 1;
                    e->dir           = DIR_DOWN;
                    e->action        = 1; // 1 = Saltos / Patrulha ereta
                    e->subAction     = 0; // 0 = Agachando no solo, 1 = No ar
                    e->bossBaseScale = 1.0f;
                    e->bossSuctionTimer = 0;
                    e->bossToppleTimer  = 0;
                    e->bossEnraged      = false;
                    e->bossDeathTimer   = 0;
                    e->aiTimer       = 35;
                    e->hitbox        = (Hitbox){ -20.0f, -40.0f, 40.0f, 48.0f };
                    break;

                case ENTITY_ITEM_HEART_CONTAINER:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->action        = 0;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                default:
                    break;
            }
            return e;
        }
    }
    return NULL; // Pool esgotado
}

void entity_manager_update(const Tilemap* map, float link_x, float link_y,
                           int* link_hearts, int* link_max_hearts, int* link_rupees,
                           int* link_invuln_timer, float* link_knock_x, float* link_knock_y) {
    s_last_link_x = link_x;
    s_last_link_y = link_y;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active) continue;

        // --------------------------------------------------------------------
        // 1. INIMIGO: OCTOROK VERMELHO (MÁQUINA DE ESTADOS E IA)
        // --------------------------------------------------------------------
        if (e->type == ENTITY_ENEMY_OCTOROK) {
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Ação 4: Recuo por dano da espada (Knockback)
            if (e->action == 4) {
                float next_x = e->x + e->knockbackVx;
                float next_y = e->y + e->knockbackVy;

                if (!entity_is_solid(map, next_x + 8.0f, next_y + 8.0f)) {
                    e->x = next_x;
                    e->y = next_y;
                }

                e->knockbackTimer--;
                if (e->knockbackTimer <= 0) {
                    if (e->health <= 0) {
                        // Inimigo derrotado! Dropa itens
                        int roll = rand() % 100;
                        if (roll < 45) {
                            entity_spawn(ENTITY_ITEM_RUPEE, e->x + 3.0f, e->y + 3.0f);
                        } else if (roll < 75) {
                            entity_spawn(ENTITY_ITEM_HEART, e->x + 3.0f, e->y + 3.0f);
                        }
                        e->is_active = false;
                        continue;
                    } else {
                        e->action = 1; // Retorna à patrulha
                        e->aiTimer = 40;
                    }
                }
            }
            // Ação 1: Patrulha e Rastreamento de Proximidade
            else if (e->action == 1) {
                float speed = 0.40f;
                float move_x = 0.0f;
                float move_y = 0.0f;

                if (e->dir == DIR_DOWN)  move_y = speed;
                if (e->dir == DIR_UP)    move_y = -speed;
                if (e->dir == DIR_LEFT)  move_x = -speed;
                if (e->dir == DIR_RIGHT) move_x = speed;

                float check_x = e->x + move_x + 8.0f;
                float check_y = e->y + move_y + 8.0f;

                // Se bater em parede ou água, inverte a direção
                if (entity_is_solid(map, check_x, check_y)) {
                    e->dir = (Direction)(rand() % 4);
                    e->aiTimer = 60;
                } else {
                    e->x += move_x;
                    e->y += move_y;
                }

                // Animação de tentáculos se movendo
                e->animTimer++;
                if (e->animTimer > 8) {
                    e->animFrame = (e->animFrame + 1) % 2;
                    e->animTimer = 0;
                }

                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->dir = (Direction)(rand() % 4);
                    e->aiTimer = 60 + (rand() % 60);
                }

                // Detecção do Link (Se estiver a menos de 130 pixels e alinhado)
                float dx = (link_x + 8.0f) - (e->x + 8.0f);
                float dy = (link_y + 8.0f) - (e->y + 8.0f);
                float dist = sqrtf(dx * dx + dy * dy);

                if (dist < 130.0f) {
                    bool aligned_h = (fabsf(dy) < 18.0f);
                    bool aligned_v = (fabsf(dx) < 18.0f);

                    if (aligned_h || aligned_v) {
                        e->action = 2; // Prepara para atirar!
                        e->aiTimer = 22; // 22 frames de antecipação (bochechas inchando)
                        if (fabsf(dx) > fabsf(dy)) {
                            e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                        } else {
                            e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                        }
                    }
                }
            }
            // Ação 2: Preparação de Tiro (Inchar bochechas)
            else if (e->action == 2) {
                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    // Dispara a pedra!
                    float spawn_x = e->x + 5.0f;
                    float spawn_y = e->y + 5.0f;
                    if (e->dir == DIR_DOWN)  spawn_y += 8.0f;
                    if (e->dir == DIR_UP)    spawn_y -= 8.0f;
                    if (e->dir == DIR_LEFT)  spawn_x -= 8.0f;
                    if (e->dir == DIR_RIGHT) spawn_x += 8.0f;

                    Entity* rock = entity_spawn(ENTITY_PROJECTILE_ROCK, spawn_x, spawn_y);
                    if (rock) {
                        float rock_speed = 2.2f;
                        rock->dir = e->dir;
                        if (e->dir == DIR_DOWN)  rock->vy = rock_speed;
                        if (e->dir == DIR_UP)    rock->vy = -rock_speed;
                        if (e->dir == DIR_LEFT)  rock->vx = -rock_speed;
                        if (e->dir == DIR_RIGHT) rock->vx = rock_speed;
                        hal_audio_play_sound(SOUND_ROLL, 0.6f, 1.4f); // Som de cusparada
                    }

                    e->action = 1; // Retorna à patrulha
                    e->aiTimer = 80; // Cooldown antes de poder atirar de novo
                }
            }

            // Colisão de contato com o Link
            if (*link_invuln_timer <= 0) {
                float ox1 = e->x + e->hitbox.offset_x;
                float oy1 = e->y + e->hitbox.offset_y;
                float ox2 = ox1 + e->hitbox.width;
                float oy2 = oy1 + e->hitbox.height;

                float lx1 = link_x + 3.0f;
                float ly1 = link_y + 4.0f;
                float lx2 = lx1 + 10.0f;
                float ly2 = ly1 + 12.0f;

                if (ox1 < lx2 && ox2 > lx1 && oy1 < ly2 && oy2 > ly1) {
                    if (*link_hearts > 0) (*link_hearts)--;
                    *link_invuln_timer = 50; // Quase 1 segundo invulnerável
                    hal_audio_play_sound(SOUND_HEART_BEEP, 0.85f, 1.0f);

                    // Empurrão de dano no Link
                    float p_dx = (link_x + 8.0f) - (e->x + 8.0f);
                    float p_dy = (link_y + 8.0f) - (e->y + 8.0f);
                    float p_len = sqrtf(p_dx * p_dx + p_dy * p_dy);
                    if (p_len > 0.01f) {
                        *link_knock_x = (p_dx / p_len) * 3.5f;
                        *link_knock_y = (p_dy / p_len) * 3.5f;
                    }
                }
            }
        }

        // --------------------------------------------------------------------
        // 2. PROJÉTIL: PEDRA DO OCTOROK
        // --------------------------------------------------------------------
        else if (e->type == ENTITY_PROJECTILE_ROCK) {
            e->x += e->vx;
            e->y += e->vy;

            // Se colidir com terreno sólido, a pedra se estilhaça
            if (entity_is_solid(map, e->x + 3.0f, e->y + 3.0f)) {
                e->is_active = false;
                continue;
            }

            // Colisão com o Link
            if (*link_invuln_timer <= 0) {
                float rx1 = e->x;
                float ry1 = e->y;
                float rx2 = rx1 + 6.0f;
                float ry2 = ry1 + 6.0f;

                float lx1 = link_x + 3.0f;
                float ly1 = link_y + 4.0f;
                float lx2 = lx1 + 10.0f;
                float ly2 = ly1 + 12.0f;

                if (rx1 < lx2 && rx2 > lx1 && ry1 < ly2 && ry2 > ly1) {
                    if (*link_hearts > 0) (*link_hearts)--;
                    *link_invuln_timer = 50;
                    hal_audio_play_sound(SOUND_HEART_BEEP, 0.85f, 1.0f);
                    *link_knock_x = (e->vx > 0.0f) ? 3.0f : (e->vx < 0.0f) ? -3.0f : 0.0f;
                    *link_knock_y = (e->vy > 0.0f) ? 3.0f : (e->vy < 0.0f) ? -3.0f : 0.0f;
                    e->is_active = false;
                    continue;
                }
            }
        }

        // --------------------------------------------------------------------
        // 3. ITENS COLETÁVEIS: RUPEE E CORAÇÃO
        // --------------------------------------------------------------------
        else if (e->type == ENTITY_ITEM_RUPEE || e->type == ENTITY_ITEM_HEART) {
            e->animTimer++;

            // Coleta pelo Link
            float ix1 = e->x;
            float iy1 = e->y;
            float ix2 = ix1 + 10.0f;
            float iy2 = iy1 + 10.0f;

            float lx1 = link_x + 2.0f;
            float ly1 = link_y + 2.0f;
            float lx2 = lx1 + 12.0f;
            float ly2 = ly1 + 14.0f;

            if (ix1 < lx2 && ix2 > lx1 && iy1 < ly2 && iy2 > ly1) {
                if (e->type == ENTITY_ITEM_RUPEE) {
                    *link_rupees += 5;
                    hal_audio_play_sound(SOUND_SECRET, 0.8f, 1.4f);
                } else if (e->type == ENTITY_ITEM_HEART) {
                    if (*link_hearts < 3) (*link_hearts)++;
                    hal_audio_play_sound(SOUND_SECRET, 0.9f, 1.2f);
                }
                e->is_active = false;
            }
        }

        // --------------------------------------------------------------------
        // 4. NPC AMIGÁVEL: MINISH DA FLORESTA
        // --------------------------------------------------------------------
        else if (e->type == ENTITY_NPC_FOREST_MINISH) {
            e->animTimer++;
            e->bubbleBob += 0.08f;

            // Se o Link estiver por perto (raio de 36px), o Minish vira o rosto na direção dele
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            float dist_sq = dx * dx + dy * dy;

            if (dist_sq <= 36.0f * 36.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            }
        }
        else if (e->type == ENTITY_CHEST_GOLD) {
            e->animTimer++;
        }

        // --------------------------------------------------------------------
        // 5. INIMIGO: KEESE (MORCEGO VOADOR)
        // --------------------------------------------------------------------
        else if (e->type == ENTITY_ENEMY_KEESE) {
            e->animTimer++;
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Ação 4: Recuo por golpe de espada
            if (e->action == 4) {
                e->x += e->knockbackVx;
                e->y += e->knockbackVy;
                e->knockbackVx *= 0.86f;
                e->knockbackVy *= 0.86f;
                e->knockbackTimer--;
                if (e->knockbackTimer <= 0) {
                    if (e->health <= 0) {
                        e->is_active = false;
                        if ((rand() % 100) < 65) {
                            EntityType drop = ((rand() % 2) == 0) ? ENTITY_ITEM_RUPEE : ENTITY_ITEM_HEART;
                            entity_spawn(drop, e->x, e->y);
                        }
                        continue;
                    }
                    e->action = 3; // Foge após o golpe
                    e->aiTimer = 60;
                }
                continue;
            }

            // Oscilação vertical senoidal de altitude no voo (bobbing)
            e->z = 10.0f + 3.5f * sinf((float)e->animTimer * 0.16f);

            float dx = link_x - e->x;
            float dy = link_y - e->y;
            float dist = sqrtf(dx * dx + dy * dy);

            // Máquina de estados da IA
            if (e->action == 1) { // Voo de cruzeiro / patrulha
                e->x += e->vx;
                e->y += e->vy;
                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    float angle = ((float)(rand() % 360)) * (PI_F / 180.0f);
                    e->vx = cosf(angle) * 0.65f;
                    e->vy = sinf(angle) * 0.65f;
                    e->aiTimer = 45 + (rand() % 45);
                }

                // Detecta Link se estiver a até 80 pixels
                if (dist < 80.0f && dist > 1.0f) {
                    e->action = 2; // Alerta e rasante
                    e->aiTimer = 90;
                    hal_audio_play_sound(SOUND_KEESE_CHIRP, 0.65f, 1.05f);
                }
            } else if (e->action == 2) { // Rasante em direção ao Link
                if (dist > 1.0f) {
                    float target_vx = (dx / dist) * 1.35f;
                    float target_vy = (dy / dist) * 1.35f;
                    e->vx += (target_vx - e->vx) * 0.10f;
                    e->vy += (target_vy - e->vy) * 0.10f;
                }
                e->x += e->vx;
                e->y += e->vy;

                // Baixa um pouco a altitude ao mergulhar
                e->z = 6.0f + 2.0f * sinf((float)e->animTimer * 0.22f);

                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->action = 3; // Afasta-se em arco
                    e->aiTimer = 50;
                }
            } else if (e->action == 3) { // Recuo / circundar para nova investida
                if (dist > 1.0f) {
                    float away_vx = -(dx / dist) * 0.90f;
                    float away_vy = -(dy / dist) * 0.90f;
                    e->vx += (away_vx - e->vx) * 0.08f;
                    e->vy += (away_vy - e->vy) * 0.08f;
                }
                e->x += e->vx;
                e->y += e->vy;
                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->action = 1;
                    e->aiTimer = 50;
                }
            }

            // Dano de contato ao herói (se estiver baixo o suficiente e o Link vulnerável)
            if (e->z <= 12.0f && *link_invuln_timer <= 0 && dist < 12.0f) {
                if (*link_hearts > 0) (*link_hearts)--;
                *link_invuln_timer = 50;
                hal_audio_play_sound(SOUND_HEART_BEEP, 0.85f, 1.0f);
                *link_knock_x = (e->vx > 0.0f) ? 3.0f : -3.0f;
                *link_knock_y = (e->vy > 0.0f) ? 3.0f : -3.0f;
                e->action = 3; // Recua após acertar
                e->aiTimer = 45;
            }
        }

        // --------------------------------------------------------------------
        // 6. INIMIGO: GREEN CHUCHU (GOSMA GELATINOSA)
        // --------------------------------------------------------------------
        else if (e->type == ENTITY_ENEMY_CHUCHU) {
            e->animTimer++;
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Ação 4: Recuo por golpe de espada
            if (e->action == 4) {
                e->x += e->knockbackVx;
                e->y += e->knockbackVy;
                e->knockbackVx *= 0.82f;
                e->knockbackVy *= 0.82f;
                e->knockbackTimer--;
                if (e->knockbackTimer <= 0) {
                    if (e->health <= 0) {
                        e->is_active = false;
                        if ((rand() % 100) < 70) {
                            EntityType drop = ((rand() % 2) == 0) ? ENTITY_ITEM_RUPEE : ENTITY_ITEM_HEART;
                            entity_spawn(drop, e->x, e->y);
                        }
                        continue;
                    }
                    e->action = 2; // Volta a se preparar para o próximo salto
                    e->aiTimer = 35;
                }
                continue;
            }

            float dx = link_x - e->x;
            float dy = link_y - e->y;
            float dist = sqrtf(dx * dx + dy * dy);

            // Estado 0: Poça disfarçada no solo
            if (e->action == 0) {
                if (dist <= 65.0f) {
                    e->action = 1; // Brotar / Emergência vertical
                    e->aiTimer = 20;
                    hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.70f, 0.90f);
                }
            }
            // Estado 1: Emergindo da terra
            else if (e->action == 1) {
                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->action = 2; // Em pé, preparando salto
                    e->aiTimer = 30;
                }
            }
            // Estado 2: Agachando (squash) para impulso de salto
            else if (e->action == 2) {
                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    if (dist > 1.0f && dist < 120.0f) {
                        e->action = 3; // Salto parabólico no ar!
                        e->vx = (dx / dist) * 1.55f;
                        e->vy = (dy / dist) * 1.55f;
                        e->vz = 3.2f; // Impulso vertical inicial
                        e->z = 0.0f;
                        hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.85f, 1.15f);
                    } else {
                        e->aiTimer = 30; // Espera o herói se aproximar
                    }
                }
            }
            // Estado 3: Salto aéreo balístico
            else if (e->action == 3) {
                e->x += e->vx;
                e->y += e->vy;
                e->z += e->vz;
                e->vz -= 0.26f; // Gravidade que puxa a gosma de volta

                // Colisão de aterrissagem
                if (e->z <= 0.0f) {
                    e->z = 0.0f;
                    e->vx = 0.0f;
                    e->vy = 0.0f;
                    e->action = 2; // Recuperação pós-salto
                    e->aiTimer = 40;
                }
            }

            // Dano de contato ao Link (quando emergido)
            if (e->action > 0 && *link_invuln_timer <= 0 && dist < 13.0f && e->z <= 8.0f) {
                if (*link_hearts > 0) (*link_hearts)--;
                *link_invuln_timer = 50;
                hal_audio_play_sound(SOUND_HEART_BEEP, 0.85f, 1.0f);
                *link_knock_x = (dx > 0.0f) ? -3.0f : 3.0f;
                *link_knock_y = (dy > 0.0f) ? -3.0f : 3.0f;
            }
        }

        // --------------------------------------------------------------------
        // 7. ITEM: HEART CONTAINER (RECIPIENTE DE CORAÇÃO PERMANENTE)
        // --------------------------------------------------------------------
        else if (e->type == ENTITY_ITEM_HEART_CONTAINER) {
            e->animTimer++;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            float dist = sqrtf(dx * dx + dy * dy);

            // Coleta pelo Link
            if (dist < 18.0f) {
                e->is_active = false;
                if (link_max_hearts) (*link_max_hearts)++;
                if (link_hearts && link_max_hearts) *link_hearts = *link_max_hearts;
                hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.0f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                printf("[RECOMPENSA] Heart Container obtido! Max Hearts: %d, Vida totalmente restaurada!\n",
                       link_max_hearts ? *link_max_hearts : 4);
                continue;
            }
        }

        // --------------------------------------------------------------------
        // 8. CHEFE: BIG GREEN CHUCHU (COLOSSO GELATINOSO DE DEEPWOOD SHRINE)
        // --------------------------------------------------------------------
        else if (e->type == ENTITY_BOSS_BIG_CHUCHU) {
            e->animTimer++;
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Transição para Fase 2 (Enrage se HP <= 5)
            if (e->health <= 5 && !e->bossEnraged) {
                e->bossEnraged = true;
                hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 1.25f);
                entity_trigger_screen_shake(12, 4);
                printf("[BOSS ENRAGE] Big Green ChuChu enfurecido! Movimentos e saltos acelerados!\n");
            }

            float dx = link_x - e->x;
            float dy = link_y - e->y;
            float dist = sqrtf(dx * dx + dy * dy);

            // AÇÃO 1: Saltos Gigantescos (Hopping)
            if (e->action == 1) {
                // Recuperação natural da base se o Pote Mágico parou de sugar
                if (e->bossSuctionTimer > 0) {
                    e->bossSuctionTimer--;
                    e->bossBaseScale = 1.0f - (float)e->bossSuctionTimer / 60.0f;
                    if (e->bossBaseScale > 1.0f) e->bossBaseScale = 1.0f;
                }

                if (e->subAction == 0) {
                    // Agachando no chão preparando o salto
                    e->aiTimer--;
                    if (e->aiTimer <= 0) {
                        e->subAction = 1; // Salta no ar
                        float hop_force = e->bossEnraged ? 4.4f : 3.5f;
                        e->vz = hop_force;
                        float spd = e->bossEnraged ? 1.55f : 0.95f;
                        if (dist > 2.0f) {
                            e->vx = (dx / dist) * spd;
                            e->vy = (dy / dist) * spd;
                        }
                        hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.90f, e->bossEnraged ? 1.15f : 0.75f);
                    }
                } else if (e->subAction == 1) {
                    // No ar
                    e->x += e->vx;
                    e->y += e->vy;
                    e->z += e->vz;
                    e->vz -= 0.22f; // Gravidade

                    // Impacto no solo
                    if (e->z <= 0.0f) {
                        e->z = 0.0f;
                        e->vx = 0.0f;
                        e->vy = 0.0f;
                        e->subAction = 0;
                        e->aiTimer = e->bossEnraged ? 20 : 35;

                        // Tremor de solo com impacto pesado
                        entity_trigger_screen_shake(10, e->bossEnraged ? 4 : 3);
                        hal_audio_play_sound(SOUND_BOSS_SLAM, 0.95f, e->bossEnraged ? 1.10f : 0.85f);
                    }
                }

                // Dano de contato ao herói (corpo gigante esmagador)
                if (*link_invuln_timer <= 0 && dist < 24.0f && e->z <= 12.0f) {
                    if (*link_hearts > 0) (*link_hearts)--;
                    *link_invuln_timer = 60;
                    hal_audio_play_sound(SOUND_HEART_BEEP, 0.9f, 0.9f);
                    float knock_force = 4.5f;
                    *link_knock_x = (dx > 0.0f) ? -knock_force : knock_force;
                    *link_knock_y = (dy > 0.0f) ? -knock_force : knock_force;
                }
            }

            // AÇÃO 2: Sendo Sugado pelo Pote Mágico (Suctioned Wobble)
            else if (e->action == 2) {
                if (e->bossSuctionTimer >= 55) {
                    // Pés totalmente sugados! Perde o equilíbrio e DESABA!
                    e->action = 3; // Toppled / Vulnerável
                    e->subAction = 0;
                    e->bossToppleTimer = 180; // 3 segundos inteiros desabado no chão
                    e->bossBaseScale = 0.15f;
                    e->z = 0.0f;
                    e->vx = 0.0f;
                    e->vy = 0.0f;
                    entity_trigger_screen_shake(16, 5);
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.70f);
                    hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 1.0f, 0.60f);
                    printf("[BOSS] Big Green ChuChu toppled! A cabeca desabou no chao, vulneravel a espada!\n");
                }
            }

            // AÇÃO 3: Desabado no Chão (Toppled & Vulnerable)
            else if (e->action == 3) {
                e->bossToppleTimer--;
                if (e->bossToppleTimer <= 0) {
                    // Recupera-se e levanta de volta!
                    e->action = 4; // Recuperação
                    e->aiTimer = 40;
                    hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.95f, 0.85f);
                }
            }

            // AÇÃO 4: Recuperação e Regeneração dos Pés
            else if (e->action == 4) {
                e->aiTimer--;
                // A base vai expandindo de volta para 1.0
                e->bossBaseScale += (1.0f - e->bossBaseScale) * 0.10f;
                if (e->aiTimer <= 0) {
                    e->bossBaseScale = 1.0f;
                    e->bossSuctionTimer = 0;
                    e->action = 1; // Volta a pular
                    e->subAction = 0;
                    e->aiTimer = e->bossEnraged ? 20 : 35;
                }
            }

            // AÇÃO 5: Sequência de Morte e Explosão de Gosma
            else if (e->action == 5) {
                e->bossDeathTimer++;
                // Tremores contínuos durante a morte
                if (e->bossDeathTimer % 6 == 0) {
                    entity_trigger_screen_shake(6, 3);
                    hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.8f, 1.0f + (float)e->bossDeathTimer * 0.01f);
                }

                if (e->bossDeathTimer >= 80) {
                    // Clímax da derrota!
                    e->is_active = false;
                    s_boss_defeated = true;
                    hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                    entity_trigger_screen_shake(20, 6);

                    // Spawna o cobiçado Heart Container!
                    entity_spawn(ENTITY_ITEM_HEART_CONTAINER, e->x, e->y);
                    printf("[BOSS DEFEATED] Big Green ChuChu explodiu em gosma verde! Recompensa liberada!\n");
                    continue;
                }
            }
        }
    }
}

bool entity_check_sword_hit(float slash_x, float slash_y, float slash_w, float slash_h,
                            int damage, Direction slash_dir) {
    bool hit_something = false;

    float sx1 = slash_x;
    float sy1 = slash_y;
    float sx2 = slash_x + slash_w;
    float sy2 = slash_y + slash_h;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active) continue;

        // Golpeando Inimigo (Octorok, Keese ou ChuChu)
        if ((e->type == ENTITY_ENEMY_OCTOROK ||
             e->type == ENTITY_ENEMY_KEESE ||
             (e->type == ENTITY_ENEMY_CHUCHU && e->action > 0)) &&
            e->invulnerableTimer <= 0) {

            // Se o Keese estiver voando alto demais fora do alcance da lâmina
            if (e->type == ENTITY_ENEMY_KEESE && e->z > 16.0f) {
                continue;
            }

            float ox1 = e->x + e->hitbox.offset_x;
            float oy1 = e->y + e->hitbox.offset_y;
            float ox2 = ox1 + e->hitbox.width;
            float oy2 = oy1 + e->hitbox.height;

            if (sx1 < ox2 && sx2 > ox1 && sy1 < oy2 && sy2 > oy1) {
                e->health -= damage;
                e->invulnerableTimer = 18; // Pisca de dano
                e->action = 4; // Knockback
                e->knockbackTimer = (e->type == ENTITY_ENEMY_KEESE) ? 14 : 10;

                float force = (e->type == ENTITY_ENEMY_KEESE) ? 4.2f : 3.0f;
                e->knockbackVx = 0.0f;
                e->knockbackVy = 0.0f;
                if (slash_dir == DIR_DOWN)  e->knockbackVy = force;
                if (slash_dir == DIR_UP)    e->knockbackVy = -force;
                if (slash_dir == DIR_LEFT)  e->knockbackVx = -force;
                if (slash_dir == DIR_RIGHT) e->knockbackVx = force;

                float hit_pitch = (e->type == ENTITY_ENEMY_CHUCHU) ? 1.45f : 1.20f;
                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, hit_pitch);
                hit_something = true;
            }
        }
        // Golpeando e rebatendo/destruindo a pedra no ar com a espada!
        else if (e->type == ENTITY_PROJECTILE_ROCK) {
            float rx1 = e->x;
            float ry1 = e->y;
            float rx2 = rx1 + 6.0f;
            float ry2 = ry1 + 6.0f;

            if (sx1 < rx2 && sx2 > rx1 && sy1 < ry2 && sy2 > ry1) {
                e->is_active = false; // Destrói a pedra
                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.5f);
                hit_something = true;
            }
        }
        // Golpeando o Chefe Big Green ChuChu
        else if (e->type == ENTITY_BOSS_BIG_CHUCHU) {
            // Se o chefe estiver toppled/desabado no chão, a cabeça está vulnerável!
            if (e->action == 3 && e->invulnerableTimer <= 0) {
                float bx1 = e->x - 22.0f;
                float by1 = e->y - 12.0f;
                float bx2 = e->x + 22.0f;
                float by2 = e->y + 16.0f;

                if (sx1 < bx2 && sx2 > bx1 && sy1 < by2 && sy2 > by1) {
                    e->health -= damage;
                    e->invulnerableTimer = 22;
                    hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.0f);
                    entity_trigger_screen_shake(8, 3);
                    printf("[BOSS HIT] Golpe na cabeca vulneravel! Dano aplicado! HP restante: %d / %d\n",
                           e->health, e->maxHealth);

                    if (e->health <= 0) {
                        e->health = 0;
                        e->action = 5; // Sequência climática de morte
                        e->bossDeathTimer = 0;
                        hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                    }
                    hit_something = true;
                }
            } else if (e->action == 1) {
                // Golpe com a espada enquanto em pé: resvala na borracha gelatinosa sem causar dano
                float bx1 = e->x - 22.0f;
                float by1 = e->y - 36.0f;
                float bx2 = e->x + 22.0f;
                float by2 = e->y + 16.0f;
                if (sx1 < bx2 && sx2 > bx1 && sy1 < by2 && sy2 > by1) {
                    hal_audio_play_sound(SOUND_SWORD_HIT, 0.75f, 0.70f);
                    hit_something = true;
                }
            }
        }
    }

    return hit_something;
}

bool entity_check_subweapon_hit(float px, float py, float pw, float ph, int damage, int* out_carried_item) {
    if (out_carried_item) *out_carried_item = 0;

    float px1 = px;
    float py1 = py;
    float px2 = px + pw;
    float py2 = py + ph;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active) continue;

        // 1. Coleta de Itens à distância pelo Bumerangue
        if (e->type == ENTITY_ITEM_RUPEE || e->type == ENTITY_ITEM_HEART) {
            float ix1 = e->x;
            float iy1 = e->y;
            float ix2 = ix1 + 10.0f;
            float iy2 = iy1 + 10.0f;

            if (px1 < ix2 && px2 > ix1 && py1 < iy2 && py2 > iy1) {
                if (out_carried_item) {
                    *out_carried_item = (e->type == ENTITY_ITEM_RUPEE) ? 1 : 2;
                }
                e->is_active = false;
                hal_audio_play_sound(SOUND_SECRET, 0.75f, 1.35f);
                return true;
            }
        }

        // 2. Destruição de Projéteis de Pedra
        else if (e->type == ENTITY_PROJECTILE_ROCK) {
            float rx1 = e->x;
            float ry1 = e->y;
            float rx2 = rx1 + 6.0f;
            float ry2 = ry1 + 6.0f;

            if (px1 < rx2 && px2 > rx1 && py1 < ry2 && py2 > ry1) {
                e->is_active = false;
                hal_audio_play_sound(SOUND_SWORD_HIT, 0.85f, 1.4f);
                return true;
            }
        }

        // 3. Impacto e atordoamento contra Inimigos
        else if ((e->type == ENTITY_ENEMY_OCTOROK ||
                  e->type == ENTITY_ENEMY_KEESE ||
                  (e->type == ENTITY_ENEMY_CHUCHU && e->action > 0)) &&
                 e->invulnerableTimer <= 0) {

            // Keese voando alto demais desvia do projétil
            if (e->type == ENTITY_ENEMY_KEESE && e->z > 16.0f) continue;

            float ex1 = e->x + e->hitbox.offset_x;
            float ey1 = e->y + e->hitbox.offset_y;
            float ex2 = ex1 + e->hitbox.width;
            float ey2 = ey1 + e->hitbox.height;

            if (px1 < ex2 && px2 > ex1 && py1 < ey2 && py2 > ey1) {
                e->health -= damage;
                e->invulnerableTimer = 20;
                e->action = 4; // Knockback / Stun
                e->knockbackTimer = 14;

                float dx = e->x - px;
                float dy = e->y - py;
                float dist = sqrtf(dx * dx + dy * dy);
                float force = (e->type == ENTITY_ENEMY_KEESE) ? 3.8f : 2.6f;

                if (dist > 0.1f) {
                    e->knockbackVx = (dx / dist) * force;
                    e->knockbackVy = (dy / dist) * force;
                } else {
                    e->knockbackVx = force;
                    e->knockbackVy = 0.0f;
                }

                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.25f);
                return true;
            }
        }

        // 4. Ataque com subarma no Chefe Big Green ChuChu quando desabado
        else if (e->type == ENTITY_BOSS_BIG_CHUCHU && e->action == 3 && e->invulnerableTimer <= 0) {
            float bx1 = e->x - 22.0f;
            float by1 = e->y - 12.0f;
            float bx2 = e->x + 22.0f;
            float by2 = e->y + 16.0f;

            if (px1 < bx2 && px2 > bx1 && py1 < by2 && py2 > by1) {
                e->health -= damage;
                e->invulnerableTimer = 22;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.15f);
                entity_trigger_screen_shake(6, 2);
                if (e->health <= 0) {
                    e->health = 0;
                    e->action = 5;
                    e->bossDeathTimer = 0;
                    hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                }
                return true;
            }
        }
    }
    return false;
}

bool entity_apply_gust_suction(float jar_x, float jar_y, Direction dir, float range, float pull_force) {
    bool absorbed_something = false;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type == ENTITY_NPC_FOREST_MINISH) continue;

        float dx = e->x - jar_x;
        float dy = e->y - jar_y;
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist > range || dist < 1.0f) continue;

        // Verificação do cone cônico de sucção direcionado
        bool in_cone = false;
        if (dir == DIR_DOWN  && dy > 0.0f  && fabsf(dx) <= (dy * 0.75f + 8.0f)) in_cone = true;
        if (dir == DIR_UP    && dy < 0.0f  && fabsf(dx) <= (-dy * 0.75f + 8.0f)) in_cone = true;
        if (dir == DIR_RIGHT && dx > 0.0f  && fabsf(dy) <= (dx * 0.75f + 8.0f)) in_cone = true;
        if (dir == DIR_LEFT  && dx < 0.0f  && fabsf(dy) <= (-dx * 0.75f + 8.0f)) in_cone = true;

        if (!in_cone) continue;

        float pull_x = -(dx / dist) * pull_force;
        float pull_y = -(dy / dist) * pull_force;

        // Atração de itens no chão
        if (e->type == ENTITY_ITEM_RUPEE || e->type == ENTITY_ITEM_HEART) {
            e->x += pull_x * 1.6f;
            e->y += pull_y * 1.6f;
        }
        // Absorção de projéteis de pedras cuspidas
        else if (e->type == ENTITY_PROJECTILE_ROCK) {
            e->x += pull_x * 2.2f;
            e->y += pull_y * 2.2f;
            if (dist < 12.0f) {
                e->is_active = false; // Engolido pelo jarro!
                absorbed_something = true;
            }
        }
        // Desenterrar ChuChu da poça e puxar inimigos
        else if (e->type == ENTITY_ENEMY_CHUCHU) {
            if (e->action == 0) {
                e->action = 1; // Forçado a brotar!
                e->aiTimer = 16;
                hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.8f, 1.2f);
            }
            e->x += pull_x * 0.75f;
            e->y += pull_y * 0.75f;
        }
        // Puxar Octorok e Keese
        else if (e->type == ENTITY_ENEMY_OCTOROK || e->type == ENTITY_ENEMY_KEESE) {
            e->x += pull_x * 0.85f;
            e->y += pull_y * 0.85f;
        }
        // Sucção na base / pés do Chefe Big Green ChuChu
        else if (e->type == ENTITY_BOSS_BIG_CHUCHU) {
            if (e->action == 1 || e->action == 2) {
                e->action = 2; // Estado de sucção ativa
                e->bossSuctionTimer += 1;
                e->bossBaseScale = 1.0f - (float)e->bossSuctionTimer / 55.0f;
                if (e->bossBaseScale < 0.15f) e->bossBaseScale = 0.15f;

                // Leve puxão físico em direção ao jarro
                e->x += pull_x * 0.22f;
                e->y += pull_y * 0.22f;
                absorbed_something = true;
            }
        }
    }

    return absorbed_something;
}

// ----------------------------------------------------------------------------
// RENDERIZADOR DE SPRITES DAS ENTIDADES
// ----------------------------------------------------------------------------
void entity_manager_render(const Camera* cam) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active) continue;

        int sx, sy;
        map_world_to_screen(cam, e->x, e->y, &sx, &sy);

        // Frustum culling básico de entidades (não desenha se estiver fora da tela)
        if (sx < -20 || sx > cam->viewport_w + 20 || sy < -20 || sy > cam->viewport_h + 20) {
            continue;
        }

        // 1. OCTOROK VERMELHO
        if (e->type == ENTITY_ENEMY_OCTOROK) {
            // Piscar quando atingido por dano (flicker)
            if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 0)) {
                continue;
            }

            if (s_octo_tex && s_octo_tex->pixels) {
                int src_x = 0;
                int src_y = 0;
                bool flip_h = false;

                if (e->action == 2) {
                    // Antecipação de tiro (bochechas inchadas)
                    if (e->dir == DIR_DOWN) {
                        src_x = 0; src_y = 16;
                    } else if (e->dir == DIR_UP) {
                        src_x = 16; src_y = 0;
                    } else if (e->dir == DIR_RIGHT) {
                        src_x = 32; src_y = 16;
                    } else if (e->dir == DIR_LEFT) {
                        src_x = 32; src_y = 16;
                        flip_h = true;
                    }
                } else {
                    // Movimento / Patrulha (alterna passos)
                    if (e->dir == DIR_DOWN) {
                        src_x = 0; src_y = 0;
                    } else if (e->dir == DIR_UP) {
                        src_x = 16; src_y = 0;
                    } else if (e->dir == DIR_RIGHT) {
                        src_x = (e->animFrame == 0) ? 32 : 48;
                        src_y = 0;
                    } else if (e->dir == DIR_LEFT) {
                        src_x = (e->animFrame == 0) ? 32 : 48;
                        src_y = 0;
                        flip_h = true;
                    }
                }

                texture_draw_ex(s_octo_tex, src_x, src_y, 16, 16, sx, sy, flip_h);
            } else {
                // Fallback Procedural caso o arquivo octorok.bmp não esteja presente
                u32 red_body   = 0xDE3030FF;
                u32 dark_red   = 0x991818FF;
                u32 highlight  = 0xF07070FF;
                u32 eye_white  = 0xFFFFFFFF;
                u32 eye_pupil  = 0x111111FF;

                int puff = (e->action == 2) ? 1 : 0; // Inchaço de disparo

                draw_filled_rect(sx + 3 - puff, sy + 3 - puff, 10 + puff * 2, 8 + puff * 2, red_body);
                draw_filled_rect(sx + 4, sy + 2, 8, 2, highlight);
                draw_filled_rect(sx + 3, sy + 11, 10, 2, dark_red);

                if (e->dir == DIR_DOWN) {
                    draw_filled_rect(sx + 4, sy + 5, 2, 3, eye_white);
                    draw_filled_rect(sx + 10, sy + 5, 2, 3, eye_white);
                    put_pixel_safe(sx + 5, sy + 6, eye_pupil);
                    put_pixel_safe(sx + 11, sy + 6, eye_pupil);
                    draw_filled_rect(sx + 6, sy + 8, 4, 3, dark_red);
                } else if (e->dir == DIR_UP) {
                    draw_filled_rect(sx + 4, sy + 4, 8, 3, dark_red);
                } else if (e->dir == DIR_LEFT) {
                    draw_filled_rect(sx + 4, sy + 5, 3, 3, eye_white);
                    put_pixel_safe(sx + 4, sy + 6, eye_pupil);
                    draw_filled_rect(sx + 1 - puff, sy + 6, 3 + puff, 3, dark_red);
                } else if (e->dir == DIR_RIGHT) {
                    draw_filled_rect(sx + 9, sy + 5, 3, 3, eye_white);
                    put_pixel_safe(sx + 11, sy + 6, eye_pupil);
                    draw_filled_rect(sx + 12, sy + 6, 3 + puff, 3, dark_red);
                }

                if (e->animFrame == 0) {
                    draw_filled_rect(sx + 3, sy + 13, 3, 2, dark_red);
                    draw_filled_rect(sx + 10, sy + 13, 3, 2, dark_red);
                } else {
                    draw_filled_rect(sx + 5, sy + 13, 3, 2, dark_red);
                    draw_filled_rect(sx + 8, sy + 13, 3, 2, dark_red);
                }
            }
        }

        // 2. PROJÉTIL: PEDRA
        else if (e->type == ENTITY_PROJECTILE_ROCK) {
            if (s_octo_tex && s_octo_tex->pixels) {
                // Sprite canônico da pedra: 8x8 pixels em (48, 32)
                texture_draw(s_octo_tex, 48, 32, 8, 8, sx + 4, sy + 4);
            } else {
                u32 rock_color = 0x8A5528FF;
                u32 rock_dark  = 0x4D2E14FF;
                draw_filled_rect(sx + 1, sy + 1, 4, 4, rock_color);
                put_pixel_safe(sx + 2, sy + 2, 0xC4864DFF); // Brilho
                put_pixel_safe(sx + 4, sy + 4, rock_dark);
            }
        }

        // 3. ITEM: RUPEE VERDE
        else if (e->type == ENTITY_ITEM_RUPEE) {
            int bob = (int)(sinf((float)e->animTimer * 0.15f) * 2.0f);
            int ry = sy + bob;
            u32 green_main  = 0x00FF88FF;
            u32 green_dark  = 0x009944FF;
            u32 green_light = 0x88FFAAFF;

            draw_filled_rect(sx + 2, ry + 2, 5, 7, green_main);
            draw_filled_rect(sx + 3, ry + 1, 3, 1, green_main);
            draw_filled_rect(sx + 3, ry + 9, 3, 1, green_main);
            put_pixel_safe(sx + 3, ry + 3, green_light); // Reflexo
            draw_filled_rect(sx + 5, ry + 4, 2, 4, green_dark);
        }

        // 4. ITEM: CORAÇÃO DE CURA
        else if (e->type == ENTITY_ITEM_HEART) {
            int bob = (int)(sinf((float)e->animTimer * 0.18f) * 2.5f);
            int hy = sy + bob;
            u32 red   = 0xE62222FF;
            u32 white = 0xFFFFFFFF;

            put_pixel_safe(sx + 1, hy, red);
            put_pixel_safe(sx + 2, hy, red);
            put_pixel_safe(sx + 4, hy, red);
            put_pixel_safe(sx + 5, hy, red);

            draw_filled_rect(sx, hy + 1, 7, 3, red);
            put_pixel_safe(sx + 1, hy + 1, white); // Brilho

            for (int x = 1; x <= 5; x++) put_pixel_safe(sx + x, hy + 4, red);
            for (int x = 2; x <= 4; x++) put_pixel_safe(sx + x, hy + 5, red);
            put_pixel_safe(sx + 3, hy + 6, red);
        }

        // 5. NPC AMIGÁVEL: MINISH DA FLORESTA
        else if (e->type == ENTITY_NPC_FOREST_MINISH) {
            int breathe = ((e->animTimer / 16) % 2 == 1) ? 1 : 0;
            int my = sy - breathe;

            // Paleta Minish
            u32 c_hat   = 0xC93B2BFF; // Gorro vermelho bolota
            u32 c_skin  = 0xFDE8CDFF; // Pele clara
            u32 c_tunic = 0x2A6F97FF; // Túnica azul
            u32 c_pom   = 0xFFFFFFFF; // Pom-pom branco
            u32 c_eye   = 0x111111FF; // Olhos pretos

            // Gorro pontudo (topo)
            draw_filled_rect(sx + 6, my + 1, 4, 3, c_hat);
            draw_filled_rect(sx + 5, my + 4, 6, 3, c_hat);
            put_pixel_safe(sx + 7, my, c_pom);
            put_pixel_safe(sx + 8, my, c_pom);

            // Rosto e orelhas pontudas
            draw_filled_rect(sx + 5, my + 7, 6, 4, c_skin);
            put_pixel_safe(sx + 4, my + 8, c_skin); // Orelha esq
            put_pixel_safe(sx + 11, my + 8, c_skin); // Orelha dir

            // Olhos
            if (e->dir == DIR_DOWN) {
                put_pixel_safe(sx + 6, my + 8, c_eye);
                put_pixel_safe(sx + 9, my + 8, c_eye);
            } else if (e->dir == DIR_UP) {
                // De costas: gorro cobre o rosto
                draw_filled_rect(sx + 5, my + 7, 6, 4, c_hat);
            } else if (e->dir == DIR_LEFT) {
                put_pixel_safe(sx + 5, my + 8, c_eye);
            } else if (e->dir == DIR_RIGHT) {
                put_pixel_safe(sx + 10, my + 8, c_eye);
            }

            // Túnica e corpo
            draw_filled_rect(sx + 5, my + 11, 6, 4, c_tunic);

            // Balão de Fusão de Kinstone sobre a cabeça do Minish (se ainda não fundiu)
            if (e->hasKinstone && !e->kinstoneFused) {
                int bubble_y = my - 16 + (int)(sinf(e->bubbleBob) * 2.0f);
                int bubble_x = sx + 8;

                // Nuvem do balão de pensamento branca
                draw_filled_rect(bubble_x - 7, bubble_y - 6, 14, 12, 0xFFFFFFFF);
                draw_filled_rect(bubble_x - 8, bubble_y - 4, 16, 8, 0xFFFFFFFF);
                draw_filled_rect(bubble_x - 6, bubble_y - 7, 12, 14, 0xFFFFFFFF);
                put_pixel_safe(bubble_x - 8, bubble_y - 4, 0xD4AF37FF);
                put_pixel_safe(bubble_x + 7, bubble_y - 4, 0xD4AF37FF);
                put_pixel_safe(bubble_x - 2, my - 2, 0xFFFFFFFF);
                put_pixel_safe(bubble_x - 1, my - 3, 0xFFFFFFFF);

                // Fragmento de Kinstone dentro do balão
                u32 c_kinstone = (e->kinstoneType == 0) ? 0x22C55EFF :
                                 (e->kinstoneType == 1) ? 0x3B82F6FF : 0xEF4444FF;
                u32 c_kgold = 0xD4AF37FF;

                for (int ky = -3; ky <= 3; ky++) {
                    for (int kx = -3; kx <= 3; kx++) {
                        if (kx * kx + ky * ky <= 9) {
                            u32 col = (kx == 3 || kx == -3 || ky == 3 || ky == -3) ? c_kgold : c_kinstone;
                            put_pixel_safe(bubble_x + kx, bubble_y + ky, col);
                        }
                    }
                }
                put_pixel_safe(bubble_x - 1, bubble_y - 1, 0xFFFFFFFF);
            }

            // Se o Link estiver ao alcance da interação (<= 28px)
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 28.0f * 28.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 26;
                int prompt_y = (e->hasKinstone && !e->kinstoneFused) ? (my - 28 + bounce) : (sy - 14 + bounce);

                if (e->hasKinstone && !e->kinstoneFused) {
                    draw_filled_rect(prompt_x - 1, prompt_y - 1, 68, 10, 0x071520F0);
                    font_draw_text(prompt_x + 1, prompt_y, "[K/L] Fusao", 0x00FFCCFF, true);
                } else {
                    draw_filled_rect(prompt_x + 10 - 1, prompt_y - 1, 48, 10, 0x0A2010EE);
                    font_draw_text(prompt_x + 10 + 1, prompt_y, "[A] Falar", 0xFFE27AFF, true);
                }
            }
        }

        // 6. INIMIGO: KEESE (MORCEGO VOADOR COM SOMBRA PROJETADA)
        else if (e->type == ENTITY_ENEMY_KEESE) {
            // Sombra oval no chão (projeção de altitude 3D no terreno)
            draw_filled_rect(sx + 3, sy + 12, 10, 3, 0x05100766);
            draw_filled_rect(sx + 4, sy + 11, 8, 4, 0x05100766);

            // Piscar ao receber dano da espada
            if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 0)) {
                continue;
            }

            int by = sy - (int)e->z;

            u32 c_body  = 0x2A0E3DFF; // Roxo escuro do corpo
            u32 c_wing  = 0x58247DFF; // Asas violeta
            u32 c_rib   = 0x8239B5FF; // Nervuras das asas
            u32 c_eye   = 0xFFD700FF; // Olhos dourados brilhantes
            u32 c_pupil = 0xEE1100FF; // Íris vermelha

            // Corpo e cabeça central do morcego
            draw_filled_rect(sx + 6, by + 4, 4, 6, c_body);
            put_pixel_safe(sx + 6, by + 3, c_body); // Orelha esq
            put_pixel_safe(sx + 9, by + 3, c_body); // Orelha dir

            // Olhos brilhantes
            put_pixel_safe(sx + 6, by + 5, c_eye);
            put_pixel_safe(sx + 9, by + 5, c_eye);
            put_pixel_safe(sx + 7, by + 6, c_pupil);
            put_pixel_safe(sx + 8, by + 6, c_pupil);

            // Presas brancas
            put_pixel_safe(sx + 6, by + 8, 0xFFFFFFFF);
            put_pixel_safe(sx + 9, by + 8, 0xFFFFFFFF);

            // Animação de bater asas (Wings Up / Wings Down)
            bool wings_up = ((e->animTimer / 6) % 2 == 0);
            if (wings_up) {
                // Asas apontadas para cima
                draw_filled_rect(sx + 2, by + 1, 4, 4, c_wing);
                draw_filled_rect(sx + 10, by + 1, 4, 4, c_wing);
                put_pixel_safe(sx + 1, by, c_rib);
                put_pixel_safe(sx + 14, by, c_rib);
            } else {
                // Asas apontadas para baixo/horizontal
                draw_filled_rect(sx + 1, by + 5, 5, 4, c_wing);
                draw_filled_rect(sx + 10, by + 5, 5, 4, c_wing);
                put_pixel_safe(sx, by + 8, c_rib);
                put_pixel_safe(sx + 15, by + 8, c_rib);
            }
        }

        // 7. INIMIGO: GREEN CHUCHU (GOSMA GELATINOSA)
        else if (e->type == ENTITY_ENEMY_CHUCHU) {
            // Piscar ao receber dano
            if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 0)) {
                continue;
            }

            u32 c_jelly = 0x26C437FF; // Verde translúcido
            u32 c_dark  = 0x146B1EFF; // Base escura
            u32 c_shine = 0x88FFAAFF; // Brilho de gelatina
            u32 c_white = 0xFFFFFFFF; // Olhos brancos
            u32 c_pupil = 0x111111FF; // Pupilas

            // Estado 0: Poça camuflada no solo
            if (e->action == 0) {
                draw_filled_rect(sx + 2, sy + 13, 12, 3, c_jelly);
                draw_filled_rect(sx + 4, sy + 12, 8, 1, c_shine);
                put_pixel_safe(sx + 3, sy + 14, c_dark);
                put_pixel_safe(sx + 12, sy + 14, c_dark);
            }
            // Estado 1: Emergindo da terra
            else if (e->action == 1) {
                int h = 4 + (20 - e->aiTimer) / 2;
                if (h > 12) h = 12;
                draw_filled_rect(sx + 3, sy + 16 - h, 10, h, c_jelly);
                draw_filled_rect(sx + 5, sy + 16 - h, 6, 2, c_shine);
            }
            // Estado 2: Agachado (Squash) preparando salto
            else if (e->action == 2 || e->action == 4) {
                int wobble = (e->action == 4) ? ((e->animTimer % 2 == 0) ? 1 : -1) : 0;
                draw_filled_rect(sx + 1 + wobble, sy + 7, 14, 8, c_jelly);
                draw_filled_rect(sx + 2 + wobble, sy + 14, 12, 2, c_dark);
                draw_filled_rect(sx + 3 + wobble, sy + 6, 10, 2, c_shine);

                // Olhos cômicos esbugalhados
                draw_filled_rect(sx + 4 + wobble, sy + 8, 3, 4, c_white);
                draw_filled_rect(sx + 9 + wobble, sy + 8, 3, 4, c_white);
                put_pixel_safe(sx + 5 + wobble, sy + 9, c_pupil);
                put_pixel_safe(sx + 10 + wobble, sy + 9, c_pupil);
            }
            // Estado 3: Salto balístico no ar
            else if (e->action == 3) {
                // Sombra no chão durante o salto
                draw_filled_rect(sx + 3, sy + 13, 10, 3, 0x05100766);

                int cy = sy - (int)e->z;
                // Formato alongado de gota d'água / lágrima
                draw_filled_rect(sx + 3, cy + 2, 10, 13, c_jelly);
                draw_filled_rect(sx + 5, cy, 6, 3, c_shine);
                draw_filled_rect(sx + 4, cy + 14, 8, 2, c_dark);

                // Olhos abertos no ar
                draw_filled_rect(sx + 4, cy + 5, 3, 4, c_white);
                draw_filled_rect(sx + 9, cy + 5, 3, 4, c_white);
                put_pixel_safe(sx + 5, cy + 7, c_pupil);
                put_pixel_safe(sx + 10, cy + 7, c_pupil);
            }
        }

        // 7. BAÚ DO TESOURO DOURADO (DESTRAVADO POR FUSÃO DE KINSTONE)
        else if (e->type == ENTITY_CHEST_GOLD) {
            u32 c_gold_base = 0xD4AF37FF; // Ouro principal
            u32 c_gold_hi   = 0xFFE27AFF; // Brilho dourado
            u32 c_gold_dk   = 0x9A7B1CFF; // Sombra dourada
            u32 c_wood      = 0x4A2810FF; // Detalhes de mogno escuro
            u32 c_lock      = 0x111111FF; // Fechadura
            u32 c_sparkle   = 0xFFFFFFFF; // Brilho de diamante

            if (e->action == 0) {
                // BAÚ FECHADO (14x12 pixels)
                // Sombra no chão
                draw_filled_rect(sx + 1, sy + 12, 14, 3, 0x05100766);

                // Tampa arredondada dourada
                draw_filled_rect(sx + 2, sy + 2, 12, 4, c_gold_base);
                draw_filled_rect(sx + 3, sy + 1, 10, 2, c_gold_hi);

                // Corpo do baú
                draw_filled_rect(sx + 1, sy + 5, 14, 7, c_gold_base);
                draw_filled_rect(sx + 1, sy + 11, 14, 2, c_gold_dk);

                // Cintas de ferro/madeira escura
                draw_filled_rect(sx + 3, sy + 2, 2, 10, c_wood);
                draw_filled_rect(sx + 11, sy + 2, 2, 10, c_wood);

                // Placa da fechadura central
                draw_filled_rect(sx + 7, sy + 6, 2, 3, c_lock);
                put_pixel_safe(sx + 7, sy + 7, c_gold_hi);

                // Brilho cintilante animado no topo
                if ((e->animTimer / 12) % 4 == 0) {
                    put_pixel_safe(sx + 4, sy + 1, c_sparkle);
                    put_pixel_safe(sx + 5, sy + 1, c_sparkle);
                }

                // Prompt interativo se Link estiver próximo (<= 24px)
                float cdx = s_last_link_x - e->x;
                float cdy = s_last_link_y - e->y;
                if (cdx * cdx + cdy * cdy <= 24.0f * 24.0f) {
                    int p_x = sx - 16;
                    int p_y = sy - 14;
                    draw_filled_rect(p_x - 1, p_y - 1, 48, 10, 0x0A2010EE);
                    font_draw_text(p_x + 1, p_y, "[A] Abrir", 0xFFE27AFF, true);
                }
            } else {
                // BAÚ ABERTO (Interior de veludo escarlate e tampa erguida)
                // Sombra no chão
                draw_filled_rect(sx + 1, sy + 12, 14, 3, 0x05100766);

                // Tampa erguida para trás
                draw_filled_rect(sx + 2, sy - 3, 12, 4, c_gold_base);
                draw_filled_rect(sx + 3, sy - 4, 10, 2, c_gold_hi);

                // Interior reluzente de veludo vermelho
                draw_filled_rect(sx + 2, sy + 1, 12, 5, 0x881111FF);
                draw_filled_rect(sx + 4, sy + 2, 8, 3, 0xCC2222FF);

                // Base dourada
                draw_filled_rect(sx + 1, sy + 6, 14, 6, c_gold_base);
                draw_filled_rect(sx + 1, sy + 11, 14, 2, c_gold_dk);
                draw_filled_rect(sx + 3, sy + 6, 2, 6, c_wood);
                draw_filled_rect(sx + 11, sy + 6, 2, 6, c_wood);
            }
        }

        // 8. ITEM: HEART CONTAINER (RECIPIENTE DE CORAÇÃO PERMANENTE)
        else if (e->type == ENTITY_ITEM_HEART_CONTAINER) {
            int bob = (int)(sinf((float)e->animTimer * 0.12f) * 3.0f);
            int hy = sy + bob;

            // Sombra oval suave no chão
            draw_filled_rect(sx - 7, sy + 8, 14, 3, 0x05100766);
            draw_filled_rect(sx - 5, sy + 7, 10, 5, 0x05100766);

            // Moldura externa circular dourada com garras (16x16)
            u32 c_gold_dark  = 0x9A7B1CFF;
            u32 c_gold_base  = 0xF59E0BFF;
            u32 c_gold_light = 0xFDE047FF;
            u32 c_heart_red  = 0xEF4444FF;
            u32 c_heart_dark = 0x991B1BFF;
            u32 c_sparkle    = 0xFFFFFFFF;

            // Anel do receptáculo dourado
            draw_filled_rect(sx - 8, hy - 4, 16, 12, c_gold_dark);
            draw_filled_rect(sx - 7, hy - 5, 14, 14, c_gold_base);
            draw_filled_rect(sx - 5, hy - 7, 10, 18, c_gold_base);
            draw_filled_rect(sx - 6, hy - 4, 12, 12, c_gold_light);

            // Coração vermelho pulsante dentro do receptáculo
            int pulse = ((e->animTimer / 15) % 2 == 0) ? 1 : 0;
            draw_filled_rect(sx - 4 - pulse, hy - 2 - pulse, 8 + pulse * 2, 7 + pulse, c_heart_red);
            draw_filled_rect(sx - 3, hy - 4, 3, 2, c_heart_red);
            draw_filled_rect(sx + 1, hy - 4, 3, 2, c_heart_red);
            draw_filled_rect(sx - 2, hy + 5, 4, 2, c_heart_dark);
            draw_filled_rect(sx - 1, hy + 7, 2, 2, c_heart_dark);

            // Brilho cintilante no coração
            put_pixel_safe(sx - 2, hy - 2, c_sparkle);
            put_pixel_safe(sx - 1, hy - 3, c_sparkle);

            // Partículas de luz sagrada orbitando o Heart Container
            for (int p = 0; p < 4; p++) {
                float ang = ((float)e->animTimer * 0.08f) + (float)p * (PI_F / 2.0f);
                int px = sx + (int)(cosf(ang) * 12.0f);
                int py = hy + (int)(sinf(ang) * 9.0f);
                put_pixel_safe(px, py, c_gold_light);
            }
        }

        // 9. CHEFE: BIG GREEN CHUCHU (COLOSSO GELATINOSO DE DEEPWOOD SHRINE)
        else if (e->type == ENTITY_BOSS_BIG_CHUCHU) {
            // Piscar quando atingido por dano da espada (flicker)
            if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 0)) {
                // Durante o dano, pisca frame alternado
                continue;
            }

            int boss_cx = sx;
            int boss_cy = sy - (int)e->z;

            // Paleta de cores autêntica do ChuChu Gigante
            u32 c_jelly_base = 0x16A34AFF; // Verde esmeralda translúcido
            u32 c_jelly_dark = 0x14532DFF; // Sombra inferior densa
            u32 c_jelly_lite = 0x4ADE80FF; // Gelatina iluminada
            u32 c_jelly_mint = 0x86EFACFF; // Destaque suave
            u32 c_white      = 0xFFFFFFFF; // Brilho especular e olhos
            u32 c_pupil      = e->bossEnraged ? 0xEF4444FF : 0x111111FF; // Pupilas normais ou enfurecidas
            u32 c_iris       = e->bossEnraged ? 0xF87171FF : 0x334155FF;

            // Se estiver sofrendo dano, tintura vermelha/branca
            if (e->invulnerableTimer > 0) {
                c_jelly_base = 0xEF4444FF;
                c_jelly_lite = 0xFCA5A5FF;
                c_jelly_dark = 0x991B1BFF;
            }

            // Sombra no chão (escala com a altitude Z)
            float shadow_scale = 1.0f - (e->z / 60.0f);
            if (shadow_scale < 0.35f) shadow_scale = 0.35f;
            int sw = (int)(44.0f * shadow_scale);
            int sh = (int)(14.0f * shadow_scale);
            draw_filled_rect(sx - sw / 2, sy + 10, sw, sh, 0x05100766);
            draw_filled_rect(sx - (sw - 8) / 2, sy + 9, sw - 8, sh + 2, 0x05100766);

            // AÇÃO 3: Desabado no chão (Toppled & Vulnerable)
            if (e->action == 3) {
                // Poça espalmada no piso (58x24 px)
                int pw = 58;
                int ph = 24;
                int wobble = (int)(sinf((float)e->animTimer * 0.25f) * 2.0f);
                pw += wobble;

                draw_filled_rect(boss_cx - pw / 2, boss_cy - 8, pw, ph, c_jelly_dark);
                draw_filled_rect(boss_cx - (pw - 6) / 2, boss_cy - 12, pw - 6, ph, c_jelly_base);
                draw_filled_rect(boss_cx - (pw - 14) / 2, boss_cy - 16, pw - 14, ph - 4, c_jelly_lite);
                draw_filled_rect(boss_cx - 16, boss_cy - 14, 12, 4, c_jelly_mint);
                draw_filled_rect(boss_cx + 4, boss_cy - 14, 12, 4, c_jelly_mint);

                // Núcleo gelatinoso vulnerável exposto (pulsando)
                int n_pulse = (int)(sinf((float)e->animTimer * 0.35f) * 2.0f);
                draw_filled_rect(boss_cx - 8 - n_pulse, boss_cy - 4 - n_pulse, 16 + n_pulse * 2, 12 + n_pulse * 2, 0xA3E635FF);
                draw_filled_rect(boss_cx - 4, boss_cy - 2, 8, 8, 0xBEF264FF);
                draw_filled_rect(boss_cx - 2, boss_cy, 4, 4, c_white);

                // Olhos tontos / em espiral (Dizzy eyes)
                int eye_y = boss_cy - 4;
                int ex1 = boss_cx - 14;
                int ex2 = boss_cx + 8;
                draw_filled_rect(ex1, eye_y, 7, 7, c_white);
                draw_filled_rect(ex2, eye_y, 7, 7, c_white);

                // Cruz/espiral de tontura nos olhos
                int spin = (e->animTimer / 6) % 4;
                int ox[4] = { 2, 4, 2, 0 };
                int oy[4] = { 0, 2, 4, 2 };
                draw_filled_rect(ex1 + ox[spin], eye_y + oy[spin], 3, 3, c_pupil);
                draw_filled_rect(ex2 + ox[(spin + 2) % 4], eye_y + oy[(spin + 2) % 4], 3, 3, c_pupil);
            }
            // AÇÃO 5: Explosão de Morte
            else if (e->action == 5) {
                // Boss inflando e piscando cores cromáticas antes de estourar
                int expand = e->bossDeathTimer / 3;
                int bw = 48 + expand;
                int bh = 60 + expand;

                u32 death_colors[4] = { 0xFFFFFFFF, 0x22C55EFF, 0xEAB308FF, 0xEC4899FF };
                u32 flash_col = death_colors[(e->bossDeathTimer / 4) % 4];

                draw_filled_rect(boss_cx - bw / 2, boss_cy - bh + 16, bw, bh, flash_col);

                // Gotículas de gosma verde explodindo radialmente
                for (int p = 0; p < 16; p++) {
                    float ang = (float)p * (PI_F / 8.0f);
                    float spd = (float)e->bossDeathTimer * 1.3f;
                    int px = boss_cx + (int)(cosf(ang) * spd);
                    int py = boss_cy - 20 + (int)(sinf(ang) * spd * 0.8f);
                    draw_filled_rect(px - 2, py - 2, 4, 4, 0x22C55EFF);
                    put_pixel_safe(px - 1, py - 1, 0x86EFACFF);
                }
            }
            // AÇÃO 1, 2 e 4: Em pé / Pulando / Sendo sugado
            else {
                // Cálculo de Squash & Stretch dinâmico
                int bw = 46;
                int bh = 62;

                if (e->z > 2.0f) {
                    // No ar: estica verticalmente (Stretch)
                    bw = 38;
                    bh = 68;
                } else if (e->subAction == 0 && e->aiTimer < 14) {
                    // Agachando antes de saltar (Squash)
                    bw = 54;
                    bh = 50;
                }

                // Efeito do Pote Mágico sugando a base:
                // Wobble lateral violento
                int wobble_x = 0;
                if (e->action == 2) {
                    wobble_x = (int)(sinf((float)e->animTimer * 0.55f) * 6.0f);
                }

                int draw_x = boss_cx + wobble_x;
                int draw_y = boss_cy;

                // Desenho do Corpo Gelatinoso Gigante (3 camadas de profundidade)
                int body_top = draw_y - bh + 16;

                // 1. Cúpula e Base Externa
                draw_filled_rect(draw_x - bw / 2, body_top, bw, bh, c_jelly_dark);
                draw_filled_rect(draw_x - (bw - 6) / 2, body_top - 4, bw - 6, bh + 4, c_jelly_dark);

                // 2. Volume Interno Translúcido
                draw_filled_rect(draw_x - (bw - 4) / 2, body_top + 2, bw - 4, bh - 6, c_jelly_base);
                draw_filled_rect(draw_x - (bw - 10) / 2, body_top - 2, bw - 10, bh, c_jelly_lite);

                // 3. Brilho especular curvado do topo gelatinoso
                draw_filled_rect(draw_x - 14, body_top + 2, 8, 6, c_jelly_mint);
                draw_filled_rect(draw_x - 12, body_top + 4, 4, 3, c_white);

                // Pés / Base: largura escala com bossBaseScale!
                int base_w = (int)((float)bw * e->bossBaseScale);
                if (base_w < 8) base_w = 8;
                draw_filled_rect(draw_x - base_w / 2, draw_y + 10, base_w, 6, c_jelly_dark);
                draw_filled_rect(draw_x - (base_w - 4) / 2, draw_y + 8, base_w - 4, 4, c_jelly_base);

                // Olhos Gigantescos Expressivos (Rastreiam a posição do Link)
                int eye_center_y = body_top + 22;
                int eye_spacing  = 12;

                // Direção do olhar para o Link
                float edx = s_last_link_x - e->x;
                float edy = s_last_link_y - e->y;
                int look_ox = (edx > 15.0f) ? 2 : ((edx < -15.0f) ? -2 : 0);
                int look_oy = (edy > 15.0f) ? 2 : ((edy < -15.0f) ? -2 : 0);

                // Olho Esquerdo
                int ex1 = draw_x - eye_spacing - 4;
                draw_filled_rect(ex1 - 1, eye_center_y - 1, 9, 11, c_jelly_dark);
                draw_filled_rect(ex1, eye_center_y, 7, 9, c_white);
                draw_filled_rect(ex1 + 1 + look_ox, eye_center_y + 2 + look_oy, 4, 5, c_pupil);
                put_pixel_safe(ex1 + 2 + look_ox, eye_center_y + 2 + look_oy, c_iris);
                put_pixel_safe(ex1 + 1, eye_center_y + 1, c_white); // Reflexo

                // Olho Direito
                int ex2 = draw_x + eye_spacing - 4;
                draw_filled_rect(ex2 - 1, eye_center_y - 1, 9, 11, c_jelly_dark);
                draw_filled_rect(ex2, eye_center_y, 7, 9, c_white);
                draw_filled_rect(ex2 + 1 + look_ox, eye_center_y + 2 + look_oy, 4, 5, c_pupil);
                put_pixel_safe(ex2 + 2 + look_ox, eye_center_y + 2 + look_oy, c_iris);
                put_pixel_safe(ex2 + 1, eye_center_y + 1, c_white); // Reflexo

                // Sobrancelhas furiosas se em Fase 2 (Enraged)
                if (e->bossEnraged) {
                    for (int b = 0; b < 6; b++) {
                        put_pixel_safe(ex1 + b, eye_center_y - 2 + (b / 2), 0x991B1BFF);
                        put_pixel_safe(ex2 + 5 - b, eye_center_y - 2 + (b / 2), 0x991B1BFF);
                    }
                }
            }

            // HUD DE VIDA DO CHEFE (BARRA DE BOSS CANÔNICA GBA)
            if (e->action != 5 && e->health > 0) {
                int bar_w = 84;
                int bar_h = 7;
                int bar_x = (cam->viewport_w - bar_w) / 2;
                int bar_y = 18;

                // Fundo preto e moldura dourada
                draw_filled_rect(bar_x - 1, bar_y - 1, bar_w + 2, bar_h + 2, 0x0B0F19FF);
                draw_filled_rect(bar_x, bar_y, bar_w, bar_h, 0x1E293BFF);

                // Segmentos de vida (10 slots de HP)
                int pip_w = 7;
                for (int hp = 0; hp < e->maxHealth; hp++) {
                    int px = bar_x + 2 + (hp * 8);
                    u32 pip_col = (hp < e->health) ? (e->bossEnraged ? 0xEF4444FF : 0x22C55EFF) : 0x475569FF;
                    draw_filled_rect(px, bar_y + 1, pip_w, bar_h - 2, pip_col);
                    if (hp < e->health) {
                        put_pixel_safe(px + 1, bar_y + 2, 0xFFFFFFFF); // Brilho no pip
                    }
                }

                // Nome do Chefe centralizado
                const char* boss_title = e->bossEnraged ? "BIG CHUCHU (FURIOSO)" : "BIG GREEN CHUCHU";
                font_draw_text(bar_x + 6, bar_y - 8, boss_title, e->bossEnraged ? 0xF87171FF : 0x4ADE80FF, true);
            }
        }
    }
}

Entity* entity_find_nearby_npc(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_FOREST_MINISH) continue;

        float dx = e->x - world_x;
        float dy = e->y - world_y;
        float dist_sq = dx * dx + dy * dy;

        if (dist_sq <= best_dist_sq) {
            best_dist_sq = dist_sq;
            best = e;
        }
    }
    return best;
}

Entity* entity_find_kinstone_npc(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_FOREST_MINISH) continue;
        if (!e->hasKinstone || e->kinstoneFused) continue;

        float dx = e->x - world_x;
        float dy = e->y - world_y;
        float dist_sq = dx * dx + dy * dy;

        if (dist_sq <= best_dist_sq) {
            best_dist_sq = dist_sq;
            best = e;
        }
    }
    return best;
}

bool entity_interact_chest(float world_x, float world_y, int* link_rupees, int* link_hearts) {
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_CHEST_GOLD || e->action != 0) continue;

        float dx = e->x - world_x;
        float dy = e->y - world_y;
        if (dx * dx + dy * dy <= 24.0f * 24.0f) {
            e->action = 1; // Baú aberto!
            hal_audio_play_sound(SOUND_CHEST_OPEN, 1.0f, 1.0f);
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);

            // Recompensa lendária do baú de fusão: +100 Rupees e restaura a vida!
            if (link_rupees) *link_rupees += 100;
            if (link_hearts) *link_hearts = 3;

            printf("[BAU DOURADO] Aberto com sucesso! Recompensa: +100 Rupees e Vida Cheia!\n");
            return true;
        }
    }
    return false;
}

void entity_manager_shutdown(void) {
    memset(s_entities, 0, sizeof(s_entities));
    printf("[HAL Entity] Gerenciador de entidades finalizado.\n");
}
