# Hangman

### Dependencies
- SDL2
- glad

### Planned Power-ups

### PAC
| Implemented? | Input              | Processing                                               | Module / Reference    | Output                         |
|--------------|--------------------|----------------------------------------------------------|-----------------------|--------------------------------|
| [ ]          | word, lives        | Initialize game state and variables                      | `initHangman()`       | Ready game state               |
| [ ]          | guessed letter     | Check if letter exists in word, update revealed or lives | `processGuess()`      | Updated state (revealed/lives) |
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
function initHangman(word, lives):  //initializes game state.
    return(word,
            ____,   //underscores for the amount of letters in the word.
            guessedLetters[],   //list of letters guessed by the player.
            lives)  //amount of lives of the player
```

processGuess():
```angular2html
procedure processGuess(gameState, guess):   //check guessed letter, update revealed word or lives.
    if guess exists inside gameState[word]:
        for i, character in gameState[word]:    //loop thru the word inside game state, also increment i
            if character == guess:
                gameState[revealedString][i] = character
    else:
        decrement gameState[lives]
    append guess to gameState[guessedLetters]
```

isGameWon():
```angular2html
function isGameWon(gameState):
    //check if gameState[revealedString] doesnt have any underscores
    return true if it doesnt have underscores, else return false
```

isGameOver():
```angular2html
function isGameOver(gameState):
    //check if gameState[lives] is 0, or player won using isGameWon(gameState)
    return false if won, if lives is 0 return true
```

getRevealedWord():
```angular2html
function getRevealedWord(gameState):    //returns revealed word as a string
    return " " + gameState[revealed]
```

getRemainingLives():
```angular2html
function getRemainingLives(gameState):  //returns remaining lives of the player
    return gameState[lives]
```

getGuessedLetters():
```angular2html
function getGuessedLetters(gameState):  //returns guessed letters by the player
    return gameState[guessedLetters]
```

validateGuess():
```angular2html
function validateGuess(gameState, guess):
    //check if guess isnt already guessed, and is a singular letter and not a number
    if guess is inside gameState[guessedLetters]:
        return false
    else if guess is a singular letter and not a number:
        return true
    else return false
```

resetGame():
```angular2html
function resetGame(word, lives):    //reset the gameState
    return initHangman(word, lives)
```


### Main function (core hangman processing):
```angular2html
word = getRandomWordFromFile(random file)
game = initHangman(word, lives)

while not isGameOver(game):
    print word: getRevealedWord(game)
    print Lives: getRemainingLives(game)
    print Guessed: None, or getGuessedLetters(game)

    guess = input a letter from user
    
    if not validateGuess(game, guess):
        print invalid guess
        continue
    
    processGuess(game, guess):

    if isGameWon(game):
        print you won
        break

if not isGameWon(game):
    print you lost! word was <word>
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