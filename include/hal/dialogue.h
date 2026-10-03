#ifndef HAL_DIALOGUE_H
#define HAL_DIALOGUE_H

/*
 * ============================================================================
 * include/hal/dialogue.h - Sistema de Diálogos, Balões de Texto e Ezlo (Companion)
 * ============================================================================
 * Reproduz o sistema autêntico de caixas de diálogo e retratos de Zelda: The Minish Cap.
 * Inclui:
 * - Balão de diálogo decorado com moldura dourada e fundo esmeralda semi-translúcido
 * - Retrato dinâmico animado do Ezlo (Gorro Falante) com bico e olhos expressivos
 * - Retrato do Forest Minish (Picori da Floresta) com gorro de semente e orelhas pontudas
 * - Efeito Typewriter com sons autênticos de fala (blip chirp) sintetizados pelo HAL
 * - Paginação de textos e indicador de seta piscante para avanço de página
 */

#include "gba/types.h"
#include <stdbool.h>

#define MAX_DIALOGUE_PAGES 8
#define MAX_PAGE_LENGTH    256

typedef enum {
    SPEAKER_EZLO = 0,        // Ezlo, o gorro pássaro falante
    SPEAKER_FOREST_MINISH,   // Ancião / Habitante Minish de Minish Woods
    SPEAKER_SWIFTBLADE,      // Mestre Espadachim Swiftblade (Dojo de Hyrule)
    SPEAKER_SIGNPOST,        // Placa de madeira / Altar sagrado
    SPEAKER_SHOPKEEPER,      // Comerciante Stockwell (Dono da Loja Geral de Hyrule)
    SPEAKER_TOWN_CITIZEN,    // Cidadã da Cidade de Hyrule
    SPEAKER_TOWN_GUARD       // Guarda Real do Castelo de Hyrule
} DialogueSpeaker;

typedef enum {
    DIALOGUE_STATE_CLOSED = 0,
    DIALOGUE_STATE_OPENING,
    DIALOGUE_STATE_TYPING,
    DIALOGUE_STATE_WAITING,
    DIALOGUE_STATE_CLOSING
} DialogueState;

/*
 * Inicializa a máquina de estados de diálogos.
 */
void dialogue_init(void);

/*
 * Abre a caixa de diálogo com o interlocutor e páginas de texto especificadas.
 */
void dialogue_show(DialogueSpeaker speaker, const char* name, const char* const* pages, int page_count);

/*
 * Aciona o chamado de Ezlo (tecla [E] ou botão [SELECT]), fornecendo dicas contextuais ao herói.
 */
void dialogue_trigger_ezlo_hint(void);

/*
 * Inicia uma conversa amigável com um NPC Minish da floresta.
 */
void dialogue_trigger_minish_talk(void);

/*
 * Inicia o treinamento do Ataque Giratório com o Mestre Espadachim Swiftblade.
 */
void dialogue_trigger_swiftblade_talk(bool already_learned);

/*
 * Inicia o diálogo de boas-vindas e catálogo de mercadorias da Loja do Stockwell.
 */
void dialogue_trigger_shopkeeper_talk(int link_rupees);

/*
 * Inicia uma conversa com a moradora da Cidade de Hyrule.
 */
void dialogue_trigger_town_citizen_talk(void);

/*
 * Inicia um diálogo com o Guarda Real que vigia o Portão do Castelo.
 */
void dialogue_trigger_town_guard_talk(void);

/*
 * Retorna true se a conclusão do diálogo concede o Pergaminho do Tigre nº 1 (Spin Attack).
 */
bool dialogue_is_swiftblade_reward_pending(void);

/*
 * Consome/limpa a recompensa pendente concedida pelo diálogo.
 */
void dialogue_clear_swiftblade_reward(void);

/*
 * Atualiza o cronômetro do typewriter, animações faciais e transições.
 */
void dialogue_update(void);

/*
 * Avança o diálogo:
 * - Se estiver digitando, revela a página instantaneamente.
 * - Se estiver aguardando, avança para a próxima página ou fecha o balão.
 */
void dialogue_advance(void);

/*
 * Acelera a velocidade de digitação (quando o jogador segura o botão B).
 */
void dialogue_fast_forward(void);

/*
 * Desenha a caixa de diálogo, o retrato e os textos na tela.
 */
void dialogue_render(void);

/*
 * Retorna true se houver um diálogo em andamento (congela a física e movimento do herói/inimigos).
 */
bool dialogue_is_active(void);

#endif // HAL_DIALOGUE_H
