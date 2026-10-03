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
 * Decodifica sprites de 16x16 pixels montados a partir de grupos de 4 tiles de 8x8
 * (disposição 1D canônica do GBA: Top-Left, Top-Right, Bottom-Left, Bottom-Right)
 * e salva em arquivo BMP (.bmp) com canal alfa nos pixels transparentes.
 */
bool export_assembled_sprites_bmp(const char* filepath,
                                  const u8* sprite_data,
                                  size_t data_size,
                                  const u32* palette,
                                  int sprites_per_row);

/*
 * Decodifica sprites do Link (formato canônico 16x24 composto por 3 fatias de 16x8)
 * a partir do banco gráfico de animações do GBA e salva em BMP com canal alfa.
 */
bool export_link_sprites_bmp(const char* filepath,
                             const u8* sprite_data,
                             size_t data_size,
                             const u32* palette,
                             int num_frames,
                             int frames_per_row);

/*
 * Salva uma matriz de pixels RGBA em um arquivo de imagem Bitmap (.bmp) padrão.
 * Compatível nativamente com Windows, macOS, Linux e qualquer navegador.
 */
bool save_bmp_image(const char* filepath, const u32* rgba_pixels, int width, int height);

/*
 * Extrai e renderiza o mapa autêntico de Minish Woods (63x63 metatiles = 1008x1008 pixels)
 * a partir das estruturas canônicas da ROM (gMapData e paletas de área).
 * Gera os arquivos:
 *  - assets/regions/<region>/map_woods.bmp (1008x1008 pixels, 32bpp)
 *  - assets/regions/<region>/map_woods_collision.bin (63x63 = 3969 bytes de colisão: 0 livre, 1 sólido)
 */
bool export_authentic_map_woods(const char* region_tag,
                                const u8* rom_buffer,
                                size_t rom_size,
                                u32 map_data_base,
                                u32 woods_pal_offset);

#endif // GFX_H

