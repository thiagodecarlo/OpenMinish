#ifndef HAL_DUNGEON_PALACE_H
#define HAL_DUNGEON_PALACE_H

/*
 * ============================================================================
 * include/hal/dungeon_palace.h - Masmorra 5: Palace of Winds (Palácio dos Ventos)
 * ============================================================================
 * Modela a quinta masmorra canônica de The Legend of Zelda: The Minish Cap:
 *
 * Características:
 * - Localizada na estratosfera acima de Cloud Tops, acessada via Grande Tornado.
 * - Arquitetura celestial da Tribo do Vento: mármore branco, frisos dourados,
 *   turbinas eólicas, passarelas de grades e abismos abertos para o céu.
 * - 6 Câmaras interconectadas (16x10 tiles = 256x160 px):
 *   - Sala 0: Vestíbulo dos Céus & Turbinas de Vento (Entrada/saída para Cloud Tops, cata-vento ativador)
 *   - Sala 1: Abismos Celestiais & Travessia com Roc's Cape (Pulo livre e flutuação sobre o vácuo; Baú com Chave)
 *   - Sala 2: Salão dos Interruptores Triplos da Four Sword (Pads de clonagem e 3 interruptores simultâneos)
 *   - Sala 3: Torre dos Wizzrobes & Câmara da Boss Key (Feiticeiros do vento teletransportadores e Big Key)
 *   - Sala 4: Passarela Aérea & Grande Portão Alado do Chefe (Passarela monumental suspensa nos céus)
 *   - Sala 5: Arena Aérea no Vácuo dos Céus: Chefe GYORG PAIR (Arraia Azul fêmea e Arraia Vermelha macho)
 *
 * Mecânicas Exclusivas:
 * - Plataformas flutuantes e travessia com a Capa de Roc (Roc's Cape).
 * - Turbinas eólicas que empurram o herói ou impulsionam saltos no ar.
 * - Quebra-cabeça de clones sincronizados da Four Sword.
 * - Confronto aéreo de dois chefes simultâneos (Gyorg Pair):
 *   - Combate no dorso da Grande Arraia Azul: golpear 3 olhos abertos com cortes e clones.
 *   - Transição aérea saltando com a Roc's Cape para o dorso da Arraia Vermelha!
 *   - Combate na Arraia Vermelha: desviar da cauda giratória saltando no ar e acertar o olho central.
 *   - Projéteis de vento e esferas de energia que devem ser desviadas ou rebatidas.
 * - Recompensas sagradas:
 *   - Recipiente de Coração permanente (Heart Container).
 *   - O sagrado ELEMENTO DO VENTO (Wind Element - 4º e último elemento)!
 *   - Portal sagrado de teletransporte (Warp Portal) de retorno ao Santuário das Nuvens.
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/entity.h"
#include <stdbool.h>

#define PALACE_ROOM_W 16
#define PALACE_ROOM_H 10

typedef enum {
    ROOM_PALACE_ENTRANCE = 0,       // Sala 0: Vestíbulo dos Céus & Turbinas de Vento
    ROOM_PALACE_ROCS_CROSSING,      // Sala 1: Abismos Celestiais & Travessia com Roc's Cape
    ROOM_PALACE_FOUR_SWITCH,        // Sala 2: Salão dos Interruptores da Four Sword
    ROOM_PALACE_WIZZROBE_TOWER,     // Sala 3: Torre dos Wizzrobes & Grande Baú da Boss Key
    ROOM_PALACE_BOSS_GATE,          // Sala 4: Passarela Aérea & Portão Alado do Chefe
    ROOM_PALACE_GYORG_ARENA,        // Sala 5: Arena Aérea dos Céus - Chefe GYORG PAIR
    ROOM_PALACE_COUNT
} DungeonPalaceRoomId;

typedef enum {
    GYORG_MOUNT_BLUE = 0,           // Link está combatendo no dorso da Grande Arraia Azul
    GYORG_MOUNT_RED,                // Link saltou e está combatendo no dorso da Arraia Vermelha
    GYORG_TRANSITION                // Salto em pleno ar entre as arraias com Roc's Cape
} GyorgMount;

#define MAX_GYORG_BALLS 4
#define MAX_PALACE_CLOUDS 16
#define MAX_WIZZROBES 2

typedef struct {
    bool  active;
    float x;
    float y;
    float vx;
    float vy;
    int   life;
} GyorgEnergyBall;

typedef struct {
    float x;
    float y;
    int   hp;
    bool  alive;
    bool  visible;
    int   teleport_timer;
    int   cast_timer;
    float target_x;
    float target_y;
} PalaceWizzrobe;

typedef struct {
    // Arraia Fêmea Azul (Grande)
    float           blue_x;
    float           blue_y;
    float           blue_vx;
    float           blue_vy;
    int             blue_health;         // 12 HP
    bool            blue_eyes_open[3];   // 3 olhos vulneráveis
    int             blue_eye_timer;
    int             blue_hit_stun;

    // Arraia Macho Vermelho (Menor e Rápido)
    float           red_x;
    float           red_y;
    float           red_vx;
    float           red_vy;
    int             red_health;          // 8 HP
    bool            red_eye_open;
    int             red_eye_timer;
    float           red_tail_angle;
    bool            red_attacking;
    int             red_hit_stun;

    // Dinâmica de Montaria e Combate
    GyorgMount      current_mount;
    int             mount_timer;         // Tempo até a troca necessária de montaria
    bool            need_jump_prompt;    // Exibe aviso para saltar com Roc's Cape
    bool            is_active;
    bool            defeated;
    int             defeat_timer;

    // Projéteis
    GyorgEnergyBall balls[MAX_GYORG_BALLS];
    int             attack_timer;
} GyorgPairBoss;

typedef struct {
    bool                is_active;
    DungeonPalaceRoomId current_room;
    int                 small_keys;
    bool                has_boss_key;
    bool                room_cleared[ROOM_PALACE_COUNT];

    // Sala 0: Entrada
    bool                pinwheel_activated;
    bool                door_entrance_open;
    int                 turbine_anim_timer;

    // Sala 1: Travessia Roc's Cape
    bool                rocs_chest_opened;
    bool                rocs_key_obtained;
    float               safe_x;
    float               safe_y;

    // Sala 2: Interruptores Four Sword
    bool                switches_pressed[3];
    int                 switch_timer;
    bool                switch_gate_open;

    // Sala 3: Torre dos Wizzrobes
    PalaceWizzrobe      wizzrobes[MAX_WIZZROBES];
    bool                boss_key_chest_spawned;
    bool                boss_key_chest_opened;

    // Sala 4: Portão do Chefe
    bool                boss_door_unlocked;

    // Sala 5: Chefe Gyorg Pair & Recompensas
    bool                boss_chamber_entered;
    bool                heart_container_spawned;
    bool                heart_container_collected;
    float               heart_container_x;
    float               heart_container_y;
    bool                wind_element_spawned;
    bool                wind_element_collected;
    float               wind_element_x;
    float               wind_element_y;
    bool                warp_portal_spawned;
    float               warp_portal_x;
    float               warp_portal_y;

    // Mapas das 6 salas
    Tilemap*            rooms[ROOM_PALACE_COUNT];
    GyorgPairBoss       boss;

    // Efeitos de vento e nuvens de fundo
    float               cloud_scroll_x;
    float               cloud_scroll_y;
} DungeonPalaceState;

// Inicialização da masmorra Palace of Winds e construção das 6 câmaras
void dungeon_palace_init(void);

// Entra no Palace of Winds na Sala 0 vindo do Grande Tornado de Cloud Tops
void dungeon_palace_enter(float* player_x, float* player_y, Direction* player_dir);

// Sai da masmorra retornando ao Santuário da Tribo do Vento em Cloud Tops
void dungeon_palace_exit(float* player_x, float* player_y, Direction* player_dir);

// Retorna true se a masmorra Palace of Winds está ativa
bool dungeon_palace_is_active(void);

// Retorna a câmara atual
DungeonPalaceRoomId dungeon_palace_get_current_room(void);

// Retorna o Tilemap da câmara ativa
Tilemap* dungeon_palace_get_current_map(void);

// Retorna ponteiro para o estado interno do Palace of Winds
DungeonPalaceState* dungeon_palace_get_state(void);

// Atualiza a lógica da masmorra, quebra-cabeças, plataformas, Roc's Cape e o chefe Gyorg Pair
void dungeon_palace_update(float* player_x, float* player_y, Direction* player_dir, bool is_moving,
                           int* player_hearts, int max_hearts, int* player_rupees,
                           bool* player_has_wind_element, bool has_rocs_cape,
                           bool is_jumping, bool is_gliding, bool is_downthrusting, bool is_attacking);

// Renderiza o cenário da câmara, o céu dinâmico, turbinas de vento, o chefe Gyorg Pair e itens
void dungeon_palace_render(const Camera* camera, float player_x, float player_y, bool is_minish);

// Renderiza ícones de chaves no HUD
void dungeon_palace_render_hud_keys(int x, int y);

// Processa ataque com espada contra os chefes ou quebra-cabeças
void dungeon_palace_check_sword_hit(float hit_x, float hit_y, float hit_w, float hit_h, int damage, Direction dir);

// Processa disparo de flecha contra os chefes ou alvos
void dungeon_palace_check_arrow_hit(float arrow_x, float arrow_y);

// Retorna true se o jogador já conquistou o Elemento do Vento
bool dungeon_palace_has_wind_element(void);

// Retorna true se a coordenada do mundo for um abismo celeste intransponível a pé
bool dungeon_palace_is_pit_tile(float world_x, float world_y);

// Finaliza a masmorra e libera memória alocada
void dungeon_palace_shutdown(void);

#endif // HAL_DUNGEON_PALACE_H
