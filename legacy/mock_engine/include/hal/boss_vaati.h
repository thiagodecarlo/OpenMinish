#ifndef HAL_BOSS_VAATI_H
#define HAL_BOSS_VAATI_H

/*
 * ============================================================================
 * include/hal/boss_vaati.h - Confronto Final Épico: Feiticeiro Vaati (Ato V)
 * ============================================================================
 * Modela a batalha final canônica em 3 fases climáticas de The Minish Cap:
 *
 * Fase 1: VAATI REBORN (O Feiticeiro Renascido)
 *   - Feiticeiro com manto púrpura levitando, protegido por 4 olhos de energia negra.
 *   - Teletransporte, esferas de fogo e projéteis teleguiados de malícia.
 *   - Destruir ou afastar os orbs expõe o olho peitoral para golpes de Four Sword.
 *
 * Fase 2: VAATI TRANSFIGURED (A Besta Maligna Alada)
 *   - Metamorfose demoníaca em uma colossal esfera ocular alada.
 *   - 8 Olhos flutuantes em órbita: flechas revelam os 4 olhos vermelhos vulneráveis.
 *   - Os 4 clones da Four Sword devem golpear os 4 olhos simultaneamente para derrubar a besta!
 *
 * Fase 3: VAATI'S WRATH (A Fúria Demoníaca Final)
 *   - Forma final colossal com asas sombrias e 2 garras gigantescas destacadas.
 *   - As garras brotam do solo: virar com Cane of Pacci -> encolher com Minish Stump
 *     e destruir os parasitas internos de malícia!
 *   - Olho principal dispara 4 feixes de laser que os 4 Clones refletem com escudos,
 *     atordoando Vaati para o golpe de misericórdia final!
 *
 * Epílogo & Créditos:
 *   - Cura da Princesa Zelda da petrificação com o poder da Four Sword e do Cap of Wishes.
 *   - Despedida emocionante do sábio Minish Ezlo em sua forma original.
 *   - Rolagem canônica dos créditos finais de The Legend of Zelda: The Minish Cap!
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/entity.h"
#include "hal/texture.h"
#include <stdbool.h>

void boss_vaati_set_npcs_texture(const Texture* tex);

typedef enum {
    VAATI_PHASE_1_REBORN = 0,       // Fase 1: Feiticeiro Vaati com 4 Orbs
    VAATI_PHASE_2_TRANSFIGURED,     // Fase 2: Besta Alada com 8 Olhos e 4 Clones
    VAATI_PHASE_3_WRATH,            // Fase 3: Fúria Demoníaca com Garras e Lasers
    VAATI_PHASE_DEFEAT_CINEMATIC,   // Epílogo: Cura de Zelda e Despedida de Ezlo
    VAATI_PHASE_CREDITS,            // Créditos finais do jogo
    VAATI_PHASE_COUNT
} VaatiPhase;

#define MAX_VAATI_ORBS 8
#define MAX_VAATI_CLAWS 2
#define MAX_VAATI_PROJECTILES 6

typedef struct {
    float x;
    float y;
    float angle;
    int   hp;
    bool  active;
    bool  revealed;
} VaatiOrb;

typedef struct {
    float x;
    float y;
    bool  active;
    bool  burrowed;
    bool  flipped;
    int   parasite_hp;
    bool  destroyed;
    int   burrow_timer;
} VaatiClaw;

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    bool  active;
    int   lifetime;
    int   type; // 0: Malice Orb, 1: Fireball, 2: Laser
} VaatiProjectile;

typedef struct {
    bool             is_active;
    VaatiPhase       phase;
    int              hp;
    int              max_hp;
    float            x;
    float            y;
    float            vx;
    float            vy;
    int              action_timer;
    int              teleport_timer;
    bool             eye_open;
    int              eye_open_timer;
    bool             stunned;
    int              stun_timer;
    int              hit_flash;

    // Fase 1: 4 Orbs protetores
    VaatiOrb         orbs[MAX_VAATI_ORBS];
    int              orb_count;

    // Fase 2: Besta Alada
    float            wing_anim_angle;
    bool             quad_eyes_revealed;
    int              quad_hit_mask;

    // Fase 3: Garras e Lasers
    VaatiClaw        claws[MAX_VAATI_CLAWS];
    bool             inside_claw_minish;
    int              current_claw_index;
    bool             laser_firing;
    int              laser_reflect_mask;

    // Projéteis
    VaatiProjectile  projectiles[MAX_VAATI_PROJECTILES];

    // Epílogo & Créditos
    bool             zelda_cured;
    int              zelda_cinematic_timer;
    bool             ezlo_farewell;
    int              credits_scroll_y;
    bool             game_complete;

    // Mapa de arena (Teto do Castelo / Vórtice Astral)
    Tilemap*         arena_map;
    int              storm_flash_timer;
} VaatiBoss;

// Inicialização e Ciclo de Vida
void vaati_boss_init(void);
void vaati_boss_start(float* link_x, float* link_y, Direction* link_dir);
void vaati_boss_exit(float* link_x, float* link_y, Direction* link_dir);
bool vaati_boss_is_active(void);
VaatiPhase vaati_boss_get_phase(void);
VaatiBoss* vaati_boss_get_state(void);
Tilemap* vaati_boss_get_arena_map(void);

// Atualização de Lógica (60 FPS)
void vaati_boss_update(float* link_x, float* link_y, Direction* link_dir,
                       bool is_attacking, bool is_spin, int clone_count,
                       const float clone_x[3], const float clone_y[3],
                       bool is_minish, int* player_hearts);

// Interações de Combate
void vaati_boss_strike_sword(float link_x, float link_y, Direction link_dir,
                             int damage, bool is_spin, int clone_count,
                             const float clone_x[3], const float clone_y[3]);
void vaati_boss_arrow_hit(float arrow_x, float arrow_y);
void vaati_boss_cane_hit(float cane_x, float cane_y);

// Renderização na HAL Video
void vaati_boss_render(const Camera* cam, float link_x, float link_y, bool is_minish);

// Estado e Conclusão
bool vaati_boss_is_defeated(void);
bool vaati_boss_is_credits(void);

#endif // HAL_BOSS_VAATI_H
