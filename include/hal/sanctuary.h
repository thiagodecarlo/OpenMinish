#ifndef HAL_SANCTUARY_H
#define HAL_SANCTUARY_H

/*
 * ============================================================================
 * include/hal/sanctuary.h - Santuário Elemental & Divisão Four Sword (3 Clones)
 * ============================================================================
 * O Santuário Elemental reside no coração do Castelo de Hyrule.
 * Link deposita a White Sword forjada no pedestal sagrado:
 * 1. Infusão 2 Elementos (Terra + Fogo) -> White Sword (Two Elements) -> 2 Links (Link + 1 Clone Vermelho)
 * 2. Infusão 3 Elementos (Terra + Fogo + Água) -> White Sword (Three Elements) -> 3 Links (Link + Clone Vermelho + Clone Azul)
 *
 * Características da Divisão em 3 Clones:
 * - 3 Pisos Mágicos de Divisão (Clone Pads) no piso sagrado.
 * - Carregamento sequencial do ataque giratório (Spin Attack) nos 3 blocos.
 * - Geração sincronizada de 2 clones:
 *   - Clone 1: Red Link (Túnica e Gorro vermelhos, aura de chamas Four Sword)
 *   - Clone 2: Blue Link (Túnica e Gorro azuis, aura de água cristalina Four Sword)
 * - Ambos os clones replicam a orientação, movimentação, animações e golpes de espada de Link.
 * - Quebra-cabeça de Interruptores Triplos: 3 interruptores de piso pesados que exigem
 *   que Link e os 2 clones pisem simultaneamente para destravar o Grande Portão do Tesouro Sagrado!
 * - Empurrão de Bloco Colossal (Heavy Push Block): pedra monumental de 3 heróis que requer
 *   a força combinada dos 3 guerreiros empurrando na mesma direção!
 */

#include "gba/types.h"
#include "hal/map.h"
#include <stdbool.h>

#define SANCTUARY_ROOM_W 16
#define SANCTUARY_ROOM_H 12
#define MAX_CLONE_LIFETIME 600 // 10 segundos
#define MAX_SANCTUARY_CLONES 2 // 2 clones + Link original = 3 heróis simultâneos!
#define MAX_SANCTUARY_PADS   3 // Trio de pisos mágicos de divisão

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
    u32         tunic_color;   // Cor heráldica da túnica (Clone 0: Vermelho, Clone 1: Azul)
    u32         cap_color;     // Cor do capuz
    u32         aura_color;    // Cor do brilho Four Sword
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
    bool        pedestal_infused;      // Espada Branca infundida com Terra e Fogo (2 Elementos)
    bool        pedestal_infused_three;// Espada Branca infundida com Terra, Fogo e Água (3 Elementos)
    bool        cutscene_playing;      // Em execução cinemática da infusão
    bool        cutscene_is_three;     // true se a cutscene é da infusão dos 3 elementos
    int         cutscene_timer;        // Contador de quadros da cutscene
    float       pedestal_x;            // Posição central do Altar
    float       pedestal_y;
    float       earth_orb_x, earth_orb_y;
    float       fire_orb_x,  fire_orb_y;
    float       water_orb_x, water_orb_y; // Orbe místico do Elemento da Água

    // Sistema de Pisos de Clones no Santuário (Trio de pisos)
    ClonePad    pads[MAX_SANCTUARY_PADS];
    bool        pad_charged[MAX_SANCTUARY_PADS];

    // Clones Ativos do Herói (Clone 0 = Red Link, Clone 1 = Blue Link)
    CloneState  clones[MAX_SANCTUARY_CLONES];

    // Quebra-cabeça de Interruptores Triplos (x=5.5, x=7.5, x=9.5, y=8.0)
    bool        switch_down[3];
    bool        gate_open;             // Primeiro portão elevado (2 interruptores)
    bool        treasury_gate_open;    // Segundo portão da câmara secreta elevado (3 interruptores)

    // Bloco Colossal de Empurrão dos 3 Heróis
    float       heavy_block_x;
    float       heavy_block_y;
    bool        heavy_block_pushed;
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
 * Atualização de física, infusão, 3 clones, blocos e quebra-cabeças no Santuário.
 */
void sanctuary_update(float* link_x, float* link_y, Direction* link_dir,
                      bool link_moving, bool is_charging, bool is_charge_ready,
                      bool is_attacking, int attack_timer,
                      int* link_hearts);

/*
 * Renderiza o cenário arquitetônico sagrado do Santuário, vitrais, altar, esferas e pisos mágicos.
 */
void sanctuary_render(const Camera* cam, float link_x, float link_y, Direction link_dir);

/*
 * Renderiza os clones ativos do Herói (Cores de Four Sword: Red Link e Blue Link).
 */
void sanctuary_render_clones(const Camera* cam);

/*
 * Retorna true se uma coordenada colide com paredes, pedestais, blocos ou grades fechadas.
 */
bool sanctuary_is_solid(float world_x, float world_y);

/*
 * Retorna true se Link completou a infusão dos 2 Elementos (Terra + Fogo).
 */
bool sanctuary_has_two_elements(void);

/*
 * Retorna true se Link completou a infusão dos 3 Elementos (Terra + Fogo + Água).
 */
bool sanctuary_has_three_elements(void);

/*
 * Define manualmente o estado de infusão (para carregamento de savegames).
 */
void sanctuary_set_two_elements(bool infused);
void sanctuary_set_three_elements(bool infused);

/*
 * Retorna true se qualquer clone da Four Sword estiver ativo.
 */
bool sanctuary_is_clone_active(void);

/*
 * Retorna o número de clones atualmente ativos (0, 1 ou 2).
 */
int sanctuary_get_active_clone_count(void);

/*
 * Retorna o ponteiro para o clone principal (Clone 0).
 */
CloneState* sanctuary_get_clone(void);

/*
 * Retorna o ponteiro para o clone no índice especificado (0 ou 1).
 */
CloneState* sanctuary_get_clone_at(int idx);

/*
 * Aciona o golpe de espada sincronizado do Clone contra inimigos (Clone 0 para retrocompatibilidade).
 */
bool sanctuary_get_clone_sword_hitbox(float* out_x, float* out_y, float* out_w, float* out_h, int* out_dmg);

/*
 * Obtém hitbox de golpe de espada para um clone específico.
 */
bool sanctuary_get_any_clone_sword_hitbox(int clone_idx, float* out_x, float* out_y, float* out_w, float* out_h, int* out_dmg);

/*
 * Interage com o Altar Pedestal no Santuário (pressionar [A]).
 * Suporta infusão de 2 Elementos e infusão de 3 Elementos.
 */
bool sanctuary_interact(float link_x, float link_y, bool has_white_sword,
                        bool has_earth_element, bool has_fire_element, bool has_water_element);

/*
 * Tenta empurrar o Bloco Colossal de 3 Heróis. Retorna true se foi empurrado.
 */
bool sanctuary_push_heavy_block(float link_x, float link_y, Direction dir, bool is_moving);

#endif // HAL_SANCTUARY_H
