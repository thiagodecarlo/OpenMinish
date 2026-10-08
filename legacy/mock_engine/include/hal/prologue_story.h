#ifndef HAL_PROLOGUE_STORY_H
#define HAL_PROLOGUE_STORY_H

/*
 * ============================================================================
 * include/hal/prologue_story.h - Prólogo Narrativo: A Lenda dos Picori
 * ============================================================================
 * Implementa a abertura canônica de The Legend of Zelda: The Minish Cap
 * em estilo pergaminho e vitrais dourados:
 *   1. Painel 1: As Trevas sobre Hyrule ("A long, long time ago...")
 *   2. Painel 2: A Descida dos Picori com a Luz Dourada e a Lâmina Picori
 *   3. Painel 3: O Herói dos Homens selando os monstros no Baú Sagrado
 *   4. Painel 4: O Festival dos Picori e o retorno da paz ao reino
 */

#include "gba/types.h"
#include <stdbool.h>

/*
 * Inicializa os recursos do prólogo e carrega o atlas de ilustrações.
 */
void prologue_story_init(void);

/*
 * Inicia a reprodução do prólogo da história.
 */
void prologue_story_start(void);

/*
 * Retorna true se o prólogo estiver em execução.
 */
bool prologue_story_is_active(void);

/*
 * Atualiza o temporizador, transições e lê inputs para avançar ou pular.
 */
void prologue_story_update(void);

/*
 * Renderiza o painel ativo, caixa de texto dourada e efeitos de fade.
 */
void prologue_story_render(void);

/*
 * Pula o prólogo e avança imediatamente para a Tela de Título.
 */
void prologue_story_skip(void);

/*
 * Libera os recursos alocados na memória RAM.
 */
void prologue_story_shutdown(void);

#endif // HAL_PROLOGUE_STORY_H
