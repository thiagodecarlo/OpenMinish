#ifndef HAL_SANCTUARY_H
#define HAL_SANCTUARY_H

/*
 * ============================================================================
 * include/hal/sanctuary.h - Santuário Elemental & Divisão Four Sword (2 Clones)
 * ============================================================================
 * O Santuário Elemental reside no coração do Castelo de Hyrule.
 * Link deposita a White Sword forjada no pedestal sagrado, onde a infusão
 * dos dois primeiros elementos (Terra e Fogo) desperta o lendário poder
 * da lâmina dividida: a geração de Clones de si mesmo em blocos emissores!
 */

#include "gba/types.h"
#include "hal/map.h"
#include <stdbool.h>

#define SANCTUARY_ROOM_W 16
#define SANCTUARY_ROOM_H 12
#define MAX_CLONE_LIFETIME 600 // 10 segundos

// Estado de um Clone gerado pela Four Sword
typedef struct {
    bool        active;
    float       x;
    float       y;
    Direction   dir;
    bool        is_moving;
    bool        is_attacking;
    int         attack_timer;
    int         anim_timer;
    int         anim_frame;
    float       offset_x;      // Distância vetorial constante relativa a Link
    float       offset_y;
    int         lifetime;      // Duração antes de dissipar em fumaça mágica
    int         sparkle_timer; // Brilho elemental constante
} CloneState;

// Bloco / Piso de Divisão Elemental (Glowing Clone Floor Tile / Pad)
typedef struct {
    float x;
    float y;
    bool  is_active; // Iluminado quando Link carrega a espada sobre ele
} ClonePad;

// Estado Geral do Santuário Elemental
typedef struct {
    bool        active;
    bool        transitioning;
    int         trans_timer;
    float       safe_x;
    float       safe_y;

    // Altar dos Elementos & Cerimônia de Infusão
    bool        pedestal_infused;      // Espada Branca já infundida com Terra e Fogo
    bool        cutscene_playing;      // Em execução cinemática da infusão
    int         cutscene_timer;        // Contador de quadros da cutscene
    float       pedestal_x;            // Posição central do Altar
    float       pedestal_y;
    float       earth_orb_x, earth_orb_y;
    float       fire_orb_x, fire_orb_y;

    // Sistema de Pisos de Clones no Santuário
    ClonePad    pads[2];               // Par de pisos mágicos de divisão (distância de 2 tiles)
    bool        pad1_charged;
    bool        pad2_charged;

    // Clone Ativo do Herói
    CloneState  clone;

    // Quebra-cabeça de Interruptores Duplos
    bool        switch_left_down;      // Interruptor de piso da esquerda (pressionado)
    bool        switch_right_down;     // Interruptor de piso da direita (pressionado)
    bool        gate_open;             // Portão sagrado elevado
} ElementalSanctuaryState;

/*
 * Inicializa os dados e estado do Santuário Elemental.
 */
void sanctuary_init(void);

/*
 * Retorna true se o Santuário Elemental está atualmente ativo.
 */
bool sanctuary_is_active(void);

/*
 * Obtém o ponteiro de estado interno do Santuário.
 */
ElementalSanctuaryState* sanctuary_get_state(void);

/*
 * Link entra no Santuário Elemental vindo do North Hyrule Field.
 */
void sanctuary_enter(float* link_x, float* link_y, Direction* link_dir);

/*
 * Link sai do Santuário Elemental de volta aos Campos de Hyrule.
 */
void sanctuary_exit(float* link_x, float* link_y, Direction* link_dir);

/*
 * Atualização de física, infusão, clones e quebra-cabeças no Santuário.
 */
void sanctuary_update(float* link_x, float* link_y, Direction* link_dir,
                      bool link_moving, bool is_charging, bool is_charge_ready,
                      bool is_attacking, int attack_timer,
                      int* link_hearts);

/*
 * Renderiza o cenário arquitetônico sagrado do Santuário, vitrais, altar e pisos mágicos.
 */
void sanctuary_render(const Camera* cam, float link_x, float link_y, Direction link_dir);

/*
 * Renderiza o clone ativo do Herói (com cores fiéis da Four Sword e partículas elementais).
 */
void sanctuary_render_clone(const Camera* cam);

/*
 * Retorna true se uma coordenada colide com paredes, pedestais ou grades fechadas.
 */
bool sanctuary_is_solid(float world_x, float world_y);

/*
 * Retorna true se Link completou a infusão dos 2 Elementos (Terra + Fogo).
 */
bool sanctuary_has_two_elements(void);

/*
 * Define manualmente o estado de infusão (para carregamento de savegames).
 */
void sanctuary_set_two_elements(bool infused);

/*
 * Retorna true se o 2º Clone da Four Sword está ativo.
 */
bool sanctuary_is_clone_active(void);

/*
 * Retorna o ponteiro para o clone ativo.
 */
CloneState* sanctuary_get_clone(void);

/*
 * Aciona o golpe de espada sincronizado do Clone contra inimigos.
 */
bool sanctuary_get_clone_sword_hitbox(float* out_x, float* out_y, float* out_w, float* out_h, int* out_dmg);

/*
 * Interage com o Altar Pedestal no Santuário (pressionar [A]).
 */
bool sanctuary_interact(float link_x, float link_y, bool has_white_sword, bool has_earth_element, bool has_fire_element);

#endif // HAL_SANCTUARY_H
