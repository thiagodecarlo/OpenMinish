#include "hal/texture.h"
#include "hal/video.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ============================================================================
 * src/hal/texture.c - Implementação do Sistema de Texturas e Blit 2D
 * ============================================================================
 */

#pragma pack(push, 1)
typedef struct {
    u16 bfType;
    u32 bfSize;
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
} BmpFileHeader;

typedef struct {
    u32 biSize;
    s32 biWidth;
    s32 biHeight;
    u16 biPlanes;
    u16 biBitCount;
    u32 biCompression;
    u32 biSizeImage;
    s32 biXPelsPerMeter;
    s32 biYPelsPerMeter;
    u32 biClrUsed;
    u32 biClrImportant;
} BmpInfoHeader;
#pragma pack(pop)

/*
 * Localizador Inteligente de Arquivos de Asset:
 * Procura o arquivo relativo ao CWD atual (.), voltando um nível (../ caso o jogo
 * seja executado de dentro da pasta build/), ou relativo ao diretório do executável.
 */
static FILE* open_asset_file(const char* rel_path, char* resolved_out, size_t out_size) {
    if (!rel_path) return NULL;

    // 1. Tenta relativo ao CWD direto
    FILE* f = fopen(rel_path, "rb");
    if (f) {
        snprintf(resolved_out, out_size, "%s", rel_path);
        return f;
    }

    // 2. Tenta voltar um diretório (caso executado a partir de build/)
    char fallback[512];
    snprintf(fallback, sizeof(fallback), "../%s", rel_path);
    f = fopen(fallback, "rb");
    if (f) {
        snprintf(resolved_out, out_size, "%s", fallback);
        return f;
    }

    // 3. Tenta resolver via diretório do executável (SDL_GetBasePath)
    char* base = SDL_GetBasePath();
    if (base) {
        // Tenta base/rel_path
        snprintf(fallback, sizeof(fallback), "%s%s", base, rel_path);
        f = fopen(fallback, "rb");
        if (f) {
            snprintf(resolved_out, out_size, "%s", fallback);
            SDL_free(base);
            return f;
        }

        // Tenta base/../rel_path (se o executável estiver em build/)
        snprintf(fallback, sizeof(fallback), "%s../%s", base, rel_path);
        f = fopen(fallback, "rb");
        if (f) {
            snprintf(resolved_out, out_size, "%s", fallback);
            SDL_free(base);
            return f;
        }

        SDL_free(base);
    }

    return NULL;
}

Texture* texture_load_bmp(const char* filepath) {
    if (!filepath) return NULL;

    // Suporte ao Mecanismo de Mods: Verifica se existe versão customizada em assets/textures/
    char mod_path[256];
    const char* filename = strrchr(filepath, '/');
    if (!filename) filename = strrchr(filepath, '\\');
    filename = filename ? filename + 1 : filepath;

    snprintf(mod_path, sizeof(mod_path), "assets/textures/%s", filename);

    char target_path[512];
    FILE* f = open_asset_file(mod_path, target_path, sizeof(target_path));
    if (f) {
        printf("[MOD ATIVO] Carregando textura personalizada: %s\n", target_path);
    } else {
        f = open_asset_file(filepath, target_path, sizeof(target_path));
    }

    if (!f) {
        printf("[ERRO TEXTURA] Nao foi possivel abrir o arquivo: %s\n", filepath);
        return NULL;
    }

    BmpFileHeader file_hdr;
    BmpInfoHeader info_hdr;

    if (fread(&file_hdr, sizeof(BmpFileHeader), 1, f) != 1 ||
        fread(&info_hdr, sizeof(BmpInfoHeader), 1, f) != 1) {
        printf("[ERRO TEXTURA] Cabecalho BMP corrompido: %s\n", target_path);
        fclose(f);
        return NULL;
    }

    // Valida a assinatura 'BM' (0x4D42)
    if (file_hdr.bfType != 0x4D42) {
        printf("[ERRO TEXTURA] Formato invalido (nao e BMP): %s\n", target_path);
        fclose(f);
        return NULL;
    }

    int width = info_hdr.biWidth;
    int height = info_hdr.biHeight;
    bool top_down = (height < 0);
    height = abs(height);

    Texture* tex = (Texture*)malloc(sizeof(Texture));
    if (!tex) {
        fclose(f);
        return NULL;
    }

    tex->width = width;
    tex->height = height;
    tex->pixels = (u32*)malloc(width * height * sizeof(u32));

    if (!tex->pixels) {
        free(tex);
        fclose(f);
        return NULL;
    }

    // Posiciona o ponteiro do arquivo no início dos dados dos pixels
    fseek(f, file_hdr.bfOffBits, SEEK_SET);

    int bytes_per_pixel = info_hdr.biBitCount / 8;
    int row_stride = ((width * bytes_per_pixel + 3) / 4) * 4; // Linhas no BMP sao alinhadas em 4 bytes

    u8* row_buffer = (u8*)malloc(row_stride);

    for (int y = 0; y < height; y++) {
        int target_y = top_down ? y : (height - 1 - y);

        if (fread(row_buffer, 1, row_stride, f) != (size_t)row_stride) {
            break;
        }

        for (int x = 0; x < width; x++) {
            u8 b = row_buffer[x * bytes_per_pixel + 0];
            u8 g = row_buffer[x * bytes_per_pixel + 1];
            u8 r = row_buffer[x * bytes_per_pixel + 2];
            u8 a = (bytes_per_pixel == 4) ? row_buffer[x * bytes_per_pixel + 3] : 0xFF;

            // Converte a ordem BGRA do BMP para o formato interno RGBA
            tex->pixels[target_y * width + x] = ((u32)r << 24) | ((u32)g << 16) | ((u32)b << 8) | (u32)a;
        }
    }

    free(row_buffer);
    fclose(f);
    return tex;
}

void texture_draw_ex(const Texture* tex,
                     int src_x, int src_y,
                     int src_w, int src_h,
                     int dest_x, int dest_y,
                     bool flip_h) {
    if (!tex || !tex->pixels) return;

    for (int y = 0; y < src_h; y++) {
        int cur_src_y = src_y + y;
        if (cur_src_y < 0 || cur_src_y >= tex->height) continue;

        for (int x = 0; x < src_w; x++) {
            int cur_src_x = flip_h ? (src_x + src_w - 1 - x) : (src_x + x);
            if (cur_src_x < 0 || cur_src_x >= tex->width) continue;

            u32 pixel = tex->pixels[cur_src_y * tex->width + cur_src_x];

            // Teste de canal Alfa (transparência): só desenha se o pixel não for 100% transparente
            u8 alpha = pixel & 0xFF;
            if (alpha > 0) {
                hal_video_put_pixel(dest_x + x, dest_y + y, pixel);
            }
        }
    }
}

void texture_draw(const Texture* tex,
                  int src_x, int src_y,
                  int src_w, int src_h,
                  int dest_x, int dest_y) {
    texture_draw_ex(tex, src_x, src_y, src_w, src_h, dest_x, dest_y, false);
}

void texture_draw_full(const Texture* tex, int dest_x, int dest_y) {
    if (!tex) return;
    texture_draw(tex, 0, 0, tex->width, tex->height, dest_x, dest_y);
}

void texture_free(Texture* tex) {
    if (tex) {
        if (tex->pixels) {
            free(tex->pixels);
            tex->pixels = NULL;
        }
        free(tex);
    }
}
