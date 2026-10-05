#ifndef HAL_KINSTONE_H
#define HAL_KINSTONE_H

/*
 * ============================================================================
 * include/hal/kinstone.h - Sistema de Fusão de Kinstones (Pedras da Sorte)
 * ============================================================================
 * Reproduz o icônico sistema de Kinstone Fusion de The Legend of Zelda: The Minish Cap.
 *
 * Características:
 * - Bolsa de Kinstones do herói (Fragmentos Verde, Azul e Vermelho)
 * - Detecção contextual de NPCs com fragmentos complementares
 * - Tela de Fusão (Kinstone Fusion Interface):
 *   - Visualização das metades complementares (NPC à direita, Link à esquerda)
 *   - Navegação e seleção de fragmentos da bolsa
 *   - Cinemática de encaixe: deslocamento suave, flash mágico,
 *     fanfarra autêntica de arpejo (SOUND_KINSTONE_FUSION) e rotação de medalhão
 *   - Mensagem de evento mundial desencadeado no mapa
 * - Disparo de eventos mundiais: spawn de baú dourado misterioso com recompensas
 */

#include "gba/types.h"
#include "hal/entity.h"
#include <stdbool.h>

typedef enum {
    KINSTONE_GREEN = 0, // Fragmento Verde Comum (Borda recortada)
    KINSTONE_BLUE,      // Fragmento Azul Incomum (Canto em L)
    KINSTONE_RED,       // Fragmento Vermelho Raro (Três encaixes)
    KINSTONE_COUNT
} KinstoneType;

typedef enum {
    KINSTONE_UI_INACTIVE = 0,
    KINSTONE_UI_SELECTING,   // Jogador escolhe o fragmento da bolsa
    KINSTONE_UI_FUSING,      // Animação das metades se unindo com flash mágico
    KINSTONE_UI_CELEBRATE,   // Medalhão completo brilhando e girando
    KINSTONE_UI_EVENT_POPUP  // Notificação do evento destravado no mapa
} KinstoneUIState;

typedef struct {
    int counts[KINSTONE_COUNT];
    int total_fusions;
} KinstoneInventory;

/*
 * Inicializa a bolsa de Kinstones com fragmentos iniciais.
 */
void kinstone_init(void);

/*
 * Retorna o inventário de Kinstones do Link.
 */
KinstoneInventory* kinstone_get_inventory(void);

/*
 * Adiciona fragmentos à bolsa de Kinstones.
 */
void kinstone_add(KinstoneType type, int count);

/*
 * Retorna true se a interface de fusão de Kinstone estiver aberta na tela.
 */
bool kinstone_is_active(void);

/*
 * Inicia o ritual de fusão com um NPC parceiro.
 */
void kinstone_start_fusion(Entity* npc);

/*
 * Atualiza o cronômetro, partículas mágicas e animações da tela de fusão.
 */
void kinstone_update(void);

/*
 * Processa comandos de confirmação, cancelamento e navegação do inventário.
 * nav_dir: -1 para esquerda, +1 para direita, 0 para nenhum.
 */
void kinstone_handle_input(bool confirm_pressed, bool cancel_pressed, int nav_dir);

/*
 * Renderiza a interface de tela cheia de Fusão de Kinstone.
 */
void kinstone_render(void);

/*
 * Retorna o nome amigável do tipo de Kinstone.
 */
const char* kinstone_get_type_name(KinstoneType type);

#endif // HAL_KINSTONE_H
