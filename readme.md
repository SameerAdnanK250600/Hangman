# Hangman

### Dependencies
- SDL2
- glad

### Planned Power-ups

### PAC
| Implemented? | Input              | Processing                                               | Module / Reference    | Output                         |
|--------------|--------------------|----------------------------------------------------------|-----------------------|--------------------------------|
| [x]          | word, lives        | Initialize game state and variables                      | `initHangman()`       | Ready game state               |
| [x]          | guessed letter     | Check if letter exists in word, update revealed or lives | `processGuess()`      | Updated state (revealed/lives) |
| [ ]          | current game state | Check if all letters guessed                             | `isGameWon()`         | Boolean (win/lose)             |
| [ ]          | current game state | Check if lives are 0 or word complete                    | `isGameOver()`        | Boolean (true/false)           |
| [ ]          | current game state | Return current revealed word with underscores            | `getRevealedWord()`   | Revealed word string           |
| [ ]          | current game state | Return remaining lives count                             | `getRemainingLives()` | Integer (lives left)           |
| [ ]          | current game state | Return list of already guessed letters                   | `getGuessedLetters()` | Character array                |
| [ ]          | guessed letter     | Validate that guess is valid and not repeated            | `validateGuess()`     | Boolean (valid/invalid)        |
| [ ]          | new word, lives    | Reset all values for new round                           | `resetGame()`         | Refreshed game state           |


### PSEUDOCODE

initHangman():
```angular2html
FUNCTION InitHangman(word, lives)
    DECLARE game:GameState

    SET game.word ← word
    SET game.revealed ← ""

    FOR i ← 1 TO LENGTH(word) DO
        SET game.revealed ← game.revealed + "_"
    END FOR

    SET game.guessed ← []       //empty array
    SET game.numGuessed ← 0
    SET game.lives ← lives

    RETURN game
END FUNCTION
```

processGuess():
```angular2html
PROCEDURE ProcessGuess(gameState, guess)
    IF guess EXISTS IN gameState.word THEN
        FOR i ← 1 TO LENGTH(gameState.word) DO
            IF gameState.word[i] = guess THEN
                SET gameState.revealed[i] ← guess
            END IF
        END FOR
    ELSE
        SET gameState.lives ← gameState.lives - 1
    END IF

    CALL appendCharToArray(guess, gameState.guessed)     //this function already exists
    
    SET gameState.numGuessed ← gameState.numGuessed + 1
    END PROCEDURE
```

isGameWon():
```angular2html
FUNCTION IsGameWon(gameState)
    IF isWordFullyRevealed(gameState.revealed) = TRUE THEN
        RETURN TRUE
    ELSE
        RETURN FALSE
    END IF
END FUNCTION
```

isGameOver():
```angular2html
FUNCTION IsGameOver(gameState)
    IF isGameWon(gameState) = TRUE THEN
        RETURN FALSE
    ELSE IF gameState.lives = 0 THEN
        RETURN TRUE
    ELSE
        RETURN FALSE
    END IF
END FUNCTION
```

getRevealedWord():
```angular2html
FUNCTION GetRevealedWord(gameState)
    RETURN " " + gameState.revealed
END FUNCTION
```

getRemainingLives():
```angular2html
FUNCTION GetRemainingLives(gameState)
    RETURN gameState.lives
END FUNCTION
```

getGuessedLetters():
```angular2html
FUNCTION GetGuessedLetters(gameState)
    RETURN gameState.guessed
END FUNCTION
```

validateGuess():
```angular2html
FUNCTION ValidateGuess(gameState, guess)
    IF charInArray(gameState.guessed, guess, gameState.numGuessed) = TRUE THEN
        RETURN FALSE
    ELSE IF (LENGTH(guess) = 1) AND (guess IS A LETTER) THEN
        RETURN TRUE
    ELSE
        RETURN FALSE
    END IF
END FUNCTION
```

resetGame():
```angular2html
FUNCTION ResetGame(word, lives)
    RETURN InitHangman(word, lives)
END FUNCTION
```


### Main function (core hangman processing):
```angular2html
word ← GetRandomWordFromFile("random_file")
game ← InitHangman(word, lives)

WHILE IsGameOver(game) = FALSE DO
    PRINT "Word: " + GetRevealedWord(game)
    PRINT "Lives: " + GetRemainingLives(game)

    IF game.numGuessed = 0 THEN
        PRINT "Guessed: None"
    ELSE
        PRINT "Guessed: " + GetGuessedLetters(game)
    END IF

    INPUT "Enter a letter: " → guess

    IF ValidateGuess(game, guess) = FALSE THEN
        PRINT "Invalid guess. Try again."
        CONTINUE
    END IF

    CALL ProcessGuess(game, guess)

    IF IsGameWon(game) = TRUE THEN
        PRINT "You won!"
        BREAK
    END IF
END WHILE

IF IsGameWon(game) = FALSE THEN
    PRINT "You lost! The word was " + word
END IF
```

## Helper functions:
- getRandomWordFromFile(filename)
  - returns random line from the specified file
- stringToLower(word)
  - returns provided string as fully lowercased
- charInArray(array, char, lengthOfArray)
  - returns true if character exists inside given array, else returns false
- appendCharToArray(array, char, lengthOfArray, maxLength)
  - adds the given character to the given array
- replaceCharInString(string, oldchar, newchar)
  - replaced old character with new character in provided string
- stringHasChar(string, char)
  - returns true if provided string has provided character, else false
- copyStringToUnderscores(destinationString, sourceString)
  - makes a copy of sourceString but completely underscores inside the destinationString
- revealGuessedLetter(originalWord, revealedWord, guessedChar)
  - replaces the underscore inside revealedWord with guessed Character according to orignal word
- isWordFullyRevealed(revealedWord)
  - checks if the string has underscores left, if not the word is completely revealed, returns true
- resetString(stringPointer)
  - resets string by setting the first letter to a terminator