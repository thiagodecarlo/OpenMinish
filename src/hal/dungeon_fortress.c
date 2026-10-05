/*
 * ============================================================================
 * src/hal/dungeon_fortress.c - Subsistema de Masmorras: Fortress of Winds (Dungeon 3)
 * ============================================================================
 */

#include "hal/dungeon_fortress.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/subweapon.h"
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

// Paleta de cores da Fortress of Winds
#define C_FORT_WALL_TOP     0x475569FF
#define C_FORT_WALL_FACE    0x1E293BFF
#define C_FORT_WALL_DARK    0x0F172AFF
#define C_FORT_FLOOR_BASE   0x334155FF
#define C_FORT_FLOOR_CRK    0x1E3A52FF
#define C_FORT_PIT          0x050811FF
#define C_FORT_WIND_CYAN    0x38BDF8FF
#define C_FORT_GOLD         0xF59E0BFF
#define C_FORT_EYE_CYAN     0x06B6D4FF
#define C_FORT_MAZAAL_HEAD  0x64748BFF
#define C_FORT_MAZAAL_BRASS 0xD97706FF

static DungeonFortressState s_fortress = { 0 };
static int s_fort_anim_timer = 0;

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

static void draw_minish_pedestal(int sx, int sy) {
    draw_filled_rect(sx + 2, sy + 4, 12, 10, 0x475569FF);
    draw_filled_rect(sx + 3, sy + 5, 10, 8, 0x64748BFF);
    draw_filled_rect(sx + 5, sy + 7, 6, 4, 0x38BDF8FF);
}

void dungeon_fortress_init(void) {
    memset(&s_fortress, 0, sizeof(s_fortress));
    s_fortress.active = false;
    s_fortress.current_room = ROOM_FORTRESS_ENTRANCE;
    s_fortress.small_keys = 0;
    s_fortress.has_boss_key = false;

    // Configuração Sala 1 (Escavação de terra fofa)
    for (int y = 0; y < FORTRESS_ROOM_H; y++) {
        for (int x = 0; x < FORTRESS_ROOM_W; x++) {
            if (x >= 3 && x <= 12 && y >= 2 && y <= 7) {
                // Paredes escaváveis no miolo da sala
                s_fortress.dirt_grid[y][x] = 1;
            } else {
                s_fortress.dirt_grid[y][x] = 0;
            }
        }
    }
    // Clareira do baú no centro
    s_fortress.dirt_grid[4][7] = 0;
    s_fortress.dirt_grid[4][8] = 0;

    // Configuração do Chefe Mazaal
    s_fortress.mazaal_head.x = 128.0f;
    s_fortress.mazaal_head.y = 38.0f;
    s_fortress.mazaal_head.core_health = 12;
    s_fortress.mazaal_head.stun_cycle = 0;
    s_fortress.mazaal_head.stunned = false;
    s_fortress.mazaal_head.mouth_open = false;
    s_fortress.mazaal_head.defeated = false;

    s_fortress.mazaal_hand_l.base_x = 64.0f;
    s_fortress.mazaal_hand_l.base_y = 68.0f;
    s_fortress.mazaal_hand_l.x = 64.0f;
    s_fortress.mazaal_hand_l.y = 68.0f;
    s_fortress.mazaal_hand_l.health = 3;
    s_fortress.mazaal_hand_l.active = true;
    s_fortress.mazaal_hand_l.eye_open = true;

    s_fortress.mazaal_hand_r.base_x = 192.0f;
    s_fortress.mazaal_hand_r.base_y = 68.0f;
    s_fortress.mazaal_hand_r.x = 192.0f;
    s_fortress.mazaal_hand_r.y = 68.0f;
    s_fortress.mazaal_hand_r.health = 3;
    s_fortress.mazaal_hand_r.active = true;
    s_fortress.mazaal_hand_r.eye_open = true;

    printf("[FORTRESS] Fortress of Winds (Dungeon 3) inicializada com sucesso!\n");
}

bool dungeon_fortress_is_active(void) {
    return s_fortress.active;
}

DungeonFortressState* dungeon_fortress_get_state(void) {
    return &s_fortress;
}

void dungeon_fortress_enter(float* link_x, float* link_y, Direction* link_dir) {
    s_fortress.active = true;
    s_fortress.current_room = ROOM_FORTRESS_ENTRANCE;
    s_fortress.transitioning = false;
    s_fortress.trans_timer = 0;

    if (link_x)   *link_x = 128.0f;
    if (link_y)   *link_y = 136.0f;
    if (link_dir) *link_dir = DIR_UP;

    s_fortress.safe_x = 128.0f;
    s_fortress.safe_y = 136.0f;

    hal_audio_play_bgm(BGM_FORTRESS_OF_WINDS);
    hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 0.9f);
    printf("[DUNGEON] Link adentrou a Fortress of Winds (Dungeon 3)!\n");
}

void dungeon_fortress_exit(float* link_x, float* link_y, Direction* link_dir) {
    s_fortress.active = false;
    s_fortress.transitioning = false;
    s_fortress.trans_timer = 0;

    if (link_x)   *link_x = 496.0f;
    if (link_y)   *link_y = 64.0f;
    if (link_dir) *link_dir = DIR_DOWN;

    hal_audio_play_bgm(BGM_WIND_RUINS);
    hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.1f);
    printf("[DUNGEON] Link retornou as Wind Ruins da Fortress of Winds!\n");
}

bool dungeon_fortress_has_ocarina(void) {
    return s_fortress.ocarina_collected;
}

void dungeon_fortress_shoot_arrow_hit(float arrow_x, float arrow_y) {
    if (!s_fortress.active) return;

    if (s_fortress.current_room == ROOM_FORTRESS_ENTRANCE) {
        // Alvo de olho norte na Sala 0 (x=120..136, y=16..32)
        if (arrow_x >= 116.0f && arrow_x <= 140.0f && arrow_y >= 12.0f && arrow_y <= 36.0f) {
            if (!s_fortress.eye_entrance_hit) {
                s_fortress.eye_entrance_hit = true;
                s_fortress.door_entrance_open = true;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                entity_trigger_screen_shake(8, 2);
                printf("[FORTRESS] Olho da entrada atingido! Portao norte destravado!\n");
            }
        }
    } else if (s_fortress.current_room == ROOM_FORTRESS_ARROW_PUZZLE) {
        // Olho esquerdo (x=48, y=24)
        if (arrow_x >= 40.0f && arrow_x <= 56.0f && arrow_y >= 16.0f && arrow_y <= 36.0f) {
            if (!s_fortress.arrow_eye_left_hit) {
                s_fortress.arrow_eye_left_hit = true;
                hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.3f);
                printf("[FORTRESS] Olho mecânico esquerdo ativado!\n");
            }
        }
        // Olho direito (x=208, y=24)
        if (arrow_x >= 200.0f && arrow_x <= 216.0f && arrow_y >= 16.0f && arrow_y <= 36.0f) {
            if (!s_fortress.arrow_eye_right_hit) {
                s_fortress.arrow_eye_right_hit = true;
                hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.3f);
                printf("[FORTRESS] Olho mecânico direito ativado!\n");
            }
        }
        // Se ambos ativados: estende a ponte sobre o abismo!
        if (s_fortress.arrow_eye_left_hit && s_fortress.arrow_eye_right_hit && !s_fortress.bridge_extended) {
            s_fortress.bridge_extended = true;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
            entity_trigger_screen_shake(10, 2);
            printf("[FORTRESS] Ambos os olhos acionados! Ponte do abismo estendida!\n");
        }
    } else if (s_fortress.current_room == ROOM_FORTRESS_BOSS_ARENA) {
        // Disparo de flecha nas palmas de Mazaal
        MazaalHand* hl = &s_fortress.mazaal_hand_l;
        if (hl->active && !hl->stunned) {
            if (fabsf(arrow_x - hl->x) < 14.0f && fabsf(arrow_y - hl->y) < 14.0f) {
                hl->health--;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.2f);
                entity_trigger_screen_shake(6, 2);
                if (hl->health <= 0) {
                    hl->stunned = true;
                    hl->active = false;
                    hl->y = 110.0f; // Cai inerte
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.9f);
                    printf("[MAZAAL] Mao esquerda desativada pela flecha!\n");
                }
            }
        }
        MazaalHand* hr = &s_fortress.mazaal_hand_r;
        if (hr->active && !hr->stunned) {
            if (fabsf(arrow_x - hr->x) < 14.0f && fabsf(arrow_y - hr->y) < 14.0f) {
                hr->health--;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.2f);
                entity_trigger_screen_shake(6, 2);
                if (hr->health <= 0) {
                    hr->stunned = true;
                    hr->active = false;
                    hr->y = 110.0f; // Cai inerte
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.9f);
                    printf("[MAZAAL] Mao direita desativada pela flecha!\n");
                }
            }
        }
    }
}

void dungeon_fortress_update(float* link_x, float* link_y, Direction* link_dir, bool is_moving,
                             int* link_hearts, int* link_rupees, bool is_minish, bool is_attacking,
                             bool* out_warp_to_surface) {
    if (!s_fortress.active || !link_x || !link_y) return;

    s_fort_anim_timer++;

    // Transição entre salas
    if (s_fortress.transitioning) {
        s_fortress.trans_timer--;
        if (s_fortress.trans_timer <= 0) {
            s_fortress.transitioning = false;
            s_fortress.current_room = s_fortress.target_room;
            *link_x = s_fortress.target_spawn_x;
            *link_y = s_fortress.target_spawn_y;
            s_fortress.safe_x = *link_x;
            s_fortress.safe_y = *link_y;
            printf("[FORTRESS] Transicao para Sala %d concluida!\n", s_fortress.current_room);
        }
        return;
    }

    float lx = *link_x;
    float ly = *link_y;

    // Atualização por sala
    switch (s_fortress.current_room) {
        case ROOM_FORTRESS_ENTRANCE: {
            // Saída sul de volta às Wind Ruins
            if (ly >= 148.0f && *link_dir == DIR_DOWN) {
                dungeon_fortress_exit(link_x, link_y, link_dir);
                return;
            }
            // Passagem norte para a Sala 1 (requer portão aberto)
            if (s_fortress.door_entrance_open && ly <= 20.0f && lx >= 112.0f && lx <= 144.0f && *link_dir == DIR_UP) {
                s_fortress.transitioning = true;
                s_fortress.trans_timer = 20;
                s_fortress.target_room = ROOM_FORTRESS_DIG_MAZE;
                s_fortress.target_spawn_x = 128.0f;
                s_fortress.target_spawn_y = 136.0f;
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.1f);
            }
            break;
        }

        case ROOM_FORTRESS_DIG_MAZE: {
            // Saída sul de volta à entrada
            if (ly >= 148.0f && *link_dir == DIR_DOWN) {
                s_fortress.transitioning = true;
                s_fortress.trans_timer = 20;
                s_fortress.target_room = ROOM_FORTRESS_ENTRANCE;
                s_fortress.target_spawn_x = 128.0f;
                s_fortress.target_spawn_y = 28.0f;
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.1f);
                return;
            }

            // Escavação de terra fofa com Mole Mitts
            if (subweapon_get_current() == ITEM_MOLE_MITTS && subweapon_is_digging()) {
                int tx = (int)((lx + 8.0f) / 16.0f);
                int ty = (int)((ly + 12.0f) / 16.0f);
                if (*link_dir == DIR_DOWN)  ty++;
                if (*link_dir == DIR_UP)    ty--;
                if (*link_dir == DIR_LEFT)  tx--;
                if (*link_dir == DIR_RIGHT) tx++;

                if (tx >= 0 && tx < FORTRESS_ROOM_W && ty >= 0 && ty < FORTRESS_ROOM_H) {
                    if (s_fortress.dirt_grid[ty][tx] == 1) {
                        s_fortress.dirt_grid[ty][tx] = 0; // Escavou o bloco!
                        hal_audio_play_sound(SOUND_SWORD_SLASH, 0.85f, 1.6f);
                        if (link_rupees && (rand() % 3 == 0)) *link_rupees += 5;
                        printf("[MOLE MITTS] Link escavou a parede na fortaleza (%d, %d)!\n", tx, ty);
                    }
                }
            }

            // Baú central com chave pequena (x=120..136, y=64..80)
            if (!s_fortress.dig_chest_opened && fabsf(lx - 128.0f) < 18.0f && fabsf(ly - 72.0f) < 18.0f) {
                if (hal_input_is_pressed(KEY_A)) {
                    s_fortress.dig_chest_opened = true;
                    s_fortress.dig_key_obtained = true;
                    s_fortress.small_keys++;
                    hal_audio_play_sound(SOUND_CHEST_OPEN, 1.0f, 1.0f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                    printf("[FORTRESS] Bau aberto! Chave Pequena obtida! (Total: %d)\n", s_fortress.small_keys);
                }
            }

            // Passagem leste para Sala 2 (requer chave pequena se trancada)
            if (lx >= 240.0f && ly >= 64.0f && ly <= 96.0f && *link_dir == DIR_RIGHT) {
                if (s_fortress.small_keys > 0) {
                    s_fortress.small_keys--;
                    s_fortress.transitioning = true;
                    s_fortress.trans_timer = 20;
                    s_fortress.target_room = ROOM_FORTRESS_ARROW_PUZZLE;
                    s_fortress.target_spawn_x = 24.0f;
                    s_fortress.target_spawn_y = 80.0f;
                    hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
                }
            }
            break;
        }

        case ROOM_FORTRESS_ARROW_PUZZLE: {
            // Saída oeste de volta para Sala 1
            if (lx <= 16.0f && ly >= 64.0f && ly <= 96.0f && *link_dir == DIR_LEFT) {
                s_fortress.transitioning = true;
                s_fortress.trans_timer = 20;
                s_fortress.target_room = ROOM_FORTRESS_DIG_MAZE;
                s_fortress.target_spawn_x = 232.0f;
                s_fortress.target_spawn_y = 80.0f;
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.1f);
                return;
            }

            // Queda no abismo caso a ponte não esteja estendida
            if (!s_fortress.bridge_extended && ly >= 64.0f && ly <= 96.0f) {
                // Afunda/cai no abismo dos ventos! Retorna ao início seguro com dano
                if (link_hearts && *link_hearts > 1) (*link_hearts)--;
                *link_x = s_fortress.safe_x;
                *link_y = s_fortress.safe_y;
                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 0.7f);
                entity_trigger_screen_shake(8, 2);
                printf("[FORTRESS] Link caiu no abismo dos ventos! Retornado a ponto seguro.\n");
            } else {
                s_fortress.safe_x = lx;
                s_fortress.safe_y = ly;
            }

            // Passagem leste para Sala 3
            if (lx >= 240.0f && ly >= 32.0f && ly <= 64.0f && *link_dir == DIR_RIGHT) {
                s_fortress.transitioning = true;
                s_fortress.trans_timer = 20;
                s_fortress.target_room = ROOM_FORTRESS_MINISH_HOLE;
                s_fortress.target_spawn_x = 24.0f;
                s_fortress.target_spawn_y = 48.0f;
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.1f);
            }
            break;
        }

        case ROOM_FORTRESS_MINISH_HOLE: {
            // Saída oeste de volta para Sala 2
            if (lx <= 16.0f && ly >= 32.0f && ly <= 64.0f && *link_dir == DIR_LEFT) {
                s_fortress.transitioning = true;
                s_fortress.trans_timer = 20;
                s_fortress.target_room = ROOM_FORTRESS_ARROW_PUZZLE;
                s_fortress.target_spawn_x = 232.0f;
                s_fortress.target_spawn_y = 48.0f;
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.1f);
                return;
            }

            // Fissura Minish na parede norte (x=120..136, y=24..40)
            if (is_minish && lx >= 116.0f && lx <= 140.0f && ly <= 36.0f) {
                if (!s_fortress.minish_switch_pressed) {
                    s_fortress.minish_switch_pressed = true;
                    s_fortress.door_minish_open = true;
                    s_fortress.has_boss_key = true;
                    hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.2f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                    printf("[FORTRESS] Minish Link acionou o mecanismo interno! CHAVE DO MESTRE (BOSS KEY) OBTIDA!\n");
                }
            }

            // Passagem sul para Sala 4 (requer portão aberto)
            if (s_fortress.door_minish_open && ly >= 148.0f && lx >= 112.0f && lx <= 144.0f && *link_dir == DIR_DOWN) {
                s_fortress.transitioning = true;
                s_fortress.trans_timer = 20;
                s_fortress.target_room = ROOM_FORTRESS_BOSS_DOOR;
                s_fortress.target_spawn_x = 128.0f;
                s_fortress.target_spawn_y = 28.0f;
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.1f);
            }
            break;
        }

        case ROOM_FORTRESS_BOSS_DOOR: {
            // Saída norte de volta para Sala 3
            if (ly <= 16.0f && lx >= 112.0f && lx <= 144.0f && *link_dir == DIR_UP) {
                s_fortress.transitioning = true;
                s_fortress.trans_timer = 20;
                s_fortress.target_room = ROOM_FORTRESS_MINISH_HOLE;
                s_fortress.target_spawn_x = 128.0f;
                s_fortress.target_spawn_y = 136.0f;
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.1f);
                return;
            }

            // Grande Portão Alado do Chefe Mazaal ao sul
            if (!s_fortress.boss_door_unlocked && ly >= 130.0f && lx >= 112.0f && lx <= 144.0f) {
                if (s_fortress.has_boss_key) {
                    s_fortress.boss_door_unlocked = true;
                    hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.1f);
                    printf("[FORTRESS] Grande Portao Alado de Mazaal destrancado com a Boss Key!\n");
                }
            }

            if (s_fortress.boss_door_unlocked && ly >= 148.0f && lx >= 112.0f && lx <= 144.0f && *link_dir == DIR_DOWN) {
                s_fortress.transitioning = true;
                s_fortress.trans_timer = 25;
                s_fortress.target_room = ROOM_FORTRESS_BOSS_ARENA;
                s_fortress.target_spawn_x = 128.0f;
                s_fortress.target_spawn_y = 136.0f;
                hal_audio_play_bgm(BGM_BOSS_BATTLE);
                hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.8f);
                printf("[BOSS] Entrando na Arena de Combate contra MAZAAL!\n");
            }
            break;
        }

        case ROOM_FORTRESS_BOSS_ARENA: {
            MazaalHead* mh = &s_fortress.mazaal_head;
            MazaalHand* hl = &s_fortress.mazaal_hand_l;
            MazaalHand* hr = &s_fortress.mazaal_hand_r;

            if (!mh->defeated) {
                // Animação e física das mãos de Mazaal se não estiverem atordoadas
                if (hl->active && !hl->stunned) {
                    hl->x = hl->base_x + sinf((float)s_fort_anim_timer * 0.05f) * 16.0f;
                    hl->y = hl->base_y + cosf((float)s_fort_anim_timer * 0.04f) * 12.0f;

                    // Golpe de espada na palma aberta
                    if (is_attacking && fabsf(lx - hl->x) < 22.0f && fabsf(ly - hl->y) < 22.0f) {
                        hl->health--;
                        hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.2f);
                        entity_trigger_screen_shake(6, 2);
                        if (hl->health <= 0) {
                            hl->stunned = true;
                            hl->active = false;
                            hl->y = 110.0f;
                            hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.9f);
                            printf("[MAZAAL] Mao esquerda desativada!\n");
                        }
                    }
                }

                if (hr->active && !hr->stunned) {
                    hr->x = hr->base_x - sinf((float)s_fort_anim_timer * 0.05f) * 16.0f;
                    hr->y = hr->base_y + cosf((float)s_fort_anim_timer * 0.04f) * 12.0f;

                    if (is_attacking && fabsf(lx - hr->x) < 22.0f && fabsf(ly - hr->y) < 22.0f) {
                        hr->health--;
                        hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.2f);
                        entity_trigger_screen_shake(6, 2);
                        if (hr->health <= 0) {
                            hr->stunned = true;
                            hr->active = false;
                            hr->y = 110.0f;
                            hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.9f);
                            printf("[MAZAAL] Mao direita desativada!\n");
                        }
                    }
                }

                // Ambas as mãos abatidas: cabeça despenca atordoada!
                if (hl->stunned && hr->stunned && !mh->stunned) {
                    mh->stunned = true;
                    mh->mouth_open = true;
                    mh->y = 70.0f; // Cai ao chão
                    mh->stun_timer = 260; // Janela para Minish Link entrar
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.7f);
                    entity_trigger_screen_shake(14, 3);
                    printf("[MAZAAL] Cabeca desabou ao chao! Boca aberta para infiltracao Minish!\n");
                }

                if (mh->stunned) {
                    mh->stun_timer--;
                    // Infiltração Minish na boca de Mazaal para golpear o núcleo
                    if (is_minish && is_attacking && fabsf(lx - mh->x) < 24.0f && fabsf(ly - (mh->y + 4.0f)) < 20.0f) {
                        mh->core_health--;
                        hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.4f);
                        entity_trigger_screen_shake(8, 2);
                        printf("[MAZAAL] Golpe certeiro no nucleo interno! Vida restante: %d/12\n", mh->core_health);

                        if (mh->core_health <= 0) {
                            // VITÓRIA CONTRA MAZAAL!
                            mh->defeated = true;
                            mh->defeat_timer = 180;
                            hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                            entity_trigger_screen_shake(20, 4);

                            // Drops de Recompensa
                            s_fortress.heart_container_spawned = true;
                            s_fortress.heart_container_x = 90.0f;
                            s_fortress.heart_container_y = 80.0f;

                            s_fortress.ocarina_spawned = true;
                            s_fortress.ocarina_x = 128.0f;
                            s_fortress.ocarina_y = 80.0f;

                            s_fortress.warp_portal_spawned = true;
                            s_fortress.warp_portal_x = 166.0f;
                            s_fortress.warp_portal_y = 80.0f;

                            printf("[VICTORY] MAZAAL FOI DERROTADO! Heart Container e Ocarina do Vento liberados!\n");
                        }
                    }

                    if (mh->stun_timer <= 0 && !mh->defeated) {
                        // Mazaal acorda e reativa as mãos
                        mh->stunned = false;
                        mh->mouth_open = false;
                        mh->y = 38.0f;
                        hl->stunned = false;
                        hl->active = true;
                        hl->health = 3;
                        hr->stunned = false;
                        hr->active = true;
                        hr->health = 3;
                        hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 1.1f);
                        printf("[MAZAAL] Mazaal reativou suas maos e ergueu-se novamente!\n");
                    }
                }
            } else {
                // Chefe já derrotado: coleta dos prêmios
                if (s_fortress.heart_container_spawned && !s_fortress.heart_container_collected) {
                    if (fabsf(lx - s_fortress.heart_container_x) < 16.0f && fabsf(ly - s_fortress.heart_container_y) < 16.0f) {
                        s_fortress.heart_container_collected = true;
                        if (link_hearts) *link_hearts = 6;
                        hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.0f);
                        printf("[FORTRESS] Recipiente de Coracao obtido! Vida maxima aumentada!\n");
                    }
                }

                // Coleta da Ocarina of Wind!
                if (s_fortress.ocarina_spawned && !s_fortress.ocarina_collected) {
                    if (fabsf(lx - s_fortress.ocarina_x) < 18.0f && fabsf(ly - s_fortress.ocarina_y) < 18.0f) {
                        s_fortress.ocarina_collected = true;
                        inventory_unlock_item(INV_ITEM_OCARINA);
                        subweapon_set_current(ITEM_OCARINA_OF_WIND);
                        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                        printf("[FORTRESS] OCARINA OF WIND COLETADA! Ocarina do Vento equipada!\n");
                    }
                }

                // Portal de teletransporte de volta às ruínas
                if (s_fortress.warp_portal_spawned && out_warp_to_surface) {
                    if (fabsf(lx - s_fortress.warp_portal_x) < 14.0f && fabsf(ly - s_fortress.warp_portal_y) < 14.0f) {
                        *out_warp_to_surface = true;
                        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                        printf("[FORTRESS] Link pisou no portal de teletransporte para a superficie!\n");
                    }
                }
            }
            break;
        }

        default:
            break;
    }
}

void dungeon_fortress_render(const Camera* cam, bool is_minish, float link_x, float link_y) {
    if (!s_fortress.active) return;

    int ox = (cam ? (int)cam->x : 0);
    int oy = (cam ? (int)cam->y : 0);

    // 1. Renderiza o chão e paredes da sala ativa (16x10 tiles)
    for (int ty = 0; ty < FORTRESS_ROOM_H; ty++) {
        for (int tx = 0; tx < FORTRESS_ROOM_W; tx++) {
            int sx = tx * 16 - ox;
            int sy = ty * 16 - oy;

            TileType t = TILE_FORTRESS_FLOOR;
            if (tx == 0 || tx == FORTRESS_ROOM_W - 1 || ty == 0 || ty == FORTRESS_ROOM_H - 1) {
                t = TILE_FORTRESS_WALL;
            }

            // Portas e aberturas por sala
            if (s_fortress.current_room == ROOM_FORTRESS_ENTRANCE) {
                if (ty == FORTRESS_ROOM_H - 1 && tx >= 7 && tx <= 8) t = TILE_FORTRESS_FLOOR; // Saída sul
                if (ty == 0 && tx >= 7 && tx <= 8) {
                    t = s_fortress.door_entrance_open ? TILE_FORTRESS_FLOOR : TILE_FORTRESS_WALL;
                }
                if (ty == 0 && tx == 7) {
                    t = s_fortress.eye_entrance_hit ? TILE_FORTRESS_EYE_OPEN : TILE_FORTRESS_EYE_CLOSED;
                }
            } else if (s_fortress.current_room == ROOM_FORTRESS_DIG_MAZE) {
                if (ty == FORTRESS_ROOM_H - 1 && tx >= 7 && tx <= 8) t = TILE_FORTRESS_FLOOR; // Saída sul
                if (tx == FORTRESS_ROOM_W - 1 && (ty == 4 || ty == 5)) t = TILE_FORTRESS_FLOOR; // Porta leste
                // Blocos de terra escavável
                if (s_fortress.dirt_grid[ty][tx] == 1) {
                    t = TILE_DIRT_WALL;
                }
            } else if (s_fortress.current_room == ROOM_FORTRESS_ARROW_PUZZLE) {
                if (tx == 0 && (ty == 4 || ty == 5)) t = TILE_FORTRESS_FLOOR; // Entrada oeste
                if (tx == FORTRESS_ROOM_W - 1 && (ty == 2 || ty == 3)) t = TILE_FORTRESS_FLOOR; // Saída leste
                // Abismo e ponte
                if (ty >= 4 && ty <= 6) {
                    if (s_fortress.bridge_extended && tx >= 7 && tx <= 8) {
                        t = TILE_FORTRESS_FLOOR; // Ponte estendida
                    } else {
                        t = TILE_FORTRESS_PIT; // Abismo
                    }
                }
                // Alvos de olho na parede norte
                if (ty == 0 && tx == 3) t = s_fortress.arrow_eye_left_hit ? TILE_FORTRESS_EYE_OPEN : TILE_FORTRESS_EYE_CLOSED;
                if (ty == 0 && tx == 13) t = s_fortress.arrow_eye_right_hit ? TILE_FORTRESS_EYE_OPEN : TILE_FORTRESS_EYE_CLOSED;
            } else if (s_fortress.current_room == ROOM_FORTRESS_MINISH_HOLE) {
                if (tx == 0 && (ty == 2 || ty == 3)) t = TILE_FORTRESS_FLOOR; // Entrada oeste
                if (ty == FORTRESS_ROOM_H - 1 && tx >= 7 && tx <= 8) {
                    t = s_fortress.door_minish_open ? TILE_FORTRESS_FLOOR : TILE_FORTRESS_WALL;
                }
            } else if (s_fortress.current_room == ROOM_FORTRESS_BOSS_DOOR) {
                if (ty == 0 && tx >= 7 && tx <= 8) t = TILE_FORTRESS_FLOOR; // Entrada norte
                if (ty == FORTRESS_ROOM_H - 1 && (tx == 7 || tx == 8)) {
                    t = s_fortress.boss_door_unlocked ? TILE_FORTRESS_FLOOR : TILE_FORTRESS_BOSS_DOOR;
                }
            }

            render_metatile(sx, sy, t);
        }
    }

    // 2. Elementos interativos específicos da sala
    if (s_fortress.current_room == ROOM_FORTRESS_DIG_MAZE) {
        // Baú da Chave Pequena
        int bx = 120 - ox;
        int by = 64 - oy;
        render_metatile(bx, by, s_fortress.dig_chest_opened ? TILE_CHEST_OPEN : TILE_CHEST_CLOSED);
    } else if (s_fortress.current_room == ROOM_FORTRESS_MINISH_HOLE) {
        // Pedestal Minish na Sala 3
        int stx = 48 - ox;
        int sty = 120 - oy;
        draw_minish_pedestal(stx, sty);
        // Fenda Minish na parede norte
        int fx = 128 - ox;
        int fy = 12 - oy;
        draw_filled_rect(fx, fy, 4, 8, 0x050811FF);
    } else if (s_fortress.current_room == ROOM_FORTRESS_BOSS_ARENA) {
        // Pedestal Minish na Arena do Chefe
        int stx = 32 - ox;
        int sty = 120 - oy;
        draw_minish_pedestal(stx, sty);

        MazaalHead* mh = &s_fortress.mazaal_head;
        MazaalHand* hl = &s_fortress.mazaal_hand_l;
        MazaalHand* hr = &s_fortress.mazaal_hand_r;

        // Renderiza as Mãos Mecânicas de Mazaal (24x24 px cada)
        for (int i = 0; i < 2; i++) {
            MazaalHand* h = (i == 0) ? hl : hr;
            int hx = (int)h->x - ox - 12;
            int hy = (int)h->y - oy - 12;

            u32 brass = h->stunned ? 0x475569FF : 0xD97706FF;
            u32 slate = h->stunned ? 0x334155FF : 0x64748BFF;

            // Dedos articulados e palma
            draw_filled_rect(hx + 2, hy + 2, 20, 20, slate);
            draw_filled_rect(hx + 4, hy + 4, 16, 16, brass);

            // Olho sensor na palma da mão
            if (!h->stunned) {
                draw_filled_rect(hx + 8, hy + 8, 8, 8, 0x0284C7FF); // Olho azul
                draw_filled_rect(hx + 10, hy + 10, 4, 4, 0xFFFFFFFF);
            } else {
                draw_filled_rect(hx + 8, hy + 8, 8, 8, 0x0F172AFF); // Olho apagado
            }
        }

        // Renderiza a Cabeça Colossal de Mazaal (48x36 px)
        if (!mh->defeated) {
            int mx = (int)mh->x - ox - 24;
            int my = (int)mh->y - oy - 18;

            u32 head_col = mh->stunned ? 0x475569FF : 0x64748BFF;
            u32 brass_col = mh->stunned ? 0x78350FFF : 0xD97706FF;

            // Crânio de pedra com chifres e glifos
            draw_filled_rect(mx, my, 48, 36, head_col);
            draw_filled_rect(mx + 4, my - 4, 8, 6, brass_col); // Chifre esquerdo
            draw_filled_rect(mx + 36, my - 4, 8, 6, brass_col); // Chifre direito

            // Faixas douradas
            draw_filled_rect(mx + 6, my + 6, 36, 4, brass_col);

            // Olhos vermelhos brilhantes
            if (!mh->stunned) {
                draw_filled_rect(mx + 10, my + 14, 8, 6, 0xEF4444FF);
                draw_filled_rect(mx + 30, my + 14, 8, 6, 0xEF4444FF);
                draw_filled_rect(mx + 13, my + 15, 2, 4, 0xFFFFFFFF);
                draw_filled_rect(mx + 33, my + 15, 2, 4, 0xFFFFFFFF);
            } else {
                draw_filled_rect(mx + 10, my + 14, 8, 6, 0x1E293BFF); // Apagados
                draw_filled_rect(mx + 30, my + 14, 8, 6, 0x1E293BFF);
            }

            // Boca: fechada se ativo, escancarada se atordoado!
            if (!mh->mouth_open) {
                draw_filled_rect(mx + 14, my + 26, 20, 4, 0x0F172AFF);
            } else {
                draw_filled_rect(mx + 12, my + 24, 24, 10, 0x050811FF); // Entrada escura da boca
                // Núcleo vermelho/ciano exposto no fundo da garganta!
                u32 core_c = ((s_fort_anim_timer / 4) % 2 == 0) ? 0xEF4444FF : 0x38BDF8FF;
                draw_filled_rect(mx + 20, my + 26, 8, 6, core_c);
                draw_filled_rect(mx + 22, my + 27, 4, 4, 0xFFFFFFFF);
            }
        }

        // Drops após a derrota de Mazaal
        if (s_fortress.heart_container_spawned && !s_fortress.heart_container_collected) {
            int hcx = (int)s_fortress.heart_container_x - ox;
            int hcy = (int)s_fortress.heart_container_y - oy;
            draw_filled_rect(hcx + 1, hcy, 4, 2, 0xEF4444FF);
            draw_filled_rect(hcx + 7, hcy, 4, 2, 0xEF4444FF);
            draw_filled_rect(hcx, hcy + 2, 12, 6, 0xEF4444FF);
            draw_filled_rect(hcx + 2, hcy + 8, 8, 3, 0xEF4444FF);
            draw_filled_rect(hcx + 4, hcy + 11, 4, 2, 0xEF4444FF);
            draw_filled_rect(hcx + 2, hcy + 2, 2, 2, 0xFFFFFFFF); // Brilho
        }

        // Pedestal e OCARINA OF WIND!
        if (s_fortress.ocarina_spawned && !s_fortress.ocarina_collected) {
            int ocx = (int)s_fortress.ocarina_x - ox;
            int ocy = (int)s_fortress.ocarina_y - oy;

            // Pedestal de pedra
            draw_filled_rect(ocx - 4, ocy + 6, 16, 8, 0x64748BFF);
            draw_filled_rect(ocx - 6, ocy + 12, 20, 4, 0x334155FF);

            // Ocarina azul do Vento flutuando sobre o pedestal
            int float_y = (int)(sinf((float)s_fort_anim_timer * 0.1f) * 3.0f);
            int oy_item = ocy + float_y;

            draw_filled_rect(ocx - 2, oy_item + 1, 3, 2, 0x38BDF8FF); // Bocal
            draw_filled_rect(ocx, oy_item, 8, 4, 0x0284C7FF); // Corpo azul celeste
            draw_filled_rect(ocx + 2, oy_item + 1, 2, 2, 0x0F172AFF); // Furos
            draw_filled_rect(ocx + 5, oy_item + 1, 2, 2, 0x0F172AFF);
            draw_filled_rect(ocx + 1, oy_item, 2, 1, 0xFFFFFFFF); // Reflexo sagrado
        }

        // Portal azul de retorno à superfície
        if (s_fortress.warp_portal_spawned) {
            int px = (int)s_fortress.warp_portal_x - ox;
            int py = (int)s_fortress.warp_portal_y - oy;
            draw_filled_rect(px - 6, py - 4, 16, 12, 0x0369A1AA);
            draw_filled_rect(px - 4, py - 2, 12, 8, 0x38BDF8FF);
            draw_filled_rect(px - 1, py, 6, 4, 0xFFFFFFFF);
        }
    }
}

void dungeon_fortress_render_hud_keys(int x, int y) {
    if (!s_fortress.active) return;

    // Ícone da Chave Pequena
    draw_filled_rect(x, y + 2, 2, 5, 0xFBBF24FF);
    draw_filled_rect(x + 2, y + 1, 4, 3, 0xFBBF24FF);
    draw_filled_rect(x + 3, y + 2, 2, 1, 0x1E293BFF);
    draw_filled_rect(x + 6, y + 2, 4, 1, 0xFBBF24FF);
    draw_filled_rect(x + 8, y + 3, 1, 2, 0xFBBF24FF);

    char txt[8];
    snprintf(txt, sizeof(txt), "x%d", s_fortress.small_keys);
    font_draw_text(x + 12, y + 1, txt, 0xFDE047FF, true);

    // Chave do Chefe (Boss Key) se possuir
    if (s_fortress.has_boss_key) {
        int bx = x + 30;
        draw_filled_rect(bx, y + 1, 8, 7, 0xF59E0BFF);
        draw_filled_rect(bx + 2, y + 3, 4, 3, 0xDC2626FF); // Olho rubi da chave mestre
        draw_filled_rect(bx + 8, y + 3, 4, 2, 0xF59E0BFF);
    }
}
