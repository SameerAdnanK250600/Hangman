#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hangman.h"
#include "../utility/utilities.h"

GameState initHangman(const char *word, int lives) {
    GameState game;

    strncpy(game.word, word, MAX_WORD_LEN);
    stringToLower(game.word);

    copyStringToUnderscores(game.revealed, game.word);

    for (int i = 0; game.word[i] != '\0'; i++) {
        if (game.word[i] == ' ')
            game.revealed[i] = ' ';
    }

    // makes random blank a super blank
    int len = strlen(game.word);
    int underscoreIndexes[MAX_WORD_LEN];
    int count = 0;
    for (int i = 0; i < len; i++) {
        if (game.revealed[i] == '_')
            underscoreIndexes[count++] = i;
    }

    if (count > 0)
        game.superBlankPos = underscoreIndexes[rand() % count];
    else
        game.superBlankPos = -1;

    if (game.superBlankPos != -1)
        game.revealed[game.superBlankPos] = '~';

    resetString(game.guessed);
    game.numGuessed = 0;
    game.lives = lives;
    game.shieldActive = 0;

    return game;
}

bool validateGuess(const GameState *game, char guess) {
    if (!isalpha(guess))
        return false;

    guess = tolower(guess);

    if (charInArray(game->guessed, guess, game->numGuessed))
        return false;

    return true;
}

// this function returns true if super blank is guessed
bool processGuess(GameState *game, char guess) {
    guess = tolower(guess);

    bool found = stringHasChar(game->word, guess);

    if (found) {
        revealGuessedLetter(game->word, game->revealed, guess);

        // checks if guess is super blank
        if (game->superBlankPos != -1 && game->word[game->superBlankPos] == guess) {
            game->revealed[game->superBlankPos] = guess;
            game->superBlankPos = -1; // super blank is used up
            return true;
        }

        return false;
    }
    if (game->shieldActive) {
        game->shieldActive = 0;
    } else {
        game->lives--;
    }
    return false;
}

bool isGameWon(const GameState *game) {
    return isWordFullyRevealed(game->revealed);
}

bool isGameOver(const GameState *game) {
    return (isGameWon(game) || game->lives <= 0);
}

const char *getRevealedWord(const GameState *game) {
    return game->revealed;
}

int getRemainingLives(const GameState *game) {
    return game->lives;
}

const char *getGuessedLetters(const GameState *game) {
    return game->guessed;
}

GameState resetGame(const char *word, int lives) {
    return initHangman(word, lives);
}

void activatePowerUp(GameState *game, int power_id) {
    switch (power_id) {
        case 1: {
            // reveals random blank
            int len = strlen(game->word);
            int validIndexes[MAX_WORD_LEN];
            int count = 0;

            for (int i = 0; i < len; i++) {
                if (game->revealed[i] == '_')
                    validIndexes[count++] = i;
            }

            if (count > 0) {
                int idx = validIndexes[rand() % count];
                game->revealed[idx] = game->word[idx];
                printf("Power-Up: A random letter was revealed!\n");
            }
        }
        break;

        case 2: // gives extra life
            game->lives++;
            printf("Power-Up: +1 Life!\n");
            break;

        case 3: // shows all vowels
            for (int i = 0; game->word[i] != '\0'; i++) {
                char c = game->word[i];
                if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
                    game->revealed[i] = c;
            }
            printf("Power-Up: All vowels revealed!\n");
            break;

        case 4: // activates shield
            game->shieldActive = 1;
            printf("Power-Up: Shield activated!\n");
            break;

        case 5: {
            // funny chance power
            int roll = rand() % 100 + 1;

            if (roll <= 5) {
                printf("Bonus random power up!\n");
                activatePowerUp(game, rand() % 4 + 1);
            } else if (roll <= 10) {
                game->lives--;
                printf("Lost 1 life.\n");
            } else {
                printf("Nothing happened.\n");
            }
        }
        break;
    }
}

void applyPowerUp(GameState *game) {
    printf("\nSuper blank found! Choose a box (1–9)\n");

    int boxPower[9];
    for (int i = 0; i < 9; i++) boxPower[i] = 0;

    // choose 3 positions
    int pos1 = rand() % 9, pos2 = rand() % 9, pos3 = rand() % 9;
    while (pos2 == pos1) pos2 = rand() % 9;
    while (pos3 == pos1 || pos3 == pos2) pos3 = rand() % 9;

    // assigns random power ids
    boxPower[pos1] = rand() % 5 + 1;
    boxPower[pos2] = rand() % 5 + 1;
    boxPower[pos3] = rand() % 5 + 1;

    // displays boxes
    for (int i = 1; i <= 9; i++) {
        printf("[%d] ", i);
        if (i % 3 == 0) printf("\n");
    }

    printf("Choose a box: ");
    int choice;
    scanf("%d", &choice);

    if (choice < 1 || choice > 9) {
        printf("Invalid box.\n");
        return;
    }

    if (boxPower[choice - 1] != 0) {
        int pid = boxPower[choice - 1];
        printf("You got power up #%d!\n", pid);
        activatePowerUp(game, pid);
    } else {
        printf("Empty box, no power up.\n");
    }
}
