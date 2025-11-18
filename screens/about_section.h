#ifndef ABOUT_SECTION_H
#define ABOUT_SECTION_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// --- Initialize the About Section ---
// window: SDL_Window pointer
// renderer: SDL_Renderer pointer
// Returns true on success, false on failure
bool about_section_init(SDL_Window *window, SDL_Renderer *renderer);

// --- Destroy resources used by About Section ---
void about_section_destroy();

// --- Render the About Section ---
void about_section_render(SDL_Renderer *renderer, SDL_Window *window);

#endif // ABOUT_SECTION_H
