/*
 * ============================================================================
 * src/hal/intro_cutscene.c - Cutscene Canônica de Introdução (Ato VI)
 * ============================================================================
 * Implementação nativa Clean-Room C11 HAL / Zero-ROM Runtime da narrativa
 * inaugural de The Legend of Zelda: The Minish Cap.
 */

#include "hal/intro_cutscene.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/input.h"
#include "hal/font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SCREEN_W 240
#define SCREEN_H 160

typedef struct {
    float x, y;
    float vx, vy;
    u32   color;
    float size;
} Sparkle;

typedef struct {
    bool               active;
    IntroCutsceneStage stage;
    int                stage_timer;
    int                total_timer;
    float              fade_alpha;
    float              flash_alpha;

    // Atores
    float link_x, link_y;
    float zelda_x, zelda_y;
    float vaati_x, vaati_y;
    float king_x, king_y;
    float smith_x, smith_y;

    // Partículas e efeitos
    Sparkle sparkles[24];
    int     chest_open_progress;
    bool    sword_broken;
    bool    zelda_petrified;
} IntroContext;

static IntroContext s_intro;
static const Texture* s_intro_npcs_tex    = NULL;
static const Texture* s_intro_bosses_tex  = NULL;
static const Texture* s_intro_link_tex    = NULL;
static const Texture* s_intro_castle_tex  = NULL;
static const Texture* s_intro_hud_tex     = NULL;
static const Texture* s_intro_enemies_tex = NULL;

void intro_cutscene_set_npcs_texture(const Texture* tex) {
    s_intro_npcs_tex = tex;
}

void intro_cutscene_set_bosses_texture(const Texture* tex) {
    s_intro_bosses_tex = tex;
}

void intro_cutscene_set_link_texture(const Texture* tex) {
    s_intro_link_tex = tex;
}

void intro_cutscene_set_castle_texture(const Texture* tex) {
    s_intro_castle_tex = tex;
}

void intro_cutscene_set_hud_texture(const Texture* tex) {
    s_intro_hud_tex = tex;
}

void intro_cutscene_set_enemies_texture(const Texture* tex) {
    s_intro_enemies_tex = tex;
}

static inline u32 blend_color(u32 dst, u32 src) {
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

static void draw_rect_fast(int x, int y, int w, int h, u32 color) {
    int x1 = (x < 0) ? 0 : x;
    int y1 = (y < 0) ? 0 : y;
    int x2 = (x + w > SCREEN_W) ? SCREEN_W : (x + w);
    int y2 = (y + h > SCREEN_H) ? SCREEN_H : (y + h);

    for (int py = y1; py < y2; py++) {
        for (int px = x1; px < x2; px++) {
            u32 current = hal_video_get_pixel(px, py);
            hal_video_put_pixel(px, py, blend_color(current, color));
        }
    }
}

void intro_cutscene_init(void) {
    memset(&s_intro, 0, sizeof(IntroContext));
    s_intro.active = false;
    s_intro.stage = INTRO_STAGE_FINISHED;
}

void intro_cutscene_start(void) {
    memset(&s_intro, 0, sizeof(IntroContext));
    s_intro.active = true;
    s_intro.stage = INTRO_STAGE_FESTIVAL_PARADE;
    s_intro.stage_timer = 0;
    s_intro.total_timer = 0;
    s_intro.fade_alpha = 1.0f;
    s_intro.flash_alpha = 0.0f;

    s_intro.link_x = 90.0f;
    s_intro.link_y = 110.0f;
    s_intro.zelda_x = 120.0f;
    s_intro.zelda_y = 110.0f;
    s_intro.vaati_x = 120.0f;
    s_intro.vaati_y = -30.0f;
    s_intro.king_x = 120.0f;
    s_intro.king_y = 50.0f;
    s_intro.smith_x = 80.0f;
    s_intro.smith_y = 60.0f;

    for (int i = 0; i < 24; i++) {
        s_intro.sparkles[i].x = (float)(30 + (rand() % 180));
        s_intro.sparkles[i].y = (float)(20 + (rand() % 120));
        s_intro.sparkles[i].vx = ((float)(rand() % 100) - 50.0f) / 60.0f;
        s_intro.sparkles[i].vy = ((float)(rand() % 100) - 50.0f) / 60.0f;
        s_intro.sparkles[i].color = 0x9333EAFF;
        s_intro.sparkles[i].size = 2.0f;
    }

    hal_audio_play_bgm(BGM_PICORI_FESTIVAL);
    printf("[INTRO] Cutscene inicial iniciada: O Festival de Picori & Aparição de Vaati!\n");
}

bool intro_cutscene_is_active(void) {
    return s_intro.active;
}

IntroCutsceneStage intro_cutscene_get_stage(void) {
    return s_intro.stage;
}

void intro_cutscene_skip(void) {
    s_intro.active = false;
    s_intro.stage = INTRO_STAGE_FINISHED;
    hal_audio_stop_music();
    printf("[INTRO] Cutscene pulada pelo jogador.\n");
}

void intro_cutscene_update(void) {
    if (!s_intro.active) return;

    s_intro.stage_timer++;
    s_intro.total_timer++;

    // Botão START ou B para pular cutscene
    if (hal_input_is_pressed(KEY_START) || hal_input_is_pressed(KEY_B)) {
        intro_cutscene_skip();
        return;
    }

    // Gerenciador de Flash
    if (s_intro.flash_alpha > 0.0f) {
        s_intro.flash_alpha -= 0.05f;
        if (s_intro.flash_alpha < 0.0f) s_intro.flash_alpha = 0.0f;
    }

    // Fade-in inicial
    if (s_intro.fade_alpha > 0.0f) {
        s_intro.fade_alpha -= 0.03f;
        if (s_intro.fade_alpha < 0.0f) s_intro.fade_alpha = 0.0f;
    }

    // Atualiza partículas de energia sombria
    for (int i = 0; i < 24; i++) {
        s_intro.sparkles[i].x += s_intro.sparkles[i].vx;
        s_intro.sparkles[i].y += s_intro.sparkles[i].vy;
        if (s_intro.sparkles[i].x < 10.0f || s_intro.sparkles[i].x > 230.0f) s_intro.sparkles[i].vx *= -1.0f;
        if (s_intro.sparkles[i].y < 10.0f || s_intro.sparkles[i].y > 150.0f) s_intro.sparkles[i].vy *= -1.0f;
    }

    switch (s_intro.stage) {
        // --------------------------------------------------------------------
        // 1. FESTIVAL DE PICORI (PARADE)
        // --------------------------------------------------------------------
        case INTRO_STAGE_FESTIVAL_PARADE: {
            if (s_intro.link_y > 85.0f) s_intro.link_y -= 0.3f;
            if (s_intro.zelda_y > 85.0f) s_intro.zelda_y -= 0.3f;

            if (s_intro.stage_timer >= 240 || hal_input_is_pressed(KEY_A)) {
                s_intro.stage = INTRO_STAGE_TOURNAMENT_WIN;
                s_intro.stage_timer = 0;
                s_intro.vaati_y = 35.0f;
                hal_audio_play_sound(SOUND_EZLO_ALERT, 1.0f, 0.8f);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 2. VAATI O VENCEDOR DO TORNEIO DE ESGRIMA
        // --------------------------------------------------------------------
        case INTRO_STAGE_TOURNAMENT_WIN: {
            if (s_intro.stage_timer >= 240 || hal_input_is_pressed(KEY_A)) {
                s_intro.stage = INTRO_STAGE_CHEST_CEREMONY;
                s_intro.stage_timer = 0;
                hal_audio_play_sound(SOUND_TOWN_BELL, 1.0f, 1.0f);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 3. ABERTURA DO BAÚ SELADO & QUEBRA DA PICORI BLADE
        // --------------------------------------------------------------------
        case INTRO_STAGE_CHEST_CEREMONY: {
            if (s_intro.stage_timer == 30) {
                s_intro.chest_open_progress = 1;
                hal_audio_play_sound(SOUND_CHEST_OPEN, 1.0f, 0.8f);
            }
            if (s_intro.stage_timer == 70) {
                s_intro.sword_broken = true;
                s_intro.flash_alpha = 1.0f;
                hal_audio_play_sound(SOUND_WALL_CRUMBLE, 1.0f, 1.2f);
            }
            if (s_intro.stage_timer >= 220 || hal_input_is_pressed(KEY_A)) {
                s_intro.stage = INTRO_STAGE_MONSTERS_RELEASED;
                s_intro.stage_timer = 0;
                hal_audio_play_bgm(BGM_BOSS_BATTLE);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 4. MONSTROS LIBERTADOS
        // --------------------------------------------------------------------
        case INTRO_STAGE_MONSTERS_RELEASED: {
            if (s_intro.stage_timer % 30 == 0) {
                hal_audio_play_sound(SOUND_KEESE_CHIRP, 0.7f, 1.0f);
            }
            if (s_intro.stage_timer >= 220 || hal_input_is_pressed(KEY_A)) {
                s_intro.stage = INTRO_STAGE_ZELDA_CURSED;
                s_intro.stage_timer = 0;
            }
            break;
        }

        // --------------------------------------------------------------------
        // 5. PETRIFICAÇÃO DA PRINCESA ZELDA
        // --------------------------------------------------------------------
        case INTRO_STAGE_ZELDA_CURSED: {
            if (s_intro.stage_timer == 40) {
                s_intro.zelda_petrified = true;
                s_intro.flash_alpha = 1.0f;
                hal_audio_play_sound(SOUND_BOSS_SLAM, 1.0f, 0.9f);
            }
            if (s_intro.stage_timer >= 260 || hal_input_is_pressed(KEY_A)) {
                s_intro.stage = INTRO_STAGE_KING_AUDIENCE;
                s_intro.stage_timer = 0;
                hal_audio_play_bgm(BGM_MINISH_WOODS);
            }
            break;
        }

        // --------------------------------------------------------------------
        // 6. AUDIÊNCIA COM O REI DALTUS & MESTRE SMITH
        // --------------------------------------------------------------------
        case INTRO_STAGE_KING_AUDIENCE: {
            if (s_intro.stage_timer >= 280 || hal_input_is_pressed(KEY_A)) {
                s_intro.stage = INTRO_STAGE_FINISHED;
                s_intro.active = false;
                hal_audio_stop_music();
                printf("[INTRO] Cutscene concluída! Conduzindo Link ao início da jornada.\n");
            }
            break;
        }

        case INTRO_STAGE_FINISHED:
        default:
            s_intro.active = false;
            break;
    }
}

void intro_cutscene_render(void) {
    if (!s_intro.active) return;
    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer) return;

    int W = ctx->render_width;
    int H = ctx->render_height;

    // 1. Cenário de Fundo (Pátio do Castelo ou Sala do Trono)
    if (s_intro.stage == INTRO_STAGE_KING_AUDIENCE) {
        // Sala do Trono Real (Vermelho e Ouro Imperial)
        draw_rect_fast(0, 0, W, H, 0x1E1B18FF);
        // Tapete Imperial Carmesim
        draw_rect_fast((W - 80) / 2, 0, 80, H, 0x7F1D1DFF);
        draw_rect_fast((W - 84) / 2, 0, 2, H, 0xD4AF37FF);
        draw_rect_fast((W + 80) / 2, 0, 2, H, 0xD4AF37FF);

        // Trono Dourado
        draw_rect_fast((W - 32) / 2, 20, 32, 28, 0xD4AF37FF);
        draw_rect_fast((W - 24) / 2, 24, 24, 20, 0xB45309FF);

        // Guardas Reais Flanqueando a Sala do Trono
        if (s_intro_npcs_tex && s_intro_npcs_tex->pixels) {
            texture_draw(s_intro_npcs_tex, 0 * 16, 8 * 16, 16, 16, 44, 44);
            texture_draw(s_intro_npcs_tex, 0 * 16, 8 * 16, 16, 16, 180, 44);
        }

        // Rei Daltus no Trono
        if (s_intro_npcs_tex && s_intro_npcs_tex->pixels) {
            texture_draw(s_intro_npcs_tex, 5 * 16, 0 * 16, 16, 16, (W - 16) / 2, 26);
        } else {
            draw_rect_fast((int)s_intro.king_x - 6, (int)s_intro.king_y, 12, 14, 0xDC2626FF);
            draw_rect_fast((int)s_intro.king_x - 4, (int)s_intro.king_y - 6, 8, 6, 0xFDE047FF); // Coroa
        }

        // Mestre Smith (Ferreiro e avô de Link)
        if (s_intro_npcs_tex && s_intro_npcs_tex->pixels) {
            texture_draw(s_intro_npcs_tex, 0 * 16, 1 * 16, 16, 16, (int)s_intro.smith_x - 8, (int)s_intro.smith_y - 8);
        } else {
            draw_rect_fast((int)s_intro.smith_x - 6, (int)s_intro.smith_y, 12, 16, 0x475569FF);
            draw_rect_fast((int)s_intro.smith_x - 5, (int)s_intro.smith_y - 6, 10, 6, 0xF8FAFCFF); // Barba branca
        }

        // Link em frente ao Trono (virado para cima, ouvindo a ordem do Rei)
        if (s_intro_link_tex && s_intro_link_tex->pixels) {
            texture_draw(s_intro_link_tex, 2 * 32, 0 * 32, 32, 32, (int)s_intro.link_x - 16, (int)s_intro.link_y - 20);
        } else {
            draw_rect_fast((int)s_intro.link_x - 5, (int)s_intro.link_y, 10, 14, 0x16A34AFF);
            draw_rect_fast((int)s_intro.link_x - 4, (int)s_intro.link_y - 6, 8, 6, 0xFBBF24FF);
        }
    } else {
        // Pátio Externo do Castelo de Hyrule
        if (s_intro_castle_tex && s_intro_castle_tex->pixels) {
            // Renderiza o cenário autêntico do pátio (512x384 centrado no pódio)
            texture_draw(s_intro_castle_tex, 136, 80, W, H, 0, 0);
        } else {
            draw_rect_fast(0, 0, W, 45, 0x475569FF); // Muralha
            draw_rect_fast(0, 45, W, H - 45, 0x15803DFF); // Gramado do pátio
            draw_rect_fast((W - 60) / 2, 40, 60, 30, 0x64748BFF);
            draw_rect_fast((W - 56) / 2, 42, 56, 26, 0x94A3B8FF);
        }

        // Guardas Reais Flanqueando o Pódio Sagrado
        if (s_intro_npcs_tex && s_intro_npcs_tex->pixels) {
            texture_draw(s_intro_npcs_tex, 0 * 16, 8 * 16, 16, 16, 68, 50);
            texture_draw(s_intro_npcs_tex, 0 * 16, 8 * 16, 16, 16, 156, 50);
        }

        // O Baú Sagrado e a Lâmina Picori (The Bound Chest & Picori Blade)
        int cx = 104;
        int cy = 46;
        if (s_intro_bosses_tex && s_intro_bosses_tex->pixels) {
            if (!s_intro.sword_broken) {
                // Baú intacto selado pela Picori Blade
                texture_draw(s_intro_bosses_tex, 128, 160, 32, 32, cx, cy);
            } else {
                // Baú violado com a lâmina partida e malícia escapando
                texture_draw(s_intro_bosses_tex, 160, 160, 32, 32, cx, cy);
            }
        } else {
            // Fallback procedural
            if (s_intro.chest_open_progress > 0) {
                draw_rect_fast(cx + 4, cy + 8, 24, 16, 0xB45309FF);
                draw_rect_fast(cx + 2, cy, 28, 8, 0xD97706FF);
            } else {
                draw_rect_fast(cx + 4, cy + 8, 24, 16, 0xB45309FF);
                draw_rect_fast(cx + 2, cy + 6, 28, 6, 0xD97706FF);
            }
            if (s_intro.sword_broken) {
                draw_rect_fast(cx + 10, cy - 6, 3, 8, 0x38BDF8FF);
                draw_rect_fast(cx + 18, cy - 2, 4, 3, 0x38BDF8FF);
            } else {
                draw_rect_fast(cx + 14, cy - 10, 3, 16, 0x38BDF8FF);
                draw_rect_fast(cx + 11, cy + 2, 9, 3, 0xFDE047FF);
            }
        }

        // Vaati (O Campeão do Torneio e Mago das Trevas)
        if (s_intro.stage >= INTRO_STAGE_TOURNAMENT_WIN && s_intro.stage < INTRO_STAGE_KING_AUDIENCE) {
            int vx = (int)s_intro.vaati_x - 16;
            int vy = (int)s_intro.vaati_y - 16;
            if (s_intro_bosses_tex && s_intro_bosses_tex->pixels) {
                int v_src_x = 0;
                if (s_intro.stage == INTRO_STAGE_TOURNAMENT_WIN) v_src_x = 0; // Floating idle
                else if (s_intro.stage == INTRO_STAGE_CHEST_CEREMONY) v_src_x = 32; // Casting spell
                else if (s_intro.stage == INTRO_STAGE_MONSTERS_RELEASED) v_src_x = 64; // Laughing
                else if (s_intro.stage == INTRO_STAGE_ZELDA_CURSED) {
                    v_src_x = s_intro.zelda_petrified ? 96 : 32; // Dark vortex / Spell
                }
                texture_draw(s_intro_bosses_tex, v_src_x, 160, 32, 32, vx, vy);
            } else {
                draw_rect_fast(vx + 9, vy + 7, 14, 18, 0x581C87FF);
                draw_rect_fast(vx + 11, vy + 1, 10, 6, 0xE0E7FFFF);
                draw_rect_fast(vx + 10, vy - 5, 12, 6, 0xC084FCFF);
            }
        }

        // Monstros libertados na quebra do selo
        if (s_intro.stage == INTRO_STAGE_MONSTERS_RELEASED) {
            if (s_intro_enemies_tex && s_intro_enemies_tex->pixels) {
                float mt = (float)s_intro.stage_timer;
                int kframe = (s_intro.stage_timer / 6) % 2;
                int k1_x = 112 - (int)(mt * 0.75f);
                int k1_y = 52 - (int)(sinf(mt * 0.1f) * 14.0f + mt * 0.25f);
                if (k1_x > 0 && k1_y > 0) {
                    texture_draw(s_intro_enemies_tex, kframe * 16, 32, 16, 16, k1_x, k1_y);
                }
                int k2_x = 112 + (int)(mt * 0.85f);
                int k2_y = 48 - (int)(cosf(mt * 0.12f) * 12.0f + mt * 0.2f);
                if (k2_x < W - 16 && k2_y > 0) {
                    texture_draw(s_intro_enemies_tex, kframe * 16, 32, 16, 16, k2_x, k2_y);
                }
                int chu_y = 54 + (int)(mt * 0.5f);
                if (chu_y < 110) {
                    texture_draw(s_intro_enemies_tex, 4 * 16, 16, 16, 16, 112, chu_y);
                }
            }
        }

        // Princesa Zelda
        int zx = (int)s_intro.zelda_x - 8;
        int zy = (int)s_intro.zelda_y - 12;
        if (s_intro_npcs_tex && s_intro_npcs_tex->pixels) {
            int z_col = 0;
            if (s_intro.stage == INTRO_STAGE_FESTIVAL_PARADE) z_col = 0;
            else if (s_intro.stage == INTRO_STAGE_TOURNAMENT_WIN || s_intro.stage == INTRO_STAGE_CHEST_CEREMONY) z_col = 1;
            else if (s_intro.stage == INTRO_STAGE_MONSTERS_RELEASED) z_col = 2;
            else if (s_intro.stage == INTRO_STAGE_ZELDA_CURSED) {
                z_col = s_intro.zelda_petrified ? 3 : 2; // Col 3 é estátua de pedra!
            }
            texture_draw(s_intro_npcs_tex, z_col * 16, 2 * 16, 16, 16, zx, zy);
        } else {
            u32 zelda_color = s_intro.zelda_petrified ? 0x64748BFF : 0xEC4899FF;
            u32 zelda_face  = s_intro.zelda_petrified ? 0x94A3B8FF : 0xFBCFE8FF;
            draw_rect_fast(zx + 3, zy + 2, 10, 14, zelda_color);
            draw_rect_fast(zx + 4, zy - 4, 8, 6, zelda_face);
        }

        // Link
        int lx = (int)s_intro.link_x - 16;
        int ly = (int)s_intro.link_y - 20;
        if (s_intro_link_tex && s_intro_link_tex->pixels) {
            if (s_intro.stage == INTRO_STAGE_ZELDA_CURSED && s_intro.zelda_petrified) {
                // Nocauteado pela explosão da malícia de Vaati
                texture_draw(s_intro_link_tex, 0 * 32, 9 * 32, 32, 32, lx, ly + 6);
            } else {
                // Link em pé
                texture_draw(s_intro_link_tex, 0 * 32, 0 * 32, 32, 32, lx, ly);
            }
        } else {
            draw_rect_fast(lx + 11, ly + 6, 10, 14, 0x16A34AFF);
            draw_rect_fast(lx + 12, ly, 8, 6, 0xFBBF24FF);
        }
    }

    // Partículas de magia de Vaati
    if (s_intro.stage == INTRO_STAGE_MONSTERS_RELEASED || s_intro.stage == INTRO_STAGE_ZELDA_CURSED) {
        for (int i = 0; i < 24; i++) {
            draw_rect_fast((int)s_intro.sparkles[i].x, (int)s_intro.sparkles[i].y, 3, 3, 0xC084FCFF);
        }
    }

    // 2. Caixa de Texto Cinematográfica (Bottom HUD Banner)
    int box_w = W - 16;
    int box_h = 40;
    int box_x = 8;
    int box_y = H - 45;

    draw_rect_fast(box_x - 1, box_y - 1, box_w + 2, box_h + 2, 0xD4AF37FF); // Borda dourada
    draw_rect_fast(box_x, box_y, box_w, box_h, 0x0F172AEE);                 // Fundo escuro translúcido

    const char* l1 = "";
    const char* l2 = "";

    switch (s_intro.stage) {
        case INTRO_STAGE_FESTIVAL_PARADE:
            l1 = "FESTIVAL DE PICORI DE HYRULE";
            l2 = "A Princesa Zelda convida Link\npara o festival secular.";
            break;
        case INTRO_STAGE_TOURNAMENT_WIN:
            l1 = "O TORNEIO DE ESGRIMA";
            l2 = "O campeao misterioso surge:\no feiticeiro sombrio Vaati!";
            break;
        case INTRO_STAGE_CHEST_CEREMONY:
            l1 = "VIOLACAO DO BAU SAGRADO!";
            l2 = "Vaati quebra a Picori Blade e\nabre o selo do Bau Sagrado!";
            break;
        case INTRO_STAGE_MONSTERS_RELEASED:
            l1 = "TREVAS LIBERTADAS NO REINO!";
            l2 = "Monstros ancestrais fogem e\nespalham trevas por Hyrule!";
            break;
        case INTRO_STAGE_ZELDA_CURSED:
            l1 = "A MALDICAO DE VAATI!";
            l2 = "Vaati lanca um raio sombrio:\nZelda vira estatua de pedra!";
            break;
        case INTRO_STAGE_KING_AUDIENCE:
            l1 = "AUDIENCIA COM O REI DALTUS";
            l2 = "O Rei Daltus envia Link\nem busca do povo Minish!";
            break;
        default:
            break;
    }

    font_draw_text(box_x + 8, box_y + 4, l1, 0xFDE047FF, false);
    font_draw_text_multiline(box_x + 8, box_y + 16, box_w - 16, 11, l2, 0xE2E8F0FF, false);

    // Indicador de Pular com START
    font_draw_text(W - 100, 6, "[START] Pular", 0xCBD5E1FF, true);

    // Efeito de Flash na tela
    if (s_intro.flash_alpha > 0.0f) {
        u8 a = (u8)(s_intro.flash_alpha * 255.0f);
        draw_rect_fast(0, 0, W, H, (0xFFFFFF00 | a));
    }

    // Efeito de Fade suave
    if (s_intro.fade_alpha > 0.0f) {
        u8 a = (u8)(s_intro.fade_alpha * 255.0f);
        draw_rect_fast(0, 0, W, H, (0x00000000 | a));
    }
}
