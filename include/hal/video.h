#ifndef HAL_VIDEO_H
#define HAL_VIDEO_H

/*
 * ============================================================================
 * include/hal/video.h - Camada de Abstração de Hardware (Vídeo e Display)
 * ============================================================================
 * Esta camada substitui a placa gráfica física (PPU) do Game Boy Advance.
 * Em vez de falar diretamente com registradores de LCD, o jogo escreve em
 * um "Framebuffer Virtual", e esta camada usa a SDL2 para enviar os pixels
 * para a GPU moderna do computador hospedeiro (Windows, Mac ou Linux).
 */

#include "gba/types.h"

// Estrutura que guarda a configuração e o estado do subsistema de vídeo
typedef struct {
    int  scale;            // Fator de escala (ex: 3x, 4x)
    bool widescreen;       // Se o modo 16:9 está ativo
    int  render_width;     // Largura atual da tela (240 ou 284 widescreen)
    int  render_height;    // Altura atual da tela (160)
    u32* framebuffer;      // Ponteiro para o array de pixels RGBA (32 bits cada)
} HalVideoContext;

/*
 * Inicializa a janela da SDL2, o contexto gráfico e aloca o Framebuffer Virtual.
 * Retorna true se a janela abriu com sucesso, ou false caso ocorra algum erro.
 */
bool hal_video_init(const char* window_title, int scale_factor, bool enable_widescreen);

/*
 * Pinta um único pixel no Framebuffer Virtual com coordenadas (x, y) e cor RGBA.
 * Formato de cor: 0xRRGGBBAA (ex: 0xFF0000FF = Vermelho Opaco).
 */
void hal_video_put_pixel(int x, int y, u32 color_rgba);

/*
 * Limpa todo o Framebuffer Virtual com uma cor sólida de fundo.
 */
void hal_video_clear(u32 color_rgba);

/*
 * Envia o Framebuffer Virtual para a tela do computador (apresentação gráfica).
 */
void hal_video_render_frame(void);

/*
 * Fecha a janela da SDL2 e libera a memória alocada do framebuffer.
 */
void hal_video_shutdown(void);

/*
 * Retorna o contexto de vídeo atual (para consulta de dimensões).
 */
const HalVideoContext* hal_video_get_context(void);

#endif // HAL_VIDEO_H
