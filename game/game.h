#pragma once
#include <stdbool.h>

#define MAX_WORD_LENGTH 128
#define MAX_GUESSED 26

typedef struct {
    char word[MAX_WORD_LENGTH];
    char revealed[MAX_WORD_LENGTH];
    char guessed[MAX_GUESSED];
    int numGuessed;
    int lives;
} GameState;

// Initialize the game with a word and starting lives
GameState initHangman(const char *word, int startingLives);

// Check if the game is over
bool isGameOver(GameState *game);

// Check if player won
bool isGameWon(GameState *game);

// Validate a guess (returns true if letter is valid)
bool validateGuess(GameState *game, char guess);

// Process a guess (returns true if special power-up hit)
bool processGuess(GameState *game, char guess);

// Apply any power-ups
void applyPowerUp(GameState *game);

// Append a character to the guessed array
void appendCharToArray(char *arr, char c, int *size, int maxSize);
