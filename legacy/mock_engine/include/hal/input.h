#ifndef HAL_INPUT_H
#define HAL_INPUT_H

/*
 * ============================================================================
 * include/hal/input.h - Camada de Abstração de Entrada (Digital + Analógico)
 * ============================================================================
 * Modela o registrador clássico REG_KEYINPUT do GBA e adiciona suporte nativo a
 * Alavancas Analógicas modernas (vetores normalizados 360° com velocidade
 * proporcional e Deadzone circular).
 */

#include "gba/types.h"
#include <SDL3/SDL.h>

// Máscara de Bits oficial dos 10 botões do Game Boy Advance
#define KEY_A        (1 << 0)  // Bit 0: Botão A (Ataque / Confirmar)
#define KEY_B        (1 << 1)  // Bit 1: Botão B (Rolar / Correr)
#define KEY_SELECT   (1 << 2)  // Bit 2: Botão Select
#define KEY_START    (1 << 3)  // Bit 3: Botão Start (Pausar / Menu)
#define KEY_RIGHT    (1 << 4)  // Bit 4: Direcional Direito
#define KEY_LEFT     (1 << 5)  // Bit 5: Direcional Esquerdo
#define KEY_UP       (1 << 6)  // Bit 6: Direcional Cima
#define KEY_DOWN     (1 << 7)  // Bit 7: Direcional Baixo
#define KEY_R        (1 << 8)  // Bit 8: Gatilho Superior Direito (R)
#define KEY_L        (1 << 9)  // Bit 9: Gatilho Superior Esquerdo (L)

#define KEY_MASK_ALL (0x03FF)

// Estrutura de Vetor Analógico 360° para alavancas analógicas
typedef struct {
    float x;         // Eixo horizontal normalizado (-1.0 a +1.0)
    float y;         // Eixo vertical normalizado   (-1.0 a +1.0)
    float magnitude; // Intensidade da inclinação   (0.0 = repouso, 1.0 = inclinação máxima)
} AnalogStick;

void hal_input_init(void);
void hal_input_process_event(const SDL_Event* event);
void hal_input_update(void);

bool hal_input_is_held(u16 key_mask);
bool hal_input_is_pressed(u16 key_mask);
bool hal_input_is_released(u16 key_mask);

/*
 * Retorna o estado vetorial da alavanca analógica esquerda.
 * Permite movimentação analógica com velocidade proporcional à inclinação do dedo.
 */
AnalogStick hal_input_get_left_stick(void);

u16 hal_input_get_gba_keyinput(void);
const char* hal_input_get_controller_name(void);
void hal_input_shutdown(void);

#endif // HAL_INPUT_H
