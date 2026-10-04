#ifndef HAL_SUBWEAPON_H
#define HAL_SUBWEAPON_H

/*
 * ============================================================================
 * include/hal/subweapon.h - Sistema de Itens Secundários e Armas Equipáveis
 * ============================================================================
 * Modela os itens clássicos de The Legend of Zelda: The Minish Cap:
 * 1. Bumerangue Mágico (Magic Boomerang):
 *    - Trajetória parabólica de ida e retorno teleguiado ao herói.
 *    - Rotação contínua 360° com zunido aerodinâmico.
 *    - Atordoa e fere inimigos (Octoroks, Keese, ChuChus).
 *    - Captura e traz itens distantes (Rupees e Corações) até o herói.
 * 2. Pote Mágico (Gust Jar):
 *    - Cone de vórtice contínuo de sucção de vento com partículas animadas.
 *    - Puxa inimigos, desmascara ChuChus e absorve projéteis de pedra.
 *    - Disparo pressurizado de ar ao soltar o botão de sucção.
 * 3. Botas de Pegasus (Pegasus Boots):
 *    - Arrancada veloz em linha reta rompendo arbustos e atropelando monstros.
 */

#include "gba/types.h"
#include "hal/map.h"
#include <stdbool.h>

typedef enum {
    ITEM_BOOMERANG = 0,   // Bumerangue Mágico (Arremesso, retorno e coleta)
    ITEM_GUST_JAR,        // Pote Mágico / Jarro de Vento (Sucção e disparo de ar)
    ITEM_PEGASUS_BOOTS,   // Botas de Pegasus (Dash veloz de corrida)
    ITEM_BOMBS,           // Bolsa de Bombas (Colocação de bombas, pavio e explosão)
    ITEM_CANE_OF_PACCI,   // Cajado de Pacci (Projétil mágico de inversão e super salto)
    ITEM_BOW,             // Arco e Flechas (Disparo de flechas velozes e ativação de estátuas de olho)
    ITEM_COUNT
} SubweaponType;

/*
 * Inicializa o subsistema de armas secundárias.
 */
void subweapon_init(void);

/*
 * Alterna ciclicamente o item secundário equipado (Bumerangue -> Pote Mágico -> Botas).
 */
void subweapon_cycle(void);

/*
 * Retorna o item atualmente equipado no botão secundário.
 */
SubweaponType subweapon_get_current(void);

/*
 * Define diretamente o item equipado no botão secundário.
 */
void subweapon_set_current(SubweaponType item);

/*
 * Retorna o nome amigável do item equipado.
 */
const char* subweapon_get_name(SubweaponType item);

/*
 * Disparado no instante em que o botão B é pressionado.
 */
void subweapon_use_pressed(float link_x, float link_y, Direction dir);

/*
 * Disparado a cada frame em que o botão B é mantido pressionado (ex: sucção do Pote Mágico).
 */
void subweapon_use_held(float link_x, float link_y, Direction dir);

/*
 * Disparado quando o botão B é liberado (ex: disparo da rajada de ar do Pote Mágico).
 */
void subweapon_use_released(float link_x, float link_y, Direction dir);

/*
 * Atualiza as físicas, trajetórias balísticas, colisões e vórtices das armas ativas.
 */
void subweapon_update(Tilemap* map, float link_x, float link_y, int* link_rupees, int* link_hearts);

/*
 * Renderiza os projéteis (bumerangue, vórtice de vento, rajada de ar) nas coordenadas da câmera.
 */
void subweapon_render(const Camera* cam);

/*
 * Renderiza o ícone do item equipado no HUD superior (caixa do botão [B]).
 */
void subweapon_render_hud_icon(int x, int y);

/*
 * Retorna true se o Pote Mágico estiver ativamente sugando ar (trava a locomoção padrão do Link).
 */
bool subweapon_is_gust_active(void);

/*
 * Retorna a quantidade de bombas restantes no inventário do herói.
 */
int subweapon_get_bomb_count(void);

/*
 * Retorna a capacidade máxima da bolsa de bombas.
 */
int subweapon_get_max_bombs(void);

/*
 * Adiciona bombas ao inventário (ex: recompensa ou compra na loja).
 */
void subweapon_add_bombs(int count);

/*
 * Retorna o deslocamento de tremor de tela (Screen Shake) causado por explosões de bombas.
 */
void subweapon_get_screen_shake(int* out_ox, int* out_oy);

/*
 * Retorna a quantidade de flechas disponíveis na aljava.
 */
int subweapon_get_arrow_count(void);

/*
 * Retorna a capacidade máxima da aljava de flechas.
 */
int subweapon_get_max_arrows(void);

/*
 * Adiciona flechas à aljava (ex: coletando ou comprando na loja).
 */
void subweapon_add_arrows(int count);

#endif // HAL_SUBWEAPON_H
