/*
 * ============================================================================
 * src/hal/sanctuary.c - Implementação do Santuário Elemental & 2 Clones
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
#define C_SANC_EARTH_ORB     0x22C55EFF // Esfera mística do Elemento Terra
#define C_SANC_FIRE_ORB      0xEF4444FF // Esfera mística do Elemento Fogo
#define C_SANC_PAD_IDLE      0x0284C7FF // Piso de divisão desativado
#define C_SANC_PAD_ACTIVE    0x38BDF8FF // Piso de divisão energizado
#define C_SANC_SWITCH_BASE   0x334155FF // Base do interruptor duplo
#define C_SANC_SWITCH_PLATE  0xD97706FF // Prato metálico do interruptor
#define C_SANC_GATE_BARS     0x64748BFF // Grades da cancela sagrada

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
    s_sanc.cutscene_playing = false;
    s_sanc.cutscene_timer = 0;

    // Altar da Four Sword (Norte central)
    s_sanc.pedestal_x = 7.5f * TILE_SIZE;
    s_sanc.pedestal_y = 2.5f * TILE_SIZE;
    s_sanc.earth_orb_x = 5.5f * TILE_SIZE;
    s_sanc.earth_orb_y = 2.5f * TILE_SIZE;
    s_sanc.fire_orb_x  = 9.5f * TILE_SIZE;
    s_sanc.fire_orb_y  = 2.5f * TILE_SIZE;

    // Par de Pisos de Clones (Spaced horizontally by 3 tiles = 48 px)
    s_sanc.pads[0].x = 6.0f * TILE_SIZE;
    s_sanc.pads[0].y = 6.0f * TILE_SIZE;
    s_sanc.pads[0].is_active = false;

    s_sanc.pads[1].x = 9.0f * TILE_SIZE;
    s_sanc.pads[1].y = 6.0f * TILE_SIZE;
    s_sanc.pads[1].is_active = false;

    s_sanc.pad1_charged = false;
    s_sanc.pad2_charged = false;

    // Interruptores duplos no solo (x=6 e x=9, y=8.5)
    s_sanc.switch_left_down = false;
    s_sanc.switch_right_down = false;
    s_sanc.gate_open = false;

    // Clone inicial inativo
    s_sanc.clone.active = false;

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
    s_sanc.clone.active = false;

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
    s_sanc.clone.active = false;

    // Posição de retorno no North Hyrule Field (em frente ao portal norte)
    if (link_x) *link_x = 248.0f;
    if (link_y) *link_y = 36.0f;
    if (link_dir) *link_dir = DIR_DOWN;

    hal_audio_play_sound(SOUND_SECRET, 0.85f, 1.2f);
    printf("[SANCTUARY] Link saiu do Santuario Elemental de volta aos campos de Hyrule!\n");
}

bool sanctuary_has_two_elements(void) {
    return s_sanc.pedestal_infused;
}

void sanctuary_set_two_elements(bool infused) {
    s_sanc.pedestal_infused = infused;
}

bool sanctuary_is_clone_active(void) {
    return s_sanc.clone.active;
}

CloneState* sanctuary_get_clone(void) {
    return &s_sanc.clone;
}

bool sanctuary_get_clone_sword_hitbox(float* out_x, float* out_w, float* out_y, float* out_h, int* out_dmg) {
    if (!s_sanc.clone.active || !s_sanc.clone.is_attacking) return false;

    float hx = s_sanc.clone.x + 4.0f;
    float hy = s_sanc.clone.y + 4.0f;
    float hw = 12.0f;
    float hh = 12.0f;

    if (s_sanc.clone.dir == DIR_DOWN)  { hx = s_sanc.clone.x + 1.0f;  hy = s_sanc.clone.y + 14.0f; hw = 14.0f; hh = 12.0f; }
    if (s_sanc.clone.dir == DIR_UP)    { hx = s_sanc.clone.x + 1.0f;  hy = s_sanc.clone.y - 10.0f; hw = 14.0f; hh = 12.0f; }
    if (s_sanc.clone.dir == DIR_LEFT)  { hx = s_sanc.clone.x - 12.0f; hy = s_sanc.clone.y + 2.0f;  hw = 12.0f; hh = 14.0f; }
    if (s_sanc.clone.dir == DIR_RIGHT) { hx = s_sanc.clone.x + 14.0f; hy = s_sanc.clone.y + 2.0f;  hw = 12.0f; hh = 14.0f; }

    if (out_x) *out_x = hx;
    if (out_y) *out_y = hy;
    if (out_w) *out_w = hw;
    if (out_h) *out_h = hh;
    if (out_dmg) *out_dmg = 2; // Espada Branca

    return true;
}

bool sanctuary_interact(float link_x, float link_y, bool has_white_sword, bool has_earth_element, bool has_fire_element) {
    if (!s_sanc.active) return false;

    // Distância do Altar Pedestal
    float dx = link_x - s_sanc.pedestal_x;
    float dy = link_y - s_sanc.pedestal_y;
    if (dx * dx + dy * dy <= 28.0f * 28.0f) {
        if (!s_sanc.pedestal_infused) {
            if (has_white_sword && has_earth_element && has_fire_element) {
                // Inicia Cutscene Sagrada de Infusão dos 2 Elementos!
                s_sanc.cutscene_playing = true;
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

void sanctuary_update(float* link_x, float* link_y, Direction* link_dir,
                      bool link_moving, bool is_charging, bool is_charge_ready,
                      bool is_attacking, int attack_timer,
                      int* link_hearts) {
    (void)link_hearts;
    if (!s_sanc.active) return;

    s_sanc_anim++;

    float lx = (link_x ? *link_x : 0.0f);
    float ly = (link_y ? *link_y : 0.0f);

    // 1. Cutscene de Infusão dos 2 Elementos
    if (s_sanc.cutscene_playing) {
        s_sanc.cutscene_timer--;
        if (s_sanc.cutscene_timer == 120) {
            entity_trigger_screen_shake(20, 4);
            hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 1.2f);
        }
        if (s_sanc.cutscene_timer <= 0) {
            s_sanc.cutscene_playing = false;
            s_sanc.pedestal_infused = true;
            hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.0f);
            hal_audio_play_sound(SOUND_HEART_CONTAINER, 1.0f, 1.0f);
            printf("[SANCTUARY] INFUSAO CONCLUIDA! White Sword (Two Elements) forjada com sucesso!\n");
        }
        return; // Pausa movimentação durante a cutscene
    }

    // 2. Saída sul para o North Hyrule Field
    if (ly >= 10.8f * TILE_SIZE && lx >= 6.5f * TILE_SIZE && lx <= 9.5f * TILE_SIZE) {
        sanctuary_exit(link_x, link_y, link_dir);
        return;
    }

    // 3. Mecânica Canônica de Divisão de Clones (Split Pads)
    if (s_sanc.pedestal_infused) {
        // Distância até o Bloco 1 (x=6.0, y=6.0)
        float d1 = sqrtf((lx - s_sanc.pads[0].x) * (lx - s_sanc.pads[0].x) + (ly - s_sanc.pads[0].y) * (ly - s_sanc.pads[0].y));
        // Distância até o Bloco 2 (x=9.0, y=6.0)
        float d2 = sqrtf((lx - s_sanc.pads[1].x) * (lx - s_sanc.pads[1].x) + (ly - s_sanc.pads[1].y) * (ly - s_sanc.pads[1].y));

        if (is_charge_ready) {
            if (d1 < 12.0f && !s_sanc.pad1_charged) {
                s_sanc.pad1_charged = true;
                s_sanc.pads[0].is_active = true;
                hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.5f);
                printf("[FOUR SWORD] Pad 1 ativado com carga elemental!\n");
            }
            if (d2 < 12.0f && !s_sanc.pad2_charged) {
                s_sanc.pad2_charged = true;
                s_sanc.pads[1].is_active = true;
                hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.5f);
                printf("[FOUR SWORD] Pad 2 ativado com carga elemental!\n");
            }

            // Se ambos os blocos forem tocados com carga completa: DIVISÃO EM 2 CLONES!
            if (s_sanc.pad1_charged && s_sanc.pad2_charged && !s_sanc.clone.active) {
                s_sanc.clone.active = true;
                s_sanc.clone.offset_x = (d1 < d2) ? (s_sanc.pads[1].x - s_sanc.pads[0].x) : (s_sanc.pads[0].x - s_sanc.pads[1].x);
                s_sanc.clone.offset_y = 0.0f;
                s_sanc.clone.x = lx + s_sanc.clone.offset_x;
                s_sanc.clone.y = ly + s_sanc.clone.offset_y;
                s_sanc.clone.dir = (link_dir ? *link_dir : DIR_UP);
                s_sanc.clone.lifetime = MAX_CLONE_LIFETIME;
                s_sanc.clone.is_moving = false;
                s_sanc.clone.is_attacking = false;

                s_sanc.pad1_charged = false;
                s_sanc.pad2_charged = false;
                s_sanc.pads[0].is_active = false;
                s_sanc.pads[1].is_active = false;

                hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.4f);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                entity_trigger_screen_shake(10, 2);
                printf("[FOUR SWORD] DIVISAO CONCLUIDA! 2 Clones simultaneos ativos!\n");
            }
        } else if (!is_charging) {
            // Se Link soltar o botão sem completar os dois pads, reseta a ativação
            s_sanc.pad1_charged = false;
            s_sanc.pad2_charged = false;
            s_sanc.pads[0].is_active = false;
            s_sanc.pads[1].is_active = false;
        }

        // 4. Atualização e Sincronização do Clone
        if (s_sanc.clone.active) {
            s_sanc.clone.lifetime--;

            // Sincroniza posição e estado com Link
            s_sanc.clone.x = lx + s_sanc.clone.offset_x;
            s_sanc.clone.y = ly + s_sanc.clone.offset_y;
            s_sanc.clone.dir = (link_dir ? *link_dir : DIR_UP);
            s_sanc.clone.is_moving = link_moving;
            s_sanc.clone.is_attacking = is_attacking;
            s_sanc.clone.attack_timer = attack_timer;
            if (link_moving) {
                s_sanc.clone.anim_timer++;
                if (s_sanc.clone.anim_timer % 12 == 0) {
                    s_sanc.clone.anim_frame = (s_sanc.clone.anim_frame == 0) ? 1 : 0;
                }
            }

            // Dissipação do clone se colidir com parede sólida
            if (sanctuary_is_solid(s_sanc.clone.x + 8.0f, s_sanc.clone.y + 12.0f)) {
                s_sanc.clone.active = false;
                hal_audio_play_sound(SOUND_GUST_BLAST, 0.9f, 1.2f);
                printf("[FOUR SWORD] Clone colidiu com barreira solida e se dissipou em fumaca!\n");
            }

            if (s_sanc.clone.lifetime <= 0) {
                s_sanc.clone.active = false;
                hal_audio_play_sound(SOUND_GUST_BLAST, 0.9f, 1.2f);
                printf("[FOUR SWORD] Tempo de clone expirado.\n");
            }
        }
    }

    // 5. Quebra-cabeça de Interruptores Duplos (x=6 e x=9, y=8.0)
    float sw1_x = 6.0f * TILE_SIZE + 8.0f;
    float sw1_y = 8.0f * TILE_SIZE + 8.0f;
    float sw2_x = 9.0f * TILE_SIZE + 8.0f;
    float sw2_y = 8.0f * TILE_SIZE + 8.0f;

    bool p1_down = false;
    bool p2_down = false;

    // Verifica se Link ou Clone pisam no interruptor 1
    if (fabsf(lx + 8.0f - sw1_x) < 10.0f && fabsf(ly + 12.0f - sw1_y) < 10.0f) p1_down = true;
    if (s_sanc.clone.active && fabsf(s_sanc.clone.x + 8.0f - sw1_x) < 10.0f && fabsf(s_sanc.clone.y + 12.0f - sw1_y) < 10.0f) p1_down = true;

    // Verifica se Link ou Clone pisam no interruptor 2
    if (fabsf(lx + 8.0f - sw2_x) < 10.0f && fabsf(ly + 12.0f - sw2_y) < 10.0f) p2_down = true;
    if (s_sanc.clone.active && fabsf(s_sanc.clone.x + 8.0f - sw2_x) < 10.0f && fabsf(s_sanc.clone.y + 12.0f - sw2_y) < 10.0f) p2_down = true;

    s_sanc.switch_left_down = p1_down;
    s_sanc.switch_right_down = p2_down;

    // Ambos os interruptores pressionados simultaneamente: Destrava o Portão Sagrado!
    if (p1_down && p2_down && !s_sanc.gate_open) {
        s_sanc.gate_open = true;
        hal_audio_play_sound(SOUND_DOOR_SHUTTER, 1.0f, 1.0f);
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
        entity_trigger_screen_shake(12, 3);
        printf("[SANCTUARY] INTERRUPTORES DUPLOS ACIONADOS! Portao sagrado elevado com exito!\n");
    }
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

    // Pedestais das Esferas Elementais
    if ((col == 5 || col == 10) && row == 2) return true;

    // Cancela / Portão Sagrado (Linha 4, entre colunas 6 a 9)
    if (row == 4 && (col >= 6 && col <= 9)) {
        return !s_sanc.gate_open;
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

            // Mármore claro
            u32 c_tile = ((c + r) % 2 == 0) ? C_SANC_FLOOR_LIGHT : C_SANC_FLOOR_DARK;
            draw_filled_rect(tx, ty, TILE_SIZE, TILE_SIZE, c_tile);
            draw_filled_rect(tx, ty, TILE_SIZE, 1, C_SANC_FLOOR_RUNE);
            draw_filled_rect(tx, ty, 1, TILE_SIZE, C_SANC_FLOOR_RUNE);
        }
    }

    // 2. Vitrais Sagrados nas Paredes Laterais (feixes de luz colorida no piso)
    for (int v = 0; v < 3; v++) {
        int vy = oy + (3 + v * 2) * TILE_SIZE;
        // Luz solar filtrada por vitrais (azul, verde, rubi)
        u32 beam_c = (v == 0) ? 0x38BDF822 : ((v == 1) ? 0x22C55E22 : 0xEF444422);
        draw_rect_blend(ox + 2 * TILE_SIZE, vy, 40, 24, beam_c);
        draw_rect_blend(ox + 11 * TILE_SIZE, vy, 40, 24, beam_c);
    }

    // 3. Pisos de Clones (Split Pads)
    for (int p = 0; p < 2; p++) {
        int px = ox + (int)s_sanc.pads[p].x;
        int py = oy + (int)s_sanc.pads[p].y;
        bool lit = s_sanc.pads[p].is_active;

        u32 c_border = lit ? 0xFFFFFFFF : C_SANC_PAD_IDLE;
        u32 c_inner  = lit ? C_SANC_PAD_ACTIVE : 0x0369A1FF;

        draw_filled_rect(px + 1, py + 1, 14, 14, c_border);
        draw_filled_rect(px + 2, py + 2, 12, 12, c_inner);

        // Runas da Four Sword no centro
        int pulse = (s_sanc_anim / 6) % 2;
        draw_filled_rect(px + 6, py + 4 - pulse, 4, 8 + pulse * 2, lit ? 0xFFFFFFFF : 0x38BDF8FF);
        draw_filled_rect(px + 4 - pulse, py + 6, 8 + pulse * 2, 4, lit ? 0xFFFFFFFF : 0x38BDF8FF);

        if (lit) {
            draw_rect_blend(px - 4, py - 4, 24, 24, 0x38BDF855);
        }
    }

    // 4. Interruptores de Piso Duplos (x=6 e x=9, y=8.0)
    int sw1_x = ox + (int)(6.0f * TILE_SIZE);
    int sw1_y = oy + (int)(8.0f * TILE_SIZE);
    int sw2_x = ox + (int)(9.0f * TILE_SIZE);
    int sw2_y = oy + (int)(8.0f * TILE_SIZE);

    // Interruptor 1
    draw_filled_rect(sw1_x + 2, sw1_y + 2, 12, 12, C_SANC_SWITCH_BASE);
    draw_filled_rect(sw1_x + 4, sw1_y + 4, 8, 8, s_sanc.switch_left_down ? 0x22C55EFF : C_SANC_SWITCH_PLATE);

    // Interruptor 2
    draw_filled_rect(sw2_x + 2, sw2_y + 2, 12, 12, C_SANC_SWITCH_BASE);
    draw_filled_rect(sw2_x + 4, sw2_y + 4, 8, 8, s_sanc.switch_right_down ? 0x22C55EFF : C_SANC_SWITCH_PLATE);

    // 5. Cancela / Portão Sagrado (Linha 4, entre colunas 6 e 9)
    int gx = ox + (int)(6.0f * TILE_SIZE);
    int gy = oy + (int)(4.0f * TILE_SIZE);
    if (!s_sanc.gate_open) {
        draw_filled_rect(gx, gy, 64, 16, C_SANC_GATE_BARS);
        for (int b = 0; b < 16; b++) {
            draw_filled_rect(gx + (b * 4), gy, 2, 16, 0x1E293BFF);
        }
        draw_filled_rect(gx + 28, gy + 4, 8, 8, C_SANC_GOLD_TRIM);
    } else {
        // Portão recolhido para o teto
        draw_filled_rect(gx, gy, 64, 4, C_SANC_GATE_BARS);
    }

    // 6. Altar Pedestal da Four Sword & Esferas Elementais
    int ped_x = ox + (int)s_sanc.pedestal_x;
    int ped_y = oy + (int)s_sanc.pedestal_y;

    // Pedestal de Mármore e Ouro
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

    // Cutscene: Feixes de Energia e Chamas Elementais circulando a espada
    if (s_sanc.cutscene_playing) {
        float angle = (float)s_sanc_anim * 0.15f;
        int orb1_x = ped_x + (int)(cosf(angle) * 22.0f);
        int orb1_y = ped_y + (int)(sinf(angle) * 16.0f) - 10;
        int orb2_x = ped_x + (int)(cosf(angle + PI_F) * 22.0f);
        int orb2_y = ped_y + (int)(sinf(angle + PI_F) * 16.0f) - 10;

        draw_filled_rect(orb1_x - 4, orb1_y - 4, 8, 8, C_SANC_EARTH_ORB);
        draw_filled_rect(orb2_x - 4, orb2_y - 4, 8, 8, C_SANC_FIRE_ORB);
        draw_rect_blend(ped_x - 16, ped_y - 28, 32, 40, 0xFFFFFF66);
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

    // 8. Renderiza o Clone da Four Sword
    sanctuary_render_clone(cam);
}

void sanctuary_render_clone(const Camera* cam) {
    if (!s_sanc.clone.active) return;

    int ox = (cam ? -(int)cam->x : 0);
    int oy = (cam ? -(int)cam->y : 0);

    int cx = ox + (int)s_sanc.clone.x;
    int cy = oy + (int)s_sanc.clone.y;

    // Sombra suave sob o clone
    draw_filled_rect(cx + 3, cy + 13, 10, 3, 0x05100766);

    // Aura etérea ciano / azul da Four Sword em volta do Clone
    draw_rect_blend(cx - 2, cy - 2, 20, 20, 0x38BDF844);

    // Corpo de Link (Cores do herói)
    u32 c_cap       = 0x22C55EFF;
    u32 c_face      = 0xFDE8CDFF;
    u32 c_tunic     = 0x16A34AFF;
    u32 c_tights    = 0xFFFFFFFF;
    u32 c_boots     = 0xB45309FF;

    // Gorro e Cabelo
    draw_filled_rect(cx + 4, cy + 1, 8, 6, c_cap);
    draw_filled_rect(cx + 4, cy + 6, 8, 6, c_face);
    draw_filled_rect(cx + 3, cy + 10, 10, 6, c_tunic);
    draw_filled_rect(cx + 4, cy + 15, 8, 3, c_tights);
    draw_filled_rect(cx + 4, cy + 17, 8, 3, c_boots);

    // Olhos e detalhes conforme a direção
    if (s_sanc.clone.dir == DIR_DOWN) {
        draw_filled_rect(cx + 5, cy + 8, 2, 2, 0x1E293BFF);
        draw_filled_rect(cx + 9, cy + 8, 2, 2, 0x1E293BFF);
    } else if (s_sanc.clone.dir == DIR_LEFT) {
        draw_filled_rect(cx + 4, cy + 8, 2, 2, 0x1E293BFF);
    } else if (s_sanc.clone.dir == DIR_RIGHT) {
        draw_filled_rect(cx + 10, cy + 8, 2, 2, 0x1E293BFF);
    }

    // Espada do Clone em ataque
    if (s_sanc.clone.is_attacking) {
        if (s_sanc.clone.dir == DIR_UP) {
            draw_filled_rect(cx + 7, cy - 10, 3, 12, 0xFFFFFFFF);
            draw_filled_rect(cx + 5, cy - 2, 7, 2, 0xFACC15FF);
        } else if (s_sanc.clone.dir == DIR_DOWN) {
            draw_filled_rect(cx + 7, cy + 15, 3, 12, 0xFFFFFFFF);
            draw_filled_rect(cx + 5, cy + 15, 7, 2, 0xFACC15FF);
        } else if (s_sanc.clone.dir == DIR_LEFT) {
            draw_filled_rect(cx - 10, cy + 8, 12, 3, 0xFFFFFFFF);
            draw_filled_rect(cx - 2, cy + 6, 2, 7, 0xFACC15FF);
        } else if (s_sanc.clone.dir == DIR_RIGHT) {
            draw_filled_rect(cx + 14, cy + 8, 12, 3, 0xFFFFFFFF);
            draw_filled_rect(cx + 14, cy + 6, 2, 7, 0xFACC15FF);
        }
    }

    // Partículas místicas de centelhas
    int spk = (s_sanc_anim / 4) % 3;
    draw_filled_rect(cx + 2 + spk * 4, cy - 2 - spk, 2, 2, 0x38BDF8FF);
}
