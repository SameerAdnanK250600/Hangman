#include "about_section.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    SDL_Texture *background;
    TTF_Font *font;
    char *rawText; // keep the full text for re-rendering
    SDL_Texture **lines; // textures for each line
    int *lineWidths;
    int *lineHeights;
    int numLines;
    int winW, winH;
} AboutSection;

static AboutSection about;

// --- Load text file (keep newlines, remove BOM and trailing whitespace) ---
static char *load_text_file(const char *path) {
    FILE *file = fopen(path, "r");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(len + 1);
    fread(buffer, 1, len, file);
    buffer[len] = '\0';
    fclose(file);

    // Remove BOM if present
    char *start = buffer;
    if (len >= 3 &&
        (unsigned char) buffer[0] == 0xEF &&
        (unsigned char) buffer[1] == 0xBB &&
        (unsigned char) buffer[2] == 0xBF) {
        start = buffer + 3;
    }

    // Remove trailing whitespace (but keep internal newlines)
    size_t textLen = strlen(start);
    while (textLen > 0 && (start[textLen - 1] == '\n' || start[textLen - 1] == '\r' || start[textLen - 1] == ' ')) {
        start[textLen - 1] = '\0';
        textLen--;
    }

    // If we skipped BOM, move the cleaned text to the start of buffer
    if (start != buffer) {
        memmove(buffer, start, textLen + 1);
    }

    return buffer;
}

// --- Render text into textures (handles empty lines and resizing) ---
static void render_textures(SDL_Renderer *renderer) {
    // Free old textures if they exist
    if (about.lines) {
        for (int i = 0; i < about.numLines; i++) {
            if (about.lines[i]) {
                SDL_DestroyTexture(about.lines[i]);
            }
        }
        free(about.lines);
        free(about.lineWidths);
        free(about.lineHeights);
    }

    if (!about.rawText) return;

    // Count lines by manually scanning for newlines
    int lineCount = 0;
    char *p = about.rawText;

    // Handle empty string case
    if (*p == '\0') {
        lineCount = 0;
    } else {
        lineCount = 1; // At least one line if text exists
        for (; *p; p++) {
            if (*p == '\n' && *(p + 1) != '\0') {
                // Only count newline if there's content after it
                lineCount++;
            }
        }
    }

    // Allocate arrays
    about.numLines = lineCount;
    about.lines = malloc(sizeof(SDL_Texture *) * lineCount);
    about.lineWidths = malloc(sizeof(int) * lineCount);
    about.lineHeights = malloc(sizeof(int) * lineCount);

    // Render each line
    SDL_Color white = {255, 255, 255, 255};
    int wrapWidth = about.winW - 100; // 50px margin each side
    int fontHeight = TTF_FontHeight(about.font);

    char *lineStart = about.rawText;
    int idx = 0;

    for (char *p = about.rawText; ; p++) {
        // Check if we've reached end of line or end of string
        if (*p == '\n' || *p == '\0') {
            // Calculate line length
            int lineLen = p - lineStart;

            SDL_Surface *surf = NULL;
            if (lineLen == 0) {
                // Empty line → create small surface for spacing
                surf = SDL_CreateRGBSurface(0, 1, fontHeight, 32, 0, 0, 0, 0);
                SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, 0, 0, 0, 0));
            } else {
                // Create a null-terminated copy of the line
                char *lineCopy = malloc(lineLen + 1);
                strncpy(lineCopy, lineStart, lineLen);
                lineCopy[lineLen] = '\0';

                // Render wrapped text
                surf = TTF_RenderText_Blended_Wrapped(about.font, lineCopy, white, wrapWidth);
                free(lineCopy);
            }

            if (surf) {
                about.lines[idx] = SDL_CreateTextureFromSurface(renderer, surf);
                about.lineWidths[idx] = surf->w;
                about.lineHeights[idx] = surf->h;
                SDL_FreeSurface(surf);
            } else {
                about.lines[idx] = NULL;
                about.lineWidths[idx] = 0;
                about.lineHeights[idx] = fontHeight;
            }

            idx++;

            // If we've reached end of string, break
            if (*p == '\0') break;

            // Move to start of next line
            lineStart = p + 1;
        }
    }
}

// --- Initialize about section ---
bool about_section_init(SDL_Window *window, SDL_Renderer *renderer) {
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return false;
    if (TTF_Init() == -1) return false;

    SDL_GetWindowSize(window, &about.winW, &about.winH);

    about.background = IMG_LoadTexture(renderer, "resources/textures/about_section/background.png");
    if (!about.background) return false;

    about.font = TTF_OpenFont("resources/font/PixelifySans-SemiBold.ttf", 36);
    if (!about.font) return false;

    about.rawText = load_text_file("resources/textures/about_section/about.txt");
    if (!about.rawText) return false;

    render_textures(renderer); // initial rendering
    return true;
}

// --- Destroy about section ---
void about_section_destroy() {
    if (about.background) SDL_DestroyTexture(about.background);
    if (about.lines) {
        for (int i = 0; i < about.numLines; i++) {
            if (about.lines[i]) SDL_DestroyTexture(about.lines[i]);
        }
        free(about.lines);
    }
    if (about.lineWidths) free(about.lineWidths);
    if (about.lineHeights) free(about.lineHeights);
    if (about.rawText) free(about.rawText);
    if (about.font) TTF_CloseFont(about.font);

    TTF_Quit();
    IMG_Quit();
}

// --- Render about section ---
void about_section_render(SDL_Renderer *renderer, SDL_Window *window) {
    static int prevW = 0, prevH = 0;
    static int prevFontSize = 0;

    SDL_GetWindowSize(window, &about.winW, &about.winH);

    // --- Determine new font size based on window height ---
    int newFontSize = about.winH / 20; // scale factor (adjust as desired)
    if (newFontSize < 12) newFontSize = 12; // minimum readable size

    bool sizeChanged = (about.winW != prevW || about.winH != prevH);
    bool fontChanged = (newFontSize != prevFontSize);

    // --- If window size changed OR font size changed, rebuild fonts/textures ---
    if (sizeChanged || fontChanged) {
        prevW = about.winW;
        prevH = about.winH;
        prevFontSize = newFontSize;

        // Re-open font at new size
        if (about.font) {
            TTF_CloseFont(about.font);
        }
        about.font = TTF_OpenFont("resources/font/PixelifySans-SemiBold.ttf", newFontSize);
        if (!about.font) {
            printf("ERROR: Could not resize font!\n");
        }

        // Re-render wrapped text
        render_textures(renderer);
    }

    // --- Draw everything ---
    SDL_RenderClear(renderer);

    SDL_Rect fullWin = {0, 0, about.winW, about.winH};
    SDL_RenderCopy(renderer, about.background, NULL, &fullWin);

    int y = (int) (about.winH * 0.3);

    for (int i = 0; i < about.numLines; i++) {
        if (about.lines[i]) {
            SDL_Rect r;
            r.w = about.lineWidths[i];
            r.h = about.lineHeights[i];
            r.x = (about.winW - r.w) / 2;
            r.y = y;
            SDL_RenderCopy(renderer, about.lines[i], NULL, &r);
        }
        y += about.lineHeights[i] + 5;
    }

    SDL_RenderPresent(renderer);
}
