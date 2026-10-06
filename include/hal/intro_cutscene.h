#ifndef HAL_INTRO_CUTSCENE_H
#define HAL_INTRO_CUTSCENE_H

/*
 * ============================================================================
 * include/hal/intro_cutscene.h - Cutscene de Abertura Canônica (Ato VI)
 * ============================================================================
 * The Legend of Zelda: The Minish Cap - Introdução Canônica Completa:
 *   - Festival de Picori & Torneio de Esgrima de Hyrule
 *   - Cerimônia de Premiação no Pátio do Castelo
 *   - Chegada Sombria de Vaati e destruição do Baú Sagrado com a Lâmina Picori
 *   - Libertação dos monstros ancestrais sobre Hyrule
 *   - Petrificação da Princesa Zelda pela magia negra de Vaati
 *   - O despertar de Link com Smith e o Rei Daltus: A Busca pelos Picori
 */

#include "gba/types.h"
#include "hal/texture.h"
#include <stdbool.h>

void intro_cutscene_set_npcs_texture(const Texture* tex);
void intro_cutscene_set_bosses_texture(const Texture* tex);
void intro_cutscene_set_link_texture(const Texture* tex);
void intro_cutscene_set_castle_texture(const Texture* tex);
void intro_cutscene_set_hud_texture(const Texture* tex);
void intro_cutscene_set_enemies_texture(const Texture* tex);

typedef enum {
    INTRO_STAGE_ROOM_WAKEUP = 0,     // 0. Quarto de Link: O Despertar & Chegada de Zelda e Mestre Smith
    INTRO_STAGE_FESTIVAL_PARADE,     // 1. Link e Princesa Zelda no Festival de Picori
    INTRO_STAGE_TOURNAMENT_WIN,      // 2. O Vencedor Misterioso do Torneio de Esgrima: Vaati
    INTRO_STAGE_CHEST_CEREMONY,      // 3. Abertura do Baú Selado e quebra da Lâmina Picori
    INTRO_STAGE_MONSTERS_RELEASED,   // 4. Erupção de energia sombria e monstros sobre o reino
    INTRO_STAGE_ZELDA_CURSED,        // 5. O Feitiço Sombrio: Princesa Zelda transformada em pedra
    INTRO_STAGE_KING_AUDIENCE,       // 6. O Despertar na Sala do Trono: O Rei Daltus e Mestre Smith
    INTRO_STAGE_FINISHED             // 7. Fim da introdução, transição suave para o gameplay
} IntroCutsceneStage;

/*
 * Inicializa os estados, contadores de tempo e partículas da cutscene.
 */
void intro_cutscene_init(void);

/*
 * Inicia a reprodução da cutscene de introdução.
 */
void intro_cutscene_start(void);

/*
 * Retorna true se a cutscene estiver em reprodução ativa.
 */
bool intro_cutscene_is_active(void);

/*
 * Atualiza lógica de animação, movimentação, transições e diálogos da cutscene.
 */
void intro_cutscene_update(void);

/*
 * Renderiza o palco atual, personagens, efeitos de magia e diálogos cinematográficos.
 */
void intro_cutscene_render(void);

/*
 * Pula ou finaliza imediatamente a cutscene.
 */
void intro_cutscene_skip(void);

/*
 * Retorna o estágio atual da cutscene.
 */
IntroCutsceneStage intro_cutscene_get_stage(void);

#endif // HAL_INTRO_CUTSCENE_H
