/*
 * ============================================================================
 * src/hal/dungeon_flames.c - Implementação da Masmorra: Cave of Flames (Dungeon 2)
 * ============================================================================
 */

#include "hal/dungeon_flames.h"
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

// Paleta de Cores Vulcânica da Cave of Flames
#define C_FLAMES_WALL_TOP    0x382F2DFF // Rocha vulcânica escura topo
#define C_FLAMES_WALL_FACE   0x1C1917FF // Rocha vulcânica frontal
#define C_FLAMES_WALL_DARK   0x0C0A09FF // Argamassa basáltica e sombras
#define C_FLAMES_FLOOR_BASE  0x292524FF // Lajes de basalto polido
#define C_FLAMES_FLOOR_CRK   0x44403CFF // Fissuras e juntas no basalto
#define C_FLAMES_LAVA_CORE   0xFEF08AFF // Núcleo incandescente de lava
#define C_FLAMES_LAVA_BRIGHT 0xF59E0BFF // Magma alaranjado vivo
#define C_FLAMES_LAVA_DARK   0x991B1BFF // Crosta vulcânica avermelhada
#define C_FLAMES_RAIL_WOOD   0x78350FFF // Dormentes de madeira nos trilhos
#define C_FLAMES_RAIL_IRON   0x94A3B8FF // Trilhos de ferro fundido
#define C_FLAMES_CART_BODY   0x475569FF // Chapa de ferro da vagoneta
#define C_FLAMES_CART_RIM    0xD97706FF // Borda reforçada de latão
#define C_FLAMES_BARREL      0x334155FF // Cilindro de ferro escuro do barril
#define C_FLAMES_SPIKE       0xE2E8F0FF // Espinhos de aço afiados
#define C_FLAMES_TORCH       0x52525BFF // Suporte de pedra dos braseiros
#define C_FLAMES_FIRE_1      0xEF4444FF // Fogo vermelho
#define C_FLAMES_FIRE_2      0xF97316FF // Fogo laranja
#define C_FLAMES_FIRE_3      0xFDE047FF // Fogo amarelo/branco
#define C_FLAMES_IRON_GATE   0x64748BFF // Ferro das grades levadiças
#define C_FLAMES_GOLD_LOCK   0xFBBF24FF // Cadeado dourado

static DungeonFlamesState s_flames = { 0 };
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

void dungeon_flames_init(void) {
    memset(&s_flames, 0, sizeof(s_flames));
    s_flames.active = false;
    s_flames.current_room = ROOM_FLAMES_ENTRANCE;
    s_flames.small_keys = 0;
    s_flames.lava_cooldown = 0;

    // Configuração Sala 0 (Vestíbulo de Entrada)
    s_flames.switch_entrance_down = false;
    s_flames.door_entrance_open = false;
    s_flames.door_north_entrance.x = 7.0f * TILE_SIZE;
    s_flames.door_north_entrance.y = 0.0f;
    s_flames.door_north_entrance.is_locked = false;
    s_flames.door_north_entrance.is_open = false;

    // Configuração Sala 1 (Lago de Lava e Vagoneta)
    s_flames.cart.x = 7.5f * TILE_SIZE;
    s_flames.cart.y = 7.0f * TILE_SIZE;
    s_flames.cart.vx = 0.0f;
    s_flames.cart.vy = 0.0f;
    s_flames.cart.flipped = true; // Começa emborcada (requer Cajado de Pacci!)
    s_flames.cart.is_riding = false;
    s_flames.cart.track_endpoint = 0; // 0 = Estação Sul (y=7.0), 1 = Estação Norte (y=2.0)
    s_flames.cart.flip_anim = 0.0f;

    // Configuração Sala 2 (Cilindro Espinhoso Giratório)
    s_flames.barrel.x = 4.0f * TILE_SIZE;
    s_flames.barrel.y = 4.0f * TILE_SIZE;
    s_flames.barrel.start_x = 3.0f * TILE_SIZE;
    s_flames.barrel.target_x = 11.5f * TILE_SIZE;
    s_flames.barrel.speed = 1.4f;
    s_flames.barrel.direction = 1;

    // Configuração Sala 3 (Crisol de Combate)
    s_flames.crucible_cleared = false;
    s_flames.key_spawned = false;
    s_flames.key_collected = false;
    s_flames.key_x = 7.5f * TILE_SIZE;
    s_flames.key_y = 4.5f * TILE_SIZE;
    s_flames.door_crucible_shutter.x = 7.0f * TILE_SIZE;
    s_flames.door_crucible_shutter.y = 0.0f;
    s_flames.door_crucible_shutter.is_locked = false;
    s_flames.door_crucible_shutter.is_open = true;

    // Configuração Sala 4 (Antecâmara do Chefe Gleerok)
    s_flames.door_boss_locked.x = 7.0f * TILE_SIZE;
    s_flames.door_boss_locked.y = 0.0f;
    s_flames.door_boss_locked.is_locked = true;
    s_flames.door_boss_locked.is_open = false;

    // Configuração Sala 5 (Arena do Chefe Gleerok)
    s_flames.door_boss_shutter.x = 7.0f * TILE_SIZE;
    s_flames.door_boss_shutter.y = 9.0f * TILE_SIZE;
    s_flames.door_boss_shutter.is_locked = false;
    s_flames.door_boss_shutter.is_open = true;
    s_flames.boss_chamber_entered = false;
    s_flames.boss_cleared = false;
    s_flames.boss_portal_spawned = false;
    s_flames.portal_x = 7.5f * TILE_SIZE;
    s_flames.portal_y = 6.5f * TILE_SIZE;
    s_flames.fire_element_spawned = false;
    s_flames.fire_element_collected = false;
    s_flames.fire_element_x = 7.5f * TILE_SIZE;
    s_flames.fire_element_y = 4.5f * TILE_SIZE;

    // Ponto seguro inicial
    s_flames.safe_x = 7.5f * TILE_SIZE;
    s_flames.safe_y = 8.0f * TILE_SIZE;
}

bool dungeon_flames_is_active(void) {
    return s_flames.active;
}

DungeonFlamesState* dungeon_flames_get_state(void) {
    return &s_flames;
}

void dungeon_flames_enter(float* link_x, float* link_y, Direction* link_dir) {
    s_flames.active = true;
    s_flames.current_room = ROOM_FLAMES_ENTRANCE;
    s_flames.transitioning = false;
    s_flames.safe_x = 7.5f * TILE_SIZE;
    s_flames.safe_y = 8.0f * TILE_SIZE;

    if (link_x) *link_x = 7.5f * TILE_SIZE;
    if (link_y) *link_y = 8.0f * TILE_SIZE; // Próximo ao arco sul
    if (link_dir) *link_dir = DIR_UP;

    hal_audio_play_bgm(BGM_CAVE_OF_FLAMES);
    hal_audio_play_sound(SOUND_SECRET, 0.85f, 1.0f);
    printf("[CAVE OF FLAMES] Link adentrou a Masmorra de Fogo (Vestibulo de Entrada)!\n");
}

void dungeon_flames_exit(float* link_x, float* link_y, Direction* link_dir) {
    s_flames.active = false;
    s_flames.transitioning = false;

    // Posição de retorno nas Minas de Melari (em frente ao portal em arco ao norte)
    if (link_x) *link_x = 248.0f;
    if (link_y) *link_y = 48.0f;
    if (link_dir) *link_dir = DIR_DOWN;

    hal_audio_play_sound(SOUND_SECRET, 0.8f, 1.2f);
    printf("[CAVE OF FLAMES] Link retornou para as Minas de Melari!\n");
}

static void trigger_room_transition(DungeonFlamesRoomId next_room, float spawn_x, float spawn_y) {
    s_flames.transitioning = true;
    s_flames.trans_timer = 20;
    s_flames.target_room = next_room;
    s_flames.target_spawn_x = spawn_x;
    s_flames.target_spawn_y = spawn_y;
    s_flames.safe_x = spawn_x;
    s_flames.safe_y = spawn_y;
    hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.70f, 1.1f);
}

bool dungeon_flames_check_pacci_hit(float px, float py, float pw, float ph) {
    if (!s_flames.active) return false;
    if (s_flames.current_room != ROOM_FLAMES_MINECART) return false;

    if (s_flames.cart.flipped) {
        float cx1 = s_flames.cart.x - 2.0f;
        float cy1 = s_flames.cart.y - 2.0f;
        float cx2 = cx1 + 20.0f;
        float cy2 = cy1 + 18.0f;

        if (px + pw > cx1 && px < cx2 && py + ph > cy1 && py < cy2) {
            s_flames.cart.flipped = false;
            s_flames.cart.flip_anim = 16.0f;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
            printf("[CANE OF PACCI] Vagoneta da Cave of Flames desvirada com sucesso!\n");
            return true;
        }
    }
    return false;
}

bool dungeon_flames_is_link_riding_cart(void) {
    return s_flames.active && s_flames.cart.is_riding;
}

void dungeon_flames_update(float* link_x, float* link_y, Direction* link_dir, bool is_moving,
                           int* link_hearts, int* link_rupees) {
    if (!s_flames.active) return;

    s_anim_timer++;
    if (s_flames.lava_cooldown > 0) s_flames.lava_cooldown--;
    if (s_flames.cart.flip_anim > 0.0f) s_flames.cart.flip_anim -= 1.0f;

    // Transição de tela entre salas
    if (s_flames.transitioning) {
        s_flames.trans_timer--;
        if (s_flames.trans_timer == 10) {
            s_flames.current_room = s_flames.target_room;
            if (link_x) *link_x = s_flames.target_spawn_x;
            if (link_y) *link_y = s_flames.target_spawn_y;
            s_flames.safe_x = s_flames.target_spawn_x;
            s_flames.safe_y = s_flames.target_spawn_y;

            // Limpa entidades ao mudar de câmara
            entity_clear_all();

            // Spawn de inimigos específicos ao entrar na sala
            if (s_flames.current_room == ROOM_FLAMES_CRUCIBLE && !s_flames.crucible_cleared) {
                entity_spawn(ENTITY_ENEMY_FIRE_KEESE, 5.0f * TILE_SIZE, 3.0f * TILE_SIZE);
                entity_spawn(ENTITY_ENEMY_FIRE_KEESE, 10.0f * TILE_SIZE, 3.0f * TILE_SIZE);
                entity_spawn(ENTITY_ENEMY_SPINY_BEETLE, 7.5f * TILE_SIZE, 5.0f * TILE_SIZE);
                s_flames.door_crucible_shutter.is_open = false; // Fecha as grades
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.9f, 0.85f);
                hal_audio_play_sound(SOUND_KEESE_CHIRP, 0.9f, 0.9f);
                printf("[CAVE OF FLAMES] Link entrou no Crisol! Portas trancadas e monstros surgem!\n");
            }
        }
        if (s_flames.trans_timer <= 0) {
            s_flames.transitioning = false;
        }
        return;
    }

    if (!link_x || !link_y || !link_dir) return;
    float lx = *link_x;
    float ly = *link_y;

    // ------------------------------------------------------------------------
    // FÍSICA E ATUALIZAÇÃO DA VAGONETA (MINECART)
    // ------------------------------------------------------------------------
    if (s_flames.current_room == ROOM_FLAMES_MINECART) {
        Minecart* c = &s_flames.cart;

        // Se Link estiver montado na vagoneta
        if (c->is_riding) {
            float speed = 2.2f;
            if (c->track_endpoint == 0) {
                // Viajando do Sul para o Norte
                c->y -= speed;
                if (c->y <= 2.0f * TILE_SIZE) {
                    c->y = 2.0f * TILE_SIZE;
                    c->is_riding = false;
                    c->track_endpoint = 1;
                    *link_x = c->x;
                    *link_y = c->y - 12.0f; // Desembarca ao norte
                    s_flames.safe_x = *link_x;
                    s_flames.safe_y = *link_y;
                    hal_audio_play_sound(SOUND_ITEM_CATCH, 0.8f, 1.2f);
                    printf("[MINECART] Vagoneta alcancou a estacao norte! Link desembarcou.\n");
                }
            } else {
                // Viajando do Norte para o Sul
                c->y += speed;
                if (c->y >= 7.0f * TILE_SIZE) {
                    c->y = 7.0f * TILE_SIZE;
                    c->is_riding = false;
                    c->track_endpoint = 0;
                    *link_x = c->x;
                    *link_y = c->y + 12.0f; // Desembarca ao sul
                    s_flames.safe_x = *link_x;
                    s_flames.safe_y = *link_y;
                    hal_audio_play_sound(SOUND_ITEM_CATCH, 0.8f, 1.2f);
                    printf("[MINECART] Vagoneta alcancou a estacao sul! Link desembarcou.\n");
                }
            }

            if (c->is_riding) {
                *link_x = c->x;
                *link_y = c->y;
                if ((s_anim_timer % 12) == 0) {
                    hal_audio_play_sound(SOUND_BLOCK_PUSH, 0.45f, 1.5f);
                }
            }
            return; // Impede locomoção livre de Link durante a viagem
        }

        // Se a vagoneta estiver desvirada e Link encostar nela para subir
        if (!c->flipped && !c->is_riding) {
            float dist = sqrtf((lx - c->x) * (lx - c->x) + (ly - c->y) * (ly - c->y));
            if (dist < 10.0f) {
                c->is_riding = true;
                hal_audio_play_sound(SOUND_ROLL, 0.9f, 1.2f);
                printf("[MINECART] Link subiu na vagoneta! Iniciando trajeto nos trilhos...\n");
                return;
            }
        }
    }

    // ------------------------------------------------------------------------
    // CILINDRO ESPINHOSO GIRATÓRIO (ROLLING SPIKED BARREL) NA SALA 2
    // ------------------------------------------------------------------------
    if (s_flames.current_room == ROOM_FLAMES_BARREL) {
        RollingBarrel* b = &s_flames.barrel;
        b->x += b->speed * b->direction;
        if (b->direction > 0 && b->x >= b->target_x) {
            b->x = b->target_x;
            b->direction = -1;
            hal_audio_play_sound(SOUND_SWORD_HIT, 0.5f, 0.7f);
        } else if (b->direction < 0 && b->x <= b->start_x) {
            b->x = b->start_x;
            b->direction = 1;
            hal_audio_play_sound(SOUND_SWORD_HIT, 0.5f, 0.7f);
        }

        // Colisão de dano com Link
        float bx1 = b->x;
        float by1 = b->y;
        float bx2 = bx1 + 44.0f;
        float by2 = by1 + 18.0f;

        float lx1 = lx + 2.0f;
        float ly1 = ly + 4.0f;
        float lx2 = lx + 14.0f;
        float ly2 = ly + 14.0f;

        if (lx1 < bx2 && lx2 > bx1 && ly1 < by2 && ly2 > by1) {
            if (s_flames.lava_cooldown <= 0 && link_hearts) {
                *link_hearts -= 2; // 1 Coração de dano
                s_flames.lava_cooldown = 40;
                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 0.8f);
                // Empurrão de recuo
                *link_y += (ly > (by1 + 9.0f)) ? 8.0f : -8.0f;
                printf("[CAVE OF FLAMES] Link foi atingido pelo Cilindro Espinhoso Giratorio! -1 Coracao!\n");
            }
        }
    }

    // ------------------------------------------------------------------------
    // SALA 0: VESTÍBULO DE ENTRADA
    // ------------------------------------------------------------------------
    if (s_flames.current_room == ROOM_FLAMES_ENTRANCE) {
        // Saída sul para as Minas de Melari
        if (ly >= 8.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            dungeon_flames_exit(link_x, link_y, link_dir);
            return;
        }

        // Interruptor mecânico de piso em x=4, y=4
        float sw_cx = 4.0f * TILE_SIZE + 8.0f;
        float sw_cy = 4.0f * TILE_SIZE + 8.0f;
        bool link_on_sw = (fabsf(lx + 8.0f - sw_cx) <= 10.0f && fabsf(ly + 12.0f - sw_cy) <= 10.0f);

        if (link_on_sw && !s_flames.switch_entrance_down) {
            s_flames.switch_entrance_down = true;
            s_flames.door_north_entrance.is_open = true;
            hal_audio_play_sound(SOUND_SWITCH_CLICK, 0.95f, 1.0f);
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.85f, 1.0f);
            printf("[CAVE OF FLAMES] Interruptor de ferro acionado! Grades norte levantadas!\n");
        }

        // Cruzar a porta norte aberta para a Sala 1
        if (s_flames.door_north_entrance.is_open && ly <= 1.0f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_FLAMES_MINECART, 7.5f * TILE_SIZE, 8.0f * TILE_SIZE);
            return;
        }
    }

    // ------------------------------------------------------------------------
    // SALA 1: LAGO DE LAVA & VAGONETA
    // ------------------------------------------------------------------------
    else if (s_flames.current_room == ROOM_FLAMES_MINECART) {
        // Saída sul de volta para Sala 0
        if (ly >= 8.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_FLAMES_ENTRANCE, 7.5f * TILE_SIZE, 1.5f * TILE_SIZE);
            return;
        }

        // Saída norte para Sala 2
        if (ly <= 1.0f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_FLAMES_BARREL, 7.5f * TILE_SIZE, 8.0f * TILE_SIZE);
            return;
        }

        // Dano de Lava: Lago no centro (y >= 3.0f * 16 && y <= 6.5f * 16) fora dos trilhos
        if (ly >= 3.0f * TILE_SIZE && ly <= 6.5f * TILE_SIZE && !s_flames.cart.is_riding) {
            if (s_flames.lava_cooldown <= 0 && link_hearts) {
                *link_hearts -= 2;
                s_flames.lava_cooldown = 40;
                hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 1.0f, 0.5f);
                *link_x = s_flames.safe_x;
                *link_y = s_flames.safe_y;
                printf("[CAVE OF FLAMES] Link caiu na lava ardente! -1 Coracao! Retornado a area segura.\n");
            }
        } else {
            // Atualiza área segura
            s_flames.safe_x = lx;
            s_flames.safe_y = ly;
        }
    }

    // ------------------------------------------------------------------------
    // SALA 2: CORREDOR DO BARRIL ESPINHOSO
    // ------------------------------------------------------------------------
    else if (s_flames.current_room == ROOM_FLAMES_BARREL) {
        // Saída sul de volta para Sala 1
        if (ly >= 8.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_FLAMES_MINECART, 7.5f * TILE_SIZE, 1.5f * TILE_SIZE);
            return;
        }

        // Saída leste para Sala 3 (Crisol de Combate)
        if (lx >= 14.5f * TILE_SIZE && ly >= 3.5f * TILE_SIZE && ly <= 5.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_FLAMES_CRUCIBLE, 1.5f * TILE_SIZE, 4.5f * TILE_SIZE);
            return;
        }
    }

    // ------------------------------------------------------------------------
    // SALA 3: CRISOL VULCÂNICO DE COMBATE (FIRE KEESE & SPINY BEETLE)
    // ------------------------------------------------------------------------
    else if (s_flames.current_room == ROOM_FLAMES_CRUCIBLE) {
        // Saída oeste de volta para Sala 2
        if (s_flames.door_crucible_shutter.is_open && lx <= 1.0f * TILE_SIZE && ly >= 3.5f * TILE_SIZE && ly <= 5.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_FLAMES_BARREL, 14.0f * TILE_SIZE, 4.5f * TILE_SIZE);
            return;
        }

        // Verificação de limpeza dos monstros
        if (!s_flames.crucible_cleared) {
            if (entity_count_active_enemies() == 0) {
                s_flames.crucible_cleared = true;
                s_flames.key_spawned = true;
                s_flames.door_crucible_shutter.is_open = true;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.9f, 1.0f);
                printf("[CAVE OF FLAMES] Crisol conquistado! Chave Pequena caiu do teto!\n");
            }
        }

        // Coleta da Chave Pequena
        if (s_flames.key_spawned && !s_flames.key_collected) {
            float dist = sqrtf((lx - s_flames.key_x) * (lx - s_flames.key_x) + (ly - s_flames.key_y) * (ly - s_flames.key_y));
            if (dist < 12.0f) {
                s_flames.key_collected = true;
                s_flames.small_keys++;
                hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.4f);
                printf("[CAVE OF FLAMES] Chave Pequena obtida! Total: %d\n", s_flames.small_keys);
            }
        }

        // Avanço norte para Sala 4 (Antecâmara do Chefe)
        if (s_flames.door_crucible_shutter.is_open && ly <= 1.0f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_FLAMES_BOSS_DOOR, 7.5f * TILE_SIZE, 8.0f * TILE_SIZE);
            return;
        }
    }

    // ------------------------------------------------------------------------
    // SALA 4: ANTECÂMARA DO PORTÃO DO CHEFE
    // ------------------------------------------------------------------------
    else if (s_flames.current_room == ROOM_FLAMES_BOSS_DOOR) {
        // Saída sul de volta para Sala 3
        if (ly >= 8.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            trigger_room_transition(ROOM_FLAMES_CRUCIBLE, 7.5f * TILE_SIZE, 1.5f * TILE_SIZE);
            return;
        }

        // Interação com o Portão Trancado do Chefe
        if (ly <= 1.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
            if (!s_flames.door_boss_locked.is_open) {
                if (s_flames.small_keys > 0) {
                    s_flames.small_keys--;
                    s_flames.door_boss_locked.is_open = true;
                    hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
                    hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.9f, 0.8f);
                    printf("[CAVE OF FLAMES] Portao do Chefe destrancado com a Chave Pequena!\n");
                }
            } else {
                // Atravessar portão aberto rumo ao Chefe Gleerok
                trigger_room_transition(ROOM_FLAMES_BOSS_ARENA, 7.5f * TILE_SIZE, 8.0f * TILE_SIZE);
                return;
            }
        }
    }

    // ------------------------------------------------------------------------
    // SALA 5: ARENA DE COMBATE CONTRA O CHEFE GLEEROK
    // ------------------------------------------------------------------------
    else if (s_flames.current_room == ROOM_FLAMES_BOSS_ARENA) {
        // Inicialização ao entrar pela primeira vez
        if (!s_flames.boss_chamber_entered) {
            s_flames.boss_chamber_entered = true;
            s_flames.door_boss_shutter.is_open = false; // Fecha a grade de ferro atrás do herói
            entity_clear_all();
            // Spawn do Chefe Gleerok na piscina de magma ao norte
            entity_spawn(ENTITY_BOSS_GLEEROK, 7.0f * TILE_SIZE - 20.0f, 1.8f * TILE_SIZE);
            hal_audio_play_bgm(BGM_BOSS_BATTLE);
            hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.9f);
            entity_trigger_screen_shake(20, 5);
            printf("[CAVE OF FLAMES] GLEEROK EMERGE DAS PROFUNDEZAS DO MAGMA!\n");
        }

        // Verificação se o chefe foi derrotado
        if (!s_flames.boss_cleared) {
            // Se não houver inimigos ativos e já tiver entrado, chefe derrotado!
            if (entity_count_active_enemies() == 0 && s_flames.boss_chamber_entered) {
                s_flames.boss_cleared = true;
                s_flames.door_boss_shutter.is_open = true; // Reabre o portão sul
                hal_audio_play_bgm(BGM_CAVE_OF_FLAMES);
                hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 1.0f);
                printf("[CAVE OF FLAMES] GLEEROK DERROTADO! Cristal e Receptaculo de Coracao gerados!\n");

                // Spawn do Heart Container e Elemento Fogo
                entity_spawn(ENTITY_ITEM_HEART_CONTAINER, 5.5f * TILE_SIZE, 5.5f * TILE_SIZE);
                entity_spawn(ENTITY_ITEM_FIRE_ELEMENT, s_flames.fire_element_x, s_flames.fire_element_y);
                s_flames.boss_portal_spawned = true;
            }
        }

        // Se o chefe foi derrotado:
        if (s_flames.boss_cleared) {
            // Checagem se Link coletou o Elemento Fogo
            if (!s_flames.fire_element_collected) {
                float dx = lx - s_flames.fire_element_x;
                float dy = ly - s_flames.fire_element_y;
                if (dx * dx + dy * dy <= 18.0f * 18.0f) {
                    s_flames.fire_element_collected = true;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                    printf("[CAVE OF FLAMES] Link recolheu o Sagrado Elemento Fogo!\n");
                }
            }

            // Checagem se Link pisa no Portal Azul de Retorno
            if (s_flames.boss_portal_spawned) {
                float px = s_flames.portal_x;
                float py = s_flames.portal_y;
                float dist_portal = sqrtf((lx - px) * (lx - px) + (ly - py) * (ly - py));
                if (dist_portal < 14.0f) {
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                    dungeon_flames_exit(link_x, link_y, link_dir);
                    return;
                }
            }

            // Saída sul de volta para Sala 4 (se o portão estiver aberto)
            if (ly >= 8.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
                trigger_room_transition(ROOM_FLAMES_BOSS_DOOR, 7.5f * TILE_SIZE, 2.0f * TILE_SIZE);
                return;
            }
        }

        // Dano de lava na Arena do Chefe:
        // A ilha de pedra segura é colunas 2 a 13, linhas 3 a 8
        int col = (int)(lx / TILE_SIZE);
        int row = (int)(ly / TILE_SIZE);
        if (col < 2 || col > 13 || row < 3 || row > 8) {
            if (s_flames.lava_cooldown <= 0) {
                s_flames.lava_cooldown = 40;
                if (link_hearts && *link_hearts > 1) (*link_hearts)--;
                *link_x = s_flames.safe_x;
                *link_y = s_flames.safe_y;
                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 0.8f);
                entity_trigger_screen_shake(12, 3);
                printf("[LAVA ARENA] Link caiu no magma fervente ao redor da ilha!\n");
            }
        }
    }
}

bool dungeon_flames_interact(float x, float y, int* link_rupees, int* link_hearts) {
    (void)link_rupees;
    (void)link_hearts;
    if (!s_flames.active) return false;

    // Desvirar vagoneta emborcada se Link interagir diretamente próximo
    if (s_flames.current_room == ROOM_FLAMES_MINECART && s_flames.cart.flipped) {
        float dist = sqrtf((x - s_flames.cart.x) * (x - s_flames.cart.x) + (y - s_flames.cart.y) * (y - s_flames.cart.y));
        if (dist < 18.0f) {
            s_flames.cart.flipped = false;
            s_flames.cart.flip_anim = 16.0f;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
            printf("[CAVE OF FLAMES] Vagoneta colocada de pe sobre os trilhos!\n");
            return true;
        }
    }

    return false;
}

bool dungeon_flames_is_solid(float world_x, float world_y) {
    if (!s_flames.active) return false;

    int col = (int)(world_x / TILE_SIZE);
    int row = (int)(world_y / TILE_SIZE);

    if (col < 0 || col >= FLAMES_ROOM_W || row < 0 || row >= FLAMES_ROOM_H) {
        return true;
    }

    // Paredes perimetrais Norte, Sul, Leste e Oeste
    // Linha 0 (Topo Norte)
    if (row <= 0) {
        // Portas / aberturas ao norte em x=7 e x=8
        if (col == 7 || col == 8) {
            if (s_flames.current_room == ROOM_FLAMES_ENTRANCE) {
                return !s_flames.door_north_entrance.is_open;
            } else if (s_flames.current_room == ROOM_FLAMES_MINECART) {
                return false; // Abertura livre para Sala 2
            } else if (s_flames.current_room == ROOM_FLAMES_CRUCIBLE) {
                return !s_flames.door_crucible_shutter.is_open;
            } else if (s_flames.current_room == ROOM_FLAMES_BOSS_DOOR) {
                return !s_flames.door_boss_locked.is_open;
            } else if (s_flames.current_room == ROOM_FLAMES_BOSS_ARENA) {
                return true; // Parede norte sólida na arena
            }
        }
        return true;
    }

    // Linha 9 (Fundo Sul)
    if (row >= 9) {
        if (col == 7 || col == 8) {
            if (s_flames.current_room == ROOM_FLAMES_BOSS_ARENA) {
                return !s_flames.door_boss_shutter.is_open;
            }
            return false; // Portais de passagem sul
        }
        return true;
    }

    // Piscina profunda de magma do Gleerok ao norte da arena (linhas 1 e 2, col 4 a 11)
    if (s_flames.current_room == ROOM_FLAMES_BOSS_ARENA) {
        if (row <= 2 && col >= 4 && col <= 11) return true;
    }

    // Coluna 0 (Parede Oeste)
    if (col <= 0) {
        if (s_flames.current_room == ROOM_FLAMES_CRUCIBLE && (row == 4 || row == 5)) {
            return !s_flames.door_crucible_shutter.is_open;
        }
        return true;
    }

    // Coluna 15 (Parede Leste)
    if (col >= 15) {
        if (s_flames.current_room == ROOM_FLAMES_BARREL && (row == 4 || row == 5)) {
            return false; // Passagem leste para Sala 3
        }
        return true;
    }

    // Pilares de pedra no Vestíbulo (Sala 0)
    if (s_flames.current_room == ROOM_FLAMES_ENTRANCE) {
        if ((col == 4 || col == 11) && (row == 2 || row == 3)) return true;
    }

    return false;
}

// ----------------------------------------------------------------------------
// RENDERIZAÇÃO DOS ELEMENTOS DA CAVE OF FLAMES
// ----------------------------------------------------------------------------
static void render_torch(int sx, int sy, int anim_tick) {
    // Pedestal de pedra
    draw_filled_rect(sx + 5, sy + 7, 6, 9, C_FLAMES_TORCH);
    draw_filled_rect(sx + 4, sy + 13, 8, 3, C_FLAMES_WALL_DARK);

    // Chamas animadas
    int f1 = (anim_tick / 4) % 3;
    u32 cf = (f1 == 0) ? C_FLAMES_FIRE_1 : ((f1 == 1) ? C_FLAMES_FIRE_2 : C_FLAMES_FIRE_3);
    draw_filled_rect(sx + 5, sy + 2 + f1, 6, 5, cf);
    draw_filled_rect(sx + 6, sy + 1, 4, 3, C_FLAMES_FIRE_3);

    // Brilho radial suave em torno da tocha
    draw_rect_blend(sx - 8, sy - 8, 32, 32, 0xF59E0B22);
}

static void render_minecart(int sx, int sy, bool flipped, float flip_anim) {
    if (flip_anim > 0.0f) {
        // Rotação mágica do Cajado de Pacci
        draw_filled_rect(sx + 2, sy + 2, 14, 12, 0x38BDF8AA);
    }

    if (flipped) {
        // Vagoneta Emborcada (Fundo de ferro curvado para cima)
        draw_filled_rect(sx + 1, sy + 3, 16, 9, C_FLAMES_CART_BODY);
        draw_filled_rect(sx + 3, sy + 1, 12, 3, C_FLAMES_CART_RIM);
        // Rodas viradas para cima
        draw_filled_rect(sx + 2, sy, 3, 3, 0x1E293BFF);
        draw_filled_rect(sx + 13, sy, 3, 3, 0x1E293BFF);
        // Rebites
        draw_filled_rect(sx + 4, sy + 5, 2, 2, 0x94A3B8FF);
        draw_filled_rect(sx + 12, sy + 5, 2, 2, 0x94A3B8FF);
    } else {
        // Vagoneta Em Pé (Pronta para embarcar)
        draw_filled_rect(sx + 1, sy + 3, 16, 11, C_FLAMES_CART_BODY);
        draw_filled_rect(sx + 2, sy + 2, 14, 3, C_FLAMES_CART_RIM);
        draw_filled_rect(sx + 4, sy + 5, 10, 5, 0x0F172AFF); // Interior oco
        // Rodas sobre os trilhos
        draw_filled_rect(sx + 2, sy + 13, 3, 3, 0x1E293BFF);
        draw_filled_rect(sx + 13, sy + 13, 3, 3, 0x1E293BFF);
        // Rebites dourados
        draw_filled_rect(sx + 3, sy + 7, 2, 2, C_FLAMES_CART_RIM);
        draw_filled_rect(sx + 13, sy + 7, 2, 2, C_FLAMES_CART_RIM);
    }
}

static void render_spiked_barrel(int sx, int sy, int anim_tick) {
    // Cilindro central de ferro (44x18)
    draw_filled_rect(sx, sy + 3, 44, 12, C_FLAMES_BARREL);
    draw_filled_rect(sx + 2, sy + 5, 40, 2, 0x475569FF); // Friso de reflexo metálico
    draw_filled_rect(sx + 2, sy + 11, 40, 3, 0x0F172AFF); // Sombra inferior

    // Espinhos pontiagudos de aço girando
    int phase = (anim_tick / 3) % 6;
    for (int i = 0; i < 5; i++) {
        int sp_x = sx + 4 + (i * 8);
        int sp_y_top = sy + 3 - ((phase + i) % 3);
        int sp_y_bot = sy + 14 + ((phase + i) % 3);

        // Espinho superior
        draw_filled_rect(sp_x, sp_y_top, 3, 3, C_FLAMES_SPIKE);
        // Espinho inferior
        draw_filled_rect(sp_x + 2, sp_y_bot, 3, 3, C_FLAMES_SPIKE);
    }
}

void dungeon_flames_render(const Camera* cam) {
    if (!s_flames.active) return;

    int ox = (cam ? -(int)cam->x : 0);
    int oy = (cam ? -(int)cam->y : 0);

    // 1. Renderiza o piso base de lajes de basalto e fendas
    for (int r = 0; r < FLAMES_ROOM_H; r++) {
        for (int c = 0; c < FLAMES_ROOM_W; c++) {
            int tx = ox + (c * TILE_SIZE);
            int ty = oy + (r * TILE_SIZE);

            // Piso padrão
            draw_filled_rect(tx, ty, TILE_SIZE, TILE_SIZE, C_FLAMES_FLOOR_BASE);
            draw_filled_rect(tx, ty, TILE_SIZE, 1, C_FLAMES_FLOOR_CRK);
            draw_filled_rect(tx, ty, 1, TILE_SIZE, C_FLAMES_FLOOR_CRK);

            // Detalhes especiais por sala
            // Sala 0: Canais de lava laterais
            if (s_flames.current_room == ROOM_FLAMES_ENTRANCE) {
                if (c == 1 || c == 2 || c == 13 || c == 14) {
                    u32 c_lava = ((s_anim_timer / 10 + c + r) % 2 == 0) ? C_FLAMES_LAVA_BRIGHT : C_FLAMES_LAVA_CORE;
                    draw_filled_rect(tx, ty, TILE_SIZE, TILE_SIZE, c_lava);
                    draw_filled_rect(tx + 2, ty + 2, 4, 3, C_FLAMES_LAVA_DARK);
                }
            }

            // Sala 1: Grande Lago de Lava ardente (linhas 3 a 6)
            if (s_flames.current_room == ROOM_FLAMES_MINECART) {
                if (r >= 3 && r <= 6) {
                    u32 c_lava = ((s_anim_timer / 8 + c + r) % 3 == 0) ? C_FLAMES_LAVA_CORE :
                                 (((s_anim_timer / 8 + c + r) % 3 == 1) ? C_FLAMES_LAVA_BRIGHT : C_FLAMES_LAVA_DARK);
                    draw_filled_rect(tx, ty, TILE_SIZE, TILE_SIZE, c_lava);
                    // Bolhas de lava borbulhante
                    if ((c + r + (s_anim_timer / 15)) % 5 == 0) {
                        draw_filled_rect(tx + 5, ty + 5, 5, 5, 0xFEF08AFF);
                    }
                }
            }

            // Sala 4: Poças de lava ao redor do caminho central
            if (s_flames.current_room == ROOM_FLAMES_BOSS_DOOR) {
                if ((c <= 4 || c >= 11) && r >= 2 && r <= 7) {
                    u32 c_lava = ((s_anim_timer / 9 + c) % 2 == 0) ? C_FLAMES_LAVA_BRIGHT : C_FLAMES_LAVA_DARK;
                    draw_filled_rect(tx, ty, TILE_SIZE, TILE_SIZE, c_lava);
                }
            }
        }
    }

    // 2. Trilhos da ferrovia de vagonetas (Sala 1)
    if (s_flames.current_room == ROOM_FLAMES_MINECART) {
        int rx = ox + (int)(7.5f * TILE_SIZE);
        for (int r = 1; r <= 8; r++) {
            int ry = oy + (r * TILE_SIZE);
            // Dormentes de madeira
            draw_filled_rect(rx - 6, ry + 3, 20, 3, C_FLAMES_RAIL_WOOD);
            draw_filled_rect(rx - 6, ry + 11, 20, 3, C_FLAMES_RAIL_WOOD);
            // Trilhos duplos de ferro
            draw_filled_rect(rx - 3, ry, 2, TILE_SIZE, C_FLAMES_RAIL_IRON);
            draw_filled_rect(rx + 9, ry, 2, TILE_SIZE, C_FLAMES_RAIL_IRON);
        }
        // Batentes de final de linha
        draw_filled_rect(rx - 6, oy + (int)(1.8f * TILE_SIZE), 20, 4, C_FLAMES_WALL_DARK);
        draw_filled_rect(rx - 6, oy + (int)(7.8f * TILE_SIZE), 20, 4, C_FLAMES_WALL_DARK);
    }

    // 3. Renderiza paredes circundantes
    for (int c = 0; c < FLAMES_ROOM_W; c++) {
        // Parede Norte (Linha 0)
        int px = ox + (c * TILE_SIZE);
        int py = oy;
        draw_filled_rect(px, py, TILE_SIZE, TILE_SIZE, C_FLAMES_WALL_FACE);
        draw_filled_rect(px, py, TILE_SIZE, 4, C_FLAMES_WALL_TOP);

        // Parede Sul (Linha 9)
        int spy = oy + (9 * TILE_SIZE);
        draw_filled_rect(px, spy, TILE_SIZE, TILE_SIZE, C_FLAMES_WALL_FACE);
        draw_filled_rect(px, spy, TILE_SIZE, 4, C_FLAMES_WALL_TOP);
    }
    for (int r = 0; r < FLAMES_ROOM_H; r++) {
        // Parede Oeste (Coluna 0)
        int py = oy + (r * TILE_SIZE);
        draw_filled_rect(ox, py, TILE_SIZE, TILE_SIZE, C_FLAMES_WALL_FACE);
        // Parede Leste (Coluna 15)
        draw_filled_rect(ox + (15 * TILE_SIZE), py, TILE_SIZE, TILE_SIZE, C_FLAMES_WALL_FACE);
    }

    // 4. Objetos e Mecânicas Específicas por Câmara
    // Sala 0: Tochas e Interruptor de Piso
    if (s_flames.current_room == ROOM_FLAMES_ENTRANCE) {
        render_torch(ox + 4 * TILE_SIZE, oy + 2 * TILE_SIZE, s_anim_timer);
        render_torch(ox + 11 * TILE_SIZE, oy + 2 * TILE_SIZE, s_anim_timer);

        // Interruptor de piso
        int sw_x = ox + 4 * TILE_SIZE;
        int sw_y = oy + 4 * TILE_SIZE;
        u32 sw_col = s_flames.switch_entrance_down ? 0x059669FF : 0xDC2626FF;
        draw_filled_rect(sw_x + 2, sw_y + 2, 12, 12, C_FLAMES_WALL_DARK);
        draw_filled_rect(sw_x + 4, sw_y + 4, 8, 8, sw_col);

        // Porta norte
        int d_x = ox + 7 * TILE_SIZE;
        int d_y = oy;
        if (!s_flames.door_north_entrance.is_open) {
            draw_filled_rect(d_x, d_y, 32, 16, C_FLAMES_IRON_GATE);
            for (int b = 0; b < 8; b++) {
                draw_filled_rect(d_x + (b * 4), d_y, 2, 16, 0x1E293BFF);
            }
        } else {
            draw_filled_rect(d_x, d_y, 32, 16, 0x0C0A09FF); // Abertura para sala norte
        }
    }

    // Sala 1: Vagoneta
    if (s_flames.current_room == ROOM_FLAMES_MINECART) {
        int cx = ox + (int)s_flames.cart.x;
        int cy = oy + (int)s_flames.cart.y;
        render_minecart(cx, cy, s_flames.cart.flipped, s_flames.cart.flip_anim);
    }

    // Sala 2: Cilindro Espinhoso
    if (s_flames.current_room == ROOM_FLAMES_BARREL) {
        int bx = ox + (int)s_flames.barrel.x;
        int by = oy + (int)s_flames.barrel.y;
        render_spiked_barrel(bx, by, s_anim_timer);

        // Passagem leste aberta
        int pe_x = ox + (15 * TILE_SIZE);
        int pe_y = oy + (4 * TILE_SIZE);
        draw_filled_rect(pe_x, pe_y, 16, 32, 0x0C0A09FF);
    }

    // Sala 3: Chave Pequena & Portas Levadiças
    if (s_flames.current_room == ROOM_FLAMES_CRUCIBLE) {
        // Chave Pequena com bobbing vertical
        if (s_flames.key_spawned && !s_flames.key_collected) {
            int kx = ox + (int)s_flames.key_x;
            int ky = oy + (int)s_flames.key_y + (int)(sinf((float)s_anim_timer * 0.15f) * 3.0f);
            draw_filled_rect(kx + 3, ky, 6, 4, C_FLAMES_GOLD_LOCK);
            draw_filled_rect(kx + 5, ky + 4, 2, 7, C_FLAMES_GOLD_LOCK);
            draw_filled_rect(kx + 7, ky + 7, 2, 2, C_FLAMES_GOLD_LOCK);
            draw_filled_rect(kx + 7, ky + 9, 2, 2, C_FLAMES_GOLD_LOCK);
            draw_rect_blend(kx - 4, ky - 4, 20, 20, 0xFDE04744); // Brilho áureo
        }

        // Porta norte do Crisol
        int d_x = ox + 7 * TILE_SIZE;
        int d_y = oy;
        if (!s_flames.door_crucible_shutter.is_open) {
            draw_filled_rect(d_x, d_y, 32, 16, C_FLAMES_IRON_GATE);
        } else {
            draw_filled_rect(d_x, d_y, 32, 16, 0x0C0A09FF);
        }
    }

    // Sala 4: Portão Trancado do Chefe Gleerok
    if (s_flames.current_room == ROOM_FLAMES_BOSS_DOOR) {
        int bd_x = ox + 7 * TILE_SIZE;
        int bd_y = oy;

        if (!s_flames.door_boss_locked.is_open) {
            // Portão maciço esculpido em basalto e bronze
            draw_filled_rect(bd_x, bd_y, 32, 20, 0x451A03FF);
            draw_filled_rect(bd_x + 2, bd_y + 2, 28, 16, 0x78350FFF);
            // Emblema de Chamas e Caveira
            draw_filled_rect(bd_x + 12, bd_y + 4, 8, 8, C_FLAMES_FIRE_2);
            draw_filled_rect(bd_x + 14, bd_y + 2, 4, 4, C_FLAMES_FIRE_3);
            // Grande Cadeado Dourado
            draw_filled_rect(bd_x + 13, bd_y + 11, 6, 6, C_FLAMES_GOLD_LOCK);
            draw_filled_rect(bd_x + 14, bd_y + 9, 4, 3, 0x94A3B8FF); // Alça de aço
        } else {
            // Portão escancarado revelando a fornalha do Gleerok
            draw_filled_rect(bd_x, bd_y, 32, 20, 0x7F1D1DFF);
            draw_rect_blend(bd_x - 8, bd_y, 48, 28, 0xEF444455);
        }

        render_torch(ox + 3 * TILE_SIZE, oy + 2 * TILE_SIZE, s_anim_timer);
        render_torch(ox + 12 * TILE_SIZE, oy + 2 * TILE_SIZE, s_anim_timer);
    }

    // Sala 5: Arena de Combate contra Gleerok
    if (s_flames.current_room == ROOM_FLAMES_BOSS_ARENA) {
        // Lago de lava fervente circundando a ilha de combate
        for (int r = 0; r < FLAMES_ROOM_H; r++) {
            for (int c = 0; c < FLAMES_ROOM_W; c++) {
                if (c < 2 || c > 13 || r < 3 || r > 8) {
                    int tx = ox + (c * TILE_SIZE);
                    int ty = oy + (r * TILE_SIZE);
                    u32 c_lava = ((s_anim_timer / 7 + c + r) % 3 == 0) ? C_FLAMES_LAVA_CORE :
                                 (((s_anim_timer / 7 + c + r) % 3 == 1) ? C_FLAMES_LAVA_BRIGHT : C_FLAMES_LAVA_DARK);
                    draw_filled_rect(tx, ty, TILE_SIZE, TILE_SIZE, c_lava);
                    if ((c * 3 + r * 7 + (s_anim_timer / 12)) % 6 == 0) {
                        draw_filled_rect(tx + 4, ty + 4, 6, 6, 0xFEF08AFF);
                    }
                }
            }
        }

        // Ilha de pedra central: tochas nos 4 cantos
        render_torch(ox + 2 * TILE_SIZE, oy + 3 * TILE_SIZE, s_anim_timer);
        render_torch(ox + 13 * TILE_SIZE, oy + 3 * TILE_SIZE, s_anim_timer);
        render_torch(ox + 2 * TILE_SIZE, oy + 8 * TILE_SIZE, s_anim_timer);
        render_torch(ox + 13 * TILE_SIZE, oy + 8 * TILE_SIZE, s_anim_timer);

        // Grade sul
        int gd_x = ox + 7 * TILE_SIZE;
        int gd_y = oy + 9 * TILE_SIZE;
        if (!s_flames.door_boss_shutter.is_open) {
            draw_filled_rect(gd_x, gd_y, 32, 16, C_FLAMES_IRON_GATE);
            for (int b = 0; b < 8; b++) {
                draw_filled_rect(gd_x + (b * 4), gd_y, 2, 16, 0x1E293BFF);
            }
        }

        // Portal Azul de Retorno (se o chefe foi derrotado)
        if (s_flames.boss_portal_spawned) {
            int px = ox + (int)s_flames.portal_x;
            int py = oy + (int)s_flames.portal_y;
            int anim_phase = (s_anim_timer / 4) % 6;

            draw_rect_blend(px - 14, py - 14, 28, 28, 0x0284C744);
            draw_rect_blend(px - 10, py - 10, 20, 20, 0x38BDF866);

            draw_filled_rect(px - 8 + (anim_phase % 2), py - 8 + (anim_phase % 2), 16 - (anim_phase % 2) * 2, 16 - (anim_phase % 2) * 2, 0x0284C7FF);
            draw_filled_rect(px - 5, py - 5, 10, 10, 0x38BDF8FF);
            draw_filled_rect(px - 2, py - 2, 5, 5, 0xE0F2FEFF);
            draw_filled_rect(px, py, 2, 2, 0xFFFFFFFF);
        }
    }
}

void dungeon_flames_render_transition(const Camera* cam) {
    (void)cam;
    if (!s_flames.transitioning) return;

    float alpha = 0.0f;
    if (s_flames.trans_timer > 10) {
        alpha = (float)(20 - s_flames.trans_timer) / 10.0f;
    } else {
        alpha = (float)s_flames.trans_timer / 10.0f;
    }

    u32 a_byte = (u32)(alpha * 255.0f);
    if (a_byte > 255) a_byte = 255;
    u32 black = (0x00000000) | a_byte;

    draw_rect_blend(0, 0, 240, 160, black);
}

void dungeon_flames_render_hud_keys(int x, int y) {
    if (!s_flames.active) return;

    // Ícone da Chave Pequena
    draw_filled_rect(x, y + 2, 5, 3, C_FLAMES_GOLD_LOCK);
    draw_filled_rect(x + 2, y + 5, 2, 4, C_FLAMES_GOLD_LOCK);
    draw_filled_rect(x + 4, y + 6, 1, 1, C_FLAMES_GOLD_LOCK);
    draw_filled_rect(x + 4, y + 8, 1, 1, C_FLAMES_GOLD_LOCK);

    char buf[16];
    snprintf(buf, sizeof(buf), "x%d", s_flames.small_keys);
    font_draw_text(x + 8, y, buf, 0xFDE047FF, true);
}

bool dungeon_flames_is_boss_cleared(void) {
    return s_flames.boss_cleared;
}

bool dungeon_flames_has_fire_element(void) {
    return s_flames.fire_element_collected;
}
