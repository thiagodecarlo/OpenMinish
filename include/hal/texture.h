#ifndef HAL_TEXTURE_H
#define HAL_TEXTURE_H

/*
 * ============================================================================
 * include/hal/texture.h - Sistema de Texturas e Operações de Blit 2D
 * ============================================================================
 * Permite que a engine carregue folhas de sprites (.bmp) do disco e as
 * desenhe no Framebuffer Virtual com suporte a transparência (Alpha Blending)
 * e recorte automático de bordas (Clipping).
 */

#include "gba/types.h"

// Representação de uma textura carregada na memória RAM
typedef struct {
    int  width;       // Largura da imagem em pixels
    int  height;      // Altura da imagem em pixels
    u32* pixels;      // Buffer de pixels em formato RGBA8888 (32 bits cada)
} Texture;

/*
 * Carrega uma imagem Bitmap (.bmp) do disco para a memória RAM.
 * Suporta o mecanismo de Mods: se existir um arquivo correspondente na pasta
 * de mods, ele tem prioridade sobre o asset original.
 */
Texture* texture_load_bmp(const char* filepath);

/*
 * Desenha uma sub-região da textura (um sprite ou tile) no Framebuffer Virtual.
 *
 * Parâmetros:
 *  - tex: Ponteiro para a textura de origem.
 *  - src_x, src_y: Posição do sprite dentro da folha (Sprite Sheet).
 *  - src_w, src_h: Dimensões do sprite (ex: 8x8, 16x16 pixels).
 *  - dest_x, dest_y: Coordenadas na tela virtual do jogo onde o sprite será desenhado.
 */
void texture_draw(const Texture* tex,
                  int src_x, int src_y,
                  int src_w, int src_h,
                  int dest_x, int dest_y);

/*
 * Desenha a textura inteira na tela a partir de uma coordenada (dest_x, dest_y).
 */
void texture_draw_full(const Texture* tex, int dest_x, int dest_y);

/*
 * Libera a memória RAM alocada para a textura.
 */
void texture_free(Texture* tex);

#endif // HAL_TEXTURE_H
