#ifndef HAL_LIBRARY_H
#define HAL_LIBRARY_H

/*
 * ============================================================================
 * include/hal/library.h - Biblioteca Real de Hyrule & Quest dos Livros de Librari
 * ============================================================================
 * Gerencia a busca pelos 3 livros gigantes atrasados:
 * 1. "A Hyrulean Bestiary" (O Bestiário de Hyrule)
 * 2. "Legend of the Picori" (A Lenda dos Picori)
 * 3. "A History of Masks" (História das Máscaras)
 * E a formação da escadaria colossal de livros para alcançar o Ancião Librari.
 */

#include "gba/types.h"
#include <stdbool.h>

typedef enum {
    BOOK_BESTIARY = 0,    // Livro 1: Bestiário de Hyrule (encontrado na casa de um cidadão)
    BOOK_PICORI_LEGEND,   // Livro 2: A Lenda dos Picori (com o Prefeito Hagen na cabana do lago)
    BOOK_MASKS_HISTORY,   // Livro 3: História das Máscaras (com o estudioso em Minish Woods)
    BOOK_COUNT
} LibraryBookId;

typedef struct {
    bool book_found[BOOK_COUNT];       // Livro coletado no mundo
    bool book_returned[BOOK_COUNT];    // Livro recolocado na estante
    bool staircase_formed;             // Escada de 3 livros formada para Minish Link
    bool librari_met;                  // Encontrou o Ancião Librari no topo da estante
    bool temple_unlocked;              // Librari abriu a passagem secreta para o Temple of Droplets
    int  books_returned_count;
} LibraryQuestState;

/*
 * Inicializa a quest da Biblioteca Real de Hyrule.
 */
void library_quest_init(void);

/*
 * Retorna o ponteiro para o estado global da quest.
 */
LibraryQuestState* library_get_quest_state(void);

/*
 * Coleta um dos livros no overworld.
 */
void library_collect_book(LibraryBookId id);

/*
 * Retorna se o livro já foi coletado pelo herói.
 */
bool library_has_book(LibraryBookId id);

/*
 * Devolve todos os livros coletados às estantes da biblioteca.
 * Se os 3 livros forem devolvidos, a escada de livros é montada.
 */
void library_return_books_to_shelf(void);

/*
 * Retorna true se a escadaria de livros gigantes está formada para o Link Minish escalar.
 */
bool library_is_staircase_formed(void);

/*
 * Interage com o Ancião Librari no topo da estante.
 * Retorna true no primeiro diálogo, concedendo o acesso à passagem para Lake Hylia / Templo.
 */
bool library_talk_to_librari(void);

/*
 * Retorna true se o caminho para o Temple of Droplets no Lake Hylia foi destravado por Librari.
 */
bool library_is_temple_unlocked(void);

/*
 * Retorna máscara de bits e flags para o SaveData.
 */
u8 library_get_save_mask(void);
void library_restore_save(u8 mask, bool librari_met, bool temple_unlocked);

#endif // HAL_LIBRARY_H
