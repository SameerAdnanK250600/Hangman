# Hangman

### Dependencies
- SDL2
- glad

### Planned Power-ups

| Implemented? | ID | Power-Up Name     | Effect                                       |
|--------------|----|-------------------|----------------------------------------------|
| [ ]          | 1  | Fill Random Blank | Reveals one correct missing letter           |
| [ ]          | 2  | Extra Life        | +1 life                                      |
| [ ]          | 3  | Reveal All Vowels | Reveals all vowels in word                   |
| [ ]          | 4  | Shield            | Next incorrect guess does NOT reduce a life  |
| [ ]          | 5  | Chance            | 5% bonus power-up, 5% lose life, 90% nothing |

### PAC
| Implemented? | Input          | Processing                             | Function / Module     | Output             |
|--------------|----------------|----------------------------------------|-----------------------|--------------------|
| [ ]          | word, lives    | Initialize game state                  | `initHangman()`       | Ready game state   |
| [ ]          | guessed letter | Update revealed letters or lose a life | `processGuess()`      | Updated game state |
| [ ]          | game state     | Check if all letters guessed           | `isGameWon()`         | Boolean            |
| [ ]          | game state     | Check if win or 0 lives                | `isGameOver()`        | Boolean            |
| [ ]          | game state     | Return revealed word (incl. spaces)    | `getRevealedWord()`   | String             |
| [ ]          | game state     | Return remaining lives                 | `getRemainingLives()` | Integer            |
| [ ]          | game state     | Return guessed letters                 | `getGuessedLetters()` | Array              |
| [ ]          | guess, state   | Validate guess: alphabetical, unused   | `validateGuess()`     | Boolean            |
| [ ]          | word, lives    | Reset game completely                  | `resetGame()`         | New game state     |


### PSEUDOCODE

### `initHangman()`
```angular2html
FUNCTION InitHangman(word, lives)
  DECLARE game: GameState
  
  SET game.word ← LOWERCASE(word)
  SET game.revealed ← ""
  
  FOR each character c IN word DO
    IF c = " " THEN
      APPEND " " to game.revealed
    ELSE
      APPEND "_" to game.revealed
    END IF
  END FOR
  
  SET game.guessed ← empty array
  SET game.lives ← lives
  SET game.numGuessed ← 0
  SET game.shieldActive ← FALSE
  
  RETURN game
END FUNCTION
```

### `processGuess()`
```angular2html
PROCEDURE ProcessGuess(gameState, guess)
  IF guess EXISTS IN gameState.word THEN
    FOR i FROM 1 TO LENGTH(gameState.word) DO
      IF gameState.word[i] = guess THEN
        SET gameState.revealed[i] ← guess
      END IF
    END FOR
  ELSE
    IF gameState.shieldActive = TRUE THEN
        SET gameState.shieldActive ← FALSE
      ELSE
        SET gameState.lives ← gameState.lives - 1
    END IF
  END IF
  
  APPEND guess TO gameState.guessed
  SET gameState.numGuessed ← gameState.numGuessed + 1
END PROCEDURE
```

### `isGameWon()`
```angular2html
FUNCTION IsGameWon(gameState)
  IF revealedWord HAS NO "_" THEN
      RETURN TRUE
    ELSE
      RETURN FALSE
  END IF
END FUNCTION
```

### `isGameOver()`
```angular2html
FUNCTION IsGameOver(gameState)
  IF IsGameWon(gameState) = TRUE THEN
    RETURN TRUE
  ELSE IF gameState.lives = 0 THEN
    RETURN TRUE
  ELSE
    RETURN FALSE
  END IF
END FUNCTION

```

### `getRevealedWord()`
```angular2html
FUNCTION GetRevealedWord(gameState)
  RETURN gameState.revealed
END FUNCTION
```

### `getRemainingLives()`
```angular2html
FUNCTION GetRemainingLives(gameState)
  RETURN gameState.lives
END FUNCTION
```

### `getGuessedLetters()`
```angular2html
FUNCTION GetGuessedLetters(gameState)
  RETURN gameState.guessed
END FUNCTION
```

### `validateGuess()`
```angular2html
FUNCTION ValidateGuess(gameState, guess)
  IF LENGTH(guess) ≠ 1 THEN
    RETURN FALSE
  ELSE IF guess NOT A LETTER THEN
    RETURN FALSE
  ELSE IF guess IN gameState.guessed THEN
    RETURN FALSE
  ELSE
    RETURN TRUE
  END IF
END FUNCTION
```

### `resetGame()`
```angular2html
FUNCTION ResetGame(word, lives)
  RETURN InitHangman(word, lives)
END FUNCTION
```


### `Main function (core hangman processing)`
```angular2html
word ← GetRandomWordFromFile("words.txt")
game ← InitHangman(word, startingLives)

DISPLAY "Welcome to Hangman!"
DISPLAY "Your word has LENGTH(word) characters (spaces auto-filled)"
DISPLAY "The SUPER BLANK is marked with '*' until revealed"

WHILE IsGameOver(game) = FALSE DO
  DISPLAY "Word: " + GetRevealedWord(game)
  DISPLAY "Lives: " + GetRemainingLives(game)
  
  IF game.numGuessed = 0 THEN
    DISPLAY "Guessed: None"
  ELSE
    DISPLAY "Guessed: " + GetGuessedLetters(game)
  END IF
  
  INPUT guess FROM player
  
  IF ValidateGuess(game, guess) = FALSE THEN
    DISPLAY "Invalid or already used. Try again."
    CONTINUE
  END IF
  
  hitSuperBlank ← ProcessGuess(game, guess)
  
  IF hitSuperBlank = TRUE THEN
    CALL ApplyPowerUp(game)
  END IF
  
  IF IsGameWon(game) = TRUE THEN
    DISPLAY "You won! The word was " + word
    BREAK
  END IF
END WHILE

IF IsGameWon(game) = FALSE THEN
  DISPLAY "Game Over! The word was " + word
END IF
```

### `applyPowerUp()`
```text
PROCEDURE ApplyPowerUp(game)
  DISPLAY "You found a SUPER BLANK! Choose a box (1-9) to get a power-up"

  SELECT 3 random box positions to contain power-ups
  ASSIGN 3 power-ups randomly to these positions
  DISPLAY 9 boxes

  INPUT choice FROM player (1-9)

  IF choice CONTAINS a power-up THEN
    power_id ← assigned power-up at choice
    CALL activatePowerUp(game, power_id)
  ELSE
    DISPLAY "Empty box, no power-up"
  END IF
END PROCEDURE
```

### `activatePowerUp()`
```angular2html
PROCEDURE ActivatePowerUp(game, power_id)
  SWITCH power_id
    CASE 1: // Reveal Random Letter
      SELECT random index i WHERE game.revealed[i] = "_"
      SET game.revealed[i] ← game.word[i]
      DISPLAY "A random letter was revealed!"

    CASE 2: // Extra Life
      INCREMENT game.lives
      DISPLAY "+1 Life!"

    CASE 3: // Shield
      SET game.shieldActive ← TRUE
      DISPLAY "Shield activated! Next mistake is free."

    CASE 4: // Reveal All Vowels
      FOR i FROM 1 TO LENGTH(game.word)
        IF game.word[i] IS A VOWEL THEN
          SET game.revealed[i] ← game.word[i]
      DISPLAY "All vowels revealed!"

    CASE 5: // Chance
      roll ← RANDOM(1,100)
      IF roll ≤ 5 THEN
        SELECT random power_id_new (1-5)
        CALL ActivatePowerUp(game, power_id_new)
        DISPLAY "LUCKY! Extra random power-up!"
      ELSE IF roll ≤ 10 THEN
        DECREMENT game.lives
        DISPLAY "Unlucky! Lose 1 life"
      ELSE
        DISPLAY "Nothing happened"
END PROCEDURE
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