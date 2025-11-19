#include "texture_manager.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_thread.h>
#include <stdio.h>
#include <string.h>

// Global texture instances
MainMenuTextures g_mainMenuTextures = {0};
IngameUITextures g_ingameUITextures = {0};
AboutSectionTextures g_aboutTextures = {0};

// Surface storage for async loading
static struct {
    SDL_Surface *mainMenu_bg;
    SDL_Surface *mainMenu_start;
    SDL_Surface *mainMenu_startHover;
    SDL_Surface *mainMenu_about;
    SDL_Surface *mainMenu_aboutHover;

    SDL_Surface *about_bg;

    SDL_Surface *ingame_frames[180];
    SDL_Surface *ingame_lives[7];
    SDL_Surface *ingame_pause;
    SDL_Surface *ingame_lettersPull;
    SDL_Surface *ingame_lettersPulled;
    SDL_Surface *ingame_powerBg;
    SDL_Surface *ingame_powerBoxes[9];
} g_loadedSurfaces = {0};

// Threading state
static SDL_Thread *g_loadThread = NULL;
static SDL_atomic_t g_loadProgress;
static SDL_atomic_t g_surfacesLoaded;
static SDL_atomic_t g_texturesCreated;

// ============================================================================
// MAIN MENU TEXTURES
// ============================================================================

bool texture_manager_init_main_menu(SDL_Renderer *renderer) {
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("[ERROR] SDL_image init failed: %s\n", IMG_GetError());
        return false;
    }

    g_mainMenuTextures.background = IMG_LoadTexture(renderer, "resources/textures/main_menu/background.png");
    if (!g_mainMenuTextures.background) {
        printf("[ERROR] Failed to load main menu background: %s\n", IMG_GetError());
        return false;
    }

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

    SDL_Surface *surf = IMG_Load("resources/textures/ingame_ui/pause_menu.png");
    if (surf) {
        g_ingameUITextures.pauseTex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

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

    if (!g_ingameUITextures.lettersTex[0] && g_ingameUITextures.lettersTex[1]) {
        g_ingameUITextures.lettersTex[0] = g_ingameUITextures.lettersTex[1];
        g_ingameUITextures.lettersSurf[0] = g_ingameUITextures.lettersSurf[1];
    }
    if (!g_ingameUITextures.lettersTex[1] && g_ingameUITextures.lettersTex[0]) {
        g_ingameUITextures.lettersTex[1] = g_ingameUITextures.lettersTex[0];
        g_ingameUITextures.lettersSurf[1] = g_ingameUITextures.lettersSurf[0];
    }

    surf = IMG_Load("resources/textures/ingame_ui/power_ui/power.png");
    if (surf) {
        g_ingameUITextures.powerUI_bg = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

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
    for (int i = 0; i < 180; i++) {
        if (g_ingameUITextures.frames[i]) {
            SDL_DestroyTexture(g_ingameUITextures.frames[i]);
        }
    }

    for (int i = 0; i <= 6; i++) {
        if (g_ingameUITextures.livesTextures[i]) {
            SDL_DestroyTexture(g_ingameUITextures.livesTextures[i]);
        }
    }

    if (g_ingameUITextures.pauseTex) SDL_DestroyTexture(g_ingameUITextures.pauseTex);

    if (g_ingameUITextures.lettersTex[0] &&
        g_ingameUITextures.lettersTex[0] != g_ingameUITextures.lettersTex[1]) {
        SDL_DestroyTexture(g_ingameUITextures.lettersTex[0]);
    }
    if (g_ingameUITextures.lettersTex[1]) {
        SDL_DestroyTexture(g_ingameUITextures.lettersTex[1]);
    }

    if (g_ingameUITextures.lettersSurf[0] &&
        g_ingameUITextures.lettersSurf[0] != g_ingameUITextures.lettersSurf[1]) {
        SDL_FreeSurface(g_ingameUITextures.lettersSurf[0]);
    }
    if (g_ingameUITextures.lettersSurf[1]) {
        SDL_FreeSurface(g_ingameUITextures.lettersSurf[1]);
    }

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
// GLOBAL SETUP (Synchronous)
// ============================================================================

bool texture_manager_init(SDL_Renderer *renderer) {
    return texture_manager_init_main_menu(renderer) &&
           texture_manager_init_about_section(renderer) &&
           texture_manager_init_ingame_ui(renderer);
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

// ============================================================================
// THREADED LOADING IMPLEMENTATION
// ============================================================================

static int surface_load_thread(void *data) {
    char path[512];
    int totalItems = 5 + 1 + 180 + 7 + 1 + 2 + 1 + 9; // Total surface count = 206
    int currentItem = 0;

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("[ERROR] SDL_image init failed in thread: %s\n", IMG_GetError());
        return 1;
    }

    SDL_AtomicSet(&g_loadProgress, 0);

    // Load main menu surfaces
    g_loadedSurfaces.mainMenu_bg = IMG_Load("resources/textures/main_menu/background.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    g_loadedSurfaces.mainMenu_start = IMG_Load("resources/textures/main_menu/start.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    g_loadedSurfaces.mainMenu_startHover = IMG_Load("resources/textures/main_menu/start_hover.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    g_loadedSurfaces.mainMenu_about = IMG_Load("resources/textures/main_menu/about.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    g_loadedSurfaces.mainMenu_aboutHover = IMG_Load("resources/textures/main_menu/about_hover.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    // Load about section
    g_loadedSurfaces.about_bg = IMG_Load("resources/textures/about_section/background.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    // Load ingame frames (180 BMPs)
    for (int i = 0; i < 180; i++) {
        snprintf(path, sizeof(path), "resources/textures/ingame_ui/background_frames/background_frame_%03d.bmp", i + 1);
        g_loadedSurfaces.ingame_frames[i] = SDL_LoadBMP(path);
        currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));
    }

    // Load lives (7 PNGs)
    for (int i = 0; i <= 6; i++) {
        snprintf(path, sizeof(path), "resources/textures/ingame_ui/%d_lives.png", i);
        g_loadedSurfaces.ingame_lives[i] = IMG_Load(path);
        currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));
    }

    // Load pause menu
    g_loadedSurfaces.ingame_pause = IMG_Load("resources/textures/ingame_ui/pause_menu.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    // Load letters
    g_loadedSurfaces.ingame_lettersPull = IMG_Load("resources/textures/ingame_ui/letters_used_pull.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    g_loadedSurfaces.ingame_lettersPulled = IMG_Load("resources/textures/ingame_ui/letters_used_pulled.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    // Load power UI
    g_loadedSurfaces.ingame_powerBg = IMG_Load("resources/textures/ingame_ui/power_ui/power.png");
    currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));

    for (int i = 0; i < 9; i++) {
        snprintf(path, sizeof(path), "resources/textures/ingame_ui/power_ui/box%d.png", i + 1);
        g_loadedSurfaces.ingame_powerBoxes[i] = IMG_Load(path);
        currentItem++; SDL_AtomicSet(&g_loadProgress, (int)((currentItem / (float)totalItems) * 100.0f));
    }

    SDL_AtomicSet(&g_loadProgress, 100);
    SDL_AtomicSet(&g_surfacesLoaded, 1);

    return 0;
}

bool texture_manager_start_async_load(void) {
    SDL_AtomicSet(&g_loadProgress, 0);
    SDL_AtomicSet(&g_surfacesLoaded, 0);
    SDL_AtomicSet(&g_texturesCreated, 0);

    g_loadThread = SDL_CreateThread(surface_load_thread, "SurfaceLoader", NULL);
    if (!g_loadThread) {
        printf("[ERROR] Failed to create surface loading thread: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool texture_manager_process_loaded_surfaces(SDL_Renderer *renderer) {
    if (!SDL_AtomicGet(&g_surfacesLoaded)) {
        return false; // Still loading
    }

    if (SDL_AtomicGet(&g_texturesCreated)) {
        return true; // Already done
    }

    // Wait for thread to complete
    if (g_loadThread) {
        SDL_WaitThread(g_loadThread, NULL);
        g_loadThread = NULL;
    }

    // Create textures from loaded surfaces on main thread

    // Main menu
    if (g_loadedSurfaces.mainMenu_bg) {
        g_mainMenuTextures.background = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.mainMenu_bg);
        SDL_FreeSurface(g_loadedSurfaces.mainMenu_bg);
    }

    if (g_loadedSurfaces.mainMenu_start) {
        g_mainMenuTextures.startSurface = g_loadedSurfaces.mainMenu_start; // Keep surface
        g_mainMenuTextures.start = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.mainMenu_start);
    }

    if (g_loadedSurfaces.mainMenu_startHover) {
        g_mainMenuTextures.startHover = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.mainMenu_startHover);
        SDL_FreeSurface(g_loadedSurfaces.mainMenu_startHover);
    }

    if (g_loadedSurfaces.mainMenu_about) {
        g_mainMenuTextures.aboutSurface = g_loadedSurfaces.mainMenu_about; // Keep surface
        g_mainMenuTextures.about = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.mainMenu_about);
    }

    if (g_loadedSurfaces.mainMenu_aboutHover) {
        g_mainMenuTextures.aboutHover = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.mainMenu_aboutHover);
        SDL_FreeSurface(g_loadedSurfaces.mainMenu_aboutHover);
    }

    // About section
    if (g_loadedSurfaces.about_bg) {
        g_aboutTextures.background = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.about_bg);
        SDL_FreeSurface(g_loadedSurfaces.about_bg);
    }

    // Ingame frames
    for (int i = 0; i < 180; i++) {
        if (g_loadedSurfaces.ingame_frames[i]) {
            g_ingameUITextures.frames[i] = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.ingame_frames[i]);
            SDL_FreeSurface(g_loadedSurfaces.ingame_frames[i]);
        }
    }

    // Ingame lives
    for (int i = 0; i <= 6; i++) {
        if (g_loadedSurfaces.ingame_lives[i]) {
            g_ingameUITextures.livesTextures[i] = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.ingame_lives[i]);
            SDL_FreeSurface(g_loadedSurfaces.ingame_lives[i]);
        }
    }

    // Pause
    if (g_loadedSurfaces.ingame_pause) {
        g_ingameUITextures.pauseTex = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.ingame_pause);
        SDL_FreeSurface(g_loadedSurfaces.ingame_pause);
    }

    // Letters
    if (g_loadedSurfaces.ingame_lettersPull) {
        g_ingameUITextures.lettersSurf[0] = g_loadedSurfaces.ingame_lettersPull; // Keep surface
        g_ingameUITextures.lettersTex[0] = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.ingame_lettersPull);
    }

    if (g_loadedSurfaces.ingame_lettersPulled) {
        g_ingameUITextures.lettersSurf[1] = g_loadedSurfaces.ingame_lettersPulled; // Keep surface
        g_ingameUITextures.lettersTex[1] = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.ingame_lettersPulled);
    }

    // Fallback for letters
    if (!g_ingameUITextures.lettersTex[0] && g_ingameUITextures.lettersTex[1]) {
        g_ingameUITextures.lettersTex[0] = g_ingameUITextures.lettersTex[1];
        g_ingameUITextures.lettersSurf[0] = g_ingameUITextures.lettersSurf[1];
    }
    if (!g_ingameUITextures.lettersTex[1] && g_ingameUITextures.lettersTex[0]) {
        g_ingameUITextures.lettersTex[1] = g_ingameUITextures.lettersTex[0];
        g_ingameUITextures.lettersSurf[1] = g_ingameUITextures.lettersSurf[0];
    }

    // Power UI
    if (g_loadedSurfaces.ingame_powerBg) {
        g_ingameUITextures.powerUI_bg = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.ingame_powerBg);
        SDL_FreeSurface(g_loadedSurfaces.ingame_powerBg);
    }

    for (int i = 0; i < 9; i++) {
        if (g_loadedSurfaces.ingame_powerBoxes[i]) {
            g_ingameUITextures.powerUI_boxSurfs[i] = g_loadedSurfaces.ingame_powerBoxes[i]; // Keep surface
            g_ingameUITextures.powerUI_boxes[i] = SDL_CreateTextureFromSurface(renderer, g_loadedSurfaces.ingame_powerBoxes[i]);
        }
    }

    SDL_AtomicSet(&g_texturesCreated, 1);
    return true;
}

float texture_manager_get_progress(void) {
    return SDL_AtomicGet(&g_loadProgress) / 100.0f;
}

bool texture_manager_surfaces_loaded(void) {
    return SDL_AtomicGet(&g_surfacesLoaded) == 1;
}

bool texture_manager_is_fully_loaded(void) {
    return SDL_AtomicGet(&g_texturesCreated) == 1;
}