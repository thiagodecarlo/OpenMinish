/*
 * ============================================================================
 * src/hal/boss_vaati.c - Confronto Final Épico: Feiticeiro Vaati (Ato V)
 * ============================================================================
 * Implementação canônica em C11 da batalha final em 3 fases climáticas
 * contra o feiticeiro Vaati, cura da Princesa Zelda e créditos finais.
 */

#include "hal/boss_vaati.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TILE_SIZE 16
#define VAATI_ARENA_W 16
#define VAATI_ARENA_H 10
#define SCREEN_W 256
#define SCREEN_H 160

// Paleta de cores do confronto final
#define C_VAATI_ROBE        0x581C87FF // Manto púrpura imperial de Vaati
#define C_VAATI_HAIR        0xE9D5FFFF // Cabelo lilás pálido
#define C_VAATI_SKIN        0xEDE9FEFF // Pele pálida cadavérica
#define C_VAATI_EYE_RED     0xDC2626FF // Olho carmesim demoníaco
#define C_VAATI_MALICE_DARK 0x3B0764FF // Malícia e escuridão profunda
#define C_VAATI_GOLD        0xF59E0BFF // Coroa e frisos dourados
#define C_VAATI_WING        0x1E1B4BFF // Asas de morcego demoníacas
#define C_VAATI_CLAW        0x18181BFF // Garras de ferro negro
#define C_VAATI_LASER       0xEF4444FF // Feixes de laser carmesim
#define C_VAATI_SHIELD_GLOW 0x38BDF8FF // Reflexo sagrado dos escudos
#define C_ZELDA_DRESS       0xF472B6FF // Vestido rosa da Princesa Zelda
#define C_EZLO_MINISH       0x22C55EFF // Mestre Ezlo na sua forma Minish real
#define C_HOLY_LIGHT        0xFEF08AFF // Luz dourada de cura

static VaatiBoss s_vaati = { 0 };
static const Texture* s_vaati_npcs_tex = NULL;

void boss_vaati_set_npcs_texture(const Texture* tex) {
    s_vaati_npcs_tex = tex;
}

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

static Tilemap* create_vaati_arena_map(void) {
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width = VAATI_ARENA_W;
    m->height = VAATI_ARENA_H;
    int total = VAATI_ARENA_W * VAATI_ARENA_H;

    m->ground_layer = (u8*)malloc(total);
    m->overlay_layer = (u8*)malloc(total);
    m->collision_map = (u8*)malloc(total);
    m->is_authentic = false;

    memset(m->ground_layer, TILE_MELARI_STONE_FLOOR, total);
    memset(m->overlay_layer, 0, total);
    memset(m->collision_map, 0, total);

    // Paredes do perímetro da arena do teto
    for (int y = 0; y < VAATI_ARENA_H; y++) {
        for (int x = 0; x < VAATI_ARENA_W; x++) {
            int idx = y * VAATI_ARENA_W + x;
            if (y == 0 || y == VAATI_ARENA_H - 1 || x == 0 || x == VAATI_ARENA_W - 1) {
                m->collision_map[idx] = 1;
            }
        }
    }
    return m;
}

void vaati_boss_init(void) {
    memset(&s_vaati, 0, sizeof(VaatiBoss));
    s_vaati.is_active = false;
    s_vaati.phase = VAATI_PHASE_1_REBORN;
    s_vaati.hp = 12;
    s_vaati.max_hp = 12;
    s_vaati.x = 128.0f;
    s_vaati.y = 48.0f;
    s_vaati.orb_count = 4;

    for (int i = 0; i < 4; i++) {
        s_vaati.orbs[i].active = true;
        s_vaati.orbs[i].hp = 2;
        s_vaati.orbs[i].angle = i * (3.14159f / 2.0f);
    }

    s_vaati.arena_map = create_vaati_arena_map();
}

void vaati_boss_start(float* link_x, float* link_y, Direction* link_dir) {
    if (!s_vaati.arena_map) {
        vaati_boss_init();
    }
    s_vaati.is_active = true;
    s_vaati.phase = VAATI_PHASE_1_REBORN;
    s_vaati.hp = 12;
    s_vaati.max_hp = 12;
    s_vaati.x = 128.0f;
    s_vaati.y = 48.0f;

    if (link_x) *link_x = 128.0f;
    if (link_y) *link_y = 128.0f;
    if (link_dir) *link_dir = DIR_UP;

    hal_audio_play_bgm(BGM_BOSS_BATTLE);
    hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.7f);
    printf("[VAATI CLIMAX] O Feiticeiro Vaati desce dos ceus corrompidos! Inicio da Fase 1!\n");
}

void vaati_boss_exit(float* link_x, float* link_y, Direction* link_dir) {
    s_vaati.is_active = false;
    printf("[VAATI CLIMAX] Batalha encerrada.\n");
}

bool vaati_boss_is_active(void) {
    return s_vaati.is_active;
}

VaatiPhase vaati_boss_get_phase(void) {
    return s_vaati.phase;
}

VaatiBoss* vaati_boss_get_state(void) {
    return &s_vaati;
}

Tilemap* vaati_boss_get_arena_map(void) {
    return s_vaati.arena_map;
}

void vaati_boss_update(float* link_x, float* link_y, Direction* link_dir,
                       bool is_attacking, bool is_spin, int clone_count,
                       const float clone_x[3], const float clone_y[3],
                       bool is_minish, int* player_hearts) {
    if (!s_vaati.is_active || !link_x || !link_y) return;

    float lx = *link_x;
    float ly = *link_y;

    if (s_vaati.hit_flash > 0) s_vaati.hit_flash--;
    if (s_vaati.storm_flash_timer > 0) s_vaati.storm_flash_timer--;

    s_vaati.action_timer++;

    // ==========================================
    // FASE 1: VAATI REBORN
    // ==========================================
    if (s_vaati.phase == VAATI_PHASE_1_REBORN) {
        // Rotação dos 4 Orbs protetores
        bool any_orb_alive = false;
        for (int i = 0; i < 4; i++) {
            if (s_vaati.orbs[i].active) {
                any_orb_alive = true;
                s_vaati.orbs[i].angle += 0.04f;
                s_vaati.orbs[i].x = s_vaati.x + cosf(s_vaati.orbs[i].angle) * 30.0f;
                s_vaati.orbs[i].y = s_vaati.y + sinf(s_vaati.orbs[i].angle) * 20.0f;

                // Colisão de orb com Link
                float odx = lx - s_vaati.orbs[i].x;
                float ody = ly - s_vaati.orbs[i].y;
                if (odx * odx + ody * ody < 14.0f * 14.0f) {
                    if (player_hearts && *player_hearts > 0) {
                        (*player_hearts)--;
                        hal_audio_play_sound(SOUND_BOSS_HIT, 0.9f, 1.2f);
                    }
                }
            }
        }

        // Se todos os 4 orbs forem destruídos, o olho peitoral abre!
        if (!any_orb_alive) {
            s_vaati.eye_open = true;
            s_vaati.eye_open_timer++;
            if (s_vaati.eye_open_timer >= 180) { // 3 segundos vulnerável
                // Restaura os orbs e teletransporta
                s_vaati.eye_open = false;
                s_vaati.eye_open_timer = 0;
                for (int i = 0; i < 4; i++) {
                    s_vaati.orbs[i].active = true;
                    s_vaati.orbs[i].hp = 2;
                }
                s_vaati.x = (s_vaati.x < 128.0f) ? 170.0f : 85.0f;
                hal_audio_play_sound(SOUND_GUST_BLAST, 0.8f, 1.4f);
            }
        }

        // Movimento senoidal de flutuação
        s_vaati.y = 48.0f + sinf(s_vaati.action_timer * 0.05f) * 6.0f;
    }

    // ==========================================
    // FASE 2: VAATI TRANSFIGURED
    // ==========================================
    else if (s_vaati.phase == VAATI_PHASE_2_TRANSFIGURED) {
        s_vaati.wing_anim_angle += 0.08f;

        // Movimento circular pelo ar
        s_vaati.x = 128.0f + cosf(s_vaati.action_timer * 0.02f) * 40.0f;
        s_vaati.y = 52.0f + sinf(s_vaati.action_timer * 0.03f) * 12.0f;

        // Se atordoado no chão:
        if (s_vaati.stunned) {
            s_vaati.y = 75.0f;
            s_vaati.stun_timer--;
            if (s_vaati.stun_timer <= 0) {
                s_vaati.stunned = false;
                s_vaati.quad_eyes_revealed = false;
                s_vaati.quad_hit_mask = 0;
                // Reativa os 8 olhos
                for (int i = 0; i < 8; i++) {
                    s_vaati.orbs[i].active = true;
                    s_vaati.orbs[i].revealed = false;
                    s_vaati.orbs[i].angle = i * (3.14159f / 4.0f);
                }
            }
        } else {
            // Órbita dos 8 olhos
            for (int i = 0; i < 8; i++) {
                if (s_vaati.orbs[i].active) {
                    s_vaati.orbs[i].angle += 0.035f;
                    s_vaati.orbs[i].x = s_vaati.x + cosf(s_vaati.orbs[i].angle) * 44.0f;
                    s_vaati.orbs[i].y = s_vaati.y + sinf(s_vaati.orbs[i].angle) * 26.0f;
                }
            }
        }
    }

    // ==========================================
    // FASE 3: VAATI'S WRATH
    // ==========================================
    else if (s_vaati.phase == VAATI_PHASE_3_WRATH) {
        s_vaati.wing_anim_angle += 0.05f;

        // Garras demoníacas
        for (int i = 0; i < 2; i++) {
            VaatiClaw* cl = &s_vaati.claws[i];
            if (!cl->destroyed) {
                cl->burrow_timer++;
                if (!cl->burrowed && cl->burrow_timer >= 120) {
                    cl->burrowed = true;
                    cl->burrow_timer = 0;
                    cl->x = (i == 0) ? lx - 20.0f : lx + 20.0f;
                    cl->y = ly - 10.0f;
                    hal_audio_play_sound(SOUND_BOSS_SLAM, 0.9f, 1.1f);
                }
            }
        }

        // Se ambas as garras foram destruídas: disparo de lasers e reflexo
        if (s_vaati.claws[0].destroyed && s_vaati.claws[1].destroyed) {
            s_vaati.laser_firing = true;
            if (s_vaati.laser_reflect_mask >= 0x0F && !s_vaati.stunned) {
                // Todos os 4 feixes refletidos com escudos!
                s_vaati.stunned = true;
                s_vaati.stun_timer = 200;
                s_vaati.laser_firing = false;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 0.7f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                printf("[VAATI'S WRATH] 4 Lasers refletidos pelos 4 Clones! Vaati esta atordoado no solo!\n");
            }
        }

        if (s_vaati.stunned) {
            s_vaati.y = 65.0f;
            s_vaati.stun_timer--;
            if (s_vaati.stun_timer <= 0) {
                s_vaati.stunned = false;
                s_vaati.laser_reflect_mask = 0;
            }
        }
    }

    // ==========================================
    // EPÍLOGO & CINEMÁTICA
    // ==========================================
    else if (s_vaati.phase == VAATI_PHASE_DEFEAT_CINEMATIC) {
        s_vaati.zelda_cinematic_timer++;
        if (s_vaati.zelda_cinematic_timer == 60) {
            s_vaati.zelda_cured = true;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
            printf("[EPILOGUE] A Princesa Zelda foi curada da petrificacao!\n");
        }
        if (s_vaati.zelda_cinematic_timer == 180) {
            s_vaati.ezlo_farewell = true;
            hal_audio_play_sound(SOUND_TIGER_SCROLL, 1.0f, 1.1f);
            printf("[EPILOGUE] O Sabio Minish Ezlo agradece a Link e se despede!\n");
        }
        if (s_vaati.zelda_cinematic_timer >= 320) {
            s_vaati.phase = VAATI_PHASE_CREDITS;
            s_vaati.credits_scroll_y = SCREEN_H;
            hal_audio_play_bgm(BGM_TITLE_THEME);
        }
    }

    // ==========================================
    // CRÉDITOS FINAIS
    // ==========================================
    else if (s_vaati.phase == VAATI_PHASE_CREDITS) {
        s_vaati.credits_scroll_y--;
        if (s_vaati.credits_scroll_y <= -300) {
            s_vaati.game_complete = true;
        }
    }
}

void vaati_boss_strike_sword(float link_x, float link_y, Direction link_dir,
                             int damage, bool is_spin, int clone_count,
                             const float clone_x[3], const float clone_y[3]) {
    if (!s_vaati.is_active) return;

    // Fase 1: golpe contra os orbs ou contra o olho peitoral aberto
    if (s_vaati.phase == VAATI_PHASE_1_REBORN) {
        if (!s_vaati.eye_open) {
            for (int i = 0; i < 4; i++) {
                if (s_vaati.orbs[i].active) {
                    float odx = link_x - s_vaati.orbs[i].x;
                    float ody = link_y - s_vaati.orbs[i].y;
                    if (odx * odx + ody * ody < 24.0f * 24.0f) {
                        s_vaati.orbs[i].hp -= damage;
                        hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.2f);
                        if (s_vaati.orbs[i].hp <= 0) {
                            s_vaati.orbs[i].active = false;
                            hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.4f);
                            printf("[VAATI 1] Orb de malicia %d destruido!\n", i + 1);
                        }
                    }
                }
            }
        } else {
            // Olho peitoral aberto e vulnerável
            float vdx = link_x - s_vaati.x;
            float vdy = link_y - s_vaati.y;
            if (vdx * vdx + vdy * vdy < 32.0f * 32.0f) {
                s_vaati.hp -= damage;
                s_vaati.hit_flash = 15;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.0f);
                printf("[VAATI 1] Golpe certeiro no olho! HP: %d/%d\n", s_vaati.hp, s_vaati.max_hp);

                if (s_vaati.hp <= 0) {
                    // Transição para Fase 2
                    s_vaati.phase = VAATI_PHASE_2_TRANSFIGURED;
                    s_vaati.hp = 16;
                    s_vaati.max_hp = 16;
                    s_vaati.storm_flash_timer = 30;
                    s_vaati.eye_open = false;
                    for (int i = 0; i < 8; i++) {
                        s_vaati.orbs[i].active = true;
                        s_vaati.orbs[i].revealed = false;
                        s_vaati.orbs[i].angle = i * (3.14159f / 4.0f);
                    }
                    hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 0.9f);
                    printf("[VAATI CLIMAX] Vaati sofreu metamorfose demoniaca! VAATI TRANSFIGURED (Fase 2)!\n");
                }
            }
        }
    }

    // Fase 2: golpe sincronizado dos 4 clones nos 4 olhos revelados
    else if (s_vaati.phase == VAATI_PHASE_2_TRANSFIGURED) {
        if (s_vaati.quad_eyes_revealed && !s_vaati.stunned) {
            // Golpe simultâneo
            if (is_spin || clone_count >= 3) {
                s_vaati.stunned = true;
                s_vaati.stun_timer = 200;
                hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.8f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                printf("[VAATI 2] Ataque quadruplo sincronizado acertou todos os olhos! A besta desabou atordoada!\n");
            }
        } else if (s_vaati.stunned) {
            float vdx = link_x - s_vaati.x;
            float vdy = link_y - s_vaati.y;
            if (vdx * vdx + vdy * vdy < 36.0f * 36.0f) {
                s_vaati.hp -= damage;
                s_vaati.hit_flash = 12;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.0f);
                printf("[VAATI 2] Golpe na besta atordoada! HP: %d/%d\n", s_vaati.hp, s_vaati.max_hp);

                if (s_vaati.hp <= 0) {
                    // Transição para Fase 3: Vaati's Wrath!
                    s_vaati.phase = VAATI_PHASE_3_WRATH;
                    s_vaati.hp = 20;
                    s_vaati.max_hp = 20;
                    s_vaati.stunned = false;
                    s_vaati.claws[0].active = true;
                    s_vaati.claws[0].parasite_hp = 4;
                    s_vaati.claws[1].active = true;
                    s_vaati.claws[1].parasite_hp = 4;
                    s_vaati.storm_flash_timer = 40;
                    hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 0.8f);
                    printf("[VAATI CLIMAX] Clímax Final: VAATI'S WRATH (Fase 3)! Garras gigantes e feixes de laser!\n");
                }
            }
        }
    }

    // Fase 3: golpe contra o núcleo do parasita na garra ou contra Vaati atordoado
    else if (s_vaati.phase == VAATI_PHASE_3_WRATH) {
        if (s_vaati.stunned) {
            float vdx = link_x - s_vaati.x;
            float vdy = link_y - s_vaati.y;
            if (vdx * vdx + vdy * vdy < 40.0f * 40.0f) {
                s_vaati.hp -= damage;
                s_vaati.hit_flash = 12;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.0f);
                printf("[VAATI 3] Golpe na furia demoniaca! HP: %d/%d\n", s_vaati.hp, s_vaati.max_hp);

                if (s_vaati.hp <= 0) {
                    // Vitória definitiva!
                    s_vaati.phase = VAATI_PHASE_DEFEAT_CINEMATIC;
                    s_vaati.zelda_cinematic_timer = 0;
                    hal_audio_play_sound(SOUND_BOSS_DEFEAT, 1.0f, 0.7f);
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.6f);
                    printf("[VAATI DERROTADO] Vaati foi aniquilado definitivamente! O reino de Hyrule esta salvo!\n");
                }
            }
        }
    }
}

void vaati_boss_arrow_hit(float arrow_x, float arrow_y) {
    if (!s_vaati.is_active) return;

    // Fase 2: flechas revelam os 4 olhos vermelhos
    if (s_vaati.phase == VAATI_PHASE_2_TRANSFIGURED && !s_vaati.quad_eyes_revealed) {
        for (int i = 0; i < 8; i++) {
            if (s_vaati.orbs[i].active && !s_vaati.orbs[i].revealed) {
                float adx = arrow_x - s_vaati.orbs[i].x;
                float ady = arrow_y - s_vaati.orbs[i].y;
                if (adx * adx + ady * ady < 16.0f * 16.0f) {
                    s_vaati.orbs[i].revealed = true;
                    hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.3f);
                    printf("[VAATI 2] Olho demoniaco %d revelado!\n", i + 1);

                    int rev_cnt = 0;
                    for (int j = 0; j < 8; j++) {
                        if (s_vaati.orbs[j].revealed) rev_cnt++;
                    }
                    if (rev_cnt >= 4) {
                        s_vaati.quad_eyes_revealed = true;
                        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.25f);
                        printf("[VAATI 2] 4 Olhos vermelhos revelados! Golpeie simultaneamente com os 4 Clones!\n");
                    }
                    break;
                }
            }
        }
    }
}

void vaati_boss_cane_hit(float cane_x, float cane_y) {
    if (!s_vaati.is_active) return;

    // Fase 3: virar a garra demoníaca com Cane of Pacci
    if (s_vaati.phase == VAATI_PHASE_3_WRATH) {
        for (int i = 0; i < 2; i++) {
            VaatiClaw* cl = &s_vaati.claws[i];
            if (cl->active && cl->burrowed && !cl->flipped && !cl->destroyed) {
                float cdx = cane_x - cl->x;
                float cdy = cane_y - cl->y;
                if (cdx * cdx + cdy * cdy < 28.0f * 28.0f) {
                    cl->flipped = true;
                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.8f);
                    printf("[CANE OF PACCI] Garra demoniaca %d virada de cabeca para baixo! Encolha com Minish para entrar!\n", i + 1);
                }
            }
        }
    }
}

void vaati_boss_render(const Camera* cam, float link_x, float link_y, bool is_minish) {
    if (!s_vaati.is_active) return;

    int ox = cam ? (int)-cam->x : 0;
    int oy = cam ? (int)-cam->y : 0;

    // Fundo da arena (Teto do Castelo / Tempestade)
    for (int y = 0; y < VAATI_ARENA_H; y++) {
        for (int x = 0; x < VAATI_ARENA_W; x++) {
            int px = ox + x * TILE_SIZE;
            int py = oy + y * TILE_SIZE;
            if (y == 0 || y == VAATI_ARENA_H - 1 || x == 0 || x == VAATI_ARENA_W - 1) {
                draw_filled_rect(px, py, TILE_SIZE, TILE_SIZE, 0x181824FF);
            } else {
                u32 col = ((x + y) % 2 == 0) ? 0x2E2A4FFF : 0x252240FF;
                draw_filled_rect(px, py, TILE_SIZE, TILE_SIZE, col);
            }
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

    // Minish Stump no canto
    draw_filled_rect(ox + 3 * TILE_SIZE, oy + 7 * TILE_SIZE, 14, 14, 0x854D0EFF);
    draw_filled_rect(ox + 3 * TILE_SIZE + 2, oy + 7 * TILE_SIZE + 2, 10, 10, 0x10B981FF);

    int vx = ox + (int)s_vaati.x;
    int vy = oy + (int)s_vaati.y;

    // Relâmpago de tempestade
    if (s_vaati.storm_flash_timer > 0) {
        draw_filled_rect(0, 0, SCREEN_W, SCREEN_H, 0xEDE9FE33);
    }

    // ==========================================
    // RENDER: FASE 1 (VAATI REBORN)
    // ==========================================
    if (s_vaati.phase == VAATI_PHASE_1_REBORN) {
        // Aura de malícia
        draw_filled_rect(vx - 18, vy - 18, 36, 36, 0x3B076444);

        // Manto Púrpura de Feiticeiro
        draw_filled_rect(vx - 10, vy - 6, 20, 24, C_VAATI_ROBE);
        draw_filled_rect(vx - 12, vy + 12, 24, 8, C_VAATI_ROBE);

        // Cabeça & Cabelo Lilás
        draw_filled_rect(vx - 7, vy - 16, 14, 10, C_VAATI_SKIN);
        draw_filled_rect(vx - 8, vy - 20, 16, 6, C_VAATI_HAIR);
        draw_filled_rect(vx - 10, vy - 16, 3, 12, C_VAATI_HAIR); // Mecha L
        draw_filled_rect(vx + 7, vy - 16, 3, 12, C_VAATI_HAIR);  // Mecha R
        // Olhos vermelhos
        draw_filled_rect(vx - 4, vy - 13, 2, 2, C_VAATI_EYE_RED);
        draw_filled_rect(vx + 2, vy - 13, 2, 2, C_VAATI_EYE_RED);

        // Olho Peitoral
        if (s_vaati.eye_open) {
            draw_filled_rect(vx - 6, vy, 12, 10, 0xFEF08AFF);
            draw_filled_rect(vx - 4, vy + 2, 8, 6, C_VAATI_EYE_RED);
            draw_filled_rect(vx - 1, vy + 4, 2, 2, 0xFFFFFFFF);
        } else {
            draw_filled_rect(vx - 4, vy + 2, 8, 4, C_VAATI_GOLD);
        }

        // 4 Orbs Protetores
        for (int i = 0; i < 4; i++) {
            if (s_vaati.orbs[i].active) {
                int ox_pos = ox + (int)s_vaati.orbs[i].x;
                int oy_pos = oy + (int)s_vaati.orbs[i].y;
                draw_filled_rect(ox_pos - 6, oy_pos - 6, 12, 12, 0x4C0519FF);
                draw_filled_rect(ox_pos - 4, oy_pos - 4, 8, 8, C_VAATI_EYE_RED);
                draw_filled_rect(ox_pos - 1, oy_pos - 1, 2, 2, 0xFFFFFFFF);
            }
        }
    }

    // ==========================================
    // RENDER: FASE 2 (VAATI TRANSFIGURED)
    // ==========================================
    else if (s_vaati.phase == VAATI_PHASE_2_TRANSFIGURED) {
        // Asas Demoníacas
        int wing_flap = (int)(sinf(s_vaati.wing_anim_angle) * 8.0f);
        draw_filled_rect(vx - 40, vy - 16 + wing_flap, 24, 30, C_VAATI_WING);
        draw_filled_rect(vx + 16, vy - 16 - wing_flap, 24, 30, C_VAATI_WING);

        // Esfera Ocular Colossal
        draw_filled_rect(vx - 20, vy - 20, 40, 40, 0x1E1B4BFF);
        draw_filled_rect(vx - 16, vy - 16, 32, 32, 0x4C0519FF);
        draw_filled_rect(vx - 10, vy - 10, 20, 20, C_VAATI_EYE_RED);
        draw_filled_rect(vx - 4, vy - 4, 8, 8, 0xFFFFFFFF);

        // 8 Olhos em Órbita
        for (int i = 0; i < 8; i++) {
            if (s_vaati.orbs[i].active) {
                int ox_pos = ox + (int)s_vaati.orbs[i].x;
                int oy_pos = oy + (int)s_vaati.orbs[i].y;
                u32 col = s_vaati.orbs[i].revealed ? C_VAATI_EYE_RED : 0x475569FF;
                draw_filled_rect(ox_pos - 5, oy_pos - 5, 10, 10, col);
                draw_filled_rect(ox_pos - 2, oy_pos - 2, 4, 4, 0xFFFFFFFF);
            }
        }
    }

    // ==========================================
    // RENDER: FASE 3 (VAATI'S WRATH)
    // ==========================================
    else if (s_vaati.phase == VAATI_PHASE_3_WRATH) {
        // Asas Gigantescas
        draw_filled_rect(vx - 50, vy - 24, 32, 44, C_VAATI_WING);
        draw_filled_rect(vx + 18, vy - 24, 32, 44, C_VAATI_WING);

        // Olho Central Supremo com 4 Pupilas
        draw_filled_rect(vx - 24, vy - 24, 48, 48, 0x0F0E17FF);
        draw_filled_rect(vx - 18, vy - 18, 36, 36, C_VAATI_EYE_RED);
        // 4 Pupilas
        draw_filled_rect(vx - 10, vy - 10, 6, 6, 0xFEF08AFF);
        draw_filled_rect(vx + 4,  vy - 10, 6, 6, 0xFEF08AFF);
        draw_filled_rect(vx - 10, vy + 4,  6, 6, 0xFEF08AFF);
        draw_filled_rect(vx + 4,  vy + 4,  6, 6, 0xFEF08AFF);

        // Garras Demoníacas
        for (int i = 0; i < 2; i++) {
            VaatiClaw* cl = &s_vaati.claws[i];
            if (!cl->destroyed) {
                int cx_pos = ox + (int)cl->x;
                int cy_pos = oy + (int)cl->y;
                u32 ccol = cl->flipped ? 0xF59E0BFF : C_VAATI_CLAW;
                draw_filled_rect(cx_pos - 12, cy_pos - 12, 24, 24, ccol);
                draw_filled_rect(cx_pos - 8, cy_pos - 8, 16, 16, 0x4C0519FF);
                // Espinhos
                draw_filled_rect(cx_pos - 14, cy_pos - 4, 4, 8, 0xE2E8F0FF);
                draw_filled_rect(cx_pos + 10, cy_pos - 4, 4, 8, 0xE2E8F0FF);
            }
        }

        // Feixes de Laser refletidos
        if (s_vaati.laser_firing) {
            for (int l = 0; l < 4; l++) {
                int lx_pos = vx - 15 + l * 10;
                draw_filled_rect(lx_pos, vy + 24, 3, 50, C_VAATI_LASER);
                draw_filled_rect(lx_pos - 3, vy + 74, 9, 4, C_VAATI_SHIELD_GLOW);
            }
        }
    }

    // ==========================================
    // RENDER: CINEMÁTICA & EPÍLOGO
    // ==========================================
    else if (s_vaati.phase == VAATI_PHASE_DEFEAT_CINEMATIC) {
        // Luz sagrada de purificação
        draw_filled_rect(vx - 30, vy - 30, 60, 60, 0xFEF08A55);

        // Princesa Zelda no centro (curada)
        int zx = ox + 140;
        int zy = oy + 90;
        if (s_vaati_npcs_tex && s_vaati_npcs_tex->pixels) {
            texture_draw(s_vaati_npcs_tex, 0 * 16, 2 * 16, 16, 16, zx - 8, zy - 12);
        } else {
            draw_filled_rect(zx - 5, zy - 4, 10, 14, C_ZELDA_DRESS);
            draw_filled_rect(zx - 4, zy - 12, 8, 8, 0xFDE8CDFF); // Rosto
            draw_filled_rect(zx - 5, zy - 14, 10, 4, 0xFDE047FF); // Cabelo loiro
        }
        font_draw_text(zx - 20, zy + 14, "ZELDA", 0xF472B6FF, true);

        // Sábio Minish Ezlo em forma real
        if (s_vaati.ezlo_farewell) {
            int ex = ox + 115;
            int ey = oy + 96;
            draw_filled_rect(ex - 3, ey - 3, 6, 8, C_EZLO_MINISH);
            draw_filled_rect(ex - 3, ey - 8, 6, 5, 0xFDE8CDFF);
            draw_filled_rect(ex - 4, ey - 3, 8, 4, 0xFFFFFFFF); // Barba branca
            draw_filled_rect(ex - 4, ey - 12, 8, 4, 0x15803DFF); // Gorro verde
            font_draw_text(ex - 16, ey + 8, "EZLO", 0x4ADE80FF, true);
        }
    }

    // ==========================================
    // RENDER: CRÉDITOS FINAIS
    // ==========================================
    else if (s_vaati.phase == VAATI_PHASE_CREDITS) {
        draw_filled_rect(0, 0, SCREEN_W, SCREEN_H, 0x09090BFF);
        int cy = s_vaati.credits_scroll_y;

        font_draw_text(16, cy + 0,   "THE LEGEND OF ZELDA: THE MINISH CAP", 0xFDE047FF, true);
        font_draw_text(24, cy + 24,  "PRODUCAO & DESENVOLVIMENTO", 0x38BDF8FF, true);
        font_draw_text(32, cy + 40,  "CAPCOM & NINTENDO", 0xFFFFFFFF, false);
        font_draw_text(24, cy + 68,  "PORT NATIVO CLEAN-ROOM C11", 0x38BDF8FF, true);
        font_draw_text(32, cy + 84,  "OPENMINISH ENGINE HAL", 0xFFFFFFFF, false);
        font_draw_text(32, cy + 98,  "ARQUITETO: Thiago De Carlo", 0x4ADE80FF, true);
        font_draw_text(24, cy + 126, "DIRETOR ORIGINAL", 0x38BDF8FF, true);
        font_draw_text(32, cy + 142, "Hidemaro Fujibayashi", 0xFFFFFFFF, false);
        font_draw_text(24, cy + 170, "PRODUTOR EXECUTIVO", 0x38BDF8FF, true);
        font_draw_text(32, cy + 186, "Eiji Aonuma & Keiji Inafune", 0xFFFFFFFF, false);
        font_draw_text(24, cy + 220, "PARABENS! O REINO DE HYRULE ESTA SALVO!", 0xFDE047FF, true);
        font_draw_text(50, cy + 244, "OBRIGADO POR JOGAR!", 0x38BDF8FF, true);
    }

    // Barra de Vida do Chefe Vaati (Fases 1 a 3)
    if (s_vaati.phase < VAATI_PHASE_DEFEAT_CINEMATIC && s_vaati.hp > 0) {
        int bar_w = 120;
        int bar_h = 6;
        int bar_x = (SCREEN_W - bar_w) / 2;
        int bar_y = 20;

        draw_filled_rect(bar_x - 1, bar_y - 1, bar_w + 2, bar_h + 2, 0x000000AA);
        draw_filled_rect(bar_x, bar_y, bar_w, bar_h, 0x475569FF);

        int fill_w = (s_vaati.hp * bar_w) / s_vaati.max_hp;
        draw_filled_rect(bar_x, bar_y, fill_w, bar_h, C_VAATI_EYE_RED);

        const char* p_name = (s_vaati.phase == VAATI_PHASE_1_REBORN) ? "VAATI REBORN" :
                             (s_vaati.phase == VAATI_PHASE_2_TRANSFIGURED) ? "VAATI TRANSFIGURED" : "VAATI'S WRATH";
        font_draw_text(bar_x + 10, bar_y - 8, p_name, 0xFDE047FF, true);
    }
}

bool vaati_boss_is_defeated(void) {
    return s_vaati.phase >= VAATI_PHASE_DEFEAT_CINEMATIC;
}

bool vaati_boss_is_credits(void) {
    return s_vaati.phase == VAATI_PHASE_CREDITS;
}
