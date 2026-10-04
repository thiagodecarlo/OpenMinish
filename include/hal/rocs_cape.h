#ifndef HAL_ROCS_CAPE_H
#define HAL_ROCS_CAPE_H

/*
 * ============================================================================
 * include/hal/rocs_cape.h - Capa de Roc (Roc's Cape)
 * ============================================================================
 * Modela a icônica Capa de Roc de The Legend of Zelda: The Minish Cap:
 *
 * 1. Salto Acrobático (Acrobatic Leaping):
 *    - Pressionar o botão equipado ([B]) faz Link saltar verticalmente no ar.
 *    - Suporta salto duplo (Double-Jump) ou impulsão adicional no ar.
 *    - Sombra dinâmica no solo que escala suavemente conforme a altitude aumenta.
 *
 * 2. Voo Planado (Gliding / Fluttering):
 *    - Manter o botão pressionado no ar ativa o modo planador.
 *    - A taxa de queda é drasticamente reduzida (sustentação aerodinâmica).
 *    - Velocidade horizontal ágil permitindo cruzar grandes abismos, buracos e espinhos.
 *    - Animação do tecido rubro esvoaçando com acabamento de plumas brancas e broche dourado.
 *
 * 3. Captura de Correntes e Redemoinhos (Updraft Catching):
 *    - Ao planar sobre redemoinhos ou correntes de ar ascendentes, a capa infla
 *      e projeta o herói a grandes altitudes para cruzar céus estratosféricos.
 *
 * 4. Ataque Descendente (Down-Thrust / Ground-Pound):
 *    - Se o jogador golpear com a espada ([A]) enquanto estiver no ar, Link crava
 *      a lâmina para baixo com ambas as mãos, despencando em mergulho veloz.
 *    - No impacto com o solo, gera uma onda de choque expansiva que atordoa e fere
 *      inimigos em área e esmaga obstáculos fissurados ou montes de terra!
 */

#include "gba/types.h"
#include "hal/map.h"
#include <stdbool.h>

#define ROCS_CAPE_MAX_HEIGHT      26.0f
#define ROCS_CAPE_JUMP_IMPULSE     4.4f
#define ROCS_CAPE_DOUBLE_IMPULSE   3.6f
#define ROCS_CAPE_GLIDE_TERMINAL_V -0.38f
#define ROCS_CAPE_GRAVITY_NORMAL   0.24f
#define ROCS_CAPE_GRAVITY_GLIDE    0.06f

typedef struct {
    bool  is_jumping;
    bool  is_gliding;
    bool  is_down_thrust;
    bool  can_double_jump;

    float z;                   // Altitude atual acima do piso (pixels)
    float vz;                  // Velocidade vertical
    int   flutter_timer;       // Animação de drapejar do manto
    int   glide_timer;         // Duração do planeio
    int   air_timer;           // Tempo total no ar

    // Efeito de Impacto no Solo (Down-Thrust Shockwave)
    int   shockwave_timer;
    float shockwave_x;
    float shockwave_y;
    float shockwave_radius;

    // Resgate de segurança contra queda em abismos
    float last_safe_x;
    float last_safe_y;
    bool  fell_in_pit;
} RocsCapeState;

/*
 * Inicializa os parâmetros da Capa de Roc.
 */
void rocs_cape_init(void);

/*
 * Acionado quando o botão do item é pressionado: realiza o salto ou salto duplo.
 */
void rocs_cape_use_pressed(float* player_x, float* player_y, Direction dir,
                           float* player_z, float* player_vz, bool* player_is_jumping);

/*
 * Acionado a cada frame em que o botão é mantido: ativa e mantém o planeio suave.
 */
void rocs_cape_use_held(float* player_z, float* player_vz, bool player_is_jumping);

/*
 * Acionado quando o botão é liberado: desativa o planeio, restaurando a gravidade normal.
 */
void rocs_cape_use_released(void);

/*
 * Acionado ao golpear com a espada no ar: executa o ataque perfurante descendente (Down-Thrust).
 */
bool rocs_cape_try_down_thrust(float* player_z, float* player_vz, bool player_is_jumping);

/*
 * Atualiza a física de voo, gravidade, travessia de abismos e ondas de choque.
 */
void rocs_cape_update(float* player_x, float* player_y, Direction player_dir, bool is_moving,
                      float* player_z, float* player_vz, bool* player_is_jumping,
                      Tilemap* map, int* player_hearts);

/*
 * Impulsiona Link ao capturar um redemoinho ascensional.
 */
void rocs_cape_catch_updraft(float lift_power, float* player_z, float* player_vz, bool* player_is_jumping);

/*
 * Retorna true se Link estiver planando ativamente.
 */
bool rocs_cape_is_gliding(void);

/*
 * Retorna true se Link estiver executando o ataque descendente.
 */
bool rocs_cape_is_down_thrust(void);

/*
 * Renderiza a capa esvoaçante adaptada à orientação e altitude do herói.
 */
void rocs_cape_render(const Camera* camera, float player_x, float player_y, Direction dir, float player_z);

/*
 * Renderiza a sombra projetada no solo durante o salto e planeio.
 */
void rocs_cape_render_shadow(const Camera* camera, float player_x, float player_y, float player_z);

/*
 * Renderiza a onda de choque gerada pelo impacto do Down-Thrust no solo.
 */
void rocs_cape_render_shockwave(const Camera* camera);

/*
 * Retorna o ponteiro para o estado interno da Capa de Roc.
 */
RocsCapeState* rocs_cape_get_state(void);

/*
 * Finaliza o subsistema.
 */
void rocs_cape_shutdown(void);

#endif // HAL_ROCS_CAPE_H
