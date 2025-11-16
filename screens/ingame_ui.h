#pragma once
#include <SDL2/SDL.h>
#include <stdbool.h>
#include "../game/hangman.h"

bool ingame_ui_init(SDL_Window* window, SDL_Renderer* renderer, GameState* game);
void ingame_ui_destroy();
void ingame_ui_update(float deltaTime);
void ingame_ui_render(SDL_Renderer* renderer, SDL_Window* window);
void ingame_ui_handle_event(SDL_Event* event);
bool ingame_ui_is_waiting_after_gameover(void);
bool ingame_ui_is_game_over(void);
void setShouldQuit(bool b);
bool ingameUiShouldQuit(void);