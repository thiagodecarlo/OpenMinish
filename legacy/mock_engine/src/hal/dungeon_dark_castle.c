/*
 * ============================================================================
 * src/hal/dungeon_dark_castle.c - Masmorra Final: Dark Hyrule Castle (Ato V)
 * ============================================================================
 * Implementação clean-room C11 da Masmorra Final de The Legend of Zelda:
 * The Minish Cap.
 *
 * Arquitetura de renderização direta no framebuffer SDL2 (HAL Video),
 * sem dependência de ROM comercial (Zero-ROM), alocação estática no loop 60 FPS,
 * e suporte integral à coordenação dos 4 Clones da Four Sword e mecânicas dos
 * Sinos de Petrificação.
 */

#include "hal/dungeon_dark_castle.h"
#include "hal/video.h"
#include "hal/audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TILE_SIZE 16
#define DHC_SCREEN_W 256
#define DHC_SCREEN_H 160

// Paleta de Cores de Dark Hyrule Castle
#define C_DHC_WALL          0x181824FF // Paredes de pedra de obsidiana púrpura
#define C_DHC_WALL_BORDER   0x0F0E17FF // Contorno de cantaria gótica
#define C_DHC_FLOOR_A       0x2E2A4FFF // Piso ladrilhado púrpura escuro
#define C_DHC_FLOOR_B       0x252240FF // Piso ladrilhado púrpura intermediário
#define C_DHC_CARPET        0x881337FF // Tapete real carmesim escuro
#define C_DHC_CARPET_GOLD   0xD97706FF // Bordas douradas do tapete real
#define C_DHC_MALICE_DARK   0x4C0519FF // Malícia e trevas de Vaati
#define C_DHC_MALICE_PULSE  0xE11D48FF // Pulsar avermelhado de malícia
#define C_DHC_HARD_LIGHT    0x38BDF8EE // Ponte de luz sólida estendida
#define C_DHC_BELL_BRONZE   0xB45309FF // Sino ancestral de bronze
#define C_DHC_BELL_SHADOW   0x78350FFF // Sombra do sino
#define C_DHC_DARKNUT_ARMOR 0x18181BFF // Armadura de ferro negro do Darknut
#define C_DHC_DARKNUT_GOLD  0xF59E0BFF // Detalhes dourados e elmo com chifres
#define C_DHC_DARKNUT_CAPE  0x6B21A8FF // Capa púrpura de malícia
#define C_DHC_FIRE_CORE     0xFEF08AFF // Núcleo incandescente de fogo
#define C_DHC_FIRE_OUTER    0xEA580CFF // Labareda externa de fogo

static DarkHyruleCastle s_dhc = { 0 };

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

static void draw_filled_rect(int x, int y, int w, int h, u32 color) {
    int x1 = (x < 0) ? 0 : x;
    int y1 = (y < 0) ? 0 : y;
    int x2 = (x + w > DHC_SCREEN_W) ? DHC_SCREEN_W : (x + w);
    int y2 = (y + h > DHC_SCREEN_H) ? DHC_SCREEN_H : (y + h);

    for (int py = y1; py < y2; py++) {
        for (int px = x1; px < x2; px++) {
            u32 current = hal_video_get_pixel(px, py);
            hal_video_put_pixel(px, py, blend_colors(current, color));
        }
    }
}

// Cria os mapas das salas de Dark Hyrule Castle
static Tilemap* create_dhc_room_map(DarkCastleRoomId room) {
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width = DHC_ROOM_W;
    m->height = DHC_ROOM_H;
    int total = DHC_ROOM_W * DHC_ROOM_H;

    m->ground_layer = (u8*)malloc(total);
    m->overlay_layer = (u8*)malloc(total);
    m->collision_map = (u8*)malloc(total);
    m->is_authentic = false;

    memset(m->ground_layer, TILE_MELARI_STONE_FLOOR, total);
    memset(m->overlay_layer, 0, total);
    memset(m->collision_map, 0, total);

    // Paredes perimetrais sólidas
    for (int y = 0; y < DHC_ROOM_H; y++) {
        for (int x = 0; x < DHC_ROOM_W; x++) {
            int idx = y * DHC_ROOM_W + x;
            if (y == 0 || y == DHC_ROOM_H - 1 || x == 0 || x == DHC_ROOM_W - 1) {
                m->collision_map[idx] = 1;
            }
        }
    }

    // Aberturas de portas canônicas
    int mid_x = DHC_ROOM_W / 2;
    int mid_y = DHC_ROOM_H / 2;

    switch (room) {
        case DHC_ROOM_ENTRANCE_FOYER:
            m->collision_map[mid_y * DHC_ROOM_W + 0] = 0; // Porta Oeste
            m->collision_map[mid_y * DHC_ROOM_W + (DHC_ROOM_W - 1)] = 0; // Porta Leste
            m->collision_map[0 * DHC_ROOM_W + mid_x] = 0; // Porta Norte (bloqueada por malícia)
            break;

        case DHC_ROOM_WEST_CHASM_PUZZLE:
            m->collision_map[mid_y * DHC_ROOM_W + (DHC_ROOM_W - 1)] = 0; // Volta para o leste
            // Abismo central (linhas 4..5)
            for (int y = 4; y <= 5; y++) {
                for (int x = 2; x < DHC_ROOM_W - 2; x++) {
                    m->collision_map[y * DHC_ROOM_W + x] = 1;
                }
            }
            break;

        case DHC_ROOM_EAST_MONOLITH_PUZZLE:
            m->collision_map[mid_y * DHC_ROOM_W + 0] = 0; // Volta para o oeste
            m->collision_map[0 * DHC_ROOM_W + mid_x] = 0; // Norte para o Darknut
            break;

        case DHC_ROOM_DARKNUT_MINIBOSS:
            m->collision_map[(DHC_ROOM_H - 1) * DHC_ROOM_W + mid_x] = 0; // Sul de volta
            m->collision_map[0 * DHC_ROOM_W + mid_x] = 0; // Norte para a Torre dos Sinos
            break;

        case DHC_ROOM_BELL_TOWER:
            m->collision_map[(DHC_ROOM_H - 1) * DHC_ROOM_W + mid_x] = 0; // Sul de volta
            m->collision_map[0 * DHC_ROOM_W + mid_x] = 0; // Norte para o Sanctum
            break;

        case DHC_ROOM_THRONE_SANCTUM_GATE:
            m->collision_map[(DHC_ROOM_H - 1) * DHC_ROOM_W + mid_x] = 0; // Sul de volta
            m->collision_map[0 * DHC_ROOM_W + mid_x] = 1; // Portão do Sanctum (abre com Boss Key)
            break;

        default:
            break;
    }

    return m;
}

void dark_castle_init(void) {
    memset(&s_dhc, 0, sizeof(DarkHyruleCastle));
    s_dhc.is_active = false;
    s_dhc.current_room = DHC_ROOM_ENTRANCE_FOYER;
    s_dhc.small_keys = 0;
    s_dhc.has_sanctum_key = false;

    // Configurações da Sala 0
    s_dhc.foyer_split_pads_active = true;
    s_dhc.malice_barrier_open = false;
    s_dhc.malice_pulse_timer = 0;

    // Configurações da Sala 1 (Ala Oeste)
    s_dhc.hard_light_bridge_active = false;
    s_dhc.west_key_chest_opened = false;

    // Configurações da Sala 2 (Ala Leste)
    s_dhc.monolith_x = 7.0f * TILE_SIZE;
    s_dhc.monolith_y = 4.0f * TILE_SIZE;
    s_dhc.monolith_slid = false;
    s_dhc.east_door_unlocked = false;

    // Configurações da Sala 3 (Darknut)
    s_dhc.darknut.x = 8.0f * TILE_SIZE;
    s_dhc.darknut.y = 3.5f * TILE_SIZE;
    s_dhc.darknut.hp = 16;
    s_dhc.darknut.max_hp = 16;
    s_dhc.darknut.state = DARKNUT_STATE_PURSUE;
    s_dhc.darknut.facing = DIR_DOWN;
    s_dhc.darknut.is_active = true;
    s_dhc.darknut.shield_raised = true;

    // Configurações da Sala 4 (Torre dos Sinos)
    s_dhc.bells_struck[0] = false;
    s_dhc.bells_struck[1] = false;
    s_dhc.bells_struck[2] = false;
    s_dhc.petrification_stage = 0;
    s_dhc.sanctum_stairs_open = false;

    // Configurações da Sala 5 (Sanctum)
    s_dhc.four_elements_charged = false;
    s_dhc.sanctum_gate_opened = false;
    s_dhc.ready_for_vaati_climax = false;

    // Inicialização dos mapas
    for (int i = 0; i < DHC_ROOM_COUNT; i++) {
        s_dhc.room_maps[i] = create_dhc_room_map((DarkCastleRoomId)i);
    }
}

void dark_castle_enter(float* link_x, float* link_y, Direction* link_dir) {
    if (!s_dhc.room_maps[0]) {
        dark_castle_init();
    }
    s_dhc.is_active = true;
    s_dhc.current_room = DHC_ROOM_ENTRANCE_FOYER;

    if (link_x) *link_x = 8.0f * TILE_SIZE;
    if (link_y) *link_y = 8.0f * TILE_SIZE;
    if (link_dir) *link_dir = DIR_UP;

    hal_audio_play_bgm(BGM_DARK_HYRULE_CASTLE);
    hal_audio_play_sound(SOUND_SECRET, 1.0f, 0.85f);
    hal_audio_play_sound(SOUND_TOWN_BELL, 0.8f, 0.6f);
    printf("[DARK HYRULE CASTLE] Link penetrou o Castelo Corrompido de Hyrule! (Ato V)\n");
}

void dark_castle_exit(float* link_x, float* link_y, Direction* link_dir) {
    s_dhc.is_active = false;
    if (link_x) *link_x = 8.0f * TILE_SIZE;
    if (link_y) *link_y = 8.5f * TILE_SIZE;
    if (link_dir) *link_dir = DIR_DOWN;
    printf("[DARK HYRULE CASTLE] Link saiu do Castelo Sombrio.\n");
}

bool dark_castle_is_active(void) {
    return s_dhc.is_active;
}

DarkCastleRoomId dark_castle_get_room(void) {
    return s_dhc.current_room;
}

void dark_castle_set_room(DarkCastleRoomId room, float* link_x, float* link_y, Direction* link_dir) {
    if (room < 0 || room >= DHC_ROOM_COUNT) return;
    s_dhc.current_room = room;

    if (link_x && link_y) {
        switch (room) {
            case DHC_ROOM_ENTRANCE_FOYER:
                *link_x = 8.0f * TILE_SIZE;
                *link_y = 8.0f * TILE_SIZE;
                break;
            case DHC_ROOM_WEST_CHASM_PUZZLE:
                *link_x = 13.5f * TILE_SIZE;
                *link_y = 5.0f * TILE_SIZE;
                break;
            case DHC_ROOM_EAST_MONOLITH_PUZZLE:
                *link_x = 2.0f * TILE_SIZE;
                *link_y = 5.0f * TILE_SIZE;
                break;
            case DHC_ROOM_DARKNUT_MINIBOSS:
                *link_x = 8.0f * TILE_SIZE;
                *link_y = 8.0f * TILE_SIZE;
                break;
            case DHC_ROOM_BELL_TOWER:
                *link_x = 8.0f * TILE_SIZE;
                *link_y = 8.0f * TILE_SIZE;
                break;
            case DHC_ROOM_THRONE_SANCTUM_GATE:
                *link_x = 8.0f * TILE_SIZE;
                *link_y = 8.0f * TILE_SIZE;
                break;
            default:
                break;
        }
    }
    if (link_dir) *link_dir = DIR_UP;
    printf("[DARK HYRULE CASTLE] Transicao para Sala %d!\n", room);
}

Tilemap* dark_castle_get_current_map(void) {
    return s_dhc.room_maps[s_dhc.current_room];
}

void dark_castle_update(float* link_x, float* link_y, Direction* link_dir,
                        bool is_attacking, bool has_four_sword, int clone_count,
                        const float clone_x[3], const float clone_y[3],
                        int* player_hearts, int* player_keys) {
    if (!s_dhc.is_active || !link_x || !link_y) return;

    float lx = *link_x;
    float ly = *link_y;

    s_dhc.malice_pulse_timer = (s_dhc.malice_pulse_timer + 1) % 60;
    if (s_dhc.screen_shake_timer > 0) s_dhc.screen_shake_timer--;

    // Badalar periódico dos Sinos de Petrificação (tensão dramática)
    s_dhc.bell_toll_timer++;
    if (s_dhc.bell_toll_timer >= 900) { // a cada 15 segundos
        s_dhc.bell_toll_timer = 0;
        if (!s_dhc.bells_struck[0] || !s_dhc.bells_struck[1] || !s_dhc.bells_struck[2]) {
            hal_audio_play_sound(SOUND_TOWN_BELL, 0.9f, 0.55f);
            s_dhc.screen_shake_timer = 20;
            printf("[DARK HYRULE CASTLE] O Sino da Petrificacao ressoa sombriamente...\n");
        }
    }

    switch (s_dhc.current_room) {
        case DHC_ROOM_ENTRANCE_FOYER: {
            // Se Link passar para a esquerda (Oeste)
            if (lx <= 0.8f * TILE_SIZE && fabsf(ly - 5.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                dark_castle_set_room(DHC_ROOM_WEST_CHASM_PUZZLE, link_x, link_y, link_dir);
                return;
            }
            // Se Link passar para a direita (Leste)
            if (lx >= 14.2f * TILE_SIZE && fabsf(ly - 5.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                dark_castle_set_room(DHC_ROOM_EAST_MONOLITH_PUZZLE, link_x, link_y, link_dir);
                return;
            }
            // Se Link tentar avançar para o norte (passagem para Torre/Trono)
            if (ly <= 1.2f * TILE_SIZE && fabsf(lx - 8.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                if (s_dhc.malice_barrier_open) {
                    dark_castle_set_room(DHC_ROOM_BELL_TOWER, link_x, link_y, link_dir);
                    return;
                } else {
                    // Empurra Link de volta com choque de malícia
                    *link_y = 2.0f * TILE_SIZE;
                    hal_audio_play_sound(SOUND_BOSS_HIT, 0.8f, 1.4f);
                    printf("[MALICE] A Barreira Central de Malicia repele o heroi!\n");
                }
            }

            // Acendimento de tochas com a Flame Lantern ou ataques
            for (int i = 0; i < MAX_DHC_TORCHES; i++) {
                float tx = (i % 2 == 0) ? 3.0f * TILE_SIZE : 12.0f * TILE_SIZE;
                float ty = (i < 2) ? 3.0f * TILE_SIZE : 7.0f * TILE_SIZE;
                float dx = lx - tx;
                float dy = ly - ty;
                if (!s_dhc.foyer_torches_lit[i] && dx * dx + dy * dy < 24.0f * 24.0f) {
                    s_dhc.foyer_torches_lit[i] = true;
                    hal_audio_play_sound(SOUND_FIRE, 1.0f, 1.0f);
                    printf("[DARK HYRULE CASTLE] Tocha ancestral %d acesa!\n", i + 1);
                }
            }

            // Verificação de abertura da barreira de malícia:
            // Todas as 4 tochas acesas + 4 Clones ou avanço das chaves
            bool all_torches = true;
            for (int i = 0; i < MAX_DHC_TORCHES; i++) {
                if (!s_dhc.foyer_torches_lit[i]) all_torches = false;
            }
            if (all_torches && (clone_count >= 3 || s_dhc.small_keys >= 1) && !s_dhc.malice_barrier_open) {
                s_dhc.malice_barrier_open = true;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                printf("[DARK HYRULE CASTLE] A Barreira Central de Malicia foi dissolvida!\n");
            }
            break;
        }

        case DHC_ROOM_WEST_CHASM_PUZZLE: {
            // Retorno para a Sala 0
            if (lx >= 14.5f * TILE_SIZE && fabsf(ly - 5.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                dark_castle_set_room(DHC_ROOM_ENTRANCE_FOYER, link_x, link_y, link_dir);
                *link_x = 1.8f * TILE_SIZE;
                return;
            }

            // Janela de acerto sincronizado dos 4 Olhos
            if (s_dhc.eye_hit_window > 0) {
                s_dhc.eye_hit_window--;
                if (s_dhc.eye_hit_window == 0 && !s_dhc.hard_light_bridge_active) {
                    // Reset dos olhos se não acertou todos dentro do prazo
                    bool all_eyes = true;
                    for (int i = 0; i < 4; i++) {
                        if (!s_dhc.eye_switches_hit[i]) all_eyes = false;
                    }
                    if (!all_eyes) {
                        for (int i = 0; i < 4; i++) s_dhc.eye_switches_hit[i] = false;
                        hal_audio_play_sound(SOUND_KEESE_CHIRP, 0.7f, 0.7f);
                    }
                }
            }

            // Atualiza colisão da ponte de luz
            Tilemap* map = s_dhc.room_maps[DHC_ROOM_WEST_CHASM_PUZZLE];
            if (s_dhc.hard_light_bridge_active && map) {
                for (int y = 4; y <= 5; y++) {
                    for (int x = 6; x <= 9; x++) {
                        map->collision_map[y * DHC_ROOM_W + x] = 0; // Ponte transitável!
                    }
                }
            }

            // Abertura do baú da chave pequena do Oeste
            if (s_dhc.hard_light_bridge_active && !s_dhc.west_key_chest_opened) {
                float chest_x = 8.0f * TILE_SIZE;
                float chest_y = 2.0f * TILE_SIZE;
                float cdx = lx - chest_x;
                float cdy = ly - chest_y;
                if (cdx * cdx + cdy * cdy < 20.0f * 20.0f) {
                    s_dhc.west_key_chest_opened = true;
                    s_dhc.small_keys++;
                    if (player_keys) (*player_keys)++;
                    hal_audio_play_sound(SOUND_CHEST_OPEN, 1.0f, 1.0f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                    printf("[DARK HYRULE CASTLE] Link obteve uma Pequena Chave da Ala Oeste!\n");
                }
            }
            break;
        }

        case DHC_ROOM_EAST_MONOLITH_PUZZLE: {
            // Retorno para Sala 0
            if (lx <= 0.8f * TILE_SIZE && fabsf(ly - 5.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                dark_castle_set_room(DHC_ROOM_ENTRANCE_FOYER, link_x, link_y, link_dir);
                *link_x = 13.5f * TILE_SIZE;
                return;
            }

            // Passagem Norte para a Sala do Darknut
            if (ly <= 1.2f * TILE_SIZE && fabsf(lx - 8.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                if (s_dhc.monolith_slid || s_dhc.east_door_unlocked) {
                    dark_castle_set_room(DHC_ROOM_DARKNUT_MINIBOSS, link_x, link_y, link_dir);
                    return;
                }
            }

            // Canhões de bola de fogo disparando
            s_dhc.cannon_cooldown++;
            if (s_dhc.cannon_cooldown >= 90) {
                s_dhc.cannon_cooldown = 0;
                for (int i = 0; i < MAX_DHC_FIREBALLS; i++) {
                    if (!s_dhc.fireballs[i].active) {
                        s_dhc.fireballs[i].active = true;
                        s_dhc.fireballs[i].x = 14.0f * TILE_SIZE;
                        s_dhc.fireballs[i].y = (i % 2 == 0) ? 3.5f * TILE_SIZE : 6.5f * TILE_SIZE;
                        s_dhc.fireballs[i].vx = -2.5f;
                        s_dhc.fireballs[i].vy = 0.0f;
                        s_dhc.fireballs[i].lifetime = 120;
                        hal_audio_play_sound(SOUND_GUST_BLAST, 0.7f, 1.2f);
                        break;
                    }
                }
            }

            // Atualiza bolas de fogo
            for (int i = 0; i < MAX_DHC_FIREBALLS; i++) {
                if (s_dhc.fireballs[i].active) {
                    s_dhc.fireballs[i].x += s_dhc.fireballs[i].vx;
                    s_dhc.fireballs[i].lifetime--;
                    if (s_dhc.fireballs[i].lifetime <= 0 || s_dhc.fireballs[i].x <= 1.0f * TILE_SIZE) {
                        s_dhc.fireballs[i].active = false;
                    }
                    // Colisão com Link
                    float fdx = lx - s_dhc.fireballs[i].x;
                    float fdy = ly - s_dhc.fireballs[i].y;
                    if (fdx * fdx + fdy * fdy < 12.0f * 12.0f) {
                        s_dhc.fireballs[i].active = false;
                        if (player_hearts && *player_hearts > 0) {
                            (*player_hearts)--;
                            hal_audio_play_sound(SOUND_BOSS_HIT, 0.9f, 1.2f);
                        }
                    }
                }
            }

            // Empurrar o monólito pesado com 4 Clones
            if (!s_dhc.monolith_slid) {
                float mdx = lx - s_dhc.monolith_x;
                float mdy = ly - s_dhc.monolith_y;
                if (fabsf(mdx) < 24.0f && fabsf(mdy) < 24.0f) {
                    if (clone_count >= 3 && ly > s_dhc.monolith_y && (link_dir ? *link_dir == DIR_UP : true)) {
                        s_dhc.monolith_y -= 0.5f;
                        if (s_dhc.monolith_y <= 2.5f * TILE_SIZE) {
                            s_dhc.monolith_slid = true;
                            s_dhc.east_door_unlocked = true;
                            hal_audio_play_sound(SOUND_BLOCK_PUSH, 1.0f, 0.8f);
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.25f);
                            printf("[DARK HYRULE CASTLE] Monolito de Obsidiana empurrado com forca dos 4 Clones!\n");
                        }
                    }
                }
            }
            break;
        }

        case DHC_ROOM_DARKNUT_MINIBOSS: {
            // Retorno para Sala 2
            if (ly >= 9.2f * TILE_SIZE && fabsf(lx - 8.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                dark_castle_set_room(DHC_ROOM_EAST_MONOLITH_PUZZLE, link_x, link_y, link_dir);
                *link_y = 1.8f * TILE_SIZE;
                return;
            }

            // Avanço para Torre dos Sinos se Darknut foi derrotado
            if (ly <= 1.2f * TILE_SIZE && fabsf(lx - 8.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                if (s_dhc.darknut.defeated) {
                    dark_castle_set_room(DHC_ROOM_BELL_TOWER, link_x, link_y, link_dir);
                    return;
                }
            }

            // Lógica e IA do Darknut
            DarknutBoss* dk = &s_dhc.darknut;
            if (dk->is_active && !dk->defeated) {
                if (dk->hit_stun > 0) dk->hit_stun--;

                float dkx = lx - dk->x;
                float dky = ly - dk->y;
                float dist = sqrtf(dkx * dkx + dky * dky);

                // Determina direção que o Darknut encara
                if (fabsf(dkx) > fabsf(dky)) {
                    dk->facing = (dkx > 0) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    dk->facing = (dky > 0) ? DIR_DOWN : DIR_UP;
                }

                dk->action_timer++;
                if (dk->action_timer >= 60) {
                    dk->action_timer = 0;
                    if (dist < 40.0f) {
                        dk->state = DARKNUT_STATE_ATTACK;
                        hal_audio_play_sound(SOUND_SWORD_SLASH, 1.0f, 0.7f);
                    } else {
                        dk->state = DARKNUT_STATE_PURSUE;
                    }
                }

                if (dk->state == DARKNUT_STATE_PURSUE && dist > 20.0f && dk->hit_stun == 0) {
                    dk->vx = (dkx / dist) * 0.75f;
                    dk->vy = (dky / dist) * 0.75f;
                    dk->x += dk->vx;
                    dk->y += dk->vy;
                }

                // Dano em Link se colidir
                if (dist < 16.0f && dk->state == DARKNUT_STATE_ATTACK) {
                    if (player_hearts && *player_hearts > 0) {
                        (*player_hearts)--;
                        hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 0.8f);
                    }
                }
            }

            // Coleta da Sanctum Boss Key caída
            if (s_dhc.sanctum_key_spawned && !s_dhc.sanctum_key_collected) {
                float kx = 8.0f * TILE_SIZE;
                float ky = 5.0f * TILE_SIZE;
                float kdx = lx - kx;
                float kdy = ly - ky;
                if (kdx * kdx + kdy * kdy < 20.0f * 20.0f) {
                    s_dhc.sanctum_key_collected = true;
                    s_dhc.has_sanctum_key = true;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                    printf("[DARK HYRULE CASTLE] Link obteve a SANCTUM BOSS KEY!\n");
                }
            }
            break;
        }

        case DHC_ROOM_BELL_TOWER: {
            // Retorno para Sala 0
            if (ly >= 9.2f * TILE_SIZE && fabsf(lx - 8.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                dark_castle_set_room(DHC_ROOM_ENTRANCE_FOYER, link_x, link_y, link_dir);
                *link_y = 1.8f * TILE_SIZE;
                return;
            }

            // Avanço para o Trono / Sanctum
            if (ly <= 1.2f * TILE_SIZE && fabsf(lx - 8.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                if (s_dhc.sanctum_stairs_open) {
                    dark_castle_set_room(DHC_ROOM_THRONE_SANCTUM_GATE, link_x, link_y, link_dir);
                    return;
                }
            }

            // Verificação dos 3 Sinos
            bool all_bells = true;
            for (int i = 0; i < MAX_DHC_BELLS; i++) {
                if (!s_dhc.bells_struck[i]) all_bells = false;
            }
            if (all_bells && !s_dhc.sanctum_stairs_open) {
                s_dhc.sanctum_stairs_open = true;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                printf("[DARK HYRULE CASTLE] Os 3 Sinos da Petrificacao foram silenciados! Escadaria aberta!\n");
            }
            break;
        }

        case DHC_ROOM_THRONE_SANCTUM_GATE: {
            // Retorno para Torre dos Sinos
            if (ly >= 9.2f * TILE_SIZE && fabsf(lx - 8.0f * TILE_SIZE) < 1.5f * TILE_SIZE) {
                dark_castle_set_room(DHC_ROOM_BELL_TOWER, link_x, link_y, link_dir);
                *link_y = 1.8f * TILE_SIZE;
                return;
            }

            // Carregamento dos 4 Elementos no altar central
            float ax = 8.0f * TILE_SIZE;
            float ay = 5.0f * TILE_SIZE;
            float adx = lx - ax;
            float ady = ly - ay;
            if (adx * adx + ady * ady < 24.0f * 24.0f) {
                if (!s_dhc.four_elements_charged && (has_four_sword || clone_count >= 3)) {
                    s_dhc.four_elements_charged = true;
                    s_dhc.charge_anim_timer = 120;
                    hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.0f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                    printf("[FOUR SWORD] O poder sagrado dos 4 Elementos irradia pelo altar real!\n");
                }
            }

            if (s_dhc.charge_anim_timer > 0) {
                s_dhc.charge_anim_timer--;
                if (s_dhc.charge_anim_timer == 0 && s_dhc.has_sanctum_key) {
                    s_dhc.sanctum_gate_opened = true;
                    s_dhc.ready_for_vaati_climax = true;
                    hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                    printf("[DARK HYRULE CASTLE] O Grande Portao do Santuario foi deslacrado! Vaati aguarda no topo!\n");
                }
            }
            break;
        }

        default:
            break;
    }
}

void dark_castle_strike_sword(float link_x, float link_y, Direction link_dir, int damage,
                              bool is_spin, int clone_count,
                              const float clone_x[3], const float clone_y[3]) {
    if (!s_dhc.is_active) return;

    // Combate na Sala 3 contra Darknut
    if (s_dhc.current_room == DHC_ROOM_DARKNUT_MINIBOSS) {
        DarknutBoss* dk = &s_dhc.darknut;
        if (dk->is_active && !dk->defeated) {
            float dx = dk->x - link_x;
            float dy = dk->y - link_y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist < 32.0f) {
                // Se golpe acertar pelas costas ou em Spin Attack ou se clones cercarem
                bool can_hit = is_spin || (clone_count >= 2);
                if (!can_hit) {
                    // Verifica se Link está golpeando por trás da direção do Darknut
                    if (dk->facing == DIR_DOWN && link_y < dk->y) can_hit = true;
                    else if (dk->facing == DIR_UP && link_y > dk->y) can_hit = true;
                    else if (dk->facing == DIR_RIGHT && link_x < dk->x) can_hit = true;
                    else if (dk->facing == DIR_LEFT && link_x > dk->x) can_hit = true;
                }

                if (can_hit) {
                    dk->hp -= damage;
                    dk->hit_stun = 15;
                    hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.0f);
                    printf("[DARKNUT] Acerto critico! HP restante: %d/%d\n", dk->hp, dk->max_hp);

                    if (dk->hp <= 0) {
                        dk->defeated = true;
                        s_dhc.sanctum_key_spawned = true;
                        hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.25f);
                        printf("[DARKNUT] O Cavaleiro Negro foi derrotado! A Sanctum Boss Key apareceu!\n");
                    }
                } else {
                    // Escudo defendeu o ataque
                    hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.2f);
                    printf("[DARKNUT] O escudo pesado de ferro bloqueou o golpe frontal!\n");
                }
            }
        }
    }

    // Sala 4: Golpe nos 3 Sinos
    if (s_dhc.current_room == DHC_ROOM_BELL_TOWER) {
        float bell_x[3] = { 4.0f * TILE_SIZE, 8.0f * TILE_SIZE, 12.0f * TILE_SIZE };
        float bell_y = 2.5f * TILE_SIZE;

        for (int i = 0; i < 3; i++) {
            float bdx = link_x - bell_x[i];
            float bdy = link_y - bell_y;
            if (!s_dhc.bells_struck[i] && bdx * bdx + bdy * bdy < 24.0f * 24.0f) {
                s_dhc.bells_struck[i] = true;
                s_dhc.petrification_stage++;
                hal_audio_play_sound(SOUND_TOWN_BELL, 1.0f, 0.9f + i * 0.15f);
                hal_audio_play_sound(SOUND_SECRET, 0.9f, 1.1f + i * 0.1f);
                s_dhc.screen_shake_timer = 15;
                printf("[BELL TOWER] Sino da Petrificacao %d badalou e foi silenciado!\n", i + 1);
            }
        }
    }

    // Sala 1: Acerto simultâneo dos 4 Olhos com espada ou clones
    if (s_dhc.current_room == DHC_ROOM_WEST_CHASM_PUZZLE) {
        float eye_x[4] = { 4.0f * TILE_SIZE, 6.5f * TILE_SIZE, 9.5f * TILE_SIZE, 12.0f * TILE_SIZE };
        float eye_y = 2.0f * TILE_SIZE;

        for (int i = 0; i < 4; i++) {
            float edx = link_x - eye_x[i];
            float edy = link_y - eye_y;
            if (edx * edx + edy * edy < 28.0f * 28.0f) {
                if (!s_dhc.eye_switches_hit[i]) {
                    s_dhc.eye_switches_hit[i] = true;
                    s_dhc.eye_hit_window = 60; // 1 segundo para acertar todos
                    hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.2f + i * 0.1f);
                    printf("[WEST CHASM] Olho interruptor %d atingido!\n", i + 1);
                }
            }
        }

        // Clones também ativam os olhos
        for (int c = 0; c < clone_count; c++) {
            for (int i = 0; i < 4; i++) {
                float edx = clone_x[c] - eye_x[i];
                float edy = clone_y[c] - eye_y;
                if (edx * edx + edy * edy < 28.0f * 28.0f && !s_dhc.eye_switches_hit[i]) {
                    s_dhc.eye_switches_hit[i] = true;
                    s_dhc.eye_hit_window = 60;
                    hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.2f + i * 0.1f);
                    printf("[WEST CHASM] Clone %d atingiu o olho interruptor %d!\n", c + 1, i + 1);
                }
            }
        }

        bool all_eyes = true;
        for (int i = 0; i < 4; i++) {
            if (!s_dhc.eye_switches_hit[i]) all_eyes = false;
        }
        if (all_eyes && !s_dhc.hard_light_bridge_active) {
            s_dhc.hard_light_bridge_active = true;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
            printf("[WEST CHASM] Os 4 Olhos foram acionados simultaneamente! Ponte de Luz Solida estendida!\n");
        }
    }
}

void dark_castle_arrow_hit(float arrow_x, float arrow_y) {
    if (!s_dhc.is_active) return;

    // Disparo com arco nos olhos da Ala Oeste
    if (s_dhc.current_room == DHC_ROOM_WEST_CHASM_PUZZLE) {
        float eye_x[4] = { 4.0f * TILE_SIZE, 6.5f * TILE_SIZE, 9.5f * TILE_SIZE, 12.0f * TILE_SIZE };
        float eye_y = 2.0f * TILE_SIZE;

        for (int i = 0; i < 4; i++) {
            float edx = arrow_x - eye_x[i];
            float edy = arrow_y - eye_y;
            if (edx * edx + edy * edy < 16.0f * 16.0f) {
                if (!s_dhc.eye_switches_hit[i]) {
                    s_dhc.eye_switches_hit[i] = true;
                    s_dhc.eye_hit_window = 60;
                    hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.3f + i * 0.1f);
                    printf("[WEST CHASM] Flecha atingiu o olho interruptor %d!\n", i + 1);
                }
            }
        }

        bool all_eyes = true;
        for (int i = 0; i < 4; i++) {
            if (!s_dhc.eye_switches_hit[i]) all_eyes = false;
        }
        if (all_eyes && !s_dhc.hard_light_bridge_active) {
            s_dhc.hard_light_bridge_active = true;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
            printf("[WEST CHASM] Todos os 4 Olhos atingidos por flechas! Ponte estendida!\n");
        }
    }
}

void dark_castle_interact_action(float link_x, float link_y, Direction link_dir,
                                 int* player_hearts, int* player_keys) {
    if (!s_dhc.is_active) return;
    // Interações gerais de botão A já checadas no loop principal
}

void dark_castle_render(const Camera* cam, float link_x, float link_y) {
    if (!s_dhc.is_active) return;

    int ox = cam ? (int)-cam->x : 0;
    int oy = cam ? (int)-cam->y : 0;

    // Screenshake decorrente dos sinos de petrificação
    if (s_dhc.screen_shake_timer > 0) {
        ox += (s_dhc.screen_shake_timer % 2 == 0) ? 2 : -2;
        oy += (s_dhc.screen_shake_timer % 4 < 2) ? 1 : -1;
    }

    // 1. Piso ladrilhado sombrio e tapetes reais
    for (int y = 0; y < DHC_ROOM_H; y++) {
        for (int x = 0; x < DHC_ROOM_W; x++) {
            int px = ox + x * TILE_SIZE;
            int py = oy + y * TILE_SIZE;

            if (y == 0 || y == DHC_ROOM_H - 1 || x == 0 || x == DHC_ROOM_W - 1) {
                // Paredes perimetrais góticas
                draw_filled_rect(px, py, TILE_SIZE, TILE_SIZE, C_DHC_WALL);
                draw_filled_rect(px, py + 14, TILE_SIZE, 2, C_DHC_WALL_BORDER);
            } else {
                // Piso em xadrez púrpura
                u32 fcol = ((x + y) % 2 == 0) ? C_DHC_FLOOR_A : C_DHC_FLOOR_B;
                draw_filled_rect(px, py, TILE_SIZE, TILE_SIZE, fcol);

                // Tapete vermelho central (col 7..8)
                if (x == 7 || x == 8) {
                    draw_filled_rect(px, py, TILE_SIZE, TILE_SIZE, C_DHC_CARPET);
                    if (x == 7) draw_filled_rect(px, py, 2, TILE_SIZE, C_DHC_CARPET_GOLD);
                    if (x == 8) draw_filled_rect(px + 14, py, 2, TILE_SIZE, C_DHC_CARPET_GOLD);
                }
            }
        }
    }

    // 2. Elementos específicos de cada sala
    switch (s_dhc.current_room) {
        case DHC_ROOM_ENTRANCE_FOYER: {
            // Tochas nos 4 cantos
            for (int i = 0; i < MAX_DHC_TORCHES; i++) {
                int tx = ox + ((i % 2 == 0) ? 3 : 12) * TILE_SIZE;
                int ty = oy + ((i < 2) ? 3 : 7) * TILE_SIZE;
                draw_filled_rect(tx + 4, ty + 6, 8, 10, C_DHC_DARKNUT_ARMOR);
                if (s_dhc.foyer_torches_lit[i]) {
                    draw_filled_rect(tx + 2, ty + 2, 12, 12, 0xFEF08A44); // Aura
                    draw_filled_rect(tx + 5, ty + 3, 6, 6, C_DHC_FIRE_CORE);
                    draw_filled_rect(tx + 6, ty + 2, 4, 4, C_DHC_FIRE_OUTER);
                }
            }

            // Four Sword Diamond Split Pads no centro
            int pad_cx = ox + 8 * TILE_SIZE;
            int pad_cy = oy + 6 * TILE_SIZE;
            int pads[4][2] = { { 0, -18 }, { -20, 0 }, { 20, 0 }, { 0, 18 } };
            for (int p = 0; p < 4; p++) {
                int px = pad_cx + pads[p][0];
                int py = pad_cy + pads[p][1];
                draw_filled_rect(px - 7, py - 7, 14, 14, 0x0284C7FF);
                draw_filled_rect(px - 5, py - 5, 10, 10, 0x38BDF8FF);
                draw_filled_rect(px - 2, py - 2, 4, 4, 0xFFFFFFFF);
            }

            // Barreira de Malícia ao Norte
            if (!s_dhc.malice_barrier_open) {
                int bx = ox + 7 * TILE_SIZE;
                int by = oy + 1 * TILE_SIZE;
                u32 mal_col = (s_dhc.malice_pulse_timer < 30) ? C_DHC_MALICE_DARK : C_DHC_MALICE_PULSE;
                draw_filled_rect(bx, by, 32, 12, mal_col);
                draw_filled_rect(bx + 12, by + 2, 8, 8, 0xFDA4AFFF); // Olho central de malícia
            }
            break;
        }

        case DHC_ROOM_WEST_CHASM_PUZZLE: {
            // Abismo nas linhas 4..5
            int chasm_y = oy + 4 * TILE_SIZE;
            draw_filled_rect(ox + 2 * TILE_SIZE, chasm_y, 12 * TILE_SIZE, 2 * TILE_SIZE, 0x09090BFF);

            // Ponte de luz sólida estendida
            if (s_dhc.hard_light_bridge_active) {
                draw_filled_rect(ox + 6 * TILE_SIZE, chasm_y, 4 * TILE_SIZE, 2 * TILE_SIZE, C_DHC_HARD_LIGHT);
                draw_filled_rect(ox + 6 * TILE_SIZE, chasm_y + 4, 4 * TILE_SIZE, 24, 0xE0F2FE88);
            }

            // 4 Olhos Interruptores ao norte do abismo
            float eye_x[4] = { 4.0f * TILE_SIZE, 6.5f * TILE_SIZE, 9.5f * TILE_SIZE, 12.0f * TILE_SIZE };
            for (int i = 0; i < 4; i++) {
                int ex = ox + (int)eye_x[i];
                int ey = oy + 2 * TILE_SIZE;
                u32 eye_col = s_dhc.eye_switches_hit[i] ? 0x22C55EFF : 0xEF4444FF;
                draw_filled_rect(ex - 6, ey - 6, 12, 12, 0x475569FF);
                draw_filled_rect(ex - 4, ey - 4, 8, 8, eye_col);
                draw_filled_rect(ex - 1, ey - 1, 2, 2, 0xFFFFFFFF);
            }

            // Baú da Chave do Oeste
            if (!s_dhc.west_key_chest_opened) {
                int cx = ox + 8 * TILE_SIZE;
                int cy = oy + 2 * TILE_SIZE;
                draw_filled_rect(cx - 7, cy - 6, 14, 12, 0xB45309FF);
                draw_filled_rect(cx - 5, cy - 4, 10, 8, 0xF59E0BFF);
                draw_filled_rect(cx - 2, cy - 1, 4, 4, 0x78350FFF);
            }
            break;
        }

        case DHC_ROOM_EAST_MONOLITH_PUZZLE: {
            // Monólito pesado de obsidiana
            int mx = ox + (int)s_dhc.monolith_x;
            int my = oy + (int)s_dhc.monolith_y;
            draw_filled_rect(mx - 14, my - 14, 28, 28, C_DHC_WALL);
            draw_filled_rect(mx - 12, my - 12, 24, 24, 0x312E81FF);
            draw_filled_rect(mx - 6, my - 6, 12, 12, C_DHC_CARPET_GOLD);

            // Canhões de fogo nas paredes
            draw_filled_rect(ox + 14 * TILE_SIZE + 4, oy + 3 * TILE_SIZE + 4, 8, 8, 0x581C87FF);
            draw_filled_rect(ox + 14 * TILE_SIZE + 4, oy + 6 * TILE_SIZE + 4, 8, 8, 0x581C87FF);

            // Bolas de fogo ativas
            for (int i = 0; i < MAX_DHC_FIREBALLS; i++) {
                if (s_dhc.fireballs[i].active) {
                    int fx = ox + (int)s_dhc.fireballs[i].x;
                    int fy = oy + (int)s_dhc.fireballs[i].y;
                    draw_filled_rect(fx - 5, fy - 5, 10, 10, C_DHC_FIRE_OUTER);
                    draw_filled_rect(fx - 2, fy - 2, 4, 4, C_DHC_FIRE_CORE);
                }
            }
            break;
        }

        case DHC_ROOM_DARKNUT_MINIBOSS: {
            // Minichefe Darknut
            DarknutBoss* dk = &s_dhc.darknut;
            if (dk->is_active && !dk->defeated) {
                int dkx = ox + (int)dk->x;
                int dky = oy + (int)dk->y;

                // Sombra do Darknut
                draw_filled_rect(dkx - 8, dky + 8, 16, 6, 0x00000066);

                // Capa púrpura
                draw_filled_rect(dkx - 9, dky - 8, 18, 16, C_DHC_DARKNUT_CAPE);

                // Armadura de ferro negro
                draw_filled_rect(dkx - 7, dky - 10, 14, 18, C_DHC_DARKNUT_ARMOR);

                // Elmo com chifres dourados
                draw_filled_rect(dkx - 6, dky - 16, 12, 8, C_DHC_DARKNUT_ARMOR);
                draw_filled_rect(dkx - 8, dky - 18, 3, 5, C_DHC_DARKNUT_GOLD);
                draw_filled_rect(dkx + 5, dky - 18, 3, 5, C_DHC_DARKNUT_GOLD);
                // Viseira vermelha
                draw_filled_rect(dkx - 3, dky - 12, 6, 2, 0xEF4444FF);

                // Escudo pesado de ferro
                if (dk->shield_raised) {
                    int sx = dkx - 12;
                    int sy = dky - 4;
                    if (dk->facing == DIR_RIGHT) sx = dkx + 4;
                    draw_filled_rect(sx, sy, 8, 14, 0x475569FF);
                    draw_filled_rect(sx + 2, sy + 2, 4, 10, C_DHC_DARKNUT_GOLD);
                }

                // Montante pesado
                draw_filled_rect(dkx + 8, dky - 6, 3, 16, 0xE2E8F0FF);
                draw_filled_rect(dkx + 6, dky + 6, 7, 3, C_DHC_DARKNUT_GOLD);
            }

            // Sanctum Boss Key caída
            if (s_dhc.sanctum_key_spawned && !s_dhc.sanctum_key_collected) {
                int kx = ox + 8 * TILE_SIZE;
                int ky = oy + 5 * TILE_SIZE;
                draw_filled_rect(kx - 6, ky - 6, 12, 12, 0xFDE04766); // Brilho
                draw_filled_rect(kx - 4, ky - 4, 8, 8, 0xF59E0BFF);
                draw_filled_rect(kx - 1, ky - 1, 2, 6, 0x78350FFF);
            }
            break;
        }

        case DHC_ROOM_BELL_TOWER: {
            // Os 3 Grandes Sinos da Petrificação
            float bell_x[3] = { 4.0f * TILE_SIZE, 8.0f * TILE_SIZE, 12.0f * TILE_SIZE };
            for (int i = 0; i < 3; i++) {
                int bx = ox + (int)bell_x[i];
                int by = oy + 2 * TILE_SIZE;

                // Correntes de sustentação
                draw_filled_rect(bx - 1, by - 16, 2, 16, 0x475569FF);

                // Corpo do sino
                u32 bcol = s_dhc.bells_struck[i] ? 0x64748BFF : C_DHC_BELL_BRONZE;
                draw_filled_rect(bx - 10, by, 20, 16, bcol);
                draw_filled_rect(bx - 13, by + 12, 26, 6, C_DHC_BELL_SHADOW);
                // Badalo
                draw_filled_rect(bx - 2, by + 16, 4, 5, 0xF59E0BFF);

                if (s_dhc.bells_struck[i]) {
                    // Silenciado / Luz sagrada
                    draw_filled_rect(bx - 6, by + 4, 12, 4, 0x4ADE80FF);
                }
            }

            // Escadaria aberta ao Norte
            if (s_dhc.sanctum_stairs_open) {
                int sx = ox + 7 * TILE_SIZE;
                int sy = oy + 1 * TILE_SIZE;
                draw_filled_rect(sx, sy, 32, 16, 0x09090BFF);
                for (int st = 0; st < 3; st++) {
                    draw_filled_rect(sx + 2, sy + 3 + st * 4, 28, 2, 0x64748BFF);
                }
            }
            break;
        }

        case DHC_ROOM_THRONE_SANCTUM_GATE: {
            // Trono Real Corrompido
            int thx = ox + 8 * TILE_SIZE;
            int thy = oy + 2 * TILE_SIZE;
            draw_filled_rect(thx - 14, thy - 10, 28, 24, C_DHC_WALL);
            draw_filled_rect(thx - 10, thy - 6, 20, 18, C_DHC_CARPET);
            draw_filled_rect(thx - 6, thy - 14, 12, 6, C_DHC_CARPET_GOLD);

            // Altar Central de Carga da Four Sword
            int ax = ox + 8 * TILE_SIZE;
            int ay = oy + 5 * TILE_SIZE;
            draw_filled_rect(ax - 18, ay - 18, 36, 36, 0x312E81FF);
            draw_filled_rect(ax - 14, ay - 14, 28, 28, 0x4338CAFF);

            // Se os 4 Elementos foram carregados
            if (s_dhc.four_elements_charged) {
                draw_filled_rect(ax - 10, ay - 10, 20, 20, 0x38BDF8FF);
                draw_filled_rect(ax - 4, ay - 4, 8, 8, 0xFFFFFFFF);
            }

            // Portão do Sanctum de Vaati ao Norte
            int gx = ox + 7 * TILE_SIZE;
            int gy = oy + 1 * TILE_SIZE;
            if (!s_dhc.sanctum_gate_opened) {
                draw_filled_rect(gx, gy, 32, 16, C_DHC_MALICE_DARK);
                draw_filled_rect(gx + 12, gy + 4, 8, 8, 0xF43F5EFF); // Olho de Vaati trancado
            } else {
                draw_filled_rect(gx, gy, 32, 16, 0x1E1B4BFF);
                draw_filled_rect(gx + 6, gy + 2, 20, 12, 0xFDE04788); // Portal dourado para o teto!
            }
            break;
        }

        default:
            break;
    }
}

void dark_castle_restore_state(bool cleared, bool bells_silenced, bool has_boss_key) {
    s_dhc.has_sanctum_key = has_boss_key;
    if (bells_silenced) {
        s_dhc.bells_struck[0] = true;
        s_dhc.bells_struck[1] = true;
        s_dhc.bells_struck[2] = true;
        s_dhc.sanctum_stairs_open = true;
    }
    if (cleared) {
        s_dhc.sanctum_gate_opened = true;
        s_dhc.ready_for_vaati_climax = true;
    }
}

bool dark_castle_is_cleared(void) {
    return s_dhc.ready_for_vaati_climax;
}

bool dark_castle_is_ready_for_vaati(void) {
    return s_dhc.ready_for_vaati_climax;
}
