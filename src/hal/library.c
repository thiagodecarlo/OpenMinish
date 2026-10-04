/*
 * ============================================================================
 * src/hal/library.c - Biblioteca Real de Hyrule & Quest dos Livros de Librari
 * ============================================================================
 */

#include "hal/library.h"
#include "hal/audio.h"
#include <stdio.h>
#include <string.h>

static LibraryQuestState s_lib_quest = { 0 };

static const char* s_book_names[BOOK_COUNT] = {
    "A Hyrulean Bestiary (O Bestiario de Hyrule)",
    "Legend of the Picori (A Lenda dos Picori)",
    "A History of Masks (Historia das Mascaras)"
};

void library_quest_init(void) {
    memset(&s_lib_quest, 0, sizeof(s_lib_quest));
}

LibraryQuestState* library_get_quest_state(void) {
    return &s_lib_quest;
}

void library_collect_book(LibraryBookId id) {
    if (id < BOOK_COUNT && !s_lib_quest.book_found[id]) {
        s_lib_quest.book_found[id] = true;
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.3f);
        printf("[LIBRARY] Livro encontrado: \"%s\"!\n", s_book_names[id]);
    }
}

bool library_has_book(LibraryBookId id) {
    if (id < BOOK_COUNT) return s_lib_quest.book_found[id];
    return false;
}

void library_return_books_to_shelf(void) {
    int newly_returned = 0;
    for (int i = 0; i < BOOK_COUNT; i++) {
        if (s_lib_quest.book_found[i] && !s_lib_quest.book_returned[i]) {
            s_lib_quest.book_returned[i] = true;
            s_lib_quest.books_returned_count++;
            newly_returned++;
            printf("[LIBRARY] Livro devolvido a estante: \"%s\"!\n", s_book_names[i]);
        }
    }

    if (newly_returned > 0) {
        hal_audio_play_sound(SOUND_SWITCH_CLICK, 1.0f, 1.0f);
        if (s_lib_quest.books_returned_count >= BOOK_COUNT && !s_lib_quest.staircase_formed) {
            s_lib_quest.staircase_formed = true;
            hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
            printf("[LIBRARY] Todos os 3 livros foram devolvidos! Uma escadaria colossal se formou para o Link Minish!\n");
        }
    }
}

bool library_is_staircase_formed(void) {
    return s_lib_quest.staircase_formed;
}

bool library_talk_to_librari(void) {
    if (!s_lib_quest.librari_met) {
        s_lib_quest.librari_met = true;
        s_lib_quest.temple_unlocked = true;
        hal_audio_play_sound(SOUND_SECRET, 1.0f, 1.0f);
        printf("[LIBRARI] \"Ho ho! Voce encontrou os livros atrasados! Eu sou Librari, o sabio da biblioteca!\"\n");
        printf("[LIBRARI] \"O Temple of Droplets repousa congelado nas profundezas de Lake Hylia! Abrirei a passagem para voce!\"\n");
        return true;
    }
    return false;
}

bool library_is_temple_unlocked(void) {
    return s_lib_quest.temple_unlocked;
}

u8 library_get_save_mask(void) {
    u8 mask = 0;
    for (int i = 0; i < BOOK_COUNT; i++) {
        if (s_lib_quest.book_found[i])    mask |= (1 << i);
        if (s_lib_quest.book_returned[i]) mask |= (1 << (i + 3));
    }
    if (s_lib_quest.staircase_formed) mask |= (1 << 6);
    return mask;
}

void library_restore_save(u8 mask, bool librari_met, bool temple_unlocked) {
    for (int i = 0; i < BOOK_COUNT; i++) {
        s_lib_quest.book_found[i]    = (mask & (1 << i)) != 0;
        s_lib_quest.book_returned[i] = (mask & (1 << (i + 3))) != 0;
        if (s_lib_quest.book_returned[i]) s_lib_quest.books_returned_count++;
    }
    s_lib_quest.staircase_formed = (mask & (1 << 6)) != 0 || (s_lib_quest.books_returned_count >= BOOK_COUNT);
    s_lib_quest.librari_met = librari_met;
    s_lib_quest.temple_unlocked = temple_unlocked;
}
