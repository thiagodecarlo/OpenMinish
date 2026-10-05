/*
 * ============================================================================
 * src/hal/dungeon_palace.c - Masmorra 5: Palace of Winds (Palácio dos Ventos)
 * ============================================================================
 */

#include "hal/dungeon_palace.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/subweapon.h"
#include "hal/inventory.h"
#include "hal/entity.h"
#include "hal/input.h"
#include "hal/rocs_cape.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

// Paleta de cores do Palace of Winds
#define C_PALACE_SKY_TOP    0x0284C7FF
#define C_PALACE_SKY_DEEP   0x0369A1FF
#define C_PALACE_GOLD       0xF59E0BFF
#define C_PALACE_GOLD_LGT   0xFDE047FF
#define C_PALACE_MARBLE     0xF8FAFCFF
#define C_PALACE_CYAN       0x38BDF8FF
#define C_PALACE_CLOUD_WHT  0xFFFFFFDD
#define C_PALACE_CLOUD_BLU  0xBAE6FD99
#define C_GYORG_BLUE        0x2563EBFF
#define C_GYORG_BLUE_LGT    0x60A5FAFF
#define C_GYORG_RED         0xDC2626FF
#define C_GYORG_RED_LGT     0xF87171FF
#define C_GYORG_EYE         0xFBBF24FF
#define C_GYORG_PUPIL       0x991B1BFF

static DungeonPalaceState s_palace = { 0 };
static int s_palace_anim_timer = 0;

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

static Tilemap* create_empty_palace_room(void) {
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;
    m->width = PALACE_ROOM_W;
    m->height = PALACE_ROOM_H;
    int total = m->width * m->height;
    m->ground_layer = (u8*)malloc(total);
    m->overlay_layer = (u8*)malloc(total);
    m->collision_map = (u8*)malloc(total);
    m->is_authentic = false;
    m->authentic_tex = NULL;

    memset(m->ground_layer, TILE_PALACE_FLOOR, total);
    memset(m->overlay_layer, 0xFF, total);
    memset(m->collision_map, 0, total);

    // Paredes e bordas celestes
    for (int y = 0; y < PALACE_ROOM_H; y++) {
        for (int x = 0; x < PALACE_ROOM_W; x++) {
            if (x == 0 || x == PALACE_ROOM_W - 1 || y == 0 || y == PALACE_ROOM_H - 1) {
                m->overlay_layer[y * m->width + x] = TILE_PALACE_WALL;
                m->collision_map[y * m->width + x] = 1;
            }
        }
    }
    return m;
}

void dungeon_palace_init(void) {
    memset(&s_palace, 0, sizeof(s_palace));
    s_palace.is_active = false;
    s_palace.current_room = ROOM_PALACE_ENTRANCE;
    s_palace.small_keys = 0;
    s_palace.has_boss_key = false;

    // Aloca as 6 câmaras com limites estáticos
    for (int r = 0; r < ROOM_PALACE_COUNT; r++) {
        s_palace.rooms[r] = create_empty_palace_room();
    }

    // --- SALA 0: Vestíbulo dos Céus & Turbinas de Vento ---
    Tilemap* r0 = s_palace.rooms[ROOM_PALACE_ENTRANCE];
    // Sul: Acesso de retorno ao Grande Tornado de Cloud Tops
    r0->overlay_layer[9 * 16 + 7] = 0xFF; r0->collision_map[9 * 16 + 7] = 0;
    r0->overlay_layer[9 * 16 + 8] = 0xFF; r0->collision_map[9 * 16 + 8] = 0;
    // Norte: Portão bloqueado até ativar o cata-vento
    r0->overlay_layer[0 * 16 + 7] = TILE_PALACE_WALL; r0->collision_map[0 * 16 + 7] = 1;
    r0->overlay_layer[0 * 16 + 8] = TILE_PALACE_WALL; r0->collision_map[0 * 16 + 8] = 1;
    // Turbina eólica central e abismos laterais
    r0->overlay_layer[3 * 16 + 8] = TILE_PALACE_FAN;
    r0->ground_layer[4 * 16 + 3] = TILE_PALACE_GRATE;
    r0->ground_layer[4 * 16 + 12] = TILE_PALACE_GRATE;
    for (int y = 2; y <= 7; y++) {
        r0->ground_layer[y * 16 + 1] = TILE_PALACE_PIT; r0->collision_map[y * 16 + 1] = 0;
        r0->ground_layer[y * 16 + 14] = TILE_PALACE_PIT; r0->collision_map[y * 16 + 14] = 0;
    }

    // --- SALA 1: Abismos Celestiais & Travessia com Roc's Cape ---
    Tilemap* r1 = s_palace.rooms[ROOM_PALACE_ROCS_CROSSING];
    // Sul: Conexão com Sala 0
    r1->overlay_layer[9 * 16 + 7] = 0xFF; r1->collision_map[9 * 16 + 7] = 0;
    r1->overlay_layer[9 * 16 + 8] = 0xFF; r1->collision_map[9 * 16 + 8] = 0;
    // Leste: Porta trancada com chave pequena (para Sala 2)
    r1->overlay_layer[4 * 16 + 15] = TILE_PALACE_WALL; r1->collision_map[4 * 16 + 15] = 1;
    r1->overlay_layer[5 * 16 + 15] = TILE_PALACE_WALL; r1->collision_map[5 * 16 + 15] = 1;
    // Grande abismo central de céu vazio
    for (int y = 2; y <= 7; y++) {
        for (int x = 2; x <= 13; x++) {
            r1->ground_layer[y * 16 + x] = TILE_PALACE_PIT;
        }
    }
    // Plataformas flutuantes separadas para travessia com Roc's Cape
    for (int y = 4; y <= 5; y++) {
        // Ilha isolada à esquerda com o Baú da Chave #1
        r1->ground_layer[y * 16 + 3] = TILE_PALACE_FLOOR;
        // Ilha de transição ao centro
        r1->ground_layer[y * 16 + 8] = TILE_PALACE_FLOOR;
        // Ilha da porta à direita
        r1->ground_layer[y * 16 + 13] = TILE_PALACE_FLOOR;
    }
    r1->overlay_layer[4 * 16 + 3] = TILE_CHEST_CLOSED;
    r1->collision_map[4 * 16 + 3] = 1;

    // --- SALA 2: Salão dos Interruptores Triplos da Four Sword ---
    Tilemap* r2 = s_palace.rooms[ROOM_PALACE_FOUR_SWITCH];
    // Oeste: Conexão com Sala 1
    r2->overlay_layer[4 * 16 + 0] = 0xFF; r2->collision_map[4 * 16 + 0] = 0;
    r2->overlay_layer[5 * 16 + 0] = 0xFF; r2->collision_map[5 * 16 + 0] = 0;
    // Norte: Portão com laser/barreira de vento (para Sala 3)
    r2->overlay_layer[0 * 16 + 7] = TILE_PALACE_WALL; r2->collision_map[0 * 16 + 7] = 1;
    r2->overlay_layer[0 * 16 + 8] = TILE_PALACE_WALL; r2->collision_map[0 * 16 + 8] = 1;
    // 3 Interruptores Four Sword alinhados
    r2->ground_layer[5 * 16 + 5] = TILE_PALACE_SWITCH;
    r2->ground_layer[5 * 16 + 8] = TILE_PALACE_SWITCH;
    r2->ground_layer[5 * 16 + 11] = TILE_PALACE_SWITCH;

    // --- SALA 3: Torre dos Wizzrobes & Câmara da Boss Key ---
    Tilemap* r3 = s_palace.rooms[ROOM_PALACE_WIZZROBE_TOWER];
    // Sul: Conexão com Sala 2
    r3->overlay_layer[9 * 16 + 7] = 0xFF; r3->collision_map[9 * 16 + 7] = 0;
    r3->overlay_layer[9 * 16 + 8] = 0xFF; r3->collision_map[9 * 16 + 8] = 0;
    // Oeste: Porta trancada até derrotar os Wizzrobes (para Sala 4)
    r3->overlay_layer[4 * 16 + 0] = TILE_PALACE_WALL; r3->collision_map[4 * 16 + 0] = 1;
    r3->overlay_layer[5 * 16 + 0] = TILE_PALACE_WALL; r3->collision_map[5 * 16 + 0] = 1;
    // Passarelas de grades e piso nobre
    for (int y = 3; y <= 6; y++) {
        r3->ground_layer[y * 16 + 4] = TILE_PALACE_GRATE;
        r3->ground_layer[y * 16 + 11] = TILE_PALACE_GRATE;
    }

    // Configura os 2 Wizzrobes celestiais
    s_palace.wizzrobes[0].x = 64.0f;
    s_palace.wizzrobes[0].y = 56.0f;
    s_palace.wizzrobes[0].hp = 4;
    s_palace.wizzrobes[0].alive = true;
    s_palace.wizzrobes[0].visible = true;
    s_palace.wizzrobes[0].teleport_timer = 90;
    s_palace.wizzrobes[0].cast_timer = 45;

    s_palace.wizzrobes[1].x = 192.0f;
    s_palace.wizzrobes[1].y = 56.0f;
    s_palace.wizzrobes[1].hp = 4;
    s_palace.wizzrobes[1].alive = true;
    s_palace.wizzrobes[1].visible = true;
    s_palace.wizzrobes[1].teleport_timer = 120;
    s_palace.wizzrobes[1].cast_timer = 60;

    // --- SALA 4: Passarela Aérea & Grande Portão Alado do Chefe ---
    Tilemap* r4 = s_palace.rooms[ROOM_PALACE_BOSS_GATE];
    // Leste: Conexão com Sala 3
    r4->overlay_layer[4 * 16 + 15] = 0xFF; r4->collision_map[4 * 16 + 15] = 0;
    r4->overlay_layer[5 * 16 + 15] = 0xFF; r4->collision_map[5 * 16 + 15] = 0;
    // Norte: O Grande Portão do Chefe (Boss Door)
    r4->overlay_layer[0 * 16 + 7] = TILE_PALACE_WALL; r4->collision_map[0 * 16 + 7] = 1;
    r4->overlay_layer[0 * 16 + 8] = TILE_PALACE_WALL; r4->collision_map[0 * 16 + 8] = 1;
    // Apenas passarela suspensa central; todo o restante é abismo celestial
    for (int y = 1; y <= 8; y++) {
        for (int x = 1; x <= 14; x++) {
            if (x < 6 || x > 9) {
                r4->ground_layer[y * 16 + x] = TILE_PALACE_PIT;
            } else {
                r4->ground_layer[y * 16 + x] = TILE_PALACE_GRATE;
            }
        }
    }

    // --- SALA 5: Arena Aérea dos Céus - Chefe GYORG PAIR ---
    Tilemap* r5 = s_palace.rooms[ROOM_PALACE_GYORG_ARENA];
    // Sul: Entrada da arena
    r5->overlay_layer[9 * 16 + 7] = 0xFF; r5->collision_map[9 * 16 + 7] = 0;
    r5->overlay_layer[9 * 16 + 8] = 0xFF; r5->collision_map[9 * 16 + 8] = 0;
    // O chão da arena aérea é céu translúcido e nuvens cúmulos
    for (int y = 1; y <= 8; y++) {
        for (int x = 1; x <= 14; x++) {
            r5->ground_layer[y * 16 + x] = TILE_PALACE_FLOOR;
        }
    }

    // Inicializa os parâmetros do chefe Gyorg Pair
    GyorgPairBoss* b = &s_palace.boss;
    b->is_active = true;
    b->blue_x = 128.0f;
    b->blue_y = 80.0f;
    b->blue_health = 12;
    b->blue_eyes_open[0] = true;
    b->blue_eyes_open[1] = true;
    b->blue_eyes_open[2] = true;
    b->blue_eye_timer = 120;

    b->red_x = 200.0f;
    b->red_y = 40.0f;
    b->red_health = 8;
    b->red_eye_open = false;
    b->red_eye_timer = 150;
    b->red_tail_angle = 0.0f;

    b->current_mount = GYORG_MOUNT_BLUE;
    b->mount_timer = 360;
    b->attack_timer = 60;
    b->defeated = false;

    printf("[PALACE OF WINDS] Masmorra 5 inicializada com 6 camaras celestes!\n");
}

void dungeon_palace_enter(float* player_x, float* player_y, Direction* player_dir) {
    s_palace.is_active = true;
    s_palace.current_room = ROOM_PALACE_ENTRANCE;
    if (player_x) *player_x = 128.0f;
    if (player_y) *player_y = (PALACE_ROOM_H * TILE_SIZE) - 26.0f;
    if (player_dir) *player_dir = DIR_UP;

    hal_audio_play_bgm(BGM_PALACE_OF_WINDS);
    printf("[PALACE OF WINDS] Link adentrou o Palacio dos Ventos nos estratosfericos ceus!\n");
}

void dungeon_palace_exit(float* player_x, float* player_y, Direction* player_dir) {
    s_palace.is_active = false;
    if (player_x) *player_x = 128.0f;
    if (player_y) *player_y = 96.0f;
    if (player_dir) *player_dir = DIR_DOWN;

    hal_audio_play_bgm(BGM_CLOUD_TOPS);
    printf("[PALACE OF WINDS] Link retornou ao Santuario da Tribo do Vento em Cloud Tops!\n");
}

bool dungeon_palace_is_active(void) {
    return s_palace.is_active;
}

DungeonPalaceRoomId dungeon_palace_get_current_room(void) {
    return s_palace.current_room;
}

Tilemap* dungeon_palace_get_current_map(void) {
    if (s_palace.current_room < ROOM_PALACE_COUNT) {
        return s_palace.rooms[s_palace.current_room];
    }
    return NULL;
}

DungeonPalaceState* dungeon_palace_get_state(void) {
    return &s_palace;
}

bool dungeon_palace_is_pit_tile(float world_x, float world_y) {
    if (!s_palace.is_active) return false;
    Tilemap* m = dungeon_palace_get_current_map();
    if (!m) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= m->width || ty < 0 || ty >= m->height) return false;

    u8 g = m->ground_layer[ty * m->width + tx];
    return (g == TILE_PALACE_PIT);
}

void dungeon_palace_check_sword_hit(float hit_x, float hit_y, float hit_w, float hit_h, int damage, Direction dir) {
    (void)dir;
    if (!s_palace.is_active) return;

    // Sala 0: Ativação do Cata-vento de Vento
    if (s_palace.current_room == ROOM_PALACE_ENTRANCE && !s_palace.pinwheel_activated) {
        float px = 4.0f * TILE_SIZE + 8.0f;
        float py = 4.0f * TILE_SIZE + 8.0f;
        if (fabsf(hit_x - px) <= 16.0f && fabsf(hit_y - py) <= 16.0f) {
            s_palace.pinwheel_activated = true;
            s_palace.door_entrance_open = true;
            Tilemap* r0 = s_palace.rooms[ROOM_PALACE_ENTRANCE];
            r0->overlay_layer[0 * 16 + 7] = 0xFF; r0->collision_map[0 * 16 + 7] = 0;
            r0->overlay_layer[0 * 16 + 8] = 0xFF; r0->collision_map[0 * 16 + 8] = 0;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
            entity_trigger_screen_shake(12, 3);
            printf("[PALACE OF WINDS] Cata-vento ativado pelo golpe! Portao norte destravado!\n");
        }
    }

    // Sala 3: Combate com os Wizzrobes
    if (s_palace.current_room == ROOM_PALACE_WIZZROBE_TOWER) {
        for (int i = 0; i < MAX_WIZZROBES; i++) {
            PalaceWizzrobe* w = &s_palace.wizzrobes[i];
            if (w->alive && w->visible) {
                if (hit_x < w->x + 12.0f && hit_x + hit_w > w->x - 12.0f &&
                    hit_y < w->y + 16.0f && hit_y + hit_h > w->y - 16.0f) {
                    w->hp -= damage;
                    hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.3f);
                    if (w->hp <= 0) {
                        w->alive = false;
                        hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.4f);
                        printf("[PALACE OF WINDS] Wizzrobe %d derrotado!\n", i + 1);
                    }
                }
            }
        }
    }

    // Sala 5: Batalha Aérea contra GYORG PAIR
    if (s_palace.current_room == ROOM_PALACE_GYORG_ARENA) {
        GyorgPairBoss* b = &s_palace.boss;
        if (b->is_active && !b->defeated) {
            // Combate na Arraia Azul
            if (b->current_mount == GYORG_MOUNT_BLUE) {
                float eye_offsets[3][2] = { { -28.0f, -4.0f }, { 0.0f, -12.0f }, { 28.0f, -4.0f } };
                for (int i = 0; i < 3; i++) {
                    if (b->blue_eyes_open[i]) {
                        float ex = b->blue_x + eye_offsets[i][0];
                        float ey = b->blue_y + eye_offsets[i][1];
                        if (fabsf(hit_x - ex) <= 18.0f && fabsf(hit_y - ey) <= 18.0f) {
                            b->blue_eyes_open[i] = false;
                            b->blue_health -= damage;
                            b->blue_hit_stun = 15;
                            hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.1f);
                            entity_trigger_screen_shake(10, 3);
                            printf("[GYORG PAIR] Olho azul %d atingido! HP restante: %d\n", i + 1, b->blue_health);
                        }
                    }
                }
            }
            // Combate na Arraia Vermelha
            else if (b->current_mount == GYORG_MOUNT_RED) {
                if (b->red_eye_open) {
                    float ex = b->red_x;
                    float ey = b->red_y - 6.0f;
                    if (fabsf(hit_x - ex) <= 16.0f && fabsf(hit_y - ey) <= 16.0f) {
                        b->red_eye_open = false;
                        b->red_health -= damage;
                        b->red_hit_stun = 15;
                        hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.3f);
                        entity_trigger_screen_shake(10, 3);
                        printf("[GYORG PAIR] Olho vermelho atingido! HP restante: %d\n", b->red_health);
                    }
                }
            }
        }
    }
}

void dungeon_palace_check_arrow_hit(float arrow_x, float arrow_y) {
    dungeon_palace_check_sword_hit(arrow_x, arrow_y, 4.0f, 4.0f, 1, DIR_UP);
}

void dungeon_palace_update(float* player_x, float* player_y, Direction* player_dir, bool is_moving,
                           int* player_hearts, int max_hearts, int* player_rupees,
                           bool* player_has_wind_element, bool has_rocs_cape,
                           bool is_jumping, bool is_gliding, bool is_downthrusting, bool is_attacking) {
    (void)is_moving;
    (void)max_hearts;
    (void)has_rocs_cape;
    (void)is_attacking;
    if (!s_palace.is_active || !player_x || !player_y) return;

    s_palace_anim_timer++;
    s_palace.cloud_scroll_x += 0.45f;
    s_palace.cloud_scroll_y += 0.20f;

    // Resgate de segurança contra queda em abismos se não estiver pulando/planando
    if (dungeon_palace_is_pit_tile(*player_x, *player_y) && !is_jumping && !is_gliding && !is_downthrusting) {
        if (s_palace.safe_x > 0.0f && s_palace.safe_y > 0.0f) {
            *player_x = s_palace.safe_x;
            *player_y = s_palace.safe_y;
            if (player_hearts && *player_hearts > 1) (*player_hearts)--;
            hal_audio_play_sound(SOUND_ROLL, 1.0f, 0.7f);
            printf("[PALACE OF WINDS] Link escorregou no abismo e foi resgatado na borda segura!\n");
        }
    } else if (!dungeon_palace_is_pit_tile(*player_x, *player_y)) {
        s_palace.safe_x = *player_x;
        s_palace.safe_y = *player_y;
    }

    // --- Sala 0: Vestíbulo de Entrada ---
    if (s_palace.current_room == ROOM_PALACE_ENTRANCE) {
        // Sul: Retorna ao Santuário das Nuvens
        if (*player_y >= (PALACE_ROOM_H * TILE_SIZE) - 16.0f) {
            dungeon_palace_exit(player_x, player_y, player_dir);
            return;
        }
        // Norte: Avança para Sala 1
        if (s_palace.door_entrance_open && *player_y <= 16.0f) {
            s_palace.current_room = ROOM_PALACE_ROCS_CROSSING;
            *player_y = (PALACE_ROOM_H * TILE_SIZE) - 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
            printf("[PALACE OF WINDS] Link avancou para os Abismos Celestiais da Sala 1!\n");
        }
    }
    // --- Sala 1: Travessia Roc's Cape ---
    else if (s_palace.current_room == ROOM_PALACE_ROCS_CROSSING) {
        // Sul de volta à Sala 0
        if (*player_y >= (PALACE_ROOM_H * TILE_SIZE) - 16.0f) {
            s_palace.current_room = ROOM_PALACE_ENTRANCE;
            *player_y = 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
        }
        // Interação com Baú da Chave Pequena #1
        if (!s_palace.rocs_chest_opened) {
            float cx = 3.0f * TILE_SIZE + 8.0f;
            float cy = 4.0f * TILE_SIZE + 8.0f;
            if (fabsf(*player_x - cx) <= 16.0f && fabsf(*player_y - cy) <= 16.0f) {
                s_palace.rocs_chest_opened = true;
                s_palace.rocs_key_obtained = true;
                s_palace.small_keys++;
                Tilemap* r1 = s_palace.rooms[ROOM_PALACE_ROCS_CROSSING];
                r1->overlay_layer[4 * 16 + 3] = TILE_CHEST_OPEN;
                hal_audio_play_sound(SOUND_CHEST_OPEN, 1.0f, 1.0f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                printf("[PALACE OF WINDS] Bau aberto! Chave Pequena #1 obtida!\n");
            }
        }
        // Porta Leste com chave para Sala 2
        if (*player_x >= (PALACE_ROOM_W * TILE_SIZE) - 20.0f && *player_y >= 50.0f && *player_y <= 90.0f) {
            if (s_palace.small_keys > 0) {
                s_palace.small_keys--;
                Tilemap* r1 = s_palace.rooms[ROOM_PALACE_ROCS_CROSSING];
                r1->overlay_layer[4 * 16 + 15] = 0xFF; r1->collision_map[4 * 16 + 15] = 0;
                r1->overlay_layer[5 * 16 + 15] = 0xFF; r1->collision_map[5 * 16 + 15] = 0;
                hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
                s_palace.current_room = ROOM_PALACE_FOUR_SWITCH;
                *player_x = 24.0f;
                printf("[PALACE OF WINDS] Porta trancada aberta! Link adentrou a Sala dos Clones!\n");
            }
        }
    }
    // --- Sala 2: Interruptores Four Sword ---
    else if (s_palace.current_room == ROOM_PALACE_FOUR_SWITCH) {
        // Oeste de volta à Sala 1
        if (*player_x <= 16.0f) {
            s_palace.current_room = ROOM_PALACE_ROCS_CROSSING;
            *player_x = (PALACE_ROOM_W * TILE_SIZE) - 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
        }
        // Ativação dos 3 Interruptores
        float sw_coords[3][2] = { { 5.0f * 16 + 8, 5.0f * 16 + 8 },
                                  { 8.0f * 16 + 8, 5.0f * 16 + 8 },
                                  { 11.0f * 16 + 8, 5.0f * 16 + 8 } };
        for (int i = 0; i < 3; i++) {
            if (fabsf(*player_x - sw_coords[i][0]) <= 14.0f && fabsf(*player_y - sw_coords[i][1]) <= 14.0f) {
                if (!s_palace.switches_pressed[i]) {
                    s_palace.switches_pressed[i] = true;
                    hal_audio_play_sound(SOUND_SWITCH_CLICK, 0.8f, 1.2f);
                }
            }
        }
        // Se os 3 forem ativados (ou pisados em sequência rápida)
        if (s_palace.switches_pressed[0] && s_palace.switches_pressed[1] && s_palace.switches_pressed[2] && !s_palace.switch_gate_open) {
            s_palace.switch_gate_open = true;
            Tilemap* r2 = s_palace.rooms[ROOM_PALACE_FOUR_SWITCH];
            r2->overlay_layer[0 * 16 + 7] = 0xFF; r2->collision_map[0 * 16 + 7] = 0;
            r2->overlay_layer[0 * 16 + 8] = 0xFF; r2->collision_map[0 * 16 + 8] = 0;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
            entity_trigger_screen_shake(15, 3);
            printf("[PALACE OF WINDS] Enigma da Four Sword resolvido! Portao norte aberto!\n");
        }
        // Norte para Sala 3
        if (s_palace.switch_gate_open && *player_y <= 16.0f) {
            s_palace.current_room = ROOM_PALACE_WIZZROBE_TOWER;
            *player_y = (PALACE_ROOM_H * TILE_SIZE) - 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
        }
    }
    // --- Sala 3: Torre dos Wizzrobes ---
    else if (s_palace.current_room == ROOM_PALACE_WIZZROBE_TOWER) {
        // Sul de volta à Sala 2
        if (*player_y >= (PALACE_ROOM_H * TILE_SIZE) - 16.0f) {
            s_palace.current_room = ROOM_PALACE_FOUR_SWITCH;
            *player_y = 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
        }
        // Atualiza Wizzrobes
        bool all_dead = true;
        for (int i = 0; i < MAX_WIZZROBES; i++) {
            PalaceWizzrobe* w = &s_palace.wizzrobes[i];
            if (w->alive) {
                all_dead = false;
                w->teleport_timer--;
                if (w->teleport_timer <= 0) {
                    w->teleport_timer = 140;
                    w->x = 48.0f + (rand() % 160);
                    w->y = 40.0f + (rand() % 80);
                    w->visible = true;
                }
            }
        }
        // Quando ambos são eliminados, surge o baú da Boss Key e destrava a porta oeste
        if (all_dead && !s_palace.boss_key_chest_spawned) {
            s_palace.boss_key_chest_spawned = true;
            Tilemap* r3 = s_palace.rooms[ROOM_PALACE_WIZZROBE_TOWER];
            r3->overlay_layer[4 * 16 + 0] = 0xFF; r3->collision_map[4 * 16 + 0] = 0;
            r3->overlay_layer[5 * 16 + 0] = 0xFF; r3->collision_map[5 * 16 + 0] = 0;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
            entity_trigger_screen_shake(12, 3);
            printf("[PALACE OF WINDS] Wizzrobes derrotados! Grande Bau da Boss Key apareceu!\n");
        }
        // Coleta da Boss Key
        if (s_palace.boss_key_chest_spawned && !s_palace.boss_key_chest_opened) {
            float bx = 128.0f;
            float by = 80.0f;
            if (fabsf(*player_x - bx) <= 18.0f && fabsf(*player_y - by) <= 18.0f) {
                s_palace.boss_key_chest_opened = true;
                s_palace.has_boss_key = true;
                hal_audio_play_sound(SOUND_CHEST_OPEN, 1.0f, 1.0f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                printf("[PALACE OF WINDS] CHAVE DO MESTRE (BOSS KEY) OBTIDA!\n");
            }
        }
        // Oeste para Sala 4
        if (all_dead && *player_x <= 16.0f) {
            s_palace.current_room = ROOM_PALACE_BOSS_GATE;
            *player_x = (PALACE_ROOM_W * TILE_SIZE) - 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
        }
    }
    // --- Sala 4: Passarela Aérea & Portão do Chefe ---
    else if (s_palace.current_room == ROOM_PALACE_BOSS_GATE) {
        // Leste de volta à Sala 3
        if (*player_x >= (PALACE_ROOM_W * TILE_SIZE) - 16.0f) {
            s_palace.current_room = ROOM_PALACE_WIZZROBE_TOWER;
            *player_x = 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
        }
        // Abertura do Grande Portão do Chefe
        if (*player_y <= 24.0f && *player_x >= 100.0f && *player_x <= 156.0f) {
            if (s_palace.has_boss_key && !s_palace.boss_door_unlocked) {
                s_palace.boss_door_unlocked = true;
                Tilemap* r4 = s_palace.rooms[ROOM_PALACE_BOSS_GATE];
                r4->overlay_layer[0 * 16 + 7] = 0xFF; r4->collision_map[0 * 16 + 7] = 0;
                r4->overlay_layer[0 * 16 + 8] = 0xFF; r4->collision_map[0 * 16 + 8] = 0;
                hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.1f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                printf("[PALACE OF WINDS] Grande Portao Alado do Chefe destravado com a Boss Key!\n");
            }
            if (s_palace.boss_door_unlocked) {
                s_palace.current_room = ROOM_PALACE_GYORG_ARENA;
                *player_y = (PALACE_ROOM_H * TILE_SIZE) - 28.0f;
                hal_audio_play_bgm(BGM_BOSS_BATTLE);
                printf("[PALACE OF WINDS] Link alcancou a Arena Celestial! Confronto contra GYORG PAIR!\n");
            }
        }
    }
    // --- Sala 5: Arena do Chefe GYORG PAIR ---
    else if (s_palace.current_room == ROOM_PALACE_GYORG_ARENA) {
        GyorgPairBoss* b = &s_palace.boss;

        if (b->is_active && !b->defeated) {
            // Movimentação flutuante da Arraia Azul
            b->blue_vx = cosf(s_palace_anim_timer * 0.04f) * 0.8f;
            b->blue_vy = sinf(s_palace_anim_timer * 0.03f) * 0.5f;
            b->blue_x += b->blue_vx;
            b->blue_y += b->blue_vy;

            // Movimentação orbital rápida da Arraia Vermelha
            float red_ang = s_palace_anim_timer * 0.05f;
            b->red_x = 128.0f + cosf(red_ang) * 90.0f;
            b->red_y = 70.0f + sinf(red_ang * 2.0f) * 36.0f;
            b->red_tail_angle += 0.12f;

            // Ciclo de olhos da Arraia Azul
            b->blue_eye_timer--;
            if (b->blue_eye_timer <= 0) {
                b->blue_eye_timer = 180;
                b->blue_eyes_open[0] = true;
                b->blue_eyes_open[1] = true;
                b->blue_eyes_open[2] = true;
            }

            // Ciclo de olho da Arraia Vermelha
            b->red_eye_timer--;
            if (b->red_eye_timer <= 0) {
                b->red_eye_timer = 140;
                b->red_eye_open = true;
            }

            // Alternância de montaria / salto com Roc's Cape
            b->mount_timer--;
            if (b->mount_timer <= 0) {
                b->mount_timer = 320;
                b->current_mount = (b->current_mount == GYORG_MOUNT_BLUE) ? GYORG_MOUNT_RED : GYORG_MOUNT_BLUE;
                hal_audio_play_sound(SOUND_SPIN_ATTACK, 1.0f, 1.2f);
                printf("[GYORG PAIR] As arraias trocaram de altitude! Nova montaria ativa: %s\n",
                       b->current_mount == GYORG_MOUNT_BLUE ? "ARRAIA AZUL" : "ARRAIA VERMELHA");
            }

            // Disparo de esferas de energia de vento
            b->attack_timer--;
            if (b->attack_timer <= 0) {
                b->attack_timer = 110;
                for (int i = 0; i < MAX_GYORG_BALLS; i++) {
                    if (!b->balls[i].active) {
                        GyorgEnergyBall* ball = &b->balls[i];
                        ball->active = true;
                        ball->x = b->red_x;
                        ball->y = b->red_y;
                        float dx = *player_x - ball->x;
                        float dy = *player_y - ball->y;
                        float dist = sqrtf(dx*dx + dy*dy);
                        if (dist < 1.0f) dist = 1.0f;
                        ball->vx = (dx / dist) * 1.8f;
                        ball->vy = (dy / dist) * 1.8f;
                        ball->life = 160;
                        hal_audio_play_sound(SOUND_BOOMERANG_FLY, 0.7f, 1.4f);
                        break;
                    }
                }
            }

            // Atualiza esferas de energia
            for (int i = 0; i < MAX_GYORG_BALLS; i++) {
                GyorgEnergyBall* ball = &b->balls[i];
                if (ball->active) {
                    ball->x += ball->vx;
                    ball->y += ball->vy;
                    ball->life--;
                    if (ball->life <= 0) ball->active = false;

                    // Colisão com Link
                    if (fabsf(*player_x - ball->x) <= 12.0f && fabsf(*player_y - ball->y) <= 12.0f) {
                        ball->active = false;
                        if (player_hearts && *player_hearts > 1) (*player_hearts)--;
                        hal_audio_play_sound(SOUND_ROLL, 1.0f, 0.8f);
                    }
                }
            }

            // Checa condição de vitória (ambas as arraias derrotadas)
            if (b->blue_health <= 0 && b->red_health <= 0) {
                b->defeated = true;
                b->defeat_timer = 120;
                hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                entity_trigger_screen_shake(30, 5);
                printf("[GYORG PAIR] O casal de arraias gigantes foi derrotado no vácuo celestial!\n");
            }
        }
        // Sequência pós-derrota: recompensas
        else if (b->defeated) {
            if (b->defeat_timer > 0) {
                b->defeat_timer--;
                if (b->defeat_timer == 0) {
                    // Spawna o Heart Container e o sagrado Elemento do Vento
                    s_palace.heart_container_spawned = true;
                    s_palace.heart_container_x = 128.0f;
                    s_palace.heart_container_y = 64.0f;

                    s_palace.wind_element_spawned = true;
                    s_palace.wind_element_x = 128.0f;
                    s_palace.wind_element_y = 96.0f;

                    s_palace.warp_portal_spawned = true;
                    s_palace.warp_portal_x = 128.0f;
                    s_palace.warp_portal_y = 128.0f;

                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                    hal_audio_play_bgm(BGM_MINISH_WOODS);
                }
            }

            // Coleta do Heart Container
            if (s_palace.heart_container_spawned && !s_palace.heart_container_collected) {
                if (fabsf(*player_x - s_palace.heart_container_x) <= 16.0f &&
                    fabsf(*player_y - s_palace.heart_container_y) <= 16.0f) {
                    s_palace.heart_container_collected = true;
                    if (player_hearts) *player_hearts += 4;
                    hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.0f);
                    printf("[PALACE OF WINDS] Recipiente de Coracao obtido permanentemente!\n");
                }
            }

            // Coleta do Elemento do Vento (Wind Element)
            if (s_palace.wind_element_spawned && !s_palace.wind_element_collected) {
                if (fabsf(*player_x - s_palace.wind_element_x) <= 16.0f &&
                    fabsf(*player_y - s_palace.wind_element_y) <= 16.0f) {
                    s_palace.wind_element_collected = true;
                    if (player_has_wind_element) *player_has_wind_element = true;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                    entity_trigger_screen_shake(20, 4);
                    printf("[PALACE OF WINDS] O SAGRADO ELEMENTO DO VENTO FOI CONQUISTADO!\n");
                }
            }

            // Entrada no Portal de Teletransporte
            if (s_palace.warp_portal_spawned && s_palace.wind_element_collected) {
                if (fabsf(*player_x - s_palace.warp_portal_x) <= 16.0f &&
                    fabsf(*player_y - s_palace.warp_portal_y) <= 16.0f) {
                    hal_audio_play_sound(SOUND_MINISH_GROW, 1.0f, 1.2f);
                    dungeon_palace_exit(player_x, player_y, player_dir);
                    return;
                }
            }
        }
    }
}

void dungeon_palace_render(const Camera* camera, float player_x, float player_y, bool is_minish) {
    (void)player_x;
    (void)player_y;
    (void)is_minish;
    if (!s_palace.is_active || !camera) return;

    Tilemap* m = dungeon_palace_get_current_map();
    if (!m) return;

    // Renderiza o mapa da câmara atual
    map_render(m, camera);

    int ox = -(int)camera->x;
    int oy = -(int)camera->y;

    // Sala 0: Renderiza turbina eólica giratória e cata-vento
    if (s_palace.current_room == ROOM_PALACE_ENTRANCE) {
        int fx = ox + 4 * TILE_SIZE + 8;
        int fy = oy + 4 * TILE_SIZE + 8;
        u32 pin_color = s_palace.pinwheel_activated ? 0x22C55EFF : 0xF59E0BFF;
        draw_rect_blend(fx - 6, fy - 6, 12, 12, pin_color);
        draw_rect_blend(fx - 2, fy - 2, 4, 4, 0xFFFFFFFF);
    }

    // Sala 1: Baú de tesouro da chave
    if (s_palace.current_room == ROOM_PALACE_ROCS_CROSSING && !s_palace.rocs_chest_opened) {
        int cx = ox + 3 * TILE_SIZE + 4;
        int cy = oy + 4 * TILE_SIZE + 4;
        draw_rect_blend(cx, cy, 10, 8, C_PALACE_GOLD);
        draw_rect_blend(cx + 2, cy + 2, 6, 4, 0xFEF08AFF);
    }

    // Sala 2: Luzes dos 3 interruptores Four Sword
    if (s_palace.current_room == ROOM_PALACE_FOUR_SWITCH) {
        int sw_coords[3][2] = { { 5 * 16 + 8, 5 * 16 + 8 },
                                { 8 * 16 + 8, 5 * 16 + 8 },
                                { 11 * 16 + 8, 5 * 16 + 8 } };
        for (int i = 0; i < 3; i++) {
            u32 c = s_palace.switches_pressed[i] ? 0x22C55EFF : 0x38BDF8FF;
            draw_rect_blend(ox + sw_coords[i][0] - 5, oy + sw_coords[i][1] - 5, 10, 10, c);
        }
    }

    // Sala 3: Wizzrobes e Baú da Boss Key
    if (s_palace.current_room == ROOM_PALACE_WIZZROBE_TOWER) {
        for (int i = 0; i < MAX_WIZZROBES; i++) {
            PalaceWizzrobe* w = &s_palace.wizzrobes[i];
            if (w->alive && w->visible) {
                int wx = ox + (int)w->x;
                int wy = oy + (int)w->y;
                // Manto celestial e capuz
                draw_rect_blend(wx - 6, wy - 10, 12, 16, 0x0284C7FF);
                draw_rect_blend(wx - 4, wy - 14, 8, 6, 0xFBBF24FF);
                // Olhos amarelos
                hal_video_put_pixel(wx - 2, wy - 6, 0xFDE047FF);
                hal_video_put_pixel(wx + 2, wy - 6, 0xFDE047FF);
            }
        }
        if (s_palace.boss_key_chest_spawned && !s_palace.boss_key_chest_opened) {
            int cx = ox + 128 - 7;
            int cy = oy + 80 - 7;
            draw_rect_blend(cx, cy, 14, 12, C_PALACE_GOLD);
            draw_rect_blend(cx + 2, cy + 2, 10, 8, 0xFDE047FF);
            draw_rect_blend(cx + 5, cy + 4, 4, 4, 0xEF4444FF); // Joia vermelha da Big Key
        }
    }

    // Sala 5: Arena do Chefe GYORG PAIR
    if (s_palace.current_room == ROOM_PALACE_GYORG_ARENA) {
        GyorgPairBoss* b = &s_palace.boss;

        if (b->is_active && !b->defeated) {
            // 1. RENDERIZAÇÃO DA GRANDE ARRAIA AZUL (Fêmea)
            int bx = ox + (int)b->blue_x;
            int by = oy + (int)b->blue_y;
            u32 blue_c = (b->blue_hit_stun > 0) ? 0xFFFFFFFF : C_GYORG_BLUE;

            // Corpo elíptico e asas majestosas
            for (int dy = -26; dy <= 26; dy++) {
                for (int dx = -54; dx <= 54; dx++) {
                    float ex = (float)dx / 54.0f;
                    float ey = (float)dy / 26.0f;
                    if (ex * ex + ey * ey <= 1.0f) {
                        hal_video_put_pixel(bx + dx, by + dy, blue_c);
                    }
                }
            }

            // 3 Olhos na Arraia Azul
            float eye_offsets[3][2] = { { -28.0f, -4.0f }, { 0.0f, -12.0f }, { 28.0f, -4.0f } };
            for (int i = 0; i < 3; i++) {
                int ex = bx + (int)eye_offsets[i][0];
                int ey = by + (int)eye_offsets[i][1];
                if (b->blue_eyes_open[i]) {
                    draw_rect_blend(ex - 4, ey - 4, 8, 8, C_GYORG_EYE);
                    draw_rect_blend(ex - 2, ey - 2, 4, 4, C_GYORG_PUPIL);
                } else {
                    draw_rect_blend(ex - 3, ey - 1, 6, 2, 0x1E3A8AFF); // Olho fechado
                }
            }

            // 2. RENDERIZAÇÃO DA ARRAIA VERMELHA (Macho)
            int rx = ox + (int)b->red_x;
            int ry = oy + (int)b->red_y;
            u32 red_c = (b->red_hit_stun > 0) ? 0xFFFFFFFF : C_GYORG_RED;

            for (int dy = -18; dy <= 18; dy++) {
                for (int dx = -32; dx <= 32; dx++) {
                    float ex = (float)dx / 32.0f;
                    float ey = (float)dy / 18.0f;
                    if (ex * ex + ey * ey <= 1.0f) {
                        hal_video_put_pixel(rx + dx, ry + dy, red_c);
                    }
                }
            }

            // Olho central na Arraia Vermelha
            if (b->red_eye_open) {
                draw_rect_blend(rx - 4, ry - 7, 8, 8, C_GYORG_EYE);
                draw_rect_blend(rx - 2, ry - 5, 4, 4, C_GYORG_PUPIL);
            }

            // Cauda chicoteante
            int tx = rx + (int)(cosf(b->red_tail_angle) * 28.0f);
            int ty = ry + (int)(sinf(b->red_tail_angle) * 28.0f);
            draw_rect_blend(tx - 3, ty - 3, 6, 6, 0x7F1D1DFF);

            // Esferas de energia de vento
            for (int i = 0; i < MAX_GYORG_BALLS; i++) {
                if (b->balls[i].active) {
                    int px = ox + (int)b->balls[i].x;
                    int py = oy + (int)b->balls[i].y;
                    draw_rect_blend(px - 4, py - 4, 8, 8, 0x38BDF8FF);
                    draw_rect_blend(px - 2, py - 2, 4, 4, 0xFFFFFFFF);
                }
            }
        }

        // Recompensas pós-vitória (Heart Container, Wind Element, Warp Portal)
        if (s_palace.heart_container_spawned && !s_palace.heart_container_collected) {
            int cx = ox + (int)s_palace.heart_container_x;
            int cy = oy + (int)s_palace.heart_container_y;
            draw_rect_blend(cx - 6, cy - 6, 12, 12, 0xEF4444FF);
            draw_rect_blend(cx - 3, cy - 3, 6, 6, 0xFCA5A5FF);
        }

        if (s_palace.wind_element_spawned && !s_palace.wind_element_collected) {
            int ex = ox + (int)s_palace.wind_element_x;
            int ey = oy + (int)s_palace.wind_element_y;
            // Sagrado Elemento do Vento (Gema Verde Esmeralda Celestial com aura dourada)
            draw_rect_blend(ex - 8, ey - 8, 16, 16, 0xFDE04766);
            draw_rect_blend(ex - 6, ey - 6, 12, 12, 0x10B981FF);
            draw_rect_blend(ex - 3, ey - 3, 6, 6, 0x6EE7B7FF);
            hal_video_put_pixel(ex, ey, 0xFFFFFFFF);
        }

        if (s_palace.warp_portal_spawned) {
            int px = ox + (int)s_palace.warp_portal_x;
            int py = oy + (int)s_palace.warp_portal_y;
            draw_rect_blend(px - 10, py - 10, 20, 20, 0x38BDF855);
            draw_rect_blend(px - 6, py - 6, 12, 12, 0x67E8F9CC);
            draw_rect_blend(px - 2, py - 2, 4, 4, 0xFFFFFFFF);
        }
    }
}

void dungeon_palace_render_hud_keys(int x, int y) {
    if (!s_palace.is_active) return;

    // Ícone da Chave Pequena
    if (s_palace.small_keys > 0) {
        hal_video_put_pixel(x, y + 2, 0xF59E0BFF);
        hal_video_put_pixel(x + 1, y + 2, 0xF59E0BFF);
        hal_video_put_pixel(x + 2, y + 2, 0xF59E0BFF);
        hal_video_put_pixel(x + 2, y + 3, 0xF59E0BFF);
        hal_video_put_pixel(x + 2, y + 4, 0xF59E0BFF);
    }
    // Ícone da Chave do Mestre (Boss Key)
    if (s_palace.has_boss_key) {
        for (int dy = 0; dy <= 6; dy++) {
            hal_video_put_pixel(x + 8, y + dy, 0xD97706FF);
        }
        hal_video_put_pixel(x + 7, y + 1, 0xFEF08AFF);
        hal_video_put_pixel(x + 9, y + 1, 0xFEF08AFF);
        hal_video_put_pixel(x + 9, y + 4, 0xD97706FF);
    }
}

bool dungeon_palace_has_wind_element(void) {
    return s_palace.wind_element_collected;
}

void dungeon_palace_shutdown(void) {
    for (int r = 0; r < ROOM_PALACE_COUNT; r++) {
        if (s_palace.rooms[r]) {
            map_destroy(s_palace.rooms[r]);
            s_palace.rooms[r] = NULL;
        }
    }
    s_palace.is_active = false;
    printf("[DUNGEON 5] Palace of Winds finalizado com sucesso!\n");
}
