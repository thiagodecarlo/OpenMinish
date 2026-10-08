#ifndef HAL_ROYAL_VALLEY_H
#define HAL_ROYAL_VALLEY_H

/*
 * ============================================================================
 * include/hal/royal_valley.h - Royal Valley, Cemitério & Tumba do Rei Gustaf
 * ============================================================================
 * Modela a jornada mística do Ato V de The Legend of Zelda: The Minish Cap:
 *
 * 1. Royal Valley (Vale Real):
 *    - Floresta ancestral sombria coberta por névoa espectral perpétua.
 *    - Labirinto de Névoa Espectral (Mist Maze): sequência canônica de direções
 *      (Sul, Oeste, Oeste, Norte, Leste, Norte). Errar a rota reseta a travessia.
 *
 * 2. Cabana de Dampé o Coveiro:
 *    - Dampé confia ao Herói a Graveyard Key (Chave do Cemitério).
 *    - O corvo Takkar rouba a chave e foge para o galho de uma árvore retorcida.
 *    - Link abate o corvo (bumerangue, arco ou golpe) para recuperar a chave e
 *      abrir os portões monumentais de ferro forjado do cemitério!
 *
 * 3. Cemitério Ancestral de Hyrule (The Graveyard):
 *    - Lápides cinzentas, musgo, árvores com olhos e espíritos errantes (Ghinis e Poes).
 *    - Lápides móveis que revelam passagens e baús.
 *    - Pisos de Divisão da Four Sword (Split Pads): formação dos 4 Heróis para
 *      empurrar a tampa colossal da Tumba do Rei Gustaf!
 *
 * 4. Cripta Real do Rei Gustaf (Royal Crypt):
 *    - Catacumbas subterrâneas com tochas apagadas (acendíveis com a Flame Lantern),
 *      armadilhas Blade Traps e múmias Gibdos.
 *    - Quebra-cabeça de 4 interruptores simultâneos acionados pelos 4 Heróis da Four Sword.
 *    - Encontro místico com o Espírito do Rei Gustaf: o antigo soberano saúda o Herói,
 *      concede a sagrada Kinstone Dourada Real (Royal Golden Kinstone) e a chave espiritual
 *      para penetrar as trevas de Vaati no Castelo de Hyrule (Dark Hyrule Castle)!
 */

#include "gba/types.h"
#include "hal/map.h"
#include <stdbool.h>

#define ROYAL_MAP_W 16
#define ROYAL_MAP_H 10
#define MAX_ROYAL_FOG 24
#define MAX_ROYAL_GHOSTS 4
#define MAX_ROYAL_PADS 4
#define MAX_ROYAL_SWITCHES 4

// Cenas da Região de Royal Valley
typedef enum {
    ROYAL_SCENE_ENTRANCE = 0,    // Entrada do Vale Real: árvores góticas e placas ancestrais
    ROYAL_SCENE_MIST_MAZE,       // Labirinto de Névoa Espectral (Lost Woods Fog Maze)
    ROYAL_SCENE_DAMPE_CABIN,     // Cabana de Dampé, portão trancado e evento do corvo Takkar
    ROYAL_SCENE_GRAVEYARD,       // Cemitério Ancestral, lápides, Ghinis e Tumba monumental de Gustaf
    ROYAL_SCENE_ROYAL_CRYPT,     // Cripta Subterrânea do Rei Gustaf, Gibdos, tochas e sarcófago
    ROYAL_SCENE_COUNT
} RoyalValleySceneId;

// Partícula de Névoa Espectral
typedef struct {
    bool  active;
    float x;
    float y;
    float vx;
    float vy;
    float size;
    u32   color;
    int   life;
} RoyalFogParticle;

// Entidade Espectral (Ghini ou Poe)
typedef struct {
    bool  active;
    float x;
    float y;
    float vx;
    float vy;
    int   hp;
    int   invuln;
    bool  is_poe;        // true = Poe azul com lanterna; false = Ghini
    float anim_timer;
} RoyalGhost;

// Estado Geral da Região de Royal Valley
typedef struct {
    bool                is_active;
    RoyalValleySceneId  current_scene;
    Tilemap*            maps[ROYAL_SCENE_COUNT];

    // Partículas de névoa e atmosfera sombria
    RoyalFogParticle    fog[MAX_ROYAL_FOG];
    int                 fog_timer;
    int                 scene_anim_timer;

    // Mecânica do Labirinto de Névoa (Mist Maze)
    int                 maze_step;          // Progresso de 0 a 6
    bool                maze_failed;        // Ativado ao errar o caminho
    int                 maze_fail_timer;    // Flash de névoa esbranquiçada ao reiniciar

    // Cabana de Dampé e Evento da Chave do Cemitério
    bool                dampe_met;          // Já conversou com Dampé
    bool                crow_has_key;       // Corvo Takkar roubou a chave
    float               crow_x;
    float               crow_y;
    bool                crow_defeated;      // Corvo abatido
    float               key_x;
    float               key_y;
    bool                key_on_ground;      // Chave brilhando no chão pronta para coleta
    bool                graveyard_unlocked; // Portão do cemitério aberto

    // Cemitério e Tumba do Rei Gustaf
    bool                tomb_pushed;        // Tumba aberta com a força dos 4 Heróis
    float               tomb_offset_y;      // Deslocamento da tampa de pedra
    RoyalGhost          ghosts[MAX_ROYAL_GHOSTS];
    int                 ghost_count;

    // Pisos de Divisão da Four Sword (Graveyard e Crypt)
    float               split_pad_x[MAX_ROYAL_PADS];
    float               split_pad_y[MAX_ROYAL_PADS];
    bool                split_pad_charged[MAX_ROYAL_PADS];

    // Cripta Real do Rei Gustaf
    bool                crypt_torches_lit[2]; // Tochas da câmara fúnebre acesas com Flame Lantern
    bool                crypt_switches[MAX_ROYAL_SWITCHES]; // 4 interruptores simultâneos
    bool                crypt_gate_open;      // Portão do sarcófago aberto
    bool                king_gustaf_met;      // Já encontrou o Espírito do Rei Gustaf
    bool                has_royal_kinstone;   // Kinstone Real Dourada obtida!
    int                 gustaf_anim;          // Pulso e flutuação do espírito
    int                 banner_royal_timer;   // Temporizador do banner da Kinstone Real

    // Posições seguras para transição entre cenas
    float               safe_x;
    float               safe_y;
} RoyalValleyState;

/*
 * Inicializa mapas, partículas e estados de Royal Valley.
 */
void royal_valley_init(void);

/*
 * Libera os recursos alocados para a região.
 */
void royal_valley_shutdown(void);

/*
 * Retorna true se Royal Valley está ativo.
 */
bool royal_valley_is_active(void);

/*
 * Obtém a cena atualmente em exibição.
 */
RoyalValleySceneId royal_valley_get_scene(void);

/*
 * Link entra em Royal Valley vindo do Santuário Elemental / North Field.
 */
void royal_valley_enter_entrance(float* link_x, float* link_y, Direction* link_dir);

/*
 * Link entra no Labirinto de Névoa Espectral.
 */
void royal_valley_enter_maze(float* link_x, float* link_y, Direction* link_dir);

/*
 * Link entra na área da Cabana de Dampé.
 */
void royal_valley_enter_dampe(float* link_x, float* link_y, Direction* link_dir);

/*
 * Link entra no Cemitério de Hyrule.
 */
void royal_valley_enter_graveyard(float* link_x, float* link_y, Direction* link_dir);

/*
 * Link entra na Cripta Real do Rei Gustaf.
 */
void royal_valley_enter_crypt(float* link_x, float* link_y, Direction* link_dir);

/*
 * Link sai de Royal Valley retornando aos campos de Hyrule.
 */
void royal_valley_exit(float* link_x, float* link_y, Direction* link_dir);

/*
 * Atualiza física, névoa, labirinto, corvo, fantasmas e quebra-cabeças de Royal Valley.
 */
void royal_valley_update(float* link_x, float* link_y, Direction* link_dir,
                         bool link_moving, int* link_hearts, int max_hearts, int* link_rupees,
                         bool is_charging_spin, bool spin_ready,
                         bool is_attacking, int attack_timer,
                         bool has_four_sword, bool has_lantern, bool lantern_lit);

/*
 * Renderiza o cenário da cena ativa, névoa volumétrica, personagens, lápides e o Rei Gustaf.
 */
void royal_valley_render(const Camera* cam, float link_x, float link_y, Direction link_dir, bool is_minish);

/*
 * Retorna true se a coordenada do mundo colide com paredes, lápides ou grades fechadas.
 */
bool royal_valley_is_solid(float world_x, float world_y);

/*
 * Link interage com o cenário [Botão A] (conversar com Dampé, examinar lápide, etc).
 */
bool royal_valley_interact(float link_x, float link_y, Direction link_dir, int* link_hearts);

/*
 * Checa impacto do golpe de espada contra o Corvo Takkar, fantasmas Ghinis ou tochas.
 */
bool royal_valley_check_sword_hit(float hit_x, float hit_y, float hit_w, float hit_h, int dmg, Direction dir);

/*
 * Checa impacto de projéteis (Flechas ou Bumerangue) contra o corvo Takkar.
 */
bool royal_valley_check_projectile_hit(float proj_x, float proj_y);

/*
 * Retorna true se Link já conquistou a sagrada Royal Golden Kinstone do Rei Gustaf.
 */
bool royal_valley_has_royal_kinstone(void);

/*
 * Retorna true se a Tumba do Rei Gustaf já foi aberta pelos 4 Heróis.
 */
bool royal_valley_is_tomb_opened(void);

/*
 * Define manualmente estados para restauração de saves.
 */
void royal_valley_restore_state(bool graveyard_unlocked, bool dampe_met, bool tomb_pushed, bool king_gustaf_met, bool has_kinstone);

/*
 * Obtém ponteiro direto para o estado de Royal Valley.
 */
RoyalValleyState* royal_valley_get_state(void);

#endif // HAL_ROYAL_VALLEY_H
