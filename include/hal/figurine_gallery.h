#ifndef HAL_FIGURINE_GALLERY_H
#define HAL_FIGURINE_GALLERY_H

/*
 * ============================================================================
 * include/hal/figurine_gallery.h - Galeria do Carlov & Sistema Gacha (Ato VI)
 * ============================================================================
 * Modela a lendária mecânica de colecionáveis de The Legend of Zelda: The Minish Cap:
 *
 * Características:
 * - A máquina de Gacha do excêntrico escultor Carlov (localizada em South Hyrule Field).
 * - Sistema de aposta com Conchas Misteriosas (Mysterious Shells):
 *   - Quanto mais conchas apostadas, maior a chance percentual de obter uma nova estatueta (até 100%).
 * - 136 Miniaturas Colecionáveis canônicas:
 *   - Heróis, Moradores de Hyrule, Tribo Minish, Monstros, Subchefes e Chefes Épicos.
 *   - Modelagem de troféus 3D rotativos em tempo real com iluminação de pedestal.
 *   - Descrições canônicas ricas em detalhes de lore.
 * - Modo Inspeção de Estatuetas (Gallery Viewer):
 *   - Navegação pela coleção completa com rotação analógica e zoom.
 * - Recompensa Suprema: Medalha de Carlov (Carlov Medal) concedida ao completar a coleção,
 *   destravando a Casa dos Herbologistas e um Pedaço de Coração permanente!
 */

#include "gba/types.h"
#include <stdbool.h>

#define TOTAL_FIGURINES 136
#define MAX_SHELLS 999

typedef enum {
    FIG_CAT_HERO = 0,
    FIG_CAT_TOWN,
    FIG_CAT_MINISH,
    FIG_CAT_ENEMY,
    FIG_CAT_BOSS
} FigurineCategory;

typedef struct {
    int              id;
    const char*      name;
    const char*      desc_line1;
    const char*      desc_line2;
    FigurineCategory category;
    u32              primary_color;
    u32              accent_color;
    bool             unlocked;
} FigurineEntry;

typedef enum {
    GALLERY_MODE_GACHA = 0,
    GALLERY_MODE_DISPENSING,
    GALLERY_MODE_REVEAL,
    GALLERY_MODE_INSPECTOR,
    GALLERY_MODE_MEDAL_AWARD
} GalleryMode;

typedef struct {
    bool          is_active;
    GalleryMode   mode;
    int           shells_owned;
    int           shells_bet;
    float         win_chance_percent;
    int           unlocked_count;
    int           dispense_timer;
    float         crank_angle;
    int           current_won_id;
    bool          is_duplicate;
    int           selected_id;
    float         trophy_rot_angle;
    bool          has_carlov_medal;
    int           medal_banner_timer;
    FigurineEntry figurines[TOTAL_FIGURINES];
} FigurineGallery;

// Inicialização e Ciclo de Vida
void figurine_gallery_init(void);
void figurine_gallery_open(int shells);
void figurine_gallery_close(void);
bool figurine_gallery_is_active(void);

// Lógica e Entrada
void figurine_gallery_handle_input(bool btn_a, bool btn_b, bool dpad_up, bool dpad_down,
                                   bool dpad_left, bool dpad_right, bool start);
void figurine_gallery_update(void);
void figurine_gallery_render(void);

// Acesso a Dados e Recompensas
int  figurine_gallery_get_shells(void);
int  figurine_gallery_get_unlocked_count(void);
bool figurine_gallery_has_medal(void);
void figurine_gallery_deposit_shells(int count);

// Persistência
void figurine_gallery_export(u8* bitmask_17bytes, int* count, bool* medal, int* shells);
void figurine_gallery_restore(const u8* bitmask_17bytes, int count, bool medal, int shells);

#endif // HAL_FIGURINE_GALLERY_H
