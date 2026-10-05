/*
 * ============================================================================
 * src/hal/sanctuary.c - Santuário Elemental & Four Sword Completa Forjada (4 Clones)
 * ============================================================================
 */

#include "hal/sanctuary.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/inventory.h"
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
#define C_SANC_WIND_ORB      0x10B981FF // Esfera mística do Elemento do Vento (Esmeralda Celeste)
#define C_SANC_PAD_IDLE      0x0284C7FF // Piso de divisão desativado
#define C_SANC_PAD_ACTIVE    0x38BDF8FF // Piso de divisão energizado
#define C_SANC_SWITCH_BASE   0x334155FF // Base do interruptor
#define C_SANC_SWITCH_PLATE  0xD97706FF // Prato metálico do interruptor
#define C_SANC_GATE_BARS     0x64748BFF // Grades da cancela sagrada
#define C_SANC_HEAVY_BLOCK   0x475569FF // Bloco colossal dos heróis

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
    s_sanc.pedestal_infused_four = false;
    s_sanc.cutscene_playing = false;
    s_sanc.cutscene_is_three = false;
    s_sanc.cutscene_is_four = false;
    s_sanc.cutscene_timer = 0;

    // Altar da Four Sword (Norte central)
    s_sanc.pedestal_x = 7.5f * TILE_SIZE;
    s_sanc.pedestal_y = 2.5f * TILE_SIZE;

    // Pedestais das 4 Esferas Elementais
    s_sanc.water_orb_x = 2.5f * TILE_SIZE;  // Extremo Esquerda (Água)
    s_sanc.water_orb_y = 2.5f * TILE_SIZE;
    s_sanc.earth_orb_x = 4.5f * TILE_SIZE;  // Meio Esquerda (Terra)
    s_sanc.earth_orb_y = 2.5f * TILE_SIZE;
    s_sanc.fire_orb_x  = 10.5f * TILE_SIZE; // Meio Direita (Fogo)
    s_sanc.fire_orb_y  = 2.5f * TILE_SIZE;
    s_sanc.wind_orb_x  = 12.5f * TILE_SIZE; // Extremo Direita (Vento)
    s_sanc.wind_orb_y  = 2.5f * TILE_SIZE;

    // 4 Pisos de Clones em Formação Sagrada de Diamante da Four Sword
    // Pad 0: Topo (Norte)
    s_sanc.pads[0].x = 7.5f * TILE_SIZE;
    s_sanc.pads[0].y = 5.0f * TILE_SIZE;
    s_sanc.pads[0].is_active = false;

    // Pad 1: Esquerda (Oeste)
    s_sanc.pads[1].x = 5.5f * TILE_SIZE;
    s_sanc.pads[1].y = 6.2f * TILE_SIZE;
    s_sanc.pads[1].is_active = false;

    // Pad 2: Direita (Leste)
    s_sanc.pads[2].x = 9.5f * TILE_SIZE;
    s_sanc.pads[2].y = 6.2f * TILE_SIZE;
    s_sanc.pads[2].is_active = false;

    // Pad 3: Fundo (Sul)
    s_sanc.pads[3].x = 7.5f * TILE_SIZE;
    s_sanc.pads[3].y = 7.4f * TILE_SIZE;
    s_sanc.pads[3].is_active = false;

    for (int p = 0; p < MAX_SANCTUARY_PADS; p++) {
        s_sanc.pad_charged[p] = false;
    }

    // 3 Clones simultâneos (Clone 0 = Red, Clone 1 = Blue, Clone 2 = Purple)
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
    // Clone 2: Purple Link (Violeta Canônico da Four Sword!)
    s_sanc.clones[2].tunic_color = 0x7E22CEFF;
    s_sanc.clones[2].cap_color   = 0xA855F7FF;
    s_sanc.clones[2].aura_color  = 0xC084FC44;

    // 4 Interruptores no solo
    for (int sw = 0; sw < 4; sw++) {
        s_sanc.switch_down[sw] = false;
    }
    s_sanc.gate_open = false;
    s_sanc.treasury_gate_open = false;
    s_sanc.secret_exit_unlocked = false;

    // Bloco Colossal de Empurrão dos Heróis
    s_sanc.heavy_block_x = 7.0f * TILE_SIZE;
    s_sanc.heavy_block_y = 4.0f * TILE_SIZE;
    s_sanc.heavy_block_pushed = false;

    // Ponto seguro de entrada
    s_sanc.safe_x = 7.5f * TILE_SIZE;
    s_sanc.safe_y = 10.0f * TILE_SIZE;

    // Zera os projéteis de Sword Beam
    for (int i = 0; i < MAX_SWORD_BEAMS; i++) {
        s_sanc.beams[i].active = false;
    }
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

bool sanctuary_has_four_elements(void) {
    return s_sanc.pedestal_infused_four;
}

void sanctuary_set_two_elements(bool infused) {
    s_sanc.pedestal_infused = infused;
}

void sanctuary_set_three_elements(bool infused) {
    s_sanc.pedestal_infused_three = infused;
    if (infused) s_sanc.pedestal_infused = true;
}

void sanctuary_set_four_elements(bool infused) {
    s_sanc.pedestal_infused_four = infused;
    if (infused) {
        s_sanc.pedestal_infused = true;
        s_sanc.pedestal_infused_three = true;
        s_sanc.secret_exit_unlocked = true;
        inventory_set_four_sword(true);
    }
}

bool sanctuary_is_clone_active(void) {
    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        if (s_sanc.clones[c].active) return true;
    }
    return false;
}

int sanctuary_get_active_clone_count(void) {
    int cnt = 0;
    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        if (s_sanc.clones[c].active) cnt++;
    }
    return cnt;
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
    if (out_dmg) *out_dmg = s_sanc.pedestal_infused_four ? 3 : 2;

    return true;
}

bool sanctuary_interact(float link_x, float link_y, bool has_white_sword,
                        bool has_earth_element, bool has_fire_element,
                        bool has_water_element, bool has_wind_element) {
    if (!s_sanc.active) return false;

    // Distância do Altar Pedestal
    float dx = link_x - s_sanc.pedestal_x;
    float dy = link_y - s_sanc.pedestal_y;
    if (dx * dx + dy * dy <= 32.0f * 32.0f) {
        // 1. Infusão do 4º Elemento (Four Sword Completa Forjada!)
        if (s_sanc.pedestal_infused_three && !s_sanc.pedestal_infused_four) {
            if (has_wind_element) {
                s_sanc.cutscene_playing = true;
                s_sanc.cutscene_is_four = true;
                s_sanc.cutscene_is_three = false;
                s_sanc.cutscene_timer = 240;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                hal_audio_play_sound(SOUND_TIGER_SCROLL, 1.0f, 1.2f);
                hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.4f);
                entity_trigger_screen_shake(25, 5);
                printf("[SANCTUARY] CERIMONIA FINAL: Os 4 Elementos Sagrados forjam a lendaria FOUR SWORD COMPLETA!\n");
                return true;
            } else {
                printf("[SANCTUARY] O Altar aguarda o sagrado Elemento do Vento do Palace of Winds!\n");
            }
        }
        // 2. Infusão do 3º Elemento (Água)
        else if (s_sanc.pedestal_infused && !s_sanc.pedestal_infused_three) {
            if (has_water_element) {
                s_sanc.cutscene_playing = true;
                s_sanc.cutscene_is_three = true;
                s_sanc.cutscene_is_four = false;
                s_sanc.cutscene_timer = 200;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                hal_audio_play_sound(SOUND_TIGER_SCROLL, 1.0f, 1.1f);
                entity_trigger_screen_shake(18, 4);
                return true;
            }
        }
        // 3. Infusão dos 2 primeiros Elementos (Terra + Fogo)
        else if (!s_sanc.pedestal_infused) {
            if (has_white_sword && has_earth_element && has_fire_element) {
                s_sanc.cutscene_playing = true;
                s_sanc.cutscene_is_three = false;
                s_sanc.cutscene_is_four = false;
                s_sanc.cutscene_timer = 180;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.25f);
                hal_audio_play_sound(SOUND_TIGER_SCROLL, 1.0f, 1.0f);
                entity_trigger_screen_shake(15, 3);
                return true;
            }
        }
    }

    return false;
}

bool sanctuary_push_heavy_block(float link_x, float link_y, Direction dir, bool is_moving) {
    if (!s_sanc.active || s_sanc.heavy_block_pushed) return false;

    // Bloco requer no mínimo 2 clones (3 heróis) empurrando juntos para cima
    if (sanctuary_get_active_clone_count() < 2) return false;
    if (dir != DIR_UP || !is_moving) return false;

    float bx = s_sanc.heavy_block_x;
    float by = s_sanc.heavy_block_y;
    if (link_x >= bx - 8.0f && link_x <= bx + 40.0f && link_y >= by + 10.0f && link_y <= by + 26.0f) {
        s_sanc.heavy_block_y -= 24.0f;
        s_sanc.heavy_block_pushed = true;
        hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.8f, 0.8f);
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
        entity_trigger_screen_shake(14, 3);
        printf("[SANCTUARY] FORCA COMBINADA! O Bloco Colossal foi empurrado!\n");
        return true;
    }
    return false;
}

void sanctuary_try_fire_sword_beam(float link_x, float link_y, Direction dir,
                                   int hearts, int max_hearts, bool has_four_sword) {
    if (!has_four_sword || hearts < max_hearts) return;

    for (int i = 0; i < MAX_SWORD_BEAMS; i++) {
        if (!s_sanc.beams[i].active) {
            SwordBeam* b = &s_sanc.beams[i];
            b->active = true;
            b->dir = dir;
            b->dmg = 3;
            b->life = 70;

            float speed = 4.4f;
            b->vx = 0.0f;
            b->vy = 0.0f;
            b->x = link_x + 8.0f;
            b->y = link_y + 8.0f;

            if (dir == DIR_UP)    { b->vy = -speed; b->y -= 12.0f; }
            if (dir == DIR_DOWN)  { b->vy =  speed; b->y += 12.0f; }
            if (dir == DIR_LEFT)  { b->vx = -speed; b->x -= 12.0f; }
            if (dir == DIR_RIGHT) { b->vx =  speed; b->x += 12.0f; }

            hal_audio_play_sound(SOUND_SPIN_READY, 0.9f, 1.4f);
            hal_audio_play_sound(SOUND_SWORD_SLASH, 1.0f, 1.3f);
            printf("[FOUR SWORD] SWORD BEAM DISPARADO COM VIDA CHEIA!\n");
            break;
        }
    }
}

void sanctuary_update_sword_beams(Tilemap* map) {
    for (int i = 0; i < MAX_SWORD_BEAMS; i++) {
        SwordBeam* b = &s_sanc.beams[i];
        if (b->active) {
            b->x += b->vx;
            b->y += b->vy;
            b->life--;
            if (b->life <= 0) {
                b->active = false;
                continue;
            }

            if (map && map_is_solid(map, b->x, b->y)) {
                b->active = false;
                hal_audio_play_sound(SOUND_SWORD_HIT, 0.8f, 1.3f);
                continue;
            }

            if (entity_check_sword_hit(b->x - 6.0f, b->y - 6.0f, 12.0f, 12.0f, b->dmg, b->dir)) {
                b->active = false;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.2f);
                continue;
            }
        }
    }
}

void sanctuary_render_sword_beams(const Camera* cam) {
    if (!cam) return;
    int ox = -(int)cam->x;
    int oy = -(int)cam->y;

    for (int i = 0; i < MAX_SWORD_BEAMS; i++) {
        const SwordBeam* b = &s_sanc.beams[i];
        if (b->active) {
            int bx = ox + (int)b->x;
            int by = oy + (int)b->y;

            if (b->dir == DIR_UP || b->dir == DIR_DOWN) {
                draw_rect_blend(bx - 8, by - 3, 16, 6, 0xFDE04766);
                draw_filled_rect(bx - 6, by - 2, 12, 4, 0x38BDF8FF);
                draw_filled_rect(bx - 3, by - 1, 6, 2, 0xFFFFFFFF);
            } else {
                draw_rect_blend(bx - 3, by - 8, 6, 16, 0xFDE04766);
                draw_filled_rect(bx - 2, by - 6, 4, 12, 0x38BDF8FF);
                draw_filled_rect(bx - 1, by - 3, 2, 6, 0xFFFFFFFF);
            }
        }
    }
}

bool sanctuary_check_sword_beam_hit(float target_x, float target_y, float target_w, float target_h, int* out_dmg) {
    for (int i = 0; i < MAX_SWORD_BEAMS; i++) {
        SwordBeam* b = &s_sanc.beams[i];
        if (b->active) {
            if (b->x >= target_x && b->x <= target_x + target_w &&
                b->y >= target_y && b->y <= target_y + target_h) {
                b->active = false;
                if (out_dmg) *out_dmg = b->dmg;
                hal_audio_play_sound(SOUND_BOSS_HIT, 1.0f, 1.2f);
                return true;
            }
        }
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
            entity_trigger_screen_shake(22, 5);
            hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 1.2f);
        }
        if (s_sanc.cutscene_timer <= 0) {
            s_sanc.cutscene_playing = false;
            if (s_sanc.cutscene_is_four) {
                s_sanc.pedestal_infused_four = true;
                s_sanc.pedestal_infused_three = true;
                s_sanc.pedestal_infused = true;
                s_sanc.secret_exit_unlocked = true;
                inventory_set_four_sword(true);
                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.4f);
                hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.2f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.6f);
                printf("[SANCTUARY] A FOUR SWORD COMPLETA FOI FORJADA! 4 Clones, Sword Beams e Passagem Norte desbloqueados!\n");
            } else if (s_sanc.cutscene_is_three) {
                s_sanc.pedestal_infused_three = true;
                s_sanc.pedestal_infused = true;
                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.2f);
                hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.1f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
            } else {
                s_sanc.pedestal_infused = true;
                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.0f);
                hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.0f);
            }
        }
        return;
    }

    // 2. Saída sul para o North Hyrule Field
    if (ly >= 10.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
        sanctuary_exit(link_x, link_y, link_dir);
        return;
    }

    // 2b. Passagem secreta norte destravada (Leva ao Royal Valley no Ato V)
    if (s_sanc.secret_exit_unlocked && ly <= 1.0f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
        if (link_y) *link_y = 1.8f * TILE_SIZE;
        if (link_dir) *link_dir = DIR_DOWN;
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
        printf("[SANCTUARY] Passagem Secreta Norte alcancada! Acesso ao Royal Valley liberado para o Ato V!\n");
    }

    // 3. Mecânica Canônica de Divisão de Clones (Split Pads)
    // Se possui 4 Elementos: usa todos os 4 blocos em diamante (pads[0..3])
    if (s_sanc.pedestal_infused_four) {
        if (is_charge_ready) {
            for (int p = 0; p < 4; p++) {
                float dp = sqrtf((lx - s_sanc.pads[p].x) * (lx - s_sanc.pads[p].x) + (ly - s_sanc.pads[p].y) * (ly - s_sanc.pads[p].y));
                if (dp < 14.0f && !s_sanc.pad_charged[p]) {
                    s_sanc.pad_charged[p] = true;
                    s_sanc.pads[p].is_active = true;
                    hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.15f + p * 0.15f);
                    printf("[FOUR SWORD] Pad %d/4 energizado!\n", p + 1);
                }
            }

            // Se os 4 blocos forem tocados: DIVISÃO EM 4 HERÓIS!
            if (s_sanc.pad_charged[0] && s_sanc.pad_charged[1] && s_sanc.pad_charged[2] && s_sanc.pad_charged[3] &&
                !s_sanc.clones[0].active) {

                // Clone 0: Red Link (Topo/Norte)
                s_sanc.clones[0].active = true;
                s_sanc.clones[0].offset_x = s_sanc.pads[0].x - s_sanc.pads[3].x;
                s_sanc.clones[0].offset_y = s_sanc.pads[0].y - s_sanc.pads[3].y;
                s_sanc.clones[0].x = lx + s_sanc.clones[0].offset_x;
                s_sanc.clones[0].y = ly + s_sanc.clones[0].offset_y;
                s_sanc.clones[0].dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clones[0].lifetime = MAX_CLONE_LIFETIME;

                // Clone 1: Blue Link (Esquerda/Oeste)
                s_sanc.clones[1].active = true;
                s_sanc.clones[1].offset_x = s_sanc.pads[1].x - s_sanc.pads[3].x;
                s_sanc.clones[1].offset_y = s_sanc.pads[1].y - s_sanc.pads[3].y;
                s_sanc.clones[1].x = lx + s_sanc.clones[1].offset_x;
                s_sanc.clones[1].y = ly + s_sanc.clones[1].offset_y;
                s_sanc.clones[1].dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clones[1].lifetime = MAX_CLONE_LIFETIME;

                // Clone 2: Purple Link (Direita/Leste)
                s_sanc.clones[2].active = true;
                s_sanc.clones[2].offset_x = s_sanc.pads[2].x - s_sanc.pads[3].x;
                s_sanc.clones[2].offset_y = s_sanc.pads[2].y - s_sanc.pads[3].y;
                s_sanc.clones[2].x = lx + s_sanc.clones[2].offset_x;
                s_sanc.clones[2].y = ly + s_sanc.clones[2].offset_y;
                s_sanc.clones[2].dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clones[2].lifetime = MAX_CLONE_LIFETIME;

                for (int p = 0; p < 4; p++) {
                    s_sanc.pad_charged[p] = false;
                    s_sanc.pads[p].is_active = false;
                }

                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.5f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                entity_trigger_screen_shake(16, 4);
                printf("[FOUR SWORD] DIVISAO COMPLETA EM 4 HERÓIS! Green, Red, Blue e Purple Link em acao!\n");
            }
        } else if (!is_charging) {
            for (int p = 0; p < 4; p++) {
                s_sanc.pad_charged[p] = false;
                s_sanc.pads[p].is_active = false;
            }
        }
    }
    // Caso de 3 Elementos
    else if (s_sanc.pedestal_infused_three) {
        if (is_charge_ready) {
            for (int p = 0; p < 3; p++) {
                float dp = sqrtf((lx - s_sanc.pads[p].x) * (lx - s_sanc.pads[p].x) + (ly - s_sanc.pads[p].y) * (ly - s_sanc.pads[p].y));
                if (dp < 14.0f && !s_sanc.pad_charged[p]) {
                    s_sanc.pad_charged[p] = true;
                    s_sanc.pads[p].is_active = true;
                    hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.3f + p * 0.15f);
                }
            }
            if (s_sanc.pad_charged[0] && s_sanc.pad_charged[1] && s_sanc.pad_charged[2] &&
                !s_sanc.clones[0].active && !s_sanc.clones[1].active) {
                s_sanc.clones[0].active = true;
                s_sanc.clones[0].offset_x = s_sanc.pads[1].x - s_sanc.pads[0].x;
                s_sanc.clones[0].offset_y = 0.0f;
                s_sanc.clones[0].x = lx + s_sanc.clones[0].offset_x;
                s_sanc.clones[0].y = ly + s_sanc.clones[0].offset_y;
                s_sanc.clones[0].dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clones[0].lifetime = MAX_CLONE_LIFETIME;

                s_sanc.clones[1].active = true;
                s_sanc.clones[1].offset_x = s_sanc.pads[2].x - s_sanc.pads[0].x;
                s_sanc.clones[1].offset_y = 0.0f;
                s_sanc.clones[1].x = lx + s_sanc.clones[1].offset_x;
                s_sanc.clones[1].y = ly + s_sanc.clones[1].offset_y;
                s_sanc.clones[1].dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clones[1].lifetime = MAX_CLONE_LIFETIME;

                for (int p = 0; p < 3; p++) {
                    s_sanc.pad_charged[p] = false;
                    s_sanc.pads[p].is_active = false;
                }
                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.4f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
            }
        } else if (!is_charging) {
            for (int p = 0; p < 3; p++) {
                s_sanc.pad_charged[p] = false;
                s_sanc.pads[p].is_active = false;
            }
        }
    }
    // Caso de 2 Elementos
    else if (s_sanc.pedestal_infused) {
        float d1 = sqrtf((lx - s_sanc.pads[1].x) * (lx - s_sanc.pads[1].x) + (ly - s_sanc.pads[1].y) * (ly - s_sanc.pads[1].y));
        float d2 = sqrtf((lx - s_sanc.pads[2].x) * (lx - s_sanc.pads[2].x) + (ly - s_sanc.pads[2].y) * (ly - s_sanc.pads[2].y));

        if (is_charge_ready) {
            if (d1 < 14.0f && !s_sanc.pad_charged[1]) {
                s_sanc.pad_charged[1] = true;
                s_sanc.pads[1].is_active = true;
                hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.5f);
            }
            if (d2 < 14.0f && !s_sanc.pad_charged[2]) {
                s_sanc.pad_charged[2] = true;
                s_sanc.pads[2].is_active = true;
                hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.6f);
            }
            if (s_sanc.pad_charged[1] && s_sanc.pad_charged[2] && !s_sanc.clones[0].active) {
                s_sanc.clones[0].active = true;
                s_sanc.clones[0].offset_x = s_sanc.pads[2].x - s_sanc.pads[1].x;
                s_sanc.clones[0].offset_y = 0.0f;
                s_sanc.clones[0].x = lx + s_sanc.clones[0].offset_x;
                s_sanc.clones[0].y = ly + s_sanc.clones[0].offset_y;
                s_sanc.clones[0].dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clones[0].lifetime = MAX_CLONE_LIFETIME;

                s_sanc.pad_charged[1] = false; s_sanc.pads[1].is_active = false;
                s_sanc.pad_charged[2] = false; s_sanc.pads[2].is_active = false;
                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.3f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
            }
        } else if (!is_charging) {
            s_sanc.pad_charged[1] = false; s_sanc.pads[1].is_active = false;
            s_sanc.pad_charged[2] = false; s_sanc.pads[2].is_active = false;
        }
    }

    // 4. Atualização da Movimentação Sincronizada dos Clones
    for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
        CloneState* cl = &s_sanc.clones[c];
        if (!cl->active) continue;

        cl->lifetime--;
        if (cl->lifetime <= 0) {
            cl->active = false;
            hal_audio_play_sound(SOUND_CHUCHU_SQUISH, 0.7f, 1.5f);
            continue;
        }

        // Replica a posição com offset rígido
        cl->x = lx + cl->offset_x;
        cl->y = ly + cl->offset_y;
        cl->dir = (link_dir ? *link_dir : DIR_UP);
        cl->is_moving = link_moving;

        // Replica animações de caminhada e ataque
        if (link_moving) {
            cl->anim_timer++;
            if (cl->anim_timer >= 6) {
                cl->anim_frame = (cl->anim_frame + 1) % 4;
                cl->anim_timer = 0;
            }
        } else {
            cl->anim_frame = 0;
            cl->anim_timer = 0;
        }

        cl->is_attacking = is_attacking;
        cl->attack_timer = attack_timer;
    }

    // 5. Atualização dos Interruptores de Piso (4 interruptores)
    float sw_coords[4] = { 4.5f, 6.5f, 8.5f, 10.5f };
    float sw_y = 8.5f * TILE_SIZE;

    for (int s = 0; s < 4; s++) {
        float sx = sw_coords[s] * TILE_SIZE;
        bool pressed = false;

        // Link original
        if (fabsf(lx - sx) <= 12.0f && fabsf(ly - sw_y) <= 12.0f) {
            pressed = true;
        }
        // Clones
        for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
            if (s_sanc.clones[c].active) {
                if (fabsf(s_sanc.clones[c].x - sx) <= 12.0f && fabsf(s_sanc.clones[c].y - sw_y) <= 12.0f) {
                    pressed = true;
                }
            }
        }
        s_sanc.switch_down[s] = pressed;
    }

    // Se 2 interruptores pressionados: abre portão 1
    if ((s_sanc.switch_down[1] && s_sanc.switch_down[2]) && !s_sanc.gate_open) {
        s_sanc.gate_open = true;
        hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.2f);
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
    }

    // Se 3 interruptores pressionados: destrava o portão do tesouro
    if (s_sanc.switch_down[0] && s_sanc.switch_down[1] && s_sanc.switch_down[2] && !s_sanc.treasury_gate_open) {
        s_sanc.treasury_gate_open = true;
        hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.0f);
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
    }

    // Se TODOS os 4 interruptores pressionados: destrava a passagem secreta norte!
    if (s_sanc.switch_down[0] && s_sanc.switch_down[1] && s_sanc.switch_down[2] && s_sanc.switch_down[3] && !s_sanc.secret_exit_unlocked) {
        s_sanc.secret_exit_unlocked = true;
        hal_audio_play_sound(SOUND_DOOR_UNLOCK, 1.0f, 1.2f);
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.6f);
        entity_trigger_screen_shake(20, 4);
        printf("[SANCTUARY] TODOS OS 4 INTERRUPTORES ACIONADOS! Passagem secreta norte totalmente destravada!\n");
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

    // Parede Norte (Linhas 0 e 1) - Se passagem secreta destravada, col 7 e 8 ficam livres!
    if (row <= 1) {
        if (s_sanc.secret_exit_unlocked && (col == 7 || col == 8)) {
            return false;
        }
        return true;
    }

    // Parede Sul (Linha 11) - Abertura em x=7 e x=8
    if (row >= 11) {
        if (col == 7 || col == 8) return false;
        return true;
    }

    // Paredes Laterais Oeste e Leste
    if (col <= 1 || col >= 14) return true;

    // Altar Pedestal Central (col 7 e 8, row 2 e 3)
    if ((col == 7 || col == 8) && (row == 2 || row == 3)) return true;

    // Pedestais das Esferas Elementais (col 2, 4, 10, 12, row 2)
    if ((col == 2 || col == 4 || col == 10 || col == 12) && row == 2) return true;

    // Cancela / Portão Sagrado 1 (Linha 4, entre colunas 5 a 10)
    if (row == 4 && (col >= 5 && col <= 10)) {
        if (!s_sanc.gate_open) return true;
    }

    // Bloco Colossal (Se não empurrado, bloqueia em x=7..8, y=4)
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

    // 1. Piso de Mármore Polido
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

    // 2. Vitrais Sagrados nas Paredes Laterais
    for (int v = 0; v < 3; v++) {
        int vy = oy + (3 + v * 2) * TILE_SIZE;
        u32 beam_c = (v == 0) ? 0x06B6D422 : ((v == 1) ? 0x22C55E22 : 0xEF444422);
        draw_rect_blend(ox + 2 * TILE_SIZE, vy, 40, 24, beam_c);
        draw_rect_blend(ox + 11 * TILE_SIZE, vy, 40, 24, beam_c);
    }

    // 3. Pisos de Clones (Split Pads)
    int max_p = s_sanc.pedestal_infused_four ? 4 : (s_sanc.pedestal_infused_three ? 3 : (s_sanc.pedestal_infused ? 3 : 0));
    for (int p = 0; p < max_p; p++) {
        if (!s_sanc.pedestal_infused_three && !s_sanc.pedestal_infused_four && p != 1 && p != 2) continue;

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

    // 4. Interruptores de Piso (4 interruptores)
    float sw_coords[4] = { 4.5f, 6.5f, 8.5f, 10.5f };
    for (int s = 0; s < 4; s++) {
        int sw_x = ox + (int)(sw_coords[s] * TILE_SIZE);
        int sw_y = oy + (int)(8.5f * TILE_SIZE);

        draw_filled_rect(sw_x + 2, sw_y + 2, 12, 12, C_SANC_SWITCH_BASE);
        draw_filled_rect(sw_x + 4, sw_y + 4, 8, 8, s_sanc.switch_down[s] ? 0x22C55EFF : C_SANC_SWITCH_PLATE);
    }

    // 5. Cancela / Portão Sagrado (Linha 4)
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

    // 5b. Bloco Colossal de Empurrão
    int bx = ox + (int)s_sanc.heavy_block_x;
    int by = oy + (int)s_sanc.heavy_block_y;
    draw_filled_rect(bx, by, 32, 16, C_SANC_HEAVY_BLOCK);
    draw_filled_rect(bx + 2, by + 2, 28, 12, 0x64748BFF);
    draw_filled_rect(bx + 14, by + 4, 4, 8, 0xF59E0BFF);
    draw_filled_rect(bx + 10, by + 6, 12, 4, 0xF59E0BFF);

    // 6. Altar Pedestal da Four Sword & As 4 Esferas Elementais
    int ped_x = ox + (int)s_sanc.pedestal_x;
    int ped_y = oy + (int)s_sanc.pedestal_y;

    draw_filled_rect(ped_x - 12, ped_y - 8, 24, 18, C_SANC_PEDESTAL_BASE);
    draw_filled_rect(ped_x - 10, ped_y - 6, 20, 14, C_SANC_PEDESTAL_TOP);
    draw_filled_rect(ped_x - 12, ped_y + 8, 24, 3, C_SANC_GOLD_TRIM);

    // Espada cravada no pedestal (White Sword ou FOUR SWORD COMPLETA)
    if (s_sanc.pedestal_infused_four) {
        // Lâmina dourada radiante com arco-íris
        draw_filled_rect(ped_x - 1, ped_y - 20, 2, 16, 0xFFFFFFFF);
        draw_filled_rect(ped_x - 5, ped_y - 6, 10, 3, 0xFACC15FF); // Guarda dourada
        // 4 Gemas na guarda da Four Sword
        hal_video_put_pixel(ped_x - 4, ped_y - 5, C_SANC_EARTH_ORB);
        hal_video_put_pixel(ped_x - 2, ped_y - 5, C_SANC_FIRE_ORB);
        hal_video_put_pixel(ped_x + 1, ped_y - 5, C_SANC_WATER_ORB);
        hal_video_put_pixel(ped_x + 3, ped_y - 5, C_SANC_WIND_ORB);
        draw_rect_blend(ped_x - 12, ped_y - 24, 24, 28, 0xFDE04744);
    } else {
        draw_filled_rect(ped_x - 1, ped_y - 18, 2, 14, 0xFFFFFFFF);
        draw_filled_rect(ped_x - 4, ped_y - 6, 8, 2, 0xFACC15FF);
        draw_filled_rect(ped_x - 2, ped_y - 4, 4, 3, 0x78350FFF);
    }

    // 1. Esfera do Elemento Água (Ciano Safira)
    int wx = ox + (int)s_sanc.water_orb_x;
    int wy = oy + (int)s_sanc.water_orb_y;
    draw_filled_rect(wx - 8, wy, 16, 12, C_SANC_PEDESTAL_BASE);
    draw_filled_rect(wx - 6, wy - 10, 12, 12, C_SANC_WATER_ORB);
    draw_filled_rect(wx - 3, wy - 7, 6, 6, 0xE0F2FEFF);
    draw_rect_blend(wx - 10, wy - 14, 20, 20, 0x06B6D433);

    // 2. Esfera do Elemento Terra (Verde Esmeralda)
    int ex = ox + (int)s_sanc.earth_orb_x;
    int ey = oy + (int)s_sanc.earth_orb_y;
    draw_filled_rect(ex - 8, ey, 16, 12, C_SANC_PEDESTAL_BASE);
    draw_filled_rect(ex - 6, ey - 10, 12, 12, C_SANC_EARTH_ORB);
    draw_filled_rect(ex - 3, ey - 7, 6, 6, 0x86EFACFF);
    draw_rect_blend(ex - 10, ey - 14, 20, 20, 0x22C55E33);

    // 3. Esfera do Elemento Fogo (Vermelho Rubi)
    int fx = ox + (int)s_sanc.fire_orb_x;
    int fy = oy + (int)s_sanc.fire_orb_y;
    draw_filled_rect(fx - 8, fy, 16, 12, C_SANC_PEDESTAL_BASE);
    draw_filled_rect(fx - 6, fy - 10, 12, 12, C_SANC_FIRE_ORB);
    draw_filled_rect(fx - 3, fy - 7, 6, 6, 0xFDE047FF);
    draw_rect_blend(fx - 10, fy - 14, 20, 20, 0xEF444433);

    // 4. Esfera do Elemento do Vento (Esmeralda / Ouro Celeste)
    int nx = ox + (int)s_sanc.wind_orb_x;
    int ny = oy + (int)s_sanc.wind_orb_y;
    draw_filled_rect(nx - 8, ny, 16, 12, C_SANC_PEDESTAL_BASE);
    draw_filled_rect(nx - 6, ny - 10, 12, 12, C_SANC_WIND_ORB);
    draw_filled_rect(nx - 3, ny - 7, 6, 6, 0xFDE047FF);
    draw_rect_blend(nx - 10, ny - 14, 20, 20, 0x10B98144);

    // Cutscene de Infusão (2, 3 ou 4 Elementos orbitando)
    if (s_sanc.cutscene_playing) {
        float angle = (float)s_sanc_anim * 0.15f;
        if (s_sanc.cutscene_is_four) {
            // 4 Orbes orbitando a 90 graus (PI / 2)
            int o1_x = ped_x + (int)(cosf(angle) * 26.0f);
            int o1_y = ped_y + (int)(sinf(angle) * 18.0f) - 10;
            int o2_x = ped_x + (int)(cosf(angle + 1.571f) * 26.0f);
            int o2_y = ped_y + (int)(sinf(angle + 1.571f) * 18.0f) - 10;
            int o3_x = ped_x + (int)(cosf(angle + 3.142f) * 26.0f);
            int o3_y = ped_y + (int)(sinf(angle + 3.142f) * 18.0f) - 10;
            int o4_x = ped_x + (int)(cosf(angle + 4.712f) * 26.0f);
            int o4_y = ped_y + (int)(sinf(angle + 4.712f) * 18.0f) - 10;

            draw_filled_rect(o1_x - 4, o1_y - 4, 8, 8, C_SANC_EARTH_ORB);
            draw_filled_rect(o2_x - 4, o2_y - 4, 8, 8, C_SANC_FIRE_ORB);
            draw_filled_rect(o3_x - 4, o3_y - 4, 8, 8, C_SANC_WATER_ORB);
            draw_filled_rect(o4_x - 4, o4_y - 4, 8, 8, C_SANC_WIND_ORB);
            draw_rect_blend(ped_x - 24, ped_y - 36, 48, 52, 0xFDE04788);
            draw_rect_blend(ped_x - 18, ped_y - 30, 36, 40, 0xFFFFFF99);
        } else if (s_sanc.cutscene_is_three) {
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

    // 7. Paredes e Passagem Secreta Norte
    for (int c = 0; c < SANCTUARY_ROOM_W; c++) {
        // Se passagem secreta aberta, colunas 7 e 8 no norte ficam abertas
        if (s_sanc.secret_exit_unlocked && (c == 7 || c == 8)) {
            draw_filled_rect(ox + c * TILE_SIZE, oy, TILE_SIZE, TILE_SIZE, 0x0F172AFF);
            draw_rect_blend(ox + c * TILE_SIZE, oy, TILE_SIZE, TILE_SIZE, 0xFDE04766);
            continue;
        }
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

    // 9. Renderiza os Raios de Espada (Sword Beams)
    sanctuary_render_sword_beams(cam);
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

        // Olhos
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

bool sanctuary_has_stepped_in_secret_exit(float link_x, float link_y) {
    if (!s_sanc.active || !s_sanc.secret_exit_unlocked) return false;
    return (link_y <= 1.2f * TILE_SIZE && link_x >= 6.5f * TILE_SIZE && link_x <= 9.5f * TILE_SIZE);
}

