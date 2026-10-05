/*
 * ============================================================================
 * src/hal/startup_menu.c - Máquina de Estados de Abertura & Seleção de Save
 * ============================================================================
 * Implementa de forma 100% nativa (Clean-Room C11 HAL / Zero-ROM Runtime):
 *   1. Logo da Capcom com jingle harmônico
 *   2. Logo da Nintendo com fade suave
 *   3. Tela de Título clássica com Four Sword, brilho e "PRESS START"
 *   4. Tela de Seleção de Arquivo (3 Slots com metadados do herói)
 *   5. Registro de Nome do Herói (Teclado virtual minish de 6 letras)
 *   6. Modos de Cópia e Exclusão de Arquivos
 */

#include "hal/startup_menu.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/input.h"
#include "hal/font.h"
#include "hal/save.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI_F 3.14159265358979323846f
#define MAX_NAME_CHARS 6

typedef struct {
    StartupMenuState state;
    int   timer;
    float alpha;
    float flash_alpha;

    // Title Screen
    int   sword_gleam_timer;
    int   press_start_timer;
    float motes_x[20];
    float motes_y[20];
    float motes_speed[20];

    // File Select
    int   selected_slot;      // 0, 1, 2
    int   cursor_bounce;
    bool  slot_has_data[3];
    SaveData slot_data[3];
    int   copy_source_slot;   // 0..2 (-1 se inativo)
    int   erase_confirm_idx;  // 0: Não, 1: Sim

    // Name Entry
    char  name_buffer[MAX_NAME_CHARS + 2];
    int   name_len;
    int   cursor_row;
    int   cursor_col;
    int   letter_blink;

    // Conclusão
    int   chosen_slot;        // 1, 2, 3
    bool  is_new_game;
    bool  is_active;
} StartupMenuContext;

static StartupMenuContext s_menu = { 0 };

// ----------------------------------------------------------------------------
// Funções Auxiliares de Desenho no Framebuffer
// ----------------------------------------------------------------------------

static void sm_draw_rect(int rx, int ry, int rw, int rh, u32 color) {
    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer) return;

    int x0 = rx < 0 ? 0 : rx;
    int y0 = ry < 0 ? 0 : ry;
    int x1 = (rx + rw) > ctx->render_width ? ctx->render_width : (rx + rw);
    int y1 = (ry + rh) > ctx->render_height ? ctx->render_height : (ry + rh);

    for (int y = y0; y < y1; y++) {
        int row_idx = y * ctx->render_width;
        for (int x = x0; x < x1; x++) {
            ctx->framebuffer[row_idx + x] = color;
        }
    }
}

static void sm_draw_rect_blend(int rx, int ry, int rw, int rh, u32 color, float alpha) {
    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer || alpha <= 0.0f) return;
    if (alpha > 1.0f) alpha = 1.0f;

    u8 cr = (color >> 24) & 0xFF;
    u8 cg = (color >> 16) & 0xFF;
    u8 cb = (color >> 8)  & 0xFF;

    int x0 = rx < 0 ? 0 : rx;
    int y0 = ry < 0 ? 0 : ry;
    int x1 = (rx + rw) > ctx->render_width ? ctx->render_width : (rx + rw);
    int y1 = (ry + rh) > ctx->render_height ? ctx->render_height : (ry + rh);

    for (int y = y0; y < y1; y++) {
        int row_idx = y * ctx->render_width;
        for (int x = x0; x < x1; x++) {
            u32 bg = ctx->framebuffer[row_idx + x];
            u8 br = (bg >> 24) & 0xFF;
            u8 bg_g = (bg >> 16) & 0xFF;
            u8 bb = (bg >> 8)  & 0xFF;

            u8 nr = (u8)(cr * alpha + br * (1.0f - alpha));
            u8 ng = (u8)(cg * alpha + bg_g * (1.0f - alpha));
            u8 nb = (u8)(cb * alpha + bb * (1.0f - alpha));
            ctx->framebuffer[row_idx + x] = (nr << 24) | (ng << 16) | (nb << 8) | 0xFF;
        }
    }
}

static void sm_draw_pixel(int x, int y, u32 color) {
    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer) return;
    if (x >= 0 && x < ctx->render_width && y >= 0 && y < ctx->render_height) {
        ctx->framebuffer[y * ctx->render_width + x] = color;
    }
}

static void sm_draw_heart_icon(int hx, int hy, bool filled) {
    u32 red   = filled ? 0xEF4444FF : 0x475569FF;
    u32 dark  = filled ? 0x991B1BFF : 0x1E293BFF;
    u32 white = filled ? 0xFEE2E2FF : 0x64748BFF;

    sm_draw_pixel(hx + 1, hy, red);
    sm_draw_pixel(hx + 2, hy, red);
    sm_draw_pixel(hx + 4, hy, red);
    sm_draw_pixel(hx + 5, hy, red);

    sm_draw_pixel(hx,     hy + 1, red);
    sm_draw_pixel(hx + 1, hy + 1, white);
    sm_draw_pixel(hx + 2, hy + 1, red);
    sm_draw_pixel(hx + 3, hy + 1, dark);
    sm_draw_pixel(hx + 4, hy + 1, red);
    sm_draw_pixel(hx + 5, hy + 1, red);
    sm_draw_pixel(hx + 6, hy + 1, red);

    sm_draw_pixel(hx,     hy + 2, red);
    sm_draw_pixel(hx + 1, hy + 2, red);
    sm_draw_pixel(hx + 2, hy + 2, red);
    sm_draw_pixel(hx + 3, hy + 2, red);
    sm_draw_pixel(hx + 4, hy + 2, red);
    sm_draw_pixel(hx + 5, hy + 2, red);
    sm_draw_pixel(hx + 6, hy + 2, red);

    sm_draw_pixel(hx + 1, hy + 3, red);
    sm_draw_pixel(hx + 2, hy + 3, red);
    sm_draw_pixel(hx + 3, hy + 3, red);
    sm_draw_pixel(hx + 4, hy + 3, red);
    sm_draw_pixel(hx + 5, hy + 3, red);

    sm_draw_pixel(hx + 2, hy + 4, red);
    sm_draw_pixel(hx + 3, hy + 4, red);
    sm_draw_pixel(hx + 4, hy + 4, red);

    sm_draw_pixel(hx + 3, hy + 5, red);
}

// ----------------------------------------------------------------------------
// Carregamento dos Metadados dos Saves
// ----------------------------------------------------------------------------

static void sm_refresh_slots(void) {
    for (int i = 0; i < 3; i++) {
        int slot = i + 1;
        s_menu.slot_has_data[i] = save_exists(slot);
        if (s_menu.slot_has_data[i]) {
            load_game(slot, &s_menu.slot_data[i]);
        } else {
            memset(&s_menu.slot_data[i], 0, sizeof(SaveData));
        }
    }
}

// ----------------------------------------------------------------------------
// Ciclo de Vida da Máquina de Estados
// ----------------------------------------------------------------------------

void startup_menu_init(void) {
    memset(&s_menu, 0, sizeof(StartupMenuContext));
    s_menu.state = STARTUP_STATE_CAPCOM_LOGO;
    s_menu.timer = 0;
    s_menu.alpha = 0.0f;
    s_menu.flash_alpha = 0.0f;
    s_menu.selected_slot = 0;
    s_menu.copy_source_slot = -1;
    s_menu.is_active = true;
    s_menu.chosen_slot = 1;
    s_menu.is_new_game = false;

    // Inicializa poeira mágica / partículas no ar
    for (int i = 0; i < 20; i++) {
        s_menu.motes_x[i] = (float)(rand() % 240);
        s_menu.motes_y[i] = (float)(rand() % 160);
        s_menu.motes_speed[i] = 0.25f + ((float)(rand() % 100) / 200.0f);
    }

    strcpy(s_menu.name_buffer, "LINK");
    s_menu.name_len = 4;
    s_menu.cursor_row = 0;
    s_menu.cursor_col = 0;

    sm_refresh_slots();
    printf("[STARTUP] Maquina de Estados de Abertura iniciada no estado: CAPCOM_LOGO\n");
}

bool startup_menu_is_active(void) {
    return s_menu.is_active && (s_menu.state != STARTUP_STATE_FINISHED);
}

StartupMenuState startup_menu_get_state(void) {
    return s_menu.state;
}

int startup_menu_get_selected_slot(void) {
    return s_menu.chosen_slot;
}

bool startup_menu_is_new_game(void) {
    return s_menu.is_new_game;
}

const char* startup_menu_get_player_name(void) {
    return s_menu.name_buffer;
}

void startup_menu_return_to_title(void) {
    s_menu.state = STARTUP_STATE_TITLE_SCREEN;
    s_menu.timer = 0;
    s_menu.alpha = 1.0f;
    s_menu.flash_alpha = 0.0f;
    s_menu.is_active = true;
    hal_audio_play_bgm(BGM_TITLE_THEME);
    printf("[STARTUP] Retornando a Tela de Titulo.\n");
}

void startup_menu_shutdown(void) {
    s_menu.is_active = false;
    s_menu.state = STARTUP_STATE_FINISHED;
}

// ----------------------------------------------------------------------------
// Atualização de Lógica & Entrada
// ----------------------------------------------------------------------------

void startup_menu_update(void) {
    if (!s_menu.is_active) return;

    s_menu.timer++;

    // Teclas globais de avanço
    bool btn_a     = hal_input_is_pressed(KEY_A);
    bool btn_b     = hal_input_is_pressed(KEY_B);
    bool btn_start = hal_input_is_pressed(KEY_START);
    bool nav_up    = hal_input_is_pressed(KEY_UP);
    bool nav_down  = hal_input_is_pressed(KEY_DOWN);
    bool nav_left  = hal_input_is_pressed(KEY_LEFT);
    bool nav_right = hal_input_is_pressed(KEY_RIGHT);

    // Efeito de flash em tela
    if (s_menu.flash_alpha > 0.0f) {
        s_menu.flash_alpha -= 0.05f;
        if (s_menu.flash_alpha < 0.0f) s_menu.flash_alpha = 0.0f;
    }

    switch (s_menu.state) {
        // --------------------------------------------------------------------
        // 1. LOGO DA CAPCOM
        // --------------------------------------------------------------------
        case STARTUP_STATE_CAPCOM_LOGO: {
            if (s_menu.timer == 5) {
                hal_audio_play_sound(SOUND_CAPCOM_CHIME, 0.95f, 1.0f);
            }

            // Fade-in e Fade-out
            if (s_menu.timer < 25) {
                s_menu.alpha = (float)s_menu.timer / 25.0f;
            } else if (s_menu.timer > 85) {
                s_menu.alpha = 1.0f - (float)(s_menu.timer - 85) / 25.0f;
            } else {
                s_menu.alpha = 1.0f;
            }

            // Pular com qualquer botão ou avanço por tempo
            if (btn_a || btn_b || btn_start || s_menu.timer >= 110) {
                s_menu.state = STARTUP_STATE_NINTENDO_LOGO;
                s_menu.timer = 0;
                s_menu.alpha = 0.0f;
            }
            break;
        }

        // --------------------------------------------------------------------
        // 2. LOGO DA NINTENDO
        // --------------------------------------------------------------------
        case STARTUP_STATE_NINTENDO_LOGO: {
            if (s_menu.timer == 6) {
                hal_audio_play_sound(SOUND_SECRET, 0.70f, 1.25f);
            }

            if (s_menu.timer < 20) {
                s_menu.alpha = (float)s_menu.timer / 20.0f;
            } else if (s_menu.timer > 75) {
                s_menu.alpha = 1.0f - (float)(s_menu.timer - 75) / 20.0f;
            } else {
                s_menu.alpha = 1.0f;
            }

            if (btn_a || btn_b || btn_start || s_menu.timer >= 95) {
                s_menu.state = STARTUP_STATE_TITLE_SCREEN;
                s_menu.timer = 0;
                s_menu.alpha = 1.0f;
                hal_audio_play_bgm(BGM_TITLE_THEME);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 3. TELA DE TÍTULO (THE MINISH CAP & PRESS START)
        // --------------------------------------------------------------------
        case STARTUP_STATE_TITLE_SCREEN: {
            s_menu.sword_gleam_timer = (s_menu.sword_gleam_timer + 1) % 180;
            s_menu.press_start_timer = (s_menu.press_start_timer + 1) % 50;

            // Movimento ascendente das partículas de luz
            for (int i = 0; i < 20; i++) {
                s_menu.motes_y[i] -= s_menu.motes_speed[i];
                if (s_menu.motes_y[i] < -4.0f) {
                    s_menu.motes_y[i] = 164.0f;
                    s_menu.motes_x[i] = (float)(rand() % 240);
                }
            }

            // Pressionar START ou A avança para seleção de save
            if (btn_start || btn_a) {
                hal_audio_play_sound(SOUND_TITLE_SWORD, 1.0f, 1.0f);
                s_menu.flash_alpha = 1.0f;
                s_menu.state = STARTUP_STATE_FILE_SELECT;
                s_menu.timer = 0;
                sm_refresh_slots();
                hal_audio_play_bgm(BGM_FILE_SELECT);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 4. SELEÇÃO DE ARQUIVO (FILE SELECT)
        // --------------------------------------------------------------------
        case STARTUP_STATE_FILE_SELECT: {
            s_menu.cursor_bounce = (s_menu.cursor_bounce + 1) % 40;

            if (nav_up) {
                s_menu.selected_slot = (s_menu.selected_slot + 2) % 3;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 0.8f, 1.0f);
            } else if (nav_down) {
                s_menu.selected_slot = (s_menu.selected_slot + 1) % 3;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 0.8f, 1.0f);
            }

            // Voltar ao título
            if (btn_b) {
                hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.8f, 0.9f);
                startup_menu_return_to_title();
                break;
            }

            // Modo Copiar (Gatilho L ou tecla de ação)
            if (hal_input_is_pressed(KEY_L) || hal_input_is_pressed(KEY_SELECT)) {
                if (s_menu.slot_has_data[s_menu.selected_slot]) {
                    s_menu.copy_source_slot = s_menu.selected_slot;
                    s_menu.state = STARTUP_STATE_FILE_COPY;
                    hal_audio_play_sound(SOUND_SWITCH_CLICK, 0.9f, 1.2f);
                    break;
                }
            }

            // Modo Apagar (Gatilho R)
            if (hal_input_is_pressed(KEY_R)) {
                if (s_menu.slot_has_data[s_menu.selected_slot]) {
                    s_menu.erase_confirm_idx = 0;
                    s_menu.state = STARTUP_STATE_FILE_ERASE;
                    hal_audio_play_sound(SOUND_BOMB_FUSE, 0.9f, 1.0f);
                    break;
                }
            }

            // Confirmar seleção
            if (btn_a || btn_start) {
                int slot = s_menu.selected_slot + 1;
                s_menu.chosen_slot = slot;

                if (s_menu.slot_has_data[s_menu.selected_slot]) {
                    // Arquivo existente -> inicia transição para gameplay
                    hal_audio_play_sound(SOUND_ITEM_CATCH, 1.0f, 1.0f);
                    s_menu.is_new_game = false;
                    strncpy(s_menu.name_buffer, s_menu.slot_data[s_menu.selected_slot].player_name, MAX_NAME_CHARS);
                    s_menu.name_buffer[MAX_NAME_CHARS] = '\0';
                    s_menu.state = STARTUP_STATE_TRANSITION_OUT;
                    s_menu.timer = 0;
                } else {
                    // Arquivo vazio -> abre tela de registro de nome
                    hal_audio_play_sound(SOUND_TEXT_BLIP, 0.85f, 1.3f);
                    s_menu.state = STARTUP_STATE_NAME_ENTRY;
                    s_menu.timer = 0;
                    strcpy(s_menu.name_buffer, "LINK");
                    s_menu.name_len = 4;
                    s_menu.cursor_row = 0;
                    s_menu.cursor_col = 0;
                }
            }
            break;
        }

        // --------------------------------------------------------------------
        // 5. REGISTRO DE NOME (NAME ENTRY)
        // --------------------------------------------------------------------
        case STARTUP_STATE_NAME_ENTRY: {
            s_menu.letter_blink = (s_menu.letter_blink + 1) % 30;

            static const char* k_grid[7] = {
                "ABCDEFGHIJ",
                "KLMNOPQRST",
                "UVWXYZ.-!?",
                "abcdefghij",
                "klmnopqrst",
                "uvwxyz0123",
                "456789\x08\x0D  " // \x08 = DEL, \x0D = FIM
            };

            if (nav_up) {
                s_menu.cursor_row = (s_menu.cursor_row + 6) % 7;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 0.8f, 1.0f);
            } else if (nav_down) {
                s_menu.cursor_row = (s_menu.cursor_row + 1) % 7;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 0.8f, 1.0f);
            } else if (nav_left) {
                s_menu.cursor_col = (s_menu.cursor_col + 9) % 10;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 0.8f, 1.0f);
            } else if (nav_right) {
                s_menu.cursor_col = (s_menu.cursor_col + 1) % 10;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 0.8f, 1.0f);
            }

            // Tecla B: Backspace
            if (btn_b) {
                if (s_menu.name_len > 0) {
                    s_menu.name_len--;
                    s_menu.name_buffer[s_menu.name_len] = '\0';
                    hal_audio_play_sound(SOUND_TEXT_BLIP, 0.7f, 0.8f);
                } else {
                    // Se nome vazio, cancela e volta à seleção de arquivo
                    s_menu.state = STARTUP_STATE_FILE_SELECT;
                    hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.8f, 0.9f);
                }
                break;
            }

            // Pressionar START é atalho direto para concluir
            bool confirm_entry = btn_start;

            if (btn_a) {
                char ch = k_grid[s_menu.cursor_row][s_menu.cursor_col];
                if (ch == '\x08') { // DEL
                    if (s_menu.name_len > 0) {
                        s_menu.name_len--;
                        s_menu.name_buffer[s_menu.name_len] = '\0';
                        hal_audio_play_sound(SOUND_TEXT_BLIP, 0.7f, 0.8f);
                    }
                } else if (ch == '\x0D') { // FIM
                    confirm_entry = true;
                } else if (ch != ' ') {
                    if (s_menu.name_len < MAX_NAME_CHARS) {
                        s_menu.name_buffer[s_menu.name_len] = ch;
                        s_menu.name_len++;
                        s_menu.name_buffer[s_menu.name_len] = '\0';
                        hal_audio_play_sound(SOUND_TEXT_BLIP, 0.8f, 1.2f);
                    }
                }
            }

            if (confirm_entry) {
                if (s_menu.name_len == 0) {
                    strcpy(s_menu.name_buffer, "LINK");
                    s_menu.name_len = 4;
                }

                // Cria o arquivo inicial e salva em disco
                SaveData new_save;
                memset(&new_save, 0, sizeof(SaveData));
                new_save.magic = SAVE_MAGIC;
                new_save.version = SAVE_VERSION;
                strncpy(new_save.player_name, s_menu.name_buffer, 15);
                new_save.player_x = 240.0f;
                new_save.player_y = 160.0f;
                new_save.player_dir = 2; // Virado para o sul
                new_save.current_map = 1; // Hyrule Town
                new_save.hearts = 3;
                new_save.max_hearts = 3;
                new_save.rupees = 0;
                new_save.slot_a = 0; // Sword
                new_save.slot_b = 0;

                save_game(s_menu.chosen_slot, &new_save);
                s_menu.is_new_game = true;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                s_menu.state = STARTUP_STATE_TRANSITION_OUT;
                s_menu.timer = 0;
            }
            break;
        }

        // --------------------------------------------------------------------
        // 6. MODO COPIAR ARQUIVO
        // --------------------------------------------------------------------
        case STARTUP_STATE_FILE_COPY: {
            if (nav_up) {
                s_menu.selected_slot = (s_menu.selected_slot + 2) % 3;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 0.8f, 1.0f);
            } else if (nav_down) {
                s_menu.selected_slot = (s_menu.selected_slot + 1) % 3;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 0.8f, 1.0f);
            }

            if (btn_b) {
                s_menu.state = STARTUP_STATE_FILE_SELECT;
                s_menu.copy_source_slot = -1;
                hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.8f, 0.9f);
                break;
            }

            if (btn_a) {
                int src = s_menu.copy_source_slot + 1;
                int dst = s_menu.selected_slot + 1;
                if (src != dst) {
                    save_copy(src, dst);
                    sm_refresh_slots();
                    hal_audio_play_sound(SOUND_ITEM_CATCH, 1.0f, 1.0f);
                }
                s_menu.state = STARTUP_STATE_FILE_SELECT;
                s_menu.copy_source_slot = -1;
            }
            break;
        }

        // --------------------------------------------------------------------
        // 7. MODO APAGAR ARQUIVO
        // --------------------------------------------------------------------
        case STARTUP_STATE_FILE_ERASE: {
            if (nav_left || nav_right) {
                s_menu.erase_confirm_idx = 1 - s_menu.erase_confirm_idx;
                hal_audio_play_sound(SOUND_MENU_CURSOR, 0.8f, 1.0f);
            }

            if (btn_b) {
                s_menu.state = STARTUP_STATE_FILE_SELECT;
                hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.8f, 0.9f);
                break;
            }

            if (btn_a) {
                if (s_menu.erase_confirm_idx == 1) { // Sim
                    save_delete(s_menu.selected_slot + 1);
                    sm_refresh_slots();
                    hal_audio_play_sound(SOUND_WALL_CRUMBLE, 1.0f, 1.1f);
                } else {
                    hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.8f, 0.9f);
                }
                s_menu.state = STARTUP_STATE_FILE_SELECT;
            }
            break;
        }

        // --------------------------------------------------------------------
        // 8. TRANSIÇÃO DE ENTRADA PARA O JOGO
        // --------------------------------------------------------------------
        case STARTUP_STATE_TRANSITION_OUT: {
            if (s_menu.timer < 30) {
                s_menu.alpha = (float)s_menu.timer / 30.0f;
            } else {
                s_menu.state = STARTUP_STATE_FINISHED;
                s_menu.is_active = false;
                hal_audio_stop_music();
                printf("[STARTUP] Transicao concluida! Iniciando jogo no Slot %d (Nome: '%s', Novo: %s)!\n",
                       s_menu.chosen_slot, s_menu.name_buffer, s_menu.is_new_game ? "SIM" : "NAO");
            }
            break;
        }

        default:
            break;
    }
}

// ----------------------------------------------------------------------------
// Renderização das Telas de Inicialização
// ----------------------------------------------------------------------------

void startup_menu_render(void) {
    if (!s_menu.is_active) return;
    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer) return;

    int W = ctx->render_width;
    int H = ctx->render_height;

    switch (s_menu.state) {
        // --------------------------------------------------------------------
        // 1. TELA CAPCOM
        // --------------------------------------------------------------------
        case STARTUP_STATE_CAPCOM_LOGO: {
            sm_draw_rect(0, 0, W, H, 0xFFFFFFFF); // Fundo branco

            // Letras da CAPCOM estilizadas (Azul 0x003A8CFF e Amarelo 0xFFCC00FF)
            const char* capcom = "C A P C O M";
            int tw = font_get_text_width(capcom);
            int tx = (W - tw) / 2;
            int ty = (H / 2) - 12;

            // Borda azul em torno das letras
            font_draw_text(tx - 1, ty,     capcom, 0x003A8CFF, false);
            font_draw_text(tx + 1, ty,     capcom, 0x003A8CFF, false);
            font_draw_text(tx,     ty - 1, capcom, 0x003A8CFF, false);
            font_draw_text(tx,     ty + 1, capcom, 0x003A8CFF, false);
            font_draw_text(tx,     ty,     capcom, 0xFFCC00FF, false);

            const char* c_copy = "(C) CAPCOM CO., LTD. 2004";
            int cw = font_get_text_width(c_copy);
            font_draw_text((W - cw) / 2, ty + 24, c_copy, 0x64748BFF, false);

            // Cortina de fade
            if (s_menu.alpha < 1.0f) {
                sm_draw_rect_blend(0, 0, W, H, 0x000000FF, 1.0f - s_menu.alpha);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 2. TELA NINTENDO
        // --------------------------------------------------------------------
        case STARTUP_STATE_NINTENDO_LOGO: {
            sm_draw_rect(0, 0, W, H, 0xFFFFFFFF);

            // Badge vermelha arredondada da Nintendo
            int bw = 96;
            int bh = 24;
            int bx = (W - bw) / 2;
            int by = (H - bh) / 2 - 4;

            sm_draw_rect(bx + 4, by, bw - 8, bh, 0xE60012FF);
            sm_draw_rect(bx, by + 4, bw, bh - 8, 0xE60012FF);
            // Cantos
            sm_draw_pixel(bx + 1, by + 1, 0xE60012FF);
            sm_draw_pixel(bx + bw - 2, by + 1, 0xE60012FF);
            sm_draw_pixel(bx + 1, by + bh - 2, 0xE60012FF);
            sm_draw_pixel(bx + bw - 2, by + bh - 2, 0xE60012FF);

            const char* nin = "Nintendo";
            int nw = font_get_text_width(nin);
            font_draw_text((W - nw) / 2, by + 8, nin, 0xFFFFFFFF, false);

            if (s_menu.alpha < 1.0f) {
                sm_draw_rect_blend(0, 0, W, H, 0x000000FF, 1.0f - s_menu.alpha);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 3. TELA DE TÍTULO (THE MINISH CAP & PRESS START)
        // --------------------------------------------------------------------
        case STARTUP_STATE_TITLE_SCREEN: {
            // Gradiente de fundo noturno real
            for (int y = 0; y < H; y++) {
                float t = (float)y / (float)H;
                u8 r = (u8)(10 + t * 20);
                u8 g = (u8)(16 + t * 25);
                u8 b = (u8)(36 + t * 30);
                u32 line_color = (r << 24) | (g << 16) | (b << 8) | 0xFF;
                sm_draw_rect(0, y, W, 1, line_color);
            }

            // Partículas de poeira mágica/luz
            for (int i = 0; i < 20; i++) {
                int px = (int)s_menu.motes_x[i];
                int py = (int)s_menu.motes_y[i];
                sm_draw_pixel(px, py, 0xFDE047CC);
                sm_draw_pixel(px + 1, py, 0xFEF08ACC);
            }

            // Pedestal de Pedra Sagrado da Four Sword
            int px = W / 2;
            int py = 120;

            // Degraus de pedra
            sm_draw_rect(px - 38, py + 8, 76, 8, 0x334155FF);
            sm_draw_rect(px - 28, py + 2, 56, 7, 0x475569FF);
            sm_draw_rect(px - 18, py - 4, 36, 7, 0x64748BFF);

            // A Four Sword cravada na rocha
            // Lâmina de prata
            sm_draw_rect(px - 1, py - 24, 3, 20, 0xF8FAFCFF);
            sm_draw_pixel(px, py - 24, 0x38BDF8FF);
            // Guarda dourada em cruz
            sm_draw_rect(px - 8, py - 27, 17, 3, 0xFBBF24FF);
            // Gema mística azul no centro da cruz
            sm_draw_pixel(px, py - 26, 0x06B6D4FF);
            // Empunhadura e pomo de ouro
            sm_draw_rect(px - 1, py - 33, 3, 6, 0x15803DFF);
            sm_draw_pixel(px, py - 34, 0xFBBF24FF);

            // Anel de luz pulsante na base da lâmina
            float pulse = 0.5f + 0.5f * sinf(s_menu.timer * 0.08f);
            int glow_r = (int)(6.0f + pulse * 4.0f);
            for (int ang = 0; ang < 360; ang += 45) {
                float rad = ang * (PI_F / 180.0f);
                int gx = px + (int)(cosf(rad) * glow_r);
                int gy = py - 4 + (int)(sinf(rad) * (glow_r * 0.45f));
                sm_draw_pixel(gx, gy, 0xFDE047EE);
            }

            // Logotipo de Zelda
            // 1. "THE LEGEND OF"
            const char* t_legend = "THE LEGEND OF";
            int lw = font_get_text_width(t_legend);
            font_draw_text((W - lw) / 2, 18, t_legend, 0xFBBF24FF, true);

            // 2. "ZELDA" Monumental
            const char* t_zelda = "Z E L D A";
            int zw = font_get_text_width(t_zelda);
            int zx = (W - zw) / 2;
            int zy = 29;
            font_draw_text(zx - 1, zy, t_zelda, 0xFDE047FF, false);
            font_draw_text(zx + 1, zy, t_zelda, 0xFDE047FF, false);
            font_draw_text(zx, zy - 1, t_zelda, 0xFDE047FF, false);
            font_draw_text(zx, zy + 1, t_zelda, 0xFDE047FF, false);
            font_draw_text(zx, zy, t_zelda, 0xDC2626FF, true);

            // 3. Faixa verde "The Minish Cap"
            const char* t_mc = "The Minish Cap";
            int mcw = font_get_text_width(t_mc);
            int mcx = (W - mcw) / 2;
            int mcy = 44;
            sm_draw_rect(mcx - 6, mcy - 2, mcw + 12, 12, 0x052E16EE);
            sm_draw_rect(mcx - 5, mcy - 1, mcw + 10, 10, 0x15803DFF);
            font_draw_text(mcx, mcy, t_mc, 0xDCFCE7FF, true);

            // Prompt piscante "PRESS START"
            if (s_menu.press_start_timer < 34) {
                const char* p_start = "PRESS START";
                int psw = font_get_text_width(p_start);
                font_draw_text((W - psw) / 2, 138, p_start, 0xFEF08AFF, true);
            }

            // Copyright
            const char* c_foot = "(C) 2004 Nintendo / CAPCOM Co., Ltd.";
            int cfw = font_get_text_width(c_foot);
            font_draw_text((W - cfw) / 2, 150, c_foot, 0x94A3B8FF, false);

            // Flash branco de acionamento
            if (s_menu.flash_alpha > 0.0f) {
                sm_draw_rect_blend(0, 0, W, H, 0xFFFFFFFF, s_menu.flash_alpha);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 4. SELEÇÃO DE ARQUIVO (FILE SELECT)
        // --------------------------------------------------------------------
        case STARTUP_STATE_FILE_SELECT:
        case STARTUP_STATE_FILE_COPY:
        case STARTUP_STATE_FILE_ERASE: {
            // Fundo de tapeçaria nobre azul-marinho
            for (int y = 0; y < H; y++) {
                u32 bg_line = (y % 4 == 0) ? 0x0F172AFF : 0x1E293BFF;
                sm_draw_rect(0, y, W, 1, bg_line);
            }

            // Cabeçalho
            const char* f_title = (s_menu.state == STARTUP_STATE_FILE_COPY)  ? "COPIAR ARQUIVO: Escolha o destino" :
                                  ((s_menu.state == STARTUP_STATE_FILE_ERASE) ? "APAGAR ARQUIVO: Confirmar exclusao?" :
                                                                                "SELECAO DE ARQUIVO");
            int ftw = font_get_text_width(f_title);
            int ftx = (W - ftw) / 2;
            sm_draw_rect(ftx - 8, 4, ftw + 16, 14, 0x020617EE);
            sm_draw_rect(ftx - 7, 5, ftw + 14, 12, 0xFBBF24FF);
            sm_draw_rect(ftx - 6, 6, ftw + 12, 10, 0x0F172AFF);
            font_draw_text(ftx, 7, f_title, 0xFDE047FF, true);

            // 3 Slots de Salvamento
            int card_w = 216;
            int card_h = 32;
            int card_x = (W - card_w) / 2;

            for (int i = 0; i < 3; i++) {
                int card_y = 24 + i * 36;
                bool is_sel = (s_menu.selected_slot == i);
                bool is_copy_src = (s_menu.state == STARTUP_STATE_FILE_COPY && s_menu.copy_source_slot == i);

                u32 border_col = is_copy_src ? 0x38BDF8FF :
                                 (is_sel ? 0xFBBF24FF : 0x475569FF);
                u32 body_col   = is_sel ? 0x1E293BFF : 0x0F172AEE;

                sm_draw_rect(card_x, card_y, card_w, card_h, border_col);
                sm_draw_rect(card_x + 1, card_y + 1, card_w - 2, card_h - 2, body_col);

                // Cursor de Coração bouncando à esquerda
                if (is_sel) {
                    int c_offset = (s_menu.cursor_bounce < 20) ? 0 : 2;
                    sm_draw_heart_icon(card_x - 12 - c_offset, card_y + 12, true);
                }

                // Número do Arquivo
                char slot_num[8];
                snprintf(slot_num, sizeof(slot_num), "[ %d ]", i + 1);
                font_draw_text(card_x + 6, card_y + 4, slot_num, 0xFDE047FF, true);

                if (s_menu.slot_has_data[i]) {
                    const SaveData* sd = &s_menu.slot_data[i];
                    // Nome do jogador
                    font_draw_text(card_x + 36, card_y + 4, sd->player_name, 0xFFFFFFFF, true);

                    // Corações
                    int max_h = sd->max_hearts > 16 ? 16 : sd->max_hearts;
                    for (int h = 0; h < max_h; h++) {
                        int hx = card_x + 85 + (h % 8) * 8;
                        int hy = card_y + 3 + (h / 8) * 7;
                        sm_draw_heart_icon(hx, hy, (h < sd->hearts));
                    }

                    // Rupees
                    char rup_str[16];
                    snprintf(rup_str, sizeof(rup_str), "%d", sd->rupees);
                    sm_draw_pixel(card_x + 155, card_y + 5, 0x22C55EFF);
                    sm_draw_pixel(card_x + 156, card_y + 4, 0x22C55EFF);
                    sm_draw_pixel(card_x + 156, card_y + 6, 0x22C55EFF);
                    font_draw_text(card_x + 160, card_y + 4, rup_str, 0x86EFACFF, false);

                    // Elementos Conquistados (Gemas Earth, Fire, Water, Wind)
                    if (sd->has_earth_element) sm_draw_rect(card_x + 195, card_y + 4, 4, 4, 0x22C55EFF);
                    if (sd->has_fire_element)  sm_draw_rect(card_x + 201, card_y + 4, 4, 4, 0xEF4444FF);
                    if (sd->has_water_element) sm_draw_rect(card_x + 195, card_y + 10, 4, 4, 0x38BDF8FF);
                    if (sd->golden_kinstones_fused >= 5) sm_draw_rect(card_x + 201, card_y + 10, 4, 4, 0xFBBF24FF);
                } else {
                    font_draw_text(card_x + 36, card_y + 4, "- NOVO JOGO -", 0x94A3B8FF, false);
                    for (int h = 0; h < 3; h++) {
                        sm_draw_heart_icon(card_x + 140 + h * 8, card_y + 4, true);
                    }
                }
            }

            // Barra de Ações Inferior
            if (s_menu.state == STARTUP_STATE_FILE_ERASE) {
                // Diálogo de confirmação
                int mw = 180;
                int mx = (W - mw) / 2;
                int my = 134;
                sm_draw_rect(mx, my, mw, 20, 0x7F1D1DFF);
                sm_draw_rect(mx + 1, my + 1, mw - 2, 18, 0x450A0AFF);
                font_draw_text(mx + 8, my + 5, "Apagar? ", 0xFECACAFF, true);

                bool sel_no  = (s_menu.erase_confirm_idx == 0);
                bool sel_yes = (s_menu.erase_confirm_idx == 1);
                font_draw_text(mx + 70, my + 5, "[ NAO ]", sel_no ? 0xFDE047FF : 0x94A3B8FF, true);
                font_draw_text(mx + 125, my + 5, "[ SIM ]", sel_yes ? 0xEF4444FF : 0x94A3B8FF, true);
            } else {
                const char* help = (s_menu.state == STARTUP_STATE_FILE_COPY) ?
                    "[A] COPIAR AQUI   [B] CANCELAR" :
                    "[A] ESCOLHER   [L] COPIAR   [R] APAGAR   [B] TITULO";
                int hw = font_get_text_width(help);
                font_draw_text((W - hw) / 2, 142, help, 0xE2E8F0FF, true);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 5. REGISTRO DE NOME (NAME ENTRY)
        // --------------------------------------------------------------------
        case STARTUP_STATE_NAME_ENTRY: {
            sm_draw_rect(0, 0, W, H, 0x0F172AFF);

            // Cabeçalho
            const char* nt = "REGISTRO DE NOME";
            int ntw = font_get_text_width(nt);
            font_draw_text((W - ntw) / 2, 6, nt, 0xFBBF24FF, true);

            // Caixa de Exibição das 6 Letras
            int box_w = 120;
            int box_x = (W - box_w) / 2;
            int box_y = 20;
            sm_draw_rect(box_x, box_y, box_w, 18, 0x1E293BFF);
            sm_draw_rect(box_x + 1, box_y + 1, box_w - 2, 16, 0x020617FF);

            for (int i = 0; i < MAX_NAME_CHARS; i++) {
                int lx = box_x + 12 + i * 16;
                if (i < s_menu.name_len) {
                    char l_str[2] = { s_menu.name_buffer[i], '\0' };
                    font_draw_text(lx, box_y + 4, l_str, 0xFFFFFFFF, true);
                }
                // Cursor piscante
                if (i == s_menu.name_len && s_menu.letter_blink < 18) {
                    sm_draw_rect(lx, box_y + 13, 8, 2, 0xFDE047FF);
                } else {
                    sm_draw_rect(lx, box_y + 13, 8, 1, 0x475569FF);
                }
            }

            // Teclado Virtual (Grid 7 linhas x 10 colunas)
            static const char* k_grid[7] = {
                "ABCDEFGHIJ",
                "KLMNOPQRST",
                "UVWXYZ.-!?",
                "abcdefghij",
                "klmnopqrst",
                "uvwxyz0123",
                "456789\x08\x0D  "
            };

            int grid_x = (W - (10 * 14)) / 2;
            int grid_y = 44;

            for (int r = 0; r < 7; r++) {
                for (int c = 0; c < 10; c++) {
                    int kx = grid_x + c * 14;
                    int ky = grid_y + r * 13;
                    bool is_cur = (s_menu.cursor_row == r && s_menu.cursor_col == c);

                    char ch = k_grid[r][c];
                    if (ch == ' ') continue;

                    if (is_cur) {
                        sm_draw_rect(kx - 2, ky - 2, 12, 12, 0xFBBF24FF);
                        sm_draw_rect(kx - 1, ky - 1, 10, 10, 0x1E293BFF);
                    }

                    if (ch == '\x08') {
                        font_draw_text(kx - 1, ky, "DL", is_cur ? 0xEF4444FF : 0xFCA5A5FF, false);
                    } else if (ch == '\x0D') {
                        font_draw_text(kx - 1, ky, "OK", is_cur ? 0x22C55EFF : 0x86EFACFF, false);
                    } else {
                        char c_str[2] = { ch, '\0' };
                        font_draw_text(kx, ky, c_str, is_cur ? 0xFDE047FF : 0xFFFFFFFF, false);
                    }
                }
            }

            // Ajuda de rodapé
            const char* n_help = "[A] INSERIR   [B] APAGAR   [START] CONCLUIR";
            int nhw = font_get_text_width(n_help);
            font_draw_text((W - nhw) / 2, 144, n_help, 0x94A3B8FF, false);
            break;
        }

        // --------------------------------------------------------------------
        // 6. TRANSIÇÃO DE SAÍDA (FADE PARA GAMEPLAY)
        // --------------------------------------------------------------------
        case STARTUP_STATE_TRANSITION_OUT: {
            sm_draw_rect(0, 0, W, H, 0x000000FF);
            if (s_menu.alpha < 1.0f) {
                // Desenha a última tela com fade escuro
                sm_draw_rect_blend(0, 0, W, H, 0x000000FF, s_menu.alpha);
            }
            break;
        }

        default:
            break;
    }
}
