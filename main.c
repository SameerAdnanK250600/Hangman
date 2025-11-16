#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include "screens/main_menu.h"
#include "screens/about_section.h"
#include "screens/ingame_ui.h"
#include "game/hangman.h"
#include "utility/utilities.h"

#define SDL_MAIN_HANDLED

int main(int argc, char* argv[]) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL Init failed: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() == -1) {
        printf("TTF Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Hangman",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        printf("Renderer failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!main_menu_init(window, renderer)) return 1;
    if (!about_section_init(window, renderer)) return 1;

    bool shouldQuit = false;
    bool inMenu = true;
    bool inAbout = false;
    bool inGame = false;

    GameState game;

    // Timing
    Uint64 lastTime = SDL_GetPerformanceCounter();

    while (!shouldQuit) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            // Window close
            if (event.type == SDL_QUIT) {
                shouldQuit = true;
            }

            // MAIN MENU INPUT
            if (inMenu) {
                MenuAction action =
                    main_menu_handle_event(window, renderer, &event);

                if (action == MENU_START) {

                    char* word = getRandomWordFromFile("continents");
                    if (!word) {
                        printf("Failed to get word\n");
                        shouldQuit = true;
                        break;
                    }

                    game = initHangman(word, 6);
                    free(word);

                    if (!ingame_ui_init(window, renderer, &game)) {
                        printf("Ingame UI failed\n");
                        shouldQuit = true;
                        break;
                    }

                    inMenu = false;
                    inGame = true;
                }
                else if (action == MENU_ABOUT) {
                    inMenu = false;
                    inAbout = true;
                }
            }

            // ABOUT INPUT
            else if (inAbout) {
                if (event.type == SDL_KEYDOWN &&
                    event.key.keysym.sym == SDLK_ESCAPE) {
                    inAbout = false;
                    inMenu = true;
                }
            }

            // IN-GAME INPUT
            else if (inGame) {

                // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
                // THE FIX: forward ALL input to the ingame UI
                ingame_ui_handle_event(&event);
                // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            }
        }

        // ---- TIME STEP ----
        Uint64 current = SDL_GetPerformanceCounter();
        float deltaTime =
            (float)(current - lastTime) / SDL_GetPerformanceFrequency();
        lastTime = current;

        // ---- RENDER ----
        if (inMenu) {
            main_menu_render(renderer, window);
        }
        else if (inAbout) {
            about_section_render(renderer, window);
        }
        else if (inGame) {
            ingame_ui_update(deltaTime);
            ingame_ui_render(renderer, window);

            // Return to menu if user pressed ESC after game over
            if (ingameUiShouldQuit()) {
                setShouldQuit(false); // reset flag
                ingame_ui_destroy();
                inGame = false;
                inMenu = true;
            }
        }
    }

    // Cleanup
    if (inGame) ingame_ui_destroy();
    main_menu_destroy();
    about_section_destroy();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
