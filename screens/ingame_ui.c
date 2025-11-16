#include "ingame_ui.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../game/hangman.h"
#include "../utility/utilities.h"

#define FRAME_FOLDER "resources/textures/ingame_ui/background_frames/"
#define FRAME_PREFIX "background_frame_"
#define FRAME_COUNT 180
#define FRAME_FPS 30.0f

#define LIVES_FOLDER "resources/textures/ingame_ui/"
#define MAX_LIVES 6

typedef struct {
    SDL_Texture* frames[FRAME_COUNT];
    SDL_Texture* livesTextures[MAX_LIVES + 1];

    int frameCount;
    int currentFrame;
    float frameTime;
    float accumulator;

    int winW, winH;

    GameState* game;
    TTF_Font* font;

    // pause system
    bool paused;
    SDL_Texture* pauseTex;

    // letters-used toggle (full-screen images)
    SDL_Texture* lettersTex[2];   // 0 = pull, 1 = pulled (for rendering)
    SDL_Surface* lettersSurf[2];  // surfaces used for pixel-perfect clicking
    bool lettersPulled;

    bool gameOver;               // game finished
    bool waitingAfterGameOver;   // waiting for Enter/ESC

    bool quitToMenu; // NEW: signals main loop to return to menu
} IngameUI;

static IngameUI ui;

// -------------------- helpers --------------------

static bool surface_pixel_opaque(SDL_Surface* surf, int x, int y) {
    if (!surf) return false;
    if (x < 0 || y < 0 || x >= surf->w || y >= surf->h) return false;

    /* Note: assume surf->format is 32-bit (IMG_Load for PNG gives 32-bit with alpha).
       Safer approach would check format->BytesPerPixel and handle accordingly.
       But for your project PNG/IMG_Load should give 32-bit RGBA. */
    Uint32 *pixels = (Uint32*)surf->pixels;
    Uint32 pixel = pixels[y * surf->w + x];
    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel, surf->format, &r, &g, &b, &a);
    return a > 0;
}

// Map mouse coords from window space to surface space (surface may have different resolution)
static void map_mouse_to_surface(int mouseX, int mouseY, int winW, int winH, int surfW, int surfH, int* outX, int* outY) {
    if (winW <= 0) winW = 1;
    if (winH <= 0) winH = 1;
    *outX = mouseX * surfW / winW;
    *outY = mouseY * surfH / winH;
    if (*outX < 0) *outX = 0;
    if (*outY < 0) *outY = 0;
    if (*outX >= surfW) *outX = surfW - 1;
    if (*outY >= surfH) *outY = surfH - 1;
}

// -------------------- init / destroy --------------------

bool ingame_ui_init(SDL_Window* window, SDL_Renderer* renderer, GameState* game) {
    // --- Reset UI state ---
    memset(&ui, 0, sizeof(ui)); // clear everything first
    ui.game = game;

    SDL_GetWindowSize(window, &ui.winW, &ui.winH);

    ui.frameCount = FRAME_COUNT;
    ui.currentFrame = 0;
    ui.accumulator = 0.0f;
    ui.frameTime = 1.0f / FRAME_FPS;

    // Reset gameplay flags
    ui.paused = false;
    ui.gameOver = false;
    ui.waitingAfterGameOver = false;
    ui.quitToMenu = false;
    ui.lettersPulled = false;

    ui.pauseTex = NULL;
    ui.lettersTex[0] = ui.lettersTex[1] = NULL;
    ui.lettersSurf[0] = ui.lettersSurf[1] = NULL;

    char path[512];

    // Load background frames
    for (int i = 0; i < FRAME_COUNT; ++i) {
        snprintf(path, sizeof(path), FRAME_FOLDER FRAME_PREFIX "%03d.png", i + 1);
        SDL_Surface* surf = IMG_Load(path);
        if (!surf) {
            printf("[ERROR] Failed to load frame %d: %s\n", i + 1, path);
            ui.frames[i] = NULL;
            continue;
        }
        ui.frames[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    // Load lives textures (0..6)
    for (int i = 0; i <= MAX_LIVES; ++i) {
        snprintf(path, sizeof(path), LIVES_FOLDER "%d_lives.png", i);
        SDL_Surface* surf = IMG_Load(path);
        if (!surf) {
            printf("[ERROR] Failed to load lives texture %d: %s\n", i, path);
            ui.livesTextures[i] = NULL;
            continue;
        }
        ui.livesTextures[i] = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }

    // Load pause menu texture (optional)
    {
        SDL_Surface* surf = IMG_Load("resources/textures/ingame_ui/pause_menu.png");
        if (!surf) {
            // not fatal, but log
            // printf("[WARN] pause_menu.png not found or failed to load\n");
            ui.pauseTex = NULL;
        } else {
            ui.pauseTex = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
        }
    }

    // Load letters-used images: keep both surface (for pixel checks) and texture (for render)
    {
        SDL_Surface* s0 = IMG_Load("resources/textures/ingame_ui/letters_used_pull.png");
        if (!s0) {
            printf("[WARN] letters_used_pull.png not found: %s\n", IMG_GetError());
            ui.lettersSurf[0] = NULL;
            ui.lettersTex[0] = NULL;
        } else {
            ui.lettersSurf[0] = s0;
            ui.lettersTex[0] = SDL_CreateTextureFromSurface(renderer, s0);
        }

        SDL_Surface* s1 = IMG_Load("resources/textures/ingame_ui/letters_used_pulled.png");
        if (!s1) {
            printf("[WARN] letters_used_pulled.png not found: %s\n", IMG_GetError());
            ui.lettersSurf[1] = NULL;
            ui.lettersTex[1] = NULL;
        } else {
            ui.lettersSurf[1] = s1;
            ui.lettersTex[1] = SDL_CreateTextureFromSurface(renderer, s1);
        }

        // If one of the textures failed, ensure toggle won't crash: use available one for both states
        if (!ui.lettersTex[0] && ui.lettersTex[1]) {
            ui.lettersTex[0] = ui.lettersTex[1];
            ui.lettersSurf[0] = ui.lettersSurf[1];
        } else if (!ui.lettersTex[1] && ui.lettersTex[0]) {
            ui.lettersTex[1] = ui.lettersTex[0];
            ui.lettersSurf[1] = ui.lettersSurf[0];
        }
    }

    // Load font
    ui.font = TTF_OpenFont("resources/font/PixelifySans-SemiBold.ttf", 36);
    if (!ui.font) {
        printf("[ERROR] Failed to load font: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void ingame_ui_destroy() {
    for (int i = 0; i < FRAME_COUNT; ++i) {
        if (ui.frames[i]) { SDL_DestroyTexture(ui.frames[i]); ui.frames[i] = NULL; }
    }
    for (int i = 0; i <= MAX_LIVES; ++i) {
        if (ui.livesTextures[i]) { SDL_DestroyTexture(ui.livesTextures[i]); ui.livesTextures[i] = NULL; }
    }

    if (ui.pauseTex) { SDL_DestroyTexture(ui.pauseTex); ui.pauseTex = NULL; }

    for (int i = 0; i < 2; ++i) {
        if (ui.lettersTex[i]) { SDL_DestroyTexture(ui.lettersTex[i]); ui.lettersTex[i] = NULL; }
        if (ui.lettersSurf[i]) { SDL_FreeSurface(ui.lettersSurf[i]); ui.lettersSurf[i] = NULL; }
    }

    if (ui.font) { TTF_CloseFont(ui.font); ui.font = NULL; }
}

// -------------------- animation update --------------------

void ingame_ui_update(float deltaTime) {
    if (ui.paused) return; // freeze animation while paused

    ui.accumulator += deltaTime;
    while (ui.accumulator >= ui.frameTime) {
        ui.accumulator -= ui.frameTime;
        ui.currentFrame++;
        if (ui.currentFrame >= ui.frameCount) ui.currentFrame = 0;
    }
}

// -------------------- event handling --------------------

void ingame_ui_handle_event(SDL_Event* event) {
    if (!ui.game) return;

    // --- Game finished, wait for Enter or ESC ---
    if (ui.gameOver || ui.waitingAfterGameOver) {
        if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.sym == SDLK_ESCAPE) {
                ui.quitToMenu = true;           // signal main loop
                ui.waitingAfterGameOver = false;
                ui.gameOver = false;
            }
            else if (event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_KP_ENTER) {
                // Restart game
                char* newWord = getRandomWordFromFile("continents");
                if (newWord) {
                    GameState newGame = resetGame(newWord, MAX_LIVES);
                    free(newWord);
                    *(ui.game) = newGame;
                }
                ui.gameOver = false;
                ui.paused = false;
                ui.waitingAfterGameOver = false;
            }
        }
        return;
    }

    // --- Mouse click for letters-used toggle ---
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        SDL_Surface* surf = ui.lettersPulled ? ui.lettersSurf[1] : ui.lettersSurf[0];
        if (surf) {
            int mx = event->button.x;
            int my = event->button.y;
            int surfX, surfY;
            map_mouse_to_surface(mx, my, ui.winW, ui.winH, surf->w, surf->h, &surfX, &surfY);
            if (surface_pixel_opaque(surf, surfX, surfY)) {
                ui.lettersPulled = !ui.lettersPulled;
                return;
            }
        }
    }

    // --- Keyboard input ---
    if (event->type == SDL_KEYDOWN) {
        int key = event->key.keysym.sym;

        // ESC toggles pause
        if (key == SDLK_ESCAPE) {
            ui.paused = !ui.paused;
            return;
        }

        // If paused: only ENTER is allowed to quit
        if (ui.paused) {
            if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                ui.quitToMenu = true;
            }
            return;
        }

        // Not paused: process guesses
        char c = (char)key;
        if (c >= 'a' && c <= 'z') {
            if (!validateGuess(ui.game, c)) return;
            bool hitSuper = processGuess(ui.game, c);
            appendCharToArray(ui.game->guessed, c, &ui.game->numGuessed, MAX_GUESSED);
            if (hitSuper) applyPowerUp(ui.game);
        } else if (c >= 'A' && c <= 'Z') {
            c = (char)tolower(key);
            if (!validateGuess(ui.game, c)) return;
            bool hitSuper = processGuess(ui.game, c);
            appendCharToArray(ui.game->guessed, c, &ui.game->numGuessed, MAX_GUESSED);
            if (hitSuper) applyPowerUp(ui.game);
        }
    }
}

// -------------------- text rendering helpers --------------------

static void render_text_scaled(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                               int x, int y, SDL_Color color, float scale) {
    if (!text || !font) return;

    int baseSize = TTF_FontHeight(font);
    int newSize = (int)(baseSize * scale);
    if (newSize < 4) newSize = 4;

    TTF_Font* scaledFont = TTF_OpenFont("resources/font/MotaPixel-Bold.otf", newSize);
    if (!scaledFont) return;

    SDL_Surface* surf = TTF_RenderText_Blended_Wrapped(scaledFont, text, color, 800);
    TTF_CloseFont(scaledFont);
    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) return;

    SDL_Rect dst = { x, y, 0, 0 };
    SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

static void render_text_fitted(SDL_Renderer* renderer, const char* text,
                               int boundX, int boundW, int y,
                               float baseScale, SDL_Color color) {
    if (!text || !ui.font) return;

    int textW, textH;
    TTF_SizeText(ui.font, text, &textW, &textH);

    float finalScale = baseScale;
    float scaledW = textW * finalScale;

    if (scaledW > boundW) {
        finalScale = boundW / (float)textW;
        scaledW = textW * finalScale;
    }

    int baseSize = TTF_FontHeight(ui.font);
    int dynSize = (int)(baseSize * finalScale);
    if (dynSize < 4) dynSize = 4;

    TTF_Font* f = TTF_OpenFont("resources/font/MotaPixel-Bold.otf", dynSize);
    if (!f) return;

    SDL_Surface* surf = TTF_RenderText_Blended(f, text, color);
    TTF_CloseFont(f);
    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) return;

    SDL_Rect dst;
    dst.w = (int)scaledW;
    dst.h = dynSize;
    dst.x = boundX + (boundW - dst.w) / 2;
    dst.y = y;

    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

static void render_text_scaled_with_shadow(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                                           int x, int y, SDL_Color color, float scale) {
    if (!text || !font) return;

    int baseSize = TTF_FontHeight(font);
    int newSize = (int)(baseSize * scale);
    if (newSize < 4) newSize = 4;

    TTF_Font* scaledFont = TTF_OpenFont("resources/font/MotaPixel-Bold.otf", newSize);
    if (!scaledFont) return;

    SDL_Color shadowColor = {0, 0, 0, 255}; // black shadow
    SDL_Surface* surf = TTF_RenderText_Blended(scaledFont, text, shadowColor);
    if (!surf) {
        TTF_CloseFont(scaledFont);
        return;
    }
    SDL_Texture* shadowTex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    surf = TTF_RenderText_Blended(scaledFont, text, color);
    SDL_Texture* mainTex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    SDL_QueryTexture(mainTex, NULL, NULL, &dst.w, &dst.h);

    // render shadow first (slightly offset)
    SDL_Rect shadowDst = dst;
    shadowDst.x += 2; // shadow offset X
    shadowDst.y += 2; // shadow offset Y
    SDL_RenderCopy(renderer, shadowTex, NULL, &shadowDst);

    // render main text on top
    SDL_RenderCopy(renderer, mainTex, NULL, &dst);

    SDL_DestroyTexture(shadowTex);
    SDL_DestroyTexture(mainTex);
    TTF_CloseFont(scaledFont);
}

void ingame_ui_render(SDL_Renderer* renderer, SDL_Window* window) {
    SDL_GetWindowSize(window, &ui.winW, &ui.winH);
    SDL_RenderClear(renderer);

    // --- Background frame ---
    if (ui.frames[ui.currentFrame]) {
        SDL_Rect full = {0, 0, ui.winW, ui.winH};
        SDL_RenderCopy(renderer, ui.frames[ui.currentFrame], NULL, &full);
    }

    // --- Lives overlay ---
    if (ui.game) {
        int lives = ui.game->lives;
        if (lives < 0) lives = 0;
        if (lives > MAX_LIVES) lives = MAX_LIVES;

        SDL_Texture* livesTex = ui.livesTextures[lives];
        if (livesTex) {
            SDL_Rect r = {0, 0, ui.winW, ui.winH};
            SDL_RenderCopy(renderer, livesTex, NULL, &r);
        }

        SDL_Color white = {255,255,255,255};

        // Fit revealed word inside the reference area (based on 1920x)
        float leftPercent  = 562.0f / 1920.0f;
        float rightPercent = 690.0f / 1920.0f;
        int boundX = (int)(ui.winW * leftPercent);
        int boundW = ui.winW - (int)(ui.winW * leftPercent) - (int)(ui.winW * rightPercent);
        int y = (int)(ui.winH * 0.4);

        render_text_fitted(renderer, ui.game->revealed, boundX, boundW, y, 2.15f, white);

        // --- GAME OVER / YOU WON MESSAGE ---
        bool gameOver = (ui.game->lives == 0 || isWordFullyRevealed(ui.game->revealed));
        if (gameOver) {
            ui.gameOver = true;
            ui.waitingAfterGameOver = true;

            SDL_Color white = {255, 255, 255, 255};

            const char* escLine = "[ESC] to quit";
            const char* enterLine = "[Enter] to play again";

            int textW, textH;
            int spacing = 8;

            // --- Winning message (only if lives > 0) ---
            if (ui.game->lives > 0) {
                const char* title = "YOU WON";

                // 10% down from top
                int titleY = (int)(ui.winH * 0.10f);

                TTF_SizeText(ui.font, title, &textW, &textH);
                int centerX = (ui.winW - 2*(textW)) / 2;

                render_text_scaled_with_shadow(renderer, ui.font, title, centerX, titleY, white, 2.0f);
            }

            // --- ESC / Enter prompts at 25% from bottom ---
            int bottomY = (int)(ui.winH * 0.75);

            // ESC line
            TTF_SizeText(ui.font, escLine, &textW, &textH);
            int escX = (ui.winW - textW) / 2;
            int escY = bottomY - (textH * 2 + spacing);
            render_text_scaled_with_shadow(renderer, ui.font, escLine, escX, escY, white, 1.0f);

            // Enter line
            TTF_SizeText(ui.font, enterLine, &textW, &textH);
            int enterX = (ui.winW - textW) / 2;
            int enterY = escY + textH + spacing;
            render_text_scaled_with_shadow(renderer, ui.font, enterLine, enterX, enterY, white, 1.0f);
        }
    }

    // --- Letters-used button (full-screen) ---
    SDL_Texture* buttonTex = ui.lettersPulled ? ui.lettersTex[1] : ui.lettersTex[0];
    if (buttonTex) {
        SDL_Rect full = {0,0,ui.winW, ui.winH};
        SDL_RenderCopy(renderer, buttonTex, NULL, &full);
    }

    // --- Guessed letters (only if letters pulled) ---
    if (ui.lettersPulled && ui.game) {
        SDL_Color white = {255,255,255,255};

        // Map reference rectangle (1920x1080) to current window
        int boundX = (int)(ui.winW * (1580.0f / 1920.0f));
        int boundW = ui.winW - boundX; // right = 0px
        int boundY = (int)(ui.winH * (218.0f / 1080.0f));
        int boundH = ui.winH - boundY - (int)(ui.winH * (267.0f / 1080.0f));

        // Render "guessed:" on top with shadow, centered
        int textW, textH;
        TTF_SizeText(ui.font, "guessed:", &textW, &textH);
        int centerX = boundX + (boundW - textW) / 2;
        render_text_scaled_with_shadow(renderer, ui.font, "guessed:", centerX, boundY, white, 0.95f);

        // Render guessed letters (centering each line)
        char line[1024] = {0};
        int lineLen = 0;
        int yOffset = textH + 4; // initial y offset after "guessed:"
        int curY = boundY + yOffset;

        for (int i = 0; i < ui.game->numGuessed; ++i) {
            char c = ui.game->guessed[i];
            char buffer[8];
            snprintf(buffer, sizeof(buffer), "%c%s", c, (i < ui.game->numGuessed - 1) ? ", " : "");

            // Tentatively append to line
            char tempLine[1024];
            snprintf(tempLine, sizeof(tempLine), "%s%s", line, buffer);

            // Check width
            TTF_SizeText(ui.font, tempLine, &textW, &textH);
            if (textW > boundW && lineLen > 0) {
                // Render current line with shadow, centered
                TTF_SizeText(ui.font, line, &textW, &textH);
                centerX = boundX + (boundW - textW) / 2;
                render_text_scaled_with_shadow(renderer, ui.font, line, centerX, curY, white, 0.95f);
                curY += textH + 2;
                // Start new line
                snprintf(line, sizeof(line), "%s", buffer);
                lineLen = strlen(buffer);
            } else {
                // Append to current line
                snprintf(line, sizeof(line), "%s", tempLine);
                lineLen += strlen(buffer);
            }
        }

        // Render the last line with shadow, centered
        if (lineLen > 0) {
            TTF_SizeText(ui.font, line, &textW, &textH);
            centerX = boundX + (boundW - textW) / 2;
            render_text_scaled_with_shadow(renderer, ui.font, line, centerX, curY, white, 0.95f);
        }
    }

    // --- Pause overlay on top ---
    if (ui.paused && ui.pauseTex) {
        SDL_Rect full = {0,0,ui.winW, ui.winH};
        SDL_RenderCopy(renderer, ui.pauseTex, NULL, &full);
    }

    SDL_RenderPresent(renderer);
}

bool ingame_ui_is_waiting_after_gameover(void) {
    return ui.waitingAfterGameOver;
}

bool ingame_ui_is_game_over(void) {
    return ui.gameOver;
}

bool ingameUiShouldQuit(void) {
    return ui.quitToMenu;
}

void setShouldQuit(bool b) {
    ui.quitToMenu = b;
}