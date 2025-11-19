#include "main_menu.h"
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#include "graphics/texture_manager.h"

typedef struct {
    int mouseX, mouseY;
    int winW, winH;
} MainMenu;

static MainMenu menu;

// --- Pixel-perfect hover check ---
static bool is_mouse_over(SDL_Surface *surf, int mouseX, int mouseY) {
    if (!surf) return false;

    if (mouseX < 0 || mouseY < 0 || mouseX >= surf->w || mouseY >= surf->h)
        return false;

    Uint32 *pixels = (Uint32 *) surf->pixels;
    Uint32 pixel = pixels[mouseY * surf->w + mouseX];

    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel, surf->format, &r, &g, &b, &a);

    return a > 0;
}

// --- Map mouse coordinates to 1920x1080 image space ---
static void map_mouse_to_image(int mouseX, int mouseY, int winW, int winH, int *outX, int *outY) {
    *outX = mouseX * 1920 / winW;
    *outY = mouseY * 1080 / winH;

    // Clamp to image bounds
    if (*outX < 0) *outX = 0;
    if (*outY < 0) *outY = 0;
    if (*outX >= 1920) *outX = 1919;
    if (*outY >= 1080) *outY = 1079;
}

// --- Initialize main menu ---
bool main_menu_init(SDL_Window *window, SDL_Renderer *renderer) {
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("[ERROR] SDL_image init failed: %s\n", IMG_GetError());
        return false;
    }

    SDL_GetWindowSize(window, &menu.winW, &menu.winH);

    return texture_manager_init_main_menu(renderer);
}

// --- Destroy textures ---
void main_menu_destroy() {
    texture_manager_destroy_main_menu();

    IMG_Quit();
}

// --- Handle events ---
MenuAction main_menu_handle_event(SDL_Window *window, SDL_Renderer *renderer, SDL_Event *e) {
    if (e->type == SDL_MOUSEMOTION) {
        int mx = e->motion.x;
        int my = e->motion.y;
        map_mouse_to_image(mx, my, menu.winW, menu.winH, &menu.mouseX, &menu.mouseY);
    }

    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        if (is_mouse_over(g_mainMenuTextures.startSurface, menu.mouseX, menu.mouseY))
            return MENU_START;
        if (is_mouse_over(g_mainMenuTextures.aboutSurface, menu.mouseX, menu.mouseY))
            return MENU_ABOUT;
    }

    if (e->type == SDL_WINDOWEVENT && e->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        menu.winW = e->window.data1;
        menu.winH = e->window.data2;
    }

    return MENU_NONE;
}

// --- Render main menu ---
void main_menu_render(SDL_Renderer *renderer, SDL_Window *window) {
    SDL_RenderClear(renderer);

    // Update window size (in case it changed while we were in another screen)
    SDL_GetWindowSize(window, &menu.winW, &menu.winH);

    // Stretch images to fill the window
    SDL_Rect fullWin = {0, 0, menu.winW, menu.winH};
    SDL_RenderCopy(renderer, g_mainMenuTextures.background, NULL, &fullWin);

    if (is_mouse_over(g_mainMenuTextures.startSurface, menu.mouseX, menu.mouseY))
        SDL_RenderCopy(renderer, g_mainMenuTextures.startHover, NULL, &fullWin);
    else
        SDL_RenderCopy(renderer, g_mainMenuTextures.start, NULL, &fullWin);

    if (is_mouse_over(g_mainMenuTextures.aboutSurface, menu.mouseX, menu.mouseY))
        SDL_RenderCopy(renderer, g_mainMenuTextures.aboutHover, NULL, &fullWin);
    else
        SDL_RenderCopy(renderer, g_mainMenuTextures.about, NULL, &fullWin);

    SDL_RenderPresent(renderer);
}
