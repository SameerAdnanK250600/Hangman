#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game/hangman.h"
#include "utility/utilities.h"

int main() {
    srand(time(NULL));

    char *word = getRandomWordFromFile("continents");

    if (!word) {
        printf("Failed to load word.\n");
        return 1;
    }

    GameState game = initHangman(word, 5);
    free(word);

    printf("test hangman:\n");

    while (!isGameOver(&game)) {
        printf("\nWord: %s\n", game.revealed);
        printf("Lives: %d\n", game.lives);
        printf("Guessed: %s\n", game.guessed);

        printf("Enter guess: ");
        char guess;
        scanf(" %c", &guess);

        if (!validateGuess(&game, guess)) {
            printf("Invalid guess.\n");
            continue;
        }

        bool hitSuper = processGuess(&game, guess);
        appendCharToArray(game.guessed, tolower(guess), &game.numGuessed, MAX_GUESSED);

        if (hitSuper)
            applyPowerUp(&game);

        if (isGameWon(&game)) {
            printf("You won! Word: %s\n", game.word);
            return 0;
        }
    }

    printf("Game over! Word was: %s\n", game.word);

    return 0;
}
