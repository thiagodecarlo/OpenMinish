#include "hal/entity.h"
#include "hal/video.h"
#include "hal/audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * ============================================================================
 * src/hal/entity.c - Gerenciador de Entidades, IA do Octorok e Sistema de Dano
 * ============================================================================
 */

static Entity s_entities[MAX_ENTITIES];

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

        // Golpeando o Octorok
        if (e->type == ENTITY_ENEMY_OCTOROK && e->invulnerableTimer <= 0) {
            float ox1 = e->x + e->hitbox.offset_x;
            float oy1 = e->y + e->hitbox.offset_y;
            float ox2 = ox1 + e->hitbox.width;
            float oy2 = oy1 + e->hitbox.height;

            if (sx1 < ox2 && sx2 > ox1 && sy1 < oy2 && sy2 > oy1) {
                e->health -= damage;
                e->invulnerableTimer = 18; // Pisca de dano
                e->action = 4; // Knockback
                e->knockbackTimer = 10;

                float force = 3.2f;
                e->knockbackVx = 0.0f;
                e->knockbackVy = 0.0f;
                if (slash_dir == DIR_DOWN)  e->knockbackVy = force;
                if (slash_dir == DIR_UP)    e->knockbackVy = -force;
                if (slash_dir == DIR_LEFT)  e->knockbackVx = -force;
                if (slash_dir == DIR_RIGHT) e->knockbackVx = force;

                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.2f);
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
            u32 red_body   = 0xDE3030FF;
            u32 dark_red   = 0x991818FF;
            u32 highlight  = 0xF07070FF;
            u32 eye_white  = 0xFFFFFFFF;
            u32 eye_pupil  = 0x111111FF;

            // Piscar em branco/amarelo quando atingido por dano
            if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 0)) {
                red_body  = 0xFFFFAAFF;
                dark_red  = 0xFFDD00FF;
                highlight = 0xFFFFFFFF;
            }

            int puff = (e->action == 2) ? 1 : 0; // Inchaço de disparo

            // Corpo redondo
            draw_filled_rect(sx + 3 - puff, sy + 3 - puff, 10 + puff * 2, 8 + puff * 2, red_body);
            draw_filled_rect(sx + 4, sy + 2, 8, 2, highlight); // Brilho no topo
            draw_filled_rect(sx + 3, sy + 11, 10, 2, dark_red);

            // Olhos
            if (e->dir == DIR_DOWN) {
                draw_filled_rect(sx + 4, sy + 5, 2, 3, eye_white);
                draw_filled_rect(sx + 10, sy + 5, 2, 3, eye_white);
                put_pixel_safe(sx + 5, sy + 6, eye_pupil);
                put_pixel_safe(sx + 11, sy + 6, eye_pupil);
                // Boca / Tromba para baixo
                draw_filled_rect(sx + 6, sy + 8, 4, 3, dark_red);
            } else if (e->dir == DIR_UP) {
                // De costas
                draw_filled_rect(sx + 4, sy + 4, 8, 3, dark_red);
            } else if (e->dir == DIR_LEFT) {
                draw_filled_rect(sx + 4, sy + 5, 3, 3, eye_white);
                put_pixel_safe(sx + 4, sy + 6, eye_pupil);
                // Tromba para a esquerda
                draw_filled_rect(sx + 1 - puff, sy + 6, 3 + puff, 3, dark_red);
            } else if (e->dir == DIR_RIGHT) {
                draw_filled_rect(sx + 9, sy + 5, 3, 3, eye_white);
                put_pixel_safe(sx + 11, sy + 6, eye_pupil);
                // Tromba para a direita
                draw_filled_rect(sx + 12, sy + 6, 3 + puff, 3, dark_red);
            }

            // Tentáculos (animados com wiggle)
            if (e->animFrame == 0) {
                draw_filled_rect(sx + 3, sy + 13, 3, 2, dark_red);
                draw_filled_rect(sx + 10, sy + 13, 3, 2, dark_red);
            } else {
                draw_filled_rect(sx + 5, sy + 13, 3, 2, dark_red);
                draw_filled_rect(sx + 8, sy + 13, 3, 2, dark_red);
            }
        }

        // 2. PROJÉTIL: PEDRA
        else if (e->type == ENTITY_PROJECTILE_ROCK) {
            u32 rock_color = 0x8A5528FF;
            u32 rock_dark  = 0x4D2E14FF;
            draw_filled_rect(sx + 1, sy + 1, 4, 4, rock_color);
            put_pixel_safe(sx + 2, sy + 2, 0xC4864DFF); // Brilho
            put_pixel_safe(sx + 4, sy + 4, rock_dark);
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
    }
}

void entity_manager_shutdown(void) {
    memset(s_entities, 0, sizeof(s_entities));
    printf("[HAL Entity] Gerenciador de entidades finalizado.\n");
}
