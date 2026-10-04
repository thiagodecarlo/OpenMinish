#include "hal/map.h"
#include "hal/video.h"
#include "hal/audio.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Localizador de arquivos de assets binarios (compativel com execucao de root ou build/)
 */
static FILE* open_binary_asset(const char* rel_path) {
    if (!rel_path) return NULL;
    FILE* f = fopen(rel_path, "rb");
    if (f) return f;

    char fallback[512];
    snprintf(fallback, sizeof(fallback), "../%s", rel_path);
    f = fopen(fallback, "rb");
    if (f) return f;

    char* base = SDL_GetBasePath();
    if (base) {
        snprintf(fallback, sizeof(fallback), "%s%s", base, rel_path);
        f = fopen(fallback, "rb");
        if (f) {
            SDL_free(base);
            return f;
        }
        snprintf(fallback, sizeof(fallback), "%s../%s", base, rel_path);
        f = fopen(fallback, "rb");
        if (f) {
            SDL_free(base);
            return f;
        }
        SDL_free(base);
    }
    return NULL;
}

/*
 * ============================================================================
 * src/hal/map.c - Implementação da Engine de Mapas, Tilemaps e Câmera Widescreen
 * ============================================================================
 */

// Paletas de cores em estilo Minish Cap (RGBA8888)
#define C_GRASS_BASE    0x42963BFF // Verde base de Hyrule
#define C_GRASS_DARK    0x2E6B28FF // Sombra da grama
#define C_GRASS_LIGHT   0x5CB84DFF // Brilho da folhagem
#define C_DIRT_BASE     0xD6A868FF // Terra batida / Cascalho
#define C_DIRT_DARK     0xA87D43FF // Borda de terra
#define C_DIRT_PEBBLE   0x78562BFF // Pedregulhos
#define C_WATER_DEEP    0x2563B8FF // Água profunda
#define C_WATER_SHALLOW 0x4896E8FF // Água rasa
#define C_WATER_WHITE   0xBDE0FFFF // Espuma / reflexo
#define C_STONE_FACE    0x78808CFF // Pedra cinza
#define C_STONE_DARK    0x4D525CFF // Argamassa escura
#define C_STONE_LIGHT   0xA1AAB8FF // Borda de pedra
#define C_BUSH_MAIN     0x2D7A32FF // Folhas do arbusto
#define C_BUSH_DARK     0x194C1DFF // Sombra do arbusto
#define C_BUSH_LIGHT    0x48B04FFF // Topo iluminado
#define C_CHEST_WOOD    0x964E19FF // Madeira do baú
#define C_CHEST_GOLD    0xE8B823FF // Fecho dourado
#define C_WOOD_DARK     0x522B0EFF // Detalhe da cerca
#define C_FLOWER_RED    0xE62A2AFF // Pétala vermelha
#define C_FLOWER_YEL    0xFADC32FF // Pétala amarela

// Paleta de Cores da Cidade de Hyrule (Hyrule Town Hub)
#define C_COBBLE_LIGHT  0xD4C5A9FF // Calçamento claro de pedras polidas
#define C_COBBLE_DARK   0x8A7B66FF // Argamassa e sombra do rejunte
#define C_COBBLE_MID    0xB4A58DFF // Tom médio da pedra de calçamento
#define C_TOWN_WALL     0xF3ECE0FF // Alvenaria e reboco claro
#define C_TOWN_BEAM     0x6E401CFF // Vigas de madeira enxaimel
#define C_ROOF_RED      0xC0392BFF // Telhas vermelhas de terracota
#define C_ROOF_RED_DK   0x8A1A12FF // Sombra da telha vermelha
#define C_ROOF_RED_LT   0xE74C3CFF // Brilho da telha vermelha
#define C_ROOF_BLUE     0x2471A3FF // Telhas azuis da loja do Stockwell
#define C_ROOF_BLUE_DK  0x1A5276FF // Sombra da telha azul
#define C_ROOF_BLUE_LT  0x5499C7FF // Brilho da telha azul
#define C_DOOR_WOOD     0x87491CFF // Madeira da porta
#define C_DOOR_DK       0x4A240BFF // Arco da porta
#define C_DOOR_GOLD     0xF1C40FFF // Maçaneta dourada
#define C_WINDOW_GLASS  0x7FB3D5FF // Vidro reflexivo da janela
#define C_WINDOW_FRAME  0x4A240BFF // Moldura da janela
#define C_FOUNTAIN_MARB 0xEAEDEDFF // Mármore polido da fonte
#define C_FOUNTAIN_DK   0xA6ACAFFF // Chanfro e sombra do mármore
#define C_STALL_RED     0xCB4335FF // Toldo listrado vermelho
#define C_STALL_WHITE   0xFDFEFEFF // Toldo listrado branco
#define C_BARREL_WOOD   0x935116FF // Madeira do barril
#define C_BARREL_BAND   0x424949FF // Cinta de ferro do barril

static int s_water_anim_frame = 0;
static int s_water_timer = 0;

static inline void draw_tile_pixel(int sx, int sy, u32 color) {
    hal_video_put_pixel(sx, sy, color);
}

// ----------------------------------------------------------------------------
// RENDERIZADOR PROCEDURAL DE METATILES 16x16 (ESTILO MINISH CAP)
// ----------------------------------------------------------------------------
static void render_metatile(int sx, int sy, TileType type) {
    switch (type) {
        case TILE_GRASS:
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = C_GRASS_BASE;
                    // Lâminas sutis de grama pontilhadas
                    if ((x == 3 && y == 4) || (x == 11 && y == 9) || (x == 7 && y == 13)) c = C_GRASS_LIGHT;
                    else if ((x == 4 && y == 5) || (x == 12 && y == 10) || (x == 8 && y == 14)) c = C_GRASS_DARK;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_DIRT_PATH:
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = C_DIRT_BASE;
                    if ((x == 2 && y == 3) || (x == 13 && y == 11)) c = C_DIRT_PEBBLE;
                    else if ((x + y) % 7 == 0) c = C_DIRT_DARK;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_WATER: {
            int offset = s_water_anim_frame;
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = C_WATER_DEEP;
                    if ((x + y + offset) % 6 == 0) c = C_WATER_SHALLOW;
                    else if ((x + y + offset) % 12 == 0) c = C_WATER_WHITE;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;
        }

        case TILE_STONE_WALL:
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = C_STONE_FACE;
                    if (y == 0 || y == 7 || y == 8 || y == 15) c = C_STONE_DARK;
                    else if (y < 7 && (x == 0 || x == 8)) c = C_STONE_DARK;
                    else if (y > 7 && (x == 4 || x == 12)) c = C_STONE_DARK;
                    else if (y == 1 || y == 9) c = C_STONE_LIGHT;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_BUSH:
            // Desenha grama no fundo
            render_metatile(sx, sy, TILE_GRASS);
            // Arbusto circular arredondado
            for (int y = 1; y < 15; y++) {
                for (int x = 1; x < 15; x++) {
                    float dist = sqrtf((float)((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)));
                    if (dist < 6.8f) {
                        u32 c = C_BUSH_MAIN;
                        if (dist < 4.0f && y < 7) c = C_BUSH_LIGHT;
                        else if (dist > 5.5f || y > 10) c = C_BUSH_DARK;
                        draw_tile_pixel(sx + x, sy + y, c);
                    }
                }
            }
            break;

        case TILE_FLOWER_RED:
            render_metatile(sx, sy, TILE_GRASS);
            // 4 pétalas vermelhas com miolo amarelo
            draw_tile_pixel(sx + 7, sy + 6, C_FLOWER_RED);
            draw_tile_pixel(sx + 9, sy + 6, C_FLOWER_RED);
            draw_tile_pixel(sx + 7, sy + 8, C_FLOWER_RED);
            draw_tile_pixel(sx + 9, sy + 8, C_FLOWER_RED);
            draw_tile_pixel(sx + 8, sy + 7, C_FLOWER_YEL);
            break;

        case TILE_FLOWER_YELLOW:
            render_metatile(sx, sy, TILE_GRASS);
            draw_tile_pixel(sx + 6, sy + 7, C_FLOWER_YEL);
            draw_tile_pixel(sx + 8, sy + 7, C_FLOWER_YEL);
            draw_tile_pixel(sx + 7, sy + 6, C_FLOWER_YEL);
            draw_tile_pixel(sx + 7, sy + 8, C_FLOWER_YEL);
            draw_tile_pixel(sx + 7, sy + 7, 0xFFFFFFFF);
            break;

        case TILE_TREE_TOP:
            render_metatile(sx, sy, TILE_GRASS);
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    float dist = sqrtf((float)((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)));
                    if (dist < 7.5f) {
                        u32 c = (dist < 4.5f && y < 7) ? C_GRASS_LIGHT : C_GRASS_DARK;
                        draw_tile_pixel(sx + x, sy + y, c);
                    }
                }
            }
            break;

        case TILE_TREE_TRUNK:
            render_metatile(sx, sy, TILE_GRASS);
            // Tronco vertical com textura de madeira
            for (int y = 0; y < 16; y++) {
                for (int x = 5; x <= 10; x++) {
                    u32 c = (x == 5 || x == 10) ? C_WOOD_DARK : C_CHEST_WOOD;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_CHEST_CLOSED:
            render_metatile(sx, sy, TILE_DIRT_PATH);
            // Baú clássico de madeira e ouro
            for (int y = 3; y <= 13; y++) {
                for (int x = 2; x <= 13; x++) {
                    u32 c = C_CHEST_WOOD;
                    if (y == 3 || y == 13 || x == 2 || x == 13) c = C_CHEST_GOLD;
                    else if (y == 8 && (x >= 6 && x <= 9)) c = C_CHEST_GOLD; // Fechadura
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_CHEST_OPEN:
            render_metatile(sx, sy, TILE_DIRT_PATH);
            for (int y = 5; y <= 13; y++) {
                for (int x = 2; x <= 13; x++) {
                    u32 c = (y < 8) ? 0x111111FF : C_CHEST_WOOD;
                    if (y == 13 || x == 2 || x == 13) c = C_CHEST_GOLD;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Brilho azulado de item coletado
            draw_tile_pixel(sx + 7, sy + 7, 0x00FFFFFF);
            draw_tile_pixel(sx + 8, sy + 7, 0x00FFFFFF);
            break;

        case TILE_WOOD_FENCE:
            render_metatile(sx, sy, TILE_GRASS);
            // Traves horizontais
            for (int x = 0; x < 16; x++) {
                draw_tile_pixel(sx + x, sy + 5, C_CHEST_WOOD);
                draw_tile_pixel(sx + x, sy + 10, C_CHEST_WOOD);
            }
            // Mourão central
            for (int y = 2; y <= 14; y++) {
                for (int x = 6; x <= 9; x++) {
                    u32 c = (x == 6 || x == 9) ? C_WOOD_DARK : C_CHEST_WOOD;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_COBBLESTONE:
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = C_COBBLE_MID;
                    // Argamassa nos cruzamentos e divisórias
                    if (y == 0 || y == 8 || ((y < 8) && (x == 0 || x == 8)) || ((y >= 8) && (x == 4 || x == 12))) {
                        c = C_COBBLE_DARK;
                    } else if (y == 1 || y == 9 || ((y < 8) && (x == 1 || x == 9)) || ((y >= 8) && (x == 5 || x == 13))) {
                        c = C_COBBLE_LIGHT;
                    } else if ((x + y * 3) % 11 == 0) {
                        c = C_COBBLE_LIGHT;
                    }
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_TOWN_WALL:
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = C_TOWN_WALL;
                    // Vigas de madeira enxaimel (bordas e trave central)
                    if (x == 0 || x == 15 || y == 0 || y == 15) {
                        c = C_TOWN_BEAM;
                    } else if (x == 7 || x == 8) {
                        c = C_TOWN_BEAM;
                    } else if ((x == y || x == (15 - y)) && (y >= 4 && y <= 11)) {
                        c = C_TOWN_BEAM;
                    }
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_ROOF_RED:
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = C_ROOF_RED;
                    int row = y % 4;
                    if (row == 0) c = C_ROOF_RED_DK;
                    else if (row == 1) c = C_ROOF_RED_LT;
                    int stagger = ((y / 4) % 2) * 4;
                    if ((x + stagger) % 8 == 0) c = C_ROOF_RED_DK;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_ROOF_BLUE:
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = C_ROOF_BLUE;
                    int row = y % 4;
                    if (row == 0) c = C_ROOF_BLUE_DK;
                    else if (row == 1) c = C_ROOF_BLUE_LT;
                    int stagger = ((y / 4) % 2) * 4;
                    if ((x + stagger) % 8 == 0) c = C_ROOF_BLUE_DK;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_TOWN_DOOR:
            render_metatile(sx, sy, TILE_TOWN_WALL);
            // Arco e madeira da porta com maçaneta dourada
            for (int y = 2; y <= 15; y++) {
                for (int x = 3; x <= 12; x++) {
                    if (y == 2 && (x == 3 || x == 12)) continue;
                    u32 c = C_DOOR_WOOD;
                    if (x == 3 || x == 12 || y == 2) c = C_DOOR_DK;
                    else if (x == 7 || x == 8) c = C_DOOR_DK;
                    if (y >= 8 && y <= 9 && x == 10) c = C_DOOR_GOLD;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_TOWN_WINDOW:
            render_metatile(sx, sy, TILE_TOWN_WALL);
            for (int y = 3; y <= 12; y++) {
                for (int x = 3; x <= 12; x++) {
                    u32 c = C_WINDOW_GLASS;
                    if (x == 3 || x == 12 || y == 3 || y == 12 || x == 7 || x == 8 || y == 7 || y == 8) {
                        c = C_WINDOW_FRAME;
                    } else if ((x == 5 && y == 5) || (x == 10 && y == 5)) {
                        c = 0xFFFFFFFF;
                    }
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_FOUNTAIN_EDGE:
            render_metatile(sx, sy, TILE_COBBLESTONE);
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    if (x >= 2 && x <= 13 && y >= 2 && y <= 13) {
                        u32 c = C_FOUNTAIN_MARB;
                        if (x == 2 || y == 2) c = 0xFFFFFFFF;
                        else if (x == 13 || y == 13) c = C_FOUNTAIN_DK;
                        else if (x >= 5 && x <= 10 && y >= 5 && y <= 10) c = C_WATER_SHALLOW;
                        draw_tile_pixel(sx + x, sy + y, c);
                    }
                }
            }
            break;

        case TILE_MARKET_STALL:
            render_metatile(sx, sy, TILE_COBBLESTONE);
            // Toldo listrado vermelho e branco no topo
            for (int y = 0; y <= 7; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = ((x / 3) % 2 == 0) ? C_STALL_RED : C_STALL_WHITE;
                    if (y == 7 && (x % 4 == 0 || x % 4 == 3)) c = C_COBBLE_DARK;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Balcão de madeira do comerciante
            for (int y = 8; y <= 14; y++) {
                for (int x = 1; x <= 14; x++) {
                    u32 c = (y == 8) ? C_CHEST_GOLD : C_CHEST_WOOD;
                    if (x == 1 || x == 14 || y == 14) c = C_WOOD_DARK;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_BARREL_CRATE:
            render_metatile(sx, sy, TILE_COBBLESTONE);
            for (int y = 2; y <= 14; y++) {
                int w = (y == 2 || y == 14) ? 8 : (y <= 4 || y >= 12) ? 10 : 12;
                int start = 8 - w / 2;
                for (int x = start; x < start + w; x++) {
                    u32 c = C_BARREL_WOOD;
                    if (y == 4 || y == 12 || y == 2 || y == 14) c = C_BARREL_BAND;
                    else if (x == start || x == start + w - 1) c = C_WOOD_DARK;
                    else if (x == start + 2) c = C_CHEST_GOLD;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        // --------------------------------------------------------------------
        // TILES DA VILA DOS MINISH (PICORI VILLAGE)
        // --------------------------------------------------------------------
        case TILE_VILLAGE_PATH:
            // Caminho de terra macia da floresta com musgo aveludado e pequenas pedrinhas
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = 0x557A3EFF;
                    if ((x + y * 2) % 5 == 0) c = 0x486B32FF;
                    else if ((x * 3 + y) % 7 == 0) c = 0x6E9B4EFF;
                    if ((x == 4 && y == 7) || (x == 11 && y == 3) || (x == 8 && y == 12)) c = 0x94A3B8FF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_MUSHROOM_CAP:
            // Chapéu de cogumelo vermelho com bolinhas brancas (telhado das casas)
            render_metatile(sx, sy, TILE_GRASS);
            for (int y = 0; y < 16; y++) {
                int r_w = (y < 4) ? (y * 2 + 8) : (y < 12) ? 16 : (16 - (y - 12) * 2);
                if (r_w > 16) r_w = 16;
                int start_x = (16 - r_w) / 2;
                for (int x = start_x; x < start_x + r_w; x++) {
                    u32 c = 0xDC2626FF;
                    if (y < 3) c = 0xEF4444FF;
                    else if (y > 11) c = 0x991B1BFF;
                    if ((x >= 3 && x <= 5 && y >= 3 && y <= 5) ||
                        (x >= 10 && x <= 12 && y >= 4 && y <= 6) ||
                        (x >= 6 && x <= 8 && y >= 8 && y <= 10)) {
                        c = (x == 4 && y == 4) || (x == 11 && y == 5) || (x == 7 && y == 9) ? 0xFFFFFFFF : 0xFEE2E2FF;
                    }
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_MUSHROOM_STEM:
            // Tronco de madeira/palha com porta redonda de noz esculpida
            render_metatile(sx, sy, TILE_VILLAGE_PATH);
            for (int y = 0; y < 16; y++) {
                for (int x = 2; x <= 13; x++) {
                    u32 c = (y % 3 == 0) ? 0xE5D0BAFF : 0xF5E6D3FF;
                    if (x == 2 || x == 13) c = 0xC4A482FF;
                    if (y >= 5 && y <= 15 && x >= 5 && x <= 10) {
                        c = 0x78350FFF;
                        if (y == 5 || x == 5 || x == 10) c = 0x451A03FF;
                        if (x == 9 && y == 10) c = 0xF59E0BFF;
                    }
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_ACORN_HOUSE:
            // Casa feita de casca de noz/bolota esculpida com cúpula e janela iluminada
            render_metatile(sx, sy, TILE_GRASS);
            for (int y = 1; y < 15; y++) {
                int a_w = (y < 6) ? (y * 2 + 4) : 14;
                int ax_start = (16 - a_w) / 2;
                for (int x = ax_start; x < ax_start + a_w; x++) {
                    u32 c = 0xB45309FF;
                    if (y <= 4) {
                        c = ((x + y) % 2 == 0) ? 0x78350FFF : 0x522306FF;
                    } else if (y >= 7 && y <= 11 && x >= 6 && x <= 9) {
                        c = (x == 6 || x == 9 || y == 7 || y == 11) ? 0x451A03FF : 0xFDE047FF;
                    }
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_VILLAGE_STREAM: {
            // Riacho de águas rasas cristalinas da vila
            int offset = s_water_anim_frame;
            for (int y = 0; y < TILE_SIZE; y++) {
                for (int x = 0; x < TILE_SIZE; x++) {
                    u32 c = 0x0284C7FF;
                    if ((x + y + offset) % 6 == 0) c = 0x38BDF8FF;
                    else if ((x + y + offset) % 12 == 0) c = 0xBAE6FDFF;
                    if ((x == 3 && y == 5) || (x == 12 && y == 10)) c = 0x0C4A6EFF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;
        }

        case TILE_VILLAGE_BRIDGE:
            // Ponte de pranchas rústicas de madeira sobre o riacho cristalino
            render_metatile(sx, sy, TILE_VILLAGE_STREAM);
            for (int y = 0; y < 16; y++) {
                for (int x = 1; x <= 14; x++) {
                    u32 c = 0x92400EFF;
                    if (y % 4 == 0) c = 0x451A03FF;
                    else if (y % 4 == 1) c = 0xB45309FF;
                    if ((x == 3 || x == 12) && (y % 4 == 2)) c = 0xD1D5DBFF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_GIANT_CLOVER:
            // Trevo gigante que serve de cobertura natural sobre a vila
            render_metatile(sx, sy, TILE_GRASS);
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    float dist = sqrtf((float)((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)));
                    if (dist <= 7.0f) {
                        u32 c = 0x10B981FF;
                        if (x == 7 || y == 7) c = 0x047857FF;
                        else if (dist <= 3.0f) c = 0x34D399FF;
                        if (x == 5 && y == 5) c = 0xFFFFFFFF;
                        draw_tile_pixel(sx + x, sy + y, c);
                    }
                }
            }
            break;

        case TILE_CHIMNEY_SMOKE: {
            render_metatile(sx, sy, TILE_MUSHROOM_CAP);
            int anim_phase = (s_water_anim_frame * 2) % 16;
            for (int y = 8; y <= 15; y++) {
                for (int x = 6; x <= 9; x++) {
                    draw_tile_pixel(sx + x, sy + y, (x == 6 || x == 9 || y == 8) ? 0x374151FF : 0x6B7280FF);
                }
            }
            int sm1_y = 6 - (anim_phase / 3);
            int sm1_x = 7 + (anim_phase % 4 < 2 ? 1 : -1);
            if (sm1_y >= 0) {
                draw_tile_pixel(sx + sm1_x, sy + sm1_y, 0xE5E7EBEE);
                draw_tile_pixel(sx + sm1_x + 1, sy + sm1_y, 0xE5E7EBEE);
            }
            int sm2_y = 2 - ((anim_phase + 8) % 16 / 4);
            int sm2_x = 8 + ((anim_phase / 2) % 3 - 1);
            if (sm2_y >= 0) {
                draw_tile_pixel(sx + sm2_x, sy + sm2_y, 0xD1D5DBEE);
            }
            break;
        }

        case TILE_GENTARI_SANCTUM:
            // Altar Sagrado de Gentari com pedestal ornado em ouro e brasão Picori
            render_metatile(sx, sy, TILE_VILLAGE_PATH);
            for (int y = 1; y < 15; y++) {
                for (int x = 1; x < 15; x++) {
                    u32 c = 0xD97706FF;
                    if (x == 1 || x == 14 || y == 1 || y == 14) c = 0x92400EFF;
                    else if (x >= 4 && x <= 11 && y >= 4 && y <= 11) {
                        c = 0xFDE047FF;
                        if (x == 7 || x == 8 || y == 7 || y == 8) c = 0x10B981FF;
                    }
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_CRACKED_WALL:
            // Parede de pedra com fissuras proeminentes destrutível por bombas
            render_metatile(sx, sy, TILE_STONE_WALL);
            {
                u32 c_crack_dark  = 0x0F172AFF;
                u32 c_crack_light = 0xE2E8F0AA;

                int crack_coords[12][2] = {
                    {7, 2}, {8, 3}, {7, 4}, {8, 5}, {9, 6}, {8, 7},
                    {7, 8}, {6, 9}, {7, 10}, {8, 11}, {7, 12}, {8, 13}
                };
                for (int i = 0; i < 12; i++) {
                    int cx = crack_coords[i][0];
                    int cy = crack_coords[i][1];
                    draw_tile_pixel(sx + cx, sy + cy, c_crack_dark);
                    draw_tile_pixel(sx + cx + 1, sy + cy, c_crack_light);
                }
                draw_tile_pixel(sx + 5, sy + 7, c_crack_dark);
                draw_tile_pixel(sx + 6, sy + 7, c_crack_light);
                draw_tile_pixel(sx + 10, sy + 6, c_crack_dark);
                draw_tile_pixel(sx + 11, sy + 6, c_crack_light);
            }
            break;

        case TILE_CRUMBLED_ROCK:
            // Rocha quebradiça de superfície áspera bloqueando caminhos
            render_metatile(sx, sy, TILE_DIRT_PATH);
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    float dist = sqrtf((float)((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)));
                    if (dist <= 7.0f) {
                        u32 c = 0x78716CFF; // Tom de rocha marrom-acinzentada
                        if (dist <= 6.0f && (x < 6 || y < 6)) c = 0xA8A29EFF; // Realce superior esquerdo
                        if (x > 10 || y > 10) c = 0x44403CFF; // Sombra inferior direita
                        if ((x + y == 14) || (x - y == 2 && y >= 6 && y <= 10)) {
                            c = 0x1C1917FF; // Fenda de fratura
                        }
                        draw_tile_pixel(sx + x, sy + y, c);
                    }
                }
            }
            break;

        case TILE_SECRET_ENTRANCE:
            // Portal em arco aberto revelando passagem secreta escura para dentro da rocha/parede
            render_metatile(sx, sy, TILE_STONE_WALL);
            for (int y = 2; y < 16; y++) {
                for (int x = 3; x <= 12; x++) {
                    if (y <= 4 && (x == 3 || x == 12)) continue;
                    u32 c = 0x050810FF; // Vazio profundo e misterioso da caverna
                    if (y == 2 || (y <= 4 && (x == 4 || x == 11))) c = 0x94A3B8FF; // Arco de pedra esculpido
                    else if (y == 3 || y == 4) c = 0x1E293BFF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_FENCE_GATE:
            // Portão de madeira rústica da Fazenda Lon Lon
            render_metatile(sx, sy, TILE_DIRT_PATH);
            for (int y = 4; y <= 14; y++) {
                draw_tile_pixel(sx + 2, sy + y, 0x78350FFF); // Poste esquerdo
                draw_tile_pixel(sx + 3, sy + y, 0xB45309FF);
                draw_tile_pixel(sx + 12, sy + y, 0x78350FFF); // Poste direito
                draw_tile_pixel(sx + 13, sy + y, 0xB45309FF);
            }
            for (int x = 3; x <= 12; x++) {
                draw_tile_pixel(sx + x, sy + 6, 0xD97706FF);
                draw_tile_pixel(sx + x, sy + 7, 0x92400EFF);
                draw_tile_pixel(sx + x, sy + 11, 0xD97706FF);
                draw_tile_pixel(sx + x, sy + 12, 0x92400EFF);
            }
            break;

        case TILE_CRENEL_ROAD_SIGN:
            // Placa de madeira rústica com seta esculpida indicando Monte Crenel
            render_metatile(sx, sy, TILE_DIRT_PATH);
            for (int y = 8; y <= 15; y++) {
                draw_tile_pixel(sx + 7, sy + y, 0x78350FFF); // Poste
                draw_tile_pixel(sx + 8, sy + y, 0x92400EFF);
            }
            for (int y = 2; y <= 8; y++) {
                for (int x = 2; x <= 13; x++) {
                    u32 c = (y == 2 || y == 8 || x == 2 || x == 13) ? 0x78350FFF : 0xD97706FF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            draw_tile_pixel(sx + 5, sy + 5, 0x451A03FF);
            draw_tile_pixel(sx + 6, sy + 4, 0x451A03FF);
            draw_tile_pixel(sx + 6, sy + 6, 0x451A03FF);
            draw_tile_pixel(sx + 7, sy + 5, 0x451A03FF);
            draw_tile_pixel(sx + 8, sy + 5, 0x451A03FF);
            break;

        case TILE_CRENEL_GRAVEL:
            // Solo terroso vulcânico de Monte Crenel (marrom-avermelhado com pedregulhos)
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 col = ((x + y * 3) % 5 == 0) ? 0x78350FFF :
                              (((x * 2 + y) % 7 == 0) ? 0x9A3412FF : 0x854D0EFF);
                    draw_tile_pixel(sx + x, sy + y, col);
                }
            }
            break;

        case TILE_CRENEL_CLIFF_FACE:
            // Paredão rochoso íngreme vulcânico de Monte Crenel
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 col = (y == 0 || x == 0) ? 0x9A3412FF :
                              (y == 15 || x == 15) ? 0x431407FF :
                              (((x ^ y) % 4 == 0) ? 0x78350FFF : 0x5B210CFF);
                    draw_tile_pixel(sx + x, sy + y, col);
                }
            }
            draw_tile_pixel(sx + 4, sy + 6, 0x270B04FF);
            draw_tile_pixel(sx + 5, sy + 7, 0x270B04FF);
            draw_tile_pixel(sx + 10, sy + 11, 0x270B04FF);
            draw_tile_pixel(sx + 11, sy + 12, 0x270B04FF);
            break;

        case TILE_CLIMBABLE_WALL:
            // Paredão com ranhuras e saliências escalável com o Grip Ring (GBA Authentic)
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 col = (y % 4 == 0) ? 0x431407FF : 0x7C2D12FF;
                    draw_tile_pixel(sx + x, sy + y, col);
                }
            }
            for (int y = 2; y <= 14; y += 4) {
                int off = (y % 8 == 2) ? 3 : 7;
                draw_tile_pixel(sx + off, sy + y, 0xC2410CFF);
                draw_tile_pixel(sx + off + 1, sy + y, 0xC2410CFF);
                draw_tile_pixel(sx + off + 6, sy + y, 0xC2410CFF);
                draw_tile_pixel(sx + off + 7, sy + y, 0xC2410CFF);
            }
            break;

        case TILE_MINERAL_WATER:
            // Água termal mineral verde efervescente de Monte Crenel
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 col = ((x + y) % 6 == 0) ? 0x34D399FF :
                              (((x * 3 + y) % 8 == 0) ? 0x10B981FF : 0x059669FF);
                    draw_tile_pixel(sx + x, sy + y, col);
                }
            }
            draw_tile_pixel(sx + 4, sy + 4, 0xA7F3D0FF);
            draw_tile_pixel(sx + 11, sy + 9, 0xA7F3D0FF);
            draw_tile_pixel(sx + 7, sy + 13, 0xA7F3D0FF);
            break;

        case TILE_MAGIC_BEAN_SPROUT:
            // Broto de feijão mágico plantado no solo fértil
            render_metatile(sx, sy, TILE_CRENEL_GRAVEL);
            for (int y = 5; y <= 13; y++) {
                for (int x = 4; x <= 11; x++) {
                    draw_tile_pixel(sx + x, sy + y, 0x451A03FF);
                }
            }
            draw_tile_pixel(sx + 7, sy + 7, 0x22C55EFF);
            draw_tile_pixel(sx + 8, sy + 7, 0x22C55EFF);
            draw_tile_pixel(sx + 6, sy + 8, 0x16A34AFF);
            draw_tile_pixel(sx + 9, sy + 8, 0x16A34AFF);
            draw_tile_pixel(sx + 7, sy + 9, 0x15803DFF);
            draw_tile_pixel(sx + 8, sy + 9, 0x15803DFF);
            break;

        case TILE_CLIMBABLE_VINE:
            // Videira gigante de feijão mágico escalável
            render_metatile(sx, sy, TILE_CRENEL_CLIFF_FACE);
            for (int y = 0; y < 16; y++) {
                draw_tile_pixel(sx + 6, sy + y, 0x166534FF);
                draw_tile_pixel(sx + 7, sy + y, 0x15803DFF);
                draw_tile_pixel(sx + 8, sy + y, 0x22C55EFF);
                draw_tile_pixel(sx + 9, sy + y, 0x15803DFF);
            }
            draw_tile_pixel(sx + 4, sy + 4, 0x4ADE80FF);
            draw_tile_pixel(sx + 5, sy + 4, 0x22C55EFF);
            draw_tile_pixel(sx + 10, sy + 10, 0x4ADE80FF);
            draw_tile_pixel(sx + 11, sy + 10, 0x22C55EFF);
            break;

        case TILE_PACCI_HOLE_NORMAL:
            // Buraco/cova circular no solo para energizar com o Cajado de Pacci
            render_metatile(sx, sy, TILE_CRENEL_GRAVEL);
            for (int y = 2; y <= 13; y++) {
                for (int x = 2; x <= 13; x++) {
                    float dist = sqrtf((float)((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)));
                    if (dist <= 5.5f) {
                        u32 c = 0x0A0604FF; // Abismo profundo do buraco
                        if (dist > 4.5f) c = 0x2D1B0FFF; // Borda de terra sombreada
                        else if (dist > 3.0f) c = 0x150C08FF;
                        draw_tile_pixel(sx + x, sy + y, c);
                    }
                }
            }
            break;

        case TILE_PACCI_HOLE_CHARGED: {
            // Buraco energizado pelo Cajado de Pacci (vórtice cintilante de super-salto)
            render_metatile(sx, sy, TILE_CRENEL_GRAVEL);
            int anim_rot = (s_water_anim_frame * 3) % 8;
            for (int y = 2; y <= 13; y++) {
                for (int x = 2; x <= 13; x++) {
                    float dist = sqrtf((float)((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)));
                    if (dist <= 5.8f) {
                        u32 c = 0x0284C7FF; // Cyan profundo
                        if (dist <= 1.8f) c = 0xFFFFFFFF; // Núcleo brilhante
                        else if (dist <= 3.8f) {
                            c = ((x + y + anim_rot) % 2 == 0) ? 0x7DD3FCFF : 0x38BDF8FF;
                        } else if (dist > 4.8f) {
                            c = 0x0369A1FF;
                        }
                        draw_tile_pixel(sx + x, sy + y, c);
                    }
                }
            }
            // Faíscas ascendentes de energia mágica prontas para catapultar
            int spark_y = 6 - (s_water_anim_frame % 4);
            draw_tile_pixel(sx + 7, sy + spark_y, 0xFDE047FF);
            draw_tile_pixel(sx + 8, sy + spark_y, 0xFFFFFFFF);
            break;
        }

        case TILE_MINECART_UPSIDE_DOWN:
            // Carrinho de mina tombado / de ponta-cabeça bloqueando os trilhos
            render_metatile(sx, sy, TILE_CRENEL_GRAVEL);
            // Trilhos no solo
            for (int x = 0; x < 16; x++) {
                draw_tile_pixel(sx + x, sy + 14, 0x475569FF);
                draw_tile_pixel(sx + x, sy + 15, 0x334155FF);
            }
            // Dormentes de madeira
            for (int x = 2; x <= 14; x += 4) {
                draw_tile_pixel(sx + x, sy + 13, 0x78350FFF);
                draw_tile_pixel(sx + x, sy + 14, 0x78350FFF);
            }
            // Rodas viradas para cima
            draw_tile_pixel(sx + 3, sy + 3, 0x94A3B8FF);
            draw_tile_pixel(sx + 4, sy + 3, 0x94A3B8FF);
            draw_tile_pixel(sx + 3, sy + 4, 0x334155FF);
            draw_tile_pixel(sx + 4, sy + 4, 0x334155FF);
            draw_tile_pixel(sx + 11, sy + 3, 0x94A3B8FF);
            draw_tile_pixel(sx + 12, sy + 3, 0x94A3B8FF);
            draw_tile_pixel(sx + 11, sy + 4, 0x334155FF);
            draw_tile_pixel(sx + 12, sy + 4, 0x334155FF);
            // Corpo de ferro do carrinho de ponta-cabeça
            for (int y = 5; y <= 12; y++) {
                for (int x = 2; x <= 13; x++) {
                    u32 c = 0x64748BFF;
                    if (y == 5 || y == 12 || x == 2 || x == 13) c = 0x334155FF;
                    else if (x == 7 || x == 8) c = 0x475569FF; // Reforço central
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_MINECART:
            // Carrinho de mina funcional e desvirado nos trilhos
            render_metatile(sx, sy, TILE_CRENEL_GRAVEL);
            // Trilhos no solo
            for (int x = 0; x < 16; x++) {
                draw_tile_pixel(sx + x, sy + 14, 0x475569FF);
                draw_tile_pixel(sx + x, sy + 15, 0x334155FF);
            }
            // Dormentes de madeira
            for (int x = 2; x <= 14; x += 4) {
                draw_tile_pixel(sx + x, sy + 13, 0x78350FFF);
                draw_tile_pixel(sx + x, sy + 14, 0x78350FFF);
            }
            // Corpo de ferro do carrinho funcional
            for (int y = 4; y <= 10; y++) {
                for (int x = 2; x <= 13; x++) {
                    u32 c = 0x64748BFF;
                    if (y == 4 || y == 10 || x == 2 || x == 13) c = 0x334155FF;
                    else if (y >= 5 && y <= 7) c = 0x1E293BFF; // Interior oco da caçamba
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Rodas de ferro no solo sobre os trilhos
            draw_tile_pixel(sx + 3, sy + 11, 0x94A3B8FF);
            draw_tile_pixel(sx + 4, sy + 11, 0x94A3B8FF);
            draw_tile_pixel(sx + 3, sy + 12, 0x334155FF);
            draw_tile_pixel(sx + 4, sy + 12, 0x334155FF);
            draw_tile_pixel(sx + 11, sy + 11, 0x94A3B8FF);
            draw_tile_pixel(sx + 12, sy + 11, 0x94A3B8FF);
            draw_tile_pixel(sx + 11, sy + 12, 0x334155FF);
            draw_tile_pixel(sx + 12, sy + 12, 0x334155FF);
            break;

        case TILE_MELARI_STONE_FLOOR:
            // Piso de lajotas de pedra escura das minas subterrâneas
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0x1E293BFF; // Ardósia escura base
                    if (x == 7 || y == 7 || x == 15 || y == 15) c = 0x0F172AFF; // Linhas de rejunte
                    else if ((x == 3 && y == 3) || (x == 11 && y == 11)) c = 0x334155FF; // Realces
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_MELARI_FORGE_ANVIL:
            // Bigorna monumental e pedestal da forja de Melari com brasas vivas
            render_metatile(sx, sy, TILE_MELARI_STONE_FLOOR);
            // Pedestal de cantaria e brasas ardentes na base
            for (int y = 11; y <= 15; y++) {
                for (int x = 2; x <= 13; x++) {
                    u32 c = 0x475569FF;
                    if (y >= 13 && (x >= 4 && x <= 11)) {
                        c = ((x + y + s_water_anim_frame) % 2 == 0) ? 0xEA580CFF : 0xF97316FF; // Brasas
                    }
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Corpo de ferro fundido da bigorna (mesa, chifre e pé)
            for (int y = 4; y <= 10; y++) {
                for (int x = 3; x <= 12; x++) {
                    u32 c = 0x64748BFF;
                    if (y == 4) c = 0xCBD5E1FF; // Face de impacto de aço polido
                    else if (x == 3 && y <= 6) c = 0x94A3B8FF; // Chifre cônico
                    else if (x >= 6 && x <= 9) c = 0x475569FF; // Cintura da bigorna
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_MELARI_LAVA_CHANNEL:
            // Canal de lava derretida incandescente fluindo pelas rochas
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0xDC2626FF; // Vermelho fogo
                    if (y == 0 || y == 15) c = 0x292524FF; // Borda de rocha queimada
                    else if (y >= 2 && y <= 13) {
                        int pulse = (x + y * 2 + s_water_anim_frame * 3) % 7;
                        if (pulse == 0 || pulse == 1) c = 0xFBBF24FF; // Bolhas amarelas brilhantes
                        else if (pulse == 2 || pulse == 3) c = 0xF97316FF; // Laranja incandescente
                        else c = 0xEA580CFF; // Magma derretido
                    }
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_MELARI_ORE_VEIN:
            // Parede de rocha vulcânica com veios de minério e cristais cintilantes
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0x1C1917FF;
                    if ((x + y) % 5 == 0) c = 0x292524FF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Cristais de minério de alta qualidade incrustados (esmeralda, safira, rubi)
            draw_tile_pixel(sx + 4, sy + 5, 0x10B981FF); // Esmeralda verde
            draw_tile_pixel(sx + 5, sy + 5, 0x34D399FF);
            draw_tile_pixel(sx + 4, sy + 6, 0xFFFFFFFF);
            draw_tile_pixel(sx + 10, sy + 10, 0x06B6D4FF); // Safira ciano
            draw_tile_pixel(sx + 11, sy + 10, 0x38BDF8FF);
            draw_tile_pixel(sx + 11, sy + 11, 0xFFFFFFFF);
            draw_tile_pixel(sx + 8, sy + 3, 0xEF4444FF); // Rubi vermelho
            draw_tile_pixel(sx + 9, sy + 3, 0xFCA5A5FF);
            draw_tile_pixel(sx + 3, sy + 12, 0xFACC15FF); // Ouro amarelo
            break;

        case TILE_MELARI_MINING_TRACK:
            // Trilhos de ferro e dormentes de madeira sobre o piso de lajotas
            render_metatile(sx, sy, TILE_MELARI_STONE_FLOOR);
            // Dormentes de madeira transversais
            for (int dy = 3; dy <= 4; dy++) {
                for (int x = 1; x <= 14; x++) draw_tile_pixel(sx + x, sy + dy, 0x78350FFF);
            }
            for (int dy = 11; dy <= 12; dy++) {
                for (int x = 1; x <= 14; x++) draw_tile_pixel(sx + x, sy + dy, 0x78350FFF);
            }
            // Trilhos longitudinais de ferro polido
            for (int y = 0; y < 16; y++) {
                draw_tile_pixel(sx + 3, sy + y, 0x475569FF);
                draw_tile_pixel(sx + 4, sy + y, 0x94A3B8FF);
                draw_tile_pixel(sx + 11, sy + y, 0x475569FF);
                draw_tile_pixel(sx + 12, sy + y, 0x94A3B8FF);
            }
            break;

        case TILE_MELARI_CAVE_ARCHWAY:
            // Portal em arco entalhado na rocha para a Caverna das Chamas (Dungeon 2)
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0x334155FF; // Arco de pedra
                    if (y >= 4 && (x >= 3 && x <= 12)) c = 0x020617FF; // Entrada da caverna escura
                    else if (y < 4 && (x >= 4 && x <= 11)) c = 0x1E293BFF; // Moldura do arco superior
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Tochas acesas iluminando a entrada
            draw_tile_pixel(sx + 1, sy + 7, 0x78350FFF);
            draw_tile_pixel(sx + 1, sy + 6, 0xF97316FF);
            draw_tile_pixel(sx + 14, sy + 7, 0x78350FFF);
            draw_tile_pixel(sx + 14, sy + 6, 0xF97316FF);
            break;

        case TILE_SWAMP_WATER: {
            // Água escura e turva do pântano de Castor Wilds com ondas verdes/azuis
            int anim_off = s_water_anim_frame;
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0x0E3846FF; // Base escura de mangue
                    if ((x + y * 2 + anim_off) % 7 == 0) c = 0x155E75FF;
                    else if ((x * 2 + y + anim_off) % 11 == 0) c = 0x0891B2FF;
                    if ((x == 4 && y == 7) || (x == 12 && y == 3)) c = 0x22D3EE88; // Reflexo de musgo
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;
        }

        case TILE_SWAMP_MUD: {
            // Lodo movediço profundo e viscoso (requer Pegasus Dash para não afundar!)
            int anim_bubble = (s_water_anim_frame * 2) % 16;
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0x2A1810FF; // Terra escura turfosa
                    if ((x ^ y) % 3 == 0) c = 0x1C0E07FF;
                    else if ((x + y * 3) % 7 == 0) c = 0x382215FF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Bolhas viscosas de gás do pântano brotando do lodo
            int bx = 5 + (anim_bubble % 6);
            int by = 8 + (anim_bubble % 4);
            draw_tile_pixel(sx + bx, sy + by, 0x5C3E25FF);
            draw_tile_pixel(sx + bx + 1, sy + by, 0x78562BFF);
            draw_tile_pixel(sx + bx, sy + by + 1, 0x170C06FF);
            break;
        }

        case TILE_SWAMP_GRASS:
            // Musgo e vegetação pantanosa verde-oliva com brotos úmidos
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0x3F4E24FF; // Verde-oliva pantanoso
                    if ((x + y) % 4 == 0) c = 0x2C3717FF;
                    else if ((x * 3 + y) % 7 == 0) c = 0x5B6D36FF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Tufos de capim do brejo
            draw_tile_pixel(sx + 3, sy + 4, 0x6F8341FF);
            draw_tile_pixel(sx + 3, sy + 3, 0x8FA456FF);
            draw_tile_pixel(sx + 11, sy + 11, 0x6F8341FF);
            draw_tile_pixel(sx + 12, sy + 10, 0x8FA456FF);
            break;

        case TILE_SWAMP_LOG:
            // Tronco de árvore caído oco no lodo (passarela / túnel de madeira envelhecida)
            render_metatile(sx, sy, TILE_SWAMP_MUD);
            for (int y = 2; y <= 13; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0x422006FF; // Casca de madeira envelhecida
                    if (y == 2 || y == 13) c = 0x2B1504FF; // Borda sombreada
                    else if (y >= 5 && y <= 10) c = 0x150B02FF; // Interior oco escuro
                    else if (x % 4 == 0) c = 0x713F12FF; // Ranhuras de madeira
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_EYE_STATUE:
            // Estátua ancestral de pedra com olho fechado (sensível a flechadas)
            render_metatile(sx, sy, TILE_SWAMP_GRASS);
            // Corpo monolítico de cantaria ancestral
            for (int y = 1; y <= 14; y++) {
                for (int x = 2; x <= 13; x++) {
                    u32 c = 0x475569FF; // Pedra ardósia
                    if (x == 2 || x == 13 || y == 1 || y == 14) c = 0x1E293BFF; // Borda cinzelada
                    else if ((x + y) % 5 == 0) c = 0x334155FF; // Musgo na rocha
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Olho ciclópico central fechado (pálpebra cerrada em sono ancestral)
            for (int x = 4; x <= 11; x++) {
                draw_tile_pixel(sx + x, sy + 7, 0x0F172AFF); // Fissura escura da pálpebra fechada
                draw_tile_pixel(sx + x, sy + 8, 0x1E293BFF);
            }
            // Detalhes da íris adormecida
            draw_tile_pixel(sx + 7, sy + 6, 0x64748BFF);
            draw_tile_pixel(sx + 8, sy + 6, 0x64748BFF);
            draw_tile_pixel(sx + 7, sy + 9, 0x334155FF);
            draw_tile_pixel(sx + 8, sy + 9, 0x334155FF);
            break;

        case TILE_EYE_STATUE_OPEN: {
            // Estátua ancestral despertada com olho místico aberto por flechada certeira!
            render_metatile(sx, sy, TILE_SWAMP_GRASS);
            for (int y = 1; y <= 14; y++) {
                for (int x = 2; x <= 13; x++) {
                    u32 c = 0x475569FF;
                    if (x == 2 || x == 13 || y == 1 || y == 14) c = 0x1E293BFF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            // Órbita ocular aberta em arco
            for (int y = 4; y <= 11; y++) {
                for (int x = 4; x <= 11; x++) {
                    float dist = sqrtf((float)((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)));
                    if (dist <= 3.8f) {
                        u32 c = 0x0891B2FF; // Íris ciano mística
                        if (dist <= 1.2f) c = 0xFFFFFFFF; // Pupila radiante brilhante
                        else if (dist > 2.6f) c = 0xFEF08AFF; // Esclera dourada ancestral
                        draw_tile_pixel(sx + x, sy + y, c);
                    }
                }
            }
            // Brilho cintilante emitido pelo olho aberto
            int eye_spark = s_water_anim_frame % 4;
            draw_tile_pixel(sx + 6 + eye_spark, sy + 4, 0x67E8F9FF);
            break;
        }

        case TILE_DIRT_WALL:
            // Parede de terra fofa compactada escavável com as Mole Mitts
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0x6B3E1EFF; // Marrom terra fofa base
                    if ((x + y * 2) % 5 == 0) c = 0x542F15FF; // Camada escura compactada
                    else if ((x * 3 + y) % 7 == 0) c = 0x824C25FF; // Terra fofa solta clara
                    // Seixos e pedregulhos incrustados na terra
                    if ((x == 4 && y == 5) || (x == 11 && y == 10) || (x == 8 && y == 13)) c = 0x3E2310FF;
                    if ((x == 5 && y == 5) || (x == 12 && y == 10)) c = 0x9A5C30FF;
                    // Borda sombreada de profundidade
                    if (y == 0 || x == 0) c = 0x542F15FF;
                    if (y == 15 || x == 15) c = 0x3E2310FF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_DIRT_WALL_TUNNEL:
            // Chão de túnel escavado com marcas de garras e terra batida
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    u32 c = 0x3D2414FF; // Terra batida escura
                    if ((x + y) % 6 == 0) c = 0x4E2F1BFF;
                    // Marcas de garras paralelas cravadas no chão
                    if ((y == 4 || y == 10) && (x >= 3 && x <= 6)) c = 0x2A180EFF;
                    if ((y == 5 || y == 11) && (x >= 9 && x <= 12)) c = 0x2A180EFF;
                    draw_tile_pixel(sx + x, sy + y, c);
                }
            }
            break;

        case TILE_DIRT_MOUND:
            // Montículo de terra fofa no chão
            render_metatile(sx, sy, TILE_DIRT_PATH);
            for (int y = 3; y <= 13; y++) {
                for (int x = 3; x <= 13; x++) {
                    float dist = sqrtf((float)((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f)));
                    if (dist <= 5.5f) {
                        u32 c = 0x6B3E1EFF;
                        if (dist <= 2.5f) c = 0x824C25FF;
                        else if (dist > 4.5f) c = 0x3E2310FF;
                        draw_tile_pixel(sx + x, sy + y, c);
                    }
                }
            }
            break;

        default:
            render_metatile(sx, sy, TILE_GRASS);
            break;
    }
}

// ----------------------------------------------------------------------------
// CONSTRUÇÃO DO MUNDO DE DEMONSTRAÇÃO (640x480 PIXELS)
// ----------------------------------------------------------------------------
Tilemap* map_create_demo_world(void) {
    int w = 40;
    int h = 30;
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width  = w;
    m->height = h;
    m->is_authentic = false;
    m->authentic_tex = NULL;
    m->ground_layer  = (u8*)malloc(w * h * sizeof(u8));
    m->overlay_layer = (u8*)malloc(w * h * sizeof(u8));
    m->collision_map = (u8*)malloc(w * h * sizeof(u8));

    // Preenche com grama base
    for (int i = 0; i < w * h; i++) {
        m->ground_layer[i]  = TILE_GRASS;
        m->overlay_layer[i] = 0xFF; // Vazio
        m->collision_map[i] = 0;    // Andável
    }

    // 1. Bordas externas do mapa com muralhas de pedra sólidas
    for (int x = 0; x < w; x++) {
        m->overlay_layer[0 * w + x] = TILE_STONE_WALL;
        m->collision_map[0 * w + x] = 1;
        m->overlay_layer[(h - 1) * w + x] = TILE_STONE_WALL;
        m->collision_map[(h - 1) * w + x] = 1;
    }
    for (int y = 0; y < h; y++) {
        m->overlay_layer[y * w + 0] = TILE_STONE_WALL;
        m->collision_map[y * w + 0] = 1;
        m->overlay_layer[y * w + (w - 1)] = TILE_STONE_WALL;
        m->collision_map[y * w + (w - 1)] = 1;
    }

    // 2. Caminho de terra central cruzando o mapa
    for (int x = 5; x < 35; x++) {
        m->ground_layer[10 * w + x] = TILE_DIRT_PATH;
        m->ground_layer[11 * w + x] = TILE_DIRT_PATH;
    }
    for (int y = 5; y < 25; y++) {
        m->ground_layer[y * w + 18] = TILE_DIRT_PATH;
        m->ground_layer[y * w + 19] = TILE_DIRT_PATH;
    }

    // 3. Lago de água límpida no sudeste
    for (int y = 18; y < 27; y++) {
        for (int x = 25; x < 37; x++) {
            m->ground_layer[y * w + x] = TILE_WATER;
            m->collision_map[y * w + x] = 1; // Água é barreira sólida
        }
    }

    // 4. Arvoredo no noroeste
    for (int y = 2; y <= 5; y += 2) {
        for (int x = 3; x <= 12; x += 3) {
            m->overlay_layer[(y - 1) * w + x] = TILE_TREE_TOP;
            m->overlay_layer[y * w + x]       = TILE_TREE_TRUNK;
            m->collision_map[y * w + x]       = 1;
        }
    }

    // 5. Cercas e flores
    for (int x = 8; x <= 14; x++) {
        m->overlay_layer[16 * w + x] = TILE_WOOD_FENCE;
        m->collision_map[16 * w + x] = 1;
    }
    m->overlay_layer[14 * w + 6]  = TILE_FLOWER_RED;
    m->overlay_layer[14 * w + 10] = TILE_FLOWER_YELLOW;
    m->overlay_layer[8 * w + 24]  = TILE_FLOWER_RED;

    // 6. Clareira secreta no nordeste com arbustos cortáveis protegendo um Baú de Tesouro!
    for (int y = 3; y <= 7; y++) {
        for (int x = 28; x <= 33; x++) {
            if (x == 31 && y == 5) {
                m->overlay_layer[y * w + x] = TILE_CHEST_CLOSED;
                m->collision_map[y * w + x] = 1;
            } else if (x == 28 || x == 33 || y == 3 || y == 7) {
                m->overlay_layer[y * w + x] = TILE_BUSH;
                m->collision_map[y * w + x] = 1;
            }
        }
    }

    // Arbustos isolados no campo
    m->overlay_layer[12 * w + 8]  = TILE_BUSH;
    m->collision_map[12 * w + 8]  = 1;
    m->overlay_layer[12 * w + 14] = TILE_BUSH;
    m->collision_map[12 * w + 14] = 1;
    m->overlay_layer[18 * w + 14] = TILE_BUSH;
    m->collision_map[18 * w + 14] = 1;

    // Parede rachada e rocha quebradiça destrutíveis por bombas
    m->overlay_layer[0 * w + 8] = TILE_CRACKED_WALL; // Parede rachada no muro norte
    m->collision_map[0 * w + 8] = 1;
    m->overlay_layer[15 * w + 22] = TILE_CRUMBLED_ROCK; // Rocha bloqueando caminho ao baú
    m->collision_map[15 * w + 22] = 1;
    m->overlay_layer[15 * w + 23] = TILE_CHEST_CLOSED;
    m->collision_map[15 * w + 23] = 1;

    return m;
}

// ----------------------------------------------------------------------------
// CONSTRUÇÃO DA CIDADE DE HYRULE (HYRULE TOWN HUB - 576x448 PIXELS)
// ----------------------------------------------------------------------------
Tilemap* map_create_hyrule_town(void) {
    int w = 36;
    int h = 28;
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width  = w;
    m->height = h;
    m->is_authentic = false;
    m->authentic_tex = NULL;
    m->ground_layer  = (u8*)malloc(w * h * sizeof(u8));
    m->overlay_layer = (u8*)malloc(w * h * sizeof(u8));
    m->collision_map = (u8*)malloc(w * h * sizeof(u8));

    // 1. Pavimenta toda a praça da cidade com calçamento de pedras (Cobblestone)
    for (int i = 0; i < w * h; i++) {
        m->ground_layer[i]  = TILE_COBBLESTONE;
        m->overlay_layer[i] = 0xFF; // Livre
        m->collision_map[i] = 0;    // Andável
    }

    // 2. Muralhas de Defesa Externas e Portões do Reino
    // Muralha Norte (Castelo de Hyrule)
    for (int y = 0; y <= 1; y++) {
        for (int x = 0; x < w; x++) {
            // Portão do Castelo de Hyrule aberto no centro (x=16..19)
            if (x >= 16 && x <= 19) {
                m->ground_layer[y * w + x] = TILE_COBBLESTONE;
            } else {
                m->overlay_layer[y * w + x] = TILE_STONE_WALL;
                m->collision_map[y * w + x] = 1;
            }
        }
    }
    m->overlay_layer[1 * w + 8] = TILE_CRACKED_WALL; // Parede secreta na muralha norte
    m->collision_map[1 * w + 8] = 1;

    // Muralhas Laterais (Oeste e Leste)
    for (int y = 0; y < h; y++) {
        m->overlay_layer[y * w + 0] = TILE_STONE_WALL;
        m->collision_map[y * w + 0] = 1;
        m->overlay_layer[y * w + (w - 1)] = TILE_STONE_WALL;
        m->collision_map[y * w + (w - 1)] = 1;
    }

    // Canal Ornamental de Hyrule Town (Canal Oeste: x=1, y=1..h-3)
    for (int y = 1; y < h - 2; y++) {
        m->ground_layer[y * w + 1] = TILE_WATER;
        m->collision_map[y * w + 1] = 1; // Sólido a pé, navegável com Zora's Flippers!
        // Pontes de pedra sobre o canal conectando às ruas (y=8 e y=14)
        if (y == 8 || y == 14) {
            m->overlay_layer[y * w + 1] = TILE_COBBLESTONE;
            m->collision_map[y * w + 1] = 0;
        }
    }

    // Muralha Sul (Portão de saída para Hyrule Field / Minish Woods)
    for (int y = h - 2; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (x >= 16 && x <= 19) {
                m->ground_layer[y * w + x] = TILE_DIRT_PATH;
            } else {
                m->overlay_layer[y * w + x] = TILE_STONE_WALL;
                m->collision_map[y * w + x] = 1;
            }
        }
    }

    // 3. Chafariz Central de Hyrule (Fountain Plaza: x=16..19, y=12..15)
    for (int y = 12; y <= 15; y++) {
        for (int x = 16; x <= 19; x++) {
            if (x == 16 || x == 19 || y == 12 || y == 15) {
                m->overlay_layer[y * w + x] = TILE_FOUNTAIN_EDGE;
                m->collision_map[y * w + x] = 1; // Borda de mármore sólida
            } else {
                m->ground_layer[y * w + x] = TILE_WATER;
                m->collision_map[y * w + x] = 1; // Água profunda da fonte
            }
        }
    }

    // 4. Loja e Bazar do Stockwell (Distrito Comercial no Nordeste: x=23..32, y=3..7)
    // Telhado de telhas azuis da loja
    for (int y = 3; y <= 4; y++) {
        for (int x = 24; x <= 32; x++) {
            m->overlay_layer[y * w + x] = TILE_ROOF_BLUE;
            m->collision_map[y * w + x] = 1;
        }
    }
    // Fachada da loja com janelas e porta
    for (int x = 24; x <= 32; x++) {
        m->overlay_layer[5 * w + x] = TILE_TOWN_WALL;
        m->collision_map[5 * w + x] = 1;
    }
    m->overlay_layer[5 * w + 25] = TILE_TOWN_WINDOW;
    m->overlay_layer[5 * w + 28] = TILE_TOWN_DOOR;
    m->overlay_layer[5 * w + 31] = TILE_TOWN_WINDOW;

    // Balcões do Mercado e Toldo do Stockwell (y=6)
    for (int x = 24; x <= 26; x++) {
        m->overlay_layer[6 * w + x] = TILE_MARKET_STALL;
        m->collision_map[6 * w + x] = 1;
    }
    for (int x = 30; x <= 32; x++) {
        m->overlay_layer[6 * w + x] = TILE_MARKET_STALL;
        m->collision_map[6 * w + x] = 1;
    }
    // Barris e caixotes do armazém da loja
    m->overlay_layer[5 * w + 23] = TILE_BARREL_CRATE;
    m->collision_map[5 * w + 23] = 1;
    m->overlay_layer[6 * w + 23] = TILE_BARREL_CRATE;
    m->collision_map[6 * w + 23] = 1;
    m->overlay_layer[5 * w + 33] = TILE_BARREL_CRATE;
    m->collision_map[5 * w + 33] = 1;
    m->overlay_layer[6 * w + 33] = TILE_BARREL_CRATE;
    m->collision_map[6 * w + 33] = 1;

    // Rocha quebradiça bloqueando um baú secreto atrás da loja do Stockwell
    m->overlay_layer[5 * w + 22] = TILE_CRUMBLED_ROCK;
    m->collision_map[5 * w + 22] = 1;
    m->overlay_layer[4 * w + 22] = TILE_CHEST_CLOSED;
    m->collision_map[4 * w + 22] = 1;

    // 5. Mansão do Prefeito Hagen e Residências (Noroeste: x=3..12, y=3..7)
    // Telhado de terracota vermelha
    for (int y = 3; y <= 4; y++) {
        for (int x = 3; x <= 12; x++) {
            m->overlay_layer[y * w + x] = TILE_ROOF_RED;
            m->collision_map[y * w + x] = 1;
        }
    }
    // Fachada residencial
    for (int x = 3; x <= 12; x++) {
        m->overlay_layer[5 * w + x] = TILE_TOWN_WALL;
        m->collision_map[5 * w + x] = 1;
    }
    m->overlay_layer[5 * w + 4]  = TILE_TOWN_WINDOW;
    m->overlay_layer[5 * w + 7]  = TILE_TOWN_DOOR;
    m->overlay_layer[5 * w + 11] = TILE_TOWN_WINDOW;

    // Cerca de madeira e jardim de flores do Prefeito
    m->overlay_layer[6 * w + 3]  = TILE_WOOD_FENCE;
    m->collision_map[6 * w + 3]  = 1;
    m->overlay_layer[6 * w + 4]  = TILE_WOOD_FENCE;
    m->collision_map[6 * w + 4]  = 1;
    m->overlay_layer[6 * w + 6]  = TILE_FLOWER_RED;
    m->overlay_layer[6 * w + 8]  = TILE_FLOWER_YELLOW;
    m->overlay_layer[6 * w + 10] = TILE_WOOD_FENCE;
    m->collision_map[6 * w + 10] = 1;
    m->overlay_layer[6 * w + 11] = TILE_WOOD_FENCE;
    m->collision_map[6 * w + 11] = 1;

    // 6. Residência dos Cidadãos no Sudoeste (x=3..11, y=17..20)
    for (int y = 17; y <= 18; y++) {
        for (int x = 3; x <= 11; x++) {
            m->overlay_layer[y * w + x] = TILE_ROOF_RED;
            m->collision_map[y * w + x] = 1;
        }
    }
    for (int x = 3; x <= 11; x++) {
        m->overlay_layer[19 * w + x] = TILE_TOWN_WALL;
        m->collision_map[19 * w + x] = 1;
    }
    m->overlay_layer[19 * w + 4]  = TILE_TOWN_WINDOW;
    m->overlay_layer[19 * w + 7]  = TILE_TOWN_DOOR;
    m->overlay_layer[19 * w + 10] = TILE_TOWN_WINDOW;
    m->overlay_layer[20 * w + 3]  = TILE_FLOWER_RED;
    m->overlay_layer[20 * w + 5]  = TILE_FLOWER_YELLOW;
    m->overlay_layer[20 * w + 9]  = TILE_FLOWER_RED;
    m->overlay_layer[20 * w + 11] = TILE_FLOWER_YELLOW;

    // 7. Armazém & Estalagem no Sudeste (x=24..32, y=17..20)
    for (int y = 17; y <= 18; y++) {
        for (int x = 24; x <= 32; x++) {
            m->overlay_layer[y * w + x] = TILE_ROOF_BLUE;
            m->collision_map[y * w + x] = 1;
        }
    }
    for (int x = 24; x <= 32; x++) {
        m->overlay_layer[19 * w + x] = TILE_TOWN_WALL;
        m->collision_map[19 * w + x] = 1;
    }
    m->overlay_layer[19 * w + 25] = TILE_TOWN_WINDOW;
    m->overlay_layer[19 * w + 28] = TILE_TOWN_DOOR;
    m->overlay_layer[19 * w + 31] = TILE_TOWN_WINDOW;
    m->overlay_layer[20 * w + 24] = TILE_BARREL_CRATE;
    m->collision_map[20 * w + 24] = 1;
    m->overlay_layer[20 * w + 25] = TILE_BARREL_CRATE;
    m->collision_map[20 * w + 25] = 1;
    m->overlay_layer[20 * w + 31] = TILE_BARREL_CRATE;
    m->collision_map[20 * w + 31] = 1;

    // 8. Canteiros de Grama e Jardins Ornamentais da Praça
    // Canteiro Noroeste com árvore
    for (int y = 9; y <= 11; y++) {
        for (int x = 8; x <= 11; x++) {
            m->ground_layer[y * w + x] = TILE_GRASS;
        }
    }
    m->overlay_layer[9 * w + 9]   = TILE_TREE_TOP;
    m->overlay_layer[10 * w + 9]  = TILE_TREE_TRUNK;
    m->collision_map[10 * w + 9]  = 1;
    m->overlay_layer[11 * w + 8]  = TILE_FLOWER_RED;
    m->overlay_layer[11 * w + 11] = TILE_FLOWER_YELLOW;

    // Canteiro Nordeste com árvore
    for (int y = 9; y <= 11; y++) {
        for (int x = 24; x <= 27; x++) {
            m->ground_layer[y * w + x] = TILE_GRASS;
        }
    }
    m->overlay_layer[9 * w + 25]   = TILE_TREE_TOP;
    m->overlay_layer[10 * w + 25]  = TILE_TREE_TRUNK;
    m->collision_map[10 * w + 25]  = 1;
    m->overlay_layer[11 * w + 24]  = TILE_FLOWER_RED;
    m->overlay_layer[11 * w + 27]  = TILE_FLOWER_YELLOW;

    // Canteiro Sudoeste
    for (int y = 16; y <= 18; y++) {
        for (int x = 8; x <= 11; x++) {
            m->ground_layer[y * w + x] = TILE_GRASS;
        }
    }
    m->overlay_layer[16 * w + 8]  = TILE_FLOWER_YELLOW;
    m->overlay_layer[18 * w + 11] = TILE_FLOWER_RED;

    // Canteiro Sudeste
    for (int y = 16; y <= 18; y++) {
        for (int x = 24; x <= 27; x++) {
            m->ground_layer[y * w + x] = TILE_GRASS;
        }
    }
    m->overlay_layer[16 * w + 27] = TILE_FLOWER_RED;
    m->overlay_layer[18 * w + 24] = TILE_FLOWER_YELLOW;

    printf("[MAPA] Cidade de Hyrule (Hyrule Town Hub) criada: %dx%d tiles (%dx%d pixels).\n",
           w, h, w * TILE_SIZE, h * TILE_SIZE);
    return m;
}

// ----------------------------------------------------------------------------
// CONSTRUÇÃO DA VILA DOS MINISH (PICORI VILLAGE - 512x384 PIXELS)
// ----------------------------------------------------------------------------
Tilemap* map_create_minish_village(void) {
    int w = 32;
    int h = 24;
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width  = w;
    m->height = h;
    m->is_authentic = false;
    m->authentic_tex = NULL;
    m->ground_layer  = (u8*)malloc(w * h * sizeof(u8));
    m->overlay_layer = (u8*)malloc(w * h * sizeof(u8));
    m->collision_map = (u8*)malloc(w * h * sizeof(u8));

    // 1. Preenche todo o solo com grama aveludada Minish
    for (int i = 0; i < w * h; i++) {
        m->ground_layer[i]  = TILE_GRASS;
        m->overlay_layer[i] = 0xFF; // Livre
        m->collision_map[i] = 0;    // Andavel
    }

    // 2. Bordas com Trevos Gigantes (Giant Clovers) formando uma barreira natural
    for (int x = 0; x < w; x++) {
        // Topo (linhas 0 e 1)
        m->overlay_layer[0 * w + x] = TILE_GIANT_CLOVER;
        m->collision_map[0 * w + x] = 1;
        m->overlay_layer[1 * w + x] = TILE_GIANT_CLOVER;
        m->collision_map[1 * w + x] = 1;

        // Base (linhas 22 e 23) - exceto o portal/saida sul nos x = 15..16
        if (x < 15 || x > 16) {
            m->overlay_layer[(h - 2) * w + x] = TILE_GIANT_CLOVER;
            m->collision_map[(h - 2) * w + x] = 1;
            m->overlay_layer[(h - 1) * w + x] = TILE_GIANT_CLOVER;
            m->collision_map[(h - 1) * w + x] = 1;
        }
    }
    for (int y = 0; y < h; y++) {
        // Laterais esquerda (colunas 0 e 1) e direita (colunas w-2 e w-1)
        m->overlay_layer[y * w + 0] = TILE_GIANT_CLOVER;
        m->collision_map[y * w + 0] = 1;
        m->overlay_layer[y * w + 1] = TILE_GIANT_CLOVER;
        m->collision_map[y * w + 1] = 1;

        m->overlay_layer[y * w + (w - 2)] = TILE_GIANT_CLOVER;
        m->collision_map[y * w + (w - 2)] = 1;
        m->overlay_layer[y * w + (w - 1)] = TILE_GIANT_CLOVER;
        m->collision_map[y * w + (w - 1)] = 1;
    }

    // 3. Riacho Cristalino cortando a vila ao meio (y = 11..12, x = 2..29)
    for (int y = 11; y <= 12; y++) {
        for (int x = 2; x < w - 2; x++) {
            m->ground_layer[y * w + x] = TILE_VILLAGE_STREAM;
            m->collision_map[y * w + x] = 1; // Barreira de agua
        }
    }

    // 4. Pontes rusticas de madeira cruzando o riacho
    // Ponte Oeste (x = 8..9, y = 11..12)
    for (int y = 11; y <= 12; y++) {
        for (int x = 8; x <= 9; x++) {
            m->overlay_layer[y * w + x] = TILE_VILLAGE_BRIDGE;
            m->collision_map[y * w + x] = 0; // Andavel!
        }
    }
    // Ponte Leste (x = 22..23, y = 11..12)
    for (int y = 11; y <= 12; y++) {
        for (int x = 22; x <= 23; x++) {
            m->overlay_layer[y * w + x] = TILE_VILLAGE_BRIDGE;
            m->collision_map[y * w + x] = 0; // Andavel!
        }
    }

    // 5. Trilhas e Caminhos de Terra da Vila (TILE_VILLAGE_PATH)
    // Saida Sul conectada ao Tronco Oco (x = 15..16, y = 19..23)
    for (int y = 19; y < h; y++) {
        for (int x = 15; x <= 16; x++) {
            m->ground_layer[y * w + x] = TILE_VILLAGE_PATH;
            m->collision_map[y * w + x] = 0;
        }
    }

    // Caminho transversal sul (y = 15..16, x = 5..26) conectando as pontes e casas do sul
    for (int y = 15; y <= 16; y++) {
        for (int x = 5; x <= 26; x++) {
            m->ground_layer[y * w + x] = TILE_VILLAGE_PATH;
        }
    }

    // Conexoes verticais sul ate as pontes
    for (int y = 13; y <= 14; y++) {
        m->ground_layer[y * w + 8]  = TILE_VILLAGE_PATH;
        m->ground_layer[y * w + 9]  = TILE_VILLAGE_PATH;
        m->ground_layer[y * w + 22] = TILE_VILLAGE_PATH;
        m->ground_layer[y * w + 23] = TILE_VILLAGE_PATH;
    }
    // Conexao vertical sul do meio
    for (int y = 17; y <= 18; y++) {
        m->ground_layer[y * w + 15] = TILE_VILLAGE_PATH;
        m->ground_layer[y * w + 16] = TILE_VILLAGE_PATH;
    }

    // Caminho transversal norte (y = 7..8, x = 5..26) conectando o santuario e casas do norte
    for (int y = 7; y <= 8; y++) {
        for (int x = 5; x <= 26; x++) {
            m->ground_layer[y * w + x] = TILE_VILLAGE_PATH;
        }
    }

    // Conexoes verticais norte ate as pontes
    for (int y = 9; y <= 10; y++) {
        m->ground_layer[y * w + 8]  = TILE_VILLAGE_PATH;
        m->ground_layer[y * w + 9]  = TILE_VILLAGE_PATH;
        m->ground_layer[y * w + 22] = TILE_VILLAGE_PATH;
        m->ground_layer[y * w + 23] = TILE_VILLAGE_PATH;
    }

    // Avenida do Santuario de Gentari ao norte (x = 15..16, y = 5..6)
    for (int y = 5; y <= 6; y++) {
        for (int x = 15; x <= 16; x++) {
            m->ground_layer[y * w + x] = TILE_VILLAGE_PATH;
        }
    }

    // 6. Santuario Sagrado do Anciao Gentari (Norte central: x = 13..18, y = 2..4)
    for (int y = 2; y <= 4; y++) {
        for (int x = 13; x <= 18; x++) {
            if (y == 4 && (x == 15 || x == 16)) {
                // Entrada aberta do santuario
                m->ground_layer[y * w + x] = TILE_VILLAGE_PATH;
            } else {
                m->overlay_layer[y * w + x] = TILE_GENTARI_SANCTUM;
                m->collision_map[y * w + x] = 1;
            }
        }
    }

    // 7. Casas de Cogumelo (Mushroom Houses)
    // Casa 1 - Noroeste (Ermida de Festari): x = 4..7, y = 4..6
    m->overlay_layer[3 * w + 5] = TILE_CHIMNEY_SMOKE;
    m->collision_map[3 * w + 5] = 1;
    for (int y = 4; y <= 5; y++) {
        for (int x = 4; x <= 7; x++) {
            m->overlay_layer[y * w + x] = TILE_MUSHROOM_CAP;
            m->collision_map[y * w + x] = 1;
        }
    }
    for (int x = 4; x <= 7; x++) {
        m->overlay_layer[6 * w + x] = TILE_MUSHROOM_STEM;
        m->collision_map[6 * w + x] = 1;
    }

    // Casa 2 - Sudeste: x = 24..27, y = 17..19
    m->overlay_layer[16 * w + 25] = TILE_CHIMNEY_SMOKE;
    m->collision_map[16 * w + 25] = 1;
    for (int y = 17; y <= 18; y++) {
        for (int x = 24; x <= 27; x++) {
            m->overlay_layer[y * w + x] = TILE_MUSHROOM_CAP;
            m->collision_map[y * w + x] = 1;
        }
    }
    for (int x = 24; x <= 27; x++) {
        m->overlay_layer[19 * w + x] = TILE_MUSHROOM_STEM;
        m->collision_map[19 * w + x] = 1;
    }

    // 8. Casas de Bolota / Noz (Acorn Houses)
    // Casa 3 - Nordeste: x = 24..27, y = 4..6
    for (int y = 4; y <= 6; y++) {
        for (int x = 24; x <= 27; x++) {
            m->overlay_layer[y * w + x] = TILE_ACORN_HOUSE;
            m->collision_map[y * w + x] = 1;
        }
    }

    // Casa 4 - Sudoeste: x = 4..7, y = 17..19
    for (int y = 17; y <= 19; y++) {
        for (int x = 4; x <= 7; x++) {
            m->overlay_layer[y * w + x] = TILE_ACORN_HOUSE;
            m->collision_map[y * w + x] = 1;
        }
    }

    // 9. Jardins, Trevos Ornamentais e Flores
    // Trevos gigantes internos decorativos
    m->overlay_layer[8 * w + 12]  = TILE_GIANT_CLOVER;
    m->collision_map[8 * w + 12]  = 1;
    m->overlay_layer[8 * w + 19]  = TILE_GIANT_CLOVER;
    m->collision_map[8 * w + 19]  = 1;
    m->overlay_layer[15 * w + 11] = TILE_GIANT_CLOVER;
    m->collision_map[15 * w + 11] = 1;
    m->overlay_layer[15 * w + 20] = TILE_GIANT_CLOVER;
    m->collision_map[15 * w + 20] = 1;

    // Flores coloridas em volta das casas
    m->overlay_layer[7 * w + 4]   = TILE_FLOWER_RED;
    m->overlay_layer[7 * w + 7]   = TILE_FLOWER_YELLOW;
    m->overlay_layer[7 * w + 24]  = TILE_FLOWER_YELLOW;
    m->overlay_layer[7 * w + 27]  = TILE_FLOWER_RED;
    m->overlay_layer[20 * w + 4]  = TILE_FLOWER_YELLOW;
    m->overlay_layer[20 * w + 7]  = TILE_FLOWER_RED;
    m->overlay_layer[20 * w + 24] = TILE_FLOWER_RED;
    m->overlay_layer[20 * w + 27] = TILE_FLOWER_YELLOW;

    // Arbustos cortaveis nas bordas dos jardins
    m->overlay_layer[5 * w + 9]   = TILE_BUSH;
    m->collision_map[5 * w + 9]   = 1;
    m->overlay_layer[5 * w + 22]  = TILE_BUSH;
    m->collision_map[5 * w + 22]  = 1;
    m->overlay_layer[18 * w + 9]  = TILE_BUSH;
    m->collision_map[18 * w + 9]  = 1;
    m->overlay_layer[18 * w + 22] = TILE_BUSH;
    m->collision_map[18 * w + 22] = 1;

    printf("[MAPA] Vila dos Minish (Picori Village) criada: %dx%d tiles (%dx%d pixels).\n",
           w, h, w * TILE_SIZE, h * TILE_SIZE);
    return m;
}

Tilemap* map_create_woods(const char* region_tag) {
    const char* tag = region_tag ? region_tag : "usa";
    char bmp_path[256];
    snprintf(bmp_path, sizeof(bmp_path), "assets/regions/%s/map_woods.bmp", tag);

    Texture* tex = texture_load_bmp(bmp_path);
    if (!tex) {
        printf("[MAPA] Mapa autentico nao encontrado (%s). Ativando mapa procedural de demonstracao.\n", bmp_path);
        return map_create_demo_world();
    }

    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) {
        texture_free(tex);
        return map_create_demo_world();
    }

    m->width = tex->width / TILE_SIZE;   // 1008 / 16 = 63
    m->height = tex->height / TILE_SIZE; // 1008 / 16 = 63
    m->ground_layer = NULL;
    m->overlay_layer = NULL;
    m->is_authentic = true;
    m->authentic_tex = tex;
    m->collision_map = (u8*)calloc((size_t)m->width * m->height, sizeof(u8));

    // Tenta carregar a matriz binaria de colisao
    char bin_path[256];
    snprintf(bin_path, sizeof(bin_path), "assets/regions/%s/map_woods_collision.bin", tag);
    FILE* fc = open_binary_asset(bin_path);
    if (fc) {
        size_t read_bytes = fread(m->collision_map, 1, (size_t)m->width * m->height, fc);
        fclose(fc);
        printf("[MAPA] Mapa autentico de Minish Woods [%s] carregado: %dx%d pixels (%dx%d tiles, %zu bytes colisao).\n",
               tag, tex->width, tex->height, m->width, m->height, read_bytes);
    } else {
        printf("[MAPA] Aviso: Arquivo de colisao nao encontrado (%s). Aplicando colisoes de borda.\n", bin_path);
        for (int y = 0; y < m->height; y++) {
            for (int x = 0; x < m->width; x++) {
                if (x <= 1 || x >= m->width - 2 || y <= 1 || y >= m->height - 2) {
                    m->collision_map[y * m->width + x] = 1;
                }
            }
        }
    }

    return m;
}

void map_set_region(Tilemap* map, const char* region_tag) {
    if (!map || !map->is_authentic) return;
    const char* tag = region_tag ? region_tag : "usa";
    char bmp_path[256];
    snprintf(bmp_path, sizeof(bmp_path), "assets/regions/%s/map_woods.bmp", tag);

    Texture* new_tex = texture_load_bmp(bmp_path);
    if (new_tex) {
        if (map->authentic_tex) {
            texture_free(map->authentic_tex);
        }
        map->authentic_tex = new_tex;
        printf("[MAPA] Textura do mapa atualizada para regiao [%s] (%dx%d)\n", tag, new_tex->width, new_tex->height);
    }
}

void map_destroy(Tilemap* map) {
    if (!map) return;
    if (map->authentic_tex) texture_free(map->authentic_tex);
    if (map->ground_layer)  free(map->ground_layer);
    if (map->overlay_layer) free(map->overlay_layer);
    if (map->collision_map) free(map->collision_map);
    free(map);
}

// ----------------------------------------------------------------------------
// CÂMERA VIRTUAL WIDESCREEN & FRUSTUM CULLING
// ----------------------------------------------------------------------------
void camera_update(Camera* cam, float target_x, float target_y, int viewport_w, int viewport_h, const Tilemap* map) {
    if (!cam || !map) return;

    cam->viewport_w = viewport_w;
    cam->viewport_h = viewport_h;

    // O centro desejado da câmera é o centro geométrico do Link (8x8 de offset)
    float desired_x = (target_x + 8.0f) - ((float)viewport_w / 2.0f);
    float desired_y = (target_y + 8.0f) - ((float)viewport_h / 2.0f);

    // Interpolação suave (Dampened Lerp 12% por frame a 60 FPS)
    cam->x += (desired_x - cam->x) * 0.12f;
    cam->y += (desired_y - cam->y) * 0.12f;

    // Limites de segurança (nunca exibe fora das bordas do mapa)
    float max_x = (float)(map->width * TILE_SIZE - viewport_w);
    float max_y = (float)(map->height * TILE_SIZE - viewport_h);

    if (cam->x < 0.0f) cam->x = 0.0f;
    if (cam->y < 0.0f) cam->y = 0.0f;
    if (cam->x > max_x) cam->x = max_x;
    if (cam->y > max_y) cam->y = max_y;
}

void map_render(const Tilemap* map, const Camera* cam) {
    if (!map || !cam) return;

    // ------------------------------------------------------------------------
    // RENDERIZADOR DO MAPA AUTÊNTICO (Zero Overhead - 1 Blit Direto a 60 FPS)
    // ------------------------------------------------------------------------
    if (map->is_authentic && map->authentic_tex) {
        texture_draw(map->authentic_tex, (int)cam->x, (int)cam->y, cam->viewport_w, cam->viewport_h, 0, 0);
        return;
    }

    // Anima a água suavemente
    s_water_timer++;
    if (s_water_timer > 12) {
        s_water_timer = 0;
        s_water_anim_frame = (s_water_anim_frame + 1) % 6;
    }

    // ------------------------------------------------------------------------
    // OTIMIZAÇÃO CRÍTICA: FRUSTUM CULLING
    // ------------------------------------------------------------------------
    // Só itera pelos tiles contidos na janela visível da câmera!
    int start_tx = (int)(cam->x / TILE_SIZE);
    int start_ty = (int)(cam->y / TILE_SIZE);
    int end_tx   = (int)((cam->x + (float)cam->viewport_w) / TILE_SIZE) + 1;
    int end_ty   = (int)((cam->y + (float)cam->viewport_h) / TILE_SIZE) + 1;

    if (start_tx < 0) start_tx = 0;
    if (start_ty < 0) start_ty = 0;
    if (end_tx > map->width)   end_tx = map->width;
    if (end_ty > map->height)  end_ty = map->height;

    for (int ty = start_ty; ty < end_ty; ty++) {
        for (int tx = start_tx; tx < end_tx; tx++) {
            int screen_x = tx * TILE_SIZE - (int)cam->x;
            int screen_y = ty * TILE_SIZE - (int)cam->y;
            int idx = ty * map->width + tx;

            // 1. Renderiza o chão (BG2)
            u8 ground = map->ground_layer[idx];
            render_metatile(screen_x, screen_y, (TileType)ground);

            // 2. Renderiza obstáculos ou decorações superiores (BG1)
            u8 overlay = map->overlay_layer[idx];
            if (overlay != 0xFF) {
                render_metatile(screen_x, screen_y, (TileType)overlay);
            }
        }
    }
}

void map_world_to_screen(const Camera* cam, float world_x, float world_y, int* out_screen_x, int* out_screen_y) {
    if (!cam) return;
    if (out_screen_x) *out_screen_x = (int)roundf(world_x - cam->x);
    if (out_screen_y) *out_screen_y = (int)roundf(world_y - cam->y);
}

bool map_is_solid(const Tilemap* map, float world_x, float world_y) {
    if (!map) return true;
    if (world_x < 0.0f || world_y < 0.0f) return true;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);

    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return true;

    return map->collision_map[ty * map->width + tx] != 0;
}

bool map_is_water(const Tilemap* map, float world_x, float world_y) {
    if (!map) return false;
    if (world_x < 0.0f || world_y < 0.0f) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);

    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    if (!map->is_authentic && map->ground_layer) {
        int idx = ty * map->width + tx;
        if (map->overlay_layer && (map->overlay_layer[idx] == TILE_VILLAGE_BRIDGE || map->overlay_layer[idx] == TILE_COBBLESTONE)) {
            return false; // Sobre a ponte de madeira ou calcamento nao e agua
        }
        u8 ground = map->ground_layer[idx];
        return (ground == TILE_WATER || ground == TILE_VILLAGE_STREAM || ground == TILE_SWAMP_WATER);
    }

    if (map->is_authentic && map->authentic_tex && map->authentic_tex->pixels) {
        int px = (int)world_x;
        int py = (int)world_y;
        if (px >= 0 && px < map->authentic_tex->width && py >= 0 && py < map->authentic_tex->height) {
            u32 c = map->authentic_tex->pixels[py * map->authentic_tex->width + px];
            u8 b = (c >> 8) & 0xFF;
            u8 g = (c >> 16) & 0xFF;
            u8 r = (c >> 24) & 0xFF;
            if (b > 120 && b > r + 25 && b >= g - 25) {
                return true;
            }
        }
    }

    return false;
}

bool map_is_climbable(const Tilemap* map, float world_x, float world_y) {
    if (!map || !map->ground_layer) return false;
    if (world_x < 0.0f || world_y < 0.0f) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;
    if (map->overlay_layer && (map->overlay_layer[idx] == TILE_CLIMBABLE_WALL || map->overlay_layer[idx] == TILE_CLIMBABLE_VINE)) {
        return true;
    }
    if (map->ground_layer[idx] == TILE_CLIMBABLE_WALL || map->ground_layer[idx] == TILE_CLIMBABLE_VINE) {
        return true;
    }
    return false;
}

bool map_is_mineral_water(const Tilemap* map, float world_x, float world_y) {
    if (!map || !map->ground_layer) return false;
    if (world_x < 0.0f || world_y < 0.0f) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;
    if (map->overlay_layer && map->overlay_layer[idx] == TILE_MINERAL_WATER) return true;
    if (map->ground_layer[idx] == TILE_MINERAL_WATER) return true;
    return false;
}

bool map_interact_grow_bean(Tilemap* map, float world_x, float world_y) {
    if (!map || !map->overlay_layer) return false;
    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int cx = tx + dx;
            int cy = ty + dy;
            if (cx < 0 || cx >= map->width || cy < 0 || cy >= map->height) continue;
            int idx = cy * map->width + cx;
            if (map->overlay_layer[idx] == TILE_MAGIC_BEAN_SPROUT) {
                // Transforma o broto e ergue a videira por 5 tiles escaláveis para o topo da escarpa!
                for (int h = 0; h < 6; h++) {
                    int vy = cy - h;
                    if (vy >= 0) {
                        int vidx = vy * map->width + cx;
                        map->overlay_layer[vidx] = TILE_CLIMBABLE_VINE;
                        map->collision_map[vidx] = 0; // Escalável!
                    }
                }
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                printf("[MAGIC BEAN] Broto de feijao regado com agua mineral de Monte Crenel! Videira gigante brotou!\n");
                return true;
            }
        }
    }
    return false;
}

bool map_interact_slash(Tilemap* map, float world_x, float world_y) {
    if (!map || !map->overlay_layer) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);

    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;

    // Cortar arbusto com a espada!
    if (map->overlay_layer[idx] == TILE_BUSH) {
        map->overlay_layer[idx] = 0xFF; // Arbusto cortado!
        map->collision_map[idx] = 0;    // Agora o Link pode passar por aqui!
        return true;
    }

    // Abrir Baú de Tesouro
    if (map->overlay_layer[idx] == TILE_CHEST_CLOSED) {
        map->overlay_layer[idx] = TILE_CHEST_OPEN;
        return true;
    }

    return false;
}

int map_interact_spin(Tilemap* map, float center_x, float center_y, float radius) {
    if (!map || !map->overlay_layer) return 0;

    int cut_count = 0;
    int min_tx = (int)((center_x - radius) / TILE_SIZE);
    int max_tx = (int)((center_x + radius) / TILE_SIZE);
    int min_ty = (int)((center_y - radius) / TILE_SIZE);
    int max_ty = (int)((center_y + radius) / TILE_SIZE);

    if (min_tx < 0) min_tx = 0;
    if (max_tx >= map->width) max_tx = map->width - 1;
    if (min_ty < 0) min_ty = 0;
    if (max_ty >= map->height) max_ty = map->height - 1;

    float r_check = radius + (TILE_SIZE * 0.45f);
    float r2 = r_check * r_check;

    for (int ty = min_ty; ty <= max_ty; ty++) {
        for (int tx = min_tx; tx <= max_tx; tx++) {
            float tile_cx = tx * TILE_SIZE + (TILE_SIZE * 0.5f);
            float tile_cy = ty * TILE_SIZE + (TILE_SIZE * 0.5f);
            float dx = tile_cx - center_x;
            float dy = tile_cy - center_y;

            if ((dx * dx + dy * dy) <= r2) {
                int idx = ty * map->width + tx;
                if (map->overlay_layer[idx] == TILE_BUSH) {
                    map->overlay_layer[idx] = 0xFF; // Arbusto cortado!
                    map->collision_map[idx] = 0;    // Desbloqueia colisão
                    cut_count++;
                } else if (map->overlay_layer[idx] == TILE_CHEST_CLOSED) {
                    map->overlay_layer[idx] = TILE_CHEST_OPEN;
                    cut_count++;
                }
            }
        }
    }
    return cut_count;
}

int map_interact_bomb(Tilemap* map, float world_x, float world_y, float radius) {
    if (!map || !map->overlay_layer) return 0;

    int destroyed_count = 0;
    int min_tx = (int)((world_x - radius) / TILE_SIZE);
    int max_tx = (int)((world_x + radius) / TILE_SIZE);
    int min_ty = (int)((world_y - radius) / TILE_SIZE);
    int max_ty = (int)((world_y + radius) / TILE_SIZE);

    if (min_tx < 0) min_tx = 0;
    if (max_tx >= map->width) max_tx = map->width - 1;
    if (min_ty < 0) min_ty = 0;
    if (max_ty >= map->height) max_ty = map->height - 1;

    float r_check = radius + (TILE_SIZE * 0.45f);
    float r2 = r_check * r_check;

    for (int ty = min_ty; ty <= max_ty; ty++) {
        for (int tx = min_tx; tx <= max_tx; tx++) {
            float tile_cx = tx * TILE_SIZE + (TILE_SIZE * 0.5f);
            float tile_cy = ty * TILE_SIZE + (TILE_SIZE * 0.5f);
            float dx = tile_cx - world_x;
            float dy = tile_cy - world_y;

            if ((dx * dx + dy * dy) <= r2) {
                int idx = ty * map->width + tx;
                u8 tile = map->overlay_layer[idx];

                if (tile == TILE_CRACKED_WALL) {
                    // Detona a parede rachada e abre o portal da passagem secreta!
                    map->overlay_layer[idx] = TILE_SECRET_ENTRANCE;
                    map->collision_map[idx] = 0; // Desobstrui passagem
                    destroyed_count++;
                } else if (tile == TILE_CRUMBLED_ROCK) {
                    // Estilhaça a rocha em pedregulhos
                    map->overlay_layer[idx] = 0xFF; // Removida
                    map->collision_map[idx] = 0;    // Desobstrui passagem
                    destroyed_count++;
                } else if (tile == TILE_BUSH) {
                    // Desintegra o arbusto na explosão
                    map->overlay_layer[idx] = 0xFF;
                    map->collision_map[idx] = 0;
                } else if (tile == TILE_CHEST_CLOSED) {
                    map->overlay_layer[idx] = TILE_CHEST_OPEN;
                }
            }
        }
    }

    if (destroyed_count > 0) {
        hal_audio_play_sound(SOUND_WALL_CRUMBLE, 1.0f, 1.0f);
        printf("[BOMB MAP] %d estruturas desmoronadas pelo impacto da explosao!\n", destroyed_count);
    }
    return destroyed_count;
}

bool map_is_pacci_charged_hole(const Tilemap* map, float world_x, float world_y) {
    if (!map) return false;
    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;
    if (map->ground_layer && map->ground_layer[idx] == TILE_PACCI_HOLE_CHARGED) return true;
    if (map->overlay_layer && map->overlay_layer[idx] == TILE_PACCI_HOLE_CHARGED) return true;
    return false;
}

bool map_discharge_pacci_hole(Tilemap* map, float world_x, float world_y) {
    if (!map) return false;
    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;
    bool found = false;
    if (map->ground_layer && map->ground_layer[idx] == TILE_PACCI_HOLE_CHARGED) {
        map->ground_layer[idx] = TILE_PACCI_HOLE_NORMAL;
        found = true;
    }
    if (map->overlay_layer && map->overlay_layer[idx] == TILE_PACCI_HOLE_CHARGED) {
        map->overlay_layer[idx] = TILE_PACCI_HOLE_NORMAL;
        found = true;
    }
    return found;
}

bool map_interact_pacci(Tilemap* map, float world_x, float world_y) {
    if (!map) return false;
    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int cx = tx + dx;
            int cy = ty + dy;
            if (cx < 0 || cx >= map->width || cy < 0 || cy >= map->height) continue;
            int idx = cy * map->width + cx;

            // 1. Energizar buraco no solo
            if ((map->ground_layer && map->ground_layer[idx] == TILE_PACCI_HOLE_NORMAL) ||
                (map->overlay_layer && map->overlay_layer[idx] == TILE_PACCI_HOLE_NORMAL)) {
                if (map->ground_layer && map->ground_layer[idx] == TILE_PACCI_HOLE_NORMAL) {
                    map->ground_layer[idx] = TILE_PACCI_HOLE_CHARGED;
                }
                if (map->overlay_layer && map->overlay_layer[idx] == TILE_PACCI_HOLE_NORMAL) {
                    map->overlay_layer[idx] = TILE_PACCI_HOLE_CHARGED;
                }
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.6f);
                printf("[CANE OF PACCI] Buraco energizado em (%d, %d)! Pronto para super-salto vertical!\n", cx, cy);
                return true;
            }

            // 2. Desvirar carrinho de mina tombado de cabeça para baixo
            if (map->overlay_layer && map->overlay_layer[idx] == TILE_MINECART_UPSIDE_DOWN) {
                map->overlay_layer[idx] = TILE_MINECART;
                map->collision_map[idx] = 0; // Desobstrui passagem / funcional
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                printf("[CANE OF PACCI] Carrinho de mina desvirado em (%d, %d) com sucesso!\n", cx, cy);
                return true;
            }
        }
    }
    return false;
}

Tilemap* map_create_south_hyrule_field(void) {
    int w = 32;
    int h = 24;
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width  = w;
    m->height = h;
    m->is_authentic = false;
    m->authentic_tex = NULL;
    m->ground_layer  = (u8*)malloc(w * h * sizeof(u8));
    m->overlay_layer = (u8*)malloc(w * h * sizeof(u8));
    m->collision_map = (u8*)malloc(w * h * sizeof(u8));

    for (int i = 0; i < w * h; i++) {
        m->ground_layer[i]  = TILE_GRASS;
        m->overlay_layer[i] = 0xFF;
        m->collision_map[i] = 0;
    }

    // Topo (linhas 0 e 1): Barreira de árvores e muros exceto passagem para a cidade (x=14..17)
    for (int x = 0; x < w; x++) {
        if (x < 14 || x > 17) {
            m->overlay_layer[0 * w + x] = TILE_STONE_WALL;
            m->collision_map[0 * w + x] = 1;
            m->overlay_layer[1 * w + x] = TILE_TREE_TRUNK;
            m->collision_map[1 * w + x] = 1;
        }
    }

    // Bordas laterais
    for (int y = 0; y < h; y++) {
        m->overlay_layer[y * w + 0] = TILE_TREE_TRUNK;
        m->collision_map[y * w + 0] = 1;
        m->overlay_layer[y * w + 1] = TILE_TREE_TRUNK;
        m->collision_map[y * w + 1] = 1;

        if (y < 10 || y > 13) {
            m->overlay_layer[y * w + (w - 2)] = TILE_TREE_TRUNK;
            m->collision_map[y * w + (w - 2)] = 1;
            m->overlay_layer[y * w + (w - 1)] = TILE_TREE_TRUNK;
            m->collision_map[y * w + (w - 1)] = 1;
        }
    }

    // Borda sul
    for (int x = 0; x < w; x++) {
        if (x < 14 || x > 17) {
            m->overlay_layer[(h - 1) * w + x] = TILE_TREE_TRUNK;
            m->collision_map[(h - 1) * w + x] = 1;
        }
    }

    // Caminhos de terra batida (Eixo Norte-Sul e ramificação Leste para Minish Woods)
    for (int y = 0; y < h; y++) {
        m->ground_layer[y * w + 15] = TILE_DIRT_PATH;
        m->ground_layer[y * w + 16] = TILE_DIRT_PATH;
    }
    for (int x = 16; x < w; x++) {
        m->ground_layer[11 * w + x] = TILE_DIRT_PATH;
        m->ground_layer[12 * w + x] = TILE_DIRT_PATH;
    }

    // Fazenda Lon Lon no sudeste (x=19..28, y=14..21)
    for (int x = 19; x <= 28; x++) {
        if (x == 22 || x == 23) {
            m->overlay_layer[14 * w + x] = TILE_FENCE_GATE;
            m->collision_map[14 * w + x] = 0;
        } else {
            m->overlay_layer[14 * w + x] = TILE_WOOD_FENCE;
            m->collision_map[14 * w + x] = 1;
        }
        m->overlay_layer[21 * w + x] = TILE_WOOD_FENCE;
        m->collision_map[21 * w + x] = 1;
    }
    for (int y = 14; y <= 21; y++) {
        m->overlay_layer[y * w + 19] = TILE_WOOD_FENCE;
        m->collision_map[y * w + 19] = 1;
        m->overlay_layer[y * w + 28] = TILE_WOOD_FENCE;
        m->collision_map[y * w + 28] = 1;
    }

    // Arbustos e flores
    m->overlay_layer[5 * w + 6]  = TILE_BUSH;
    m->overlay_layer[5 * w + 7]  = TILE_BUSH;
    m->overlay_layer[6 * w + 6]  = TILE_BUSH;
    m->overlay_layer[16 * w + 7] = TILE_FLOWER_RED;
    m->overlay_layer[16 * w + 8] = TILE_FLOWER_YELLOW;
    m->overlay_layer[8 * w + 22] = TILE_FLOWER_RED;
    m->overlay_layer[9 * w + 23] = TILE_FLOWER_YELLOW;

    // Baú protegido por rocha quebradiça
    m->overlay_layer[4 * w + 25] = TILE_CHEST_CLOSED;
    m->collision_map[4 * w + 25] = 1;
    m->overlay_layer[5 * w + 25] = TILE_CRUMBLED_ROCK;
    m->collision_map[5 * w + 25] = 1;

    printf("[MAP] South Hyrule Field (Campos Sul) criado com sucesso (%dx%d tiles)!\n", w, h);
    return m;
}

Tilemap* map_create_north_hyrule_field(void) {
    int w = 32;
    int h = 24;
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width  = w;
    m->height = h;
    m->is_authentic = false;
    m->authentic_tex = NULL;
    m->ground_layer  = (u8*)malloc(w * h * sizeof(u8));
    m->overlay_layer = (u8*)malloc(w * h * sizeof(u8));
    m->collision_map = (u8*)malloc(w * h * sizeof(u8));

    for (int i = 0; i < w * h; i++) {
        m->ground_layer[i]  = TILE_GRASS;
        m->overlay_layer[i] = 0xFF;
        m->collision_map[i] = 0;
    }

    // Topo (linhas 0 e 1): Muro norte do Castelo de Hyrule
    for (int x = 0; x < w; x++) {
        if (x < 14 || x > 17) {
            m->overlay_layer[0 * w + x] = TILE_STONE_WALL;
            m->collision_map[0 * w + x] = 1;
            m->overlay_layer[1 * w + x] = TILE_STONE_WALL;
            m->collision_map[1 * w + x] = 1;
        }
    }

    // Base sul: Entrada para Hyrule Town
    for (int x = 0; x < w; x++) {
        if (x < 14 || x > 17) {
            m->overlay_layer[(h - 2) * w + x] = TILE_STONE_WALL;
            m->collision_map[(h - 2) * w + x] = 1;
            m->overlay_layer[(h - 1) * w + x] = TILE_TREE_TRUNK;
            m->collision_map[(h - 1) * w + x] = 1;
        }
    }

    // Oeste: Colinas rochosas com passagem para Monte Crenel (y=9..12)
    for (int y = 0; y < h; y++) {
        if (y < 9 || y > 12) {
            m->overlay_layer[y * w + 0] = TILE_STONE_WALL;
            m->collision_map[y * w + 0] = 1;
            m->overlay_layer[y * w + 1] = TILE_STONE_WALL;
            m->collision_map[y * w + 1] = 1;
        }
    }
    // Leste: Bosque
    for (int y = 0; y < h; y++) {
        m->overlay_layer[y * w + (w - 1)] = TILE_TREE_TRUNK;
        m->collision_map[y * w + (w - 1)] = 1;
        m->overlay_layer[y * w + (w - 2)] = TILE_TREE_TRUNK;
        m->collision_map[y * w + (w - 2)] = 1;
    }

    // Caminho de terra: Norte-Sul
    for (int y = 0; y < h; y++) {
        m->ground_layer[y * w + 15] = TILE_DIRT_PATH;
        m->ground_layer[y * w + 16] = TILE_DIRT_PATH;
    }
    // Ramificação oeste em direção ao Monte Crenel
    for (int x = 0; x <= 15; x++) {
        m->ground_layer[10 * w + x] = TILE_DIRT_PATH;
        m->ground_layer[11 * w + x] = TILE_DIRT_PATH;
    }

    // Placa apontando para Monte Crenel
    m->overlay_layer[9 * w + 3] = TILE_CRENEL_ROAD_SIGN;
    m->collision_map[9 * w + 3] = 1;

    // Parede rachada escondendo passagem secreta
    m->overlay_layer[1 * w + 8] = TILE_CRACKED_WALL;
    m->collision_map[1 * w + 8] = 1;

    printf("[MAP] North Hyrule Field (Campos Norte) criado com sucesso (%dx%d tiles)!\n", w, h);
    return m;
}

Tilemap* map_create_mount_crenel_base(void) {
    int w = 32;
    int h = 24;
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width  = w;
    m->height = h;
    m->is_authentic = false;
    m->authentic_tex = NULL;
    m->ground_layer  = (u8*)malloc(w * h * sizeof(u8));
    m->overlay_layer = (u8*)malloc(w * h * sizeof(u8));
    m->collision_map = (u8*)malloc(w * h * sizeof(u8));

    for (int i = 0; i < w * h; i++) {
        m->ground_layer[i]  = TILE_CRENEL_GRAVEL;
        m->overlay_layer[i] = 0xFF;
        m->collision_map[i] = 0;
    }

    // Topo (linhas 0 a 2): Paredões inescaláveis do cume da montanha
    for (int y = 0; y <= 2; y++) {
        for (int x = 0; x < w; x++) {
            m->overlay_layer[y * w + x] = TILE_CRENEL_CLIFF_FACE;
            m->collision_map[y * w + x] = 1;
        }
    }

    // Base sul (linhas 22 e 23): Paredões de contenção
    for (int y = 22; y <= 23; y++) {
        for (int x = 0; x < w; x++) {
            m->overlay_layer[y * w + x] = TILE_CRENEL_CLIFF_FACE;
            m->collision_map[y * w + x] = 1;
        }
    }

    // Borda Oeste (colunas 0 e 1): Escarpas intransponíveis
    for (int y = 0; y < h; y++) {
        m->overlay_layer[y * w + 0] = TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + 0] = 1;
        m->overlay_layer[y * w + 1] = TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + 1] = 1;
    }

    // Borda Leste: Saída para os Campos Norte de Hyrule (y=10..12)
    for (int y = 0; y < h; y++) {
        if (y < 10 || y > 12) {
            m->overlay_layer[y * w + (w - 2)] = TILE_CRENEL_CLIFF_FACE;
            m->collision_map[y * w + (w - 2)] = 1;
            m->overlay_layer[y * w + (w - 1)] = TILE_CRENEL_CLIFF_FACE;
            m->collision_map[y * w + (w - 1)] = 1;
        } else {
            m->ground_layer[y * w + (w - 2)] = TILE_DIRT_PATH;
            m->ground_layer[y * w + (w - 1)] = TILE_DIRT_PATH;
        }
    }

    // Paredão Divisório Central de Rocha (y=8 e y=9) separando o vale inferior do platô superior
    for (int x = 2; x < w - 2; x++) {
        // Seções de paredão com apoios para escalada com Grip Ring (x=7..9 e x=23..25)
        if ((x >= 7 && x <= 9) || (x >= 23 && x <= 25)) {
            m->overlay_layer[8 * w + x] = TILE_CLIMBABLE_WALL;
            m->collision_map[8 * w + x] = 0; // Atravessável se tiver Grip Ring!
            m->overlay_layer[9 * w + x] = TILE_CLIMBABLE_WALL;
            m->collision_map[9 * w + x] = 0;
        } else {
            m->overlay_layer[8 * w + x] = TILE_CRENEL_CLIFF_FACE;
            m->collision_map[8 * w + x] = 1;
            m->overlay_layer[9 * w + x] = TILE_CRENEL_CLIFF_FACE;
            m->collision_map[9 * w + x] = 1;
        }
    }

    // Fonte Termal de Água Mineral Verde (y=16..19, x=5..9)
    for (int y = 16; y <= 19; y++) {
        for (int x = 5; x <= 9; x++) {
            m->ground_layer[y * w + x] = TILE_MINERAL_WATER;
            m->collision_map[y * w + x] = 1;
        }
    }

    // Broto de Feijão Mágico no sopé do paredão (x=16, y=10)
    m->overlay_layer[10 * w + 16] = TILE_MAGIC_BEAN_SPROUT;
    m->collision_map[10 * w + 16] = 1;

    // Caverna do Business Scrub (Deku Scrub) no sudeste (x=21..23, y=14)
    m->overlay_layer[14 * w + 21] = TILE_CRENEL_CLIFF_FACE;
    m->collision_map[14 * w + 21] = 1;
    m->overlay_layer[14 * w + 22] = TILE_SECRET_ENTRANCE; // Entrada escura da caverna
    m->collision_map[14 * w + 22] = 0;
    m->overlay_layer[14 * w + 23] = TILE_CRENEL_CLIFF_FACE;
    m->collision_map[14 * w + 23] = 1;

    // Parede rachada na escarpa oeste
    m->overlay_layer[9 * w + 4] = TILE_CRACKED_WALL;
    m->collision_map[9 * w + 4] = 1;

    // Platô Superior: Caminho para a forja de Melari e Caverna das Chamas
    // Rocha quebradiça bloqueando caminho estreito no platô (x=15, y=5)
    m->overlay_layer[5 * w + 15] = TILE_CRUMBLED_ROCK;
    m->collision_map[5 * w + 15] = 1;

    // Baú fechado no platô nordeste
    m->overlay_layer[4 * w + 26] = TILE_CHEST_CLOSED;
    m->collision_map[4 * w + 26] = 1;

    // Buracos no solo para energizar com o Cajado de Pacci (Super Salto Vertical)
    m->overlay_layer[10 * w + 12] = TILE_PACCI_HOLE_NORMAL; // Sopé do paredão
    m->overlay_layer[11 * w + 19] = TILE_PACCI_HOLE_NORMAL; // Perto da clareira do Deku
    m->overlay_layer[5 * w + 11]  = TILE_PACCI_HOLE_NORMAL; // Platô superior

    // Carrinho de mina tombado bloqueando a ferrovia das Minas de Melari
    m->overlay_layer[5 * w + 18] = TILE_MINECART_UPSIDE_DOWN;
    m->collision_map[5 * w + 18] = 1; // Bloqueado até ser desvirado pelo Cajado de Pacci!

    printf("[MAP] Mount Crenel Base (Sope do Monte Crenel) criado com sucesso (%dx%d tiles)!\n", w, h);
    return m;
}

bool map_is_lava(const Tilemap* map, float world_x, float world_y) {
    if (!map) return false;
    if (world_x < 0.0f || world_y < 0.0f) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);

    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;
    if (map->ground_layer && map->ground_layer[idx] == TILE_MELARI_LAVA_CHANNEL) return true;
    if (map->overlay_layer && map->overlay_layer[idx] == TILE_MELARI_LAVA_CHANNEL) return true;

    return false;
}

Tilemap* map_create_melari_mines(void) {
    int w = 32;
    int h = 24;
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width  = w;
    m->height = h;
    m->is_authentic = false;
    m->authentic_tex = NULL;
    m->ground_layer  = (u8*)malloc(w * h * sizeof(u8));
    m->overlay_layer = (u8*)malloc(w * h * sizeof(u8));
    m->collision_map = (u8*)malloc(w * h * sizeof(u8));

    for (int i = 0; i < w * h; i++) {
        m->ground_layer[i]  = TILE_MELARI_STONE_FLOOR;
        m->overlay_layer[i] = 0xFF;
        m->collision_map[i] = 0;
    }

    // Paredes da caverna circundante (rocha vulcânica escura)
    // Topo (linhas 0 e 1)
    for (int y = 0; y <= 1; y++) {
        for (int x = 0; x < w; x++) {
            m->overlay_layer[y * w + x] = TILE_CRENEL_CLIFF_FACE;
            m->collision_map[y * w + x] = 1;
        }
    }

    // Fundo sul (linha 23): Saída para Mount Crenel Base (x=14..17 livre)
    for (int x = 0; x < w; x++) {
        if (x < 14 || x > 17) {
            m->overlay_layer[23 * w + x] = TILE_CRENEL_CLIFF_FACE;
            m->collision_map[23 * w + x] = 1;
        }
    }

    // Paredes laterais Oeste (colunas 0 e 1) e Leste (colunas 30 e 31) com veios de minério
    for (int y = 0; y < h; y++) {
        m->overlay_layer[y * w + 0] = TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + 0] = 1;
        m->overlay_layer[y * w + 1] = (y % 3 == 0) ? TILE_MELARI_ORE_VEIN : TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + 1] = 1;

        m->overlay_layer[y * w + (w - 2)] = (y % 3 == 1) ? TILE_MELARI_ORE_VEIN : TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + (w - 2)] = 1;
        m->overlay_layer[y * w + (w - 1)] = TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + (w - 1)] = 1;
    }

    // Portal em arco ao norte: Entrada da Cave of Flames (Dungeon 2) em x=15..16, y=1
    m->overlay_layer[1 * w + 15] = TILE_MELARI_CAVE_ARCHWAY;
    m->collision_map[1 * w + 15] = 0; // Passagem
    m->overlay_layer[1 * w + 16] = TILE_MELARI_CAVE_ARCHWAY;
    m->collision_map[1 * w + 16] = 0;

    // Canais de Lava Incandescente (aquecendo os foles e forja):
    // Canal Oeste (y=7..8, x=3..11)
    for (int y = 7; y <= 8; y++) {
        for (int x = 3; x <= 11; x++) {
            m->ground_layer[y * w + x] = TILE_MELARI_LAVA_CHANNEL;
            m->collision_map[y * w + x] = 1;
        }
    }
    // Canal Leste (y=7..8, x=20..28)
    for (int y = 7; y <= 8; y++) {
        for (int x = 20; x <= 28; x++) {
            m->ground_layer[y * w + x] = TILE_MELARI_LAVA_CHANNEL;
            m->collision_map[y * w + x] = 1;
        }
    }

    // Plataforma central da Forja de Melari (x=13..18, y=4..10):
    // Bigorna monumental e pedestal da forja em x=15..16, y=7
    m->overlay_layer[7 * w + 15] = TILE_MELARI_FORGE_ANVIL;
    m->collision_map[7 * w + 15] = 1;
    m->overlay_layer[7 * w + 16] = TILE_MELARI_FORGE_ANVIL;
    m->collision_map[7 * w + 16] = 1;

    // Ferrovia / Trilhos de mineração:
    // Trilho principal vertical sul (x=16, y=14..23)
    for (int y = 14; y <= 23; y++) {
        m->ground_layer[y * w + 16] = TILE_MELARI_MINING_TRACK;
    }
    // Ramal de trilhos leste para o veio de minério (y=18, x=17..27)
    for (int x = 17; x <= 27; x++) {
        m->ground_layer[18 * w + x] = TILE_MELARI_MINING_TRACK;
    }
    // Carrinho de mina funcional no ramal leste (x=24, y=18)
    m->overlay_layer[18 * w + 24] = TILE_MINECART;
    m->collision_map[18 * w + 24] = 1;

    // Pilhas de caixotes de ferramentas (x=6..7, y=16)
    m->overlay_layer[16 * w + 6] = TILE_BARREL_CRATE;
    m->collision_map[16 * w + 6] = 1;
    m->overlay_layer[16 * w + 7] = TILE_BARREL_CRATE;
    m->collision_map[16 * w + 7] = 1;

    // Baú fechado com tesouro nos fundos noroeste (x=4, y=4)
    m->overlay_layer[4 * w + 4] = TILE_CHEST_CLOSED;
    m->collision_map[4 * w + 4] = 1;

    printf("[MAP] Melari's Mines (Minas de Melari) criado com sucesso (%dx%d tiles)!\n", w, h);
    return m;
}

bool map_is_swamp_mud(const Tilemap* map, float world_x, float world_y) {
    if (!map || !map->ground_layer) return false;
    if (world_x < 0.0f || world_y < 0.0f) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;
    if (map->overlay_layer && (map->overlay_layer[idx] == TILE_SWAMP_LOG || map->overlay_layer[idx] == TILE_VILLAGE_BRIDGE)) {
        return false; // Tronco ou ponte protege o herói de afundar no lodo
    }
    return (map->ground_layer[idx] == TILE_SWAMP_MUD || (map->overlay_layer && map->overlay_layer[idx] == TILE_SWAMP_MUD));
}

bool map_is_eye_statue(const Tilemap* map, float world_x, float world_y) {
    if (!map || !map->overlay_layer) return false;
    if (world_x < 0.0f || world_y < 0.0f) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;
    return (map->overlay_layer[idx] == TILE_EYE_STATUE || map->overlay_layer[idx] == TILE_EYE_STATUE_OPEN);
}

bool map_hit_eye_statue(Tilemap* map, float world_x, float world_y) {
    if (!map || !map->overlay_layer) return false;
    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int cx = tx + dx;
            int cy = ty + dy;
            if (cx < 0 || cx >= map->width || cy < 0 || cy >= map->height) continue;
            int idx = cy * map->width + cx;
            if (map->overlay_layer[idx] == TILE_EYE_STATUE) {
                map->overlay_layer[idx] = TILE_EYE_STATUE_OPEN;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                printf("[EYE STATUE] Estatua ancestral de olho aberta com flecha certeira! (%d, %d)\n", cx, cy);
                return true;
            }
        }
    }
    return false;
}

Tilemap* map_create_castor_wilds(void) {
    int w = 36;
    int h = 28;
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width  = w;
    m->height = h;
    m->is_authentic = false;
    m->authentic_tex = NULL;
    m->ground_layer  = (u8*)malloc(w * h * sizeof(u8));
    m->overlay_layer = (u8*)malloc(w * h * sizeof(u8));
    m->collision_map = (u8*)malloc(w * h * sizeof(u8));

    // 1. Preenche o terreno inicial com vegetação pantanosa (TILE_SWAMP_GRASS)
    for (int i = 0; i < w * h; i++) {
        m->ground_layer[i]  = TILE_SWAMP_GRASS;
        m->overlay_layer[i] = 0xFF;
        m->collision_map[i] = 0;
    }

    // 2. Fronteiras densas de árvores e vinhedos do pântano
    // Norte (y=0..1)
    for (int x = 0; x < w; x++) {
        m->overlay_layer[0 * w + x] = TILE_TREE_TOP;
        m->overlay_layer[1 * w + x] = TILE_TREE_TRUNK;
        m->collision_map[0 * w + x] = 1;
        m->collision_map[1 * w + x] = 1;
    }
    // Sul (y=26..27)
    for (int x = 0; x < w; x++) {
        m->overlay_layer[26 * w + x] = TILE_TREE_TOP;
        m->overlay_layer[27 * w + x] = TILE_TREE_TRUNK;
        m->collision_map[26 * w + x] = 1;
        m->collision_map[27 * w + x] = 1;
    }
    // Oeste (x=0..1)
    for (int y = 0; y < h; y++) {
        m->overlay_layer[y * w + 0] = TILE_TREE_TRUNK;
        m->collision_map[y * w + 0] = 1;
        m->overlay_layer[y * w + 1] = TILE_TREE_TRUNK;
        m->collision_map[y * w + 1] = 1;
    }
    // Leste (x=34..35) com passagem aberta em y=12..16 (Conexão para South Hyrule Field)
    for (int y = 0; y < h; y++) {
        if (y < 12 || y > 16) {
            m->overlay_layer[y * w + 34] = TILE_TREE_TRUNK;
            m->collision_map[y * w + 34] = 1;
            m->overlay_layer[y * w + 35] = TILE_TREE_TRUNK;
            m->collision_map[y * w + 35] = 1;
        }
    }

    // 3. Grandes charcos de lodo movediço (TILE_SWAMP_MUD)
    // Lago de Lodo Central (x=8..26, y=6..16)
    for (int y = 6; y <= 16; y++) {
        for (int x = 8; x <= 26; x++) {
            m->ground_layer[y * w + x] = TILE_SWAMP_MUD;
        }
    }
    // Lago de Lodo Sudoeste (x=4..14, y=18..24)
    for (int y = 18; y <= 24; y++) {
        for (int x = 4; x <= 14; x++) {
            m->ground_layer[y * w + x] = TILE_SWAMP_MUD;
        }
    }

    // Ilhotas de musgo sólido dentro do charco de lodo
    for (int y = 10; y <= 12; y++) {
        for (int x = 16; x <= 18; x++) {
            m->ground_layer[y * w + x] = TILE_SWAMP_GRASS;
        }
    }
    m->overlay_layer[11 * w + 17] = TILE_BUSH;
    m->collision_map[11 * w + 17] = 1;

    // 4. Troncos caídos ocos (passarelas sobre o lodo)
    // Tronco horizontal que corta o charco central (x=12..15, y=9)
    for (int x = 12; x <= 15; x++) {
        m->overlay_layer[9 * w + x] = TILE_SWAMP_LOG;
        m->collision_map[9 * w + x] = 0; // Passável a pé!
    }
    // Tronco horizontal no charco sudoeste (x=7..10, y=21)
    for (int x = 7; x <= 10; x++) {
        m->overlay_layer[21 * w + x] = TILE_SWAMP_LOG;
        m->collision_map[21 * w + x] = 0;
    }

    // 5. Charco de água escura do pântano no sudeste (x=21..31, y=19..24)
    for (int y = 19; y <= 24; y++) {
        for (int x = 21; x <= 31; x++) {
            m->ground_layer[y * w + x] = TILE_SWAMP_WATER;
            m->collision_map[y * w + x] = 1; // Água profunda (requer flippers para nadar)
        }
    }

    // 6. Ruínas Ancestrais e Pedestal do Arco e Flechas no Noroeste (x=4..10, y=3..6)
    // Pedestal sagrado guardado por estátuas de olho
    m->overlay_layer[3 * w + 5] = TILE_EYE_STATUE;
    m->collision_map[3 * w + 5] = 1;
    m->overlay_layer[3 * w + 9] = TILE_EYE_STATUE;
    m->collision_map[3 * w + 9] = 1;

    // Baú contendo o Arco e Flechas no pedestal central das ruínas
    m->overlay_layer[3 * w + 7] = TILE_CHEST_CLOSED;
    m->collision_map[3 * w + 7] = 1;

    // Estátua de olho bloqueando atalho no lodo em x=13, y=13
    m->overlay_layer[13 * w + 13] = TILE_EYE_STATUE;
    m->collision_map[13 * w + 13] = 1;

    // Arbustos pantanosos espalhados
    m->overlay_layer[5 * w + 14]  = TILE_BUSH;
    m->collision_map[5 * w + 14]  = 1;
    m->overlay_layer[5 * w + 20]  = TILE_BUSH;
    m->collision_map[5 * w + 20]  = 1;
    m->overlay_layer[14 * w + 28] = TILE_BUSH;
    m->collision_map[14 * w + 28] = 1;
    m->overlay_layer[17 * w + 18] = TILE_BUSH;
    m->collision_map[17 * w + 18] = 1;

    printf("[MAP] Castor Wilds Swamp (Pantano de Castor Wilds) criado com sucesso (%dx%d tiles)!\n", w, h);
    return m;
}

bool map_is_diggable(const Tilemap* map, float world_x, float world_y) {
    if (!map) return false;
    if (world_x < 0.0f || world_y < 0.0f) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;
    if (map->overlay_layer && (map->overlay_layer[idx] == TILE_DIRT_WALL || map->overlay_layer[idx] == TILE_DIRT_MOUND)) {
        return true;
    }
    if (map->ground_layer && (map->ground_layer[idx] == TILE_DIRT_WALL || map->ground_layer[idx] == TILE_DIRT_MOUND)) {
        return true;
    }
    return false;
}

bool map_dig_tile(Tilemap* map, float world_x, float world_y, int* out_drop) {
    if (!map) return false;
    if (world_x < 0.0f || world_y < 0.0f) return false;

    int tx = (int)(world_x / TILE_SIZE);
    int ty = (int)(world_y / TILE_SIZE);
    if (tx < 0 || tx >= map->width || ty < 0 || ty >= map->height) return false;

    int idx = ty * map->width + tx;
    bool dug = false;

    if (map->overlay_layer && (map->overlay_layer[idx] == TILE_DIRT_WALL || map->overlay_layer[idx] == TILE_DIRT_MOUND)) {
        map->overlay_layer[idx] = TILE_DIRT_WALL_TUNNEL;
        if (map->collision_map) map->collision_map[idx] = 0; // Desobstrui passagem
        dug = true;
    } else if (map->ground_layer && (map->ground_layer[idx] == TILE_DIRT_WALL || map->ground_layer[idx] == TILE_DIRT_MOUND)) {
        map->ground_layer[idx] = TILE_DIRT_WALL_TUNNEL;
        if (map->collision_map) map->collision_map[idx] = 0;
        dug = true;
    }

    if (dug) {
        hal_audio_play_sound(SOUND_WALL_CRUMBLE, 0.85f, 1.30f);
        if (out_drop) {
            int roll = rand() % 100;
            if (roll < 35)      *out_drop = 1; // Rupee verde (+1)
            else if (roll < 55) *out_drop = 2; // Rupee azul (+5)
            else if (roll < 75) *out_drop = 3; // Coração (+1 HP)
            else if (roll < 90) *out_drop = 4; // Fragmento de Kinstone
            else                *out_drop = 0; // Nada
        }
        return true;
    }
    return false;
}

Tilemap* map_create_mole_cave(void) {
    int w = 32;
    int h = 24;
    Tilemap* m = (Tilemap*)malloc(sizeof(Tilemap));
    if (!m) return NULL;

    m->width  = w;
    m->height = h;
    m->is_authentic = false;
    m->authentic_tex = NULL;
    m->ground_layer  = (u8*)malloc(w * h * sizeof(u8));
    m->overlay_layer = (u8*)malloc(w * h * sizeof(u8));
    m->collision_map = (u8*)malloc(w * h * sizeof(u8));

    // 1. Chão da caverna de terra batida (TILE_DIRT_WALL_TUNNEL)
    for (int i = 0; i < w * h; i++) {
        m->ground_layer[i]  = TILE_DIRT_WALL_TUNNEL;
        m->overlay_layer[i] = 0xFF;
        m->collision_map[i] = 0;
    }

    // 2. Paredes de rocha impenetráveis no perímetro
    // Topo (y=0..1)
    for (int y = 0; y <= 1; y++) {
        for (int x = 0; x < w; x++) {
            m->overlay_layer[y * w + x] = TILE_CRENEL_CLIFF_FACE;
            m->collision_map[y * w + x] = 1;
        }
    }
    // Fundo (y=22..23) com saída central em x=15..16
    for (int y = 22; y <= 23; y++) {
        for (int x = 0; x < w; x++) {
            if (x < 14 || x > 17) {
                m->overlay_layer[y * w + x] = TILE_CRENEL_CLIFF_FACE;
                m->collision_map[y * w + x] = 1;
            }
        }
    }
    // Laterais Oeste (x=0..1) e Leste (x=30..31)
    for (int y = 0; y < h; y++) {
        m->overlay_layer[y * w + 0] = TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + 0] = 1;
        m->overlay_layer[y * w + 1] = TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + 1] = 1;
        m->overlay_layer[y * w + (w - 2)] = TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + (w - 2)] = 1;
        m->overlay_layer[y * w + (w - 1)] = TILE_CRENEL_CLIFF_FACE;
        m->collision_map[y * w + (w - 1)] = 1;
    }

    // 3. Paredes maciças de terra fofa escavável (TILE_DIRT_WALL)
    // Bloco Oeste (x=3..13, y=4..20)
    for (int y = 4; y <= 20; y++) {
        for (int x = 3; x <= 13; x++) {
            m->overlay_layer[y * w + x] = TILE_DIRT_WALL;
            m->collision_map[y * w + x] = 1; // Sólido até escavar com as Mole Mitts!
        }
    }
    // Bloco Leste (x=18..28, y=4..20)
    for (int y = 4; y <= 20; y++) {
        for (int x = 18; x <= 28; x++) {
            m->overlay_layer[y * w + x] = TILE_DIRT_WALL;
            m->collision_map[y * w + x] = 1;
        }
    }

    // Câmaras secretas escavadas dentro dos blocos de terra:
    // Câmara Secreta Oeste (x=6..8, y=8..10) com Baú de 50 Rupees
    for (int y = 8; y <= 10; y++) {
        for (int x = 6; x <= 8; x++) {
            m->overlay_layer[y * w + x] = 0xFF;
            m->collision_map[y * w + x] = 0;
        }
    }
    m->overlay_layer[9 * w + 7] = TILE_CHEST_CLOSED;
    m->collision_map[9 * w + 7] = 1;

    // Câmara Secreta Leste (x=23..25, y=8..10) com Baú de Kinstones
    for (int y = 8; y <= 10; y++) {
        for (int x = 23; x <= 25; x++) {
            m->overlay_layer[y * w + x] = 0xFF;
            m->collision_map[y * w + x] = 0;
        }
    }
    m->overlay_layer[9 * w + 24] = TILE_CHEST_CLOSED;
    m->collision_map[9 * w + 24] = 1;

    // Montículos de terra fofa espalhados no corredor central
    m->overlay_layer[6 * w + 15]  = TILE_DIRT_MOUND;
    m->collision_map[6 * w + 15]  = 1;
    m->overlay_layer[12 * w + 16] = TILE_DIRT_MOUND;
    m->collision_map[12 * w + 16] = 1;
    m->overlay_layer[17 * w + 15] = TILE_DIRT_MOUND;
    m->collision_map[17 * w + 15] = 1;

    // Pedestal ancestral das Luvas de Toupeira na câmara norte (x=15..16, y=3)
    m->overlay_layer[3 * w + 15] = TILE_CHEST_CLOSED;
    m->collision_map[3 * w + 15] = 1;

    printf("[MAP] Mole Mitts Cavern (Caverna de Escavacao) criada com sucesso (%dx%d tiles)!\n", w, h);
    return m;
}
