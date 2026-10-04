#ifndef HAL_DUNGEON_FLAMES_H
#define HAL_DUNGEON_FLAMES_H

/*
 * ============================================================================
 * include/hal/dungeon_flames.h - Subsistema de Masmorras: Cave of Flames (Dungeon 2)
 * ============================================================================
 * Modela a segunda masmorra canônica de The Legend of Zelda: The Minish Cap:
 *
 * Características:
 * - 5 Câmaras subterrâneas vulcânicas interconectadas (16x10 tiles = 256x160 pixels):
 *   - Câmara 0: Vestíbulo de Entrada & Interruptor de Piso de Ferro
 *   - Câmara 1: Lago de Lava & Sistema Ferroviário de Vagoneta (Minecart & Pacci Flip)
 *   - Câmara 2: Corredor do Cilindro Espinhoso Giratório (Rolling Spiked Barrel)
 *   - Câmara 3: Crisol Vulcânico de Combate & Drop de Chave Pequena (Fire Keese & Beetles)
 *   - Câmara 4: Antecâmara do Portão do Chefe Trancado (Acesso ao Gleerok)
 * - Mecânicas Exclusivas da Masmorra de Fogo:
 *   - Vagonetas de Mina (Minecarts) montáveis e inversão de vagoneta com Cajado de Pacci
 *   - Cilindro de ferro espinhoso oscilante (Rolling Spiked Barrel)
 *   - Dano ambiental por contato com Lava Ardente (reseta posição para área segura)
 *   - Portas levadiças (Shutter Doors) e Portas Trancadas com Cadeado (Locked Doors)
 *   - Inimigos temáticos: Fire Keese e Spiny Beetles
 *   - Transição cinematográfica e HUD integrado com Small Keys
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/entity.h"
#include <stdbool.h>

#define FLAMES_ROOM_W 16
#define FLAMES_ROOM_H 10

typedef enum {
    ROOM_FLAMES_ENTRANCE = 0, // Sala 0: Entrada e saída para as Minas de Melari
    ROOM_FLAMES_MINECART,     // Sala 1: Trilhos sobre lava e vagoneta com Pacci Flip
    ROOM_FLAMES_BARREL,       // Sala 2: Cilindro espinhoso giratório
    ROOM_FLAMES_CRUCIBLE,     // Sala 3: Crisol de combate (Fire Keese + Small Key)
    ROOM_FLAMES_BOSS_DOOR,    // Sala 4: Antecâmara com porta trancada do Chefe Gleerok
    ROOM_FLAMES_COUNT
} DungeonFlamesRoomId;

typedef struct {
    float x;
    float y;
    float start_x;
    float target_x;
    float speed;
    int   direction; // 1: dir, -1: esq
} RollingBarrel;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    bool  flipped;        // true se estiver emborcada (requer Pacci para desvirar)
    bool  is_riding;      // true se Link estiver montado nela
    int   track_endpoint; // 0 = Estação Sul, 1 = Estação Norte
    float flip_anim;      // timer de animação do giro mágico
} Minecart;

typedef struct {
    float x;
    float y;
    bool  is_down;
} FlamesSwitch;

typedef struct {
    float x;
    float y;
    bool  is_locked;
    bool  is_open;
} FlamesDoor;

typedef struct {
    bool                active;
    DungeonFlamesRoomId current_room;
    int                 small_keys;

    // Estados de quebra-cabeças
    bool                switch_entrance_down;
    bool                door_entrance_open;

    // Vagoneta
    Minecart            cart;

    // Cilindro espinhoso
    RollingBarrel       barrel;

    // Crisol de combate
    bool                crucible_cleared;
    bool                key_spawned;
    bool                key_collected;
    float               key_x;
    float               key_y;

    // Portas
    FlamesDoor          door_north_entrance;
    FlamesDoor          door_crucible_shutter;
    FlamesDoor          door_boss_locked;

    // Ponto de segurança contra lava
    float               safe_x;
    float               safe_y;

    // Transição de sala
    bool                transitioning;
    int                 trans_timer;
    DungeonFlamesRoomId target_room;
    float               target_spawn_x;
    float               target_spawn_y;

    // Invulnerabilidade temporária a lava
    int                 lava_cooldown;
} DungeonFlamesState;

/*
 * Inicializa a masmorra Cave of Flames.
 */
void dungeon_flames_init(void);

/*
 * Retorna true se Link estiver na Cave of Flames.
 */
bool dungeon_flames_is_active(void);

/*
 * Retorna ponteiro para o estado atual da Cave of Flames.
 */
DungeonFlamesState* dungeon_flames_get_state(void);

/*
 * Entra na Cave of Flames a partir das Minas de Melari.
 */
void dungeon_flames_enter(float* link_x, float* link_y, Direction* link_dir);

/*
 * Retorna da Cave of Flames para as Minas de Melari.
 */
void dungeon_flames_exit(float* link_x, float* link_y, Direction* link_dir);

/*
 * Atualiza todas as mecânicas da Cave of Flames (lava, vagoneta, barril, chaves, portas).
 */
void dungeon_flames_update(float* link_x, float* link_y, Direction* link_dir, bool is_moving,
                           int* link_hearts, int* link_rupees);

/*
 * Renderiza o cenário da masmorra, lava animada, trilhos, vagoneta, barril e portas.
 */
void dungeon_flames_render(const Camera* cam);

/*
 * Renderiza transição de fade/wipe entre câmaras.
 */
void dungeon_flames_render_transition(const Camera* cam);

/*
 * Verifica se um ponto colide com paredes, obstáculos sólidos ou portas fechadas.
 */
bool dungeon_flames_is_solid(float world_x, float world_y);

/*
 * Interage com o cenário da Cave of Flames (montar na vagoneta, abrir portas com chaves, etc.).
 */
bool dungeon_flames_interact(float x, float y, int* link_rupees, int* link_hearts);

/*
 * Chamado quando um projétil do Cajado de Pacci atinge uma posição no mundo.
 * Retorna true se atingiu e desvirou a vagoneta emborcada.
 */
bool dungeon_flames_check_pacci_hit(float px, float py, float pw, float ph);

/*
 * Retorna true se Link estiver atualmente montado dentro da vagoneta.
 */
bool dungeon_flames_is_link_riding_cart(void);

/*
 * Renderiza o contador de chaves pequenas no HUD.
 */
void dungeon_flames_render_hud_keys(int x, int y);

#endif // HAL_DUNGEON_FLAMES_H
