#ifndef HAL_DUNGEON_DROPLETS_H
#define HAL_DUNGEON_DROPLETS_H

/*
 * ============================================================================
 * include/hal/dungeon_droplets.h - Masmorra 4: Temple of Droplets (Templo das Gotas)
 * ============================================================================
 * Modela a quarta masmorra canônica de The Legend of Zelda: The Minish Cap:
 *
 * Características:
 * - Localizada nas profundezas congeladas de Lake Hylia, infiltrada como Minish Link.
 * - 6 Câmaras congeladas (16x10 tiles = 256x160 px):
 *   - Sala 0: Vestíbulo Glacial & Canal Aquático de Ice Floes (Entrada e saída para Lake Hylia)
 *   - Sala 1: Câmara do Facho Solar (Prisma e alavanca que redireciona o raio de sol para derreter o gelo)
 *   - Sala 2: Caverna de Gelo Escura (Requer iluminação da Flame Lantern; 4 piras de pedra para acender)
 *   - Sala 3: Santuário do Elemento da Água (O sagrado Water Element encravado num monolito de gelo)
 *   - Sala 4: Antecâmara do Grande Portão Glacial (Requer a Big Key)
 *   - Sala 5: Arena Glacial do Chefe BIG OCTOROK (Grande Octorok Glacial)
 * - Mecânicas Exclusivas:
 *   - Física de Inércia no Gelo: Link escorrega e desliza suavemente sobre lajes de gelo translúcido.
 *   - Interação Térmica: a Flame Lantern derrete blocos de gelo e acende tochas.
 *   - Redirecionamento da Luz Solar: prismas ópticos rotativos que concentram luz para descongelar caminhos.
 *   - Chefe BIG OCTOROK:
 *     - Fase 1: Link rebate pedras pontiagudas cuspidas de volta ao focinho para atordoá-lo.
 *     - Fase 2: Com a Flame Lantern, Link queima e derrete a cauda congelada do monstro.
 *     - Fase 3: Big Octorok em chamas corre furioso pela arena, vulnerável a golpes de espada.
 *     - Recompensa: Recipiente de Coração (Heart Container) e o Sagrado Elemento da Água (Water Element)!
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/entity.h"
#include <stdbool.h>

#define DROPLETS_ROOM_W 16
#define DROPLETS_ROOM_H 10

typedef enum {
    ROOM_DROPLETS_ENTRANCE = 0,    // Sala 0: Vestíbulo de entrada e águas gélidas
    ROOM_DROPLETS_SUNBEAM,         // Sala 1: Facho solar e prisma óptico
    ROOM_DROPLETS_DARK_CAVERN,     // Sala 2: Caverna escura e 4 piras de fogo
    ROOM_DROPLETS_SANCTUARY,       // Sala 3: Monólito do Elemento da Água
    ROOM_DROPLETS_BOSS_DOOR,       // Sala 4: Portão ornamental do Chefe
    ROOM_DROPLETS_BOSS_ARENA,      // Sala 5: Arena de combate contra Big Octorok
    ROOM_DROPLETS_COUNT
} DungeonDropletsRoomId;

typedef enum {
    OCTO_PHASE_FROZEN = 0,         // Cauda congelada; cospe pedregulhos pontiagudos
    OCTO_PHASE_BURNING,            // Cauda em chamas; investidas em pânico pelo gelo
    OCTO_PHASE_STUNNED,            // Atordoado após rocha rebatida; cauda vulnerável ao fogo
    OCTO_PHASE_DEFEATED            // Dissolvido em vapor e cristais de gelo
} BigOctoPhase;

#define MAX_OCTO_ROCKS 4

typedef struct {
    bool  is_active;
    float x;
    float y;
    float vx;
    float vy;
    bool  reflected;               // true se foi rebatida pelo escudo/espada de Link
} OctoRock;

typedef struct {
    float        x;
    float        y;
    float        vx;
    float        vy;
    int          health;           // 12 HP
    BigOctoPhase phase;
    int          phase_timer;
    int          spit_timer;
    int          burn_timer;
    float        tail_x;
    float        tail_y;
    bool         tail_frozen;
    Direction    dir;
    float        anim_frame;
    bool         is_active;
    OctoRock     rocks[MAX_OCTO_ROCKS];
} BigOctorokBoss;

typedef struct {
    bool                  is_active;
    DungeonDropletsRoomId current_room;
    int                   small_keys;
    bool                  has_boss_key;
    bool                  room_cleared[ROOM_DROPLETS_COUNT];
    bool                  sunbeam_redirected;    // Prisma redirecionado para o bloco de gelo
    bool                  sunbeam_ice_melted;    // Bloco da chave pequena derretido
    bool                  element_ice_melted;    // Monólito do Elemento da Água derretido
    bool                  torches_lit[4];        // 4 tochas da câmara escura
    bool                  dark_room_gate_open;   // Portão de grades da sala escura aberto
    bool                  boss_door_unlocked;
    bool                  boss_defeated;
    bool                  heart_container_collected;
    bool                  water_element_collected;
    float                 ice_slide_vx;          // Velocidade de escorregamento no gelo
    float                 ice_slide_vy;
    Tilemap*              rooms[ROOM_DROPLETS_COUNT];
    BigOctorokBoss        boss;
} DungeonDropletsState;

/*
 * Inicializa a arquitetura e constrói todas as 6 câmaras do Temple of Droplets.
 */
void dungeon_droplets_init(void);

/*
 * Entra na masmorra posicionando o jogador na entrada da Sala 0.
 */
void dungeon_droplets_enter(float* player_x, float* player_y, Direction* player_dir);

/*
 * Sai da masmorra retornando ao Lago Hylia em frente ao portal glacial.
 */
void dungeon_droplets_exit(float* player_x, float* player_y, Direction* player_dir);

/*
 * Retorna true se a masmorra Temple of Droplets está ativa no game loop.
 */
bool dungeon_droplets_is_active(void);

/*
 * Retorna o identificador da câmara atual.
 */
DungeonDropletsRoomId dungeon_droplets_get_current_room(void);

/*
 * Retorna o ponteiro para o Tilemap da câmara atual.
 */
Tilemap* dungeon_droplets_get_current_map(void);

/*
 * Atualiza lógica de física no gelo, quebra-cabeças solares, tochas, transições e o chefe Big Octorok.
 */
void dungeon_droplets_update(float* player_x, float* player_y, Direction player_dir, bool is_moving,
                             int* player_hearts, int max_hearts, int* player_rupees, bool* player_has_water_element);

/*
 * Renderiza o mapa da câmara atual, blocos de gelo, fachos solares, tochas e o chefe Big Octorok.
 */
void dungeon_droplets_render(const Camera* camera, bool is_minish, float player_x, float player_y);

/*
 * Renderiza ícones de chaves pequenas e Boss Key no HUD superior.
 */
void dungeon_droplets_render_hud_keys(int x, int y);

/*
 * Retorna true se a coordenada do mundo estiver sobre uma laje de gelo escorregadio.
 */
bool dungeon_droplets_is_ice_tile(float world_x, float world_y);

/*
 * Aplica inércia física de deslizamento sobre gelo ao movimento do Link.
 */
void dungeon_droplets_apply_ice_physics(float* player_x, float* player_y, float speed, Direction dir, bool is_moving);

/*
 * Checa impacto da espada do Link contra o chefe ou contra projéteis cuspidos.
 */
void dungeon_droplets_check_boss_sword_hit(float hit_x, float hit_y, float hit_w, float hit_h, int damage, Direction dir);

/*
 * Checa calor da Flame Lantern contra a cauda do Big Octorok ou contra blocos de gelo da masmorra.
 */
void dungeon_droplets_check_boss_lantern_hit(float flame_x, float flame_y, float radius);

/*
 * Finaliza e libera os mapas alocados da masmorra.
 */
void dungeon_droplets_shutdown(void);

#endif // HAL_DUNGEON_DROPLETS_H
