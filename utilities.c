//
// Created by Sameer on 04/10/2025.
//
#include <stdio.h>
#include "utilities.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

char* getRandomWordFromFile(const char* fileName) {
    char path[256];
    snprintf(path, sizeof(path), "resources/words/%s.txt", fileName); //look for filename txt file inside resources folder

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    //read all lines into array of strings
    char* lines[256]; //max 256 lines possible
    int lineCount = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), file)) {
        // remove new line characters
        buffer[strcspn(buffer, "\n")] = '\0';

        //store each line
        lines[lineCount] = malloc(strlen(buffer) + 1);
        strcpy(lines[lineCount], buffer);
        lineCount++;

        if (lineCount >= 256) break; //prevent overflow
    }

    fclose(file);

    if (lineCount == 0) return NULL; //file would be empty

    //choose random number
    srand(time(NULL));
    int random_index = rand() % (lineCount);

    //choose and copy the random line to return
    char* selectedLine = malloc(strlen(lines[random_index]) + 1);
    strcpy(selectedLine, lines[random_index]);

    //free all others
    for (int i = 0; i < lineCount; i++) {
        free(lines[i]);
    }

    return selectedLine;
}