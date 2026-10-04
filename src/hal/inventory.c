#include "hal/inventory.h"
#include "hal/video.h"
#include "hal/audio.h"
#include "hal/font.h"
#include "hal/subweapon.h"
#include <stdio.h>
#include <string.h>

#define GRID_COLS 4
#define GRID_ROWS 3
#define GRID_TOTAL (GRID_COLS * GRID_ROWS)

static bool s_is_paused = false;
static int s_cursor_x = 0;
static int s_cursor_y = 0;
static int s_cursor_pulse = 0;

static bool s_items_unlocked[INV_ITEM_COUNT] = { false };
static InventoryItem s_slot_a = INV_ITEM_SWORD;
static InventoryItem s_slot_b = INV_ITEM_BOOMERANG;

void inventory_init(void) {
    s_is_paused = false;
    s_cursor_x = 0;
    s_cursor_y = 0;
    s_cursor_pulse = 0;

    memset(s_items_unlocked, 0, sizeof(s_items_unlocked));

    // Itens padrão desbloqueados na progressão inicial
    s_items_unlocked[INV_ITEM_SWORD]         = true;
    s_items_unlocked[INV_ITEM_BOOMERANG]     = true;
    s_items_unlocked[INV_ITEM_GUST_JAR]      = true;
    s_items_unlocked[INV_ITEM_BOMBS]         = true;
    s_items_unlocked[INV_ITEM_PEGASUS_BOOTS] = true;
    s_items_unlocked[INV_ITEM_FLIPPERS]      = true;
    s_items_unlocked[INV_ITEM_CANE_OF_PACCI] = true;

    s_slot_a = INV_ITEM_SWORD;
    s_slot_b = INV_ITEM_BOOMERANG;
}

bool inventory_is_paused(void) {
    return s_is_paused;
}

void inventory_toggle_pause(void) {
    s_is_paused = !s_is_paused;
    if (s_is_paused) {
        hal_audio_play_sound(SOUND_SECRET, 0.65f, 1.4f);
        printf("[PAUSE] Menu de Inventario ABERTO\n");
    } else {
        hal_audio_play_sound(SOUND_TEXT_ADVANCE, 0.8f, 1.2f);
        printf("[PAUSE] Menu de Inventario FECHADO\n");
    }
}

void inventory_set_paused(bool paused) {
    s_is_paused = paused;
}

bool inventory_has_item(InventoryItem item) {
    if (item <= INV_ITEM_NONE || item >= INV_ITEM_COUNT) return false;
    return s_items_unlocked[item];
}

void inventory_unlock_item(InventoryItem item) {
    if (item > INV_ITEM_NONE && item < INV_ITEM_COUNT) {
        s_items_unlocked[item] = true;
        printf("[INVENTORY] Item desbloqueado: %s\n", inventory_get_item_name(item));
    }
}

InventoryItem inventory_get_slot_a(void) {
    return s_slot_a;
}

InventoryItem inventory_get_slot_b(void) {
    return s_slot_b;
}

void inventory_set_slot_a(InventoryItem item) {
    s_slot_a = item;
}

void inventory_set_slot_b(InventoryItem item) {
    s_slot_b = item;
}

const char* inventory_get_item_name(InventoryItem item) {
    switch (item) {
        case INV_ITEM_SWORD:         return "Espada do Smith";
        case INV_ITEM_BOOMERANG:     return "Bumerangue Magico";
        case INV_ITEM_GUST_JAR:      return "Pote Magico (Gust Jar)";
        case INV_ITEM_BOMBS:         return "Bolsa de Bombas";
        case INV_ITEM_PEGASUS_BOOTS: return "Botas de Pegasus";
        case INV_ITEM_FLIPPERS:      return "Nadadeiras de Zora";
        case INV_ITEM_CANE_OF_PACCI: return "Cajado de Pacci";
        case INV_ITEM_MOLE_MITTS:    return "Luvas de Toupeira";
        case INV_ITEM_LANTERN:       return "Lanterna de Fogo";
        case INV_ITEM_ROCS_CAPE:     return "Capa de Roc";
        case INV_ITEM_BOW:           return "Arco e Flechas";
        case INV_ITEM_GRIP_RING:     return "Anel de Escalada (Grip Ring)";
        default:                     return "---";
    }
}

const char* inventory_get_item_desc(InventoryItem item) {
    switch (item) {
        case INV_ITEM_SWORD:
            return "Lâmina forjada por Mestre Smith. Desfere cortes e carrega Spin Attack.";
        case INV_ITEM_BOOMERANG:
            return "Arma rotativa voadora. Atordoa monstros e resgata itens distantes.";
        case INV_ITEM_GUST_JAR:
            return "Relíquia mágica Minish. Suga poeira e teias e dispara rajadas de ar.";
        case INV_ITEM_BOMBS:
            return "Explosivos potentes. Destroem paredes fissuradas e rochas frágeis.";
        case INV_ITEM_PEGASUS_BOOTS:
            return "Botas sagradas velozes. Permitem investidas e atravessar pântanos.";
        case INV_ITEM_FLIPPERS:
            return "Presente do povo Zora. Permitem nadar em águas profundas e mergulhar.";
        case INV_ITEM_CANE_OF_PACCI:
            return "Cajado místico. Energiza buracos para saltos e vira inimigos.";
        case INV_ITEM_MOLE_MITTS:
            return "Garras afiadas. Escavam terra macia revelando segredos soterrados.";
        case INV_ITEM_LANTERN:
            return "Chama eterna. Ilumina masmorras escuras e derrete gelo e teias.";
        case INV_ITEM_ROCS_CAPE:
            return "Manto celestial. Permite saltar no ar e planar sobre abismos.";
        case INV_ITEM_BOW:
            return "Arco de precisão. Dispara flechas e aciona interruptores distantes.";
        case INV_ITEM_GRIP_RING:
            return "Anel com garras de escalada. Permite escalar escarpas e paredões de rocha.";
        default:
            return "Slot de inventário vazio.";
    }
}

static InventoryItem get_item_at_grid(int gx, int gy) {
    int idx = gy * GRID_COLS + gx + 1;
    if (idx >= 1 && idx < INV_ITEM_COUNT) {
        return (InventoryItem)idx;
    }
    return INV_ITEM_NONE;
}

void inventory_cursor_move(int dx, int dy) {
    s_cursor_x = (s_cursor_x + dx + GRID_COLS) % GRID_COLS;
    s_cursor_y = (s_cursor_y + dy + GRID_ROWS) % GRID_ROWS;
    hal_audio_play_sound(SOUND_TEXT_BLIP, 0.6f, 1.5f);
}

void inventory_assign_to_slot_a(void) {
    InventoryItem sel = get_item_at_grid(s_cursor_x, s_cursor_y);
    if (sel == INV_ITEM_NONE || !s_items_unlocked[sel]) {
        hal_audio_play_sound(SOUND_SWORD_HIT, 0.5f, 0.7f);
        return;
    }

    // Se já estava no slot B, inverte
    if (s_slot_b == sel) {
        s_slot_b = s_slot_a;
    }
    s_slot_a = sel;

    hal_audio_play_sound(SOUND_ITEM_CATCH, 0.9f, 1.2f);
    printf("[INVENTORY] Atribuido ao Botao [A]: %s\n", inventory_get_item_name(sel));
}

void inventory_assign_to_slot_b(void) {
    InventoryItem sel = get_item_at_grid(s_cursor_x, s_cursor_y);
    if (sel == INV_ITEM_NONE || !s_items_unlocked[sel]) {
        hal_audio_play_sound(SOUND_SWORD_HIT, 0.5f, 0.7f);
        return;
    }

    // Se já estava no slot A, inverte
    if (s_slot_a == sel) {
        s_slot_a = s_slot_b;
    }
    s_slot_b = sel;

    // Sincroniza subarma correspondente se for subweapon
    if (s_slot_b == INV_ITEM_BOOMERANG)          subweapon_set_current(ITEM_BOOMERANG);
    else if (s_slot_b == INV_ITEM_GUST_JAR)      subweapon_set_current(ITEM_GUST_JAR);
    else if (s_slot_b == INV_ITEM_PEGASUS_BOOTS)  subweapon_set_current(ITEM_PEGASUS_BOOTS);
    else if (s_slot_b == INV_ITEM_BOMBS)          subweapon_set_current(ITEM_BOMBS);
    else if (s_slot_b == INV_ITEM_CANE_OF_PACCI)  subweapon_set_current(ITEM_CANE_OF_PACCI);

    hal_audio_play_sound(SOUND_ITEM_CATCH, 0.9f, 1.2f);
    printf("[INVENTORY] Atribuido ao Botao [B]: %s\n", inventory_get_item_name(sel));
}

// Renderizador dos ícones em miniatura de cada item (14x14)
static void draw_item_icon(int x, int y, InventoryItem item) {
    switch (item) {
        case INV_ITEM_SWORD:
            // Espada brilhante: lâmina de aço, guarda dourada e empunhadura azul
            for (int i = 0; i < 9; i++) {
                hal_video_put_pixel(x + 2 + i, y + 11 - i, 0xE2E8F0FF); // Lâmina
                hal_video_put_pixel(x + 3 + i, y + 11 - i, 0xFFFFFFFF);
            }
            hal_video_put_pixel(x + 3, y + 10, 0xF59E0BFF); // Guarda
            hal_video_put_pixel(x + 4, y + 11, 0xF59E0BFF);
            hal_video_put_pixel(x + 2, y + 12, 0x2563EBFF); // Cabo
            hal_video_put_pixel(x + 1, y + 13, 0xF59E0BFF); // Pomo
            break;

        case INV_ITEM_BOOMERANG:
            // Bumerangue dourado em V com gema vermelha
            hal_video_put_pixel(x + 4, y + 3, 0xFBBF24FF);
            hal_video_put_pixel(x + 5, y + 4, 0xFBBF24FF);
            hal_video_put_pixel(x + 6, y + 5, 0xFBBF24FF);
            hal_video_put_pixel(x + 7, y + 6, 0xDC2626FF); // Gema
            hal_video_put_pixel(x + 6, y + 7, 0xFBBF24FF);
            hal_video_put_pixel(x + 5, y + 8, 0xFBBF24FF);
            hal_video_put_pixel(x + 4, y + 9, 0xFBBF24FF);
            break;

        case INV_ITEM_GUST_JAR:
            // Pote cerâmico bojudo com bocal dourado
            for (int dy = 5; dy <= 10; dy++) {
                for (int dx = 4; dx <= 10; dx++) {
                    hal_video_put_pixel(x + dx, y + dy, 0xB45309FF);
                }
            }
            hal_video_put_pixel(x + 6, y + 3, 0xF59E0BFF);
            hal_video_put_pixel(x + 7, y + 3, 0xF59E0BFF);
            hal_video_put_pixel(x + 8, y + 3, 0xF59E0BFF);
            hal_video_put_pixel(x + 7, y + 4, 0xD97706FF);
            break;

        case INV_ITEM_BOMBS:
            // Bomba esférica preta com pavio faiscante
            for (int dy = 5; dy <= 11; dy++) {
                for (int dx = 4; dx <= 10; dx++) {
                    if ((dx == 4 || dx == 10) && (dy == 5 || dy == 11)) continue;
                    hal_video_put_pixel(x + dx, y + dy, 0x1E293BFF);
                }
            }
            hal_video_put_pixel(x + 6, y + 7, 0x94A3B8FF); // Brilho
            hal_video_put_pixel(x + 7, y + 4, 0xD97706FF); // Gargalo
            hal_video_put_pixel(x + 8, y + 3, 0xF59E0BFF); // Pavio
            hal_video_put_pixel(x + 9, y + 2, 0xFDE047FF); // Faísca
            break;

        case INV_ITEM_PEGASUS_BOOTS:
            // Bota vermelha estilosa com asas brancas
            for (int dy = 6; dy <= 11; dy++) {
                for (int dx = 5; dx <= 10; dx++) {
                    hal_video_put_pixel(x + dx, y + dy, 0xDC2626FF);
                }
            }
            hal_video_put_pixel(x + 4, y + 10, 0x7F1D1DFF); // Sola
            hal_video_put_pixel(x + 4, y + 11, 0x7F1D1DFF);
            hal_video_put_pixel(x + 8, y + 5, 0xFFFFFFFF);  // Asa
            hal_video_put_pixel(x + 9, y + 4, 0xFFFFFFFF);
            hal_video_put_pixel(x + 10, y + 5, 0xFFFFFFFF);
            break;

        case INV_ITEM_FLIPPERS:
            // Par de nadadeiras verdes de Zora
            for (int dy = 4; dy <= 11; dy++) {
                hal_video_put_pixel(x + 5, y + dy, 0x059669FF);
                hal_video_put_pixel(x + 6, y + dy, 0x10B981FF);
                hal_video_put_pixel(x + 8, y + dy, 0x059669FF);
                hal_video_put_pixel(x + 9, y + dy, 0x10B981FF);
            }
            hal_video_put_pixel(x + 4, y + 11, 0x34D399FF);
            hal_video_put_pixel(x + 10, y + 11, 0x34D399FF);
            break;

        case INV_ITEM_CANE_OF_PACCI:
            // Cajado mágico com orbe azul celeste cintilante
            for (int i = 0; i < 7; i++) {
                hal_video_put_pixel(x + 4 + i, y + 11 - i, 0x78350FFF); // Haste
            }
            hal_video_put_pixel(x + 10, y + 4, 0x38BDF8FF); // Orbe
            hal_video_put_pixel(x + 11, y + 4, 0x38BDF8FF);
            hal_video_put_pixel(x + 10, y + 3, 0x0284C7FF);
            hal_video_put_pixel(x + 11, y + 3, 0xFFFFFFFF);
            break;

        case INV_ITEM_MOLE_MITTS:
            // Luva de couro marrom com garras afiadas
            for (int dy = 5; dy <= 10; dy++) {
                for (int dx = 5; dx <= 9; dx++) {
                    hal_video_put_pixel(x + dx, y + dy, 0x78350FFF);
                }
            }
            hal_video_put_pixel(x + 5, y + 4, 0xE2E8F0FF); // Garras
            hal_video_put_pixel(x + 7, y + 4, 0xE2E8F0FF);
            hal_video_put_pixel(x + 9, y + 4, 0xE2E8F0FF);
            break;

        case INV_ITEM_LANTERN:
            // Lanterna de latão com chama viva no centro
            for (int dy = 4; dy <= 11; dy++) {
                for (int dx = 5; dx <= 9; dx++) {
                    if (dy == 4 || dy == 11 || dx == 5 || dx == 9)
                        hal_video_put_pixel(x + dx, y + dy, 0xD97706FF);
                    else
                        hal_video_put_pixel(x + dx, y + dy, 0xFDE047FF); // Fogo
                }
            }
            hal_video_put_pixel(x + 7, y + 2, 0x78350FFF); // Alça
            break;

        case INV_ITEM_ROCS_CAPE:
            // Capa azul e vermelha esvoaçante
            for (int dy = 4; dy <= 10; dy++) {
                for (int dx = 4; dx <= 10; dx++) {
                    hal_video_put_pixel(x + dx, y + dy, (dx < 7) ? 0x2563EBFF : 0xDC2626FF);
                }
            }
            hal_video_put_pixel(x + 7, y + 3, 0xF59E0BFF); // Broche
            break;

        case INV_ITEM_BOW:
            // Arco de madeira recurvo e flecha com pena branca
            for (int dy = 3; dy <= 11; dy++) {
                hal_video_put_pixel(x + 4, y + dy, 0xB45309FF);
            }
            hal_video_put_pixel(x + 5, y + 3, 0xB45309FF);
            hal_video_put_pixel(x + 5, y + 11, 0xB45309FF);
            for (int dx = 4; dx <= 11; dx++) {
                hal_video_put_pixel(x + dx, y + 7, 0xF8FAFCFF); // Flecha
            }
            break;

        case INV_ITEM_GRIP_RING:
            // Anel dourado com garras afiadas e rubi central
            for (int dy = 4; dy <= 10; dy++) {
                for (int dx = 4; dx <= 10; dx++) {
                    if (dy == 4 || dy == 10 || dx == 4 || dx == 10)
                        hal_video_put_pixel(x + dx, y + dy, 0xF59E0BFF); // Aro dourado
                }
            }
            // Garras de escalada
            hal_video_put_pixel(x + 3, y + 4, 0x94A3B8FF);
            hal_video_put_pixel(x + 7, y + 2, 0x94A3B8FF);
            hal_video_put_pixel(x + 11, y + 4, 0x94A3B8FF);
            // Rubi central
            hal_video_put_pixel(x + 7, y + 7, 0xEF4444FF);
            break;

        default:
            break;
    }
}

void inventory_render_pause_menu(int screen_w, int screen_h, int hearts, int max_hearts, int rupees) {
    if (!s_is_paused) return;

    s_cursor_pulse = (s_cursor_pulse + 1) % 60;

    // 1. Escurecimento do fundo com tom translúcido de pergaminho/esmeralda
    for (int y = 0; y < screen_h; y++) {
        for (int x = 0; x < screen_w; x++) {
            u32 c = hal_video_get_pixel(x, y);
            u8 r = (c >> 24) & 0xFF;
            u8 g = (c >> 16) & 0xFF;
            u8 b = (c >> 8) & 0xFF;
            // Efeito escurecedor profundo (20% do original + tom verde musgo clássico Minish)
            r = (u8)(r * 0.18f + 5);
            g = (u8)(g * 0.18f + 18);
            b = (u8)(b * 0.18f + 10);
            hal_video_put_pixel(x, y, (r << 24) | (g << 16) | (b << 8) | 0xFF);
        }
    }

    // 2. Moldura central elegante do Menu de Pausa (GBA Minish Cap style)
    int box_x = 10, box_y = 6, box_w = 220, box_h = 148;
    for (int y = box_y; y < box_y + box_h; y++) {
        for (int x = box_x; x < box_x + box_w; x++) {
            if (y == box_y || y == box_y + box_h - 1 || x == box_x || x == box_x + box_w - 1) {
                hal_video_put_pixel(x, y, 0xD4AF37FF); // Borda externa de ouro
            } else if (y == box_y + 1 || y == box_y + box_h - 2 || x == box_x + 1 || x == box_x + box_w - 2) {
                hal_video_put_pixel(x, y, 0x1E3A24FF); // Borda interna verde musgo
            } else {
                hal_video_put_pixel(x, y, 0x0A180FFF); // Fundo escuro elegante
            }
        }
    }

    // 3. Cabeçalho: "ITENS // PAUSA" e Status de Vida/Rupees
    font_draw_text(box_x + 8, box_y + 6, "ITENS", 0xFDE047FF, true);

    // Corações
    for (int h = 0; h < max_hearts; h++) {
        int hx = box_x + 80 + h * 9;
        int hy = box_y + 6;
        u32 c_heart = (h < hearts) ? 0xEF4444FF : 0x475569FF;
        hal_video_put_pixel(hx + 1, hy, c_heart);
        hal_video_put_pixel(hx + 3, hy, c_heart);
        for (int dx = 0; dx < 5; dx++) hal_video_put_pixel(hx + dx, hy + 1, c_heart);
        for (int dx = 1; dx < 4; dx++) hal_video_put_pixel(hx + dx, hy + 2, c_heart);
        hal_video_put_pixel(hx + 2, hy + 3, c_heart);
    }

    // Rupees
    char rup_str[16];
    snprintf(rup_str, sizeof(rup_str), "%d R", rupees);
    font_draw_text(box_x + 175, box_y + 6, rup_str, 0x34D399FF, false);

    // Divisória dourada superior
    for (int x = box_x + 6; x < box_x + box_w - 6; x++) {
        hal_video_put_pixel(x, box_y + 16, 0xD4AF37FF);
    }

    // 4. Grade de Itens (4 Colunas x 3 Linhas)
    int start_gx = box_x + 24;
    int start_gy = box_y + 24;
    int slot_w = 40;
    int slot_h = 24;

    for (int gy = 0; gy < GRID_ROWS; gy++) {
        for (int gx = 0; gx < GRID_COLS; gx++) {
            int sx = start_gx + gx * (slot_w + 4);
            int sy = start_gy + gy * (slot_h + 3);
            InventoryItem it = get_item_at_grid(gx, gy);
            bool unlocked = (it != INV_ITEM_NONE && s_items_unlocked[it]);

            // Caixa do Slot
            for (int y = 0; y < slot_h; y++) {
                for (int x = 0; x < slot_w; x++) {
                    u32 c_slot = 0x14281BFF;
                    if (y == 0 || y == slot_h - 1 || x == 0 || x == slot_w - 1) {
                        c_slot = unlocked ? 0x2E5A3DFF : 0x1E293BFF;
                    }
                    hal_video_put_pixel(sx + x, sy + y, c_slot);
                }
            }

            if (unlocked) {
                // Desenha o ícone do item
                draw_item_icon(sx + 13, sy + 5, it);

                // Badge [A] se equipado no botão A
                if (s_slot_a == it) {
                    for (int dy = 1; dy < 8; dy++) {
                        for (int dx = 1; dx < 8; dx++) hal_video_put_pixel(sx + dx, sy + dy, 0x2563EBFF);
                    }
                    font_draw_char(sx + 2, sy + 1, 'A', 0xFFFFFFFF, false);
                }

                // Badge [B] se equipado no botão B
                if (s_slot_b == it) {
                    for (int dy = 1; dy < 8; dy++) {
                        for (int dx = slot_w - 9; dx < slot_w - 2; dx++) hal_video_put_pixel(sx + dx, sy + dy, 0xD97706FF);
                    }
                    font_draw_char(sx + slot_w - 8, sy + 1, 'B', 0xFFFFFFFF, false);
                }
            } else {
                // Ponto sombreado indicando slot bloqueado
                hal_video_put_pixel(sx + slot_w / 2, sy + slot_h / 2, 0x334155FF);
            }

            // 5. Cursor Dourado Pulsante no item selecionado
            if (gx == s_cursor_x && gy == s_cursor_y) {
                u32 c_cur = (s_cursor_pulse < 30) ? 0xFDE047FF : 0xFFFFFFFF;
                for (int y = -2; y <= slot_h + 1; y++) {
                    for (int x = -2; x <= slot_w + 1; x++) {
                        if (y == -2 || y == slot_h + 1 || x == -2 || x == slot_w + 1 ||
                            y == -1 || y == slot_h     || x == -1 || x == slot_w) {
                            hal_video_put_pixel(sx + x, sy + y, c_cur);
                        }
                    }
                }
            }
        }
    }

    // 6. Painel Inferior: Nome e Descrição do Item Selecionado
    int desc_y = box_y + 104;
    for (int x = box_x + 6; x < box_x + box_w - 6; x++) {
        hal_video_put_pixel(x, desc_y - 2, 0xD4AF37FF);
    }

    InventoryItem cur_item = get_item_at_grid(s_cursor_x, s_cursor_y);
    bool cur_unlocked = (cur_item != INV_ITEM_NONE && s_items_unlocked[cur_item]);

    if (cur_unlocked) {
        font_draw_text(box_x + 10, desc_y + 2, inventory_get_item_name(cur_item), 0xFDE047FF, true);
        font_draw_text(box_x + 10, desc_y + 13, inventory_get_item_desc(cur_item), 0xE2E8F0FF, false);
    } else {
        font_draw_text(box_x + 10, desc_y + 2, "Slot Bloqueado", 0x64748BFF, false);
        font_draw_text(box_x + 10, desc_y + 13, "Item ainda nao descoberto na aventura.", 0x94A3B8FF, false);
    }

    // 7. Legenda de Controles no Rodapé
    font_draw_text(box_x + 8, box_y + box_h - 12, "[Z] A   [X] B   [S] Salvar   [ENTER] Voltar", 0x38BDF8FF, false);
}
