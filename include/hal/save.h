#ifndef HAL_SAVE_H
#define HAL_SAVE_H

/*
 * ============================================================================
 * include/hal/save.h - Sistema de Persistência e Salvamento (Save/Load)
 * ============================================================================
 * Gerencia a gravação e carregamento de arquivos de progresso (.dat) em disco
 * com validação por cabeçalho mágico e soma de verificação (checksum).
 */

#include "gba/types.h"
#include <stdbool.h>

#define SAVE_MAGIC 0x494E494D // "MINI" em little endian
#define SAVE_VERSION 2
#define MAX_SAVE_SLOTS 3

typedef struct {
    u32   magic;
    u32   version;
    char  player_name[16];
    float player_x;
    float player_y;
    int   player_dir;
    int   current_map;       // 0: Woods, 1: Hyrule Town, 2: Minish Village, 3: Dungeon, 4: South Field, 5: North Field, 6: Crenel Base, 7: Melari's Mines, 8: Cave of Flames
    int   hearts;
    int   max_hearts;
    int   rupees;
    int   bomb_count;
    bool  has_flippers;
    bool  has_spin_attack;
    bool  is_minish;
    bool  has_grip_ring;
    bool  has_cane_of_pacci;
    bool  has_white_sword;
    bool  has_earth_element;
    bool  has_fire_element;
    bool  has_water_element;
    bool  has_wind_element;
    bool  has_two_elements;
    bool  has_three_elements;
    bool  has_four_sword;
    bool  secret_exit_unlocked;
    bool  dungeon_droplets_cleared;
    bool  dungeon_palace_cleared;
    bool  has_bow;
    bool  has_mole_mitts;
    bool  has_armos_activated;
    bool  has_ocarina;
    bool  dungeon_fortress_cleared;
    bool  has_veil_falls_unlocked;
    u8    golden_kinstones_fused;
    bool  cloud_tornado_active;
    u8    unlocked_wind_crests;
    u8    library_books_mask;
    bool  librari_met;
    bool  lake_temple_unlocked;
    bool  has_flame_lantern;
    bool  lantern_lit;
    bool  has_rocs_cape;
    bool  royal_valley_unlocked;
    bool  graveyard_gate_unlocked;
    bool  dampe_met;
    bool  tomb_pushed;
    bool  king_gustaf_met;
    bool  has_royal_kinstone;
    bool  dark_castle_unlocked;
    bool  dark_castle_cleared;
    bool  dark_castle_bells_silenced;
    bool  has_sanctum_key;
    bool  vaati_defeated;
    u8    figurines_mask[17];
    u16   shells_owned;
    bool  has_carlov_medal;
    u8    cucco_level_cleared;
    bool  cucco_heart_piece_obtained;
    int   slot_a;
    int   slot_b;
    u32   unlocked_items_mask;
    u32   checksum;
} SaveData;

// Inicialização do sistema de persistência (cria diretório de saves se não existir)
void save_system_init(void);

// Salva os dados do jogo em um slot específico (1 a 3)
bool save_game(int slot, const SaveData* data);

// Carrega os dados do jogo de um slot específico (1 a 3)
bool load_game(int slot, SaveData* out_data);

// Verifica se existe um arquivo de save válido no slot
bool save_exists(int slot);

// Remove o arquivo de save do slot
bool save_delete(int slot);

// Copia o arquivo de save de um slot para outro
bool save_copy(int src_slot, int dst_slot);

// Calcula o checksum para validação de integridade
u32 save_calculate_checksum(const SaveData* data);

#endif // HAL_SAVE_H
