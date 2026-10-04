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
    TILE_CRENEL_CLIFF_FACE,// Paredão de rocha vulcânica escarpado de Monte Crenel (sólido intransponível)
    TILE_CLIMBABLE_WALL,  // Paredão rochoso com fissuras e saliências escalável com o Grip Ring
    TILE_MINERAL_WATER,   // Água mineral termal verde efervescente de Monte Crenel
    TILE_MAGIC_BEAN_SPROUT,// Broto de feijão mágico plantado na terra fofa
    TILE_CLIMBABLE_VINE,  // Caule/videira gigante de feijão escalável que alcança o platô
    TILE_CRENEL_GRAVEL,   // Solo terroso de cascalho e pedriscos vulcânicos
    TILE_PACCI_HOLE_NORMAL,  // Buraco/cova no chão suscetível ao Cajado de Pacci
    TILE_PACCI_HOLE_CHARGED, // Buraco energizado pelo Cajado de Pacci (catapulta Link no ar)
    TILE_MINECART_UPSIDE_DOWN,// Carrinho de mina tombado/de cabeça para baixo
    TILE_MINECART,           // Carrinho de mina desvirado funcional nos trilhos
    TILE_MELARI_STONE_FLOOR, // Piso subterrâneo de lajotas de pedra das minas
    TILE_MELARI_FORGE_ANVIL, // Bigorna de forja do Mestre Melari sobre pedestal com brasas
    TILE_MELARI_LAVA_CHANNEL,// Canal de lava incandescente borbulhante (dano de fogo)
    TILE_MELARI_ORE_VEIN,    // Veio de minério com cristais e gemas brilhantes incrustadas
    TILE_MELARI_MINING_TRACK,// Trilhos da ferrovia de mineração com dormentes de madeira
    TILE_MELARI_CAVE_ARCHWAY,// Portal de pedra em arco ao norte (entrada para Cave of Flames)
    TILE_SWAMP_WATER,        // Água escura rasa do pântano de Castor Wilds
    TILE_SWAMP_MUD,          // Lodo movediço profundo (requer Pegasus Dash para atravessar sem afundar!)
    TILE_SWAMP_GRASS,        // Musgo e vegetação pantanosa verde-oliva
    TILE_SWAMP_LOG,          // Tronco de árvore caído oco no lodo (passarela / túnel)
    TILE_EYE_STATUE,         // Estátua ancestral de pedra com olho fechado (ativa com Flecha!)
    TILE_EYE_STATUE_OPEN,    // Estátua de olho aberta por disparo certeiro de flecha
    TILE_DIRT_WALL,          // Parede de terra fofa escavável pelas Luvas de Toupeira (Mole Mitts)
    TILE_DIRT_WALL_TUNNEL,   // Piso de túnel escavado com marcas de garras
    TILE_DIRT_MOUND,         // Montículo de terra fofa escavável
    TILE_WIND_RUINS_STONE,   // Laje de pedra das Wind Ruins com musgo e rachaduras
    TILE_WIND_RUINS_WALL,    // Parede de alvenaria ancestral das ruínas
    TILE_WIND_PILLAR,        // Coluna/pilar dórico de pedra esculpida
    TILE_WIND_FORTRESS_GATE, // Portão em arco de entrada da Fortaleza dos Ventos (Fortress of Winds)
    TILE_ARMOS_CIRCUIT_FLOOR,// Piso de cobre e engrenagens do interior do robô Armos
    TILE_ARMOS_CIRCUIT_WALL, // Parede de circuitos elétricos e bobinas de bronze
    TILE_ARMOS_CIRCUIT_SWITCH,// Alavanca de ignição do circuito do Armos
    TILE_FORTRESS_FLOOR,     // Laje de piso da Fortaleza dos Ventos (pedra turquesa/cinza com relevos)
    TILE_FORTRESS_WALL,      // Paredão da Fortaleza dos Ventos com frisos e relevos aerodinâmicos
    TILE_FORTRESS_PIT,       // Abismo escuro com correntes de ar ascendentes
    TILE_FORTRESS_EYE_CLOSED,// Estátua de olho na parede da fortaleza (fechado)
    TILE_FORTRESS_EYE_OPEN,  // Estátua de olho aberta por flecha (ativa ponte ou porta)
    TILE_FORTRESS_BOSS_DOOR, // Portão dourado do Chefe Mazaal
    TILE_WIND_CREST,         // Placa de pedra com símbolo de crista de vento (Wind Crest para Zeffa)
    TILE_BOOKSHELF,          // Estante de madeira maciça da Biblioteca de Hyrule com tomos coloridos
    TILE_BOOK_STACK,         // Pilha monumental de livros gigantes formando escada para Link Minish
    TILE_LIBRARY_CARPET,     // Tapete nobre aveludado vermelho com frisos dourados da Biblioteca
    TILE_ICE_CAVERN_ENTRANCE,// Portal de gelo translúcido congelado (entrada do Temple of Droplets em Lake Hylia)
    TILE_TORCH_UNLIT,        // Pira/tocha de pedra apagada (acendível pela Flame Lantern)
    TILE_TORCH_LIT,          // Tocha de pedra acesa com chamas incandescentes animadas
    TILE_ICE_BLOCK,          // Bloco maciço de gelo translúcido derretível pela Flame Lantern
    TILE_COBWEB,             // Teia de aranha espessa inflamável pela Flame Lantern
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
 * Constrói o sopé e paredões rochosos do Monte Crenel (Mount Crenel Base)
 * Dimensões: 32x24 tiles (512x384 pixels) contendo paredões escaláveis com Grip Ring,
 * fontes de água mineral termal, broto de feijão mágico e caverna do Deku Scrub.
 */
Tilemap* map_create_mount_crenel_base(void);

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
void render_metatile(int sx, int sy, TileType type);

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
 * Retorna true se o ponto no mundo for um paredão ou videira escalável (TILE_CLIMBABLE_WALL ou TILE_CLIMBABLE_VINE).
 */
bool map_is_climbable(const Tilemap* map, float world_x, float world_y);

/*
 * Retorna true se o ponto contiver água mineral termal de Monte Crenel (TILE_MINERAL_WATER).
 */
bool map_is_mineral_water(const Tilemap* map, float world_x, float world_y);

/*
 * Rega e faz brotar o feijão mágico em Monte Crenel, transformando-o em videira gigante escalável.
 * Retorna true se regou um broto com sucesso.
 */
bool map_interact_grow_bean(Tilemap* map, float world_x, float world_y);

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

/*
 * Retorna true se a coordenada no mundo for um buraco energizado pelo Cajado de Pacci (TILE_PACCI_HOLE_CHARGED).
 */
bool map_is_pacci_charged_hole(const Tilemap* map, float world_x, float world_y);

/*
 * Descarrega a energia do buraco após o super-salto vertical do herói, revertendo para TILE_PACCI_HOLE_NORMAL.
 */
bool map_discharge_pacci_hole(Tilemap* map, float world_x, float world_y);

/*
 * Constrói o vilarejo subterrâneo dos Mountain Minish: As Minas de Melari (Melari's Mines).
 * Dimensões: 32x24 tiles (512x384 pixels) contendo a forja e bigorna do Mestre Melari,
 * canais de lava incandescente, veios de minério reluzentes, trilhos e portal para a Masmorra 2.
 */
Tilemap* map_create_melari_mines(void);

/*
 * Retorna true se o ponto no mundo contiver lava incandescente (TILE_MELARI_LAVA_CHANNEL).
 */
bool map_is_lava(const Tilemap* map, float world_x, float world_y);

/*
 * Interage com o cenário no ponto do impacto mágico do Cajado de Pacci:
 * - Energiza buracos normais (TILE_PACCI_HOLE_NORMAL -> TILE_PACCI_HOLE_CHARGED)
 * - Desvira carrinhos de mina (TILE_MINECART_UPSIDE_DOWN -> TILE_MINECART)
 * Retorna true se interagiu com um elemento do cenário.
 */
bool map_interact_pacci(Tilemap* map, float world_x, float world_y);

/*
 * Constrói o Pântano de Castor Wilds (Castor Wilds Swamp).
 * Dimensões: 36x28 tiles (576x448 pixels) contendo charcos de lodo movediço (TILE_SWAMP_MUD),
 * águas escuras, troncos caídos ocos, estátuas ancestrais de olho de pedra e pedestal do Arco.
 */
Tilemap* map_create_castor_wilds(void);

/*
 * Retorna true se o ponto no mundo contiver lodo movediço profundo (TILE_SWAMP_MUD).
 */
bool map_is_swamp_mud(const Tilemap* map, float world_x, float world_y);

/*
 * Retorna true se a coordenada no mundo for uma estátua de olho ancestral (TILE_EYE_STATUE).
 */
bool map_is_eye_statue(const Tilemap* map, float world_x, float world_y);

/*
 * Atinge uma estátua de olho ancestral com uma flecha ou projétil certeiro,
 * abrindo o olho (TILE_EYE_STATUE -> TILE_EYE_STATUE_OPEN) e liberando passagens.
 * Retorna true se acertou e abriu uma estátua de olho.
 */
bool map_hit_eye_statue(Tilemap* map, float world_x, float world_y);

/*
 * Constrói a Caverna das Luvas de Toupeira (Mole Mitts Cavern).
 * Dimensões: 32x24 tiles (512x384 pixels) repleta de paredes de terra fofa escaváveis,
 * túneis subterrâneos, arcas de tesouro e saída para Castor Wilds.
 */
Tilemap* map_create_mole_cave(void);

/*
 * Retorna true se a coordenada no mundo for um bloco ou parede de terra fofa escavável (TILE_DIRT_WALL ou TILE_DIRT_MOUND).
 */
bool map_is_diggable(const Tilemap* map, float world_x, float world_y);

/*
 * Executa a escavação no ponto especificado com as Luvas de Toupeira (Mole Mitts):
 * - Converte TILE_DIRT_WALL em TILE_DIRT_WALL_TUNNEL transitável.
 * - Desobstrui o mapa de colisão.
 * - Sorteia drops enterrados (out_drop: 0 = nada, 1 = Rupee verde, 2 = Rupee azul, 3 = Coração, 4 = Kinstone).
 * Retorna true se um bloco foi escavado com sucesso.
 */
bool map_dig_tile(Tilemap* map, float world_x, float world_y, int* out_drop);

/*
 * Constrói o mapa de Wind Ruins (Ruínas do Vento).
 * Dimensões: 36x24 tiles (576x384 pixels) com ruínas antigas de pedra, pilares caídos,
 * toco de árvore Minish e estátuas Armos bloqueando os desfiladeiros até a Fortaleza dos Ventos.
 */
Tilemap* map_create_wind_ruins(void);

/*
 * Constrói o interior mecânico do robô Armos (Armos Interior).
 * Dimensões: 16x16 tiles (256x256 pixels) com engrenagens de bronze, bobinas elétricas
 * e o interruptor central que reativa a estátua ancestral.
 */
Tilemap* map_create_armos_interior(void);

/*
 * Gerenciamento do estado global de ativação do Armos.
 */
bool armos_circuit_is_active(void);
void armos_circuit_set_active(bool active);

/*
 * Constrói a Biblioteca Real da Cidade de Hyrule (Hyrule Town Library).
 * Dimensões: 24x18 tiles (384x288 pixels) com estantes de livros, mesas de leitura,
 * tapete real vermelho e o toco Minish que permite subir nas estantes gigantes.
 */
Tilemap* map_create_library(void);

/*
 * Constrói o Grande Lago Hylia (Lake Hylia Overworld).
 * Dimensões: 36x28 tiles (576x448 pixels) com águas profundas para mergulho e nado com Flippers,
 * ilhotas, toco Minish e a entrada glacial para o Temple of Droplets (Masmorra 4).
 */
Tilemap* map_create_lake_hylia(void);

/*
 * Interage com o cenário usando o fogo da Flame Lantern:
 * - Acende tochas apagadas (TILE_TORCH_UNLIT -> TILE_TORCH_LIT, *out_type = 1).
 * - Derrete blocos de gelo (TILE_ICE_BLOCK -> chão/ar desobstruído, *out_type = 2).
 * - Queima teias de aranha (TILE_COBWEB -> ar desobstruído, *out_type = 3).
 * - Queima arbustos (TILE_BUSH -> cinzas/drops, *out_type = 4).
 * Retorna true se houve interação bem-sucedida.
 */
bool map_interact_lantern(Tilemap* map, float world_x, float world_y, int* out_type);

#endif // HAL_MAP_H
