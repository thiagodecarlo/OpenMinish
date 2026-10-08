#ifndef HAL_LANTERN_H
#define HAL_LANTERN_H

/*
 * ============================================================================
 * include/hal/lantern.h - Lanterna de Chamas (Flame Lantern)
 * ============================================================================
 * Item canônico de The Legend of Zelda: The Minish Cap obtido no Templo das Gotas.
 * Funcionalidades:
 * 1. Iluminação Dinâmica: projeta um círculo de luz suave e oscilante em ambientes
 *    escuros (cavernas, masmorras), com suporte a múltiplas fontes de luz (tochas).
 * 2. Golpe de Chamas: desfere um jato/arco de fogo ardente na direção do herói.
 * 3. Interação Térmica com o Cenário:
 *    - Acende tochas apagadas (TILE_TORCH_UNLIT -> TILE_TORCH_LIT).
 *    - Derrete blocos maciços de gelo (TILE_ICE_BLOCK) liberando caminhos.
 *    - Incinera teias de aranha (TILE_COBWEB) e queima arbustos.
 * 4. Combate Ígneo: causa 2 pontos de dano térmico contínuo e queima monstros.
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/texture.h"
#include <stdbool.h>

#define MAX_LANTERN_SPARKS 24

typedef struct {
    bool  is_active;
    float x;
    float y;
    float vx;
    float vy;
    int   life;
    int   max_life;
    u32   color;
    float size;
} LanternSpark;

typedef struct {
    bool         is_lit;               // Lanterna ligada/acesa emitindo luz
    bool         is_swinging;          // Link desferindo ataque de fogo
    int          swing_timer;          // Duração do arco de chamas
    Direction    swing_dir;            // Direção do golpe de fogo
    float        swing_x;              // Posição da chama no mundo
    float        swing_y;
    float        flicker_phase;        // Oscilação senoidal do raio de luz
    LanternSpark sparks[MAX_LANTERN_SPARKS];
    bool         dark_room_override;   // Modo de ambiente escuro (para teste/dungeon)
} LanternState;

/*
 * Inicializa o subsistema da Flame Lantern.
 */
void lantern_init(void);

/*
 * Retorna o ponteiro para o estado interno da lanterna.
 */
LanternState* lantern_get_state(void);

/*
 * Consulta e controle do estado aceso/apagado.
 */
bool lantern_is_lit(void);
void lantern_set_lit(bool lit);
void lantern_toggle_lit(void);

/*
 * Ações de uso do botão secundário [B] com a Flame Lantern equipada.
 */
void lantern_use_pressed(float link_x, float link_y, Direction dir);
void lantern_use_held(float link_x, float link_y, Direction dir);
void lantern_use_released(void);

/*
 * Atualiza partículas de faíscas, chamas, colisão térmica e timer de oscilação da luz.
 */
void lantern_update(float delta_time, float link_x, float link_y, Tilemap* map);

/*
 * Renderiza o sprite da lanterna de bronze nas mãos de Link e as faíscas/labaredas ativas.
 */
void lantern_render(const Camera* camera);

/*
 * Renderiza a máscara de iluminação dinâmica circular e suave no Framebuffer Virtual.
 * Ilumina a área ao redor de Link e ao redor de todas as tochas acesas na tela.
 */
void lantern_render_lighting(const Camera* camera, float link_x, float link_y, Tilemap* map, bool is_dark_room);

/*
 * Controle de ambiente escuro (cavernas escuras, masmorras e debug).
 */
void lantern_set_dark_room(bool dark);
bool lantern_is_dark_room(void);
void lantern_toggle_dark_room(void);

#endif // HAL_LANTERN_H
