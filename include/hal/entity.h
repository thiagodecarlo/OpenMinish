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
    ENTITY_NPC_FOREST_MINISH, // Habitante Minish amigável dos bosques
    ENTITY_ENEMY_KEESE,       // Morcego clássico Keese (Voo com sombra e oscilação)
    ENTITY_ENEMY_CHUCHU,      // Gosma gelatinosa Green ChuChu (Brota e salta)
    ENTITY_CHEST_GOLD,        // Baú do Tesouro Dourado (destravado por Fusão de Kinstone)
    ENTITY_BOSS_BIG_CHUCHU,   // Chefe Gigante do Deepwood Shrine (Big Green ChuChu)
    ENTITY_ITEM_HEART_CONTAINER, // Recipiente de Coração permanente (+1 Coração Máximo e Cura Total)
    ENTITY_NPC_SWIFTBLADE,    // Mestre Espadachim Swiftblade (Treinador do Spin Attack e Tiger Scrolls)
    ENTITY_NPC_SHOPKEEPER,    // Comerciante Stockwell (Dono da loja de Hyrule com balcão de compras)
    ENTITY_NPC_TOWN_CITIZEN,  // Cidadã da Cidade de Hyrule (NPC de praça com Kinstone)
    ENTITY_NPC_TOWN_GUARD,    // Guarda Real do Castelo de Hyrule (Cavaleiro em armadura)
    ENTITY_TOWN_FOUNTAIN,     // Chafariz central com jatos d'água borbulhantes animados
    ENTITY_MINISH_STUMP,      // Toco de Árvore / Portal Mágico de Encolhimento Minish (Minish Stump & Urn)
    ENTITY_NPC_GENTARI,       // Ancião da Vila Minish (Elder Gentari do Santuário)
    ENTITY_NPC_FESTARI,       // Sacerdote Minish Festari (Guardião da ermida para Deepwood Shrine)
    ENTITY_NPC_VILLAGE_MINISH,// Morador da Vila Minish (Picori civil com chapéu colorido)
    ENTITY_ENEMY_MOBLIN,      // Moblin com armadura de couro e lança afiada (Guardião dos campos)
    ENTITY_ENEMY_PEAHAT,      // Flor voadora helicóptero Peahat (Voa alto e aterrissa vulnerável)
    ENTITY_NPC_MALON,         // Malon da Fazenda Lon Lon (Moça do campo)
    ENTITY_ENEMY_TEKTITE,     // Tektite saltitante (Aracnídeo vulcânico com 4 patas que salta alto)
    ENTITY_ENEMY_SPINY_BEETLE,// Besouro com carapaça de pedra/arbusto que corre em investida
    ENTITY_NPC_BUSINESS_SCRUB,// Deku Scrub comerciante do Monte Crenel (Vendedor do Grip Ring)
    ENTITY_NPC_MELARI,        // Mestre Ferreiro Melari (Forjador da White Sword)
    ENTITY_NPC_MOUNTAIN_MINISH// Minerador / Aprendiz das Minas de Melari
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
    u8         action;             // Estado principal (0=Init, 1=Patrulha/Idle, 2=Alerta/Salto, 3=Ataque/Mergulho, 4=Recuo, 5=Morte)
    u8         subAction;          // Sub-estágio da ação
    float      x;                  // Coordenada X no mundo (float sub-pixel)
    float      y;                  // Coordenada Y no mundo (float sub-pixel)
    float      z;                  // Altitude Z / Altura acima do solo (para entidades voadoras como Keese)
    float      vx;                 // Vetor de velocidade X
    float      vy;                 // Vetor de velocidade Y
    float      vz;                 // Vetor de velocidade Z
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

    // Campos dedicados para NPCs e Fusão de Kinstones
    bool       hasKinstone;        // NPC possui metade de Kinstone para fundir
    u8         kinstoneType;       // 0=Verde, 1=Azul, 2=Vermelho
    bool       kinstoneFused;      // Fusão já foi completada
    float      bubbleBob;          // Oscilação do balão de Kinstone flutuante

    // Campos dedicados para o Chefe Big Green ChuChu
    float      bossBaseScale;      // Escala da base gelatinosa (1.0 -> 0.1 sugada pelo Pote Mágico)
    int        bossSuctionTimer;   // Duração de sucção contínua recebida
    int        bossToppleTimer;    // Temporizador do chefe desabado no chão vulnerável
    bool       bossEnraged;        // Fase 2 (HP <= 5): olhos vermelhos e saltos furiosos
    int        bossDeathTimer;     // Temporizador da animação dramática de derrota
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
 * Retorna a contagem de monstros vivos no mundo (Octorok, Keese, ChuChu, Chefe).
 */
int entity_count_active_enemies(void);

/*
 * Retorna true se o Big Green ChuChu estiver ativo e vivo na masmorra.
 */
bool entity_is_boss_alive(void);

/*
 * Retorna se o chefe Big Green ChuChu foi derrotado.
 */
bool entity_is_boss_defeated(void);

/*
 * Retorna os deslocamentos X e Y de tremor de tela (Screen Shake) ativos no momento.
 */
void entity_get_screen_shake(int* out_x, int* out_y);

/*
 * Dispara um efeito de tremor de tela com duração e magnitude especificadas.
 */
void entity_trigger_screen_shake(int duration_frames, int magnitude);

/*
 * Remove todas as entidades ativas do pool de memória.
 */
void entity_clear_all(void);

/*
 * Atualiza o ciclo de vida, IA, físicas de colisão e projéteis de todas as entidades ativas.
 * Atualiza também a vida, vida máxima, rupees e recuo do Link caso seja atingido ou colete itens.
 */
void entity_manager_update(const Tilemap* map, float link_x, float link_y,
                           int* link_hearts, int* link_max_hearts, int* link_rupees,
                           int* link_invuln_timer, float* link_knock_x, float* link_knock_y);

/*
 * Verifica se o golpe de espada do Link atingiu algum inimigo ou projétil ativo.
 * Retorna true se um golpe conectou (tocando som de impacto e causando knockback no monstro).
 */
bool entity_check_sword_hit(float slash_x, float slash_y, float slash_w, float slash_h,
                            int damage, Direction slash_dir);

/*
 * Verifica se o Ataque Giratório (Spin Attack 360°) atingiu inimigos ou projéteis no raio circular.
 * Aplica dano multiplicado (2 HP) e knockback radial vetorial para fora do círculo de giro.
 * Retorna a quantidade de alvos atingidos.
 */
int entity_check_spin_attack_hit(float center_x, float center_y, float radius, int damage);

/*
 * Verifica se a explosão de uma bomba atingiu inimigos ou projéteis no raio de detonação.
 * Aplica dano massivo (4 HP) e forte knockback radial para longe do epicentro da explosão.
 * Retorna o número de monstros atingidos.
 */
int entity_check_bomb_explosion(float center_x, float center_y, float radius, int damage);

/*
 * Procura um NPC amigável próximo às coordenadas fornecidas dentro do raio max_dist.
 * Retorna o ponteiro para a entidade do NPC ou NULL se nenhum estiver por perto.
 */
Entity* entity_find_nearby_npc(float world_x, float world_y, float max_dist);

/*
 * Procura pelo Mestre Espadachim Swiftblade próximo às coordenadas fornecidas.
 */
Entity* entity_find_nearby_swiftblade(float world_x, float world_y, float max_dist);

/*
 * Procura pelo Comerciante Stockwell próximo às coordenadas fornecidas dentro do raio max_dist.
 */
Entity* entity_find_nearby_shopkeeper(float world_x, float world_y, float max_dist);

/*
 * Procura pela Cidadã da Cidade de Hyrule próxima ao herói.
 */
Entity* entity_find_nearby_town_citizen(float world_x, float world_y, float max_dist);

/*
 * Procura pelo Guarda Real do Castelo de Hyrule próximo ao herói.
 */
Entity* entity_find_nearby_town_guard(float world_x, float world_y, float max_dist);

/*
 * Procura por um Toco de Árvore ou Portal Minish próximo às coordenadas fornecidas.
 */
Entity* entity_find_nearby_minish_stump(float world_x, float world_y, float max_dist);

/*
 * Procura pelo Ancião Gentari (Elder Gentari) na Vila Minish.
 */
Entity* entity_find_nearby_gentari(float world_x, float world_y, float max_dist);

/*
 * Procura pelo Sacerdote Festari na ermida da Vila Minish.
 */
Entity* entity_find_nearby_festari(float world_x, float world_y, float max_dist);

/*
 * Procura por um morador Picori (Village Minish) na Vila Minish.
 */
Entity* entity_find_nearby_village_minish(float world_x, float world_y, float max_dist);

/*
 * Procura por Malon na Fazenda Lon Lon (South Hyrule Field).
 */
Entity* entity_find_nearby_malon(float world_x, float world_y, float max_dist);

/*
 * Procura pelo Business Scrub (Deku Scrub comerciante) no Monte Crenel.
 */
Entity* entity_find_nearby_business_scrub(float world_x, float world_y, float max_dist);

/*
 * Procura pelo Mestre Ferreiro Melari nas Minas de Melari.
 */
Entity* entity_find_nearby_melari(float world_x, float world_y, float max_dist);

/*
 * Procura por um minerador Mountain Minish nas Minas de Melari.
 */
Entity* entity_find_nearby_mountain_minish(float world_x, float world_y, float max_dist);

/*
 * Realiza a compra do Grip Ring com o Business Scrub por 40 Rupees.
 */
bool entity_buy_grip_ring(int* link_rupees, bool* out_has_grip_ring);

/*
 * Executa a compra de uma mercadoria na Loja do Stockwell:
 * item_idx: 0 = Poção Vermelha (30 Rupees, restaura vida total)
 *           1 = Pedaço de Coração (80 Rupees, concede +1 Coração Máximo)
 *           2 = Bolsa de Bombas / Provisões (50 Rupees)
 * Retorna true se a compra teve êxito (Rupees deduzidos e efeito concedido).
 */
bool entity_buy_shop_item(int item_idx, int* link_rupees, int* link_hearts, int* link_max_hearts);

/*
 * Procura um NPC amigável com fusão de Kinstone pendente próximo ao herói.
 */
Entity* entity_find_kinstone_npc(float world_x, float world_y, float max_dist);

/*
 * Tenta interagir e abrir um baú de tesouro dourado destravado por fusão de Kinstone.
 * Retorna true se abriu um baú com sucesso (concedendo recompensa).
 */
bool entity_interact_chest(float world_x, float world_y, int* link_rupees, int* link_hearts);

/*
 * Verifica se um projétil disparado pelo jogador (Bumerangue ou rajada de ar) atingiu uma entidade.
 * Retorna true se conectou. Se atingir um item coletável (Rupee/Coração), captura-o no parâmetro out_carried_item.
 */
bool entity_check_subweapon_hit(float px, float py, float pw, float ph, int damage, int* out_carried_item);

/*
 * Disparo mágico do Cajado de Pacci interagindo com monstros:
 * Vira o Spiny Beetle de cabeça para baixo (carapaça invertida, patinhas para cima e barriga vulnerável a 1 golpe).
 * Atordoa Keese, Octoroks, ChuChus e Moblins.
 * Retorna true se conectou e afetou uma entidade.
 */
bool entity_check_pacci_hit(float px, float py, float pw, float ph);

/*
 * Aplica força de sucção gravitacional do Pote Mágico nas entidades próximas dentro do cone.
 * Desenterra ChuChus camuflados, puxa monstros, atrai itens e absorve projéteis de pedras.
 * Retorna true se um projétil ou inimigo foi engolido/absorvido pelo jarro (carregando o tiro de ar).
 */
bool entity_apply_gust_suction(float jar_x, float jar_y, Direction dir, float range, float pull_force);

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
