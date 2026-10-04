#ifndef HAL_INVENTORY_H
#define HAL_INVENTORY_H

/*
 * ============================================================================
 * include/hal/inventory.h - Menu de Pausa e Sistema de Inventário [START]
 * ============================================================================
 * Modela a tela de inventário canônica de The Legend of Zelda: The Minish Cap.
 * Permite ao jogador pausar o jogo a qualquer momento, visualizar os itens
 * coletados, seus detalhes e atribuir livremente itens aos botões [A] e [B].
 */

#include "gba/types.h"
#include <stdbool.h>

typedef enum {
    INV_ITEM_NONE = 0,
    INV_ITEM_SWORD,          // Espada do Smith / White Sword
    INV_ITEM_BOOMERANG,      // Bumerangue Mágico
    INV_ITEM_GUST_JAR,       // Pote Mágico (Gust Jar)
    INV_ITEM_BOMBS,          // Bolsa de Bombas
    INV_ITEM_PEGASUS_BOOTS,  // Botas de Pegasus (Dash veloz)
    INV_ITEM_FLIPPERS,       // Nadadeiras de Zora (Natação/Mergulho)
    INV_ITEM_CANE_OF_PACCI,  // Cajado de Pacci (Buracos/Virar)
    INV_ITEM_MOLE_MITTS,     // Luvas de Toupeira (Escavação)
    INV_ITEM_LANTERN,        // Lanterna de Fogo (Iluminação/Gelo)
    INV_ITEM_ROCS_CAPE,      // Capa de Roc (Pulo livre/Planar)
    INV_ITEM_BOW,            // Arco e Flechas
    INV_ITEM_GRIP_RING,      // Anel de Escalada (Grip Ring - Escala escarpas e paredões)
    INV_ITEM_COUNT
} InventoryItem;

// Inicialização do estado de inventário
void inventory_init(void);

// Controle do estado de pausa
bool inventory_is_paused(void);
void inventory_toggle_pause(void);
void inventory_set_paused(bool paused);

// Gerenciamento de itens desbloqueados
bool inventory_has_item(InventoryItem item);
void inventory_unlock_item(InventoryItem item);

// Itens equipados nos botões de ação [A] e [B]
InventoryItem inventory_get_slot_a(void);
InventoryItem inventory_get_slot_b(void);
void inventory_set_slot_a(InventoryItem item);
void inventory_set_slot_b(InventoryItem item);

// Metadados do item
const char* inventory_get_item_name(InventoryItem item);
const char* inventory_get_item_desc(InventoryItem item);

// Navegação e comandos na tela de pausa
void inventory_cursor_move(int dx, int dy);
void inventory_assign_to_slot_a(void);
void inventory_assign_to_slot_b(void);

// Renderização da interface gráfica do Menu de Pausa (240x160)
void inventory_render_pause_menu(int screen_w, int screen_h, int hearts, int max_hearts, int rupees);

#endif // HAL_INVENTORY_H
