#ifndef GFX_H
#define GFX_H

/*
 * ============================================================================
 * tools/extractor/gfx.h - Decodificador Gráfico e Conversor de Cores do GBA
 * ============================================================================
 */

#include "gba/types.h"

/*
 * Converte uma cor de 15 bits do GBA (BGR555) para 32 bits (RGBA8888).
 * No GBA: Bits 0..4 = Red, Bits 5..9 = Green, Bits 10..14 = Blue.
 */
u32 bgr555_to_rgba8888(u16 bgr_color, u8 alpha);

/*
 * Decodifica uma paleta inteira de cores do GBA para cores RGBA modernas.
 */
void decode_palette(const u16* gba_pal, u32* out_rgba_pal, int num_colors);

/*
 * Decodifica um Tile de 8x8 pixels em formato 4bpp (4 bits por pixel).
 * Um tile de 4bpp ocupa 32 bytes no cartucho (64 pixels / 2 pixels por byte).
 *
 * Parâmetros:
 *  - tile_data: Ponteiro para os 32 bytes do tile na memória.
 *  - palette: Paleta de 16 cores RGBA (cada índice de 0 a 15 aponta aqui).
 *  - out_image: Buffer de imagem de destino em RGBA.
 *  - out_x, out_y: Posição onde o tile será inserido na imagem final.
 *  - image_width: Largura total da imagem final em pixels.
 */
void decode_tile_4bpp(const u8* tile_data,
                      const u32* palette,
                      u32* out_image,
                      int out_x, int out_y,
                      int image_width);

/*
 * Decodifica um conjunto de tiles 4bpp e salva diretamente em um arquivo BMP (.bmp).
 * Organiza os tiles em grade retangular (ex: 16 tiles por linha = 128 pixels de largura).
 */
bool export_tilesheet_bmp(const char* filepath,
                          const u8* tile_data,
                          size_t data_size,
                          const u32* palette,
                          int width_in_tiles);

/*
 * Salva uma matriz de pixels RGBA em um arquivo de imagem Bitmap (.bmp) padrão.
 * Compatível nativamente com Windows, macOS, Linux e qualquer navegador.
 */
bool save_bmp_image(const char* filepath, const u32* rgba_pixels, int width, int height);

#endif // GFX_H
