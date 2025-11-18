#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef enum {
    MENU_NONE = 0,
    MENU_START,
    MENU_ABOUT
} MenuAction;

bool main_menu_init(SDL_Window *window, SDL_Renderer *renderer);

void main_menu_destroy();

MenuAction main_menu_handle_event(SDL_Window *window, SDL_Renderer *renderer, SDL_Event *e);

void main_menu_render(SDL_Renderer *renderer, SDL_Window *window);

#endif // MAIN_MENU_H
