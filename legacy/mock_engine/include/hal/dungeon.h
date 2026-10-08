#ifndef HAL_DUNGEON_H
#define HAL_DUNGEON_H

/*
 * ============================================================================
 * include/hal/dungeon.h - Subsistema de Masmorras: Deepwood Shrine
 * ============================================================================
 * Modela a primeira masmorra canônica de The Legend of Zelda: The Minish Cap:
 *
 * Características:
 * - 4 Câmaras subterrâneas interconectadas (16x10 tiles = 256x160 pixels):
 *   - Câmara 0: Vestíbulo de Entrada & Interruptores de Piso
 *   - Câmara 1: Arena de Combate com Fechamento de Portas & Chave Pequena
 *   - Câmara 2: Canal de Águas Subterrâneas & Porta Trancada com Cadeado
 *   - Câmara 3: Santuário Interno & Altar do Elemento Terra com Grande Baú
 * - Mecânicas Clássicas de Dungeon:
 *   - Blocos de pedra empurráveis (Pushable Blocks)
 *   - Interruptores mecânicos de piso (Floor Pressure Switches)
 *   - Portas com grades levadiças (Shutter Doors)
 *   - Portas trancadas com cadeado (Locked Doors) e Chaves Pequenas (Small Keys)
 *   - Transição cinematográfica de salas (Wipe transition)
 *   - HUD integrado com contador de Small Keys
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/entity.h"
#include <stdbool.h>

#define DUNGEON_ROOM_W 16
#define DUNGEON_ROOM_H 10

typedef enum {
    ROOM_ENTRANCE = 0,    // Sala 0: Entrada e escada para Minish Woods
    ROOM_GUARDIANS,       // Sala 1: Arena com inimigos e drop de Small Key
    ROOM_WATER_CHANNEL,   // Sala 2: Canal de água e porta trancada
    ROOM_SANCTUARY_ALTAR, // Sala 3: Altar com grande baú do tesouro
    ROOM_BOSS_ARENA,      // Sala 4: Câmara do Chefe Big Green ChuChu
    ROOM_COUNT
} DungeonRoomId;

typedef struct {
    float x;
    float y;
    bool  is_down;
} DungeonSwitch;

typedef struct {
    float x;
    float y;
    float start_x;
    float start_y;
    float target_x;
    float target_y;
    bool  is_moving;
    int   push_timer;
    Direction push_dir;
} DungeonBlock;

typedef struct {
    float x;
    float y;
    bool  is_locked;
    bool  is_open;
} DungeonDoorState;

typedef struct {
    bool             active;
    DungeonRoomId    current_room;
    int              small_keys;

    // Estado dos quebra-cabeças
    bool             switch_entrance_down;
    bool             door_entrance_open;
    bool             room_guardians_cleared;
    bool             key_spawned;
    bool             key_collected;
    float            key_x;
    float            key_y;
    bool             door_locked_open;
    bool             chest_altar_opened;

    // Estado da Câmara do Chefe (Big Green ChuChu)
    bool             boss_chamber_entered;
    bool             boss_cleared;
    bool             boss_portal_spawned;
    float            portal_x;
    float            portal_y;

    DungeonBlock     block;
    DungeonSwitch    sw_entrance;
    DungeonDoorState door_north_entrance;
    DungeonDoorState door_locked_water;
    DungeonDoorState door_boss_shutter;

    // Transição de sala / fade
    bool             transitioning;
    int              trans_timer;
    DungeonRoomId    target_room;
    float            target_spawn_x;
    float            target_spawn_y;
} DungeonState;

/*
 * Inicializa a masmorra Deepwood Shrine e seus estados de salas e quebra-cabeças.
 */
void dungeon_init(void);

/*
 * Retorna true se Link estiver dentro da masmorra Deepwood Shrine.
 */
bool dungeon_is_active(void);

/*
 * Retorna o ponteiro para o estado atual da masmorra.
 */
DungeonState* dungeon_get_state(void);

/*
 * Realiza a entrada de Link no Deepwood Shrine a partir de Minish Woods.
 */
void dungeon_enter(float* link_x, float* link_y, Direction* link_dir);

/*
 * Realiza a saída de Link do Deepwood Shrine de volta para Minish Woods.
 */
void dungeon_exit(float* link_x, float* link_y, Direction* link_dir);

/*
 * Atualiza todas as mecânicas ativas da masmorra (interruptores, blocos, portas, chaves).
 */
void dungeon_update(float* link_x, float* link_y, Direction* link_dir, bool is_moving,
                    int* link_hearts, int* link_rupees);

/*
 * Renderiza o cenário da câmara atual da masmorra, portas, tochas, blocos e efeitos.
 */
void dungeon_render(const Camera* cam);

/*
 * Renderiza o efeito cinematográfico de fade/wipe preto entre salas.
 */
void dungeon_render_transition(const Camera* cam);

/*
 * Verifica se um ponto colide com paredes, blocos ou portas fechadas da masmorra.
 */
bool dungeon_is_solid(float world_x, float world_y);

/*
 * Interage com o cenário da masmorra (ex: golpe de espada em tochas, corte de teias ou abertura de baú).
 */
bool dungeon_interact(float x, float y, int* link_rupees, int* link_hearts);

/*
 * Renderiza o contador de chaves pequenas (🔑 xN) no HUD superior.
 */
void dungeon_render_hud_keys(int x, int y);

#endif // HAL_DUNGEON_H
