#ifndef HAL_CUCCO_MINIGAME_H
#define HAL_CUCCO_MINIGAME_H

/*
 * ============================================================================
 * include/hal/cucco_minigame.h - Minigame das Galinhas Cucco de Anju (Ato VI)
 * ============================================================================
 * Modela o clássico e divertido minigame das galinhas Cucco de Anju em Hyrule Town:
 *
 * Características Canônicas:
 * - Anju, a criadora de galinhas de Hyrule Town, precisa de ajuda para resgatar
 *   seus Cuccos que escaparam do cercado pelo vilarejo.
 * - 10 Níveis de Dificuldade Progressivos:
 *   - Nível 1: 2 Cuccos em 25 segundos
 *   - Nível 2: 3 Cuccos em 30 segundos
 *   - Nível 3: 4 Cuccos em 35 segundos
 *   - Nível 4: 5 Cuccos em 40 segundos
 *   - Nível 5: 5 Cuccos com pintinhos em 45 segundos
 *   - Nível 6: 6 Cuccos em 45 segundos
 *   - Nível 7: 7 Cuccos em 50 segundos
 *   - Nível 8: 8 Cuccos velozes em 50 segundos
 *   - Nível 9: 9 Cuccos ariscos em 55 segundos
 *   - Nível 10: 10 Cuccos e Galinhas Douradas em 55 segundos (Desafio Supremo!)
 * - IA Dinâmica de Cuccos:
 *   - Cucco Branco Adulto: bica o chão, cacareja, bate asas e foge ao ser abordado.
 *   - Pintinho Amarelo (Chick): minúsculo, hiperativo, escorrega e muda de direção rápido.
 *   - Cucco Dourado (Golden Cucco): voa baixo em saltos planados velozes.
 * - Física de Transporte e Arremesso:
 *   - Link pega o Cucco acima da cabeça com [A]/[R].
 *   - Carregando um Cucco, Link pode planar temporariamente ao pular!
 *   - Arremesso parabólico por cima da cerca ou soltura dentro do cercado.
 *   - Ao entrar no cercado de Anju, o Cucco é resgatado com som de secret chime!
 * - Recompensas:
 *   - Níveis 1 a 9: Conchas Misteriosas (Mysterious Shells) e Rupees generosos.
 *   - Nível 10: Pedaço de Coração Canônico Permanente (Heart Piece) e título de Cucco Master!
 */

#include "gba/types.h"
#include "hal/video.h"
#include "hal/map.h"
#include <stdbool.h>

#define MAX_CUCCOS 12
#define MAX_CUCCO_LEVELS 10

typedef enum {
    CUCCO_TYPE_WHITE = 0,
    CUCCO_TYPE_CHICK,
    CUCCO_TYPE_GOLDEN
} CuccoType;

typedef enum {
    CUCCO_STATE_IDLE = 0,
    CUCCO_STATE_WANDER,
    CUCCO_STATE_PANIC_FLEE,
    CUCCO_STATE_CARRIED,
    CUCCO_STATE_THROWN,
    CUCCO_STATE_PEN_SECURED
} CuccoBehaviorState;

typedef struct {
    bool                active;
    CuccoType           type;
    CuccoBehaviorState  state;
    float               x;
    float               y;
    float               vx;
    float               vy;
    float               z;
    float               vz;
    int                 direction;       // 0: Baixo, 1: Cima, 2: Esquerda, 3: Direita
    int                 anim_frame;
    int                 anim_timer;
    int                 flap_timer;
    int                 peck_timer;
    int                 flee_timer;
    bool                is_flapping;
} CuccoEntity;

typedef enum {
    CUCCO_GAME_INACTIVE = 0,
    CUCCO_GAME_READY,
    CUCCO_GAME_RUNNING,
    CUCCO_GAME_SUCCESS,
    CUCCO_GAME_FAILED,
    CUCCO_GAME_REWARD
} CuccoGameMode;

typedef struct {
    bool          is_active;
    CuccoGameMode mode;
    int           current_level;          // 1 a 10
    int           highest_cleared_level;  // 0 a 10
    int           total_cuccos;
    int           secured_cuccos;
    int           time_remaining_frames;  // 60 fps ticks
    int           time_limit_frames;
    int           carrying_index;         // -1 se nenhuma galinha estiver sendo carregada
    int           state_timer;
    int           banner_timer;
    bool          has_heart_piece_awarded;
    int           reward_rupees;
    int           reward_shells;
    CuccoEntity   cuccos[MAX_CUCCOS];
} CuccoMinigame;

// Inicialização e Ciclo de Vida
void cucco_minigame_init(void);
void cucco_minigame_start_level(int level);
void cucco_minigame_stop(void);
bool cucco_minigame_is_active(void);
bool cucco_minigame_is_carrying(void);

// Lógica e Entrada
void cucco_minigame_handle_action(float link_x, float link_y, int link_dir);
void cucco_minigame_update(float* link_x, float* link_y, int link_dir, bool* link_carrying,
                           int* out_rupees, int* out_shells, int* out_hearts, int max_hearts);
void cucco_minigame_render(const Camera* camera, float link_x, float link_y, int link_dir);
void cucco_minigame_render_hud(void);

// Acesso a Dados
int  cucco_minigame_get_level(void);
int  cucco_minigame_get_highest_cleared(void);
bool cucco_minigame_has_heart_piece(void);

// Persistência
void cucco_minigame_restore(int highest_cleared, bool heart_piece_awarded);
void cucco_minigame_export(int* out_highest_cleared, bool* out_heart_piece_awarded);

#endif // HAL_CUCCO_MINIGAME_H
