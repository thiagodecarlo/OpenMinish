#include "hal/save.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

static void get_save_path(int slot, char* out_path, size_t max_len) {
    if (slot < 1) slot = 1;
    if (slot > MAX_SAVE_SLOTS) slot = MAX_SAVE_SLOTS;
    snprintf(out_path, max_len, "saves/save%d.dat", slot);
}

void save_system_init(void) {
    MKDIR("saves");
    printf("[SAVE] Sistema de Persistencia inicializado. Diretorio 'saves/' verificado.\n");
}

u32 save_calculate_checksum(const SaveData* data) {
    if (!data) return 0;
    const u8* bytes = (const u8*)data;
    size_t len = sizeof(SaveData) - sizeof(u32); // Exclui o campo checksum
    u32 sum = 0x5A5A5A5A;
    for (size_t i = 0; i < len; i++) {
        sum = ((sum << 5) + sum) + bytes[i];
    }
    return sum;
}

bool save_game(int slot, const SaveData* data) {
    if (!data || slot < 1 || slot > MAX_SAVE_SLOTS) return false;

    char path[256];
    get_save_path(slot, path, sizeof(path));

    SaveData to_write = *data;
    to_write.magic = SAVE_MAGIC;
    to_write.version = SAVE_VERSION;
    to_write.checksum = save_calculate_checksum(&to_write);

    FILE* f = fopen(path, "wb");
    if (!f) {
        // Tenta criar o diretório caso tenha sido removido
        MKDIR("saves");
        f = fopen(path, "wb");
        if (!f) {
            printf("[ERRO SAVE] Falha ao abrir '%s' para escrita!\n", path);
            return false;
        }
    }

    size_t written = fwrite(&to_write, sizeof(SaveData), 1, f);
    fclose(f);

    if (written == 1) {
        printf("[SAVE] Slot %d salvo com exito em '%s' (HP: %d/%d, R: %d, Mapa: %d, Checksum: 0x%08X)!\n",
               slot, path, to_write.hearts, to_write.max_hearts, to_write.rupees, to_write.current_map, to_write.checksum);
        return true;
    } else {
        printf("[ERRO SAVE] Erro de gravacao incompleta no slot %d!\n", slot);
        return false;
    }
}

bool load_game(int slot, SaveData* out_data) {
    if (!out_data || slot < 1 || slot > MAX_SAVE_SLOTS) return false;

    char path[256];
    get_save_path(slot, path, sizeof(path));

    FILE* f = fopen(path, "rb");
    if (!f) {
        return false;
    }

    SaveData loaded;
    size_t read_bytes = fread(&loaded, sizeof(SaveData), 1, f);
    fclose(f);

    if (read_bytes != 1) {
        printf("[ERRO LOAD] Arquivo de save '%s' corrompido ou truncado!\n", path);
        return false;
    }

    if (loaded.magic != SAVE_MAGIC) {
        printf("[ERRO LOAD] Assinatura magica invalida (0x%08X vs esperado 0x%08X)!\n", loaded.magic, SAVE_MAGIC);
        return false;
    }

    u32 expected_cksum = save_calculate_checksum(&loaded);
    if (loaded.checksum != expected_cksum) {
        printf("[ERRO LOAD] Falha de integridade por checksum invalido no slot %d!\n", slot);
        return false;
    }

    *out_data = loaded;
    printf("[LOAD] Slot %d carregado com sucesso de '%s'! (HP: %d/%d, R: %d, Mapa: %d, Pos: %.1f, %.1f)\n",
           slot, path, out_data->hearts, out_data->max_hearts, out_data->rupees,
           out_data->current_map, out_data->player_x, out_data->player_y);
    return true;
}

bool save_exists(int slot) {
    if (slot < 1 || slot > MAX_SAVE_SLOTS) return false;
    char path[256];
    get_save_path(slot, path, sizeof(path));

    FILE* f = fopen(path, "rb");
    if (!f) return false;

    SaveData test;
    size_t r = fread(&test, sizeof(SaveData), 1, f);
    fclose(f);

    return (r == 1 && test.magic == SAVE_MAGIC && test.checksum == save_calculate_checksum(&test));
}

bool save_delete(int slot) {
    if (slot < 1 || slot > MAX_SAVE_SLOTS) return false;
    char path[256];
    get_save_path(slot, path, sizeof(path));
    return (remove(path) == 0);
}

bool save_copy(int src_slot, int dst_slot) {
    if (src_slot < 1 || src_slot > MAX_SAVE_SLOTS) return false;
    if (dst_slot < 1 || dst_slot > MAX_SAVE_SLOTS) return false;
    if (src_slot == dst_slot) return false;

    SaveData data;
    if (!load_game(src_slot, &data)) return false;
    return save_game(dst_slot, &data);
}
