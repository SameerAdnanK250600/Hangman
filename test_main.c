#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "game/hangman.h"
#include "utility/utilities.h"

int main() {
    srand(time(NULL));

    //get random word file name, and a random word from that file
    char *wordFile = getRandomWordFileName();
    char *word = getRandomWordFromFile(wordFile);

    if (!word) {
        printf("Failed to load word.\n");
        return 1;
    }

    //initialise game state
    GameState game = initHangman(wordFile, word, 6);

    printf("test hangman:\n");

    //main game loop
    while (!isGameOver(&game)) {
        printf("\nHint: %s\n", game.wordFile);
        printf("Word: %s\n", game.revealed);
        printf("Lives: %d\n", game.lives);
        printf("Guessed: %s\n", game.guessed);

        //user guesses character here
        printf("Enter guess: ");
        char guess;
        scanf(" %c", &guess);

        if (!validateGuess(&game, guess)) {
            printf("Invalid guess.\n");
            continue;
        }

        //check if guessed letter is on super blank
        bool hitSuper = processGuess(&game, guess);
        appendCharToArray(game.guessed, tolower(guess), &game.numGuessed, MAX_GUESSED);


        if (hitSuper) {
            powerUpMenu(&game);
        }

        //check if player won
        if (isGameWon(&game)) {
            printf("You won! Word: %s\n", game.word);
            return 0;
        }
    }

    printf("Game over! Word was: %s\n", game.word);

    return 0;
}
