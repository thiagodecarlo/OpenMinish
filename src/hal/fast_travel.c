/*
 * ============================================================================
 * src/hal/fast_travel.c - Sistema de Transporte Rápido: Ocarina of Wind & Zeffa
 * ============================================================================
 */

#include "hal/fast_travel.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef PI_F
#define PI_F 3.14159265358979323846f
#endif

#define MAX_NOTES 12

typedef struct {
    float x;
    float y;
    float vy;
    float phase;
    u32   color;
    bool  active;
} NoteParticle;

typedef struct {
    ZeffaState state;
    int        state_timer;
    float      zeffa_x;
    float      zeffa_y;
    float      target_x;
    float      target_y;
    int        wing_frame;
    int        selected_dest;
    bool       is_airborne;
    WindCrestInfo crests[CREST_COUNT];
    NoteParticle notes[MAX_NOTES];
    int        song_notes_spawn_timer;
} FastTravelManager;

static FastTravelManager s_ft = { 0 };

static inline u32 ft_blend(u32 dst, u32 src) {
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

static void ft_draw_rect(int rx, int ry, int rw, int rh, u32 color) {
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

static void ft_draw_blend_rect(int rx, int ry, int rw, int rh, u32 color) {
    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer) return;

    for (int y = ry; y < ry + rh; y++) {
        if (y < 0 || y >= ctx->render_height) continue;
        for (int x = rx; x < rx + rw; x++) {
            if (x < 0 || x >= ctx->render_width) continue;
            int idx = y * ctx->render_width + x;
            ctx->framebuffer[idx] = ft_blend(ctx->framebuffer[idx], color);
        }
    }
}

void fast_travel_init(void) {
    memset(&s_ft, 0, sizeof(s_ft));
    s_ft.state = ZEFFA_STATE_INACTIVE;
    s_ft.selected_dest = 0;

    // 0: Hyrule Town
    s_ft.crests[CREST_HYRULE_TOWN].id = CREST_HYRULE_TOWN;
    s_ft.crests[CREST_HYRULE_TOWN].name = "Cidade de Hyrule (Town Center)";
    s_ft.crests[CREST_HYRULE_TOWN].world_map_x = 120;
    s_ft.crests[CREST_HYRULE_TOWN].world_map_y = 68;
    s_ft.crests[CREST_HYRULE_TOWN].target_map_id = 1;
    s_ft.crests[CREST_HYRULE_TOWN].spawn_x = 280.0f;
    s_ft.crests[CREST_HYRULE_TOWN].spawn_y = 300.0f;
    s_ft.crests[CREST_HYRULE_TOWN].unlocked = true;

    // 1: Minish Woods
    s_ft.crests[CREST_MINISH_WOODS].id = CREST_MINISH_WOODS;
    s_ft.crests[CREST_MINISH_WOODS].name = "Bosque dos Minish (Minish Woods)";
    s_ft.crests[CREST_MINISH_WOODS].world_map_x = 185;
    s_ft.crests[CREST_MINISH_WOODS].world_map_y = 108;
    s_ft.crests[CREST_MINISH_WOODS].target_map_id = 0;
    s_ft.crests[CREST_MINISH_WOODS].spawn_x = 448.0f;
    s_ft.crests[CREST_MINISH_WOODS].spawn_y = 636.0f;
    s_ft.crests[CREST_MINISH_WOODS].unlocked = true;

    // 2: Mt. Crenel
    s_ft.crests[CREST_MOUNT_CRENEL].id = CREST_MOUNT_CRENEL;
    s_ft.crests[CREST_MOUNT_CRENEL].name = "Base do Monte Crenel (Mt. Crenel)";
    s_ft.crests[CREST_MOUNT_CRENEL].world_map_x = 65;
    s_ft.crests[CREST_MOUNT_CRENEL].world_map_y = 35;
    s_ft.crests[CREST_MOUNT_CRENEL].target_map_id = 6;
    s_ft.crests[CREST_MOUNT_CRENEL].spawn_x = 240.0f;
    s_ft.crests[CREST_MOUNT_CRENEL].spawn_y = 260.0f;
    s_ft.crests[CREST_MOUNT_CRENEL].unlocked = true;

    // 3: Castor Wilds
    s_ft.crests[CREST_CASTOR_WILDS].id = CREST_CASTOR_WILDS;
    s_ft.crests[CREST_CASTOR_WILDS].name = "Pantano de Castor (Castor Wilds)";
    s_ft.crests[CREST_CASTOR_WILDS].world_map_x = 45;
    s_ft.crests[CREST_CASTOR_WILDS].world_map_y = 90;
    s_ft.crests[CREST_CASTOR_WILDS].target_map_id = 10;
    s_ft.crests[CREST_CASTOR_WILDS].spawn_x = 460.0f;
    s_ft.crests[CREST_CASTOR_WILDS].spawn_y = 180.0f;
    s_ft.crests[CREST_CASTOR_WILDS].unlocked = true;

    // 4: Wind Ruins
    s_ft.crests[CREST_WIND_RUINS].id = CREST_WIND_RUINS;
    s_ft.crests[CREST_WIND_RUINS].name = "Ruinas do Vento (Wind Ruins)";
    s_ft.crests[CREST_WIND_RUINS].world_map_x = 55;
    s_ft.crests[CREST_WIND_RUINS].world_map_y = 125;
    s_ft.crests[CREST_WIND_RUINS].target_map_id = 12;
    s_ft.crests[CREST_WIND_RUINS].spawn_x = 300.0f;
    s_ft.crests[CREST_WIND_RUINS].spawn_y = 320.0f;
    s_ft.crests[CREST_WIND_RUINS].unlocked = true;

    // 5: Lake Hylia
    s_ft.crests[CREST_LAKE_HYLIA].id = CREST_LAKE_HYLIA;
    s_ft.crests[CREST_LAKE_HYLIA].name = "Lago Hylia (Lake Hylia)";
    s_ft.crests[CREST_LAKE_HYLIA].world_map_x = 195;
    s_ft.crests[CREST_LAKE_HYLIA].world_map_y = 60;
    s_ft.crests[CREST_LAKE_HYLIA].target_map_id = 4; // South Field margem do lago
    s_ft.crests[CREST_LAKE_HYLIA].spawn_x = 360.0f;
    s_ft.crests[CREST_LAKE_HYLIA].spawn_y = 200.0f;
    s_ft.crests[CREST_LAKE_HYLIA].unlocked = true;
}

void fast_travel_start(float link_x, float link_y) {
    s_ft.state = ZEFFA_STATE_PLAYING_SONG;
    s_ft.state_timer = 0;
    s_ft.target_x = link_x;
    s_ft.target_y = link_y;
    s_ft.is_airborne = false;
    s_ft.wing_frame = 0;

    for (int i = 0; i < MAX_NOTES; i++) s_ft.notes[i].active = false;
    s_ft.song_notes_spawn_timer = 0;

    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.25f);
    printf("[ZEFFA] Link toca a Cancao do Vento! Convocando o grande passaro Zeffa!\n");
}

bool fast_travel_is_active(void) {
    return s_ft.state != ZEFFA_STATE_INACTIVE;
}

bool fast_travel_is_link_airborne(void) {
    return s_ft.is_airborne;
}

void fast_travel_unlock_crest(FastTravelDestination dest) {
    if (dest < CREST_COUNT) {
        s_ft.crests[dest].unlocked = true;
        printf("[ZEFFA] Crista de Vento desbloqueada: %s!\n", s_ft.crests[dest].name);
    }
}

bool fast_travel_is_crest_unlocked(FastTravelDestination dest) {
    if (dest < CREST_COUNT) return s_ft.crests[dest].unlocked;
    return false;
}

u8 fast_travel_get_unlocked_mask(void) {
    u8 mask = 0;
    for (int i = 0; i < CREST_COUNT; i++) {
        if (s_ft.crests[i].unlocked) mask |= (1 << i);
    }
    return mask;
}

void fast_travel_set_unlocked_mask(u8 mask) {
    for (int i = 0; i < CREST_COUNT; i++) {
        s_ft.crests[i].unlocked = (mask & (1 << i)) != 0;
    }
}

bool fast_travel_check_crest_activation(float world_x, float world_y, const char** out_crest_name) {
    for (int i = 0; i < CREST_COUNT; i++) {
        float dx = world_x - s_ft.crests[i].spawn_x;
        float dy = world_y - s_ft.crests[i].spawn_y;
        if (dx * dx + dy * dy < 20.0f * 20.0f) {
            if (!s_ft.crests[i].unlocked) {
                s_ft.crests[i].unlocked = true;
                if (out_crest_name) *out_crest_name = s_ft.crests[i].name;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                return true;
            }
        }
    }
    return false;
}

bool fast_travel_update(float* link_x, float* link_y, int* out_new_map, float* out_new_x, float* out_new_y) {
    if (s_ft.state == ZEFFA_STATE_INACTIVE) return false;

    s_ft.state_timer++;
    s_ft.wing_frame = (s_ft.state_timer / 4) % 2;

    switch (s_ft.state) {
        case ZEFFA_STATE_PLAYING_SONG: {
            // Gera notas musicais flutuantes
            s_ft.song_notes_spawn_timer++;
            if (s_ft.song_notes_spawn_timer % 6 == 0) {
                for (int i = 0; i < MAX_NOTES; i++) {
                    if (!s_ft.notes[i].active) {
                        s_ft.notes[i].active = true;
                        s_ft.notes[i].x = *link_x + (float)((rand() % 16) - 8);
                        s_ft.notes[i].y = *link_y - 4.0f;
                        s_ft.notes[i].vy = -0.8f - (float)(rand() % 10) * 0.05f;
                        s_ft.notes[i].phase = (float)(rand() % 100);
                        u32 colors[4] = { 0x38BDF8FF, 0xFEF08AFF, 0x34D399FF, 0xF472B6FF };
                        s_ft.notes[i].color = colors[rand() % 4];
                        break;
                    }
                }
            }

            // Atualiza notas
            for (int i = 0; i < MAX_NOTES; i++) {
                if (s_ft.notes[i].active) {
                    s_ft.notes[i].y += s_ft.notes[i].vy;
                    s_ft.notes[i].x += sinf(s_ft.notes[i].phase + s_ft.state_timer * 0.15f) * 0.4f;
                    if (s_ft.notes[i].y < *link_y - 48.0f) s_ft.notes[i].active = false;
                }
            }

            // Após 60 frames da melodia, Zeffa surge
            if (s_ft.state_timer >= 60) {
                s_ft.state = ZEFFA_STATE_SWOOP_IN;
                s_ft.state_timer = 0;
                s_ft.zeffa_x = *link_x - 60.0f;
                s_ft.zeffa_y = *link_y - 120.0f;
                hal_audio_play_sound(SOUND_TOWN_BELL, 0.7f, 1.8f); // Som agudo do bater de asas
            }
            break;
        }

        case ZEFFA_STATE_SWOOP_IN: {
            // Zeffa voa em mergulho parabólico até o Link
            float dx = *link_x - s_ft.zeffa_x;
            float dy = (*link_y - 14.0f) - s_ft.zeffa_y;
            s_ft.zeffa_x += dx * 0.12f;
            s_ft.zeffa_y += dy * 0.12f;

            if (fabsf(dx) < 2.5f && fabsf(dy) < 2.5f) {
                // Agarrou Link!
                s_ft.is_airborne = true;
                s_ft.state = ZEFFA_STATE_MAP_SELECT;
                s_ft.state_timer = 0;
                hal_audio_play_sound(SOUND_TEXT_ADVANCE, 1.0f, 1.0f);
            }
            break;
        }

        case ZEFFA_STATE_MAP_SELECT: {
            // Navegação no Mapa de Seleção
            if (hal_input_is_pressed(KEY_LEFT) || hal_input_is_pressed(KEY_UP)) {
                hal_audio_play_sound(SOUND_TEXT_BLIP, 0.8f, 1.0f);
                for (int step = 1; step < CREST_COUNT; step++) {
                    int next = (s_ft.selected_dest - step + CREST_COUNT) % CREST_COUNT;
                    if (s_ft.crests[next].unlocked) {
                        s_ft.selected_dest = next;
                        break;
                    }
                }
            } else if (hal_input_is_pressed(KEY_RIGHT) || hal_input_is_pressed(KEY_DOWN)) {
                hal_audio_play_sound(SOUND_TEXT_BLIP, 0.8f, 1.0f);
                for (int step = 1; step < CREST_COUNT; step++) {
                    int next = (s_ft.selected_dest + step) % CREST_COUNT;
                    if (s_ft.crests[next].unlocked) {
                        s_ft.selected_dest = next;
                        break;
                    }
                }
            }

            // Confirmar voo com [A]
            if (hal_input_is_pressed(KEY_A)) {
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                s_ft.state = ZEFFA_STATE_FLYING_HIGH;
                s_ft.state_timer = 0;
            }
            // Cancelar com [B]
            else if (hal_input_is_pressed(KEY_B)) {
                hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.0f);
                s_ft.is_airborne = false;
                s_ft.state = ZEFFA_STATE_FINISHED;
                s_ft.state_timer = 0;
            }
            break;
        }

        case ZEFFA_STATE_FLYING_HIGH: {
            // Zeffa sobe velozmente cortando o céu
            s_ft.zeffa_x += 3.5f;
            s_ft.zeffa_y -= 4.0f;

            if (s_ft.state_timer >= 35) {
                // Chegou ao ápice: carrega o mapa do destino selecionado
                WindCrestInfo* dest = &s_ft.crests[s_ft.selected_dest];
                if (out_new_map) *out_new_map = dest->target_map_id;
                if (out_new_x)   *out_new_x   = dest->spawn_x;
                if (out_new_y)   *out_new_y   = dest->spawn_y;

                *link_x = dest->spawn_x;
                *link_y = dest->spawn_y;

                s_ft.zeffa_x = dest->spawn_x + 50.0f;
                s_ft.zeffa_y = dest->spawn_y - 120.0f;

                s_ft.state = ZEFFA_STATE_SWOOP_DOWN;
                s_ft.state_timer = 0;
                return true; // Aciona troca de cena no game loop
            }
            break;
        }

        case ZEFFA_STATE_SWOOP_DOWN: {
            WindCrestInfo* dest = &s_ft.crests[s_ft.selected_dest];
            float dx = dest->spawn_x - s_ft.zeffa_x;
            float dy = (dest->spawn_y - 14.0f) - s_ft.zeffa_y;

            s_ft.zeffa_x += dx * 0.12f;
            s_ft.zeffa_y += dy * 0.12f;

            *link_x = s_ft.zeffa_x;
            *link_y = s_ft.zeffa_y + 14.0f;

            if (fabsf(dx) < 2.0f && fabsf(dy) < 2.0f) {
                // Pousou na crista de vento!
                s_ft.is_airborne = false;
                *link_x = dest->spawn_x;
                *link_y = dest->spawn_y;
                hal_audio_play_sound(SOUND_SWORD_HIT, 0.8f, 1.4f); // Pouso firme
                s_ft.state = ZEFFA_STATE_FINISHED;
                s_ft.state_timer = 0;
            }
            break;
        }

        case ZEFFA_STATE_FINISHED: {
            // Zeffa vai embora voando alto
            s_ft.zeffa_y -= 4.0f;
            s_ft.zeffa_x += 2.0f;
            if (s_ft.state_timer >= 30) {
                s_ft.state = ZEFFA_STATE_INACTIVE;
                printf("[ZEFFA] Voo concluido! Link chegou ao destino!\n");
            }
            break;
        }

        default:
            break;
    }

    return false;
}

static void draw_zeffa_bird(int sx, int sy, int wing_up, bool carrying_link) {
    // Cores do grande pássaro real Zeffa
    u32 c_white  = 0xF8FAFCFF; // Penas brancas puras
    u32 c_shade  = 0xCBD5E1FF; // Penas sombreadas
    u32 c_gold   = 0xF59E0BFF; // Crista real e bico dourado
    u32 c_cyan   = 0x38BDF8FF; // Pontas das asas em ciano místico
    u32 c_talon  = 0xD97706FF; // Garras afiadas
    u32 c_eye    = 0xDC2626FF; // Olho rubi

    // Corpo central de Zeffa (16x10)
    ft_draw_rect(sx + 10, sy + 6, 12, 10, c_white);
    ft_draw_rect(sx + 11, sy + 8, 10, 6, c_shade);

    // Cauda em leque branca com pontas ciano
    ft_draw_rect(sx + 14, sy + 16, 4, 6, c_white);
    ft_draw_rect(sx + 13, sy + 21, 6, 2, c_cyan);

    // Cabeça do pássaro
    ft_draw_rect(sx + 12, sy, 8, 7, c_white);
    // Crista real dourada
    ft_draw_rect(sx + 14, sy - 3, 4, 4, c_gold);
    ft_draw_rect(sx + 15, sy - 5, 2, 3, c_gold);
    // Bico aquilino curvado
    ft_draw_rect(sx + 14, sy + 6, 4, 3, c_gold);
    ft_draw_rect(sx + 15, sy + 8, 2, 2, 0xB45309FF);
    // Olhos rubis
    ft_draw_rect(sx + 13, sy + 2, 2, 2, c_eye);
    ft_draw_rect(sx + 17, sy + 2, 2, 2, c_eye);

    // Asas majestosas (Asas para cima vs Asas para baixo)
    if (wing_up) {
        // Asa esquerda para cima
        ft_draw_rect(sx, sy - 4, 11, 8, c_white);
        ft_draw_rect(sx + 1, sy - 6, 8, 4, c_shade);
        ft_draw_rect(sx, sy - 7, 6, 3, c_cyan);

        // Asa direita para cima
        ft_draw_rect(sx + 21, sy - 4, 11, 8, c_white);
        ft_draw_rect(sx + 23, sy - 6, 8, 4, c_shade);
        ft_draw_rect(sx + 26, sy - 7, 6, 3, c_cyan);
    } else {
        // Asa esquerda plana / para baixo
        ft_draw_rect(sx - 2, sy + 4, 13, 8, c_white);
        ft_draw_rect(sx, sy + 8, 10, 4, c_shade);
        ft_draw_rect(sx - 3, sy + 6, 5, 4, c_cyan);

        // Asa direita plana / para baixo
        ft_draw_rect(sx + 21, sy + 4, 13, 8, c_white);
        ft_draw_rect(sx + 22, sy + 8, 10, 4, c_shade);
        ft_draw_rect(sx + 30, sy + 6, 5, 4, c_cyan);
    }

    // Garras douradas
    ft_draw_rect(sx + 11, sy + 15, 3, 4, c_talon);
    ft_draw_rect(sx + 18, sy + 15, 3, 4, c_talon);

    // Se estiver carregando Link no ar, desenha Link pendurado nas garras
    if (carrying_link) {
        int lx = sx + 8;
        int ly = sy + 18;
        // Gorro verde
        ft_draw_rect(lx + 4, ly, 8, 4, 0x22C55EFF);
        // Cabelo loiro e rosto
        ft_draw_rect(lx + 5, ly + 3, 6, 3, 0xFDE047FF);
        // Túnica verde
        ft_draw_rect(lx + 4, ly + 6, 8, 6, 0x22C55EFF);
        // Pernas balançando no vento
        ft_draw_rect(lx + 4, ly + 12, 3, 4, 0xFFFFFFFF);
        ft_draw_rect(lx + 9, ly + 12, 3, 4, 0xFFFFFFFF);
        ft_draw_rect(lx + 4, ly + 15, 3, 2, 0xA16207FF);
        ft_draw_rect(lx + 9, ly + 15, 3, 2, 0xA16207FF);
    }
}

void fast_travel_render(const Camera* cam, float link_x, float link_y) {
    if (s_ft.state == ZEFFA_STATE_INACTIVE) return;

    int ox = (int)cam->x;
    int oy = (int)cam->y;

    // 1. Renderiza notas musicais subindo se estiver tocando canção
    if (s_ft.state == ZEFFA_STATE_PLAYING_SONG) {
        for (int i = 0; i < MAX_NOTES; i++) {
            if (s_ft.notes[i].active) {
                int nx = (int)s_ft.notes[i].x - ox;
                int ny = (int)s_ft.notes[i].y - oy;
                // Desenha nota musical (cabeça + haste)
                ft_draw_rect(nx, ny + 3, 4, 3, s_ft.notes[i].color);
                ft_draw_rect(nx + 3, ny, 1, 4, s_ft.notes[i].color);
                ft_draw_rect(nx + 4, ny, 2, 1, s_ft.notes[i].color);
            }
        }
    }

    // 2. Renderiza o pássaro Zeffa voando
    if (s_ft.state == ZEFFA_STATE_SWOOP_IN ||
        s_ft.state == ZEFFA_STATE_FLYING_HIGH ||
        s_ft.state == ZEFFA_STATE_SWOOP_DOWN ||
        s_ft.state == ZEFFA_STATE_FINISHED) {
        int zx = (int)s_ft.zeffa_x - ox - 16;
        int zy = (int)s_ft.zeffa_y - oy - 8;
        draw_zeffa_bird(zx, zy, s_ft.wing_frame, s_ft.is_airborne);
    }

    // 3. Renderiza a Tela de Seleção de Crista de Vento (World Map)
    if (s_ft.state == ZEFFA_STATE_MAP_SELECT) {
        const HalVideoContext* ctx = hal_video_get_context();
        int sw = ctx ? ctx->render_width : 284;
        int sh = ctx ? ctx->render_height : 160;

        // Overlay escuro com transparência suave
        ft_draw_blend_rect(0, 0, sw, sh, 0x0A0F1ACC);

        // Painel do Mapa Mundi de Hyrule
        int mw = 250;
        int mh = 140;
        int mx = (sw - mw) / 2;
        int my = (sh - mh) / 2;

        // Moldura em madeira nobre e dourado
        ft_draw_rect(mx - 2, my - 2, mw + 4, mh + 4, 0x1E293BFF);
        ft_draw_rect(mx, my, mw, mh, 0x172554FF); // Fundo azul mar profundo de Hyrule
        ft_draw_rect(mx + 2, my + 2, mw - 4, mh - 4, 0x0F172AFF);

        // Contorno das terras de Hyrule (estilizado em verde musgo e montanhas marrons)
        ft_draw_rect(mx + 30, my + 20, 190, 95, 0x14532DFF); // Continente
        ft_draw_rect(mx + 45, my + 25, 60, 40, 0x78350FFF);  // Monte Crenel (Noroeste)
        ft_draw_rect(mx + 35, my + 75, 55, 35, 0x1E3A1EFF);  // Castor Wilds (Oeste)
        ft_draw_rect(mx + 175, my + 50, 40, 45, 0x0369A1FF); // Lago Hylia (Leste)
        ft_draw_rect(mx + 160, my + 95, 55, 20, 0x064E3BFF); // Minish Woods (Sudeste)
        ft_draw_rect(mx + 105, my + 55, 45, 35, 0xD97706FF); // Cidade de Hyrule (Centro)

        // Barra de Título
        ft_draw_rect(mx, my, mw, 18, 0x1E293BFF);
        ft_draw_rect(mx, my + 17, mw, 1, 0x38BDF8FF);
        font_draw_text(mx + 20, my + 5, "MAPA DE HYRULE - VOO DE ZEFFA", 0xBAE6FDFF, true);

        // Renderiza cada Crista de Vento no Mapa
        for (int i = 0; i < CREST_COUNT; i++) {
            WindCrestInfo* c = &s_ft.crests[i];
            int cx = mx + (c->world_map_x * mw) / 240;
            int cy = my + (c->world_map_y * mh) / 160;

            if (c->unlocked) {
                // Crista ativa: anéis ciano pulsantes
                bool is_sel = (s_ft.selected_dest == i);
                u32 col = is_sel ? 0xFDE047FF : 0x38BDF8FF;
                u32 ring = is_sel ? 0xF59E0BFF : 0x0284C7FF;

                ft_draw_rect(cx - 3, cy - 3, 7, 7, ring);
                ft_draw_rect(cx - 2, cy - 2, 5, 5, col);
                ft_draw_rect(cx - 1, cy - 1, 3, 3, 0xFFFFFFFF);

                if (is_sel) {
                    // Cursor em losango animado ao redor da crista selecionada
                    int pulse = (s_ft.state_timer / 4) % 3;
                    ft_draw_rect(cx - 6 - pulse, cy, 3, 1, 0xFDE047FF);
                    ft_draw_rect(cx + 4 + pulse, cy, 3, 1, 0xFDE047FF);
                    ft_draw_rect(cx, cy - 6 - pulse, 1, 3, 0xFDE047FF);
                    ft_draw_rect(cx, cy + 4 + pulse, 1, 3, 0xFDE047FF);
                }
            } else {
                // Crista bloqueada
                ft_draw_rect(cx - 2, cy - 2, 5, 5, 0x475569FF);
            }
        }

        // Rodapé com o nome do destino selecionado
        WindCrestInfo* sel = &s_ft.crests[s_ft.selected_dest];
        ft_draw_rect(mx, my + mh - 22, mw, 22, 0x0F172AE6);
        ft_draw_rect(mx, my + mh - 22, mw, 1, 0xF59E0BFF);

        font_draw_text(mx + 8, my + mh - 18, sel->name, 0xFDE047FF, true);
        font_draw_text(mx + 120, my + mh - 9, "[A] VOAR   [B] CANCELAR", 0x94A3B8FF, true);
    }
}
