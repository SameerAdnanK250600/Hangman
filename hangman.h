//
// Created by Sameer on 03/10/2025.
//

#ifndef HANGMAN_HANGMAN_H
#define HANGMAN_HANGMAN_H

#include <stdbool.h>

#define MAX_WORD_LEN 50
#define MAX_GUESSED 26

// struct representing the current state of hangman game
typedef struct {
    char word[MAX_WORD_LEN];        // word to guess
    char revealed[MAX_WORD_LEN];    // underscores and revealed letters
    char guessed[MAX_GUESSED];      // letters player guessed so far
    int numGuessed;                 // no of letters guessed
    int lives;                      // remaining lives
} GameState;

void test();

#endif //HANGMAN_HANGMAN_H