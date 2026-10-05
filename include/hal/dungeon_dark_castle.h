#ifndef HAL_DUNGEON_DARK_CASTLE_H
#define HAL_DUNGEON_DARK_CASTLE_H

/*
 * ============================================================================
 * include/hal/dungeon_dark_castle.h - Masmorra Final: Dark Hyrule Castle (Ato V)
 * ============================================================================
 * Modela a 6ª e última grande masmorra canônica de The Legend of Zelda: The Minish Cap:
 *
 * Características:
 * - O Castelo Sagrado de Hyrule corrompido pelas trevas e malícia do feiticeiro Vaati.
 * - Paredes de pedra de obsidiana púrpura, vitrais despedaçados, tapetes escuros e chamas góticas.
 * - Os 3 Sinos da Maldição (Petrification Bells):
 *   - O badalar fúnebre que dita a contagem regressiva para a petrificação definitiva de Zelda.
 *   - Badalar os 3 sinos com a Four Sword dissipa os selos de malícia que bloqueiam a torre.
 * - 6 Câmaras interconectadas (16x10 tiles = 256x160 px):
 *   - Sala 0: Vestíbulo Corrompido & Barreira Central de Malícia (Split Pads e tochas escuras)
 *   - Sala 1: Ala Oeste - Abismo dos 4 Olhos Sincronizados (Disparo simultâneo dos 4 clones com arco/espada)
 *   - Sala 2: Ala Leste - Monólito de Obsidiana & Canhões de Fogo (Força quadruplicada empurrando monólito)
 *   - Sala 3: Salão da Guarda Corrompida - Minichefe CAVALEIRO NEGRO (Darknut com armadura, escudo e montante)
 *   - Sala 4: A Torre dos Sinos da Petrificação (Os 3 sinos do castelo, tempestade de raios e estátua de Zelda)
 *   - Sala 5: Antecâmara do Santuário de Vaati (Portão da Boss Key e carregamento dos 4 Elementos)
 *
 * Mecânicas Exclusivas:
 * - Puzzles de coordenação extrema dos 4 Clones da Four Sword (divisão em diamante e linha).
 * - Minichefe Darknut (Cavaleiro Negro com escudo defensivo e ataques circulares de espada).
 * - Sinos de Petrificação ressonantes com screenshake e batidas rítmicas.
 * - Concede a Chave do Santuário (Sanctum Boss Key) e abre o caminho para o clímax final.
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/entity.h"
#include <stdbool.h>

#define DHC_ROOM_W 16
#define DHC_ROOM_H 10

typedef enum {
    DHC_ROOM_ENTRANCE_FOYER = 0,    // Sala 0: Vestíbulo Corrompido & Barreira de Malícia
    DHC_ROOM_WEST_CHASM_PUZZLE,     // Sala 1: Ala Oeste - Abismo e 4 Olhos Sincronizados
    DHC_ROOM_EAST_MONOLITH_PUZZLE,  // Sala 2: Ala Leste - Monólito de Obsidiana & Canhões
    DHC_ROOM_DARKNUT_MINIBOSS,      // Sala 3: Salão dos Cavaleiros Negros (Darknut Miniboss)
    DHC_ROOM_BELL_TOWER,            // Sala 4: Torre dos Sinos da Petrificação
    DHC_ROOM_THRONE_SANCTUM_GATE,   // Sala 5: Antecâmara do Santuário de Vaati
    DHC_ROOM_COUNT
} DarkCastleRoomId;

typedef enum {
    DARKNUT_STATE_IDLE = 0,
    DARKNUT_STATE_PURSUE,
    DARKNUT_STATE_ATTACK,
    DARKNUT_STATE_SHIELD_DEFEND,
    DARKNUT_STATE_STUNNED,
    DARKNUT_STATE_DEFEATED
} DarknutState;

#define MAX_DHC_TORCHES 4
#define MAX_DHC_SWITCHES 4
#define MAX_DHC_BELLS 3
#define MAX_DHC_FIREBALLS 4

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    bool  active;
    int   lifetime;
} DhcFireball;

typedef struct {
    float         x;
    float         y;
    float         vx;
    float         vy;
    int           hp;             // 16 HP
    int           max_hp;
    DarknutState  state;
    Direction     facing;
    int           action_timer;
    int           hit_stun;
    bool          is_active;
    bool          defeated;
    bool          shield_raised;
} DarknutBoss;

typedef struct {
    bool              is_active;
    DarkCastleRoomId  current_room;
    int               small_keys;
    bool              has_sanctum_key;
    bool              room_cleared[DHC_ROOM_COUNT];

    // Sala 0: Vestíbulo Corrompido
    bool              foyer_split_pads_active;
    bool              foyer_torches_lit[MAX_DHC_TORCHES];
    bool              malice_barrier_open;
    int               malice_pulse_timer;

    // Sala 1: Ala Oeste (Abismo & 4 Olhos)
    bool              eye_switches_hit[MAX_DHC_SWITCHES];
    int               eye_hit_window;
    bool              hard_light_bridge_active;
    bool              west_key_chest_opened;

    // Sala 2: Ala Leste (Monólito & Canhões)
    float             monolith_x;
    float             monolith_y;
    bool              monolith_slid;
    bool              east_door_unlocked;
    DhcFireball       fireballs[MAX_DHC_FIREBALLS];
    int               cannon_cooldown;

    // Sala 3: Minichefe Darknut
    DarknutBoss       darknut;
    bool              sanctum_key_spawned;
    bool              sanctum_key_collected;

    // Sala 4: Torre dos Sinos da Petrificação
    bool              bells_struck[MAX_DHC_BELLS];
    int               bell_toll_timer;
    int               petrification_stage;  // 0, 1, 2, 3 (badaladas)
    int               screen_shake_timer;
    bool              sanctum_stairs_open;

    // Sala 5: Antecâmara do Santuário de Vaati
    bool              four_elements_charged;
    int               charge_anim_timer;
    bool              sanctum_gate_opened;
    bool              ready_for_vaati_climax;

    // Transição de tela
    int               transition_alpha;
    Tilemap*          room_maps[DHC_ROOM_COUNT];
} DarkHyruleCastle;

// Inicialização e Ciclo de Vida
void dark_castle_init(void);
void dark_castle_enter(float* link_x, float* link_y, Direction* link_dir);
void dark_castle_exit(float* link_x, float* link_y, Direction* link_dir);
bool dark_castle_is_active(void);
DarkCastleRoomId dark_castle_get_room(void);

// Troca de Salas e Progressão
void dark_castle_set_room(DarkCastleRoomId room, float* link_x, float* link_y, Direction* link_dir);
Tilemap* dark_castle_get_current_map(void);

// Atualização e Lógica (60 FPS)
void dark_castle_update(float* link_x, float* link_y, Direction* link_dir,
                        bool is_attacking, bool has_four_sword, int clone_count,
                        const float clone_x[3], const float clone_y[3],
                        int* player_hearts, int* player_keys);

// Interação e Combate
void dark_castle_interact_action(float link_x, float link_y, Direction link_dir,
                                 int* player_hearts, int* player_keys);
void dark_castle_strike_sword(float link_x, float link_y, Direction link_dir, int damage,
                              bool is_spin, int clone_count,
                              const float clone_x[3], const float clone_y[3]);
void dark_castle_arrow_hit(float arrow_x, float arrow_y);

// Renderização na HAL Video
void dark_castle_render(const Camera* cam, float link_x, float link_y);

// Salvar / Restaurar Estado
void dark_castle_restore_state(bool cleared, bool bells_silenced, bool has_boss_key);
bool dark_castle_is_cleared(void);
bool dark_castle_is_ready_for_vaati(void);

#endif // HAL_DUNGEON_DARK_CASTLE_H
