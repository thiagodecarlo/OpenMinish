#ifndef HAL_SWORD_DOJO_H
#define HAL_SWORD_DOJO_H

/*
 * ============================================================================
 * include/hal/sword_dojo.h - Dojos de Esgrima & 8 Pergaminhos do Tigre (Ato VI)
 * ============================================================================
 * Modela os lendários Mestres Espadachins (Blade Brothers) de Hyrule e seus
 * 8 Pergaminhos do Tigre (Tiger Scrolls) canônicos:
 *
 * 1. Spin Attack (Ataque Giratório) - Mestre Swiftblade (Hyrule Town)
 *    - Carrega energia segurando a espada e libera corte devastador de 360°.
 * 2. Roll Attack (Estocada Rolando) - Mestre Grayblade (Monte Crenel)
 *    - Pressionar ataque ao rolar executa estocada instantânea com alcance duplo.
 * 3. Dash Attack (Investida com Espada) - Mestre Swiftblade (Hyrule Town)
 *    - Corrida veloz com Botas de Pégaso mantendo a lâmina empunhada à frente.
 * 4. Rock Breaker (Quebra-Pedras) - Mestre Swiftblade (Hyrule Town)
 *    - Corta e esmigalha pedras e potes diretamente com a lâmina sem gastar bombas.
 * 5. Sword Beam (Raio de Espada Sagrado) - Mestre Grimblade (Castelo de Hyrule)
 *    - Com vida cheia, golpes de espada disparam lâminas de energia sagrada.
 * 6. Down Thrust (Estocada Aérea) - Mestre Waveblade (Lago Hylia)
 *    - No ar, ataca descendo verticalmente, quebrando escudos e abrindo armaduras.
 * 7. Peril Beam (Raio de Perigo/Desespero) - Mestre Splitblade (Veil Falls)
 *    - Com 1 coração ou menos, golpes liberam raios carmesins desesperados.
 * 8. Great Spin Attack (Grande Furacão) - Mestre Swiftblade the First (Castor Wilds)
 *    - Apertar repetidamente o botão durante o giro estende o Spin Attack em um
 *      verdadeiro tufão móvel de 5 revoluções com sucção de vórtice!
 */

#include "gba/types.h"
#include <stdbool.h>

#define TOTAL_TIGER_SCROLLS 8

typedef enum {
    SCROLL_SPIN_ATTACK = 0,
    SCROLL_ROLL_ATTACK,
    SCROLL_DASH_ATTACK,
    SCROLL_ROCK_BREAKER,
    SCROLL_SWORD_BEAM,
    SCROLL_DOWN_THRUST,
    SCROLL_PERIL_BEAM,
    SCROLL_GREAT_SPIN
} TigerScrollId;

typedef struct {
    TigerScrollId id;
    const char*   name;
    const char*   master_name;
    const char*   location;
    const char*   desc_line1;
    const char*   desc_line2;
    u32           ribbon_color;
} TigerScrollInfo;

// Inicialização e Consulta
void sword_dojo_init(void);
void sword_dojo_unlock_scroll(TigerScrollId scroll);
bool sword_dojo_has_scroll(TigerScrollId scroll);
u8   sword_dojo_get_unlocked_mask(void);
int  sword_dojo_get_unlocked_count(void);
const TigerScrollInfo* sword_dojo_get_info(TigerScrollId scroll);

// Mecânica de Combate dos Pergaminhos
bool sword_dojo_can_fire_beam(int hearts, int max_hearts, bool* out_is_peril);
bool sword_dojo_can_break_rocks(void);
bool sword_dojo_can_roll_attack(void);
bool sword_dojo_can_dash_attack(void);
bool sword_dojo_can_down_thrust(void);
bool sword_dojo_can_great_spin(void);

// Banners e Menus Festivos
void sword_dojo_trigger_banner(TigerScrollId scroll);
void sword_dojo_update(void);
void sword_dojo_render_banner(void);

// Persistência
void sword_dojo_export(u8* out_mask);
void sword_dojo_restore(u8 mask);

#endif // HAL_SWORD_DOJO_H
