#ifndef HAL_SANCTUARY_H
#define HAL_SANCTUARY_H

/*
 * ============================================================================
 * include/hal/sanctuary.h - Santuário Elemental & Four Sword Completa Forjada (4 Clones)
 * ============================================================================
 * O Santuário Elemental reside no coração do Castelo de Hyrule.
 * Link deposita a White Sword no pedestal sagrado:
 * 1. Infusão 2 Elementos (Terra + Fogo) -> White Sword (Two Elements) -> 2 Links (Link + Clone Vermelho)
 * 2. Infusão 3 Elementos (Terra + Fogo + Água) -> White Sword (Three Elements) -> 3 Links (Link + Vermelho + Azul)
 * 3. Infusão 4 Elementos (Terra + Fogo + Água + Vento) -> FOUR SWORD COMPLETA FORJADA!
 *    - 4 Links simultâneos: Green Link (Original), Red Link, Blue Link e Purple Link (Violeta)!
 *    - 4 Pisos Mágicos de Divisão (Diamond/Square Clone Formation).
 *    - Sword Beams (Raios de Espada) radiantes disparados com HP cheio!
 *    - Quebra-cabeça de 4 Interruptores pesados simultâneos.
 *    - Abertura da passagem secreta atrás dos vitrais para o Royal Valley & Gustaf's Tomb!
 */

#include "gba/types.h"
#include "hal/map.h"
#include <stdbool.h>

#define SANCTUARY_ROOM_W 16
#define SANCTUARY_ROOM_H 12
#define MAX_CLONE_LIFETIME 600 // 10 segundos
#define MAX_SANCTUARY_CLONES 3 // 3 clones + Link original = 4 heróis simultâneos!
#define MAX_SANCTUARY_PADS   4 // Formação clássica de 4 pisos de divisão
#define MAX_SWORD_BEAMS      4 // Limite de raios de espada simultâneos

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
    u32         tunic_color;   // Cor heráldica da túnica (0: Vermelho, 1: Azul, 2: Roxo/Violeta)
    u32         cap_color;     // Cor do capuz
    u32         aura_color;    // Cor do brilho Four Sword
} CloneState;

// Bloco / Piso de Divisão Elemental (Glowing Clone Floor Tile / Pad)
typedef struct {
    float x;
    float y;
    bool  is_active; // Iluminado quando Link carrega a espada sobre ele
} ClonePad;

// Projétil do Raio de Espada (Four Sword Beam)
typedef struct {
    bool      active;
    float     x;
    float     y;
    float     vx;
    float     vy;
    Direction dir;
    int       life;
    int       dmg;
} SwordBeam;

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
    bool        pedestal_infused_four; // FOUR SWORD FORJADA: Terra, Fogo, Água e Vento!
    bool        cutscene_playing;      // Em execução cinemática da infusão
    bool        cutscene_is_three;     // true se a cutscene é de 3 elementos
    bool        cutscene_is_four;      // true se a cutscene é da forja dos 4 elementos!
    int         cutscene_timer;        // Contador de quadros da cutscene
    float       pedestal_x;            // Posição central do Altar
    float       pedestal_y;
    float       earth_orb_x, earth_orb_y;
    float       fire_orb_x,  fire_orb_y;
    float       water_orb_x, water_orb_y;
    float       wind_orb_x,  wind_orb_y; // Orbe místico do Elemento do Vento

    // Sistema de Pisos de Clones no Santuário (4 pisos em formação sagrada)
    ClonePad    pads[MAX_SANCTUARY_PADS];
    bool        pad_charged[MAX_SANCTUARY_PADS];

    // Clones Ativos do Herói (Clone 0 = Red, Clone 1 = Blue, Clone 2 = Purple)
    CloneState  clones[MAX_SANCTUARY_CLONES];

    // Quebra-cabeça de Interruptores Quádruplos
    bool        switch_down[4];
    bool        gate_open;             // Primeiro portão elevado (2 interruptores)
    bool        treasury_gate_open;    // Segundo portão elevado (3 interruptores)
    bool        secret_exit_unlocked;  // Passagem secreta norte destravada (4 interruptores / 4 elementos)

    // Bloco Colossal de Empurrão dos Heróis
    float       heavy_block_x;
    float       heavy_block_y;
    bool        heavy_block_pushed;

    // Subsistema de Raios de Espada (Sword Beams)
    SwordBeam   beams[MAX_SWORD_BEAMS];
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
 * Atualização de física, infusão, 4 clones, blocos e quebra-cabeças no Santuário.
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
 * Renderiza os clones ativos do Herói (Red Link, Blue Link e Purple Link).
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
 * Retorna true se Link forjou a Four Sword Completa com os 4 Elementos sagrados!
 */
bool sanctuary_has_four_elements(void);

/*
 * Define manualmente o estado de infusão (para carregamento de savegames).
 */
void sanctuary_set_two_elements(bool infused);
void sanctuary_set_three_elements(bool infused);
void sanctuary_set_four_elements(bool infused);

/*
 * Retorna true se qualquer clone da Four Sword estiver ativo.
 */
bool sanctuary_is_clone_active(void);

/*
 * Retorna o número de clones atualmente ativos (0 a 3).
 */
int sanctuary_get_active_clone_count(void);

/*
 * Retorna o ponteiro para o clone principal (Clone 0).
 */
CloneState* sanctuary_get_clone(void);

/*
 * Retorna o ponteiro para o clone no índice especificado (0, 1 ou 2).
 */
CloneState* sanctuary_get_clone_at(int idx);

/*
 * Aciona o golpe de espada sincronizado do Clone contra inimigos (Clone 0 para retrocompatibilidade).
 */
bool sanctuary_get_clone_sword_hitbox(float* out_x, float* out_y, float* out_w, float* out_h, int* out_dmg);

/*
 * Obtém hitbox de golpe de espada para um clone específico (0 a 2).
 */
bool sanctuary_get_any_clone_sword_hitbox(int clone_idx, float* out_x, float* out_y, float* out_w, float* out_h, int* out_dmg);

/*
 * Interage com o Altar Pedestal no Santuário (pressionar [A]).
 * Suporta infusão de 2, 3 e 4 Elementos sagrados.
 */
bool sanctuary_interact(float link_x, float link_y, bool has_white_sword,
                        bool has_earth_element, bool has_fire_element,
                        bool has_water_element, bool has_wind_element);

/*
 * Tenta empurrar o Bloco Colossal dos Heróis. Retorna true se foi empurrado.
 */
bool sanctuary_push_heavy_block(float link_x, float link_y, Direction dir, bool is_moving);

/*
 * Dispara um Raio de Espada (Four Sword Beam) se Link possui a Four Sword e HP cheio.
 */
void sanctuary_try_fire_sword_beam(float link_x, float link_y, Direction dir,
                                   int hearts, int max_hearts, bool has_four_sword);

/*
 * Atualiza trajetória e impactos dos Raios de Espada.
 */
void sanctuary_update_sword_beams(Tilemap* map);

/*
 * Renderiza os Raios de Espada brilhantes no ar.
 */
void sanctuary_render_sword_beams(const Camera* cam);

/*
 * Checa impacto do Raio de Espada contra uma caixa delimitadora de alvo.
 */
bool sanctuary_check_sword_beam_hit(float target_x, float target_y, float target_w, float target_h, int* out_dmg);

#endif // HAL_SANCTUARY_H
