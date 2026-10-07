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

typedef struct {
    const char* name;
    const char* desc1;
    const char* desc2;
    FigurineCategory cat;
} FigDesc;

static const FigDesc s_canon_figs[36] = {
    { "001: Link (O Heroi)",       "Jovem aprendiz de ferreiro que empunha", "a lendaria White Sword e salva Hyrule.", FIG_CAT_HERO },
    { "002: Ezlo (Gorro Picori)",  "Antigo sabio dos Minish transformado",   "em gorro falante pela maldicao de Vaati.", FIG_CAT_HERO },
    { "003: Princesa Zelda",       "Bondosa princesa que carrega o misterioso", "Poder da Luz sagrado dos Minish.", FIG_CAT_HERO },
    { "004: Rei Daltus",           "Soberano de Hyrule e zelador das velhas", "tradicoes do Festival dos Picori.", FIG_CAT_HERO },
    { "005: Mestre Smith",         "Avo de Link e o mais renomado ferreiro", "de laminas de todo o reino de Hyrule.", FIG_CAT_TOWN },
    { "006: Swiftblade",           "Mestre espadachim de Hyrule Town que",   "ensina o lendario Spin Attack a Link.", FIG_CAT_TOWN },
    { "007: Grayblade",            "Mestre solitario de Mt. Crenel capaz",   "de ensinar o poderoso Roll Attack.", FIG_CAT_TOWN },
    { "008: Grimblade",            "Mestre oculto nos jardins do castelo que", "revela os segredos do Sword Beam.", FIG_CAT_TOWN },
    { "009: Waveblade",            "Mestre do Lago Hylia que aprimora o",    "temivel Peril Beam quando a vida mingua.", FIG_CAT_TOWN },
    { "010: Coveiro Dampe",        "Guardiao sombrio do cemiterio real que", "cuida dos mausoleus dos nobres ancestrais.", FIG_CAT_TOWN },
    { "011: Rei Gustaf",           "Antigo monarca de Hyrule sepultado no",  "vale das criptas reais de outrora.", FIG_CAT_HERO },
    { "012: Anciao Librari",       "Sabio arquivista Minish que habita o",   "topo da biblioteca de Hyrule Town.", FIG_CAT_MINISH },
    { "013: Minish da Floresta",   "Pequenino habitante de Deepwood Shrine", "que adora nozes e sementes silvestres.", FIG_CAT_MINISH },
    { "014: Minish da Cidade",     "Artesao engenhoso que ajuda os humanos", "fazendo sapatos e assando paes a noite.", FIG_CAT_MINISH },
    { "015: Mountain Minish",      "Ferreiro corajoso que forja minerais e", "laminas nas minas de Mt. Crenel.", FIG_CAT_MINISH },
    { "016: Octorok Vermelho",     "Criatura tentacular que cospe pedregulhos", "polidos em qualquer viajante incauto.", FIG_CAT_ENEMY },
    { "017: Morcego Keese",        "Voador cavernoso que se esconde em tetos", "e ataca em mergulhos rasantes rapidos.", FIG_CAT_ENEMY },
    { "018: Green ChuChu",         "Gosma gelatinosa verde que surge do chao", "ao menor sinal de vibracao no solo.", FIG_CAT_ENEMY },
    { "019: Spiny Beetle",         "Inseto casca-dura que se disfarca de",   "arbusto verdejante nos descampados.", FIG_CAT_ENEMY },
    { "020: Guarda Moblin",        "Guerreiro suino que patrulha passagens", "armado com longas lancas afiadas.", FIG_CAT_ENEMY },
    { "021: Darknut (Cavaleiro)",  "Guardiao de armadura pesada com escudo", "e espada capaz de desferir cortes brutais.", FIG_CAT_ENEMY },
    { "022: Big Green ChuChu",     "Guardiao colossal de Deepwood Shrine",   "derrotado com o poder do Gust Jar.", FIG_CAT_BOSS },
    { "023: Gleerok (Lava)",       "Dragao de fogo milenar adormecido na",   "cratera de lava da Cave of Flames.", FIG_CAT_BOSS },
    { "024: Mazaal (Fortaleza)",   "Colosso mecanico da Fortaleza dos Ventos", "com punhos voadores e olhos opticos.", FIG_CAT_BOSS },
    { "025: Big Octorok (Gelo)",   "Polvo gigante congelado no Templo das",  "Gotas que dispara rajadas glaciais.", FIG_CAT_BOSS },
    { "026: Gyorg Pair (Ceu)",     "Mantarrayas aladas gigantescas que",     "sulcam as nuvens do Palacio dos Ventos.", FIG_CAT_BOSS },
    { "027: Feiticeiro Vaati",     "Antigo aprendiz de Ezlo corrompido pela", "ambicao e sede insaciavel de poder.", FIG_CAT_BOSS },
    { "028: Vaati Transfigured",   "Forma monstruosa esferica com quatro",   "olhos orbitais e garras de trevas.", FIG_CAT_BOSS },
    { "029: Vaati's Wrath",        "Encarnacao final do senhor do caos com", "bracos colossais e raios demoniacos.", FIG_CAT_BOSS },
    { "030: Mestre Carlov",        "O genial criador desta propria maquina", "que esculpe miniaturas com sua alma!", FIG_CAT_TOWN },
    { "031: Malon da Fazenda",     "Garota gentil do Lon Lon Ranch que vende", "leite fresco e cuida dos poneis.", FIG_CAT_TOWN },
    { "032: Talon da Fazenda",     "Pai dorminhoco de Malon que frequentemente", "perde as chaves da porteira do rancho.", FIG_CAT_TOWN },
    { "033: Epona (Ponei)",        "A fiel e graciosa potranca do rancho",   "que adora cenouras e passeios campestres.", FIG_CAT_TOWN },
    { "034: Stockwell (Lojista)",  "Comerciante astuto de Hyrule Town que",  "vende a cobicada carteira e bolsas.", FIG_CAT_TOWN },
    { "035: Anju das Galinhas",    "Criadora atenciosa da praca do vilarejo", "que premia quem resgatar seus Cuccos!", FIG_CAT_TOWN },
    { "036: Vovozinha Minish",     "Acolhedora matriarca que prepara chas",  "revigorantes sob o assoalho das casas.", FIG_CAT_MINISH }
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
            s_gallery.figurines[i].name = s_canon_figs[i].name;
            s_gallery.figurines[i].desc_line1 = s_canon_figs[i].desc1;
            s_gallery.figurines[i].desc_line2 = s_canon_figs[i].desc2;
            s_gallery.figurines[i].category = s_canon_figs[i].cat;
        } else {
            static char s_gen_names[100][32];
            snprintf(s_gen_names[i - 36], 32, "%03d: Trofeu de Hyrule %d", i + 1, i + 1);
            s_gallery.figurines[i].name = s_gen_names[i - 36];
            s_gallery.figurines[i].desc_line1 = "Miniatura esculpida com maestria pelo";
            s_gallery.figurines[i].desc_line2 = "lendario artesao Carlov em sua arvore!";
            s_gallery.figurines[i].category = (i % 5);
        }
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

static inline void put_pixel_safe(int x, int y, u32 color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        u32 cur = hal_video_get_pixel(x, y);
        hal_video_put_pixel(x, y, blend_colors(cur, color));
    }
}

static void draw_mysterious_shell_icon(int x, int y) {
    draw_filled_rect(x + 2, y,     5, 2, 0x38BDF8FF);
    draw_filled_rect(x + 1, y + 2, 7, 2, 0x7DD3FCFF);
    draw_filled_rect(x,     y + 4, 9, 3, 0xBAE6FDFF);
    draw_filled_rect(x + 1, y + 7, 7, 2, 0x0284C7FF);
    draw_filled_rect(x + 3, y + 9, 3, 1, 0x0369A1FF);
    put_pixel_safe(x + 3, y + 3, 0x0284C7FF);
    put_pixel_safe(x + 4, y + 4, 0x0284C7FF);
    put_pixel_safe(x + 5, y + 3, 0x0369A1FF);
    put_pixel_safe(x + 3, y + 5, 0xFFFFFFFF);
}

static void render_figurine_trophy(int px, int py, int fig_id, float rot_angle, bool is_silhouette) {
    // 1. Cone de iluminação mística (Spotlight do teto)
    for (int y = 20; y < py; y += 4) {
        int w = 24 + ((y - 20) * 36) / (py - 20);
        draw_filled_rect(px - w / 2, y, w, 4, 0x38BDF80C);
    }

    // 2. Pedestal de Mármore e Ouro
    draw_filled_rect(px - 32, py + 19, 64, 5, 0x05100788); // Sombra de contato
    draw_filled_rect(px - 30, py + 15, 60, 4, C_GACHA_GOLD); // Anel inferior dourado
    draw_filled_rect(px - 26, py + 3,  52, 12, C_PEDESTAL_STONE); // Coluna mármore
    draw_filled_rect(px - 24, py + 5,  48, 2,  0x94A3B8FF); // Destaque mármore
    draw_filled_rect(px - 28, py,      56, 4,  C_PEDESTAL_DARK); // Tampo superior
    draw_filled_rect(px - 26, py,      52, 1,  0xCBD5E1FF);

    // Efeito de rotação 3D sutil (offset horizontal e inclinação)
    int rox = (int)(sinf(rot_angle) * 7.0f);
    int ty = py - 34;

    if (is_silhouette) {
        // Silhueta misteriosa negra com contorno sutil
        draw_filled_rect(px - 12 + rox, ty, 24, 32, 0x1E1B18FF);
        draw_filled_rect(px - 10 + rox, ty - 2, 20, 2, 0x44403CFF);
        draw_filled_rect(px - 12 + rox, ty, 2, 32, 0x44403CFF);
        font_draw_text(px - 4 + rox, ty + 10, "?", 0xF59E0BFF, true);
        return;
    }

    // 3. Estatuetas Específicas / Canônicas
    if (fig_id == 0) {
        // --- LINK (O HERÓI) ---
        draw_filled_rect(px - 5 + rox, ty - 6, 10, 6, 0x16A34AFF);
        draw_filled_rect(px + 1 + rox, ty - 9, 6, 4, 0x15803DFF);
        draw_filled_rect(px - 4 + rox, ty, 8, 7, 0xFED7AAFF);
        draw_filled_rect(px - 4 + rox, ty, 8, 2, 0xFACC15FF);
        put_pixel_safe(px - 2 + rox, ty + 2, 0x0284C7FF);
        put_pixel_safe(px + 1 + rox, ty + 2, 0x0284C7FF);
        draw_filled_rect(px - 6 + rox, ty + 7, 12, 14, 0x16A34AFF);
        draw_filled_rect(px - 6 + rox, ty + 13, 12, 2, 0xF8FAFCFF);
        put_pixel_safe(px - 1 + rox, ty + 13, 0xF59E0BFF);
        draw_filled_rect(px - 4 + rox, ty + 21, 3, 9, 0x78350FFF);
        draw_filled_rect(px + 1 + rox, ty + 21, 3, 9, 0x78350FFF);
        draw_filled_rect(px - 11 + rox, ty + 8, 5, 11, 0x2563EBFF);
        draw_filled_rect(px - 10 + rox, ty + 11, 3, 5, 0xEF4444FF);
        draw_filled_rect(px + 7 + rox, ty - 4, 2, 14, 0xCBD5E1FF);
        put_pixel_safe(px + 7 + rox, ty - 5, 0xFFFFFFFF);
        draw_filled_rect(px + 6 + rox, ty + 10, 4, 2, 0xF59E0BFF);
    } else if (fig_id == 1) {
        // --- EZLO (GORRO PICORI) ---
        draw_filled_rect(px - 12, ty + 24, 24, 4, 0x78350FFF);
        draw_filled_rect(px - 8 + rox, ty + 6, 16, 16, 0x059669FF);
        draw_filled_rect(px - 9 + rox, ty + 8, 3, 10, 0x047857FF);
        draw_filled_rect(px - 7 + rox, ty - 2, 14, 10, 0x059669FF);
        draw_filled_rect(px - 3 + rox, ty - 7, 6, 5, 0xDC2626FF);
        draw_filled_rect(px - 6 + rox, ty, 5, 5, 0xFFFFFFFF);
        draw_filled_rect(px + 1 + rox, ty, 5, 5, 0xFFFFFFFF);
        put_pixel_safe(px - 4 + rox, ty + 2, 0x18181BFF);
        put_pixel_safe(px + 3 + rox, ty + 2, 0x18181BFF);
        draw_filled_rect(px - 2 + rox, ty + 5, 5, 4, 0xEA580CFF);
        draw_filled_rect(px - 1 + rox, ty + 8, 3, 3, 0xD97706FF);
    } else if (fig_id == 2) {
        // --- PRINCESA ZELDA ---
        draw_filled_rect(px - 5 + rox, ty - 7, 10, 2, 0xF59E0BFF);
        put_pixel_safe(px + rox, ty - 8, 0x38BDF8FF);
        draw_filled_rect(px - 6 + rox, ty - 5, 12, 18, 0xFDE047FF);
        draw_filled_rect(px - 4 + rox, ty - 4, 8, 7, 0xFED7AAFF);
        put_pixel_safe(px - 2 + rox, ty - 1, 0x1D4ED8FF);
        put_pixel_safe(px + 1 + rox, ty - 1, 0x1D4ED8FF);
        draw_filled_rect(px - 7 + rox, ty + 4, 14, 24, 0xF472B6FF);
        draw_filled_rect(px - 3 + rox, ty + 6, 6, 22, 0xFFFFFFFF);
        draw_filled_rect(px - 2 + rox, ty + 12, 4, 3, 0xF59E0BFF);
    } else if (fig_id == 3) {
        // --- REI DALTUS ---
        draw_filled_rect(px - 6 + rox, ty - 7, 12, 3, 0xF59E0BFF);
        put_pixel_safe(px - 5 + rox, ty - 9, 0xF59E0BFF);
        put_pixel_safe(px + rox,     ty - 9, 0xF59E0BFF);
        put_pixel_safe(px + 4 + rox, ty - 9, 0xF59E0BFF);
        draw_filled_rect(px - 5 + rox, ty - 4, 10, 6, 0xFED7AAFF);
        draw_filled_rect(px - 6 + rox, ty + 2, 12, 8, 0xF1F5F9FF);
        draw_filled_rect(px - 8 + rox, ty + 6, 16, 22, 0xDC2626FF);
        draw_filled_rect(px - 8 + rox, ty + 6, 16, 3, 0xFFFFFFFF);
        draw_filled_rect(px + 8 + rox, ty + 2, 2, 22, 0xF59E0BFF);
    } else if (fig_id == 15) {
        // --- OCTOROK VERMELHO ---
        draw_filled_rect(px - 8 + rox, ty + 6, 16, 14, 0xEF4444FF);
        draw_filled_rect(px - 6 + rox, ty + 4, 12, 4, 0xF87171FF);
        draw_filled_rect(px - 4 + rox, ty + 12, 8, 7, 0xFACC15FF);
        draw_filled_rect(px - 2 + rox, ty + 14, 4, 3, 0x78350FFF);
        put_pixel_safe(px - 5 + rox, ty + 9, 0x18181BFF);
        put_pixel_safe(px + 4 + rox, ty + 9, 0x18181BFF);
    } else if (fig_id == 17) {
        // --- GREEN CHUCHU ---
        draw_filled_rect(px - 7 + rox, ty + 8, 14, 16, 0x22C55EFF);
        draw_filled_rect(px - 5 + rox, ty + 6, 10, 4, 0x4ADE80FF);
        draw_filled_rect(px - 3 + rox, ty + 6, 4, 2, 0xFFFFFFFF);
        draw_filled_rect(px - 5 + rox, ty + 12, 3, 3, 0xFFFFFFFF);
        draw_filled_rect(px + 2 + rox, ty + 12, 3, 3, 0xFFFFFFFF);
        put_pixel_safe(px - 4 + rox, ty + 13, 0x18181BFF);
        put_pixel_safe(px + 3 + rox, ty + 13, 0x18181BFF);
    } else if (fig_id == 21) {
        // --- BIG GREEN CHUCHU (CHEFE) ---
        draw_filled_rect(px - 14 + rox, ty - 2, 28, 28, 0x16A34AFF);
        draw_filled_rect(px - 11 + rox, ty - 4, 22, 6, 0x22C55EFF);
        draw_filled_rect(px - 6 + rox, ty + 6, 12, 12, 0x84CC16FF);
        draw_filled_rect(px - 6 + rox, ty - 10, 12, 4, 0xF59E0BFF);
        put_pixel_safe(px - 5 + rox, ty - 12, 0xF59E0BFF);
        put_pixel_safe(px + rox,     ty - 12, 0xF59E0BFF);
        put_pixel_safe(px + 4 + rox, ty - 12, 0xF59E0BFF);
    } else if (fig_id == 22) {
        // --- GLEEROK (DRAGÃO DE FOGO) ---
        draw_filled_rect(px - 12 + rox, ty + 2, 24, 20, 0xB91C1CFF);
        draw_filled_rect(px - 14 + rox, ty + 16, 28, 8, 0xF97316FF);
        draw_filled_rect(px - 11 + rox, ty - 6, 4, 8, 0x94A3B8FF);
        draw_filled_rect(px + 7 + rox,  ty - 6, 4, 8, 0x94A3B8FF);
        draw_filled_rect(px - 6 + rox, ty + 8, 4, 3, 0xFDE047FF);
        draw_filled_rect(px + 2 + rox, ty + 8, 4, 3, 0xFDE047FF);
        put_pixel_safe(px - 4 + rox, ty + 9, 0xDC2626FF);
        put_pixel_safe(px + 3 + rox, ty + 9, 0xDC2626FF);
    } else if (fig_id == 26) {
        // --- FEITICEIRO VAATI ---
        draw_filled_rect(px - 7 + rox, ty - 8, 14, 8, 0x8B5CF6FF);
        draw_filled_rect(px - 1 + rox, ty - 13, 3, 5, 0x7C3AEDFF);
        put_pixel_safe(px + rox, ty - 6, 0xEF4444FF);
        draw_filled_rect(px - 5 + rox, ty, 10, 6, 0xE9D5FFFF);
        draw_filled_rect(px - 2 + rox, ty + 2, 4, 3, 0xDC2626FF);
        put_pixel_safe(px + rox, ty + 3, 0xFDE047FF);
        draw_filled_rect(px - 9 + rox, ty + 6, 18, 22, 0x3B0764FF);
        draw_filled_rect(px - 9 + rox, ty + 24, 18, 4, 0x991B1BFF);
    } else {
        // --- CATEGORIAS GENÉRICAS ESTILIZADAS ---
        FigurineEntry* f = &s_gallery.figurines[fig_id];
        u32 c_main = f->primary_color;
        u32 c_acc  = f->accent_color;

        draw_filled_rect(px - 8 + rox, ty + 2, 16, 24, c_main);
        draw_filled_rect(px - 6 + rox, ty - 4, 12, 8, c_acc);
        draw_filled_rect(px - 3 + rox, ty, 6, 3, 0xFFFFFFFF);
        draw_filled_rect(px - 10 + rox, ty + 20, 20, 2, C_GACHA_GOLD);
    }
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

    // Ícone de concha misteriosa no topo
    draw_mysterious_shell_icon(152, 4);
    char shells_txt[32];
    snprintf(shells_txt, sizeof(shells_txt), "CONCHAS: %d", s_gallery.shells_owned);
    font_draw_text(166, 5, shells_txt, 0x38BDF8FF, true);

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

        // Se estiver dispensando, anima cápsula rolando na calha
        if (s_gallery.mode == GALLERY_MODE_DISPENSING) {
            int drop_y = my + 10 + (60 - s_gallery.dispense_timer) / 2;
            if (drop_y > my + 32) drop_y = my + 32;
            draw_filled_rect(mx - 5, drop_y, 10, 5, C_CAPSULE_TOP);
            draw_filled_rect(mx - 5, drop_y + 5, 10, 5, C_CAPSULE_BOT);
        }

        // Painel de Apostas & Probabilidades (Direita)
        int px = 135;
        int py = 35;
        draw_filled_rect(px, py, 110, 95, 0x292524FF);
        draw_filled_rect(px, py, 110, 95, 0x57534EFF);

        font_draw_text(px + 8, py + 8, "APOSTA DE CONCHAS", 0xFDE047FF, true);

        // Ícone da concha no painel de aposta
        draw_mysterious_shell_icon(px + 8, py + 25);
        char bet_txt[32];
        snprintf(bet_txt, sizeof(bet_txt), "[ %02d ]", s_gallery.shells_bet);
        font_draw_text(px + 22, py + 26, bet_txt, 0xFFFFFFFF, true);

        char chance_txt[32];
        snprintf(chance_txt, sizeof(chance_txt), "CHANCE: %.1f%%", s_gallery.win_chance_percent);
        u32 ch_col = (s_gallery.win_chance_percent >= 100.0f) ? 0x4ADE80FF : (s_gallery.win_chance_percent >= 50.0f ? 0xFDE047FF : 0xF87171FF);
        font_draw_text(px + 8, py + 44, chance_txt, ch_col, true);

        char col_txt[32];
        snprintf(col_txt, sizeof(col_txt), "TOTAL: %d/%d", s_gallery.unlocked_count, TOTAL_FIGURINES);
        font_draw_text(px + 8, py + 62, col_txt, 0x38BDF8FF, true);

        // Instruções no rodapé
        draw_filled_rect(0, SCREEN_H - 18, SCREEN_W, 18, 0x0C0A09EE);
        font_draw_text(6, SCREEN_H - 14, "[A] GIRAR | [+/-10] ESQ/DIR | [+/-1] CIMA/BAIXO | [START] GALERIA", 0xA8A29EFF, false);
    }

    // ==========================================
    // MODO REVELAÇÃO DA ESTATUETA
    // ==========================================
    else if (s_gallery.mode == GALLERY_MODE_REVEAL) {
        int fig_idx = s_gallery.current_won_id;
        FigurineEntry* fig = &s_gallery.figurines[fig_idx];

        // Pedestal e Estatueta 3D Renderizada
        render_figurine_trophy(68, 115, fig_idx, s_gallery.trophy_rot_angle, false);

        // Brilho de estrelas orbitais se for nova estatueta
        if (!s_gallery.is_duplicate) {
            for (int s = 0; s < 4; s++) {
                float a = s_gallery.trophy_rot_angle * 1.5f + (float)s * 1.57f;
                int sx = 68 + (int)(cosf(a) * 22.0f);
                int sy = 90 + (int)(sinf(a) * 16.0f);
                put_pixel_safe(sx, sy, 0xFDE047FF);
                put_pixel_safe(sx + 1, sy, 0xFFFFFFFF);
            }
        }

        // Painel de Lore e Nome
        int lx = 118;
        int ly = 30;
        draw_filled_rect(lx, ly, 130, 105, 0x292524FF);

        const char* status = s_gallery.is_duplicate ? "[REPETIDA]" : "[NOVA ESTATUETA!]";
        u32 st_col = s_gallery.is_duplicate ? 0x94A3B8FF : 0x4ADE80FF;
        font_draw_text(lx + 8, ly + 6, status, st_col, true);
        font_draw_text(lx + 8, ly + 22, fig->name, 0xFDE047FF, true);

        // Categoria da estatueta
        const char* cat_names[5] = { "HEROI", "VILAREJO", "MINISH", "INIMIGO", "CHEFE" };
        char cat_buf[32];
        snprintf(cat_buf, sizeof(cat_buf), "CAT: [%s]", cat_names[fig->category % 5]);
        font_draw_text(lx + 8, ly + 38, cat_buf, 0x38BDF8FF, true);

        font_draw_text(lx + 8, ly + 54, fig->desc_line1, 0xE7E5E4FF, false);
        font_draw_text(lx + 8, ly + 68, fig->desc_line2, 0xE7E5E4FF, false);

        font_draw_text(lx + 8, ly + 88, "[A] CONTINUAR", 0x38BDF8FF, true);
    }

    // ==========================================
    // MODO INSPETOR DE GALERIA (BROWSER)
    // ==========================================
    else if (s_gallery.mode == GALLERY_MODE_INSPECTOR) {
        int fig_idx = s_gallery.selected_id;
        FigurineEntry* fig = &s_gallery.figurines[fig_idx];

        // Pedestal e Estatueta 3D Renderizada
        render_figurine_trophy(68, 115, fig_idx, s_gallery.trophy_rot_angle, !fig->unlocked);

        // Painel de Detalhes
        int lx = 118;
        int ly = 30;
        draw_filled_rect(lx, ly, 130, 105, 0x292524FF);

        if (fig->unlocked) {
            font_draw_text(lx + 8, ly + 6, fig->name, 0xFDE047FF, true);

            const char* cat_names[5] = { "HEROI", "VILAREJO", "MINISH", "INIMIGO", "CHEFE" };
            char cat_buf[32];
            snprintf(cat_buf, sizeof(cat_buf), "CAT: [%s]", cat_names[fig->category % 5]);
            font_draw_text(lx + 8, ly + 22, cat_buf, 0x38BDF8FF, true);

            font_draw_text(lx + 8, ly + 40, fig->desc_line1, 0xE7E5E4FF, false);
            font_draw_text(lx + 8, ly + 54, fig->desc_line2, 0xE7E5E4FF, false);
            font_draw_text(lx + 8, ly + 84, "STATUS: COLETADA", 0x4ADE80FF, true);
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
