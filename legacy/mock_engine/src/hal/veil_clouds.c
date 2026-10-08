/*
 * ============================================================================
 * src/hal/veil_clouds.c - Veil Falls & Cloud Tops
 * ============================================================================
 */

#include "hal/veil_clouds.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/fast_travel.h"
#include "hal/dungeon_palace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

// Cores temáticas de Veil Falls e Cloud Tops
#define C_VEIL_ROCK_DARK     0x334155FF // Rocha escarpada ardósia
#define C_VEIL_ROCK_LIGHT    0x475569FF // Face rochosa iluminada
#define C_VEIL_CLIFF_TOP     0x64748BFF // Borda superior do penhasco
#define C_VEIL_WATER_DEEP    0x0369A1FF // Lago azul profundo
#define C_VEIL_WATER_FOAM    0xE0F2FEFF // Espuma branca da cachoeira
#define C_VEIL_WATER_FALL    0x38BDF8EE // Véu d'água translúcido
#define C_CLOUD_WHITE        0xF8FAFCFF // Topo fofo da nuvem
#define C_CLOUD_SHADOW       0xCBD5E1FF // Sombra inferior da nuvem
#define C_CLOUD_DARK_STORM   0x64748BFF // Nuvem cinza de tempestade escavável
#define C_CLOUD_SKY_BG       0x0284C7FF // Céu estratosférico límpido
#define C_WHIRL_YELLOW       0xFDE047FF // Redemoinho amarelo de impulso
#define C_WHIRL_RED          0xEF4444FF // Redemoinho vermelho de super-impulso
#define C_GOLD_KINSTONE      0xF59E0BFF // Kinstone dourada da Tribo do Vento
#define C_WIND_TRIBE_ROBE    0x0284C7FF // Manto azul celeste da Tribo do Vento

static VeilCloudsState s_veil = { 0 };
static int s_veil_anim_timer = 0;

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

static Tilemap* create_scene_map(VeilCloudSceneId scene) {
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;
    memset(m, 0, sizeof(Tilemap));

    m->width = VEIL_MAP_W;
    m->height = VEIL_MAP_H;
    int total = VEIL_MAP_W * VEIL_MAP_H;

    m->ground_layer = (u8*)malloc(total);
    m->overlay_layer = (u8*)malloc(total);
    m->collision_map = (u8*)malloc(total);
    m->is_authentic = false;

    memset(m->ground_layer, 0, total);
    memset(m->overlay_layer, 0xFF, total);
    memset(m->collision_map, 0, total);

    // --- Configuração Específica por Cena ---
    if (scene == VEIL_SCENE_FALLS_BASE) {
        // Paredões laterais de pedra e rio ao fundo
        for (int y = 0; y < VEIL_MAP_H; y++) {
            for (int x = 0; x < VEIL_MAP_W; x++) {
                int idx = y * VEIL_MAP_W + x;
                // Paredão rochoso escarpado à esquerda (cols 0-3) e direita (cols 12-15)
                if (x <= 3 || x >= 12) {
                    m->collision_map[idx] = 1;
                    m->ground_layer[idx] = 0x01; // Rocha
                }
                // Cachoeira torrencial no centro (cols 6-9, y=0..7)
                if (x >= 6 && x <= 9 && y <= 7) {
                    m->overlay_layer[idx] = 0x02; // Água de cachoeira
                    m->collision_map[idx] = 1;    // Não atravessável sem subir
                }
                // Lago profundo ao sul (y=7..9)
                if (y >= 7 && x >= 4 && x <= 11) {
                    m->ground_layer[idx] = 0x03; // Água profunda navegável com Nadadeiras
                }
                // Paredão escalável com Grip Ring na coluna 12 (y=1..6)
                if (x == 12 && y >= 1 && y <= 6) {
                    m->ground_layer[idx] = 0x05; // Paredão com ranhuras escalável
                    m->collision_map[idx] = 0;    // Escalável com Grip Ring
                }
            }
        }
        // Entrada da caverna para o cume no norte (x=7, y=0)
        m->collision_map[0 * VEIL_MAP_W + 7] = 0;
        m->collision_map[0 * VEIL_MAP_W + 8] = 0;
    }
    else if (scene == VEIL_SCENE_FALLS_SUMMIT) {
        // Planalto do cume de Veil Falls: bordas são abismo
        for (int y = 0; y < VEIL_MAP_H; y++) {
            for (int x = 0; x < VEIL_MAP_W; x++) {
                int idx = y * VEIL_MAP_W + x;
                if (x <= 1 || x >= 14 || y <= 0) {
                    m->collision_map[idx] = 1; // Abismo
                }
            }
        }
        // Saída ao sul de volta à base
        m->collision_map[9 * VEIL_MAP_W + 7] = 0;
        m->collision_map[9 * VEIL_MAP_W + 8] = 0;
    }
    else if (scene == VEIL_SCENE_CLOUD_LOWER) {
        // Nuvens baixas fofas com nuvens escuras de tempestade escaváveis
        for (int y = 0; y < VEIL_MAP_H; y++) {
            for (int x = 0; x < VEIL_MAP_W; x++) {
                int idx = y * VEIL_MAP_W + x;
                // Abismo celeste no perímetro
                if (x == 0 || x == 15 || (y == 0 && (x < 7 || x > 8))) {
                    m->collision_map[idx] = 1;
                }
                // Nuvens escuras escaváveis com Mole Mitts (blocos de tempestade)
                if ((x >= 2 && x <= 4 && y >= 3 && y <= 5) ||
                    (x >= 11 && x <= 13 && y >= 3 && y <= 5)) {
                    m->overlay_layer[idx] = 0x10; // Nuvem escura de tempestade
                    m->collision_map[idx] = 1;
                }
            }
        }
    }
    else if (scene == VEIL_SCENE_CLOUD_SANCTUARY) {
        // Santuário da Tribo do Vento
        for (int y = 0; y < VEIL_MAP_H; y++) {
            for (int x = 0; x < VEIL_MAP_W; x++) {
                int idx = y * VEIL_MAP_W + x;
                if (x <= 1 || x >= 14 || y <= 0) {
                    m->collision_map[idx] = 1; // Borda das nuvens sagradas
                }
            }
        }
        // Saída ao sul para as nuvens baixas
        m->collision_map[9 * VEIL_MAP_W + 7] = 0;
        m->collision_map[9 * VEIL_MAP_W + 8] = 0;
    }

    return m;
}

void veil_clouds_init(void) {
    memset(&s_veil, 0, sizeof(s_veil));
    s_veil.is_active = false;
    s_veil.current_scene = VEIL_SCENE_FALLS_BASE;
    s_veil.golden_kinstones_fused = 0;
    s_veil.palace_tornado_active = false;
    s_veil.is_in_updraft = false;

    for (int s = 0; s < VEIL_SCENE_COUNT; s++) {
        s_veil.maps[s] = create_scene_map((VeilCloudSceneId)s);
    }

    // Inicializa os 5 pedestais de Kinstone Dourada
    for (int k = 0; k < TOTAL_GOLDEN_KINSTONES; k++) {
        s_veil.kinstone_pedestals[k] = false;
    }

    // Configura redemoinhos de vento para a cena do cume e das nuvens
    s_veil.whirlpools[0].x = 128.0f; s_veil.whirlpools[0].y = 70.0f;  s_veil.whirlpools[0].is_red = true; // Cume -> Cloud Tops
    s_veil.whirlpools[1].x = 64.0f;  s_veil.whirlpools[1].y = 80.0f;  s_veil.whirlpools[1].is_red = false;
    s_veil.whirlpools[2].x = 192.0f; s_veil.whirlpools[2].y = 80.0f;  s_veil.whirlpools[2].is_red = false;
    s_veil.whirlpools[3].x = 128.0f; s_veil.whirlpools[3].y = 60.0f;  s_veil.whirlpools[3].is_red = true; // Tornado do Palácio do Vento
    s_veil.whirlpool_count = 4;

    printf("[VEIL CLOUDS] Veil Falls & Cloud Tops inicializados com sucesso!\n");
}

bool veil_clouds_is_active(void) {
    return s_veil.is_active;
}

bool veil_clouds_is_in_clouds(void) {
    return s_veil.is_active && (s_veil.current_scene == VEIL_SCENE_CLOUD_LOWER ||
                                s_veil.current_scene == VEIL_SCENE_CLOUD_SANCTUARY);
}

VeilCloudSceneId veil_clouds_get_scene(void) {
    return s_veil.current_scene;
}

Tilemap* veil_clouds_get_current_map(void) {
    if (!s_veil.is_active) return NULL;
    return s_veil.maps[s_veil.current_scene];
}

void veil_clouds_enter_falls(float* player_x, float* player_y, Direction* player_dir) {
    s_veil.is_active = true;
    s_veil.current_scene = VEIL_SCENE_FALLS_BASE;
    s_veil.is_in_updraft = false;

    if (player_x) *player_x = 128.0f;
    if (player_y) *player_y = 135.0f;
    if (player_dir) *player_dir = DIR_UP;

    s_veil.last_safe_x = 128.0f;
    s_veil.last_safe_y = 135.0f;

    hal_audio_play_bgm(BGM_CLOUD_TOPS);
    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.1f);
    printf("[VEIL FALLS] Link chegou a base das impetuosas Quedas do Veu!\n");
}

void veil_clouds_enter_clouds(float* player_x, float* player_y, Direction* player_dir) {
    s_veil.is_active = true;
    s_veil.current_scene = VEIL_SCENE_CLOUD_LOWER;
    s_veil.is_in_updraft = false;

    if (player_x) *player_x = 128.0f;
    if (player_y) *player_y = 130.0f;
    if (player_dir) *player_dir = DIR_UP;

    s_veil.last_safe_x = 128.0f;
    s_veil.last_safe_y = 130.0f;

    hal_audio_play_bgm(BGM_CLOUD_TOPS);
    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
    printf("[CLOUD TOPS] Link alcancou o místico Topo das Nuvens na estratosfera!\n");
}

void veil_clouds_exit(float* player_x, float* player_y, Direction* player_dir) {
    s_veil.is_active = false;
    s_veil.is_in_updraft = false;

    // Retorna a North Hyrule Field (em frente ao desfiladeiro nordeste)
    if (player_x) *player_x = 248.0f;
    if (player_y) *player_y = 48.0f;
    if (player_dir) *player_dir = DIR_DOWN;

    hal_audio_play_sound(SOUND_SECRET, 0.9f, 1.1f);
    printf("[VEIL FALLS] Link retornou aos campos de Hyrule vindo de Veil Falls.\n");
}

u8 veil_clouds_get_golden_kinstones(void) {
    return s_veil.golden_kinstones_fused;
}

void veil_clouds_set_golden_kinstones(u8 count) {
    if (count > TOTAL_GOLDEN_KINSTONES) count = TOTAL_GOLDEN_KINSTONES;
    s_veil.golden_kinstones_fused = count;
    for (int k = 0; k < count; k++) {
        s_veil.kinstone_pedestals[k] = true;
    }
    if (count >= TOTAL_GOLDEN_KINSTONES) {
        s_veil.palace_tornado_active = true;
    }
}

bool veil_clouds_is_tornado_active(void) {
    return s_veil.palace_tornado_active;
}

bool veil_clouds_dig_cloud(float world_x, float world_y, int* out_rupees) {
    if (!s_veil.is_active || !veil_clouds_is_in_clouds()) return false;
    Tilemap* m = s_veil.maps[s_veil.current_scene];
    if (!m) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= m->width || ty < 0 || ty >= m->height) return false;

    int idx = ty * m->width + tx;
    if (m->overlay_layer && m->overlay_layer[idx] == 0x10) {
        // Escavou com sucesso a nuvem de tempestade!
        m->overlay_layer[idx] = 0xFF;
        m->collision_map[idx] = 0;
        hal_audio_play_sound(SOUND_WALL_CRUMBLE, 1.0f, 1.1f);

        if (out_rupees) *out_rupees += 10;
        printf("[CLOUD TOPS] Nuvem de tempestade escavada com as Mole Mitts! +10 Rupees encontrados!\n");
        return true;
    }
    return false;
}

bool veil_clouds_interact(float player_x, float player_y, Direction dir) {
    if (!s_veil.is_active) return false;

    // Se estiver no Santuário da Tribo do Vento:
    if (s_veil.current_scene == VEIL_SCENE_CLOUD_SANCTUARY) {
        // Checa distância até os 5 pedestais de Kinstones Douradas
        float ped_coords[5][2] = {
            { 64.0f, 60.0f },
            { 96.0f, 44.0f },
            { 128.0f, 36.0f },
            { 160.0f, 44.0f },
            { 192.0f, 60.0f }
        };

        for (int k = 0; k < TOTAL_GOLDEN_KINSTONES; k++) {
            float dx = player_x - ped_coords[k][0];
            float dy = player_y - ped_coords[k][1];
            if (dx * dx + dy * dy <= 24.0f * 24.0f && !s_veil.kinstone_pedestals[k]) {
                s_veil.kinstone_pedestals[k] = true;
                s_veil.golden_kinstones_fused++;
                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.2f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                printf("[WIND TRIBE] Kinstone Dourada %d/5 fundida com o Ancião do Vento!\n", s_veil.golden_kinstones_fused);

                // Se completou todas as 5:
                if (s_veil.golden_kinstones_fused >= TOTAL_GOLDEN_KINSTONES) {
                    s_veil.palace_tornado_active = true;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                    entity_trigger_screen_shake(20, 4);
                    printf("[PALACE OF WINDS] O GRANDE TORNADO DO PALACIO DO VENTO FOI CONVOCADO!\n");
                }
                return true;
            }
        }
    }

    return false;
}

void veil_clouds_update(float* player_x, float* player_y, Direction player_dir, bool is_moving,
                        int* player_hearts, int* player_rupees, bool has_mole_mitts) {
    (void)player_dir;
    (void)is_moving;
    (void)player_hearts;
    (void)player_rupees;
    (void)has_mole_mitts;
    if (!s_veil.is_active || !player_x || !player_y) return;

    s_veil_anim_timer++;

    // 1. Spawning e atualização de partículas atmosféricas (névoa e tufos de nuvens)
    s_veil.particle_timer++;
    if (s_veil.particle_timer % 4 == 0) {
        for (int p = 0; p < MAX_CLOUD_PARTICLES; p++) {
            if (!s_veil.particles[p].active) {
                AtmosphericParticle* pt = &s_veil.particles[p];
                pt->active = true;
                pt->x = (float)(rand() % 256);
                pt->y = (s_veil.current_scene == VEIL_SCENE_FALLS_BASE) ? (float)(rand() % 80 + 30) : 155.0f;
                pt->vx = ((float)rand() / (float)RAND_MAX) * 0.8f - 0.4f;
                pt->vy = (s_veil.current_scene == VEIL_SCENE_FALLS_BASE) ? ((float)rand() / (float)RAND_MAX) * 1.2f + 0.4f : -((float)rand() / (float)RAND_MAX) * 0.8f - 0.4f;
                pt->size = (float)(rand() % 3 + 2);
                pt->color = (s_veil.current_scene == VEIL_SCENE_FALLS_BASE) ? 0xE0F2FE88 : 0xF8FAFC66;
                pt->life = 40 + rand() % 30;
                pt->max_life = pt->life;
                break;
            }
        }
    }

    for (int p = 0; p < MAX_CLOUD_PARTICLES; p++) {
        if (s_veil.particles[p].active) {
            s_veil.particles[p].x += s_veil.particles[p].vx;
            s_veil.particles[p].y += s_veil.particles[p].vy;
            s_veil.particles[p].life--;
            if (s_veil.particles[p].life <= 0) {
                s_veil.particles[p].active = false;
            }
        }
    }

    // 2. Atualização dos Redemoinhos de Vento
    for (int w = 0; w < s_veil.whirlpool_count; w++) {
        s_veil.whirlpools[w].angle += 0.12f;
        if (s_veil.whirlpools[w].angle > 2.0f * PI_F) {
            s_veil.whirlpools[w].angle -= 2.0f * PI_F;
        }
    }

    // 3. Catapulta Aérea do Redemoinho (Updraft Whirlpool Arc Flight)
    if (s_veil.is_in_updraft) {
        s_veil.updraft_arc_timer -= 0.04f;
        float progress = 1.0f - s_veil.updraft_arc_timer;
        *player_x = *player_x * 0.9f + s_veil.updraft_target_x * 0.1f;
        *player_y = *player_y * 0.9f + s_veil.updraft_target_y * 0.1f;

        if (s_veil.updraft_arc_timer <= 0.0f) {
            s_veil.is_in_updraft = false;
            *player_x = s_veil.updraft_target_x;
            *player_y = s_veil.updraft_target_y;
            hal_audio_play_sound(SOUND_ROLL, 0.8f, 1.2f);
            printf("[UPDRAFT] Pouso suave nas nuvens apos voo no redemoinho!\n");
        }
        return;
    }

    // 4. Checagem de Transições entre Cenas
    // --- Base de Veil Falls ---
    if (s_veil.current_scene == VEIL_SCENE_FALLS_BASE) {
        // Sul para North Hyrule Field
        if (*player_y >= (VEIL_MAP_H * TILE_SIZE) - 16.0f) {
            veil_clouds_exit(player_x, player_y, NULL);
            return;
        }
        // Norte: Entrada da caverna para o Cume (Summit)
        if (*player_y <= 20.0f && *player_x >= 100.0f && *player_x <= 156.0f) {
            s_veil.current_scene = VEIL_SCENE_FALLS_SUMMIT;
            *player_x = 128.0f;
            *player_y = (VEIL_MAP_H * TILE_SIZE) - 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
            printf("[VEIL FALLS] Link subiu pelas cavernas escarpadas e alcancou o Cume!\n");
        }
    }
    // --- Cume de Veil Falls ---
    else if (s_veil.current_scene == VEIL_SCENE_FALLS_SUMMIT) {
        // Sul de volta à base
        if (*player_y >= (VEIL_MAP_H * TILE_SIZE) - 16.0f) {
            s_veil.current_scene = VEIL_SCENE_FALLS_BASE;
            *player_x = 128.0f;
            *player_y = 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
        }
        // Redemoinho Gigante Central: Catapulta Link para Cloud Tops!
        float dx = *player_x - 128.0f;
        float dy = *player_y - 70.0f;
        if (dx * dx + dy * dy <= 22.0f * 22.0f) {
            hal_audio_play_sound(SOUND_SPIN_ATTACK, 1.0f, 1.2f);
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
            veil_clouds_enter_clouds(player_x, player_y, NULL);
            return;
        }
    }
    // --- Nuvens Baixas (Cloud Tops Lower) ---
    else if (s_veil.current_scene == VEIL_SCENE_CLOUD_LOWER) {
        // Norte para o Santuário da Tribo do Vento
        if (*player_y <= 16.0f && *player_x >= 100.0f && *player_x <= 156.0f) {
            s_veil.current_scene = VEIL_SCENE_CLOUD_SANCTUARY;
            *player_x = 128.0f;
            *player_y = (VEIL_MAP_H * TILE_SIZE) - 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
            printf("[CLOUD TOPS] Link adentrou o Terraco Sagrado da Tribo do Vento!\n");
        }
        // Redemoinhos de impulso (whirlpool 1 e 2)
        for (int w = 1; w <= 2; w++) {
            float wx = s_veil.whirlpools[w].x;
            float wy = s_veil.whirlpools[w].y;
            float dist = sqrtf((*player_x - wx)*(*player_x - wx) + (*player_y - wy)*(*player_y - wy));
            if (dist <= 16.0f && !s_veil.is_in_updraft) {
                s_veil.is_in_updraft = true;
                s_veil.updraft_arc_timer = 1.0f;
                // Impulsiona para o lado oposto
                s_veil.updraft_target_x = (w == 1) ? 192.0f : 64.0f;
                s_veil.updraft_target_y = wy;
                hal_audio_play_sound(SOUND_SPIN_ATTACK, 0.85f, 1.3f);
                printf("[UPDRAFT] Redemoinho de vento impulsionou Link pelo abismo das nuvens!\n");
                break;
            }
        }
    }
    // --- Santuário da Tribo do Vento ---
    else if (s_veil.current_scene == VEIL_SCENE_CLOUD_SANCTUARY) {
        // Sul de volta às nuvens baixas
        if (*player_y >= (VEIL_MAP_H * TILE_SIZE) - 16.0f) {
            s_veil.current_scene = VEIL_SCENE_CLOUD_LOWER;
            *player_x = 128.0f;
            *player_y = 28.0f;
            hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 1.2f);
        }
        // Grande Tornado Ciclônico para o Palace of Winds
        if (s_veil.palace_tornado_active) {
            float dx = *player_x - 128.0f;
            float dy = *player_y - 80.0f;
            if (dx * dx + dy * dy <= 18.0f * 18.0f) {
                hal_audio_play_sound(SOUND_SPIN_ATTACK, 1.0f, 1.3f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                s_veil.is_active = false;
                dungeon_palace_enter(player_x, player_y, NULL);
                return;
            }
        }
    }
}

void veil_clouds_render(const Camera* camera, float player_x, float player_y) {
    (void)player_x;
    (void)player_y;
    if (!s_veil.is_active || !camera) return;

    int ox = -(int)camera->x;
    int oy = -(int)camera->y;

    // --- RENDERIZAÇÃO DA CENA 0: BASE DAS QUEDAS DO VÉU ---
    if (s_veil.current_scene == VEIL_SCENE_FALLS_BASE) {
        // Paredões rochosos e céu ao norte
        draw_filled_rect(0, 0, 256, 160, C_VEIL_ROCK_DARK);

        // Paredões escarpados à esquerda e direita
        draw_filled_rect(ox, oy, 64, 160, C_VEIL_ROCK_LIGHT);
        draw_filled_rect(ox + 192, oy, 64, 160, C_VEIL_ROCK_LIGHT);

        // Parede escalável com Grip Ring (col 12)
        int cl_x = ox + 192;
        for (int y = 16; y < 112; y += 8) {
            draw_filled_rect(cl_x + 2, oy + y, 12, 3, 0xD97706FF); // Fendas de escalada
        }

        // Cachoeira torrencial monumental no centro (cols 6-9)
        int w_x = ox + 96;
        draw_filled_rect(w_x, oy, 64, 128, C_VEIL_WATER_FALL);
        for (int i = 0; i < 4; i++) {
            int line_x = w_x + 8 + i * 16;
            int foam_off = (s_veil_anim_timer * 4 + i * 12) % 32;
            for (int y = 0; y < 128; y += 32) {
                draw_filled_rect(line_x, oy + y + foam_off, 3, 14, C_VEIL_WATER_FOAM);
            }
        }

        // Lago azul na base
        draw_filled_rect(ox + 64, oy + 112, 128, 48, C_VEIL_WATER_DEEP);

        // Crista de Vento (Wind Crest) da base (x=48, y=100)
        int cx = ox + 48;
        int cy = oy + 100;
        draw_filled_rect(cx - 10, cy - 10, 20, 20, 0x1E293BFF);
        draw_filled_rect(cx - 8, cy - 8, 16, 16, 0x38BDF8FF);
        // Símbolo do pássaro alado
        draw_filled_rect(cx - 4, cy - 2, 8, 4, 0xFFFFFFFF);
        draw_filled_rect(cx - 6, cy - 4, 3, 3, 0xFDE047FF);
        draw_filled_rect(cx + 3, cy - 4, 3, 3, 0xFDE047FF);
    }
    // --- RENDERIZAÇÃO DA CENA 1: CUME DE VEIL FALLS ---
    else if (s_veil.current_scene == VEIL_SCENE_FALLS_SUMMIT) {
        // Céu aberto e planalto rochoso
        draw_filled_rect(0, 0, 256, 160, C_CLOUD_SKY_BG);
        draw_filled_rect(ox + 24, oy + 16, 208, 128, C_VEIL_ROCK_LIGHT);
        draw_filled_rect(ox + 32, oy + 24, 192, 112, C_VEIL_ROCK_DARK);

        // Grande Redemoinho Central de Ascensão
        int wx = ox + (int)s_veil.whirlpools[0].x;
        int wy = oy + (int)s_veil.whirlpools[0].y;
        float ang = s_veil.whirlpools[0].angle;

        for (int r = 24; r > 0; r -= 3) {
            float f = (float)r / 24.0f;
            u32 col = (r % 6 == 0) ? C_WHIRL_RED : C_WHIRL_YELLOW;
            for (int a = 0; a < 4; a++) {
                float arm_ang = ang + a * (PI_F / 2.0f) + (1.0f - f) * 2.0f;
                int ax = wx + (int)(cosf(arm_ang) * r);
                int ay = wy + (int)(sinf(arm_ang) * r * 0.7f);
                draw_filled_rect(ax - 2, ay - 2, 4, 4, col);
            }
        }
    }
    // --- RENDERIZAÇÃO DA CENA 2: NUVENS BAIXAS (CLOUD TOPS LOWER) ---
    else if (s_veil.current_scene == VEIL_SCENE_CLOUD_LOWER) {
        // Céu azul anil profundo
        draw_filled_rect(0, 0, 256, 160, C_CLOUD_SKY_BG);

        // Grande massa de nuvens fofas
        for (int r = 1; r < VEIL_MAP_H - 1; r++) {
            for (int c = 1; c < VEIL_MAP_W - 1; c++) {
                int tx = ox + c * TILE_SIZE;
                int ty = oy + r * TILE_SIZE;
                draw_filled_rect(tx, ty, 16, 16, C_CLOUD_WHITE);
                draw_filled_rect(tx, ty + 12, 16, 4, C_CLOUD_SHADOW);
            }
        }

        // Nuvens escuras de tempestade escaváveis
        Tilemap* m = s_veil.maps[VEIL_SCENE_CLOUD_LOWER];
        if (m && m->overlay_layer) {
            for (int idx = 0; idx < m->width * m->height; idx++) {
                if (m->overlay_layer[idx] == 0x10) {
                    int cx = ox + (idx % m->width) * TILE_SIZE;
                    int cy = oy + (idx / m->width) * TILE_SIZE;
                    draw_filled_rect(cx, cy, 16, 16, C_CLOUD_DARK_STORM);
                    draw_filled_rect(cx + 2, cy + 2, 12, 12, 0x475569FF);
                    draw_filled_rect(cx + 5, cy + 5, 6, 6, 0x334155FF);
                }
            }
        }

        // Redemoinhos 1 e 2
        for (int w = 1; w <= 2; w++) {
            int wx = ox + (int)s_veil.whirlpools[w].x;
            int wy = oy + (int)s_veil.whirlpools[w].y;
            float ang = s_veil.whirlpools[w].angle;
            for (int r = 16; r > 0; r -= 3) {
                for (int a = 0; a < 3; a++) {
                    float arm_ang = ang + a * (2.0f * PI_F / 3.0f);
                    int ax = wx + (int)(cosf(arm_ang) * r);
                    int ay = wy + (int)(sinf(arm_ang) * r * 0.7f);
                    draw_filled_rect(ax - 2, ay - 2, 4, 4, C_WHIRL_YELLOW);
                }
            }
        }
    }
    // --- RENDERIZAÇÃO DA CENA 3: SANTUÁRIO DA TRIBO DO VENTO ---
    else if (s_veil.current_scene == VEIL_SCENE_CLOUD_SANCTUARY) {
        draw_filled_rect(0, 0, 256, 160, C_CLOUD_SKY_BG);

        // Terraço sagrado das nuvens
        draw_filled_rect(ox + 32, oy + 24, 192, 112, C_CLOUD_WHITE);
        draw_filled_rect(ox + 48, oy + 36, 160, 88, 0xE2E8F0FF); // Lajes nobres de mármore

        // 5 Pedestais de Kinstones Douradas em semi-círculo
        float ped_coords[5][2] = {
            { 64.0f, 60.0f },
            { 96.0f, 44.0f },
            { 128.0f, 36.0f },
            { 160.0f, 44.0f },
            { 192.0f, 60.0f }
        };

        for (int k = 0; k < TOTAL_GOLDEN_KINSTONES; k++) {
            int px = ox + (int)ped_coords[k][0];
            int py = oy + (int)ped_coords[k][1];

            draw_filled_rect(px - 8, py - 4, 16, 14, 0x64748BFF);
            draw_filled_rect(px - 6, py - 2, 12, 10, 0x94A3B8FF);

            if (s_veil.kinstone_pedestals[k]) {
                // Kinstone Dourada encaixada brilhando!
                draw_filled_rect(px - 5, py - 8, 10, 8, C_GOLD_KINSTONE);
                draw_filled_rect(px - 3, py - 6, 6, 4, 0xFEF08AFF);
                draw_rect_blend(px - 8, py - 12, 16, 16, 0xFDE04744);
            }
        }

        // Ancião da Tribo do Vento ao lado do Altar Central
        int ex = ox + 144;
        int ey = oy + 76;
        draw_filled_rect(ex - 4, ey - 10, 8, 6, 0xFDE8CDFF); // Rosto
        draw_filled_rect(ex - 6, ey - 4, 12, 14, C_WIND_TRIBE_ROBE); // Manto azul
        draw_filled_rect(ex - 6, ey - 14, 12, 5, 0x0284C7FF); // Chapéu celestial
        draw_filled_rect(ex - 2, ey - 2, 4, 6, 0xFFFFFFFF); // Barba branca longa

        // Grande Tornado Ciclônico para o Palácio do Vento (se ativado)
        if (s_veil.palace_tornado_active) {
            int tx = ox + 128;
            int ty = oy + 80;
            float tang = (float)s_veil_anim_timer * 0.18f;

            for (int h = 0; h < 60; h += 4) {
                float rad = 8.0f + ((float)h / 60.0f) * 22.0f;
                int ly = ty - h;
                float la = tang + h * 0.12f;
                int lx1 = tx + (int)(cosf(la) * rad);
                int lx2 = tx - (int)(cosf(la) * rad);
                draw_filled_rect(lx1 - 3, ly, 6, 3, C_WHIRL_YELLOW);
                draw_filled_rect(lx2 - 3, ly, 6, 3, 0xFFFFFFFF);
            }
        }
    }

    // Renderiza partículas atmosféricas flutuando
    for (int p = 0; p < MAX_CLOUD_PARTICLES; p++) {
        if (s_veil.particles[p].active) {
            int px = ox + (int)s_veil.particles[p].x;
            int py = oy + (int)s_veil.particles[p].y;
            int sz = (int)s_veil.particles[p].size;
            draw_filled_rect(px, py, sz, sz, s_veil.particles[p].color);
        }
    }
}

void veil_clouds_shutdown(void) {
    for (int s = 0; s < VEIL_SCENE_COUNT; s++) {
        if (s_veil.maps[s]) {
            if (s_veil.maps[s]->ground_layer) free(s_veil.maps[s]->ground_layer);
            if (s_veil.maps[s]->overlay_layer) free(s_veil.maps[s]->overlay_layer);
            if (s_veil.maps[s]->collision_map) free(s_veil.maps[s]->collision_map);
            free(s_veil.maps[s]);
            s_veil.maps[s] = NULL;
        }
    }
    s_veil.is_active = false;
    printf("[VEIL CLOUDS] Recursos de Veil Falls & Cloud Tops liberados com sucesso!\n");
}
