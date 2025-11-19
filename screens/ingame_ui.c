#include "ingame_ui.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../game/hangman.h"
#include "../utility/utilities.h"
#include "graphics/texture_manager.h"

#define FRAME_COUNT 180
#define FRAME_FPS 30.0f
#define MAX_LIVES 6
#define POWER_BOX_SIZE 128

typedef struct {
    int frameCount;
    int currentFrame;
    float frameTime;
    float accumulator;

    int winW, winH;

    GameState *game;
    TTF_Font *font;

    bool paused;
    bool lettersPulled;

    bool gameOver;
    bool waitingAfterGameOver;

    bool quitToMenu;

    bool powerUIActive;
    SDL_Rect boxRects[9];
    int selectedBox;
    bool showPowerResult;
    char powerResultText[256];
    float powerResultTimer;
} IngameUI;

static IngameUI ui;

// -------------------- helpers --------------------

static bool surface_pixel_opaque(SDL_Surface *surf, int x, int y) {
    if (!surf) return false;
    if (x < 0 || y < 0 || x >= surf->w || y >= surf->h) return false;
    Uint32 *pixels = (Uint32 *) surf->pixels;
    Uint32 pixel = pixels[y * surf->w + x];
    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel, surf->format, &r, &g, &b, &a);
    return a > 0;
}

static void map_mouse_to_surface(int mouseX, int mouseY, int winW, int winH, int surfW, int surfH, int *outX,
                                 int *outY) {
    if (winW <= 0) winW = 1;
    if (winH <= 0) winH = 1;
    *outX = mouseX * surfW / winW;
    *outY = mouseY * surfH / winH;
    if (*outX < 0) *outX = 0;
    if (*outY < 0) *outY = 0;
    if (*outX >= surfW) *outX = surfW - 1;
    if (*outY >= surfH) *outY = surfH - 1;
}

// -------------------- text rendering --------------------

static void render_text_scaled_with_shadow(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                                           int x, int y, SDL_Color color, float scale) {
    if (!text || !font) return;
    int baseSize = TTF_FontHeight(font);
    int newSize = (int) (baseSize * scale);
    if (newSize < 4) newSize = 4;
    TTF_Font *scaledFont = TTF_OpenFont("resources/font/MotaPixel-Bold.otf", newSize);
    if (!scaledFont) return;

    SDL_Color shadowColor = {0, 0, 0, 255};
    SDL_Surface *surf = TTF_RenderText_Blended(scaledFont, text, shadowColor);
    if (!surf) {
        TTF_CloseFont(scaledFont);
        return;
    }
    SDL_Texture *shadowTex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    surf = TTF_RenderText_Blended(scaledFont, text, color);
    SDL_Texture *mainTex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    SDL_QueryTexture(mainTex, NULL, NULL, &dst.w, &dst.h);

    SDL_Rect shadowDst = dst;
    shadowDst.x += 2;
    shadowDst.y += 2;
    SDL_RenderCopy(renderer, shadowTex, NULL, &shadowDst);
    SDL_RenderCopy(renderer, mainTex, NULL, &dst);

    SDL_DestroyTexture(shadowTex);
    SDL_DestroyTexture(mainTex);
    TTF_CloseFont(scaledFont);
}

// -------------------- power-up wrapper --------------------

void ingame_ui_activate_powerup(GameState *game, int power_id, char *outMessage, size_t size) {
    if (!game || !outMessage || size == 0) return;
    outMessage[0] = 0;
    char buffer[256] = {0};

    switch (power_id) {
        case 1: {
            int len = strlen(game->word);
            int validIndexes[MAX_WORD_LEN];
            int count = 0;
            for (int i = 0; i < len; i++) if (game->revealed[i] == '_') validIndexes[count++] = i;
            if (count > 0) {
                int idx = validIndexes[rand() % count];
                game->revealed[idx] = game->word[idx];
                snprintf(buffer, sizeof(buffer), "Power-Up: A random letter was revealed!");
            } else snprintf(buffer, sizeof(buffer), "Nothing happened.");
        }
        break;
        case 2: game->lives++;
            snprintf(buffer, sizeof(buffer), "Power-Up: +1 Life!");
            break;
        case 3: for (int i = 0; game->word[i]; i++) {
                char c = game->word[i];
                if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')game->revealed[i] = c;
            }
            snprintf(buffer, sizeof(buffer), "Power-Up: All vowels revealed!");
            break;
        case 4: game->shieldActive = 1;
            snprintf(buffer, sizeof(buffer), "Power-Up: Shield activated!");
            break;
        case 5: {
            int roll = rand() % 100 + 1;
            if (roll <= 5) {
                int pid = rand() % 4 + 1;
                char submsg[128];
                ingame_ui_activate_powerup(game, pid, submsg, sizeof(submsg));
                snprintf(buffer, sizeof(buffer), "Bonus random power up!\n%s", submsg);
            } else if (roll <= 10) {
                game->lives--;
                snprintf(buffer, sizeof(buffer), "Lost 1 life.");
            } else {
                snprintf(buffer, sizeof(buffer), "No power-up was obtained.");
            }
        }
        break;
        default: snprintf(buffer, sizeof(buffer), "Empty Box...");
    }
    strncpy(outMessage, buffer, size - 1);
    outMessage[size - 1] = 0;
}

// -------------------- init --------------------

bool ingame_ui_init(SDL_Window *window, SDL_Renderer *renderer, GameState *game) {
    memset(&ui, 0, sizeof(ui));
    ui.game = game;
    SDL_GetWindowSize(window, &ui.winW, &ui.winH);
    ui.frameCount = FRAME_COUNT;
    ui.currentFrame = 0;
    ui.accumulator = 0.0f;
    ui.frameTime = 1.0f / FRAME_FPS;
    ui.paused = false;
    ui.gameOver = false;
    ui.waitingAfterGameOver = false;
    ui.quitToMenu = false;
    ui.lettersPulled = false;

    // Load textures through texture manager
    if (!texture_manager_init_ingame_ui(renderer)) {
        printf("[ERROR] Failed to load ingame UI textures\n");
        return false;
    }

    // Load font (this is UI-specific, not a texture)
    ui.font = TTF_OpenFont("resources/font/PixelifySans-SemiBold.ttf", 36);
    if (!ui.font) {
        printf("[ERROR] Failed to load font\n");
        return false;
    }

    return true;
}

// -------------------- trigger power UI --------------------

void ingame_ui_trigger_powerup(void) {
    ui.powerUIActive = true;
    ui.selectedBox = -1;
    ui.showPowerResult = false;
    ui.powerResultTimer = 0.0f;

    int startX = ui.winW / 2 - 1.5 * POWER_BOX_SIZE;
    int startY = ui.winH / 2 - 1.5 * POWER_BOX_SIZE;

    for (int i = 0; i < 9; i++) {
        ui.boxRects[i].x = startX + (i % 3) * POWER_BOX_SIZE;
        ui.boxRects[i].y = startY + (i / 3) * POWER_BOX_SIZE;
        ui.boxRects[i].w = POWER_BOX_SIZE;
        ui.boxRects[i].h = POWER_BOX_SIZE;
    }
}

// -------------------- destroy --------------------

void ingame_ui_destroy() {
    // Destroy textures through texture manager
    texture_manager_destroy_ingame_ui();

    // Close font
    if (ui.font) TTF_CloseFont(ui.font);
}

// -------------------- update --------------------

void ingame_ui_update(float deltaTime) {
    if (ui.paused) return;
    ui.accumulator += deltaTime;
    while (ui.accumulator >= ui.frameTime) {
        ui.accumulator -= ui.frameTime;
        ui.currentFrame++;
        if (ui.currentFrame >= ui.frameCount) ui.currentFrame = 0;
    }
    if (ui.showPowerResult) {
        ui.powerResultTimer -= deltaTime;
        if (ui.powerResultTimer <= 0) ui.showPowerResult = false;
    }
}

// -------------------- handle events --------------------

void ingame_ui_handle_event(SDL_Event *event) {
    if (!ui.game) return;

    if (ui.gameOver || ui.waitingAfterGameOver) {
        if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.sym == SDLK_ESCAPE) {
                ui.quitToMenu = true;
                ui.waitingAfterGameOver = false;
                ui.gameOver = false;
            } else if (event->key.keysym.sym == SDLK_RETURN || event->key.keysym.sym == SDLK_KP_ENTER) {
                char *newWord = getRandomWordFromFile(getRandomWordFileName());
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

    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        SDL_Surface *surf = ui.lettersPulled ? g_ingameUITextures.lettersSurf[1] : g_ingameUITextures.lettersSurf[0];
        if (surf) {
            int sx, sy;
            map_mouse_to_surface(event->button.x, event->button.y, ui.winW, ui.winH, surf->w, surf->h, &sx, &sy);
            if (surface_pixel_opaque(surf, sx, sy)) {
                ui.lettersPulled = !ui.lettersPulled;
                return;
            }
        }
    }

    if (event->type == SDL_KEYDOWN) {
        int key = event->key.keysym.sym;
        if (key == SDLK_ESCAPE) {
            ui.paused = !ui.paused;
            return;
        }
        if (ui.paused) {
            if (key == SDLK_RETURN || key == SDLK_KP_ENTER) ui.quitToMenu = true;
            return;
        }

        char c = (char) key;
        if (c >= 'a' && c <= 'z') {
            if (!validateGuess(ui.game, c)) return;
            if (processGuess(ui.game, c)) ingame_ui_trigger_powerup();
            appendCharToArray(ui.game->guessed, c, &ui.game->numGuessed, MAX_GUESSED);
        } else if (c >= 'A' && c <= 'Z') {
            c = (char) tolower(key);
            if (!validateGuess(ui.game, c)) return;
            if (processGuess(ui.game, c)) ingame_ui_trigger_powerup();
            appendCharToArray(ui.game->guessed, c, &ui.game->numGuessed, MAX_GUESSED);
        }
    }

    if (ui.powerUIActive && event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mx = event->button.x;
        int my = event->button.y;

        for (int i = 0; i < 9; i++) {
            SDL_Surface *surf = g_ingameUITextures.powerUI_boxSurfs[i];
            if (surf) {
                int sx, sy;
                map_mouse_to_surface(mx, my, ui.winW, ui.winH, surf->w, surf->h, &sx, &sy);
                if (surface_pixel_opaque(surf, sx, sy)) {
                    ui.selectedBox = i;
                    ingame_ui_activate_powerup(ui.game, i + 1, ui.powerResultText, sizeof(ui.powerResultText));
                    ui.showPowerResult = true;
                    ui.powerResultTimer = 3.0f;
                    ui.powerUIActive = false;
                    break;
                }
            }
        }
    }
}

static void render_text_fitted(SDL_Renderer *renderer, const char *text, int boundX, int boundW, int y, float baseScale,
                               SDL_Color color) {
    if (!text || !ui.font) return;
    int textW, textH;
    TTF_SizeText(ui.font, text, &textW, &textH);
    float finalScale = baseScale;
    float scaledW = textW * finalScale;
    if (scaledW > boundW) {
        finalScale = boundW / (float) textW;
        scaledW = textW * finalScale;
    }
    int baseSize = TTF_FontHeight(ui.font);
    int dynSize = (int) (baseSize * finalScale);
    if (dynSize < 4) dynSize = 4;
    TTF_Font *f = TTF_OpenFont("resources/font/MotaPixel-Bold.otf", dynSize);
    if (!f) return;
    SDL_Surface *surf = TTF_RenderText_Blended(f, text, color);
    TTF_CloseFont(f);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) return;
    SDL_Rect dst;
    dst.w = (int) scaledW;
    dst.h = dynSize;
    dst.x = boundX + (boundW - dst.w) / 2;
    dst.y = y;
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
}

// -------------------- render --------------------

void ingame_ui_render(SDL_Renderer *renderer, SDL_Window *window) {
    SDL_GetWindowSize(window, &ui.winW, &ui.winH);
    SDL_RenderClear(renderer);

    // --- Background frame ---
    if (g_ingameUITextures.frames[ui.currentFrame]) {
        SDL_Rect full = {0, 0, ui.winW, ui.winH};
        SDL_RenderCopy(renderer, g_ingameUITextures.frames[ui.currentFrame], NULL, &full);
    }

    // --- Lives overlay ---
    if (ui.game) {
        int lives = ui.game->lives;
        if (lives < 0) lives = 0;
        if (lives > MAX_LIVES) lives = MAX_LIVES;

        SDL_Texture *livesTex = g_ingameUITextures.livesTextures[lives];
        if (livesTex) {
            SDL_Rect r = {0, 0, ui.winW, ui.winH};
            SDL_RenderCopy(renderer, livesTex, NULL, &r);
        }

        SDL_Color white = {255, 255, 255, 255};

        // Fit revealed word inside the reference area (based on 1920x)
        float leftPercent = 562.0f / 1920.0f;
        float rightPercent = 690.0f / 1920.0f;
        int boundX = (int) (ui.winW * leftPercent);
        int boundW = ui.winW - (int) (ui.winW * leftPercent) - (int) (ui.winW * rightPercent);
        int y = (int) (ui.winH * 0.4);

        render_text_fitted(renderer, ui.game->revealed, boundX, boundW, y, 2.15f, white);

        // --- GAME OVER / YOU WON MESSAGE ---
        bool gameOver = (ui.game->lives == 0 || isWordFullyRevealed(ui.game->revealed));
        if (gameOver) {
            ui.gameOver = true;
            ui.waitingAfterGameOver = true;

            const char *escLine = "[ESC] to quit";
            const char *enterLine = "[Enter] to play again";

            int textW, textH;
            int spacing = 8;

            // --- Winning message (only if lives > 0) ---
            if (ui.game->lives > 0) {
                const char *title = "YOU WON";
                int titleY = (int) (ui.winH * 0.10f);
                TTF_SizeText(ui.font, title, &textW, &textH);
                int centerX = (ui.winW - 2 * (textW)) / 2;
                render_text_scaled_with_shadow(renderer, ui.font, title, centerX, titleY, white, 2.0f);
            }

            // --- ESC / Enter prompts at 25% from bottom ---
            int bottomY = (int) (ui.winH * 0.75);

            TTF_SizeText(ui.font, escLine, &textW, &textH);
            int escX = (ui.winW - textW) / 2;
            int escY = bottomY - (textH * 2 + spacing);
            render_text_scaled_with_shadow(renderer, ui.font, escLine, escX, escY, white, 1.0f);

            TTF_SizeText(ui.font, enterLine, &textW, &textH);
            int enterX = (ui.winW - textW) / 2;
            int enterY = escY + textH + spacing;
            render_text_scaled_with_shadow(renderer, ui.font, enterLine, enterX, enterY, white, 1.0f);
        }
    }

    // --- Letters-used button (full-screen) ---
    SDL_Texture *buttonTex = ui.lettersPulled ? g_ingameUITextures.lettersTex[1] : g_ingameUITextures.lettersTex[0];
    if (buttonTex) {
        SDL_Rect full = {0, 0, ui.winW, ui.winH};
        SDL_RenderCopy(renderer, buttonTex, NULL, &full);
    }

    // --- Guessed letters (only if letters pulled) ---
    if (ui.lettersPulled && ui.game) {
        SDL_Color white = {255, 255, 255, 255};

        int boundX = (int) (ui.winW * (1580.0f / 1920.0f));
        int boundW = ui.winW - boundX;
        int boundY = (int) (ui.winH * (218.0f / 1080.0f));

        int textW, textH;
        TTF_SizeText(ui.font, "guessed:", &textW, &textH);
        int centerX = boundX + (boundW - textW) / 2;
        render_text_scaled_with_shadow(renderer, ui.font, "guessed:", centerX, boundY, white, 0.95f);

        char line[1024] = {0};
        int lineLen = 0;
        int yOffset = textH + 4;
        int curY = boundY + yOffset;

        for (int i = 0; i < ui.game->numGuessed; ++i) {
            char c = ui.game->guessed[i];
            char buffer[8];
            snprintf(buffer, sizeof(buffer), "%c%s", c, (i < ui.game->numGuessed - 1) ? ", " : "");

            char tempLine[1024];
            snprintf(tempLine, sizeof(tempLine), "%s%s", line, buffer);

            TTF_SizeText(ui.font, tempLine, &textW, &textH);
            if (textW > boundW && lineLen > 0) {
                TTF_SizeText(ui.font, line, &textW, &textH);
                centerX = boundX + (boundW - textW) / 2;
                render_text_scaled_with_shadow(renderer, ui.font, line, centerX, curY, white, 0.95f);
                curY += textH + 2;
                snprintf(line, sizeof(line), "%s", buffer);
                lineLen = strlen(buffer);
            } else {
                snprintf(line, sizeof(line), "%s", tempLine);
                lineLen += strlen(buffer);
            }
        }

        if (lineLen > 0) {
            TTF_SizeText(ui.font, line, &textW, &textH);
            centerX = boundX + (boundW - textW) / 2;
            render_text_scaled_with_shadow(renderer, ui.font, line, centerX, curY, white, 0.95f);
        }
    }

    // --- Pause overlay on top ---
    if (ui.paused && g_ingameUITextures.pauseTex) {
        SDL_Rect full = {0, 0, ui.winW, ui.winH};
        SDL_RenderCopy(renderer, g_ingameUITextures.pauseTex, NULL, &full);
    }

    // --- Power UI ---
    if (ui.powerUIActive) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_Rect full = {0, 0, ui.winW, ui.winH};
        SDL_RenderFillRect(renderer, &full);

        if (g_ingameUITextures.powerUI_bg) SDL_RenderCopy(renderer, g_ingameUITextures.powerUI_bg, NULL, &full);

        for (int i = 0; i < 9; i++) {
            if (g_ingameUITextures.powerUI_boxes[i])
                SDL_RenderCopy(renderer, g_ingameUITextures.powerUI_boxes[i], NULL, &full);
        }
    }

    // --- Power result text ---
    if (ui.showPowerResult) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100);
        SDL_Rect full = {0, 0, ui.winW, ui.winH};
        SDL_RenderFillRect(renderer, &full);

        SDL_Color white = {255, 255, 255, 255};

        int textW, textH;
        if (TTF_SizeText(ui.font, ui.powerResultText, &textW, &textH) == 0) {
            int x = (ui.winW - textW) / 2;
            int y = (ui.winH - textH) / 2;
            render_text_scaled_with_shadow(renderer, ui.font, ui.powerResultText, x, y, white, 1.5f);
        }
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