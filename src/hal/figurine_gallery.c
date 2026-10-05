/*
 * ============================================================================
 * src/hal/figurine_gallery.c - Galeria do Carlov & Sistema Gacha (Ato VI)
 * ============================================================================
 * Implementação clean-room C11 da Galeria de Estatuetas de Carlov e minigame Gacha
 * com probabilidades dinâmicas de Conchas Misteriosas, troféus 3D e medalha de Carlov.
 */

#include "hal/figurine_gallery.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SCREEN_W 256
#define SCREEN_H 160

// Paleta de cores da Galeria do Carlov
#define C_GALLERY_BG        0x1C1917FF // Interior de madeira nobre da árvore de Carlov
#define C_GALLERY_WOOD_DARK 0x0C0A09FF // Viga rústica entalhada
#define C_GACHA_DOME        0x38BDF855 // Cúpula translúcida da máquina
#define C_GACHA_BASE        0xDC2626FF // Base metálica vermelha vintage
#define C_GACHA_GOLD        0xF59E0BFF // Manivela dourada e detalhes
#define C_PEDESTAL_STONE    0x64748BFF // Mármore do pedestal de exibição
#define C_PEDESTAL_DARK     0x334155FF // Sombra do pedestal
#define C_CAPSULE_TOP       0xEF4444FF // Metade vermelha da cápsula
#define C_CAPSULE_BOT       0xF8FAFCFF // Metade branca da cápsula

static FigurineGallery s_gallery = { 0 };

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

static const char* s_fig_names[TOTAL_FIGURINES] = {
    "001: Link (O Heroi)", "002: Ezlo (Gorro Picori)", "003: Princesa Zelda", "004: Rei Daltus",
    "005: Mestre Smith", "006: Swiftblade", "007: Grayblade", "008: Grimblade",
    "009: Waveblade", "010: Coveiro Dampe", "011: Rei Gustaf", "012: Anciao Librari",
    "013: Minish da Floresta", "014: Minish da Cidade", "015: Mountain Minish", "016: Octorok Vermelho",
    "017: Morcego Keese", "018: Green ChuChu", "019: Spiny Beetle", "020: Guarda Moblin",
    "021: Darknut (Cavaleiro)", "022: Big Green ChuChu", "023: Gleerok (Lava)", "024: Mazaal (Fortaleza)",
    "025: Big Octorok (Gelo)", "026: Gyorg Pair (Ceu)", "027: Feiticeiro Vaati", "028: Vaati Transfigured",
    "029: Vaati's Wrath", "030: Mestre Carlov", "031: Malon da Fazenda", "032: Talon da Fazenda",
    "033: Epona (Ponei)", "034: Stockwell (Lojista)", "035: Anju das Galinhas", "036: Vovozinha Minish"
};

void figurine_gallery_init(void) {
    memset(&s_gallery, 0, sizeof(FigurineGallery));
    s_gallery.is_active = false;
    s_gallery.mode = GALLERY_MODE_GACHA;
    s_gallery.shells_owned = 50; // Quantidade inicial generosa de conchas
    s_gallery.shells_bet = 1;
    s_gallery.unlocked_count = 0;
    s_gallery.selected_id = 0;

    for (int i = 0; i < TOTAL_FIGURINES; i++) {
        s_gallery.figurines[i].id = i + 1;
        if (i < 36) {
            s_gallery.figurines[i].name = s_fig_names[i];
        } else {
            static char s_gen_names[100][32];
            snprintf(s_gen_names[i - 36], 32, "%03d: Trofeu de Hyrule %d", i + 1, i + 1);
            s_gallery.figurines[i].name = s_gen_names[i - 36];
        }
        s_gallery.figurines[i].desc_line1 = "Miniatura esculpida com maestria pelo";
        s_gallery.figurines[i].desc_line2 = "lendario artesao Carlov em sua arvore!";
        s_gallery.figurines[i].primary_color = (i % 2 == 0) ? 0x22C55EFF : 0x38BDF8FF;
        s_gallery.figurines[i].accent_color = 0xF59E0BFF;
        s_gallery.figurines[i].unlocked = false;
    }

    // Desbloqueia as 3 primeiras como brinde inicial
    s_gallery.figurines[0].unlocked = true;
    s_gallery.figurines[1].unlocked = true;
    s_gallery.figurines[2].unlocked = true;
    s_gallery.unlocked_count = 3;
}

void figurine_gallery_open(int shells) {
    if (!s_gallery.figurines[0].name) {
        figurine_gallery_init();
    }
    s_gallery.is_active = true;
    s_gallery.mode = GALLERY_MODE_GACHA;
    if (shells > 0) s_gallery.shells_owned = shells;
    s_gallery.shells_bet = 1;

    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
    printf("[CARLOV GALLERY] Galeria de Estatuetas aberta! Conchas: %d | Desbloqueadas: %d/%d\n",
           s_gallery.shells_owned, s_gallery.unlocked_count, TOTAL_FIGURINES);
}

void figurine_gallery_close(void) {
    s_gallery.is_active = false;
    printf("[CARLOV GALLERY] Galeria fechada.\n");
}

bool figurine_gallery_is_active(void) {
    return s_gallery.is_active;
}

static void calculate_win_chance(void) {
    float remaining_ratio = (float)(TOTAL_FIGURINES - s_gallery.unlocked_count) / (float)TOTAL_FIGURINES;
    if (remaining_ratio <= 0.0f) {
        s_gallery.win_chance_percent = 0.0f;
        return;
    }
    // Fórmula de probabilidade: 1 concha dá chance base, conchas adicionais escalam até 100%
    float chance = (float)s_gallery.shells_bet * (remaining_ratio * 8.0f + 1.5f);
    if (chance > 100.0f) chance = 100.0f;
    s_gallery.win_chance_percent = chance;
}

void figurine_gallery_handle_input(bool btn_a, bool btn_b, bool dpad_up, bool dpad_down,
                                   bool dpad_left, bool dpad_right, bool start) {
    if (!s_gallery.is_active) return;

    if (btn_b) {
        if (s_gallery.mode == GALLERY_MODE_INSPECTOR || s_gallery.mode == GALLERY_MODE_REVEAL) {
            s_gallery.mode = GALLERY_MODE_GACHA;
            hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 0.9f);
        } else {
            figurine_gallery_close();
            return;
        }
    }

    if (start) {
        // Alterna para o modo Inspetor de Coleção
        if (s_gallery.mode == GALLERY_MODE_GACHA) {
            s_gallery.mode = GALLERY_MODE_INSPECTOR;
            hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 1.2f);
        } else if (s_gallery.mode == GALLERY_MODE_INSPECTOR) {
            s_gallery.mode = GALLERY_MODE_GACHA;
            hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 0.9f);
        }
    }

    if (s_gallery.mode == GALLERY_MODE_GACHA) {
        // Ajuste da quantidade de conchas apostadas
        if (dpad_up) {
            if (s_gallery.shells_bet < s_gallery.shells_owned && s_gallery.shells_bet < 99) {
                s_gallery.shells_bet++;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 1.1f);
            }
        }
        if (dpad_down) {
            if (s_gallery.shells_bet > 1) {
                s_gallery.shells_bet--;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 0.9f);
            }
        }
        if (dpad_right) {
            // Aposta máxima ou +10
            s_gallery.shells_bet = (s_gallery.shells_bet + 10 <= s_gallery.shells_owned) ? s_gallery.shells_bet + 10 : s_gallery.shells_owned;
            if (s_gallery.shells_bet < 1) s_gallery.shells_bet = 1;
            hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 1.3f);
        }
        if (dpad_left) {
            s_gallery.shells_bet = (s_gallery.shells_bet - 10 >= 1) ? s_gallery.shells_bet - 10 : 1;
            hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 0.8f);
        }

        // Girar a manivela da máquina Gacha!
        if (btn_a && s_gallery.shells_owned >= s_gallery.shells_bet) {
            s_gallery.shells_owned -= s_gallery.shells_bet;
            s_gallery.mode = GALLERY_MODE_DISPENSING;
            s_gallery.dispense_timer = 60;
            hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.0f);
            printf("[GACHA] Manivela girada com %d conchas! Sorteando...\n", s_gallery.shells_bet);
        }
    } else if (s_gallery.mode == GALLERY_MODE_REVEAL) {
        if (btn_a) {
            s_gallery.mode = GALLERY_MODE_GACHA;
            s_gallery.shells_bet = 1;
            hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 1.0f);
        }
    } else if (s_gallery.mode == GALLERY_MODE_INSPECTOR) {
        if (dpad_right) {
            s_gallery.selected_id = (s_gallery.selected_id + 1) % TOTAL_FIGURINES;
            hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 1.1f);
        }
        if (dpad_left) {
            s_gallery.selected_id = (s_gallery.selected_id - 1 + TOTAL_FIGURINES) % TOTAL_FIGURINES;
            hal_audio_play_sound(SOUND_MENU_CURSOR, 1.0f, 0.9f);
        }
    }
}

void figurine_gallery_update(void) {
    if (!s_gallery.is_active) return;

    calculate_win_chance();

    if (s_gallery.mode == GALLERY_MODE_DISPENSING) {
        s_gallery.crank_angle += 0.25f;
        s_gallery.dispense_timer--;

        if (s_gallery.dispense_timer <= 0) {
            // Sorteio
            float roll = (float)(rand() % 10000) / 100.0f;
            bool win_new = (roll <= s_gallery.win_chance_percent) && (s_gallery.unlocked_count < TOTAL_FIGURINES);

            if (win_new) {
                // Seleciona uma estatueta ainda não desbloqueada
                int target = rand() % (TOTAL_FIGURINES - s_gallery.unlocked_count);
                int count = 0;
                for (int i = 0; i < TOTAL_FIGURINES; i++) {
                    if (!s_gallery.figurines[i].unlocked) {
                        if (count == target) {
                            s_gallery.figurines[i].unlocked = true;
                            s_gallery.current_won_id = i;
                            s_gallery.unlocked_count++;
                            s_gallery.is_duplicate = false;
                            break;
                        }
                        count++;
                    }
                }
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                printf("[GACHA] SUCESSO! Nova Estatueta conquistada: %s!\n", s_gallery.figurines[s_gallery.current_won_id].name);

                // Checa premiação da Medalha de Carlov
                if (s_gallery.unlocked_count >= 130 && !s_gallery.has_carlov_medal) {
                    s_gallery.has_carlov_medal = true;
                    s_gallery.medal_banner_timer = 240;
                    hal_audio_play_sound(SOUND_TIGER_SCROLL, 1.0f, 1.0f);
                    printf("[CARLOV MEDAL] O MESTRE CARLOV CONCEDEU A LENDARIA MEDALHA DE CARLOV!\n");
                }
            } else {
                // Repetida
                s_gallery.current_won_id = rand() % TOTAL_FIGURINES;
                s_gallery.is_duplicate = true;
                hal_audio_play_sound(SOUND_ITEM_CATCH, 0.9f, 0.8f);
                printf("[GACHA] Estatueta repetida! Tente novamente com mais conchas!\n");
            }

            s_gallery.mode = GALLERY_MODE_REVEAL;
        }
    }

    // Rotação contínua da estatueta no troféu 3D
    s_gallery.trophy_rot_angle += 0.03f;
    if (s_gallery.medal_banner_timer > 0) s_gallery.medal_banner_timer--;
}

void figurine_gallery_render(void) {
    if (!s_gallery.is_active) return;

    // Fundo de oficina de madeira de Carlov
    draw_filled_rect(0, 0, SCREEN_W, SCREEN_H, C_GALLERY_BG);
    for (int y = 0; y < SCREEN_H; y += 16) {
        draw_filled_rect(0, y, SCREEN_W, 1, C_GALLERY_WOOD_DARK);
    }

    // Top Header Banner
    draw_filled_rect(0, 0, SCREEN_W, 20, 0x0C0A09EE);
    draw_filled_rect(0, 19, SCREEN_W, 1, C_GACHA_GOLD);
    font_draw_text(10, 5, "GALERIA DO CARLOV - GACHA", 0xFDE047FF, true);

    char shells_txt[32];
    snprintf(shells_txt, sizeof(shells_txt), "CONCHAS: %d", s_gallery.shells_owned);
    font_draw_text(170, 5, shells_txt, 0x38BDF8FF, true);

    // ==========================================
    // MODO GACHA
    // ==========================================
    if (s_gallery.mode == GALLERY_MODE_GACHA || s_gallery.mode == GALLERY_MODE_DISPENSING) {
        // Máquina Gacha clássica (Centro Esquerda)
        int mx = 70;
        int my = 80;

        // Cúpula de Vidro
        draw_filled_rect(mx - 28, my - 45, 56, 45, 0x0284C744);
        draw_filled_rect(mx - 26, my - 43, 52, 41, 0x38BDF833);
        // Cápsulas coloridas dentro do globo
        int ball_colors[6] = { 0xEF4444FF, 0x3B82F6FF, 0x10B981FF, 0xF59E0BFF, 0x8B5CF6FF, 0xEC4899FF };
        for (int b = 0; b < 6; b++) {
            int bx = mx - 16 + (b % 3) * 12;
            int by = my - 35 + (b / 3) * 14;
            draw_filled_rect(bx, by, 10, 10, ball_colors[b]);
            draw_filled_rect(bx + 2, by + 2, 4, 4, 0xFFFFFFFF);
        }

        // Base Vermelha da Máquina
        draw_filled_rect(mx - 32, my, 64, 45, C_GACHA_BASE);
        draw_filled_rect(mx - 28, my + 4, 56, 4, 0x991B1BFF);

        // Manivela Gacha Dourada
        int crank_x = mx + (int)(cosf(s_gallery.crank_angle) * 10.0f);
        int crank_y = my + 20 + (int)(sinf(s_gallery.crank_angle) * 10.0f);
        draw_filled_rect(mx - 3, my + 17, 6, 6, 0x78350FFF);
        draw_filled_rect(crank_x - 4, crank_y - 4, 8, 8, C_GACHA_GOLD);

        // Calha de saída de cápsula
        draw_filled_rect(mx - 10, my + 30, 20, 12, 0x18181BFF);

        // Painel de Apostas & Probabilidades (Direita)
        int px = 135;
        int py = 35;
        draw_filled_rect(px, py, 110, 95, 0x292524FF);
        draw_filled_rect(px, py, 110, 95, 0x57534EFF);

        font_draw_text(px + 8, py + 8, "APOSTA DE CONCHAS", 0xFDE047FF, true);

        char bet_txt[32];
        snprintf(bet_txt, sizeof(bet_txt), "CONCHAS: [ %d ]", s_gallery.shells_bet);
        font_draw_text(px + 8, py + 26, bet_txt, 0xFFFFFFFF, true);

        char chance_txt[32];
        snprintf(chance_txt, sizeof(chance_txt), "CHANCE: %.1f%%", s_gallery.win_chance_percent);
        u32 ch_col = (s_gallery.win_chance_percent >= 100.0f) ? 0x4ADE80FF : (s_gallery.win_chance_percent >= 50.0f ? 0xFDE047FF : 0xF87171FF);
        font_draw_text(px + 8, py + 44, chance_txt, ch_col, true);

        char col_txt[32];
        snprintf(col_txt, sizeof(col_txt), "TOTAL: %d/%d", s_gallery.unlocked_count, TOTAL_FIGURINES);
        font_draw_text(px + 8, py + 62, col_txt, 0x38BDF8FF, true);

        // Instruções no rodapé
        draw_filled_rect(0, SCREEN_H - 18, SCREEN_W, 18, 0x0C0A09EE);
        font_draw_text(10, SCREEN_H - 14, "[A] GIRAR | [CIMA/BAIXO] CONCHAS | [START] GALERIA", 0xA8A29EFF, false);
    }

    // ==========================================
    // MODO REVELAÇÃO DA ESTATUETA
    // ==========================================
    else if (s_gallery.mode == GALLERY_MODE_REVEAL) {
        int fig_idx = s_gallery.current_won_id;
        FigurineEntry* fig = &s_gallery.figurines[fig_idx];

        // Pedestal Iluminado
        int px = 70;
        int py = 105;
        draw_filled_rect(px - 30, py, 60, 16, C_PEDESTAL_STONE);
        draw_filled_rect(px - 34, py + 12, 68, 6, C_PEDESTAL_DARK);

        // Troféu 3D Rotativo
        int rot_off = (int)(sinf(s_gallery.trophy_rot_angle) * 8.0f);
        draw_filled_rect(px - 10 + rot_off, py - 35, 20, 32, fig->primary_color);
        draw_filled_rect(px - 6 + rot_off, py - 32, 12, 10, fig->accent_color);
        draw_filled_rect(px - 14 + rot_off, py - 38, 28, 6, C_GACHA_GOLD); // Brilho de troféu

        // Painel de Lore e Nome
        int lx = 120;
        int ly = 32;
        draw_filled_rect(lx, ly, 126, 100, 0x292524FF);

        const char* status = s_gallery.is_duplicate ? "[REPETIDA]" : "[NOVA ESTATUETA!]";
        u32 st_col = s_gallery.is_duplicate ? 0x94A3B8FF : 0x4ADE80FF;
        font_draw_text(lx + 8, ly + 8, status, st_col, true);
        font_draw_text(lx + 8, ly + 24, fig->name, 0xFDE047FF, true);
        font_draw_text(lx + 8, ly + 46, fig->desc_line1, 0xE7E5E4FF, false);
        font_draw_text(lx + 8, ly + 60, fig->desc_line2, 0xE7E5E4FF, false);

        font_draw_text(lx + 8, ly + 80, "[A] CONTINUAR", 0x38BDF8FF, true);
    }

    // ==========================================
    // MODO INSPETOR DE GALERIA (BROWSER)
    // ==========================================
    else if (s_gallery.mode == GALLERY_MODE_INSPECTOR) {
        int fig_idx = s_gallery.selected_id;
        FigurineEntry* fig = &s_gallery.figurines[fig_idx];

        // Pedestal de exibição no centro
        int px = 70;
        int py = 105;
        draw_filled_rect(px - 30, py, 60, 16, C_PEDESTAL_STONE);
        draw_filled_rect(px - 34, py + 12, 68, 6, C_PEDESTAL_DARK);

        if (fig->unlocked) {
            int rot_off = (int)(sinf(s_gallery.trophy_rot_angle) * 8.0f);
            draw_filled_rect(px - 10 + rot_off, py - 35, 20, 32, fig->primary_color);
            draw_filled_rect(px - 6 + rot_off, py - 32, 12, 10, fig->accent_color);
            draw_filled_rect(px - 14 + rot_off, py - 38, 28, 6, C_GACHA_GOLD);
        } else {
            // Silhueta misteriosa com ponto de interrogação
            draw_filled_rect(px - 10, py - 35, 20, 32, 0x292524FF);
            font_draw_text(px - 4, py - 26, "?", 0x78716CFF, true);
        }

        // Descrição
        int lx = 120;
        int ly = 32;
        draw_filled_rect(lx, ly, 126, 100, 0x292524FF);

        if (fig->unlocked) {
            font_draw_text(lx + 8, ly + 8, fig->name, 0xFDE047FF, true);
            font_draw_text(lx + 8, ly + 30, fig->desc_line1, 0xE7E5E4FF, false);
            font_draw_text(lx + 8, ly + 46, fig->desc_line2, 0xE7E5E4FF, false);
            font_draw_text(lx + 8, ly + 76, "STATUS: COLETADA", 0x4ADE80FF, true);
        } else {
            font_draw_text(lx + 8, ly + 8, "??? (BLOQUEADA)", 0x94A3B8FF, true);
            font_draw_text(lx + 8, ly + 36, "Aposte conchas misteriosas", 0x78716CFF, false);
            font_draw_text(lx + 8, ly + 50, "na maquina para desbloquear!", 0x78716CFF, false);
        }

        draw_filled_rect(0, SCREEN_H - 18, SCREEN_W, 18, 0x0C0A09EE);
        font_draw_text(10, SCREEN_H - 14, "[ESQ/DIR] NAVEGAR | [B/START] VOLTAR", 0xA8A29EFF, false);
    }

    // Banner da Medalha de Carlov (quando alcança 130+)
    if (s_gallery.medal_banner_timer > 0) {
        int bw = 240;
        int bx = (SCREEN_W - bw) / 2;
        int by = 35;
        draw_filled_rect(bx - 4, by - 4, bw + 8, 32, 0x1C1917EE);
        draw_filled_rect(bx - 3, by - 3, bw + 6, 30, 0xF59E0BFF);
        draw_filled_rect(bx - 2, by - 2, bw + 4, 28, 0x451A03EE);
        font_draw_text(bx + 20, by + 2, "MEDALHA DE CARLOV CONQUISTADA!", 0xFDE047FF, true);
        font_draw_text(bx + 4, by + 14, "Mestre Carlov premiou sua colecao! Casa de Hyrule aberta!", 0xFEF08AFF, false);
    }
}

int figurine_gallery_get_shells(void) {
    return s_gallery.shells_owned;
}

int figurine_gallery_get_unlocked_count(void) {
    return s_gallery.unlocked_count;
}

bool figurine_gallery_has_medal(void) {
    return s_gallery.has_carlov_medal;
}

void figurine_gallery_deposit_shells(int count) {
    s_gallery.shells_owned += count;
    if (s_gallery.shells_owned > MAX_SHELLS) s_gallery.shells_owned = MAX_SHELLS;
}

void figurine_gallery_export(u8* bitmask_17bytes, int* count, bool* medal, int* shells) {
    if (bitmask_17bytes) {
        memset(bitmask_17bytes, 0, 17);
        for (int i = 0; i < TOTAL_FIGURINES; i++) {
            if (s_gallery.figurines[i].unlocked) {
                bitmask_17bytes[i / 8] |= (1 << (i % 8));
            }
        }
    }
    if (count) *count = s_gallery.unlocked_count;
    if (medal) *medal = s_gallery.has_carlov_medal;
    if (shells) *shells = s_gallery.shells_owned;
}

void figurine_gallery_restore(const u8* bitmask_17bytes, int count, bool medal, int shells) {
    if (!s_gallery.figurines[0].name) {
        figurine_gallery_init();
    }
    if (bitmask_17bytes) {
        int restored_cnt = 0;
        for (int i = 0; i < TOTAL_FIGURINES; i++) {
            bool unl = (bitmask_17bytes[i / 8] & (1 << (i % 8))) != 0;
            s_gallery.figurines[i].unlocked = unl;
            if (unl) restored_cnt++;
        }
        s_gallery.unlocked_count = restored_cnt;
    }
    s_gallery.has_carlov_medal = medal;
    s_gallery.shells_owned = shells;
}
