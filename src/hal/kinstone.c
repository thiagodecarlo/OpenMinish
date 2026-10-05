/*
 * ============================================================================
 * src/hal/kinstone.c - Implementação do Sistema de Fusão de Kinstones
 * ============================================================================
 */

#include "hal/kinstone.h"
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

#define MAX_PARTICLES 24

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    int   life;
    int   max_life;
    u32   color;
} SparkleParticle;

static KinstoneInventory s_inv = { { 2, 1, 1 }, 0 }; // Link começa com 2 verdes, 1 azul, 1 vermelha
static KinstoneUIState   s_ui_state = KINSTONE_UI_INACTIVE;
static Entity*           s_partner_npc = NULL;
static int               s_selected_idx = 0;
static int               s_anim_timer = 0;
static float             s_left_x = 0.0f;
static float             s_right_x = 0.0f;
static int               s_flash_alpha = 0;
static SparkleParticle   s_particles[MAX_PARTICLES] = { 0 };

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

void kinstone_init(void) {
    s_inv.counts[KINSTONE_GREEN] = 2; // 2 fragmentos verdes
    s_inv.counts[KINSTONE_BLUE]  = 1; // 1 fragmento azul
    s_inv.counts[KINSTONE_RED]   = 1; // 1 fragmento vermelho
    s_inv.total_fusions = 0;
    s_ui_state = KINSTONE_UI_INACTIVE;
    s_partner_npc = NULL;
    s_selected_idx = 0;
    s_anim_timer = 0;
    s_flash_alpha = 0;
    memset(s_particles, 0, sizeof(s_particles));
}

KinstoneInventory* kinstone_get_inventory(void) {
    return &s_inv;
}

void kinstone_add(KinstoneType type, int count) {
    if (type >= 0 && type < KINSTONE_COUNT) {
        s_inv.counts[type] += count;
        printf("[KINSTONE] Obtido +%d %s! Total: %d\n", count, kinstone_get_type_name(type), s_inv.counts[type]);
    }
}

bool kinstone_is_active(void) {
    return (s_ui_state != KINSTONE_UI_INACTIVE);
}

const char* kinstone_get_type_name(KinstoneType type) {
    switch (type) {
        case KINSTONE_GREEN: return "Fragmento Verde";
        case KINSTONE_BLUE:  return "Fragmento Azul";
        case KINSTONE_RED:   return "Fragmento Vermelho";
        default:             return "Desconhecido";
    }
}

void kinstone_start_fusion(Entity* npc) {
    if (!npc) return;
    s_partner_npc = npc;
    s_ui_state = KINSTONE_UI_SELECTING;
    s_selected_idx = 0;
    s_anim_timer = 0;
    s_flash_alpha = 0;

    // Toca som de chamada do balão
    hal_audio_play_sound(SOUND_KINSTONE_PROMPT, 0.90f, 1.0f);
    printf("[KINSTONE] Iniciando fusao de Kinstone com NPC (Tipo requerido: %s)\n",
           kinstone_get_type_name((KinstoneType)npc->kinstoneType));
}

static void spawn_fusion_sparkles(float cx, float cy) {
    u32 colors[4] = { 0xFFE27AFF, 0x00FFCCFF, 0xFFFFFFFF, 0x77FF99FF };
    for (int i = 0; i < MAX_PARTICLES; i++) {
        float angle = ((float)i / (float)MAX_PARTICLES) * 2.0f * PI_F + (((float)(rand() % 20)) * 0.05f);
        float speed = 1.2f + ((float)(rand() % 100)) * 0.018f;
        s_particles[i].x = cx;
        s_particles[i].y = cy;
        s_particles[i].vx = cosf(angle) * speed;
        s_particles[i].vy = sinf(angle) * speed;
        s_particles[i].life = 35 + (rand() % 20);
        s_particles[i].max_life = s_particles[i].life;
        s_particles[i].color = colors[i % 4];
    }
}

void kinstone_update(void) {
    if (s_ui_state == KINSTONE_UI_INACTIVE) return;

    s_anim_timer++;

    // Atualiza partículas de brilho
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (s_particles[i].life > 0) {
            s_particles[i].x += s_particles[i].vx;
            s_particles[i].y += s_particles[i].vy;
            s_particles[i].vx *= 0.95f;
            s_particles[i].vy *= 0.95f;
            s_particles[i].life--;
        }
    }

    if (s_flash_alpha > 0) {
        s_flash_alpha -= 8;
        if (s_flash_alpha < 0) s_flash_alpha = 0;
    }

    if (s_ui_state == KINSTONE_UI_FUSING) {
        // As duas metades se aproximam do centro (cx = 120)
        float center_x = 120.0f;
        float progress = (float)s_anim_timer / 32.0f;
        if (progress > 1.0f) progress = 1.0f;

        s_left_x  = 84.0f + (center_x - 12.0f - 84.0f) * progress;
        s_right_x = 156.0f - (156.0f - (center_x + 12.0f)) * progress;

        if (s_anim_timer == 32) {
            // Instante do ENCAIXE PERFEITO!
            s_flash_alpha = 230; // Flash branco dourado na tela
            spawn_fusion_sparkles(center_x, 68.0f);
            hal_audio_play_sound(SOUND_KINSTONE_FUSION, 1.0f, 1.0f);
            s_ui_state = KINSTONE_UI_CELEBRATE;
            s_anim_timer = 0;
        }
    } else if (s_ui_state == KINSTONE_UI_CELEBRATE) {
        // Medalhão completo brilhando e girando por 55 frames
        if (s_anim_timer >= 55) {
            // Conclui e exibe o popup de evento
            if (s_partner_npc) {
                s_partner_npc->kinstoneFused = true;
            }
            if (s_inv.counts[s_selected_idx] > 0) {
                s_inv.counts[s_selected_idx]--;
            }
            s_inv.total_fusions++;

            // Spawna o Baú Dourado na clareira ensolarada do santuário em Minish Woods
            entity_spawn(ENTITY_CHEST_GOLD, 480.0f, 590.0f);
            printf("[KINSTONE] Evento destravado: Bau dourado lendario spawnado em Minish Woods!\n");

            s_ui_state = KINSTONE_UI_EVENT_POPUP;
            s_anim_timer = 0;
        }
    }
}

void kinstone_handle_input(bool confirm_pressed, bool cancel_pressed, int nav_dir) {
    if (s_ui_state == KINSTONE_UI_SELECTING) {
        if (nav_dir < 0) {
            s_selected_idx = (s_selected_idx - 1 + KINSTONE_COUNT) % KINSTONE_COUNT;
            hal_audio_play_sound(SOUND_TEXT_BLIP, 0.6f, 1.2f);
        } else if (nav_dir > 0) {
            s_selected_idx = (s_selected_idx + 1) % KINSTONE_COUNT;
            hal_audio_play_sound(SOUND_TEXT_BLIP, 0.6f, 1.2f);
        }

        if (cancel_pressed) {
            s_ui_state = KINSTONE_UI_INACTIVE;
            s_partner_npc = NULL;
            hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.70f, 0.9f);
        } else if (confirm_pressed) {
            // Checa se o fragmento selecionado combina com o do parceiro e se Link possui pelo menos 1
            if (s_partner_npc && s_selected_idx == s_partner_npc->kinstoneType && s_inv.counts[s_selected_idx] > 0) {
                s_ui_state = KINSTONE_UI_FUSING;
                s_anim_timer = 0;
                s_left_x = 84.0f;
                s_right_x = 156.0f;
                hal_audio_play_sound(SOUND_SECRET, 0.70f, 1.5f);
            } else {
                // Não combina ou não tem peça
                hal_audio_play_sound(SOUND_SWORD_HIT, 0.75f, 0.8f);
            }
        }
    } else if (s_ui_state == KINSTONE_UI_EVENT_POPUP) {
        if (confirm_pressed || cancel_pressed) {
            s_ui_state = KINSTONE_UI_INACTIVE;
            s_partner_npc = NULL;
            hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.80f, 1.0f);
        }
    }
}

// ----------------------------------------------------------------------------
// RENDERIZAÇÃO GRÁFICA DOS FRAGMENTOS E MEDALHÕES DE KINSTONE
// ----------------------------------------------------------------------------

static void get_kinstone_colors(KinstoneType type, u32* out_main, u32* out_dark, u32* out_light) {
    if (type == KINSTONE_GREEN) {
        *out_main  = 0x22C55EFF; // Verde esmeralda vivo
        *out_dark  = 0x15803DFF; // Verde escuro sombreado
        *out_light = 0x86EFACFF; // Destaque claro
    } else if (type == KINSTONE_BLUE) {
        *out_main  = 0x3B82F6FF; // Azul safira
        *out_dark  = 0x1D4ED8FF;
        *out_light = 0x93C5FDFF;
    } else {
        *out_main  = 0xEF4444FF; // Vermelho rubi
        *out_dark  = 0xB91C1CFF;
        *out_light = 0xFCA5A5FF;
    }
}

/*
 * Desenha uma metade de Kinstone (Left Half para Link, Right Half para o NPC).
 */
static void draw_kinstone_half(int cx, int cy, KinstoneType type, bool is_left_half) {
    u32 c_main, c_dark, c_light;
    get_kinstone_colors(type, &c_main, &c_dark, &c_light);
    u32 c_gold = 0xD4AF37FF;
    u32 c_gold_hi = 0xFFE27AFF;

    int r = 13;

    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                // Metade esquerda ou direita
                bool in_half = is_left_half ? (x <= 1) : (x >= -1);
                if (!in_half) continue;

                // Encaixe / Recorte central tipo quebra-cabeça
                if (type == KINSTONE_GREEN) {
                    // Recorte ondulado suave no centro
                    if (is_left_half && x >= 0 && (y >= -4 && y <= 4)) continue;
                } else if (type == KINSTONE_BLUE) {
                    // Recorte em degrau / canto L
                    if (is_left_half && x >= 0 && y >= 0) continue;
                } else {
                    // Três dentes recortados
                    if (is_left_half && x >= 0 && ((y >= -8 && y <= -5) || (y >= 4 && y <= 7))) continue;
                }

                // Borda dourada externa
                bool is_edge = (x * x + y * y >= (r - 2) * (r - 2));
                u32 col = is_edge ? (y < 0 ? c_gold_hi : c_gold) :
                          (x * x + y * y < 20 ? c_light : (y > 3 ? c_dark : c_main));

                hal_video_put_pixel(cx + x, cy + y, col);
            }
        }
    }
}

/*
 * Desenha o medalhão completo fundido girando e brilhando.
 */
static void draw_kinstone_full(int cx, int cy, KinstoneType type, int timer) {
    u32 c_main, c_dark, c_light;
    get_kinstone_colors(type, &c_main, &c_dark, &c_light);
    u32 c_gold = 0xD4AF37FF;
    u32 c_gold_hi = 0xFFE27AFF;

    int r = 14;
    float pulse = 1.0f + 0.08f * sinf((float)timer * 0.25f);
    int pr = (int)((float)r * pulse);

    for (int y = -pr; y <= pr; y++) {
        for (int x = -pr; x <= pr; x++) {
            if (x * x + y * y <= pr * pr) {
                bool is_edge = (x * x + y * y >= (pr - 2) * (pr - 2));
                u32 col = is_edge ? (y < 0 ? c_gold_hi : c_gold) :
                          (x * x + y * y < 16 ? c_light : (y > 4 ? c_dark : c_main));

                // Costura mística dourada brilhante na união central
                if (abs(x) <= 1) col = 0xFFFFFFFF;

                hal_video_put_pixel(cx + x, cy + y, col);
            }
        }
    }

    // Estrelas cintilantes nos quatro polos
    hal_video_put_pixel(cx, cy - pr - 2, c_gold_hi);
    hal_video_put_pixel(cx, cy + pr + 2, c_gold_hi);
    hal_video_put_pixel(cx - pr - 2, cy, c_gold_hi);
    hal_video_put_pixel(cx + pr + 2, cy, c_gold_hi);
}

void kinstone_render(void) {
    if (s_ui_state == KINSTONE_UI_INACTIVE) return;

    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx) return;

    int screen_w = ctx->render_width;
    int screen_h = ctx->render_height;

    // 1. Pano de fundo com efeito de vinheta azul marinho translúcido
    draw_rect_blend(0, 0, screen_w, screen_h, 0x060E18D8);

    // 2. Barra Superior Dourada / Título
    draw_rect_blend(0, 3, screen_w, 15, 0x0C1C28EE);
    draw_rect_blend(0, 3, screen_w, 1, 0xD4AF37FF);
    draw_rect_blend(0, 17, screen_w, 1, 0xD4AF37FF);

    const char* title = "FUSAO DE KINSTONE";
    font_draw_text((screen_w - (int)strlen(title) * 8) / 2, 6, title, 0xFFE27AFF, true);

    // 3. Arena Central de Fusão
    int arena_y = 66;

    // Pedestal decorado
    int ped_w = 120;
    int ped_x = (screen_w - ped_w) / 2;
    draw_rect_blend(ped_x, arena_y + 20, ped_w, 4, 0x1B3045FF);
    draw_rect_blend(ped_x + 10, arena_y + 24, ped_w - 20, 2, 0x0E1E2EFF);

    if (s_ui_state == KINSTONE_UI_SELECTING) {
        // Exibe metade do Link (esquerda) e metade do NPC (direita)
        int left_cx  = 96;
        int right_cx = 144;

        KinstoneType player_type = (KinstoneType)s_selected_idx;
        KinstoneType npc_type    = (KinstoneType)(s_partner_npc ? s_partner_npc->kinstoneType : 0);

        // Nomes dos portadores
        font_draw_text(left_cx - 16, arena_y - 28, "Link", 0x77FF99FF, true);
        font_draw_text(right_cx - 16, arena_y - 28, "Parceiro", 0xFFE27AFF, true);

        // Desenha a metade do Link (se tiver fragmentos desse tipo)
        if (s_inv.counts[player_type] > 0) {
            draw_kinstone_half(left_cx, arena_y, player_type, true);
        } else {
            // Silhueta cinza escura vazia
            for (int y = -12; y <= 12; y++) {
                for (int x = -12; x <= 0; x++) {
                    if (x * x + y * y <= 144) hal_video_put_pixel(left_cx + x, arena_y + y, 0x223344FF);
                }
            }
        }

        // Desenha a metade do NPC parceiro
        draw_kinstone_half(right_cx, arena_y, npc_type, false);

        // Feedback de Compatibilidade
        bool matches = (player_type == npc_type && s_inv.counts[player_type] > 0);
        if (matches) {
            const char* msg = "As pecas se encaixam!";
            font_draw_text((screen_w - (int)strlen(msg) * 8) / 2, arena_y + 32, msg, 0x00FFCCFF, true);

            const char* btn = "[A] Fundir Pedras";
            font_draw_text((screen_w - (int)strlen(btn) * 8) / 2, arena_y + 44, btn, 0xFFE27AFF, true);
        } else {
            const char* msg = (s_inv.counts[player_type] == 0) ? "Nao possui este fragmento!" : "As pecas nao se encaixam...";
            font_draw_text((screen_w - (int)strlen(msg) * 8) / 2, arena_y + 32, msg, 0xF87171FF, true);

            const char* btn = "[B] Cancelar";
            font_draw_text((screen_w - (int)strlen(btn) * 8) / 2, arena_y + 44, btn, 0xCCCCCCFF, true);
        }
    } else if (s_ui_state == KINSTONE_UI_FUSING) {
        // Animação das duas metades deslizando para o centro
        KinstoneType type = (KinstoneType)s_selected_idx;
        draw_kinstone_half((int)s_left_x, arena_y, type, true);
        draw_kinstone_half((int)s_right_x, arena_y, type, false);

        // Linhas de energia mística convergindo
        for (int p = 0; p < 6; p++) {
            float t = ((float)((s_anim_timer * 4 + p * 20) % 60)) / 60.0f;
            int px = (int)(s_left_x + ((120.0f - s_left_x) * t));
            int py = arena_y + (int)(sinf(t * PI_F * 2.0f + p) * 6.0f);
            hal_video_put_pixel(px, py, 0xFFE27AFF);
        }
    } else if (s_ui_state == KINSTONE_UI_CELEBRATE) {
        // Medalhão completo formado e brilhando
        KinstoneType type = (KinstoneType)s_selected_idx;
        draw_kinstone_full(120, arena_y, type, s_anim_timer);

        const char* msg = "ENCAIXE PERFEITO!";
        font_draw_text((screen_w - (int)strlen(msg) * 8) / 2, arena_y + 34, msg, 0xFFDD00FF, true);
    }

    // 4. Partículas Mágicas de Brilho
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (s_particles[i].life > 0) {
            int px = (int)s_particles[i].x;
            int py = (int)s_particles[i].y;
            hal_video_put_pixel(px, py, s_particles[i].color);
            hal_video_put_pixel(px + 1, py, s_particles[i].color);
            hal_video_put_pixel(px, py + 1, s_particles[i].color);
        }
    }

    // 5. Flash Mágico na Tela
    if (s_flash_alpha > 0) {
        u32 flash_col = (0xFF << 24) | (0xF5 << 16) | (0xCC << 8) | (s_flash_alpha & 0xFF);
        draw_rect_blend(0, 0, screen_w, screen_h, flash_col);
    }

    // 6. Inventário de Kinstones na Base (Slot Selector)
    if (s_ui_state == KINSTONE_UI_SELECTING) {
        int inv_y = screen_h - 26;
        int inv_w = 150;
        int inv_x = (screen_w - inv_w) / 2;

        draw_rect_blend(inv_x, inv_y, inv_w, 22, 0x0A1824EE);
        draw_rect_blend(inv_x, inv_y, inv_w, 1, 0x335577FF);

        for (int k = 0; k < KINSTONE_COUNT; k++) {
            int slot_x = inv_x + 18 + (k * 44);
            int slot_y = inv_y + 11;

            // Borda de seleção destacada
            if (k == s_selected_idx) {
                int bounce = ((s_anim_timer / 10) % 2 == 1) ? 1 : 0;
                draw_rect_blend(slot_x - 14, slot_y - 9 - bounce, 28, 19, 0xD4AF3788);
                font_draw_char(slot_x - 2, slot_y - 13 - bounce, FONT_CHAR_ARROW_DOWN, 0xFFE27AFF, false);
            }

            // Ícone da Kinstone em miniatura
            u32 c_m, c_d, c_l;
            get_kinstone_colors((KinstoneType)k, &c_m, &c_d, &c_l);
            for (int dy = -4; dy <= 4; dy++) {
                for (int dx = -4; dx <= 4; dx++) {
                    if (dx * dx + dy * dy <= 16) hal_video_put_pixel(slot_x + dx - 4, slot_y + dy, c_m);
                }
            }

            // Contador de quantidade (ex: x2)
            char count_str[8];
            snprintf(count_str, sizeof(count_str), "x%d", s_inv.counts[k]);
            font_draw_text(slot_x + 3, slot_y - 3, count_str, (s_inv.counts[k] > 0 ? 0xF5F7FAFF : 0x777777FF), false);
        }
    }

    // 7. Janela Pop-up de Notificação do Evento Mundial Destravado
    if (s_ui_state == KINSTONE_UI_EVENT_POPUP) {
        int pw = 200;
        int ph = 56;
        int px = (screen_w - pw) / 2;
        int py = (screen_h - ph) / 2 + 10;

        draw_rect_blend(px, py, pw, ph, 0x081A12F5);
        draw_rect_blend(px - 1, py - 1, pw + 2, 1, 0xD4AF37FF);
        draw_rect_blend(px - 1, py + ph, pw + 2, 1, 0xD4AF37FF);
        draw_rect_blend(px - 1, py - 1, 1, ph + 2, 0xD4AF37FF);
        draw_rect_blend(px + pw, py - 1, 1, ph + 2, 0xD4AF37FF);

        font_draw_text(px + 12, py + 8,  "Fusao bem-sucedida!", 0xFFE27AFF, true);
        font_draw_text(px + 12, py + 22, "Um bau dourado apareceu", 0xF5F7FAFF, true);
        font_draw_text(px + 12, py + 32, "na clareira dos bosques!", 0x77FF99FF, true);

        font_draw_text(px + 110, py + 43, "[A] Fechar", 0xFFCC00FF, true);
    }
}
