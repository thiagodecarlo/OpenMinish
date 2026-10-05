/*
 * ============================================================================
 * src/hal/royal_valley.c - Royal Valley, Cemitério & Tumba do Rei Gustaf
 * ============================================================================
 */

#include "hal/royal_valley.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/sanctuary.h"
#include "hal/lantern.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

// Paleta de Cores de Royal Valley & Graveyard
#define C_ROYAL_GRASS_DARK   0x0F172AFF // Solo gramado sombrio escuro
#define C_ROYAL_GRASS_LIGHT  0x1E293BFF // Grama úmida lúgubre
#define C_ROYAL_PATH         0x18181BFF // Terra batida escura
#define C_ROYAL_TREE_BARK    0x27272AFF // Tronco gótico retorcido
#define C_ROYAL_TREE_LEAVES  0x14532DFF // Folhagem espectral verde musgo profundo
#define C_ROYAL_WATER        0x082F49FF // Canal de água escura do vale
#define C_GRAVE_STONE_LIGHT  0x94A3B8FF // Face da lápide iluminada
#define C_GRAVE_STONE_DARK   0x475569FF // Lado da lápide em sombra
#define C_GUSTAF_TOMB_BASE   0x334155FF // Base do sarcófago monumental
#define C_GUSTAF_TOMB_LID    0x64748BFF // Tampa de pedra do sarcófago
#define C_GUSTAF_GOLD        0xF59E0BFF // Frisos dourados e coroa real
#define C_IRON_GATE          0x1E293BFF // Grades pontiagudas de ferro forjado
#define C_IRON_BARS          0x475569FF // Barras de ferro
#define C_GRAVE_ENGRAVE      0x334155FF // Gravura e relevo nas lápides
#define C_CRYPT_FLOOR        0x1E293BFF // Mármore fúnebre da catacumba
#define C_CRYPT_WALL         0x0F172AFF // Paredão ancestral
#define C_GHOST_AURA         0x38BDF855 // Aura espectral translúcida
#define C_GUSTAF_AURA        0xFDE04766 // Aura dourada ancestral do Rei Gustaf

static RoyalValleyState s_rv = { 0 };

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

static void draw_filled_rect(int rx, int ry, int rw, int rh, u32 color) {
    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer) return;

    for (int y = ry; y < ry + rh; y++) {
        if (y < 0 || y >= ctx->render_height) continue;
        for (int x = rx; x < rx + rw; x++) {
            if (x < 0 || x >= ctx->render_width) continue;
            ctx->framebuffer[y * ctx->render_width + x] = color;
        }
    }
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

static Tilemap* create_royal_map(RoyalValleySceneId scene) {
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;
    memset(m, 0, sizeof(Tilemap));

    m->width = ROYAL_MAP_W;
    m->height = ROYAL_MAP_H;
    int total = ROYAL_MAP_W * ROYAL_MAP_H;

    m->ground_layer = (u8*)malloc(total);
    m->overlay_layer = (u8*)malloc(total);
    m->collision_map = (u8*)malloc(total);
    m->is_authentic = false;

    memset(m->ground_layer, TILE_GRASS, total);
    memset(m->overlay_layer, 0, total);
    memset(m->collision_map, 0, total);

    // Configuração de colisões perimetrais básicas
    for (int y = 0; y < ROYAL_MAP_H; y++) {
        for (int x = 0; x < ROYAL_MAP_W; x++) {
            int idx = y * ROYAL_MAP_W + x;

            if (scene == ROYAL_SCENE_ROYAL_CRYPT) {
                // Catacumbas fechadas
                m->ground_layer[idx] = TILE_MELARI_STONE_FLOOR;
                if (y == 0 || y == ROYAL_MAP_H - 1 || x == 0 || x == ROYAL_MAP_W - 1) {
                    m->collision_map[idx] = 1;
                }
            } else {
                // Cenários externos
                if (y == 0 && (x < 6 || x > 9)) m->collision_map[idx] = 1;
                if (y == ROYAL_MAP_H - 1 && (x < 6 || x > 9)) m->collision_map[idx] = 1;
                if (x == 0 || x == ROYAL_MAP_W - 1) m->collision_map[idx] = 1;
            }
        }
    }

    return m;
}

void royal_valley_init(void) {
    memset(&s_rv, 0, sizeof(s_rv));
    s_rv.is_active = false;
    s_rv.current_scene = ROYAL_SCENE_ENTRANCE;

    for (int i = 0; i < ROYAL_SCENE_COUNT; i++) {
        s_rv.maps[i] = create_royal_map((RoyalValleySceneId)i);
    }

    // Inicializa partículas de névoa
    for (int i = 0; i < MAX_ROYAL_FOG; i++) {
        s_rv.fog[i].active = true;
        s_rv.fog[i].x = (float)(rand() % (ROYAL_MAP_W * TILE_SIZE));
        s_rv.fog[i].y = (float)(rand() % (ROYAL_MAP_H * TILE_SIZE));
        s_rv.fog[i].vx = 0.2f + ((rand() % 10) * 0.03f);
        s_rv.fog[i].vy = ((rand() % 7) - 3) * 0.05f;
        s_rv.fog[i].size = 24.0f + (rand() % 20);
        s_rv.fog[i].color = 0xCBD5E122; // Névoa translúcida suave
        s_rv.fog[i].life = 120 + (rand() % 180);
    }

    // Corvo Takkar na cena da cabana
    s_rv.crow_has_key = true;
    s_rv.crow_x = 3.5f * TILE_SIZE;
    s_rv.crow_y = 3.0f * TILE_SIZE;
    s_rv.crow_defeated = false;
    s_rv.key_on_ground = false;
    s_rv.graveyard_unlocked = false;

    // Fantasmas Ghinis do Cemitério
    s_rv.ghost_count = 3;
    float gh_coords[3][2] = { { 4.0f, 4.0f }, { 11.0f, 4.5f }, { 7.5f, 6.5f } };
    for (int g = 0; g < 3; g++) {
        s_rv.ghosts[g].active = true;
        s_rv.ghosts[g].x = gh_coords[g][0] * TILE_SIZE;
        s_rv.ghosts[g].y = gh_coords[g][1] * TILE_SIZE;
        s_rv.ghosts[g].hp = 3;
        s_rv.ghosts[g].is_poe = (g == 1); // Ghost 1 é um Poe com lanterna
    }

    // Split pads da Four Sword no cemitério (formação em diamante)
    s_rv.split_pad_x[0] = 7.5f * TILE_SIZE; s_rv.split_pad_y[0] = 5.0f * TILE_SIZE; // Top
    s_rv.split_pad_x[1] = 5.5f * TILE_SIZE; s_rv.split_pad_y[1] = 6.2f * TILE_SIZE; // Left
    s_rv.split_pad_x[2] = 9.5f * TILE_SIZE; s_rv.split_pad_y[2] = 6.2f * TILE_SIZE; // Right
    s_rv.split_pad_x[3] = 7.5f * TILE_SIZE; s_rv.split_pad_y[3] = 7.4f * TILE_SIZE; // Bottom

    s_rv.tomb_pushed = false;
    s_rv.tomb_offset_y = 0.0f;

    // Cripta Real
    s_rv.crypt_torches_lit[0] = false;
    s_rv.crypt_torches_lit[1] = false;
    s_rv.crypt_gate_open = false;
    s_rv.king_gustaf_met = false;
    s_rv.has_royal_kinstone = false;
    s_rv.banner_royal_timer = 0;
}

void royal_valley_shutdown(void) {
    for (int i = 0; i < ROYAL_SCENE_COUNT; i++) {
        if (s_rv.maps[i]) {
            if (s_rv.maps[i]->ground_layer) free(s_rv.maps[i]->ground_layer);
            if (s_rv.maps[i]->overlay_layer) free(s_rv.maps[i]->overlay_layer);
            if (s_rv.maps[i]->collision_map) free(s_rv.maps[i]->collision_map);
            free(s_rv.maps[i]);
            s_rv.maps[i] = NULL;
        }
    }
    s_rv.is_active = false;
}

bool royal_valley_is_active(void) {
    return s_rv.is_active;
}

RoyalValleySceneId royal_valley_get_scene(void) {
    return s_rv.current_scene;
}

void royal_valley_enter_entrance(float* link_x, float* link_y, Direction* link_dir) {
    s_rv.is_active = true;
    s_rv.current_scene = ROYAL_SCENE_ENTRANCE;
    if (link_x) *link_x = 7.5f * TILE_SIZE;
    if (link_y) *link_y = 8.5f * TILE_SIZE;
    if (link_dir) *link_dir = DIR_UP;

    s_rv.safe_x = 7.5f * TILE_SIZE;
    s_rv.safe_y = 8.5f * TILE_SIZE;

    hal_audio_play_bgm(BGM_DEEPWOOD_SHRINE);
    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
    printf("[ROYAL VALLEY] Link adentrou o lendario Vale Real (Royal Valley)!\n");
}

void royal_valley_enter_maze(float* link_x, float* link_y, Direction* link_dir) {
    s_rv.is_active = true;
    s_rv.current_scene = ROYAL_SCENE_MIST_MAZE;
    s_rv.maze_step = 0;
    s_rv.maze_failed = false;

    if (link_x) *link_x = 7.5f * TILE_SIZE;
    if (link_y) *link_y = 8.0f * TILE_SIZE;
    if (link_dir) *link_dir = DIR_UP;

    hal_audio_play_sound(SOUND_GUST_BLAST, 1.0f, 0.9f);
    printf("[ROYAL VALLEY] Link penetrou o Labirinto de Nevoa Espectral (Mist Maze)!\n");
}

void royal_valley_enter_dampe(float* link_x, float* link_y, Direction* link_dir) {
    s_rv.is_active = true;
    s_rv.current_scene = ROYAL_SCENE_DAMPE_CABIN;
    if (link_x) *link_x = 7.5f * TILE_SIZE;
    if (link_y) *link_y = 8.5f * TILE_SIZE;
    if (link_dir) *link_dir = DIR_UP;

    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
    printf("[ROYAL VALLEY] Link alcancou a Cabana do Coveiro Dampe!\n");
}

void royal_valley_enter_graveyard(float* link_x, float* link_y, Direction* link_dir) {
    s_rv.is_active = true;
    s_rv.current_scene = ROYAL_SCENE_GRAVEYARD;
    if (link_x) *link_x = 7.5f * TILE_SIZE;
    if (link_y) *link_y = 8.5f * TILE_SIZE;
    if (link_dir) *link_dir = DIR_UP;

    hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
    printf("[GRAVEYARD] Os portoes se abriram! Link adentrou o Cemiterio de Hyrule!\n");
}

void royal_valley_enter_crypt(float* link_x, float* link_y, Direction* link_dir) {
    s_rv.is_active = true;
    s_rv.current_scene = ROYAL_SCENE_ROYAL_CRYPT;
    if (link_x) *link_x = 7.5f * TILE_SIZE;
    if (link_y) *link_y = 8.0f * TILE_SIZE;
    if (link_dir) *link_dir = DIR_UP;

    hal_audio_play_sound(SOUND_CHEST_OPEN, 1.0f, 1.2f);
    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
    printf("[ROYAL CRYPT] Link desceu a Cripta Real do Rei Gustaf!\n");
}

void royal_valley_exit(float* link_x, float* link_y, Direction* link_dir) {
    s_rv.is_active = false;
    if (link_x) *link_x = 7.5f * TILE_SIZE;
    if (link_y) *link_y = 2.0f * TILE_SIZE;
    if (link_dir) *link_dir = DIR_DOWN;

    hal_audio_play_bgm(BGM_MINISH_WOODS);
    hal_audio_play_sound(SOUND_SECRET, 0.9f, 1.1f);
    printf("[ROYAL VALLEY] Link retornou do Vale Real para Hyrule!\n");
}

void royal_valley_update(float* link_x, float* link_y, Direction* link_dir,
                         bool link_moving, int* link_hearts, int max_hearts, int* link_rupees,
                         bool is_charging_spin, bool spin_ready,
                         bool is_attacking, int attack_timer,
                         bool has_four_sword, bool has_lantern, bool lantern_lit) {
    (void)link_moving;
    (void)max_hearts;
    (void)link_rupees;
    (void)is_charging_spin;
    (void)spin_ready;
    (void)is_attacking;
    (void)attack_timer;
    (void)has_four_sword;

    if (!s_rv.is_active) return;

    s_rv.scene_anim_timer++;
    float lx = (link_x ? *link_x : 0.0f);
    float ly = (link_y ? *link_y : 0.0f);

    // 1. Atualização das partículas de névoa atmosférica
    for (int i = 0; i < MAX_ROYAL_FOG; i++) {
        RoyalFogParticle* p = &s_rv.fog[i];
        p->x += p->vx;
        p->y += p->vy;
        p->life--;
        if (p->life <= 0 || p->x > ROYAL_MAP_W * TILE_SIZE + 30.0f) {
            p->x = -20.0f;
            p->y = (float)(rand() % (ROYAL_MAP_H * TILE_SIZE));
            p->life = 140 + (rand() % 160);
        }
    }

    if (s_rv.banner_royal_timer > 0) s_rv.banner_royal_timer--;

    // 2. Comportamento específico por cena
    switch (s_rv.current_scene) {
        case ROYAL_SCENE_ENTRANCE: {
            // Saída norte para o Mist Maze
            if (ly <= 0.8f * TILE_SIZE && lx >= 6.0f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
                royal_valley_enter_maze(link_x, link_y, link_dir);
                return;
            }
            // Saída sul de volta ao Santuário / North Field
            if (ly >= 9.2f * TILE_SIZE && lx >= 6.0f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
                royal_valley_exit(link_x, link_y, link_dir);
                return;
            }
            break;
        }

        case ROYAL_SCENE_MIST_MAZE: {
            // Sequência canônica das 6 etapas do labirinto:
            // 0: Sul (DIR_DOWN), 1: Oeste (DIR_LEFT), 2: Oeste (DIR_LEFT),
            // 3: Norte (DIR_UP), 4: Leste (DIR_RIGHT), 5: Norte (DIR_UP)
            Direction expected_dirs[6] = { DIR_DOWN, DIR_LEFT, DIR_LEFT, DIR_UP, DIR_RIGHT, DIR_UP };

            int stepped_dir = -1;
            if (ly >= 9.2f * TILE_SIZE) stepped_dir = (int)DIR_DOWN;
            else if (lx <= 0.8f * TILE_SIZE) stepped_dir = (int)DIR_LEFT;
            else if (lx >= 14.8f * TILE_SIZE) stepped_dir = (int)DIR_RIGHT;
            else if (ly <= 0.8f * TILE_SIZE) stepped_dir = (int)DIR_UP;

            if (stepped_dir >= 0) {
                if (s_rv.maze_step < 6 && (Direction)stepped_dir == expected_dirs[s_rv.maze_step]) {
                    s_rv.maze_step++;
                    hal_audio_play_sound(SOUND_TEXT_BLIP, 1.0f, 1.4f + s_rv.maze_step * 0.1f);
                    printf("[MIST MAZE] Rota correta! Etapa %d/6 alcancada!\n", s_rv.maze_step);

                    if (s_rv.maze_step >= 6) {
                        // Concluiu o labirinto com maestria!
                        royal_valley_enter_dampe(link_x, link_y, link_dir);
                        return;
                    } else {
                        // Centraliza Link para o próximo passo do labirinto
                        if (link_x) *link_x = 7.5f * TILE_SIZE;
                        if (link_y) *link_y = 5.0f * TILE_SIZE;
                    }
                } else {
                    // Errou a rota! Teleporta de volta ao início
                    s_rv.maze_step = 0;
                    s_rv.maze_fail_timer = 20;
                    if (link_x) *link_x = 7.5f * TILE_SIZE;
                    if (link_y) *link_y = 5.0f * TILE_SIZE;
                    hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.8f, 0.7f);
                    hal_audio_play_sound(SOUND_KEESE_CHIRP, 0.7f, 0.8f);
                    printf("[MIST MAZE] O heroi se perdeu na densa nevoa espectral! Retornou ao inicio!\n");
                }
            }
            if (s_rv.maze_fail_timer > 0) s_rv.maze_fail_timer--;
            break;
        }

        case ROYAL_SCENE_DAMPE_CABIN: {
            // Se a chave caiu no chão e Link caminha sobre ela
            if (s_rv.key_on_ground && !s_rv.graveyard_unlocked) {
                float dist_key = sqrtf((lx - s_rv.key_x) * (lx - s_rv.key_x) + (ly - s_rv.key_y) * (ly - s_rv.key_y));
                if (dist_key < 16.0f) {
                    s_rv.key_on_ground = false;
                    hal_audio_play_sound(SOUND_ITEM_CATCH, 1.0f, 1.2f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                    printf("[ROYAL VALLEY] Link recuperou a Graveyard Key (Chave do Cemiterio)!\n");
                }
            }

            // Portão ao norte (y <= 1.5 * TILE_SIZE, x = 7..8)
            if (ly <= 1.5f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.0f * TILE_SIZE) {
                if (s_rv.graveyard_unlocked) {
                    royal_valley_enter_graveyard(link_x, link_y, link_dir);
                    return;
                } else if (!s_rv.key_on_ground && !s_rv.crow_has_key) {
                    // Possui a chave no bolso
                    s_rv.graveyard_unlocked = true;
                    hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                    printf("[ROYAL VALLEY] O portao de ferro forjado foi destravado com a Graveyard Key!\n");
                    royal_valley_enter_graveyard(link_x, link_y, link_dir);
                    return;
                }
            }
            break;
        }

        case ROYAL_SCENE_GRAVEYARD: {
            // Fantasmas Ghinis rondando
            for (int g = 0; g < s_rv.ghost_count; g++) {
                RoyalGhost* gh = &s_rv.ghosts[g];
                if (!gh->active) continue;

                gh->anim_timer += 0.05f;
                float d_x = lx - gh->x;
                float d_y = ly - gh->y;
                float dist = sqrtf(d_x * d_x + d_y * d_y);

                if (dist > 10.0f && dist < 120.0f) {
                    gh->x += (d_x / dist) * 0.45f;
                    gh->y += (d_y / dist) * 0.45f;
                }

                // Dano em Link por toque
                if (dist < 12.0f && gh->invuln <= 0) {
                    if (link_hearts && *link_hearts > 1) {
                        *link_hearts -= 1;
                        hal_audio_play_sound(SOUND_HEART_BEEP, 1.0f, 1.0f);
                    }
                    gh->invuln = 45;
                }
                if (gh->invuln > 0) gh->invuln--;
            }

            // Abertura da Tumba do Rei Gustaf com os 4 Clones
            if (!s_rv.tomb_pushed && sanctuary_is_clone_active() && sanctuary_get_active_clone_count() >= 2) {
                // Se Link e seus clones empurram a lápide central (x ~ 7.5, y ~ 3.5) para cima
                if (lx >= 6.5f * TILE_SIZE && lx <= 9.0f * TILE_SIZE && ly >= 3.8f * TILE_SIZE && ly <= 4.8f * TILE_SIZE) {
                    if (link_dir && *link_dir == DIR_UP) {
                        s_rv.tomb_offset_y -= 1.0f;
                        if (s_rv.tomb_offset_y <= -24.0f) {
                            s_rv.tomb_pushed = true;
                            hal_audio_play_sound(SOUND_BLOCK_PUSH, 1.0f, 0.9f);
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                            printf("[GRAVEYARD] FORCA DOS 4 HEROIS! A Tumba do Rei Gustaf foi aberta!\n");
                        }
                    }
                }
            }

            // Entrada na Cripta aberta (descer as escadas da tumba)
            if (s_rv.tomb_pushed && lx >= 7.0f * TILE_SIZE && lx <= 8.5f * TILE_SIZE && ly <= 3.2f * TILE_SIZE) {
                royal_valley_enter_crypt(link_x, link_y, link_dir);
                return;
            }
            break;
        }

        case ROYAL_SCENE_ROYAL_CRYPT: {
            // Acendimento de tochas com a Flame Lantern
            if (has_lantern && lantern_lit) {
                // Tocha 1 (esquerda: x=3.5, y=3.0)
                if (fabsf(lx - 3.5f * TILE_SIZE) < 20.0f && fabsf(ly - 3.0f * TILE_SIZE) < 20.0f) {
                    if (!s_rv.crypt_torches_lit[0]) {
                        s_rv.crypt_torches_lit[0] = true;
                        hal_audio_play_sound(SOUND_FIRE, 1.0f, 1.2f);
                    }
                }
                // Tocha 2 (direita: x=11.5, y=3.0)
                if (fabsf(lx - 11.5f * TILE_SIZE) < 20.0f && fabsf(ly - 3.0f * TILE_SIZE) < 20.0f) {
                    if (!s_rv.crypt_torches_lit[1]) {
                        s_rv.crypt_torches_lit[1] = true;
                        hal_audio_play_sound(SOUND_FIRE, 1.0f, 1.2f);
                    }
                }
            }

            // 4 Interruptores de piso simultâneos na cripta
            float sw_coords[4] = { 4.5f, 6.5f, 8.5f, 10.5f };
            float sw_y = 6.0f * TILE_SIZE;
            int pressed_count = 0;

            for (int s = 0; s < 4; s++) {
                float sx = sw_coords[s] * TILE_SIZE;
                bool pressed = (fabsf(lx - sx) < 14.0f && fabsf(ly - sw_y) < 14.0f);

                // Checa também clones da Four Sword
                for (int c = 0; c < 3; c++) {
                    CloneState* cl = sanctuary_get_clone_at(c);
                    if (cl && cl->active) {
                        if (fabsf(cl->x - sx) < 14.0f && fabsf(cl->y - sw_y) < 14.0f) {
                            pressed = true;
                        }
                    }
                }

                s_rv.crypt_switches[s] = pressed;
                if (pressed) pressed_count++;
            }

            // Se as tochas estão acesas e os 4 interruptores acionados: destrava o sarcófago real!
            if (s_rv.crypt_torches_lit[0] && s_rv.crypt_torches_lit[1] && pressed_count == 4 && !s_rv.crypt_gate_open) {
                s_rv.crypt_gate_open = true;
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.2f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.6f);
                printf("[ROYAL CRYPT] O santuario fúnebre sagrado do Rei Gustaf foi desvelado!\n");
            }

            // Flutuação animada do Espírito de Gustaf
            s_rv.gustaf_anim++;
            break;
        }

        default:
            break;
    }
}

bool royal_valley_is_solid(float world_x, float world_y) {
    if (!s_rv.is_active) return false;

    int col = (int)(world_x / TILE_SIZE);
    int row = (int)(world_y / TILE_SIZE);

    if (col < 0 || col >= ROYAL_MAP_W || row < 0 || row >= ROYAL_MAP_H) {
        return true;
    }

    if (s_rv.current_scene == ROYAL_SCENE_ROYAL_CRYPT) {
        // Paredes das catacumbas
        if (row <= 1 || row >= ROYAL_MAP_H - 1 || col <= 1 || col >= ROYAL_MAP_W - 2) return true;
        // Grade da câmara do sarcófago (row 3, entre colunas 6 e 9)
        if (row == 3 && col >= 6 && col <= 9 && !s_rv.crypt_gate_open) return true;
        // Sarcófago central (row 2, col 7 e 8)
        if (row == 2 && (col == 7 || col == 8)) return true;
        return false;
    }

    if (s_rv.current_scene == ROYAL_SCENE_DAMPE_CABIN) {
        // Cabana de Dampé (colunas 2 a 5, linhas 2 a 5)
        if (col >= 2 && col <= 5 && row >= 2 && row <= 5) return true;
        // Portão de ferro fechado (linha 1, colunas 6 a 9)
        if (row <= 1 && col >= 6 && col <= 9 && !s_rv.graveyard_unlocked) return true;
    }

    if (s_rv.current_scene == ROYAL_SCENE_GRAVEYARD) {
        // Tumba monumental do Rei Gustaf (col 7 e 8, row 2 e 3)
        if (!s_rv.tomb_pushed && (col == 7 || col == 8) && (row == 2 || row == 3)) return true;
        // Lápides secundárias
        if (row == 4 && (col == 3 || col == 12)) return true;
        if (row == 6 && (col == 4 || col == 11)) return true;
    }

    // Colisões de borda para as outras cenas
    if (row <= 0 && (col < 6 || col > 9)) return true;
    if (row >= ROYAL_MAP_H - 1 && (col < 6 || col > 9)) return true;
    if (col <= 0 || col >= ROYAL_MAP_W - 1) return true;

    return false;
}

bool royal_valley_interact(float link_x, float link_y, Direction link_dir, int* link_hearts) {
    (void)link_dir;
    (void)link_hearts;
    if (!s_rv.is_active) return false;

    // 1. Interação com Dampé na Cabana
    if (s_rv.current_scene == ROYAL_SCENE_DAMPE_CABIN) {
        float dampe_x = 6.0f * TILE_SIZE;
        float dampe_y = 5.0f * TILE_SIZE;
        if (fabsf(link_x - dampe_x) < 24.0f && fabsf(link_y - dampe_y) < 24.0f) {
            s_rv.dampe_met = true;
            hal_audio_play_sound(SOUND_TEXT_BLIP, 1.0f, 1.0f);
            if (s_rv.crow_has_key) {
                printf("[DAMPE] 'Ola forasteiro! Voce busca a Tumba Real? Eu tinha a chave... mas aquele corvo Takkar roubou-a e esta naquela arvore!'\n");
            } else if (s_rv.key_on_ground) {
                printf("[DAMPE] 'Otimo tiro! Pegue a chave no chao e abra o portao de ferro!'\n");
            } else if (!s_rv.graveyard_unlocked) {
                printf("[DAMPE] 'Voce tem a chave! Use-a na fechadura do grande portao ao norte!'\n");
            } else {
                printf("[DAMPE] 'O cemiterio esta aberto... tome cuidado com os espiritos e que a luz de Gustaf o proteja.'\n");
            }
            return true;
        }
    }

    // 2. Interação com o Espírito do Rei Gustaf na Cripta
    if (s_rv.current_scene == ROYAL_SCENE_ROYAL_CRYPT) {
        float gustaf_x = 7.5f * TILE_SIZE;
        float gustaf_y = 2.8f * TILE_SIZE;
        if (fabsf(link_x - gustaf_x) < 28.0f && fabsf(link_y - gustaf_y) < 28.0f) {
            if (!s_rv.has_royal_kinstone) {
                s_rv.king_gustaf_met = true;
                s_rv.has_royal_kinstone = true;
                s_rv.banner_royal_timer = 240;
                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.5f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.6f);
                printf("[KING GUSTAF] 'Jovem Heroi portador da Four Sword... O destino de Hyrule repousa em sua coragem. Receba a Kinstone Dourada Real e dissipe as trevas de Vaati!'\n");
                return true;
            }
        }
    }

    return false;
}

bool royal_valley_check_sword_hit(float hit_x, float hit_y, float hit_w, float hit_h, int dmg, Direction dir) {
    (void)dmg;
    (void)dir;
    if (!s_rv.is_active) return false;

    // 1. Golpe contra o Corvo Takkar na Cabana de Dampé
    if (s_rv.current_scene == ROYAL_SCENE_DAMPE_CABIN && s_rv.crow_has_key) {
        if (hit_x <= s_rv.crow_x + 16.0f && hit_x + hit_w >= s_rv.crow_x - 16.0f &&
            hit_y <= s_rv.crow_y + 16.0f && hit_y + hit_h >= s_rv.crow_y - 16.0f) {
            s_rv.crow_has_key = false;
            s_rv.crow_defeated = true;
            s_rv.key_on_ground = true;
            s_rv.key_x = s_rv.crow_x + 8.0f;
            s_rv.key_y = s_rv.crow_y + 24.0f;
            hal_audio_play_sound(SOUND_KEESE_CHIRP, 1.0f, 1.4f);
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
            printf("[ROYAL VALLEY] Golpe certeiro! O corvo Takkar soltou a Graveyard Key!\n");
            return true;
        }
    }

    // 2. Golpe contra Fantasmas Ghinis no Cemitério
    if (s_rv.current_scene == ROYAL_SCENE_GRAVEYARD) {
        for (int g = 0; g < s_rv.ghost_count; g++) {
            RoyalGhost* gh = &s_rv.ghosts[g];
            if (!gh->active || gh->invuln > 0) continue;

            if (hit_x <= gh->x + 12.0f && hit_x + hit_w >= gh->x - 12.0f &&
                hit_y <= gh->y + 12.0f && hit_y + hit_h >= gh->y - 12.0f) {
                gh->hp -= dmg;
                gh->invuln = 20;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.3f);
                if (gh->hp <= 0) {
                    gh->active = false;
                    hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.7f, 1.6f);
                }
                return true;
            }
        }
    }

    return false;
}

bool royal_valley_check_projectile_hit(float proj_x, float proj_y) {
    if (!s_rv.is_active) return false;

    // Disparo de Flecha ou Bumerangue no Corvo Takkar
    if (s_rv.current_scene == ROYAL_SCENE_DAMPE_CABIN && s_rv.crow_has_key) {
        if (fabsf(proj_x - s_rv.crow_x) < 18.0f && fabsf(proj_y - s_rv.crow_y) < 18.0f) {
            s_rv.crow_has_key = false;
            s_rv.crow_defeated = true;
            s_rv.key_on_ground = true;
            s_rv.key_x = s_rv.crow_x + 8.0f;
            s_rv.key_y = s_rv.crow_y + 24.0f;
            hal_audio_play_sound(SOUND_KEESE_CHIRP, 1.0f, 1.4f);
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
            printf("[ROYAL VALLEY] Projétil acertou o corvo Takkar! A Graveyard Key caiu ao solo!\n");
            return true;
        }
    }
    return false;
}

bool royal_valley_has_royal_kinstone(void) {
    return s_rv.has_royal_kinstone;
}

bool royal_valley_is_tomb_opened(void) {
    return s_rv.tomb_pushed;
}

void royal_valley_restore_state(bool graveyard_unlocked, bool dampe_met, bool tomb_pushed, bool king_gustaf_met, bool has_kinstone) {
    s_rv.graveyard_unlocked = graveyard_unlocked;
    s_rv.dampe_met = dampe_met;
    s_rv.tomb_pushed = tomb_pushed;
    s_rv.king_gustaf_met = king_gustaf_met;
    s_rv.has_royal_kinstone = has_kinstone;
    if (tomb_pushed) s_rv.tomb_offset_y = -24.0f;
    if (graveyard_unlocked) s_rv.crow_has_key = false;
}

RoyalValleyState* royal_valley_get_state(void) {
    return &s_rv;
}

void royal_valley_render(const Camera* cam, float link_x, float link_y, Direction link_dir, bool is_minish) {
    (void)link_x;
    (void)link_y;
    (void)link_dir;
    (void)is_minish;

    if (!s_rv.is_active) return;

    int ox = (cam ? -(int)cam->x : 0);
    int oy = (cam ? -(int)cam->y : 0);

    // 1. Renderiza o solo quadriculado da cena
    for (int r = 0; r < ROYAL_MAP_H; r++) {
        for (int c = 0; c < ROYAL_MAP_W; c++) {
            int tx = ox + c * TILE_SIZE;
            int ty = oy + r * TILE_SIZE;

            if (s_rv.current_scene == ROYAL_SCENE_ROYAL_CRYPT) {
                u32 col = ((c + r) % 2 == 0) ? C_CRYPT_FLOOR : 0x0F172AFF;
                draw_filled_rect(tx, ty, TILE_SIZE, TILE_SIZE, col);
                draw_filled_rect(tx, ty, TILE_SIZE, 1, 0x334155FF);
                draw_filled_rect(tx, ty, 1, TILE_SIZE, 0x334155FF);
            } else {
                u32 col = ((c + r) % 2 == 0) ? C_ROYAL_GRASS_DARK : C_ROYAL_GRASS_LIGHT;
                draw_filled_rect(tx, ty, TILE_SIZE, TILE_SIZE, col);
            }
        }
    }

    // 2. Renderização de elementos por cena
    switch (s_rv.current_scene) {
        case ROYAL_SCENE_ENTRANCE: {
            // Caminho central de terra batida
            for (int r = 0; r < ROYAL_MAP_H; r++) {
                draw_filled_rect(ox + 7 * TILE_SIZE, oy + r * TILE_SIZE, 32, TILE_SIZE, C_ROYAL_PATH);
            }
            // Árvores góticas nas laterais
            for (int r = 1; r < ROYAL_MAP_H - 1; r += 2) {
                draw_filled_rect(ox + 2 * TILE_SIZE, oy + r * TILE_SIZE, 24, 28, C_ROYAL_TREE_BARK);
                draw_rect_blend(ox + 1 * TILE_SIZE, oy + (r - 1) * TILE_SIZE, 40, 36, C_ROYAL_TREE_LEAVES);
                draw_filled_rect(ox + 13 * TILE_SIZE, oy + r * TILE_SIZE, 24, 28, C_ROYAL_TREE_BARK);
                draw_rect_blend(ox + 12 * TILE_SIZE, oy + (r - 1) * TILE_SIZE, 40, 36, C_ROYAL_TREE_LEAVES);
            }
            // Placa ancestral de advertência
            int sx = ox + 6 * TILE_SIZE;
            int sy = oy + 6 * TILE_SIZE;
            draw_filled_rect(sx, sy, 12, 10, 0x854D0EFF);
            draw_filled_rect(sx + 5, sy + 10, 2, 6, 0x713F12FF);
            break;
        }

        case ROYAL_SCENE_MIST_MAZE: {
            // Labirinto de árvores com 4 passagens abertas
            for (int r = 0; r < ROYAL_MAP_H; r++) {
                for (int c = 0; c < ROYAL_MAP_W; c++) {
                    // Árvores circundantes exceto cruz central
                    if ((c < 6 || c > 9) && (r < 3 || r > 6)) {
                        draw_filled_rect(ox + c * TILE_SIZE, oy + r * TILE_SIZE, TILE_SIZE, TILE_SIZE, C_ROYAL_TREE_BARK);
                        draw_rect_blend(ox + c * TILE_SIZE - 2, oy + r * TILE_SIZE - 2, 20, 20, C_ROYAL_TREE_LEAVES);
                    }
                }
            }
            // Flash de névoa esbranquiçada ao falhar
            if (s_rv.maze_fail_timer > 0) {
                draw_rect_blend(0, 0, 300, 200, 0xFFFFFF88);
            }
            break;
        }

        case ROYAL_SCENE_DAMPE_CABIN: {
            // Cabana rústica de Dampé
            int hx = ox + 2 * TILE_SIZE;
            int hy = oy + 2 * TILE_SIZE;
            draw_filled_rect(hx, hy, 54, 48, 0x3F3F46FF); // Paredes de tábuas
            draw_filled_rect(hx - 4, hy - 10, 62, 14, 0x713F12FF); // Telhado
            draw_filled_rect(hx + 20, hy + 24, 14, 24, 0x18181BFF); // Porta
            draw_filled_rect(hx + 38, hy + 10, 10, 10, 0xFEF08AFF); // Janela iluminada

            // NPC Dampé em pé em frente à cabana
            int dx = ox + (int)(6.0f * TILE_SIZE);
            int dy = oy + (int)(5.0f * TILE_SIZE);
            draw_filled_rect(dx + 2, dy + 1, 8, 6, 0x52525BFF); // Gorro cinza
            draw_filled_rect(dx + 2, dy + 6, 8, 5, 0xFDE8CDFF); // Rosto envelhecido
            draw_filled_rect(dx + 1, dy + 11, 10, 7, 0x78350FFF); // Manto marrom
            draw_filled_rect(dx + 11, dy + 4, 2, 14, 0x713F12FF); // Cabo da pá de coveiro
            draw_filled_rect(dx + 9, dy + 16, 6, 4, 0x94A3B8FF); // Lâmina de ferro da pá

            // Árvore do Corvo Takkar
            int ax = ox + (int)s_rv.crow_x;
            int ay = oy + (int)s_rv.crow_y;
            draw_filled_rect(ax - 6, ay, 12, 32, C_ROYAL_TREE_BARK);
            draw_rect_blend(ax - 18, ay - 14, 36, 24, C_ROYAL_TREE_LEAVES);

            // Corvo Takkar no galho
            if (s_rv.crow_has_key) {
                draw_filled_rect(ax - 2, ay - 6, 8, 6, 0x09090BFF); // Corpo preto
                hal_video_put_pixel(ax + 5, ay - 5, 0xEF4444FF); // Olho vermelho
                hal_video_put_pixel(ax + 6, ay - 4, 0xFACC15FF); // Bico amarelo
                // Chave cintilante presa no bico
                draw_filled_rect(ax + 7, ay - 3, 5, 3, 0xF59E0BFF);
                hal_video_put_pixel(ax + 11, ay - 4, 0xFFFFFFFF);
            }

            // Chave no chão cintilando
            if (s_rv.key_on_ground) {
                int kx = ox + (int)s_rv.key_x;
                int ky = oy + (int)s_rv.key_y;
                draw_filled_rect(kx, ky, 6, 4, 0xF59E0BFF);
                draw_filled_rect(kx + 4, ky - 2, 2, 8, 0xFDE047FF);
                draw_rect_blend(kx - 3, ky - 3, 12, 12, 0xFDE04755);
            }

            // Portão de ferro monumental (Norte: x=6..9, y=1)
            int gx = ox + 6 * TILE_SIZE;
            int gy = oy + (int)(1.0f * TILE_SIZE);
            if (!s_rv.graveyard_unlocked) {
                draw_filled_rect(gx, gy, 56, 16, C_IRON_GATE);
                for (int b = 0; b < 14; b++) {
                    draw_filled_rect(gx + b * 4, gy, 2, 16, C_IRON_BARS);
                }
                // Cadeado dourado no meio
                draw_filled_rect(gx + 24, gy + 4, 8, 8, 0xF59E0BFF);
            } else {
                draw_filled_rect(gx, gy, 56, 4, C_IRON_GATE);
            }
            break;
        }

        case ROYAL_SCENE_GRAVEYARD: {
            // Lápides de pedra espalhadas
            int graves[4][2] = { { 3, 4 }, { 12, 4 }, { 4, 6 }, { 11, 6 } };
            for (int i = 0; i < 4; i++) {
                int gx = ox + graves[i][0] * TILE_SIZE;
                int gy = oy + graves[i][1] * TILE_SIZE;
                draw_filled_rect(gx + 2, gy + 2, 12, 14, C_GRAVE_STONE_LIGHT);
                draw_filled_rect(gx + 4, gy + 4, 8, 2, C_GRAVE_ENGRAVE);
                draw_filled_rect(gx + 7, gy + 4, 2, 8, C_GRAVE_ENGRAVE); // Cruz
            }

            // Pisos de Divisão da Four Sword (Diamond Split Pads)
            for (int p = 0; p < MAX_ROYAL_PADS; p++) {
                int px = ox + (int)s_rv.split_pad_x[p];
                int py = oy + (int)s_rv.split_pad_y[p];
                draw_filled_rect(px - 7, py - 7, 14, 14, 0x0284C7FF);
                draw_filled_rect(px - 5, py - 5, 10, 10, 0x38BDF8FF);
                draw_filled_rect(px - 2, py - 2, 4, 4, 0xFFFFFFFF);
            }

            // Tumba Monumental do Rei Gustaf (col 7..8, row 2..3)
            int tx = ox + 7 * TILE_SIZE;
            int ty = oy + 2 * TILE_SIZE + (int)s_rv.tomb_offset_y;

            // Base de alvenaria e escadas secretas para a cripta
            draw_filled_rect(tx - 4, oy + 2 * TILE_SIZE, 40, 28, 0x09090BFF); // Fosso escuro
            for (int st = 0; st < 4; st++) {
                draw_filled_rect(tx + st * 2, oy + 2 * TILE_SIZE + st * 6, 32 - st * 4, 4, 0x334155FF);
            }

            // Tampa colossal do sarcófago com brasão dourado de Hyrule
            draw_filled_rect(tx - 4, ty, 40, 26, C_GUSTAF_TOMB_BASE);
            draw_filled_rect(tx - 2, ty + 2, 36, 22, C_GUSTAF_TOMB_LID);
            draw_filled_rect(tx + 6, ty + 6, 20, 10, C_GUSTAF_GOLD);
            draw_filled_rect(tx + 14, ty + 4, 4, 14, C_GUSTAF_GOLD); // Cruz real

            // Fantasmas Ghinis flutuando
            for (int g = 0; g < s_rv.ghost_count; g++) {
                RoyalGhost* gh = &s_rv.ghosts[g];
                if (!gh->active) continue;

                int gx = ox + (int)gh->x;
                int gy = oy + (int)(gh->y + sinf(gh->anim_timer) * 3.0f);

                // Aura azulada translúcida
                draw_rect_blend(gx - 4, gy - 4, 20, 20, C_GHOST_AURA);
                draw_filled_rect(gx, gy, 12, 12, 0xF8FAFCFF); // Corpo branco espectral
                hal_video_put_pixel(gx + 3, gy + 3, 0xEF4444FF); // Olho
                hal_video_put_pixel(gx + 8, gy + 3, 0xEF4444FF);

                if (gh->is_poe) {
                    // Lanterna azul do Poe
                    draw_filled_rect(gx + 12, gy + 4, 4, 6, 0x38BDF8FF);
                }
            }
            break;
        }

        case ROYAL_SCENE_ROYAL_CRYPT: {
            // Tochas nas paredes da cripta
            int torches[2][2] = { { 3, 3 }, { 12, 3 } };
            for (int t = 0; t < 2; t++) {
                int tx = ox + torches[t][0] * TILE_SIZE;
                int ty = oy + torches[t][1] * TILE_SIZE;
                draw_filled_rect(tx + 4, ty + 6, 8, 8, 0x713F12FF);
                if (s_rv.crypt_torches_lit[t]) {
                    // Chama acesa
                    draw_filled_rect(tx + 6, ty + 2, 4, 5, 0xEF4444FF);
                    draw_filled_rect(tx + 7, ty + 1, 2, 3, 0xFACC15FF);
                    draw_rect_blend(tx - 6, ty - 6, 24, 24, 0xF9731644);
                }
            }

            // 4 Interruptores de piso simultâneos
            float sw_coords[4] = { 4.5f, 6.5f, 8.5f, 10.5f };
            for (int s = 0; s < 4; s++) {
                int sx = ox + (int)(sw_coords[s] * TILE_SIZE);
                int sy = oy + (int)(6.0f * TILE_SIZE);
                draw_filled_rect(sx - 7, sy - 7, 14, 14, 0x334155FF);
                draw_filled_rect(sx - 5, sy - 5, 10, 10, s_rv.crypt_switches[s] ? 0x22C55EFF : 0xF59E0BFF);
            }

            // Grade da câmara do sarcófago
            int cx = ox + 6 * TILE_SIZE;
            int cy = oy + 3 * TILE_SIZE;
            if (!s_rv.crypt_gate_open) {
                draw_filled_rect(cx, cy, 64, 16, 0x1E293BFF);
                for (int b = 0; b < 16; b++) {
                    draw_filled_rect(cx + b * 4, cy, 2, 16, 0xF59E0BFF);
                }
            }

            // Sarcófago Real e Espírito do Rei Gustaf
            int sx = ox + 7 * TILE_SIZE;
            int sy = oy + (int)(1.8f * TILE_SIZE);
            draw_filled_rect(sx - 4, sy + 10, 40, 18, 0x334155FF);
            draw_filled_rect(sx - 2, sy + 12, 36, 14, C_GUSTAF_GOLD);

            // Espírito de Gustaf flutuando
            float float_y = sinf((float)s_rv.gustaf_anim * 0.08f) * 4.0f;
            int kx = sx + 8;
            int ky = sy - 12 + (int)float_y;

            draw_rect_blend(kx - 12, ky - 10, 40, 40, C_GUSTAF_AURA);
            draw_filled_rect(kx + 2, ky - 4, 12, 4, 0xFDE047FF); // Coroa real
            draw_filled_rect(kx + 3, ky, 10, 8, 0xFEF08AFF);    // Rosto etéreo
            draw_filled_rect(kx + 1, ky + 8, 14, 16, 0x67E8F9CC); // Manto translúcido
            draw_filled_rect(kx + 4, ky + 24, 8, 4, 0xFDE047CC); // Barra dourada

            if (!s_rv.has_royal_kinstone) {
                // Kinstone Real Dourada flutuando entre as mãos de Gustaf
                draw_filled_rect(kx + 6, ky + 10, 5, 5, 0xF59E0BFF);
                draw_rect_blend(kx + 2, ky + 6, 12, 12, 0xFDE04788);
            }
            break;
        }

        default:
            break;
    }

    // 3. Renderização da névoa volumétrica
    for (int i = 0; i < MAX_ROYAL_FOG; i++) {
        RoyalFogParticle* p = &s_rv.fog[i];
        if (p->active) {
            int fx = ox + (int)p->x;
            int fy = oy + (int)p->y;
            draw_rect_blend(fx, fy, (int)p->size, (int)(p->size * 0.6f), p->color);
        }
    }

    // 4. Banner comemorativo de aquisição da Kinstone Real
    if (s_rv.banner_royal_timer > 0) {
        const HalVideoContext* ctx = hal_video_get_context();
        if (ctx) {
            int ban_w = 236;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_filled_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x0F172AEE);
            draw_filled_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0xF59E0BFF);
            draw_filled_rect(ban_x, ban_y, ban_w, ban_h, 0x1E1B4BFF);

            // Ícone da Golden Kinstone da Realeza
            draw_filled_rect(ban_x + 6, ban_y + 8, 12, 16, 0xF59E0BFF);
            draw_filled_rect(ban_x + 9, ban_y + 11, 6, 10, 0xFDE047FF);
            hal_video_put_pixel(ban_x + 12, ban_y + 16, 0xFFFFFFFF);

            font_draw_text(ban_x + 26, ban_y + 6, "ROYAL GOLDEN KINSTONE OBTIDA!", 0xFDE047FF, true);
            font_draw_text(ban_x + 26, ban_y + 18, "O ESPIRITO DO REI GUSTAF O ABENCOA!", 0x38BDF8FF, true);
        }
    }
}
