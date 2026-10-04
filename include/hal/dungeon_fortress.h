#ifndef HAL_DUNGEON_FORTRESS_H
#define HAL_DUNGEON_FORTRESS_H

/*
 * ============================================================================
 * include/hal/dungeon_fortress.h - Subsistema de Masmorras: Fortress of Winds (Dungeon 3)
 * ============================================================================
 * Modela a terceira masmorra canônica de The Legend of Zelda: The Minish Cap:
 *
 * Características:
 * - 6 Câmaras interconectadas no complexo ancestral de pedra e vento (16x10 tiles = 256x160 px):
 *   - Sala 0: Vestíbulo de Entrada & Alvo de Olho Mecânico (Acionamento com Flecha)
 *   - Sala 1: Câmara de Escavação com Mole Mitts (Labirinto de Terra & Baú com Chave)
 *   - Sala 2: Abismo dos Ventos & Olhos Duplos (Ponte Retrátil Ativada por Flechas)
 *   - Sala 3: Sala de Fissura Minish (Toco Minish, fresta na pedra & Chave do Mestre)
 *   - Sala 4: Antecâmara do Grande Portão Alado do Chefe
 *   - Sala 5: Arena de Combate contra o Chefe MAZAAL (Cabeça e Mãos Mecânicas)
 * - Mecânicas Exclusivas:
 *   - Disparo de flechas em estátuas de olho para baixar portas e estender pontes
 *   - Escavação de paredes de terra fofa com Luvas de Toupeira (Mole Mitts)
 *   - Infiltração Minish em frestas e na boca de Mazaal para golpear o núcleo interno
 *   - Chefe Mazaal: desativar duas mãos voadoras, atordoar a cabeça e destruir o núcleo
 *   - Recompensa sagrada: Recipiente de Coração e Ocarina do Vento (Ocarina of Wind)!
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/entity.h"
#include <stdbool.h>

#define FORTRESS_ROOM_W 16
#define FORTRESS_ROOM_H 10

typedef enum {
    ROOM_FORTRESS_ENTRANCE = 0,   // Sala 0: Entrada e saída para as Ruínas do Vento
    ROOM_FORTRESS_DIG_MAZE,       // Sala 1: Labirinto de escavação com Mole Mitts
    ROOM_FORTRESS_ARROW_PUZZLE,   // Sala 2: Abismo e alvos de olhos duplos
    ROOM_FORTRESS_MINISH_HOLE,    // Sala 3: Fresta Minish e obtenção da Boss Key
    ROOM_FORTRESS_BOSS_DOOR,      // Sala 4: Antecâmara com portão alado do Chefe Mazaal
    ROOM_FORTRESS_BOSS_ARENA,     // Sala 5: Arena de Combate contra MAZAAL
    ROOM_FORTRESS_COUNT
} DungeonFortressRoomId;

typedef struct {
    float x;
    float y;
    float base_x;
    float base_y;
    float vx;
    float vy;
    int   health;       // 3 HP por mão
    bool  active;       // flutuando e atacando
    bool  stunned;      // desativada no solo
    int   stun_timer;
    int   slam_timer;
    bool  eye_open;     // olho aberto vulnerável a flecha ou espada
} MazaalHand;

typedef struct {
    float x;
    float y;
    int   core_health;  // 12 HP total (4 por ciclo de atordoamento)
    int   stun_cycle;   // 0 a 3
    bool  stunned;      // caída no chão com boca aberta
    int   stun_timer;
    bool  mouth_open;   // Link Minish pode entrar
    bool  defeated;     // morto em faíscas
    int   defeat_timer;
} MazaalHead;

typedef struct {
    bool                 active;
    DungeonFortressRoomId current_room;
    int                  small_keys;
    bool                 has_boss_key;

    // Sala 0: Entrada
    bool                 eye_entrance_hit;
    bool                 door_entrance_open;

    // Sala 1: Escavação Mole Mitts
    bool                 dig_chest_opened;
    bool                 dig_key_obtained;
    u8                   dirt_grid[FORTRESS_ROOM_H][FORTRESS_ROOM_W]; // 1 = terra fofa presente, 0 = escavado

    // Sala 2: Abismo & Flechas
    bool                 arrow_eye_left_hit;
    bool                 arrow_eye_right_hit;
    bool                 bridge_extended;

    // Sala 3: Minish
    bool                 minish_switch_pressed;
    bool                 door_minish_open;

    // Sala 4: Boss Door
    bool                 boss_door_unlocked;

    // Sala 5: Chefe Mazaal
    bool                 boss_chamber_entered;
    MazaalHead           mazaal_head;
    MazaalHand           mazaal_hand_l;
    MazaalHand           mazaal_hand_r;
    bool                 ocarina_spawned;
    bool                 ocarina_collected;
    float                ocarina_x;
    float                ocarina_y;
    bool                 heart_container_spawned;
    bool                 heart_container_collected;
    float                heart_container_x;
    float                heart_container_y;
    bool                 warp_portal_spawned;
    float                warp_portal_x;
    float                warp_portal_y;

    // Ponto seguro para respawn
    float                safe_x;
    float                safe_y;

    // Transição entre salas
    bool                 transitioning;
    int                  trans_timer;
    DungeonFortressRoomId target_room;
    float                target_spawn_x;
    float                target_spawn_y;
} DungeonFortressState;

/*
 * Inicializa o subsistema da masmorra Fortress of Winds.
 */
void dungeon_fortress_init(void);

/*
 * Retorna true se Link estiver atualmente na Fortress of Winds.
 */
bool dungeon_fortress_is_active(void);

/*
 * Retorna ponteiro para o estado atual da Fortaleza dos Ventos.
 */
DungeonFortressState* dungeon_fortress_get_state(void);

/*
 * Entra na Fortress of Winds a partir das Ruínas do Vento.
 */
void dungeon_fortress_enter(float* link_x, float* link_y, Direction* link_dir);

/*
 * Retorna da Fortress of Winds para as Ruínas do Vento.
 */
void dungeon_fortress_exit(float* link_x, float* link_y, Direction* link_dir);

/*
 * Atualiza todas as mecânicas da Fortress of Winds.
 */
void dungeon_fortress_update(float* link_x, float* link_y, Direction* link_dir, bool is_moving,
                             int* link_hearts, int* link_rupees, bool is_minish, bool is_attacking,
                             bool* out_warp_to_surface);

/*
 * Renderiza o cenário da sala ativa, quebra-cabeças, Mazaal e itens.
 */
void dungeon_fortress_render(const Camera* cam, bool is_minish, float link_x, float link_y);

/*
 * Renderiza o ícone de chaves pequenas da fortaleza no HUD.
 */
void dungeon_fortress_render_hud_keys(int x, int y);

/*
 * Retorna true se o jogador já coletou a Ocarina do Vento.
 */
bool dungeon_fortress_has_ocarina(void);

/*
 * Checa impacto de flecha disparada contra estátuas de olho da masmorra.
 */
void dungeon_fortress_shoot_arrow_hit(float arrow_x, float arrow_y);

#endif // HAL_DUNGEON_FORTRESS_H
