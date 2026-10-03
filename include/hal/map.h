#ifndef HAL_MAP_H
#define HAL_MAP_H

/*
 * ============================================================================
 * include/hal/map.h - Engine de Mapas, Tilemaps e Câmera Widescreen
 * ============================================================================
 * Modela a PPU do GBA em Modo Tilemap (matrizes de blocos 16x16 / 8x8)
 * e implementa Câmera Virtual suave com Frustum Culling e suporte a Widescreen 16:9.
 */

#include "gba/types.h"
#include "hal/texture.h"
#include <stdbool.h>

#define TILE_SIZE 16 // Dimensão padrão dos metatiles de Minish Cap (16x16 pixels)

// Tipos de ladrilhos (Tiles) do cenário
typedef enum {
    TILE_GRASS = 0,       // Grama viva verde
    TILE_DIRT_PATH,      // Caminho de terra batida / cascalho
    TILE_WATER,          // Água profunda com ondas
    TILE_STONE_WALL,     // Paredes de pedra / ruínas
    TILE_BUSH,           // Arbusto cortável com espada
    TILE_FLOWER_RED,     // Flores vermelhas decorativas
    TILE_FLOWER_YELLOW,  // Flores amarelas
    TILE_TREE_TOP,       // Copa da árvore
    TILE_TREE_TRUNK,     // Tronco da árvore (sólido)
    TILE_CHEST_CLOSED,   // Baú de tesouro fechado
    TILE_CHEST_OPEN,     // Baú de tesouro aberto
    TILE_WOOD_FENCE,     // Cerca de madeira
    TILE_COUNT
} TileType;

// Estrutura da Câmera Virtual com interpolação suave
typedef struct {
    float x;             // Posição X da câmera no mundo (sub-pixel float)
    float y;             // Posição Y da câmera no mundo (sub-pixel float)
    int   viewport_w;    // Largura do visor (240 no GBA padrão, 284 no Widescreen 16:9)
    int   viewport_h;    // Altura do visor (160 fixos)
} Camera;

// Estrutura do Mapa de Cenário (Tilemap bidimensional ou Mapa Autêntico do GBA)
typedef struct {
    int      width;          // Largura em tiles (ex: 40 tiles = 640 pixels, ou 63 tiles = 1008 pixels)
    int      height;         // Altura em tiles  (ex: 30 tiles = 480 pixels, ou 63 tiles = 1008 pixels)
    u8*      ground_layer;   // Camada de chão (BG2 - Grama, Caminhos, Água)
    u8*      overlay_layer;  // Camada de obstáculos e decorações (BG1 - Arbustos, Baús, Troncos)
    u8*      collision_map;  // 0 = livre, 1 = barreira intransponível
    bool     is_authentic;   // true se renderiza a partir da textura autêntica da ROM
    Texture* authentic_tex;  // Textura do mapa autêntico (ex: 1008x1008 pixels)
} Tilemap;

/*
 * Cria e preenche um mapa de demonstração do overworld de Minish Cap (40x30 tiles = 640x480 pixels)
 * contendo caminhos, árvores, lago, arbustos cortáveis e baú secreto.
 */
Tilemap* map_create_demo_world(void);

/*
 * Carrega o mapa de Minish Woods para a região ativa (ex: "usa", "eur", "jpn").
 * Se os assets autênticos (map_woods.bmp e map_woods_collision.bin) existirem,
 * ativa o modo de renderização autêntica com colisão precisa.
 * Se não existirem, faz fallback gracioso para o mapa de demonstração procedural.
 */
Tilemap* map_create_woods(const char* region_tag);

/*
 * Atualiza a textura da região ativa no mapa autêntico (sem resetar posição ou estado).
 */
void map_set_region(Tilemap* map, const char* region_tag);

/*
 * Libera a memória do mapa.
 */
void map_destroy(Tilemap* map);

/*
 * Atualiza a posição da câmera para seguir o Link com interpolação suave (Lerp)
 * e aplica Frustum Clamping para nunca mostrar fora das bordas do mapa.
 */
void camera_update(Camera* cam, float target_x, float target_y, int viewport_w, int viewport_h, const Tilemap* map);

/*
 * Renderiza o mapa com Frustum Culling inteligente (desenha apenas os tiles
 * visíveis na janela da câmera, economizando 80% do custo de desenho).
 */
void map_render(const Tilemap* map, const Camera* cam);

/*
 * Converte coordenadas do mundo do jogo para coordenadas de tela da câmera.
 */
void map_world_to_screen(const Camera* cam, float world_x, float world_y, int* out_screen_x, int* out_screen_y);

/*
 * Verifica se um ponto no mundo colide com um obstáculo sólido do mapa.
 */
bool map_is_solid(const Tilemap* map, float world_x, float world_y);

/*
 * Interage com o cenário no ponto do golpe de espada (ex: corta arbusto e revela item).
 * Retorna true se um arbusto foi cortado ou baú foi aberto.
 */
bool map_interact_slash(Tilemap* map, float world_x, float world_y);

/*
 * Interage com o cenário em um raio circular de 360 graus (Ataque Giratório / Spin Attack).
 * Corta todos os arbustos e abre baús dentro do raio especificado em torno do centro.
 * Retorna a quantidade de elementos afetados.
 */
int map_interact_spin(Tilemap* map, float center_x, float center_y, float radius);

#endif // HAL_MAP_H
