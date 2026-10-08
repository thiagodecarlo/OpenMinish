#ifndef HAL_VEIL_CLOUDS_H
#define HAL_VEIL_CLOUDS_H

/*
 * ============================================================================
 * include/hal/veil_clouds.h - Veil Falls (Quedas do Véu) & Cloud Tops (Topo das Nuvens)
 * ============================================================================
 * Modela as regiões montanhosas e celestiais canônicas de The Legend of Zelda: The Minish Cap:
 *
 * 1. Veil Falls (Quedas do Véu):
 *    - Paredões montanhosos escarpados com cachoeiras torrenciais (escaláveis com Grip Ring).
 *    - Crista de Vento (Wind Crest) da base das quedas para transporte com a Ocarina of Wind.
 *    - Caverna das quedas com breu (iluminada pela Flame Lantern).
 *    - Redemoinho colossal no topo que arremessa Link em direção à estratosfera!
 *
 * 2. Cloud Tops (Topo das Nuvens):
 *    - Solo fofo e denso de nuvens cúmulos transitáveis pelo herói.
 *    - Nuvens cinzentas de tempestade escaváveis com as Luvas de Toupeira (Mole Mitts).
 *    - Redemoinhos impulsionadores (Updraft Whirlpools) que lançam Link em saltos parabólicos
 *      para atravessar abismos aéreos.
 *    - Santuário da Tribo do Vento (Wind Tribe): sábios celestiais (Hailey, Gregal, Gale).
 *    - 5 Fusões Sagradas de Kinstones Douradas (Golden Kinstones) que dissipam as tempestades
 *      e ativam o Grande Tornado Ciclônico que dá acesso ao Palácio do Vento (Palace of Winds)!
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/entity.h"
#include <stdbool.h>

#define VEIL_MAP_W 16
#define VEIL_MAP_H 10
#define MAX_CLOUD_PARTICLES 32
#define MAX_WATER_SPRAY 24
#define TOTAL_GOLDEN_KINSTONES 5

typedef enum {
    VEIL_SCENE_FALLS_BASE = 0,    // Base das Quedas: cachoeiras, paredões e Wind Crest
    VEIL_SCENE_FALLS_SUMMIT,      // Topo das Quedas: penhasco alto e redemoinho ascensional
    VEIL_SCENE_CLOUD_LOWER,       // Nuvens Baixas: redemoinhos impulsionadores e nuvens escuras
    VEIL_SCENE_CLOUD_SANCTUARY,   // Santuário da Tribo do Vento e tornado do Palácio do Vento
    VEIL_SCENE_COUNT
} VeilCloudSceneId;

typedef struct {
    bool  active;
    float x;
    float y;
    float vx;
    float vy;
    float size;
    u32   color;
    int   life;
    int   max_life;
} AtmosphericParticle;

typedef struct {
    float x;
    float y;
    bool  is_red;                 // true = redemoinho vermelho super-impulsão; false = amarelo
    float angle;
} UpdraftWhirlwind;

typedef struct {
    bool                is_active;
    VeilCloudSceneId    current_scene;
    Tilemap*            maps[VEIL_SCENE_COUNT];

    // Partículas atmosféricas (névoa de água nas quedas e tufos de nuvens no céu)
    AtmosphericParticle particles[MAX_CLOUD_PARTICLES];
    int                 particle_timer;

    // Mecânica de Voo no Redemoinho / Catapulta Aérea
    bool                is_in_updraft;
    float               updraft_arc_timer;
    float               updraft_target_x;
    float               updraft_target_y;

    // Redemoinhos ativos na cena
    UpdraftWhirlwind    whirlpools[4];
    int                 whirlpool_count;

    // Progresso das Kinstones Douradas da Tribo do Vento (0 a 5)
    u8                  golden_kinstones_fused;
    bool                kinstone_pedestals[TOTAL_GOLDEN_KINSTONES];
    bool                palace_tornado_active;  // Grande tornado para o Palace of Winds ativado

    // Transição segura
    float               last_safe_x;
    float               last_safe_y;
} VeilCloudsState;

/*
 * Inicializa os mapas e sistemas de Veil Falls e Cloud Tops.
 */
void veil_clouds_init(void);

/*
 * Entra na Base das Quedas do Véu vindo de North Hyrule Field.
 */
void veil_clouds_enter_falls(float* player_x, float* player_y, Direction* player_dir);

/*
 * Entra em Cloud Tops através do grande redemoinho ascensional do topo.
 */
void veil_clouds_enter_clouds(float* player_x, float* player_y, Direction* player_dir);

/*
 * Retorna à superfície de Hyrule.
 */
void veil_clouds_exit(float* player_x, float* player_y, Direction* player_dir);

/*
 * Retorna true se Veil Falls ou Cloud Tops estiver ativo no game loop.
 */
bool veil_clouds_is_active(void);

/*
 * Retorna true se a cena atual estiver no Topo das Nuvens (Cloud Tops).
 */
bool veil_clouds_is_in_clouds(void);

/*
 * Retorna a cena atual.
 */
VeilCloudSceneId veil_clouds_get_scene(void);

/*
 * Retorna o ponteiro para o Tilemap da cena ativa.
 */
Tilemap* veil_clouds_get_current_map(void);

/*
 * Atualiza partículas, redemoinhos de vento, catapultas aéreas, transições e fusões Kinstone.
 */
void veil_clouds_update(float* player_x, float* player_y, Direction player_dir, bool is_moving,
                        int* player_hearts, int* player_rupees, bool has_mole_mitts);

/*
 * Renderiza o cenário da cena ativa, redemoinhos de vento animados, névoa e partículas celestes.
 */
void veil_clouds_render(const Camera* camera, float player_x, float player_y);

/*
 * Interação do botão [A]: Fusão com Anciãos da Tribo do Vento ou acionamento de redemoinhos.
 */
bool veil_clouds_interact(float player_x, float player_y, Direction dir);

/*
 * Escavação com Mole Mitts em nuvens cinzentas de tempestade.
 */
bool veil_clouds_dig_cloud(float world_x, float world_y, int* out_rupees);

/*
 * Retorna o número de Kinstones Douradas já fundidas (0 a 5).
 */
u8 veil_clouds_get_golden_kinstones(void);

/*
 * Define o número de Kinstones Douradas fundidas (para carregamento de save).
 */
void veil_clouds_set_golden_kinstones(u8 count);

/*
 * Retorna true se o Grande Tornado para o Palace of Winds está desbloqueado.
 */
bool veil_clouds_is_tornado_active(void);

/*
 * Finaliza e libera memória alocada dos mapas.
 */
void veil_clouds_shutdown(void);

#endif // HAL_VEIL_CLOUDS_H
