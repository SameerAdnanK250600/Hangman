#include "loading_screen.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

static TTF_Font *g_loadingFont = NULL;
static SDL_Color g_textColor = {255, 255, 255, 255};

bool loading_screen_init(SDL_Window *window, SDL_Renderer *renderer) {
    // Load a font for "Loading..." text
    g_loadingFont = TTF_OpenFont("resources/font/PixelifySans-Bold.ttf", 48);
    if (!g_loadingFont) {
        printf("[WARNING] Failed to load font for loading screen: %s\n", TTF_GetError());
        // Continue anyway - we can still show progress bar
    }
    return true;
}

void loading_screen_render(SDL_Renderer *renderer, SDL_Window *window, float progress) {
    int windowW, windowH;
    SDL_GetWindowSize(window, &windowW, &windowH);

    // Clear to dark background
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_RenderClear(renderer);

    // Draw "Loading..." text
    if (g_loadingFont) {
        SDL_Surface *textSurf = TTF_RenderText_Blended(g_loadingFont, "Loading...", g_textColor);
        if (textSurf) {
            SDL_Texture *textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
            if (textTex) {
                SDL_Rect textRect = {
                    windowW / 2 - textSurf->w / 2,
                    windowH / 2 - 80,
                    textSurf->w,
                    textSurf->h
                };
                SDL_RenderCopy(renderer, textTex, NULL, &textRect);
                SDL_DestroyTexture(textTex);
            }
            SDL_FreeSurface(textSurf);
        }
    }

    // Draw progress bar background
    SDL_Rect barBg = {
        windowW / 2 - 200,
        windowH / 2,
        400,
        30
    };
    SDL_SetRenderDrawColor(renderer, 50, 50, 60, 255);
    SDL_RenderFillRect(renderer, &barBg);

    // Draw progress bar fill
    if (progress > 0.0f) {
        SDL_Rect barFill = {
            windowW / 2 - 200,
            windowH / 2,
            (int)(400 * progress),
            30
        };
        SDL_SetRenderDrawColor(renderer, 100, 200, 100, 255);
        SDL_RenderFillRect(renderer, &barFill);
    }

    // Draw percentage text
    if (g_loadingFont) {
        char percentText[32];
        snprintf(percentText, sizeof(percentText), "%.0f%%", progress * 100.0f);
        
        SDL_Surface *percentSurf = TTF_RenderText_Blended(g_loadingFont, percentText, g_textColor);
        if (percentSurf) {
            SDL_Texture *percentTex = SDL_CreateTextureFromSurface(renderer, percentSurf);
            if (percentTex) {
                SDL_Rect percentRect = {
                    windowW / 2 - percentSurf->w / 2,
                    windowH / 2 + 50,
                    percentSurf->w,
                    percentSurf->h
                };
                SDL_RenderCopy(renderer, percentTex, NULL, &percentRect);
                SDL_DestroyTexture(percentTex);
            }
            SDL_FreeSurface(percentSurf);
        }
    }

    SDL_RenderPresent(renderer);
}

void loading_screen_destroy(void) {
    if (g_loadingFont) {
        TTF_CloseFont(g_loadingFont);
        g_loadingFont = NULL;
    }
}