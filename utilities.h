//
// Created by Sameer on 04/10/2025.
//

#ifndef HANGMAN_UTILITIES_H
#define HANGMAN_UTILITIES_H
#include <stdbool.h>

char* getRandomWordFromFile(const char* fileName);

char* stringToLower(char* word);

bool charInArray(const char array[], char ch, int length);

bool appendCharToArray(char array[], char ch, int *length, int maxLength);

void replaceCharInString(char *str, char oldCh, char newCh);

bool stringHasChar(const char *str, char ch);

void copyStringToUnderscores(char *dest, const char *src);

void revealGuessedLetter(const char *word, char *revealed, char guess);

bool isWordFullyRevealed(const char *revealed);

void resetString(char *str);

#endif //HANGMAN_UTILITIES_H