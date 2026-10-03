#ifndef HAL_ENTITY_H
#define HAL_ENTITY_H

/*
 * ============================================================================
 * include/hal/entity.h - Arquitetura de Entidades, Atores e IA de Inimigos
 * ============================================================================
 * Inspirado diretamente na arquitetura original do decompilador zeldaret/tmc:
 * Modela a OAM (Object Attribute Memory) do GBA com máquinas de estado (FSM),
 * caixas de colisão (hitboxes), IA de patrulha e disparo, knockback e drop de itens.
 */

#include "gba/types.h"
#include "hal/map.h"
#include "hal/texture.h"
#include <stdbool.h>

#define MAX_ENTITIES 32

// Tipos de Entidades
typedef enum {
    ENTITY_NONE = 0,
    ENTITY_ENEMY_OCTOROK,     // Inimigo clássico Octorok Vermelho
    ENTITY_PROJECTILE_ROCK,   // Pedra disparada pelo Octorok
    ENTITY_ITEM_RUPEE,        // Rupee Verde (+5) dropado no chão
    ENTITY_ITEM_HEART,        // Coração de cura (+1 HP) dropado
    ENTITY_NPC_FOREST_MINISH  // Habitante Minish amigável dos bosques
} EntityType;

// Caixa delimitadora de colisão e dano (Hitbox / Hurtbox)
typedef struct {
    float offset_x;
    float offset_y;
    float width;
    float height;
} Hitbox;

// Estrutura canônica de Entidade (Actor)
typedef struct Entity {
    u8         type;               // Tipo da entidade (EntityType)
    u8         action;             // Estado principal (0=Init, 1=Patrulha, 2=Alerta, 3=Disparo, 4=Recuo, 5=Morte)
    u8         subAction;          // Sub-estágio da ação
    float      x;                  // Coordenada X no mundo (float sub-pixel)
    float      y;                  // Coordenada Y no mundo (float sub-pixel)
    float      vx;                 // Vetor de velocidade X
    float      vy;                 // Vetor de velocidade Y
    Direction  dir;                // Orientação visual (DIR_DOWN, DIR_UP, etc.)
    int        health;             // Vida atual
    int        maxHealth;          // Vida máxima
    int        damage;             // Dano de contato infligido ao herói
    int        invulnerableTimer;  // Temporizador de piscar em vermelho/branco ao levar dano
    int        knockbackTimer;     // Duração do recuo/empurrão
    float      knockbackVx;        // Vetor de empurrão X
    float      knockbackVy;        // Vetor de empurrão Y
    int        aiTimer;            // Cronômetro de decisão da IA
    int        animTimer;          // Cronômetro de animação
    int        animFrame;          // Frame do sprite atual
    Hitbox     hitbox;             // Caixa de colisão no corpo
    bool       is_active;          // Slot ocupado no pool
} Entity;

/*
 * Inicializa o gerenciador de entidades e limpa o pool de memória.
 */
void entity_manager_init(void);

/*
 * Spawna uma nova entidade no mundo.
 */
Entity* entity_spawn(EntityType type, float world_x, float world_y);

/*
 * Atualiza o ciclo de vida, IA, físicas de colisão e projéteis de todas as entidades ativas.
 * Atualiza também a vida, rupees e recuo do Link caso seja atingido.
 */
void entity_manager_update(const Tilemap* map, float link_x, float link_y,
                           int* link_hearts, int* link_rupees,
                           int* link_invuln_timer, float* link_knock_x, float* link_knock_y);

/*
 * Verifica se o golpe de espada do Link atingiu algum inimigo ou projétil ativo.
 * Retorna true se um golpe conectou (tocando som de impacto e causando knockback no monstro).
 */
bool entity_check_sword_hit(float slash_x, float slash_y, float slash_w, float slash_h,
                            int damage, Direction slash_dir);

/*
 * Procura um NPC amigável próximo às coordenadas fornecidas dentro do raio max_dist.
 * Retorna o ponteiro para a entidade do NPC ou NULL se nenhum estiver por perto.
 */
Entity* entity_find_nearby_npc(float world_x, float world_y, float max_dist);

/*
 * Renderiza todas as entidades ativas nas coordenadas relativas da câmera virtual.
 */
void entity_manager_render(const Camera* cam);

/*
 * Define a folha de sprites das entidades (octorok.bmp).
 */
void entity_set_texture(const Texture* tex);

/*
 * Finaliza o subsistema de entidades.
 */
void entity_manager_shutdown(void);

#endif // HAL_ENTITY_H
