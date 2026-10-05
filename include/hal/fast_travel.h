#ifndef HAL_FAST_TRAVEL_H
#define HAL_FAST_TRAVEL_H

/*
 * ============================================================================
 * include/hal/fast_travel.h - Sistema de Transporte Rápido: Ocarina of Wind & Zeffa
 * ============================================================================
 * Implementa a convocação do grande pássaro Zeffa através da Ocarina of Wind,
 * o mapa de seleção de cristas de vento (Wind Crests) e o voo pelos céus de Hyrule.
 */

#include "gba/types.h"
#include "hal/map.h"
#include <stdbool.h>

typedef enum {
    CREST_HYRULE_TOWN = 0,
    CREST_MINISH_WOODS,
    CREST_MOUNT_CRENEL,
    CREST_CASTOR_WILDS,
    CREST_WIND_RUINS,
    CREST_LAKE_HYLIA,
    CREST_VEIL_FALLS,
    CREST_CLOUD_TOPS,
    CREST_COUNT
} FastTravelDestination;

typedef enum {
    ZEFFA_STATE_INACTIVE = 0,
    ZEFFA_STATE_PLAYING_SONG, // Link tocando a Ocarina, notas musicais flutuando
    ZEFFA_STATE_SWOOP_IN,     // Zeffa desce voando em arco e pega Link
    ZEFFA_STATE_MAP_SELECT,   // Menu do mapa mundi de Hyrule com cristas de vento
    ZEFFA_STATE_FLYING_HIGH,  // Voo veloz pelos céus em direção ao destino
    ZEFFA_STATE_SWOOP_DOWN,   // Zeffa desce na crista de vento e pousa Link
    ZEFFA_STATE_FINISHED
} ZeffaState;

typedef struct {
    FastTravelDestination id;
    const char* name;
    int world_map_x;       // Posição no minimapa de seleção (pixels de tela)
    int world_map_y;
    int target_map_id;     // ID da cena (0: Woods, 1: Town, 4: South Field, 6: Crenel Base, 9: Castor Wilds, 11: Wind Ruins, etc.)
    float spawn_x;         // Coordenada X no mapa de destino
    float spawn_y;         // Coordenada Y no mapa de destino
    bool unlocked;         // Crista ativada pelo herói
} WindCrestInfo;

/*
 * Inicializa o subsistema de transporte rápido e cristas de vento.
 */
void fast_travel_init(void);

/*
 * Inicia a convocação de Zeffa ao tocar a Canção do Vento na Ocarina of Wind.
 */
void fast_travel_start(float link_x, float link_y);

/*
 * Atualiza o ciclo de vida do Zeffa, navegação do mapa de seleção e transição de destino.
 * Retorna true se uma mudança de mapa/coordenadas for acionada.
 */
bool fast_travel_update(float* link_x, float* link_y, int* out_new_map, float* out_new_x, float* out_new_y);

/*
 * Renderiza o Zeffa, as partículas de vento/música e a tela de seleção de mapa do overworld.
 */
void fast_travel_render(const Camera* cam, float link_x, float link_y);

/*
 * Retorna true se o voo ou a seleção do Zeffa estiver em andamento (trava controle padrão do herói).
 */
bool fast_travel_is_active(void);

/*
 * Retorna true se Link está no ar carregado pelo pássaro (deve ocultar sprite do Link no chão).
 */
bool fast_travel_is_link_airborne(void);

/*
 * Desbloqueia uma crista de vento específica.
 */
void fast_travel_unlock_crest(FastTravelDestination dest);

/*
 * Retorna se uma crista de vento está desbloqueada.
 */
bool fast_travel_is_crest_unlocked(FastTravelDestination dest);

/*
 * Obtém a máscara de bits de cristas desbloqueadas para o Save/Load.
 */
u8 fast_travel_get_unlocked_mask(void);

/*
 * Restaura a máscara de cristas desbloqueadas a partir do SaveData.
 */
void fast_travel_set_unlocked_mask(u8 mask);

/*
 * Verifica se a coordenada no mapa toca em uma crista de vento e a ativa caso esteja inativa.
 */
bool fast_travel_check_crest_activation(float world_x, float world_y, const char** out_crest_name);

#endif // HAL_FAST_TRAVEL_H
