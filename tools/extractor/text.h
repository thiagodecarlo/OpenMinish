#ifndef TEXT_EXTRACTOR_H
#define TEXT_EXTRACTOR_H

/*
 * ============================================================================
 * tools/extractor/text.h - Extrator de Textos, Diálogos e Scripts da ROM
 * ============================================================================
 * Extrai a tabela de ponteiros de mensagens e decodifica as strings canônicas
 * de The Legend of Zelda: The Minish Cap (GBA), exportando para arquivos JSON
 * estruturados em assets/lang/ para permitir localização em PT-BR.
 */

#include "gba/types.h"
#include <stdbool.h>
#include <stddef.h>

/*
 * Endereços das tabelas de mensagens na memória do GBA (0x08xxxxxx):
 *  - USA: 0x089B1D90 -> Offset no arquivo ROM: 0x009B1D90
 *  - EUR (English):  0x089AEB60 -> Offset: 0x009AEB60
 *  - EUR (French):   0x089F7420 -> Offset: 0x009F7420
 *  - EUR (German):   0x08A3EEB0 -> Offset: 0x00A3EEB0
 *  - EUR (Spanish):  0x08A81E70 -> Offset: 0x00A81E70
 *  - EUR (Italian):  0x08AC37A0 -> Offset: 0x00AC37A0
 */
#define ROM_TEXT_TABLE_USA          0x009B1D90
#define ROM_TEXT_TABLE_EUR_EN       0x009AEB60
#define ROM_TEXT_TABLE_EUR_FR       0x009F7420
#define ROM_TEXT_TABLE_EUR_DE       0x00A3EEB0
#define ROM_TEXT_TABLE_EUR_ES       0x00A81E70
#define ROM_TEXT_TABLE_EUR_IT       0x00AC37A0

/*
 * Decodifica uma única string binária do TMC (terminada em 0x00) e escreve
 * uma string UTF-8 formatada com tags de controle ({Color:...}, {Player}, etc.).
 *
 * Parâmetros:
 *   - src_bytes: ponteiro para os bytes da string na ROM.
 *   - max_src_len: limite máximo de leitura de segurança na ROM.
 *   - out_buf: buffer de saída para o texto decodificado em UTF-8.
 *   - out_buf_size: tamanho do buffer de saída.
 * Retorna true em caso de sucesso.
 */
bool tmc_decode_string(const u8* src_bytes, size_t max_src_len, char* out_buf, size_t out_buf_size);

/*
 * Processa a tabela de textos da ROM a partir de table_offset e exporta
 * todos os grupos e mensagens para um arquivo JSON estruturado.
 *
 * Parâmetros:
 *   - rom_buffer: ponteiro para os dados da ROM completa.
 *   - rom_size: tamanho total da ROM em bytes.
 *   - table_offset: offset no arquivo ROM onde inicia a tabela de categorias.
 *   - out_filepath: caminho do arquivo JSON de destino (ex: "assets/lang/en_US_dialogues.json").
 *   - lang_code: código de idioma (ex: "en_US", "es_ES").
 *   - lang_name: nome descritivo do idioma.
 * Retorna true se a extração foi bem sucedida.
 */
bool export_text_dialogues_json(const u8* rom_buffer,
                                size_t rom_size,
                                u32 table_offset,
                                const char* out_filepath,
                                const char* lang_code,
                                const char* lang_name);

/*
 * Extrai todos os idiomas disponíveis na ROM de acordo com a região detectada.
 *  - Para USA: exporta assets/lang/en_US_dialogues.json
 *  - Para EUR: exporta en_US, es_ES, fr_FR, de_DE, it_IT
 */
bool export_all_rom_texts(const u8* rom_buffer, size_t rom_size, const char* region_tag);

#endif // TEXT_EXTRACTOR_H
