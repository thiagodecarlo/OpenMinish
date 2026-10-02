#include "lz77.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * ============================================================================
 * tools/extractor/lz77.c - Implementação da Descompressão LZ77 do GBA
 * ============================================================================
 */

u8* lz77_decompress(const u8* src, size_t src_size, size_t* out_decompressed_size) {
    if (!src || src_size < 4) {
        return NULL;
    }

    // 1. Validação do identificador de compressão do GBA (0x10)
    if (src[0] != 0x10) {
        return NULL;
    }

    // 2. Extração do tamanho descompactado (24 bits Little-Endian)
    u32 decompressed_size = (u32)src[1] | ((u32)src[2] << 8) | ((u32)src[3] << 16);
    if (decompressed_size == 0) {
        return NULL;
    }

    // 3. Alocação da memória de destino exata
    u8* dest = (u8*)malloc(decompressed_size);
    if (!dest) {
        printf("[ERRO LZ77] Falha de memoria ao alocar %u bytes!\n", decompressed_size);
        return NULL;
    }

    size_t src_idx = 4;
    size_t dest_idx = 0;

    // 4. Processamento dos blocos com base no Byte de Flags
    while (dest_idx < decompressed_size && src_idx < src_size) {
        u8 flag_byte = src[src_idx++];

        // O byte de flag é lido do bit mais significativo (7) ao menos significativo (0)
        for (int b = 7; b >= 0; b--) {
            if (dest_idx >= decompressed_size) {
                break;
            }

            bool is_compressed = (flag_byte >> b) & 1;

            if (!is_compressed) {
                // Bit = 0: Dado Literal (copia o byte diretamente sem compressão)
                if (src_idx >= src_size) break;
                dest[dest_idx++] = src[src_idx++];
            } else {
                // Bit = 1: Bloco Comprimido codificado em 2 bytes subsequentes
                if (src_idx + 1 >= src_size) break;

                u8 byte_a = src[src_idx++];
                u8 byte_b = src[src_idx++];

                // Os 4 bits superiores de byte_a definem o comprimento da cópia (+3 de offset)
                int length = (byte_a >> 4) + 3;

                // Os 4 bits inferiores de byte_a combinados com byte_b formam a distância (+1 de offset)
                int displacement = (((byte_a & 0x0F) << 8) | byte_b) + 1;

                if (displacement > (int)dest_idx) {
                    // Proteção de segurança: não podemos olhar para trás antes do início do buffer!
                    free(dest);
                    return NULL;
                }

                // Copia os bytes repetidos da janela deslizante na memória
                for (int i = 0; i < length && dest_idx < decompressed_size; i++) {
                    dest[dest_idx] = dest[dest_idx - displacement];
                    dest_idx++;
                }
            }
        }
    }

    if (out_decompressed_size) {
        *out_decompressed_size = dest_idx;
    }

    return dest;
}
