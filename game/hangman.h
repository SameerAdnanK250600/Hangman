#ifndef HANGMAN_H
#define HANGMAN_H

#include <stdbool.h>

#define MAX_WORD_LEN 128
#define MAX_GUESSED 64

typedef struct {
    char word[MAX_WORD_LEN];
    char revealed[MAX_WORD_LEN];
    char guessed[MAX_GUESSED];
    int numGuessed;
    int lives;
    int shieldActive;
    int superBlankPos;
} GameState;


// hangman
GameState initHangman(const char *word, int lives);

bool processGuess(GameState *game, char guess);

bool validateGuess(const GameState *game, char guess);

bool isGameWon(const GameState *game);

bool isGameOver(const GameState *game);

const char *getRevealedWord(const GameState *game);

int getRemainingLives(const GameState *game);

const char *getGuessedLetters(const GameState *game);

GameState resetGame(const char *word, int lives);

// powers
void applyPowerUp(GameState *game);

void activatePowerUp(GameState *game, int power_id);

#endif
