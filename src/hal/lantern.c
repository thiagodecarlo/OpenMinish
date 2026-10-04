/*
 * ============================================================================
 * src/hal/lantern.c - Lanterna de Chamas (Flame Lantern)
 * ============================================================================
 */

#include "hal/lantern.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/entity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static LanternState s_lantern = { 0 };

#define MAX_TORCH_LIGHTS 16

typedef struct {
    float x;
    float y;
    float r_in;
    float r_out;
} LightSource;

void lantern_init(void) {
    memset(&s_lantern, 0, sizeof(s_lantern));
    s_lantern.is_lit = true; // Ao equipar a lanterna, a chama eterna se acende
    s_lantern.dark_room_override = false;
    printf("[LANTERN] Flame Lantern inicializada com chama eterna pronta para iluminacao!\n");
}

LanternState* lantern_get_state(void) {
    return &s_lantern;
}

bool lantern_is_lit(void) {
    return s_lantern.is_lit;
}

void lantern_set_lit(bool lit) {
    s_lantern.is_lit = lit;
}

void lantern_toggle_lit(void) {
    s_lantern.is_lit = !s_lantern.is_lit;
    if (s_lantern.is_lit) {
        hal_audio_play_sound(SOUND_FIRE, 0.9f, 1.2f);
        printf("[LANTERN] Lanterna de Chamas ACESA!\n");
    } else {
        hal_audio_play_sound(SOUND_SWITCH_CLICK, 0.7f, 0.8f);
        printf("[LANTERN] Lanterna de Chamas APAGADA.\n");
    }
}

static void spawn_spark(float x, float y, float vx, float vy, int life, u32 color, float size) {
    for (int i = 0; i < MAX_LANTERN_SPARKS; i++) {
        if (!s_lantern.sparks[i].is_active) {
            LanternSpark* sp = &s_lantern.sparks[i];
            sp->is_active = true;
            sp->x = x;
            sp->y = y;
            sp->vx = vx;
            sp->vy = vy;
            sp->life = life;
            sp->max_life = life;
            sp->color = color;
            sp->size = size;
            break;
        }
    }
}

void lantern_use_pressed(float link_x, float link_y, Direction dir) {
    s_lantern.is_lit = true;
    s_lantern.is_swinging = true;
    s_lantern.swing_timer = 14; // 14 frames (~0.23s)
    s_lantern.swing_dir = dir;

    float fx = link_x + 8.0f;
    float fy = link_y + 8.0f;
    if (dir == DIR_DOWN)  fy += 16.0f;
    if (dir == DIR_UP)    fy -= 16.0f;
    if (dir == DIR_LEFT)  fx -= 16.0f;
    if (dir == DIR_RIGHT) fx += 16.0f;

    s_lantern.swing_x = fx;
    s_lantern.swing_y = fy;

    // Emite rajada de faíscas incandescentes
    for (int i = 0; i < 8; i++) {
        float angle = ((float)rand() / (float)RAND_MAX) * 1.2f - 0.6f;
        float spd = 1.5f + ((float)rand() / (float)RAND_MAX) * 2.2f;
        float vx = 0.0f, vy = 0.0f;

        if (dir == DIR_DOWN)  { vx = sinf(angle) * spd; vy = cosf(angle) * spd; }
        if (dir == DIR_UP)    { vx = sinf(angle) * spd; vy = -cosf(angle) * spd; }
        if (dir == DIR_LEFT)  { vx = -cosf(angle) * spd; vy = sinf(angle) * spd; }
        if (dir == DIR_RIGHT) { vx = cosf(angle) * spd; vy = sinf(angle) * spd; }

        u32 spark_cols[3] = { 0xFEF08AFF, 0xF97316FF, 0xEF4444FF };
        spawn_spark(fx, fy, vx, vy, 12 + rand() % 8, spark_cols[i % 3], 2.0f + (rand() % 2));
    }

    hal_audio_play_sound(SOUND_FIRE, 0.95f, 1.15f);

    // Causa dano de fogo (2 HP) em monstros em frente ao herói
    float hit_w = (dir == DIR_LEFT || dir == DIR_RIGHT) ? 20.0f : 16.0f;
    float hit_h = (dir == DIR_UP || dir == DIR_DOWN) ? 20.0f : 16.0f;
    entity_check_sword_hit(fx - hit_w * 0.5f, fy - hit_h * 0.5f, hit_w, hit_h, 2, dir);
}

void lantern_use_held(float link_x, float link_y, Direction dir) {
    if (s_lantern.is_lit && (rand() % 6 == 0)) {
        float fx = link_x + 8.0f;
        float fy = link_y + 8.0f;
        if (dir == DIR_DOWN)  fy += 12.0f;
        if (dir == DIR_UP)    fy -= 12.0f;
        if (dir == DIR_LEFT)  fx -= 12.0f;
        if (dir == DIR_RIGHT) fx += 12.0f;

        float drift_vx = ((float)rand() / (float)RAND_MAX) * 0.6f - 0.3f;
        float drift_vy = -0.5f - ((float)rand() / (float)RAND_MAX) * 0.5f;
        spawn_spark(fx, fy, drift_vx, drift_vy, 16, 0xFBA026FF, 1.5f);
    }
}

void lantern_use_released(void) {
    // Mantém a lanterna acesa
}

void lantern_update(float delta_time, float link_x, float link_y, Tilemap* map) {
    s_lantern.flicker_phase += delta_time * 9.0f;

    // Atualiza o arco de golpe
    if (s_lantern.is_swinging) {
        s_lantern.swing_timer--;
        if (s_lantern.swing_timer <= 0) {
            s_lantern.is_swinging = false;
        }

        // Interação com o mapa na frente do Link
        if (map) {
            int out_type = 0;
            // Checa no centro da chama e em raio próximo
            float check_points[3][2] = {
                { s_lantern.swing_x, s_lantern.swing_y },
                { s_lantern.swing_x - 6.0f, s_lantern.swing_y - 6.0f },
                { s_lantern.swing_x + 6.0f, s_lantern.swing_y + 6.0f }
            };
            for (int p = 0; p < 3; p++) {
                if (map_interact_lantern(map, check_points[p][0], check_points[p][1], &out_type)) {
                    if (out_type == 1) {
                        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                        printf("[LANTERN] Tocha de pedra ACESA pelo fogo da lanterna!\n");
                    } else if (out_type == 2) {
                        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                        printf("[LANTERN] Bloco de gelo DERRETIDO pelo calor da lanterna!\n");
                    } else if (out_type == 3) {
                        hal_audio_play_sound(SOUND_FIRE, 0.8f, 1.4f);
                        printf("[LANTERN] Teia de aranha QUEIMADA e desintegrada!\n");
                    } else if (out_type == 4) {
                        hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.25f);
                    }
                }
            }
        }
    }

    // Auto-acendimento de tochas próximas se Link passar perto com a lanterna acesa
    if (s_lantern.is_lit && map) {
        int out_type = 0;
        float lx = link_x + 8.0f;
        float ly = link_y + 8.0f;
        float offsets[4][2] = { {0, 16}, {0, -16}, {16, 0}, {-16, 0} };
        for (int o = 0; o < 4; o++) {
            if (map_interact_lantern(map, lx + offsets[o][0], ly + offsets[o][1], &out_type)) {
                if (out_type == 1) {
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                    printf("[LANTERN] Tocha adjacente acesa pelo calor continuo da lanterna!\n");
                }
            }
        }
    }

    // Atualiza partículas de fagulhas
    for (int i = 0; i < MAX_LANTERN_SPARKS; i++) {
        if (s_lantern.sparks[i].is_active) {
            LanternSpark* sp = &s_lantern.sparks[i];
            sp->x += sp->vx;
            sp->y += sp->vy;
            sp->life--;
            if (sp->life <= 0) {
                sp->is_active = false;
            }
        }
    }
}

void lantern_render(const Camera* camera) {
    if (!camera) return;

    // 1. Renderiza faíscas ativas
    for (int i = 0; i < MAX_LANTERN_SPARKS; i++) {
        if (s_lantern.sparks[i].is_active) {
            const LanternSpark* sp = &s_lantern.sparks[i];
            int sx = (int)(sp->x - camera->x);
            int sy = (int)(sp->y - camera->y);
            int sz = (int)sp->size;
            for (int dy = 0; dy < sz; dy++) {
                for (int dx = 0; dx < sz; dx++) {
                    hal_video_put_pixel(sx + dx, sy + dy, sp->color);
                }
            }
        }
    }

    // 2. Renderiza labareda do arco de fogo durante o ataque
    if (s_lantern.is_swinging) {
        int fx = (int)(s_lantern.swing_x - camera->x);
        int fy = (int)(s_lantern.swing_y - camera->y);
        for (int y = -6; y <= 6; y++) {
            for (int x = -6; x <= 6; x++) {
                float d = sqrtf((float)(x * x + y * y));
                if (d <= 2.2f) {
                    hal_video_put_pixel(fx + x, fy + y, 0xFFFFFFFF); // Núcleo branco incandescente
                } else if (d <= 4.2f) {
                    hal_video_put_pixel(fx + x, fy + y, 0xFDE047FF); // Amarelo fogo
                } else if (d <= 6.0f) {
                    hal_video_put_pixel(fx + x, fy + y, 0xEA580CFF); // Laranja avermelhado
                }
            }
        }
    }
}

void lantern_render_lighting(const Camera* camera, float link_x, float link_y, Tilemap* map, bool is_dark_room) {
    if (!is_dark_room && !s_lantern.dark_room_override) return;
    if (!camera) return;

    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer) return;

    int screen_w = ctx->render_width;
    int screen_h = ctx->render_height;

    // Coleta todas as fontes de luz visíveis na tela
    LightSource lights[MAX_TORCH_LIGHTS];
    int light_count = 0;

    // 1. Fonte principal: Herói (Link) com sua lanterna
    float link_sx = link_x + 8.0f - camera->x;
    float link_sy = link_y + 8.0f - camera->y;

    float hero_r_out = s_lantern.is_lit ?
        (68.0f + 3.2f * sinf(s_lantern.flicker_phase) + 1.6f * cosf(s_lantern.flicker_phase * 1.6f)) : 20.0f;
    float hero_r_in = hero_r_out * 0.35f;

    lights[light_count].x = link_sx;
    lights[light_count].y = link_sy;
    lights[light_count].r_in = hero_r_in;
    lights[light_count].r_out = hero_r_out;
    light_count++;

    // 2. Tochas acesas visíveis no mapa (TILE_TORCH_LIT)
    if (map && map->width > 0 && map->height > 0) {
        int start_tx = (int)(camera->x / TILE_SIZE) - 1;
        int end_tx   = (int)((camera->x + screen_w) / TILE_SIZE) + 1;
        int start_ty = (int)(camera->y / TILE_SIZE) - 1;
        int end_ty   = (int)((camera->y + screen_h) / TILE_SIZE) + 1;

        if (start_tx < 0) start_tx = 0;
        if (end_tx >= map->width) end_tx = map->width - 1;
        if (start_ty < 0) start_ty = 0;
        if (end_ty >= map->height) end_ty = map->height - 1;

        for (int ty = start_ty; ty <= end_ty && light_count < MAX_TORCH_LIGHTS; ty++) {
            for (int tx = start_tx; tx <= end_tx && light_count < MAX_TORCH_LIGHTS; tx++) {
                int idx = ty * map->width + tx;
                bool is_torch = false;
                if (map->overlay_layer && map->overlay_layer[idx] == TILE_TORCH_LIT) is_torch = true;
                if (map->ground_layer && map->ground_layer[idx] == TILE_TORCH_LIT) is_torch = true;

                if (is_torch) {
                    float tsx = (tx * TILE_SIZE + 8.0f) - camera->x;
                    float tsy = (ty * TILE_SIZE + 8.0f) - camera->y;
                    float t_rout = 52.0f + 2.5f * sinf(s_lantern.flicker_phase + (tx + ty));
                    lights[light_count].x = tsx;
                    lights[light_count].y = tsy;
                    lights[light_count].r_in = t_rout * 0.32f;
                    lights[light_count].r_out = t_rout;
                    light_count++;
                }
            }
        }
    }

    // 3. Aplica máscara de iluminação com atenuação cúbica/quadrática suave sobre o framebuffer
    const float ambient = 0.12f; // Escuridão de 88% fora do feixe de luz
    u32* fb = ctx->framebuffer;

    for (int py = 0; py < screen_h; py++) {
        for (int px = 0; px < screen_w; px++) {
            float max_l = 0.0f;

            for (int i = 0; i < light_count; i++) {
                float dx = (float)px - lights[i].x;
                float dy = (float)py - lights[i].y;
                float dist_sq = dx * dx + dy * dy;
                float rout_sq = lights[i].r_out * lights[i].r_out;

                if (dist_sq < rout_sq) {
                    float rin_sq = lights[i].r_in * lights[i].r_in;
                    float factor = 1.0f;
                    if (dist_sq > rin_sq) {
                        float dist = sqrtf(dist_sq);
                        factor = 1.0f - (dist - lights[i].r_in) / (lights[i].r_out - lights[i].r_in);
                        if (factor < 0.0f) factor = 0.0f;
                        factor = factor * factor; // Suavização exponencial suave
                    }
                    if (factor > max_l) max_l = factor;
                }
            }

            float bright = ambient + (1.0f - ambient) * max_l;

            // Tom quente suave de fogo quando iluminado
            float r_boost = 1.0f + 0.12f * max_l;
            float g_boost = 1.0f + 0.04f * max_l;
            float b_boost = 1.0f - 0.08f * max_l;

            int idx = py * screen_w + px;
            u32 c = fb[idx];
            int r = (int)(((c >> 24) & 0xFF) * bright * r_boost);
            int g = (int)(((c >> 16) & 0xFF) * bright * g_boost);
            int b = (int)(((c >> 8)  & 0xFF) * bright * b_boost);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            fb[idx] = ((u32)r << 24) | ((u32)g << 16) | ((u32)b << 8) | (c & 0xFF);
        }
    }
}

void lantern_set_dark_room(bool dark) {
    s_lantern.dark_room_override = dark;
}

bool lantern_is_dark_room(void) {
    return s_lantern.dark_room_override;
}

void lantern_toggle_dark_room(void) {
    s_lantern.dark_room_override = !s_lantern.dark_room_override;
    printf("[LIGHTING] Ambiente escuro (Dark Room Lighting): %s\n",
           s_lantern.dark_room_override ? "ATIVO (Testando Iluminacao Dinamica da Lanterna)" : "DESATIVADO");
}
