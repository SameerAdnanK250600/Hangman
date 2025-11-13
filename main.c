#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <glad/glad.h>
#include <stdbool.h>
#include "screens/main_menu.h"
#include "game/hangman.h"
#include "utility/utilities.h"

#define SDL_MAIN_HANDLED

int main(int argc, char* argv[]) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("Could not init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() == -1) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        exit(1);
    }

    SDL_Window *window = SDL_CreateWindow(
        "Hangman", // puts the window name as Hangman
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        exit(1);
    }

    if (!main_menu_init(window, renderer)) {
        printf("Main menu failed to initialize.\n");
        exit(1);
    }

    //window icon
    SDL_Surface* icon = SDL_LoadBMP("resources/icon.bmp");
    if (!icon) {
        printf("Could not load icon: %s\n", SDL_GetError());
    } else {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon); //free after setting
    }

    if (!window) {
        printf("Failed to init window: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to load GL: %s\n", SDL_GetError());
        exit(1);
    }

    puts("OpenGL loaded");
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));

    bool shouldQuit = false;
    bool inMenu = true;

    while (!shouldQuit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                shouldQuit = true;
            }

            if (inMenu) {
                MenuAction action = main_menu_handle_event(window, renderer, &event);
                if (action == MENU_START) {
                    printf("Start Game pressed!\n");
                    // TODO: switch to the game screen here
                } else if (action == MENU_ABOUT) {
                    printf("About pressed!\n");
                    // TODO: switch to about screen
                }
            }
        }

        if (inMenu) {
            main_menu_render(renderer);
        }
    }

    main_menu_destroy();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    printf("%s\n", getRandomWordFromFile("continents"));
    printf("%s\n", getRandomWordFromFile("planets"));
    printf("%s\n", getRandomWordFromFile("countries"));
    printf("%s\n", getRandomWordFromFile("animals"));
    printf("%s\n", getRandomWordFromFile("fruits"));
    printf("%s\n", getRandomWordFromFile("vegetables"));
    return 0;
}