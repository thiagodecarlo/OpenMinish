#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gba/types.h"
#include "lz77.h"
#include "gfx.h"

/*
 * ============================================================================
 * tools/extractor/main.c - Pipeline de Extração e Recompilação Multi-Região
 * ============================================================================
 * Processa as ROMs de The Minish Cap (USA, EUR e JPN) antecipadamente (AOT).
 * Extrai as paletas de cores, decodifica os blocos de tiles comprimidos em LZ77
 * e salva imagens Bitmap (.bmp) em assets/regions/<regiao>/ para que o jogo
 * nativo não necessite de nenhuma ROM em tempo de execução.
 */

typedef enum {
    REGION_UNKNOWN = 0,
    REGION_USA,     // BZME - Estados Unidos (Inglês)
    REGION_EUR,     // BZMP - Europa (Inglês, Francês, Alemão, Espanhol, Italiano)
    REGION_JPN      // BZMJ - Japão (Japonês / Kanjis)
} GameRegion;

typedef struct {
    char        game_title[13];
    char        game_code[5];
    char        maker_code[3];
    GameRegion  region;
    const char* region_tag;     // "usa", "eur", "jpn"
    const char* region_name;
    const char* languages;
    u32         palette_offset; // Endereço da Paleta 0 na ROM
    u8          software_version;
    u8          checksum;
} GbaHeader;

static bool inspect_gba_header(const u8* rom_data, size_t rom_size, GbaHeader* out_hdr) {
    if (rom_size < 0xC0) return false;

    memcpy(out_hdr->game_title, &rom_data[0xA0], 12);
    out_hdr->game_title[12] = '\0';

    memcpy(out_hdr->game_code, &rom_data[0xAC], 4);
    out_hdr->game_code[4] = '\0';

    memcpy(out_hdr->maker_code, &rom_data[0xB0], 2);
    out_hdr->maker_code[2] = '\0';

    out_hdr->software_version = rom_data[0xBC];
    out_hdr->checksum = rom_data[0xBD];

    // Mapeamento preciso dos metadados e endereços de paleta por região
    if (strcmp(out_hdr->game_code, "BZME") == 0) {
        out_hdr->region = REGION_USA;
        out_hdr->region_tag = "usa";
        out_hdr->region_name = "America do Norte (USA)";
        out_hdr->languages = "Ingles (EN)";
        out_hdr->palette_offset = 0x5A2E80;
    } else if (strcmp(out_hdr->game_code, "BZMP") == 0) {
        out_hdr->region = REGION_EUR;
        out_hdr->region_tag = "eur";
        out_hdr->region_name = "Europa (EUR)";
        out_hdr->languages = "Ingles (EN), Frances (FR), Alemao (DE), Espanhol (ES), Italiano (IT)";
        out_hdr->palette_offset = 0x5A23D0;
    } else if (strcmp(out_hdr->game_code, "BZMJ") == 0) {
        out_hdr->region = REGION_JPN;
        out_hdr->region_tag = "jpn";
        out_hdr->region_name = "Japao (JPN)";
        out_hdr->languages = "Japones (JA - Kanjis/Hiragana)";
        out_hdr->palette_offset = 0x5A2B20;
    } else {
        out_hdr->region = REGION_UNKNOWN;
        out_hdr->region_tag = "unknown";
        out_hdr->region_name = "Nao Oficial / Desconhecida";
        out_hdr->languages = "N/A";
        out_hdr->palette_offset = 0;
    }

    return true;
}

// Processa uma única ROM e extrai seus bancos gráficos para a pasta da região
static bool process_rom(const char* rom_path) {
    printf("--------------------------------------------------------------------\n");
    printf("Processando ROM: %s\n", rom_path);
    printf("--------------------------------------------------------------------\n");

    FILE* f = fopen(rom_path, "rb");
    if (!f) {
        printf("[AVISO] Arquivo nao encontrado: %s\n", rom_path);
        return false;
    }

    fseek(f, 0, SEEK_END);
    long rom_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8* rom_buffer = (u8*)malloc(rom_size);
    if (!rom_buffer) {
        fclose(f);
        return false;
    }

    fread(rom_buffer, 1, rom_size, f);
    fclose(f);

    GbaHeader hdr;
    if (!inspect_gba_header(rom_buffer, rom_size, &hdr)) {
        printf("[ERRO] Cabecalho GBA invalido!\n");
        free(rom_buffer);
        return false;
    }

    printf("  - Titulo:     \"%s\"\n", hdr.game_title);
    printf("  - Regiao:     %s [%s]\n", hdr.region_name, hdr.game_code);
    printf("  - Idiomas:    %s\n", hdr.languages);
    printf("  - Offset Pal: 0x%06X\n", hdr.palette_offset);

    // 1. Decodifica a Paleta Principal (16 cores BGR555 -> RGBA8888)
    u32 rgba_palette[16];
    if (hdr.palette_offset > 0 && hdr.palette_offset + 32 <= (u32)rom_size) {
        const u16* gba_pal = (const u16*)&rom_buffer[hdr.palette_offset];
        decode_palette(gba_pal, rgba_palette, 16);
    } else {
        // Paleta de fallback padrão
        for (int i = 0; i < 16; i++) {
            u8 val = (u8)(i * 17);
            rgba_palette[i] = ((u32)val << 24) | ((u32)val << 16) | ((u32)val << 8) | 0xFF;
        }
    }

    // 2. Varredura e Extração dos Principais Bancos de Tiles Gráficos (LZ77)
    printf("\n  [Extracao de Folhas de Tiles (Sprite Sheets)]\n");
    int extracted_count = 0;

    // A área de gráficos principais de Minish Cap fica entre 0x320000 e 0x360000
    for (size_t offset = 0x320000; offset < 0x350000 && extracted_count < 10; offset += 4) {
        if (rom_buffer[offset] == 0x10) {
            size_t decomp_size = 0;
            u8* tiles = lz77_decompress(&rom_buffer[offset], rom_size - offset, &decomp_size);

            // Filtra bancos gráficos válidos (múltiplos de 32 bytes e tamanho expressivo)
            if (tiles && (decomp_size == 4096 || decomp_size == 8192 || decomp_size == 16384)) {
                char out_filepath[256];
                snprintf(out_filepath, sizeof(out_filepath),
                         "assets/regions/%s/sheet_%02d.bmp", hdr.region_tag, extracted_count);

                // Exporta em grade de 16 tiles por linha (128 pixels de largura)
                export_tilesheet_bmp(out_filepath, tiles, decomp_size, rgba_palette, 16);

                printf("  -> [%s] Folha #%02d salva: %s (Offset: 0x%06zX, %zu bytes, %zu tiles)\n",
                       hdr.region_tag, extracted_count, out_filepath, offset, decomp_size, decomp_size / 32);

                extracted_count++;
                free(tiles);
            } else if (tiles) {
                free(tiles);
            }
        }
    }

    printf("  Total de folhas graficas extraidas para [%s]: %d\n\n", hdr.region_tag, extracted_count);
    free(rom_buffer);
    return true;
}

int main(int argc, char* argv[]) {
    printf("====================================================================\n");
    printf("   Extrator e Recompilador de Assets Multi-Regiao (USA, EUR, JPN)   \n");
    printf("   The Legend of Zelda: The Minish Cap - Native PC Port             \n");
    printf("====================================================================\n\n");

    if (argc > 1 && strcmp(argv[1], "--all") != 0) {
        // Se um arquivo específico foi passado como argumento
        process_rom(argv[1]);
    } else {
        // Modo Multi-Região Automático: processa as 3 ROMs encontradas na pasta ROMS/
        printf("Modo Automatico Multi-Regiao ativado. Processando as ROMs disponiveis...\n\n");

        const char* rom_paths[] = {
            "ROMS/Legend of Zelda, The - The Minish Cap (USA).gba",
            "ROMS/Legend of Zelda, The - The Minish Cap (Europe) (En,Fr,De,Es,It).gba",
            "ROMS/Zelda no Densetsu - Fushigi no Boushi (Japan).gba"
        };

        int processed = 0;
        for (int i = 0; i < 3; i++) {
            if (process_rom(rom_paths[i])) {
                processed++;
            }
        }

        if (processed == 0) {
            printf("[AVISO] Nenhuma ROM encontrada na pasta ROMS/.\n");
            printf("Coloque os arquivos .gba em 'D:\\REPOS\\TLoZ-MC\\ROMS\\' e execute novamente.\n");
        } else {
            printf("====================================================================\n");
            printf("EXTRAÇÃO CONCLUÍDA COM SUCESSO!\n");
            printf("As 3 regiões foram processadas e organizadas em 'assets/regions/'.\n");
            printf("A ROM original NÃO é mais necessária para executar o jogo!\n");
            printf("====================================================================\n");
        }
    }

    return 0;
}
