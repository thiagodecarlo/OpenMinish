#ifndef HAL_FONT_H
#define HAL_FONT_H

/*
 * ============================================================================
 * include/hal/font.h - Motor de Renderização de Fontes Bitmap Retrô 8x8
 * ============================================================================
 * Implementa renderização em software de tipografia pixel art estilo clássico
 * de The Legend of Zelda para diálogos, menus e HUD.
 * Totalmente autônomo, embutido em código C puro (Zero-ROM e Zero dependências).
 */

#include "gba/types.h"
#include <stdbool.h>

// Caracteres Especiais Zelda
#define FONT_CHAR_HEART       '\x01'
#define FONT_CHAR_RUPEE       '\x02'
#define FONT_CHAR_ARROW_DOWN  '\x03'
#define FONT_CHAR_BTN_A       '\x04'
#define FONT_CHAR_BTN_B       '\x05'

/*
 * Inicializa a tabela de glifos da fonte bitmap.
 */
void font_init(void);

/*
 * Renderiza um caractere 8x8 nas coordenadas virtuais (x, y).
 * Se shadow for true, desenha uma sombra projetada com 1px de offset.
 */
void font_draw_char(int x, int y, char c, u32 color, bool shadow);

/*
 * Renderiza uma linha de texto terminada em '\0'.
 * Retorna a coordenada X após o término do texto renderizado.
 */
int font_draw_text(int x, int y, const char* text, u32 color, bool shadow);

/*
 * Renderiza até max_chars caracteres de uma string (utilizado no efeito typewriter).
 * Retorna a quantidade de caracteres efetivamente impressos.
 */
int font_draw_text_len(int x, int y, const char* text, int max_chars, u32 color, bool shadow);

/*
 * Retorna o comprimento horizontal em pixels de um texto.
 */
int font_get_text_width(const char* text);

#endif // HAL_FONT_H
