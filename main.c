#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <stdbool.h>
#include "hangman.h"
#include "utilities.h"

#define SDL_MAIN_HANDLED

int main(int argc, char* argv[]) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("Could not init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_Window *window = SDL_CreateWindow(
        "Hangman", // puts the window name as Hangman
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1080, 720,
        SDL_WINDOW_OPENGL
    );

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

    while (!shouldQuit) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    shouldQuit = true;
                    break;
                default:
                    break;
            }
        }
    }

    test();
    printf("%s\n", getRandomWordFromFile("continents"));
    printf("%s\n", getRandomWordFromFile("planets"));
    printf("%s\n", getRandomWordFromFile("countries"));
    printf("%s\n", getRandomWordFromFile("animals"));
    printf("%s\n", getRandomWordFromFile("fruits"));
    printf("%s\n", getRandomWordFromFile("vegetables"));
    return 0;
}