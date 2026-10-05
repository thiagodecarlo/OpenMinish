/*
 * ============================================================================
 * tools/extractor/text.c - Extrator de Textos, Diálogos e Scripts da ROM
 * ============================================================================
 */

#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline u32 read_u32_le(const u8* ptr) {
    return (u32)ptr[0] | ((u32)ptr[1] << 8) | ((u32)ptr[2] << 16) | ((u32)ptr[3] << 24);
}

// Nomes descritivos canônicos para os principais grupos de mensagens
static const char* get_group_name(int group_idx) {
    switch (group_idx) {
        case 0x00: return "Sistema, Salvamento & Menus";
        case 0x01: return "Créditos Finais & Epílogo";
        case 0x02: return "Nomes de NPCs & Lugares de Hyrule";
        case 0x03: return "Boletim dos Espadachins (Newsletters)";
        case 0x04: return "Nomes de Itens & Equipamentos";
        case 0x05: return "Mensagens de Obtenção de Itens (Fanfarras)";
        case 0x06: return "Descrições de Itens do Inventário";
        case 0x07: return "Descrições de Kinstones";
        case 0x08: return "Galeria de Estatuetas do Carlov (Figurines 1-136)";
        case 0x09: return "Prólogo & Festival dos Minish";
        case 0x0A: return "Castelo de Hyrule & Cerimônia";
        case 0x0B: return "Cidade de Hyrule (Comércio & Cidadãos)";
        case 0x0C: return "Minish Woods & Toco de Transformação";
        case 0x0D: return "Vila dos Minish (Picori Village)";
        case 0x0E: return "Deepwood Shrine (Dungeon 1)";
        case 0x0F: return "Fazenda Lon Lon & Campos Centrais";
        case 0x10: return "Monte Crenel & Escalada";
        case 0x11: return "Minas de Melari & Forja";
        case 0x12: return "Cave of Flames (Dungeon 2)";
        case 0x13: return "Santuário Elemental & Infusão de Clones";
        case 0x14: return "Pântano de Castor Wilds";
        case 0x15: return "Wind Ruins & Robôs Armos";
        case 0x16: return "Fortress of Winds (Dungeon 3)";
        case 0x17: return "Lago Hylia & Biblioteca de Hyrule";
        case 0x18: return "Temple of Droplets (Dungeon 4)";
        case 0x19: return "Veil Falls & Subida às Nuvens";
        case 0x1A: return "Palace of Winds (Dungeon 5)";
        case 0x1B: return "Vale Real & Cemitério Real";
        case 0x1C: return "Dark Hyrule Castle (Dungeon Final)";
        case 0x1D: return "Confronto contra Vaati & Despedida do Ezlo";
        case 0x1E: return "Fusões de Kinstone & Eventos Mundiais";
        case 0x1F: return "Dojos dos Mestres Espadachins (Tiger Scrolls)";
        case 0x20: return "Dicas do Gorro Ezlo (Companheiro)";
        default:   return "Diálogos de Missões e NPCs";
    }
}

static const char* s_color_names[] = {
    "White", "Red", "Green", "Blue", "Yellow"
};

static const char* s_key_names[] = {
    "A", "B", "Left", "Right", "DUp", "DDown", "DLeft", "DRight", "Dpad", "Select", "Start"
};

bool tmc_decode_string(const u8* src_bytes, size_t max_src_len, char* out_buf, size_t out_buf_size) {
    if (!src_bytes || !out_buf || out_buf_size == 0) return false;

    size_t in_pos = 0;
    size_t out_pos = 0;

    #define EMIT_CHAR(c) do { \
        if (out_pos + 1 < out_buf_size) { \
            out_buf[out_pos++] = (char)(c); \
        } \
    } while (0)

    #define EMIT_STR(str) do { \
        const char* s = (str); \
        while (*s && out_pos + 1 < out_buf_size) { \
            out_buf[out_pos++] = *s++; \
        } \
    } while (0)

    while (in_pos < max_src_len) {
        u8 b = src_bytes[in_pos++];
        if (b == 0x00) {
            break; // Fim da string
        }

        // 1. Quebra de linha
        if (b == 0x0A) {
            EMIT_CHAR('\n');
            continue;
        }
        if (b == 0x0D) {
            continue; // ignora CR
        }

        // 2. Comandos de Controle (0x01 - 0x0F)
        if (b == 0x01) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            snprintf(tag, sizeof(tag), "{01:%02X}", a);
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x02) {
            u8 col = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            if (col < 5) {
                snprintf(tag, sizeof(tag), "{Color:%s}", s_color_names[col]);
            } else {
                snprintf(tag, sizeof(tag), "{Color:%02X}", col);
            }
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x03) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            u8 b2 = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            snprintf(tag, sizeof(tag), "{Sound:%02X:%02X}", a, b2);
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x04) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            if (a == 0x10) {
                u8 b2 = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
                snprintf(tag, sizeof(tag), "{04:10:%02X}", b2);
            } else {
                snprintf(tag, sizeof(tag), "{04:%02X}", a);
            }
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x05) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            if (a == 0xFF) {
                snprintf(tag, sizeof(tag), "{Choice:FF}");
            } else {
                u8 b2 = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
                snprintf(tag, sizeof(tag), "{Choice:%02X:%02X}", a, b2);
            }
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x06) {
            u8 var_type = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            if (var_type == 0) {
                snprintf(tag, sizeof(tag), "{Player}");
            } else {
                snprintf(tag, sizeof(tag), "{Var:%X}", var_type);
            }
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x07) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            u8 b2 = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            snprintf(tag, sizeof(tag), "{07:%02X:%02X}", a, b2);
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x08) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            snprintf(tag, sizeof(tag), "{08:%02X}", a);
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x09) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            snprintf(tag, sizeof(tag), "{09:%02X}", a);
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x0B) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            snprintf(tag, sizeof(tag), "{0B:%02X}", a);
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x0C) {
            u8 key = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            if (key < 11) {
                snprintf(tag, sizeof(tag), "{Key:%s}", s_key_names[key]);
            } else {
                snprintf(tag, sizeof(tag), "{Key:%02X}", key);
            }
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x0D) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            snprintf(tag, sizeof(tag), "{0D:%02X}", a);
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x0E) {
            u8 a = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            snprintf(tag, sizeof(tag), "{0E:%02X}", a);
            EMIT_STR(tag);
            continue;
        }
        if (b == 0x0F) {
            u8 sym = (in_pos < max_src_len) ? src_bytes[in_pos++] : 0;
            char tag[32];
            snprintf(tag, sizeof(tag), "{Symbol:%02X}", sym);
            EMIT_STR(tag);
            continue;
        }

        // 3. Caracteres ASCII imprimíveis (0x20 a 0x7E)
        if (b >= 0x20 && b <= 0x7E) {
            EMIT_CHAR(b);
            continue;
        }

        // 4. Pontuações e Símbolos Especiais Windows-1252 / GBA
        if (b == 0x82) { EMIT_CHAR(','); continue; }
        if (b == 0x84) { EMIT_STR("„"); continue; }
        if (b == 0x85) { EMIT_STR("…"); continue; }
        if (b == 0x91) { EMIT_STR("‘"); continue; }
        if (b == 0x92) { EMIT_STR("’"); continue; }
        if (b == 0x93) { EMIT_STR("“"); continue; }
        if (b == 0x94) { EMIT_STR("”"); continue; }
        if (b == 0x95) { EMIT_STR("·"); continue; }
        if (b == 0x99) { EMIT_STR("™"); continue; }
        if (b == 0xA1) { EMIT_STR("¡"); continue; }
        if (b == 0xA3) { EMIT_STR("♪"); continue; }
        if (b == 0xAA) { EMIT_STR("ª"); continue; }
        if (b == 0xAB) { EMIT_STR("«"); continue; }
        if (b == 0xB0) { EMIT_STR("°"); continue; }
        if (b == 0xBA) { EMIT_STR("º"); continue; }
        if (b == 0xBB) { EMIT_STR("»"); continue; }
        if (b == 0xBF) { EMIT_STR("¿"); continue; }

        // 5. Caracteres Acentuados Latin-1 (0xC0 - 0xFF) convertidos para UTF-8 de 2 bytes
        if (b >= 0x80) {
            u8 utf8_b1 = 0xC0 | (b >> 6);
            u8 utf8_b2 = 0x80 | (b & 0x3F);
            EMIT_CHAR(utf8_b1);
            EMIT_CHAR(utf8_b2);
            continue;
        }
    }

    out_buf[out_pos] = '\0';
    return true;
}

// Escreve string com escape JSON adequado (" -> \", \ -> \\, newline -> \n)
static void write_json_escaped_string(FILE* f, const char* str) {
    fputc('"', f);
    while (*str) {
        unsigned char c = (unsigned char)*str++;
        if (c == '"') {
            fputs("\\\"", f);
        } else if (c == '\\') {
            fputs("\\\\", f);
        } else if (c == '\n') {
            fputs("\\n", f);
        } else if (c == '\r') {
            fputs("\\r", f);
        } else if (c == '\t') {
            fputs("\\t", f);
        } else if (c < 0x20) {
            fprintf(f, "\\u%04x", c);
        } else {
            fputc(c, f);
        }
    }
    fputc('"', f);
}

bool export_text_dialogues_json(const u8* rom_buffer,
                                size_t rom_size,
                                u32 table_offset,
                                const char* out_filepath,
                                const char* lang_code,
                                const char* lang_name) {
    if (!rom_buffer || table_offset >= rom_size) {
        printf("[ERRO] Offset da tabela de texto invalido: 0x%06X (Tamanho ROM: %zu)\n",
               table_offset, rom_size);
        return false;
    }

    // A contagem de categorias é o primeiro offset dividido por 4
    u32 first_cat_offset = read_u32_le(&rom_buffer[table_offset]);
    u32 category_count = first_cat_offset / 4;

    if (category_count == 0 || category_count > 128) {
        printf("[ERRO] Contagem de categorias de texto fora do padrao: %u\n", category_count);
        return false;
    }

    FILE* f = fopen(out_filepath, "w");
    if (!f) {
        printf("[ERRO] Nao foi possivel criar o arquivo de saida: %s\n", out_filepath);
        return false;
    }

    printf("  [Extracao de Textos -> %s]\n", out_filepath);
    printf("  - Tabela ROM:    0x%06X\n", table_offset);
    printf("  - Grupos:        %u\n", category_count);
    printf("  - Idioma:        %s (%s)\n", lang_name, lang_code);

    fprintf(f, "{\n");
    fprintf(f, "  \"locale\": \"%s\",\n", lang_code);
    fprintf(f, "  \"language_name\": \"%s\",\n", lang_name);
    fprintf(f, "  \"total_groups\": %u,\n", category_count);
    fprintf(f, "  \"groups\": [\n");

    u32 total_messages_extracted = 0;
    static char s_decoded_text[4096];

    for (u32 g = 0; g < category_count; g++) {
        u32 cat_offset = read_u32_le(&rom_buffer[table_offset + g * 4]);
        u32 cat_start = table_offset + cat_offset;

        if (cat_start + 4 > rom_size) {
            printf("[AVISO] Grupo %u fora dos limites da ROM (Offset: 0x%06X)\n", g, cat_start);
            continue;
        }

        u32 first_msg_offset = read_u32_le(&rom_buffer[cat_start]);
        u32 msg_count = first_msg_offset / 4;
        if (msg_count > 500) msg_count = 500; // Limite de seguranca

        const char* group_name = get_group_name(g);

        fprintf(f, "    {\n");
        fprintf(f, "      \"group_id\": %u,\n", g);
        fprintf(f, "      \"group_hex\": \"0x%02X\",\n", g);
        fprintf(f, "      \"name\": \"%s\",\n", group_name);
        fprintf(f, "      \"message_count\": %u,\n", msg_count);
        fprintf(f, "      \"messages\": [\n");

        for (u32 m = 0; m < msg_count; m++) {
            u32 msg_offset = read_u32_le(&rom_buffer[cat_start + m * 4]);
            u32 str_start = cat_start + msg_offset;

            s_decoded_text[0] = '\0';
            if (str_start < rom_size) {
                size_t max_len = rom_size - str_start;
                if (max_len > 2048) max_len = 2048;
                tmc_decode_string(&rom_buffer[str_start], max_len, s_decoded_text, sizeof(s_decoded_text));
            }

            u16 message_id = (u16)((g << 8) | m);

            fprintf(f, "        {\n");
            fprintf(f, "          \"id\": %u,\n", m);
            fprintf(f, "          \"global_id\": %u,\n", (u32)message_id);
            fprintf(f, "          \"hex_id\": \"0x%04X\",\n", message_id);
            fprintf(f, "          \"text\": ");
            write_json_escaped_string(f, s_decoded_text);
            fprintf(f, "\n");
            fprintf(f, "        }%s\n", (m + 1 < msg_count) ? "," : "");

            total_messages_extracted++;
        }

        fprintf(f, "      ]\n");
        fprintf(f, "    }%s\n", (g + 1 < category_count) ? "," : "");
    }

    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    fclose(f);

    printf("  -> Sucesso! %u mensagens extraidas para '%s'.\n\n",
           total_messages_extracted, out_filepath);
    return true;
}

bool export_all_rom_texts(const u8* rom_buffer, size_t rom_size, const char* region_tag) {
    if (!rom_buffer || !region_tag) return false;

    if (strcmp(region_tag, "usa") == 0) {
        return export_text_dialogues_json(rom_buffer, rom_size,
                                          ROM_TEXT_TABLE_USA,
                                          "assets/lang/en_US_dialogues.json",
                                          "en_US", "English (United States)");
    } else if (strcmp(region_tag, "eur") == 0) {
        bool ok = true;
        ok &= export_text_dialogues_json(rom_buffer, rom_size,
                                         ROM_TEXT_TABLE_EUR_EN,
                                         "assets/lang/en_EUR_dialogues.json",
                                         "en_GB", "English (Europe)");
        ok &= export_text_dialogues_json(rom_buffer, rom_size,
                                         ROM_TEXT_TABLE_EUR_ES,
                                         "assets/lang/es_ES_dialogues.json",
                                         "es_ES", "Español (España)");
        ok &= export_text_dialogues_json(rom_buffer, rom_size,
                                         ROM_TEXT_TABLE_EUR_FR,
                                         "assets/lang/fr_FR_dialogues.json",
                                         "fr_FR", "Français (France)");
        ok &= export_text_dialogues_json(rom_buffer, rom_size,
                                         ROM_TEXT_TABLE_EUR_DE,
                                         "assets/lang/de_DE_dialogues.json",
                                         "de_DE", "Deutsch (Deutschland)");
        ok &= export_text_dialogues_json(rom_buffer, rom_size,
                                         ROM_TEXT_TABLE_EUR_IT,
                                         "assets/lang/it_IT_dialogues.json",
                                         "it_IT", "Italiano (Italia)");
        return ok;
    }

    return false;
}
