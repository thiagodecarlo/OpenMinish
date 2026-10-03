#include "hal/entity.h"
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

void entity_manager_init(void) {
    memset(s_entities, 0, sizeof(s_entities));
    printf("[HAL Entity] Gerenciador de entidades inicializado (Pool: %d slots).\n", MAX_ENTITIES);
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
                    e->health    = 999;
                    e->maxHealth = 999;
                    e->damage    = 0;
                    e->dir       = DIR_DOWN;
                    e->action    = 1; // Idle/respirando
                    e->z         = 0.0f;
                    e->vz        = 0.0f;
                    e->hitbox    = (Hitbox){ 2.0f, 2.0f, 12.0f, 12.0f };
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

                default:
                    break;
            }
            return e;
        }
    }
    return NULL; // Pool esgotado
}

void entity_manager_update(const Tilemap* map, float link_x, float link_y,
                           int* link_hearts, int* link_rupees,
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

                if (!map_is_solid(map, next_x + 8.0f, next_y + 8.0f)) {
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
                if (map_is_solid(map, check_x, check_y)) {
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
            if (map_is_solid(map, e->x + 3.0f, e->y + 3.0f)) {
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

            // Se o Link estiver ao alcance da interação (<= 28px), exibe o prompt animado "[A] Falar"
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 28.0f * 28.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 16;
                int prompt_y = sy - 14 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 48, 10, 0x0A2010EE);
                font_draw_text(prompt_x + 1, prompt_y, "[A] Falar", 0xFFE27AFF, true);
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

void entity_manager_shutdown(void) {
    memset(s_entities, 0, sizeof(s_entities));
    printf("[HAL Entity] Gerenciador de entidades finalizado.\n");
}
