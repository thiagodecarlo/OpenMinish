#include "hal/entity.h"
#include "hal/map.h"
#include "hal/dungeon.h"
#include "hal/subweapon.h"
#include "hal/inventory.h"
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
static const Texture* s_enemies_tex = NULL;
static const Texture* s_npcs_tex = NULL;
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

void entity_set_enemies_texture(const Texture* tex) {
    s_enemies_tex = tex;
}

void entity_set_npcs_texture(const Texture* tex) {
    s_npcs_tex = tex;
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

static inline u32 blend_entity_colors(u32 dst, u32 src) {
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
            ctx->framebuffer[idx] = blend_entity_colors(ctx->framebuffer[idx], color);
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
            s_entities[i].type == ENTITY_ENEMY_FIRE_KEESE ||
            s_entities[i].type == ENTITY_ENEMY_CHUCHU ||
            s_entities[i].type == ENTITY_ENEMY_MOBLIN ||
            s_entities[i].type == ENTITY_ENEMY_PEAHAT ||
            s_entities[i].type == ENTITY_ENEMY_TEKTITE ||
            s_entities[i].type == ENTITY_ENEMY_SPINY_BEETLE ||
            s_entities[i].type == ENTITY_ENEMY_ROPE ||
            (s_entities[i].type == ENTITY_BOSS_BIG_CHUCHU && s_entities[i].health > 0) ||
            (s_entities[i].type == ENTITY_BOSS_GLEEROK && s_entities[i].health > 0)) {
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
                case ENTITY_ENEMY_FIRE_KEESE:
                    e->health    = (e->type == ENTITY_ENEMY_FIRE_KEESE) ? 2 : 1;
                    e->maxHealth = e->health;
                    e->damage    = (e->type == ENTITY_ENEMY_FIRE_KEESE) ? 2 : 1;
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

                case ENTITY_NPC_SWIFTBLADE:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_NPC_SHOPKEEPER:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_NPC_TOWN_CITIZEN:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    e->hasKinstone   = true;
                    e->kinstoneType  = 1; // KINSTONE_BLUE (Fragmento azul de Hyrule Town)
                    e->kinstoneFused = false;
                    e->bubbleBob     = 0.0f;
                    break;

                case ENTITY_NPC_TOWN_GUARD:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_TOWN_FOUNTAIN:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->action        = 1;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -16.0f, -16.0f, 32.0f, 32.0f };
                    break;

                case ENTITY_MINISH_STUMP:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->action        = 0; // 0 = Toco Minish (Woods), 1 = Vaso Minish (Town)
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_NPC_GENTARI:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_NPC_FESTARI:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_NPC_VILLAGE_MINISH:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    e->hasKinstone   = true;
                    e->kinstoneType  = 0; // KINSTONE_GREEN
                    e->kinstoneFused = false;
                    e->bubbleBob     = 0.0f;
                    break;

                case ENTITY_ENEMY_MOBLIN:
                    e->health        = 4;
                    e->maxHealth     = 4;
                    e->damage        = 2; // 1 Coração de dano
                    e->dir           = DIR_DOWN;
                    e->action        = 1; // Patrulha com lança
                    e->aiTimer       = 70;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_ENEMY_PEAHAT:
                    e->health        = 3;
                    e->maxHealth     = 3;
                    e->damage        = 1; // 1/2 Coração
                    e->dir           = DIR_DOWN;
                    e->action        = 1; // Voo alto
                    e->z             = 18.0f;
                    e->aiTimer       = 140;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -7.0f, -7.0f, 14.0f, 14.0f };
                    break;

                case ENTITY_NPC_MALON:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    e->hasKinstone   = true;
                    e->kinstoneType  = 1; // KINSTONE_BLUE
                    e->kinstoneFused = false;
                    e->bubbleBob     = 0.0f;
                    break;

                case ENTITY_ENEMY_TEKTITE:
                    e->health        = 3;
                    e->maxHealth     = 3;
                    e->damage        = 1;
                    e->dir           = DIR_DOWN;
                    e->action        = 1; // Solo se preparando para saltar
                    e->aiTimer       = 40 + (rand() % 40);
                    e->animTimer     = 0;
                    e->z             = 0.0f;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_ENEMY_SPINY_BEETLE:
                    e->health        = 2;
                    e->maxHealth     = 2;
                    e->damage        = 1;
                    e->dir           = (Direction)(rand() % 4);
                    e->action        = 1; // Patrulha / espreita sob a rocha
                    e->aiTimer       = 60 + (rand() % 30);
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -7.0f, -7.0f, 14.0f, 14.0f };
                    break;

                case ENTITY_NPC_BUSINESS_SCRUB:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1; // No arbusto espiando
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_NPC_MELARI:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1; // Na forja martelando a bigorna
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_NPC_MOUNTAIN_MINISH:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1; // Minerando minérios com picareta
                    e->animTimer     = rand() % 30;
                    e->hitbox        = (Hitbox){ -6.0f, -6.0f, 12.0f, 12.0f };
                    break;

                case ENTITY_BOSS_GLEEROK:
                    e->health        = 12;
                    e->maxHealth     = 12;
                    e->damage        = 2;
                    e->dir           = DIR_DOWN;
                    e->action        = 0; // 0 = Emergindo do magma
                    e->subAction     = 0;
                    e->bossToppleTimer  = 0;
                    e->bossEnraged      = false;
                    e->bossDeathTimer   = 0;
                    e->bossFireTimer    = 60;
                    e->bossTargetX      = world_x;
                    e->bossTargetY      = world_y;
                    e->aiTimer       = 60;
                    e->hitbox        = (Hitbox){ 4.0f, 4.0f, 56.0f, 40.0f };
                    break;

                case ENTITY_ITEM_FIRE_ELEMENT:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->action        = 0;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ 2.0f, 2.0f, 12.0f, 12.0f };
                    break;

                case ENTITY_PROJECTILE_FIREBALL:
                    e->health        = 1;
                    e->maxHealth     = 1;
                    e->damage        = 2;
                    e->action        = 1;
                    e->aiTimer       = 90;
                    e->hitbox        = (Hitbox){ 2.0f, 2.0f, 8.0f, 8.0f };
                    break;

                case ENTITY_ENEMY_ROPE:
                    e->health        = 2;
                    e->maxHealth     = 2;
                    e->damage        = 1;
                    e->dir           = (Direction)(rand() % 4);
                    e->action        = 1; // 1 = Patrulha/espreita, 2 = Investida (Charge)
                    e->aiTimer       = 40 + (rand() % 40);
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -6.0f, -6.0f, 12.0f, 12.0f };
                    break;

                case ENTITY_ITEM_BOW:
                case ENTITY_ITEM_MOLE_MITTS:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->action        = 0;
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_ARMOS:
                    e->health        = 12;
                    e->maxHealth     = 12;
                    e->damage        = 1;
                    e->action        = 0; // 0: DORMANT, 1: WAKING, 2: AWAKE
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -12.0f, 16.0f, 24.0f };
                    break;

                case ENTITY_ARMOS_SWITCH:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->action        = 0; // 0: OFF, 1: ON
                    e->animTimer     = 0;
                    e->hitbox        = (Hitbox){ -8.0f, -8.0f, 16.0f, 16.0f };
                    break;

                case ENTITY_NPC_LIBRARI:
                case ENTITY_NPC_MAYOR_HAGEN:
                    e->health        = 999;
                    e->maxHealth     = 999;
                    e->damage        = 0;
                    e->dir           = DIR_DOWN;
                    e->action        = 1;
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
                float speed = 0.25f; // Velocidade suave e cadenciada autêntica do GBA Minish Cap
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
                        e->aiTimer = 42; // ~700ms de antecipação visual com bochechas infladas antes do disparo
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
                        float rock_speed = 1.20f; // Velocidade equilibrada do projétil, permitindo esquiva e corte com espada
                        rock->dir = e->dir;
                        if (e->dir == DIR_DOWN)  rock->vy = rock_speed;
                        if (e->dir == DIR_UP)    rock->vy = -rock_speed;
                        if (e->dir == DIR_LEFT)  rock->vx = -rock_speed;
                        if (e->dir == DIR_RIGHT) rock->vx = rock_speed;
                        hal_audio_play_sound(SOUND_ROLL, 0.6f, 1.4f); // Som de cusparada
                    }

                    e->action = 1; // Retorna à patrulha
                    e->aiTimer = 180; // 3.0 segundos de cooldown entre disparos (ritmo autêntico)
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
        // 5. INIMIGO: KEESE & FIRE KEESE (MORCEGO VOADOR / EM CHAMAS)
        // --------------------------------------------------------------------
        else if (e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE) {
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

        // --------------------------------------------------------------------
        // CHEFE GLEEROK (CAVE OF FLAMES - DRAGÃO DE LAVA VULCÂNICO)
        // --------------------------------------------------------------------
        else if (e->type == ENTITY_BOSS_GLEEROK) {
            e->animTimer++;
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            float dx = link_x - (e->x + 28.0f);
            float dy = link_y - (e->y + 20.0f);
            float dist = sqrtf(dx * dx + dy * dy);

            // AÇÃO 0: Emergindo do Magma Vulcânico
            if (e->action == 0) {
                e->aiTimer--;
                if ((e->aiTimer % 15) == 0) {
                    entity_trigger_screen_shake(6, 3);
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 0.8f, 0.7f);
                }
                if (e->aiTimer <= 0) {
                    e->action = 1; // Patrulha ativa e ataques de fogo
                    e->aiTimer = 60;
                    e->bossFireTimer = 90;
                    entity_trigger_screen_shake(18, 5);
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.9f);
                    printf("[BOSS GLEEROK] O Dragao de Fogo emergiu da lava incandescente!\n");
                }
            }

            // AÇÃO 1: Patrulha no Lago de Lava e Ataques de Fogo
            else if (e->action == 1) {
                // Movimento oscilatório de nado no magma
                float patrol_speed = e->bossEnraged ? 1.4f : 0.85f;
                e->x += e->vx;
                if (e->x <= 64.0f) { e->x = 64.0f; e->vx = patrol_speed; }
                if (e->x >= 144.0f) { e->x = 144.0f; e->vx = -patrol_speed; }
                if (e->vx == 0.0f) e->vx = patrol_speed;

                // Temporizador de baforada de fogo
                e->bossFireTimer--;
                if (e->bossFireTimer <= 0) {
                    e->bossFireTimer = e->bossEnraged ? 80 : 120;
                    // Cospe bola de magma incandescente na direção de Link
                    float spd = 2.4f;
                    float f_dx = link_x - (e->x + 28.0f);
                    float f_dy = link_y - (e->y + 30.0f);
                    float f_dist = sqrtf(f_dx * f_dx + f_dy * f_dy);
                    if (f_dist > 1.0f) {
                        Entity* fb = entity_spawn(ENTITY_PROJECTILE_FIREBALL, e->x + 24.0f, e->y + 32.0f);
                        if (fb) {
                            fb->vx = (f_dx / f_dist) * spd;
                            fb->vy = (f_dy / f_dist) * spd;
                            hal_audio_play_sound(SOUND_SWORD_SLASH, 0.9f, 0.6f);
                            printf("[BOSS GLEEROK] Disparo de bola de fogo contra Link!\n");
                        }
                    }
                }

                // Dano de colisão contra o herói se Link chegar perto demais da carapaça sem virá-la
                if (*link_invuln_timer <= 0 && dist < 28.0f) {
                    if (*link_hearts > 0) *link_hearts -= 2; // 1 Coração
                    *link_invuln_timer = 60;
                    *link_knock_x = (dx > 0.0f) ? 4.0f : -4.0f;
                    *link_knock_y = 5.0f;
                    hal_audio_play_sound(SOUND_HEART_BEEP, 1.0f, 0.8f);
                }
            }

            // AÇÃO 3: Desabado na Plataforma (Ponte Vulnerável criada pelo Cajado de Pacci)
            else if (e->action == 3) {
                e->bossToppleTimer--;
                if (e->bossToppleTimer <= 0) {
                    e->action = 4; // Recuperação Enraivecida
                    e->aiTimer = 40;
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 1.1f);
                    printf("[BOSS GLEEROK] Gleerok recuperou o equilibrio e vai mergulhar de volta!\n");
                }
            }

            // AÇÃO 4: Recuperação Enraivecida (Sacode Link e mergulha)
            else if (e->action == 4) {
                e->aiTimer--;
                if (e->aiTimer == 30) {
                    // Empurra Link para trás para a ilha
                    *link_knock_y = 6.0f;
                    *link_invuln_timer = 20;
                    entity_trigger_screen_shake(12, 4);
                }
                if (e->aiTimer <= 0) {
                    e->bossEnraged = true;
                    e->action = 1; // Retorna à patrulha
                    e->bossFireTimer = 50;
                }
            }

            // AÇÃO 5: Derrota e Extinção do Dragão
            else if (e->action == 5) {
                e->bossDeathTimer++;
                if ((e->bossDeathTimer % 8) == 0) {
                    entity_trigger_screen_shake(8, 4);
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 0.9f, 0.8f + (float)e->bossDeathTimer * 0.005f);
                }
                if (e->bossDeathTimer >= 80) {
                    e->is_active = false;
                    s_boss_defeated = true;
                    hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                    entity_trigger_screen_shake(24, 6);

                    // Spawna Heart Container permanente e o Elemento Fogo!
                    entity_spawn(ENTITY_ITEM_HEART_CONTAINER, e->x + 12.0f, e->y + 24.0f);
                    entity_spawn(ENTITY_ITEM_FIRE_ELEMENT, e->x + 40.0f, e->y + 24.0f);
                    printf("[BOSS DEFEATED] Gleerok foi destruido! Heart Container e Elemento Fogo liberados!\n");
                    continue;
                }
            }
        }

        // PROJÉTIL: BOLA DE FOGO DO GLEEROK
        else if (e->type == ENTITY_PROJECTILE_FIREBALL) {
            e->animTimer++;
            e->x += e->vx;
            e->y += e->vy;
            e->aiTimer--;
            if (e->aiTimer <= 0) {
                e->is_active = false;
                continue;
            }

            // Dano de fogo em Link
            float dx = (link_x + 8.0f) - (e->x + 4.0f);
            float dy = (link_y + 8.0f) - (e->y + 4.0f);
            if (*link_invuln_timer <= 0 && (dx * dx + dy * dy) < 144.0f) {
                if (*link_hearts > 0) *link_hearts -= 2;
                *link_invuln_timer = 50;
                *link_knock_x = e->vx * 1.5f;
                *link_knock_y = e->vy * 1.5f;
                e->is_active = false;
                hal_audio_play_sound(SOUND_HEART_BEEP, 1.0f, 0.9f);
                continue;
            }
        }

        // ITEM SAGRADO: ELEMENTO DO FOGO (FIRE ELEMENT)
        else if (e->type == ENTITY_ITEM_FIRE_ELEMENT) {
            e->animTimer++;
            // Coleta por Link
            float dx = (link_x + 8.0f) - (e->x + 8.0f);
            float dy = (link_y + 8.0f) - (e->y + 8.0f);
            if ((dx * dx + dy * dy) < 196.0f) {
                e->is_active = false;
                hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.0f);
                printf("[SACRED ELEMENT] Link obteve o sagrado Elemento Fogo (Fire Element)!\n");
            }
        }

        // 6. NPC: MESTRE ESPADACHIM SWIFTBLADE
        else if (e->type == ENTITY_NPC_SWIFTBLADE) {
            e->animTimer++;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 48.0f * 48.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                e->dir = DIR_DOWN;
            }
        }

        // 7. NPC: COMERCIANTE STOCKWELL
        else if (e->type == ENTITY_NPC_SHOPKEEPER) {
            e->animTimer++;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 54.0f * 54.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                e->dir = DIR_DOWN;
            }
        }

        // 8. NPC: CIDADÃ DE HYRULE (COM FUSÃO DE KINSTONE)
        else if (e->type == ENTITY_NPC_TOWN_CITIZEN) {
            e->animTimer++;
            e->bubbleBob += 0.08f;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 42.0f * 42.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                if (e->animTimer % 180 == 0) {
                    Direction dirs[4] = { DIR_DOWN, DIR_RIGHT, DIR_DOWN, DIR_LEFT };
                    e->dir = dirs[(e->animTimer / 180) % 4];
                }
            }
        }

        // 9. NPC: GUARDA REAL DO CASTELO
        else if (e->type == ENTITY_NPC_TOWN_GUARD) {
            e->animTimer++;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 36.0f * 36.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                e->dir = DIR_DOWN;
            }
        }

        // 10. ELEMENTO DINÂMICO: CHAFARIZ CENTRAL
        else if (e->type == ENTITY_TOWN_FOUNTAIN) {
            e->animTimer++;
        }

        // 11. PORTAL MINISH (TOCO OU VASO)
        else if (e->type == ENTITY_MINISH_STUMP) {
            e->animTimer++;
        }

        // 12. NPCS DA VILA DOS MINISH (GENTARI, FESTARI, MORADORES PICORI, LIBRARI, HAGEN)
        else if (e->type == ENTITY_NPC_GENTARI || e->type == ENTITY_NPC_FESTARI || e->type == ENTITY_NPC_VILLAGE_MINISH ||
                 e->type == ENTITY_NPC_LIBRARI || e->type == ENTITY_NPC_MAYOR_HAGEN) {
            e->animTimer++;
            if (e->type == ENTITY_NPC_VILLAGE_MINISH) {
                e->bubbleBob += 0.08f;
            }
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 40.0f * 40.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                e->dir = DIR_DOWN;
            }
        }

        // 13. INIMIGO: MOBLIN (GUARDIÃO DOS CAMPOS COM LANÇA)
        else if (e->type == ENTITY_ENEMY_MOBLIN) {
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Ação 4: Recuo por dano da espada ou bomba (Knockback)
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
                        int roll = rand() % 100;
                        if (roll < 60) {
                            entity_spawn(ENTITY_ITEM_RUPEE, e->x + 4.0f, e->y + 4.0f);
                        } else {
                            entity_spawn(ENTITY_ITEM_HEART, e->x + 4.0f, e->y + 4.0f);
                        }
                        e->is_active = false;
                        continue;
                    } else {
                        e->action = 1;
                        e->aiTimer = 40;
                    }
                }
            }
            // Ação 1: Patrulha e Ataque com Lança
            else if (e->action == 1) {
                float speed = 0.35f;
                float move_x = 0.0f;
                float move_y = 0.0f;
                if (e->dir == DIR_DOWN)  move_y = speed;
                if (e->dir == DIR_UP)    move_y = -speed;
                if (e->dir == DIR_LEFT)  move_x = -speed;
                if (e->dir == DIR_RIGHT) move_x = speed;

                float check_x = e->x + move_x + 8.0f;
                float check_y = e->y + move_y + 8.0f;

                if (entity_is_solid(map, check_x, check_y)) {
                    e->dir = (Direction)(rand() % 4);
                    e->aiTimer = 50;
                } else {
                    e->x += move_x;
                    e->y += move_y;
                }

                e->animTimer++;
                if (e->animTimer > 10) {
                    e->animFrame = (e->animFrame + 1) % 2;
                    e->animTimer = 0;
                }

                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->dir = (Direction)(rand() % 4);
                    e->aiTimer = 60 + (rand() % 60);
                }

                // Dano por contato da lança com Link
                if (*link_invuln_timer <= 0) {
                    float ox1 = e->x - 6.0f;
                    float oy1 = e->y - 6.0f;
                    float ox2 = e->x + 14.0f;
                    float oy2 = e->y + 14.0f;
                    float lx1 = link_x + 2.0f;
                    float ly1 = link_y + 4.0f;
                    float lx2 = lx1 + 12.0f;
                    float ly2 = ly1 + 12.0f;

                    if (ox1 < lx2 && ox2 > lx1 && oy1 < ly2 && oy2 > ly1) {
                        if (*link_hearts > 0) {
                            *link_hearts -= e->damage;
                            if (*link_hearts < 0) *link_hearts = 0;
                        }
                        *link_invuln_timer = 50;
                        hal_audio_play_sound(SOUND_HEART_BEEP, 0.85f, 1.0f);

                        float p_dx = (link_x + 8.0f) - (e->x + 8.0f);
                        float p_dy = (link_y + 8.0f) - (e->y + 8.0f);
                        float p_len = sqrtf(p_dx * p_dx + p_dy * p_dy);
                        if (p_len > 0.01f) {
                            *link_knock_x = (p_dx / p_len) * 3.8f;
                            *link_knock_y = (p_dy / p_len) * 3.8f;
                        }
                    }
                }
            }
        }

        // 14. INIMIGO: PEAHAT (FLOR-HELICÓPTERO VOADORA)
        else if (e->type == ENTITY_ENEMY_PEAHAT) {
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Ação 4: Knockback
            if (e->action == 4) {
                e->x += e->knockbackVx;
                e->y += e->knockbackVy;
                e->knockbackTimer--;
                if (e->knockbackTimer <= 0) {
                    if (e->health <= 0) {
                        entity_spawn(ENTITY_ITEM_HEART, e->x + 3.0f, e->y + 3.0f);
                        e->is_active = false;
                        continue;
                    } else {
                        e->action = 2; // Desce no solo atordoado
                        e->aiTimer = 60;
                    }
                }
            }
            // Ação 1: Voando alto
            else if (e->action == 1) {
                e->animTimer++;
                e->z = 16.0f + sinf((float)e->animTimer * 0.1f) * 3.0f;

                // Movimento suave em direção ao Link
                float dx = (link_x + 8.0f) - (e->x + 8.0f);
                float dy = (link_y + 8.0f) - (e->y + 8.0f);
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist > 1.0f) {
                    e->x += (dx / dist) * 0.4f;
                    e->y += (dy / dist) * 0.4f;
                }

                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->action = 2; // Desce para descansar no chão
                    e->aiTimer = 90; // Fica no solo por 1.5s
                    e->z = 0.0f;
                }

                // Dano por colisão com hélice giratória
                if (*link_invuln_timer <= 0) {
                    float px1 = e->x - 6.0f;
                    float py1 = e->y - 6.0f;
                    float px2 = e->x + 14.0f;
                    float py2 = e->y + 14.0f;
                    float lx1 = link_x + 2.0f;
                    float ly1 = link_y + 4.0f;
                    float lx2 = lx1 + 12.0f;
                    float ly2 = ly1 + 12.0f;

                    if (px1 < lx2 && px2 > lx1 && py1 < ly2 && py2 > ly1) {
                        if (*link_hearts > 0) (*link_hearts)--;
                        *link_invuln_timer = 50;
                        hal_audio_play_sound(SOUND_HEART_BEEP, 0.85f, 1.0f);
                    }
                }
            }
            // Ação 2: Descansando no solo (vulnerável a ataques de espada!)
            else if (e->action == 2) {
                e->z = 0.0f;
                e->animTimer++;
                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->action = 1; // Decola novamente
                    e->aiTimer = 150;
                }
            }
        }

        // 15. NPC: MALON (FAZENDA LON LON)
        else if (e->type == ENTITY_NPC_MALON) {
            e->animTimer++;
            e->bubbleBob += 0.08f;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 48.0f * 48.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                e->dir = DIR_DOWN;
            }
        }

        // 16. INIMIGO: TEKTITE (ARACNÍDEO SALTITANTE DO MONTE CRENEL)
        else if (e->type == ENTITY_ENEMY_TEKTITE) {
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Ação 4: Recuo por dano (Knockback)
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
                        int roll = rand() % 100;
                        if (roll < 55) {
                            entity_spawn(ENTITY_ITEM_RUPEE, e->x + 4.0f, e->y + 4.0f);
                        } else {
                            entity_spawn(ENTITY_ITEM_HEART, e->x + 4.0f, e->y + 4.0f);
                        }
                        e->is_active = false;
                        continue;
                    } else {
                        e->action = 1;
                        e->aiTimer = 35;
                    }
                }
            }
            // Ação 1: No solo, preparando o impulso de salto
            else if (e->action == 1) {
                e->animTimer++;
                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->action = 2; // Salto parabólico!
                    float dx = link_x - e->x;
                    float dy = link_y - e->y;
                    float dist = sqrtf(dx * dx + dy * dy);
                    float spd = 1.35f;

                    if (dist > 1.0f && dist < 130.0f) {
                        e->vx = (dx / dist) * spd;
                        e->vy = (dy / dist) * spd;
                    } else {
                        float ang = (float)(rand() % 360) * (PI_F / 180.0f);
                        e->vx = cosf(ang) * spd;
                        e->vy = sinf(ang) * spd;
                    }
                    e->vz = 3.6f; // Impulso aéreo vertical
                    e->z = 0.0f;
                    hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.85f, 1.35f);
                }
            }
            // Ação 2: Trajetória balística no ar
            else if (e->action == 2) {
                e->animTimer++;
                e->x += e->vx;
                e->y += e->vy;
                e->z += e->vz;
                e->vz -= 0.22f; // Gravidade

                if (e->z <= 0.0f) {
                    e->z = 0.0f;
                    e->vx = 0.0f;
                    e->vy = 0.0f;
                    e->action = 1;
                    e->aiTimer = 40 + (rand() % 35);
                }
            }

            // Dano por contato com Link (quando perto do solo)
            if (e->z <= 8.0f && *link_invuln_timer <= 0) {
                float tx1 = e->x - 6.0f;
                float ty1 = e->y - 6.0f;
                float tx2 = e->x + 14.0f;
                float ty2 = e->y + 14.0f;
                float lx1 = link_x + 2.0f;
                float ly1 = link_y + 4.0f;
                float lx2 = lx1 + 12.0f;
                float ly2 = ly1 + 12.0f;

                if (tx1 < lx2 && tx2 > lx1 && ty1 < ly2 && ty2 > ly1) {
                    if (*link_hearts > 0) (*link_hearts)--;
                    *link_invuln_timer = 50;
                    hal_audio_play_sound(SOUND_HEART_BEEP, 0.85f, 1.0f);

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

        // 17. INIMIGO: SPINY BEETLE (BESOURO COM CARAPAÇA DE ROCHA ESPINHOSA)
        else if (e->type == ENTITY_ENEMY_SPINY_BEETLE) {
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Ação 4: Recuo por dano (Knockback)
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
                        int roll = rand() % 100;
                        if (roll < 60) {
                            entity_spawn(ENTITY_ITEM_RUPEE, e->x + 4.0f, e->y + 4.0f);
                        } else {
                            entity_spawn(ENTITY_ITEM_HEART, e->x + 4.0f, e->y + 4.0f);
                        }
                        e->is_active = false;
                        continue;
                    } else {
                        e->action = 1;
                        e->aiTimer = 40;
                    }
                }
            }
            // Ação 1: Patrulha e espreita lenta
            else if (e->action == 1) {
                float spd = 0.45f;
                float mx = 0.0f, my = 0.0f;
                if (e->dir == DIR_DOWN)  my = spd;
                if (e->dir == DIR_UP)    my = -spd;
                if (e->dir == DIR_LEFT)  mx = -spd;
                if (e->dir == DIR_RIGHT) mx = spd;

                float cx = e->x + mx + 8.0f;
                float cy = e->y + my + 8.0f;

                if (entity_is_solid(map, cx, cy)) {
                    e->dir = (Direction)(rand() % 4);
                    e->aiTimer = 45;
                } else {
                    e->x += mx;
                    e->y += my;
                }

                e->animTimer++;
                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->dir = (Direction)(rand() % 4);
                    e->aiTimer = 50 + (rand() % 40);
                }

                // Se Link estiver próximo (linha de visão / alerta), entra em investida rápida!
                float dx = (link_x + 8.0f) - (e->x + 8.0f);
                float dy = (link_y + 8.0f) - (e->y + 8.0f);
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist <= 65.0f) {
                    e->action = 2; // Investida agressiva!
                    e->aiTimer = 80;
                    hal_audio_play_sound(SOUND_SWORD_HIT, 0.5f, 1.6f);
                }
            }
            // Ação 2: Investida veloz em perseguição direta
            else if (e->action == 2) {
                float dx = (link_x + 8.0f) - (e->x + 8.0f);
                float dy = (link_y + 8.0f) - (e->y + 8.0f);
                float dist = sqrtf(dx * dx + dy * dy);
                float spd = 1.30f;

                if (dist > 1.0f) {
                    float mx = (dx / dist) * spd;
                    float my = (dy / dist) * spd;
                    if (!entity_is_solid(map, e->x + mx + 8.0f, e->y + my + 8.0f)) {
                        e->x += mx;
                        e->y += my;
                    } else {
                        // Bateu na parede, fica atordoado e volta a patrulhar
                        e->action = 1;
                        e->aiTimer = 40;
                    }
                }

                e->animTimer += 2;
                e->aiTimer--;
                if (e->aiTimer <= 0 || dist > 110.0f) {
                    e->action = 1;
                    e->aiTimer = 50;
                }
            }
            // Ação 3: Virado de cabeça para baixo pelo Cajado de Pacci (indefeso e esperneando)
            else if (e->action == 3) {
                e->damage = 0; // Inofensivo enquanto virado
                e->animTimer++;
                e->aiTimer--;
                if (e->aiTimer <= 0) {
                    e->action = 1;
                    e->damage = 1;
                    e->aiTimer = 60;
                    hal_audio_play_sound(SOUND_ROLL, 0.70f, 1.2f);
                    printf("[SPINY BEETLE] Besouro conseguiu se desvirar e voltou a patrulhar!\n");
                }
            }

            // Dano por colisão espinhosa com Link (apenas se NÃO estiver virado de costas)
            if (*link_invuln_timer <= 0 && e->action != 3) {
                float bx1 = e->x - 6.0f;
                float by1 = e->y - 6.0f;
                float bx2 = e->x + 14.0f;
                float by2 = e->y + 14.0f;
                float lx1 = link_x + 2.0f;
                float ly1 = link_y + 4.0f;
                float lx2 = lx1 + 12.0f;
                float ly2 = ly1 + 12.0f;

                if (bx1 < lx2 && bx2 > lx1 && by1 < ly2 && by2 > ly1) {
                    if (*link_hearts > 0) (*link_hearts)--;
                    *link_invuln_timer = 50;
                    hal_audio_play_sound(SOUND_HEART_BEEP, 0.85f, 1.0f);

                    float p_dx = (link_x + 8.0f) - (e->x + 8.0f);
                    float p_dy = (link_y + 8.0f) - (e->y + 8.0f);
                    float p_len = sqrtf(p_dx * p_dx + p_dy * p_dy);
                    if (p_len > 0.01f) {
                        *link_knock_x = (p_dx / p_len) * 3.6f;
                        *link_knock_y = (p_dy / p_len) * 3.6f;
                    }
                }
            }
        }

        // 18. NPC: BUSINESS SCRUB (DEKU SCRUB COMERCIANTE DO MONTE CRENEL)
        else if (e->type == ENTITY_NPC_BUSINESS_SCRUB) {
            e->animTimer++;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 48.0f * 48.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                e->dir = DIR_DOWN;
            }
        }

        // 19. NPC: MESTRE FERREIRO MELARI (CHEFE DOS MOUNTAIN MINISH)
        else if (e->type == ENTITY_NPC_MELARI) {
            e->animTimer++;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 40.0f * 40.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                e->dir = DIR_DOWN;
            }
        }

        // 20. NPC: MINERADOR MOUNTAIN MINISH
        else if (e->type == ENTITY_NPC_MOUNTAIN_MINISH) {
            e->animTimer++;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 36.0f * 36.0f) {
                if (fabsf(dx) > fabsf(dy)) {
                    e->dir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    e->dir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            }
        }

        // 21. INIMIGO: ROPE (SERPENTE ÁGIL DO PÂNTANO DE CASTOR WILDS)
        else if (e->type == ENTITY_ENEMY_ROPE) {
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Ação 4: Recuo por dano (Knockback)
            if (e->action == 4) {
                float next_x = e->x + e->knockbackVx;
                float next_y = e->y + e->knockbackVy;
                if (!entity_is_solid(map, next_x + 6.0f, next_y + 6.0f)) {
                    e->x = next_x;
                    e->y = next_y;
                }
                e->knockbackTimer--;
                if (e->knockbackTimer <= 0) {
                    if (e->health <= 0) {
                        int roll = rand() % 100;
                        if (roll < 60) {
                            entity_spawn(ENTITY_ITEM_RUPEE, e->x + 4.0f, e->y + 4.0f);
                        } else {
                            entity_spawn(ENTITY_ITEM_HEART, e->x + 4.0f, e->y + 4.0f);
                        }
                        e->is_active = false;
                        continue;
                    } else {
                        e->action = 1;
                        e->subAction = 0;
                        e->aiTimer = 40;
                    }
                }
            }
            // Ação 1: Rastejando devagar / espreitando (Patrulha)
            else if (e->action == 1) {
                e->animTimer++;
                e->aiTimer--;

                // Locomoção suave na direção atual
                float spd = 0.65f;
                float next_x = e->x;
                float next_y = e->y;
                if (e->dir == DIR_DOWN)  next_y += spd;
                if (e->dir == DIR_UP)    next_y -= spd;
                if (e->dir == DIR_LEFT)  next_x -= spd;
                if (e->dir == DIR_RIGHT) next_x += spd;

                if (!entity_is_solid(map, next_x + 6.0f, next_y + 6.0f)) {
                    e->x = next_x;
                    e->y = next_y;
                } else {
                    e->dir = (Direction)(rand() % 4);
                }

                if (e->aiTimer <= 0) {
                    e->dir = (Direction)(rand() % 4);
                    e->aiTimer = 40 + (rand() % 40);
                }

                // Verificação de Linha de Visada com Link para INVESTIDA (Charge!)
                float ldx = link_x - e->x;
                float ldy = link_y - e->y;
                float dist = sqrtf(ldx * ldx + ldy * ldy);

                if (dist < 110.0f) {
                    if (fabsf(ldy) <= 12.0f) {
                        // Alinhado horizontalmente!
                        e->dir = (ldx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                        e->action = 2; // Entra em modo de INVESTIDA!
                        e->aiTimer = 45;
                        hal_audio_play_sound(SOUND_SWORD_HIT, 0.7f, 1.6f);
                    } else if (fabsf(ldx) <= 12.0f) {
                        // Alinhado verticalmente!
                        e->dir = (ldy > 0.0f) ? DIR_DOWN : DIR_UP;
                        e->action = 2; // Entra em modo de INVESTIDA!
                        e->aiTimer = 45;
                        hal_audio_play_sound(SOUND_SWORD_HIT, 0.7f, 1.6f);
                    }
                }
            }
            // Ação 2: INVESTIDA RÁPIDA (Charge em linha reta na direção do Link!)
            else if (e->action == 2) {
                e->animTimer += 2;
                e->aiTimer--;

                float charge_spd = 2.8f;
                float next_x = e->x;
                float next_y = e->y;
                if (e->dir == DIR_DOWN)  next_y += charge_spd;
                if (e->dir == DIR_UP)    next_y -= charge_spd;
                if (e->dir == DIR_LEFT)  next_x -= charge_spd;
                if (e->dir == DIR_RIGHT) next_x += charge_spd;

                if (!entity_is_solid(map, next_x + 6.0f, next_y + 6.0f)) {
                    e->x = next_x;
                    e->y = next_y;
                } else {
                    // Colidiu com obstáculo: encerra a investida e fica atordoado de leve
                    e->action = 1;
                    e->aiTimer = 35;
                }

                if (e->aiTimer <= 0) {
                    e->action = 1;
                    e->aiTimer = 50;
                }
            }

            // Dano por contato com Link
            if (*link_invuln_timer <= 0) {
                float rx1 = e->x - 4.0f;
                float ry1 = e->y - 4.0f;
                float rx2 = e->x + 12.0f;
                float ry2 = e->y + 12.0f;

                float lx1 = link_x + 2.0f;
                float ly1 = link_y + 4.0f;
                float lx2 = link_x + 14.0f;
                float ly2 = link_y + 16.0f;

                if (rx1 < lx2 && rx2 > lx1 && ry1 < ly2 && ry2 > ly1) {
                    if (*link_hearts > 0) {
                        *link_hearts -= e->damage;
                        if (*link_hearts < 0) *link_hearts = 0;
                    }
                    *link_invuln_timer = 40;
                    hal_audio_play_sound(SOUND_HEART_BEEP, 1.0f, 1.0f);

                    float dx = link_x - e->x;
                    float dy = link_y - e->y;
                    float d = sqrtf(dx * dx + dy * dy);
                    if (d > 0.1f) {
                        *link_knock_x = (dx / d) * 3.5f;
                        *link_knock_y = (dy / d) * 3.5f;
                    } else {
                        *link_knock_x = 0.0f;
                        *link_knock_y = 3.5f;
                    }
                }
            }
        }

        // 22. ITEM: ARCO E FLECHAS (RELÍQUIA ANCESTRAL DE CASTOR WILDS)
        else if (e->type == ENTITY_ITEM_BOW) {
            e->animTimer++;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 16.0f * 16.0f) {
                // Link coletou o Arco e Flechas!
                e->is_active = false;
                subweapon_set_current(ITEM_BOW);
                subweapon_add_arrows(30);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                printf("[ITEM BOW] Link obteve o Arco e Flechas! Aljava abastecida com 30 flechas!\n");
            }
        }

        // 23. ITEM: LUVAS DE TOUPEIRA (MOLE MITTS)
        else if (e->type == ENTITY_ITEM_MOLE_MITTS) {
            e->animTimer++;
            float dx = link_x - e->x;
            float dy = link_y - e->y;
            if (dx * dx + dy * dy <= 16.0f * 16.0f) {
                // Link coletou as Luvas de Toupeira!
                e->is_active = false;
                subweapon_set_current(ITEM_MOLE_MITTS);
                inventory_unlock_item(INV_ITEM_MOLE_MITTS);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                printf("[ITEM MOLE MITTS] Link obteve as Luvas de Toupeira (Mole Mitts)! Pronto para escavar paredes e montes!\n");
            }
        }

        // 24. ENTIDADE ROBÔ ARMOS (SENTINELA ANCESTRAL DAS RUÍNAS)
        else if (e->type == ENTITY_ARMOS) {
            if (e->invulnerableTimer > 0) e->invulnerableTimer--;

            // Se o circuito do robô foi ativado e ele ainda está dormente, inicia o despertar!
            if (armos_circuit_is_active() && e->action == 0) {
                e->action = 1;
                e->animTimer = 45;
                hal_audio_play_sound(SOUND_BLOCK_PUSH, 0.9f, 1.1f);
                printf("[ARMOS] Sentinela Armos recebendo energia do circuito! Despertando!\n");
            }

            if (e->action == 1) {
                // Despertando: estátua treme, poeira ancestral sobe
                e->animTimer--;
                if (e->animTimer <= 0) {
                    e->action = 2; // Totalmente acordado!
                    // Desloca-se lateralmente para liberar a passagem
                    e->x += 24.0f;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                    printf("[ARMOS] Armos liberou a passagem do desfiladeiro para a Fortaleza dos Ventos!\n");
                }
            } else if (e->action == 2) {
                // Ativo: respiração mecânica
                e->animTimer++;
            }
        }

        // 25. ENTIDADE INTERRUPTOR ARMOS (ARMOS CIRCUIT SWITCH)
        else if (e->type == ENTITY_ARMOS_SWITCH) {
            e->animTimer++;
            if (e->action == 1 && !armos_circuit_is_active()) {
                armos_circuit_set_active(true);
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

        // Golpeando Interruptor interno do Armos
        if (e->type == ENTITY_ARMOS_SWITCH && e->action == 0) {
            float ox1 = e->x + e->hitbox.offset_x;
            float oy1 = e->y + e->hitbox.offset_y;
            float ox2 = ox1 + e->hitbox.width;
            float oy2 = oy1 + e->hitbox.height;
            if (sx1 < ox2 && sx2 > ox1 && sy1 < oy2 && sy2 > oy1) {
                e->action = 1; // Ligado!
                armos_circuit_set_active(true);
                hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.0f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                entity_trigger_screen_shake(12, 3);
                hit_something = true;
                printf("[ARMOS SWITCH] Circuito interno acionado por Minish Link! Armos acordando!\n");
            }
        }

        // Golpeando sentinela Armos
        if (e->type == ENTITY_ARMOS && e->invulnerableTimer <= 0) {
            float ox1 = e->x + e->hitbox.offset_x;
            float oy1 = e->y + e->hitbox.offset_y;
            float ox2 = ox1 + e->hitbox.width;
            float oy2 = oy1 + e->hitbox.height;
            if (sx1 < ox2 && sx2 > ox1 && sy1 < oy2 && sy2 > oy1) {
                if (e->action == 0) {
                    // Estátua dormente impenetrável
                    hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.0f);
                    hit_something = true;
                } else if (e->action >= 1) {
                    e->health -= damage;
                    e->invulnerableTimer = 20;
                    hal_audio_play_sound(SOUND_BOSS_HIT, 0.8f, 1.2f);
                    hit_something = true;
                    if (e->health <= 0) {
                        e->is_active = false;
                        hal_audio_play_sound(SOUND_BOSS_DEFEAT, 0.8f, 1.2f);
                        entity_spawn(ENTITY_ITEM_RUPEE, e->x, e->y);
                    }
                }
            }
        }

        // Golpeando Inimigo (Octorok, Keese, Fire Keese, ChuChu, Moblin, Peahat, Tektite ou Spiny Beetle)
        if ((e->type == ENTITY_ENEMY_OCTOROK ||
             e->type == ENTITY_ENEMY_KEESE ||
             e->type == ENTITY_ENEMY_FIRE_KEESE ||
             e->type == ENTITY_ENEMY_MOBLIN ||
             e->type == ENTITY_ENEMY_PEAHAT ||
             e->type == ENTITY_ENEMY_TEKTITE ||
             e->type == ENTITY_ENEMY_SPINY_BEETLE ||
             e->type == ENTITY_ENEMY_ROPE ||
             (e->type == ENTITY_ENEMY_CHUCHU && e->action > 0)) &&
            e->invulnerableTimer <= 0) {

            // Se o Keese, Peahat ou Tektite estiver voando alto demais fora do alcance da lâmina
            if ((e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE || e->type == ENTITY_ENEMY_PEAHAT || e->type == ENTITY_ENEMY_TEKTITE) && e->z > 8.0f) {
                continue;
            }

            float ox1 = e->x + e->hitbox.offset_x;
            float oy1 = e->y + e->hitbox.offset_y;
            float ox2 = ox1 + e->hitbox.width;
            float oy2 = oy1 + e->hitbox.height;

            if (sx1 < ox2 && sx2 > ox1 && sy1 < oy2 && sy2 > oy1) {
                if (e->type == ENTITY_ENEMY_SPINY_BEETLE && e->action == 3) {
                    e->health = 0; // Golpe na barriga macia exposta: derrota instantânea em 1 golpe!
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                } else {
                    e->health -= damage;
                }
                e->invulnerableTimer = 18; // Pisca de dano
                e->action = 4; // Knockback
                e->knockbackTimer = (e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE) ? 14 : ((e->type == ENTITY_ENEMY_MOBLIN) ? 12 : 10);

                float force = (e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE) ? 4.2f : ((e->type == ENTITY_ENEMY_MOBLIN) ? 2.8f : 3.0f);
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
        // Golpeando o Chefe Gleerok
        else if (e->type == ENTITY_BOSS_GLEEROK) {
            if (e->action == 3 && e->invulnerableTimer <= 0) {
                // Núcleo de rubi / fogo exposto no dorso
                float gx1 = e->x + 16.0f;
                float gy1 = e->y + 8.0f;
                float gx2 = e->x + 40.0f;
                float gy2 = e->y + 32.0f;

                if (sx1 < gx2 && sx2 > gx1 && sy1 < gy2 && sy2 > gy1) {
                    e->health -= damage;
                    e->invulnerableTimer = 22;
                    hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.2f);
                    entity_trigger_screen_shake(10, 4);
                    hit_something = true;
                    printf("[BOSS GLEEROK] Golpe direto no nucleo de rubi! Dano: %d | HP: %d\n", damage, e->health);

                    if (e->health <= 0) {
                        e->health = 0;
                        e->action = 5;
                        e->bossDeathTimer = 0;
                        hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.65f);
                    }
                }
            } else if (e->action == 1) {
                // Carapaça blindada de pedra: a espada resvala sem ferir
                float gx1 = e->x + 8.0f;
                float gy1 = e->y + 8.0f;
                float gx2 = e->x + 48.0f;
                float gy2 = e->y + 36.0f;
                if (sx1 < gx2 && sx2 > gx1 && sy1 < gy2 && sy2 > gy1) {
                    hal_audio_play_sound(SOUND_SWORD_HIT, 0.8f, 0.7f);
                    hit_something = true;
                }
            }
        }
    }

    return hit_something;
}

int entity_check_spin_attack_hit(float center_x, float center_y, float radius, int damage) {
    int hit_count = 0;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active) continue;

        // Inimigos: Octorok, Keese, Fire Keese, ChuChu, Moblin, Peahat, Tektite, Spiny Beetle
        if ((e->type == ENTITY_ENEMY_OCTOROK ||
             e->type == ENTITY_ENEMY_KEESE ||
             e->type == ENTITY_ENEMY_FIRE_KEESE ||
             e->type == ENTITY_ENEMY_MOBLIN ||
             e->type == ENTITY_ENEMY_PEAHAT ||
             e->type == ENTITY_ENEMY_TEKTITE ||
             e->type == ENTITY_ENEMY_SPINY_BEETLE ||
             e->type == ENTITY_ENEMY_ROPE ||
             (e->type == ENTITY_ENEMY_CHUCHU && e->action > 0)) &&
            e->invulnerableTimer <= 0) {

            // Se voando alto demais desvia do golpe circular
            if ((e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE || e->type == ENTITY_ENEMY_PEAHAT || e->type == ENTITY_ENEMY_TEKTITE) && e->z > 8.0f) continue;

            float ex = e->x + e->hitbox.offset_x + (e->hitbox.width * 0.5f);
            float ey = e->y + e->hitbox.offset_y + (e->hitbox.height * 0.5f);
            float dx = ex - center_x;
            float dy = ey - center_y;
            float dist = sqrtf(dx * dx + dy * dy);
            float max_reach = radius + (e->hitbox.width * 0.5f);

            if (dist <= max_reach) {
                if (e->type == ENTITY_ENEMY_SPINY_BEETLE && e->action == 3) {
                    e->health = 0;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                } else {
                    e->health -= damage;
                }
                e->invulnerableTimer = 22; // Pisca de dano
                e->action = 4; // Knockback
                e->knockbackTimer = (e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE) ? 18 : 14;

                // Força de repulsão radial centrífuga para longe do herói
                float force = (e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE) ? 5.0f : 4.0f;
                if (dist > 0.1f) {
                    e->knockbackVx = (dx / dist) * force;
                    e->knockbackVy = (dy / dist) * force;
                } else {
                    e->knockbackVx = force;
                    e->knockbackVy = 0.0f;
                }

                float hit_pitch = (e->type == ENTITY_ENEMY_CHUCHU) ? 1.55f : 1.30f;
                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, hit_pitch);
                hit_count++;
            }
        }
        // Destruição e reflexão de projéteis de pedra no ar
        else if (e->type == ENTITY_PROJECTILE_ROCK) {
            float rx = e->x + 3.0f;
            float ry = e->y + 3.0f;
            float dx = rx - center_x;
            float dy = ry - center_y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist <= radius + 5.0f) {
                e->is_active = false; // Destrói a pedra
                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.6f);
                hit_count++;
            }
        }
        // Golpe no Chefe Big Green ChuChu
        else if (e->type == ENTITY_BOSS_BIG_CHUCHU) {
            float bx = e->x;
            float by = e->y;
            float dx = bx - center_x;
            float dy = by - center_y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (e->action == 3 && e->invulnerableTimer <= 0) { // Toppled vulnerável!
                if (dist <= radius + 22.0f) {
                    e->health -= damage;
                    e->invulnerableTimer = 24;
                    hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.25f);
                    entity_trigger_screen_shake(10, 4);
                    printf("[BOSS HIT] SPIN ATTACK devastador na cabeca vulneravel! HP restante: %d / %d\n",
                           e->health, e->maxHealth);

                    if (e->health <= 0) {
                        e->health = 0;
                        e->action = 5;
                        e->bossDeathTimer = 0;
                        hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                    }
                    hit_count++;
                }
            } else if (e->action == 1) { // Em pé
                if (dist <= radius + 26.0f) {
                    hal_audio_play_sound(SOUND_SWORD_HIT, 0.75f, 0.70f);
                    hit_count++;
                }
            }
        }
        // Golpe no Chefe Gleerok
        else if (e->type == ENTITY_BOSS_GLEEROK) {
            float gx = e->x + 28.0f;
            float gy = e->y + 20.0f;
            float dx = gx - center_x;
            float dy = gy - center_y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (e->action == 3 && e->invulnerableTimer <= 0) { // Toppled vulnerável!
                if (dist <= radius + 24.0f) {
                    e->health -= damage;
                    e->invulnerableTimer = 24;
                    hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.25f);
                    entity_trigger_screen_shake(12, 5);
                    printf("[BOSS GLEEROK] SPIN ATTACK devastador no nucleo de fogo! HP restante: %d / %d\n",
                           e->health, e->maxHealth);

                    if (e->health <= 0) {
                        e->health = 0;
                        e->action = 5;
                        e->bossDeathTimer = 0;
                        hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.65f);
                    }
                    hit_count++;
                }
            } else if (e->action == 1) { // Em pé / carapaça blindada
                if (dist <= radius + 32.0f) {
                    hal_audio_play_sound(SOUND_SWORD_HIT, 0.8f, 0.7f);
                    hit_count++;
                }
            }
        }
    }

    return hit_count;
}

int entity_check_bomb_explosion(float center_x, float center_y, float radius, int damage) {
    int hit_count = 0;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active) continue;

        // Inimigos padrão: Octorok, Keese, Fire Keese, ChuChu, Moblin, Peahat, Tektite, Spiny Beetle
        if ((e->type == ENTITY_ENEMY_OCTOROK ||
             e->type == ENTITY_ENEMY_KEESE ||
             e->type == ENTITY_ENEMY_FIRE_KEESE ||
             e->type == ENTITY_ENEMY_CHUCHU ||
             e->type == ENTITY_ENEMY_MOBLIN ||
             e->type == ENTITY_ENEMY_PEAHAT ||
             e->type == ENTITY_ENEMY_TEKTITE ||
             e->type == ENTITY_ENEMY_SPINY_BEETLE ||
             e->type == ENTITY_ENEMY_ROPE) &&
            e->invulnerableTimer <= 0) {

            float ex = e->x + e->hitbox.offset_x + (e->hitbox.width * 0.5f);
            float ey = e->y + e->hitbox.offset_y + (e->hitbox.height * 0.5f);
            float dx = ex - center_x;
            float dy = ey - center_y;
            float dist = sqrtf(dx * dx + dy * dy);
            float max_reach = radius + (e->hitbox.width * 0.5f);

            if (dist <= max_reach) {
                e->health -= damage;
                e->invulnerableTimer = 28; // Pisca de dano pela explosão

                // Forte recuo radial e centrífugo
                if (dist > 0.1f) {
                    float push = 8.5f;
                    e->knockbackVx = (dx / dist) * push;
                    e->knockbackVy = (dy / dist) * push;
                } else {
                    e->knockbackVx = 8.5f;
                    e->knockbackVy = 0.0f;
                }
                e->action = 4; // Modo knockback
                e->knockbackTimer = 20;

                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 0.75f);

                if (e->health <= 0) {
                    e->is_active = false;
                    int r = rand() % 100;
                    if (r < 40) {
                        entity_spawn(ENTITY_ITEM_RUPEE, ex, ey);
                    } else if (r < 75) {
                        entity_spawn(ENTITY_ITEM_HEART, ex, ey);
                    }
                }
                hit_count++;
            }
        }
        // Destruição de Projéteis de Rocha lançados por Octoroks
        else if (e->type == ENTITY_PROJECTILE_ROCK) {
            float rx = e->x + 3.0f;
            float ry = e->y + 3.0f;
            float dx = rx - center_x;
            float dy = ry - center_y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist <= radius + 8.0f) {
                e->is_active = false;
                hit_count++;
            }
        }
        // Explosão no Chefe Big Green ChuChu
        else if (e->type == ENTITY_BOSS_BIG_CHUCHU) {
            float bx = e->x;
            float by = e->y;
            float dx = bx - center_x;
            float dy = by - center_y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist <= radius + 30.0f) {
                if (e->action == 3 && e->invulnerableTimer <= 0) {
                    e->health -= damage;
                    e->invulnerableTimer = 30;
                    hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.10f);
                    entity_trigger_screen_shake(12, 5);
                    printf("[BOSS HIT] EXPLOSAO DE BOMBA na cabeca toppled do chefe! Dano: %d, HP: %d / %d\n",
                           damage, e->health, e->maxHealth);
                    if (e->health <= 0) {
                        e->health = 0;
                        e->action = 5;
                        e->bossDeathTimer = 0;
                        hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                    }
                    hit_count++;
                } else if (e->action == 1) {
                    e->bossBaseScale -= 0.35f;
                    if (e->bossBaseScale < 0.15f) {
                        e->action = 3;
                        e->bossToppleTimer = 180;
                        hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 1.0f);
                        printf("[BOSS TOPPLE] Base gelatinosa do chefe implodida por bomba! Chefe desabou!\n");
                    }
                    hit_count++;
                }
            }
        }
    }

    return hit_count;
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
                  e->type == ENTITY_ENEMY_FIRE_KEESE ||
                  e->type == ENTITY_ENEMY_MOBLIN ||
                  e->type == ENTITY_ENEMY_PEAHAT ||
                  e->type == ENTITY_ENEMY_TEKTITE ||
                  e->type == ENTITY_ENEMY_SPINY_BEETLE ||
                  e->type == ENTITY_ENEMY_ROPE ||
                  (e->type == ENTITY_ENEMY_CHUCHU && e->action > 0)) &&
                 e->invulnerableTimer <= 0) {

            // Se voando alto demais desvia do projétil
            if ((e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE || e->type == ENTITY_ENEMY_PEAHAT || e->type == ENTITY_ENEMY_TEKTITE) && e->z > 8.0f) continue;

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
                float force = (e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE) ? 3.8f : 2.6f;

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

bool entity_check_pacci_hit(float px, float py, float pw, float ph) {
    float px1 = px;
    float py1 = py;
    float px2 = px + pw;
    float py2 = py + ph;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active) continue;

        float ex1 = e->x + e->hitbox.offset_x;
        float ey1 = e->y + e->hitbox.offset_y;
        float ex2 = ex1 + e->hitbox.width;
        float ey2 = ey1 + e->hitbox.height;

        if (px1 < ex2 && px2 > ex1 && py1 < ey2 && py2 > ey1) {
            // 1. Spiny Beetle: VIRA DE CABEÇA PARA BAIXO! (Flipped / Exposed belly)
            if (e->type == ENTITY_ENEMY_SPINY_BEETLE) {
                e->action = 3; // Modo Flipped
                e->aiTimer = 240; // 4 segundos inteiros imobilizado esperneando
                e->damage = 0; // Inofensivo enquanto virado
                e->invulnerableTimer = 15;
                e->knockbackTimer = 0;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.6f);
                printf("[CANE OF PACCI] Spiny Beetle virado de costas! Carapaca invertida, barriga vulneravel!\n");
                return true;
            }
            // 1b. Chefe Gleerok: Cajado de Pacci inverte a carapaça rochosa e derruba o pescoço!
            else if (e->type == ENTITY_BOSS_GLEEROK) {
                if (e->action == 1) {
                    e->action = 3; // Toppled! Carapaça virada e pescoço estendido como ponte!
                    e->aiTimer = 360; // 6 segundos de vulnerabilidade
                    e->invulnerableTimer = 15;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.8f);
                    entity_trigger_screen_shake(15, 6);
                    printf("[CANE OF PACCI] Gleerok atingido pela magia de Pacci! Carapaca virada e nucleo exposto!\n");
                    return true;
                }
            }
            // 2. Inimigos normais (Octorok, Keese, ChuChu, Moblin, Peahat, Tektite):
            // Sofrem forte impacto mágico de inversão e atordoamento
            else if (e->type == ENTITY_ENEMY_OCTOROK ||
                     e->type == ENTITY_ENEMY_KEESE ||
                     e->type == ENTITY_ENEMY_FIRE_KEESE ||
                     e->type == ENTITY_ENEMY_MOBLIN ||
                     e->type == ENTITY_ENEMY_PEAHAT ||
                     e->type == ENTITY_ENEMY_TEKTITE ||
                     (e->type == ENTITY_ENEMY_CHUCHU && e->action > 0)) {
                e->invulnerableTimer = 24;
                e->action = 4; // Knockback / Stun
                e->knockbackTimer = 20;
                if (e->z > 0.0f) e->z = 0.0f; // Derruba monstros voadores ao solo

                float dx = e->x - px;
                float dy = e->y - py;
                float dist = sqrtf(dx * dx + dy * dy);
                float force = 4.2f;
                if (dist > 0.1f) {
                    e->knockbackVx = (dx / dist) * force;
                    e->knockbackVy = (dy / dist) * force;
                } else {
                    e->knockbackVx = force;
                    e->knockbackVy = 0.0f;
                }

                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.45f);
                printf("[CANE OF PACCI] Inimigo atingido por rajada de energia e atordoado!\n");
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
        // Puxar Octorok, Keese e Fire Keese
        else if (e->type == ENTITY_ENEMY_OCTOROK || e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE) {
            e->x += pull_x * 0.85f;
            e->y += pull_y * 0.85f;
        }
        // Puxar Peahat para o chão e puxar Moblin
        else if (e->type == ENTITY_ENEMY_PEAHAT) {
            e->x += pull_x * 0.85f;
            e->y += pull_y * 0.85f;
            if (e->z > 0.0f) {
                e->z -= 0.6f;
                if (e->z <= 0.0f) {
                    e->z = 0.0f;
                    e->action = 2; // Derrubado ao solo!
                    e->aiTimer = 60;
                }
            }
        }
        else if (e->type == ENTITY_ENEMY_MOBLIN) {
            e->x += pull_x * 0.45f;
            e->y += pull_y * 0.45f;
        }
        else if (e->type == ENTITY_ENEMY_TEKTITE || e->type == ENTITY_ENEMY_SPINY_BEETLE) {
            e->x += pull_x * 0.75f;
            e->y += pull_y * 0.75f;
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

            const Texture* use_tex = s_enemies_tex ? s_enemies_tex : s_octo_tex;
            if (use_tex && use_tex->pixels) {
                int src_x = 0;
                int src_y = 0;
                bool flip_h = false;

                if (s_enemies_tex) {
                    if (e->action == 2) {
                        if (e->dir == DIR_DOWN)       src_x = 4 * 16;
                        else if (e->dir == DIR_UP)    src_x = 1 * 16;
                        else if (e->dir == DIR_RIGHT) src_x = 5 * 16;
                        else if (e->dir == DIR_LEFT)  { src_x = 5 * 16; flip_h = true; }
                    } else {
                        if (e->dir == DIR_DOWN)       src_x = 0 * 16;
                        else if (e->dir == DIR_UP)    src_x = 1 * 16;
                        else if (e->dir == DIR_RIGHT) src_x = (e->animFrame == 0) ? 2 * 16 : 3 * 16;
                        else if (e->dir == DIR_LEFT)  { src_x = (e->animFrame == 0) ? 2 * 16 : 3 * 16; flip_h = true; }
                    }
                    src_y = 0;
                } else {
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
                }

                texture_draw_ex(use_tex, src_x, src_y, 16, 16, sx, sy, flip_h);
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
            if (s_enemies_tex && s_enemies_tex->pixels) {
                texture_draw(s_enemies_tex, 6 * 16, 0, 16, 16, sx, sy);
            } else if (s_octo_tex && s_octo_tex->pixels) {
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

        // 6. INIMIGO: KEESE & FIRE KEESE (MORCEGO VOADOR / EM CHAMAS)
        else if (e->type == ENTITY_ENEMY_KEESE || e->type == ENTITY_ENEMY_FIRE_KEESE) {
            // Sombra oval no chão (projeção de altitude 3D no terreno)
            if (s_enemies_tex && s_enemies_tex->pixels) {
                texture_draw(s_enemies_tex, 5 * 16, 2 * 16, 16, 16, sx, sy);
            } else {
                draw_filled_rect(sx + 3, sy + 12, 10, 3, 0x05100766);
                draw_filled_rect(sx + 4, sy + 11, 8, 4, 0x05100766);
            }

            // Piscar ao receber dano da espada
            if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 0)) {
                continue;
            }

            int by = sy - (int)e->z;
            bool is_fire = (e->type == ENTITY_ENEMY_FIRE_KEESE);

            if (s_enemies_tex && s_enemies_tex->pixels) {
                bool wings_up = ((e->animTimer / 6) % 2 == 0);
                int src_x = is_fire ? (wings_up ? 3 * 16 : 4 * 16) : (wings_up ? 0 * 16 : 1 * 16);
                texture_draw(s_enemies_tex, src_x, 2 * 16, 16, 16, sx, by);
            } else {
                u32 c_body  = is_fire ? 0x7F1D1DFF : 0x2A0E3DFF; // Vermelho escuro ou Roxo escuro do corpo
                u32 c_wing  = is_fire ? 0xEA580CFF : 0x58247DFF; // Asas laranja-fogo ou violeta
                u32 c_rib   = is_fire ? 0xFBBF24FF : 0x8239B5FF; // Nervuras douradas de chama ou nervuras das asas
                u32 c_eye   = is_fire ? 0xFEF08AFF : 0xFFD700FF; // Olhos amarelos incandescentes
                u32 c_pupil = 0xEE1100FF; // Íris vermelha

                // Partículas de fagulhas de fogo para o Fire Keese
                if (is_fire) {
                    int f_tick = (e->animTimer / 3) % 4;
                    put_pixel_safe(sx + 2 + f_tick, by - 2 + (f_tick % 2), 0xFDE047FF);
                    put_pixel_safe(sx + 12 - f_tick, by - 1 - (f_tick % 2), 0xEF4444FF);
                    put_pixel_safe(sx + 7 + ((f_tick * 3) % 5) - 2, by + 10, 0xF97316FF);
                }

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
        }

        // 7. INIMIGO: GREEN CHUCHU (GOSMA GELATINOSA)
        else if (e->type == ENTITY_ENEMY_CHUCHU) {
            // Piscar ao receber dano
            if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 0)) {
                continue;
            }

            if (s_enemies_tex && s_enemies_tex->pixels) {
                int src_x = 2 * 16; // Standing
                int draw_cy = sy;
                if (e->action == 0) {
                    src_x = 0 * 16; // Puddle
                } else if (e->action == 1) {
                    src_x = 1 * 16; // Emerging
                } else if (e->action == 2 || e->action == 4) {
                    src_x = (e->action == 4) ? 5 * 16 : 3 * 16; // Wobble / Squash
                } else if (e->action == 3) {
                    src_x = 4 * 16; // Stretch jump
                    draw_cy = sy - (int)e->z;
                    texture_draw(s_enemies_tex, 5 * 16, 2 * 16, 16, 16, sx, sy);
                }
                texture_draw(s_enemies_tex, src_x, 1 * 16, 16, 16, sx, draw_cy);
            } else {
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
                if (s_enemies_tex && s_enemies_tex->pixels) {
                    texture_draw(s_enemies_tex, 3 * 32, 6 * 16, 32, 32, boss_cx - 16, boss_cy - 12);
                } else {
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
                if (s_enemies_tex && s_enemies_tex->pixels) {
                    int src_col = 0; // Col 0: Idle
                    if (e->bossEnraged) {
                        src_col = 2; // Col 2: Shock / Enraged
                    } else if (e->subAction == 0 && e->aiTimer < 14) {
                        src_col = 1; // Col 1: Squash
                    }
                    int wobble_x = 0;
                    if (e->action == 2) {
                        wobble_x = (int)(sinf((float)e->animTimer * 0.55f) * 6.0f);
                    }
                    texture_draw(s_enemies_tex, src_col * 32, 6 * 16, 32, 32, boss_cx - 16 + wobble_x, boss_cy - 16);
                } else {
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

        // 10. NPC: MESTRE ESPADACHIM SWIFTBLADE (BLADE BROTHER)
        else if (e->type == ENTITY_NPC_SWIFTBLADE) {
            int breathe = ((e->animTimer / 18) % 2 == 1) ? 1 : 0;
            int sy_b = sy - breathe;

            if (s_npcs_tex && s_npcs_tex->pixels) {
                int src_x = 0;
                bool flip_h = false;
                if (e->dir == DIR_DOWN) {
                    src_x = 0 * 16;
                } else if (e->dir == DIR_UP) {
                    src_x = 2 * 16;
                } else if (e->dir == DIR_RIGHT) {
                    src_x = 1 * 16;
                    flip_h = false;
                } else { // DIR_LEFT
                    src_x = 1 * 16;
                    flip_h = true;
                }
                texture_draw_ex(s_npcs_tex, src_x, 0 * 16, 16, 16, sx, sy_b, flip_h);
            } else {
                u32 c_fur       = 0x7A6A5AFF; // Pelo canino marrom-acinzentado do mestre
                u32 c_fur_dk    = 0x544638FF;
                u32 c_muzzle    = 0xC2B2A0FF;
                u32 c_nose      = 0x1C140FFF;
                u32 c_band      = 0xDC2626FF; // Faixa vermelha marcial
                u32 c_band_dk   = 0x991B1BFF;
                u32 c_gi        = 0x1E3324FF; // Gi verde floresta dojo
                u32 c_belt      = 0xE5E7EBFF; // Faixa branca
                u32 c_wood      = 0x92400EFF; // Espada de madeira de treino (Bokken)
                u32 c_wood_hi   = 0xD97706FF;

                // Orelhas caninas
                put_pixel_safe(sx + 4, sy_b + 0, c_fur);
                put_pixel_safe(sx + 5, sy_b + 1, c_fur);
                put_pixel_safe(sx + 10, sy_b + 1, c_fur);
                put_pixel_safe(sx + 11, sy_b + 0, c_fur);

                // Cabeça
                draw_filled_rect(sx + 4, sy_b + 2, 8, 5, c_fur);

                // Faixa vermelha marcial na testa
                draw_filled_rect(sx + 3, sy_b + 3, 10, 2, c_band);
                put_pixel_safe(sx + 2, sy_b + 4, c_band_dk); // Nó
                put_pixel_safe(sx + 1, sy_b + 5, c_band);    // Fita pendente

                // Olhos e focinho
                if (e->dir == DIR_DOWN) {
                    put_pixel_safe(sx + 5, sy_b + 5, 0x111111FF);
                    put_pixel_safe(sx + 10, sy_b + 5, 0x111111FF);
                    draw_filled_rect(sx + 6, sy_b + 6, 4, 2, c_muzzle);
                    put_pixel_safe(sx + 7, sy_b + 6, c_nose);
                    put_pixel_safe(sx + 8, sy_b + 6, c_nose);
                } else if (e->dir == DIR_LEFT) {
                    put_pixel_safe(sx + 4, sy_b + 5, 0x111111FF);
                    draw_filled_rect(sx + 3, sy_b + 6, 3, 2, c_muzzle);
                    put_pixel_safe(sx + 3, sy_b + 6, c_nose);
                } else if (e->dir == DIR_RIGHT) {
                    put_pixel_safe(sx + 11, sy_b + 5, 0x111111FF);
                    draw_filled_rect(sx + 10, sy_b + 6, 3, 2, c_muzzle);
                    put_pixel_safe(sx + 12, sy_b + 6, c_nose);
                } else { // DIR_UP
                    put_pixel_safe(sx + 8, sy_b + 4, c_band);
                    put_pixel_safe(sx + 9, sy_b + 5, c_band_dk);
                }

                // Corpo / Kimono do Mestre
                draw_filled_rect(sx + 4, sy_b + 8, 8, 5, c_gi);
                draw_filled_rect(sx + 4, sy_b + 11, 8, 2, c_belt);

                // Pernas / Calças
                draw_filled_rect(sx + 4, sy_b + 13, 3, 3, c_gi);
                draw_filled_rect(sx + 9, sy_b + 13, 3, 3, c_gi);

                // Espada de madeira de treino (Bokken)
                draw_filled_rect(sx + 12, sy_b + 7, 2, 7, c_wood);
                put_pixel_safe(sx + 12, sy_b + 6, c_wood_hi);
                put_pixel_safe(sx + 13, sy_b + 6, c_wood_hi);
            }

            // Balão de interação [A] Treinar quando Link se aproxima
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 30.0f * 30.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 22;
                int prompt_y = sy_b - 15 + bounce;

                draw_filled_rect(prompt_x - 1, prompt_y - 1, 62, 10, 0x180D0AEF);
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 62, 1, 0xDC2626FF);
                draw_filled_rect(prompt_x - 1, prompt_y + 8, 62, 1, 0x991B1BFF);
                font_draw_text(prompt_x + 2, prompt_y, "[A] Treinar", 0xFCA5A5FF, true);
            }
        }

        // 9. NPC: COMERCIANTE STOCKWELL (LOJA DE HYRULE)
        else if (e->type == ENTITY_NPC_SHOPKEEPER) {
            int breathe = ((e->animTimer / 16) % 2 == 1) ? 1 : 0;
            int sy_b = sy - breathe;

            if (s_npcs_tex && s_npcs_tex->pixels) {
                int anim = (e->animTimer / 30) % 3;
                texture_draw(s_npcs_tex, anim * 16, 6 * 16, 16, 16, sx, sy_b);
            } else {
                u32 c_cap       = 0x4A148CFF; // Boina roxa do comerciante
                u32 c_cap_trim  = 0xF1C40FFF; // Fita dourada na boina
                u32 c_skin      = 0xFDE8CDFF; // Pele clara
                u32 c_hair      = 0x5D4037FF; // Cabelo castanho
                u32 c_glasses   = 0xF59E0BFF; // Óculos redondos dourados
                u32 c_lens      = 0xE0F2FEFF; // Vidro dos óculos
                u32 c_apron     = 0x059669FF; // Avental verde esmeralda
                u32 c_shirt     = 0xFFFFFFFF; // Camisa branca
                u32 c_tie       = 0xDC2626FF; // Gravata borboleta vermelha

                draw_filled_rect(sx + 3, sy + 13, 10, 3, 0x05100766);

                // Boina mercantil
                draw_filled_rect(sx + 4, sy_b + 0, 8, 3, c_cap);
                draw_filled_rect(sx + 3, sy_b + 2, 10, 2, c_cap);
                draw_filled_rect(sx + 4, sy_b + 3, 8, 1, c_cap_trim);

                // Rosto
                draw_filled_rect(sx + 4, sy_b + 4, 8, 5, c_skin);
                put_pixel_safe(sx + 3, sy_b + 4, c_hair);
                put_pixel_safe(sx + 12, sy_b + 4, c_hair);

                // Óculos redondos de Stockwell
                if (e->dir != DIR_UP) {
                    put_pixel_safe(sx + 5, sy_b + 5, c_glasses);
                    put_pixel_safe(sx + 6, sy_b + 5, c_lens);
                    put_pixel_safe(sx + 7, sy_b + 5, c_glasses);
                    put_pixel_safe(sx + 9, sy_b + 5, c_glasses);
                    put_pixel_safe(sx + 10, sy_b + 5, c_lens);
                    put_pixel_safe(sx + 11, sy_b + 5, c_glasses);
                    put_pixel_safe(sx + 8, sy_b + 5, c_glasses);
                    put_pixel_safe(sx + 7, sy_b + 7, c_hair);
                    put_pixel_safe(sx + 8, sy_b + 7, c_hair);
                }

                // Camisa branca e gravata
                draw_filled_rect(sx + 5, sy_b + 9, 6, 2, c_shirt);
                put_pixel_safe(sx + 7, sy_b + 9, c_tie);
                put_pixel_safe(sx + 8, sy_b + 9, c_tie);

                // Avental verde
                draw_filled_rect(sx + 4, sy_b + 11, 8, 4, c_apron);
            }

            // Prompt de compras da loja quando Link se aproxima
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 38.0f * 38.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 32;
                int prompt_y = sy_b - 20 + bounce;

                draw_filled_rect(prompt_x - 1, prompt_y - 1, 80, 14, 0x062816F0);
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 80, 1, 0xD4AF37FF);
                draw_filled_rect(prompt_x - 1, prompt_y + 12, 80, 1, 0xD4AF37FF);
                font_draw_text(prompt_x + 2, prompt_y + 2, "[A] Comprar (Loja)", 0xFDE047FF, true);
            }
        }

        // 10. NPC: CIDADÃ DE HYRULE (COM FUSÃO DE KINSTONE)
        else if (e->type == ENTITY_NPC_TOWN_CITIZEN) {
            int breathe = ((e->animTimer / 18) % 2 == 1) ? 1 : 0;
            int cy = sy - breathe;

            if (s_npcs_tex && s_npcs_tex->pixels) {
                int src_x = 0;
                bool flip_h = false;
                if (e->dir == DIR_RIGHT) {
                    src_x = 1 * 16;
                } else if (e->dir == DIR_LEFT) {
                    src_x = 1 * 16;
                    flip_h = true;
                } else {
                    src_x = (e->hasKinstone && !e->kinstoneFused) ? (2 * 16) : 0;
                }
                texture_draw_ex(s_npcs_tex, src_x, 7 * 16, 16, 16, sx, cy, flip_h);
            } else {
                u32 c_bonnet   = 0xF472B6FF; // Touca rosa
                u32 c_bonnet_lt= 0xFBCFE8FF;
                u32 c_skin     = 0xFDE8CDFF; // Pele clara
                u32 c_hair     = 0xD97706FF; // Cabelos ruivos
                u32 c_blush    = 0xFCA5A5FF; // Bochechas
                u32 c_dress    = 0x38BDF8FF; // Vestido azul
                u32 c_apron    = 0xFFFFFFFF; // Avental branco

                draw_filled_rect(sx + 3, sy + 13, 10, 3, 0x05100766);

                // Touca / Chapéu com laço
                draw_filled_rect(sx + 4, cy + 1, 8, 3, c_bonnet);
                draw_filled_rect(sx + 3, cy + 3, 10, 2, c_bonnet_lt);

                // Cabelos
                draw_filled_rect(sx + 4, cy + 4, 8, 2, c_hair);
                put_pixel_safe(sx + 3, cy + 5, c_hair);
                put_pixel_safe(sx + 12, cy + 5, c_hair);

                // Rosto e olhos
                draw_filled_rect(sx + 4, cy + 6, 8, 4, c_skin);
                if (e->dir != DIR_UP) {
                    put_pixel_safe(sx + 5, cy + 7, 0x111111FF);
                    put_pixel_safe(sx + 10, cy + 7, 0x111111FF);
                    put_pixel_safe(sx + 4, cy + 8, c_blush);
                    put_pixel_safe(sx + 11, cy + 8, c_blush);
                    put_pixel_safe(sx + 7, cy + 8, 0xE11D48FF);
                    put_pixel_safe(sx + 8, cy + 8, 0xE11D48FF);
                }

                // Vestido azul e avental branco
                draw_filled_rect(sx + 4, cy + 10, 8, 5, c_dress);
                draw_filled_rect(sx + 6, cy + 10, 4, 4, c_apron);
            }

            // Balão flutuante de Kinstone se pendente
            if (e->hasKinstone && !e->kinstoneFused) {
                int bubble_y = cy - 16 + (int)(sinf(e->bubbleBob) * 2.0f);
                int bubble_x = sx + 8;

                draw_filled_rect(bubble_x - 7, bubble_y - 6, 14, 12, 0xFFFFFFFF);
                draw_filled_rect(bubble_x - 8, bubble_y - 4, 16, 8, 0xFFFFFFFF);
                draw_filled_rect(bubble_x - 6, bubble_y - 7, 12, 14, 0xFFFFFFFF);

                u32 c_kinstone = 0x3B82F6FF;
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

            // Prompt se próximo
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 28.0f * 28.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 16;
                int prompt_y = cy - 15 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 48, 10, 0x1A1424F0);
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 48, 1, 0xF472B6FF);
                font_draw_text(prompt_x + 2, prompt_y, "[A] Conversar", 0xFBCFE8FF, true);
            }
        }

        // 11. NPC: GUARDA REAL DO CASTELO DE HYRULE
        else if (e->type == ENTITY_NPC_TOWN_GUARD) {
            if (s_npcs_tex && s_npcs_tex->pixels) {
                int src_x = 0;
                bool flip_h = false;
                if (e->dir == DIR_RIGHT) {
                    src_x = 1 * 16;
                } else if (e->dir == DIR_LEFT) {
                    src_x = 1 * 16;
                    flip_h = true;
                } else {
                    float gdx = s_last_link_x - e->x;
                    float gdy = s_last_link_y - e->y;
                    src_x = (gdx * gdx + gdy * gdy <= 28.0f * 28.0f) ? (2 * 16) : 0;
                }
                texture_draw_ex(s_npcs_tex, src_x, 8 * 16, 16, 16, sx, sy, flip_h);
            } else {
                u32 c_steel     = 0xD1D5DBFF; // Aço brilhante
                u32 c_steel_dk  = 0x6B7280FF; // Sombra do metal
                u32 c_plume     = 0xDC2626FF; // Pluma vermelha no elmo
                u32 c_tunic     = 0x1E3A8AFF; // Azul real
                u32 c_visor     = 0x111827FF; // Viseira
                u32 c_spear     = 0x78350FFF; // Lança
                u32 c_blade     = 0xF3F4F6FF; // Ponta de aço

                draw_filled_rect(sx + 3, sy + 14, 10, 3, 0x05100766);

                // Pluma vermelha
                draw_filled_rect(sx + 7, sy - 2, 3, 4, c_plume);
                put_pixel_safe(sx + 6, sy - 1, c_plume);

                // Elmo de ferro
                draw_filled_rect(sx + 4, sy + 2, 8, 6, c_steel);
                draw_filled_rect(sx + 5, sy + 1, 6, 2, c_steel);
                put_pixel_safe(sx + 4, sy + 7, c_steel_dk);
                put_pixel_safe(sx + 11, sy + 7, c_steel_dk);

                if (e->dir != DIR_UP) {
                    draw_filled_rect(sx + 5, sy + 4, 6, 2, c_visor);
                    put_pixel_safe(sx + 7, sy + 4, 0x60A5FAFF);
                }

                // Armadura e manto
                draw_filled_rect(sx + 4, sy + 8, 8, 5, c_steel);
                draw_filled_rect(sx + 5, sy + 9, 6, 3, c_steel_dk);
                draw_filled_rect(sx + 4, sy + 13, 8, 2, c_tunic);

                // Alabarda em prontidão
                draw_filled_rect(sx + 13, sy - 4, 1, 19, c_spear);
                draw_filled_rect(sx + 12, sy - 7, 3, 4, c_blade);
                put_pixel_safe(sx + 13, sy - 8, 0xFFFFFFFF);
            }

            // Prompt se próximo
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 28.0f * 28.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 16;
                int prompt_y = sy - 16 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 48, 10, 0x0F172AF0);
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 48, 1, 0x60A5FAFF);
                font_draw_text(prompt_x + 2, prompt_y, "[A] Guarda", 0x93C5FDFF, true);
            }
        }

        // 12. ELEMENTO DINÂMICO: JATOS D'ÁGUA DO CHAFARIZ CENTRAL
        else if (e->type == ENTITY_TOWN_FOUNTAIN) {
            u32 c_spray1 = 0xBAE6FDFF;
            u32 c_spray2 = 0x38BDF8FF;
            u32 c_white  = 0xFFFFFFFF;

            int f_cx = sx + 8;
            int f_cy = sy + 8;

            for (int p = 0; p < 8; p++) {
                float phase = (float)(e->animTimer * 4 + p * 45) * (PI_F / 180.0f);
                int ox = (int)(cosf(phase) * (4.0f + 2.0f * sinf((float)e->animTimer * 0.1f)));
                int oy = (int)(sinf(phase * 1.5f) * 5.0f);

                put_pixel_safe(f_cx + ox, f_cy + oy - 4, (p % 2 == 0) ? c_spray1 : c_spray2);
                if (p % 3 == 0) {
                    put_pixel_safe(f_cx + ox, f_cy + oy - 5, c_white);
                }
            }
            draw_filled_rect(f_cx - 1, f_cy - 7, 3, 6, c_spray1);
            put_pixel_safe(f_cx, f_cy - 8, c_white);
        }

        // 13. TOCO DE ÁRVORE / VASO MINISH (MINISH PORTAL)
        else if (e->type == ENTITY_MINISH_STUMP) {
            int cx = sx + 8;
            int cy = sy + 8;

            // Se for portal Vaso (Town, action == 1): desenha o vaso cerâmico ornamental
            if (e->action == 1) {
                u32 c_clay_dark  = 0x1E4620FF;
                u32 c_clay_med   = 0x2D7A32FF;
                u32 c_clay_light = 0x48B850FF;
                u32 c_gold       = 0xF5B041FF;
                u32 c_portal     = 0x00FFCCFF;

                // Base do vaso
                draw_filled_rect(sx + 4, sy + 13, 8, 3, c_clay_dark);
                draw_filled_rect(sx + 5, sy + 14, 6, 1, c_gold);

                // Bojo do vaso
                draw_filled_rect(sx + 2, sy + 6, 12, 7, c_clay_med);
                draw_filled_rect(sx + 3, sy + 7, 10, 5, c_clay_light);
                draw_filled_rect(sx + 2, sy + 9, 12, 2, c_gold);

                // Gargalo
                draw_filled_rect(sx + 4, sy + 3, 8, 3, c_clay_dark);

                // Abertura superior com luz mágica de portal
                draw_filled_rect(sx + 3, sy + 1, 10, 2, c_clay_dark);
                draw_filled_rect(sx + 4, sy + 1, 8, 2, c_portal);

                // Alças laterais
                put_pixel_safe(sx + 1, sy + 7, c_gold);
                put_pixel_safe(sx + 1, sy + 8, c_gold);
                put_pixel_safe(sx + 14, sy + 7, c_gold);
                put_pixel_safe(sx + 14, sy + 8, c_gold);
            }

            // Aura Mística do Portal Minish (Emanações de partículas mágicas e anel pulsante)
            float pulse = 0.5f + 0.5f * sinf((float)e->animTimer * 0.08f);
            int ring_r = 7 + (int)(pulse * 2.0f);
            u32 c_aura = (e->animTimer % 40 < 20) ? 0x00FFCCAA : 0x70FF80AA;

            // Anel de luz esmeralda no topo do toco/vaso
            for (int a = 0; a < 16; a++) {
                float ang = (float)a * (2.0f * PI_F / 16.0f);
                int rx = cx + (int)(cosf(ang) * (float)ring_r);
                int ry = cy + (int)(sinf(ang) * ((float)ring_r * 0.60f));
                put_pixel_safe(rx, ry, c_aura);
            }

            // Partículas orbitantes de poeira mágica Minish
            for (int p = 0; p < 4; p++) {
                float ang = (float)e->animTimer * 0.05f + (float)p * (PI_F / 2.0f);
                float dist = 6.0f + 3.0f * sinf((float)e->animTimer * 0.1f + p);
                int sp_x = cx + (int)(cosf(ang) * dist);
                int sp_y = cy + (int)(sinf(ang) * (dist * 0.65f));
                put_pixel_safe(sp_x, sp_y, (p % 2 == 0) ? 0xFFFFFFFF : 0x00FFAAFF);
                put_pixel_safe(sp_x + 1, sp_y, 0x00FFCC88);
            }
        }

        // 14. NPC: ANCIÃO GENTARI (ELDER GENTARI DO SANTUÁRIO)
        else if (e->type == ENTITY_NPC_GENTARI) {
            int breathe = ((e->animTimer / 18) % 2 == 1) ? 1 : 0;
            int gy = sy - breathe;

            u32 c_mitre    = 0xF59E0BFF; // Mitra sagrada dourada
            u32 c_gem      = 0x10B981FF; // Joia esmeralda Picori
            u32 c_beard    = 0xFFFFFFFF; // Longa barba branca do anciao
            u32 c_skin     = 0xFDE8CDFF; // Pele clara
            u32 c_robe     = 0x7C3AEDFF; // Manto violeta cerimonial
            u32 c_gold_trim= 0xFCD34DFF; // Detalhes dourados do manto
            u32 c_eye      = 0x1E1B4BFF; // Olhos sabios

            draw_filled_rect(sx + 3, sy + 13, 10, 3, 0x05100766); // Sombra

            // Mitra cerimonial do Anciao Picori
            draw_filled_rect(sx + 6, gy - 2, 4, 3, c_mitre);
            draw_filled_rect(sx + 5, gy + 1, 6, 3, c_mitre);
            put_pixel_safe(sx + 7, gy - 3, c_gem);
            put_pixel_safe(sx + 8, gy - 3, c_gem);

            // Rosto e orelhas
            draw_filled_rect(sx + 5, gy + 4, 6, 4, c_skin);
            put_pixel_safe(sx + 4, gy + 5, c_skin);
            put_pixel_safe(sx + 11, gy + 5, c_skin);

            if (e->dir != DIR_UP) {
                put_pixel_safe(sx + 6, gy + 5, c_eye);
                put_pixel_safe(sx + 9, gy + 5, c_eye);
                // Longa barba branca de Anciao
                draw_filled_rect(sx + 6, gy + 7, 4, 6, c_beard);
                draw_filled_rect(sx + 7, gy + 13, 2, 2, c_beard);
            } else {
                // De costas: capuz e manto cobrem
                draw_filled_rect(sx + 5, gy + 4, 6, 8, c_robe);
            }

            // Manto cerimonial violeta
            draw_filled_rect(sx + 4, gy + 8, 8, 7, c_robe);
            draw_filled_rect(sx + 7, gy + 8, 2, 7, c_gold_trim);

            // Prompt quando Link estiver perto (<= 28px)
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 28.0f * 28.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 16;
                int prompt_y = gy - 16 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 52, 10, 0x1A0D28F0);
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 52, 1, 0xF59E0BFF);
                font_draw_text(prompt_x + 2, prompt_y, "[A] Gentari", 0xFDE047FF, true);
            }
        }

        // 15. NPC: SACERDOTE FESTARI (ERMIDA / DEEPWOOD SHRINE)
        else if (e->type == ENTITY_NPC_FESTARI) {
            int breathe = ((e->animTimer / 18) % 2 == 1) ? 1 : 0;
            int fy = sy - breathe;

            u32 c_hood     = 0x2563EBFF; // Capuz azul de sacerdote Picori
            u32 c_hood_trim= 0xE0E7FFFF; // Borda branca do capuz
            u32 c_skin     = 0xFDE8CDFF; // Pele clara
            u32 c_robe     = 0x1D4ED8FF; // Habito azul
            u32 c_stole    = 0xF59E0BFF; // Estola dourada sacerdotal
            u32 c_eye      = 0x0F172AFF; // Olhos

            draw_filled_rect(sx + 3, sy + 13, 10, 3, 0x05100766);

            // Capuz de monge
            draw_filled_rect(sx + 5, fy + 0, 6, 3, c_hood);
            draw_filled_rect(sx + 4, fy + 3, 8, 4, c_hood);
            draw_filled_rect(sx + 4, fy + 3, 8, 1, c_hood_trim);

            // Rosto e orelhas
            draw_filled_rect(sx + 5, fy + 4, 6, 4, c_skin);
            put_pixel_safe(sx + 4, fy + 5, c_skin);
            put_pixel_safe(sx + 11, fy + 5, c_skin);

            if (e->dir != DIR_UP) {
                put_pixel_safe(sx + 6, fy + 5, c_eye);
                put_pixel_safe(sx + 9, fy + 5, c_eye);
            } else {
                draw_filled_rect(sx + 5, fy + 4, 6, 4, c_hood);
            }

            // Habito azul e estola sacerdotal
            draw_filled_rect(sx + 4, fy + 8, 8, 7, c_robe);
            draw_filled_rect(sx + 6, fy + 8, 4, 7, c_stole);

            // Prompt quando Link estiver perto (<= 28px)
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 28.0f * 28.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 16;
                int prompt_y = fy - 16 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 50, 10, 0x0D1F38F0);
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 50, 1, 0x3B82F6FF);
                font_draw_text(prompt_x + 2, prompt_y, "[A] Festari", 0x93C5FDFF, true);
            }
        }

        // 16. NPC: MORADOR DA VILA DOS MINISH (PICORI VILLAGER)
        else if (e->type == ENTITY_NPC_VILLAGE_MINISH) {
            int breathe = ((e->animTimer / 16) % 2 == 1) ? 1 : 0;
            int my = sy - breathe;

            u32 c_hat   = 0x10B981FF; // Chapeu verde esmeralda
            u32 c_skin  = 0xFDE8CDFF; // Pele clara
            u32 c_tunic = 0xD97706FF; // Tunica terracota
            u32 c_pom   = 0xFDE047FF; // Pom-pom dourado
            u32 c_eye   = 0x111111FF; // Olhos

            // Gorro pontudo
            draw_filled_rect(sx + 6, my + 1, 4, 3, c_hat);
            draw_filled_rect(sx + 5, my + 4, 6, 3, c_hat);
            put_pixel_safe(sx + 7, my, c_pom);
            put_pixel_safe(sx + 8, my, c_pom);

            // Rosto e orelhas
            draw_filled_rect(sx + 5, my + 7, 6, 4, c_skin);
            put_pixel_safe(sx + 4, my + 8, c_skin);
            put_pixel_safe(sx + 11, my + 8, c_skin);

            if (e->dir == DIR_DOWN) {
                put_pixel_safe(sx + 6, my + 8, c_eye);
                put_pixel_safe(sx + 9, my + 8, c_eye);
            } else if (e->dir == DIR_UP) {
                draw_filled_rect(sx + 5, my + 7, 6, 4, c_hat);
            } else if (e->dir == DIR_LEFT) {
                put_pixel_safe(sx + 5, my + 8, c_eye);
            } else if (e->dir == DIR_RIGHT) {
                put_pixel_safe(sx + 10, my + 8, c_eye);
            }

            draw_filled_rect(sx + 5, my + 11, 6, 4, c_tunic);

            // Balao de Fusao de Kinstone sobre a cabeca do Minish (se ainda nao fundiu)
            if (e->hasKinstone && !e->kinstoneFused) {
                int bubble_y = my - 16 + (int)(sinf(e->bubbleBob) * 2.0f);
                int bubble_x = sx + 8;
                draw_filled_rect(bubble_x - 7, bubble_y - 6, 14, 12, 0xFFFFFFFF);
                draw_filled_rect(bubble_x - 8, bubble_y - 4, 16, 8, 0xFFFFFFFF);
                draw_filled_rect(bubble_x - 6, bubble_y - 7, 12, 14, 0xFFFFFFFF);
                u32 c_kinstone = 0x22C55EFF;
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

            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 28.0f * 28.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 16;
                int prompt_y = my - 14 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 48, 10, 0x0A2010EE);
                font_draw_text(prompt_x + 2, prompt_y, "[A] Falar", 0x34D399FF, true);
            }
        }

        // 17. INIMIGO: MOBLIN DOS CAMPOS DE HYRULE
        else if (e->type == ENTITY_ENEMY_MOBLIN) {
            if (e->invulnerableTimer > 0 && (e->invulnerableTimer % 4 < 2)) {
                draw_filled_rect(sx, sy, 16, 16, 0xFFFFFFFF);
            } else if (s_enemies_tex && s_enemies_tex->pixels) {
                int walk_frame = (e->animFrame == 1) ? 1 : 0;
                int src_x = 0;
                bool flip_h = false;
                if (e->dir == DIR_DOWN) {
                    src_x = walk_frame * 16;
                } else if (e->dir == DIR_UP) {
                    src_x = (1 - walk_frame) * 16;
                } else if (e->dir == DIR_RIGHT) {
                    src_x = (e->action == 2) ? (4 * 16) : ((2 + walk_frame) * 16);
                    flip_h = false;
                } else { // DIR_LEFT
                    src_x = (e->action == 2) ? (4 * 16) : ((2 + walk_frame) * 16);
                    flip_h = true;
                }
                texture_draw_ex(s_enemies_tex, src_x, 3 * 16, 16, 16, sx, sy, flip_h);
            } else {
                int walk_bob = (e->animFrame == 1) ? 1 : 0;
                int my = sy - walk_bob;

                u32 c_skin  = 0xFB923CFF; // Laranja suíno
                u32 c_armor = 0xB45309FF; // Armadura de couro
                u32 c_snout = 0xF43F5EFF; // Focinho rosado
                u32 c_tusk  = 0xFFFFFFFF; // Presas brancas
                u32 c_helm  = 0x78350FFF; // Elmo de couro
                u32 c_shaft = 0x78350FFF; // Haste da lança
                u32 c_spear = 0xE2E8F0FF; // Ponta de ferro

                // Cabeça e elmo
                draw_filled_rect(sx + 4, my + 1, 8, 4, c_helm);
                draw_filled_rect(sx + 3, my + 5, 10, 5, c_skin);

                // Orelhas de porco pontudas
                put_pixel_safe(sx + 2, my + 3, c_skin);
                put_pixel_safe(sx + 13, my + 3, c_skin);

                // Focinho e presas
                draw_filled_rect(sx + 6, my + 7, 4, 3, c_snout);
                put_pixel_safe(sx + 5, my + 9, c_tusk);
                put_pixel_safe(sx + 10, my + 9, c_tusk);

                // Olhos
                put_pixel_safe(sx + 5, my + 6, 0x111111FF);
                put_pixel_safe(sx + 10, my + 6, 0x111111FF);

                // Tronco e armadura de couro
                draw_filled_rect(sx + 4, my + 10, 8, 4, c_armor);
                // Botas
                draw_filled_rect(sx + 4, my + 14, 3, 2, 0x451A03FF);
                draw_filled_rect(sx + 9, my + 14, 3, 2, 0x451A03FF);

                // Lança afiada direcionada
                if (e->dir == DIR_DOWN) {
                    for (int y = 0; y < 14; y++) put_pixel_safe(sx + 13, my + 4 + y, c_shaft);
                    draw_filled_rect(sx + 12, my + 17, 3, 3, c_spear);
                } else if (e->dir == DIR_UP) {
                    for (int y = 0; y < 14; y++) put_pixel_safe(sx + 2, my - 2 + y, c_shaft);
                    draw_filled_rect(sx + 1, my - 5, 3, 3, c_spear);
                } else if (e->dir == DIR_LEFT) {
                    for (int x = 0; x < 14; x++) put_pixel_safe(sx - 3 + x, my + 8, c_shaft);
                    draw_filled_rect(sx - 6, my + 7, 3, 3, c_spear);
                } else if (e->dir == DIR_RIGHT) {
                    for (int x = 0; x < 14; x++) put_pixel_safe(sx + 6 + x, my + 8, c_shaft);
                    draw_filled_rect(sx + 19, my + 7, 3, 3, c_spear);
                }
            }
        }

        // 18. INIMIGO: PEAHAT (FLOR-HELICÓPTERO VOADORA)
        else if (e->type == ENTITY_ENEMY_PEAHAT) {
            // Sombra no solo se estiver voando
            if (e->z > 0.0f) {
                if (s_enemies_tex && s_enemies_tex->pixels) {
                    texture_draw(s_enemies_tex, 5 * 16, 2 * 16, 16, 16, sx, sy);
                } else {
                    draw_filled_rect(sx + 4, sy + 12, 8, 3, 0x00000044);
                }
            }

            int py = sy - (int)e->z;
            if (s_enemies_tex && s_enemies_tex->pixels) {
                int src_x = (e->z > 0.0f) ? (3 * 16) : (2 * 16);
                texture_draw(s_enemies_tex, src_x, 4 * 16, 16, 16, sx, py);
            } else {
                u32 c_bulb  = 0xEAB308FF; // Miolo dourado
                u32 c_petal = 0x22C55EFF; // Pétalas verdes giratórias
                u32 c_root  = 0x78350FFF; // Raízes inferiores

                // Bulbo central
                draw_filled_rect(sx + 5, py + 5, 6, 6, c_bulb);
                put_pixel_safe(sx + 7, py + 7, 0xFEF08AFF); // Brilho

                // Hélices/Pétalas rotativas em 4 direções baseadas na animação
                int rot = (e->animTimer / 2) % 4;
                if (rot == 0 || rot == 2) {
                    draw_filled_rect(sx + 1, py + 7, 4, 2, c_petal); // Oeste
                    draw_filled_rect(sx + 11, py + 7, 4, 2, c_petal); // Leste
                    draw_filled_rect(sx + 7, py + 1, 2, 4, c_petal); // Norte
                    draw_filled_rect(sx + 7, py + 11, 2, 4, c_petal); // Sul
                } else {
                    draw_filled_rect(sx + 2, py + 2, 3, 3, c_petal); // Noroeste
                    draw_filled_rect(sx + 11, py + 2, 3, 3, c_petal); // Nordeste
                    draw_filled_rect(sx + 2, py + 11, 3, 3, c_petal); // Sudoeste
                    draw_filled_rect(sx + 11, py + 11, 3, 3, c_petal); // Sudeste
                }

                // Raízes suspensas
                put_pixel_safe(sx + 6, py + 11, c_root);
                put_pixel_safe(sx + 7, py + 12, c_root);
                put_pixel_safe(sx + 9, py + 11, c_root);
            }
        }

        // 19. NPC: MALON (MOÇA DA FAZENDA LON LON)
        else if (e->type == ENTITY_NPC_MALON) {
            int breathe = ((e->animTimer / 16) % 2 == 1) ? 1 : 0;
            int my = sy - breathe;

            if (s_npcs_tex && s_npcs_tex->pixels) {
                int src_x = 0;
                bool flip_h = false;
                if (e->dir == DIR_RIGHT) {
                    src_x = 1 * 16;
                } else if (e->dir == DIR_LEFT) {
                    src_x = 1 * 16;
                    flip_h = true;
                } else {
                    float mdx = s_last_link_x - e->x;
                    float mdy = s_last_link_y - e->y;
                    src_x = (mdx * mdx + mdy * mdy <= 28.0f * 28.0f) ? (2 * 16) : (((e->animTimer / 60) % 2 == 0) ? 0 : (3 * 16));
                }
                texture_draw_ex(s_npcs_tex, src_x, 4 * 16, 16, 16, sx, my, flip_h);
            } else {

            u32 c_hair    = 0xEA580CFF; // Cabelos ruivos ondulados
            u32 c_bandana = 0xFACC15FF; // Faixa amarela no cabelo
            u32 c_skin    = 0xFDE8CDFF; // Pele clara
            u32 c_bodice  = 0x3B82F6FF; // Colete azul
            u32 c_apron   = 0xFEF3C7FF; // Avental branco
            u32 c_cheek   = 0xFB7185FF; // Bochechas coradas

            // Cabelos ruivos e bandana
            draw_filled_rect(sx + 4, my + 1, 8, 4, c_hair);
            draw_filled_rect(sx + 3, my + 5, 10, 5, c_hair);
            draw_filled_rect(sx + 5, my + 2, 6, 2, c_bandana);

            // Rosto amigável
            draw_filled_rect(sx + 5, my + 6, 6, 4, c_skin);
            put_pixel_safe(sx + 6, my + 7, 0x1E293BFF); // Olho esquerdo
            put_pixel_safe(sx + 9, my + 7, 0x1E293BFF); // Olho direito
            put_pixel_safe(sx + 5, my + 8, c_cheek);    // Bochecha
            put_pixel_safe(sx + 10, my + 8, c_cheek);

            // Blusa, colete azul e avental de fazendeira
            draw_filled_rect(sx + 5, my + 10, 6, 4, c_bodice);
            draw_filled_rect(sx + 6, my + 11, 4, 3, c_apron);

            // Botas
            draw_filled_rect(sx + 5, my + 14, 2, 2, 0x78350FFF);
            draw_filled_rect(sx + 9, my + 14, 2, 2, 0x78350FFF);
            }

            // Balão de Fusão de Kinstone Azul (se não fundida)
            if (e->hasKinstone && !e->kinstoneFused) {
                int bubble_y = my - 16 + (int)(sinf(e->bubbleBob) * 2.0f);
                int bubble_x = sx + 8;
                draw_filled_rect(bubble_x - 7, bubble_y - 6, 14, 12, 0xFFFFFFFF);
                draw_filled_rect(bubble_x - 8, bubble_y - 4, 16, 8, 0xFFFFFFFF);
                draw_filled_rect(bubble_x - 6, bubble_y - 7, 12, 14, 0xFFFFFFFF);
                u32 c_kinstone = 0x3B82F6FF; // Kinstone Azul da Fazenda Lon Lon
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

            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 28.0f * 28.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 16;
                int prompt_y = my - 14 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 48, 10, 0x0A2010EE);
                font_draw_text(prompt_x + 2, prompt_y, "[A] Falar", 0x60A5FAFF, true);
            }
        }

        // 20. INIMIGO: TEKTITE (ARACNÍDEO SALTITANTE DO MONTE CRENEL)
        else if (e->type == ENTITY_ENEMY_TEKTITE) {
            if (e->z > 0.0f) {
                if (s_enemies_tex && s_enemies_tex->pixels) {
                    texture_draw(s_enemies_tex, 5 * 16, 2 * 16, 16, 16, sx, sy);
                } else {
                    draw_filled_rect(sx + 4, sy + 13, 8, 3, 0x00000044); // Sombra no solo
                }
            }

            int ty = sy - (int)e->z;
            if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 1)) {
                draw_filled_rect(sx + 2, ty + 2, 12, 12, 0xFFFFFFFF);
            } else if (s_enemies_tex && s_enemies_tex->pixels) {
                int src_x = (e->z > 0.0f) ? (1 * 16) : (0 * 16);
                texture_draw(s_enemies_tex, src_x, 4 * 16, 16, 16, sx, ty);
            } else {
                u32 c_chitin    = (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 1)) ? 0xFFFFFFFF : 0xDC2626FF;
                u32 c_chitin_dk = 0x991B1BFF;
                u32 c_eye       = 0xFEF08AFF;
                u32 c_pupil     = 0x111111FF;
                u32 c_leg       = 0x78350FFF;

                // Carapaça vermelha arredondada
                draw_filled_rect(sx + 5, ty + 4, 6, 6, c_chitin);
                draw_filled_rect(sx + 6, ty + 3, 4, 8, c_chitin);
                put_pixel_safe(sx + 6, ty + 4, c_chitin_dk);
                put_pixel_safe(sx + 9, ty + 4, c_chitin_dk);

                // Olho central amarelo
                put_pixel_safe(sx + 7, ty + 6, c_eye);
                put_pixel_safe(sx + 8, ty + 6, c_eye);
                put_pixel_safe(sx + 7, ty + 7, c_pupil);

                // 4 Patas articuladas de aracnídeo
                if (e->z <= 0.0f) {
                    // No chão: patas abertas para estabilidade
                    put_pixel_safe(sx + 4, ty + 5, c_leg);
                    put_pixel_safe(sx + 3, ty + 4, c_leg);
                    put_pixel_safe(sx + 2, ty + 7, c_leg);
                    put_pixel_safe(sx + 1, ty + 9, c_leg);

                    put_pixel_safe(sx + 11, ty + 5, c_leg);
                    put_pixel_safe(sx + 12, ty + 4, c_leg);
                    put_pixel_safe(sx + 13, ty + 7, c_leg);
                    put_pixel_safe(sx + 14, ty + 9, c_leg);

                    put_pixel_safe(sx + 4, ty + 8, c_leg);
                    put_pixel_safe(sx + 3, ty + 10, c_leg);
                    put_pixel_safe(sx + 2, ty + 12, c_leg);

                    put_pixel_safe(sx + 11, ty + 8, c_leg);
                    put_pixel_safe(sx + 12, ty + 10, c_leg);
                    put_pixel_safe(sx + 13, ty + 12, c_leg);
                } else {
                    // No ar: patas dobradas balísticas
                    put_pixel_safe(sx + 4, ty + 7, c_leg);
                    put_pixel_safe(sx + 3, ty + 9, c_leg);
                    put_pixel_safe(sx + 4, ty + 11, c_leg);

                    put_pixel_safe(sx + 11, ty + 7, c_leg);
                    put_pixel_safe(sx + 12, ty + 9, c_leg);
                    put_pixel_safe(sx + 11, ty + 11, c_leg);
                }
            }
        }

        // 21. INIMIGO: SPINY BEETLE (BESOURO COM CARAPAÇA DE ROCHA ESPINHOSA)
        else if (e->type == ENTITY_ENEMY_SPINY_BEETLE) {
            if (e->action != 3 && s_enemies_tex && s_enemies_tex->pixels) {
                if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 1)) {
                    draw_filled_rect(sx + 2, sy + 2, 12, 12, 0xFFFFFFFF);
                } else {
                    int src_x = (e->action == 0) ? (2 * 16) : (3 * 16);
                    bool flip_h = (e->dir == DIR_LEFT);
                    texture_draw_ex(s_enemies_tex, src_x, 5 * 16, 16, 16, sx, sy, flip_h);
                }
            } else {
                u32 c_rock    = (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 1)) ? 0xFFFFFFFF : 0x64748BFF;
                u32 c_rock_dk = 0x334155FF;
                u32 c_rock_hi = 0x94A3B8FF;
                u32 c_spike   = 0xCBD5E1FF;
                u32 c_eyes    = 0xEF4444FF;
                u32 c_leg     = 0x1E293BFF;

                if (e->action == 3) {
                    // VIRADO DE CABEÇA PARA BAIXO PELO CAJADO DE PACCI!
                    // Carapaça rochosa no chão (invertida)
                    draw_filled_rect(sx + 3, sy + 7, 10, 6, c_rock);
                    draw_filled_rect(sx + 4, sy + 6, 8, 7, c_rock);
                    put_pixel_safe(sx + 3, sy + 10, c_rock_dk);
                    put_pixel_safe(sx + 12, sy + 10, c_rock_dk);

                    // Barriga mole e vulnerável exposta para cima
                    u32 c_belly = (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 2) % 2 == 1)) ? 0xFFFFFFFF : 0xFDE68AFF;
                    draw_filled_rect(sx + 4, sy + 4, 8, 3, c_belly);
                    draw_filled_rect(sx + 5, sy + 3, 6, 2, c_belly);
                    put_pixel_safe(sx + 6, sy + 5, 0xD97706FF); // Nervura central
                    put_pixel_safe(sx + 9, sy + 5, 0xD97706FF);

                    // Patinhas para o ar esperneando freneticamente
                    int kick = (e->animTimer / 3) % 2;
                    put_pixel_safe(sx + 2, sy + 2 + kick, c_leg);
                    put_pixel_safe(sx + 1, sy + 1 + kick, c_leg);
                    put_pixel_safe(sx + 13, sy + 2 + (1 - kick), c_leg);
                    put_pixel_safe(sx + 14, sy + 1 + (1 - kick), c_leg);
                    put_pixel_safe(sx + 4, sy + 1 + (1 - kick), c_leg);
                    put_pixel_safe(sx + 11, sy + 1 + kick, c_leg);

                    // Estrelas de tontura flutuando
                    int star_x = sx + 7 + (int)(cosf((float)e->animTimer * 0.18f) * 6.0f);
                    int star_y = sy - 2 + (int)(sinf((float)e->animTimer * 0.18f) * 2.5f);
                    put_pixel_safe(star_x, star_y, 0xFDE047FF);
                    continue;
                }

                // Patas de inseto scurrying
                int leg_anim = (e->animTimer / 4) % 2;
                if (leg_anim == 0) {
                    put_pixel_safe(sx + 2, sy + 6, c_leg);
                    put_pixel_safe(sx + 1, sy + 7, c_leg);
                    put_pixel_safe(sx + 2, sy + 10, c_leg);
                    put_pixel_safe(sx + 1, sy + 11, c_leg);
                    put_pixel_safe(sx + 13, sy + 6, c_leg);
                    put_pixel_safe(sx + 14, sy + 7, c_leg);
                    put_pixel_safe(sx + 13, sy + 10, c_leg);
                    put_pixel_safe(sx + 14, sy + 11, c_leg);
                } else {
                    put_pixel_safe(sx + 2, sy + 5, c_leg);
                    put_pixel_safe(sx + 1, sy + 6, c_leg);
                    put_pixel_safe(sx + 2, sy + 9, c_leg);
                    put_pixel_safe(sx + 1, sy + 10, c_leg);
                    put_pixel_safe(sx + 13, sy + 5, c_leg);
                    put_pixel_safe(sx + 14, sy + 6, c_leg);
                    put_pixel_safe(sx + 13, sy + 9, c_leg);
                    put_pixel_safe(sx + 14, sy + 10, c_leg);
                }

                // Carapaça rochosa pontiaguda
                draw_filled_rect(sx + 3, sy + 3, 10, 9, c_rock);
                draw_filled_rect(sx + 4, sy + 2, 8, 11, c_rock);

                // Espinhos e relevos de pedra
                put_pixel_safe(sx + 7, sy + 1, c_spike);
                put_pixel_safe(sx + 8, sy + 1, c_spike);
                put_pixel_safe(sx + 2, sy + 5, c_rock_hi);
                put_pixel_safe(sx + 13, sy + 6, c_rock_hi);
                put_pixel_safe(sx + 6, sy + 6, c_rock_dk);
                put_pixel_safe(sx + 9, sy + 7, c_rock_dk);

                // Olhos vermelhos ameaçadores espreitando por baixo da rocha
                if (e->action == 2 || e->dir == DIR_DOWN) {
                    put_pixel_safe(sx + 5, sy + 11, c_eyes);
                    put_pixel_safe(sx + 10, sy + 11, c_eyes);
                }
            }
        }

        // 22. NPC: BUSINESS SCRUB (DEKU SCRUB COMERCIANTE DO MONTE CRENEL)
        else if (e->type == ENTITY_NPC_BUSINESS_SCRUB) {
            int bob = ((e->animTimer / 12) % 2 == 1) ? 1 : 0;
            int by = sy - bob;

            if (s_npcs_tex && s_npcs_tex->pixels) {
                float sdx = s_last_link_x - e->x;
                float sdy = s_last_link_y - e->y;
                int src_x = (0 * 16); // hiding in bush
                if (sdx * sdx + sdy * sdy <= 36.0f * 36.0f) {
                    src_x = (bob == 1) ? (2 * 16) : (3 * 16);
                } else if (sdx * sdx + sdy * sdy <= 60.0f * 60.0f) {
                    src_x = (1 * 16);
                }
                texture_draw(s_npcs_tex, src_x, 5 * 16, 16, 16, sx, by);
            } else {
                u32 c_bush    = 0x16A34AFF; // Folhagem verde
                u32 c_bush_dk = 0x15803DFF;
                u32 c_scrub   = 0x78350FFF; // Madeira Deku
                u32 c_snout   = 0x9A3412FF; // Bico de trombeta
                u32 c_eyes    = 0xFDE047FF; // Olhos amarelos
                u32 c_leaves  = 0x22C55EFF;

                // Arbusto ao redor
                draw_filled_rect(sx + 2, sy + 9, 12, 6, c_bush);
                draw_filled_rect(sx + 4, sy + 7, 8, 4, c_bush);
                put_pixel_safe(sx + 3, sy + 10, c_bush_dk);
                put_pixel_safe(sx + 12, sy + 10, c_bush_dk);

                // Cabeça do Deku Scrub espiando do arbusto
                draw_filled_rect(sx + 5, by + 3, 6, 6, c_scrub);
                // Coroa de folhas
                draw_filled_rect(sx + 4, by + 1, 8, 2, c_leaves);
                put_pixel_safe(sx + 7, by, 0x86EFACFF);

                // Olhos amarelos
                put_pixel_safe(sx + 6, by + 4, c_eyes);
                put_pixel_safe(sx + 9, by + 4, c_eyes);

                // Focinho de madeira
                draw_filled_rect(sx + 6, by + 6, 4, 3, c_snout);
                put_pixel_safe(sx + 7, by + 7, 0x451A03FF);
            }

            // Prompt de interação [A] Falar (Comerciante)
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 32.0f * 32.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 32;
                int prompt_y = by - 16 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 80, 12, 0x1A0F06F0);
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 80, 1, 0xF59E0BFF);
                font_draw_text(prompt_x + 2, prompt_y + 1, "[A] Comerciante", 0xFDE047FF, true);
            }
        }

        // 23. NPC: MESTRE FERREIRO MELARI
        else if (e->type == ENTITY_NPC_MELARI) {
            draw_filled_rect(sx + 3, sy + 13, 10, 3, 0x05100766); // Sombra

            int strike = (e->animTimer / 15) % 4; // Ciclo de forja com o martelo
            int my = sy;

            if (s_npcs_tex && s_npcs_tex->pixels) {
                texture_draw(s_npcs_tex, 0 * 16, 9 * 16, 16, 16, sx, my);
            } else {
                u32 c_skin   = 0xFDE8CDFF;
                u32 c_beard  = 0xE2E8F0FF;
                u32 c_goggle = 0x92400EFF;
                u32 c_lens   = 0xF97316FF;
                u32 c_shirt  = 0xDC2626FF;
                u32 c_apron  = 0x78350FFF;
                u32 c_hammer = 0x64748BFF;
                u32 c_gold   = 0xFACC15FF;

                // Rosto e barba espessa
                draw_filled_rect(sx + 5, my + 4, 6, 4, c_skin);
                // Óculos de proteção na testa
                draw_filled_rect(sx + 4, my + 2, 8, 2, c_goggle);
                put_pixel_safe(sx + 5, my + 2, c_lens);
                put_pixel_safe(sx + 8, my + 2, c_lens);

                // Olhos e sobrancelha
                put_pixel_safe(sx + 6, my + 5, 0x0F172AFF);
                put_pixel_safe(sx + 9, my + 5, 0x0F172AFF);

                // Barba branca cheia de ferreiro
                draw_filled_rect(sx + 5, my + 7, 6, 4, c_beard);
                draw_filled_rect(sx + 6, my + 11, 4, 2, c_beard);

                // Corpo com avental de couro
                draw_filled_rect(sx + 4, my + 8, 8, 7, c_shirt);
                draw_filled_rect(sx + 5, my + 9, 6, 6, c_apron);

                // Martelo de ferreiro em movimento
                if (strike == 0 || strike == 1) {
                    // Martelo erguido alto
                    draw_filled_rect(sx + 12, my, 4, 3, c_hammer);
                    put_pixel_safe(sx + 13, my - 1, c_gold);
                    draw_filled_rect(sx + 11, my + 3, 2, 6, 0x78350FFF); // Cabo
                } else {
                    // Martelo golpeando a bigorna com faíscas
                    draw_filled_rect(sx + 12, my + 9, 4, 3, c_hammer);
                    draw_filled_rect(sx + 11, my + 7, 2, 4, 0x78350FFF);
                    // Faíscas douradas e brancas do impacto
                    put_pixel_safe(sx + 14, my + 8, 0xFACC15FF);
                    put_pixel_safe(sx + 15, my + 7, 0xFFFFFFFF);
                    put_pixel_safe(sx + 13, my + 6, 0xF97316FF);
                }
            }

            // Prompt de interação [A] Melari
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 36.0f * 36.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 16;
                int prompt_y = my - 16 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 52, 12, 0x1A0F06F0);
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 52, 1, 0xF97316FF);
                font_draw_text(prompt_x + 2, prompt_y + 1, "[A] Melari", 0xFDE047FF, true);
            }
        }

        // 24. NPC: MINERADOR MOUNTAIN MINISH
        else if (e->type == ENTITY_NPC_MOUNTAIN_MINISH) {
            draw_filled_rect(sx + 4, sy + 13, 8, 3, 0x05100766); // Sombra

            int breathe = ((e->animTimer / 16) % 2 == 1) ? 1 : 0;
            int my = sy - breathe;

            u32 c_helmet = 0xFACC15FF;
            u32 c_skin   = 0xFDE8CDFF;
            u32 c_cloth  = 0x2563EBFF;
            u32 c_pick   = 0x64748BFF;

            // Capacete amarelo com lâmpada acesa
            draw_filled_rect(sx + 5, my + 1, 6, 4, c_helmet);
            put_pixel_safe(sx + 7, my + 2, 0xFFFFFFFF); // Lâmpada
            put_pixel_safe(sx + 8, my + 2, 0xFEF08AFF);

            // Rosto e orelhas
            draw_filled_rect(sx + 5, my + 5, 6, 3, c_skin);
            put_pixel_safe(sx + 4, my + 5, c_skin);
            put_pixel_safe(sx + 11, my + 5, c_skin);
            put_pixel_safe(sx + 6, my + 6, 0x1E293BFF);
            put_pixel_safe(sx + 9, my + 6, 0x1E293BFF);

            // Macacão azul
            draw_filled_rect(sx + 5, my + 8, 6, 6, c_cloth);

            // Picareta de minerador
            int swing = (e->animTimer / 12) % 3;
            if (swing == 0) {
                draw_filled_rect(sx + 11, my + 4, 3, 2, c_pick);
                draw_filled_rect(sx + 10, my + 6, 2, 5, 0x78350FFF);
            } else {
                draw_filled_rect(sx + 11, my + 9, 3, 2, c_pick);
                draw_filled_rect(sx + 10, my + 7, 2, 3, 0x78350FFF);
                put_pixel_safe(sx + 13, my + 10, 0xFFFFFFFF); // Faísca de mineração
            }

            // Prompt de interação [A] Conversar
            float dx = s_last_link_x - e->x;
            float dy = s_last_link_y - e->y;
            if (dx * dx + dy * dy <= 28.0f * 28.0f) {
                int bounce = ((e->animTimer / 10) % 2 == 1) ? 1 : 0;
                int prompt_x = sx - 16;
                int prompt_y = my - 16 + bounce;
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 52, 12, 0x0F172AF0);
                draw_filled_rect(prompt_x - 1, prompt_y - 1, 52, 1, 0x38BDF8FF);
                font_draw_text(prompt_x + 2, prompt_y + 1, "[A] Conversar", 0xE0F2FEFF, true);
            }
        }

        // 25. PROJETIL: BOLA DE FOGO DO CHEFE GLEEROK
        else if (e->type == ENTITY_PROJECTILE_FIREBALL) {
            int cx = sx + 4;
            int cy = sy + 4;
            int pulse = (e->animTimer / 3) % 2;

            draw_filled_rect(cx - 4 - pulse, cy - 4 - pulse, 9 + pulse * 2, 9 + pulse * 2, 0xDC262688);
            draw_filled_rect(cx - 3, cy - 3, 7, 7, 0xEA580CFF);
            draw_filled_rect(cx - 2, cy - 2, 5, 5, 0xF97316FF);
            draw_filled_rect(cx - 1, cy - 1, 3, 3, 0xFDE047FF);
            put_pixel_safe(cx, cy, 0xFFFFFFFF);
        }

        // 26. ITEM: SAGRADO ELEMENTO FOGO (FIRE ELEMENT)
        else if (e->type == ENTITY_ITEM_FIRE_ELEMENT) {
            int bob = (int)(sinf((float)e->animTimer * 0.12f) * 3.0f);
            int ey = sy + bob;

            draw_rect_blend(sx - 4, ey - 4, 24, 24, 0xF9731633);
            draw_rect_blend(sx - 2, ey - 2, 20, 20, 0xFDE04744);

            draw_filled_rect(sx + 3, ey + 1, 10, 14, 0xF59E0BFF);
            draw_filled_rect(sx + 4, ey + 2, 8, 12, 0x78350FFF);

            int flame_phase = (e->animTimer / 4) % 3;
            u32 c_flame = (flame_phase == 0) ? 0xDC2626FF : ((flame_phase == 1) ? 0xEA580CFF : 0xF97316FF);
            draw_filled_rect(sx + 5, ey + 4 + flame_phase, 6, 7 - flame_phase, c_flame);
            draw_filled_rect(sx + 6, ey + 6, 4, 4, 0xFDE047FF);
            put_pixel_safe(sx + 7, ey + 5, 0xFFFFFFFF);
            put_pixel_safe(sx + 8, ey + 7, 0xFFFFFFFF);

            int spark_y = ey - 4 - ((e->animTimer / 3) % 8);
            put_pixel_safe(sx + 7 + ((e->animTimer / 5) % 3 - 1), spark_y, 0xFDE047CC);
        }

        // 27. CHEFE: GLEEROK - DRAGÃO DE FOGO DA CAVE OF FLAMES
        else if (e->type == ENTITY_BOSS_GLEEROK) {
            bool flash = (e->invulnerableTimer > 0 && (e->invulnerableTimer / 2) % 2 == 1);
            int neck_sway = (int)(sinf((float)e->animTimer * 0.08f) * 4.0f);

            u32 c_rock_base  = flash ? 0xFFFFFFFF : 0x292524FF;
            u32 c_rock_crust = flash ? 0xFEE2E2FF : 0x44403CFF;
            u32 c_rock_dark  = flash ? 0xE2E8F0FF : 0x1C1917FF;
            u32 c_magma_seam = flash ? 0xFDE047FF : 0xDC2626FF;
            u32 c_magma_glow = flash ? 0xFFFFFFFF : 0xF97316FF;
            u32 c_ruby       = flash ? 0xFFFFFFFF : 0xEF4444FF;
            u32 c_ruby_glow  = flash ? 0xFFFFFFFF : 0xFDE047FF;
            u32 c_horn       = flash ? 0xFDE047FF : 0xD97706FF;

            if (e->action == 3) {
                // ESTADO TOPPLED (VULNERÁVEL): CARAPAÇA INVERTIDA & PESCOÇO COMO PONTE
                // 1. Pescoço estendido no piso formando rampa de acesso
                int ramp_x = sx + 20;
                int ramp_y = sy + 28;
                draw_filled_rect(ramp_x, ramp_y, 16, 12, c_rock_dark);
                draw_filled_rect(ramp_x + 2, ramp_y + 2, 12, 8, c_rock_crust);
                draw_filled_rect(ramp_x + 4, ramp_y + 4, 8, 4, 0x78350FFF); // Caminho de escamas

                // Cabeça adormecida / atordoada no solo
                int hx = ramp_x - 4;
                int hy = ramp_y + 8;
                draw_filled_rect(hx, hy, 14, 10, c_rock_dark);
                draw_filled_rect(hx + 2, hy + 2, 10, 6, c_rock_base);
                put_pixel_safe(hx + 3, hy + 4, 0x1E293BFF); // Olho fechado / em X
                put_pixel_safe(hx + 5, hy + 4, 0x1E293BFF);

                // 2. Carapaça virada ao avesso (expondo o grande Núcleo de Rubi)
                int body_x = sx + 4;
                int body_y = sy + 4;
                draw_filled_rect(body_x, body_y, 48, 26, c_rock_dark);
                draw_filled_rect(body_x + 2, body_y + 2, 44, 22, c_rock_base);
                draw_filled_rect(body_x + 4, body_y + 4, 40, 18, c_rock_crust);

                // Núcleo de Rubi pulsante
                int core_x = body_x + 15;
                int core_y = body_y + 5;
                int pulse = (int)(sinf((float)e->animTimer * 0.2f) * 2.0f);
                draw_rect_blend(core_x - 4, core_y - 4, 26, 22, 0xDC262655);
                draw_filled_rect(core_x - pulse, core_y - pulse, 18 + pulse * 2, 14 + pulse * 2, c_ruby);
                draw_filled_rect(core_x + 3, core_y + 2, 12, 10, c_magma_glow);
                draw_filled_rect(core_x + 6, core_y + 4, 6, 6, c_ruby_glow);
                put_pixel_safe(core_x + 8, core_y + 5, 0xFFFFFFFF); // Brilho de reflexo da joia

                // Garras adormecidas nos lados
                draw_filled_rect(body_x - 3, body_y + 8, 4, 8, c_horn);
                draw_filled_rect(body_x + 47, body_y + 8, 4, 8, c_horn);
            } else if (e->action == 5) {
                // MORTE CLIMÁTICA: EXPLOSÕES E AFUNDAMENTO NA LAVA
                int sink = e->bossDeathTimer / 4;
                int body_x = sx + 4;
                int body_y = sy + 4 + sink;

                draw_filled_rect(body_x, body_y, 48, 28, c_rock_dark);
                draw_filled_rect(body_x + 4, body_y + 2, 40, 24, c_rock_base);

                // Círculos de explosão e clarões de destruição
                int ex_phase = (e->bossDeathTimer / 6) % 4;
                int ex_x = body_x + 6 + (ex_phase * 10);
                int ex_y = body_y + 4 + ((ex_phase % 2) * 10);
                draw_filled_rect(ex_x - 6, ex_y - 6, 12, 12, 0xFDE047CC);
                draw_filled_rect(ex_x - 3, ex_y - 3, 6, 6, 0xFFFFFFFF);
            } else {
                // ESTADO ATIVO / PATRULHA / ATAQUE DE FOGO (Action 0, 1, 4)
                // 1. Carapaça blindada de basalto com fissuras de magma
                int body_x = sx + 4;
                int body_y = sy + 8;
                draw_filled_rect(body_x, body_y, 48, 28, c_rock_dark);
                draw_filled_rect(body_x + 2, body_y + 2, 44, 24, c_rock_base);
                draw_filled_rect(body_x + 4, body_y + 4, 40, 20, c_rock_crust);

                // Fissuras de magma incandescente na rocha
                for (int f = 0; f < 3; f++) {
                    int fx = body_x + 10 + (f * 12);
                    int fy = body_y + 6;
                    draw_filled_rect(fx, fy, 4, 16, c_magma_seam);
                    draw_filled_rect(fx + 1, fy + 2, 2, 12, c_magma_glow);
                    put_pixel_safe(fx + 1, fy + 6, 0xFFFFFFFF);
                }

                // Chifres e espigões rochosos da couraça
                draw_filled_rect(body_x + 2, body_y - 4, 6, 6, c_horn);
                draw_filled_rect(body_x + 40, body_y - 4, 6, 6, c_horn);
                draw_filled_rect(body_x + 21, body_y - 5, 6, 6, c_horn);

                // 2. Pescoço serpentino de dragão ondulando
                int neck_x = body_x + 18 + neck_sway;
                int neck_y = body_y + 16;
                draw_filled_rect(neck_x, neck_y, 12, 14, c_rock_dark);
                draw_filled_rect(neck_x + 2, neck_y, 8, 14, c_rock_base);
                draw_filled_rect(neck_x + 4, neck_y + 2, 4, 10, c_magma_seam);

                // 3. Cabeça de Dragão com mandíbula e olhos
                int head_x = neck_x - 3;
                int head_y = neck_y + 10;
                draw_filled_rect(head_x, head_y, 18, 14, c_rock_dark);
                draw_filled_rect(head_x + 2, head_y + 2, 14, 10, c_rock_base);

                // Grandes Chifres de Dragão
                draw_filled_rect(head_x - 3, head_y + 1, 4, 5, c_horn);
                draw_filled_rect(head_x + 17, head_y + 1, 4, 5, c_horn);

                // Olhos amarelos flamejantes
                draw_filled_rect(head_x + 3, head_y + 4, 3, 3, 0xFDE047FF);
                draw_filled_rect(head_x + 12, head_y + 4, 3, 3, 0xFDE047FF);
                put_pixel_safe(head_x + 4, head_y + 5, 0xDC2626FF); // Pupila rubi
                put_pixel_safe(head_x + 13, head_y + 5, 0xDC2626FF);

                // Boca / Focinho cuspindo fogo ou fumaça
                if (e->bossFireTimer < 35) {
                    draw_filled_rect(head_x + 5, head_y + 9, 8, 5, c_magma_glow);
                    draw_filled_rect(head_x + 6, head_y + 10, 6, 3, 0xFDE047FF);
                    put_pixel_safe(head_x + 8, head_y + 11, 0xFFFFFFFF);
                } else {
                    draw_filled_rect(head_x + 6, head_y + 10, 6, 3, c_rock_dark);
                }
            }

            // HUD DE VIDA DO CHEFE GLEEROK (BARRA DE BOSS GBA CANÔNICA)
            if (e->action != 5 && e->health > 0) {
                int bar_w = 92;
                int bar_h = 7;
                int bar_x = (cam->viewport_w - bar_w) / 2;
                int bar_y = 18;

                draw_filled_rect(bar_x - 1, bar_y - 1, bar_w + 2, bar_h + 2, 0x0B0F19FF);
                draw_filled_rect(bar_x, bar_y, bar_w, bar_h, 0x1C1917FF);

                int pip_w = 7;
                for (int hp = 0; hp < e->maxHealth; hp++) {
                    int px = bar_x + 2 + (hp * 9);
                    u32 pip_col = (hp < e->health) ? 0xEF4444FF : 0x44403CFF;
                    draw_filled_rect(px, bar_y + 1, pip_w, bar_h - 2, pip_col);
                    if (hp < e->health) {
                        draw_filled_rect(px + 1, bar_y + 2, pip_w - 2, 1, 0xFDE047FF);
                    }
                }

                font_draw_text(bar_x + 2, bar_y - 8, "GLEEROK - DRAGAO DE FOGO", 0xF97316FF, true);
            }
        }

        // 28. INIMIGO: ROPE (SERPENTE ÁGIL DO PÂNTANO)
        else if (e->type == ENTITY_ENEMY_ROPE) {
            // Sombra oval no chão
            draw_filled_rect(sx + 1, sy + 10, 10, 3, 0x05100766);

            // Piscar se estiver invulnerável
            if (e->invulnerableTimer > 0 && ((e->invulnerableTimer / 3) % 2 == 0)) {
                continue;
            }

            if (s_enemies_tex && s_enemies_tex->pixels) {
                int frame = (e->animTimer / 6) % 2;
                int src_x = frame * 16;
                bool flip_h = (e->dir == DIR_LEFT);
                texture_draw_ex(s_enemies_tex, src_x, 5 * 16, 16, 16, sx, sy, flip_h);
            } else {
                u32 c_skin_main = 0xD97706FF; // Pele de cobra âmbar/laranja
                u32 c_skin_dark = 0x78350FFF; // Manchas escuras / escamas dorsais
                u32 c_skin_hi   = 0xF59E0BFF; // Destaque dorsal
                u32 c_belly     = 0xFEF08AFF; // Ventre amarelo claro
                u32 c_eye       = 0xDC2626FF; // Olhos vermelhos brilhantes
                u32 c_tongue    = 0xEF4444FF; // Língua bífida

                // Se em Investida rápida (Action 2), olhos incandescentes
                if (e->action == 2) {
                    c_eye = 0xFFFFFFFF;
                }

                int wag = ((e->animTimer / 4) % 2 == 0) ? 1 : -1;

                if (e->dir == DIR_DOWN) {
                    // Cabeça triangular
                    draw_filled_rect(sx + 3, sy + 6, 6, 5, c_skin_main);
                    draw_filled_rect(sx + 4, sy + 5, 4, 1, c_skin_hi);
                    draw_filled_rect(sx + 4, sy + 9, 4, 3, c_skin_main);
                    // Olhos
                    put_pixel_safe(sx + 3, sy + 7, c_eye);
                    put_pixel_safe(sx + 8, sy + 7, c_eye);
                    // Língua bifurcada
                    if ((e->animTimer / 5) % 2 == 0) {
                        put_pixel_safe(sx + 5, sy + 12, c_tongue);
                        put_pixel_safe(sx + 6, sy + 12, c_tongue);
                        put_pixel_safe(sx + 4, sy + 13, c_tongue);
                        put_pixel_safe(sx + 7, sy + 13, c_tongue);
                    }
                    // Corpo ondulante (cauda para cima)
                    draw_filled_rect(sx + 4 + wag, sy + 2, 4, 4, c_skin_main);
                    draw_filled_rect(sx + 5 - wag, sy - 1, 3, 3, c_skin_dark);
                    put_pixel_safe(sx + 5 + wag, sy + 3, c_skin_dark);
                } else if (e->dir == DIR_UP) {
                    // Cabeça voltada para cima
                    draw_filled_rect(sx + 3, sy + 2, 6, 5, c_skin_main);
                    draw_filled_rect(sx + 4, sy + 1, 4, 2, c_skin_main);
                    put_pixel_safe(sx + 3, sy + 3, c_eye);
                    put_pixel_safe(sx + 8, sy + 3, c_eye);
                    if ((e->animTimer / 5) % 2 == 0) {
                        put_pixel_safe(sx + 5, sy - 1, c_tongue);
                        put_pixel_safe(sx + 6, sy - 1, c_tongue);
                    }
                    // Corpo para baixo
                    draw_filled_rect(sx + 4 + wag, sy + 7, 4, 4, c_skin_main);
                    draw_filled_rect(sx + 5 - wag, sy + 11, 3, 3, c_skin_dark);
                } else if (e->dir == DIR_LEFT) {
                    // Cabeça para a esquerda
                    draw_filled_rect(sx + 1, sy + 4, 5, 5, c_skin_main);
                    draw_filled_rect(sx, sy + 5, 2, 3, c_skin_main);
                    put_pixel_safe(sx + 2, sy + 4, c_eye);
                    if ((e->animTimer / 5) % 2 == 0) {
                        put_pixel_safe(sx - 2, sy + 6, c_tongue);
                        put_pixel_safe(sx - 1, sy + 6, c_tongue);
                    }
                    // Corpo ondulante para a direita
                    draw_filled_rect(sx + 6, sy + 4 + wag, 4, 4, c_skin_main);
                    draw_filled_rect(sx + 10, sy + 5 - wag, 4, 3, c_skin_dark);
                    put_pixel_safe(sx + 8, sy + 5 + wag, c_belly);
                } else { // DIR_RIGHT
                    // Cabeça para a direita
                    draw_filled_rect(sx + 6, sy + 4, 5, 5, c_skin_main);
                    draw_filled_rect(sx + 10, sy + 5, 2, 3, c_skin_main);
                    put_pixel_safe(sx + 9, sy + 4, c_eye);
                    if ((e->animTimer / 5) % 2 == 0) {
                        put_pixel_safe(sx + 12, sy + 6, c_tongue);
                        put_pixel_safe(sx + 13, sy + 6, c_tongue);
                    }
                    // Corpo ondulante para a esquerda
                    draw_filled_rect(sx + 2, sy + 4 + wag, 4, 4, c_skin_main);
                    draw_filled_rect(sx - 2, sy + 5 - wag, 4, 3, c_skin_dark);
                    put_pixel_safe(sx + 4, sy + 5 + wag, c_belly);
                }
            }
        }

        // 29. ITEM: ARCO E FLECHAS (PEDESTAL EM CASTOR WILDS)
        else if (e->type == ENTITY_ITEM_BOW) {
            int bob = (int)(sinf((float)e->animTimer * 0.12f) * 2.5f);
            int by = sy + bob;

            // Sombra suave no chão
            draw_filled_rect(sx - 5, sy + 10, 14, 3, 0x05100766);
            draw_filled_rect(sx - 3, sy + 9, 10, 5, 0x05100766);

            // Halo místico cintilante em volta do Arco
            draw_rect_blend(sx - 6, by - 6, 20, 20, 0x38BDF833);
            draw_rect_blend(sx - 4, by - 4, 16, 16, 0xFDE04744);

            // Arco recurvo de madeira nobre e reforços dourados (12x12 px)
            u32 c_wood_dark  = 0x78350FFF;
            u32 c_wood_main  = 0x92400EFF;
            u32 c_wood_light = 0xB45309FF;
            u32 c_gold       = 0xF59E0BFF;
            u32 c_string     = 0xF1F5F9FF;
            u32 c_arrow      = 0xE2E8F0FF;
            u32 c_fletch     = 0xEF4444FF;

            // Curvatura do arco (madeira e ponteiras douradas)
            draw_filled_rect(sx + 2, by - 5, 3, 2, c_gold); // Ponta superior
            draw_filled_rect(sx + 4, by - 3, 2, 3, c_wood_light);
            draw_filled_rect(sx + 5, by, 2, 4, c_wood_main); // Empunhadura
            draw_filled_rect(sx + 4, by + 4, 2, 3, c_wood_dark);
            draw_filled_rect(sx + 2, by + 7, 3, 2, c_gold); // Ponta inferior

            // Corda esticada
            draw_filled_rect(sx + 1, by - 4, 1, 12, c_string);

            // Flecha encaixada pronta para o disparo
            draw_filled_rect(sx - 2, by + 1, 9, 1, c_arrow);
            put_pixel_safe(sx + 7, by, c_arrow); // Ponta da flecha
            put_pixel_safe(sx + 8, by + 1, c_arrow);
            put_pixel_safe(sx + 7, by + 2, c_arrow);
            put_pixel_safe(sx - 3, by, c_fletch); // Penas traseiras
            put_pixel_safe(sx - 3, by + 2, c_fletch);

            // Partícula de brilho cintilante
            int spark_tick = (e->animTimer / 6) % 4;
            if (spark_tick == 0) put_pixel_safe(sx + 8, by - 2, 0xFFFFFFFF);
            else if (spark_tick == 2) put_pixel_safe(sx - 2, by + 4, 0xFFFFFFFF);
        }

        // 30. ITEM: LUVAS DE TOUPEIRA (MOLE MITTS)
        else if (e->type == ENTITY_ITEM_MOLE_MITTS) {
            int bob = (int)(sinf((float)e->animTimer * 0.12f) * 2.5f);
            int by = sy + bob;

            // Sombra suave no chão
            draw_filled_rect(sx - 6, sy + 10, 16, 3, 0x05100766);
            draw_filled_rect(sx - 4, sy + 9, 12, 5, 0x05100766);

            // Halo místico âmbar / dourado
            draw_rect_blend(sx - 7, by - 7, 22, 22, 0xD9770633);
            draw_rect_blend(sx - 5, by - 5, 18, 18, 0xFDE04744);

            // Cores das luvas de toupeira
            u32 c_cuff       = 0xF59E0BFF; // Pulseira dourada
            u32 c_leather_hi = 0x92400EFF; // Couro marrom claro
            u32 c_leather_mid= 0x78350FFF; // Couro base
            u32 c_leather_dk = 0x451A03FF; // Sombra couro
            u32 c_claw_base  = 0x64748BFF; // Base metálica
            u32 c_claw_mid   = 0xCBD5E1FF; // Aço prateado
            u32 c_claw_tip   = 0xFFFFFFFF; // Brilho na ponta afiada

            // Punho / Bracelete dourado
            draw_filled_rect(sx - 4, by + 5, 10, 2, c_cuff);

            // Corpo da luva em couro resistente
            draw_filled_rect(sx - 5, by - 1, 12, 6, c_leather_mid);
            draw_filled_rect(sx - 4, by - 2, 10, 1, c_leather_hi);
            draw_filled_rect(sx - 5, by + 3, 12, 2, c_leather_dk);

            // 3 Garras afiadas escavadoras
            // Garra esquerda
            draw_filled_rect(sx - 4, by - 4, 2, 3, c_claw_mid);
            put_pixel_safe(sx - 4, by - 5, c_claw_tip);
            put_pixel_safe(sx - 3, by - 2, c_claw_base);

            // Garra central (mais longa)
            draw_filled_rect(sx - 1, by - 6, 2, 5, c_claw_mid);
            put_pixel_safe(sx - 1, by - 7, c_claw_tip);
            put_pixel_safe(sx, by - 2, c_claw_base);

            // Garra direita
            draw_filled_rect(sx + 2, by - 4, 2, 3, c_claw_mid);
            put_pixel_safe(sx + 3, by - 5, c_claw_tip);
            put_pixel_safe(sx + 2, by - 2, c_claw_base);

            // Partícula de brilho cintilante
            int spark_tick = (e->animTimer / 6) % 4;
            if (spark_tick == 0) put_pixel_safe(sx + 6, by - 3, 0xFFFFFFFF);
            else if (spark_tick == 2) put_pixel_safe(sx - 5, by + 1, 0xFFFFFFFF);
        }

        // 31. ROBÔ / SENTINELA ANCESTRAL ARMOS
        else if (e->type == ENTITY_ARMOS) {
            int render_x = sx;
            if (e->action == 1) {
                // Efeito de tremer/sacudir ao despertar
                render_x += (e->animTimer % 2 == 0) ? -1 : 1;
            }

            // Sombra oval no solo
            draw_filled_rect(render_x - 8, sy + 10, 16, 4, 0x05100766);
            draw_filled_rect(render_x - 6, sy + 9, 12, 6, 0x05100766);

            // Cores do Armos
            u32 c_stone_base = 0x475569FF;
            u32 c_stone_dark = 0x1E293BFF;
            u32 c_stone_lite = 0x94A3B8FF;
            u32 c_bronze_horn= 0xD97706FF;
            u32 c_bronze_lite= 0xF59E0BFF;
            u32 c_portal     = 0x090D16FF;

            // 1. Chifres / Crista de bronze no capacete
            draw_filled_rect(render_x - 8, sy - 11, 2, 4, c_bronze_horn);
            put_pixel_safe(render_x - 7, sy - 12, c_bronze_lite);
            draw_filled_rect(render_x + 6, sy - 11, 2, 4, c_bronze_horn);
            put_pixel_safe(render_x + 6, sy - 12, c_bronze_lite);
            draw_filled_rect(render_x - 6, sy - 9, 12, 2, c_bronze_horn);

            // 2. Capacete e Cabeça de Pedra
            draw_filled_rect(render_x - 6, sy - 7, 12, 6, c_stone_base);
            draw_filled_rect(render_x - 6, sy - 7, 12, 1, c_stone_lite);
            draw_filled_rect(render_x - 6, sy - 2, 12, 1, c_stone_dark);

            // 3. Olho Ciclópico Sensor Central
            if (e->action == 0) {
                // Dormente: fenda escura horizontal adormecida
                draw_filled_rect(render_x - 3, sy - 5, 6, 2, 0x0F172AFF);
                put_pixel_safe(render_x, sy - 5, 0x334155FF);
            } else if (e->action == 1) {
                // Despertando: pulsando em âmbar/rubi
                u32 eye_pulse = ((e->animTimer / 4) % 2 == 0) ? 0xF59E0BFF : 0xEF4444FF;
                draw_filled_rect(render_x - 4, sy - 6, 8, 4, eye_pulse);
                draw_filled_rect(render_x - 2, sy - 5, 4, 2, 0xFFFFFFFF);
            } else {
                // Ativo / Desperto: Olho radiante vermelho rubi com brilho celestial
                draw_rect_blend(render_x - 6, sy - 7, 12, 6, 0xEF444444);
                draw_filled_rect(render_x - 3, sy - 5, 6, 3, 0xEF4444FF);
                draw_filled_rect(render_x - 1, sy - 5, 2, 2, 0xFFFFFFFF);
            }

            // 4. Peitoral e Torso de Pedra Esculpida
            draw_filled_rect(render_x - 7, sy, 14, 9, c_stone_base);
            draw_filled_rect(render_x - 7, sy, 1, 9, c_stone_lite);
            draw_filled_rect(render_x + 6, sy, 1, 9, c_stone_dark);
            put_pixel_safe(render_x - 2, sy + 3, c_stone_dark);
            put_pixel_safe(render_x + 1, sy + 3, c_stone_dark);
            draw_filled_rect(render_x - 1, sy + 4, 2, 3, c_stone_lite);

            // 5. Escudo de Pedra no Braço Esquerdo
            draw_filled_rect(render_x - 9, sy - 1, 3, 8, c_stone_lite);
            draw_filled_rect(render_x - 10, sy + 1, 1, 4, c_bronze_lite);
            draw_filled_rect(render_x - 9, sy + 6, 3, 1, c_stone_dark);

            // 6. Base / Pernas e Entrada Minish
            draw_filled_rect(render_x - 6, sy + 9, 12, 3, c_stone_dark);
            draw_filled_rect(render_x - 2, sy + 8, 4, 4, c_portal);
            draw_filled_rect(render_x - 1, sy + 7, 2, 1, c_portal);
        }

        // 32. INTERRUPTOR DE CIRCUITO INTERNO DO ARMOS (ARMOS CIRCUIT SWITCH)
        else if (e->type == ENTITY_ARMOS_SWITCH) {
            draw_filled_rect(sx - 7, sy + 6, 14, 4, 0x05100766);
            draw_filled_rect(sx - 6, sy - 2, 12, 8, 0x475569FF);
            draw_filled_rect(sx - 5, sy - 3, 10, 1, 0x94A3B8FF);
            draw_filled_rect(sx - 6, sy + 5, 12, 1, 0x1E293BFF);

            if (e->action == 0) {
                draw_filled_rect(sx - 1, sy - 8, 2, 6, 0xCBD5E1FF);
                draw_filled_rect(sx - 2, sy - 10, 4, 3, 0xEF4444FF);
                put_pixel_safe(sx - 1, sy - 10, 0xFFFFFFFF);
            } else {
                draw_rect_blend(sx - 8, sy - 8, 16, 16, 0x38BDF844);
                draw_filled_rect(sx, sy - 5, 5, 2, 0x38BDF8FF);
                draw_filled_rect(sx + 4, sy - 4, 3, 3, 0x22C55EFF);
                int spk = (e->animTimer / 4) % 3;
                if (spk == 0) put_pixel_safe(sx - 3, sy - 7, 0xFFFFFFFF);
                else if (spk == 1) put_pixel_safe(sx + 5, sy - 8, 0xBAE6FDFF);
            }
        }

        // 33. ANCIÃO LIBRARI (BIBLIOTECA REAL DE HYRULE)
        else if (e->type == ENTITY_NPC_LIBRARI) {
            // Sábio Minish sentado no topo da estante com óculos dourados e livro
            draw_filled_rect(sx - 5, sy - 8, 10, 8, 0x064E3BFF); // Manto verde sábio
            draw_filled_rect(sx - 4, sy - 11, 8, 5, 0xFDE8CDFF); // Rosto e cabeça
            draw_filled_rect(sx - 5, sy - 7, 10, 5, 0xFFFFFFFF); // Barba branca longa
            // Óculos redondos de leitura dourados
            draw_filled_rect(sx - 4, sy - 10, 3, 2, 0xF59E0BFF);
            draw_filled_rect(sx + 1, sy - 10, 3, 2, 0xF59E0BFF);
            put_pixel_safe(sx - 3, sy - 10, 0x38BDF8FF); // Lentes brilhantes
            put_pixel_safe(sx + 2, sy - 10, 0x38BDF8FF);
            // Livro aberto em seu colo
            draw_filled_rect(sx - 6, sy - 2, 12, 4, 0x991B1BFF);
            draw_filled_rect(sx - 5, sy - 1, 10, 2, 0xFEF08AFF);
        }

        // 34. PREFEITO HAGEN (CABANA DE LAKE HYLIA)
        else if (e->type == ENTITY_NPC_MAYOR_HAGEN) {
            draw_filled_rect(sx - 6, sy + 10, 12, 3, 0x05100766); // Sombra

            if (s_npcs_tex && s_npcs_tex->pixels) {
                int src_x = 0;
                bool flip_h = false;
                if (e->dir == DIR_RIGHT) {
                    src_x = 1 * 16;
                } else if (e->dir == DIR_LEFT) {
                    src_x = 1 * 16;
                    flip_h = true;
                } else {
                    int anim = (e->animTimer / 30) % 3;
                    src_x = (anim == 0) ? (0 * 16) : ((anim == 1) ? (2 * 16) : (3 * 16));
                }
                texture_draw_ex(s_npcs_tex, src_x, 3 * 16, 16, 16, sx - 8, sy - 6, flip_h);
            } else {
                // Chapéu azul com pluma vermelha
                draw_filled_rect(sx - 6, sy - 10, 12, 4, 0x1E40AFFF);
                draw_filled_rect(sx - 3, sy - 13, 3, 4, 0xDC2626FF); // Pluma
                // Rosto redondo e bigode castanho
                draw_filled_rect(sx - 5, sy - 6, 10, 6, 0xFDE8CDFF);
                draw_filled_rect(sx - 4, sy - 2, 8, 2, 0x78350FFF); // Bigode
                // Colete dourado e casaco azul
                draw_filled_rect(sx - 6, sy, 12, 8, 0x1E3A8AFF);
                draw_filled_rect(sx - 3, sy + 1, 6, 6, 0xFACC15FF); // Colete
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

Entity* entity_find_nearby_swiftblade(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_SWIFTBLADE) continue;

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

Entity* entity_find_nearby_shopkeeper(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_SHOPKEEPER) continue;

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

Entity* entity_find_nearby_town_citizen(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_TOWN_CITIZEN) continue;

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

Entity* entity_find_nearby_town_guard(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_TOWN_GUARD) continue;

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

bool entity_buy_shop_item(int item_idx, int* link_rupees, int* link_hearts, int* link_max_hearts) {
    if (!link_rupees) return false;

    if (item_idx == 0) {
        // Poção Vermelha (30 Rupees): Restaura todos os corações de vida
        int cost = 30;
        if (*link_rupees >= cost) {
            *link_rupees -= cost;
            if (link_hearts && link_max_hearts) {
                *link_hearts = *link_max_hearts;
            }
            hal_audio_play_sound(SOUND_SHOP_BUY, 1.0f, 1.0f);
            printf("[LOJA STOCKWELL] Comprou Pocao Vermelha por %d Rupees! Vida restaurada com exito!\n", cost);
            return true;
        }
    } else if (item_idx == 1) {
        // Pedaço de Coração (80 Rupees): Concede +1 Coração Máximo permanente e cura total
        int cost = 80;
        if (*link_rupees >= cost) {
            *link_rupees -= cost;
            if (link_max_hearts && link_hearts) {
                (*link_max_hearts)++;
                *link_hearts = *link_max_hearts;
            }
            hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.0f);
            printf("[LOJA STOCKWELL] Comprou Recipiente de Coracao por %d Rupees! Vida Maxima aumentada para %d!\n", cost, link_max_hearts ? *link_max_hearts : 0);
            return true;
        }
    } else if (item_idx == 2) {
        // Bolsa de Bombas / Provisões (50 Rupees): Adiciona +10 Bombas ao estoque
        int cost = 50;
        if (*link_rupees >= cost) {
            *link_rupees -= cost;
            subweapon_add_bombs(10);
            hal_audio_play_sound(SOUND_SHOP_BUY, 1.0f, 1.0f);
            printf("[LOJA STOCKWELL] Comprou Bolsa de Bombas por %d Rupees! +10 Bombas adicionadas (Total: %d)!\n",
                   cost, subweapon_get_bomb_count());
            return true;
        }
    }

    hal_audio_play_sound(SOUND_SWORD_HIT, 0.7f, 0.8f);
    printf("[LOJA STOCKWELL] Rupees insuficientes para a compra (Item %d)!\n", item_idx);
    return false;
}

Entity* entity_find_kinstone_npc(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active) continue;
        if (e->type != ENTITY_NPC_FOREST_MINISH && e->type != ENTITY_NPC_TOWN_CITIZEN &&
            e->type != ENTITY_NPC_VILLAGE_MINISH && e->type != ENTITY_NPC_MALON) continue;
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

Entity* entity_find_nearby_minish_stump(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_MINISH_STUMP) continue;

        float dx = (e->x + 8.0f) - (world_x + 8.0f);
        float dy = (e->y + 8.0f) - (world_y + 8.0f);
        float dist_sq = dx * dx + dy * dy;

        if (dist_sq <= best_dist_sq) {
            best_dist_sq = dist_sq;
            best = e;
        }
    }
    return best;
}

Entity* entity_find_nearby_gentari(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_GENTARI) continue;

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

Entity* entity_find_nearby_festari(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_FESTARI) continue;

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

Entity* entity_find_nearby_village_minish(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_VILLAGE_MINISH) continue;

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

Entity* entity_find_nearby_malon(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_MALON) continue;

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

Entity* entity_find_nearby_business_scrub(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_BUSINESS_SCRUB) continue;

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

Entity* entity_find_nearby_melari(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_MELARI) continue;

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

Entity* entity_find_nearby_mountain_minish(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_MOUNTAIN_MINISH) continue;

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

bool entity_buy_grip_ring(int* link_rupees, bool* out_has_grip_ring) {
    if (!link_rupees) return false;
    int cost = 40;
    if (*link_rupees >= cost) {
        *link_rupees -= cost;
        if (out_has_grip_ring) *out_has_grip_ring = true;
        hal_audio_play_sound(SOUND_SHOP_BUY, 1.0f, 1.0f);
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.25f);
        printf("[BUSINESS SCRUB] Comprou Grip Ring por %d Rupees! Escalada destravada!\n", cost);
        return true;
    }

    hal_audio_play_sound(SOUND_SWORD_HIT, 0.7f, 0.8f);
    printf("[BUSINESS SCRUB] Rupees insuficientes para comprar o Grip Ring (Custa %d Rupees)!\n", cost);
    return false;
}

Entity* entity_find_nearby_armos(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_ARMOS) continue;

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

Entity* entity_find_nearby_librari(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_LIBRARI) continue;

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

Entity* entity_find_nearby_mayor_hagen(float world_x, float world_y, float max_dist) {
    float best_dist_sq = max_dist * max_dist;
    Entity* best = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity* e = &s_entities[i];
        if (!e->is_active || e->type != ENTITY_NPC_MAYOR_HAGEN) continue;

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
