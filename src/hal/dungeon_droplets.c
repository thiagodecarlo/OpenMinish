/*
 * ============================================================================
 * src/hal/dungeon_droplets.c - Masmorra 4: Temple of Droplets (Templo das Gotas)
 * ============================================================================
 */

#include "hal/dungeon_droplets.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/subweapon.h"
#include "hal/lantern.h"
#include "hal/inventory.h"
#include "hal/entity.h"
#include "hal/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

// Paleta de cores do Temple of Droplets
#define C_DROP_WALL_TOP     0x7DD3FCFF
#define C_DROP_WALL_FACE    0x1E3A8AFF
#define C_DROP_WALL_DARK    0x0F172AFF
#define C_DROP_ICE_BASE     0xBAE6FDFF
#define C_DROP_ICE_SHINE    0xE0F2FEFF
#define C_DROP_ICE_DEEP     0x38BDF8FF
#define C_DROP_WATER        0x0284C7FF
#define C_DROP_SUNBEAM      0xFDE04766
#define C_DROP_GOLD         0xF59E0BFF

static DungeonDropletsState s_droplets = { 0 };
static int s_drop_anim_timer = 0;
static const Texture* s_droplets_bosses_tex = NULL;

void dungeon_droplets_set_bosses_texture(const Texture* tex) {
    s_droplets_bosses_tex = tex;
}

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

static Tilemap* create_empty_room(void) {
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;
    m->width = DROPLETS_ROOM_W;
    m->height = DROPLETS_ROOM_H;
    int total = m->width * m->height;
    m->ground_layer = (u8*)malloc(total);
    m->overlay_layer = (u8*)malloc(total);
    m->collision_map = (u8*)malloc(total);
    m->is_authentic = false;
    m->authentic_tex = NULL;

    memset(m->ground_layer, TILE_MELARI_STONE_FLOOR, total);
    memset(m->overlay_layer, 0xFF, total);
    memset(m->collision_map, 0, total);

    // Paredes de contorno
    for (int y = 0; y < DROPLETS_ROOM_H; y++) {
        for (int x = 0; x < DROPLETS_ROOM_W; x++) {
            if (x == 0 || x == DROPLETS_ROOM_W - 1 || y == 0 || y == DROPLETS_ROOM_H - 1) {
                m->overlay_layer[y * m->width + x] = TILE_STONE_WALL;
                m->collision_map[y * m->width + x] = 1;
            }
        }
    }
    return m;
}

void dungeon_droplets_init(void) {
    memset(&s_droplets, 0, sizeof(s_droplets));
    s_droplets.is_active = false;
    s_droplets.current_room = ROOM_DROPLETS_ENTRANCE;
    s_droplets.small_keys = 0;
    s_droplets.has_boss_key = false;

    // Constrói as 6 câmaras do Temple of Droplets
    for (int r = 0; r < ROOM_DROPLETS_COUNT; r++) {
        s_droplets.rooms[r] = create_empty_room();
    }

    // --- SALA 0: Vestíbulo de Entrada & Canal Glacial ---
    Tilemap* r0 = s_droplets.rooms[ROOM_DROPLETS_ENTRANCE];
    // Abertura da porta sul (saída para Lake Hylia) e porta norte (para Sala 1)
    r0->overlay_layer[9 * 16 + 7] = 0xFF; r0->collision_map[9 * 16 + 7] = 0;
    r0->overlay_layer[9 * 16 + 8] = 0xFF; r0->collision_map[9 * 16 + 8] = 0;
    r0->overlay_layer[0 * 16 + 7] = 0xFF; r0->collision_map[0 * 16 + 7] = 0;
    r0->overlay_layer[0 * 16 + 8] = 0xFF; r0->collision_map[0 * 16 + 8] = 0;
    // Canal de gelo translúcido escorregadio no centro
    for (int y = 2; y <= 7; y++) {
        for (int x = 4; x <= 11; x++) {
            r0->ground_layer[y * 16 + x] = TILE_ICE_BLOCK; // Pista de gelo lisa
        }
    }
    // Tochas apagadas na entrada
    r0->overlay_layer[7 * 16 + 3] = TILE_TORCH_UNLIT;
    r0->overlay_layer[7 * 16 + 12] = TILE_TORCH_UNLIT;

    // --- SALA 1: Câmara do Facho Solar & Prisma Óptico ---
    Tilemap* r1 = s_droplets.rooms[ROOM_DROPLETS_SUNBEAM];
    r1->overlay_layer[9 * 16 + 7] = 0xFF; r1->collision_map[9 * 16 + 7] = 0; // Entrada Sul (da Sala 0)
    r1->overlay_layer[9 * 16 + 8] = 0xFF; r1->collision_map[9 * 16 + 8] = 0;
    r1->overlay_layer[4 * 16 + 15] = 0xFF; r1->collision_map[4 * 16 + 15] = 0; // Porta Leste (para Sala 2)
    r1->overlay_layer[5 * 16 + 15] = 0xFF; r1->collision_map[5 * 16 + 15] = 0;
    // Bloco maciço de gelo encasulando a Chave Pequena #1
    r1->overlay_layer[4 * 16 + 11] = TILE_ICE_BLOCK;
    r1->collision_map[4 * 16 + 11] = 1;

    // --- SALA 2: Caverna Escura & 4 Piras de Pedra ---
    Tilemap* r2 = s_droplets.rooms[ROOM_DROPLETS_DARK_CAVERN];
    r2->overlay_layer[4 * 16 + 0] = 0xFF; r2->collision_map[4 * 16 + 0] = 0; // Porta Oeste (da Sala 1)
    r2->overlay_layer[5 * 16 + 0] = 0xFF; r2->collision_map[5 * 16 + 0] = 0;
    r2->overlay_layer[0 * 16 + 7] = TILE_STONE_WALL; r2->collision_map[0 * 16 + 7] = 1; // Grade trancada
    r2->overlay_layer[0 * 16 + 8] = TILE_STONE_WALL; r2->collision_map[0 * 16 + 8] = 1;
    // 4 Tochas nos cantos para acender com a Flame Lantern
    r2->overlay_layer[2 * 16 + 3]  = TILE_TORCH_UNLIT;
    r2->overlay_layer[2 * 16 + 12] = TILE_TORCH_UNLIT;
    r2->overlay_layer[7 * 16 + 3]  = TILE_TORCH_UNLIT;
    r2->overlay_layer[7 * 16 + 12] = TILE_TORCH_UNLIT;

    // --- SALA 3: Santuário do Elemento da Água ---
    Tilemap* r3 = s_droplets.rooms[ROOM_DROPLETS_SANCTUARY];
    r3->overlay_layer[9 * 16 + 7] = 0xFF; r3->collision_map[9 * 16 + 7] = 0; // Entrada Sul (da Sala 2)
    r3->overlay_layer[9 * 16 + 8] = 0xFF; r3->collision_map[9 * 16 + 8] = 0;
    r3->overlay_layer[0 * 16 + 7] = 0xFF; r3->collision_map[0 * 16 + 7] = 0; // Porta Norte (para Sala 4)
    r3->overlay_layer[0 * 16 + 8] = 0xFF; r3->collision_map[0 * 16 + 8] = 0;
    // Pedestal de gelo central com o Elemento da Água
    r3->overlay_layer[4 * 16 + 7] = TILE_ICE_BLOCK;
    r3->overlay_layer[4 * 16 + 8] = TILE_ICE_BLOCK;
    r3->collision_map[4 * 16 + 7] = 1;
    r3->collision_map[4 * 16 + 8] = 1;

    // --- SALA 4: Antecâmara do Grande Portão Glacial ---
    Tilemap* r4 = s_droplets.rooms[ROOM_DROPLETS_BOSS_DOOR];
    r4->overlay_layer[9 * 16 + 7] = 0xFF; r4->collision_map[9 * 16 + 7] = 0; // Entrada Sul (da Sala 3)
    r4->overlay_layer[9 * 16 + 8] = 0xFF; r4->collision_map[9 * 16 + 8] = 0;
    // Grande Portão do Chefe ao Norte (requer Big Key)
    r4->overlay_layer[0 * 16 + 7] = TILE_FORTRESS_BOSS_DOOR;
    r4->overlay_layer[0 * 16 + 8] = TILE_FORTRESS_BOSS_DOOR;
    r4->collision_map[0 * 16 + 7] = 1;
    r4->collision_map[0 * 16 + 8] = 1;

    // --- SALA 5: Arena Glacial do Chefe BIG OCTOROK ---
    Tilemap* r5 = s_droplets.rooms[ROOM_DROPLETS_BOSS_ARENA];
    r5->overlay_layer[9 * 16 + 7] = 0xFF; r5->collision_map[9 * 16 + 7] = 0;
    r5->overlay_layer[9 * 16 + 8] = 0xFF; r5->collision_map[9 * 16 + 8] = 0;
    // Piso da arena inteira em gelo escorregadio
    for (int y = 1; y < DROPLETS_ROOM_H - 1; y++) {
        for (int x = 1; x < DROPLETS_ROOM_W - 1; x++) {
            r5->ground_layer[y * 16 + x] = TILE_ICE_BLOCK;
        }
    }

    // Inicialização do Chefe Big Octorok
    BigOctorokBoss* b = &s_droplets.boss;
    b->x = 128.0f;
    b->y = 60.0f;
    b->vx = 0.0f;
    b->vy = 0.0f;
    b->health = 12;
    b->phase = OCTO_PHASE_FROZEN;
    b->phase_timer = 0;
    b->spit_timer = 80;
    b->burn_timer = 0;
    b->tail_x = b->x;
    b->tail_y = b->y - 20.0f;
    b->tail_frozen = true;
    b->dir = DIR_DOWN;
    b->anim_frame = 0.0f;
    b->is_active = true;

    for (int i = 0; i < MAX_OCTO_ROCKS; i++) {
        b->rocks[i].is_active = false;
    }

    printf("[DUNGEON 4] Temple of Droplets (Templo das Gotas) inicializado com 6 camaras congeladas e Chefe Big Octorok!\n");
}

void dungeon_droplets_enter(float* player_x, float* player_y, Direction* player_dir) {
    s_droplets.is_active = true;
    s_droplets.current_room = ROOM_DROPLETS_ENTRANCE;
    if (player_x) *player_x = 120.0f;
    if (player_y) *player_y = 120.0f;
    if (player_dir) *player_dir = DIR_UP;

    entity_clear_all();
    hal_audio_play_bgm(BGM_TEMPLE_OF_DROPLETS);
    printf("[DUNGEON 4] Link adentrou o Temple of Droplets nas profundezas congeladas de Lake Hylia!\n");
}

void dungeon_droplets_exit(float* player_x, float* player_y, Direction* player_dir) {
    s_droplets.is_active = false;
    if (player_x) *player_x = 450.0f; // Em frente ao portal glacial em Lake Hylia
    if (player_y) *player_y = 52.0f;
    if (player_dir) *player_dir = DIR_DOWN;
    hal_audio_play_bgm(BGM_HYRULE_OVERWORLD);
    printf("[DUNGEON 4] Link saiu do Temple of Droplets e retornou ao Lago Hylia!\n");
}

bool dungeon_droplets_is_active(void) {
    return s_droplets.is_active;
}

DungeonDropletsRoomId dungeon_droplets_get_current_room(void) {
    return s_droplets.current_room;
}

Tilemap* dungeon_droplets_get_current_map(void) {
    if (!s_droplets.is_active) return NULL;
    return s_droplets.rooms[s_droplets.current_room];
}

bool dungeon_droplets_is_ice_tile(float world_x, float world_y) {
    if (!s_droplets.is_active) return false;
    Tilemap* cur = s_droplets.rooms[s_droplets.current_room];
    if (!cur) return false;
    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= cur->width || ty < 0 || ty >= cur->height) return false;
    int idx = ty * cur->width + tx;
    if (cur->ground_layer && cur->ground_layer[idx] == TILE_ICE_BLOCK) return true;
    return false;
}

void dungeon_droplets_apply_ice_physics(float* player_x, float* player_y, float speed, Direction dir, bool is_moving) {
    if (!player_x || !player_y) return;

    if (is_moving) {
        float ax = 0.0f, ay = 0.0f;
        if (dir == DIR_UP)    ay = -speed;
        if (dir == DIR_DOWN)  ay = speed;
        if (dir == DIR_LEFT)  ax = -speed;
        if (dir == DIR_RIGHT) ax = speed;

        // Aceleração suave no gelo
        s_droplets.ice_slide_vx = s_droplets.ice_slide_vx * 0.85f + ax * 0.15f;
        s_droplets.ice_slide_vy = s_droplets.ice_slide_vy * 0.85f + ay * 0.15f;
    } else {
        // Deslizamento com inércia contínua (atrito reduzido)
        s_droplets.ice_slide_vx *= 0.94f;
        s_droplets.ice_slide_vy *= 0.94f;
        if (fabsf(s_droplets.ice_slide_vx) < 0.08f) s_droplets.ice_slide_vx = 0.0f;
        if (fabsf(s_droplets.ice_slide_vy) < 0.08f) s_droplets.ice_slide_vy = 0.0f;

        *player_x += s_droplets.ice_slide_vx;
        *player_y += s_droplets.ice_slide_vy;
    }
}

void dungeon_droplets_check_boss_sword_hit(float hit_x, float hit_y, float hit_w, float hit_h, int damage, Direction dir) {
    if (!s_droplets.is_active || s_droplets.current_room != ROOM_DROPLETS_BOSS_ARENA) return;
    BigOctorokBoss* b = &s_droplets.boss;
    if (!b->is_active || b->phase == OCTO_PHASE_DEFEATED) return;

    // 1. Checa rebatida das pedras pontiagudas cuspidas
    for (int i = 0; i < MAX_OCTO_ROCKS; i++) {
        OctoRock* r = &b->rocks[i];
        if (r->is_active && !r->reflected) {
            if (hit_x < r->x + 8.0f && hit_x + hit_w > r->x - 8.0f &&
                hit_y < r->y + 8.0f && hit_y + hit_h > r->y - 8.0f) {
                r->reflected = true;
                r->vx = -r->vx * 1.5f;
                r->vy = -r->vy * 1.5f;
                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.4f);
                printf("[BOSS] Link REBATEU a pedra com a espada de volta para o Big Octorok!\n");
            }
        }
    }

    // 2. Se o Big Octorok estiver em chamas correndo em pânico, acertos de espada causam dano corporal!
    if (b->phase == OCTO_PHASE_BURNING) {
        if (hit_x < b->x + 20.0f && hit_x + hit_w > b->x - 20.0f &&
            hit_y < b->y + 20.0f && hit_y + hit_h > b->y - 20.0f) {
            b->health -= damage;
            hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.1f);
            printf("[BOSS] Golpe na couraça do Big Octorok! HP restante: %d/12\n", b->health);

            if (b->health <= 0) {
                b->phase = OCTO_PHASE_DEFEATED;
                b->phase_timer = 90;
                s_droplets.boss_defeated = true;
                hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                printf("[BOSS] GRANDE OCTOROK GLACIAL DERROTADO! Vapor e cristais de gelo se dissipam!\n");
            }
        }
    }
}

void dungeon_droplets_check_boss_lantern_hit(float flame_x, float flame_y, float radius) {
    if (!s_droplets.is_active || s_droplets.current_room != ROOM_DROPLETS_BOSS_ARENA) return;
    BigOctorokBoss* b = &s_droplets.boss;
    if (!b->is_active || b->phase != OCTO_PHASE_STUNNED) return;

    // Checa calor na cauda congelada exposta
    float dist = sqrtf((flame_x - b->tail_x)*(flame_x - b->tail_x) + (flame_y - b->tail_y)*(flame_y - b->tail_y));
    if (dist <= radius + 14.0f) {
        b->tail_frozen = false;
        b->phase = OCTO_PHASE_BURNING;
        b->burn_timer = 180; // 3 segundos de corrida em chamas
        hal_audio_play_sound(SOUND_FIRE, 1.0f, 1.1f);
        hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.3f);
        printf("[BOSS] A CAUDA DO BIG OCTOROK PEGOU FOGO! Monstro em panico pela arena!\n");
    }
}

void dungeon_droplets_update(float* player_x, float* player_y, Direction player_dir, bool is_moving,
                             int* player_hearts, int max_hearts, int* player_rupees, bool* player_has_water_element) {
    if (!s_droplets.is_active || !player_x || !player_y) return;
    s_drop_anim_timer++;

    Tilemap* cur_map = s_droplets.rooms[s_droplets.current_room];

    // 1. Checagem de Transição entre Câmaras da Masmorra
    // --- Sala 0: Saída Sul para Lake Hylia ou Norte para Sala 1 ---
    if (s_droplets.current_room == ROOM_DROPLETS_ENTRANCE) {
        if (*player_y >= (DROPLETS_ROOM_H * TILE_SIZE) - 20.0f && player_dir == DIR_DOWN) {
            dungeon_droplets_exit(player_x, player_y, &player_dir);
            return;
        } else if (*player_y <= 16.0f && player_dir == DIR_UP) {
            s_droplets.current_room = ROOM_DROPLETS_SUNBEAM;
            *player_y = (DROPLETS_ROOM_H * TILE_SIZE) - 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.7f, 1.2f);
        }
    }
    // --- Sala 1: Sul para Sala 0 ou Leste para Sala 2 ---
    else if (s_droplets.current_room == ROOM_DROPLETS_SUNBEAM) {
        if (*player_y >= (DROPLETS_ROOM_H * TILE_SIZE) - 20.0f && player_dir == DIR_DOWN) {
            s_droplets.current_room = ROOM_DROPLETS_ENTRANCE;
            *player_y = 24.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.7f, 1.2f);
        } else if (*player_x >= (DROPLETS_ROOM_W * TILE_SIZE) - 20.0f && player_dir == DIR_RIGHT) {
            s_droplets.current_room = ROOM_DROPLETS_DARK_CAVERN;
            *player_x = 24.0f;
            lantern_set_dark_room(true); // Ativa o breu e iluminação da lanterna
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.7f, 1.2f);
        }

        // Interação com o Prisma Solar: alavanca no centro (x=8, y=4)
        if (hal_input_is_pressed(KEY_A) && fabsf(*player_x - 128.0f) < 24.0f && fabsf(*player_y - 72.0f) < 24.0f) {
            s_droplets.sunbeam_redirected = !s_droplets.sunbeam_redirected;
            hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.0f);
            if (s_droplets.sunbeam_redirected && !s_droplets.sunbeam_ice_melted) {
                s_droplets.sunbeam_ice_melted = true;
                cur_map->overlay_layer[4 * 16 + 11] = 0xFF; // Gelo derretido
                cur_map->collision_map[4 * 16 + 11] = 0;
                s_droplets.small_keys++;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                printf("[DROPLETS] Prisma solar redirecionado! O calor do sol derreteu o gelo, revelando a Chave Pequena!\n");
            }
        }
    }
    // --- Sala 2: Caverna Escura e Tochas ---
    else if (s_droplets.current_room == ROOM_DROPLETS_DARK_CAVERN) {
        if (*player_x <= 16.0f && player_dir == DIR_LEFT) {
            s_droplets.current_room = ROOM_DROPLETS_SUNBEAM;
            *player_x = (DROPLETS_ROOM_W * TILE_SIZE) - 28.0f;
            lantern_set_dark_room(false);
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.7f, 1.2f);
        } else if (s_droplets.dark_room_gate_open && *player_y <= 16.0f && player_dir == DIR_UP) {
            s_droplets.current_room = ROOM_DROPLETS_SANCTUARY;
            *player_y = (DROPLETS_ROOM_H * TILE_SIZE) - 28.0f;
            lantern_set_dark_room(false);
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.7f, 1.2f);
        }

        // Checa se todas as 4 tochas foram acesas com a Flame Lantern
        int lit_count = 0;
        int torch_indices[4] = { 2 * 16 + 3, 2 * 16 + 12, 7 * 16 + 3, 7 * 16 + 12 };
        for (int i = 0; i < 4; i++) {
            if (cur_map->overlay_layer[torch_indices[i]] == TILE_TORCH_LIT) {
                lit_count++;
            }
        }
        if (lit_count >= 4 && !s_droplets.dark_room_gate_open) {
            s_droplets.dark_room_gate_open = true;
            s_droplets.has_boss_key = true;
            cur_map->overlay_layer[0 * 16 + 7] = 0xFF; cur_map->collision_map[0 * 16 + 7] = 0;
            cur_map->overlay_layer[0 * 16 + 8] = 0xFF; cur_map->collision_map[0 * 16 + 8] = 0;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
            printf("[DROPLETS] Todas as 4 tochas acesas! Portao aberto e Chave do Mestre (Boss Key) conquistada!\n");
        }
    }
    // --- Sala 3: Santuário do Elemento da Água ---
    else if (s_droplets.current_room == ROOM_DROPLETS_SANCTUARY) {
        if (*player_y >= (DROPLETS_ROOM_H * TILE_SIZE) - 20.0f && player_dir == DIR_DOWN) {
            s_droplets.current_room = ROOM_DROPLETS_DARK_CAVERN;
            *player_y = 24.0f;
            lantern_set_dark_room(true);
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.7f, 1.2f);
        } else if (*player_y <= 16.0f && player_dir == DIR_UP) {
            s_droplets.current_room = ROOM_DROPLETS_BOSS_DOOR;
            *player_y = (DROPLETS_ROOM_H * TILE_SIZE) - 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.7f, 1.2f);
        }

        // Se Link usar a Flame Lantern perto do monólito de gelo central:
        if (lantern_is_lit() && fabsf(*player_x - 120.0f) < 28.0f && fabsf(*player_y - 72.0f) < 28.0f) {
            if (!s_droplets.element_ice_melted) {
                s_droplets.element_ice_melted = true;
                cur_map->overlay_layer[4 * 16 + 7] = 0xFF; cur_map->collision_map[4 * 16 + 7] = 0;
                cur_map->overlay_layer[4 * 16 + 8] = 0xFF; cur_map->collision_map[4 * 16 + 8] = 0;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                printf("[DROPLETS] O calor da Flame Lantern descongelou o sagrado Elemento da Agua!\n");
            }
        }
    }
    // --- Sala 4: Antecâmara do Boss Door ---
    else if (s_droplets.current_room == ROOM_DROPLETS_BOSS_DOOR) {
        if (*player_y >= (DROPLETS_ROOM_H * TILE_SIZE) - 20.0f && player_dir == DIR_DOWN) {
            s_droplets.current_room = ROOM_DROPLETS_SANCTUARY;
            *player_y = 24.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.7f, 1.2f);
        } else if (*player_y <= 24.0f && player_dir == DIR_UP) {
            if (s_droplets.has_boss_key && !s_droplets.boss_door_unlocked) {
                s_droplets.boss_door_unlocked = true;
                cur_map->overlay_layer[0 * 16 + 7] = 0xFF; cur_map->collision_map[0 * 16 + 7] = 0;
                cur_map->overlay_layer[0 * 16 + 8] = 0xFF; cur_map->collision_map[0 * 16 + 8] = 0;
                hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                printf("[DROPLETS] Grande Portao do Chefe aberto com a Boss Key!\n");
            }
            if (s_droplets.boss_door_unlocked) {
                s_droplets.current_room = ROOM_DROPLETS_BOSS_ARENA;
                *player_y = (DROPLETS_ROOM_H * TILE_SIZE) - 28.0f;
                hal_audio_play_bgm(BGM_BOSS_BATTLE);
            }
        }
    }
    // --- Sala 5: Arena do Chefe Big Octorok ---
    else if (s_droplets.current_room == ROOM_DROPLETS_BOSS_ARENA) {
        BigOctorokBoss* b = &s_droplets.boss;

        if (b->is_active && b->phase != OCTO_PHASE_DEFEATED) {
            // Atualiza cauda
            b->tail_x = b->x;
            b->tail_y = b->y - 22.0f;

            // FASE 0: Congelado, cospe pedras pontiagudas
            if (b->phase == OCTO_PHASE_FROZEN) {
                b->spit_timer--;
                if (b->spit_timer <= 0) {
                    b->spit_timer = 90;
                    for (int i = 0; i < MAX_OCTO_ROCKS; i++) {
                        if (!b->rocks[i].is_active) {
                            OctoRock* r = &b->rocks[i];
                            r->is_active = true;
                            r->x = b->x;
                            r->y = b->y + 16.0f;
                            float dx = *player_x - r->x;
                            float dy = *player_y - r->y;
                            float dist = sqrtf(dx*dx + dy*dy);
                            if (dist < 1.0f) dist = 1.0f;
                            r->vx = (dx / dist) * 2.2f;
                            r->vy = (dy / dist) * 2.2f;
                            r->reflected = false;
                            hal_audio_play_sound(SOUND_SWORD_SLASH, 0.7f, 1.5f);
                            break;
                        }
                    }
                }
            }
            // FASE 1: Atordoado no gelo após pedra rebatida
            else if (b->phase == OCTO_PHASE_STUNNED) {
                b->phase_timer--;
                if (b->phase_timer <= 0) {
                    b->phase = OCTO_PHASE_FROZEN;
                    b->tail_frozen = true;
                }
            }
            // FASE 2: Em chamas, correndo desesperado pela arena
            else if (b->phase == OCTO_PHASE_BURNING) {
                b->burn_timer--;
                // Movimento errático acelerado
                b->x += cosf(s_drop_anim_timer * 0.08f) * 2.4f;
                b->y += sinf(s_drop_anim_timer * 0.06f) * 1.8f;
                if (b->x < 48.0f) b->x = 48.0f;
                if (b->x > 208.0f) b->x = 208.0f;
                if (b->y < 40.0f) b->y = 40.0f;
                if (b->y > 110.0f) b->y = 110.0f;

                if (b->burn_timer <= 0) {
                    b->phase = OCTO_PHASE_FROZEN;
                    b->tail_frozen = true;
                }
            }

            // Atualiza projéteis de pedras
            for (int i = 0; i < MAX_OCTO_ROCKS; i++) {
                OctoRock* r = &b->rocks[i];
                if (r->is_active) {
                    r->x += r->vx;
                    r->y += r->vy;

                    // Se pedra rebatida atingir o focinho do Big Octorok:
                    if (r->reflected) {
                        float d = sqrtf((r->x - b->x)*(r->x - b->x) + (r->y - (b->y + 12.0f))*(r->y - (b->y + 12.0f)));
                        if (d < 16.0f) {
                            r->is_active = false;
                            b->phase = OCTO_PHASE_STUNNED;
                            b->phase_timer = 140; // Atordoado por ~2.3 segundos
                            hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.2f);
                            printf("[BOSS] PEDRA REBATIDA ACERTOU O FOCINHO! Big Octorok atordoado no gelo! Queime a cauda!\n");
                        }
                    } else {
                        // Causa dano ao jogador se atingir
                        float d = sqrtf((r->x - *player_x)*(r->x - *player_x) + (r->y - *player_y)*(r->y - *player_y));
                        if (d < 10.0f) {
                            r->is_active = false;
                            if (player_hearts && *player_hearts > 1) (*player_hearts)--;
                            hal_audio_play_sound(SOUND_BOSS_HIT, 0.8f, 0.8f);
                        }
                    }

                    // Destrói se sair da tela
                    if (r->x < 0 || r->x > 256 || r->y < 0 || r->y > 160) {
                        r->is_active = false;
                    }
                }
            }
        }

        // Coleta de Recompensas: Coração permanente e Elemento da Água
        if (s_droplets.boss_defeated) {
            // Heart Container
            if (!s_droplets.heart_container_collected) {
                float d = sqrtf((*player_x - 128.0f)*(*player_x - 128.0f) + (*player_y - 80.0f)*(*player_y - 80.0f));
                if (d < 16.0f) {
                    s_droplets.heart_container_collected = true;
                    if (player_hearts) *player_hearts = max_hearts;
                    hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.0f);
                    printf("[DROPLETS] Recipiente de Coracao permanente obtido!\n");
                }
            }
            // Water Element
            if (!s_droplets.water_element_collected) {
                float d = sqrtf((*player_x - 128.0f)*(*player_x - 128.0f) + (*player_y - 50.0f)*(*player_y - 50.0f));
                if (d < 16.0f) {
                    s_droplets.water_element_collected = true;
                    if (player_has_water_element) *player_has_water_element = true;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                    printf("[DROPLETS] O SAGRADO ELEMENTO DA AGUA FOI CONQUISTADO!\n");
                }
            }
        }
    }
}

void dungeon_droplets_render(const Camera* camera, bool is_minish, float player_x, float player_y) {
    if (!s_droplets.is_active || !camera) return;
    Tilemap* m = s_droplets.rooms[s_droplets.current_room];
    if (!m) return;

    // Renderiza o mapa da câmara
    map_render(m, camera);

    // --- RENDERIZADORES ESPECÍFICOS POR CÂMARA ---

    // Sala 1: Facho Solar Oblíquo
    if (s_droplets.current_room == ROOM_DROPLETS_SUNBEAM) {
        draw_rect_blend(80, 0, 70, 160, C_DROP_SUNBEAM);
        // Prisma rotativo no chão
        int px = (int)(128.0f - camera->x);
        int py = (int)(72.0f - camera->y);
        for (int dy = -4; dy <= 4; dy++) {
            for (int dx = -4; dx <= 4; dx++) {
                if (abs(dx) + abs(dy) <= 5) {
                    hal_video_put_pixel(px + dx, py + dy, C_DROP_GOLD);
                }
            }
        }
    }

    // Sala 3: Monólito e Elemento da Água
    if (s_droplets.current_room == ROOM_DROPLETS_SANCTUARY) {
        int ex = (int)(120.0f - camera->x);
        int ey = (int)(72.0f - camera->y);
        // Ícone sagrado da gota de água
        for (int r = 12; r > 0; r--) {
            u32 c = (r < 5) ? 0xFFFFFFFF : 0x06B6D4FF;
            for (int dy = -r; dy <= r; dy++) {
                for (int dx = -r; dx <= r; dx++) {
                    if (dx*dx + dy*dy <= r*r) {
                        hal_video_put_pixel(ex + dx, ey + dy, c);
                    }
                }
            }
        }
    }

    // Sala 5: Chefe Big Octorok e Pedras
    if (s_droplets.current_room == ROOM_DROPLETS_BOSS_ARENA) {
        BigOctorokBoss* b = &s_droplets.boss;
        if (b->is_active && b->phase != OCTO_PHASE_DEFEATED) {
            int ox = (int)(b->x - camera->x);
            int oy = (int)(b->y - camera->y);

            // Cauda (congelada ou em chamas)
            int tx = (int)(b->tail_x - camera->x);
            int ty = (int)(b->tail_y - camera->y);

            if (s_droplets_bosses_tex && s_droplets_bosses_tex->pixels) {
                // Cauda (16x16 em Y=80)
                int tail_src_x = (b->phase == OCTO_PHASE_BURNING) ? 112 : 96;
                texture_draw(s_droplets_bosses_tex, tail_src_x, 80, 16, 16, tx - 8, ty - 8);

                // Corpo maciço (32x32 em Y=80)
                int body_src_x = (b->phase == OCTO_PHASE_BURNING) ? 64 :
                                 (b->phase == OCTO_PHASE_STUNNED) ? 32 : 0;
                texture_draw(s_droplets_bosses_tex, body_src_x, 80, 32, 32, ox - 16, oy - 16);
            } else {
                u32 tail_col = b->tail_frozen ? 0x7DD3FCFF : 0xEA580CFF;
                for (int dy = -6; dy <= 6; dy++) {
                    for (int dx = -6; dx <= 6; dx++) {
                        if (dx*dx + dy*dy <= 36) {
                            hal_video_put_pixel(tx + dx, ty + dy, tail_col);
                        }
                    }
                }

                // Corpo maciço do Big Octorok (32x32 pixels)
                u32 body_c = (b->phase == OCTO_PHASE_BURNING) ? 0xF97316FF : 0x0284C7FF;
                u32 snout_c = (b->phase == OCTO_PHASE_STUNNED) ? 0xFDE047FF : 0x38BDF8FF;
                for (int dy = -16; dy <= 16; dy++) {
                    for (int dx = -16; dx <= 16; dx++) {
                        if (dx*dx + dy*dy <= 256) {
                            hal_video_put_pixel(ox + dx, oy + dy, body_c);
                        }
                    }
                }
                // Focinho/boca proeminente
                for (int dy = 4; dy <= 16; dy++) {
                    for (int dx = -6; dx <= 6; dx++) {
                        hal_video_put_pixel(ox + dx, oy + dy, snout_c);
                    }
                }
                // Olhos
                hal_video_put_pixel(ox - 8, oy - 4, 0xFFFFFFFF);
                hal_video_put_pixel(ox - 7, oy - 4, 0x0F172AFF);
                hal_video_put_pixel(ox + 8, oy - 4, 0xFFFFFFFF);
                hal_video_put_pixel(ox + 7, oy - 4, 0x0F172AFF);
            }
        }

        // Pedras arremessadas
        for (int i = 0; i < MAX_OCTO_ROCKS; i++) {
            const OctoRock* r = &b->rocks[i];
            if (r->is_active) {
                int rx = (int)(r->x - camera->x);
                int ry = (int)(r->y - camera->y);
                if (s_droplets_bosses_tex && s_droplets_bosses_tex->pixels) {
                    int rock_src_x = r->reflected ? 136 : 128;
                    texture_draw(s_droplets_bosses_tex, rock_src_x, 80, 8, 8, rx - 4, ry - 4);
                } else {
                    u32 rc = r->reflected ? 0xFDE047FF : 0x475569FF;
                    for (int dy = -4; dy <= 4; dy++) {
                        for (int dx = -4; dx <= 4; dx++) {
                            if (dx*dx + dy*dy <= 16) {
                                hal_video_put_pixel(rx + dx, ry + dy, rc);
                            }
                        }
                    }
                }
            }
        }

        // Recompensas pós-vitória (Coração e Elemento)
        if (s_droplets.boss_defeated) {
            if (!s_droplets.heart_container_collected) {
                int cx = (int)(128.0f - camera->x);
                int cy = (int)(80.0f - camera->y);
                // Coração de recipiente
                for (int dy = -5; dy <= 5; dy++) {
                    for (int dx = -5; dx <= 5; dx++) {
                        hal_video_put_pixel(cx + dx, cy + dy, 0xEF4444FF);
                    }
                }
            }
            if (!s_droplets.water_element_collected) {
                int ex = (int)(128.0f - camera->x);
                int ey = (int)(50.0f - camera->y);
                for (int dy = -6; dy <= 6; dy++) {
                    for (int dx = -6; dx <= 6; dx++) {
                        if (dx*dx + dy*dy <= 36) {
                            hal_video_put_pixel(ex + dx, ey + dy, 0x06B6D4FF);
                        }
                    }
                }
            }
        }
    }
}

void dungeon_droplets_render_hud_keys(int x, int y) {
    if (!s_droplets.is_active) return;

    // Ícone da Chave Pequena
    if (s_droplets.small_keys > 0) {
        hal_video_put_pixel(x, y + 2, 0xF59E0BFF);
        hal_video_put_pixel(x + 1, y + 2, 0xF59E0BFF);
        hal_video_put_pixel(x + 2, y + 2, 0xF59E0BFF);
        hal_video_put_pixel(x + 2, y + 3, 0xF59E0BFF);
        hal_video_put_pixel(x + 2, y + 4, 0xF59E0BFF);
    }
    // Ícone da Chave do Mestre (Boss Key)
    if (s_droplets.has_boss_key) {
        for (int dy = 0; dy <= 6; dy++) {
            hal_video_put_pixel(x + 8, y + dy, 0xD97706FF);
        }
        hal_video_put_pixel(x + 7, y + 1, 0xFEF08AFF);
        hal_video_put_pixel(x + 9, y + 1, 0xFEF08AFF);
        hal_video_put_pixel(x + 9, y + 4, 0xD97706FF);
    }
}

void dungeon_droplets_shutdown(void) {
    for (int r = 0; r < ROOM_DROPLETS_COUNT; r++) {
        if (s_droplets.rooms[r]) {
            map_destroy(s_droplets.rooms[r]);
            s_droplets.rooms[r] = NULL;
        }
    }
    s_droplets.is_active = false;
    printf("[DUNGEON 4] Temple of Droplets finalizado com sucesso!\n");
}
