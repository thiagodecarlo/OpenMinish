/*
 * ============================================================================
 * src/hal/dungeon.c - Implementação da Masmorra: Deepwood Shrine
 * ============================================================================
 */

#include "hal/dungeon.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/entity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

#define C_DUNG_WALL_TOP   0x4A5568FF // Parede de pedra topo
#define C_DUNG_WALL_FACE  0x2D3748FF // Parede de pedra frontal
#define C_DUNG_WALL_DARK  0x1A202CFF // Argamassa e sombra
#define C_DUNG_FLOOR_BASE 0x334155FF // Laje cinza azulada polida
#define C_DUNG_FLOOR_CRK  0x1E293BFF // Fissuras na pedra
#define C_DUNG_MOSS       0x15803DFF // Musgo antigo nos cantos
#define C_DUNG_WATER      0x1E3A8AFF // Água profunda do canal
#define C_DUNG_WATER_LGT  0x3B82F6FF // Água rasa / reflexo
#define C_DUNG_BRIDGE     0x78350FFF // Madeira da ponte
#define C_DUNG_TORCH_WOOD 0x451A03FF // Cabo da tocha
#define C_DUNG_FIRE_1     0xEF4444FF // Fogo vermelho
#define C_DUNG_FIRE_2     0xF59E0BFF // Fogo laranja
#define C_DUNG_FIRE_3     0xFDE047FF // Fogo amarelo/branco
#define C_DUNG_IRON       0x64748BFF // Ferro das grades
#define C_DUNG_GOLD       0xF59E0BFF // Dourado do cadeado e detalhes

static DungeonState s_dungeon = { 0 };
static int s_anim_timer = 0;

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

void dungeon_init(void) {
    memset(&s_dungeon, 0, sizeof(s_dungeon));
    s_dungeon.active = false;
    s_dungeon.current_room = ROOM_ENTRANCE;
    s_dungeon.small_keys = 0;

    // Configuração da Sala 0 (Entrada)
    s_dungeon.block.x = 6.0f * TILE_SIZE;
    s_dungeon.block.y = 4.0f * TILE_SIZE;
    s_dungeon.block.start_x = s_dungeon.block.x;
    s_dungeon.block.start_y = s_dungeon.block.y;
    s_dungeon.block.target_x = s_dungeon.block.x;
    s_dungeon.block.target_y = s_dungeon.block.y;
    s_dungeon.block.is_moving = false;
    s_dungeon.block.push_timer = 0;

    s_dungeon.sw_entrance.x = 4.0f * TILE_SIZE;
    s_dungeon.sw_entrance.y = 4.0f * TILE_SIZE;
    s_dungeon.sw_entrance.is_down = false;

    s_dungeon.door_north_entrance.x = 7.0f * TILE_SIZE;
    s_dungeon.door_north_entrance.y = 0.0f;
    s_dungeon.door_north_entrance.is_locked = false;
    s_dungeon.door_north_entrance.is_open = false;

    // Configuração da Sala 2 (Canal de Água)
    s_dungeon.door_locked_water.x = 7.0f * TILE_SIZE;
    s_dungeon.door_locked_water.y = 0.0f;
    s_dungeon.door_locked_water.is_locked = true;
    s_dungeon.door_locked_water.is_open = false;

    // Sala 1 (Arena de Inimigos)
    s_dungeon.key_x = 7.5f * TILE_SIZE;
    s_dungeon.key_y = 4.5f * TILE_SIZE;

    // Sala 4 (Arena do Chefe Big Green ChuChu)
    s_dungeon.door_boss_shutter.x = 7.0f * TILE_SIZE;
    s_dungeon.door_boss_shutter.y = 9.0f * TILE_SIZE;
    s_dungeon.door_boss_shutter.is_locked = false;
    s_dungeon.door_boss_shutter.is_open = false;
    s_dungeon.boss_chamber_entered = false;
    s_dungeon.boss_cleared = false;
    s_dungeon.boss_portal_spawned = false;
    s_dungeon.portal_x = 7.5f * TILE_SIZE;
    s_dungeon.portal_y = 4.5f * TILE_SIZE;
}

bool dungeon_is_active(void) {
    return s_dungeon.active;
}

DungeonState* dungeon_get_state(void) {
    return &s_dungeon;
}

void dungeon_enter(float* link_x, float* link_y, Direction* link_dir) {
    s_dungeon.active = true;
    s_dungeon.current_room = ROOM_ENTRANCE;
    s_dungeon.transitioning = false;

    if (link_x) *link_x = 7.5f * TILE_SIZE;
    if (link_y) *link_y = 8.0f * TILE_SIZE; // Próximo à escada da entrada
    if (link_dir) *link_dir = DIR_UP;

    hal_audio_play_bgm(BGM_DEEPWOOD_SHRINE);
    hal_audio_play_sound(SOUND_SECRET, 0.8f, 0.9f);
    printf("[DUNGEON] Link entrou em Deepwood Shrine (Vestibulo de Entrada)!\n");
}

void dungeon_exit(float* link_x, float* link_y, Direction* link_dir) {
    s_dungeon.active = false;
    s_dungeon.transitioning = false;

    if (link_x) *link_x = 448.0f;
    if (link_y) *link_y = 510.0f; // Clareira do santuário em Minish Woods
    if (link_dir) *link_dir = DIR_DOWN;

    hal_audio_play_bgm(BGM_MINISH_WOODS);
    hal_audio_play_sound(SOUND_SECRET, 0.8f, 1.2f);
    printf("[DUNGEON] Link retornou para Minish Woods!\n");
}

static void trigger_room_transition(DungeonRoomId next_room, float spawn_x, float spawn_y) {
    s_dungeon.transitioning = true;
    s_dungeon.trans_timer = 20;
    s_dungeon.target_room = next_room;
    s_dungeon.target_spawn_x = spawn_x;
    s_dungeon.target_spawn_y = spawn_y;
    hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.70f, 1.1f);
}

void dungeon_update(float* link_x, float* link_y, Direction* link_dir, bool is_moving,
                    int* link_hearts, int* link_rupees) {
    if (!s_dungeon.active) return;

    s_anim_timer++;

    // Transição de tela suave entre câmaras
    if (s_dungeon.transitioning) {
        s_dungeon.trans_timer--;
        if (s_dungeon.trans_timer == 10) {
            s_dungeon.current_room = s_dungeon.target_room;
            if (link_x) *link_x = s_dungeon.target_spawn_x;
            if (link_y) *link_y = s_dungeon.target_spawn_y;

            // Limpa entidades anteriores ao mudar de câmara
            entity_clear_all();

            // Spawn de inimigos específicos da sala ao entrar
            if (s_dungeon.current_room == ROOM_GUARDIANS && !s_dungeon.room_guardians_cleared) {
                entity_spawn(ENTITY_ENEMY_KEESE, 5.0f * TILE_SIZE, 3.0f * TILE_SIZE);
                entity_spawn(ENTITY_ENEMY_KEESE, 10.0f * TILE_SIZE, 3.0f * TILE_SIZE);
                entity_spawn(ENTITY_ENEMY_CHUCHU, 7.5f * TILE_SIZE, 5.0f * TILE_SIZE);
                hal_audio_play_sound(SOUND_KEESE_CHIRP, 0.80f, 1.0f);
            } else if (s_dungeon.current_room == ROOM_BOSS_ARENA) {
                if (!s_dungeon.boss_cleared) {
                    s_dungeon.door_boss_shutter.is_open = false; // Fecha as grades com estrondo!
                    entity_spawn(ENTITY_BOSS_BIG_CHUCHU, 7.5f * TILE_SIZE, 3.5f * TILE_SIZE);
                    hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.95f, 0.85f);
                    hal_audio_play_bgm(BGM_BOSS_BATTLE);
                    printf("[DUNGEON] Link entrou na Arena do Chefe! Big Green ChuChu apareceu!\n");
                } else {
                    s_dungeon.door_boss_shutter.is_open = true;
                }
            } else if (s_dungeon.current_room == ROOM_SANCTUARY_ALTAR) {
                if (hal_audio_get_current_bgm() == BGM_BOSS_BATTLE) {
                    hal_audio_play_bgm(BGM_DEEPWOOD_SHRINE);
                }
            }
        }
        if (s_dungeon.trans_timer <= 0) {
            s_dungeon.transitioning = false;
        }
        return;
    }

    if (!link_x || !link_y || !link_dir) return;
    float lx = *link_x;
    float ly = *link_y;
    Direction ldir = *link_dir;

    // ------------------------------------------------------------------------
    // CÂMARA 0: VESTÍBULO DE ENTRADA
    // ------------------------------------------------------------------------
    if (s_dungeon.current_room == ROOM_ENTRANCE) {
        // 1. Saída da Masmorra pela escada ao sul
        if (ly >= 8.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            dungeon_exit(link_x, link_y, link_dir);
            return;
        }

        // 2. Físicas do Bloco Empurrável
        DungeonBlock* b = &s_dungeon.block;
        if (b->is_moving) {
            float spd = 1.0f;
            if (b->x < b->target_x) { b->x += spd; if (b->x > b->target_x) b->x = b->target_x; }
            if (b->x > b->target_x) { b->x -= spd; if (b->x < b->target_x) b->x = b->target_x; }
            if (b->y < b->target_y) { b->y += spd; if (b->y > b->target_y) b->y = b->target_y; }
            if (b->y > b->target_y) { b->y -= spd; if (b->y < b->target_y) b->y = b->target_y; }

            if (b->x == b->target_x && b->y == b->target_y) {
                b->is_moving = false;
            }
        } else if (is_moving) {
            // Checa se Link está encostando no bloco e empurrando
            float bx1 = b->x;
            float by1 = b->y;
            float bx2 = bx1 + TILE_SIZE;
            float by2 = by1 + TILE_SIZE;

            bool touching = false;
            if (ldir == DIR_RIGHT && lx + 14.0f >= bx1 && lx < bx1 && ly + 8.0f >= by1 && ly + 8.0f <= by2) touching = true;
            if (ldir == DIR_LEFT  && lx <= bx2 && lx + 8.0f > bx2 && ly + 8.0f >= by1 && ly + 8.0f <= by2) touching = true;
            if (ldir == DIR_DOWN  && ly + 16.0f >= by1 && ly < by1 && lx + 8.0f >= bx1 && lx + 8.0f <= bx2) touching = true;
            if (ldir == DIR_UP    && ly <= by2 && ly + 8.0f > by2 && lx + 8.0f >= bx1 && lx + 8.0f <= bx2) touching = true;

            if (touching) {
                b->push_timer++;
                if (b->push_timer >= 12) { // 12 frames empurrando para iniciar o deslizamento
                    float tx = b->x;
                    float ty = b->y;
                    if (ldir == DIR_LEFT)  tx -= TILE_SIZE;
                    if (ldir == DIR_RIGHT) tx += TILE_SIZE;
                    if (ldir == DIR_UP)    ty -= TILE_SIZE;
                    if (ldir == DIR_DOWN)  ty += TILE_SIZE;

                    // Não pode empurrar para dentro da parede
                    int t_col = (int)(tx / TILE_SIZE);
                    int t_row = (int)(ty / TILE_SIZE);
                    if (t_col >= 2 && t_col <= 13 && t_row >= 2 && t_row <= 7) {
                        b->target_x = tx;
                        b->target_y = ty;
                        b->is_moving = true;
                        b->push_timer = 0;
                        hal_audio_play_sound(SOUND_BLOCK_PUSH, 0.85f, 1.0f);
                    }
                }
            } else {
                b->push_timer = 0;
            }
        }

        // 3. Verificação do Interruptor de Piso
        DungeonSwitch* sw = &s_dungeon.sw_entrance;
        float sw_cx = sw->x + 8.0f;
        float sw_cy = sw->y + 8.0f;

        // Ativado se Link ou o Bloco estiver sobre ele
        bool link_on_sw = (fabsf(lx + 8.0f - sw_cx) <= 10.0f && fabsf(ly + 12.0f - sw_cy) <= 10.0f);
        bool block_on_sw = (fabsf(b->x - sw->x) <= 4.0f && fabsf(b->y - sw->y) <= 4.0f);

        bool should_be_down = (link_on_sw || block_on_sw);
        if (should_be_down && !sw->is_down) {
            sw->is_down = true;
            s_dungeon.door_north_entrance.is_open = true;
            hal_audio_play_sound(SOUND_SWITCH_CLICK, 0.95f, 1.0f);
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.85f, 1.0f);
            printf("[DUNGEON] Interruptor ativado! Grades da porta norte abertas!\n");
        } else if (!should_be_down && sw->is_down && !block_on_sw) {
            // Se Link sair de cima e o bloco não estiver no switch, fecha
            sw->is_down = false;
            s_dungeon.door_north_entrance.is_open = false;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.75f, 0.85f);
        }

        // 4. Cruzar a porta norte para a Câmara 1 (se aberta)
        if (s_dungeon.door_north_entrance.is_open && ly <= 1.0f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_GUARDIANS, 7.5f * TILE_SIZE, 8.2f * TILE_SIZE);
        }
    }

    // ------------------------------------------------------------------------
    // CÂMARA 1: ARENA DE COMBATE & CHAVE PEQUENA
    // ------------------------------------------------------------------------
    else if (s_dungeon.current_room == ROOM_GUARDIANS) {
        if (s_dungeon.room_guardians_cleared) {
            // 1. Volta para Sala 0 pelo sul (portas abertas)
            if (ly >= 8.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
                trigger_room_transition(ROOM_ENTRANCE, 7.5f * TILE_SIZE, 1.5f * TILE_SIZE);
                return;
            }

            // 2. Avança para Sala 2 pelo leste (portas abertas)
            if (lx >= 14.5f * TILE_SIZE && ly >= 3.8f * TILE_SIZE && ly <= 6.2f * TILE_SIZE) {
                trigger_room_transition(ROOM_WATER_CHANNEL, 1.5f * TILE_SIZE, 4.5f * TILE_SIZE);
                return;
            }
        }

        // 3. Drop da Chave Pequena após derrotar os guardiões
        if (!s_dungeon.room_guardians_cleared) {
            if (entity_count_active_enemies() == 0) {
                s_dungeon.room_guardians_cleared = true;
                s_dungeon.key_spawned = true;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.85f, 1.0f); // Levanta as grades das portas!
                printf("[DUNGEON] Guardioes derrotados! Chave Pequena (Small Key) surgiu no centro!\n");
            }
        }

        // 4. Coleta da Chave Pequena pelo Link (manual ou com Bumerangue)
        if (s_dungeon.key_spawned && !s_dungeon.key_collected) {
            float kx = s_dungeon.key_x;
            float ky = s_dungeon.key_y;
            if (fabsf(lx + 8.0f - kx) <= 14.0f && fabsf(ly + 8.0f - ky) <= 14.0f) {
                s_dungeon.key_collected = true;
                s_dungeon.small_keys++;
                hal_audio_play_sound(SOUND_ITEM_CATCH, 1.0f, 1.0f);
                printf("[DUNGEON] Chave Pequena obtida! (Total: %d)\n", s_dungeon.small_keys);
            }
        }
    }

    // ------------------------------------------------------------------------
    // CÂMARA 2: CANAL DE ÁGUA & PORTA TRANCADA
    // ------------------------------------------------------------------------
    else if (s_dungeon.current_room == ROOM_WATER_CHANNEL) {
        // 1. Volta para Sala 1 pelo oeste
        if (lx <= 1.0f * TILE_SIZE && ly >= 3.8f * TILE_SIZE && ly <= 6.2f * TILE_SIZE) {
            trigger_room_transition(ROOM_GUARDIANS, 14.0f * TILE_SIZE, 4.5f * TILE_SIZE);
            return;
        }

        // 2. Destrancamento da Porta com Cadeado ao norte
        DungeonDoorState* d = &s_dungeon.door_locked_water;
        if (d->is_locked && !d->is_open) {
            if (ly <= 1.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
                if (s_dungeon.small_keys > 0) {
                    s_dungeon.small_keys--;
                    d->is_locked = false;
                    d->is_open = true;
                    hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                    printf("[DUNGEON] Porta com cadeado destrancada com sucesso!\n");
                }
            }
        }

        // 3. Entra na Sala 3 (Santuário Interno) pelo norte
        if (d->is_open && ly <= 1.0f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_SANCTUARY_ALTAR, 7.5f * TILE_SIZE, 8.2f * TILE_SIZE);
        }
    }

    // ------------------------------------------------------------------------
    // CÂMARA 3: SANTUÁRIO INTERNO & ALTAR SAGRADO
    // ------------------------------------------------------------------------
    else if (s_dungeon.current_room == ROOM_SANCTUARY_ALTAR) {
        // 1. Volta para Sala 2 pelo sul
        if (ly >= 8.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_WATER_CHANNEL, 7.5f * TILE_SIZE, 1.5f * TILE_SIZE);
            return;
        }

        // 2. Interação com o Grande Baú do Altar
        if (!s_dungeon.chest_altar_opened) {
            float cx = 7.5f * TILE_SIZE;
            float cy = 3.2f * TILE_SIZE;
            if (fabsf(lx + 8.0f - cx) <= 20.0f && fabsf(ly + 8.0f - cy) <= 20.0f) {
                dungeon_interact(lx, ly, link_rupees, link_hearts);
            }
        }

        // 3. Passagem Norte para a Arena do Chefe (ROOM_BOSS_ARENA)
        if (ly <= 1.0f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_BOSS_ARENA, 7.5f * TILE_SIZE, 8.0f * TILE_SIZE);
            return;
        }
    }

    // ------------------------------------------------------------------------
    // CÂMARA 4: ARENA DO CHEFE (BIG GREEN CHUCHU)
    // ------------------------------------------------------------------------
    else if (s_dungeon.current_room == ROOM_BOSS_ARENA) {
        // 1. Verifica se o chefe acabou de ser derrotado
        if (!s_dungeon.boss_cleared && entity_is_boss_defeated()) {
            s_dungeon.boss_cleared = true;
            s_dungeon.boss_portal_spawned = true;
            s_dungeon.door_boss_shutter.is_open = true;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.90f, 1.15f);
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.1f);
            hal_audio_play_bgm(BGM_DEEPWOOD_SHRINE);
            printf("[BOSS] Big Green ChuChu derrotado! Grades abertas e Portal de Teletransporte ativado!\n");
        }

        // 2. Passagem de volta para o Santuário Interno pelo sul (se as grades estiverem abertas)
        if (s_dungeon.door_boss_shutter.is_open) {
            if (ly >= 8.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
                trigger_room_transition(ROOM_SANCTUARY_ALTAR, 7.5f * TILE_SIZE, 1.5f * TILE_SIZE);
                return;
            }
        }

        // 3. Teletransporte pelo Portal Sagrado de volta a Minish Woods
        if (s_dungeon.boss_portal_spawned) {
            float px = s_dungeon.portal_x;
            float py = s_dungeon.portal_y;
            if (fabsf(lx + 8.0f - px) <= 14.0f && fabsf(ly + 8.0f - py) <= 14.0f) {
                dungeon_exit(link_x, link_y, link_dir);
                return;
            }
        }
    }
}

bool dungeon_interact(float x, float y, int* link_rupees, int* link_hearts) {
    if (!s_dungeon.active) return false;

    // Interação com o Grande Baú do Altar na Sala 3
    if (s_dungeon.current_room == ROOM_SANCTUARY_ALTAR && !s_dungeon.chest_altar_opened) {
        float cx = 7.5f * TILE_SIZE + 8.0f;
        float cy = 3.2f * TILE_SIZE + 8.0f;
        if (fabsf(x + 8.0f - cx) <= 24.0f && fabsf(y + 8.0f - cy) <= 24.0f) {
            s_dungeon.chest_altar_opened = true;
            hal_audio_play_sound(SOUND_CHEST_OPEN, 1.0f, 1.0f);
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 0.85f);

            if (link_rupees) *link_rupees += 100;
            if (link_hearts) *link_hearts = 3;

            printf("[DUNGEON] VITORIA! Grande Bau do Altar aberto: +100 Rupees e Sagrado Elemento Terra!\n");
            return true;
        }
    }

    return false;
}

bool dungeon_is_solid(float world_x, float world_y) {
    if (!s_dungeon.active) return false;

    int col = (int)(world_x / TILE_SIZE);
    int row = (int)(world_y / TILE_SIZE);

    // Paredes externas da câmara (16x10 tiles)
    if (col < 1 || col >= 15 || row < 1 || row >= 9) {
        // Permite passagem apenas nos pontos exatos das portas abertas
        if (row == 0 && (col == 7 || col == 8)) {
            if (s_dungeon.current_room == ROOM_ENTRANCE && s_dungeon.door_north_entrance.is_open) return false;
            if (s_dungeon.current_room == ROOM_WATER_CHANNEL && s_dungeon.door_locked_water.is_open) return false;
            if (s_dungeon.current_room == ROOM_SANCTUARY_ALTAR) return false; // Passagem para o boss
        }
        if (row == 9 && (col == 7 || col == 8)) {
            // Escada de saída na Sala 0 ou portas sul nas outras salas
            if (s_dungeon.current_room == ROOM_GUARDIANS && !s_dungeon.room_guardians_cleared) return true;
            if (s_dungeon.current_room == ROOM_BOSS_ARENA && !s_dungeon.door_boss_shutter.is_open) return true;
            return false;
        }
        if (col == 15 && (row == 4 || row == 5) && s_dungeon.current_room == ROOM_GUARDIANS) {
            if (!s_dungeon.room_guardians_cleared) return true;
            return false;
        }
        if (col == 0 && (row == 4 || row == 5) && s_dungeon.current_room == ROOM_WATER_CHANNEL) return false;
        return true;
    }

    // Colisão com o Bloco Empurrável na Sala 0
    if (s_dungeon.current_room == ROOM_ENTRANCE) {
        DungeonBlock* b = &s_dungeon.block;
        if (world_x >= b->x && world_x < b->x + TILE_SIZE &&
            world_y >= b->y && world_y < b->y + TILE_SIZE) {
            return true;
        }
    }

    // Colisão com a Água na Sala 2 (exceto ponte de madeira central)
    if (s_dungeon.current_room == ROOM_WATER_CHANNEL) {
        if (col >= 6 && col <= 9 && row >= 2 && row <= 7) {
            // Canal de água
            if (!(row >= 4 && row <= 5)) { // Ponte de madeira em row 4 e 5
                return true;
            }
        }
    }

    // Colisão com o Altar na Sala 3 (apenas o bloco cerimonial)
    if (s_dungeon.current_room == ROOM_SANCTUARY_ALTAR) {
        if (col >= 6 && col <= 9 && row >= 2 && row <= 3) return true;
    }

    // Colisão na Sala do Chefe (4 pilares rituais nos cantos)
    if (s_dungeon.current_room == ROOM_BOSS_ARENA) {
        if ((col == 3 || col == 12) && (row == 3 || row == 7)) return true;
    }

    return false;
}

// ----------------------------------------------------------------------------
// RENDERIZAÇÃO GRÁFICA DA MASMORRA (ESTILO RETRÔ GBA)
// ----------------------------------------------------------------------------

static void render_dungeon_room_base(const Camera* cam) {
    for (int row = 0; row < DUNGEON_ROOM_H; row++) {
        for (int col = 0; col < DUNGEON_ROOM_W; col++) {
            int sx, sy;
            map_world_to_screen(cam, (float)(col * TILE_SIZE), (float)(row * TILE_SIZE), &sx, &sy);

            bool is_wall = (col == 0 || col == 15 || row == 0 || row == 9);

            if (is_wall) {
                // Paredes da masmorra
                draw_rect_blend(sx, sy, TILE_SIZE, TILE_SIZE, C_DUNG_WALL_FACE);
                draw_rect_blend(sx, sy, TILE_SIZE, 3, C_DUNG_WALL_TOP);
                draw_rect_blend(sx, sy + 13, TILE_SIZE, 3, C_DUNG_WALL_DARK);

                // Entalhes de hera/musgo
                if ((col + row) % 3 == 0) {
                    draw_rect_blend(sx + 3, sy + 4, 3, 5, C_DUNG_MOSS);
                }
            } else {
                // Piso de pedra polida
                draw_rect_blend(sx, sy, TILE_SIZE, TILE_SIZE, C_DUNG_FLOOR_BASE);
                draw_rect_blend(sx, sy, TILE_SIZE, 1, C_DUNG_FLOOR_CRK);
                draw_rect_blend(sx, sy, 1, TILE_SIZE, C_DUNG_FLOOR_CRK);
            }
        }
    }
}

void dungeon_render(const Camera* cam) {
    if (!s_dungeon.active || !cam) return;

    // 1. Renderiza o piso e paredes base da câmara
    render_dungeon_room_base(cam);

    // 2. Renderização Específica por Câmara
    if (s_dungeon.current_room == ROOM_ENTRANCE) {
        // Escada de saída ao sul
        int ex_sx, ex_sy;
        map_world_to_screen(cam, 7.0f * TILE_SIZE, 9.0f * TILE_SIZE, &ex_sx, &ex_sy);
        for (int step = 0; step < 4; step++) {
            draw_rect_blend(ex_sx, ex_sy + (step * 4), 32, 2, 0x1E293BFF);
            draw_rect_blend(ex_sx, ex_sy + (step * 4) + 2, 32, 2, 0x475569FF);
        }

        // Interruptor de Piso
        DungeonSwitch* sw = &s_dungeon.sw_entrance;
        int sw_sx, sw_sy;
        map_world_to_screen(cam, sw->x, sw->y, &sw_sx, &sw_sy);
        u32 sw_col = sw->is_down ? 0x22C55EFF : 0x3B82F6FF; // Verde ativo / Azul pronto
        draw_rect_blend(sw_sx + 2, sw_sy + 2, 12, 12, sw_col);
        draw_rect_blend(sw_sx + 4, sw_sy + 4, 8, 8, 0xD4AF37FF);

        // Bloco Empurrável
        DungeonBlock* b = &s_dungeon.block;
        int b_sx, b_sy;
        map_world_to_screen(cam, b->x, b->y, &b_sx, &b_sy);
        draw_rect_blend(b_sx + 1, b_sy + 1, 14, 14, 0x475569FF);
        draw_rect_blend(b_sx + 2, b_sy + 2, 12, 12, 0x64748BFF);
        draw_rect_blend(b_sx + 5, b_sy + 5, 6, 6, 0x94A3B8FF); // Símbolo sagrado central

        // Porta Norte (Grades Shutter)
        int dn_sx, dn_sy;
        map_world_to_screen(cam, 7.0f * TILE_SIZE, 0.0f, &dn_sx, &dn_sy);
        if (!s_dungeon.door_north_entrance.is_open) {
            // Grades de ferro baixadas
            for (int bar = 0; bar < 6; bar++) {
                draw_rect_blend(dn_sx + 2 + (bar * 5), dn_sy + 2, 2, 14, C_DUNG_IRON);
            }
        } else {
            // Passagem livre aberta
            draw_rect_blend(dn_sx, dn_sy, 32, 16, 0x050810FF);
        }
    } else if (s_dungeon.current_room == ROOM_GUARDIANS) {
        // Porta Sul
        int ds_sx, ds_sy;
        map_world_to_screen(cam, 7.0f * TILE_SIZE, 9.0f * TILE_SIZE, &ds_sx, &ds_sy);
        if (!s_dungeon.room_guardians_cleared) {
            for (int bar = 0; bar < 6; bar++) {
                draw_rect_blend(ds_sx + 2 + (bar * 5), ds_sy + 2, 2, 14, C_DUNG_IRON);
            }
        } else {
            draw_rect_blend(ds_sx, ds_sy, 32, 16, 0x050810FF);
        }

        // Porta Leste
        int de_sx, de_sy;
        map_world_to_screen(cam, 15.0f * TILE_SIZE, 4.0f * TILE_SIZE, &de_sx, &de_sy);
        if (!s_dungeon.room_guardians_cleared) {
            for (int bar = 0; bar < 6; bar++) {
                draw_rect_blend(de_sx + 2, de_sy + 2 + (bar * 5), 12, 2, C_DUNG_IRON);
            }
        } else {
            draw_rect_blend(de_sx, de_sy, 16, 32, 0x050810FF);
        }

        // Chave Pequena dropada no chão
        if (s_dungeon.key_spawned && !s_dungeon.key_collected) {
            int k_sx, k_sy;
            map_world_to_screen(cam, s_dungeon.key_x, s_dungeon.key_y, &k_sx, &k_sy);
            int bob = (int)(sinf((float)s_anim_timer * 0.15f) * 2.5f);

            // Cabeça circular da chave
            draw_rect_blend(k_sx - 2, k_sy - 3 + bob, 5, 4, 0xFFD700FF);
            hal_video_put_pixel(k_sx, k_sy - 2 + bob, 0x111111FF);
            // Haste e dentes
            draw_rect_blend(k_sx, k_sy + 1 + bob, 2, 6, 0xFFD700FF);
            draw_rect_blend(k_sx + 2, k_sy + 4 + bob, 2, 2, 0xFFE27AFF);

            // Brilho cintilante
            if ((s_anim_timer / 10) % 2 == 0) {
                hal_video_put_pixel(k_sx - 2, k_sy - 4 + bob, 0xFFFFFFFF);
            }
        }
    } else if (s_dungeon.current_room == ROOM_WATER_CHANNEL) {
        // Porta Oeste (passagem de volta para Sala 1)
        int dw_sx, dw_sy;
        map_world_to_screen(cam, 0.0f, 4.0f * TILE_SIZE, &dw_sx, &dw_sy);
        draw_rect_blend(dw_sx, dw_sy, 16, 32, 0x050810FF);

        // Canal de Água e Ponte de Madeira
        int w_sx, w_sy;
        map_world_to_screen(cam, 6.0f * TILE_SIZE, 2.0f * TILE_SIZE, &w_sx, &w_sy);
        draw_rect_blend(w_sx, w_sy, 64, 96, C_DUNG_WATER);

        // Ondas na água
        for (int row = 0; row < 6; row++) {
            int wave_x = (s_anim_timer + row * 12) % 64;
            draw_rect_blend(w_sx + wave_x, w_sy + (row * 16) + 6, 12, 2, C_DUNG_WATER_LGT);
        }

        // Ponte de Madeira (row 4 e 5)
        int br_sx, br_sy;
        map_world_to_screen(cam, 6.0f * TILE_SIZE, 4.0f * TILE_SIZE, &br_sx, &br_sy);
        draw_rect_blend(br_sx, br_sy, 64, 32, C_DUNG_BRIDGE);
        for (int p = 0; p < 8; p++) {
            draw_rect_blend(br_sx + (p * 8), br_sy, 1, 32, 0x451A03FF);
        }

        // Porta Trancada com Cadeado ao Norte
        DungeonDoorState* d = &s_dungeon.door_locked_water;
        int dl_sx, dl_sy;
        map_world_to_screen(cam, 7.0f * TILE_SIZE, 0.0f, &dl_sx, &dl_sy);
        if (d->is_locked) {
            // Madeira de carvalho
            draw_rect_blend(dl_sx, dl_sy, 32, 16, 0x451A03FF);
            // Cadeado grande de ouro e ferro
            draw_rect_blend(dl_sx + 12, dl_sy + 5, 8, 8, C_DUNG_GOLD);
            draw_rect_blend(dl_sx + 13, dl_sy + 2, 6, 3, C_DUNG_IRON);
            hal_video_put_pixel(dl_sx + 15, dl_sy + 8, 0x111111FF); // Buraco da fechadura
        } else {
            // Porta aberta
            draw_rect_blend(dl_sx, dl_sy, 32, 16, 0x050810FF);
        }
    } else if (s_dungeon.current_room == ROOM_SANCTUARY_ALTAR) {
        // Porta Sul (passagem de volta para Sala 2)
        int ds_sx, ds_sy;
        map_world_to_screen(cam, 7.0f * TILE_SIZE, 9.0f * TILE_SIZE, &ds_sx, &ds_sy);
        draw_rect_blend(ds_sx, ds_sy, 32, 16, 0x050810FF);

        // Altar Cerimonial
        int al_sx, al_sy;
        map_world_to_screen(cam, 6.0f * TILE_SIZE, 2.0f * TILE_SIZE, &al_sx, &al_sy);
        draw_rect_blend(al_sx, al_sy, 64, 32, 0x1E293BFF);
        draw_rect_blend(al_sx + 2, al_sy + 2, 60, 4, 0xD4AF37FF);

        // Grande Baú Dourado do Altar
        int ch_sx = al_sx + 24;
        int ch_sy = al_sy + 10;
        if (!s_dungeon.chest_altar_opened) {
            draw_rect_blend(ch_sx, ch_sy, 16, 12, 0xD4AF37FF);
            draw_rect_blend(ch_sx + 2, ch_sy + 2, 12, 4, 0xFFE27AFF);
            draw_rect_blend(ch_sx + 7, ch_sy + 6, 2, 3, 0x111111FF);
        } else {
            // Baú aberto
            draw_rect_blend(ch_sx, ch_sy + 4, 16, 8, 0xD4AF37FF);
            draw_rect_blend(ch_sx + 2, ch_sy + 5, 12, 4, 0x881111FF);
        }

        // Porta Norte para a Arena do Chefe (ROOM_BOSS_ARENA)
        int dn_sx, dn_sy;
        map_world_to_screen(cam, 7.0f * TILE_SIZE, 0.0f, &dn_sx, &dn_sy);
        draw_rect_blend(dn_sx, dn_sy, 32, 16, 0x050810FF);
        draw_rect_blend(dn_sx + 2, dn_sy + 2, 28, 2, 0xD4AF37FF);
        draw_rect_blend(dn_sx + 14, dn_sy + 4, 4, 3, 0x22C55EFF);

        // Tochas Místicas de Fogo Azul do Altar
        int b1_x, b1_y, b2_x, b2_y;
        map_world_to_screen(cam, 4.0f * TILE_SIZE, 2.0f * TILE_SIZE, &b1_x, &b1_y);
        map_world_to_screen(cam, 11.0f * TILE_SIZE, 2.0f * TILE_SIZE, &b2_x, &b2_y);
        int blue_anim = (s_anim_timer / 5) % 3;
        u32 blue_fire = (blue_anim == 0) ? 0x38BDF8FF : ((blue_anim == 1) ? 0x60A5FAFF : 0x93C5FDFF);
        draw_rect_blend(b1_x + 4, b1_y + 8, 8, 8, 0xD4AF37FF); // Pedestal dourado
        draw_rect_blend(b2_x + 4, b2_y + 8, 8, 8, 0xD4AF37FF);
        draw_rect_blend(b1_x + 5, b1_y + 3, 6, 6, blue_fire);
        draw_rect_blend(b2_x + 5, b2_y + 3, 6, 6, blue_fire);
    } else if (s_dungeon.current_room == ROOM_BOSS_ARENA) {
        // Porta Sul (Grades Shutter para o Santuário)
        int ds_sx, ds_sy;
        map_world_to_screen(cam, 7.0f * TILE_SIZE, 9.0f * TILE_SIZE, &ds_sx, &ds_sy);
        if (!s_dungeon.door_boss_shutter.is_open) {
            for (int bar = 0; bar < 6; bar++) {
                draw_rect_blend(ds_sx + 2 + (bar * 5), ds_sy + 2, 2, 14, C_DUNG_IRON);
            }
        } else {
            draw_rect_blend(ds_sx, ds_sy, 32, 16, 0x050810FF);
        }

        // Círculo Sagrado Ritual da Arena do Chefe (Círculos concêntricos de pedra polida)
        int ac_x, ac_y;
        map_world_to_screen(cam, 7.5f * TILE_SIZE + 8.0f, 4.5f * TILE_SIZE + 8.0f, &ac_x, &ac_y);
        for (int r = 48; r >= 16; r -= 16) {
            u32 ring_col = (r == 48) ? 0x1E293BFF : ((r == 32) ? 0x475569FF : 0xD4AF37FF);
            for (int deg = 0; deg < 360; deg += 6) {
                float rad = (float)deg * (PI_F / 180.0f);
                int rx = ac_x + (int)(cosf(rad) * (float)r);
                int ry = ac_y + (int)(sinf(rad) * (float)(r * 0.65f));
                hal_video_put_pixel(rx, ry, ring_col);
                hal_video_put_pixel(rx + 1, ry, ring_col);
            }
        }

        // 4 Pilares Arcanos nos cantos (col 3, col 12, row 3, row 7) com Tochas Místicas
        int pil_cols[4] = { 3, 12, 3, 12 };
        int pil_rows[4] = { 3, 3, 7, 7 };
        for (int p = 0; p < 4; p++) {
            int p_sx, p_sy;
            map_world_to_screen(cam, (float)(pil_cols[p] * TILE_SIZE), (float)(pil_rows[p] * TILE_SIZE), &p_sx, &p_sy);
            draw_rect_blend(p_sx + 1, p_sy + 1, 14, 14, 0x1E293BFF);
            draw_rect_blend(p_sx + 3, p_sy + 3, 10, 10, 0x475569FF);
            draw_rect_blend(p_sx + 4, p_sy + 4, 8, 8, 0xD4AF37FF);

            // Fogo Místico de Deepwood Shrine (Verde Esmeralda e Ciano)
            int b_anim = ((s_anim_timer + p * 8) / 5) % 3;
            u32 boss_fire = (b_anim == 0) ? 0x22C55EFF : ((b_anim == 1) ? 0x10B981FF : 0x06B6D4FF);
            draw_rect_blend(p_sx + 5, p_sy + 1, 6, 6, boss_fire);
            draw_rect_blend(p_sx + 6, p_sy + 2, 4, 4, 0xFFFFFFFF);
        }

        // Portal Mágico de Teletransporte de Saída (Spawned após derrotar o Boss)
        if (s_dungeon.boss_portal_spawned) {
            int pt_sx, pt_sy;
            map_world_to_screen(cam, s_dungeon.portal_x, s_dungeon.portal_y, &pt_sx, &pt_sy);

            // Anéis concêntricos girando e pulsando em ciano e azul cobalto
            float time_f = (float)s_anim_timer * 0.12f;
            for (int ring = 1; ring <= 3; ring++) {
                float rad_base = (float)(ring * 7) + sinf(time_f + (float)ring) * 2.0f;
                u32 pcol = (ring == 1) ? 0xFFFFFFFF : ((ring == 2) ? 0x38BDF8FF : 0x1D4ED8FF);
                for (int d = 0; d < 360; d += 15) {
                    float a = ((float)d * (PI_F / 180.0f)) + (time_f * (ring % 2 == 0 ? 1.0f : -1.0f));
                    int px = pt_sx + (int)(cosf(a) * rad_base);
                    int py = pt_sy + (int)(sinf(a) * rad_base * 0.65f);
                    hal_video_put_pixel(px, py, pcol);
                    hal_video_put_pixel(px + 1, py, pcol);
                }
            }

            hal_video_put_pixel(pt_sx, pt_sy, 0xFFFFFFFF);
            hal_video_put_pixel(pt_sx - 1, pt_sy, 0x38BDF8FF);
            hal_video_put_pixel(pt_sx + 1, pt_sy, 0x38BDF8FF);

            font_draw_text(pt_sx - 24, pt_sy - 22, "PORTAL DE SAIDA", 0x38BDF8FF, true);
        }
    }

    // 3. Tochas Animadas nas Paredes
    int torch_anim = (s_anim_timer / 6) % 3;
    u32 fire_col = (torch_anim == 0) ? C_DUNG_FIRE_1 : ((torch_anim == 1) ? C_DUNG_FIRE_2 : C_DUNG_FIRE_3);

    int t1_x, t1_y, t2_x, t2_y;
    map_world_to_screen(cam, 3.0f * TILE_SIZE, 1.0f * TILE_SIZE, &t1_x, &t1_y);
    map_world_to_screen(cam, 12.0f * TILE_SIZE, 1.0f * TILE_SIZE, &t2_x, &t2_y);

    draw_rect_blend(t1_x + 6, t1_y + 8, 4, 8, C_DUNG_TORCH_WOOD);
    draw_rect_blend(t2_x + 6, t2_y + 8, 4, 8, C_DUNG_TORCH_WOOD);
    draw_rect_blend(t1_x + 5, t1_y + 3, 6, 6, fire_col);
    draw_rect_blend(t2_x + 5, t2_y + 3, 6, 6, fire_col);
}

void dungeon_render_transition(const Camera* cam) {
    if (!s_dungeon.active || !cam) return;

    // Efeito de Wipe / Fade Preto na Transição de Telas
    if (s_dungeon.transitioning) {
        int alpha = (s_dungeon.trans_timer > 10) ?
                    (int)((20 - s_dungeon.trans_timer) * 25.5f) :
                    (int)(s_dungeon.trans_timer * 25.5f);
        if (alpha > 255) alpha = 255;
        if (alpha < 0) alpha = 0;
        draw_rect_blend(0, 0, cam->viewport_w, cam->viewport_h, (alpha & 0xFF));
    }
}

void dungeon_render_hud_keys(int x, int y) {
    if (s_dungeon.small_keys <= 0 && !s_dungeon.active) return;

    // Mini ícone de chave dourada
    draw_rect_blend(x, y + 2, 4, 3, 0xFFD700FF);
    hal_video_put_pixel(x + 1, y + 3, 0x111111FF);
    draw_rect_blend(x + 3, y + 3, 4, 1, 0xFFD700FF);
    hal_video_put_pixel(x + 5, y + 4, 0xFFD700FF);
    hal_video_put_pixel(x + 7, y + 4, 0xFFD700FF);

    char count_str[8];
    snprintf(count_str, sizeof(count_str), "x%d", s_dungeon.small_keys);
    font_draw_text(x + 9, y, count_str, 0xFFE27AFF, false);
}
