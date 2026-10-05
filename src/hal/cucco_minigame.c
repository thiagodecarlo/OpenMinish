/*
 * ============================================================================
 * src/hal/cucco_minigame.c - Minigame das Galinhas Cucco de Anju (Ato VI)
 * ============================================================================
 * Implementação clean-room C11 do minigame clássico das galinhas de Hyrule Town.
 */

#include "hal/cucco_minigame.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SCREEN_W 256
#define SCREEN_H 160

// Cercado de Anju em Hyrule Town (Sudeste do vilarejo)
#define PEN_X1 170.0f
#define PEN_Y1 150.0f
#define PEN_X2 235.0f
#define PEN_Y2 210.0f

// Paleta de Cores do Minigame
#define C_CUCCO_WHITE       0xFFFFFFFF
#define C_CUCCO_COMB        0xEF4444FF // Crista vermelha vibrante
#define C_CUCCO_BEAK        0xF59E0BFF // Bico amarelo alaranjado
#define C_CUCCO_WING_SHADOW 0xCBD5E1FF // Sombra da asa branca
#define C_CHICK_YELLOW      0xFDE047FF // Pintinho amarelo fofo
#define C_GOLDEN_BODY       0xFBBF24FF // Galinha dourada brilhante
#define C_GOLDEN_CREST      0xDC2626FF // Crista da galinha dourada
#define C_FENCE_WOOD        0x78350FFF // Cerca de madeira do cercado
#define C_FENCE_LIGHT       0xB45309FF
#define C_HUD_BG            0x1E1B4BCC // Fundo azul escuro do HUD
#define C_TIMER_ALERT       0xEF4444FF // Alerta vermelho nos ultimos segundos

static CuccoMinigame s_game = { 0 };

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
    int x2 = (x + w > SCREEN_W) ? SCREEN_W : (x + w);
    int y2 = (y + h > SCREEN_H) ? SCREEN_H : (y + h);

    for (int py = y1; py < y2; py++) {
        for (int px = x1; px < x2; px++) {
            u32 current = hal_video_get_pixel(px, py);
            hal_video_put_pixel(px, py, blend_colors(current, color));
        }
    }
}

// Configurações canônicas de cada um dos 10 níveis
typedef struct {
    int cuccos_count;
    int seconds;
    int chicks;
    int golden;
    int rupee_reward;
    int shell_reward;
} LevelConfig;

static const LevelConfig s_level_configs[MAX_CUCCO_LEVELS] = {
    { 2,  25, 0, 0,  20, 10 }, // Nivel 1
    { 3,  30, 0, 0,  30, 15 }, // Nivel 2
    { 4,  35, 1, 0,  40, 20 }, // Nivel 3
    { 5,  40, 1, 0,  50, 25 }, // Nivel 4
    { 5,  45, 2, 0,  60, 30 }, // Nivel 5
    { 6,  45, 2, 1,  70, 35 }, // Nivel 6
    { 7,  50, 3, 1,  80, 40 }, // Nivel 7
    { 8,  50, 3, 2, 100, 50 }, // Nivel 8
    { 9,  55, 4, 2, 120, 60 }, // Nivel 9
    { 10, 55, 3, 3, 200, 100}  // Nivel 10 (Desafio Supremo -> Heart Piece!)
};

// Posições de espalhamento dos Cuccos pela cidade de Hyrule
static const struct { float x; float y; } s_spawn_points[MAX_CUCCOS] = {
    { 110.0f, 170.0f },
    { 135.0f, 130.0f },
    {  85.0f,  90.0f },
    { 160.0f,  80.0f },
    {  60.0f, 140.0f },
    { 190.0f,  75.0f },
    { 120.0f, 210.0f },
    {  50.0f, 190.0f },
    { 145.0f,  55.0f },
    {  80.0f, 220.0f },
    { 100.0f,  60.0f },
    { 175.0f, 115.0f }
};

void cucco_minigame_init(void) {
    memset(&s_game, 0, sizeof(CuccoMinigame));
    s_game.is_active = false;
    s_game.mode = CUCCO_GAME_INACTIVE;
    s_game.current_level = 1;
    s_game.highest_cleared_level = 0;
    s_game.carrying_index = -1;
    s_game.has_heart_piece_awarded = false;
}

void cucco_minigame_start_level(int level) {
    if (level < 1) level = 1;
    if (level > MAX_CUCCO_LEVELS) level = MAX_CUCCO_LEVELS;

    s_game.is_active = true;
    s_game.mode = CUCCO_GAME_RUNNING;
    s_game.current_level = level;
    s_game.carrying_index = -1;
    s_game.banner_timer = 120; // 2 segundos de banner de inicio

    const LevelConfig* cfg = &s_level_configs[level - 1];
    s_game.total_cuccos = cfg->cuccos_count;
    s_game.secured_cuccos = 0;
    s_game.time_limit_frames = cfg->seconds * 60;
    s_game.time_remaining_frames = s_game.time_limit_frames;
    s_game.reward_rupees = cfg->rupee_reward;
    s_game.reward_shells = cfg->shell_reward;

    // Inicializa cada Cucco
    int chicks_left = cfg->chicks;
    int golden_left = cfg->golden;

    for (int i = 0; i < MAX_CUCCOS; i++) {
        CuccoEntity* c = &s_game.cuccos[i];
        if (i < s_game.total_cuccos) {
            c->active = true;
            c->state = CUCCO_STATE_IDLE;
            c->x = s_spawn_points[i].x;
            c->y = s_spawn_points[i].y;
            c->vx = 0.0f;
            c->vy = 0.0f;
            c->z = 0.0f;
            c->vz = 0.0f;
            c->direction = i % 4;
            c->anim_frame = 0;
            c->anim_timer = 0;
            c->flap_timer = 0;
            c->peck_timer = 30 + (i * 17) % 60;
            c->flee_timer = 0;
            c->is_flapping = false;

            if (golden_left > 0) {
                c->type = CUCCO_TYPE_GOLDEN;
                golden_left--;
            } else if (chicks_left > 0) {
                c->type = CUCCO_TYPE_CHICK;
                chicks_left--;
            } else {
                c->type = CUCCO_TYPE_WHITE;
            }
        } else {
            c->active = false;
        }
    }

    hal_audio_play_bgm(BGM_CUCCO_MINIGAME);
    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
    printf("[CUCCO] Minigame da Anju iniciado! Nivel %d (%d Cuccos em %d segundos)!\n",
           level, s_game.total_cuccos, cfg->seconds);
}

void cucco_minigame_stop(void) {
    s_game.is_active = false;
    s_game.mode = CUCCO_GAME_INACTIVE;
    s_game.carrying_index = -1;
    hal_audio_play_bgm(BGM_HYRULE_TOWN);
    printf("[CUCCO] Minigame de Anju encerrado.\n");
}

bool cucco_minigame_is_active(void) {
    return s_game.is_active;
}

bool cucco_minigame_is_carrying(void) {
    return (s_game.carrying_index >= 0);
}

static bool is_inside_pen(float x, float y) {
    return (x >= PEN_X1 && x <= PEN_X2 && y >= PEN_Y1 && y <= PEN_Y2);
}

void cucco_minigame_handle_action(float link_x, float link_y, int link_dir) {
    if (!s_game.is_active || s_game.mode != CUCCO_GAME_RUNNING) return;

    // Se já estiver carregando um Cucco, arremessa!
    if (s_game.carrying_index >= 0) {
        int idx = s_game.carrying_index;
        CuccoEntity* c = &s_game.cuccos[idx];
        c->state = CUCCO_STATE_THROWN;
        c->x = link_x;
        c->y = link_y;
        c->z = 12.0f;
        c->vz = 2.5f;

        float throw_speed = 3.2f;
        switch (link_dir) {
            case 0: c->vx =  0.0f;        c->vy =  throw_speed; break; // Baixo
            case 1: c->vx =  0.0f;        c->vy = -throw_speed; break; // Cima
            case 2: c->vx = -throw_speed; c->vy =  0.0f;        break; // Esquerda
            case 3: c->vx =  throw_speed; c->vy =  0.0f;        break; // Direita
            default: c->vx = 0.0f;        c->vy =  throw_speed; break;
        }

        s_game.carrying_index = -1;
        hal_audio_play_sound(SOUND_SWORD_SLASH, 1.2f, 1.4f);
        return;
    }

    // Procura Cucco próximo para agarrar (alcance 24px)
    float best_dist_sq = 24.0f * 24.0f;
    int best_idx = -1;

    for (int i = 0; i < s_game.total_cuccos; i++) {
        CuccoEntity* c = &s_game.cuccos[i];
        if (!c->active || c->state == CUCCO_STATE_PEN_SECURED) continue;

        float dx = (c->x) - link_x;
        float dy = (c->y) - link_y;
        float dist_sq = dx * dx + dy * dy;

        if (dist_sq < best_dist_sq) {
            best_dist_sq = dist_sq;
            best_idx = i;
        }
    }

    if (best_idx >= 0) {
        s_game.carrying_index = best_idx;
        CuccoEntity* c = &s_game.cuccos[best_idx];
        c->state = CUCCO_STATE_CARRIED;
        c->vx = 0.0f;
        c->vy = 0.0f;
        c->z = 16.0f;
        hal_audio_play_sound(SOUND_CUCCO_CALL, 1.0f, (c->type == CUCCO_TYPE_CHICK) ? 1.4f : 1.0f);
    }
}

void cucco_minigame_update(float* link_x, float* link_y, int link_dir, bool* link_carrying,
                           int* out_rupees, int* out_shells, int* out_hearts, int max_hearts) {
    if (!s_game.is_active) return;

    if (s_game.banner_timer > 0) {
        s_game.banner_timer--;
    }

    if (s_game.mode == CUCCO_GAME_RUNNING) {
        if (s_game.time_remaining_frames > 0) {
            s_game.time_remaining_frames--;
        } else {
            // Tempo esgotado!
            s_game.mode = CUCCO_GAME_FAILED;
            s_game.state_timer = 180;
            hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 0.7f);
            printf("[CUCCO] Tempo esgotado! As galinhas fugiram!\n");
            return;
        }

        // Atualiza a posição do Cucco que Link está carregando
        if (s_game.carrying_index >= 0) {
            *link_carrying = true;
            CuccoEntity* c = &s_game.cuccos[s_game.carrying_index];
            c->x = *link_x;
            c->y = *link_y - 12.0f;
            c->z = 14.0f;
            c->direction = link_dir;

            // Se Link entrar a pé dentro do cercado com o Cucco
            if (is_inside_pen(*link_x, *link_y)) {
                c->state = CUCCO_STATE_PEN_SECURED;
                c->z = 0.0f;
                c->vx = (float)((rand() % 10) - 5) * 0.1f;
                c->vy = (float)((rand() % 10) - 5) * 0.1f;
                s_game.secured_cuccos++;
                s_game.carrying_index = -1;
                *link_carrying = false;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);

                if (s_game.secured_cuccos >= s_game.total_cuccos) {
                    s_game.mode = CUCCO_GAME_SUCCESS;
                    s_game.state_timer = 180;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                }
            }
        } else {
            *link_carrying = false;
        }

        // Atualiza cada Cucco
        for (int i = 0; i < s_game.total_cuccos; i++) {
            CuccoEntity* c = &s_game.cuccos[i];
            if (!c->active) continue;

            if (c->state == CUCCO_STATE_CARRIED) {
                // Já atualizado acima
                continue;
            }

            if (c->state == CUCCO_STATE_THROWN) {
                c->x += c->vx;
                c->y += c->vy;
                c->z += c->vz;
                c->vz -= 0.22f; // Gravidade
                c->is_flapping = true;

                if (c->z <= 0.0f) {
                    c->z = 0.0f;
                    c->vx = 0.0f;
                    c->vy = 0.0f;

                    // Caiu dentro do cercado?
                    if (is_inside_pen(c->x, c->y)) {
                        c->state = CUCCO_STATE_PEN_SECURED;
                        s_game.secured_cuccos++;
                        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);

                        if (s_game.secured_cuccos >= s_game.total_cuccos) {
                            s_game.mode = CUCCO_GAME_SUCCESS;
                            s_game.state_timer = 180;
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                        }
                    } else {
                        c->state = CUCCO_STATE_PANIC_FLEE;
                        c->flee_timer = 90;
                    }
                }
                continue;
            }

            if (c->state == CUCCO_STATE_PEN_SECURED) {
                // Anda devagar e feliz dentro do cercado
                c->peck_timer--;
                if (c->peck_timer <= 0) {
                    c->peck_timer = 40 + rand() % 80;
                    c->direction = rand() % 4;
                }
                float speed = 0.3f;
                if (c->direction == 0 && c->y < PEN_Y2 - 8.0f) c->y += speed;
                if (c->direction == 1 && c->y > PEN_Y1 + 8.0f) c->y -= speed;
                if (c->direction == 2 && c->x > PEN_X1 + 8.0f) c->x -= speed;
                if (c->direction == 3 && c->x < PEN_X2 - 8.0f) c->x += speed;
                continue;
            }

            // IA no mundo aberto de Hyrule: Fuga ou Passeio
            float dx = c->x - *link_x;
            float dy = c->y - *link_y;
            float dist_sq = dx * dx + dy * dy;

            if (dist_sq < 42.0f * 42.0f) {
                c->state = CUCCO_STATE_PANIC_FLEE;
                c->flee_timer = 70;
            }

            if (c->state == CUCCO_STATE_PANIC_FLEE) {
                c->flee_timer--;
                if (c->flee_timer <= 0) {
                    c->state = CUCCO_STATE_IDLE;
                }
                c->is_flapping = true;
                float flee_speed = (c->type == CUCCO_TYPE_GOLDEN) ? 2.5f :
                                   (c->type == CUCCO_TYPE_CHICK)  ? 2.0f : 1.4f;

                float dist = sqrtf(dist_sq);
                if (dist > 0.1f) {
                    c->x += (dx / dist) * flee_speed;
                    c->y += (dy / dist) * flee_speed;
                }

                // Limites da cidade para não fugir do mapa
                if (c->x < 30.0f)  c->x = 30.0f;
                if (c->x > 230.0f) c->x = 230.0f;
                if (c->y < 40.0f)  c->y = 40.0f;
                if (c->y > 230.0f) c->y = 230.0f;

                c->z = fabsf(sinf((float)s_game.time_remaining_frames * 0.4f)) * 4.0f;
            } else {
                c->is_flapping = false;
                c->z = 0.0f;
                c->peck_timer--;
                if (c->peck_timer <= 0) {
                    c->peck_timer = 60 + rand() % 90;
                    c->direction = rand() % 4;
                }
            }
        }
    } else if (s_game.mode == CUCCO_GAME_SUCCESS) {
        s_game.state_timer--;
        if (s_game.state_timer == 120) {
            // Entrega de Recompensas
            if (out_rupees) *out_rupees += s_game.reward_rupees;
            if (out_shells) *out_shells += s_game.reward_shells;

            if (s_game.current_level == 10 && !s_game.has_heart_piece_awarded) {
                s_game.has_heart_piece_awarded = true;
                if (out_hearts && *out_hearts < max_hearts) {
                    *out_hearts += 4; // Concede 1 coração cheio permanente
                }
                printf("[CUCCO] VITORIA SUPREMA! PEDACO DE CORACAO CONCEDIDO!\n");
            }

            if (s_game.current_level > s_game.highest_cleared_level) {
                s_game.highest_cleared_level = s_game.current_level;
            }
        }
        if (s_game.state_timer <= 0) {
            s_game.mode = CUCCO_GAME_INACTIVE;
            s_game.is_active = false;
            hal_audio_play_bgm(BGM_HYRULE_TOWN);
        }
    } else if (s_game.mode == CUCCO_GAME_FAILED) {
        s_game.state_timer--;
        if (s_game.state_timer <= 0) {
            s_game.mode = CUCCO_GAME_INACTIVE;
            s_game.is_active = false;
            hal_audio_play_bgm(BGM_HYRULE_TOWN);
        }
    }
}

// Renderização procedural dos Cuccos e do cercado de Anju
void cucco_minigame_render(const Camera* camera, float link_x, float link_y, int link_dir) {
    (void)link_x;
    (void)link_y;
    (void)link_dir;
    if (!s_game.is_active) return;

    int cam_x = camera ? (int)camera->x : 0;
    int cam_y = camera ? (int)camera->y : 0;

    // 1. Desenha o Cercado de Anju (Madeira e Portal)
    int px1 = (int)PEN_X1 - cam_x;
    int py1 = (int)PEN_Y1 - cam_y;
    int px2 = (int)PEN_X2 - cam_x;
    int py2 = (int)PEN_Y2 - cam_y;

    // Cerca de ripas de madeira rústica
    draw_filled_rect(px1, py1, px2 - px1, 3, C_FENCE_WOOD);
    draw_filled_rect(px1, py2 - 3, px2 - px1, 3, C_FENCE_WOOD);
    draw_filled_rect(px1, py1, 3, py2 - py1, C_FENCE_WOOD);
    draw_filled_rect(px2 - 3, py1, 3, py2 - py1, C_FENCE_WOOD);

    // Postes da cerca
    for (int fx = px1; fx <= px2; fx += 16) {
        draw_filled_rect(fx, py1 - 2, 4, 7, C_FENCE_LIGHT);
        draw_filled_rect(fx, py2 - 5, 4, 7, C_FENCE_LIGHT);
    }

    // Anju, a criadora de galinhas (NPC estilizada na entrada do cercado)
    int anju_sx = (int)PEN_X1 - 12 - cam_x;
    int anju_sy = (int)(PEN_Y1 + PEN_Y2) / 2 - cam_y;
    draw_filled_rect(anju_sx - 4, anju_sy - 14, 8, 7, 0xF9A8D4FF); // Cabelo castanho/fita
    draw_filled_rect(anju_sx - 3, anju_sy - 10, 6, 6, 0xFED7AAFF); // Rosto
    draw_filled_rect(anju_sx - 5, anju_sy - 4, 10, 10, 0x3B82F6FF); // Vestido azul
    draw_filled_rect(anju_sx - 4, anju_sy - 2, 8, 6, 0xFFFFFFFF);  // Avental branco

    // 2. Desenha cada um dos Cuccos
    for (int i = 0; i < s_game.total_cuccos; i++) {
        CuccoEntity* c = &s_game.cuccos[i];
        if (!c->active) continue;

        int sx = (int)c->x - cam_x;
        int sy = (int)(c->y - c->z) - cam_y;
        int shadow_y = (int)c->y - cam_y;

        // Sombra suave no chão
        if (c->z > 2.0f) {
            draw_filled_rect(sx - 4, shadow_y + 4, 8, 3, 0x00000044);
        }

        if (c->type == CUCCO_TYPE_CHICK) {
            // Pintinho Amarelo (Chick)
            draw_filled_rect(sx - 3, sy - 4, 6, 5, C_CHICK_YELLOW);
            draw_filled_rect(sx + 2, sy - 3, 2, 2, C_CUCCO_BEAK);
            draw_filled_rect(sx - 2, sy + 1, 2, 2, 0xEA580CFF); // Pés
            draw_filled_rect(sx + 1, sy + 1, 2, 2, 0xEA580CFF);
            if (c->is_flapping) {
                draw_filled_rect(sx - 4, sy - 3, 2, 2, 0xFEF08AFF);
                draw_filled_rect(sx + 3, sy - 3, 2, 2, 0xFEF08AFF);
            }
        } else if (c->type == CUCCO_TYPE_GOLDEN) {
            // Cucco Dourado Majestoso
            draw_filled_rect(sx - 5, sy - 6, 10, 8, C_GOLDEN_BODY);
            draw_filled_rect(sx - 2, sy - 9, 4, 3, C_GOLDEN_CREST);
            draw_filled_rect(sx + 4, sy - 4, 3, 3, C_CUCCO_BEAK);
            draw_filled_rect(sx - 3, sy + 2, 2, 3, 0xD97706FF);
            draw_filled_rect(sx + 1, sy + 2, 2, 3, 0xD97706FF);

            // Asas douradas batendo
            int wing_offset = c->is_flapping ? -3 : 0;
            draw_filled_rect(sx - 7, sy - 5 + wing_offset, 3, 5, 0xFDE047FF);
            draw_filled_rect(sx + 4, sy - 5 + wing_offset, 3, 5, 0xFDE047FF);

            // Partícula cintilante dourada
            if ((s_game.time_remaining_frames + i * 5) % 15 < 7) {
                draw_filled_rect(sx - 6, sy - 8, 2, 2, 0xFFFFFFFF);
            }
        } else {
            // Cucco Adulto Branco Tradicional
            draw_filled_rect(sx - 5, sy - 6, 10, 8, C_CUCCO_WHITE);
            draw_filled_rect(sx - 2, sy - 9, 4, 3, C_CUCCO_COMB);
            draw_filled_rect(sx + 4, sy - 4, 3, 3, C_CUCCO_BEAK);
            draw_filled_rect(sx - 3, sy + 2, 2, 3, 0xEA580CFF);
            draw_filled_rect(sx + 1, sy + 2, 2, 3, 0xEA580CFF);

            int wing_offset = c->is_flapping ? -2 : 0;
            draw_filled_rect(sx - 7, sy - 5 + wing_offset, 3, 5, C_CUCCO_WING_SHADOW);
            draw_filled_rect(sx + 4, sy - 5 + wing_offset, 3, 5, C_CUCCO_WING_SHADOW);
        }
    }
}

// HUD do Minigame (Cronômetro regressivo e contador de galinhas)
void cucco_minigame_render_hud(void) {
    if (!s_game.is_active) return;

    // Painel do Topo
    draw_filled_rect(60, 4, 136, 20, C_HUD_BG);
    draw_filled_rect(60, 4, 136, 1, 0x6366F1FF);

    // Tempo restante (Segundos e décimos)
    int secs = s_game.time_remaining_frames / 60;
    int tenths = (s_game.time_remaining_frames % 60) * 10 / 60;
    char time_str[32];
    snprintf(time_str, sizeof(time_str), "%02d.%ds", secs, tenths);

    u32 timer_col = (secs <= 5) ? C_TIMER_ALERT : 0x38BDF8FF;
    font_draw_text(66, 9, time_str, timer_col, true);

    // Contador de Cuccos resgatados
    char count_str[32];
    snprintf(count_str, sizeof(count_str), "CUCCO: %d/%d", s_game.secured_cuccos, s_game.total_cuccos);
    font_draw_text(118, 9, count_str, 0xFACC15FF, true);

    // Banner Inicial de Rodada
    if (s_game.banner_timer > 0) {
        draw_filled_rect(40, 65, 176, 26, 0x0F172AEE);
        draw_filled_rect(40, 65, 176, 2, 0xF59E0BFF);
        char round_buf[32];
        snprintf(round_buf, sizeof(round_buf), "ANJU CUCCO: ROUND %d!", s_game.current_level);
        font_draw_text(52, 73, round_buf, 0xFEF08AFF, true);
    }

    // Banner de Vitória
    if (s_game.mode == CUCCO_GAME_SUCCESS) {
        draw_filled_rect(30, 50, 196, 45, 0x14532DEE);
        draw_filled_rect(30, 50, 196, 2, 0x22C55EFF);
        font_draw_text(55, 58, "EXCELENTE! CUCCOS SALVAS!", 0x86EFACFF, true);

        char rew_buf[48];
        snprintf(rew_buf, sizeof(rew_buf), "+%d Rupees  +%d Conchas!", s_game.reward_rupees, s_game.reward_shells);
        font_draw_text(48, 72, rew_buf, 0xFDE047FF, true);

        if (s_game.current_level == 10) {
            font_draw_text(45, 84, "PEDACO DE CORACAO CONCEDIDO!", 0xF43F5EFF, true);
        }
    }

    // Banner de Derrota
    if (s_game.mode == CUCCO_GAME_FAILED) {
        draw_filled_rect(40, 60, 176, 30, 0x7F1D1DEE);
        draw_filled_rect(40, 60, 176, 2, 0xEF4444FF);
        font_draw_text(58, 68, "TEMPO ESGOTADO!", 0xFCA5A5FF, true);
        font_draw_text(46, 78, "Tente novamente com Anju!", 0xFECACAFF, true);
    }
}

int cucco_minigame_get_level(void) {
    return s_game.current_level;
}

int cucco_minigame_get_highest_cleared(void) {
    return s_game.highest_cleared_level;
}

bool cucco_minigame_has_heart_piece(void) {
    return s_game.has_heart_piece_awarded;
}

void cucco_minigame_restore(int highest_cleared, bool heart_piece_awarded) {
    s_game.highest_cleared_level = highest_cleared;
    s_game.has_heart_piece_awarded = heart_piece_awarded;
    printf("[CUCCO] Progresso restaurado: Nivel maximo %d, Heart Piece: %s\n",
           highest_cleared, heart_piece_awarded ? "SIM" : "NAO");
}

void cucco_minigame_export(int* out_highest_cleared, bool* out_heart_piece_awarded) {
    if (out_highest_cleared) *out_highest_cleared = s_game.highest_cleared_level;
    if (out_heart_piece_awarded) *out_heart_piece_awarded = s_game.has_heart_piece_awarded;
}
