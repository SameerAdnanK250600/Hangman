//
// Created by Sameer on 03/10/2025.
//
#include <stdio.h>
#include "hangman.h"

#include <string.h>

void initHangman(GameState *game, const char *word, int lives) {
    strcpy(game->word, word);

    for (int i = 0; i < strlen(word); i++) {
        game->revealed[i] = '_';
    }
    game->revealed[strlen(word)] = '\0';

    game->guessed[0] = '\0';
    game->numGuessed = 0;
    game->lives = lives;
}