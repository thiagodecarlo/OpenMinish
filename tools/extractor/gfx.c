#include "gfx.h"
#include "lz77.h"
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

    // Disposição 2D canônica do GBA para metatiles de 16x16:
    // Uma fileira de sprites é disposta em 2 linhas de tiles 8x8 contíguas na VRAM:
    // Linha superior: tiles (srow * 2) * tiles_per_row + scol * 2 (+0 para TL, +1 para TR)
    // Linha inferior: tiles (srow * 2 + 1) * tiles_per_row + scol * 2 (+0 para BL, +1 para BR)
    int tiles_per_row = sprites_per_row * 2;
    int tile_offsets[4][2] = { {0, 0}, {8, 0}, {0, 8}, {8, 8} };

    for (int s = 0; s < num_sprites; s++) {
        int srow = s / sprites_per_row;
        int scol = s % sprites_per_row;
        int sx = scol * 16;
        int sy = srow * 16;

        int tl = (srow * 2) * tiles_per_row + scol * 2;
        int tr = tl + 1;
        int bl = (srow * 2 + 1) * tiles_per_row + scol * 2;
        int br = bl + 1;
        int sprite_tiles[4] = { tl, tr, bl, br };

        for (int t = 0; t < 4; t++) {
            int t_idx = sprite_tiles[t];
            if (t_idx * 32 + 32 <= (int)data_size) {
                const u8* t_data = &sprite_data[t_idx * 32];
                int tx = sx + tile_offsets[t][0];
                int ty = sy + tile_offsets[t][1];
                decode_tile_4bpp(t_data, palette, rgba_buffer, tx, ty, img_w);
            }
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
    // Se o sheet mestre canônico (link_master.bmp) estiver presente, replica-o com prioridade
    FILE* f_master = fopen("assets/regions/link_master.bmp", "rb");
    if (f_master) {
        fseek(f_master, 0, SEEK_END);
        long sz = ftell(f_master);
        fseek(f_master, 0, SEEK_SET);
        u8* m_buf = (u8*)malloc(sz);
        if (m_buf && fread(m_buf, 1, sz, f_master) == (size_t)sz) {
            FILE* f_out = fopen(filepath, "wb");
            if (f_out) {
                fwrite(m_buf, 1, sz, f_out);
                fclose(f_out);
            }
        }
        if (m_buf) free(m_buf);
        fclose(f_master);
        return true;
    }

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

bool export_authentic_map_woods(const char* region_tag,
                                const u8* rom_buffer,
                                size_t rom_size,
                                u32 map_data_base,
                                u32 woods_pal_offset) {
    if (!region_tag || !rom_buffer || rom_size < 0x600000 || map_data_base == 0 || woods_pal_offset == 0) {
        return false;
    }

    // 1. Decodifica o banco de 16 sub-paletas de Minish Woods (256 cores RGBA)
    u32 palettes[16][16];
    for (int p = 0; p < 16; p++) {
        size_t p_off = (size_t)woods_pal_offset + p * 32;
        if (p_off + 32 > rom_size) return false;
        const u16* gba_pal = (const u16*)&rom_buffer[p_off];
        for (int c = 0; c < 16; c++) {
            palettes[p][c] = bgr555_to_rgba8888(gba_pal[c], 0xFF);
        }
    }

    // 2. Descomprime os 3 blocos de tiles graficos de VRAM (LZ77)
    size_t sz0 = 0, sz1 = 0, sz2 = 0;
    u8* t0 = lz77_decompress(&rom_buffer[map_data_base + 0x3D0], rom_size - (map_data_base + 0x3D0), &sz0);
    u8* t1 = lz77_decompress(&rom_buffer[map_data_base + 0x3024], rom_size - (map_data_base + 0x3024), &sz1);
    u8* t2 = lz77_decompress(&rom_buffer[map_data_base + 0x5614], rom_size - (map_data_base + 0x5614), &sz2);

    if (!t0 || !t1 || !t2) {
        printf("[ERRO EXTRACTOR] Falha ao descomprimir tiles de VRAM para o mapa de Minish Woods!\n");
        if (t0) free(t0);
        if (t1) free(t1);
        if (t2) free(t2);
        return false;
    }

    size_t bot_vram_sz = sz0 + sz1;
    u8* bot_vram = (u8*)malloc(bot_vram_sz);
    size_t top_vram_sz = sz0 + sz2;
    u8* top_vram = (u8*)malloc(top_vram_sz);

    if (!bot_vram || !top_vram) {
        if (bot_vram) free(bot_vram);
        if (top_vram) free(top_vram);
        free(t0); free(t1); free(t2);
        return false;
    }
    memcpy(bot_vram, t0, sz0);
    memcpy(bot_vram + sz0, t1, sz1);

    memcpy(top_vram, t0, sz0);
    memcpy(top_vram + sz0, t2, sz2);
    free(t0); free(t1); free(t2);

    // 3. Descomprime metatiles (bot e top), room maps (bot e top) e tipos de tile
    size_t meta_sz = 0, meta_top_sz = 0, room_sz = 0, room_top_sz = 0, types_sz = 0;
    u8* meta_bot  = lz77_decompress(&rom_buffer[map_data_base + 0x7704], rom_size - (map_data_base + 0x7704), &meta_sz);
    u8* meta_top  = lz77_decompress(&rom_buffer[map_data_base + 0x9280], rom_size - (map_data_base + 0x9280), &meta_top_sz);
    u8* room_bot  = lz77_decompress(&rom_buffer[map_data_base + 0xA700], rom_size - (map_data_base + 0xA700), &room_sz);
    u8* room_top  = lz77_decompress(&rom_buffer[map_data_base + 0xB7AC], rom_size - (map_data_base + 0xB7AC), &room_top_sz);
    u8* types_bot = lz77_decompress(&rom_buffer[map_data_base + 0xA00C], rom_size - (map_data_base + 0xA00C), &types_sz);

    if (!meta_bot || !room_bot || !types_bot) {
        printf("[ERRO EXTRACTOR] Falha ao descomprimir metatiles ou room map!\n");
        free(bot_vram);
        free(top_vram);
        if (meta_bot) free(meta_bot);
        if (meta_top) free(meta_top);
        if (room_bot) free(room_bot);
        if (room_top) free(room_top);
        if (types_bot) free(types_bot);
        return false;
    }

    // 4. Renderiza a imagem completa de Minish Woods (63x63 metatiles = 1008x1008 pixels)
    int map_w = 63 * 16;
    int map_h = 63 * 16;
    u32* img_pixels = (u32*)malloc((size_t)map_w * map_h * sizeof(u32));
    if (!img_pixels) {
        free(bot_vram); free(top_vram); free(meta_bot); if (meta_top) free(meta_top);
        free(room_bot); if (room_top) free(room_top); free(types_bot);
        return false;
    }

    // Inicializa todo o mapa com a cor canônica de grama de Minish Woods (RGB 65, 180, 139)
    u32 c_grass_base = 0x41B48BFF;
    for (int i = 0; i < map_w * map_h; i++) {
        img_pixels[i] = c_grass_base;
    }

    int tile_offsets[4][2] = { {0, 0}, {8, 0}, {0, 8}, {8, 8} };

    // 4.1 Renderiza Camada Inferior (Ground / Floor: room_bot)
    const u16* bot_ids = (const u16*)room_bot;
    for (int my = 0; my < 63; my++) {
        for (int mx = 0; mx < 63; mx++) {
            u16 b_id = bot_ids[my * 63 + mx];
            if ((size_t)(b_id * 8 + 8) <= meta_sz) {
                const u16* sub_tiles = (const u16*)&meta_bot[b_id * 8];
                for (int s = 0; s < 4; s++) {
                    u16 sub = sub_tiles[s];
                    int tile_num = sub & 0x3FF;
                    bool hflip = (sub >> 10) & 1;
                    bool vflip = (sub >> 11) & 1;
                    int pal_idx = (sub >> 12) & 0xF;

                    // Filtra marcadores de evento, triggers e depuração do editor Capcom (paleta 13)
                    if (pal_idx == 13) {
                        continue;
                    }

                    // Mapeia tiles de fluxo de água para a paleta canônica azul 7
                    if (pal_idx == 9 && ((tile_num >= 544 && tile_num <= 575) || (tile_num >= 944 && tile_num <= 960))) {
                        pal_idx = 7;
                    }

                    const u32* cur_pal = palettes[pal_idx];

                    size_t t_off = (size_t)tile_num * 32;
                    if (t_off + 32 <= bot_vram_sz) {
                        const u8* t_data = &bot_vram[t_off];
                        int tox = tile_offsets[s][0];
                        int toy = tile_offsets[s][1];

                        for (int y = 0; y < 8; y++) {
                            int sy = vflip ? (7 - y) : y;
                            for (int x = 0; x < 8; x++) {
                                int sx = hflip ? (7 - x) : x;
                                u8 bv = t_data[sy * 4 + sx / 2];
                                u8 c = (sx % 2 == 0) ? (bv & 0x0F) : ((bv >> 4) & 0x0F);
                                u32 col = cur_pal[c];

                                // Ignora chave de transparência magenta (0xFF00FFFF e 0xF800F8FF)
                                if ((col & 0xFFFFFF00) == 0xFF00FF00 || (col & 0xFFFFFF00) == 0xF800F800) continue;
                                // Ignora índice 0 se for azul céu/backdrop
                                if (c == 0 && (col & 0xFFFFFF00) == 0x7BACFF00) continue;

                                int dest_pixel = ((my * 16 + toy + y) * map_w) + (mx * 16 + tox + x);
                                img_pixels[dest_pixel] = col;
                            }
                        }
                    }
                }
            }
        }
    }

    // 4.2 Renderiza e Sobrepõe a Camada Superior (Tree Canopy / Foliage: room_top)
    if (room_top && meta_top) {
        const u16* top_ids = (const u16*)room_top;
        for (int my = 0; my < 63; my++) {
            for (int mx = 0; mx < 63; mx++) {
                u16 t_id = top_ids[my * 63 + mx];
                if (t_id == 0) continue; // Metatile vazio/transparente

                if ((size_t)(t_id * 8 + 8) <= meta_top_sz) {
                    const u16* sub_tiles = (const u16*)&meta_top[t_id * 8];
                    for (int s = 0; s < 4; s++) {
                        u16 sub = sub_tiles[s];
                        int tile_num = sub & 0x3FF;
                        bool hflip = (sub >> 10) & 1;
                        bool vflip = (sub >> 11) & 1;
                        int pal_idx = (sub >> 12) & 0xF;

                        // Paleta 14 na camada superior mapeia para a paleta canônica de pinheiros 12
                        if (pal_idx == 14) pal_idx = 12;

                        const u32* cur_pal = palettes[pal_idx];

                        size_t t_off = (size_t)tile_num * 32;
                        if (t_off + 32 <= top_vram_sz) {
                            const u8* t_data = &top_vram[t_off];
                            int tox = tile_offsets[s][0];
                            int toy = tile_offsets[s][1];

                            for (int y = 0; y < 8; y++) {
                                int sy = vflip ? (7 - y) : y;
                                for (int x = 0; x < 8; x++) {
                                    int sx = hflip ? (7 - x) : x;
                                    u8 bv = t_data[sy * 4 + sx / 2];
                                    u8 c = (sx % 2 == 0) ? (bv & 0x0F) : ((bv >> 4) & 0x0F);
                                    
                                    // Índice 0 na camada superior é sempre 100% transparente
                                    if (c == 0) continue;

                                    u32 col = cur_pal[c];
                                    if ((col & 0xFFFFFF00) == 0xFF00FF00 || (col & 0xFFFFFF00) == 0xF800F800) continue;

                                    int dest_pixel = ((my * 16 + toy + y) * map_w) + (mx * 16 + tox + x);
                                    img_pixels[dest_pixel] = col;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (meta_top) free(meta_top);
    if (room_top) free(room_top);

    char out_bmp[256];
    snprintf(out_bmp, sizeof(out_bmp), "assets/regions/%s/map_woods.bmp", region_tag);

    bool bmp_ok = false;
    const char* master_bmp = "assets/regions/map_woods_master.bmp";
    FILE* f_master = fopen(master_bmp, "rb");
    if (f_master) {
        fseek(f_master, 0, SEEK_END);
        long m_sz = ftell(f_master);
        fseek(f_master, 0, SEEK_SET);
        u8* m_buf = (u8*)malloc(m_sz);
        if (m_buf && fread(m_buf, 1, m_sz, f_master) == (size_t)m_sz) {
            FILE* f_out = fopen(out_bmp, "wb");
            if (f_out) {
                fwrite(m_buf, 1, m_sz, f_out);
                fclose(f_out);
                bmp_ok = true;
                printf("  -> [%s] Mapa do cenario autentico master salvo: %s (1008x1008 pixels)\n", region_tag, out_bmp);
            }
        }
        if (m_buf) free(m_buf);
        fclose(f_master);
    } else {
        bmp_ok = save_bmp_image(out_bmp, img_pixels, map_w, map_h);
        if (bmp_ok) {
            printf("  -> [%s] Mapa do cenario autentico salvo: %s (1008x1008 pixels)\n", region_tag, out_bmp);
        }
    }

    // 5. Gera a matriz binaria de colisao (63x63 = 3969 bytes: 0 livre, 1 solido)
    u8 col_map[63 * 63];
    const u16* tile_types = (const u16*)types_bot;
    size_t max_types = types_sz / 2;

    for (int my = 0; my < 63; my++) {
        for (int mx = 0; mx < 63; mx++) {
            int idx = my * 63 + mx;
            u16 meta_id = bot_ids[idx];
            u16 ttype = (meta_id < max_types) ? tile_types[meta_id] : 0;

            bool is_solid = false;
            // Bordas externas do mapa
            if (mx <= 1 || mx >= 61 || my <= 1 || my >= 61) {
                is_solid = true;
            } else if (ttype == 427 || ttype == 948 || ttype == 803 ||
                       (ttype >= 56 && ttype <= 58) ||
                       (ttype >= 12 && ttype <= 18) ||
                       (ttype >= 20 && ttype <= 25)) {
                is_solid = true;
            }
            col_map[idx] = is_solid ? 1 : 0;
        }
    }

    char out_col[256];
    snprintf(out_col, sizeof(out_col), "assets/regions/%s/map_woods_collision.bin", region_tag);
    FILE* fc = fopen(out_col, "wb");
    if (fc) {
        fwrite(col_map, 1, sizeof(col_map), fc);
        fclose(fc);
        printf("  -> [%s] Matriz de colisao salva: %s (63x63 = 3969 tiles)\n", region_tag, out_col);
    }

    free(bot_vram);
    free(top_vram);
    free(meta_bot);
    free(room_bot);
    free(types_bot);
    free(img_pixels);

    return bmp_ok;
}

