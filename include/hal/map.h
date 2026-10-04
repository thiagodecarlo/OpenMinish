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
    TILE_COBBLESTONE,    // Calçamento de pedras polidas da Cidade de Hyrule
    TILE_TOWN_WALL,      // Fachada e paredes de alvenaria/madeira da cidade
    TILE_ROOF_RED,       // Telhado de telhas vermelhas de terracota
    TILE_ROOF_BLUE,      // Telhado azul da Loja do Stockwell
    TILE_TOWN_DOOR,      // Porta de madeira da loja/casas com maçaneta de latão
    TILE_TOWN_WINDOW,    // Janela com venezianas e vidros reflexivos
    TILE_FOUNTAIN_EDGE,  // Borda esculpida em mármore da fonte central
    TILE_MARKET_STALL,   // Toldo listrado e balcão do mercado
    TILE_BARREL_CRATE,   // Caixotes e barris de provisões do armazém
    TILE_VILLAGE_PATH,   // Caminho de terra macia e musgo verde da Vila Minish
    TILE_MUSHROOM_CAP,   // Chapéu de cogumelo vermelho com bolinhas brancas (telhado das casas)
    TILE_MUSHROOM_STEM,  // Tronco/caule da casa cogumelo com porta entalhada
    TILE_ACORN_HOUSE,    // Casa cúpula feita de noz/bolota esculpida
    TILE_VILLAGE_STREAM, // Riacho de águas rasas cristalinas que corta a vila
    TILE_VILLAGE_BRIDGE, // Pontinha de pranchas rústicas de madeira sobre o riacho
    TILE_GIANT_CLOVER,   // Trevo gigante que se ergue sobre os pequeninos
    TILE_CHIMNEY_SMOKE,  // Chaminé com fumaça animada das lareiras minish
    TILE_GENTARI_SANCTUM,// Santuário sagrado do Ancião Gentari com runas douradas
    TILE_CRACKED_WALL,    // Parede de pedra com fissuras destrutível por bombas
    TILE_CRUMBLED_ROCK,   // Rocha quebradiça bloqueando caminho, destrutível por bombas
    TILE_SECRET_ENTRANCE, // Passagem secreta revelada após a explosão da parede rachada
    TILE_FENCE_GATE,      // Portão de madeira da Fazenda Lon Lon
    TILE_CRENEL_ROAD_SIGN,// Placa de madeira indicando caminho para Monte Crenel
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
 * Cria e constrói a metrópole central do reino: A Cidade de Hyrule (Hyrule Town Hub)
 * Dimensões: 36x28 tiles (576x448 pixels) contendo a praça central com chafariz,
 * calçamento de pedras (cobblestone), o Mercado e Loja do Stockwell, residências,
 * canteiros de flores e portões para o Castelo de Hyrule e Minish Woods.
 */
Tilemap* map_create_hyrule_town(void);

/*
 * Constrói o vilarejo secreto dos Picori: A Vila dos Minish (Minish Village)
 * Dimensões: 32x24 tiles (512x384 pixels) contendo casas em formato de cogumelo e bolota,
 * riacho cristalino com pontes de madeira rústica, trevos gigantes e o Santuário de Gentari.
 */
Tilemap* map_create_minish_village(void);

/*
 * Constrói as planícies verdejantes do Sul de Hyrule (South Hyrule Field)
 * Dimensões: 32x24 tiles (512x384 pixels) conectando a saída sul da Cidade de Hyrule,
 * o bosque de Minish Woods a leste e a entrada cercada da Fazenda Lon Lon.
 */
Tilemap* map_create_south_hyrule_field(void);

/*
 * Constrói os campos do Norte de Hyrule (North Hyrule Field)
 * Dimensões: 32x24 tiles (512x384 pixels) conectando o portão norte da cidade,
 * a estrada rochosa para a base do Monte Crenel e os jardins do Castelo.
 */
Tilemap* map_create_north_hyrule_field(void);

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
 * Retorna true se o ponto no mundo contiver água (TILE_WATER, TILE_VILLAGE_STREAM ou lago autêntico).
 */
bool map_is_water(const Tilemap* map, float world_x, float world_y);

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

/*
 * Detonação de bomba interagindo com o cenário no ponto da explosão:
 * Destrói paredes rachadas (TILE_CRACKED_WALL -> TILE_SECRET_ENTRANCE),
 * estilhaça rochas (TILE_CRUMBLED_ROCK -> 0)
 * e corta arbustos próximos.
 * Retorna a quantidade de estruturas desmoronadas.
 */
int map_interact_bomb(Tilemap* map, float world_x, float world_y, float radius);

#endif // HAL_MAP_H
