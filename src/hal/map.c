#include "hal/map.h"
#include "hal/video.h"
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
