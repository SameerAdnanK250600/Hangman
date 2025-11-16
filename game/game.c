#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "hangman.h"
#include "../screens/ingame_ui.h"
#include "../utility/utilities.h"

int runGame(SDL_Window* window, SDL_Renderer* renderer) {
    srand(time(NULL));

    // Get random word
    char *word = getRandomWordFromFile("continents");
    if (!word) {
        printf("Failed to load word.\n");
        return 1;
    }

    // Initialize game state
    GameState game = initHangman(word, 5);
    free(word);

    // Initialize ingame UI with pointer to game state
    if (!ingame_ui_init(window, renderer, &game)) {
        printf("Failed to init ingame UI.\n");
        return 1;
    }

    printf("Starting Hangman:\n");

    Uint64 lastTime = SDL_GetPerformanceCounter();

    while (!isGameOver(&game)) {
        // --- Handle delta time ---
        Uint64 now = SDL_GetPerformanceCounter();
        float deltaTime = (float)(now - lastTime) / SDL_GetPerformanceFrequency();
        lastTime = now;

        // --- Update UI ---
        ingame_ui_update(deltaTime);
        ingame_ui_render(renderer, window);

        // --- CLI Input ---
        printf("\nWord: %s\n", game.revealed);
        printf("Lives: %d\n", game.lives);
        printf("Guessed: %s\n", game.guessed);

        printf("Enter guess: ");
        char guess;
        if (scanf(" %c", &guess) != 1) continue;

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
            break;
        }

        // Small delay so the UI has time to render
        SDL_Delay(10);
    }

    if (!isGameWon(&game)) {
        printf("Game over! Word was: %s\n", game.word);
    }

    // Clean up UI
    ingame_ui_destroy();

    return 0;
}
