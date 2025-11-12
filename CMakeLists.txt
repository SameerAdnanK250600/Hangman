cmake_minimum_required(VERSION 3.16)
project(Hangman C)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 CONFIG REQUIRED)

add_executable(Hangman main.c
        hangman.h
        hangman.c
        utilities.h
        utilities.c
        resources/app_icon.rc
)

target_link_libraries(Hangman
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
find_package(glad CONFIG REQUIRED)
target_link_libraries(Hangman PRIVATE glad::glad)