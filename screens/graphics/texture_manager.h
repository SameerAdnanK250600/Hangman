#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <SDL2/SDL.h>
#include <stdbool.h>

// Main Menu Textures
typedef struct {
    SDL_Texture *background;
    SDL_Texture *start;
    SDL_Texture *startHover;
    SDL_Surface *startSurface;
    SDL_Texture *about;
    SDL_Texture *aboutHover;
    SDL_Surface *aboutSurface;
} MainMenuTextures;

// Ingame UI Textures
typedef struct {
    SDL_Texture *frames[180];
    SDL_Texture *livesTextures[7]; // 0-6 lives
    SDL_Texture *pauseTex;
    SDL_Texture *lettersTex[2];
    SDL_Surface *lettersSurf[2];
    SDL_Texture *powerUI_bg;
    SDL_Texture *powerUI_boxes[9];
    SDL_Surface *powerUI_boxSurfs[9];
} IngameUITextures;

// About Section Textures
typedef struct {
    SDL_Texture *background;
} AboutSectionTextures;

// Global texture storage
extern MainMenuTextures g_mainMenuTextures;
extern IngameUITextures g_ingameUITextures;
extern AboutSectionTextures g_aboutTextures;

// Initialization functions
bool texture_manager_init_main_menu(SDL_Renderer *renderer);
bool texture_manager_init_ingame_ui(SDL_Renderer *renderer);
bool texture_manager_init_about_section(SDL_Renderer *renderer);

// Destruction functions
void texture_manager_destroy_main_menu(void);
void texture_manager_destroy_ingame_ui(void);
void texture_manager_destroy_about_section(void);

// Setup all textures (synchronous - old way)
bool texture_manager_init(SDL_Renderer *renderer);

// Destroy all textures
void texture_manager_destroy_all(void);

// ============================================================================
// THREADED LOADING API
// ============================================================================

// Start loading surfaces in background thread
bool texture_manager_start_async_load(void);

// Call from main thread to convert loaded surfaces to textures
// Returns true when all textures are created
bool texture_manager_process_loaded_surfaces(SDL_Renderer *renderer);

// Get loading progress (0.0 to 1.0)
float texture_manager_get_progress(void);

// Check if surface loading is complete
bool texture_manager_surfaces_loaded(void);

// Check if texture creation is complete
bool texture_manager_is_fully_loaded(void);

#endif // TEXTURE_MANAGER_H