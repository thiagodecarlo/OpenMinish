#include "gfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ============================================================================
 * tools/extractor/gfx.c - Implementação do Decodificador Gráfico do GBA
 * ============================================================================
 */

u32 bgr555_to_rgba8888(u16 bgr_color, u8 alpha) {
    // 1. Extração dos canais de 5 bits
    u8 r5 = bgr_color & 0x1F;
    u8 g5 = (bgr_color >> 5) & 0x1F;
    u8 b5 = (bgr_color >> 10) & 0x1F;

    // 2. Mapeamento matemático de 5 bits (0-31) para 8 bits (0-255)
    u8 r8 = (u8)((r5 * 255) / 31);
    u8 g8 = (u8)((g5 * 255) / 31);
    u8 b8 = (u8)((b5 * 255) / 31);

    // 3. Montagem do inteiro de 32 bits no padrão RGBA
    return ((u32)r8 << 24) | ((u32)g8 << 16) | ((u32)b8 << 8) | (u32)alpha;
}

void decode_palette(const u16* gba_pal, u32* out_rgba_pal, int num_colors) {
    for (int i = 0; i < num_colors; i++) {
        // No GBA, o índice 0 de uma paleta de sprite é sempre a cor transparente!
        if (i == 0) {
            out_rgba_pal[0] = 0x00000000; // RGBA transparente (preto com alpha 0)
        } else {
            out_rgba_pal[i] = bgr555_to_rgba8888(gba_pal[i], 0xFF);
        }
    }
}

void decode_tile_4bpp(const u8* tile_data,
                      const u32* palette,
                      u32* out_image,
                      int out_x, int out_y,
                      int image_width) {
    // Um Tile do GBA tem sempre 8x8 pixels
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // Em 4bpp, cada byte na memória guarda 2 pixels:
            // - Metade inferior (nibble baixo, bits 0..3): pixel par
            // - Metade superior (nibble alto, bits 4..7): pixel ímpar
            int byte_index = (y * 4) + (x / 2);
            u8 byte_val = tile_data[byte_index];

            u8 color_index = (x % 2 == 0) ? (byte_val & 0x0F) : ((byte_val >> 4) & 0x0F);

            // Grava o pixel RGBA decodificado na imagem final
            int dest_index = (out_y + y) * image_width + (out_x + x);
            out_image[dest_index] = palette[color_index];
        }
    }
}

bool export_tilesheet_bmp(const char* filepath,
                          const u8* tile_data,
                          size_t data_size,
                          const u32* palette,
                          int width_in_tiles) {
    if (!tile_data || data_size < 32 || !palette || width_in_tiles <= 0) {
        return false;
    }

    int num_tiles = (int)(data_size / 32);
    int height_in_tiles = (num_tiles + width_in_tiles - 1) / width_in_tiles;
    int img_w = width_in_tiles * 8;
    int img_h = height_in_tiles * 8;

    u32* rgba_buffer = (u32*)calloc(img_w * img_h, sizeof(u32));
    if (!rgba_buffer) {
        return false;
    }

    for (int t = 0; t < num_tiles; t++) {
        int tx = (t % width_in_tiles) * 8;
        int ty = (t / width_in_tiles) * 8;
        decode_tile_4bpp(&tile_data[t * 32], palette, rgba_buffer, tx, ty, img_w);
    }

    bool ok = save_bmp_image(filepath, rgba_buffer, img_w, img_h);
    free(rgba_buffer);
    return ok;
}

bool export_assembled_sprites_bmp(const char* filepath,
                                  const u8* sprite_data,
                                  size_t data_size,
                                  const u32* palette,
                                  int sprites_per_row) {
    if (!sprite_data || data_size < 128 || !palette || sprites_per_row <= 0) {
        return false;
    }

    int num_tiles = (int)(data_size / 32);
    int num_sprites = num_tiles / 4;
    int num_rows = (num_sprites + sprites_per_row - 1) / sprites_per_row;
    int img_w = sprites_per_row * 16;
    int img_h = num_rows * 16;

    u32* rgba_buffer = (u32*)calloc(img_w * img_h, sizeof(u32));
    if (!rgba_buffer) return false;

    // Disposição canônica 1D do GBA para metatiles de 16x16:
    // Tile 0: Top-Left (0, 0)
    // Tile 1: Top-Right (8, 0)
    // Tile 2: Bottom-Left (0, 8)
    // Tile 3: Bottom-Right (8, 8)
    int tile_offsets[4][2] = { {0, 0}, {8, 0}, {0, 8}, {8, 8} };

    for (int s = 0; s < num_sprites; s++) {
        int sx = (s % sprites_per_row) * 16;
        int sy = (s / sprites_per_row) * 16;

        for (int t = 0; t < 4; t++) {
            const u8* t_data = &sprite_data[(s * 4 + t) * 32];
            int tx = sx + tile_offsets[t][0];
            int ty = sy + tile_offsets[t][1];
            decode_tile_4bpp(t_data, palette, rgba_buffer, tx, ty, img_w);
        }
    }

    bool ok = save_bmp_image(filepath, rgba_buffer, img_w, img_h);
    free(rgba_buffer);
    return ok;
}

bool export_link_sprites_bmp(const char* filepath,
                             const u8* sprite_data,
                             size_t data_size,
                             const u32* palette,
                             int num_frames,
                             int frames_per_row) {
    if (!sprite_data || data_size < 256 || !palette || num_frames <= 0 || frames_per_row <= 0) {
        return false;
    }

    int num_rows = (num_frames + frames_per_row - 1) / frames_per_row;
    int img_w = frames_per_row * 16;
    int img_h = num_rows * 24;

    u32* rgba_buffer = (u32*)calloc(img_w * img_h, sizeof(u32));
    if (!rgba_buffer) return false;

    // Cada frame canônico do Link ocupa 256 bytes na ROM:
    // 3 fatias verticais de 16x8 pixels (64 bytes cada = 2 tiles de 8x8)
    // Fatia 0: Gorro/Topo da Cabeça (y: 0..7)
    // Fatia 1: Rosto e Cabelo (y: 8..15)
    // Fatia 2: Corpo, Túnica e Botas (y: 16..23)
    // Fatia 3: Padding/sombra (vazio)
    for (int f = 0; f < num_frames; f++) {
        size_t frame_offset = (size_t)f * 256;
        if (frame_offset + 192 > data_size) break;

        int fx = (f % frames_per_row) * 16;
        int fy = (f / frames_per_row) * 24;

        for (int chunk = 0; chunk < 3; chunk++) {
            size_t c_offset = frame_offset + (size_t)chunk * 64;
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 16; x++) {
                    int t = x / 8;
                    int tx = x % 8;
                    size_t byte_idx = c_offset + t * 32 + y * 4 + tx / 2;
                    u8 b = sprite_data[byte_idx];
                    u8 c = (tx % 2 == 0) ? (b & 0x0F) : ((b >> 4) & 0x0F);

                    int dest_idx = (fy + chunk * 8 + y) * img_w + (fx + x);
                    rgba_buffer[dest_idx] = palette[c];
                }
            }
        }
    }

    bool ok = save_bmp_image(filepath, rgba_buffer, img_w, img_h);
    free(rgba_buffer);
    return ok;
}


#pragma pack(push, 1)
// Estrutura de Cabeçalho de Arquivo Bitmap (.BMP) conforme especificação da Microsoft
typedef struct {
    u16 bfType;       // Sempre 'BM' (0x4D42)
    u32 bfSize;       // Tamanho total do arquivo em bytes
    u16 bfReserved1;  // Reservado (0)
    u16 bfReserved2;  // Reservado (0)
    u32 bfOffBits;    // Deslocamento até os bytes reais dos pixels (54 bytes)
} BmpFileHeader;

typedef struct {
    u32 biSize;          // Tamanho deste cabeçalho (40 bytes)
    s32 biWidth;         // Largura da imagem em pixels
    s32 biHeight;        // Altura da imagem (negativo = ordem de cima para baixo)
    u16 biPlanes;        // Planos de cor (sempre 1)
    u16 biBitCount;      // Bits por pixel (32 bits = BGRA)
    u32 biCompression;   // Compressão (0 = sem compressão / BI_RGB)
    u32 biSizeImage;     // Tamanho da imagem (width * height * 4)
    s32 biXPelsPerMeter; // Resolução horizontal (0)
    s32 biYPelsPerMeter; // Resolução vertical (0)
    u32 biClrUsed;       // Cores na paleta (0)
    u32 biClrImportant;  // Cores importantes (0)
} BmpInfoHeader;
#pragma pack(pop)

bool save_bmp_image(const char* filepath, const u32* rgba_pixels, int width, int height) {
    FILE* f = fopen(filepath, "wb");
    if (!f) {
        printf("[ERRO BMP] Nao foi possivel abrir o arquivo para escrita: %s\n", filepath);
        return false;
    }

    u32 image_bytes = width * height * 4;

    BmpFileHeader file_hdr;
    file_hdr.bfType = 0x4D42; // Letras 'B' e 'M' em ASCII
    file_hdr.bfSize = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader) + image_bytes;
    file_hdr.bfReserved1 = 0;
    file_hdr.bfReserved2 = 0;
    file_hdr.bfOffBits = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);

    BmpInfoHeader info_hdr;
    memset(&info_hdr, 0, sizeof(info_hdr));
    info_hdr.biSize = sizeof(BmpInfoHeader);
    info_hdr.biWidth = width;
    info_hdr.biHeight = -height; // Altura negativa define orientação Top-to-Bottom
    info_hdr.biPlanes = 1;
    info_hdr.biBitCount = 32;
    info_hdr.biCompression = 0;
    info_hdr.biSizeImage = image_bytes;

    fwrite(&file_hdr, sizeof(BmpFileHeader), 1, f);
    fwrite(&info_hdr, sizeof(BmpInfoHeader), 1, f);

    // O formato BMP armazena 32 bits como BGRA (Blue, Green, Red, Alpha)
    // Nosso formato interno é RGBA, portanto fazemos a escrita convertendo a ordem dos canais
    for (int i = 0; i < width * height; i++) {
        u32 pixel = rgba_pixels[i];
        u8 r = (pixel >> 24) & 0xFF;
        u8 g = (pixel >> 16) & 0xFF;
        u8 b = (pixel >> 8) & 0xFF;
        u8 a = pixel & 0xFF;

        u8 bgra[4] = { b, g, r, a };
        fwrite(bgra, 4, 1, f);
    }

    fclose(f);
    printf("[BMP] Imagem salva com sucesso: %s (%dx%d pixels)\n", filepath, width, height);
    return true;
}
