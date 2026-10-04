/*
 * ============================================================================
 * src/hal/rocs_cape.c - Capa de Roc (Roc's Cape)
 * ============================================================================
 */

#include "hal/rocs_cape.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/entity.h"
#include "hal/map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

// Paleta temática da Capa de Roc
#define C_ROCS_RED_BRIGHT   0xEF4444FF // Manto vermelho escarlate vivo
#define C_ROCS_RED_MID      0xDC2626FF // Dobras do tecido carmesim
#define C_ROCS_RED_DARK     0x991B1BFF // Sombra inferior do tecido
#define C_ROCS_GOLD         0xF59E0BFF // Broche / Fivela de ouro ancestral
#define C_ROCS_GOLD_LIGHT   0xFDE047FF // Brilho dourado
#define C_ROCS_FEATHER      0xF8FAFCFF // Acabamento de plumas celestiais
#define C_ROCS_SHOCKWAVE    0x38BDF8FF // Onda de choque ciano do impacto
#define C_ROCS_SHOCK_INNER  0xE0F2FEFF // Centro branco reluzente do impacto

static RocsCapeState s_cape = { 0 };

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

void rocs_cape_init(void) {
    memset(&s_cape, 0, sizeof(RocsCapeState));
    printf("[ROCS CAPE] Sistema da Capa de Roc inicializado com sucesso!\n");
}

void rocs_cape_use_pressed(float* player_x, float* player_y, Direction dir,
                           float* player_z, float* player_vz, bool* player_is_jumping) {
    (void)dir;
    if (!player_x || !player_y || !player_z || !player_vz || !player_is_jumping) return;

    // Primeiro salto (a partir do solo)
    if (!(*player_is_jumping) && *player_z <= 0.0f) {
        *player_is_jumping = true;
        *player_z = 2.0f;
        *player_vz = ROCS_CAPE_JUMP_IMPULSE;

        s_cape.is_jumping = true;
        s_cape.is_gliding = false;
        s_cape.is_down_thrust = false;
        s_cape.can_double_jump = true;
        s_cape.air_timer = 0;
        s_cape.glide_timer = 0;
        s_cape.last_safe_x = *player_x;
        s_cape.last_safe_y = *player_y;

        hal_audio_play_sound(SOUND_ROCS_JUMP, 1.0f, 1.0f);
        printf("[ROCS CAPE] Link saltou acrobaticamente no ar com a Capa de Roc! (vz = +%.1f)\n", ROCS_CAPE_JUMP_IMPULSE);
    }
    // Salto duplo / impulso no ar (Double-Jump)
    else if (s_cape.can_double_jump && *player_z > 3.0f && !s_cape.is_down_thrust) {
        *player_vz = ROCS_CAPE_DOUBLE_IMPULSE;
        s_cape.can_double_jump = false;
        s_cape.is_gliding = false;

        hal_audio_play_sound(SOUND_ROCS_JUMP, 1.0f, 1.25f);
        printf("[ROCS CAPE] Salto Duplo (Double Jump) executado no ar! (vz = +%.1f)\n", ROCS_CAPE_DOUBLE_IMPULSE);
    }
}

void rocs_cape_use_held(float* player_z, float* player_vz, bool player_is_jumping) {
    if (!player_z || !player_vz) return;

    if (player_is_jumping && *player_z > 3.0f && !s_cape.is_down_thrust) {
        s_cape.is_gliding = true;
        s_cape.glide_timer++;
        s_cape.flutter_timer++;

        // Limita a velocidade de queda ao patamar suave de planeio
        if (*player_vz < ROCS_CAPE_GLIDE_TERMINAL_V) {
            *player_vz = ROCS_CAPE_GLIDE_TERMINAL_V;
        }

        // Áudio suave de tecido tremulando ao vento a cada 18 frames
        if (s_cape.glide_timer % 18 == 1) {
            hal_audio_play_sound(SOUND_ROCS_GLIDE, 0.70f, 1.0f);
        }
    }
}

void rocs_cape_use_released(void) {
    s_cape.is_gliding = false;
}

bool rocs_cape_try_down_thrust(float* player_z, float* player_vz, bool player_is_jumping) {
    if (!player_z || !player_vz) return false;

    if (player_is_jumping && *player_z > 4.0f && !s_cape.is_down_thrust) {
        s_cape.is_down_thrust = true;
        s_cape.is_gliding = false;
        s_cape.can_double_jump = false;
        *player_vz = -8.2f; // Mergulho vertical afiado

        hal_audio_play_sound(SOUND_SWORD_SLASH, 1.1f, 1.35f);
        printf("[ROCS CAPE] DOWN-THRUST DISPARADO! Link mergulha apontando a lamina sagrada para o chao!\n");
        return true;
    }
    return false;
}

void rocs_cape_catch_updraft(float lift_power, float* player_z, float* player_vz, bool* player_is_jumping) {
    if (!player_z || !player_vz || !player_is_jumping) return;

    *player_is_jumping = true;
    s_cape.is_jumping = true;
    s_cape.is_down_thrust = false;
    s_cape.can_double_jump = true;
    *player_vz = lift_power;

    hal_audio_play_sound(SOUND_ROCS_JUMP, 1.0f, 1.4f);
    printf("[ROCS CAPE] Redemoinho / Corrente de Vento capturada! Ascensao vigorosa no ar! (vz = +%.1f)\n", lift_power);
}

bool rocs_cape_is_gliding(void) {
    return s_cape.is_gliding;
}

bool rocs_cape_is_down_thrust(void) {
    return s_cape.is_down_thrust;
}

void rocs_cape_update(float* player_x, float* player_y, Direction player_dir, bool is_moving,
                      float* player_z, float* player_vz, bool* player_is_jumping,
                      Tilemap* map, int* player_hearts) {
    (void)player_dir;
    (void)is_moving;
    if (!player_x || !player_y || !player_z || !player_vz || !player_is_jumping) return;

    // 1. Atualização do voo e da gravidade vertical
    if (*player_is_jumping || *player_z > 0.0f) {
        s_cape.air_timer++;
        *player_z += *player_vz;

        float grav = s_cape.is_gliding ? ROCS_CAPE_GRAVITY_GLIDE : ROCS_CAPE_GRAVITY_NORMAL;
        *player_vz -= grav;

        // Limite máximo de teto de voo
        if (*player_z > ROCS_CAPE_MAX_HEIGHT) {
            *player_z = ROCS_CAPE_MAX_HEIGHT;
            if (*player_vz > 0.0f) *player_vz = 0.0f;
        }

        // Checa acerto contínuo contra inimigos durante a descida veloz do Down-Thrust
        if (s_cape.is_down_thrust) {
            entity_check_sword_hit(*player_x + 1.0f, *player_y + 4.0f, 14.0f, 14.0f, 3, DIR_DOWN);
        }

        // 2. Detecção de Impacto / Aterrissagem no Solo
        if (*player_z <= 0.0f) {
            *player_z = 0.0f;
            *player_vz = 0.0f;
            *player_is_jumping = false;

            if (s_cape.is_down_thrust) {
                // Impacto retumbante do Down-Thrust no solo!
                s_cape.shockwave_timer = 18;
                s_cape.shockwave_x = *player_x + 8.0f;
                s_cape.shockwave_y = *player_y + 12.0f;
                s_cape.shockwave_radius = 8.0f;

                entity_trigger_screen_shake(10, 3);
                hal_audio_play_sound(SOUND_WALL_CRUMBLE, 1.0f, 1.25f);
                hal_audio_play_sound(SOUND_BOSS_HIT, 0.9f, 1.2f);

                // Dano em área centrífugo da onda de choque em 360 graus
                entity_check_spin_attack_hit(s_cape.shockwave_x, s_cape.shockwave_y, 34.0f, 4);

                // Esmagamento de rochas ou paredes rachadas ao redor
                if (map) {
                    map_interact_bomb(map, s_cape.shockwave_x, s_cape.shockwave_y, 22.0f);
                }
                printf("[ROCS CAPE] ATERRISSAGEM DESTRUCTIVA DO DOWN-THRUST! Onda de choque provocada em (%.1f, %.1f)!\n",
                       s_cape.shockwave_x, s_cape.shockwave_y);
            } else {
                // Aterrissagem suave normal
                hal_audio_play_sound(SOUND_ROLL, 0.65f, 1.2f);
            }

            s_cape.is_jumping = false;
            s_cape.is_gliding = false;
            s_cape.is_down_thrust = false;
            s_cape.can_double_jump = false;

            // 3. Checagem de queda em abismos / buracos sem fundo (Pits)
            if (map && map_is_pit(map, *player_x + 8.0f, *player_y + 12.0f)) {
                hal_audio_play_sound(SOUND_MINISH_SHRINK, 1.0f, 1.35f);
                if (player_hearts && *player_hearts > 1) {
                    (*player_hearts)--;
                    hal_audio_play_sound(SOUND_HEART_BEEP, 1.0f, 1.0f);
                }
                *player_x = s_cape.last_safe_x;
                *player_y = s_cape.last_safe_y;
                printf("[ROCS CAPE] Link aterrissou sobre um abismo! Resgatado em terra firme segura (%.1f, %.1f)!\n",
                       s_cape.last_safe_x, s_cape.last_safe_y);
            } else {
                s_cape.last_safe_x = *player_x;
                s_cape.last_safe_y = *player_y;
            }
        }
    } else {
        // No chão: memoriza posição segura se estiver sobre terreno firme
        if (map && !map_is_pit(map, *player_x + 8.0f, *player_y + 12.0f) &&
            !map_is_water(map, *player_x + 8.0f, *player_y + 12.0f)) {
            s_cape.last_safe_x = *player_x;
            s_cape.last_safe_y = *player_y;
        }
    }

    // 4. Expansão da onda de choque no solo
    if (s_cape.shockwave_timer > 0) {
        s_cape.shockwave_timer--;
        s_cape.shockwave_radius += 1.8f;
    }
}

void rocs_cape_render_shadow(const Camera* camera, float player_x, float player_y, float player_z) {
    if (!camera || player_z <= 0.0f) return;

    int sx, sy;
    map_world_to_screen(camera, player_x, player_y, &sx, &sy);

    // Conforme a altitude aumenta, a sombra diminui levemente e se torna mais tênue
    float alt_ratio = player_z / ROCS_CAPE_MAX_HEIGHT;
    if (alt_ratio > 1.0f) alt_ratio = 1.0f;

    int sw = 14 - (int)(alt_ratio * 4.0f);
    int sh = 5  - (int)(alt_ratio * 2.0f);
    if (sw < 6) sw = 6;
    if (sh < 3) sh = 3;

    u8 alpha = (u8)(110 - (alt_ratio * 50.0f));
    u32 shadow_color = (0x0F << 24) | (0x17 << 16) | (0x2A << 8) | alpha;

    draw_rect_blend(sx + 8 - sw / 2, sy + 13 - sh / 2, sw, sh, shadow_color);
    draw_rect_blend(sx + 8 - (sw - 4) / 2, sy + 13 - (sh - 1) / 2, sw - 4, sh - 1, shadow_color);
}

void rocs_cape_render(const Camera* camera, float player_x, float player_y, Direction dir, float player_z) {
    if (!camera) return;

    int sx, sy;
    map_world_to_screen(camera, player_x, player_y, &sx, &sy);

    // Aplica o deslocamento visual de altitude no ar
    sy -= (int)player_z;

    // Se estiver em Down-Thrust, desenha a lâmina vertical apontada para baixo
    if (s_cape.is_down_thrust) {
        draw_filled_rect(sx + 7, sy + 14, 2, 8, 0xFFFFFFFF); // Lâmina prateada
        draw_filled_rect(sx + 5, sy + 13, 6, 2, 0xF59E0BFF); // Guarda dourada
        draw_filled_rect(sx + 7, sy + 11, 2, 2, 0xEF4444FF); // Empunhadura
        return;
    }

    // Renderização do Manto Carmesim da Capa de Roc
    int flutter = (s_cape.flutter_timer / 4) % 3;
    int wave_dx = (flutter == 1) ? 1 : ((flutter == 2) ? -1 : 0);

    if (s_cape.is_gliding) {
        // Manto totalmente aberto em asas planadoras laterais
        // Asa esquerda
        draw_filled_rect(sx - 4 + wave_dx, sy + 6, 6, 8, C_ROCS_RED_BRIGHT);
        draw_filled_rect(sx - 6 + wave_dx, sy + 9, 3, 5, C_ROCS_RED_MID);
        draw_filled_rect(sx - 7 + wave_dx, sy + 12, 4, 3, C_ROCS_FEATHER); // Penas brancas na borda

        // Asa direita
        draw_filled_rect(sx + 14 - wave_dx, sy + 6, 6, 8, C_ROCS_RED_BRIGHT);
        draw_filled_rect(sx + 19 - wave_dx, sy + 9, 3, 5, C_ROCS_RED_MID);
        draw_filled_rect(sx + 19 - wave_dx, sy + 12, 4, 3, C_ROCS_FEATHER);

        // Broche dourado central no pescoço
        draw_filled_rect(sx + 7, sy + 4, 3, 3, C_ROCS_GOLD);
        hal_video_put_pixel(sx + 8, sy + 5, C_ROCS_GOLD_LIGHT);
    } else {
        // Manto pendendo nas costas / flutuando de acordo com a direção
        switch (dir) {
            case DIR_DOWN:
                // Atrás dos ombros
                draw_filled_rect(sx + 1, sy + 5, 3, 7 + flutter, C_ROCS_RED_MID);
                draw_filled_rect(sx + 12, sy + 5, 3, 7 + flutter, C_ROCS_RED_MID);
                draw_filled_rect(sx + 1, sy + 11 + flutter, 3, 2, C_ROCS_FEATHER);
                draw_filled_rect(sx + 12, sy + 11 + flutter, 3, 2, C_ROCS_FEATHER);
                break;
            case DIR_UP:
                // Visível nas costas em primeiro plano
                draw_filled_rect(sx + 3, sy + 5, 10, 9 + flutter, C_ROCS_RED_BRIGHT);
                draw_filled_rect(sx + 4, sy + 7, 8, 7 + flutter, C_ROCS_RED_MID);
                draw_filled_rect(sx + 4, sy + 13 + flutter, 8, 2, C_ROCS_FEATHER);
                draw_filled_rect(sx + 7, sy + 4, 2, 2, C_ROCS_GOLD);
                break;
            case DIR_LEFT:
                // Esvoaçando para a direita
                draw_filled_rect(sx + 10 + wave_dx, sy + 5, 6, 8 + flutter, C_ROCS_RED_BRIGHT);
                draw_filled_rect(sx + 12 + wave_dx, sy + 7, 4, 6 + flutter, C_ROCS_RED_MID);
                draw_filled_rect(sx + 13 + wave_dx, sy + 12 + flutter, 3, 2, C_ROCS_FEATHER);
                break;
            case DIR_RIGHT:
                // Esvoaçando para a esquerda
                draw_filled_rect(sx - 1 - wave_dx, sy + 5, 6, 8 + flutter, C_ROCS_RED_BRIGHT);
                draw_filled_rect(sx - 1 - wave_dx, sy + 7, 4, 6 + flutter, C_ROCS_RED_MID);
                draw_filled_rect(sx - 1 - wave_dx, sy + 12 + flutter, 3, 2, C_ROCS_FEATHER);
                break;
        }
    }
}

void rocs_cape_render_shockwave(const Camera* camera) {
    if (!camera || s_cape.shockwave_timer <= 0) return;

    int sx, sy;
    map_world_to_screen(camera, s_cape.shockwave_x, s_cape.shockwave_y, &sx, &sy);

    float r = s_cape.shockwave_radius;
    int points = 16;
    float alpha = (float)s_cape.shockwave_timer / 18.0f;
    u8 a = (u8)(alpha * 220.0f);
    u32 color_outer = (0x38 << 24) | (0xBD << 16) | (0xF8 << 8) | a;
    u32 color_inner = (0xFF << 24) | (0xFF << 16) | (0xFF << 8) | a;

    for (int i = 0; i < points; i++) {
        float ang = (float)i * (2.0f * PI_F / (float)points);
        int px = sx + (int)(cosf(ang) * r);
        int py = sy + (int)(sinf(ang) * (r * 0.55f)); // Elipse perspectiva

        draw_rect_blend(px - 1, py - 1, 3, 3, color_outer);
        draw_rect_blend(px, py, 1, 1, color_inner);
    }
}

RocsCapeState* rocs_cape_get_state(void) {
    return &s_cape;
}

void rocs_cape_shutdown(void) {
    memset(&s_cape, 0, sizeof(RocsCapeState));
    printf("[ROCS CAPE] Sistema da Capa de Roc finalizado.\n");
}
