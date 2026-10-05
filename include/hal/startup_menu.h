#ifndef HAL_STARTUP_MENU_H
#define HAL_STARTUP_MENU_H

/*
 * ============================================================================
 * include/hal/startup_menu.h - Sistema de Telas de Abertura & Seleção de Save
 * ============================================================================
 * Implementa a máquina de estados autêntica de boot de The Minish Cap:
 *   1. Logo da Capcom (com jingle harmônico característico)
 *   2. Logo da Nintendo (vermelho clássico com fade suave)
 *   3. Tela de Título com Logotipo, Four Sword reluzente e "PRESS START"
 *   4. Tela de Seleção de Arquivo (3 Slots com corações, rupees e elementos)
 *   5. Registro de Nome do Herói (Teclado virtual minish de 6 letras)
 *   6. Modos de Cópia e Exclusão de Arquivos de Save
 */

#include "gba/types.h"
#include "hal/save.h"
#include <stdbool.h>

typedef enum {
    STARTUP_STATE_NONE = 0,
    STARTUP_STATE_CAPCOM_LOGO,
    STARTUP_STATE_NINTENDO_LOGO,
    STARTUP_STATE_TITLE_SCREEN,
    STARTUP_STATE_FILE_SELECT,
    STARTUP_STATE_NAME_ENTRY,
    STARTUP_STATE_FILE_COPY,
    STARTUP_STATE_FILE_ERASE,
    STARTUP_STATE_TRANSITION_OUT,
    STARTUP_STATE_FINISHED
} StartupMenuState;

/*
 * Inicializa a máquina de estados do menu inicial e carrega os metadados
 * dos slots de salvamento existentes.
 */
void startup_menu_init(void);

/*
 * Retorna true se a sequência de inicialização/menus ainda estiver ativa
 * (antes de passar o controle definitivo para o gameplay do mundo).
 */
bool startup_menu_is_active(void);

/*
 * Atualiza lógica de animação, transições, timers e processa o REG_KEYINPUT.
 */
void startup_menu_update(void);

/*
 * Renderiza a cena atual do menu no framebuffer do HAL Video.
 */
void startup_menu_render(void);

/*
 * Retorna o slot selecionado pelo jogador (1, 2 ou 3).
 */
int startup_menu_get_selected_slot(void);

/*
 * Retorna true se o slot selecionado é um arquivo recém-criado (Novo Jogo).
 */
bool startup_menu_is_new_game(void);

/*
 * Retorna o nome registrado para o herói (máximo 6 letras, padrão "LINK").
 */
const char* startup_menu_get_player_name(void);

/*
 * Retorna o estado atual da máquina de estados.
 */
StartupMenuState startup_menu_get_state(void);

/*
 * Força o retorno do jogo para a tela de título (ex: Save & Quit ou F10).
 */
void startup_menu_return_to_title(void);

/*
 * Libera quaisquer recursos temporários alocados pelo menu.
 */
void startup_menu_shutdown(void);

#endif // HAL_STARTUP_MENU_H
