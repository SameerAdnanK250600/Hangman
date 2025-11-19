#ifndef LOADING_SCREEN_H
#define LOADING_SCREEN_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Initialize the loading screen
bool loading_screen_init(SDL_Window *window, SDL_Renderer *renderer);

// Render the loading screen with current progress (0.0 to 1.0)
void loading_screen_render(SDL_Renderer *renderer, SDL_Window *window, float progress);

// Cleanup loading screen resources
void loading_screen_destroy(void);

#endif // LOADING_SCREEN_H