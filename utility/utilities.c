//
// Created by Sameer on 04/10/2025.
//
#include <stdio.h>
#include "utilities.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

char *getRandomWordFileName() {
    char *words[] = {
        "animals",
        "continents",
        "countries",
        "fruits",
        "planets",
        "vegetables"
    };

    int word_count = sizeof(words) / sizeof(words[0]);
    int random_index = rand() % word_count;

    return words[random_index];
}

char *getRandomWordFromFile(const char *fileName) {
    char path[256];
    snprintf(path, sizeof(path), "resources/words/%s.txt", fileName);
    //look for filename txt file inside resources folder

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    // read all lines into array of strings
    char *lines[256]; // max 256 lines possible
    int lineCount = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file)) {
        // remove new line characters
        buffer[strcspn(buffer, "\n")] = '\0';

        // store each line
        lines[lineCount] = malloc(strlen(buffer) + 1);
        strcpy(lines[lineCount], buffer);
        lineCount++;

        if (lineCount >= 256) break; // prevent overflow
    }

    fclose(file);

    if (lineCount == 0) return NULL; // file would be empty

    // choose random number
    srand(time(NULL));
    int random_index = rand() % (lineCount);

    // choose and copy the random line to return
    char *selectedLine = malloc(strlen(lines[random_index]) + 1);
    strcpy(selectedLine, lines[random_index]);

    // free all others
    for (int i = 0; i < lineCount; i++) {
        free(lines[i]);
    }

    return selectedLine;
}

char *stringToLower(char *word) {
    // use tolower char function for entire string
    for (int i = 0; word[i] != '\0'; i++) {
        word[i] = tolower(word[i]);
    }
    return word;
}

bool charInArray(const char array[], char ch, int length) {
    // loop thru the array to find the char
    for (int i = 0; i < length; i++) {
        if (array[i] == ch) return true;
    }
    return false;
}

bool appendCharToArray(char array[], char ch, int *length, int maxLength) {
    // leave space for null terminator
    if ((*length) >= maxLength - 1) return false;

    array[*length] = ch;
    (*length)++;
    array[*length] = '\0'; // null terminate
    return true;
}

bool stringHasChar(const char *str, char ch) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == ch) return true;
    }
    return false;
}

void copyStringToUnderscores(char *dest, const char *src) {
    // make blank version of the given string
    int len = strlen(src);
    for (int i = 0; i < len; i++) {
        dest[i] = '_';
    }
    dest[len] = '\0';
}

void revealGuessedLetter(const char *word, char *revealed, char guess) {
    for (int i = 0; word[i] != '\0'; i++) {
        if (word[i] == guess) {
            revealed[i] = guess;
        }
    }
}

bool isWordFullyRevealed(const char *revealed) {
    for (int i = 0; revealed[i] != '\0'; i++) {
        if (revealed[i] == '_') {
            return false; // still letters to guess
        }
    }
    return true; // all letters revealed
}

void resetString(char *str) {
    str[0] = '\0';
}
