/*
 * ============================================================================
 * src/hal/sanctuary.c - Implementação do Santuário Elemental & Divisão Four Sword (3 Clones)
 * ============================================================================
 */

#include "hal/sanctuary.h"
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

// Paleta de Cores Sacras do Santuário Elemental
#define C_SANC_FLOOR_LIGHT   0xE2E8F0FF // Mármore claro polido
#define C_SANC_FLOOR_DARK    0xCBD5E1FF // Friso de mármore azulado
#define C_SANC_FLOOR_RUNE    0x94A3B8FF // Gravuras sacras em baixo relevo
#define C_SANC_WALL_FACE     0x1E293BFF // Muro basáltico ancestral
#define C_SANC_WALL_TOP      0x334155FF // Cornija superior de ardósia
#define C_SANC_GOLD_TRIM     0xF59E0BFF // Frisos dourados em folha de ouro
#define C_SANC_PEDESTAL_BASE 0x475569FF // Pedestal de pedra da Four Sword
#define C_SANC_PEDESTAL_TOP  0x64748BFF // Topo do pedestal
#define C_SANC_EARTH_ORB     0x22C55EFF // Esfera mística do Elemento Terra (Verde)
#define C_SANC_FIRE_ORB      0xEF4444FF // Esfera mística do Elemento Fogo (Vermelho)
#define C_SANC_WATER_ORB     0x06B6D4FF // Esfera mística do Elemento da Água (Ciano Safira)
#define C_SANC_PAD_IDLE      0x0284C7FF // Piso de divisão desativado
#define C_SANC_PAD_ACTIVE    0x38BDF8FF // Piso de divisão energizado
#define C_SANC_SWITCH_BASE   0x334155FF // Base do interruptor
#define C_SANC_SWITCH_PLATE  0xD97706FF // Prato metálico do interruptor
#define C_SANC_GATE_BARS     0x64748BFF // Grades da cancela sagrada
#define C_SANC_HEAVY_BLOCK   0x475569FF // Bloco colossal de 3 heróis

static ElementalSanctuaryState s_sanc = { 0 };
static int s_sanc_anim = 0;

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

void sanctuary_init(void) {
    memset(&s_sanc, 0, sizeof(s_sanc));
    s_sanc.active = false;
    s_sanc.transitioning = false;
    s_sanc.pedestal_infused = false;
    s_sanc.pedestal_infused_three = false;
    s_sanc.cutscene_playing = false;
    s_sanc.cutscene_is_three = false;
    s_sanc.cutscene_timer = 0;

    // Altar da Four Sword (Norte central)
    s_sanc.pedestal_x = 7.5f * TILE_SIZE;
    s_sanc.pedestal_y = 2.5f * TILE_SIZE;

    // Pedestais das Esferas Elementais
    s_sanc.earth_orb_x = 4.5f * TILE_SIZE; // Esquerda
    s_sanc.earth_orb_y = 2.5f * TILE_SIZE;
    s_sanc.fire_orb_x  = 10.5f * TILE_SIZE; // Direita
    s_sanc.fire_orb_y  = 2.5f * TILE_SIZE;
    s_sanc.water_orb_x = 2.5f * TILE_SIZE; // Extremo Esquerda (conquistado no Templo das Gotas)
    s_sanc.water_orb_y = 2.5f * TILE_SIZE;

    // Trio de Pisos de Clones (Spaced horizontally across center of sanctuary)
    s_sanc.pads[0].x = 5.5f * TILE_SIZE;
    s_sanc.pads[0].y = 6.0f * TILE_SIZE;
    s_sanc.pads[0].is_active = false;

    s_sanc.pads[1].x = 7.5f * TILE_SIZE;
    s_sanc.pads[1].y = 6.0f * TILE_SIZE;
    s_sanc.pads[1].is_active = false;

    s_sanc.pads[2].x = 9.5f * TILE_SIZE;
    s_sanc.pads[2].y = 6.0f * TILE_SIZE;
    s_sanc.pads[2].is_active = false;

    for (int p = 0; p < MAX_SANCTUARY_PADS; p++) {
        s_sanc.pad_charged[p] = false;
    }

    // Clones iniciais inativos
    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        s_sanc.clones[c].active = false;
    }
    // Clone 0: Red Link
    s_sanc.clones[0].tunic_color = 0xDC2626FF;
    s_sanc.clones[0].cap_color   = 0xEF4444FF;
    s_sanc.clones[0].aura_color  = 0xF8717144;
    // Clone 1: Blue Link
    s_sanc.clones[1].tunic_color = 0x0284C7FF;
    s_sanc.clones[1].cap_color   = 0x38BDF8FF;
    s_sanc.clones[1].aura_color  = 0x38BDF844;

    // Interruptores triplos no solo (x=5.5, x=7.5, x=9.5, y=8.0)
    for (int sw = 0; sw < 3; sw++) {
        s_sanc.switch_down[sw] = false;
    }
    s_sanc.gate_open = false;
    s_sanc.treasury_gate_open = false;

    // Bloco Colossal de Empurrão dos 3 Heróis
    s_sanc.heavy_block_x = 7.0f * TILE_SIZE;
    s_sanc.heavy_block_y = 4.0f * TILE_SIZE;
    s_sanc.heavy_block_pushed = false;

    // Ponto seguro de entrada
    s_sanc.safe_x = 7.5f * TILE_SIZE;
    s_sanc.safe_y = 10.0f * TILE_SIZE;
}

bool sanctuary_is_active(void) {
    return s_sanc.active;
}

ElementalSanctuaryState* sanctuary_get_state(void) {
    return &s_sanc;
}

void sanctuary_enter(float* link_x, float* link_y, Direction* link_dir) {
    s_sanc.active = true;
    s_sanc.transitioning = false;
    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        s_sanc.clones[c].active = false;
    }

    if (link_x) *link_x = 7.5f * TILE_SIZE;
    if (link_y) *link_y = 10.0f * TILE_SIZE;
    if (link_dir) *link_dir = DIR_UP;

    s_sanc.safe_x = 7.5f * TILE_SIZE;
    s_sanc.safe_y = 10.0f * TILE_SIZE;

    hal_audio_play_bgm(BGM_MINISH_WOODS);
    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
    printf("[SANCTUARY] Link adentrou o sagrado Santuario Elemental de Hyrule!\n");
}

void sanctuary_exit(float* link_x, float* link_y, Direction* link_dir) {
    s_sanc.active = false;
    s_sanc.transitioning = false;
    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        s_sanc.clones[c].active = false;
    }

    if (link_x) *link_x = 248.0f;
    if (link_y) *link_y = 36.0f;
    if (link_dir) *link_dir = DIR_DOWN;

    hal_audio_play_sound(SOUND_SECRET, 0.85f, 1.2f);
    printf("[SANCTUARY] Link saiu do Santuario Elemental de volta aos campos de Hyrule!\n");
}

bool sanctuary_has_two_elements(void) {
    return s_sanc.pedestal_infused;
}

bool sanctuary_has_three_elements(void) {
    return s_sanc.pedestal_infused_three;
}

void sanctuary_set_two_elements(bool infused) {
    s_sanc.pedestal_infused = infused;
}

void sanctuary_set_three_elements(bool infused) {
    s_sanc.pedestal_infused_three = infused;
    if (infused) {
        s_sanc.pedestal_infused = true;
    }
}

bool sanctuary_is_clone_active(void) {
    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        if (s_sanc.clones[c].active) return true;
    }
    return false;
}

int sanctuary_get_active_clone_count(void) {
    int count = 0;
    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        if (s_sanc.clones[c].active) count++;
    }
    return count;
}

CloneState* sanctuary_get_clone(void) {
    return &s_sanc.clones[0];
}

CloneState* sanctuary_get_clone_at(int idx) {
    if (idx < 0 || idx >= MAX_SANCTUARY_CLONES) return NULL;
    return &s_sanc.clones[idx];
}

bool sanctuary_get_clone_sword_hitbox(float* out_x, float* out_y, float* out_w, float* out_h, int* out_dmg) {
    return sanctuary_get_any_clone_sword_hitbox(0, out_x, out_y, out_w, out_h, out_dmg);
}

bool sanctuary_get_any_clone_sword_hitbox(int clone_idx, float* out_x, float* out_y, float* out_w, float* out_h, int* out_dmg) {
    if (clone_idx < 0 || clone_idx >= MAX_SANCTUARY_CLONES) return false;
    CloneState* cl = &s_sanc.clones[clone_idx];
    if (!cl->active || !cl->is_attacking) return false;

    float hx = cl->x + 4.0f;
    float hy = cl->y + 4.0f;
    float hw = 12.0f;
    float hh = 12.0f;

    if (cl->dir == DIR_DOWN)  { hx = cl->x + 1.0f;  hy = cl->y + 14.0f; hw = 14.0f; hh = 12.0f; }
    if (cl->dir == DIR_UP)    { hx = cl->x + 1.0f;  hy = cl->y - 10.0f; hw = 14.0f; hh = 12.0f; }
    if (cl->dir == DIR_LEFT)  { hx = cl->x - 12.0f; hy = cl->y + 2.0f;  hw = 12.0f; hh = 14.0f; }
    if (cl->dir == DIR_RIGHT) { hx = cl->x + 14.0f; hy = cl->y + 2.0f;  hw = 12.0f; hh = 14.0f; }

    if (out_x) *out_x = hx;
    if (out_y) *out_y = hy;
    if (out_w) *out_w = hw;
    if (out_h) *out_h = hh;
    if (out_dmg) *out_dmg = 2; // Espada Branca

    return true;
}

bool sanctuary_interact(float link_x, float link_y, bool has_white_sword,
                        bool has_earth_element, bool has_fire_element, bool has_water_element) {
    if (!s_sanc.active) return false;

    // Distância do Altar Pedestal
    float dx = link_x - s_sanc.pedestal_x;
    float dy = link_y - s_sanc.pedestal_y;
    if (dx * dx + dy * dy <= 30.0f * 30.0f) {
        // Se já possui 3 elementos e ainda não infundiu o 3º:
        if (s_sanc.pedestal_infused && !s_sanc.pedestal_infused_three) {
            if (has_water_element) {
                // Inicia Cutscene Sagrada de Infusão do 3º Elemento (Água)!
                s_sanc.cutscene_playing = true;
                s_sanc.cutscene_is_three = true;
                s_sanc.cutscene_timer = 200;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                hal_audio_play_sound(SOUND_TIGER_SCROLL, 1.0f, 1.1f);
                entity_trigger_screen_shake(18, 4);
                printf("[SANCTUARY] CERIMONIA DE INFUSAO DO 3º ELEMENTO INICIADA: A agua sagrada desperta o poder dos 3 Clones!\n");
                return true;
            } else {
                printf("[SANCTUARY] O Altar aguarda o Sagrado Elemento da Agua obtido no Templo das Gotas!\n");
            }
        }
        // Se ainda não infundiu os 2 primeiros:
        else if (!s_sanc.pedestal_infused) {
            if (has_white_sword && has_earth_element && has_fire_element) {
                s_sanc.cutscene_playing = true;
                s_sanc.cutscene_is_three = false;
                s_sanc.cutscene_timer = 180;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.25f);
                hal_audio_play_sound(SOUND_TIGER_SCROLL, 1.0f, 1.0f);
                entity_trigger_screen_shake(15, 3);
                printf("[SANCTUARY] CERIMONIA DE INFUSAO INICIADA: Terra e Fogo despertam a Four Sword!\n");
                return true;
            } else {
                printf("[SANCTUARY] O Altar aguarda a White Sword e os 2 primeiros Elementos Sagrados!\n");
            }
        }
    }

    return false;
}

bool sanctuary_push_heavy_block(float link_x, float link_y, Direction dir, bool is_moving) {
    if (!s_sanc.active || s_sanc.heavy_block_pushed) return false;

    // Bloco requer 3 heróis (Link + 2 Clones) empurrando juntos para cima
    if (sanctuary_get_active_clone_count() < 2) return false;
    if (dir != DIR_UP || !is_moving) return false;

    // Checa posição de Link logo abaixo do bloco
    float bx = s_sanc.heavy_block_x;
    float by = s_sanc.heavy_block_y;
    if (link_x >= bx - 8.0f && link_x <= bx + 40.0f && link_y >= by + 10.0f && link_y <= by + 26.0f) {
        // Empurra o bloco colossal!
        s_sanc.heavy_block_y -= 24.0f;
        s_sanc.heavy_block_pushed = true;
        hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 0.8f);
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
        entity_trigger_screen_shake(14, 3);
        printf("[SANCTUARY] FORCA DOS 3 HERÓIS! O Bloco Colossal foi empurrado, revelando a Sala do Tesouro!\n");
        return true;
    }
    return false;
}

void sanctuary_update(float* link_x, float* link_y, Direction* link_dir,
                      bool link_moving, bool is_charging, bool is_charge_ready,
                      bool is_attacking, int attack_timer,
                      int* link_hearts) {
    (void)link_hearts;
    if (!s_sanc.active) return;

    s_sanc_anim++;

    float lx = (link_x ? *link_x : 0.0f);
    float ly = (link_y ? *link_y : 0.0f);

    // 1. Cutscene de Infusão
    if (s_sanc.cutscene_playing) {
        s_sanc.cutscene_timer--;
        if (s_sanc.cutscene_timer == 120) {
            entity_trigger_screen_shake(20, 4);
            hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 1.2f);
        }
        if (s_sanc.cutscene_timer <= 0) {
            s_sanc.cutscene_playing = false;
            if (s_sanc.cutscene_is_three) {
                s_sanc.pedestal_infused_three = true;
                s_sanc.pedestal_infused = true;
                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.2f);
                hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.1f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                printf("[SANCTUARY] INFUSAO DOS 3 ELEMENTOS CONCLUIDA! White Sword (Three Elements) forjada! Divisao em 3 Clones desbloqueada!\n");
            } else {
                s_sanc.pedestal_infused = true;
                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.0f);
                hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.0f);
                printf("[SANCTUARY] INFUSAO CONCLUIDA! White Sword (Two Elements) forjada com sucesso!\n");
            }
        }
        return; // Pausa movimentação durante a cutscene
    }

    // 2. Saída sul para o North Hyrule Field
    if (ly >= 10.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
        sanctuary_exit(link_x, link_y, link_dir);
        return;
    }

    // 3. Mecânica Canônica de Divisão de Clones (Split Pads)
    // Se possui 3 Elementos: usa todos os 3 blocos (pads[0], pads[1], pads[2])
    // Se possui 2 Elementos: usa 2 blocos (pads[0] e pads[2])
    if (s_sanc.pedestal_infused_three) {
        if (is_charge_ready) {
            for (int p = 0; p < 3; p++) {
                float dp = sqrtf((lx - s_sanc.pads[p].x) * (lx - s_sanc.pads[p].x) + (ly - s_sanc.pads[p].y) * (ly - s_sanc.pads[p].y));
                if (dp < 12.0f && !s_sanc.pad_charged[p]) {
                    s_sanc.pad_charged[p] = true;
                    s_sanc.pads[p].is_active = true;
                    hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.3f + p * 0.15f);
                    printf("[FOUR SWORD] Pad %d energizado com carga elemental sagrada!\n", p + 1);
                }
            }

            // Se os 3 blocos forem tocados com carga completa: DIVISÃO EM 3 CLONES!
            if (s_sanc.pad_charged[0] && s_sanc.pad_charged[1] && s_sanc.pad_charged[2] &&
                !s_sanc.clones[0].active && !s_sanc.clones[1].active) {

                // Clone 0: Red Link (offset para a esquerda/centro)
                s_sanc.clones[0].active = true;
                s_sanc.clones[0].offset_x = s_sanc.pads[0].x - s_sanc.pads[1].x;
                s_sanc.clones[0].offset_y = 0.0f;
                s_sanc.clones[0].x = lx + s_sanc.clones[0].offset_x;
                s_sanc.clones[0].y = ly + s_sanc.clones[0].offset_y;
                s_sanc.clones[0].dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clones[0].lifetime = MAX_CLONE_LIFETIME;
                s_sanc.clones[0].is_moving = false;
                s_sanc.clones[0].is_attacking = false;

                // Clone 1: Blue Link (offset para a direita)
                s_sanc.clones[1].active = true;
                s_sanc.clones[1].offset_x = s_sanc.pads[2].x - s_sanc.pads[1].x;
                s_sanc.clones[1].offset_y = 0.0f;
                s_sanc.clones[1].x = lx + s_sanc.clones[1].offset_x;
                s_sanc.clones[1].y = ly + s_sanc.clones[1].offset_y;
                s_sanc.clones[1].dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clones[1].lifetime = MAX_CLONE_LIFETIME;
                s_sanc.clones[1].is_moving = false;
                s_sanc.clones[1].is_attacking = false;

                for (int p = 0; p < 3; p++) {
                    s_sanc.pad_charged[p] = false;
                    s_sanc.pads[p].is_active = false;
                }

                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.4f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                entity_trigger_screen_shake(12, 3);
                printf("[FOUR SWORD] DIVISAO EM 3 CLONES CONCLUIDA! Green Link, Red Link e Blue Link marcham juntos!\n");
            }
        } else if (!is_charging) {
            for (int p = 0; p < 3; p++) {
                s_sanc.pad_charged[p] = false;
                s_sanc.pads[p].is_active = false;
            }
        }
    }
    // Caso de 2 Elementos (retrocompatibilidade)
    else if (s_sanc.pedestal_infused) {
        float d1 = sqrtf((lx - s_sanc.pads[0].x) * (lx - s_sanc.pads[0].x) + (ly - s_sanc.pads[0].y) * (ly - s_sanc.pads[0].y));
        float d2 = sqrtf((lx - s_sanc.pads[2].x) * (lx - s_sanc.pads[2].x) + (ly - s_sanc.pads[2].y) * (ly - s_sanc.pads[2].y));

        if (is_charge_ready) {
            if (d1 < 12.0f && !s_sanc.pad_charged[0]) {
                s_sanc.pad_charged[0] = true;
                s_sanc.pads[0].is_active = true;
                hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.5f);
            }
            if (d2 < 12.0f && !s_sanc.pad_charged[2]) {
                s_sanc.pad_charged[2] = true;
                s_sanc.pads[2].is_active = true;
                hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.5f);
            }

            if (s_sanc.pad_charged[0] && s_sanc.pad_charged[2] && !s_sanc.clones[0].active) {
                s_sanc.clones[0].active = true;
                s_sanc.clones[0].offset_x = (d1 < d2) ? (s_sanc.pads[2].x - s_sanc.pads[0].x) : (s_sanc.pads[0].x - s_sanc.pads[2].x);
                s_sanc.clones[0].offset_y = 0.0f;
                s_sanc.clones[0].x = lx + s_sanc.clones[0].offset_x;
                s_sanc.clones[0].y = ly + s_sanc.clones[0].offset_y;
                s_sanc.clones[0].dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clones[0].lifetime = MAX_CLONE_LIFETIME;
                s_sanc.clones[0].is_moving = false;
                s_sanc.clones[0].is_attacking = false;

                s_sanc.pad_charged[0] = false;
                s_sanc.pad_charged[2] = false;
                s_sanc.pads[0].is_active = false;
                s_sanc.pads[2].is_active = false;

                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.4f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                entity_trigger_screen_shake(10, 2);
                printf("[FOUR SWORD] DIVISAO CONCLUIDA! 2 Clones simultaneos ativos!\n");
            }
        } else if (!is_charging) {
            s_sanc.pad_charged[0] = false;
            s_sanc.pad_charged[2] = false;
            s_sanc.pads[0].is_active = false;
            s_sanc.pads[2].is_active = false;
        }
    }

    // 4. Atualização e Sincronização de Todos os Clones Ativos
    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        CloneState* cl = &s_sanc.clones[c];
        if (cl->active) {
            cl->lifetime--;

            cl->x = lx + cl->offset_x;
            cl->y = ly + cl->offset_y;
            cl->dir = (link_dir ? *link_dir : DIR_UP);
            cl->is_moving = link_moving;
            cl->is_attacking = is_attacking;
            cl->attack_timer = attack_timer;
            if (link_moving) {
                cl->anim_timer++;
                if (cl->anim_timer % 12 == 0) {
                    cl->anim_frame = (cl->anim_frame == 0) ? 1 : 0;
                }
            }

            // Dissipação do clone se colidir com parede sólida
            if (sanctuary_is_solid(cl->x + 8.0f, cl->y + 12.0f)) {
                cl->active = false;
                hal_audio_play_sound(SOUND_GUST_BLAST, 0.9f, 1.2f);
                printf("[FOUR SWORD] Clone %d colidiu com obstaculo e se dissipou em fumaca!\n", c + 1);
            }

            if (cl->lifetime <= 0) {
                cl->active = false;
                hal_audio_play_sound(SOUND_GUST_BLAST, 0.9f, 1.2f);
            }
        }
    }

    // 5. Quebra-cabeça de Interruptores Triplos (x=5.5, x=7.5, x=9.5, y=8.0)
    float sw_pos[3][2] = {
        { 5.5f * TILE_SIZE + 8.0f, 8.0f * TILE_SIZE + 8.0f },
        { 7.5f * TILE_SIZE + 8.0f, 8.0f * TILE_SIZE + 8.0f },
        { 9.5f * TILE_SIZE + 8.0f, 8.0f * TILE_SIZE + 8.0f }
    };

    bool sw_down[3] = { false, false, false };
    for (int s = 0; s < 3; s++) {
        // Checa se Link pisa no interruptor
        if (fabsf(lx + 8.0f - sw_pos[s][0]) < 10.0f && fabsf(ly + 12.0f - sw_pos[s][1]) < 10.0f) {
            sw_down[s] = true;
        }
        // Checa se qualquer clone ativo pisa no interruptor
        for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
            if (s_sanc.clones[c].active &&
                fabsf(s_sanc.clones[c].x + 8.0f - sw_pos[s][0]) < 10.0f &&
                fabsf(s_sanc.clones[c].y + 12.0f - sw_pos[s][1]) < 10.0f) {
                sw_down[s] = true;
            }
        }
        s_sanc.switch_down[s] = sw_down[s];
    }

    // Se os interruptores 0 e 2 forem pressionados: Destrava o Portão 1
    if (sw_down[0] && sw_down[2] && !s_sanc.gate_open) {
        s_sanc.gate_open = true;
        hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.0f);
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
        entity_trigger_screen_shake(12, 3);
        printf("[SANCTUARY] INTERRUPTORES LATERAIS ACIONADOS! Portao sagrado elevado!\n");
    }

    // Se TODOS os 3 interruptores forem pressionados simultaneamente: Destrava o Portão do Tesouro Sagrado!
    if (sw_down[0] && sw_down[1] && sw_down[2] && !s_sanc.treasury_gate_open) {
        s_sanc.treasury_gate_open = true;
        hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
        entity_trigger_screen_shake(16, 4);
        printf("[SANCTUARY] TODOS OS 3 INTERRUPTORES ACIONADOS! Grande Portao da Camara Secreta Destravado!\n");
    }

    // Checa empurrão do Bloco Colossal
    sanctuary_push_heavy_block(lx, ly, (link_dir ? *link_dir : DIR_UP), link_moving);
}

bool sanctuary_is_solid(float world_x, float world_y) {
    if (!s_sanc.active) return false;

    int col = (int)(world_x / TILE_SIZE);
    int row = (int)(world_y / TILE_SIZE);

    if (col < 0 || col >= SANCTUARY_ROOM_W || row < 0 || row >= SANCTUARY_ROOM_H) {
        return true;
    }

    // Parede Norte (Linhas 0 e 1)
    if (row <= 1) return true;

    // Parede Sul (Linha 11) - Abertura em x=7 e x=8
    if (row >= 11) {
        if (col == 7 || col == 8) return false;
        return true;
    }

    // Paredes Laterais Oeste e Leste
    if (col <= 1 || col >= 14) return true;

    // Altar Pedestal Central (col 7 e 8, row 2 e 3)
    if ((col == 7 || col == 8) && (row == 2 || row == 3)) return true;

    // Pedestais das Esferas Elementais (Terra em col 4, Fogo em col 10, Água em col 2)
    if ((col == 4 || col == 10 || col == 2) && row == 2) return true;

    // Cancela / Portão Sagrado 1 (Linha 4, entre colunas 5 a 10)
    if (row == 4 && (col >= 5 && col <= 10)) {
        if (!s_sanc.gate_open) return true;
    }

    // Bloco Colossal de 3 Heróis (Se não empurrado, bloqueia em x=7..8, y=4)
    if (!s_sanc.heavy_block_pushed) {
        if ((col == 7 || col == 8) && row == 4) return true;
    }

    return false;
}

void sanctuary_render(const Camera* cam, float link_x, float link_y, Direction link_dir) {
    (void)link_x;
    (void)link_y;
    (void)link_dir;
    if (!s_sanc.active) return;

    int ox = (cam ? -(int)cam->x : 0);
    int oy = (cam ? -(int)cam->y : 0);

    // 1. Renderiza o Piso de Mármore Polido e Lajes Sagradas
    for (int r = 0; r < SANCTUARY_ROOM_H; r++) {
        for (int c = 0; c < SANCTUARY_ROOM_W; c++) {
            int tx = ox + (c * TILE_SIZE);
            int ty = oy + (r * TILE_SIZE);

            u32 c_tile = ((c + r) % 2 == 0) ? C_SANC_FLOOR_LIGHT : C_SANC_FLOOR_DARK;
            draw_filled_rect(tx, ty, TILE_SIZE, TILE_SIZE, c_tile);
            draw_filled_rect(tx, ty, TILE_SIZE, 1, C_SANC_FLOOR_RUNE);
            draw_filled_rect(tx, ty, 1, TILE_SIZE, C_SANC_FLOOR_RUNE);
        }
    }

    // 2. Vitrais Sagrados nas Paredes Laterais (feixes de luz colorida no piso)
    for (int v = 0; v < 3; v++) {
        int vy = oy + (3 + v * 2) * TILE_SIZE;
        u32 beam_c = (v == 0) ? 0x06B6D422 : ((v == 1) ? 0x22C55E22 : 0xEF444422);
        draw_rect_blend(ox + 2 * TILE_SIZE, vy, 40, 24, beam_c);
        draw_rect_blend(ox + 11 * TILE_SIZE, vy, 40, 24, beam_c);
    }

    // 3. Trio de Pisos de Clones (Split Pads)
    int max_p = s_sanc.pedestal_infused_three ? 3 : (s_sanc.pedestal_infused ? 3 : 0);
    for (int p = 0; p < max_p; p++) {
        // Se só tem 2 elementos, desenha os pads 0 e 2
        if (!s_sanc.pedestal_infused_three && p == 1) continue;

        int px = ox + (int)s_sanc.pads[p].x;
        int py = oy + (int)s_sanc.pads[p].y;
        bool lit = s_sanc.pads[p].is_active;

        u32 c_border = lit ? 0xFFFFFFFF : C_SANC_PAD_IDLE;
        u32 c_inner  = lit ? C_SANC_PAD_ACTIVE : 0x0369A1FF;

        draw_filled_rect(px + 1, py + 1, 14, 14, c_border);
        draw_filled_rect(px + 2, py + 2, 12, 12, c_inner);

        int pulse = (s_sanc_anim / 6) % 2;
        draw_filled_rect(px + 6, py + 4 - pulse, 4, 8 + pulse * 2, lit ? 0xFFFFFFFF : 0x38BDF8FF);
        draw_filled_rect(px + 4 - pulse, py + 6, 8 + pulse * 2, 4, lit ? 0xFFFFFFFF : 0x38BDF8FF);

        if (lit) {
            draw_rect_blend(px - 4, py - 4, 24, 24, 0x38BDF855);
        }
    }

    // 4. Interruptores de Piso Triplos (x=5.5, x=7.5, x=9.5, y=8.0)
    float sw_coords[3] = { 5.5f, 7.5f, 9.5f };
    for (int s = 0; s < 3; s++) {
        int sw_x = ox + (int)(sw_coords[s] * TILE_SIZE);
        int sw_y = oy + (int)(8.0f * TILE_SIZE);

        draw_filled_rect(sw_x + 2, sw_y + 2, 12, 12, C_SANC_SWITCH_BASE);
        draw_filled_rect(sw_x + 4, sw_y + 4, 8, 8, s_sanc.switch_down[s] ? 0x22C55EFF : C_SANC_SWITCH_PLATE);
    }

    // 5. Cancela / Portão Sagrado (Linha 4, entre colunas 5 e 10)
    int gx = ox + (int)(5.5f * TILE_SIZE);
    int gy = oy + (int)(4.0f * TILE_SIZE);
    if (!s_sanc.gate_open) {
        draw_filled_rect(gx, gy, 80, 16, C_SANC_GATE_BARS);
        for (int b = 0; b < 20; b++) {
            draw_filled_rect(gx + (b * 4), gy, 2, 16, 0x1E293BFF);
        }
        draw_filled_rect(gx + 36, gy + 4, 8, 8, C_SANC_GOLD_TRIM);
    } else {
        draw_filled_rect(gx, gy, 80, 4, C_SANC_GATE_BARS);
    }

    // 5b. Bloco Colossal de 3 Heróis
    int bx = ox + (int)s_sanc.heavy_block_x;
    int by = oy + (int)s_sanc.heavy_block_y;
    draw_filled_rect(bx, by, 32, 16, C_SANC_HEAVY_BLOCK);
    draw_filled_rect(bx + 2, by + 2, 28, 12, 0x64748BFF);
    draw_filled_rect(bx + 14, by + 4, 4, 8, 0xF59E0BFF); // Runa dourada dos 3 Heróis
    draw_filled_rect(bx + 10, by + 6, 12, 4, 0xF59E0BFF);

    // 6. Altar Pedestal da Four Sword & Esferas Elementais
    int ped_x = ox + (int)s_sanc.pedestal_x;
    int ped_y = oy + (int)s_sanc.pedestal_y;

    draw_filled_rect(ped_x - 12, ped_y - 8, 24, 18, C_SANC_PEDESTAL_BASE);
    draw_filled_rect(ped_x - 10, ped_y - 6, 20, 14, C_SANC_PEDESTAL_TOP);
    draw_filled_rect(ped_x - 12, ped_y + 8, 24, 3, C_SANC_GOLD_TRIM);

    // Espada Branca cravada no pedestal
    draw_filled_rect(ped_x - 1, ped_y - 18, 2, 14, 0xFFFFFFFF);
    draw_filled_rect(ped_x - 4, ped_y - 6, 8, 2, 0xFACC15FF); // Guarda asas
    draw_filled_rect(ped_x - 2, ped_y - 4, 4, 3, 0x78350FFF); // Empunhadura

    // Esfera do Elemento Terra (Verde Esmeralda)
    int ex = ox + (int)s_sanc.earth_orb_x;
    int ey = oy + (int)s_sanc.earth_orb_y;
    draw_filled_rect(ex - 8, ey, 16, 12, C_SANC_PEDESTAL_BASE);
    draw_filled_rect(ex - 6, ey - 10, 12, 12, C_SANC_EARTH_ORB);
    draw_filled_rect(ex - 3, ey - 7, 6, 6, 0x86EFACFF);
    draw_rect_blend(ex - 10, ey - 14, 20, 20, 0x22C55E33);

    // Esfera do Elemento Fogo (Vermelho Rubi)
    int fx = ox + (int)s_sanc.fire_orb_x;
    int fy = oy + (int)s_sanc.fire_orb_y;
    draw_filled_rect(fx - 8, fy, 16, 12, C_SANC_PEDESTAL_BASE);
    draw_filled_rect(fx - 6, fy - 10, 12, 12, C_SANC_FIRE_ORB);
    draw_filled_rect(fx - 3, fy - 7, 6, 6, 0xFDE047FF);
    draw_rect_blend(fx - 10, fy - 14, 20, 20, 0xEF444433);

    // Esfera do Elemento da Água (Ciano Safira - Terceiro Elemento!)
    int wx = ox + (int)s_sanc.water_orb_x;
    int wy = oy + (int)s_sanc.water_orb_y;
    draw_filled_rect(wx - 8, wy, 16, 12, C_SANC_PEDESTAL_BASE);
    draw_filled_rect(wx - 6, wy - 10, 12, 12, C_SANC_WATER_ORB);
    draw_filled_rect(wx - 3, wy - 7, 6, 6, 0xE0F2FEFF);
    draw_rect_blend(wx - 10, wy - 14, 20, 20, 0x06B6D433);

    // Cutscene de Infusão (2 ou 3 Elementos orbitando)
    if (s_sanc.cutscene_playing) {
        float angle = (float)s_sanc_anim * 0.15f;
        if (s_sanc.cutscene_is_three) {
            // 3 Orbes orbitando a 120 graus (2*PI / 3)
            int o1_x = ped_x + (int)(cosf(angle) * 24.0f);
            int o1_y = ped_y + (int)(sinf(angle) * 16.0f) - 10;
            int o2_x = ped_x + (int)(cosf(angle + 2.094f) * 24.0f);
            int o2_y = ped_y + (int)(sinf(angle + 2.094f) * 16.0f) - 10;
            int o3_x = ped_x + (int)(cosf(angle + 4.188f) * 24.0f);
            int o3_y = ped_y + (int)(sinf(angle + 4.188f) * 16.0f) - 10;

            draw_filled_rect(o1_x - 4, o1_y - 4, 8, 8, C_SANC_EARTH_ORB);
            draw_filled_rect(o2_x - 4, o2_y - 4, 8, 8, C_SANC_FIRE_ORB);
            draw_filled_rect(o3_x - 4, o3_y - 4, 8, 8, C_SANC_WATER_ORB);
            draw_rect_blend(ped_x - 20, ped_y - 30, 40, 44, 0x38BDF866);
        } else {
            int o1_x = ped_x + (int)(cosf(angle) * 22.0f);
            int o1_y = ped_y + (int)(sinf(angle) * 16.0f) - 10;
            int o2_x = ped_x + (int)(cosf(angle + PI_F) * 22.0f);
            int o2_y = ped_y + (int)(sinf(angle + PI_F) * 16.0f) - 10;

            draw_filled_rect(o1_x - 4, o1_y - 4, 8, 8, C_SANC_EARTH_ORB);
            draw_filled_rect(o2_x - 4, o2_y - 4, 8, 8, C_SANC_FIRE_ORB);
            draw_rect_blend(ped_x - 16, ped_y - 28, 32, 40, 0xFFFFFF66);
        }
    }

    // 7. Paredes do Santuário
    for (int c = 0; c < SANCTUARY_ROOM_W; c++) {
        draw_filled_rect(ox + c * TILE_SIZE, oy, TILE_SIZE, TILE_SIZE, C_SANC_WALL_FACE);
        draw_filled_rect(ox + c * TILE_SIZE, oy, TILE_SIZE, 4, C_SANC_WALL_TOP);
        draw_filled_rect(ox + c * TILE_SIZE, oy + 12, TILE_SIZE, 2, C_SANC_GOLD_TRIM);
    }
    for (int r = 0; r < SANCTUARY_ROOM_H; r++) {
        draw_filled_rect(ox, oy + r * TILE_SIZE, TILE_SIZE, TILE_SIZE, C_SANC_WALL_FACE);
        draw_filled_rect(ox + 15 * TILE_SIZE, oy + r * TILE_SIZE, TILE_SIZE, TILE_SIZE, C_SANC_WALL_FACE);
    }

    // 8. Renderiza os Clones da Four Sword
    sanctuary_render_clones(cam);
}

void sanctuary_render_clones(const Camera* cam) {
    int ox = (cam ? -(int)cam->x : 0);
    int oy = (cam ? -(int)cam->y : 0);

    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        CloneState* cl = &s_sanc.clones[c];
        if (!cl->active) continue;

        int cx = ox + (int)cl->x;
        int cy = oy + (int)cl->y;

        // Sombra suave sob o clone
        draw_filled_rect(cx + 3, cy + 13, 10, 3, 0x05100766);

        // Aura Four Sword colorida em volta do Clone
        draw_rect_blend(cx - 2, cy - 2, 20, 20, cl->aura_color);

        u32 c_cap       = cl->cap_color;
        u32 c_face      = 0xFDE8CDFF;
        u32 c_tunic     = cl->tunic_color;
        u32 c_tights    = 0xFFFFFFFF;
        u32 c_boots     = 0xB45309FF;

        // Gorro e Cabelo
        draw_filled_rect(cx + 4, cy + 1, 8, 6, c_cap);
        draw_filled_rect(cx + 4, cy + 6, 8, 6, c_face);
        draw_filled_rect(cx + 3, cy + 10, 10, 6, c_tunic);
        draw_filled_rect(cx + 4, cy + 15, 8, 3, c_tights);
        draw_filled_rect(cx + 4, cy + 17, 8, 3, c_boots);

        // Olhos e detalhes conforme a direção
        if (cl->dir == DIR_DOWN) {
            draw_filled_rect(cx + 5, cy + 8, 2, 2, 0x1E293BFF);
            draw_filled_rect(cx + 9, cy + 8, 2, 2, 0x1E293BFF);
        } else if (cl->dir == DIR_LEFT) {
            draw_filled_rect(cx + 4, cy + 8, 2, 2, 0x1E293BFF);
        } else if (cl->dir == DIR_RIGHT) {
            draw_filled_rect(cx + 10, cy + 8, 2, 2, 0x1E293BFF);
        }

        // Espada do Clone em ataque
        if (cl->is_attacking) {
            if (cl->dir == DIR_UP) {
                draw_filled_rect(cx + 7, cy - 10, 3, 12, 0xFFFFFFFF);
                draw_filled_rect(cx + 5, cy - 2, 7, 2, 0xFACC15FF);
            } else if (cl->dir == DIR_DOWN) {
                draw_filled_rect(cx + 7, cy + 15, 3, 12, 0xFFFFFFFF);
                draw_filled_rect(cx + 5, cy + 15, 7, 2, 0xFACC15FF);
            } else if (cl->dir == DIR_LEFT) {
                draw_filled_rect(cx - 10, cy + 8, 12, 3, 0xFFFFFFFF);
                draw_filled_rect(cx - 2, cy + 6, 2, 7, 0xFACC15FF);
            } else if (cl->dir == DIR_RIGHT) {
                draw_filled_rect(cx + 14, cy + 8, 12, 3, 0xFFFFFFFF);
                draw_filled_rect(cx + 14, cy + 6, 2, 7, 0xFACC15FF);
            }
        }

        // Partículas místicas de centelhas
        int spk = (s_sanc_anim / 4 + c * 2) % 3;
        draw_filled_rect(cx + 2 + spk * 4, cy - 2 - spk, 2, 2, cl->cap_color);
    }
}
