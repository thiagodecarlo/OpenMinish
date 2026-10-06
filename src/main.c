#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "gba/types.h"
#include "hal/video.h"
#include "hal/texture.h"
#include "hal/input.h"
#include "hal/audio.h"
#include "hal/map.h"
#include "hal/entity.h"
#include "hal/font.h"
#include "hal/dialogue.h"
#include "hal/subweapon.h"
#include "hal/kinstone.h"
#include "hal/dungeon.h"
#include "hal/dungeon_flames.h"
#include "hal/dungeon_fortress.h"
#include "hal/dungeon_droplets.h"
#include "hal/inventory.h"
#include "hal/save.h"
#include "hal/sanctuary.h"
#include "hal/fast_travel.h"
#include "hal/library.h"
#include "hal/lantern.h"
#include "hal/veil_clouds.h"
#include "hal/rocs_cape.h"
#include "hal/dungeon_palace.h"
#include "hal/royal_valley.h"
#include "hal/dungeon_dark_castle.h"
#include "hal/boss_vaati.h"
#include "hal/startup_menu.h"
#include "hal/figurine_gallery.h"
#include "hal/cucco_minigame.h"
#include "hal/sword_dojo.h"
#include "hal/intro_cutscene.h"
#include <math.h>

/*
 * ============================================================================
 * PROJETO ACADÊMICO: The Legend of Zelda: The Minish Cap - Native PC Port
 * Arquivo: src/main.c (Ponto de Entrada, Game Loop, Input e Entidade do Herói)
 * ============================================================================
 */

typedef enum {
    REGION_USA = 0,
    REGION_EUR = 1,
    REGION_JPN = 2,
    REGION_COUNT = 3
} SelectedRegion;

typedef struct {
    float x;
    float y;
    float speed;
    Direction dir;
    bool is_moving;
    bool is_attacking;
    int  attack_timer;
    int  anim_timer;
    int  anim_frame;
    int  hearts;
    int  max_hearts;
    int  health_quarters; // Vida autêntica GBA em quartos de coração (0..max_hearts * 4)
    int  magic;           // Barra de magia GBA (0..100)
    int  max_magic;       // Capacidade total de magia (100)
    int  rupees;
    int  invuln_timer;
    float knock_x;
    float knock_y;

    // Rolamento Acrobático Canônico (Somersault Roll & Tiger Scroll #2)
    bool is_rolling;
    int  roll_timer;

    // Habilidade Lendária: Ataque Giratório (Tiger Scroll #1)
    bool has_spin_attack;
    bool is_charging_spin;
    int  spin_charge_timer;
    bool spin_ready;
    bool is_spinning;
    int  spin_timer;
    int  tiger_scroll_banner_timer;

    // Mecânica Canônica de Encolhimento Minish (Minish Shrinking & Portal)
    bool is_minish;             // Está no tamanho Minish (8x8 px, hitbox 4x4)
    bool is_transforming;       // Em transição cinemática de transformação
    int  transform_timer;       // Temporizador da animação de transformação (0..50 frames)
    bool transform_to_minish;   // true = encolhendo para Minish, false = crescendo para Humano

    // Item Canônico: Nadadeiras de Zora (Zora's Flippers) & Natação
    bool has_flippers;          // Possui as Nadadeiras de Zora
    bool is_swimming;          // Nadando na água
    int  swim_stroke_timer;    // Temporizador de braçada e áudio de água
    bool is_diving;            // Mergulhado sob a água
    int  dive_timer;           // Duração do mergulho (0..45 frames)
    int  water_ripple_timer;   // Animação de ondulações na água

    // Item Canônico: Anel de Escalada (Grip Ring) & Monte Crenel
    bool has_grip_ring;         // Possui o Grip Ring (comprado do Business Scrub)
    bool is_climbing;          // Escalando paredão rochoso ou vinha
    int  climb_anim_timer;     // Temporizador de animação de escalada
    bool has_mineral_water;    // Carrega Água Mineral do Monte Crenel para regar sementes

    // Item Canônico: Cajado de Pacci (Cane of Pacci) & Salto Vertical
    bool has_cane_of_pacci;    // Possui o Cajado de Pacci
    float z;                   // Altitude / Altura do salto no ar
    float vz;                  // Velocidade vertical do salto
    bool is_jumping;           // Está no ar executando super-salto

    // Melhoria Canônica de Espada: White Sword (Espada Branca)
    bool has_white_sword;          // Possui a lendária Espada Branca forjada por Melari (Dano: 2)
    int  white_sword_banner_timer; // Temporizador do banner comemorativo de aquisição

    // Elemento Canônico: Sagrado Elemento Fogo (Fire Element)
    bool has_fire_element;          // Conquistado ao derrotar Gleerok na Cave of Flames
    int  fire_element_banner_timer; // Temporizador do banner festivo de obtenção

    // Elemento Canônico: Sagrado Elemento Água (Water Element)
    bool has_water_element;         // Conquistado ao derrotar Big Octorok no Temple of Droplets
    int  water_element_banner_timer;// Temporizador do banner festivo de obtenção

    // Elemento Canônico: Sagrado Elemento Vento (Wind Element)
    bool has_wind_element;          // Conquistado ao derrotar Gyorg Pair no Palace of Winds
    int  wind_element_banner_timer; // Temporizador do banner festivo de obtenção
    bool dungeon_palace_cleared;

    // Habilidade Lendária Four Sword: Infusão de 2, 3 e 4 Elementos & Clones
    bool has_two_elements;          // White Sword (Two Elements) infundida no Santuário
    int  two_elements_banner_timer; // Temporizador do banner da Four Sword
    bool has_three_elements;        // White Sword (Three Elements) infundida no Santuário
    int  three_elements_banner_timer; // Temporizador do banner dos 3 Elementos
    bool has_four_sword;            // Four Sword Completa Forjada (4 Clones + Sword Beams)
    int  four_sword_banner_timer;   // Temporizador do banner da Four Sword Forjada
    bool has_royal_kinstone;        // Royal Golden Kinstone (Rei Gustaf)
    int  royal_kinstone_banner_timer; // Temporizador do banner da Kinstone Real
    bool has_sanctum_key;           // Chave do Santuario de Vaati (Dark Hyrule Castle)
    int  sanctum_banner_timer;      // Temporizador do banner de abertura do Santuario
    bool sanctum_banner_shown;      // Flag para exibir o banner do Santuário apenas uma vez
    bool vaati_defeated;            // Vaati derrotado definitivamente (Ato V concluido)

    // Regiões Canônicas: Quedas do Véu (Veil Falls) e Topo das Nuvens (Cloud Tops)
    bool has_veil_falls_unlocked;
    u8   golden_kinstones_fused;
    bool cloud_tornado_active;
    int  veil_banner_timer;
    int  cloud_banner_timer;

    // Item Canônico: Arco e Flechas & Pântano de Castor Wilds
    bool has_bow;                  // Possui o Arco e Flechas
    int  bow_banner_timer;         // Temporizador do banner do Arco
    int  mud_sink_timer;           // Temporizador de afundamento no lodo movediço
    float last_safe_x;             // Posição segura para respawn caso afunde
    float last_safe_y;

    // Item Canônico: Luvas de Toupeira (Mole Mitts) & Mole Cave
    bool has_mole_mitts;           // Possui as Luvas de Toupeira
    int  mole_mitts_banner_timer;  // Temporizador do banner das Mole Mitts

    // Mecânica Canônica: Wind Ruins & Robô Armos
    bool has_armos_activated;      // Circuito do robô Armos ativado pelo Minish
    int  armos_banner_timer;       // Temporizador do banner do Armos

    // Item Canônico: Ocarina do Vento (Ocarina of Wind) & Fortaleza dos Ventos
    bool has_ocarina;              // Possui a Ocarina do Vento
    int  ocarina_banner_timer;     // Temporizador do banner da Ocarina

    // Item Canônico: Lanterna de Chamas (Flame Lantern)
    bool has_lantern;              // Possui a Lanterna de Chamas
    int  lantern_banner_timer;     // Temporizador do banner da Lanterna

    // Item Canônico: Capa de Roc (Roc's Cape) - Salto Livre, Planar e Down-Thrust
    bool has_rocs_cape;            // Possui a Capa de Roc
    int  rocs_banner_timer;        // Temporizador do banner comemorativo

    // Sistema Canônico de Colecionáveis: Galeria de Estatuetas & Conchas Misteriosas (Ato VI)
    int  shells;                   // Conchas Misteriosas (Mysterious Shells) possuídas
    bool has_carlov_medal;         // Medalha de Carlov concedida
    bool is_carrying_cucco;        // Carregando galinha Cucco nos braços
} Player;

static const char* s_region_tags[REGION_COUNT] = { "usa", "eur", "jpn" };
static const char* s_region_names[REGION_COUNT] = {
    "USA (Ingles) [BZME]",
    "EUR (Multi-5) [BZMP]",
    "JPN (Japao)   [BZMJ]"
};

static Texture* s_sheet0 = NULL;
static Texture* s_sheet1 = NULL;
static Texture* s_link_tex = NULL;
static Texture* s_octo_tex = NULL;
static Texture* s_enemies_tex = NULL;
static Texture* s_npcs_tex = NULL;
static Texture* s_bosses_tex = NULL;
static Texture* s_hud_items_tex = NULL;
static Tilemap* s_world_map          = NULL;
static Tilemap* s_town_map           = NULL;
static Tilemap* s_village_map        = NULL;
static Tilemap* s_south_field_map    = NULL;
static Tilemap* s_north_field_map    = NULL;
static Tilemap* s_crenel_base_map    = NULL;
static Tilemap* s_melari_mines_map   = NULL;
static Tilemap* s_castor_wilds_map   = NULL;
static Tilemap* s_mole_cave_map      = NULL;
static Tilemap* s_wind_ruins_map     = NULL;
static Tilemap* s_armos_interior_map = NULL;
static Tilemap* s_library_map        = NULL;
static Tilemap* s_lake_hylia_map     = NULL;
static Tilemap* s_castle_courtyard_map = NULL;
static bool     s_in_town            = false;
static bool     s_in_village         = false;
static bool     s_in_south_field     = false;
static bool     s_in_north_field     = false;
static bool     s_in_crenel_base     = false;
static bool     s_in_melari_mines    = false;
static bool     s_in_castor_wilds    = false;
static bool     s_in_mole_cave       = false;
static bool     s_in_wind_ruins      = false;
static bool     s_in_armos_interior  = false;
static bool     s_in_library         = false;
static bool     s_in_lake_hylia      = false;
static bool     s_in_castle_courtyard= false;
static SelectedRegion s_current_region = REGION_USA;

static void load_region_sheets(SelectedRegion region) {
    if (s_sheet0)        { texture_free(s_sheet0);        s_sheet0 = NULL; }
    if (s_sheet1)        { texture_free(s_sheet1);        s_sheet1 = NULL; }
    if (s_link_tex)      { texture_free(s_link_tex);      s_link_tex = NULL; }
    if (s_octo_tex)      { texture_free(s_octo_tex);      s_octo_tex = NULL; }
    if (s_enemies_tex)   { texture_free(s_enemies_tex);   s_enemies_tex = NULL; }
    if (s_npcs_tex)      { texture_free(s_npcs_tex);      s_npcs_tex = NULL; }
    if (s_bosses_tex)    { texture_free(s_bosses_tex);    s_bosses_tex = NULL; }
    if (s_hud_items_tex) { texture_free(s_hud_items_tex); s_hud_items_tex = NULL; }

    s_current_region = region;

    char path0[256];
    char path1[256];
    char path_link[256];
    char path_octo[256];
    char path_enemies[256];
    char path_npcs[256];
    char path_bosses[256];
    char path_hud[256];
    snprintf(path0, sizeof(path0), "assets/regions/%s/sheet_00.bmp", s_region_tags[region]);
    snprintf(path1, sizeof(path1), "assets/regions/%s/sheet_01.bmp", s_region_tags[region]);
    snprintf(path_link, sizeof(path_link), "assets/regions/%s/link.bmp", s_region_tags[region]);
    snprintf(path_octo, sizeof(path_octo), "assets/regions/%s/octorok.bmp", s_region_tags[region]);
    snprintf(path_enemies, sizeof(path_enemies), "assets/regions/%s/enemies.bmp", s_region_tags[region]);
    snprintf(path_npcs, sizeof(path_npcs), "assets/regions/%s/npcs.bmp", s_region_tags[region]);
    snprintf(path_bosses, sizeof(path_bosses), "assets/regions/%s/bosses.bmp", s_region_tags[region]);
    snprintf(path_hud, sizeof(path_hud), "assets/ui/hud_items.bmp");

    s_sheet0 = texture_load_bmp(path0);
    s_sheet1 = texture_load_bmp(path1);
    s_link_tex = texture_load_bmp(path_link);
    if (!s_link_tex) {
        s_link_tex = texture_load_bmp("assets/regions/link_master.bmp");
    }
    s_octo_tex = texture_load_bmp(path_octo);
    s_enemies_tex = texture_load_bmp(path_enemies);
    if (!s_enemies_tex) {
        s_enemies_tex = texture_load_bmp("assets/regions/enemies_master.bmp");
    }
    s_npcs_tex = texture_load_bmp(path_npcs);
    if (!s_npcs_tex) {
        s_npcs_tex = texture_load_bmp("assets/regions/npcs_master.bmp");
    }
    s_bosses_tex = texture_load_bmp(path_bosses);
    if (!s_bosses_tex) {
        s_bosses_tex = texture_load_bmp("assets/regions/bosses_master.bmp");
    }
    s_hud_items_tex = texture_load_bmp(path_hud);
    if (!s_hud_items_tex) {
        char fallback_hud[256];
        snprintf(fallback_hud, sizeof(fallback_hud), "assets/regions/%s/hud_items.bmp", s_region_tags[region]);
        s_hud_items_tex = texture_load_bmp(fallback_hud);
    }

    entity_set_texture(s_octo_tex);
    entity_set_enemies_texture(s_enemies_tex);
    entity_set_npcs_texture(s_npcs_tex);
    entity_set_bosses_texture(s_bosses_tex);
    dungeon_fortress_set_bosses_texture(s_bosses_tex);
    dungeon_droplets_set_bosses_texture(s_bosses_tex);
    dungeon_palace_set_bosses_texture(s_bosses_tex);
    intro_cutscene_set_npcs_texture(s_npcs_tex);
    intro_cutscene_set_bosses_texture(s_bosses_tex);
    intro_cutscene_set_link_texture(s_link_tex);
    intro_cutscene_set_hud_texture(s_hud_items_tex);
    intro_cutscene_set_enemies_texture(s_enemies_tex);
    if (s_castle_courtyard_map) {
        intro_cutscene_set_castle_texture(s_castle_courtyard_map->authentic_tex);
    }
    boss_vaati_set_npcs_texture(s_npcs_tex);

    map_load_tileset(s_region_tags[region]);

    if (s_world_map)           map_set_region(s_world_map, s_region_tags[region]);
    if (s_town_map)            map_set_region(s_town_map, s_region_tags[region]);
    if (s_crenel_base_map)     map_set_region(s_crenel_base_map, s_region_tags[region]);
    if (s_castor_wilds_map)    map_set_region(s_castor_wilds_map, s_region_tags[region]);
    if (s_castle_courtyard_map)map_set_region(s_castle_courtyard_map, s_region_tags[region]);

    printf("[REGIAO ATUALIZADA] -> %s (Link: %s, Inimigos: %s, NPCs: %s, Chefes: %s, Octorok: %s, HUD: %s, Mapa: %s)\n",
           s_region_names[region],
           s_link_tex ? "Autentico GBA" : "Procedural",
           s_enemies_tex ? "Autentico GBA" : "Procedural",
           s_npcs_tex ? "Autentico GBA" : "Procedural",
           s_bosses_tex ? "Autentico GBA" : "Procedural",
           s_octo_tex ? "Autentico GBA" : "Procedural",
           s_hud_items_tex ? "Autentico GBA" : "Procedural",
           (s_world_map && s_world_map->is_authentic) ? "Autentico Minish Woods" : "Procedural");
}

static void spawn_overworld_entities(Tilemap* world_map) {
    entity_clear_all();
    if (world_map && world_map->is_authentic) {
        // Inimigos clássicos Octorok distribuídos nas clareiras norte e sul
        entity_spawn(ENTITY_ENEMY_OCTOROK, 430.0f, 440.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 490.0f, 440.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 448.0f, 704.0f);
        // Morcegos voadores Keese (com sombra e vôo senoidal) nas clareiras abertas
        entity_spawn(ENTITY_ENEMY_KEESE, 510.0f, 420.0f);
        entity_spawn(ENTITY_ENEMY_KEESE, 420.0f, 710.0f);
        // Gosmas Green ChuChu nas laterais da praça do jardim
        entity_spawn(ENTITY_ENEMY_CHUCHU, 472.0f, 620.0f);
        entity_spawn(ENTITY_ENEMY_CHUCHU, 472.0f, 644.0f);
        // Habitante Minish amigável no lado oeste da praça do santuário (espaço livre sem sobreposição)
        entity_spawn(ENTITY_NPC_FOREST_MINISH, 416.0f, 636.0f);
        // Mestre Espadachim Swiftblade na ampla clareira norte de treinamento
        entity_spawn(ENTITY_NPC_SWIFTBLADE, 380.0f, 448.0f);
        // Toco de Árvore Minish (Minish Tree Stump Portal) na clareira sul junto ao Tronco Oco
        Entity* stump = entity_spawn(ENTITY_MINISH_STUMP, 312.0f, 760.0f);
        if (stump) stump->action = 0;
    } else {
        entity_spawn(ENTITY_ENEMY_OCTOROK, 160.0f, 220.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 420.0f, 150.0f);
        entity_spawn(ENTITY_ENEMY_OCTOROK, 340.0f, 310.0f);
        entity_spawn(ENTITY_ENEMY_KEESE, 200.0f, 130.0f);
        entity_spawn(ENTITY_ENEMY_CHUCHU, 320.0f, 220.0f);
        entity_spawn(ENTITY_NPC_FOREST_MINISH, 250.0f, 176.0f);
        entity_spawn(ENTITY_NPC_SWIFTBLADE, 260.0f, 220.0f);
        Entity* stump = entity_spawn(ENTITY_MINISH_STUMP, 280.0f, 200.0f);
        if (stump) stump->action = 0;
    }
}

static void spawn_town_entities(void) {
    entity_clear_all();
    // 1. Comerciante Stockwell em seu mercado (balcão a leste da praça)
    entity_spawn(ENTITY_NPC_SHOPKEEPER, 448.0f, 104.0f);

    // 2. Chafariz Central ornamental borbulhante no meio da praça
    entity_spawn(ENTITY_TOWN_FOUNTAIN, 280.0f, 216.0f);

    // 3. Cidadãos e Moradores de Hyrule
    // Cidadã com metade de Kinstone azul passeando pela praça
    entity_spawn(ENTITY_NPC_TOWN_CITIZEN, 224.0f, 240.0f);
    // Cidadã perto das residências a oeste
    entity_spawn(ENTITY_NPC_TOWN_CITIZEN, 112.0f, 128.0f);

    // 4. Guardas Reais vigiando o Portão Norte do Castelo de Hyrule
    entity_spawn(ENTITY_NPC_TOWN_GUARD, 240.0f, 32.0f);
    entity_spawn(ENTITY_NPC_TOWN_GUARD, 320.0f, 32.0f);

    // 5. Portal Vaso Minish (Minish Urn Portal) a sudoeste da praça
    Entity* urn = entity_spawn(ENTITY_MINISH_STUMP, 216.0f, 280.0f);
    if (urn) urn->action = 1;

    // 6. Mestre Ferreiro Smith em sua oficina / ferraria
    entity_spawn(ENTITY_NPC_SMITH, 120.0f, 200.0f);

    // 7. Mestre Espadachim Swiftblade no Dojo de esgrima
    entity_spawn(ENTITY_NPC_SWIFTBLADE, 360.0f, 140.0f);

    // 8. Prefeito Hagen em frente à sua residência/prefeitura
    entity_spawn(ENTITY_NPC_MAYOR_HAGEN, 180.0f, 100.0f);

    // 9. Deku Business Scrub no mercado/beco de mercadorias raras
    entity_spawn(ENTITY_NPC_BUSINESS_SCRUB, 300.0f, 280.0f);

    printf("[TOWN] Entidades de Hyrule Town spawnadas com sucesso (Smith, Swiftblade, Hagen, Scrub, Stockwell, Chafariz, Cidadaos, Guardas)!\n");
}

static void transition_to_town(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = true;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    link->x = 280.0f; // Portão Sul (x = 17.5 * 16)
    link->y = 392.0f; // Entrada Sul (y = 24.5 * 16)
    link->dir = DIR_UP;
    link->is_moving = false;
    spawn_town_entities();
    hal_audio_play_bgm(BGM_HYRULE_TOWN);
    hal_audio_play_sound(SOUND_TOWN_BELL, 1.0f, 1.0f);
    printf("[SCENE] Entrando na Cidade de Hyrule (Hyrule Town Hub)!\n");
}

static void transition_to_overworld(Player* link, Tilemap* world_map) {
    if (dungeon_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    if (world_map && world_map->is_authentic) {
        link->x = 448.0f;
        link->y = 636.0f;
    } else {
        link->x = 296.0f;
        link->y = 240.0f;
    }
    link->dir = DIR_DOWN;
    link->is_moving = false;
    spawn_overworld_entities(world_map);
    hal_audio_play_bgm(BGM_MINISH_WOODS);
    printf("[SCENE] Retornando a Minish Woods / Overworld!\n");
}

static void spawn_minish_village_entities(void) {
    entity_clear_all();

    // 1. Ancião Gentari em seu Altar Sagrado ao norte da vila
    entity_spawn(ENTITY_NPC_GENTARI, 248.0f, 60.0f);

    // 2. Sacerdote Festari guardando a ermida a noroeste (caminho para Deepwood Shrine)
    entity_spawn(ENTITY_NPC_FESTARI, 96.0f, 108.0f);

    // 3. Habitantes e Moradores Minish passeando pela vila (com fragmentos de Kinstone)
    entity_spawn(ENTITY_NPC_VILLAGE_MINISH, 144.0f, 252.0f);
    entity_spawn(ENTITY_NPC_VILLAGE_MINISH, 384.0f, 108.0f);
    entity_spawn(ENTITY_NPC_VILLAGE_MINISH, 384.0f, 280.0f);

    // 4. Toco Minish (Portal de Encolhimento / Crescimento) junto ao jardim
    Entity* stump = entity_spawn(ENTITY_MINISH_STUMP, 176.0f, 288.0f);
    if (stump) stump->action = 0;

    printf("[MINISH VILLAGE] Entidades da Vila spawnadas (Gentari, Festari, Moradores, Toco)!\n");
}

static void transition_to_minish_village(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = false;
    s_in_village = true;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    link->is_minish = true; // Link sempre no tamanho Minish na Vila dos Minish!
    link->x = 248.0f; // Saída sul (x = 15.5 * 16)
    link->y = 352.0f; // Entrada sul da vila (y = 22 * 16)
    link->dir = DIR_UP;
    link->is_moving = false;
    spawn_minish_village_entities();
    hal_audio_play_bgm(BGM_MINISH_VILLAGE);
    printf("[SCENE] Entrando na Vila dos Minish (Picori Village)!\n");
}

static void transition_to_woods_from_village(Player* link, Tilemap* world_map) {
    if (dungeon_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    if (world_map && world_map->is_authentic) {
        link->x = 336.0f; // Topo do Tronco Oco (coluna 21 * 16 = 336)
        link->y = 720.0f; // Logo acima da boca norte do tronco oco
    } else {
        link->x = 296.0f;
        link->y = 240.0f;
    }
    link->dir = DIR_DOWN;
    link->is_moving = false;
    spawn_overworld_entities(world_map);
    hal_audio_play_bgm(BGM_MINISH_WOODS);
    printf("[SCENE] Retornando a Minish Woods a partir da Vila dos Minish!\n");
}

static void spawn_south_field_entities(void) {
    entity_clear_all();
    // Inimigos Moblin guardando as planícies abertas
    entity_spawn(ENTITY_ENEMY_MOBLIN, 192.0f, 160.0f);
    entity_spawn(ENTITY_ENEMY_MOBLIN, 320.0f, 220.0f);
    // Peahat sobrevoando o centro dos campos
    entity_spawn(ENTITY_ENEMY_PEAHAT, 224.0f, 112.0f);
    // Malon da Fazenda Lon Lon próxima ao portão de madeira
    entity_spawn(ENTITY_NPC_MALON, 360.0f, 232.0f);

    printf("[SOUTH FIELD] Entidades de South Hyrule Field spawnadas (Moblins, Peahat, Malon)!\n");
}

static void spawn_north_field_entities(void) {
    entity_clear_all();
    // Moblins de patrulha no caminho norte para o Castelo
    entity_spawn(ENTITY_ENEMY_MOBLIN, 208.0f, 144.0f);
    entity_spawn(ENTITY_ENEMY_MOBLIN, 304.0f, 144.0f);
    // Peahat na encosta rochosa em direção ao Monte Crenel
    entity_spawn(ENTITY_ENEMY_PEAHAT, 128.0f, 192.0f);

    printf("[NORTH FIELD] Entidades de North Hyrule Field spawnadas (Moblins, Peahat)!\n");
}

static void transition_to_south_field(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = true;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    link->x = 248.0f; // Topo central (saída norte da planície)
    link->y = 36.0f;
    link->dir = DIR_DOWN;
    link->is_moving = false;
    spawn_south_field_entities();
    hal_audio_play_bgm(BGM_HYRULE_OVERWORLD);
    printf("[SCENE] Entrando em South Hyrule Field (Planicies do Sul)!\n");
}

static void transition_to_north_field(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = true;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    link->x = 248.0f; // Base sul da planície norte
    link->y = 330.0f;
    link->dir = DIR_UP;
    link->is_moving = false;
    spawn_north_field_entities();
    hal_audio_play_bgm(BGM_HYRULE_OVERWORLD);
    printf("[SCENE] Entrando em North Hyrule Field (Planicies do Norte)!\n");
}

static void transition_to_town_from_south(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = true;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    link->x = 280.0f;
    link->y = 380.0f;
    link->dir = DIR_UP;
    link->is_moving = false;
    spawn_town_entities();
    hal_audio_play_bgm(BGM_HYRULE_TOWN);
    hal_audio_play_sound(SOUND_TOWN_BELL, 1.0f, 1.0f);
    printf("[SCENE] Retornando a Hyrule Town via Portao Sul!\n");
}

static void transition_to_town_from_north(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = true;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    link->x = 280.0f;
    link->y = 50.0f;
    link->dir = DIR_DOWN;
    link->is_moving = false;
    spawn_town_entities();
    hal_audio_play_bgm(BGM_HYRULE_TOWN);
    hal_audio_play_sound(SOUND_TOWN_BELL, 1.0f, 1.0f);
    printf("[SCENE] Retornando a Hyrule Town via Portao Norte!\n");
}

static void spawn_crenel_base_entities(void) {
    entity_clear_all();
    // 1. Tektites saltitantes nas encostas rochosas
    entity_spawn(ENTITY_ENEMY_TEKTITE, 160.0f, 160.0f);
    entity_spawn(ENTITY_ENEMY_TEKTITE, 320.0f, 120.0f);
    // 2. Spiny Beetles camuflados sob pedras
    entity_spawn(ENTITY_ENEMY_SPINY_BEETLE, 220.0f, 220.0f);
    entity_spawn(ENTITY_ENEMY_SPINY_BEETLE, 380.0f, 260.0f);
    // 3. Business Scrub comerciante (vendedor do Grip Ring) em sua clareira a sudeste
    entity_spawn(ENTITY_NPC_BUSINESS_SCRUB, 416.0f, 304.0f);

    printf("[CRENEL BASE] Entidades de Mount Crenel Base spawnadas (Tektites, Spiny Beetles, Business Scrub)!\n");
}

static void transition_to_crenel_base(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = true;
    s_in_melari_mines = false;
    link->x = 480.0f; // Entrada leste vindo de North Field
    link->y = 240.0f;
    link->dir = DIR_LEFT;
    link->is_moving = false;
    spawn_crenel_base_entities();
    hal_audio_play_bgm(BGM_MT_CRENEL);
    printf("[SCENE] Entrando em Mount Crenel Base (Base do Monte Crenel)!\n");
}

static void transition_to_north_field_from_crenel(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = true;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    link->x = 32.0f; // Estrada oeste de North Field
    link->y = 192.0f;
    link->dir = DIR_RIGHT;
    link->is_moving = false;
    spawn_north_field_entities();
    hal_audio_play_bgm(BGM_HYRULE_OVERWORLD);
    printf("[SCENE] Retornando a North Hyrule Field a partir do Monte Crenel!\n");
}

static void spawn_melari_mines_entities(void) {
    entity_clear_all();
    // 1. Mestre Ferreiro Melari na bigorna da forja central
    entity_spawn(ENTITY_NPC_MELARI, 248.0f, 100.0f);
    // 2. Mineradores Mountain Minish
    entity_spawn(ENTITY_NPC_MOUNTAIN_MINISH, 96.0f, 160.0f);  // minerando no veio oeste
    entity_spawn(ENTITY_NPC_MOUNTAIN_MINISH, 380.0f, 220.0f); // trabalhando junto ao carrinho leste
    entity_spawn(ENTITY_NPC_MOUNTAIN_MINISH, 350.0f, 110.0f); // observando os canais de lava
    printf("[MELARI MINES] Entidades de Melari's Mines spawnadas (Mestre Melari, 3 Mountain Minish)!\n");
}

static void transition_to_melari_mines(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = true;
    link->x = 256.0f; // Entrada sul vindo do Monte Crenel
    link->y = 336.0f;
    link->dir = DIR_UP;
    link->is_moving = false;
    spawn_melari_mines_entities();
    hal_audio_play_bgm(BGM_MT_CRENEL);
    printf("[SCENE] Entrando em Melari's Mines (Minas de Melari - Forja da White Sword)!\n");
}

static void transition_to_crenel_base_from_melari(Player* link) {
    if (dungeon_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_melari_mines = false;
    s_in_crenel_base = true;
    link->x = 288.0f; // Plataforma norte de Mount Crenel Base
    link->y = 64.0f;
    link->dir = DIR_DOWN;
    link->is_moving = false;
    spawn_crenel_base_entities();
    hal_audio_play_bgm(BGM_MT_CRENEL);
    printf("[SCENE] Retornando a Mount Crenel Base a partir das Minas de Melari!\n");
}

static void transition_to_cave_of_flames(Player* link) {
    if (dungeon_is_active() || dungeon_flames_is_active() || sanctuary_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    entity_clear_all();
    dungeon_flames_enter(&link->x, &link->y, &link->dir);
}

static void transition_to_sanctuary(Player* link) {
    if (dungeon_is_active() || dungeon_flames_is_active() || sanctuary_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    entity_clear_all();
    sanctuary_enter(&link->x, &link->y, &link->dir);
}

static void spawn_castor_wilds_entities(void) {
    entity_clear_all();
    // 1. Serpentes ágeis Rope patrulhando o pântano
    entity_spawn(ENTITY_ENEMY_ROPE, 240.0f, 160.0f);
    entity_spawn(ENTITY_ENEMY_ROPE, 350.0f, 260.0f);
    entity_spawn(ENTITY_ENEMY_ROPE, 140.0f, 320.0f);
    // 2. Pedestal ancestral com o Arco e Flechas no noroeste (x=7*16=112, y=4*16=64)
    entity_spawn(ENTITY_ITEM_BOW, 112.0f, 64.0f);
    printf("[CASTOR WILDS] Entidades de Castor Wilds spawnadas (3 Ropes, Pedestal do Arco e Flechas)!\n");
}

static void transition_to_castor_wilds(Player* link) {
    if (dungeon_is_active() || dungeon_flames_is_active() || sanctuary_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = true;
    s_in_mole_cave = false;
    link->x = 520.0f; // Entrada leste vindo de South Hyrule Field
    link->y = 224.0f;
    link->dir = DIR_LEFT;
    link->is_moving = false;
    spawn_castor_wilds_entities();
    hal_audio_play_bgm(BGM_CASTOR_WILDS);
    printf("[SCENE] Entrando no Pantano de Castor Wilds (Castor Wilds Swamp)!\n");
}

static void transition_to_south_field_from_castor(Player* link) {
    if (dungeon_is_active() || dungeon_flames_is_active() || sanctuary_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = true;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = false;
    s_in_mole_cave = false;
    link->x = 32.0f; // Fronteira oeste de South Hyrule Field
    link->y = 224.0f;
    link->dir = DIR_RIGHT;
    link->is_moving = false;
    spawn_south_field_entities();
    hal_audio_play_bgm(BGM_HYRULE_OVERWORLD);
    printf("[SCENE] Retornando a South Hyrule Field a partir de Castor Wilds!\n");
}

static void spawn_mole_cave_entities(bool has_mole_mitts) {
    entity_clear_all();
    if (!has_mole_mitts) {
        // Pedestal com as Luvas de Toupeira (Mole Mitts) na câmara central da caverna
        entity_spawn(ENTITY_ITEM_MOLE_MITTS, 256.0f, 80.0f);
    }
    // Inimigos e baús no labirinto de terra escavável
    entity_spawn(ENTITY_ENEMY_KEESE, 120.0f, 160.0f);
    entity_spawn(ENTITY_ENEMY_KEESE, 380.0f, 160.0f);
    entity_spawn(ENTITY_ENEMY_CHUCHU, 256.0f, 220.0f);
    entity_spawn(ENTITY_CHEST_GOLD, 80.0f, 80.0f);
    entity_spawn(ENTITY_CHEST_GOLD, 432.0f, 80.0f);
    printf("[MOLE CAVE] Entidades da Mole Cave spawnadas (Mole Mitts, Keese, ChuChu, Baus de Ouro)!\n");
}

static void transition_to_mole_cave(Player* link) {
    if (dungeon_is_active() || dungeon_flames_is_active() || sanctuary_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = false;
    s_in_mole_cave = true;
    link->x = 256.0f; // Entrada sul da caverna
    link->y = 350.0f;
    link->dir = DIR_UP;
    link->is_moving = false;
    spawn_mole_cave_entities(link->has_mole_mitts);
    hal_audio_play_bgm(BGM_DEEPWOOD_SHRINE);
    printf("[SCENE] Entrando na Caverna das Luvas de Toupeira (Mole Cave)!\n");
}

static void transition_to_castor_from_cave(Player* link) {
    if (dungeon_is_active() || dungeon_flames_is_active() || sanctuary_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = true;
    s_in_mole_cave = false;
    s_in_wind_ruins = false;
    s_in_armos_interior = false;
    link->x = 112.0f; // Saída na caverna no noroeste de Castor Wilds
    link->y = 120.0f;
    link->dir = DIR_DOWN;
    link->is_moving = false;
    spawn_castor_wilds_entities();
    hal_audio_play_bgm(BGM_CASTOR_WILDS);
    printf("[SCENE] Retornando a Castor Wilds a partir da Caverna!\n");
}

static void spawn_wind_ruins_entities(bool has_circuit_active) {
    entity_clear_all();
    // 1. Sentinela robô Armos bloqueando a passagem central (x=17.5*16=280, y=10*16=160)
    Entity* armos = entity_spawn(ENTITY_ARMOS, 280.0f, 160.0f);
    if (armos && has_circuit_active) {
        armos->action = 2; // Já desperto e movido para o lado
        armos->x += 24.0f;
    }
    // 2. Toco Minish (Portal de Encolhimento) para infiltrar o robô Armos (x=10*16=160, y=15*16=240)
    entity_spawn(ENTITY_MINISH_STUMP, 160.0f, 240.0f);
    // 3. Inimigos patrulhando as ruínas ancestrais
    entity_spawn(ENTITY_ENEMY_ROPE, 190.0f, 290.0f);
    entity_spawn(ENTITY_ENEMY_SPINY_BEETLE, 380.0f, 220.0f);
    entity_spawn(ENTITY_ENEMY_ROPE, 340.0f, 100.0f);
    // 4. Baú com tesouro no terraço leste
    entity_spawn(ENTITY_CHEST_GOLD, 496.0f, 48.0f);
    printf("[WIND RUINS] Entidades das Ruinas do Vento spawnadas (Armos, Toco Minish, Ropes, Spiny Beetle)!\n");
}

static void spawn_armos_interior_entities(bool is_active) {
    entity_clear_all();
    // 1. Interruptor / Dínamo Central do Armos (x=7.5*16=120, y=7.5*16=120)
    Entity* sw = entity_spawn(ENTITY_ARMOS_SWITCH, 120.0f, 120.0f);
    if (sw && is_active) {
        sw->action = 1;
    }
    printf("[ARMOS INTERIOR] Mecanismo interno do Armos spawnado (Interruptor Central)!\n");
}

static void transition_to_wind_ruins(Player* link) {
    if (dungeon_is_active() || dungeon_flames_is_active() || sanctuary_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = false;
    s_in_mole_cave = false;
    s_in_wind_ruins = true;
    s_in_armos_interior = false;
    link->x = 24.0f; // Entrada oeste vindo de Castor Wilds
    link->y = 192.0f;
    link->dir = DIR_RIGHT;
    link->is_moving = false;
    spawn_wind_ruins_entities(armos_circuit_is_active());
    hal_audio_play_bgm(BGM_WIND_RUINS);
    printf("[SCENE] Entrando em Wind Ruins (Ruinas do Vento)!\n");
}

static void transition_to_castor_from_ruins(Player* link) {
    if (dungeon_is_active() || dungeon_flames_is_active() || sanctuary_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = true;
    s_in_mole_cave = false;
    s_in_wind_ruins = false;
    s_in_armos_interior = false;
    link->x = 520.0f; // Saída leste de Castor Wilds
    link->y = 224.0f;
    link->dir = DIR_LEFT;
    link->is_moving = false;
    spawn_castor_wilds_entities();
    hal_audio_play_bgm(BGM_CASTOR_WILDS);
    printf("[SCENE] Retornando a Castor Wilds a partir das Ruinas do Vento!\n");
}

static void transition_to_armos_interior(Player* link) {
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = false;
    s_in_mole_cave = false;
    s_in_wind_ruins = false;
    s_in_armos_interior = true;
    link->x = 120.0f; // Entrada sul dentro do Armos
    link->y = 216.0f;
    link->dir = DIR_UP;
    link->is_moving = false;
    spawn_armos_interior_entities(armos_circuit_is_active());
    hal_audio_play_sound(SOUND_DOOR_SHUTTER, 0.9f, 1.2f);
    printf("[SCENE] Minish Link entrando no interior do robo Armos!\n");
}

static void transition_to_wind_ruins_from_armos(Player* link) {
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = false;
    s_in_mole_cave = false;
    s_in_wind_ruins = true;
    s_in_armos_interior = false;
    link->x = 280.0f; // Em frente à sentinela Armos nas ruínas
    link->y = 180.0f;
    link->dir = DIR_DOWN;
    link->is_moving = false;
    spawn_wind_ruins_entities(armos_circuit_is_active());
    hal_audio_play_bgm(BGM_WIND_RUINS);
    printf("[SCENE] Saindo do Armos e retornando a Wind Ruins!\n");
}

static void spawn_library_entities(void) {
    entity_clear_all();
    // 1. Bibliotecária no balcão de pesquisa
    entity_spawn(ENTITY_NPC_TOWN_CITIZEN, 184.0f, 56.0f);
    // 2. Portal Urna Minish no canto sudoeste da biblioteca
    Entity* urn = entity_spawn(ENTITY_MINISH_STUMP, 48.0f, 224.0f);
    if (urn) urn->action = 1;
    // 3. Ancião Librari no topo da estante noroeste
    entity_spawn(ENTITY_NPC_LIBRARI, 64.0f, 32.0f);
    printf("[LIBRARY] Entidades da Biblioteca Real spawnadas (Bibliotecaria, Urna Minish, Anciao Librari)!\n");
}

static void transition_to_library(Player* link) {
    s_in_town = s_in_village = s_in_south_field = s_in_north_field = false;
    s_in_crenel_base = s_in_melari_mines = s_in_castor_wilds = s_in_mole_cave = false;
    s_in_wind_ruins = s_in_armos_interior = s_in_lake_hylia = false;
    s_in_library = true;
    link->x = 184.0f; // Entrada sul da biblioteca
    link->y = 250.0f;
    link->dir = DIR_UP;
    link->is_moving = false;
    spawn_library_entities();
    hal_audio_play_bgm(BGM_HYRULE_TOWN);
    printf("[SCENE] Entrando na Biblioteca Real da Cidade de Hyrule!\n");
}

static void transition_to_town_from_library(Player* link) {
    s_in_library = false;
    s_in_town = true;
    link->x = 240.0f; // Porta da biblioteca na praça da cidade
    link->y = 120.0f;
    link->dir = DIR_DOWN;
    link->is_moving = false;
    spawn_town_entities();
    hal_audio_play_bgm(BGM_HYRULE_TOWN);
    printf("[SCENE] Saindo da Biblioteca e retornando a Hyrule Town!\n");
}

static void spawn_lake_hylia_entities(void) {
    entity_clear_all();
    // 1. Inimigos nas águas abertas
    entity_spawn(ENTITY_ENEMY_OCTOROK, 240.0f, 160.0f);
    entity_spawn(ENTITY_ENEMY_OCTOROK, 380.0f, 240.0f);
    entity_spawn(ENTITY_ENEMY_TEKTITE, 80.0f, 80.0f);
    // 2. Toco Minish na ilhota central
    Entity* stump = entity_spawn(ENTITY_MINISH_STUMP, 288.0f, 224.0f);
    if (stump) stump->action = 0;
    // 3. Prefeito Hagen em frente a cabana do lago
    entity_spawn(ENTITY_NPC_MAYOR_HAGEN, 96.0f, 320.0f);
    printf("[LAKE HYLIA] Entidades do Lago Hylia spawnadas (Octoroks, Tektites, Toco da Ilha, Prefeito Hagen)!\n");
}

static void transition_to_lake_hylia(Player* link) {
    s_in_town = s_in_village = s_in_north_field = s_in_south_field = false;
    s_in_crenel_base = s_in_melari_mines = s_in_castor_wilds = s_in_mole_cave = false;
    s_in_wind_ruins = s_in_armos_interior = s_in_library = false;
    s_in_lake_hylia = true;
    link->x = 64.0f; // Cais oeste na margem
    link->y = 120.0f;
    link->dir = DIR_RIGHT;
    link->is_moving = false;
    spawn_lake_hylia_entities();
    hal_audio_play_bgm(BGM_HYRULE_OVERWORLD);
    printf("[SCENE] Entrando no Grande Lago Hylia (Lake Hylia)!\n");
}

static void transition_to_south_field_from_lake(Player* link) {
    s_in_lake_hylia = false;
    s_in_south_field = true;
    link->x = 360.0f;
    link->y = 200.0f;
    link->dir = DIR_LEFT;
    link->is_moving = false;
    spawn_south_field_entities();
    hal_audio_play_bgm(BGM_HYRULE_OVERWORLD);
    printf("[SCENE] Retornando a South Hyrule Field vindo do Lago Hylia!\n");
}

static void handle_fast_travel_transition(int new_map_id, float new_x, float new_y, Player* link, Tilemap* world_map) {
    if (dungeon_is_active() || dungeon_flames_is_active() || dungeon_fortress_is_active() || dungeon_droplets_is_active()) return;
    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = false;
    s_in_mole_cave = false;
    s_in_wind_ruins = false;
    s_in_armos_interior = false;
    s_in_library = false;
    s_in_lake_hylia = false;
    if (veil_clouds_is_active()) {
        veil_clouds_exit(&link->x, &link->y, &link->dir);
    }
    link->is_swimming = false;
    link->is_diving = false;
    link->is_minish = false;
    link->is_moving = false;
    link->x = new_x;
    link->y = new_y;

    if (new_map_id == 1) { // Hyrule Town
        s_in_town = true;
        spawn_town_entities();
        hal_audio_play_bgm(BGM_HYRULE_TOWN);
        hal_audio_play_sound(SOUND_TOWN_BELL, 1.0f, 1.0f);
    } else if (new_map_id == 0) { // Minish Woods
        spawn_overworld_entities(world_map);
        hal_audio_play_bgm(BGM_MINISH_WOODS);
    } else if (new_map_id == 4) { // Lake Hylia
        s_in_lake_hylia = true;
        spawn_lake_hylia_entities();
        hal_audio_play_bgm(BGM_HYRULE_OVERWORLD);
    } else if (new_map_id == 6) { // Crenel Base
        s_in_crenel_base = true;
        spawn_crenel_base_entities();
        hal_audio_play_bgm(BGM_MT_CRENEL);
    } else if (new_map_id == 10) { // Castor Wilds
        s_in_castor_wilds = true;
        spawn_castor_wilds_entities();
        hal_audio_play_bgm(BGM_CASTOR_WILDS);
    } else if (new_map_id == 12) { // Wind Ruins
        s_in_wind_ruins = true;
        spawn_wind_ruins_entities(armos_circuit_is_active());
        hal_audio_play_bgm(BGM_WIND_RUINS);
    } else if (new_map_id == 18) { // Veil Falls
        veil_clouds_enter_falls(&link->x, &link->y, &link->dir);
        hal_audio_play_bgm(BGM_CLOUD_TOPS);
    } else if (new_map_id == 19) { // Cloud Tops
        veil_clouds_enter_clouds(&link->x, &link->y, &link->dir);
        hal_audio_play_bgm(BGM_CLOUD_TOPS);
    }
    printf("[ZEFFA] Voo concluido com sucesso! Pouso no mapa %d em (%.1f, %.1f)!\n", new_map_id, new_x, new_y);
}

static void draw_rect(int rx, int ry, int rw, int rh, u32 color) {
    for (int y = ry; y < ry + rh; y++) {
        for (int x = rx; x < rx + rw; x++) {
            hal_video_put_pixel(x, y, color);
        }
    }
}

// Renderiza um coração clássico de vida de Zelda em pixel art 7x7
static void draw_heart(int hx, int hy) {
    u32 red   = 0xE62222FF;
    u32 dark  = 0x660808FF;
    u32 white = 0xFFFFFFFF;

    // Linha 0: dois topos
    hal_video_put_pixel(hx + 1, hy, red);
    hal_video_put_pixel(hx + 2, hy, red);
    hal_video_put_pixel(hx + 4, hy, red);
    hal_video_put_pixel(hx + 5, hy, red);

    // Linhas 1 a 3: corpo do coração com brilho
    for (int y = 1; y <= 3; y++) {
        for (int x = 0; x <= 6; x++) {
            hal_video_put_pixel(hx + x, hy + y, red);
        }
    }
    hal_video_put_pixel(hx + 1, hy + 1, white); // Brilho

    // Ponta inferior em V
    for (int x = 1; x <= 5; x++) hal_video_put_pixel(hx + x, hy + 4, red);
    for (int x = 2; x <= 4; x++) hal_video_put_pixel(hx + x, hy + 5, red);
    hal_video_put_pixel(hx + 3, hy + 6, red);
}

// Renderiza a moldura de um coração vazio no HUD (quando com dano)
static void draw_empty_heart(int hx, int hy) {
    u32 dark = 0x660808FF;
    u32 gray = 0x1E293BFF;

    hal_video_put_pixel(hx + 1, hy, dark);
    hal_video_put_pixel(hx + 2, hy, dark);
    hal_video_put_pixel(hx + 4, hy, dark);
    hal_video_put_pixel(hx + 5, hy, dark);

    for (int y = 1; y <= 3; y++) {
        for (int x = 0; x <= 6; x++) {
            bool border = (x == 0 || x == 6 || (y == 1 && x == 3));
            hal_video_put_pixel(hx + x, hy + y, border ? dark : gray);
        }
    }

    for (int x = 1; x <= 5; x++) hal_video_put_pixel(hx + x, hy + 4, (x == 1 || x == 5) ? dark : gray);
    for (int x = 2; x <= 4; x++) hal_video_put_pixel(hx + x, hy + 5, (x == 2 || x == 4) ? dark : gray);
    hal_video_put_pixel(hx + 3, hy + 6, dark);
}

// Renderiza efeitos de lâmina do Link: Golpe normal, Carga de Spin e Ataque Giratório 360°
static void draw_link_sword_effects(const Player* p, int px, int py) {
    u32 sword_steel = p->has_white_sword ? 0xFFFFFFFF : 0xCFE2F3FF;
    u32 sword_edge  = p->has_white_sword ? 0xBAE6FDFF : 0x94A3B8FF;
    u32 sword_gold  = 0xFFD700FF;
    u32 sword_ruby  = 0xEF4444FF;
    u32 cyan_glow   = 0x38BDF8FF;
    u32 cyan_white  = 0xBAE6FDFF;
    u32 white       = 0xFFFFFFFF;

    // 1. GOLPE DE ESPADA NORMAL (12 FRAMES)
    if (p->is_attacking) {
        if (!s_link_tex) {
            switch (p->dir) {
                case DIR_DOWN:
                    draw_rect(px + 6, py + 16, 4, 10, sword_steel);
                    draw_rect(px + 4, py + 16, 8, 2, sword_gold);
                    break;
                case DIR_UP:
                    draw_rect(px + 6, py - 8, 4, 10, sword_steel);
                    draw_rect(px + 4, py + 0, 8, 2, sword_gold);
                    break;
                case DIR_LEFT:
                    draw_rect(px - 10, py + 9, 10, 4, sword_steel);
                    draw_rect(px + 0,  py + 7, 2, 8, sword_gold);
                    break;
                case DIR_RIGHT:
                    draw_rect(px + 14, py + 9, 10, 4, sword_steel);
                    draw_rect(px + 13, py + 7, 2, 8, sword_gold);
                    break;
            }
        }
        if (p->has_white_sword) {
            switch (p->dir) {
                case DIR_DOWN:
                    hal_video_put_pixel(px + 7, py + 16, sword_ruby);
                    hal_video_put_pixel(px + 8, py + 16, sword_ruby);
                    hal_video_put_pixel(px + 3, py + 22, white);
                    hal_video_put_pixel(px + 12, py + 22, cyan_glow);
                    break;
                case DIR_UP:
                    hal_video_put_pixel(px + 7, py + 0, sword_ruby);
                    hal_video_put_pixel(px + 8, py + 0, sword_ruby);
                    hal_video_put_pixel(px + 3, py - 4, white);
                    hal_video_put_pixel(px + 12, py - 4, cyan_glow);
                    break;
                case DIR_LEFT:
                    hal_video_put_pixel(px + 0, py + 10, sword_ruby);
                    hal_video_put_pixel(px + 0, py + 11, sword_ruby);
                    hal_video_put_pixel(px - 6, py + 5, white);
                    hal_video_put_pixel(px - 6, py + 15, cyan_glow);
                    break;
                case DIR_RIGHT:
                    hal_video_put_pixel(px + 13, py + 10, sword_ruby);
                    hal_video_put_pixel(px + 13, py + 11, sword_ruby);
                    hal_video_put_pixel(px + 20, py + 5, white);
                    hal_video_put_pixel(px + 20, py + 15, cyan_glow);
                    break;
            }
        }
    }

    // 2. CARREGANDO O ATAQUE GIRATÓRIO (SPIN CHARGE)
    else if (p->is_charging_spin) {
        u32 blade_col = p->spin_ready ? cyan_white : sword_steel;
        u32 tip_spark = p->spin_ready ? 0x00FFFFFF : 0xFACC15FF;

        int tip_x = px + 8;
        int tip_y = py + 8;

        switch (p->dir) {
            case DIR_DOWN:
                draw_rect(px + 6, py + 15, 4, 9, blade_col);
                draw_rect(px + 4, py + 15, 8, 2, sword_gold);
                tip_x = px + 8; tip_y = py + 24;
                break;
            case DIR_UP:
                draw_rect(px + 6, py - 7, 4, 9, blade_col);
                draw_rect(px + 4, py + 0, 8, 2, sword_gold);
                tip_x = px + 8; tip_y = py - 7;
                break;
            case DIR_LEFT:
                draw_rect(px - 9, py + 9, 9, 4, blade_col);
                draw_rect(px + 0, py + 7, 2, 8, sword_gold);
                tip_x = px - 9; tip_y = py + 11;
                break;
            case DIR_RIGHT:
                draw_rect(px + 14, py + 9, 9, 4, blade_col);
                draw_rect(px + 13, py + 7, 2, 8, sword_gold);
                tip_x = px + 23; tip_y = py + 11;
                break;
        }

        // Faíscas cintilantes na ponta da lâmina acumulando energia
        int spark_ox = ((p->spin_charge_timer * 3) % 5) - 2;
        int spark_oy = ((p->spin_charge_timer * 5) % 5) - 2;
        hal_video_put_pixel(tip_x + spark_ox, tip_y + spark_oy, tip_spark);
        hal_video_put_pixel(tip_x - spark_ox, tip_y + spark_oy, white);

        // Anel de pulso celestial quando a carga máxima do Spin Attack está pronta!
        if (p->spin_ready) {
            int cx = px + 8;
            int cy = py + 8;
            int pulse_r = 13 + (p->anim_timer % 7);
            for (int a = 0; a < 8; a++) {
                float ang = (float)a * (3.14159265f / 4.0f) + (float)p->anim_timer * 0.12f;
                int ax = cx + (int)(cosf(ang) * (float)pulse_r);
                int ay = cy + (int)(sinf(ang) * (float)pulse_r);
                hal_video_put_pixel(ax, ay, cyan_glow);
            }
        }
    }

    // 3. EXECUÇÃO DO ATAQUE GIRATÓRIO (360° SPIN ATTACK)
    else if (p->is_spinning) {
        int cx = px + 8;
        int cy = py + 8;
        float progress = (float)(16 - p->spin_timer) / 16.0f; // 0.0 a 1.0

        // Lâmina giratória em alta velocidade
        float sword_ang = progress * 2.0f * 3.14159265f;
        int sw_x = cx + (int)(cosf(sword_ang) * 18.0f);
        int sw_y = cy + (int)(sinf(sword_ang) * 18.0f);
        draw_rect(sw_x - 1, sw_y - 1, 3, 3, cyan_white);

        // Rastro circular sweeping crescent arc (r = 21..25px)
        int num_trail_points = 36;
        for (int i = 0; i < num_trail_points; i++) {
            float ang = (float)i * (2.0f * 3.14159265f / (float)num_trail_points);
            int r_outer = 25;
            int r_inner = 21;

            int ox = cx + (int)(cosf(ang) * (float)r_outer);
            int oy = cy + (int)(sinf(ang) * (float)r_outer);
            int ix = cx + (int)(cosf(ang) * (float)r_inner);
            int iy = cy + (int)(sinf(ang) * (float)r_inner);

            u32 trail_col = ((i % 3) == 0) ? white : (((i % 2) == 0) ? cyan_white : cyan_glow);
            hal_video_put_pixel(ox, oy, trail_col);
            hal_video_put_pixel(ix, iy, cyan_glow);
        }

        // 4 faíscas dinâmicas ao redor da circunferência
        for (int s = 0; s < 4; s++) {
            float s_ang = (float)s * (3.14159265f / 2.0f) + sword_ang;
            int sx_spark = cx + (int)(cosf(s_ang) * 27.0f);
            int sy_spark = cy + (int)(sinf(s_ang) * 27.0f);
            draw_rect(sx_spark - 1, sy_spark - 1, 2, 2, white);
        }
    }
}

static void draw_link_transformation_effects(const Player* p, int cx, int cy) {
    int timer = p->transform_timer;

    // 1. Anéis concêntricos pulsantes de energia mágica
    int r1 = ((50 - timer) * 2) % 22 + 4;
    int r2 = ((50 - timer) * 2 + 11) % 22 + 4;
    u32 col_ring1 = (timer % 6 < 3) ? 0x00FFCCFF : 0x70FF80FF;
    u32 col_ring2 = (timer % 6 < 3) ? 0xFFD700FF : 0xFFFFFFFF;

    for (int a = 0; a < 24; a++) {
        float ang = (float)a * (2.0f * 3.14159265f / 24.0f);
        int rx1 = cx + (int)(cosf(ang) * (float)r1);
        int ry1 = cy + (int)(sinf(ang) * ((float)r1 * 0.65f));
        int rx2 = cx + (int)(cosf(-ang) * (float)r2);
        int ry2 = cy + (int)(sinf(-ang) * ((float)r2 * 0.65f));
        hal_video_put_pixel(rx1, ry1, col_ring1);
        hal_video_put_pixel(rx2, ry2, col_ring2);
    }

    // 2. Partículas místicas ascendentes em espiral
    for (int sp = 0; sp < 6; sp++) {
        float ang = (float)sp * (3.14159265f / 3.0f) + (float)timer * 0.25f;
        float dist = 8.0f + 4.0f * sinf((float)timer * 0.2f + sp);
        int sx_p = cx + (int)(cosf(ang) * dist);
        int sy_p = cy + (int)(sinf(ang) * (dist * 0.65f)) - ((50 - timer) % 14);
        hal_video_put_pixel(sx_p, sy_p, 0xFFFFFFFF);
        hal_video_put_pixel(sx_p, sy_p - 1, 0x00FFCCFF);
    }

    // 3. Flash no clímax da transformação (frame 25)
    if (timer >= 22 && timer <= 28) {
        int flash_r = (28 - timer) * 2 + 6;
        for (int fa = 0; fa < 32; fa++) {
            float ang = (float)fa * (2.0f * 3.14159265f / 32.0f);
            int fx = cx + (int)(cosf(ang) * (float)flash_r);
            int fy = cy + (int)(sinf(ang) * ((float)flash_r * 0.70f));
            hal_video_put_pixel(fx, fy, 0xFFFFFFFF);
            hal_video_put_pixel(fx + 1, fy, 0x00FFAAFF);
        }
    }
}

static void draw_rect_blend(int rx, int ry, int rw, int rh, u32 color, float alpha) {
    const HalVideoContext* ctx = hal_video_get_context();
    if (!ctx || !ctx->framebuffer || alpha <= 0.0f) return;
    if (alpha > 1.0f) alpha = 1.0f;
    u8 cr = (color >> 24) & 0xFF;
    u8 cg = (color >> 16) & 0xFF;
    u8 cb = (color >> 8) & 0xFF;

    for (int y = ry; y < ry + rh; y++) {
        if (y < 0 || y >= ctx->render_height) continue;
        for (int x = rx; x < rx + rw; x++) {
            if (x < 0 || x >= ctx->render_width) continue;
            int idx = y * ctx->render_width + x;
            u32 bg = ctx->framebuffer[idx];
            u8 br = (bg >> 24) & 0xFF;
            u8 bg_g = (bg >> 16) & 0xFF;
            u8 bb = (bg >> 8) & 0xFF;

            u8 nr = (u8)(cr * alpha + br * (1.0f - alpha));
            u8 ng = (u8)(cg * alpha + bg_g * (1.0f - alpha));
            u8 nb = (u8)(cb * alpha + bb * (1.0f - alpha));
            ctx->framebuffer[idx] = (nr << 24) | (ng << 16) | (nb << 8) | 0xFF;
        }
    }
}

// Renderiza efeitos de água de natação e mergulho com as Nadadeiras de Zora
static void draw_link_swimming_effects(const Player* p, int px, int py) {
    if (!p->is_swimming) return;

    int cx = px + 8;
    int cy = py + 12;

    // 1. Ondulações concêntricas de natação na água
    int phase = (p->water_ripple_timer / 4) % 8;
    int r_x1 = 7 + (phase % 4);
    int r_y1 = 3 + (phase % 4) / 2;
    int r_x2 = 11 + ((phase + 2) % 4);
    int r_y2 = 5 + ((phase + 2) % 4) / 2;

    u32 ripple_col1 = 0xBAE6FDFF; // Espuma brilhante (Cyan White)
    u32 ripple_col2 = 0x38BDF8AA; // Ondulação d'água (Sky Blue)

    for (int a = 0; a < 20; a++) {
        float ang = (float)a * (2.0f * 3.14159265f / 20.0f);
        int ox = cx + (int)(cosf(ang) * (float)r_x1);
        int oy = cy + (int)(sinf(ang) * (float)r_y1);
        hal_video_put_pixel(ox, oy, ripple_col1);
    }
    for (int a = 0; a < 24; a++) {
        float ang = (float)a * (2.0f * 3.14159265f / 24.0f);
        int ox = cx + (int)(cosf(ang) * (float)r_x2);
        int oy = cy + (int)(sinf(ang) * (float)r_y2);
        hal_video_put_pixel(ox, oy, ripple_col2);
    }

    // 2. Respingos d'água dinâmicos enquanto se desloca
    if (p->is_moving) {
        int splash_phase = (p->anim_timer % 6);
        u32 splash_col = 0xE0F2FEFF;
        hal_video_put_pixel(cx - 10, cy - 2 + splash_phase, splash_col);
        hal_video_put_pixel(cx + 10, cy - 2 + splash_phase, splash_col);
        if (splash_phase < 3) {
            hal_video_put_pixel(cx - 12, cy - 4, 0xFFFFFFFF);
            hal_video_put_pixel(cx + 12, cy - 4, 0xFFFFFFFF);
        }
    }

    // 3. Submersão corporal
    if (!p->is_diving) {
        // Cintura e pernas sob a superfície da água translúcida
        draw_rect_blend(px + 2, py + 12, 12, 6, 0x0284C7FF, 0.45f);
    } else {
        // Mergulho profundo: corpo inteiro submerso sob tom azul marinho e bolhas de ar
        draw_rect_blend(px - 1, py + 2, 18, 16, 0x0369A1FF, 0.70f);

        // Bolhas de ar subindo à superfície
        int b1_y = cy - (p->dive_timer % 14);
        int b1_x = cx + ((p->dive_timer % 4) - 2);
        int b2_y = cy - ((p->dive_timer + 7) % 14);
        int b2_x = cx - ((p->dive_timer % 3) - 1);

        hal_video_put_pixel(b1_x, b1_y, 0xFFFFFFFF);
        hal_video_put_pixel(b1_x + 1, b1_y, 0x7DD3FCFF);
        hal_video_put_pixel(b2_x, b2_y, 0xFFFFFFFF);
        hal_video_put_pixel(b2_x - 1, b2_y, 0x7DD3FCFF);

        for (int a = 0; a < 12; a++) {
            float ang = (float)a * (2.0f * 3.14159265f / 12.0f);
            int ox = cx + (int)(cosf(ang) * 5.0f);
            int oy = cy + (int)(sinf(ang) * 2.5f);
            hal_video_put_pixel(ox, oy, 0x38BDF888);
        }
    }
}

static void draw_link_climbing_effects(const Player* p, int px, int py) {
    if (!p->is_climbing) return;
    int step = ((p->climb_anim_timer / 8) % 2 == 1) ? 1 : 0;
    u32 c_ring = 0xFDE047FF; // Brilho dourado do Grip Ring
    u32 c_ruby = 0xEF4444FF; // Rubi central do Grip Ring

    // Mãos agarradas no paredão rochoso com o Grip Ring reluzindo
    if (step == 0) {
        draw_rect(px + 1, py - 2, 3, 3, c_ring);
        hal_video_put_pixel(px + 2, py - 1, c_ruby);
        draw_rect(px + 12, py + 1, 3, 3, c_ring);
        hal_video_put_pixel(px + 13, py + 2, c_ruby);
    } else {
        draw_rect(px + 1, py + 1, 3, 3, c_ring);
        hal_video_put_pixel(px + 2, py + 2, c_ruby);
        draw_rect(px + 12, py - 2, 3, 3, c_ring);
        hal_video_put_pixel(px + 13, py - 1, c_ruby);
    }
}

static void draw_link_mud_effects(const Player* p, int px, int py) {
    if (p->mud_sink_timer <= 0) return;

    int sink_pixels = p->mud_sink_timer / 12;
    if (sink_pixels > 5) sink_pixels = 5;

    u32 c_mud_dark   = 0x1C0E07FF;
    u32 c_mud_base   = 0x2A1810FF;
    u32 c_mud_ripple = 0x5C3E25FF;

    // Lama cobrindo as botas e pernas do herói
    draw_rect(px + 2, py + 16 - sink_pixels, 12, sink_pixels + 2, c_mud_base);
    draw_rect(px + 1, py + 17 - sink_pixels, 14, sink_pixels + 1, c_mud_dark);

    // Ondulações de lodo nos lados
    hal_video_put_pixel(px, py + 16 - sink_pixels, c_mud_ripple);
    hal_video_put_pixel(px + 15, py + 16 - sink_pixels, c_mud_ripple);
    if ((p->mud_sink_timer / 6) % 2 == 0) {
        hal_video_put_pixel(px + 4, py + 15 - sink_pixels, c_mud_ripple);
        hal_video_put_pixel(px + 11, py + 15 - sink_pixels, c_mud_ripple);
    }
}

static void draw_link_digging_effects(const Player* p, int px, int py) {
    if (!subweapon_is_digging()) return;

    u32 c_mitt_leather = 0x78350FFF;
    u32 c_claw_steel   = 0xE2E8F0FF;
    u32 c_claw_tip     = 0xFFFFFFFF;

    // Garras afiadas cortando a terra na direção do herói
    switch (p->dir) {
        case DIR_DOWN:
            draw_rect(px + 2, py + 12, 4, 3, c_mitt_leather);
            draw_rect(px + 10, py + 12, 4, 3, c_mitt_leather);
            draw_rect(px + 3, py + 15, 1, 3, c_claw_steel);
            draw_rect(px + 5, py + 15, 1, 3, c_claw_steel);
            draw_rect(px + 10, py + 15, 1, 3, c_claw_steel);
            draw_rect(px + 12, py + 15, 1, 3, c_claw_steel);
            hal_video_put_pixel(px + 3, py + 18, c_claw_tip);
            hal_video_put_pixel(px + 5, py + 18, c_claw_tip);
            hal_video_put_pixel(px + 10, py + 18, c_claw_tip);
            hal_video_put_pixel(px + 12, py + 18, c_claw_tip);
            break;
        case DIR_UP:
            draw_rect(px + 2, py + 1, 4, 3, c_mitt_leather);
            draw_rect(px + 10, py + 1, 4, 3, c_mitt_leather);
            draw_rect(px + 3, py - 2, 1, 3, c_claw_steel);
            draw_rect(px + 5, py - 2, 1, 3, c_claw_steel);
            draw_rect(px + 10, py - 2, 1, 3, c_claw_steel);
            draw_rect(px + 12, py - 2, 1, 3, c_claw_steel);
            hal_video_put_pixel(px + 3, py - 3, c_claw_tip);
            hal_video_put_pixel(px + 5, py - 3, c_claw_tip);
            hal_video_put_pixel(px + 10, py - 3, c_claw_tip);
            hal_video_put_pixel(px + 12, py - 3, c_claw_tip);
            break;
        case DIR_LEFT:
            draw_rect(px - 1, py + 8, 3, 4, c_mitt_leather);
            draw_rect(px - 4, py + 9, 3, 1, c_claw_steel);
            draw_rect(px - 4, py + 11, 3, 1, c_claw_steel);
            hal_video_put_pixel(px - 5, py + 9, c_claw_tip);
            hal_video_put_pixel(px - 5, py + 11, c_claw_tip);
            break;
        case DIR_RIGHT:
            draw_rect(px + 14, py + 8, 3, 4, c_mitt_leather);
            draw_rect(px + 17, py + 9, 3, 1, c_claw_steel);
            draw_rect(px + 17, py + 11, 3, 1, c_claw_steel);
            hal_video_put_pixel(px + 20, py + 9, c_claw_tip);
            hal_video_put_pixel(px + 20, py + 11, c_claw_tip);
            break;
    }
}

// Renderiza o Link no estilo clássico de Minish Cap na posição da Câmera
static void draw_link(const Player* p, const Camera* cam) {
    if (fast_travel_is_link_airborne()) {
        return;
    }

    // Efeito clássico de piscar ao receber dano (flicker)
    if (p->invuln_timer > 0 && ((p->invuln_timer / 3) % 2 == 0)) {
        return;
    }

    int px, py;
    map_world_to_screen(cam, p->x, p->y, &px, &py);

    // Sombra dinâmica no solo durante o salto vertical e planeio com a Capa de Roc
    if (p->z > 0.0f) {
        rocs_cape_render_shadow(cam, p->x, p->y, p->z);
    }
    int render_offset_y = (int)p->z;
    py -= render_offset_y;

    int draw_x = px - 8;
    int draw_y = py - 9;

    // Efeitos visuais do portal mágico de transformação Minish
    if (p->is_transforming) {
        draw_link_transformation_effects(p, px + 8, py + 8);
    }

    // ------------------------------------------------------------------------
    // RENDERIZADOR AUTÊNTICO COM SPRITES EXTRAÍDOS DA ROM
    // ------------------------------------------------------------------------
    if (s_link_tex && s_link_tex->pixels) {
        // Se estiver no tamanho Minish (ou durante a transformação na fase Minish)
        bool render_as_minish = p->is_minish;
        if (p->is_transforming) {
            if (p->transform_to_minish) {
                render_as_minish = (p->transform_timer <= 25);
            } else {
                render_as_minish = (p->transform_timer > 25);
            }
        }

        if (render_as_minish) {
            int m_col = 0;
            bool m_flip = false;
            int step = (p->anim_frame / 5) % 2;

            if (!p->is_moving) {
                if (p->dir == DIR_DOWN)       m_col = 0;
                else if (p->dir == DIR_RIGHT) m_col = 1;
                else if (p->dir == DIR_UP)    m_col = 2;
                else if (p->dir == DIR_LEFT)  m_col = 3;
            } else {
                if (p->dir == DIR_DOWN) {
                    m_col = 4 + step;
                } else if (p->dir == DIR_RIGHT) {
                    m_col = 6 + step;
                    m_flip = false;
                } else if (p->dir == DIR_UP) {
                    m_col = 8 + step;
                } else if (p->dir == DIR_LEFT) {
                    m_col = 6 + step;
                    m_flip = true;
                }
            }

            // 1. Desenha o corpo diminuto do Link Minish (Row 4)
            texture_draw_ex(s_link_tex, m_col * 32, 4 * 32, 32, 32, draw_x, draw_y, m_flip);

            // 2. Balão Indicador Canônico Flutuante (Speech Bubble Beacon, Row 5)
            int b_col = 0;
            if (p->dir == DIR_DOWN)       b_col = 0;
            else if (p->dir == DIR_RIGHT) b_col = 1;
            else if (p->dir == DIR_UP)    b_col = 2;
            else if (p->dir == DIR_LEFT)  b_col = 3;

            int beacon_bob = (int)(sinf((float)p->anim_timer * 0.15f) * 2.0f);
            texture_draw_ex(s_link_tex, b_col * 32, 5 * 32, 32, 32, draw_x, draw_y - 20 + beacon_bob, false);

            if (p->is_attacking) {
                draw_link_sword_effects(p, px, py);
            }
            draw_link_swimming_effects(p, px, py);
            draw_link_climbing_effects(p, px, py);
            return;
        }

        // Link Tamanho Humano Normal - Máquina de Estados de Animação Canônica GBA
        int row = 0;
        int col = 0;
        bool flip_h = false;

        // 1. Invulnerability / Hurt (Reação autêntica de dano e recuo - Row 9)
        if (p->invuln_timer > 0 && (fabsf(p->knock_x) > 0.05f || fabsf(p->knock_y) > 0.05f || !p->is_moving)) {
            row = 9;
            if (p->dir == DIR_DOWN) col = 0;
            else if (p->dir == DIR_UP) col = 2;
            else {
                col = 1;
                flip_h = (p->dir == DIR_LEFT);
            }
        }
        // 2. Somersault Roll (Rolamento acrobático fluido de 8 quadros - Row 7)
        else if (p->is_rolling) {
            row = 7;
            int roll_step = (16 - p->roll_timer) / 2;
            if (roll_step < 0) roll_step = 0;
            if (roll_step > 7) roll_step = 7;
            col = roll_step;
            if (p->dir == DIR_LEFT) flip_h = true;
        }
        // 3. Spin Attack (Ataque Giratório 360° em alta rotação - Row 7 e Row 3)
        else if (p->is_spinning) {
            int spin_rot = (16 - p->spin_timer) % 4;
            if (spin_rot == 0) { row = 7; col = 8; }
            else if (spin_rot == 1) { row = 7; col = 9; flip_h = false; }
            else if (spin_rot == 2) { row = 3; col = 0; }
            else { row = 7; col = 9; flip_h = true; }
        }
        // 4. Spin Charge (Postura tensa com lâmina cintilando pronta para o giro - Row 6, Col 9)
        else if (p->is_charging_spin) {
            row = 6;
            col = 9;
            if (p->dir == DIR_LEFT) flip_h = true;
        }
        // 5. Sword Attack (Golpe com lâmina em 4 direções canônicas - Row 6)
        else if (p->is_attacking) {
            row = 6;
            int slash_step = (12 - p->attack_timer) / 4;
            if (slash_step < 0) slash_step = 0;
            if (slash_step > 2) slash_step = 2;
            if (p->dir == DIR_DOWN) {
                col = 0 + slash_step;
            } else if (p->dir == DIR_RIGHT) {
                col = 3 + slash_step;
                flip_h = false;
            } else if (p->dir == DIR_LEFT) {
                col = 3 + slash_step;
                flip_h = true;
            } else if (p->dir == DIR_UP) {
                col = 6 + slash_step;
            }
        }
        // 6. Airborne Jump (Salto vertical e planeio com Capa de Roc / Pacci - Row 8)
        else if (p->is_jumping) {
            row = 8;
            if (p->vz > 1.0f) col = 0;
            else if (p->vz < -1.0f) col = 2;
            else col = 1;
            if (p->dir == DIR_LEFT) flip_h = true;
        }
        // 7. Swimming & Diving (Braçadas e mergulho sob a água - Row 9)
        else if (p->is_swimming) {
            row = 9;
            if (p->is_diving) {
                col = 9;
            } else if (p->dir == DIR_RIGHT) {
                col = 7 + (p->anim_frame % 2);
                flip_h = false;
            } else if (p->dir == DIR_LEFT) {
                col = 7 + (p->anim_frame % 2);
                flip_h = true;
            } else {
                col = 5 + (p->anim_frame % 2);
            }
        }
        // 8. Walk cycle (Ciclo de caminhada canônico fluido de 10 quadros - Rows 1..3)
        else if (p->is_moving) {
            col = p->anim_frame % 10;
            if (p->dir == DIR_DOWN) {
                row = 1;
            } else if (p->dir == DIR_RIGHT) {
                row = 2;
            } else if (p->dir == DIR_UP) {
                row = 3;
            } else if (p->dir == DIR_LEFT) {
                row = 2;
                flip_h = true;
            }
        }
        // 9. Idle (Repouso - Row 0)
        else {
            row = 0;
            if (p->dir == DIR_DOWN)       col = 0;
            else if (p->dir == DIR_RIGHT) col = 1;
            else if (p->dir == DIR_UP)    col = 2;
            else if (p->dir == DIR_LEFT)  col = 3;
        }

        int src_x = col * 32;
        int src_y = row * 32;

        texture_draw_ex(s_link_tex, src_x, src_y, 32, 32, draw_x, draw_y, flip_h);
        if (!s_link_tex || p->has_white_sword || p->is_charging_spin || p->is_spinning) {
            draw_link_sword_effects(p, px, py);
        }
        draw_link_swimming_effects(p, px, py);
        draw_link_climbing_effects(p, px, py);
        draw_link_mud_effects(p, px, py);
        draw_link_digging_effects(p, px, py);
        return;
    }

    // ------------------------------------------------------------------------
    // FALLBACK PROCEDURAL (utilizado caso os assets não estejam extraídos)
    // ------------------------------------------------------------------------
    if (p->is_minish) {
        draw_rect(px + 6, py + 11, 4, 3, 0x228B22FF); // Túnica
        draw_rect(px + 6, py + 8,  4, 3, 0x32CD32FF); // Gorro
        hal_video_put_pixel(px + 8, py + 7, 0xFFFFFFFF); // Pom-pom
        hal_video_put_pixel(px + 7, py + 9, 0x111111FF); // Olho
        int b_y = py - 10 + (int)(sinf((float)p->anim_timer * 0.15f) * 2.0f);
        draw_rect(px + 4, b_y, 8, 8, 0xFFFFFFFF);
        draw_rect(px + 5, b_y + 1, 6, 6, 0x38BDF8FF);
        hal_video_put_pixel(px + 8, b_y + 8, 0xFFFFFFFF);
        return;
    }
    u32 tunic_green = 0x228B22FF; // Verde Floresta
    u32 hat_bright   = 0x32CD32FF; // Verde Gorro
    u32 skin_tone    = 0xF5CBA7FF; // Tom de Pele
    u32 belt_brown   = 0x8B4513FF; // Cinto Marrom
    u32 boot_color   = 0xD2691EFF; // Botas
    u32 sword_gold   = 0xFFD700FF; // Empunhadura

    int step_offset = (p->is_moving && (p->anim_frame == 1)) ? 1 : 0;

    // 1. Gorro e Cabelo (topo da cabeça)
    draw_rect(px + 3, py + 0, 10, 3, hat_bright);
    draw_rect(px + 2, py + 3, 12, 3, hat_bright);

    // Ponta do Gorro Minish pendendo conforme a direção
    if (p->dir == DIR_LEFT)  draw_rect(px + 12, py + 2, 3, 4, hat_bright);
    if (p->dir == DIR_RIGHT) draw_rect(px + 1,  py + 2, 3, 4, hat_bright);

    // 2. Rosto
    draw_rect(px + 4, py + 6, 8, 4, skin_tone);
    if (p->dir == DIR_DOWN) {
        // Olhos olhando para frente
        hal_video_put_pixel(px + 5, py + 7, 0x111111FF);
        hal_video_put_pixel(px + 9, py + 7, 0x111111FF);
    } else if (p->dir == DIR_LEFT) {
        hal_video_put_pixel(px + 4, py + 7, 0x111111FF);
    } else if (p->dir == DIR_RIGHT) {
        hal_video_put_pixel(px + 10, py + 7, 0x111111FF);
    }

    // 3. Túnica Verde
    draw_rect(px + 3, py + 10, 10, 5, tunic_green);

    // 4. Cinto
    draw_rect(px + 4, py + 13, 8, 2, belt_brown);
    hal_video_put_pixel(px + 7, py + 13, sword_gold); // Fivela dourada

    // 5. Pernas e Botas animadas pelo ciclo de caminhada
    if (step_offset == 0) {
        draw_rect(px + 4, py + 15, 3, 3, boot_color);
        draw_rect(px + 9, py + 15, 3, 3, boot_color);
    } else {
        draw_rect(px + 3, py + 14, 3, 4, boot_color);
        draw_rect(px + 10, py + 15, 3, 3, boot_color);
    }

    // 6. Efeitos de espada, Spin Attack e Escalada
    draw_link_sword_effects(p, px, py);
    draw_link_swimming_effects(p, px, py);
    draw_link_climbing_effects(p, px, py);
    draw_link_mud_effects(p, px, py);
    draw_link_digging_effects(p, px, py);
    if (p->has_rocs_cape || subweapon_get_current() == ITEM_ROCS_CAPE) {
        rocs_cape_render(cam, p->x, p->y, p->dir, p->z);
    }
}

static inline bool is_world_solid_for_player(const Tilemap* map, float wx, float wy, bool is_minish, bool has_flippers, bool has_grip_ring, float z) {
    if (dungeon_is_active()) {
        return dungeon_is_solid(wx, wy);
    }
    if (dungeon_flames_is_active()) {
        return dungeon_flames_is_solid(wx, wy);
    }
    if (sanctuary_is_active()) {
        return sanctuary_is_solid(wx, wy);
    }
    if (royal_valley_is_active()) {
        return royal_valley_is_solid(wx, wy);
    }
    // Mecânica Salto do Cajado de Pacci: no ar (z > 4.0f) salta por cima de buracos e escarpas
    if (z > 4.0f) {
        return false;
    }
    // Mecânica Grip Ring: paredes escaláveis e vinhas não bloqueiam se tiver o Anel de Escalada!
    if (has_grip_ring && map_is_climbable(map, wx, wy)) {
        return false;
    }
    // Mecânica Zora's Flippers: com as nadadeiras, a água NÃO bloqueia o movimento (permite nadar!)
    if (has_flippers && map_is_water(map, wx, wy)) {
        return false;
    }
    // Mecânica Minish: o interior do Tronco Oco (Hollow Log em Minish Woods) é transitável apenas quando Minish!
    if (is_minish && map && map->is_authentic) {
        int tx = (int)(wx / TILE_SIZE);
        int ty = (int)(wy / TILE_SIZE);
        // Coluna 21, linhas 45 a 48 (Tronco Oco vertical entre Y=720 e Y=768)
        if (tx == 21 && ty >= 45 && ty <= 48) {
            return false;
        }
    }
    return map_is_solid(map, wx, wy);
}

static void apply_save_data(const SaveData* save, Player* link_ptr) {
    if (!save || !link_ptr) return;
    link_ptr->x = save->player_x;
    link_ptr->y = save->player_y;
    link_ptr->dir = (Direction)save->player_dir;
    link_ptr->hearts = save->hearts;
    link_ptr->max_hearts = save->max_hearts;
    link_ptr->rupees = save->rupees;
    link_ptr->has_flippers = save->has_flippers;
    link_ptr->has_spin_attack = save->has_spin_attack;
    link_ptr->is_minish = save->is_minish;
    link_ptr->has_grip_ring = save->has_grip_ring;
    link_ptr->has_cane_of_pacci = save->has_cane_of_pacci;
    link_ptr->has_white_sword = save->has_white_sword;
    link_ptr->has_two_elements = save->has_two_elements;
    link_ptr->has_three_elements = save->has_three_elements;
    link_ptr->has_four_sword = save->has_four_sword;
    link_ptr->has_bow = save->has_bow;
    if (link_ptr->has_four_sword) {
        sanctuary_set_four_elements(true);
        inventory_set_four_sword(true);
    } else if (link_ptr->has_three_elements) {
        sanctuary_set_three_elements(true);
    } else if (link_ptr->has_two_elements) {
        sanctuary_set_two_elements(true);
    }
    if (save->secret_exit_unlocked) {
        ElementalSanctuaryState* sanc_st = sanctuary_get_state();
        if (sanc_st) sanc_st->secret_exit_unlocked = true;
    }
    if (link_ptr->has_grip_ring) {
        inventory_unlock_item(INV_ITEM_GRIP_RING);
    }
    if (link_ptr->has_cane_of_pacci) {
        inventory_unlock_item(INV_ITEM_CANE_OF_PACCI);
    }
    if (link_ptr->has_white_sword) {
        inventory_set_white_sword(true);
    }
    if (link_ptr->has_bow) {
        inventory_unlock_item(INV_ITEM_BOW);
    }
    link_ptr->has_mole_mitts = save->has_mole_mitts;
    if (link_ptr->has_mole_mitts) {
        inventory_unlock_item(INV_ITEM_MOLE_MITTS);
    }
    link_ptr->has_armos_activated = save->has_armos_activated;
    if (link_ptr->has_armos_activated) {
        armos_circuit_set_active(true);
    }
    link_ptr->has_ocarina = save->has_ocarina;
    if (link_ptr->has_ocarina) {
        inventory_unlock_item(INV_ITEM_OCARINA);
    }
    if (save->unlocked_wind_crests > 0) {
        fast_travel_set_unlocked_mask(save->unlocked_wind_crests);
    }
    if (save->bomb_count > 0) {
        subweapon_add_bombs(save->bomb_count - subweapon_get_bomb_count());
    }
    if (save->slot_a > 0) inventory_set_slot_a((InventoryItem)save->slot_a);
    if (save->slot_b > 0) inventory_set_slot_b((InventoryItem)save->slot_b);

    s_in_town = false;
    s_in_village = false;
    s_in_south_field = false;
    s_in_north_field = false;
    s_in_crenel_base = false;
    s_in_melari_mines = false;
    s_in_castor_wilds = false;
    s_in_mole_cave = false;
    s_in_wind_ruins = false;
    s_in_armos_interior = false;
    s_in_library = false;
    s_in_lake_hylia = false;

    if (save->current_map == 1) {
        s_in_town = true;
    } else if (save->current_map == 2) {
        s_in_village = true;
    } else if (save->current_map == 4) {
        s_in_south_field = true;
    } else if (save->current_map == 5) {
        s_in_north_field = true;
    } else if (save->current_map == 6) {
        s_in_crenel_base = true;
    } else if (save->current_map == 7) {
        s_in_melari_mines = true;
    } else if (save->current_map == 8) {
        dungeon_flames_enter(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 9) {
        sanctuary_enter(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 10) {
        s_in_castor_wilds = true;
    } else if (save->current_map == 11) {
        s_in_mole_cave = true;
    } else if (save->current_map == 12) {
        s_in_wind_ruins = true;
    } else if (save->current_map == 13) {
        s_in_armos_interior = true;
    } else if (save->current_map == 14) {
        dungeon_fortress_enter(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 15) {
        s_in_library = true;
    } else if (save->current_map == 16) {
        s_in_lake_hylia = true;
    } else if (save->current_map == 17) {
        dungeon_droplets_enter(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 18) {
        veil_clouds_enter_falls(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 19) {
        veil_clouds_enter_clouds(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 20) {
        royal_valley_enter_entrance(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 21) {
        royal_valley_enter_maze(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 22) {
        royal_valley_enter_dampe(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 23) {
        royal_valley_enter_graveyard(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 24) {
        royal_valley_enter_crypt(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map >= 25 && save->current_map <= 30) {
        dark_castle_enter(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
        dark_castle_set_room((DarkCastleRoomId)(save->current_map - 25), &link_ptr->x, &link_ptr->y, &link_ptr->dir);
    } else if (save->current_map == 31) {
        vaati_boss_start(&link_ptr->x, &link_ptr->y, &link_ptr->dir);
    }
    link_ptr->vaati_defeated = save->vaati_defeated;
    link_ptr->has_sanctum_key = save->has_sanctum_key;
    dark_castle_restore_state(save->dark_castle_cleared, save->dark_castle_bells_silenced, save->has_sanctum_key);
    link_ptr->has_royal_kinstone = save->has_royal_kinstone;
    royal_valley_restore_state(save->graveyard_gate_unlocked, save->dampe_met, save->tomb_pushed, save->king_gustaf_met, save->has_royal_kinstone);
    link_ptr->has_lantern = save->has_flame_lantern;
    lantern_set_lit(save->lantern_lit);
    if (link_ptr->has_lantern) inventory_unlock_item(INV_ITEM_LANTERN);
    link_ptr->has_water_element = save->has_water_element;
    link_ptr->has_wind_element = save->has_wind_element;
    link_ptr->dungeon_palace_cleared = save->dungeon_palace_cleared;
    library_restore_save(save->library_books_mask, save->librari_met, save->lake_temple_unlocked);
    link_ptr->has_veil_falls_unlocked = save->has_veil_falls_unlocked;
    link_ptr->golden_kinstones_fused = save->golden_kinstones_fused;
    link_ptr->cloud_tornado_active = save->cloud_tornado_active;
    veil_clouds_set_golden_kinstones(save->golden_kinstones_fused);
    link_ptr->has_rocs_cape = save->has_rocs_cape;
    if (link_ptr->has_rocs_cape) {
        inventory_unlock_item(INV_ITEM_ROCS_CAPE);
    }
    figurine_gallery_restore(save->figurines_mask, 0, save->has_carlov_medal, (int)save->shells_owned);
    link_ptr->shells = figurine_gallery_get_shells();
    link_ptr->has_carlov_medal = save->has_carlov_medal;
    cucco_minigame_restore(save->cucco_level_cleared, save->cucco_heart_piece_obtained);
    link_ptr->is_carrying_cucco = false;
    sword_dojo_restore(save->tiger_scrolls_mask);
}

static void trigger_link_sword_attack(Player* link) {
    link->is_attacking = true;
    link->attack_timer = 12;
    hal_audio_play_sound(SOUND_SWORD_SLASH, link->is_minish ? 0.7f : 1.0f, link->is_minish ? 1.38f : 1.0f);
    if (!link->is_minish && (rand() % 100 < 45)) {
        hal_audio_play_sound((rand() % 2 == 0) ? SOUND_LINK_ATTACK1 : SOUND_LINK_ATTACK2, 0.95f, 1.0f);
    }
}

int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("====================================================================\n");
    printf("   The Legend of Zelda: The Minish Cap - Native PC Port (v0.99.0)   \n");
    printf("   Camada de Input & Entidade Controlavel do Heroi (Link)           \n");
    printf("====================================================================\n");
    printf("Controles Disponiveis:\n");
    printf("  - Mover Link:     [WASD] ou [Setas do Teclado] ou [D-Pad/Analogico]\n");
    printf("  - Atacar / Acao:  [Z] ou [Espaco] ou [Botao A do Gamepad] (Atacar espada / Falar / Abrir bau!)\n");
    printf("  - Ataque Girat.:  Segurar [A] (Carregar espada -> Soltar para Spin Attack 360°!)\n");
    printf("  - Portal Minish:  [R] ou [Botao R do Gamepad] (Encolher no Toco Minish / Crescer de volta!)\n");
    printf("  - Item Secundar.: [X] ou [Botao B do Gamepad] (Bumerangue / Pote Magico / Pegasus Boots!)\n");
    printf("  - Fusao Kinstone: [K] ou [Gatilho L no Gamepad] (Unir pedras da sorte com NPCs parceiros!)\n");
    printf("  - Ciclar Itens:   [Q] ou [Gatilho L no Gamepad] (Alternar item secundario equipado)\n");
    printf("  - Falar com Ezlo: [E] ou [Select no Gamepad] (Dicas e orientacoes do gorro companheiro!)\n");
    printf("  - Trilha Sonora:  [T] ou [Gatilho R no Gamepad] (Woods / Hyrule / Dungeon / Boss / Town / Village)\n");
    printf("  - Cidade Hyrule:  [H] Entrar/Sair do Hub da Cidade de Hyrule (Hyrule Town Hub!)\n");
    printf("  - Vila Minish:    [V] Entrar/Sair da Vila dos Minish (Picori Village!)\n");
    printf("  - Masmorra:       [D] Entrar/Sair de Deepwood Shrine (ou caminhar ao santuario ao norte!)\n");
    printf("  - South Field:    [4] Entrar em South Hyrule Field (Planicies do Sul!)\n");
    printf("  - North Field:    [5] Entrar em North Hyrule Field (Planicies do Norte!)\n");
    printf("  - Monte Crenel:   [6] Entrar em Mount Crenel Base (Base do Monte Crenel!)\n");
    printf("  - Castor Wilds:   [P] Entrar no Pantano de Castor Wilds (Lodo, Ropes e Estatuas de Olho!)\n");
    printf("  - Arco e Flechas: [O] Equipar o Arco e Flechas e Aljava no Botao [B]\n");
    printf("  - Segredo Zelda:  [M] (Chime lendario de 8 notas!)\n");
    printf("  - Trocar Regiao:  [1] USA | [2] EUR | [3] JPN\n");
    printf("  - Widescreen:     [W] Alternar proporcao 16:9\n");
    printf("  - Sair do Jogo:   [ESC]\n\n");

    int scale = 4;
    bool widescreen = false;

    if (!hal_video_init("The Legend of Zelda: The Minish Cap (Port Nativo)", scale, widescreen)) {
        return 1;
    }

    hal_input_init();
    hal_audio_init();
    font_init();
    dialogue_init();
    subweapon_init();
    kinstone_init();
    dungeon_init();
    dungeon_flames_init();
    dungeon_fortress_init();
    fast_travel_init();
    sanctuary_init();
    inventory_init();
    save_system_init();

    // Inicialização da Engine de Mapas e Câmera Widescreen (Autêntico Minish Woods ou Fallback)
    Tilemap* world_map = map_create_woods(s_region_tags[REGION_USA]);
    s_world_map = world_map;
    Tilemap* town_map = map_create_hyrule_town();
    s_town_map = town_map;
    Tilemap* village_map = map_create_minish_village();
    s_village_map = village_map;
    Tilemap* south_field_map = map_create_south_hyrule_field();
    s_south_field_map = south_field_map;
    Tilemap* north_field_map = map_create_north_hyrule_field();
    s_north_field_map = north_field_map;
    Tilemap* crenel_base_map = map_create_mount_crenel_base();
    s_crenel_base_map = crenel_base_map;
    Tilemap* melari_mines_map = map_create_melari_mines();
    s_melari_mines_map = melari_mines_map;
    Tilemap* castor_wilds_map = map_create_castor_wilds();
    s_castor_wilds_map = castor_wilds_map;
    Tilemap* mole_cave_map = map_create_mole_cave();
    s_mole_cave_map = mole_cave_map;
    Tilemap* wind_ruins_map = map_create_wind_ruins();
    s_wind_ruins_map = wind_ruins_map;
    Tilemap* armos_interior_map = map_create_armos_interior();
    s_armos_interior_map = armos_interior_map;
    Tilemap* library_map = map_create_library();
    s_library_map = library_map;
    Tilemap* lake_hylia_map = map_create_lake_hylia();
    s_lake_hylia_map = lake_hylia_map;
    Tilemap* castle_courtyard_map = map_create_castle_courtyard(s_region_tags[REGION_USA]);
    s_castle_courtyard_map = castle_courtyard_map;
    map_load_tileset(s_region_tags[REGION_USA]);
    library_quest_init();
    lantern_init();
    dungeon_droplets_init();
    load_region_sheets(REGION_USA);

    // Inicialização da entidade do Link
    Player link = { 0 };
    if (world_map && world_map->is_authentic) {
        // Caminho do jardim em frente ao santuário em Minish Woods (tx = 28, ty = 39)
        link.x = 448.0f;
        link.y = 636.0f;
    } else {
        link.x = 296.0f;
        link.y = 176.0f;
    }
    link.speed = 1.05f; // Calibrado com a velocidade autêntica do GBA (1.0 pixel/frame)
    link.dir = DIR_DOWN;
    link.is_moving = false;
    link.is_attacking = false;
    link.attack_timer = 0;
    link.anim_timer = 0;
    link.anim_frame = 0;
    link.hearts = 3;
    link.max_hearts = 3;
    link.health_quarters = 12; // 3 recipientes completos x 4 quartos = 12
    link.magic = 100;
    link.max_magic = 100;
    link.rupees = 50;
    link.invuln_timer = 0;
    link.knock_x = 0.0f;
    link.knock_y = 0.0f;
    link.has_spin_attack = false;
    link.is_charging_spin = false;
    link.spin_charge_timer = 0;
    link.spin_ready = false;
    link.is_spinning = false;
    link.spin_timer = 0;
    link.tiger_scroll_banner_timer = 0;
    link.is_minish = false;
    link.is_transforming = false;
    link.transform_timer = 0;
    link.transform_to_minish = false;
    link.has_flippers = true;
    link.is_swimming = false;
    link.swim_stroke_timer = 0;
    link.is_diving = false;
    link.dive_timer = 0;
    link.water_ripple_timer = 0;
    link.has_grip_ring = false;
    link.is_climbing = false;
    link.climb_anim_timer = 0;
    link.has_mineral_water = false;
    link.has_cane_of_pacci = true;
    link.z = 0.0f;
    link.vz = 0.0f;
    link.is_jumping = false;
    link.has_white_sword = false;
    link.white_sword_banner_timer = 0;
    link.has_bow = false;
    link.bow_banner_timer = 0;
    link.mud_sink_timer = 0;
    link.last_safe_x = link.x;
    link.last_safe_y = link.y;
    link.has_mole_mitts = false;
    link.mole_mitts_banner_timer = 0;
    link.has_armos_activated = false;
    link.armos_banner_timer = 0;
    link.has_ocarina = false;
    link.ocarina_banner_timer = 0;
    link.has_lantern = true;
    link.lantern_banner_timer = 0;
    link.has_water_element = false;
    link.water_element_banner_timer = 0;
    link.has_wind_element = false;
    link.wind_element_banner_timer = 0;
    link.dungeon_palace_cleared = false;
    dungeon_palace_init();
    link.has_three_elements = false;
    link.three_elements_banner_timer = 0;
    link.has_four_sword = false;
    link.four_sword_banner_timer = 0;
    link.has_veil_falls_unlocked = false;
    link.golden_kinstones_fused = 0;
    link.cloud_tornado_active = false;
    link.veil_banner_timer = 0;
    link.cloud_banner_timer = 0;
    veil_clouds_init();
    link.has_rocs_cape = false;
    link.rocs_banner_timer = 0;
    rocs_cape_init();
    link.has_royal_kinstone = false;
    link.royal_kinstone_banner_timer = 0;
    royal_valley_init();
    link.shells = 50;
    link.has_carlov_medal = false;
    figurine_gallery_init();
    link.is_carrying_cucco = false;
    cucco_minigame_init();
    sword_dojo_init();
    intro_cutscene_init();

    // Inicialização da Máquina de Estados de Abertura & Seleção de Save (Capcom, Nintendo, Title, File Select)
    startup_menu_init();

    // Inicialização do Subsistema de Entidades e Spawn de Inimigos e NPCs
    entity_manager_init();
    if (s_in_armos_interior) {
        spawn_armos_interior_entities(armos_circuit_is_active());
    } else if (s_in_wind_ruins) {
        spawn_wind_ruins_entities(armos_circuit_is_active());
    } else if (s_in_mole_cave) {
        spawn_mole_cave_entities(link.has_mole_mitts);
    } else if (s_in_castor_wilds) {
        spawn_castor_wilds_entities();
    } else if (s_in_melari_mines) {
        spawn_melari_mines_entities();
    } else if (s_in_village) {
        spawn_minish_village_entities();
    } else if (s_in_town) {
        spawn_town_entities();
    } else if (s_in_south_field) {
        spawn_south_field_entities();
    } else if (s_in_north_field) {
        spawn_north_field_entities();
    } else if (s_in_crenel_base) {
        spawn_crenel_base_entities();
    } else {
        spawn_overworld_entities(world_map);
    }

    Camera camera;
    camera.viewport_w = widescreen ? 284 : 240;
    camera.viewport_h = 160;
    camera.x = link.x - ((float)camera.viewport_w / 2.0f);
    camera.y = link.y - ((float)camera.viewport_h / 2.0f);

    bool running = true;
    SDL_Event event;

    // ========================================================================
    // O GAME LOOP MULTIPLATAFORMA A 60 FPS
    // ========================================================================
    while (running) {
        // --------------------------------------------------------------------
        // 1. CAPTURA DE EVENTOS DO SISTEMA OPERACIONAL
        // --------------------------------------------------------------------
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else {
                hal_input_process_event(&event);

                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.key == SDLK_RETURN || event.key.key == SDLK_P) {
                        if (!dialogue_is_active() && !kinstone_is_active()) {
                            inventory_toggle_pause();
                            continue;
                        }
                    }

                    if (inventory_is_paused()) {
                        switch (event.key.key) {
                            case SDLK_UP:
                                inventory_cursor_move(0, -1);
                                break;
                            case SDLK_DOWN:
                                inventory_cursor_move(0, 1);
                                break;
                            case SDLK_LEFT:
                                inventory_cursor_move(-1, 0);
                                break;
                            case SDLK_RIGHT:
                                inventory_cursor_move(1, 0);
                                break;
                            case SDLK_Z:
                            case SDLK_SPACE:
                                inventory_assign_to_slot_a();
                                break;
                            case SDLK_X:
                                inventory_assign_to_slot_b();
                                break;
                            case SDLK_S: {
                                SaveData current_save = { 0 };
                                strncpy(current_save.player_name, "LINK", sizeof(current_save.player_name));
                                current_save.player_x = link.x;
                                current_save.player_y = link.y;
                                current_save.player_dir = (int)link.dir;
                                int cur_m = 0;
                                if (vaati_boss_is_active()) cur_m = 31;
                                else if (dark_castle_is_active()) cur_m = 25 + (int)dark_castle_get_room();
                                else if (royal_valley_is_active()) cur_m = 20 + (int)royal_valley_get_scene();
                                else if (veil_clouds_is_active()) {
                                    cur_m = (veil_clouds_get_scene() == VEIL_SCENE_FALLS_BASE || veil_clouds_get_scene() == VEIL_SCENE_FALLS_SUMMIT) ? 18 : 19;
                                }
                                else if (dungeon_fortress_is_active()) cur_m = 14;
                                else if (s_in_armos_interior) cur_m = 13;
                                else if (s_in_wind_ruins) cur_m = 12;
                                else if (s_in_mole_cave) cur_m = 11;
                                else if (s_in_castor_wilds) cur_m = 10;
                                else if (s_in_melari_mines) cur_m = 7;
                                else if (s_in_crenel_base) cur_m = 6;
                                else if (s_in_north_field) cur_m = 5;
                                else if (s_in_south_field) cur_m = 4;
                                else if (dungeon_is_active()) cur_m = 3;
                                else if (dungeon_flames_is_active()) cur_m = 8;
                                else if (sanctuary_is_active()) cur_m = 9;
                                else if (dungeon_fortress_is_active()) cur_m = 14;
                                else if (dungeon_droplets_is_active()) cur_m = 17;
                                else if (s_in_library) cur_m = 15;
                                else if (s_in_lake_hylia) cur_m = 16;
                                else if (s_in_village) cur_m = 2;
                                else if (s_in_town) cur_m = 1;
                                current_save.current_map = cur_m;
                                current_save.hearts = link.hearts;
                                current_save.max_hearts = link.max_hearts;
                                current_save.rupees = link.rupees;
                                current_save.bomb_count = subweapon_get_bomb_count();
                                current_save.has_flippers = link.has_flippers;
                                current_save.has_spin_attack = link.has_spin_attack;
                                current_save.is_minish = link.is_minish;
                                current_save.has_grip_ring = link.has_grip_ring;
                                current_save.has_cane_of_pacci = link.has_cane_of_pacci;
                                current_save.has_white_sword = link.has_white_sword;
                                current_save.has_two_elements = link.has_two_elements;
                                current_save.has_three_elements = link.has_three_elements;
                                current_save.has_four_sword = link.has_four_sword;
                                current_save.secret_exit_unlocked = sanctuary_get_state() ? sanctuary_get_state()->secret_exit_unlocked : false;
                                current_save.has_water_element = link.has_water_element;
                                current_save.has_wind_element = link.has_wind_element;
                                current_save.dungeon_palace_cleared = link.dungeon_palace_cleared;
                                current_save.has_bow = link.has_bow;
                                current_save.has_mole_mitts = link.has_mole_mitts;
                                current_save.has_armos_activated = link.has_armos_activated;
                                current_save.has_ocarina = link.has_ocarina;
                                current_save.unlocked_wind_crests = fast_travel_get_unlocked_mask();
                                current_save.library_books_mask = library_get_save_mask();
                                current_save.librari_met = library_get_quest_state()->librari_met;
                                current_save.lake_temple_unlocked = library_is_temple_unlocked();
                                current_save.has_flame_lantern = link.has_lantern;
                                current_save.lantern_lit = lantern_is_lit();
                                current_save.has_veil_falls_unlocked = link.has_veil_falls_unlocked;
                                current_save.golden_kinstones_fused = veil_clouds_get_golden_kinstones();
                                current_save.cloud_tornado_active = veil_clouds_is_tornado_active();
                                current_save.has_rocs_cape = link.has_rocs_cape;
                                RoyalValleyState* rv_st = royal_valley_get_state();
                                current_save.royal_valley_unlocked = rv_st ? rv_st->is_active : false;
                                current_save.graveyard_gate_unlocked = rv_st ? rv_st->graveyard_unlocked : false;
                                current_save.dampe_met = rv_st ? rv_st->dampe_met : false;
                                current_save.tomb_pushed = rv_st ? rv_st->tomb_pushed : false;
                                current_save.king_gustaf_met = rv_st ? rv_st->king_gustaf_met : false;
                                current_save.has_royal_kinstone = link.has_royal_kinstone;
                                current_save.dark_castle_unlocked = dark_castle_is_active();
                                current_save.dark_castle_cleared = dark_castle_is_cleared();
                                current_save.dark_castle_bells_silenced = dark_castle_is_ready_for_vaati();
                                current_save.has_sanctum_key = link.has_sanctum_key;
                                current_save.vaati_defeated = vaati_boss_is_defeated();
                                int shells_temp = 0;
                                figurine_gallery_export(current_save.figurines_mask, NULL, &current_save.has_carlov_medal, &shells_temp);
                                current_save.shells_owned = (u16)shells_temp;
                                int cucco_cleared = 0;
                                bool cucco_hp = false;
                                cucco_minigame_export(&cucco_cleared, &cucco_hp);
                                current_save.cucco_level_cleared = (u8)cucco_cleared;
                                current_save.cucco_heart_piece_obtained = cucco_hp;
                                sword_dojo_export(&current_save.tiger_scrolls_mask);
                                current_save.slot_a = (int)inventory_get_slot_a();
                                current_save.slot_b = (int)inventory_get_slot_b();
                                if (save_game(1, &current_save)) {
                                    hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                                    printf("[SAVE] Progresso salvo no Slot 1 com sucesso!\n");
                                }
                                break;
                            }
                            case SDLK_ESCAPE:
                                inventory_toggle_pause();
                                break;
                            default:
                                break;
                        }
                        continue;
                    }

                    switch (event.key.key) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                        case SDLK_1:
                            load_region_sheets(REGION_USA);
                            break;
                        case SDLK_2:
                            load_region_sheets(REGION_EUR);
                            break;
                        case SDLK_3:
                            load_region_sheets(REGION_JPN);
                            break;
                        case SDLK_4:
                            if (!dungeon_is_active()) {
                                transition_to_south_field(&link);
                            }
                            break;
                        case SDLK_5:
                            if (!dungeon_is_active()) {
                                transition_to_north_field(&link);
                            }
                            break;
                        case SDLK_6:
                            if (!dungeon_is_active()) {
                                transition_to_crenel_base(&link);
                            }
                            break;
                        case SDLK_7:
                            if (!dungeon_is_active()) {
                                transition_to_melari_mines(&link);
                            }
                            break;
                        case SDLK_8:
                            link.has_white_sword = !link.has_white_sword;
                            inventory_set_white_sword(link.has_white_sword);
                            if (link.has_white_sword) link.white_sword_banner_timer = 180;
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                            printf("[DEBUG] Toggle White Sword: %s\n", link.has_white_sword ? "ON (Dano: 2)" : "OFF (Dano: 1)");
                            break;
                        case SDLK_9:
                            if (!dungeon_is_active()) {
                                if (dungeon_flames_is_active()) {
                                    dungeon_flames_exit(&link.x, &link.y, &link.dir);
                                    s_in_melari_mines = true;
                                    spawn_melari_mines_entities();
                                } else {
                                    transition_to_cave_of_flames(&link);
                                }
                            }
                            break;
                        case SDLK_0:
                            if (!dungeon_is_active() && !dungeon_flames_is_active()) {
                                if (sanctuary_is_active()) {
                                    sanctuary_exit(&link.x, &link.y, &link.dir);
                                    s_in_north_field = true;
                                    spawn_north_field_entities();
                                } else {
                                    transition_to_sanctuary(&link);
                                }
                            }
                            break;
                        case SDLK_W:
                            widescreen = !widescreen;
                            hal_video_shutdown();
                            hal_video_init("The Legend of Zelda: The Minish Cap (Port Nativo)", scale, widescreen);
                            break;
                        case SDLK_M:
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                            break;
                        case SDLK_H:
                            if (!dungeon_is_active()) {
                                if (s_in_town) {
                                    transition_to_overworld(&link, world_map);
                                } else {
                                    transition_to_town(&link);
                                }
                            }
                            break;
                        case SDLK_V:
                            if (!dungeon_is_active()) {
                                if (s_in_village) {
                                    transition_to_woods_from_village(&link, world_map);
                                } else {
                                    transition_to_minish_village(&link);
                                }
                            }
                            break;
                        case SDLK_T:
                            hal_audio_cycle_bgm();
                            break;
                        case SDLK_E:
                            if (!dialogue_is_active()) {
                                if (link.is_minish) {
                                    dialogue_trigger_ezlo_minish_hint();
                                } else {
                                    dialogue_trigger_ezlo_hint();
                                }
                            }
                            break;
                        case SDLK_R:
                            if (!link.is_transforming && !dialogue_is_active() && !kinstone_is_active()) {
                                Entity* stump = entity_find_nearby_minish_stump(link.x, link.y, 24.0f);
                                if (stump) {
                                    link.is_transforming = true;
                                    link.transform_timer = 50;
                                    link.transform_to_minish = !link.is_minish;
                                    link.is_moving = false;
                                    link.x = stump->x;
                                    link.y = stump->y;
                                    if (link.transform_to_minish) {
                                        hal_audio_play_sound(SOUND_MINISH_SHRINK, 1.0f, 1.0f);
                                        printf("[MINISH] [R] Link subiu no portal e esta ENCOLHENDO para tamanho Minish!\n");
                                    } else {
                                        hal_audio_play_sound(SOUND_MINISH_GROW, 1.0f, 1.0f);
                                        printf("[MINISH] [R] Link subiu no portal e esta CRESCENDO para tamanho Humano!\n");
                                    }
                                }
                            }
                            break;
                        case SDLK_F:
                            link.has_flippers = !link.has_flippers;
                            if (link.has_flippers) {
                                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                                printf("[FLIPPERS] [F] Nadadeiras de Zora EQUIPADAS! Link agora pode nadar na agua profunda!\n");
                            } else {
                                printf("[FLIPPERS] [F] Nadadeiras de Zora REMOVIDAS.\n");
                            }
                            break;
                        case SDLK_Q:
                            subweapon_cycle();
                            break;
                        case SDLK_C:
                            subweapon_set_current(ITEM_CANE_OF_PACCI);
                            hal_audio_play_sound(SOUND_SECRET, 0.9f, 1.8f);
                            printf("[CANE OF PACCI] [C] Cajado de Pacci EQUIPADO no botao [B]!\n");
                            break;
                        case SDLK_B:
                            subweapon_add_bombs(10);
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                            printf("[DEBUG] [B] +10 Bombas adicionadas a Bolsa de Bombas! (Total: %d)\n", subweapon_get_bomb_count());
                            break;
                        case SDLK_K:
                            if (!kinstone_is_active() && !dialogue_is_active()) {
                                Entity* knpc = entity_find_kinstone_npc(link.x, link.y, 32.0f);
                                if (knpc) {
                                    kinstone_start_fusion(knpc);
                                }
                            }
                            break;
                        case SDLK_D:
                            if (dungeon_is_active()) {
                                dungeon_exit(&link.x, &link.y, &link.dir);
                            } else {
                                entity_clear_all();
                                dungeon_enter(&link.x, &link.y, &link.dir);
                            }
                            break;
                        case SDLK_P:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !sanctuary_is_active()) {
                                if (s_in_castor_wilds) {
                                    transition_to_south_field_from_castor(&link);
                                } else {
                                    transition_to_castor_wilds(&link);
                                }
                            }
                            break;
                        case SDLK_O:
                            link.has_bow = !link.has_bow;
                            if (link.has_bow) {
                                inventory_unlock_item(INV_ITEM_BOW);
                                subweapon_set_current(ITEM_BOW);
                                subweapon_add_arrows(30);
                                link.bow_banner_timer = 200;
                                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                                printf("[DEBUG] [O] Arco e Flechas EQUIPADO! (Flechas: %d)\n", subweapon_get_arrow_count());
                            } else {
                                printf("[DEBUG] [O] Arco e Flechas DESEQUIPADO.\n");
                            }
                            break;
                        case SDLK_J:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !sanctuary_is_active()) {
                                if (s_in_mole_cave) {
                                    transition_to_castor_from_cave(&link);
                                } else {
                                    transition_to_mole_cave(&link);
                                }
                            }
                            break;
                        case SDLK_U:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !sanctuary_is_active()) {
                                if (s_in_wind_ruins) {
                                    transition_to_castor_from_ruins(&link);
                                } else {
                                    transition_to_wind_ruins(&link);
                                }
                            }
                            break;
                        case SDLK_Y:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !sanctuary_is_active()) {
                                if (s_in_armos_interior) {
                                    transition_to_wind_ruins_from_armos(&link);
                                } else {
                                    link.is_minish = true;
                                    transition_to_armos_interior(&link);
                                }
                            }
                            break;
                        case SDLK_I:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !sanctuary_is_active()) {
                                if (dungeon_fortress_is_active()) {
                                    dungeon_fortress_exit(&link.x, &link.y, &link.dir);
                                    s_in_wind_ruins = true;
                                    spawn_wind_ruins_entities(armos_circuit_is_active());
                                } else {
                                    dungeon_fortress_enter(&link.x, &link.y, &link.dir);
                                }
                            }
                            break;
                        case SDLK_Z:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active()) {
                                fast_travel_start(link.x, link.y);
                            }
                            break;
                        case SDLK_L:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active()) {
                                if (s_in_library) {
                                    transition_to_town_from_library(&link);
                                } else {
                                    transition_to_library(&link);
                                }
                            }
                            break;
                        case SDLK_N:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active()) {
                                if (s_in_lake_hylia) {
                                    transition_to_south_field_from_lake(&link);
                                } else {
                                    transition_to_lake_hylia(&link);
                                }
                            }
                            break;
                        case SDLK_G:
                            link.has_lantern = !link.has_lantern;
                            if (link.has_lantern) {
                                inventory_unlock_item(INV_ITEM_LANTERN);
                                subweapon_set_current(ITEM_FLAME_LANTERN);
                                lantern_set_lit(true);
                                link.lantern_banner_timer = 200;
                                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                                printf("[LANTERN] [G] Flame Lantern EQUIPADA no botao [B]!\n");
                            } else {
                                lantern_set_lit(false);
                                printf("[LANTERN] [G] Flame Lantern DESEQUIPADA.\n");
                            }
                            break;
                        case SDLK_F1:
                            lantern_toggle_dark_room();
                            break;
                        case SDLK_F2:
                            lantern_toggle_lit();
                            break;
                        case SDLK_F3:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active()) {
                                if (dungeon_droplets_is_active()) {
                                    dungeon_droplets_exit(&link.x, &link.y, &link.dir);
                                    s_in_lake_hylia = true;
                                } else {
                                    dungeon_droplets_enter(&link.x, &link.y, &link.dir);
                                }
                            }
                            break;
                        case SDLK_F4:
                            if (sanctuary_is_active()) {
                                if (!sanctuary_has_three_elements()) {
                                    sanctuary_set_three_elements(true);
                                    link.has_three_elements = true;
                                    link.three_elements_banner_timer = 200;
                                    printf("[DEBUG] [F4] Infusao de 3 Elementos ativada no Santuario!\n");
                                } else {
                                    sanctuary_set_four_elements(true);
                                    link.has_four_sword = true;
                                    link.four_sword_banner_timer = 240;
                                    inventory_set_four_sword(true);
                                    printf("[DEBUG] [F4] Infusao FINAL: FOUR SWORD FORJADA (4 Elementos)!\n");
                                }
                            } else if (!dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active() && !dungeon_palace_is_active()) {
                                transition_to_sanctuary(&link);
                            }
                            break;
                        case SDLK_F5:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active() && !sanctuary_is_active()) {
                                if (veil_clouds_is_active()) {
                                    if (veil_clouds_get_scene() == VEIL_SCENE_FALLS_BASE) {
                                        veil_clouds_enter_clouds(&link.x, &link.y, &link.dir);
                                    } else {
                                        veil_clouds_exit(&link.x, &link.y, &link.dir);
                                        s_in_north_field = true;
                                        spawn_north_field_entities();
                                    }
                                } else {
                                    s_in_town = s_in_village = s_in_south_field = s_in_crenel_base = s_in_melari_mines = s_in_castor_wilds = s_in_mole_cave = s_in_wind_ruins = s_in_armos_interior = s_in_library = s_in_lake_hylia = s_in_north_field = false;
                                    veil_clouds_enter_falls(&link.x, &link.y, &link.dir);
                                }
                            }
                            break;
                        case SDLK_F6:
                            link.has_rocs_cape = !link.has_rocs_cape;
                            if (link.has_rocs_cape) {
                                inventory_unlock_item(INV_ITEM_ROCS_CAPE);
                                subweapon_set_current(ITEM_ROCS_CAPE);
                                link.rocs_banner_timer = 200;
                                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                                printf("[ROCS CAPE] [F6] Capa de Roc EQUIPADA no botao [B]!\n");
                            } else {
                                printf("[ROCS CAPE] [F6] Capa de Roc DESEQUIPADA.\n");
                            }
                            break;
                        case SDLK_F7:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active()) {
                                if (dungeon_palace_is_active()) {
                                    dungeon_palace_exit(&link.x, &link.y, &link.dir);
                                    veil_clouds_enter_clouds(&link.x, &link.y, &link.dir);
                                } else {
                                    s_in_town = s_in_village = s_in_south_field = s_in_crenel_base = s_in_melari_mines = s_in_castor_wilds = s_in_mole_cave = s_in_wind_ruins = s_in_armos_interior = s_in_library = s_in_lake_hylia = s_in_north_field = false;
                                    dungeon_palace_enter(&link.x, &link.y, &link.dir);
                                }
                            }
                            break;
                        case SDLK_F8:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active() && !dungeon_palace_is_active() && !sanctuary_is_active()) {
                                if (royal_valley_is_active()) {
                                    RoyalValleySceneId next_sc = (royal_valley_get_scene() + 1) % ROYAL_SCENE_COUNT;
                                    if (next_sc == ROYAL_SCENE_ENTRANCE) royal_valley_enter_entrance(&link.x, &link.y, &link.dir);
                                    else if (next_sc == ROYAL_SCENE_MIST_MAZE) royal_valley_enter_maze(&link.x, &link.y, &link.dir);
                                    else if (next_sc == ROYAL_SCENE_DAMPE_CABIN) royal_valley_enter_dampe(&link.x, &link.y, &link.dir);
                                    else if (next_sc == ROYAL_SCENE_GRAVEYARD) royal_valley_enter_graveyard(&link.x, &link.y, &link.dir);
                                    else if (next_sc == ROYAL_SCENE_ROYAL_CRYPT) royal_valley_enter_crypt(&link.x, &link.y, &link.dir);
                                    printf("[DEBUG] [F8] Royal Valley cena alternada para %d!\n", next_sc);
                                } else {
                                    s_in_town = s_in_village = s_in_south_field = s_in_crenel_base = s_in_melari_mines = s_in_castor_wilds = s_in_mole_cave = s_in_wind_ruins = s_in_armos_interior = s_in_library = s_in_lake_hylia = s_in_north_field = false;
                                    royal_valley_enter_entrance(&link.x, &link.y, &link.dir);
                                    printf("[DEBUG] [F8] Entrada em Royal Valley!\n");
                                }
                            }
                            break;
                        case SDLK_F9:
                            if (!dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active() && !dungeon_palace_is_active() && !sanctuary_is_active()) {
                                if (dark_castle_is_active()) {
                                    DarkCastleRoomId next_rm = (dark_castle_get_room() + 1) % DHC_ROOM_COUNT;
                                    dark_castle_set_room(next_rm, &link.x, &link.y, &link.dir);
                                    printf("[DEBUG] [F9] Dark Hyrule Castle sala alternada para %d!\n", next_rm);
                                } else {
                                    s_in_town = s_in_village = s_in_south_field = s_in_crenel_base = s_in_melari_mines = s_in_castor_wilds = s_in_mole_cave = s_in_wind_ruins = s_in_armos_interior = s_in_library = s_in_lake_hylia = s_in_north_field = false;
                                    dark_castle_enter(&link.x, &link.y, &link.dir);
                                    printf("[DEBUG] [F9] Entrada em Dark Hyrule Castle!\n");
                                }
                            }
                            break;
                        case SDLK_F10:
                            startup_menu_return_to_title();
                            printf("[STARTUP] [F10] Retornando a Tela de Titulo e Selecao de Save!\n");
                            break;
                        case SDLK_F11:
                            if (vaati_boss_is_active()) {
                                vaati_boss_exit(&link.x, &link.y, &link.dir);
                                dark_castle_enter(&link.x, &link.y, &link.dir);
                                printf("[DEBUG] [F11] Saindo da Arena de Vaati e retornando ao Castelo!\n");
                            } else {
                                s_in_town = s_in_village = s_in_south_field = s_in_crenel_base = s_in_melari_mines = s_in_castor_wilds = s_in_mole_cave = s_in_wind_ruins = s_in_armos_interior = s_in_library = s_in_lake_hylia = s_in_north_field = false;
                                if (dark_castle_is_active()) dark_castle_exit(&link.x, &link.y, &link.dir);
                                vaati_boss_start(&link.x, &link.y, &link.dir);
                                printf("[DEBUG] [F11] Entrada direta no Confronto Final contra Vaati!\n");
                            }
                            break;
                        case SDLK_F12:
                            if (figurine_gallery_is_active()) {
                                figurine_gallery_close();
                                link.shells = figurine_gallery_get_shells();
                                link.has_carlov_medal = figurine_gallery_has_medal();
                                printf("[DEBUG] [F12] Fechando a Galeria do Carlov!\n");
                            } else {
                                figurine_gallery_open(link.shells > 0 ? link.shells : 50);
                                printf("[DEBUG] [F12] Abrindo a Galeria do Carlov (Gacha / Miniaturas)!\n");
                            }
                            break;
                        case SDLK_TAB:
                            if (s_in_town) {
                                if (cucco_minigame_is_active()) {
                                    cucco_minigame_stop();
                                } else {
                                    int next_lvl = cucco_minigame_get_highest_cleared() + 1;
                                    if (next_lvl > 10) next_lvl = 10;
                                    cucco_minigame_start_level(next_lvl);
                                }
                            }
                            break;
                        case SDLK_GRAVE: {
                            // Debug: desbloqueia o próximo Tiger Scroll não-desbloqueado
                            int unlocked_before = sword_dojo_get_unlocked_count();
                            for (int ts = 0; ts < TOTAL_TIGER_SCROLLS; ts++) {
                                if (!sword_dojo_has_scroll((TigerScrollId)ts)) {
                                    sword_dojo_unlock_scroll((TigerScrollId)ts);
                                    const TigerScrollInfo* info = sword_dojo_get_info((TigerScrollId)ts);
                                    printf("[DEBUG] [~] Tiger Scroll desbloqueado: %s (%d/%d)!\n",
                                           info ? info->name : "???",
                                           sword_dojo_get_unlocked_count(), TOTAL_TIGER_SCROLLS);
                                    break;
                                }
                            }
                            if (sword_dojo_get_unlocked_count() == unlocked_before && unlocked_before == TOTAL_TIGER_SCROLLS) {
                                printf("[DEBUG] [~] Todos os %d Tiger Scrolls ja desbloqueados!\n", TOTAL_TIGER_SCROLLS);
                            }
                            break;
                        }
                        case SDLK_INSERT: {
                            if (intro_cutscene_is_active()) {
                                intro_cutscene_skip();
                                printf("[DEBUG] [INS] Cutscene inicial pulada!\n");
                            } else {
                                intro_cutscene_start();
                                printf("[DEBUG] [INS] Disparando Cutscene Inicial Canônica (Festival de Picori & Vaati)!\n");
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        }

        // Atualiza os estados de transição (borda de subida/descida dos botões)
        hal_input_update();

        // --------------------------------------------------------------------
        // 1b. GESTÃO DAS TELAS DE ABERTURA, LOGOS, TÍTULO E SELEÇÃO DE ARQUIVO
        // --------------------------------------------------------------------
        if (startup_menu_is_active()) {
            startup_menu_update();
            startup_menu_render();

            if (!startup_menu_is_active()) {
                // Menu de inicialização concluído: carrega os dados do save e posiciona o jogador
                int slot = startup_menu_get_selected_slot();
                SaveData loaded;
                if (load_game(slot, &loaded)) {
                    apply_save_data(&loaded, &link);
                }

                // Reinicializa entidades para o mapa selecionado
                entity_manager_init();
                if (s_in_armos_interior) {
                    spawn_armos_interior_entities(armos_circuit_is_active());
                } else if (s_in_wind_ruins) {
                    spawn_wind_ruins_entities(armos_circuit_is_active());
                } else if (s_in_mole_cave) {
                    spawn_mole_cave_entities(link.has_mole_mitts);
                } else if (s_in_castor_wilds) {
                    spawn_castor_wilds_entities();
                } else if (s_in_melari_mines) {
                    spawn_melari_mines_entities();
                } else if (s_in_village) {
                    spawn_minish_village_entities();
                } else if (s_in_town) {
                    spawn_town_entities();
                } else if (s_in_south_field) {
                    spawn_south_field_entities();
                } else if (s_in_north_field) {
                    spawn_north_field_entities();
                } else if (s_in_crenel_base) {
                    spawn_crenel_base_entities();
                } else {
                    spawn_overworld_entities(world_map);
                }

                // Ajusta a câmera centrada em Link
                camera.x = link.x - ((float)camera.viewport_w / 2.0f);
                camera.y = link.y - ((float)camera.viewport_h / 2.0f);

                // Se for um Novo Jogo, dispara a introdução canônica (Torneio, Picori Blade & Petrificação de Zelda)
                if (startup_menu_is_new_game()) {
                    intro_cutscene_start();
                }

                // Inicia a música tema do ambiente carregado (se cutscene não estiver ativa)
                if (!intro_cutscene_is_active()) {
                    if (s_in_town) {
                        hal_audio_play_bgm(BGM_HYRULE_TOWN);
                    } else if (s_in_village) {
                        hal_audio_play_bgm(BGM_MINISH_VILLAGE);
                    } else {
                        hal_audio_play_bgm(BGM_HYRULE_OVERWORLD);
                    }
                }
            }

            hal_video_render_frame();
            continue;
        }

        // --------------------------------------------------------------------
        // 1b2. GESTÃO DA CUTSCENE CANÔNICA DE INTRODUÇÃO (FESTIVAL & VAATI)
        // --------------------------------------------------------------------
        if (intro_cutscene_is_active()) {
            intro_cutscene_update();
            intro_cutscene_render();

            if (!intro_cutscene_is_active()) {
                // Cutscene encerrou: entra diretamente no gameplay do Vilarejo de Hyrule
                s_in_town = true;
                s_in_village = s_in_south_field = s_in_north_field = s_in_crenel_base = s_in_melari_mines = s_in_castor_wilds = s_in_mole_cave = s_in_wind_ruins = s_in_armos_interior = s_in_library = s_in_lake_hylia = false;
                entity_manager_init();
                spawn_town_entities();
                link.x = 240.0f;
                link.y = 160.0f;
                link.dir = DIR_DOWN;
                camera.x = link.x - ((float)camera.viewport_w / 2.0f);
                camera.y = link.y - ((float)camera.viewport_h / 2.0f);
                hal_audio_play_bgm(BGM_HYRULE_TOWN);
                printf("[FLOW] Transicao direta da Cutscene para o Gameplay em Hyrule Town!\n");
            }

            hal_video_render_frame();
            continue;
        }

        // --------------------------------------------------------------------
        // 1c. GESTÃO DA GALERIA DO CARLOV & SISTEMA GACHA (ATO VI)
        // --------------------------------------------------------------------
        if (figurine_gallery_is_active()) {
            bool btn_a = hal_input_is_pressed(KEY_A);
            bool btn_b = hal_input_is_pressed(KEY_B);
            bool d_up = hal_input_is_pressed(KEY_UP);
            bool d_down = hal_input_is_pressed(KEY_DOWN);
            bool d_left = hal_input_is_pressed(KEY_LEFT);
            bool d_right = hal_input_is_pressed(KEY_RIGHT);
            bool start = hal_input_is_pressed(KEY_START);

            figurine_gallery_handle_input(btn_a, btn_b, d_up, d_down, d_left, d_right, start);
            figurine_gallery_update();
            figurine_gallery_render();

            if (!figurine_gallery_is_active()) {
                link.shells = figurine_gallery_get_shells();
                link.has_carlov_medal = figurine_gallery_has_medal();
            }

            hal_video_render_frame();
            continue;
        }

        if (hal_input_is_pressed(KEY_START)) {
            if (!dialogue_is_active() && !kinstone_is_active()) {
                inventory_toggle_pause();
            }
        }

        // --------------------------------------------------------------------
        // 2. ATUALIZAÇÃO DA LÓGICA DO JOGADOR (INPUT -> FÍSICA)
        // --------------------------------------------------------------------
        const HalVideoContext* ctx = hal_video_get_context();
        Tilemap* active_map = sanctuary_is_active() ? s_town_map :
                              (vaati_boss_is_active() ? vaati_boss_get_arena_map() :
                              (dark_castle_is_active() ? dark_castle_get_current_map() :
                              (royal_valley_is_active() ? royal_valley_get_state()->maps[royal_valley_get_scene()] :
                              (dungeon_palace_is_active() ? dungeon_palace_get_current_map() :
                              (veil_clouds_is_active() ? veil_clouds_get_current_map() :
                              (dungeon_droplets_is_active() ? dungeon_droplets_get_current_map() :
                              (s_in_armos_interior ? s_armos_interior_map :
                              (s_in_wind_ruins ? s_wind_ruins_map :
                              (s_in_mole_cave ? s_mole_cave_map :
                              (s_in_castor_wilds ? s_castor_wilds_map :
                              (s_in_melari_mines ? s_melari_mines_map :
                              (s_in_crenel_base ? s_crenel_base_map :
                              (s_in_library ? s_library_map :
                              (s_in_lake_hylia ? s_lake_hylia_map :
                              (s_in_castle_courtyard ? s_castle_courtyard_map :
                              (s_in_village ? s_village_map :
                              (s_in_town ? s_town_map :
                              (s_in_south_field ? s_south_field_map :
                              (s_in_north_field ? s_north_field_map : world_map)))))))))))))))))));

        if (inventory_is_paused()) {
            if (hal_input_is_pressed(KEY_UP))    inventory_cursor_move(0, -1);
            if (hal_input_is_pressed(KEY_DOWN))  inventory_cursor_move(0, 1);
            if (hal_input_is_pressed(KEY_LEFT))  inventory_cursor_move(-1, 0);
            if (hal_input_is_pressed(KEY_RIGHT)) inventory_cursor_move(1, 0);
            if (hal_input_is_pressed(KEY_A))     inventory_assign_to_slot_a();
            if (hal_input_is_pressed(KEY_B))     inventory_assign_to_slot_b();
        } else {
            if (link.is_transforming) {
            link.transform_timer--;
            link.is_moving = false;

            // Clímax da transformação (frame 25): troca de tamanho e Screen Shake
            if (link.transform_timer == 25) {
                link.is_minish = link.transform_to_minish;
                entity_trigger_screen_shake(8, 2);
            }

            if (link.transform_timer <= 0) {
                link.is_transforming = false;
                printf("[MINISH] Transformacao concluida! Novo tamanho do Link: %s\n",
                       link.is_minish ? "MINISH (8x8 px)" : "HUMANO (16x16 px)");
            }
        } else if (kinstone_is_active()) {
            kinstone_update();

            bool confirm = hal_input_is_pressed(KEY_A) || hal_input_is_pressed(KEY_START);
            bool cancel  = hal_input_is_pressed(KEY_B);
            int  nav_dir = 0;

            if (hal_input_is_pressed(KEY_LEFT))  nav_dir = -1;
            if (hal_input_is_pressed(KEY_RIGHT)) nav_dir = 1;

            AnalogStick stick = hal_input_get_left_stick();
            static bool s_stick_was_left = false;
            static bool s_stick_was_right = false;
            if (stick.x < -0.5f && !s_stick_was_left) { nav_dir = -1; s_stick_was_left = true; }
            if (stick.x > -0.2f) s_stick_was_left = false;
            if (stick.x > 0.5f && !s_stick_was_right) { nav_dir = 1; s_stick_was_right = true; }
            if (stick.x < 0.2f) s_stick_was_right = false;

            kinstone_handle_input(confirm, cancel, nav_dir);
            link.is_moving = false;
        } else if (dialogue_is_active()) {
            dialogue_update();
            if (hal_input_is_pressed(KEY_A) || hal_input_is_pressed(KEY_START)) {
                dialogue_advance();
            }
            if (hal_input_is_held(KEY_B)) {
                dialogue_fast_forward();
            }
            link.is_moving = false;
        } else if (fast_travel_is_active()) {
            int ft_map = -1;
            float ft_x = 0.0f, ft_y = 0.0f;
            if (fast_travel_update(&link.x, &link.y, &ft_map, &ft_x, &ft_y)) {
                handle_fast_travel_transition(ft_map, ft_x, ft_y, &link, s_world_map);
            }
            link.is_moving = false;
        } else {
            const char* crest_name = NULL;
            if (fast_travel_check_crest_activation(link.x, link.y, &crest_name)) {
                printf("[ZEFFA] Crista de Vento ativada: %s!\n", crest_name);
            }
            // Recompensa do Mestre Swiftblade: Concede o Pergaminho do Tigre nº 1
            if (dialogue_is_swiftblade_reward_pending()) {
                dialogue_clear_swiftblade_reward();
                link.has_spin_attack = true;
                sword_dojo_unlock_scroll(SCROLL_SPIN_ATTACK);
            }

            // Recompensa do Mestre Melari: Forja a sagrada White Sword (Espada Branca)
            if (dialogue_is_melari_reward_pending()) {
                dialogue_clear_melari_reward();
                link.has_white_sword = true;
                inventory_set_white_sword(true);
                link.white_sword_banner_timer = 200;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                printf("[WHITE SWORD] Lamina Picori reforjada com sucesso na lendaria WHITE SWORD (Dano: 2)!\n");
            }
            if (subweapon_get_current() == ITEM_BOW && !link.has_bow) {
                link.has_bow = true;
                inventory_unlock_item(INV_ITEM_BOW);
                link.bow_banner_timer = 200;
            }
            if (subweapon_get_current() == ITEM_MOLE_MITTS && !link.has_mole_mitts) {
                link.has_mole_mitts = true;
                inventory_unlock_item(INV_ITEM_MOLE_MITTS);
                link.mole_mitts_banner_timer = 200;
            }
            if (armos_circuit_is_active() && !link.has_armos_activated) {
                link.has_armos_activated = true;
                link.armos_banner_timer = 220;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                printf("[ARMOS] Circuito central energizado! O robo Armos despertou e moveu-se!\n");
            }

            // Ação com Botão A: Primeiro Natação (Mergulho), Portal Minish, Masmorra / Loja / Guarda / Cidadã / Baús / Swiftblade / NPCs, depois golpe de espada!
            if (hal_input_is_pressed(KEY_A) && !link.is_attacking && !link.is_spinning && !link.is_charging_spin) {
                if (link.is_jumping && (link.has_rocs_cape || subweapon_get_current() == ITEM_ROCS_CAPE)) {
                    if (rocs_cape_try_down_thrust(&link.z, &link.vz, link.is_jumping)) {
                        // Down-Thrust iniciado com sucesso!
                    }
                } else if (link.is_swimming) {
                    if (!link.is_diving) {
                        link.is_diving = true;
                        link.dive_timer = 45;
                        hal_audio_play_sound(SOUND_DIVE, 1.0f, 1.0f);
                        printf("[FLIPPERS] [A] Link mergulhou no fundo da agua! (Diving)\n");
                    }
                } else {
                    Entity* nearby_stump = entity_find_nearby_minish_stump(link.x, link.y, 22.0f);
                    if (nearby_stump) {
                        link.is_transforming = true;
                        link.transform_timer = 50;
                        link.transform_to_minish = !link.is_minish;
                        link.is_moving = false;
                        link.x = nearby_stump->x;
                        link.y = nearby_stump->y;
                        if (link.transform_to_minish) {
                            hal_audio_play_sound(SOUND_MINISH_SHRINK, 1.0f, 1.0f);
                            printf("[MINISH] [A] Link subiu no portal e esta ENCOLHENDO para tamanho Minish!\n");
                        } else {
                            hal_audio_play_sound(SOUND_MINISH_GROW, 1.0f, 1.0f);
                            printf("[MINISH] [A] Link subiu no portal e esta CRESCENDO para tamanho Humano!\n");
                        }
                    } else if (dungeon_is_active()) {
                        if (dungeon_interact(link.x, link.y, &link.rupees, &link.hearts)) {
                            // Abriu o baú do altar da masmorra!
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (dungeon_flames_is_active()) {
                        if (dungeon_flames_interact(link.x, link.y, &link.rupees, &link.hearts)) {
                            // Interagiu com a vagoneta ou elementos da Cave of Flames!
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (sanctuary_is_active()) {
                        if (sanctuary_interact(link.x, link.y, link.has_white_sword, true, link.has_fire_element, link.has_water_element, link.has_wind_element)) {
                            if (link.has_wind_element && link.has_three_elements && !link.has_four_sword) {
                                link.has_four_sword = true;
                                link.four_sword_banner_timer = 240;
                                inventory_set_four_sword(true);
                            } else if (link.has_water_element && !link.has_three_elements) {
                                link.has_three_elements = true;
                                link.three_elements_banner_timer = 240;
                            } else if (!link.has_two_elements) {
                                link.has_two_elements = true;
                                link.two_elements_banner_timer = 240;
                            }
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (veil_clouds_is_active()) {
                        if (veil_clouds_interact(link.x, link.y, link.dir)) {
                            link.golden_kinstones_fused = veil_clouds_get_golden_kinstones();
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (royal_valley_is_active()) {
                        if (royal_valley_interact(link.x, link.y, link.dir, &link.hearts)) {
                            if (royal_valley_has_royal_kinstone() && !link.has_royal_kinstone) {
                                link.has_royal_kinstone = true;
                                link.royal_kinstone_banner_timer = 240;
                            }
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (s_in_town) {
                        if (cucco_minigame_is_active()) {
                            cucco_minigame_handle_action(link.x, link.y, link.dir);
                            if (!cucco_minigame_is_carrying()) {
                                trigger_link_sword_attack(&link);
                            }
                        } else if (link.x >= 150.0f && link.x <= 185.0f && link.y >= 155.0f && link.y <= 200.0f) {
                            int next_lvl = cucco_minigame_get_highest_cleared() + 1;
                            if (next_lvl > 10) next_lvl = 10;
                            cucco_minigame_start_level(next_lvl);
                            printf("[ANJU] Link conversou com Anju e aceitou a rodada %d do resgate de Cuccos!\n", next_lvl);
                        } else {
                            Entity* smith = entity_find_nearby_smith(link.x, link.y, 32.0f);
                            if (smith) {
                                dialogue_trigger_smith_talk();
                            } else {
                                Entity* swift = entity_find_nearby_swiftblade(link.x, link.y, 32.0f);
                                if (swift) {
                                    dialogue_trigger_swiftblade_talk(link.has_spin_attack);
                                } else {
                                    Entity* hagen = entity_find_nearby_mayor_hagen(link.x, link.y, 32.0f);
                                    if (hagen) {
                                        dialogue_trigger_mayor_hagen_talk();
                                    } else {
                                        Entity* scrub = entity_find_nearby_business_scrub(link.x, link.y, 32.0f);
                                        if (scrub) {
                                            dialogue_trigger_business_scrub_talk(link.rupees, link.has_grip_ring);
                                            if (!link.has_grip_ring) {
                                                if (entity_buy_grip_ring(&link.rupees, &link.has_grip_ring)) {
                                                    inventory_unlock_item(INV_ITEM_GRIP_RING);
                                                }
                                            }
                                        } else {
                                            Entity* shopkeeper = entity_find_nearby_shopkeeper(link.x, link.y, 40.0f);
                                            if (shopkeeper) {
                                                dialogue_trigger_shopkeeper_talk(link.rupees);
                                                // Se estiver perto do balcão de compras:
                                                if (link.x >= 420.0f && link.x <= 468.0f && link.y <= 136.0f) {
                                                    if (link.hearts < link.max_hearts && link.rupees >= 30) {
                                                        entity_buy_shop_item(0, &link.rupees, &link.hearts, &link.max_hearts); // Poção Vermelha
                                                    } else if (link.max_hearts < 6 && link.rupees >= 80) {
                                                        entity_buy_shop_item(1, &link.rupees, &link.hearts, &link.max_hearts); // Piece of Heart
                                                    } else if (link.rupees >= 50) {
                                                        entity_buy_shop_item(2, &link.rupees, &link.hearts, &link.max_hearts); // Bolsa de Bombas
                                                    }
                                                }
                                            } else {
                                                Entity* guard = entity_find_nearby_town_guard(link.x, link.y, 30.0f);
                                                if (guard) {
                                                    dialogue_trigger_town_guard_talk();
                                                } else {
                                                    Entity* citizen = entity_find_nearby_town_citizen(link.x, link.y, 30.0f);
                                                    if (citizen) {
                                                        dialogue_trigger_town_citizen_talk();
                                                        if (!library_has_book(BOOK_BESTIARY)) {
                                                            library_collect_book(BOOK_BESTIARY);
                                                        }
                                                    } else {
                                                        trigger_link_sword_attack(&link);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    } else if (s_in_village) {
                        Entity* gentari = entity_find_nearby_gentari(link.x, link.y, 30.0f);
                        if (gentari) {
                            dialogue_trigger_gentari_talk();
                            if (!library_has_book(BOOK_MASKS_HISTORY)) {
                                library_collect_book(BOOK_MASKS_HISTORY);
                            }
                        } else {
                            Entity* festari = entity_find_nearby_festari(link.x, link.y, 30.0f);
                            if (festari) {
                                dialogue_trigger_festari_talk();
                            } else {
                                Entity* villager = entity_find_nearby_village_minish(link.x, link.y, 28.0f);
                                if (villager) {
                                    dialogue_trigger_village_minish_talk();
                                } else {
                                    trigger_link_sword_attack(&link);
                                }
                            }
                        }
                    } else if (s_in_library) {
                        Entity* librari = entity_find_nearby_librari(link.x, link.y, 32.0f);
                        if (librari) {
                            library_talk_to_librari();
                        } else if (link.y <= 68.0f) {
                            library_return_books_to_shelf();
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (s_in_lake_hylia) {
                        Entity* hagen = entity_find_nearby_mayor_hagen(link.x, link.y, 32.0f);
                        if (hagen) {
                            dialogue_trigger_mayor_hagen_talk();
                            library_collect_book(BOOK_PICORI_LEGEND);
                        } else if (entity_interact_chest(link.x, link.y, &link.rupees, &link.hearts)) {
                            // Bau do Lago aberto!
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (s_in_south_field) {
                        Entity* malon = entity_find_nearby_malon(link.x, link.y, 32.0f);
                        if (malon) {
                            dialogue_trigger_malon_talk();
                        } else if (link.x >= 32.0f && link.x <= 96.0f && link.y >= 32.0f && link.y <= 96.0f) {
                            figurine_gallery_open(link.shells > 0 ? link.shells : 50);
                            printf("[CARLOV] Link entrou na Arvore de Carlov para jogar no Gacha de Estatuetas!\n");
                        } else if (entity_interact_chest(link.x, link.y, &link.rupees, &link.hearts)) {
                            // Baú da fazenda Lon Lon aberto!
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (s_in_north_field) {
                        float pdx = link.x - 48.0f;
                        float pdy = link.y - 144.0f;
                        if (pdx * pdx + pdy * pdy <= 32.0f * 32.0f) {
                            dialogue_trigger_crenel_sign_talk();
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (s_in_crenel_base) {
                        Entity* scrub = entity_find_nearby_business_scrub(link.x, link.y, 32.0f);
                        if (scrub) {
                            dialogue_trigger_business_scrub_talk(link.rupees, link.has_grip_ring);
                            if (!link.has_grip_ring) {
                                if (entity_buy_grip_ring(&link.rupees, &link.has_grip_ring)) {
                                    inventory_unlock_item(INV_ITEM_GRIP_RING);
                                }
                            }
                        } else if (map_is_mineral_water(active_map, link.x + 8.0f, link.y + 12.0f)) {
                            link.has_mineral_water = true;
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                            printf("[CRENEL] Link coletou Agua Mineral Borbulhante do Monte Crenel!\n");
                        } else if (link.has_mineral_water && map_interact_grow_bean(active_map, link.x + 8.0f, link.y + 8.0f)) {
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.25f);
                            hal_audio_play_sound(SOUND_CHEST_OPEN, 1.0f, 1.0f);
                            printf("[CRENEL] Broto de Feijao Magico regado! Cresceu um pe de feijao escalavel gigante!\n");
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (s_in_melari_mines) {
                        Entity* melari = entity_find_nearby_melari(link.x, link.y, 36.0f);
                        Entity* miner = entity_find_nearby_mountain_minish(link.x, link.y, 28.0f);
                        if (melari) {
                            dialogue_trigger_melari_talk(link.has_white_sword);
                        } else if (miner) {
                            dialogue_trigger_mountain_minish_talk();
                        } else {
                            trigger_link_sword_attack(&link);
                        }
                    } else if (entity_interact_chest(link.x, link.y, &link.rupees, &link.hearts)) {
                        // Abriu o baú dourado!
                    } else {
                        Entity* nearby_swiftblade = entity_find_nearby_swiftblade(link.x, link.y, 28.0f);
                        if (nearby_swiftblade) {
                            dialogue_trigger_swiftblade_talk(link.has_spin_attack);
                        } else {
                            Entity* nearby_npc = entity_find_nearby_npc(link.x, link.y, 28.0f);
                            if (nearby_npc) {
                                dialogue_trigger_minish_talk();
                            } else {
                                trigger_link_sword_attack(&link);
                            }
                        }
                    }
                }
            }

            if (link.is_attacking) {
                // Hitbox do golpe de espada dependendo da orientação do Link
                float hit_x = link.x + 4.0f;
                float hit_y = link.y + 4.0f;
                float hit_w = 12.0f;
                float hit_h = 12.0f;

                if (link.dir == DIR_DOWN)  { hit_x = link.x + 1.0f;  hit_y = link.y + 14.0f; hit_w = 14.0f; hit_h = 12.0f; }
                if (link.dir == DIR_UP)    { hit_x = link.x + 1.0f;  hit_y = link.y - 10.0f; hit_w = 14.0f; hit_h = 12.0f; }
                if (link.dir == DIR_LEFT)  { hit_x = link.x - 12.0f; hit_y = link.y + 2.0f;  hit_w = 12.0f; hit_h = 14.0f; }
                if (link.dir == DIR_RIGHT) { hit_x = link.x + 14.0f; hit_y = link.y + 2.0f;  hit_w = 12.0f; hit_h = 14.0f; }

                // Checa acerto contra inimigos (Octoroks, Keese, ChuChu) e projéteis
                int sword_dmg = link.has_four_sword ? 3 : (link.has_white_sword ? 2 : 1);
                entity_check_sword_hit(hit_x, hit_y, hit_w, hit_h, sword_dmg, link.dir);
                if (dungeon_droplets_is_active()) {
                    dungeon_droplets_check_boss_sword_hit(hit_x, hit_y, hit_w, hit_h, sword_dmg, link.dir);
                }
                if (dungeon_palace_is_active()) {
                    dungeon_palace_check_sword_hit(hit_x, hit_y, hit_w, hit_h, sword_dmg, link.dir);
                }
                if (royal_valley_is_active()) {
                    royal_valley_check_sword_hit(hit_x, hit_y, hit_w, hit_h, sword_dmg, link.dir);
                }
                if (dark_castle_is_active()) {
                    float cl_x[3] = { 0 };
                    float cl_y[3] = { 0 };
                    int cl_cnt = 0;
                    for (int c = 0; c < 3; c++) {
                        CloneState* cl = sanctuary_get_clone_at(c);
                        if (cl && cl->active) {
                            cl_x[cl_cnt] = cl->x;
                            cl_y[cl_cnt] = cl->y;
                            cl_cnt++;
                        }
                    }
                    dark_castle_strike_sword(link.x, link.y, link.dir, sword_dmg, link.is_spinning, cl_cnt, cl_x, cl_y);
                }
                if (vaati_boss_is_active()) {
                    float cl_x[3] = { 0 };
                    float cl_y[3] = { 0 };
                    int cl_cnt = 0;
                    for (int c = 0; c < 3; c++) {
                        CloneState* cl = sanctuary_get_clone_at(c);
                        if (cl && cl->active) {
                            cl_x[cl_cnt] = cl->x;
                            cl_y[cl_cnt] = cl->y;
                            cl_cnt++;
                        }
                    }
                    vaati_boss_strike_sword(link.x, link.y, link.dir, sword_dmg, link.is_spinning, cl_cnt, cl_x, cl_y);
                }

                // Disparo de Raio da Four Sword com HP cheio no início do golpe
                if (link.attack_timer == 11) {
                    sanctuary_try_fire_sword_beam(link.x, link.y, link.dir, link.hearts, link.max_hearts, link.has_four_sword);
                }

                // Ataque sincronizado dos Clones da Four Sword (até 3 clones: Red, Blue, Purple)
                for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
                    float c_hx, c_hy, c_hw, c_hh;
                    int c_dmg;
                    if (sanctuary_get_any_clone_sword_hitbox(c, &c_hx, &c_hy, &c_hw, &c_hh, &c_dmg)) {
                        CloneState* cl = sanctuary_get_clone_at(c);
                        if (cl) {
                            entity_check_sword_hit(c_hx, c_hy, c_hw, c_hh, c_dmg, cl->dir);
                        }
                    }
                }

                // Interação da espada com o cenário (cortar arbustos ou abrir baú no overworld)
                if (!dungeon_is_active() && !dungeon_flames_is_active() && !sanctuary_is_active() && map_interact_slash(active_map, hit_x + 6.0f, hit_y + 6.0f)) {
                    hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.25f);
                    link.rupees += 5; // Recompensa clássica de Zelda!
                }

                link.attack_timer--;
                if (link.attack_timer <= 0) {
                    link.is_attacking = false;
                    // Se o herói possui o Pergaminho do Tigre e manteve o botão A segurado: inicia a carga!
                    if (link.has_spin_attack && hal_input_is_held(KEY_A) && !link.is_swimming) {
                        link.is_charging_spin = true;
                        link.spin_charge_timer = 0;
                        link.spin_ready = false;
                    }
                }
            }

            // ----------------------------------------------------------------
            // MÁQUINA DE ESTADOS DO ATAQUE GIRATÓRIO (SPIN ATTACK FSM)
            // ----------------------------------------------------------------
            if (link.is_charging_spin) {
                if (hal_input_is_held(KEY_A)) {
                    link.spin_charge_timer++;
                    if (!link.spin_ready) {
                        if (link.spin_charge_timer % 20 == 0) {
                            hal_audio_play_sound(SOUND_SPIN_CHARGE, 0.70f, 1.0f);
                        }
                        if (link.spin_charge_timer >= 38) {
                            link.spin_ready = true;
                            hal_audio_play_sound(SOUND_SPIN_READY, 1.0f, 1.0f);
                        }
                    }
                } else {
                    // Botão A liberado pelo jogador!
                    if (link.spin_ready) {
                        // DISPARAR O ATAQUE GIRATÓRIO EM 360 GRAUS!
                        link.is_spinning = true;
                        link.spin_timer = 16;
                        hal_audio_play_sound(SOUND_SPIN_ATTACK, 1.0f, 1.0f);
                        if (!link.is_minish) {
                            hal_audio_play_sound(SOUND_LINK_SPIN, 1.0f, 1.0f);
                        }
                    }
                    link.is_charging_spin = false;
                    link.spin_ready = false;
                    link.spin_charge_timer = 0;
                }
            }

            // Execução da rotação e física de corte circular 360°
            if (link.is_spinning) {
                link.spin_timer--;
                int rot_step = (16 - link.spin_timer) / 4;
                Direction rot_dirs[4] = { DIR_DOWN, DIR_RIGHT, DIR_UP, DIR_LEFT };
                link.dir = rot_dirs[rot_step % 4];

                float spin_cx = link.x + 8.0f;
                float spin_cy = link.y + 8.0f;
                float spin_r = 26.0f;

                // 2 HP / 4 HP de dano duplicado e knockback radial centrífugo
                int spin_dmg = link.has_white_sword ? 4 : 2;
                entity_check_spin_attack_hit(spin_cx, spin_cy, spin_r, spin_dmg);
                for (int c = 0; c < MAX_SANCTUARY_CLONES; c++) {
                    CloneState* cl = sanctuary_get_clone_at(c);
                    if (cl && cl->active) {
                        entity_check_spin_attack_hit(cl->x + 8.0f, cl->y + 8.0f, spin_r, spin_dmg);
                    }
                }
                if (dungeon_droplets_is_active()) {
                    dungeon_droplets_check_boss_sword_hit(spin_cx - spin_r, spin_cy - spin_r, spin_r * 2.0f, spin_r * 2.0f, spin_dmg, link.dir);
                }
                if (dungeon_palace_is_active()) {
                    dungeon_palace_check_sword_hit(spin_cx - spin_r, spin_cy - spin_r, spin_r * 2.0f, spin_r * 2.0f, spin_dmg, link.dir);
                }

                // Corte simultâneo de todos os arbustos no raio de 360 graus
                if (!dungeon_is_active() && !dungeon_flames_is_active()) {
                    int bushes = map_interact_spin(active_map, spin_cx, spin_cy, spin_r);
                    if (bushes > 0) {
                        hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 1.30f);
                        link.rupees += bushes * 5;
                    }
                }

                if (link.spin_timer <= 0) {
                    link.is_spinning = false;
                }
            }

            // Subsistema de Armas Secundarias e Itens Equipaveis (Botao B)
            if (hal_input_is_pressed(KEY_B)) {
                if (subweapon_get_current() == ITEM_PEGASUS_BOOTS) {
                    hal_audio_play_sound(SOUND_ROLL, 0.70f, 1.0f);
                } else if (subweapon_get_current() == ITEM_ROCS_CAPE) {
                    rocs_cape_use_pressed(&link.x, &link.y, link.dir, &link.z, &link.vz, &link.is_jumping);
                    if (!link.is_minish) {
                        hal_audio_play_sound(SOUND_LINK_JUMP, 0.95f, 1.0f);
                    }
                }
                subweapon_use_pressed(link.x, link.y, link.dir);
            }
            if (hal_input_is_held(KEY_B)) {
                if (subweapon_get_current() == ITEM_ROCS_CAPE) {
                    rocs_cape_use_held(&link.z, &link.vz, link.is_jumping);
                }
                subweapon_use_held(link.x, link.y, link.dir);
            }
            if (hal_input_is_released(KEY_B)) {
                if (subweapon_get_current() == ITEM_ROCS_CAPE) {
                    rocs_cape_use_released();
                }
                subweapon_use_released(link.x, link.y, link.dir);
            }

            // Chamado de Orientação do Companheiro Ezlo
            if (hal_input_is_pressed(KEY_SELECT)) {
                if (link.is_minish) {
                    dialogue_trigger_ezlo_minish_hint();
                } else {
                    dialogue_trigger_ezlo_hint();
                }
            }
            if (hal_input_is_pressed(KEY_L)) {
                Entity* knpc = entity_find_kinstone_npc(link.x, link.y, 32.0f);
                if (knpc) {
                    kinstone_start_fusion(knpc);
                } else {
                    subweapon_cycle();
                }
            }
            if (hal_input_is_pressed(KEY_R)) {
                Entity* nearby_stump = entity_find_nearby_minish_stump(link.x, link.y, 24.0f);
                if (nearby_stump) {
                    link.is_transforming = true;
                    link.transform_timer = 50;
                    link.transform_to_minish = !link.is_minish;
                    link.is_moving = false;
                    link.x = nearby_stump->x;
                    link.y = nearby_stump->y;
                    if (link.transform_to_minish) {
                        hal_audio_play_sound(SOUND_MINISH_SHRINK, 1.0f, 1.0f);
                        printf("[MINISH] [KEY_R] Link subiu no portal e esta ENCOLHENDO para tamanho Minish!\n");
                    } else {
                        hal_audio_play_sound(SOUND_MINISH_GROW, 1.0f, 1.0f);
                        printf("[MINISH] [KEY_R] Link subiu no portal e esta CRESCENDO para tamanho Humano!\n");
                    }
                } else if (!link.is_minish && !link.is_rolling && !link.is_swimming && !link.is_climbing) {
                    // Rolamento somersault acrobático canônico do Minish Cap!
                    link.is_rolling = true;
                    link.roll_timer = 16;
                    hal_audio_play_sound(SOUND_ROLL, 0.75f, 1.0f);
                } else {
                    hal_audio_cycle_bgm();
                }
            }

            // Atualização do Rolamento Acrobático (Somersault Roll)
            if (link.is_rolling) {
                link.roll_timer--;
                if (link.roll_timer <= 0) {
                    link.is_rolling = false;
                }
                // Roll Attack: Se pressionar [A] durante o rolamento e possuir o Tiger Scroll
                if (hal_input_is_pressed(KEY_A) && sword_dojo_has_scroll(SCROLL_ROLL_ATTACK)) {
                    link.is_rolling = false;
                    link.is_attacking = true;
                    link.attack_timer = 14;
                    hal_audio_play_sound(SOUND_SWORD_SLASH, 1.1f, 1.15f);
                }
            }

        // --------------------------------------------------------------------
        // LÓGICA DE NATAÇÃO & MERGULHO (ZORA'S FLIPPERS)
        // --------------------------------------------------------------------
        link.water_ripple_timer++;
        bool on_water = map_is_water(active_map, link.x + 8.0f, link.y + 12.0f);
        if (on_water && link.has_flippers && !dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active()) {
            if (!link.is_swimming) {
                link.is_swimming = true;
                link.swim_stroke_timer = 0;
                hal_audio_play_sound(SOUND_SWIM_STROKE, 0.85f, 1.0f);
                printf("[FLIPPERS] Link entrou na agua e comecou a nadar!\n");
            }
        } else {
            if (link.is_swimming) {
                link.is_swimming = false;
                if (link.is_diving) {
                    link.is_diving = false;
                    link.dive_timer = 0;
                }
                hal_audio_play_sound(SOUND_SURFACE, 1.0f, 1.0f);
                printf("[FLIPPERS] Link saiu da agua e voltou a terra firme!\n");
            }
        }

        if (link.is_swimming) {
            if (link.is_diving) {
                link.dive_timer--;
                link.invuln_timer = 2; // Invulnerável a perigos da superfície enquanto mergulhado
                if (link.dive_timer <= 0) {
                    link.is_diving = false;
                    hal_audio_play_sound(SOUND_SURFACE, 1.0f, 1.0f);
                    printf("[FLIPPERS] Link voltou a tona (resurfaced)!\n");
                }
            } else if (link.is_moving) {
                link.swim_stroke_timer++;
                if (link.swim_stroke_timer >= 22) {
                    link.swim_stroke_timer = 0;
                    hal_audio_play_sound(SOUND_SWIM_STROKE, 0.85f, 1.0f);
                }
            } else {
                link.swim_stroke_timer = 0;
            }
        }

        // --------------------------------------------------------------------
        // LÓGICA DE ESCALADA EM PAREDÕES & VINHAS (GRIP RING)
        // --------------------------------------------------------------------
        bool on_climbable = map_is_climbable(active_map, link.x + 8.0f, link.y + 12.0f);
        if (on_climbable && link.has_grip_ring && !dungeon_is_active() && !dungeon_flames_is_active() && !dungeon_fortress_is_active() && !dungeon_droplets_is_active()) {
            if (!link.is_climbing) {
                link.is_climbing = true;
                link.climb_anim_timer = 0;
                printf("[GRIP RING] Link comecou a escalar com o Grip Ring!\n");
            }
        } else {
            if (link.is_climbing) {
                link.is_climbing = false;
                printf("[GRIP RING] Link alcancou terra firme!\n");
            }
        }

        // --------------------------------------------------------------------
        // LÓGICA DE SUPER-SALTO VERTICAL DO CAJADO DE PACCI (HOLE CATAPULT)
        // --------------------------------------------------------------------
        if (map_is_pacci_charged_hole(active_map, link.x + 8.0f, link.y + 12.0f) && !link.is_jumping) {
            map_discharge_pacci_hole(active_map, link.x + 8.0f, link.y + 12.0f);
            link.is_jumping = true;
            link.vz = 5.2f;
            link.z = 2.0f;
            entity_trigger_screen_shake(6, 2);
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.85f);
            if (!link.is_minish) {
                hal_audio_play_sound(SOUND_LINK_JUMP, 0.95f, 1.0f);
            }
            printf("[CANE OF PACCI] Link pisou no buraco energizado! Super salto vertical no ar!\n");
        }

        if (subweapon_get_current() == ITEM_ROCS_CAPE || link.has_rocs_cape) {
            rocs_cape_update(&link.x, &link.y, link.dir, link.is_moving,
                             &link.z, &link.vz, &link.is_jumping,
                             active_map, &link.hearts);
        } else if (link.is_jumping || link.z > 0.0f) {
            link.z += link.vz;
            link.vz -= 0.22f; // Gravidade
            if (link.z <= 0.0f) {
                link.z = 0.0f;
                link.vz = 0.0f;
                link.is_jumping = false;
                hal_audio_play_sound(SOUND_ROLL, 0.70f, 1.2f); // Aterrissagem
                printf("[CANE OF PACCI] Link aterrissou em seguranca apos o super-salto!\n");
            }
        }

        // --------------------------------------------------------------------
        // LÓGICA DE LODO MOVEDIÇO & AFUNDAMENTO NO PÂNTANO (CASTOR WILDS MUD)
        // --------------------------------------------------------------------
        bool in_swamp_mud = (link.z <= 0.0f) && map_is_swamp_mud(active_map, link.x + 8.0f, link.y + 12.0f);
        bool is_dashing = hal_input_is_held(KEY_B) && subweapon_get_current() == ITEM_PEGASUS_BOOTS && !link.is_swimming && !link.is_climbing;

        if (in_swamp_mud) {
            if (is_dashing) {
                // Com as Botas de Pegasus em disparada, Link corre velozmente sobre a superfície da lama sem afundar!
                link.mud_sink_timer = 0;
            } else {
                // A pé normal: afunda gradualmente na lama espessa!
                link.mud_sink_timer++;
                if (link.mud_sink_timer % 24 == 0) {
                    hal_audio_play_sound(SOUND_FOOTSTEP, 0.6f, 0.65f); // Som borbulhante de lama
                }
                if (link.mud_sink_timer >= 60) {
                    // Afundou completamente! Dano de 1 coração e respawn no ponto seguro
                    if (link.hearts > 1) link.hearts -= 1;
                    link.invuln_timer = 45;
                    link.x = (link.last_safe_x > 0.0f) ? link.last_safe_x : 520.0f;
                    link.y = (link.last_safe_y > 0.0f) ? link.last_safe_y : 224.0f;
                    link.mud_sink_timer = 0;
                    entity_trigger_screen_shake(10, 2);
                    hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 0.8f);
                    if (!link.is_minish) {
                        hal_audio_play_sound(SOUND_LINK_FALL, 1.0f, 1.0f);
                    }
                    printf("[SWAMP HAZARD] Link afundou no lodo movedico! Dano e respawn em solo firme!\n");
                }
            }
        } else {
            // Em solo firme, reseta o afundamento e atualiza última posição segura
            link.mud_sink_timer = 0;
            if (!link.is_swimming && !link.is_jumping) {
                link.last_safe_x = link.x;
                link.last_safe_y = link.y;
            }
        }

        // Modificador de Velocidade: Dash / Pegasus Boots (Botao B segurado), Planeio da Capa de Roc, Lodo do Pântano, Carga da Espada, Natação ou Escalada
        float dash_mult = 1.0f;
        if (is_dashing) {
            dash_mult = 1.85f; // Arrancada veloz das Botas de Pegasus!
        } else if (rocs_cape_is_gliding()) {
            dash_mult = 1.45f; // Planeio aerodinâmico veloz com a Capa de Roc!
        } else if (in_swamp_mud) {
            dash_mult = 0.38f; // Lama viscosa pesada retém os passos do herói
        } else if (link.is_charging_spin) {
            dash_mult = 0.85f; // Movimentação prudente enquanto acumula energia na lâmina
        } else if (link.is_swimming) {
            dash_mult = link.is_diving ? 0.65f : 0.82f; // Arrasto hidro-dinâmico natural da água
        } else if (link.is_climbing) {
            dash_mult = 0.75f; // Velocidade de escalada vertical e lateral com firmeza
            if (link.is_moving) link.climb_anim_timer++;
        }

        link.is_moving = false;
        float actual_speed = 0.0f;
        float move_x = 0.0f;
        float move_y = 0.0f;

        if (subweapon_is_gust_active()) {
            // Durante a succao do Pote Magico, o heroi ancora no chao mas pode mirar o cone de vento
            AnalogStick stick = hal_input_get_left_stick();
            if (stick.magnitude > 0.15f) {
                if (fabsf(stick.x) > fabsf(stick.y)) {
                    link.dir = (stick.x > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    link.dir = (stick.y > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                if (hal_input_is_held(KEY_LEFT))  link.dir = DIR_LEFT;
                if (hal_input_is_held(KEY_RIGHT)) link.dir = DIR_RIGHT;
                if (hal_input_is_held(KEY_UP))    link.dir = DIR_UP;
                if (hal_input_is_held(KEY_DOWN))  link.dir = DIR_DOWN;
            }
            subweapon_use_held(link.x, link.y, link.dir);
            link.is_moving = false;
        } else if (link.is_rolling) {
            actual_speed = link.speed * 1.80f;
            float r_dx = 0.0f, r_dy = 0.0f;
            if (link.dir == DIR_DOWN)  r_dy = 1.0f;
            if (link.dir == DIR_UP)    r_dy = -1.0f;
            if (link.dir == DIR_LEFT)  r_dx = -1.0f;
            if (link.dir == DIR_RIGHT) r_dx = 1.0f;
            move_x = r_dx * actual_speed;
            move_y = r_dy * actual_speed;
            link.is_moving = true;
        } else if (!link.is_attacking && !link.is_spinning) {
            AnalogStick stick = hal_input_get_left_stick();

            if (stick.magnitude > 0.08f) {
                // ============================================================
                // MODO ANALÓGICO PROPORCIONAL: 360° com aceleração gradual!
                // ============================================================
                actual_speed = link.speed * stick.magnitude * dash_mult;
                move_x = stick.x * actual_speed;
                move_y = stick.y * actual_speed;
                link.is_moving = true;

                // Define a orientação do sprite pelo eixo de maior inclinação
                if (fabsf(stick.x) > fabsf(stick.y)) {
                    link.dir = (stick.x > 0.0f) ? DIR_RIGHT : DIR_LEFT;
                } else {
                    link.dir = (stick.y > 0.0f) ? DIR_DOWN : DIR_UP;
                }
            } else {
                // ============================================================
                // MODO DIGITAL: D-Pad da Cruz ou Teclado (WASD / Setas)
                // ============================================================
                float dx = 0.0f;
                float dy = 0.0f;

                if (hal_input_is_held(KEY_LEFT))  { dx -= 1.0f; link.dir = DIR_LEFT; }
                if (hal_input_is_held(KEY_RIGHT)) { dx += 1.0f; link.dir = DIR_RIGHT; }
                if (hal_input_is_held(KEY_UP))    { dy -= 1.0f; link.dir = DIR_UP; }
                if (hal_input_is_held(KEY_DOWN))  { dy += 1.0f; link.dir = DIR_DOWN; }

                if (dx != 0.0f || dy != 0.0f) {
                    // Normalização diagonal (1 / sqrt(2) ≈ 0.7071)
                    if (dx != 0.0f && dy != 0.0f) {
                        dx *= 0.7071f;
                        dy *= 0.7071f;
                    }
                    actual_speed = link.speed * dash_mult;
                    move_x = dx * actual_speed;
                    move_y = dy * actual_speed;
                    link.is_moving = true;
                }
            }
        }

        // --------------------------------------------------------------------
        // SISTEMA DE COLISÃO COM O MAPA OU MASMORRA (DESLIZAMENTO SUAVE EM X E Y)
        // --------------------------------------------------------------------
        if (link.is_moving) {
            float new_x = link.x + move_x;
            float new_y = link.y + move_y;

            bool blocked_x = false;
            bool blocked_y = false;

            float off_x1 = link.is_minish ? 6.0f : 4.0f;
            float off_x2 = link.is_minish ? 10.0f : 12.0f;
            float off_y1 = link.is_minish ? 13.0f : 12.0f;
            float off_y2 = link.is_minish ? 15.0f : 16.0f;

            blocked_x = is_world_solid_for_player(active_map, new_x + off_x1, link.y + off_y1, link.is_minish, link.has_flippers, link.has_grip_ring, link.z) ||
                        is_world_solid_for_player(active_map, new_x + off_x2, link.y + off_y1, link.is_minish, link.has_flippers, link.has_grip_ring, link.z) ||
                        is_world_solid_for_player(active_map, new_x + off_x1, link.y + off_y2, link.is_minish, link.has_flippers, link.has_grip_ring, link.z) ||
                        is_world_solid_for_player(active_map, new_x + off_x2, link.y + off_y2, link.is_minish, link.has_flippers, link.has_grip_ring, link.z);
            blocked_y = is_world_solid_for_player(active_map, link.x + off_x1, new_y + off_y1, link.is_minish, link.has_flippers, link.has_grip_ring, link.z) ||
                        is_world_solid_for_player(active_map, link.x + off_x2, new_y + off_y1, link.is_minish, link.has_flippers, link.has_grip_ring, link.z) ||
                        is_world_solid_for_player(active_map, link.x + off_x1, new_y + off_y2, link.is_minish, link.has_flippers, link.has_grip_ring, link.z) ||
                        is_world_solid_for_player(active_map, link.x + off_x2, new_y + off_y2, link.is_minish, link.has_flippers, link.has_grip_ring, link.z);

            if (!blocked_x) {
                link.x = new_x;
            }
            if (!blocked_y) {
                link.y = new_y;
            }

            if (dungeon_is_active() || dungeon_flames_is_active() || dungeon_fortress_is_active() || dungeon_droplets_is_active()) {
                if (link.x < 0.0f) link.x = 0.0f;
                if (link.x > 256.0f - 16.0f) link.x = 256.0f - 16.0f;
                if (link.y < 0.0f) link.y = 0.0f;
                if (link.y > 160.0f - 16.0f) link.y = 160.0f - 16.0f;
            } else {
                // Confinamento dentro dos limites do mapa ativo (Overworld ou Hyrule Town)
                if (link.x < 16.0f) link.x = 16.0f;
                if (link.x > (active_map->width * TILE_SIZE) - 32.0f) link.x = (active_map->width * TILE_SIZE) - 32.0f;
                if (link.y < 16.0f) link.y = 16.0f;
                if (link.y > (active_map->height * TILE_SIZE) - 32.0f) link.y = (active_map->height * TILE_SIZE) - 32.0f;

                // Transições de Mapa
                if (s_in_village) {
                    // Saída sul da Vila dos Minish de volta ao Tronco Oco
                    if (link.x >= 232.0f && link.x <= 264.0f && link.y >= (active_map->height * TILE_SIZE) - 36.0f && link.dir == DIR_DOWN) {
                        transition_to_woods_from_village(&link, world_map);
                    }
                } else if (s_in_town) {
                    // Porta da Biblioteca Real de Hyrule Town (Norte da Praça, x: 230..260, y <= 130)
                    if (link.x >= 230.0f && link.x <= 260.0f && link.y <= 130.0f && link.dir == DIR_UP) {
                        transition_to_library(&link);
                    }
                    // Portão Sul de Hyrule Town -> South Hyrule Field
                    else if (link.x >= 240.0f && link.x <= 320.0f && link.y >= (active_map->height * TILE_SIZE) - 36.0f && link.dir == DIR_DOWN) {
                        transition_to_south_field(&link);
                    }
                    // Portão Norte de Hyrule Town -> North Hyrule Field
                    else if (link.x >= 220.0f && link.x <= 300.0f && link.y <= 36.0f && link.dir == DIR_UP) {
                        transition_to_north_field(&link);
                    }
                } else if (s_in_library) {
                    // Saída Sul da Biblioteca Real de volta para Hyrule Town
                    if (link.y >= (active_map->height * TILE_SIZE) - 36.0f && link.dir == DIR_DOWN) {
                        transition_to_town_from_library(&link);
                    }
                } else if (s_in_south_field) {
                    // Portão Norte de South Field de volta para Hyrule Town
                    if (link.x >= 210.0f && link.x <= 290.0f && link.y <= 24.0f && link.dir == DIR_UP) {
                        transition_to_town_from_south(&link);
                    }
                    // Estrada Leste de South Field para Minish Woods
                    else if (link.x >= (active_map->width * TILE_SIZE) - 36.0f && link.y >= 140.0f && link.y <= 220.0f && link.dir == DIR_RIGHT) {
                        transition_to_overworld(&link, world_map);
                    }
                    // Estrada Leste (inferior) de South Field para Lake Hylia
                    else if (link.x >= (active_map->width * TILE_SIZE) - 36.0f && link.y >= 240.0f && link.dir == DIR_RIGHT) {
                        transition_to_lake_hylia(&link);
                    }
                    // Estrada Oeste de South Field para Castor Wilds Swamp
                    else if (link.x <= 24.0f && link.y >= 180.0f && link.y <= 260.0f && link.dir == DIR_LEFT) {
                        transition_to_castor_wilds(&link);
                    }
                } else if (s_in_lake_hylia) {
                    // Saída Oeste do Lago Hylia de volta para South Hyrule Field
                    if (link.x <= 24.0f && link.dir == DIR_LEFT) {
                        transition_to_south_field_from_lake(&link);
                    }
                    // Entrada glacial para Temple of Droplets (Dungeon 4)
                    else if (link.is_minish && link.x >= 430.0f && link.x <= 480.0f && link.y <= 64.0f && link.dir == DIR_UP) {
                        s_in_lake_hylia = false;
                        dungeon_droplets_enter(&link.x, &link.y, &link.dir);
                    }
                } else if (s_in_castor_wilds) {
                    // Saída Leste do Pântano de Castor Wilds:
                    // Se y <= 140: leva a Wind Ruins (Ruínas do Vento)
                    // Se y > 140: leva a South Hyrule Field
                    if (link.x >= (active_map->width * TILE_SIZE) - 32.0f && link.dir == DIR_RIGHT) {
                        if (link.y <= 140.0f) {
                            transition_to_wind_ruins(&link);
                        } else {
                            transition_to_south_field_from_castor(&link);
                        }
                    }
                    // Entrada para a Caverna das Luvas de Toupeira no noroeste de Castor Wilds
                    else if (link.x <= 64.0f && link.y <= 64.0f && link.dir == DIR_UP) {
                        transition_to_mole_cave(&link);
                    }
                } else if (s_in_mole_cave) {
                    // Saída Sul da Caverna das Luvas de Toupeira de volta para Castor Wilds
                    if (link.y >= (active_map->height * TILE_SIZE) - 32.0f && link.dir == DIR_DOWN) {
                        transition_to_castor_from_cave(&link);
                    }
                } else if (s_in_wind_ruins) {
                    // Saída Oeste de Wind Ruins de volta para Castor Wilds
                    if (link.x <= 24.0f && link.dir == DIR_LEFT) {
                        transition_to_castor_from_ruins(&link);
                    }
                    // Portão Leste monumental para Fortress of Winds (Dungeon 3)
                    else if (link.x >= (active_map->width * TILE_SIZE) - 32.0f && link.y >= 40.0f && link.y <= 120.0f && link.dir == DIR_RIGHT) {
                        dungeon_fortress_enter(&link.x, &link.y, &link.dir);
                    }
                    // Infiltração Minish no Robô Armos adormecido
                    if (link.is_minish && (hal_input_is_pressed(KEY_A) || hal_input_is_pressed(KEY_R))) {
                        Entity* armos = entity_find_nearby_armos(link.x, link.y, 28.0f);
                        if (armos) {
                            transition_to_armos_interior(&link);
                        }
                    }
                } else if (s_in_armos_interior) {
                    // Saída Sul do interior do Armos de volta para Wind Ruins
                    if (link.y >= (active_map->height * TILE_SIZE) - 32.0f && link.dir == DIR_DOWN) {
                        transition_to_wind_ruins_from_armos(&link);
                    }
                } else if (s_in_north_field) {
                    // Portão Sul de North Field de volta para Hyrule Town
                    if (link.x >= 210.0f && link.x <= 290.0f && link.y >= (active_map->height * TILE_SIZE) - 36.0f && link.dir == DIR_DOWN) {
                        transition_to_town_from_north(&link);
                    }
                    // Estrada Oeste de North Field para Mount Crenel Base
                    else if (link.x <= 24.0f && link.y >= 140.0f && link.y <= 240.0f && link.dir == DIR_LEFT) {
                        transition_to_crenel_base(&link);
                    }
                    // Portal Norte de North Field para o Santuario Elemental (Hyrule Castle Courtyard)
                    else if (link.x >= 210.0f && link.x <= 290.0f && link.y <= 24.0f && link.dir == DIR_UP) {
                        transition_to_sanctuary(&link);
                    }
                    // Desfiladeiro Nordeste de North Field para Veil Falls (Quedas do Veu)
                    else if (link.x >= 400.0f && link.y <= 36.0f && link.dir == DIR_UP) {
                        s_in_north_field = false;
                        veil_clouds_enter_falls(&link.x, &link.y, &link.dir);
                    }
                } else if (s_in_crenel_base) {
                    // Estrada Leste de Mount Crenel Base de volta para North Field
                    if (link.x >= (active_map->width * TILE_SIZE) - 32.0f && link.dir == DIR_RIGHT) {
                        transition_to_north_field_from_crenel(&link);
                    }
                    // Entrada Norte para as Minas de Melari (x=260..320, y<=48, DIR_UP)
                    else if (link.x >= 260.0f && link.x <= 320.0f && link.y <= 48.0f && link.dir == DIR_UP) {
                        transition_to_melari_mines(&link);
                    }
                } else if (s_in_melari_mines) {
                    // Saída Sul das Minas de Melari de volta para Mount Crenel Base
                    if (link.y >= (active_map->height * TILE_SIZE) - 32.0f && link.dir == DIR_DOWN) {
                        transition_to_crenel_base_from_melari(&link);
                    }
                    // Portal em arco ao norte: Entrada da Cave of Flames (Dungeon 2)
                    else if (link.x >= 220.0f && link.x <= 280.0f && link.y <= 36.0f && link.dir == DIR_UP) {
                        transition_to_cave_of_flames(&link);
                    }
                } else {
                    // Em Minish Woods: Se estiver no tamanho Minish e atravessar a ponta norte do Tronco Oco (coluna 21)
                    if (link.is_minish && world_map && world_map->is_authentic) {
                        if (link.x >= 324.0f && link.x <= 348.0f && link.y <= 722.0f && link.dir == DIR_UP) {
                            transition_to_minish_village(&link);
                        }
                    }

                    // Checagem de Entrada no Deepwood Shrine pelo archway do santuário ao norte
                    bool enter_shrine = false;
                    if (world_map && world_map->is_authentic) {
                        if (link.x >= 436.0f && link.x <= 468.0f && link.y <= 485.0f && link.dir == DIR_UP) {
                            enter_shrine = true;
                        }
                    } else {
                        if (link.x >= 280.0f && link.x <= 312.0f && link.y <= 40.0f && link.dir == DIR_UP) {
                            enter_shrine = true;
                        }
                    }

                    if (enter_shrine) {
                        entity_clear_all();
                        dungeon_enter(&link.x, &link.y, &link.dir);
                    }
                }
            }

            // Checagem de Entrada em Passagem Secreta revelada por explosão de Bomba
            if (active_map && active_map->overlay_layer) {
                int l_tx = (int)((link.x + 8.0f) / TILE_SIZE);
                int l_ty = (int)((link.y + 12.0f) / TILE_SIZE);
                if (l_tx >= 0 && l_tx < active_map->width && l_ty >= 0 && l_ty < active_map->height) {
                    int idx = l_ty * active_map->width + l_tx;
                    if (active_map->overlay_layer[idx] == TILE_SECRET_ENTRANCE) {
                        static int s_secret_cooldown = 0;
                        if (s_secret_cooldown > 0) s_secret_cooldown--;
                        if (s_secret_cooldown <= 0) {
                            s_secret_cooldown = 180;
                            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
                            link.rupees += 50;
                            link.hearts = link.max_hearts;
                            subweapon_add_bombs(5);
                            printf("[SECRET ROOM] Link adentrou a alcova secreta da fada! +50 Rupees, +5 Bombas e Vida Restaurada!\n");
                        }
                    }
                }
            }
        }

        // Inércia física de deslizamento sobre gelo no Temple of Droplets
        if (dungeon_droplets_is_active() && dungeon_droplets_is_ice_tile(link.x + 8.0f, link.y + 12.0f)) {
            dungeon_droplets_apply_ice_physics(&link.x, &link.y, link.speed, link.dir, link.is_moving);
        }

        // --------------------------------------------------------------------
        // FÍSICA DE KNOCKBACK DO HERÓI (RECÚO AO RECEBER DANO)
        // --------------------------------------------------------------------
        if (fabsf(link.knock_x) > 0.05f || fabsf(link.knock_y) > 0.05f) {
            float k_new_x = link.x + link.knock_x;
            float k_new_y = link.y + link.knock_y;

            bool k_blocked_x = false;
            bool k_blocked_y = false;

            float off_x1 = link.is_minish ? 6.0f : 4.0f;
            float off_x2 = link.is_minish ? 10.0f : 12.0f;
            float off_y1 = link.is_minish ? 13.0f : 12.0f;
            float off_y2 = link.is_minish ? 15.0f : 16.0f;

            k_blocked_x = is_world_solid_for_player(active_map, k_new_x + off_x1, link.y + off_y1, link.is_minish, link.has_flippers, link.has_grip_ring, link.z) ||
                          is_world_solid_for_player(active_map, k_new_x + off_x2, link.y + off_y1, link.is_minish, link.has_flippers, link.has_grip_ring, link.z);
            k_blocked_y = is_world_solid_for_player(active_map, link.x + off_x1, k_new_y + off_y1, link.is_minish, link.has_flippers, link.has_grip_ring, link.z) ||
                          is_world_solid_for_player(active_map, link.x + off_x2, k_new_y + off_y2, link.is_minish, link.has_flippers, link.has_grip_ring, link.z);

            if (!k_blocked_x) link.x = k_new_x;
            if (!k_blocked_y) link.y = k_new_y;

            link.knock_x *= 0.82f; // Amortecimento de inércia do recuo
            link.knock_y *= 0.82f;
        } else {
            link.knock_x = 0.0f;
            link.knock_y = 0.0f;
        }

        if (link.invuln_timer > 0) {
            link.invuln_timer--;
        }

        // Perigo Ambiental: Canais de Lava Incandescente em Melari's Mines
        if (s_in_melari_mines && map_is_lava(active_map, link.x + 8.0f, link.y + 12.0f)) {
            if (link.invuln_timer <= 0 && !link.is_jumping) {
                if (link.hearts > 1) link.hearts -= 1;
                link.invuln_timer = 60;
                link.knock_x = (link.dir == DIR_LEFT) ? 4.0f : (link.dir == DIR_RIGHT ? -4.0f : 0.0f);
                link.knock_y = (link.dir == DIR_UP) ? 4.0f : -4.0f;
                hal_audio_play_sound(SOUND_SWORD_HIT, 1.0f, 0.8f);
                entity_trigger_screen_shake(12, 3);
                printf("[LAVA HAZARD] Link tocou na lava incandescente! Dano recebido!\n");
            }
        }

        // --------------------------------------------------------------------
        // ATUALIZAÇÃO DO SUBSISTEMA DE MASMORRA & ENTIDADES
        // --------------------------------------------------------------------
        static bool s_was_dungeon_active = false;
        if (s_was_dungeon_active && !dungeon_is_active()) {
            if (s_in_melari_mines) {
                spawn_melari_mines_entities();
            } else if (s_in_village) {
                spawn_minish_village_entities();
            } else if (s_in_town) {
                spawn_town_entities();
            } else if (s_in_south_field) {
                spawn_south_field_entities();
            } else if (s_in_north_field) {
                spawn_north_field_entities();
            } else if (s_in_crenel_base) {
                spawn_crenel_base_entities();
            } else if (s_in_castor_wilds) {
                spawn_castor_wilds_entities();
            } else if (s_in_mole_cave) {
                spawn_mole_cave_entities(link.has_mole_mitts);
            } else if (s_in_wind_ruins) {
                spawn_wind_ruins_entities(armos_circuit_is_active());
            } else if (s_in_armos_interior) {
                spawn_armos_interior_entities(armos_circuit_is_active());
            } else {
                spawn_overworld_entities(world_map);
            }
        }
        s_was_dungeon_active = dungeon_is_active();

        static bool s_was_dungeon_flames_active = false;
        if (s_was_dungeon_flames_active && !dungeon_flames_is_active()) {
            s_in_melari_mines = true;
            spawn_melari_mines_entities();
            hal_audio_play_bgm(BGM_MINISH_WOODS);
        }
        s_was_dungeon_flames_active = dungeon_flames_is_active();

        static bool s_was_sanctuary_active = false;
        if (s_was_sanctuary_active && !sanctuary_is_active()) {
            if (!royal_valley_is_active()) {
                s_in_north_field = true;
                spawn_north_field_entities();
                hal_audio_play_bgm(BGM_MINISH_WOODS);
            }
        }
        s_was_sanctuary_active = sanctuary_is_active();

        static bool s_was_royal_valley_active = false;
        if (s_was_royal_valley_active && !royal_valley_is_active()) {
            sanctuary_enter(&link.x, &link.y, &link.dir);
            link.y = 2.0f * TILE_SIZE;
            link.dir = DIR_DOWN;
        }
        s_was_royal_valley_active = royal_valley_is_active();

        static bool s_was_dungeon_fortress_active = false;
        if (s_was_dungeon_fortress_active && !dungeon_fortress_is_active()) {
            s_in_wind_ruins = true;
            spawn_wind_ruins_entities(armos_circuit_is_active());
            hal_audio_play_bgm(BGM_MINISH_WOODS);
        }
        s_was_dungeon_fortress_active = dungeon_fortress_is_active();

        if (dungeon_is_active()) {
            dungeon_update(&link.x, &link.y, &link.dir, link.is_moving,
                           &link.hearts, &link.rupees);
        } else if (dungeon_flames_is_active()) {
            dungeon_flames_update(&link.x, &link.y, &link.dir, link.is_moving,
                                  &link.hearts, &link.rupees);
            if (dungeon_flames_has_fire_element() && !link.has_fire_element) {
                link.has_fire_element = true;
                link.fire_element_banner_timer = 220;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                printf("[FIRE ELEMENT] Link conquistou o segundo elemento sagrado: ELEMENTO FOGO!\n");
            }
        } else if (sanctuary_is_active()) {
            sanctuary_update(&link.x, &link.y, &link.dir, link.is_moving,
                             link.is_charging_spin, link.spin_ready,
                             link.is_attacking, link.attack_timer,
                             &link.hearts);
            if (sanctuary_has_four_elements() && !link.has_four_sword) {
                link.has_four_sword = true;
                link.has_three_elements = true;
                link.has_two_elements = true;
                link.four_sword_banner_timer = 240;
                inventory_set_four_sword(true);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.6f);
                printf("[FOUR SWORD] FOUR SWORD COMPLETA FORJADA! 4 Herois simultaneos & Sword Beams ativados!\n");
            } else if (sanctuary_has_three_elements() && !link.has_three_elements) {
                link.has_three_elements = true;
                link.three_elements_banner_timer = 240;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.4f);
                printf("[FOUR SWORD] White Sword infundida com Terra, Fogo e Agua! Divisao em 3 Clones!\n");
            } else if (sanctuary_has_two_elements() && !link.has_two_elements) {
                link.has_two_elements = true;
                link.two_elements_banner_timer = 240;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
                printf("[FOUR SWORD] White Sword infundida com Terra e Fogo! Divisao em 2 Clones!\n");
            }
            if (sanctuary_has_stepped_in_secret_exit(link.x, link.y)) {
                sanctuary_exit(&link.x, &link.y, &link.dir);
                royal_valley_enter_entrance(&link.x, &link.y, &link.dir);
            }
        } else if (dungeon_fortress_is_active()) {
            bool warp_to_surface = false;
            dungeon_fortress_update(&link.x, &link.y, &link.dir, link.is_moving,
                                    &link.hearts, &link.rupees, link.is_minish,
                                    link.is_attacking, &warp_to_surface);
            if (dungeon_fortress_has_ocarina() && !link.has_ocarina) {
                link.has_ocarina = true;
                link.ocarina_banner_timer = 220;
                inventory_unlock_item(INV_ITEM_OCARINA);
                subweapon_set_current(ITEM_OCARINA_OF_WIND);
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.2f);
                printf("[OCARINA] Link obteve a lendaria Ocarina do Vento (Ocarina of Wind)!\n");
            }
            if (warp_to_surface) {
                dungeon_fortress_exit(&link.x, &link.y, &link.dir);
                s_in_wind_ruins = true;
                spawn_wind_ruins_entities(armos_circuit_is_active());
            }
        } else if (dungeon_droplets_is_active()) {
            bool prev_water = link.has_water_element;
            dungeon_droplets_update(&link.x, &link.y, link.dir, link.is_moving,
                                    &link.hearts, link.max_hearts, &link.rupees, &link.has_water_element);
            if (link.has_water_element && !prev_water) {
                link.water_element_banner_timer = 240;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
            }
        } else if (veil_clouds_is_active()) {
            veil_clouds_update(&link.x, &link.y, link.dir, link.is_moving,
                               &link.hearts, &link.rupees, link.has_mole_mitts);
            if (veil_clouds_is_tornado_active() && !link.cloud_tornado_active) {
                link.cloud_tornado_active = true;
                link.cloud_banner_timer = 240;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
                printf("[PALACE OF WINDS] O GRANDE TORNADO PARA O PALACIO DO VENTO FOI DESPERTADO!\n");
            }
        } else if (dungeon_palace_is_active()) {
            RocsCapeState* rc = rocs_cape_get_state();
            bool is_downthrust = rc ? rc->is_down_thrust : false;
            bool is_gliding = rc ? rc->is_gliding : false;
            bool prev_wind = link.has_wind_element;
            dungeon_palace_update(&link.x, &link.y, &link.dir, link.is_moving,
                                  &link.hearts, link.max_hearts, &link.rupees,
                                  &link.has_wind_element, link.has_rocs_cape,
                                  link.is_jumping, is_gliding,
                                  is_downthrust, link.is_attacking);
            if (link.has_wind_element && !prev_wind) {
                link.wind_element_banner_timer = 240;
                link.dungeon_palace_cleared = true;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
            }
        } else if (royal_valley_is_active()) {
            royal_valley_update(&link.x, &link.y, &link.dir, link.is_moving,
                                &link.hearts, link.max_hearts, &link.rupees,
                                link.is_charging_spin, link.spin_ready,
                                link.is_attacking, link.attack_timer,
                                link.has_four_sword, link.has_lantern, lantern_is_lit());
            if (royal_valley_has_royal_kinstone() && !link.has_royal_kinstone) {
                link.has_royal_kinstone = true;
                link.royal_kinstone_banner_timer = 240;
            }
        } else if (dark_castle_is_active()) {
            float cl_x[3] = { 0 };
            float cl_y[3] = { 0 };
            int cl_cnt = 0;
            for (int c = 0; c < 3; c++) {
                CloneState* cl = sanctuary_get_clone_at(c);
                if (cl && cl->active) {
                    cl_x[cl_cnt] = cl->x;
                    cl_y[cl_cnt] = cl->y;
                    cl_cnt++;
                }
            }
            dark_castle_update(&link.x, &link.y, &link.dir,
                               link.is_attacking, link.has_four_sword, cl_cnt,
                               cl_x, cl_y, &link.hearts, NULL);
            if (dark_castle_is_ready_for_vaati() && !link.sanctum_banner_shown) {
                link.sanctum_banner_shown = true;
                link.sanctum_banner_timer = 240;
                hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.5f);
            }
        } else if (vaati_boss_is_active()) {
            float cl_x[3] = { 0 };
            float cl_y[3] = { 0 };
            int cl_cnt = 0;
            for (int c = 0; c < 3; c++) {
                CloneState* cl = sanctuary_get_clone_at(c);
                if (cl && cl->active) {
                    cl_x[cl_cnt] = cl->x;
                    cl_y[cl_cnt] = cl->y;
                    cl_cnt++;
                }
            }
            vaati_boss_update(&link.x, &link.y, &link.dir,
                             link.is_attacking, link.is_spinning, cl_cnt,
                             cl_x, cl_y, link.is_minish, &link.hearts);
        }

        static bool s_was_dungeon_droplets_active = false;
        if (s_was_dungeon_droplets_active && !dungeon_droplets_is_active()) {
            s_in_lake_hylia = true;
            spawn_lake_hylia_entities();
            hal_audio_play_bgm(BGM_MINISH_WOODS);
        }
        s_was_dungeon_droplets_active = dungeon_droplets_is_active();

        static bool s_was_dungeon_palace_active = false;
        if (s_was_dungeon_palace_active && !dungeon_palace_is_active()) {
            veil_clouds_enter_clouds(&link.x, &link.y, &link.dir);
            hal_audio_play_bgm(BGM_MINISH_WOODS);
        }
        s_was_dungeon_palace_active = dungeon_palace_is_active();

        static bool s_was_veil_clouds_active = false;
        if (s_was_veil_clouds_active && !veil_clouds_is_active()) {
            s_in_north_field = true;
            spawn_north_field_entities();
            hal_audio_play_bgm(BGM_MINISH_WOODS);
        }
        s_was_veil_clouds_active = veil_clouds_is_active();

        if (s_in_town && cucco_minigame_is_active()) {
            cucco_minigame_update(&link.x, &link.y, link.dir, &link.is_carrying_cucco,
                                  &link.rupees, &link.shells, &link.hearts, link.max_hearts);
        }
        sword_dojo_update();

        entity_manager_update(active_map, link.x, link.y,
                              &link.hearts, &link.max_hearts, &link.rupees,
                              &link.invuln_timer, &link.knock_x, &link.knock_y);

        // Atualizacao do Subsistema de Subarmas (Bumerangue, Vórtice do Pote Magico, Projeteis)
        subweapon_update(active_map, link.x, link.y, &link.rupees, &link.hearts);

        // Atualização dos Raios de Espada da Four Sword (Sword Beams)
        sanctuary_update_sword_beams(active_map);

        if (veil_clouds_is_active() && veil_clouds_is_in_clouds() && subweapon_is_digging()) {
            float dig_x = link.x + (link.dir == DIR_RIGHT ? 14.0f : (link.dir == DIR_LEFT ? -14.0f : 0.0f));
            float dig_y = link.y + (link.dir == DIR_DOWN ? 16.0f : (link.dir == DIR_UP ? -14.0f : 0.0f));
            veil_clouds_dig_cloud(dig_x, dig_y, &link.rupees);
        }

        if (dungeon_droplets_is_active()) {
            LanternState* lstate = lantern_get_state();
            if (lstate && (lstate->is_swinging || lstate->is_lit)) {
                dungeon_droplets_check_boss_lantern_hit(lstate->swing_x, lstate->swing_y, 20.0f);
            }
        }

        // Monitoramento de alteração de vida (dano/cura) e rupees para áudio reativo autêntico
        static int s_prev_hearts = -1;
        static int s_prev_rupees = -1;
        if (s_prev_hearts == -1) {
            s_prev_hearts = link.hearts;
            s_prev_rupees = link.rupees;
        } else {
            if (link.hearts < s_prev_hearts) {
                if (link.hearts <= 0) {
                    hal_audio_play_sound(SOUND_LINK_DIE, 1.0f, 1.0f);
                } else if (!link.is_minish) {
                    hal_audio_play_sound(SOUND_LINK_HURT, 1.0f, 1.0f);
                }
            } else if (link.hearts > s_prev_hearts) {
                hal_audio_play_sound(SOUND_HEART_GET, 1.0f, 1.0f);
            }
            if (link.rupees > s_prev_rupees) {
                hal_audio_play_sound(SOUND_RUPEE_GET, 1.0f, 1.0f);
            }
            s_prev_hearts = link.hearts;
            s_prev_rupees = link.rupees;
        }

        // Respawn de teste caso o Link zere os corações
        if (link.hearts <= 0) {
            link.hearts = link.max_hearts;
            s_prev_hearts = link.max_hearts;
            if (dungeon_is_active() || dungeon_flames_is_active() || sanctuary_is_active()) {
                link.x = 7.5f * TILE_SIZE;
                link.y = 8.0f * TILE_SIZE;
            } else if (s_in_village) {
                link.x = 248.0f;
                link.y = 352.0f;
            } else if (s_in_town) {
                link.x = 280.0f;
                link.y = 392.0f;
            } else if (s_in_south_field) {
                link.x = 248.0f;
                link.y = 36.0f;
            } else if (s_in_north_field) {
                link.x = 248.0f;
                link.y = 330.0f;
            } else if (s_in_castor_wilds) {
                link.x = 520.0f;
                link.y = 224.0f;
            } else if (s_in_mole_cave) {
                link.x = 256.0f;
                link.y = 350.0f;
            } else {
                link.x = (world_map && world_map->is_authentic) ? 448.0f : 296.0f;
                link.y = (world_map && world_map->is_authentic) ? 636.0f : 176.0f;
            }
            link.invuln_timer = 90;
            link.knock_x = 0.0f;
            link.knock_y = 0.0f;
            hal_audio_play_sound(SOUND_SECRET, 0.7f, 0.8f);
        }

        // Atualização da animação dos passos com frequência dinâmica proporcional (Ciclo fluido de 10 quadros)
        if (link.is_moving && actual_speed > 0.0f) {
            link.anim_timer += (int)(actual_speed * 6.0f + 0.5f);
            int step_thresh = link.is_minish ? 16 : 24;
            if (link.anim_timer >= step_thresh) {
                link.anim_frame = (link.anim_frame + 1) % 10;
                link.anim_timer = 0;
                float pitch_mult = link.is_minish ? 1.38f : 1.0f;
                if (link.anim_frame == 0) {
                    hal_audio_play_sound(SOUND_FOOTSTEP, 0.45f, 0.94f * pitch_mult);
                } else if (link.anim_frame == 5) {
                    hal_audio_play_sound(SOUND_FOOTSTEP, 0.45f, 1.06f * pitch_mult);
                }
            }
        } else {
            link.anim_frame = 0;
            link.anim_timer = 0;
        }
        } // Fim do bloco de gameplay (se não estiver em diálogo ativo)

        // Decrementa temporizadores de banners festivos de conquistas/itens a cada quadro
        if (link.tiger_scroll_banner_timer > 0) link.tiger_scroll_banner_timer--;
        if (link.white_sword_banner_timer > 0) link.white_sword_banner_timer--;
        if (link.fire_element_banner_timer > 0) link.fire_element_banner_timer--;
        if (link.water_element_banner_timer > 0) link.water_element_banner_timer--;
        if (link.wind_element_banner_timer > 0) link.wind_element_banner_timer--;
        if (link.two_elements_banner_timer > 0) link.two_elements_banner_timer--;
        if (link.three_elements_banner_timer > 0) link.three_elements_banner_timer--;
        if (link.four_sword_banner_timer > 0) link.four_sword_banner_timer--;
        if (link.royal_kinstone_banner_timer > 0) link.royal_kinstone_banner_timer--;
        if (link.sanctum_banner_timer > 0) link.sanctum_banner_timer--;
        if (link.veil_banner_timer > 0) link.veil_banner_timer--;
        if (link.cloud_banner_timer > 0) link.cloud_banner_timer--;
        if (link.bow_banner_timer > 0) link.bow_banner_timer--;
        if (link.mole_mitts_banner_timer > 0) link.mole_mitts_banner_timer--;
        if (link.armos_banner_timer > 0) link.armos_banner_timer--;
        if (link.ocarina_banner_timer > 0) link.ocarina_banner_timer--;
        if (link.lantern_banner_timer > 0) link.lantern_banner_timer--;
        if (link.rocs_banner_timer > 0) link.rocs_banner_timer--;

        // Atualização da Câmera Virtual Widescreen (Segue o Link ou centraliza na Masmorra / Santuário)
        if (dungeon_is_active() || dungeon_flames_is_active() || dungeon_fortress_is_active() || dungeon_droplets_is_active() || dungeon_palace_is_active() || veil_clouds_is_active() || sanctuary_is_active()) {
            camera.viewport_w = widescreen ? 284 : 240;
            camera.viewport_h = 160;
            camera.x = (float)(256 - camera.viewport_w) / 2.0f;
            camera.y = 0.0f;
        } else {
            camera_update(&camera, link.x, link.y, ctx->render_width, ctx->render_height, active_map);
        }

        // Aplicação do tremor de tela (Screen Shake) causado por explosões de bombas e impacto do Chefe
        int shake_x = 0, shake_y = 0;
        entity_get_screen_shake(&shake_x, &shake_y);
        int b_shake_x = 0, b_shake_y = 0;
        subweapon_get_screen_shake(&b_shake_x, &b_shake_y);
        camera.x += (float)(shake_x + b_shake_x);
        camera.y += (float)(shake_y + b_shake_y);
        } // Fim do if (!inventory_is_paused())

        // --------------------------------------------------------------------
        // 3. RENDERIZAÇÃO NO FRAMEBUFFER VIRTUAL
        // --------------------------------------------------------------------
        if (dungeon_is_active()) {
            // Renderiza as câmaras subterrâneas, tochas, canais e portas da masmorra
            dungeon_render(&camera);
            // Renderiza inimigos ativos da câmara (Keese / ChuChu)
            entity_manager_render(&camera);
            // Desenha Link
            draw_link(&link, &camera);
            // Renderiza subarmas ativas (Bumerangue / Vórtice)
            subweapon_render(&camera);
            // Renderiza efeito suave de transição de salas (wipe)
            dungeon_render_transition(&camera);
        } else if (dungeon_flames_is_active()) {
            // Renderiza Cave of Flames (lava ardente, trilhos, vagoneta, cilindro espinhoso)
            dungeon_flames_render(&camera);
            // Renderiza inimigos ativos da câmara (Fire Keese / Spiny Beetle)
            entity_manager_render(&camera);
            // Desenha Link se não estiver montado dentro da vagoneta
            if (!dungeon_flames_is_link_riding_cart()) {
                draw_link(&link, &camera);
            }
            // Renderiza subarmas ativas (Cajado de Pacci / Bumerangue / Bombas)
            subweapon_render(&camera);
            // Renderiza transição de salas
            dungeon_flames_render_transition(&camera);
        } else if (sanctuary_is_active()) {
            // Renderiza o Santuário Elemental (mármore sagrado, vitrais, altar, pisos de clones)
            sanctuary_render(&camera, link.x, link.y, link.dir);
            entity_manager_render(&camera);
            draw_link(&link, &camera);
            subweapon_render(&camera);
        } else if (dungeon_fortress_is_active()) {
            // Renderiza Fortress of Winds (Mazaal, ponte de vento, abismo, fresta minish)
            dungeon_fortress_render(&camera, link.is_minish, link.x, link.y);
            entity_manager_render(&camera);
            draw_link(&link, &camera);
            subweapon_render(&camera);
        } else if (dungeon_droplets_is_active()) {
            // Renderiza Temple of Droplets (gelo translúcido, facho solar, Big Octorok)
            dungeon_droplets_render(&camera, link.is_minish, link.x, link.y);
            entity_manager_render(&camera);
            draw_link(&link, &camera);
            subweapon_render(&camera);
        } else if (dungeon_palace_is_active()) {
            // Renderiza Palace of Winds (turbinas, abismos, Roc's Cape crossing, Gyorg Pair)
            dungeon_palace_render(&camera, link.x, link.y, link.is_minish);
            entity_manager_render(&camera);
            draw_link(&link, &camera);
            subweapon_render(&camera);
        } else if (veil_clouds_is_active()) {
            veil_clouds_render(&camera, link.x, link.y);
            entity_manager_render(&camera);
            draw_link(&link, &camera);
            subweapon_render(&camera);
        } else if (royal_valley_is_active()) {
            royal_valley_render(&camera, link.x, link.y, link.dir, link.is_minish);
            entity_manager_render(&camera);
            draw_link(&link, &camera);
            subweapon_render(&camera);
        } else if (dark_castle_is_active()) {
            dark_castle_render(&camera, link.x, link.y);
            entity_manager_render(&camera);
            draw_link(&link, &camera);
            subweapon_render(&camera);
        } else if (vaati_boss_is_active()) {
            vaati_boss_render(&camera, link.x, link.y, link.is_minish);
            if (!vaati_boss_is_credits()) {
                entity_manager_render(&camera);
                draw_link(&link, &camera);
                subweapon_render(&camera);
            }
        } else {
            // 1. Renderiza o mapa com Frustum Culling inteligente
            map_render(active_map, &camera);
            if (s_in_town && cucco_minigame_is_active()) {
                cucco_minigame_render(&camera, link.x, link.y, link.dir);
            }
            // 2. Renderiza as entidades ativas (Octoroks, Projéteis e Itens no chão)
            entity_manager_render(&camera);
            // 3. Desenha a entidade do Link nas coordenadas relativas da câmera
            draw_link(&link, &camera);
            // 4. Renderiza as subarmas e efeitos em voo (Bumerangue, Vórtice de ar)
            subweapon_render(&camera);
        }

        // 4b. Anel de Choque do Impacto de Down-Thrust (Roc's Cape)
        rocs_cape_render_shockwave(&camera);

        // 4c. Raios de Espada da Four Sword (Sword Beams)
        if (!sanctuary_is_active()) {
            sanctuary_render_sword_beams(&camera);
        }

        // 2b. Máscara de Iluminação Dinâmica da Flame Lantern (Dark Rooms & Dungeons)
        lantern_render_lighting(&camera, link.x, link.y, active_map, false);

        // 3. Barra Superior de HUD (Status do Jogo fixo na tela)
        draw_rect(0, 0, ctx->render_width, 14, 0x0C1C0DFF);

        // Corações de Vida de Zelda com quartos parciais e relevo 3D (Autênticos GBA ou Procedural)
        int cur_quarters = (link.health_quarters > 0) ? link.health_quarters : (link.hearts * 4);
        if (s_hud_items_tex && s_hud_items_tex->pixels) {
            for (int h = 0; h < link.max_hearts; h++) {
                int q = cur_quarters - (h * 4);
                int h_col = 4; // Vazio por padrão
                if (q >= 4)      h_col = 0; // Cheio (4/4)
                else if (q == 3) h_col = 1; // 3/4
                else if (q == 2) h_col = 2; // 2/4 (Metade)
                else if (q == 1) h_col = 3; // 1/4
                texture_draw(s_hud_items_tex, h_col * 16, 0, 16, 16, 2 + (h * 11), -1);
            }
        } else {
            for (int h = 0; h < link.max_hearts; h++) {
                if (h < link.hearts) {
                    draw_heart(4 + (h * 9), 3);
                } else {
                    draw_empty_heart(4 + (h * 9), 3);
                }
            }
        }

        // Barra de Magia GBA 1:1 (Magic Meter) entre corações e rúpias
        if (s_hud_items_tex && s_hud_items_tex->pixels) {
            // Ponta esquerda dourada com rubi incrustado
            texture_draw(s_hud_items_tex, 0 * 16, 5 * 16, 16, 16, 36, -1);
            // Segmento 1 esmeralda ou vazio
            int seg1_col = (link.magic >= 40) ? 1 : 2;
            texture_draw(s_hud_items_tex, seg1_col * 16, 5 * 16, 16, 16, 44, -1);
            // Segmento 2 esmeralda ou vazio
            int seg2_col = (link.magic >= 80) ? 1 : 2;
            texture_draw(s_hud_items_tex, seg2_col * 16, 5 * 16, 16, 16, 52, -1);
            // Ponta direita dourada
            texture_draw(s_hud_items_tex, 3 * 16, 5 * 16, 16, 16, 60, -1);
        } else {
            draw_rect(38, 5, 26, 4, 0x064E3BFF);
            int m_fill = (link.magic * 24) / 100;
            if (m_fill > 0) draw_rect(39, 6, m_fill, 2, 0x10B981FF);
        }

        // Indicador de Controle Conectado & Mini Radar Analógico no canto direito
        int pad_hud_x = ctx->render_width - 60;
        const char* pad_name = hal_input_get_controller_name();
        u32 pad_indicator_color = pad_name ? 0x00FF66FF : 0x555555FF;
        draw_rect(pad_hud_x, 4, 10, 6, pad_indicator_color); // Ícone do controle
        hal_video_put_pixel(pad_hud_x + 1, 3, pad_indicator_color);
        hal_video_put_pixel(pad_hud_x + 8, 3, pad_indicator_color);

        // Mini Radar Analógico no HUD
        AnalogStick stick_hud = hal_input_get_left_stick();
        draw_rect(pad_hud_x + 13, 3, 9, 8, 0x1A2E1CFF);
        hal_video_put_pixel(pad_hud_x + 17, 7, 0x446644FF);
        if (stick_hud.magnitude > 0.05f) {
            int dot_x = pad_hud_x + 17 + (int)(stick_hud.x * 3.2f);
            int dot_y = 7  + (int)(stick_hud.y * 2.8f);
            u32 dot_color = (stick_hud.magnitude > 0.8f) ? 0xFFDD00FF : 0x00FFCCFF;
            hal_video_put_pixel(dot_x, dot_y, dot_color);
        }

        // Contador de Rupees (Gemas Autênticas Verdes de Zelda)
        int rupee_hud_x = 70;
        if (s_hud_items_tex && s_hud_items_tex->pixels) {
            texture_draw(s_hud_items_tex, 5 * 16, 0, 16, 16, rupee_hud_x, -1);
            char r_txt[16];
            snprintf(r_txt, sizeof(r_txt), "%d", link.rupees);
            font_draw_text(rupee_hud_x + 12, 3, r_txt, 0x00FF88FF, true);
        } else {
            draw_rect(rupee_hud_x, 4, 5, 6, 0x00FF88FF); // Gema verde
            hal_video_put_pixel(rupee_hud_x + 2, 3, 0x00FF88FF);
            hal_video_put_pixel(rupee_hud_x + 2, 10, 0x00FF88FF);
            char r_txt[16];
            snprintf(r_txt, sizeof(r_txt), "%d", link.rupees);
            font_draw_text(rupee_hud_x + 8, 4, r_txt, 0x00FF88FF, true);
        }

        // Indicador de Trilha Sonora BGM no HUD
        BgmTrack current_bgm = hal_audio_get_current_bgm();
        u32 bgm_color = (current_bgm == BGM_MINISH_WOODS)     ? 0x00E5FFFF :
                        (current_bgm == BGM_HYRULE_OVERWORLD) ? 0xFFD700FF :
                        (current_bgm == BGM_DEEPWOOD_SHRINE)  ? 0xA855F7FF :
                        (current_bgm == BGM_BOSS_BATTLE)      ? 0xEF4444FF :
                        (current_bgm == BGM_HYRULE_TOWN)      ? 0x10B981FF :
                                                                0x666666FF;
        draw_rect(106, 5, 3, 5, bgm_color);
        hal_video_put_pixel(109, 4, bgm_color);
        hal_video_put_pixel(110, 5, bgm_color);
        hal_video_put_pixel(110, 6, bgm_color);

        // Badges Canônicos [A] (Espada) e [B] (Subarma)
        int hud_badge_x = 118;
        if (s_hud_items_tex && s_hud_items_tex->pixels) {
            // Badge [A] + Espada
            texture_draw(s_hud_items_tex, 8 * 16, 0, 16, 16, hud_badge_x, -1);
            int sword_col = link.has_four_sword ? 2 : (link.has_white_sword ? 1 : 0);
            texture_draw(s_hud_items_tex, sword_col * 16, 1 * 16, 16, 16, hud_badge_x + 11, -1);

            // Badge [B] + Subarma
            texture_draw(s_hud_items_tex, 9 * 16, 0, 16, 16, hud_badge_x + 28, -1);
            int sub_col = 0, sub_row = 2;
            SubweaponType cur_sub = subweapon_get_current();
            switch (cur_sub) {
                case ITEM_BOOMERANG:      sub_col = 0; sub_row = 2; break;
                case ITEM_GUST_JAR:       sub_col = 6; sub_row = 2; break;
                case ITEM_PEGASUS_BOOTS:  sub_col = 7; sub_row = 2; break;
                case ITEM_BOMBS:          sub_col = 2; sub_row = 2; break;
                case ITEM_CANE_OF_PACCI:  sub_col = 3; sub_row = 3; break;
                case ITEM_BOW:            sub_col = 4; sub_row = 2; break;
                case ITEM_MOLE_MITTS:     sub_col = 0; sub_row = 3; break;
                case ITEM_OCARINA_OF_WIND:sub_col = 4; sub_row = 3; break;
                case ITEM_FLAME_LANTERN:  sub_col = 2; sub_row = 3; break;
                case ITEM_ROCS_CAPE:      sub_col = 1; sub_row = 3; break;
                default:                  sub_col = 0; sub_row = 2; break;
            }
            texture_draw(s_hud_items_tex, sub_col * 16, sub_row * 16, 16, 16, hud_badge_x + 39, -1);
        } else {
            subweapon_render_hud_icon(88, 1);
        }

        // Contador de Bombas restantes quando a Bolsa de Bombas estiver equipada
        if (subweapon_get_current() == ITEM_BOMBS && !dungeon_is_active() && !dungeon_flames_is_active()) {
            char b_txt[8];
            snprintf(b_txt, sizeof(b_txt), "%d", subweapon_get_bomb_count());
            font_draw_text(104, 3, b_txt, 0xFDE047FF, true);
        }

        // Contador de Flechas restantes quando o Arco estiver equipado
        if (subweapon_get_current() == ITEM_BOW && !dungeon_is_active() && !dungeon_flames_is_active()) {
            char a_txt[8];
            snprintf(a_txt, sizeof(a_txt), "%d", subweapon_get_arrow_count());
            font_draw_text(104, 3, a_txt, 0x38BDF8FF, true);
        }

        // Contador de Chaves Pequenas da Masmorra (Small Keys 🔑 xN)
        if (dungeon_is_active()) {
            dungeon_render_hud_keys(106, 2);
        } else if (dungeon_flames_is_active()) {
            dungeon_flames_render_hud_keys(106, 2);
        } else if (dungeon_fortress_is_active()) {
            dungeon_fortress_render_hud_keys(106, 2);
        } else if (dungeon_droplets_is_active()) {
            dungeon_droplets_render_hud_keys(106, 2);
        } else if (dungeon_palace_is_active()) {
            dungeon_palace_render_hud_keys(106, 2);
        }

        // Ícone das Nadadeiras de Zora (Zora's Flippers) no HUD
        if (link.has_flippers) {
            int fx = 120;
            int fy = 2;
            // Nadadeira esquerda
            draw_rect(fx, fy + 2, 3, 6, 0x0284C7FF);
            draw_rect(fx + 1, fy + 4, 2, 4, 0x38BDF8FF);
            // Nadadeira direita
            draw_rect(fx + 4, fy + 2, 3, 6, 0x0284C7FF);
            draw_rect(fx + 5, fy + 4, 2, 4, 0x38BDF8FF);
            // Tira superior dourada
            draw_rect(fx + 1, fy + 1, 5, 2, 0xFBBF24FF);
        }

        // Ícone do Anel de Escalada (Grip Ring) no HUD
        if (link.has_grip_ring) {
            int gx = 132;
            int gy = 2;
            draw_rect(gx + 1, gy + 1, 6, 6, 0xF59E0BFF);
            draw_rect(gx + 2, gy + 2, 4, 4, 0x0C1C0DFF);
            hal_video_put_pixel(gx + 3, gy + 1, 0xEF4444FF);
            hal_video_put_pixel(gx + 4, gy + 1, 0xEF4444FF);
        }

        // Ícone do Pergaminho do Tigre nº 1 no HUD (se Link dominou o Spin Attack)
        if (link.has_spin_attack) {
            int sx = 142;
            int sy = 2;
            draw_rect(sx, sy + 1, 9, 8, 0xFEF08AFF);
            draw_rect(sx - 1, sy, 11, 2, 0xD97706FF);
            draw_rect(sx - 1, sy + 8, 11, 2, 0xD97706FF);
            draw_rect(sx + 3, sy + 2, 3, 6, 0xDC2626FF); // Fita vermelha marcial
            hal_video_put_pixel(sx + 4, sy + 4, 0xFFFFFFFF);
        }

        // Badge da Região Ativa no canto superior direito
        u32 reg_color = (s_current_region == REGION_USA) ? 0x4287F5FF :
                        (s_current_region == REGION_EUR) ? 0xF5A742FF : 0xF54242FF;
        draw_rect(ctx->render_width - 32, 2, 28, 10, reg_color);
        const char* reg_label = (s_current_region == REGION_USA) ? "USA" :
                                (s_current_region == REGION_EUR) ? "EUR" : "JPN";
        font_draw_text(ctx->render_width - 28, 3, reg_label, 0xFFFFFFFF, true);

        // 4. Balão de Diálogos e Retratos de Personagens (Ezlo / NPCs)
        dialogue_render();

        // 5. Interface de Fusão de Kinstones (Pedras da Sorte)
        kinstone_render();

        // 5b. HUD do Minigame das Galinhas Cucco de Anju (Ato VI)
        if (s_in_town && cucco_minigame_is_active()) {
            cucco_minigame_render_hud();
        }

        // 6. Banners Festivos dos Pergaminhos do Tigre (Tiger Scrolls - Sword Dojo)
        sword_dojo_render_banner();

        // 6b. Banner Festivo de Aquisição da Lendária White Sword (Espada Branca)
        if (link.white_sword_banner_timer > 0) {
            int ban_w = 216;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x0A0806EE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0x38BDF8FF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x0F172AFF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x1E293BEE);

            // Ícone da Espada Branca
            draw_rect(ban_x + 10, ban_y + 6, 4, 16, 0xFFFFFFFF);
            draw_rect(ban_x + 11, ban_y + 6, 2, 16, 0xBAE6FDFF);
            draw_rect(ban_x + 8, ban_y + 18, 8, 3, 0xFACC15FF); // Guarda asas ouro
            hal_video_put_pixel(ban_x + 11, ban_y + 19, 0xEF4444FF); // Rubi
            draw_rect(ban_x + 10, ban_y + 21, 4, 4, 0xF8FAFCFF); // Cabo

            font_draw_text(ban_x + 26, ban_y + 6, "ESPADA BRANCA FORJADA!", 0xFFFFFFFF, true);
            font_draw_text(ban_x + 26, ban_y + 18, "Dano Dobrado (2 HP) | Melari", 0xFACC15FF, true);
        }

        // 6c. Banner Festivo de Aquisicao do Sagrado Elemento Fogo (Fire Element)
        if (link.fire_element_banner_timer > 0) {
            int ban_w = 216;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x1A0505EE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0xF97316FF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x2A0808FF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x450A0AEE);

            // Icone da Chama Sagrada / Elemento Fogo
            draw_rect(ban_x + 9, ban_y + 7, 8, 14, 0xDC2626FF);
            draw_rect(ban_x + 11, ban_y + 9, 4, 10, 0xF97316FF);
            draw_rect(ban_x + 12, ban_y + 12, 2, 4, 0xFDE047FF);
            hal_video_put_pixel(ban_x + 13, ban_y + 8, 0xFFFFFFFF);

            font_draw_text(ban_x + 26, ban_y + 6, "ELEMENTO FOGO OBTIDO!", 0xFDE047FF, true);
            font_draw_text(ban_x + 26, ban_y + 18, "Cave of Flames | Gleerok", 0xF97316FF, true);
        }

        // 6d. Banner Festivo da Four Sword: White Sword (Two Elements)
        if (link.two_elements_banner_timer > 0) {
            int ban_w = 216;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x061826EE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0x38BDF8FF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x0A2540FF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x0E3A64EE);

            // Icones dos 2 Clones da Four Sword (dois herois lado a lado)
            draw_rect(ban_x + 6, ban_y + 7, 7, 14, 0x22C55EFF);  // Link 1 (Verde)
            draw_rect(ban_x + 14, ban_y + 7, 7, 14, 0x38BDF8FF); // Link 2 (Clone Ciano)
            hal_video_put_pixel(ban_x + 9, ban_y + 9, 0xFDE8CDFF);
            hal_video_put_pixel(ban_x + 17, ban_y + 9, 0xFDE8CDFF);

            font_draw_text(ban_x + 26, ban_y + 6, "ESPADA BRANCA (2 ELEMENTOS)!", 0xFFFFFFFF, true);
            font_draw_text(ban_x + 26, ban_y + 18, "DIVISAO FOUR SWORD (2 CLONES)!", 0x38BDF8FF, true);
        }

        // 6d2. Banner Festivo da Four Sword: White Sword (Three Elements)
        if (link.three_elements_banner_timer > 0) {
            int ban_w = 224;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x061826EE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0x06B6D4FF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x0A2540FF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x0E3A64EE);

            // Icones dos 3 Clones da Four Sword (tres herois: Verde, Vermelho, Azul)
            draw_rect(ban_x + 4, ban_y + 7, 6, 14, 0x22C55EFF);  // Link 1 (Verde)
            draw_rect(ban_x + 11, ban_y + 7, 6, 14, 0xEF4444FF); // Link 2 (Vermelho)
            draw_rect(ban_x + 18, ban_y + 7, 6, 14, 0x0284C7FF); // Link 3 (Azul)
            hal_video_put_pixel(ban_x + 7, ban_y + 9, 0xFDE8CDFF);
            hal_video_put_pixel(ban_x + 14, ban_y + 9, 0xFDE8CDFF);
            hal_video_put_pixel(ban_x + 21, ban_y + 9, 0xFDE8CDFF);

            font_draw_text(ban_x + 28, ban_y + 6, "ESPADA BRANCA (3 ELEMENTOS)!", 0xFFFFFFFF, true);
            font_draw_text(ban_x + 28, ban_y + 18, "DIVISAO FOUR SWORD (3 CLONES)!", 0x38BDF8FF, true);
        }

        // 6d3. Banner Festivo da FOUR SWORD COMPLETA FORJADA (Four Elements Infused!)
        if (link.four_sword_banner_timer > 0) {
            int ban_w = 236;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x1E1B4BEE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0xF59E0BFF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x0F172AFF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x1E1B4BEE);

            // Icones dos 4 Herois da Four Sword: Verde, Vermelho, Azul, Roxo/Violeta
            draw_rect(ban_x + 4, ban_y + 7, 5, 14, 0x22C55EFF);  // Link 1 (Verde)
            draw_rect(ban_x + 10, ban_y + 7, 5, 14, 0xEF4444FF); // Link 2 (Vermelho)
            draw_rect(ban_x + 16, ban_y + 7, 5, 14, 0x0284C7FF); // Link 3 (Azul)
            draw_rect(ban_x + 22, ban_y + 7, 5, 14, 0x7E22CEFF); // Link 4 (Roxo)
            hal_video_put_pixel(ban_x + 6, ban_y + 9, 0xFDE8CDFF);
            hal_video_put_pixel(ban_x + 12, ban_y + 9, 0xFDE8CDFF);
            hal_video_put_pixel(ban_x + 18, ban_y + 9, 0xFDE8CDFF);
            hal_video_put_pixel(ban_x + 24, ban_y + 9, 0xFDE8CDFF);

            font_draw_text(ban_x + 30, ban_y + 6, "FOUR SWORD COMPLETA FORJADA!", 0xFDE047FF, true);
            font_draw_text(ban_x + 30, ban_y + 18, "4 HEROIS & SWORD BEAMS ATIVOS!", 0x38BDF8FF, true);
        }

        // 6d4. Banner Festivo da ROYAL GOLDEN KINSTONE (Rei Gustaf / Cripta Real)
        if (link.royal_kinstone_banner_timer > 0 && !royal_valley_is_active()) {
            int ban_w = 236;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x0F172AEE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0xF59E0BFF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x1E1B4BFF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x0F172AEE);

            // Ícone da Golden Kinstone Real
            draw_rect(ban_x + 6, ban_y + 8, 12, 16, 0xF59E0BFF);
            draw_rect(ban_x + 9, ban_y + 11, 6, 10, 0xFDE047FF);
            hal_video_put_pixel(ban_x + 12, ban_y + 16, 0xFFFFFFFF);

            font_draw_text(ban_x + 26, ban_y + 6, "ROYAL GOLDEN KINSTONE OBTIDA!", 0xFDE047FF, true);
            font_draw_text(ban_x + 26, ban_y + 18, "O ESPIRITO DO REI GUSTAF O ABENCOA!", 0x38BDF8FF, true);
        }

        // 6d5. Banner Festivo do SANTUARIO DE VAATI (Dark Hyrule Castle)
        if (link.sanctum_banner_timer > 0) {
            int ban_w = 236;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x181824EE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0xF43F5EFF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x0F0E17FF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x1E1B4BEE);

            // Ícone do Olho de Malícia de Vaati
            draw_rect(ban_x + 6, ban_y + 10, 14, 12, 0x4C0519FF);
            draw_rect(ban_x + 9, ban_y + 12, 8, 8, 0xF43F5EFF);
            hal_video_put_pixel(ban_x + 12, ban_y + 15, 0xFFFFFFFF);

            font_draw_text(ban_x + 26, ban_y + 6, "PORTAO DO SANTUARIO DESLACRADO!", 0xFDE047FF, true);
            font_draw_text(ban_x + 26, ban_y + 18, "VAATI AGUARDA NO TOPO DO CASTELO!", 0xF43F5EFF, true);
        }

        // 6e. Banner Festivo de Aquisição do Arco e Flechas (Bow & Arrow)
        if (link.bow_banner_timer > 0) {
            int ban_w = 216;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x061806EE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0x22C55EFF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x0A200AFF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x143514EE);

            // Ícone do Arco e Flecha em pixel art
            draw_rect(ban_x + 8, ban_y + 8, 3, 16, 0x854D0EFF);
            hal_video_put_pixel(ban_x + 11, ban_y + 9, 0xA16207FF);
            hal_video_put_pixel(ban_x + 11, ban_y + 22, 0xA16207FF);
            draw_rect(ban_x + 12, ban_y + 11, 2, 10, 0xE2E8F0FF); // Corda
            draw_rect(ban_x + 10, ban_y + 15, 10, 2, 0x94A3B8FF); // Flecha
            hal_video_put_pixel(ban_x + 20, ban_y + 14, 0x38BDF8FF); // Ponta
            hal_video_put_pixel(ban_x + 20, ban_y + 17, 0x38BDF8FF);

            font_draw_text(ban_x + 26, ban_y + 6, "ARCO E FLECHAS OBTIDO!", 0xFDE047FF, true);
            font_draw_text(ban_x + 26, ban_y + 18, "Disparo a Distancia | 30 Flechas", 0x34D399FF, true);
        }

        // 6f. Banner Festivo de Aquisição das Luvas de Toupeira (Mole Mitts)
        if (link.mole_mitts_banner_timer > 0) {
            int ban_w = 216;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x1A0F05EE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0xD97706FF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x2A180AFF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x3D2210EE);

            // Ícone das Luvas de Toupeira em pixel art
            draw_rect(ban_x + 8, ban_y + 12, 10, 8, 0x78350FFF);
            draw_rect(ban_x + 7, ban_y + 18, 12, 3, 0xF59E0BFF); // Punho dourado
            // 3 Garras afiadas
            draw_rect(ban_x + 9,  ban_y + 7, 2, 5, 0xE2E8F0FF);
            draw_rect(ban_x + 12, ban_y + 5, 2, 7, 0xE2E8F0FF);
            draw_rect(ban_x + 15, ban_y + 7, 2, 5, 0xE2E8F0FF);
            hal_video_put_pixel(ban_x + 9,  ban_y + 6, 0xFFFFFFFF);
            hal_video_put_pixel(ban_x + 12, ban_y + 4, 0xFFFFFFFF);
            hal_video_put_pixel(ban_x + 15, ban_y + 6, 0xFFFFFFFF);

            font_draw_text(ban_x + 26, ban_y + 6, "LUVAS DE TOUPEIRA OBTIDAS!", 0xF59E0BFF, true);
            font_draw_text(ban_x + 26, ban_y + 18, "Escave Paredes de Terra e Segredos!", 0xFDE047FF, true);
        }

        // 6g. Banner Festivo de Ativação do Robô Armos
        if (link.armos_banner_timer > 0) {
            int ban_w = 216;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x0A0F14EE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0x38BDF8FF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x0F172AFF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x1E293BEE);

            // Ícone do Robô Armos com sensor luminoso ciano
            draw_rect(ban_x + 8, ban_y + 8, 12, 16, 0x64748BFF);
            draw_rect(ban_x + 10, ban_y + 6, 8, 4, 0x475569FF);
            draw_rect(ban_x + 12, ban_y + 12, 4, 4, 0x38BDF8FF); // Olho azul reativado
            hal_video_put_pixel(ban_x + 13, ban_y + 13, 0xFFFFFFFF);

            font_draw_text(ban_x + 26, ban_y + 6, "CIRCUITO ARMOS ATIVADO!", 0x38BDF8FF, true);
            font_draw_text(ban_x + 26, ban_y + 18, "Mecanismo Desperto pelo Minish!", 0xFDE047FF, true);
        }

        // 6h. Banner Festivo de Aquisição da Ocarina do Vento (Ocarina of Wind)
        if (link.ocarina_banner_timer > 0) {
            int ban_w = 216;
            int ban_h = 32;
            int ban_x = (ctx->render_width - ban_w) / 2;
            int ban_y = 64;

            draw_rect(ban_x - 2, ban_y - 2, ban_w + 4, ban_h + 4, 0x031B2AEE);
            draw_rect(ban_x - 1, ban_y - 1, ban_w + 2, ban_h + 2, 0x38BDF8FF);
            draw_rect(ban_x, ban_y, ban_w, ban_h, 0x082F49FF);
            draw_rect(ban_x + 2, ban_y + 2, ban_w - 4, ban_h - 4, 0x0C4A6EEE);

            // Ícone da Ocarina do Vento
            draw_rect(ban_x + 7, ban_y + 11, 4, 4, 0x38BDF8FF); // Bocal
            draw_rect(ban_x + 10, ban_y + 9, 13, 8, 0x0284C7FF); // Corpo azul
            hal_video_put_pixel(ban_x + 14, ban_y + 11, 0x0F172AFF);
            hal_video_put_pixel(ban_x + 18, ban_y + 11, 0x0F172AFF);
            hal_video_put_pixel(ban_x + 16, ban_y + 13, 0x0F172AFF);
            hal_video_put_pixel(ban_x + 12, ban_y + 10, 0xFFFFFFFF); // Brilho

            font_draw_text(ban_x + 28, ban_y + 6, "OCARINA DO VENTO OBTIDA!", 0x38BDF8FF, true);
            font_draw_text(ban_x + 28, ban_y + 18, "Invoque o Passaro Zeffa | Mazaal", 0xFDE047FF, true);
        }

        // Prompt de Interação para Infiltrar no Robô Armos (Link Minish próximo a Armos dormente)
        if (s_in_wind_ruins && link.is_minish && !link.is_transforming && !dialogue_is_active()) {
            Entity* armos = entity_find_nearby_armos(link.x, link.y, 28.0f);
            if (armos) {
                const char* armos_prompt = "[A] Infiltrar no Armos (Minish)";
                int text_w = 176;
                int apx = (ctx->render_width - text_w) / 2;
                int apy = ctx->render_height - 18;
                draw_rect(apx - 4, apy - 2, text_w + 8, 14, 0x0F172AEE);
                draw_rect(apx - 3, apy - 1, text_w + 6, 12, 0x38BDF8FF);
                draw_rect(apx - 2, apy, text_w + 4, 10, 0x1E293BEE);
                font_draw_text(apx, apy + 1, armos_prompt, 0xBAE6FDFF, true);
            }
        }

        // 7. Badge do Estado Minish no HUD
        if (link.is_minish) {
            int mx = 146;
            int my = 2;
            draw_rect(mx, my + 1, 8, 8, 0x064E3BFF);
            draw_rect(mx + 2, my + 2, 4, 3, 0xEF4444FF); // Gorro bolota
            draw_rect(mx + 1, my + 5, 6, 3, 0x10B981FF); // Túnica
            hal_video_put_pixel(mx + 3, my + 1, 0xFFFFFFFF); // Pom-pom
            font_draw_text(mx + 11, 3, "MINISH", 0x34D399FF, true);
        }

        // 8. Prompt de Interação do Portal / Toco Minish [R]
        Entity* nearby_stump = entity_find_nearby_minish_stump(link.x, link.y, 22.0f);
        if (nearby_stump && !link.is_transforming && !dialogue_is_active() && !kinstone_is_active()) {
            const char* prompt_text = link.is_minish ? "[R] Crescer (Toco Minish)" : "[R] Encolher (Toco Minish)";
            int text_w = 140;
            int bpx = (ctx->render_width - text_w) / 2;
            int bpy = ctx->render_height - 18;
            draw_rect(bpx - 4, bpy - 2, text_w + 8, 14, 0x0A0F0CEE);
            draw_rect(bpx - 3, bpy - 1, text_w + 6, 12, 0x10B981FF);
            draw_rect(bpx - 2, bpy, text_w + 4, 10, 0x0A1F14EE);
            font_draw_text(bpx, bpy + 1, prompt_text, 0x6EE7B7FF, true);
        }

        // 9. Prompt de Ação Aquática (Natação / Mergulho Zora)
        if (link.is_swimming && !dialogue_is_active() && !kinstone_is_active()) {
            const char* swim_prompt = link.is_diving ? "MERGULHADO (SOB A AGUA)" : "[A] Mergulhar (Zora's Flippers)";
            int text_w = link.is_diving ? 144 : 166;
            int spx = (ctx->render_width - text_w) / 2;
            int spy = ctx->render_height - 18;
            draw_rect(spx - 4, spy - 2, text_w + 8, 14, 0x04192FEE);
            draw_rect(spx - 3, spy - 1, text_w + 6, 12, 0x0284C7FF);
            draw_rect(spx - 2, spy, text_w + 4, 10, 0x0C4A6EEE);
            font_draw_text(spx, spy + 1, swim_prompt, 0xBAE6FDFF, true);
        }

        // 10. Prompt de Escalada (Grip Ring)
        if (link.is_climbing && !dialogue_is_active() && !kinstone_is_active()) {
            const char* climb_prompt = "ESCALANDO PAREDAO (GRIP RING)";
            int text_w = 170;
            int cpx = (ctx->render_width - text_w) / 2;
            int cpy = ctx->render_height - 18;
            draw_rect(cpx - 4, cpy - 2, text_w + 8, 14, 0x1E1208EE);
            draw_rect(cpx - 3, cpy - 1, text_w + 6, 12, 0xD4AF37FF);
            draw_rect(cpx - 2, cpy, text_w + 4, 10, 0x331B0CEE);
            font_draw_text(cpx, cpy + 1, climb_prompt, 0xFDE047FF, true);
        }

        // 10c. Prompts de Contexto: Biblioteca Real e Lake Hylia
        if (s_in_library && !dialogue_is_active() && !inventory_is_paused()) {
            char lib_info[64];
            LibraryQuestState* qs = library_get_quest_state();
            snprintf(lib_info, sizeof(lib_info), "BIBLIOTECA REAL - LIVROS: %d/3%s",
                     qs->books_returned_count,
                     qs->staircase_formed ? " (ESCADA PRONTA!)" : "");
            int tw = 200;
            int px = (ctx->render_width - tw) / 2;
            int py = ctx->render_height - 18;
            draw_rect(px - 4, py - 2, tw + 8, 14, 0x1E1208EE);
            draw_rect(px - 3, py - 1, tw + 6, 12, 0xD4AF37FF);
            draw_rect(px - 2, py, tw + 4, 10, 0x331B0CEE);
            font_draw_text(px, py + 1, lib_info, 0xFDE047FF, true);
        } else if (s_in_lake_hylia && !dialogue_is_active() && !inventory_is_paused() && !link.is_swimming) {
            const char* lake_prompt = library_is_temple_unlocked() ? "LAKE HYLIA - PASSAGEM DO TEMPLO ABERTA!" : "LAKE HYLIA (TEMPLO CONGELADO)";
            int tw = 210;
            int px = (ctx->render_width - tw) / 2;
            int py = ctx->render_height - 18;
            draw_rect(px - 4, py - 2, tw + 8, 14, 0x051F3EEE);
            draw_rect(px - 3, py - 1, tw + 6, 12, 0x38BDF8FF);
            draw_rect(px - 2, py, tw + 4, 10, 0x0C4A6EEE);
            font_draw_text(px, py + 1, lake_prompt, 0xE0F2FEFF, true);
        }

        // Prompt de Contexto: Veil Falls & Cloud Tops
        if (veil_clouds_is_active() && !dialogue_is_active() && !inventory_is_paused()) {
            char v_info[64];
            if (veil_clouds_is_in_clouds()) {
                snprintf(v_info, sizeof(v_info), "TOPO DAS NUVENS - KINSTONES: %d/5%s",
                         veil_clouds_get_golden_kinstones(),
                         veil_clouds_is_tornado_active() ? " (TORNADO ATIVO!)" : "");
            } else {
                snprintf(v_info, sizeof(v_info), "%s",
                         (veil_clouds_get_scene() == VEIL_SCENE_FALLS_BASE) ? "QUEDAS DO VEU (BASE DAS CACHOEIRAS)" : "QUEDAS DO VEU (CUME E REDEMOINHO)");
            }
            int tw = 210;
            int px = (ctx->render_width - tw) / 2;
            int py = ctx->render_height - 18;
            draw_rect(px - 4, py - 2, tw + 8, 14, 0x041E38EE);
            draw_rect(px - 3, py - 1, tw + 6, 12, 0x38BDF8FF);
            draw_rect(px - 2, py, tw + 4, 10, 0x0A2B4EEE);
            font_draw_text(px, py + 1, v_info, 0xE0F2FEFF, true);
        }

        // Prompt da Flame Lantern se equipada
        if (subweapon_get_current() == ITEM_FLAME_LANTERN && !dialogue_is_active() && !inventory_is_paused()) {
            char l_info[64];
            snprintf(l_info, sizeof(l_info), "LANTERNA [%s] (F1: ESCURO | F2: FOGO)",
                     lantern_is_lit() ? "ACESA" : "APAGADA");
            int tw = 200;
            int px = (ctx->render_width - tw) / 2;
            int py = ctx->render_height - 18;
            draw_rect(px - 4, py - 2, tw + 8, 14, 0x1E0D03EE);
            draw_rect(px - 3, py - 1, tw + 6, 12, 0xF97316FF);
            draw_rect(px - 2, py, tw + 4, 10, 0x451A03EE);
            font_draw_text(px, py + 1, l_info, 0xFDE047FF, true);
        }

        // Banner comemorativo de aquisição da Flame Lantern
        if (link.lantern_banner_timer > 0) {
            const char* b1 = "FLAME LANTERN ADQUIRIDA!";
            const char* b2 = "Chama Eterna: Ilumina a escuridao e derrete o gelo!";
            int bw = 240;
            int bx = (ctx->render_width - bw) / 2;
            int by = 35;
            draw_rect(bx - 6, by - 4, bw + 12, 30, 0x1E0D03EE);
            draw_rect(bx - 5, by - 3, bw + 10, 28, 0xF97316FF);
            draw_rect(bx - 4, by - 2, bw + 8, 26, 0x431407EE);
            font_draw_text(bx + 35, by + 1, b1, 0xFDE047FF, true);
            font_draw_text(bx + 4, by + 13, b2, 0xFFEDD5FF, false);
        }

        // Banner comemorativo de aquisição do Elemento da Água (Water Element)
        if (link.water_element_banner_timer > 0) {
            const char* b1 = "ELEMENTO DA AGUA CONQUISTADO!";
            const char* b2 = "A pureza glacial e o fluxo eterno restauram a Forca Divina!";
            int bw = 240;
            int bx = (ctx->render_width - bw) / 2;
            int by = 35;
            draw_rect(bx - 6, by - 4, bw + 12, 30, 0x082F49EE);
            draw_rect(bx - 5, by - 3, bw + 10, 28, 0x0284C7FF);
            draw_rect(bx - 4, by - 2, bw + 8, 26, 0x0C4A6EEE);
            font_draw_text(bx + 20, by + 1, b1, 0x38BDF8FF, true);
            font_draw_text(bx + 4, by + 13, b2, 0xE0F2FEFF, false);
        }

        // Banner comemorativo de aquisição do Elemento do Vento (Wind Element)
        if (link.wind_element_banner_timer > 0) {
            const char* b1 = "ELEMENTO DO VENTO CONQUISTADO!";
            const char* b2 = "As brisas eternas e os 4 Elementos Sagrados foram reunidos!";
            int bw = 240;
            int bx = (ctx->render_width - bw) / 2;
            int by = 35;
            draw_rect(bx - 6, by - 4, bw + 12, 30, 0x064E3BEE);
            draw_rect(bx - 5, by - 3, bw + 10, 28, 0x10B981FF);
            draw_rect(bx - 4, by - 2, bw + 8, 26, 0x065F46EE);
            font_draw_text(bx + 18, by + 1, b1, 0x6EE7B7FF, true);
            font_draw_text(bx + 4, by + 13, b2, 0xECFDF5FF, false);
        }

        // Banner do Grande Tornado para o Palácio do Vento (Palace of Winds)
        if (link.cloud_banner_timer > 0) {
            const char* b1 = "GRANDE TORNADO DO PALACIO DO VENTO!";
            const char* b2 = "5 Kinstones Douradas fundidas! O caminho para o ceu esta aberto!";
            int bw = 240;
            int bx = (ctx->render_width - bw) / 2;
            int by = 35;
            draw_rect(bx - 6, by - 4, bw + 12, 30, 0x022849EE);
            draw_rect(bx - 5, by - 3, bw + 10, 28, 0x38BDF8FF);
            draw_rect(bx - 4, by - 2, bw + 8, 26, 0x0A3A64EE);
            font_draw_text(bx + 12, by + 1, b1, 0xFDE047FF, true);
            font_draw_text(bx + 4, by + 13, b2, 0xE0F2FEFF, false);
        }

        // Prompt da Capa de Roc se equipada
        if (subweapon_get_current() == ITEM_ROCS_CAPE && !dialogue_is_active() && !inventory_is_paused()) {
            const char* rc_info = link.is_jumping ?
                (rocs_cape_is_gliding() ? "PLANANDO NO AR (CAPA DE ROC) | [A] DOWN-THRUST" :
                 (rocs_cape_is_down_thrust() ? "DOWN-THRUST EM QUEDA!" : "[B] PLANAR | [A] DOWN-THRUST")) :
                "CAPA DE ROC [B: PULAR / PLANAR]";
            int tw = 216;
            int px = (ctx->render_width - tw) / 2;
            int py = ctx->render_height - 18;
            draw_rect(px - 4, py - 2, tw + 8, 14, 0x1E0505EE);
            draw_rect(px - 3, py - 1, tw + 6, 12, 0xEF4444FF);
            draw_rect(px - 2, py, tw + 4, 10, 0x380A0AEE);
            font_draw_text(px, py + 1, rc_info, 0xFDE047FF, true);
        }

        // Banner comemorativo de aquisição da Capa de Roc
        if (link.rocs_banner_timer > 0) {
            const char* b1 = "CAPA DE ROC (ROC'S CAPE) ADQUIRIDA!";
            const char* b2 = "Salto Acrobatico: Pule, plane sobre abismos e execute o Down-Thrust!";
            int bw = 240;
            int bx = (ctx->render_width - bw) / 2;
            int by = 35;
            draw_rect(bx - 6, by - 4, bw + 12, 30, 0x1E0505EE);
            draw_rect(bx - 5, by - 3, bw + 10, 28, 0xEF4444FF);
            draw_rect(bx - 4, by - 2, bw + 8, 26, 0x3F0A0AEE);
            font_draw_text(bx + 15, by + 1, b1, 0xFDE047FF, true);
            font_draw_text(bx + 4, by + 13, b2, 0xFEE2E2FF, false);
        }

        // 10b. Sistema de Transporte Rapido: Ocarina of Wind, Zeffa e Mapa de Cristas de Vento
        fast_travel_render(&camera, link.x, link.y);

        // 11. Subtela de Inventario e Menu de Pausa [START / ENTER]
        if (inventory_is_paused()) {
            inventory_render_pause_menu(ctx->render_width, ctx->render_height, link.hearts, link.max_hearts, link.rupees);
        }

        // --------------------------------------------------------------------
        // 6. APRESENTAÇÃO NA TELA (SDL2 GPU)
        // --------------------------------------------------------------------
        hal_video_render_frame();
    }

    if (s_sheet0)        texture_free(s_sheet0);
    if (s_sheet1)        texture_free(s_sheet1);
    if (s_link_tex)      texture_free(s_link_tex);
    if (s_octo_tex)      texture_free(s_octo_tex);
    if (s_enemies_tex)   texture_free(s_enemies_tex);
    if (s_npcs_tex)      texture_free(s_npcs_tex);
    if (s_bosses_tex)    texture_free(s_bosses_tex);
    if (s_hud_items_tex) texture_free(s_hud_items_tex);

    entity_manager_shutdown();
    if (s_town_map)        map_destroy(s_town_map);
    if (s_village_map)     map_destroy(s_village_map);
    if (s_south_field_map) map_destroy(s_south_field_map);
    if (s_north_field_map) map_destroy(s_north_field_map);
    if (s_crenel_base_map) map_destroy(s_crenel_base_map);
    if (s_melari_mines_map) map_destroy(s_melari_mines_map);
    if (s_castor_wilds_map)    map_destroy(s_castor_wilds_map);
    if (s_mole_cave_map)       map_destroy(s_mole_cave_map);
    if (s_wind_ruins_map)      map_destroy(s_wind_ruins_map);
    if (s_armos_interior_map)  map_destroy(s_armos_interior_map);
    if (s_library_map)         map_destroy(s_library_map);
    if (s_lake_hylia_map)      map_destroy(s_lake_hylia_map);
    if (s_castle_courtyard_map)map_destroy(s_castle_courtyard_map);
    map_free_tileset();
    dungeon_droplets_shutdown();
    dungeon_palace_shutdown();
    veil_clouds_shutdown();
    rocs_cape_shutdown();
    startup_menu_shutdown();
    map_destroy(world_map);
    hal_audio_shutdown();
    hal_input_shutdown();
    hal_video_shutdown();
    printf("[SUCESSO] Aplicacao finalizada com exito!\n");
    return 0;
}
