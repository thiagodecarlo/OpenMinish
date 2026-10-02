#ifndef LZ77_H
#define LZ77_H

/*
 * ============================================================================
 * tools/extractor/lz77.h - Descompressor de Dados LZ77 (Padrão BIOS do GBA)
 * ============================================================================
 * No Game Boy Advance, a Nintendo embutiu um algoritmo de descompressão de
 * dados direto na memória ROM da BIOS (chamada de sistema SWI 0x11).
 * Quase todos os sprites, mapas e gráficos de The Minish Cap usam esse formato.
 *
 * Estrutura do Cabeçalho LZ77 (4 bytes):
 * - Byte 0:     Identificador (deve ser 0x10 para o formato padrão do GBA).
 * - Bytes 1..3: Tamanho descompactado em 24 bits (Little-Endian).
 */

#include "gba/types.h"

/*
 * Descompacta um fluxo de dados no padrão LZ77 (Tipo 0x10) do GBA.
 *
 * Parâmetros:
 *  - src: Ponteiro para o início dos dados comprimidos na ROM.
 *  - src_size: Tamanho máximo disponível para leitura.
 *  - out_decompressed_size: Ponteiro que receberá o tamanho final dos dados descompactados.
 *
 * Retorno:
 *  - Ponteiro para o buffer alocado na memória RAM contendo os dados brutos descompactados.
 *    (O chamador é responsável por liberar com free() quando terminar).
 *  - Retorna NULL se os dados não forem um bloco LZ77 válido (cabeçalho != 0x10).
 */
u8* lz77_decompress(const u8* src, size_t src_size, size_t* out_decompressed_size);

#endif // LZ77_H
