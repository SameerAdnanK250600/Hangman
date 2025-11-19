#include "texture_manager.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

// Global texture instances
MainMenuTextures g_mainMenuTextures = {0};
IngameUITextures g_ingameUITextures = {0};
AboutSectionTextures g_aboutTextures = {0};

// ============================================================================
// MAIN MENU TEXTURES
// ============================================================================

bool texture_manager_init_main_menu(SDL_Renderer *renderer) {
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("[ERROR] SDL_image init failed: %s\n", IMG_GetError());
        return false;
    }

    // Load background
    g_mainMenuTextures.background = IMG_LoadTexture(renderer, "resources/textures/main_menu/background.png");
    if (!g_mainMenuTextures.background) {
        printf("[ERROR] Failed to load main menu background: %s\n", IMG_GetError());
        return false;
    }

    // Load Start button
    g_mainMenuTextures.startSurface = IMG_Load("resources/textures/main_menu/start.png");
    if (!g_mainMenuTextures.startSurface) {
        printf("[ERROR] Failed to load start surface: %s\n", IMG_GetError());
        return false;
    }
    g_mainMenuTextures.start = SDL_CreateTextureFromSurface(renderer, g_mainMenuTextures.startSurface);

    SDL_Surface *startHoverSurf = IMG_Load("resources/textures/main_menu/start_hover.png");
    if (startHoverSurf) {
        g_mainMenuTextures.startHover = SDL_CreateTextureFromSurface(renderer, startHoverSurf);
        SDL_FreeSurface(startHoverSurf);
    }

    // Load About button
    g_mainMenuTextures.aboutSurface = IMG_Load("resources/textures/main_menu/about.png");
    if (!g_mainMenuTextures.aboutSurface) {
        printf("[ERROR] Failed to load about surface: %s\n", IMG_GetError());
        return false;
    }
    g_mainMenuTextures.about = SDL_CreateTextureFromSurface(renderer, g_mainMenuTextures.aboutSurface);

    SDL_Surface *aboutHoverSurf = IMG_Load("resources/textures/main_menu/about_hover.png");
    if (aboutHoverSurf) {
        g_mainMenuTextures.aboutHover = SDL_CreateTextureFromSurface(renderer, aboutHoverSurf);
        SDL_FreeSurface(aboutHoverSurf);
    }

    return true;
}

void texture_manager_destroy_main_menu(void) {
    if (g_mainMenuTextures.background) SDL_DestroyTexture(g_mainMenuTextures.background);
    if (g_mainMenuTextures.start) SDL_DestroyTexture(g_mainMenuTextures.start);
    if (g_mainMenuTextures.startHover) SDL_DestroyTexture(g_mainMenuTextures.startHover);
    if (g_mainMenuTextures.startSurface) SDL_FreeSurface(g_mainMenuTextures.startSurface);
    if (g_mainMenuTextures.about) SDL_DestroyTexture(g_mainMenuTextures.about);
    if (g_mainMenuTextures.aboutHover) SDL_DestroyTexture(g_mainMenuTextures.aboutHover);
    if (g_mainMenuTextures.aboutSurface) SDL_FreeSurface(g_mainMenuTextures.aboutSurface);

    memset(&g_mainMenuTextures, 0, sizeof(MainMenuTextures));
}

// ============================================================================
// INGAME UI TEXTURES
// ============================================================================

bool texture_manager_init_ingame_ui(SDL_Renderer *renderer) {
    char path[512];

    // Load background frames
    for (int i = 0; i < 180; i++) {
        snprintf(path, sizeof(path), "resources/textures/ingame_ui/background_frames/background_frame_%03d.bmp", i + 1);
        SDL_Surface *surf = SDL_LoadBMP(path);
        if (!surf) {
            g_ingameUITextures.frames[i] = NULL;
            continue;
        }
        g_ingameUITextures.frames[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    // Load lives textures (0-6)
    for (int i = 0; i <= 6; i++) {
        snprintf(path, sizeof(path), "resources/textures/ingame_ui/%d_lives.png", i);
        SDL_Surface *surf = IMG_Load(path);
        if (!surf) {
            g_ingameUITextures.livesTextures[i] = NULL;
            continue;
        }
        g_ingameUITextures.livesTextures[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    // Load pause menu
    SDL_Surface *surf = IMG_Load("resources/textures/ingame_ui/pause_menu.png");
    if (surf) {
        g_ingameUITextures.pauseTex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    // Load letters used textures
    surf = IMG_Load("resources/textures/ingame_ui/letters_used_pull.png");
    if (surf) {
        g_ingameUITextures.lettersSurf[0] = surf;
        g_ingameUITextures.lettersTex[0] = SDL_CreateTextureFromSurface(renderer, surf);
    }

    surf = IMG_Load("resources/textures/ingame_ui/letters_used_pulled.png");
    if (surf) {
        g_ingameUITextures.lettersSurf[1] = surf;
        g_ingameUITextures.lettersTex[1] = SDL_CreateTextureFromSurface(renderer, surf);
    }

    // Fallback if one is missing
    if (!g_ingameUITextures.lettersTex[0] && g_ingameUITextures.lettersTex[1]) {
        g_ingameUITextures.lettersTex[0] = g_ingameUITextures.lettersTex[1];
        g_ingameUITextures.lettersSurf[0] = g_ingameUITextures.lettersSurf[1];
    }
    if (!g_ingameUITextures.lettersTex[1] && g_ingameUITextures.lettersTex[0]) {
        g_ingameUITextures.lettersTex[1] = g_ingameUITextures.lettersTex[0];
        g_ingameUITextures.lettersSurf[1] = g_ingameUITextures.lettersSurf[0];
    }

    // Load power UI background
    surf = IMG_Load("resources/textures/ingame_ui/power_ui/power.png");
    if (surf) {
        g_ingameUITextures.powerUI_bg = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    // Load power UI boxes
    for (int i = 0; i < 9; i++) {
        snprintf(path, sizeof(path), "resources/textures/ingame_ui/power_ui/box%d.png", i + 1);
        SDL_Surface *surf = IMG_Load(path);
        if (surf) {
            g_ingameUITextures.powerUI_boxSurfs[i] = surf;
            g_ingameUITextures.powerUI_boxes[i] = SDL_CreateTextureFromSurface(renderer, surf);
        }
    }

    return true;
}

void texture_manager_destroy_ingame_ui(void) {
    // Destroy frames
    for (int i = 0; i < 180; i++) {
        if (g_ingameUITextures.frames[i]) {
            SDL_DestroyTexture(g_ingameUITextures.frames[i]);
        }
    }

    // Destroy lives textures
    for (int i = 0; i <= 6; i++) {
        if (g_ingameUITextures.livesTextures[i]) {
            SDL_DestroyTexture(g_ingameUITextures.livesTextures[i]);
        }
    }

    // Destroy pause texture
    if (g_ingameUITextures.pauseTex) SDL_DestroyTexture(g_ingameUITextures.pauseTex);

    // Destroy letters textures (be careful with duplicates)
    if (g_ingameUITextures.lettersTex[0] &&
        g_ingameUITextures.lettersTex[0] != g_ingameUITextures.lettersTex[1]) {
        SDL_DestroyTexture(g_ingameUITextures.lettersTex[0]);
    }
    if (g_ingameUITextures.lettersTex[1]) {
        SDL_DestroyTexture(g_ingameUITextures.lettersTex[1]);
    }

    // Destroy letters surfaces (be careful with duplicates)
    if (g_ingameUITextures.lettersSurf[0] &&
        g_ingameUITextures.lettersSurf[0] != g_ingameUITextures.lettersSurf[1]) {
        SDL_FreeSurface(g_ingameUITextures.lettersSurf[0]);
    }
    if (g_ingameUITextures.lettersSurf[1]) {
        SDL_FreeSurface(g_ingameUITextures.lettersSurf[1]);
    }

    // Destroy power UI
    if (g_ingameUITextures.powerUI_bg) SDL_DestroyTexture(g_ingameUITextures.powerUI_bg);

    for (int i = 0; i < 9; i++) {
        if (g_ingameUITextures.powerUI_boxes[i]) {
            SDL_DestroyTexture(g_ingameUITextures.powerUI_boxes[i]);
        }
        if (g_ingameUITextures.powerUI_boxSurfs[i]) {
            SDL_FreeSurface(g_ingameUITextures.powerUI_boxSurfs[i]);
        }
    }

    memset(&g_ingameUITextures, 0, sizeof(IngameUITextures));
}

// ============================================================================
// ABOUT SECTION TEXTURES
// ============================================================================

bool texture_manager_init_about_section(SDL_Renderer *renderer) {
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("[ERROR] SDL_image init failed for about section: %s\n", IMG_GetError());
        return false;
    }

    g_aboutTextures.background = IMG_LoadTexture(renderer, "resources/textures/about_section/background.png");
    if (!g_aboutTextures.background) {
        printf("[ERROR] Failed to load about section background: %s\n", IMG_GetError());
        return false;
    }

    return true;
}

void texture_manager_destroy_about_section(void) {
    if (g_aboutTextures.background) SDL_DestroyTexture(g_aboutTextures.background);
    memset(&g_aboutTextures, 0, sizeof(AboutSectionTextures));
}

// ============================================================================
// GLOBAL CLEANUP
// ============================================================================

void texture_manager_destroy_all(void) {
    texture_manager_destroy_main_menu();
    texture_manager_destroy_ingame_ui();
    texture_manager_destroy_about_section();
    IMG_Quit();
}